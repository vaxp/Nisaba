#include <cassert>
#include <iostream>
#include <cmath>
#include <string>
#include <vector>
#include <cstring>

#include "orion/orion.hpp"
#include "orion/physics/scene/physics_scene.hpp"
#include "orion/physics/collision/box_shape.hpp"
#include "orion/physics/collision/sphere_shape.hpp"
#include "orion/physics/collision/plane_shape.hpp"

using namespace orion;
using namespace orion::physics;

static std::string base64_encode(const uint8_t* data, size_t len) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((len + 2) / 3) * 4);

    size_t i = 0;
    while (i < len) {
        uint32_t b0 = data[i++];
        uint32_t b1 = (i < len) ? data[i++] : 0;
        uint32_t b2 = (i < len) ? data[i++] : 0;

        uint32_t triple = (b0 << 16) | (b1 << 8) | b2;

        out.push_back(tbl[(triple >> 18) & 0x3F]);
        out.push_back(tbl[(triple >> 12) & 0x3F]);
        out.push_back((i > len + 1) ? '=' : tbl[(triple >> 6) & 0x3F]);
        out.push_back((i > len) ? '=' : tbl[triple & 0x3F]);
    }
    return out;
}

// ---------------------------------------------------------------------------
// Test 1: Physics-to-Render Bridge Registration & Shape Factory
// ---------------------------------------------------------------------------
void test_physics_bridge_registration() {
    std::cout << "[Test 1] PhysicsRenderBridge Registration & Auto Shape Binding..." << std::endl;

    PhysicsScene physics_scene;
    Scene3D render_scene;
    PhysicsRenderBridge bridge;

    // Create a dynamic box body
    auto box_shape = std::make_shared<BoxShape>(Vec3(1.0f, 2.0f, 0.5f));
    BodyCreationSettings box_settings;
    box_settings.position = Vec3(0.0f, 5.0f, 0.0f);
    box_settings.shape = box_shape.get();
    box_settings.motion_type = MotionType::Dynamic;
    BodyID box_id = physics_scene.create_body(box_settings);

    // Create a dynamic sphere body
    auto sphere_shape = std::make_shared<SphereShape>(1.5f);
    BodyCreationSettings sphere_settings;
    sphere_settings.position = Vec3(10.0f, 2.0f, -5.0f);
    sphere_settings.shape = sphere_shape.get();
    sphere_settings.motion_type = MotionType::Dynamic;
    BodyID sphere_id = physics_scene.create_body(sphere_settings);

    auto gold_mat = PbrMaterial::create_gold();
    auto box_renderable = bridge.create_and_bind(physics_scene, box_id, gold_mat, &render_scene);
    auto sphere_renderable = bridge.create_and_bind(physics_scene, sphere_id, nullptr, &render_scene);

    assert(box_renderable != nullptr);
    assert(sphere_renderable != nullptr);
    assert(bridge.binding_count() == 2);
    assert(bridge.has_binding(box_id));
    assert(bridge.has_binding(sphere_id));
    assert(render_scene.renderable_count() == 2);

    // Verify initial transforms match body positions
    assert(std::abs(box_renderable->transform.m[3] - 0.0f) < 1e-4f);
    assert(std::abs(box_renderable->transform.m[7] - 5.0f) < 1e-4f);
    assert(std::abs(box_renderable->transform.m[11] - 0.0f) < 1e-4f);

    assert(std::abs(sphere_renderable->transform.m[3] - 10.0f) < 1e-4f);
    assert(std::abs(sphere_renderable->transform.m[7] - 2.0f) < 1e-4f);
    assert(std::abs(sphere_renderable->transform.m[11] - (-5.0f)) < 1e-4f);

    // Verify world bounds are initialized
    assert(box_renderable->get_world_bounds().is_valid());
    assert(sphere_renderable->get_world_bounds().is_valid());

    // Test unbind
    bridge.unbind(box_id);
    assert(bridge.binding_count() == 1);
    assert(!bridge.has_binding(box_id));
    assert(bridge.has_binding(sphere_id));

    std::cout << "  -> Registration and Auto-Shape Binding PASSED!" << std::endl;
}

// ---------------------------------------------------------------------------
// Test 2: Physics Simulation Lockstep Synchronization
// ---------------------------------------------------------------------------
void test_physics_simulation_sync() {
    std::cout << "[Test 2] Physics Simulation Lockstep Synchronization..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene physics_scene(settings);

    // Floor plane at Y = 0
    auto floor_shape = std::make_shared<PlaneShape>();
    BodyCreationSettings floor_settings;
    floor_settings.position = Vec3::zero();
    floor_settings.shape = floor_shape.get();
    floor_settings.motion_type = MotionType::Static;
    BodyID floor_id = physics_scene.create_body(floor_settings);
    (void)floor_id;

    // Dynamic box starting at Y = 10
    auto box_shape = std::make_shared<BoxShape>(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_settings;
    box_settings.position = Vec3(0.0f, 10.0f, 0.0f);
    box_settings.shape = box_shape.get();
    box_settings.motion_type = MotionType::Dynamic;
    BodyID box_id = physics_scene.create_body(box_settings);

    PhysicsRenderBridge bridge;
    auto box_renderable = bridge.create_and_bind(physics_scene, box_id);

    // Initial position verify
    assert(std::abs(box_renderable->transform.m[7] - 10.0f) < 1e-4f);

    // Step physics for 60 frames (1 second)
    const float dt = 1.0f / 60.0f;
    for (int step = 0; step < 60; ++step) {
        physics_scene.step(dt);
        bridge.sync(physics_scene, 1.0f);
    }

    const Body* body = physics_scene.get_body(box_id);
    assert(body != nullptr);

    // Renderable transform must precisely match simulated body position
    const Vec3 body_pos = body->get_position();
    assert(std::abs(box_renderable->transform.m[3] - body_pos.x) < 1e-4f);
    assert(std::abs(box_renderable->transform.m[7] - body_pos.y) < 1e-4f);
    assert(std::abs(box_renderable->transform.m[11] - body_pos.z) < 1e-4f);

    // Box fell down due to gravity (should be lower than 10.0f)
    assert(body_pos.y < 9.0f);

    // World bounding box updated dynamically
    assert(box_renderable->get_world_bounds().contains(body_pos));

    std::cout << "  -> Lockstep physics sync: Box fell from Y=10.0 to Y=" << body_pos.y
              << " with matching renderable matrix PASSED!" << std::endl;
}

// ---------------------------------------------------------------------------
// Test 3: Sub-Frame Temporal Interpolation
// ---------------------------------------------------------------------------
void test_physics_bridge_interpolation() {
    std::cout << "[Test 3] Sub-Frame Temporal Interpolation..." << std::endl;

    PhysicsScene physics_scene;
    auto box_shape = std::make_shared<BoxShape>(Vec3(1.0f, 1.0f, 1.0f));
    BodyCreationSettings settings;
    settings.position = Vec3(0.0f, 10.0f, 0.0f);
    settings.shape = box_shape.get();
    settings.motion_type = MotionType::Kinematic;
    BodyID body_id = physics_scene.create_body(settings);

    PhysicsRenderBridge bridge;
    auto renderable = bridge.create_and_bind(physics_scene, body_id);

    // Initial state: Y = 10.0
    bridge.record_physics_step(physics_scene);

    // Move body directly to Y = 20.0
    Body* body = physics_scene.get_body(body_id);
    body->set_position(Vec3(0.0f, 20.0f, 0.0f));

    // Interpolate at alpha = 0.5 (halfway)
    bridge.sync(physics_scene, 0.5f);
    assert(std::abs(renderable->transform.m[7] - 15.0f) < 1e-3f);

    // Interpolate at alpha = 0.25 (quarterway)
    bridge.sync(physics_scene, 0.25f);
    assert(std::abs(renderable->transform.m[7] - 12.5f) < 1e-3f);

    // Interpolate at alpha = 1.0 (full step)
    bridge.sync(physics_scene, 1.0f);
    assert(std::abs(renderable->transform.m[7] - 20.0f) < 1e-3f);

    std::cout << "  -> Sub-frame interpolation (alpha 0.5 -> Y=15, alpha 1.0 -> Y=20) PASSED!" << std::endl;
}

// ---------------------------------------------------------------------------
// Test 4: glTF 2.0 Ingestion with Embedded Data URI
// ---------------------------------------------------------------------------
void test_gltf_json_parsing_embedded() {
    std::cout << "[Test 4] glTF 2.0 JSON Ingestion with Embedded Data URI..." << std::endl;

    // Construct raw binary buffer containing 3 triangle vertices & 3 indices
    // Vertex format: Position (3 floats = 12 bytes), Normal (3 floats = 12 bytes), UV (2 floats = 8 bytes)
    // 3 vertices * 32 bytes = 96 bytes
    // Indices: 3 uint16_t = 6 bytes (padded to 8 bytes)
    // Total buffer size = 104 bytes

    struct TestVertex {
        float px, py, pz;
        float nx, ny, nz;
        float u, v;
    };

    const TestVertex raw_verts[3] = {
        { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f },
        { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f }
    };

    const uint16_t raw_indices[4] = { 0, 1, 2, 0 }; // 4 uint16 = 8 bytes

    std::vector<uint8_t> buffer_bytes(sizeof(raw_verts) + sizeof(raw_indices));
    std::memcpy(buffer_bytes.data(), raw_verts, sizeof(raw_verts));
    std::memcpy(buffer_bytes.data() + sizeof(raw_verts), raw_indices, sizeof(raw_indices));

    std::string b64 = base64_encode(buffer_bytes.data(), buffer_bytes.size());

    std::string gltf_json = R"({
        "asset": { "version": "2.0" },
        "buffers": [
            { "byteLength": )" + std::to_string(buffer_bytes.size()) + R"(, "uri": "data:application/octet-stream;base64,)" + b64 + R"(" }
        ],
        "bufferViews": [
            { "buffer": 0, "byteOffset": 0, "byteLength": 96, "byteStride": 32 },
            { "buffer": 0, "byteOffset": 96, "byteLength": 8 }
        ],
        "accessors": [
            { "bufferView": 0, "byteOffset": 0, "componentType": 5126, "count": 3, "type": "VEC3" },
            { "bufferView": 0, "byteOffset": 12, "componentType": 5126, "count": 3, "type": "VEC3" },
            { "bufferView": 0, "byteOffset": 24, "componentType": 5126, "count": 3, "type": "VEC2" },
            { "bufferView": 1, "byteOffset": 0, "componentType": 5123, "count": 3, "type": "SCALAR" }
        ],
        "materials": [
            {
                "name": "RedMetallic",
                "pbrMetallicRoughness": {
                    "baseColorFactor": [0.8, 0.1, 0.2, 1.0],
                    "metallicFactor": 0.95,
                    "roughnessFactor": 0.15
                },
                "doubleSided": true
            }
        ],
        "meshes": [
            {
                "primitives": [
                    {
                        "attributes": {
                            "POSITION": 0,
                            "NORMAL": 1,
                            "TEXCOORD_0": 2
                        },
                        "indices": 3,
                        "material": 0
                    }
                ]
            }
        ],
        "nodes": [
            {
                "mesh": 0,
                "translation": [3.0, 4.0, -5.0],
                "scale": [2.0, 2.0, 2.0]
            }
        ],
        "scenes": [
            { "nodes": [0] }
        ],
        "scene": 0
    })";

    GltfLoadResult result = GltfLoader::load_from_json(gltf_json);
    assert(result.success);
    assert(result.meshes.size() == 1);
    assert(result.materials.size() == 1);
    assert(result.renderables.size() == 1);

    // Validate Material
    const auto& mat = result.materials[0];
    assert(std::abs(mat->base_color_factor.x - 0.8f) < 1e-4f);
    assert(std::abs(mat->base_color_factor.y - 0.1f) < 1e-4f);
    assert(std::abs(mat->metallic_factor - 0.95f) < 1e-4f);
    assert(std::abs(mat->roughness_factor - 0.15f) < 1e-4f);
    assert(mat->double_sided == true);

    // Validate Mesh & Submesh
    const auto& mesh = result.meshes[0];
    assert(mesh->vertices.size() == 3);
    assert(mesh->indices.size() == 3);
    assert(mesh->submesh_count() == 1);

    // Validate Vertex positions
    assert(std::abs(mesh->vertices[1].position.x - 1.0f) < 1e-4f);
    assert(std::abs(mesh->vertices[2].position.y - 1.0f) < 1e-4f);

    // Validate Renderable transform and bounds
    const auto& rend = result.renderables[0];
    assert(std::abs(rend->transform.m[3] - 3.0f) < 1e-4f);
    assert(std::abs(rend->transform.m[7] - 4.0f) < 1e-4f);
    assert(std::abs(rend->transform.m[11] - (-5.0f)) < 1e-4f);
    assert(rend->get_world_bounds().is_valid());

    std::cout << "  -> Embedded glTF parsing & material/mesh extraction PASSED!" << std::endl;
}

// ---------------------------------------------------------------------------
// Test 5: Binary GLB Container Ingestion
// ---------------------------------------------------------------------------
void test_glb_container_parsing() {
    std::cout << "[Test 5] Binary GLB Container Ingestion..." << std::endl;

    // Prepare JSON chunk
    std::string json_str = R"({
        "asset": { "version": "2.0" },
        "buffers": [ { "byteLength": 36 } ],
        "bufferViews": [ { "buffer": 0, "byteOffset": 0, "byteLength": 36 } ],
        "accessors": [
            { "bufferView": 0, "byteOffset": 0, "componentType": 5126, "count": 3, "type": "VEC3" }
        ],
        "meshes": [
            { "primitives": [ { "attributes": { "POSITION": 0 } } ] }
        ],
        "nodes": [ { "mesh": 0 } ],
        "scenes": [ { "nodes": [0] } ],
        "scene": 0
    })";

    // Align JSON chunk to 4 bytes with spaces
    while (json_str.size() % 4 != 0) {
        json_str.push_back(' ');
    }

    // Binary chunk: 3 vertices * 3 floats = 9 floats = 36 bytes
    float bin_data[9] = {
        0.0f, 0.0f, 0.0f,
        2.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f
    };

    const uint32_t json_chunk_len = static_cast<uint32_t>(json_str.size());
    const uint32_t bin_chunk_len = sizeof(bin_data);
    const uint32_t total_glb_len = 12 + (8 + json_chunk_len) + (8 + bin_chunk_len);

    std::vector<uint8_t> glb_bytes(total_glb_len);
    size_t offset = 0;

    // Header: magic (0x46546C67), version (2), total length
    const uint32_t magic = 0x46546C67;
    const uint32_t version = 2;
    std::memcpy(glb_bytes.data() + offset, &magic, 4); offset += 4;
    std::memcpy(glb_bytes.data() + offset, &version, 4); offset += 4;
    std::memcpy(glb_bytes.data() + offset, &total_glb_len, 4); offset += 4;

    // Chunk 0: JSON
    const uint32_t json_type = 0x4E4F534A; // "JSON"
    std::memcpy(glb_bytes.data() + offset, &json_chunk_len, 4); offset += 4;
    std::memcpy(glb_bytes.data() + offset, &json_type, 4); offset += 4;
    std::memcpy(glb_bytes.data() + offset, json_str.data(), json_chunk_len); offset += json_chunk_len;

    // Chunk 1: BIN
    const uint32_t bin_type = 0x004E4942; // "BIN\0"
    std::memcpy(glb_bytes.data() + offset, &bin_chunk_len, 4); offset += 4;
    std::memcpy(glb_bytes.data() + offset, &bin_type, 4); offset += 4;
    std::memcpy(glb_bytes.data() + offset, bin_data, bin_chunk_len); offset += bin_chunk_len;

    GltfLoadResult result = GltfLoader::load_from_memory(glb_bytes);
    assert(result.success);
    assert(result.meshes.size() == 1);
    assert(result.renderables.size() == 1);

    const auto& mesh = result.meshes[0];
    assert(mesh->vertices.size() == 3);
    assert(std::abs(mesh->vertices[1].position.x - 2.0f) < 1e-4f);
    assert(std::abs(mesh->vertices[2].position.y - 3.0f) < 1e-4f);

    std::cout << "  -> Binary GLB container chunk extraction PASSED!" << std::endl;
}

// ---------------------------------------------------------------------------
// Test 6: End-to-End Pipeline (glTF Ingestion + Physics Bridge + 64-Bit Rendering)
// ---------------------------------------------------------------------------
void test_end_to_end_pipeline() {
    std::cout << "[Test 6] End-to-End Pipeline (glTF + Physics + 64-bit PBR Renderer)..." << std::endl;

    // 1. Create procedural physical scene
    PhysicsSettings phys_settings;
    phys_settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene physics_scene(phys_settings);

    auto box_shape = std::make_shared<BoxShape>(Vec3(1.0f, 1.0f, 1.0f));
    BodyCreationSettings body_settings;
    body_settings.position = Vec3(0.0f, 15.0f, 0.0f);
    body_settings.shape = box_shape.get();
    body_settings.motion_type = MotionType::Dynamic;
    BodyID dynamic_body_id = physics_scene.create_body(body_settings);

    // 2. Setup 3D Scene and Bridge
    auto scene3d = std::make_shared<Scene3D>();
    PhysicsRenderBridge bridge;
    auto renderable = bridge.create_and_bind(physics_scene, dynamic_body_id, PbrMaterial::create_gold(), scene3d.get());

    // Add Directional Sun Light
    auto sun_light = std::make_shared<Light3D>(
        Light3D::create_directional(
            Vec3(-0.5f, -1.0f, -0.3f).normalized(),
            Vec3(1.0f, 0.95f, 0.85f),
            3.0f
        )
    );
    scene3d->add_light(*sun_light);

    // 3. Setup Camera & View
    auto camera = std::make_shared<Camera3D>();
    camera->set_look_at(Vec3(0.0f, 5.0f, 25.0f), Vec3(0.0f, 5.0f, 0.0f), Vec3::up());
    camera->set_perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);

    View3D view;
    view.set_scene(scene3d);
    view.set_camera(camera);
    view.set_viewport(0.0f, 0.0f, 1280.0f, 720.0f);

    // 4. Setup Renderer
    Renderer3D renderer;

    // Step physics & sync renderable
    for (int i = 0; i < 30; ++i) {
        physics_scene.step(1.0f / 60.0f);
        bridge.sync(physics_scene);
    }

    // Render frame
    renderer.render(view);
    const auto& stats = renderer.get_stats();

    assert(stats.total_draw_calls > 0);
    assert(stats.total_triangles > 0);
    assert(stats.visible_objects >= 1);

    std::cout << "  -> Rendered frame: " << stats.total_draw_calls << " draw calls, "
              << stats.total_triangles << " triangles, "
              << stats.visible_objects << " visible renderables PASSED!" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D Pipeline: Phase 6 glTF & Physics Tests" << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_physics_bridge_registration();
    test_physics_simulation_sync();
    test_physics_bridge_interpolation();
    test_gltf_json_parsing_embedded();
    test_glb_container_parsing();
    test_end_to_end_pipeline();

    std::cout << "\n>>> ALL PHASE 6 GLTF & PHYSICS BRIDGE TESTS PASSED! <<<" << std::endl;
    return 0;
}

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cstring>
#include <algorithm>
#include <filesystem>

#include "nisaba/gpu/gpu_device.hpp"
#include "orion/orion.hpp"
#include "nisaba/image/image_io.hpp"

using namespace nisaba;
using namespace orion;
using namespace orion::physics;

int main(int argc, char** argv) {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D Real-Time glTF/GLB Hardware Showcase" << std::endl;
    std::cout << "==========================================================" << std::endl;

    // 1. Resolve 3D Model File Path
    std::string model_path = "third_party/models/Lantern/Lantern.glb";
    if (argc > 1 && argv[1] != nullptr && std::strlen(argv[1]) > 0) {
        model_path = argv[1];
    }

    if (!std::filesystem::exists(model_path)) {
        // Try fallback search
        const std::vector<std::string> fallbacks = {
            "third_party/models/Sponza/sponza.glb",
            "third_party/models/nativeStaticScene.glb"
        };
        for (const auto& fb : fallbacks) {
            if (std::filesystem::exists(fb)) {
                model_path = fb;
                break;
            }
        }
    }

    std::cout << "[Model] Loading 3D model asset: " << model_path << std::endl;
    if (!std::filesystem::exists(model_path)) {
        std::cerr << "[Error] Could not locate 3D model file: " << model_path << std::endl;
        return 1;
    }

    // 2. Ingest Model via Sovereign GltfLoader
    auto load_result = GltfLoader::load_from_file(model_path);
    if (!load_result.success) {
        std::cerr << "[Error] Failed to load glTF/GLB asset: " << load_result.error_message << std::endl;
        return 1;
    }

    std::cout << "[Model] Successfully loaded glTF asset hierarchy:" << std::endl;
    std::cout << "  - Meshes Loaded:       " << load_result.meshes.size() << std::endl;
    std::cout << "  - Materials Loaded:    " << load_result.materials.size() << std::endl;
    std::cout << "  - Textures Loaded:     " << load_result.textures.size() << std::endl;
    std::cout << "  - Renderable Nodes:    " << load_result.renderables.size() << std::endl;

    for (size_t i = 0; i < load_result.textures.size(); ++i) {
        const auto& t = load_result.textures[i];
        if (t) {
            std::cout << "    * Texture[" << i << "]: " << t->width << "x" << t->height
                      << " (" << t->channels << " channels, "
                      << (t->pixels.size() / 1024) << " KB)" << std::endl;
        }
    }

    // 3. Compute Scene AABB Bounding Box
    Vec3 min_pt( 1e9f,  1e9f,  1e9f);
    Vec3 max_pt(-1e9f, -1e9f, -1e9f);
    size_t total_verts = 0;
    size_t total_tris = 0;

    for (const auto& r : load_result.renderables) {
        if (!r || !r->mesh) continue;
        const auto& mesh = *r->mesh;
        total_verts += mesh.vertices.size();
        total_tris += mesh.indices.size() / 3;

        for (const auto& v : mesh.vertices) {
            const Vec3 p = r->transform.transform_point(v.position);
            min_pt = min_pt.min(p);
            max_pt = max_pt.max(p);
        }
    }

    if (total_verts == 0) {
        std::cerr << "[Error] Loaded 3D model contains zero vertices!" << std::endl;
        return 1;
    }

    const Vec3 center = (min_pt + max_pt) * 0.5f;
    const Vec3 extents = max_pt - min_pt;
    float radius = std::max({extents.x, extents.y, extents.z}) * 0.5f;
    if (radius < 1e-4f) radius = 1.0f;

    std::cout << "  - Total Geometry:      " << total_verts << " vertices, " << total_tris << " triangles" << std::endl;
    std::cout << "  - Bounds Center:       (" << center.x << ", " << center.y << ", " << center.z << ")" << std::endl;
    std::cout << "  - Bounds Extents:      (" << extents.x << ", " << extents.y << ", " << extents.z << ")" << std::endl;
    std::cout << "  - Bounding Radius:     " << radius << std::endl;

    // 4. Initialize Hardware GPU Context
    auto device = nisaba::gpu::GpuDevice::create();
    if (!device || !device->is_valid()) {
        std::cerr << "[Error] Hardware GPU device not available on this platform." << std::endl;
        return 1;
    }
    device->make_current();
    std::cout << "[GPU] Active Hardware Device: " << device->renderer_name()
              << " (" << device->version_name() << ")" << std::endl;

    const uint32_t width = 1920;
    const uint32_t height = 1080;
    RenderSurface3D surface(width, height);
    GpuBackend3D gpu_backend(2048); // 2048x2048 Cascaded Shadow Map

    // 5. Setup Cinematic Three-Point Lighting
    auto scene = load_result.scene;
    scene->lights.clear(); // Clear existing lights for controlled studio lighting

    // Key Light: Warm Sun from upper-front-left
    Light3D key_light = Light3D::create_directional(
        Vec3(-0.6f, -0.7f, -0.5f).normalized(),
        Vec3(1.0f, 0.98f, 0.94f),
        4.8f
    );
    scene->add_light(key_light);

    // Fill Light: Cool soft light from right
    Light3D fill_light = Light3D::create_directional(
        Vec3(0.7f, -0.3f, 0.4f).normalized(),
        Vec3(0.6f, 0.75f, 0.95f),
        1.8f
    );
    scene->add_light(fill_light);

    // Rim / Back Light: Sharp rim highlight from behind
    Light3D rim_light = Light3D::create_directional(
        Vec3(0.0f, 0.5f, 1.0f).normalized(),
        Vec3(0.9f, 0.9f, 1.0f),
        2.2f
    );
    scene->add_light(rim_light);

    scene->ambient_color = Vec3(0.30f, 0.32f, 0.38f);

    const bool is_sponza = (model_path.find("sponza") != std::string::npos || model_path.find("Sponza") != std::string::npos);

    if (is_sponza) {
        // Dramatic sunlight streaming through the open atrium roof
        scene->lights.clear();
        Light3D sun_light = Light3D::create_directional(
            Vec3(-0.25f, -0.92f, 0.28f).normalized(),
            Vec3(1.0f, 0.95f, 0.88f),
            6.5f
        );
        scene->add_light(sun_light);
        scene->ambient_color = Vec3(0.50f, 0.52f, 0.58f);
    }

    // 6. Setup Orbit Perspective Camera
    auto camera = std::make_shared<Camera3D>();
    // Position camera dynamically according to model bounding sphere (or inside for architectural interiors)
    Vec3 eye_pos = center + Vec3(radius * 1.5f, radius * 0.75f, radius * 2.2f);
    Vec3 target_pos = center;

    if (is_sponza) {
        eye_pos = Vec3(-5.5f, 1.8f, -0.1f);
        target_pos = Vec3(4.0f, 3.2f, 0.0f);
    }

    camera->set_look_at(eye_pos, target_pos, Vec3::up());
    camera->set_perspective(
        60.0f * (3.1415926535f / 180.0f), // 60 deg wide FOV for interior architectural immersion
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        radius * 10.0f,
        false // OpenGL NDC depth [-1, 1]
    );

    View3D view;
    view.set_scene(scene);
    view.set_camera(camera);
    view.set_viewport(0, 0, width, height);
    view.set_clear_color(Vec4(0.05f, 0.06f, 0.09f, 1.0f)); // Elegant dark slate backdrop
    view.shadows_enabled = true;
    view.shadow_distance = radius * 10.0f;

    std::cout << "[GPU] Rendering 3D model with hardware PBR & CSM shadow pipeline..." << std::endl;
    surface.render_3d(gpu_backend, view, true);

    const auto& stats = gpu_backend.last_stats();
    std::cout << "[GPU] Frame Statistics:" << std::endl;
    std::cout << "  - Total Objects:        " << stats.total_objects << std::endl;
    std::cout << "  - Visible Objects:      " << stats.visible_objects << std::endl;
    std::cout << "  - Total Draw Calls:     " << stats.total_draw_calls << std::endl;
    std::cout << "  - Total Triangles:      " << stats.total_triangles << std::endl;

    // 7. Read Back Pixels & Export High-Res PNG
    std::vector<uint8_t> pixels(width * height * 4);
    surface.read_pixels(pixels.data());

    std::vector<uint8_t> flipped(width * height * 4);
    for (uint32_t y = 0; y < height; ++y) {
        const uint32_t src_row = (height - 1 - y) * width * 4;
        const uint32_t dst_row = y * width * 4;
        std::memcpy(&flipped[dst_row], &pixels[src_row], width * 4);
    }

    auto sz = IntSize::from_wh(width, height);
    if (!sz) {
        std::cerr << "[Error] Invalid frame dimensions!" << std::endl;
        return 1;
    }

    PixmapRef pref(flipped.data(), flipped.size(), *sz, PixelFormat::RGBA8888);
    std::string out_path = "showcase/showcase_gltf_model.png";
    if (argc > 2 && argv[2] != nullptr && std::strlen(argv[2]) > 0) {
        out_path = argv[2];
    }
    bool saved = image::save_image_file(pref, out_path);
    if (saved) {
        std::cout << "[Showcase] Successfully saved rendered 3D model frame to: " << out_path << std::endl;
    } else {
        std::cerr << "[Showcase] Failed to save rendered frame: " << out_path << std::endl;
        return 1;
    }

    std::cout << "\n>>> SHOWCASE GLTF 3D MODEL RENDERING COMPLETE! <<<" << std::endl;
    return 0;
}

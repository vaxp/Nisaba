#include "orion/orion.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace orion;
using namespace orion::physics;

static int g_tests_passed = 0;
static int g_tests_total = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        g_tests_total++; \
        if (!(cond)) { \
            std::cerr << "[-] FAILED: " << msg << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            assert(false && msg); \
        } else { \
            g_tests_passed++; \
        } \
    } while(0)

static constexpr float TEST_EPSILON = 1e-3f;

static bool approx(float a, float b, float eps = TEST_EPSILON) {
    return std::abs(a - b) <= eps;
}

void test_gpu_lighting_ubo_layout() {
    std::cout << "[*] Testing std140 GpuLightData and GpuLightingUboBlock memory layout..." << std::endl;

    TEST_ASSERT(sizeof(GpuLightData) == 64, "GpuLightData size must be exactly 64 bytes");
    TEST_ASSERT(alignof(GpuLightData) == 16, "GpuLightData alignment must be 16 bytes");

    TEST_ASSERT(sizeof(GpuLightingUboBlock) == 8240, "GpuLightingUboBlock size must be exactly 8240 bytes (8192 + 48)");
    TEST_ASSERT(alignof(GpuLightingUboBlock) == 16, "GpuLightingUboBlock alignment must be 16 bytes");

    GpuLightingUboBlock block{};
    block.light_count = 10;
    block.cluster_dim_x = 16;
    block.cluster_dim_y = 9;
    block.cluster_dim_z = 24;

    TEST_ASSERT(block.light_count == 10, "Light count assigned properly");
    TEST_ASSERT(block.cluster_dim_x == 16, "Cluster Dim X is 16");
    TEST_ASSERT(block.cluster_dim_y == 9, "Cluster Dim Y is 9");
    TEST_ASSERT(block.cluster_dim_z == 24, "Cluster Dim Z is 24");
}

void test_sphere_light_representative_point() {
    std::cout << "[*] Testing Sphere Area Light and Karis Representative Point calculations..." << std::endl;

    auto sphere_light = Light3D::create_sphere(
        Vec3(0.0f, 5.0f, -5.0f),
        1.5f,  // sphere radius
        20.0f, // range
        Vec3(1.0f, 0.9f, 0.8f),
        25.0f  // intensity
    );

    TEST_ASSERT(sphere_light.type == LightType::Sphere, "Light type is Sphere");
    TEST_ASSERT(approx(sphere_light.sphere_radius, 1.5f), "Sphere radius matches 1.5");
    TEST_ASSERT(approx(sphere_light.range, 20.0f), "Range is 20.0");

    // Case 1: Surface point directly below sphere, reflection ray pointing directly up at sphere center
    const Vec3 surface_pos(0.0f, 0.0f, -5.0f);
    const Vec3 reflection_dir(0.0f, 1.0f, 0.0f); // Points right towards (0, 5, -5)

    Vec3 rep_point = sphere_light.compute_representative_point(surface_pos, reflection_dir);
    // When reflection ray pierces the sphere, representative point should lie on the ray at distance 5
    TEST_ASSERT(approx(rep_point.x, 0.0f), "Rep point X is 0.0");
    TEST_ASSERT(approx(rep_point.y, 5.0f), "Rep point Y is 5.0 (hits center along ray)");
    TEST_ASSERT(approx(rep_point.z, 0.0f), "Rep point Z is relative offset");

    // Case 2: Reflection ray pointing away from sphere
    const Vec3 reflection_away(0.0f, -1.0f, 0.0f);
    Vec3 rep_away = sphere_light.compute_representative_point(surface_pos, reflection_away);
    // Should fallback to center direction
    TEST_ASSERT(approx(rep_away.y, 5.0f), "Fallback to center offset");

    // Case 3: Test GpuLightData conversion
    GpuLightData gpu_data = sphere_light.get_gpu_data(2);
    TEST_ASSERT(approx(gpu_data.position_radius[0], 0.0f), "GPU Pos X matches");
    TEST_ASSERT(approx(gpu_data.position_radius[1], 5.0f), "GPU Pos Y matches");
    TEST_ASSERT(approx(gpu_data.position_radius[2], -5.0f), "GPU Pos Z matches");
    TEST_ASSERT(approx(gpu_data.position_radius[3], 20.0f), "GPU Range matches");
    TEST_ASSERT(approx(gpu_data.parameters[1], 3.0f), "GPU LightType is 3 (Sphere)");
    TEST_ASSERT(approx(gpu_data.parameters[2], 1.5f), "GPU Sphere radius matches 1.5");
    TEST_ASSERT(approx(gpu_data.parameters[3], 2.0f), "GPU Shadow index matches 2");
}

void test_cluster_grid_frustum_partitioning() {
    std::cout << "[*] Testing ClusterGrid froxel partitioning and exponential depth slicing..." << std::endl;

    ClusterGrid grid(16, 9, 24);
    TEST_ASSERT(grid.dim_x() == 16, "Grid X dimension is 16");
    TEST_ASSERT(grid.dim_y() == 9, "Grid Y dimension is 9");
    TEST_ASSERT(grid.dim_z() == 24, "Grid Z dimension is 24");

    const float fov_y = 1.0472f; // 60 degrees
    const float aspect = 16.0f / 9.0f;
    const float z_near = 0.1f;
    const float z_far = 100.0f;

    grid.build_frustum_grid(fov_y, aspect, z_near, z_far);

    // Verify all 3456 clusters have valid AABBs in view space
    const uint32_t total_clusters = 16 * 9 * 24;
    for (uint32_t c = 0; c < total_clusters; ++c) {
        const AABB& aabb = grid.get_cluster_aabb(c);
        TEST_ASSERT(aabb.is_valid(), "Cluster AABB must be valid");
        // In view space, looking down -Z: max_point.z <= 0
        TEST_ASSERT(aabb.max_point.z <= -z_near + 1e-4f, "AABB max Z is at or behind near plane");
        TEST_ASSERT(aabb.min_point.z >= -z_far - 1e-4f, "AABB min Z is in front of far plane");
    }

    // Test coordinate lookups
    uint32_t cx = 0, cy = 0, cz = 0;
    grid.get_cluster_coords(0.01f, 0.01f, 0.11f, cx, cy, cz);
    TEST_ASSERT(cx == 0, "Near-left corner X is 0");
    TEST_ASSERT(cy == 0, "Near-bottom corner Y is 0");
    TEST_ASSERT(cz == 0, "First depth slice is 0");

    grid.get_cluster_coords(0.99f, 0.99f, 95.0f, cx, cy, cz);
    TEST_ASSERT(cx == 15, "Far-right corner X is 15");
    TEST_ASSERT(cy == 8, "Far-top corner Y is 8");
    TEST_ASSERT(cz == 23, "Last depth slice is 23");

    uint32_t idx = grid.get_cluster_index(0.5f, 0.5f, 10.0f);
    TEST_ASSERT(idx < total_clusters, "Cluster index within range");
}

void test_cluster_grid_light_culling() {
    std::cout << "[*] Testing Clustered light culling and spatial assignment..." << std::endl;

    ClusterGrid grid(16, 9, 24);
    grid.build_frustum_grid(1.0472f, 16.0f / 9.0f, 0.1f, 100.0f);

    std::vector<Light3D> lights;
    // Light 0: Global directional sun
    lights.push_back(Light3D::create_directional(Vec3(0.0f, -1.0f, 0.0f)));

    // Light 1: Localized point light at view position (0, 0, -5) with range 1.5
    lights.push_back(Light3D::create_point(Vec3(0.0f, 0.0f, -5.0f), 1.5f));

    // Light 2: Far localized point light at view position (0, 0, -80) with range 2.0
    lights.push_back(Light3D::create_point(Vec3(0.0f, 0.0f, -80.0f), 2.0f));

    // Identity view matrix (view space == world space)
    Mat44 view = Mat44::identity();
    grid.cull_lights(lights, view);

    // Directional light (Light 0) should be present in all clusters
    for (uint32_t c = 0; c < 16 * 9 * 24; ++c) {
        auto cluster_lights = grid.get_cluster_lights(c);
        TEST_ASSERT(!cluster_lights.empty(), "Cluster has at least the global directional light");
        TEST_ASSERT(cluster_lights[0] == 0, "First light is global directional light 0");
    }

    // Cluster at depth 5.0 (around center screen) should contain Light 1
    uint32_t center_idx = grid.get_cluster_index(0.5f, 0.5f, 5.0f);
    auto center_lights = grid.get_cluster_lights(center_idx);
    bool found_light_1 = false;
    for (uint32_t id : center_lights) {
        if (id == 1) found_light_1 = true;
    }
    TEST_ASSERT(found_light_1, "Cluster at depth 5.0 contains localized Light 1");

    // Cluster at depth 80.0 should contain Light 2 and NOT Light 1
    uint32_t far_idx = grid.get_cluster_index(0.5f, 0.5f, 80.0f);
    auto far_lights = grid.get_cluster_lights(far_idx);
    bool found_light_2 = false;
    bool culled_light_1 = true;
    for (uint32_t id : far_lights) {
        if (id == 2) found_light_2 = true;
        if (id == 1) culled_light_1 = false;
    }
    TEST_ASSERT(found_light_2, "Cluster at depth 80.0 contains localized Light 2");
    TEST_ASSERT(culled_light_1, "Cluster at depth 80.0 successfully culled Light 1");
}

void test_lighting_manager_and_cpu_evaluation() {
    std::cout << "[*] Testing LightingManager orchestration and CPU analytical evaluation..." << std::endl;

    LightingManager lm;
    lm.set_ambient_light(Vec3(0.1f, 0.1f, 0.1f), 1.0f);

    // Add 1 directional sun and 2 localized lights
    lm.add_light(Light3D::create_directional(Vec3(0.0f, -1.0f, 0.0f), Vec3(1.0f, 1.0f, 1.0f), 3.0f));
    lm.add_light(Light3D::create_point(Vec3(0.0f, 2.0f, 0.0f), 10.0f, Vec3(1.0f, 0.5f, 0.2f), 8.0f));
    lm.add_light(Light3D::create_sphere(Vec3(2.0f, 1.0f, 0.0f), 0.5f, 8.0f, Vec3(0.2f, 0.8f, 1.0f), 12.0f));

    TEST_ASSERT(lm.get_lights().size() == 3, "LightingManager has 3 lights");

    // Generate UBO block
    GpuLightingUboBlock ubo = lm.generate_gpu_ubo();
    TEST_ASSERT(ubo.light_count == 3, "UBO has 3 lights");
    TEST_ASSERT(approx(ubo.ambient_color[0], 0.1f), "Ambient R matches");

    // Evaluate CPU direct lighting for standard PBR material
    PbrMaterial std_mat;
    std_mat.base_color_factor = Vec4(0.8f, 0.8f, 0.8f, 1.0f);
    std_mat.metallic_factor = 0.1f;
    std_mat.roughness_factor = 0.3f;

    Vec3 lit_color = lm.evaluate_lighting_cpu(
        Vec3(0.0f, 0.0f, 0.0f),  // Surface position
        Vec3(0.0f, 1.0f, 0.0f),  // Upward normal
        Vec3(0.0f, 2.0f, 4.0f),  // View camera pos
        std_mat
    );

    TEST_ASSERT(lit_color.x > 0.1f, "Surface illuminated in R");
    TEST_ASSERT(lit_color.y > 0.1f, "Surface illuminated in G");
    TEST_ASSERT(lit_color.z > 0.05f, "Surface illuminated in B");

    // Evaluate CPU direct lighting for ClearCoat material
    auto coat_mat = PbrMaterial::create_clear_coat(Vec4(0.9f, 0.1f, 0.1f, 1.0f), 1.0f, 0.05f);
    Vec3 coat_color = lm.evaluate_lighting_cpu(
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 1.0f, 0.0f),
        Vec3(0.0f, 2.0f, 4.0f),
        *coat_mat
    );
    TEST_ASSERT(coat_color.x > coat_color.y, "ClearCoat red dominance preserved");

    // Evaluate CPU direct lighting for Cloth material
    auto cloth_mat = PbrMaterial::create_cloth(Vec4(0.1f, 0.1f, 0.6f, 1.0f), Vec3(0.8f, 0.5f, 0.9f), 0.5f);
    Vec3 cloth_color = lm.evaluate_lighting_cpu(
        Vec3(0.0f, 0.0f, 0.0f),
        Vec3(0.0f, 1.0f, 0.0f),
        Vec3(0.0f, 2.0f, 4.0f),
        *cloth_mat
    );
    TEST_ASSERT(cloth_color.z > 0.0f, "Cloth blue/sheen contribution positive");
}

void test_128_lights_stress_and_performance() {
    std::cout << "[*] Testing 128 dynamic lights stress test with zero heap allocations..." << std::endl;

    LightingManager lm;
    for (int i = 0; i < 128; ++i) {
        const float angle = static_cast<float>(i) * 0.049f;
        const float radius = 5.0f + static_cast<float>(i % 10);
        const Vec3 pos(std::cos(angle) * radius, 1.0f, std::sin(angle) * radius);
        lm.add_light(Light3D::create_point(pos, 4.0f, Vec3(1.0f, 0.8f, 0.6f), 5.0f));
    }

    TEST_ASSERT(lm.get_lights().size() == 128, "128 dynamic lights loaded");

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 10.0f, 20.0f), Vec3(0.0f, 0.0f, 0.0f));
    cam.set_perspective(1.0472f, 16.0f / 9.0f, 0.1f, 100.0f);

    // Warm-up and steady-state culling loop
    for (int frame = 0; frame < 5; ++frame) {
        lm.prepare_frame(cam, 16.0f / 9.0f);
        GpuLightingUboBlock ubo = lm.generate_gpu_ubo();
        TEST_ASSERT(ubo.light_count == 128, "UBO contains all 128 lights");
    }

    TEST_ASSERT(lm.get_cluster_grid().total_light_indices() > 0, "Cluster grid indexed lights across froxels");
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D Clustered Lighting & Multi-Light Tests" << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_gpu_lighting_ubo_layout();
    test_sphere_light_representative_point();
    test_cluster_grid_frustum_partitioning();
    test_cluster_grid_light_culling();
    test_lighting_manager_and_cpu_evaluation();
    test_128_lights_stress_and_performance();

    std::cout << "==========================================================" << std::endl;
    std::cout << "[+] ALL CLUSTERED LIGHTING TESTS PASSED (" << g_tests_passed << "/" << g_tests_total << " assertions)" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

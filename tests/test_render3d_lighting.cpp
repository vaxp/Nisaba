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

void test_light_creation_and_types() {
    std::cout << "[*] Testing Light3D factory functions and parameters..." << std::endl;

    auto sun = Light3D::create_directional(Vec3(0.5f, -1.0f, 0.2f), Vec3(1.0f, 0.95f, 0.9f), 5.0f);
    TEST_ASSERT(sun.type == LightType::Directional, "Sun type is Directional");
    TEST_ASSERT(approx(sun.direction.length(), 1.0f), "Sun direction is normalized");
    TEST_ASSERT(approx(sun.intensity, 5.0f), "Sun intensity is 5.0");

    auto point = Light3D::create_point(Vec3(0.0f, 5.0f, 0.0f), 25.0f, Vec3(1.0f, 0.0f, 0.0f), 12.0f);
    TEST_ASSERT(point.type == LightType::Point, "Point light type is Point");
    TEST_ASSERT(approx(point.range, 25.0f), "Point range is 25.0");
    TEST_ASSERT(approx(point.position.y, 5.0f), "Point position Y is 5.0");

    auto spot = Light3D::create_spot(
        Vec3(0.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f),
        0.5f, 0.8f, 30.0f, Vec3(0.0f, 1.0f, 0.0f), 20.0f
    );
    TEST_ASSERT(spot.type == LightType::Spot, "Spot light type is Spot");
    TEST_ASSERT(approx(spot.inner_cone_angle, 0.5f), "Inner cone angle matches");
    TEST_ASSERT(approx(spot.outer_cone_angle, 0.8f), "Outer cone angle matches");
}

void test_light_attenuation() {
    std::cout << "[*] Testing physical distance attenuation curves..." << std::endl;

    auto sun = Light3D::create_directional(Vec3(0.0f, -1.0f, 0.0f));
    TEST_ASSERT(approx(sun.compute_attenuation(0.0f), 1.0f), "Directional light has 1.0 attenuation at dist 0");
    TEST_ASSERT(approx(sun.compute_attenuation(100.0f), 1.0f), "Directional light has 1.0 attenuation at dist 100");

    auto point = Light3D::create_point(Vec3(0.0f, 0.0f, 0.0f), 20.0f);
    const float att_near = point.compute_attenuation(1.0f);
    const float att_mid = point.compute_attenuation(5.0f);
    const float att_far = point.compute_attenuation(15.0f);
    const float att_out = point.compute_attenuation(25.0f);

    TEST_ASSERT(att_near > att_mid, "Attenuation decreases with distance");
    TEST_ASSERT(att_mid > att_far, "Attenuation decreases monotonically");
    TEST_ASSERT(att_out == 0.0f, "Attenuation drops to exactly 0.0 beyond range cutoff");
}

void test_spot_light_falloff() {
    std::cout << "[*] Testing Spot light cone penumbra falloff..." << std::endl;

    // Spot pointing down along -Y
    // Inner angle: 30 deg (~0.523 rad), Outer angle: 45 deg (~0.785 rad)
    auto spot = Light3D::create_spot(
        Vec3(0.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f),
        30.0f * (3.14159265f / 180.0f),
        45.0f * (3.14159265f / 180.0f),
        50.0f
    );

    // Directly along beam (angle 0) -> fully illuminated
    const float falloff_center = spot.compute_spot_falloff(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(approx(falloff_center, 1.0f), "Center of spot beam has 1.0 falloff");

    // Inside inner cone (15 deg) -> fully illuminated
    const float cos15 = std::cos(15.0f * (3.14159265f / 180.0f));
    const float sin15 = std::sin(15.0f * (3.14159265f / 180.0f));
    const float falloff_inner = spot.compute_spot_falloff(Vec3(sin15, cos15, 0.0f));
    TEST_ASSERT(approx(falloff_inner, 1.0f), "Inside inner cone has 1.0 falloff");

    // In penumbra (between 30 and 45 deg, e.g. 37.5 deg)
    const float cos37 = std::cos(37.5f * (3.14159265f / 180.0f));
    const float sin37 = std::sin(37.5f * (3.14159265f / 180.0f));
    const float falloff_penumbra = spot.compute_spot_falloff(Vec3(sin37, cos37, 0.0f));
    TEST_ASSERT(falloff_penumbra > 0.0f && falloff_penumbra < 1.0f, "Penumbra has smooth fractional falloff");

    // Outside outer cone (60 deg) -> zero light
    const float cos60 = std::cos(60.0f * (3.14159265f / 180.0f));
    const float sin60 = std::sin(60.0f * (3.14159265f / 180.0f));
    const float falloff_outside = spot.compute_spot_falloff(Vec3(sin60, cos60, 0.0f));
    TEST_ASSERT(falloff_outside == 0.0f, "Outside outer cone has 0.0 falloff");
}

void test_light_uniforms_layout() {
    std::cout << "[*] Testing LightUniforms std140 64-byte packing..." << std::endl;

    TEST_ASSERT(sizeof(LightUniforms) == 64, "LightUniforms must be exactly 64 bytes");
    TEST_ASSERT(alignof(LightUniforms) == 16, "LightUniforms must be 16-byte aligned");

    auto spot = Light3D::create_spot(
        Vec3(1.0f, 2.0f, 3.0f), Vec3(0.0f, -1.0f, 0.0f),
        0.4f, 0.7f, 18.0f, Vec3(0.8f, 0.7f, 0.6f), 9.0f
    );

    LightUniforms u = spot.get_uniforms();
    TEST_ASSERT(approx(u.color_intensity[0], 0.8f), "Packed color R");
    TEST_ASSERT(approx(u.color_intensity[3], 9.0f), "Packed intensity");
    TEST_ASSERT(approx(u.position_range[0], 1.0f), "Packed position X");
    TEST_ASSERT(approx(u.position_range[3], 18.0f), "Packed range");
    TEST_ASSERT(approx(u.direction_type[3], static_cast<float>(LightType::Spot)), "Packed light type");
}

void test_shadow_map_buffer() {
    std::cout << "[*] Testing ShadowMap depth buffer operations..." << std::endl;

    ShadowMap sm(512, 512);
    TEST_ASSERT(sm.width == 512 && sm.height == 512, "Shadow map resolution");
    TEST_ASSERT(sm.depth_buffer.size() == 512 * 512, "Depth buffer size matches");

    sm.clear(1.0f);
    TEST_ASSERT(approx(sm.get_depth(100, 100), 1.0f), "Cleared depth is 1.0");

    sm.set_depth(100, 100, 0.35f);
    TEST_ASSERT(approx(sm.get_depth(100, 100), 0.35f), "Set/get depth matches");
}

void test_shadow_map_pcf() {
    std::cout << "[*] Testing ShadowMap PCF sampling..." << std::endl;

    ShadowMap sm(256, 256);
    sm.clear(1.0f);

    // Identity light VP: world space equals clip space
    sm.light_vp_matrix = Mat44::identity();

    // Place an occluder at depth 0.5 across a region
    for (uint32_t y = 120; y <= 136; ++y) {
        for (uint32_t x = 120; x <= 136; ++x) {
            sm.set_depth(x, y, 0.5f);
        }
    }

    // A point at center (x=0, y=0) in world space maps to UV (0.5, 0.5) which is pixel (128, 128)
    // If point is behind occluder at depth Z = 0.8 -> should be in shadow
    float shadow_behind = sm.sample_shadow_pcf(Vec3(0.0f, 0.0f, 0.8f), 0.001f);
    TEST_ASSERT(shadow_behind == 0.0f, "Point behind occluder is fully in shadow (0.0 visibility)");

    // If point is in front of occluder at depth Z = 0.2 -> should be lit
    float shadow_front = sm.sample_shadow_pcf(Vec3(0.0f, 0.0f, 0.2f), 0.001f);
    TEST_ASSERT(shadow_front == 1.0f, "Point in front of occluder is fully lit (1.0 visibility)");
}

void test_csm_matrix_solver() {
    std::cout << "[*] Testing Cascaded Shadow Mapping (CSM) matrix solver..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 5.0f, 20.0f), Vec3(0.0f, 0.0f, 0.0f));
    cam.set_perspective(60.0f * (3.14159265f / 180.0f), 16.0f / 9.0f, 0.1f, 100.0f);

    auto sun = Light3D::create_directional(Vec3(0.5f, -1.0f, 0.3f));

    Mat44 csm_vp = ShadowMap::compute_directional_shadow_matrix(sun, cam, 40.0f, 1024);

    // Matrix must be invertible (non-zero determinant)
    const float det = csm_vp.determinant();
    TEST_ASSERT(std::abs(det) > 1e-6f, "CSM matrix determinant is non-zero");

    // Camera target (0, 0, 0) should project inside the shadow map clip space [-1, 1]x[-1, 1]x[0, 1]
    Vec3 p_proj = csm_vp.transform_point(Vec3(0.0f, 0.0f, 0.0f));
    TEST_ASSERT(p_proj.x >= -1.2f && p_proj.x <= 1.2f, "Target projected X is within bounds");
    TEST_ASSERT(p_proj.y >= -1.2f && p_proj.y <= 1.2f, "Target projected Y is within bounds");
    TEST_ASSERT(p_proj.z >= 0.0f && p_proj.z <= 1.0f, "Target projected Z is within Vulkan depth [0, 1]");
}

int main() {
    std::cout << "========================================================\n"
              << "   NISABA 3D RENDERING PIPELINE: LIGHTING & SHADOW TEST \n"
              << "========================================================" << std::endl;

    test_light_creation_and_types();
    test_light_attenuation();
    test_spot_light_falloff();
    test_light_uniforms_layout();
    test_shadow_map_buffer();
    test_shadow_map_pcf();
    test_csm_matrix_solver();

    std::cout << "\n[+] ALL TESTS COMPLETED: " << g_tests_passed << "/" << g_tests_total << " assertions passed." << std::endl;
    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

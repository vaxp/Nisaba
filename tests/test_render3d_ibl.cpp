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

void test_spherical_harmonics_evaluation() {
    std::cout << "[*] Testing Spherical Harmonics 9-band analytical evaluation..." << std::endl;

    SphericalHarmonics9 sh{};
    // Test pure isotropic DC term L00
    sh.coefficients[0] = Vec3(1.0f, 1.0f, 1.0f);

    // Constant c4 = 0.886227
    Vec3 irr_up = sh.evaluate(Vec3(0.0f, 1.0f, 0.0f));
    Vec3 irr_right = sh.evaluate(Vec3(1.0f, 0.0f, 0.0f));
    Vec3 irr_forward = sh.evaluate(Vec3(0.0f, 0.0f, 1.0f));

    TEST_ASSERT(approx(irr_up.x, 0.886227f, 1e-4f), "SH9 isotropic DC term matches c4 constant along +Y");
    TEST_ASSERT(approx(irr_right.x, 0.886227f, 1e-4f), "SH9 isotropic DC term matches c4 constant along +X");
    TEST_ASSERT(approx(irr_forward.x, 0.886227f, 1e-4f), "SH9 isotropic DC term matches c4 constant along +Z");

    // Test scaling
    SphericalHarmonics9 sh_scaled = sh;
    sh_scaled.scale(2.5f);
    Vec3 irr_scaled = sh_scaled.evaluate(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(approx(irr_scaled.x, irr_up.x * 2.5f, 1e-3f), "SH9 scaling multiplies irradiance linearly");

    // Test addition
    SphericalHarmonics9 sh_sum = sh;
    sh_sum.add(sh);
    Vec3 irr_sum = sh_sum.evaluate(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(approx(irr_sum.x, irr_up.x * 2.0f, 1e-3f), "SH9 addition sums irradiance linearly");
}

void test_spherical_harmonics_presets() {
    std::cout << "[*] Testing Spherical Harmonics daylight, sunset, and studio presets..." << std::endl;

    auto daylight = SphericalHarmonics9::create_daylight(Vec3(0.0f, 1.0f, 0.0f));
    Vec3 day_up = daylight.evaluate(Vec3(0.0f, 1.0f, 0.0f));
    Vec3 day_down = daylight.evaluate(Vec3(0.0f, -1.0f, 0.0f));

    TEST_ASSERT(day_up.x > 0.0f && day_up.y > 0.0f && day_up.z > 0.0f, "Daylight zenith irradiance is strictly positive");
    TEST_ASSERT(day_down.x >= 0.0f && day_down.y >= 0.0f && day_down.z >= 0.0f, "Daylight nadir irradiance is non-negative");
    TEST_ASSERT(day_up.y > day_down.y, "Daylight zenith (+Y) is brighter than nadir (-Y)");

    auto sunset = SphericalHarmonics9::create_sunset();
    Vec3 sunset_irr = sunset.evaluate(Vec3(1.0f, 0.2f, 0.0f).normalized());
    TEST_ASSERT(sunset_irr.x > sunset_irr.z, "Sunset irradiance exhibits warm red/golden tint (R > B)");

    auto studio = SphericalHarmonics9::create_studio();
    Vec3 studio_front = studio.evaluate(Vec3(0.0f, 0.0f, 1.0f));
    TEST_ASSERT(studio_front.x > 0.0f && studio_front.y > 0.0f && studio_front.z > 0.0f, "Studio preset irradiance is valid");
}

void test_dfg_lut_generation() {
    std::cout << "[*] Testing 2D Split-Sum Cook-Torrance DFG LUT generation..." << std::endl;

    // Test analytical approximation boundary limits
    Vec3 dfg_glancing = DfgLutGenerator::sample_analytical(0.0f, 0.0f);
    TEST_ASSERT(approx(dfg_glancing.x, 0.0f, 0.05f), "DFG scale approaches 0 at glancing angle for mirror surface");
    TEST_ASSERT(approx(dfg_glancing.y, 1.0f, 0.05f), "DFG bias approaches 1 at glancing angle for mirror surface");

    Vec3 dfg_normal = DfgLutGenerator::sample_analytical(1.0f, 0.0f);
    TEST_ASSERT(approx(dfg_normal.x, 1.0f, 0.05f), "DFG scale approaches 1 at normal incidence for mirror surface");
    TEST_ASSERT(approx(dfg_normal.y, 0.0f, 0.05f), "DFG bias approaches 0 at normal incidence for mirror surface");

    // Parameter space sweep
    for (int i = 0; i <= 10; ++i) {
        float NoV = static_cast<float>(i) / 10.0f;
        for (int j = 0; j <= 10; ++j) {
            float roughness = static_cast<float>(j) / 10.0f;
            Vec3 sample_val = DfgLutGenerator::sample_analytical(NoV, roughness);
            TEST_ASSERT(sample_val.x >= -1e-5f && sample_val.x <= 1.05f, "DFG scale is bounded in [0, 1]");
            TEST_ASSERT(sample_val.y >= -1e-5f && sample_val.y <= 1.05f, "DFG bias is bounded in [0, 1]");
            TEST_ASSERT(sample_val.x + sample_val.y <= 1.05f, "DFG energy conservation: scale + bias <= 1");
        }
    }

    // Test table generation with small size for speed
    constexpr uint32_t w = 32;
    constexpr uint32_t h = 32;
    constexpr uint32_t num_samples = 64;
    std::vector<float> table = DfgLutGenerator::generate_table(w, h, num_samples);

    TEST_ASSERT(table.size() == w * h * 4, "Generated DFG LUT table has correct size (w * h * 4)");

    // Verify monotonicity across roughness
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            size_t idx = (y * w + x) * 4;
            float scale = table[idx + 0];
            float bias = table[idx + 1];
            float sheen = table[idx + 2];
            float alpha = table[idx + 3];

            TEST_ASSERT(scale >= 0.0f && scale <= 1.05f, "Table scale is in [0, 1]");
            TEST_ASSERT(bias >= 0.0f && bias <= 1.05f, "Table bias is in [0, 1]");
            TEST_ASSERT(sheen >= 0.0f && sheen <= 1.05f, "Table sheen is in [0, 1]");
            TEST_ASSERT(approx(alpha, 1.0f, 1e-5f), "Table alpha channel is 1.0");
        }
    }
}

void test_environment_map_procedural_sky() {
    std::cout << "[*] Testing procedural HDR atmospheric sky dome cubemap..." << std::endl;

    constexpr uint32_t res = 32;
    auto env = EnvironmentMap::create_procedural_sky(
        Vec3(0.0f, 1.0f, 0.0f),
        Vec3(1.0f, 0.95f, 0.9f),
        4.0f,
        res
    );

    TEST_ASSERT(env != nullptr, "Procedural sky environment map created successfully");
    TEST_ASSERT(env->base_resolution() == res, "Environment map base resolution matches requested resolution");
    TEST_ASSERT(env->mip_levels_count() > 1, "Environment map generated pre-filtered roughness mip chain");

    // Inspect base mip level faces
    const auto& base_mip = env->get_mip_level(0);
    TEST_ASSERT(base_mip.resolution == res, "Base mip level resolution matches");

    for (int f = 0; f < 6; ++f) {
        TEST_ASSERT(base_mip.faces[f].size() == res * res * 4, "Cubemap face has valid buffer size");
    }

    // Directional sampling tests
    Vec3 sun_sample = env->sample_radiance(Vec3(0.0f, 1.0f, 0.0f), 0.0f);
    Vec3 ground_sample = env->sample_radiance(Vec3(0.0f, -1.0f, 0.0f), 0.0f);

    TEST_ASSERT(sun_sample.x > 0.0f && sun_sample.y > 0.0f && sun_sample.z > 0.0f, "Sun zenith radiance is positive");
    TEST_ASSERT(ground_sample.x > 0.0f && ground_sample.y > 0.0f && ground_sample.z > 0.0f, "Ground radiance is positive");
    TEST_ASSERT(sun_sample.y > ground_sample.y, "Sun zenith is brighter than ground");

    // Roughness filtering test
    Vec3 sharp_sample = env->sample_radiance(Vec3(0.0f, 1.0f, 0.0f), 0.0f);
    Vec3 blurred_sample = env->sample_radiance(Vec3(0.0f, 1.0f, 0.0f), 1.0f);
    TEST_ASSERT(sharp_sample.x > 0.0f && blurred_sample.x > 0.0f, "Filtered roughness sample produces valid positive radiance");

    // Spherical Harmonics projection test
    SphericalHarmonics9 projected_sh = env->compute_sh9(200);
    Vec3 sh_zenith = projected_sh.evaluate(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(sh_zenith.x > 0.0f && sh_zenith.y > 0.0f && sh_zenith.z > 0.0f, "Projected SH9 produces positive zenith irradiance");
}

void test_scene_environment_integration() {
    std::cout << "[*] Testing Scene3D environment map and SH9 irradiance integration..." << std::endl;

    Scene3D scene;
    TEST_ASSERT(scene.environment_map == nullptr, "Default scene has null environment map");
    TEST_ASSERT(scene.environment_intensity == 1.0f, "Default environment intensity is 1.0");

    auto env = EnvironmentMap::create_procedural_sky(
        Vec3(0.0f, 1.0f, 0.0f),
        Vec3(1.0f, 0.95f, 0.9f),
        4.0f,
        32
    );
    scene.set_environment(env, 2.5f);

    TEST_ASSERT(scene.environment_map == env, "Scene environment map assigned");
    TEST_ASSERT(scene.environment_intensity == 2.5f, "Scene environment intensity assigned");

    Vec3 zenith_irr = scene.sh_irradiance.evaluate(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(zenith_irr.x > 0.0f && zenith_irr.y > 0.0f && zenith_irr.z > 0.0f, "Scene SH9 irradiance automatically projected and valid");
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  Nisaba Sovereign 3D IBL & Environment Lighting Suite    " << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_spherical_harmonics_evaluation();
    test_spherical_harmonics_presets();
    test_dfg_lut_generation();
    test_environment_map_procedural_sky();
    test_scene_environment_integration();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST RESULTS: " << g_tests_passed << " / " << g_tests_total << " ASSERTIONS PASSED" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

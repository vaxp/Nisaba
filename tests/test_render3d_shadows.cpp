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

void test_vogel_disk_sampling() {
    std::cout << "[*] Testing 2D Vogel Disk quasi-random sampling..." << std::endl;

    constexpr size_t count = 32;
    float sum_x = 0.0f;
    float sum_y = 0.0f;

    for (size_t i = 0; i < count; ++i) {
        Vec3 sample = ShadowMap::get_vogel_disk_sample(i, count);
        float radius_sq = sample.x * sample.x + sample.y * sample.y;

        TEST_ASSERT(radius_sq <= 1.0f + 1e-5f, "Vogel disk sample lies within the unit circle");
        TEST_ASSERT(sample.z == 0.0f, "Z component of Vogel disk sample is zero");

        sum_x += sample.x;
        sum_y += sample.y;
    }

    // Centroid of uniform distribution on circle approaches (0, 0)
    float avg_x = sum_x / static_cast<float>(count);
    float avg_y = sum_y / static_cast<float>(count);
    TEST_ASSERT(std::abs(avg_x) < 0.15f, "Vogel disk samples are well-centered around origin (X)");
    TEST_ASSERT(std::abs(avg_y) < 0.15f, "Vogel disk samples are well-centered around origin (Y)");
}

void test_pcss_blocker_and_penumbra() {
    std::cout << "[*] Testing PCSS Blocker Search and Contact Hardening calculation..." << std::endl;

    ShadowMap sm(256, 256);
    sm.clear(1.0f); // Default: no occluders
    sm.light_vp_matrix = Mat44::identity();

    // 1. Point with no blockers should be 100% illuminated (shadow factor 1.0)
    float lit_factor = sm.sample_shadow_pcss(Vec3(0.0f, 0.0f, 0.5f), Vec3(0.0f, 0.0f, -1.0f));
    TEST_ASSERT(approx(lit_factor, 1.0f, 0.01f), "PCSS with no blockers returns fully illuminated (1.0)");

    // 2. Introduce an occluder into the shadow map
    // Place an occluder at light depth z = 0.25 covering the center
    for (uint32_t y = 100; y <= 156; ++y) {
        for (uint32_t x = 100; x <= 156; ++x) {
            sm.set_depth(x, y, 0.25f);
        }
    }

    // A receiver positioned behind the occluder at depth z = 0.5
    float occluded_factor = sm.sample_shadow_pcss(Vec3(0.0f, 0.0f, 0.5f), Vec3(0.0f, 0.0f, -1.0f));
    TEST_ASSERT(occluded_factor < 0.1f, "PCSS correctly detects blocker and shadows receiver");

    // Receiver positioned in front of the occluder at depth z = 0.20
    float unshadowed_front = sm.sample_shadow_pcss(Vec3(0.0f, 0.0f, 0.20f), Vec3(0.0f, 0.0f, -1.0f));
    TEST_ASSERT(approx(unshadowed_front, 1.0f, 0.01f), "Receiver in front of blocker is fully illuminated");

    // 3. Test out of bounds point
    float oob_factor = sm.sample_shadow_pcss(Vec3(5.0f, 0.0f, 0.5f), Vec3(0.0f, 0.0f, -1.0f));
    TEST_ASSERT(approx(oob_factor, 1.0f, 0.01f), "Points outside shadow frustum are unshadowed");
}

void test_spotlight_shadow_matrix() {
    std::cout << "[*] Testing Spotlight shadow perspective matrix..." << std::endl;

    Light3D spot = Light3D::create_spot(
        Vec3(0.0f, 5.0f, 0.0f),
        Vec3(0.0f, -1.0f, 0.0f),
        0.3f, 0.6f,
        25.0f,
        Vec3(1.0f, 1.0f, 1.0f),
        10.0f
    );

    Mat44 spot_vp = ShadowMap::compute_spot_shadow_matrix(spot, 0.1f);
    bool inv_ok = false;
    Mat44 inv_spot_vp = spot_vp.inverse(inv_ok);

    TEST_ASSERT(inv_ok && inv_spot_vp(3, 3) != 0.0f, "Spotlight shadow matrix is invertible and valid");

    // Point along spotlight center axis
    Vec3 target_pt(0.0f, 1.0f, 0.0f);
    Vec3 clip = spot_vp.transform_point(target_pt);

    TEST_ASSERT(approx(clip.x, 0.0f, 0.05f), "Target on spot axis projects to center X (0.0)");
    TEST_ASSERT(approx(clip.y, 0.0f, 0.05f), "Target on spot axis projects to center Y (0.0)");
    TEST_ASSERT(clip.z >= 0.0f && clip.z <= 1.0f, "Target on spot axis has valid depth in [0, 1]");
}

void test_shadow_settings_defaults() {
    std::cout << "[*] Testing ShadowSettings parameter configuration..." << std::endl;

    ShadowSettings settings;
    TEST_ASSERT(settings.enable_shadows, "Shadows enabled by default");
    TEST_ASSERT(!settings.enable_pcss, "PCSS soft shadows are opt-in by default to guarantee 60+ FPS baseline");
    TEST_ASSERT(!settings.enable_contact_shadows, "Contact shadows are opt-in by default to guarantee 60+ FPS baseline");
    TEST_ASSERT(settings.light_size > 0.0f, "PCSS light size is positive");
    TEST_ASSERT(settings.contact_shadow_steps >= 16, "Contact shadow raymarching has >= 16 steps");
    TEST_ASSERT(settings.normal_bias > 0.0f, "Normal bias is positive to eliminate surface acne");

    // Enable high-end cinematic features and verify
    settings.enable_pcss = true;
    settings.enable_contact_shadows = true;
    TEST_ASSERT(settings.enable_pcss, "PCSS soft shadows can be explicitly enabled");
    TEST_ASSERT(settings.enable_contact_shadows, "Contact shadows can be explicitly enabled");
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "  Nisaba Sovereign 3D Advanced Shadows (PCSS/SSCS) Suite  " << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_vogel_disk_sampling();
    test_pcss_blocker_and_penumbra();
    test_spotlight_shadow_matrix();
    test_shadow_settings_defaults();

    std::cout << "==========================================================" << std::endl;
    std::cout << "  TEST RESULTS: " << g_tests_passed << " / " << g_tests_total << " ASSERTIONS PASSED" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

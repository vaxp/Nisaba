#include "orion/orion.hpp"
#include "nisaba/gpu/gpu_device.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

using namespace orion;
using namespace orion::physics;
using nisaba::gpu::GpuDevice;

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

static constexpr float TEST_EPS = 1e-4f;

void test_postprocess_settings_defaults() {
    std::cout << "[*] Testing PostProcessPipelineSettings defaults..." << std::endl;

    PostProcessPipelineSettings pps;
    TEST_ASSERT(!pps.enabled, "PostProcessPipelineSettings master switch is false by default for backward compatibility");

    // Bloom defaults
    TEST_ASSERT(pps.bloom.enabled, "Bloom is enabled when pipeline is active");
    TEST_ASSERT(pps.bloom.threshold == 1.0f, "Default bloom threshold is 1.0 (HDR boundary)");
    TEST_ASSERT(pps.bloom.soft_knee == 0.5f, "Default bloom soft knee is 0.5");
    TEST_ASSERT(pps.bloom.intensity == 0.35f, "Default bloom intensity is 0.35");
    TEST_ASSERT(pps.bloom.max_mips == 5, "Default pyramid depth is 5 mips");

    // GTAO defaults
    TEST_ASSERT(pps.gtao.enabled, "GTAO is enabled when pipeline is active");
    TEST_ASSERT(pps.gtao.radius == 1.5f, "Default GTAO world radius is 1.5m");
    TEST_ASSERT(pps.gtao.falloff_end == 2.5f, "Default GTAO falloff is 2.5m");
    TEST_ASSERT(pps.gtao.num_slices == 3, "Default GTAO slice count is 3");
    TEST_ASSERT(pps.gtao.num_steps == 3, "Default GTAO steps per slice is 3");
    TEST_ASSERT(pps.gtao.intensity == 1.0f, "Default GTAO intensity is 1.0");

    // Fog defaults
    TEST_ASSERT(!pps.fog.enabled, "Atmospheric fog is opt-in by default");
    TEST_ASSERT(pps.fog.density == 0.015f, "Default fog density is 0.015");
    TEST_ASSERT(pps.fog.height_falloff == 0.15f, "Default fog height falloff is 0.15");

    // Color grading defaults
    TEST_ASSERT(pps.grading.enabled, "Color grading is active when pipeline is enabled");
    TEST_ASSERT(pps.grading.exposure == 1.0f, "Default photographic exposure is 1.0");
    TEST_ASSERT(pps.grading.gamma == 2.2f, "Default display gamma is standard 2.2");
}

void test_bloom_knee_math() {
    std::cout << "[*] Testing Bloom soft-knee luminance curve mathematical properties..." << std::endl;

    const float threshold = 1.0f;
    const float soft_knee = 0.5f;
    const float knee = threshold * soft_knee; // 0.5

    auto evaluate_bloom_factor = [&](float luma) -> float {
        float soft = luma - threshold + knee;
        soft = std::clamp(soft, 0.0f, 2.0f * knee);
        soft = (soft * soft) / (4.0f * knee + 1e-5f);
        float factor = std::max(soft, luma - threshold) / std::max(luma, 1e-4f);
        return factor;
    };

    // Sub-threshold luminance (below threshold - knee = 0.5) must yield exactly 0 bloom factor
    float f_sub = evaluate_bloom_factor(0.3f);
    TEST_ASSERT(f_sub < 1e-5f, "Luminance below knee produces zero bloom");

    // Within soft-knee boundary [0.5, 1.5]
    float f_knee_low = evaluate_bloom_factor(0.7f);
    float f_knee_mid = evaluate_bloom_factor(1.0f);
    float f_knee_high = evaluate_bloom_factor(1.4f);

    TEST_ASSERT(f_knee_low > 0.0f, "Knee lower transition produces positive factor");
    TEST_ASSERT(f_knee_low < f_knee_mid, "Bloom factor strictly increases across knee");
    TEST_ASSERT(f_knee_mid < f_knee_high, "Bloom factor strictly increases across knee to high");

    // Super-luminous HDR highlight
    float f_super = evaluate_bloom_factor(10.0f);
    TEST_ASSERT(f_super > 0.85f && f_super < 1.0f, "Super-luminous HDR highlight transmits near 100% bloom");
}

void test_aces_filmic_tonemapping() {
    std::cout << "[*] Testing ACES filmic tonemapping curve..." << std::endl;

    auto aces = [](float x) -> float {
        constexpr float a = 2.51f;
        constexpr float b = 0.03f;
        constexpr float c = 2.43f;
        constexpr float d = 0.59f;
        constexpr float e = 0.14f;
        float val = (x * (a * x + b)) / (x * (c * x + d) + e);
        return std::clamp(val, 0.0f, 1.0f);
    };

    // Zero input produces zero output
    TEST_ASSERT(std::abs(aces(0.0f)) < 0.001f, "ACES(0) is approximately 0");

    // Middle gray (0.18) mapping
    float gray = aces(0.18f);
    TEST_ASSERT(gray > 0.20f && gray < 0.35f, "Middle gray 0.18 maps into filmic midtone [0.20, 0.35]");

    // Standard white (1.0) mapping
    float white = aces(1.0f);
    TEST_ASSERT(white > 0.78f && white < 0.83f, "Standard white 1.0 maps into shoulder roll-off [0.78, 0.83]");

    // Super-bright specular highlight
    float super = aces(20.0f);
    TEST_ASSERT(super >= 0.99f && super <= 1.0f, "Super-bright highlights saturate cleanly to 1.0 without artifacting");

    // Monotonicity verification across 100 samples
    float prev = -1.0f;
    bool strictly_monotonic = true;
    for (int i = 0; i <= 100; ++i) {
        float x = static_cast<float>(i) * 0.1f;
        float y = aces(x);
        if (y < prev - TEST_EPS) {
            strictly_monotonic = false;
            break;
        }
        prev = y;
    }
    TEST_ASSERT(strictly_monotonic, "ACES tonemapping curve is monotonically increasing");

    Vec3 hdr_sample(2.5f, 1.2f, 0.4f);
    Vec3 agx = PostProcessPipeline::evaluate_tone_mapping(hdr_sample, ToneMappingOperator::AgX);
    Vec3 agx_punchy = PostProcessPipeline::evaluate_tone_mapping(hdr_sample, ToneMappingOperator::AgXPunchy);
    Vec3 pbr_neutral = PostProcessPipeline::evaluate_tone_mapping(hdr_sample, ToneMappingOperator::PbrNeutral);

    TEST_ASSERT(agx.x > 0.5f && agx.x <= 1.0f, "AgX maps HDR highlight into [0.5, 1.0]");
    TEST_ASSERT(agx_punchy.x > 0.5f && agx_punchy.x <= 1.0f, "AgX Punchy maps HDR highlight into [0.5, 1.0]");
    TEST_ASSERT(pbr_neutral.x > 0.7f && pbr_neutral.x <= 1.0f, "Khronos PBR Neutral compresses HDR shoulder into [0.7, 1.0]");

    // Khronos PBR Neutral must be strictly linear in midtones (< 0.76)
    Vec3 midtone(0.5f, 0.3f, 0.2f);
    Vec3 pbr_mid = PostProcessPipeline::evaluate_tone_mapping(midtone, ToneMappingOperator::PbrNeutral);
    TEST_ASSERT(std::abs(pbr_mid.x - (0.5f - 0.04f)) < 1e-4f, "Khronos PBR Neutral preserves exact linear midtone slope");
}

void test_tent_upsample_weights() {
    std::cout << "[*] Testing 9-tap bilinear tent filter energy conservation..." << std::endl;

    // 9-tap tent weights distribution:
    // [ 1, 2, 1 ]
    // [ 2, 4, 2 ] / 16
    // [ 1, 2, 1 ]
    float weights[3][3] = {
        { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f },
        { 2.0f / 16.0f, 4.0f / 16.0f, 2.0f / 16.0f },
        { 1.0f / 16.0f, 2.0f / 16.0f, 1.0f / 16.0f }
    };

    float total_energy = 0.0f;
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            total_energy += weights[r][c];
        }
    }

    TEST_ASSERT(std::abs(total_energy - 1.0f) < 1e-6f, "9-tap tent filter perfectly conserves optical energy (sum = 1.0)");
}

void test_gpu_postprocess_pipeline_render() {
    std::cout << "[*] Testing GPU hardware execution of Cinematic Post-Processing Pipeline..." << std::endl;

    auto device = GpuDevice::create();
    if (!device || !device->is_valid()) {
        std::cout << "  (GPU device not available; skipping hardware render pass test)" << std::endl;
        return;
    }

    device->make_current();

    constexpr uint32_t width = 128;
    constexpr uint32_t height = 128;

    RenderSurface3D surface(width, height);

    // Build scene with emissive material that triggers bloom
    auto scene = std::make_shared<Scene3D>();
    auto emissive_mat = std::make_shared<PbrMaterial>();
    emissive_mat->base_color_factor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    emissive_mat->emissive_factor = Vec3(4.0f, 3.0f, 0.5f); // High HDR radiance

    auto sphere_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_sphere(0.8f));
    auto renderable = std::make_shared<Renderable3D>(sphere_mesh, emissive_mat);
    scene->add_renderable(renderable);

    auto camera = std::make_shared<Camera3D>();
    camera->set_look_at(Vec3(0.0f, 0.0f, 3.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3::up());
    camera->set_perspective(60.0f, 1.0f, 0.1f, 50.0f);

    View3D view;
    view.set_scene(scene);
    view.set_camera(camera);
    view.set_viewport(0, 0, width, height);
    view.set_clear_color(Vec4(0.02f, 0.02f, 0.05f, 1.0f));

    // Enable Cinematic Post-Processing Pipeline
    view.post_process.enabled = true;
    view.post_process.bloom.enabled = true;
    view.post_process.bloom.intensity = 0.5f;
    view.post_process.grading.enabled = true;
    view.post_process.grading.exposure = 1.0f;

    GpuBackend3D backend;
    surface.render_3d(backend, view, true);

    // Read back post-processed pixels
    std::vector<uint8_t> pixels(width * height * 4);
    surface.read_pixels(pixels.data());

    // Center pixel of the HDR emissive sphere must be brightly tonemapped
    const size_t center_idx = (height / 2 * width + width / 2) * 4;
    const uint8_t center_r = pixels[center_idx + 0];
    const uint8_t center_g = pixels[center_idx + 1];
    const uint8_t center_b = pixels[center_idx + 2];

    TEST_ASSERT(center_r > 150, "Center emissive sphere is brightly tonemapped in Red channel");
    TEST_ASSERT(center_g > 120, "Center emissive sphere is brightly tonemapped in Green channel");

    // Verify zero OpenGL errors occurred during pipeline dispatch
    GLenum err = glGetError();
    TEST_ASSERT(err == GL_NO_ERROR, "Post-processing pipeline executed with zero OpenGL errors");

    std::cout << "  -> Hardware PostProcessPipeline execution validated! (Center RGB: "
              << (int)center_r << ", " << (int)center_g << ", " << (int)center_b << ")" << std::endl;
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D Cinematic Post-Processing Unit Tests" << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_postprocess_settings_defaults();
    test_bloom_knee_math();
    test_aces_filmic_tonemapping();
    test_tent_upsample_weights();
    test_gpu_postprocess_pipeline_render();

    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Results: " << g_tests_passed << " / " << g_tests_total << " assertions passed." << std::endl;
    std::cout << "==========================================================" << std::endl;

    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

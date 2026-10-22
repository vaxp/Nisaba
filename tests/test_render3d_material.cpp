#include "nisaba/render3d/render3d.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>

using namespace nisaba::render3d;
using namespace nisaba::physics;

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

void test_texture2d_creation_and_sampling() {
    std::cout << "[*] Testing Texture2D creation, bilinear filtering & wrap modes..." << std::endl;

    // Create 2x2 RGBA texture:
    // (0,0)=Red,   (1,0)=Green
    // (0,1)=Blue,  (1,1)=White
    Texture2D tex(2, 2, 4);
    uint8_t raw_pixels[16] = {
        255, 0,   0,   255,   // (0,0) Red
        0,   255, 0,   255,   // (1,0) Green
        0,   0,   255, 255,   // (0,1) Blue
        255, 255, 255, 255    // (1,1) White
    };
    tex.set_pixels(raw_pixels, sizeof(raw_pixels));

    // Sample top-left corner
    Vec4 c00 = tex.sample_rgba(0.0f, 0.0f);
    TEST_ASSERT(approx(c00.x, 1.0f) && approx(c00.y, 0.0f) && approx(c00.z, 0.0f), "Corner (0,0) is Red");

    // Sample top-right corner
    Vec4 c10 = tex.sample_rgba(1.0f, 0.0f);
    TEST_ASSERT(approx(c10.x, 0.0f) && approx(c10.y, 1.0f) && approx(c10.z, 0.0f), "Corner (1,0) is Green");

    // Sample exact center with bilinear interpolation: average of all 4 corners
    // R: (1 + 0 + 0 + 1) / 4 = 0.5
    // G: (0 + 1 + 0 + 1) / 4 = 0.5
    // B: (0 + 0 + 1 + 1) / 4 = 0.5
    Vec4 center = tex.sample_rgba(0.5f, 0.5f);
    TEST_ASSERT(approx(center.x, 0.5f) && approx(center.y, 0.5f) && approx(center.z, 0.5f), "Bilinear center interpolation");

    // Repeat wrapping check: UV (2.0, 3.0) should wrap to (0.0, 0.0) -> Red
    Vec4 wrapped = tex.sample_rgba(2.0f, 3.0f);
    TEST_ASSERT(approx(wrapped.x, 1.0f) && approx(wrapped.y, 0.0f), "Repeat wrap addressing");

    // Solid color factory
    auto solid = Texture2D::create_solid(Vec4(0.2f, 0.4f, 0.6f, 1.0f));
    Vec4 s_sample = solid->sample_rgba(0.5f, 0.5f);
    TEST_ASSERT(approx(s_sample.x, 0.2f, 0.02f) && approx(s_sample.y, 0.4f, 0.02f) && approx(s_sample.z, 0.6f, 0.02f), "Solid color texture creation");
}

void test_pbr_material_uniforms_layout() {
    std::cout << "[*] Testing PbrMaterial uniform packing & std140 layout..." << std::endl;

    TEST_ASSERT(sizeof(PbrMaterialUniforms) == 64, "PbrMaterialUniforms must be exactly 64 bytes");
    TEST_ASSERT(alignof(PbrMaterialUniforms) == 16, "PbrMaterialUniforms must be 16-byte aligned");

    PbrMaterial mat;
    mat.base_color_factor = Vec4(0.8f, 0.1f, 0.2f, 1.0f);
    mat.metallic_factor = 0.9f;
    mat.roughness_factor = 0.15f;
    mat.emissive_factor = Vec3(2.0f, 1.0f, 0.5f);
    mat.normal_scale = 1.5f;

    // Attach base color and normal textures
    mat.base_color_texture = Texture2D::create_solid(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mat.normal_texture = Texture2D::create_solid(Vec4(0.5f, 0.5f, 1.0f, 1.0f));

    uint32_t flags = mat.compute_texture_flags();
    TEST_ASSERT((flags & HAS_BASE_COLOR_MAP) != 0, "Flag HAS_BASE_COLOR_MAP set");
    TEST_ASSERT((flags & HAS_NORMAL_MAP) != 0, "Flag HAS_NORMAL_MAP set");
    TEST_ASSERT((flags & HAS_METALLIC_ROUGHNESS_MAP) == 0, "Flag HAS_METALLIC_ROUGHNESS_MAP cleared");

    PbrMaterialUniforms u = mat.get_uniforms();
    TEST_ASSERT(approx(u.base_color[0], 0.8f), "Packed base color R");
    TEST_ASSERT(approx(u.base_color[1], 0.1f), "Packed base color G");
    TEST_ASSERT(approx(u.base_color[2], 0.2f), "Packed base color B");
    TEST_ASSERT(approx(u.metallic, 0.9f), "Packed metallic");
    TEST_ASSERT(approx(u.roughness, 0.15f), "Packed roughness");
    TEST_ASSERT(approx(u.emissive[0], 2.0f), "Packed emissive R");
    TEST_ASSERT(approx(u.normal_scale, 1.5f), "Packed normal scale");
    TEST_ASSERT(u.texture_flags == flags, "Uniform texture flags match");
}

void test_brdf_ggx_distribution() {
    std::cout << "[*] Testing CookTorranceBRDF GGX normal distribution function..." << std::endl;

    // Smooth surface (roughness = 0.1) has much higher peak than rough surface (roughness = 0.8)
    const float d_smooth = CookTorranceBRDF::distribution_ggx(1.0f, 0.1f);
    const float d_medium = CookTorranceBRDF::distribution_ggx(1.0f, 0.5f);
    const float d_rough  = CookTorranceBRDF::distribution_ggx(1.0f, 0.9f);

    TEST_ASSERT(d_smooth > d_medium, "Smooth surface has sharper specular peak than medium");
    TEST_ASSERT(d_medium > d_rough,  "Medium surface has sharper specular peak than rough");

    // When N.H drops away from 1.0 (off-peak), D drops sharply
    const float d_off_peak = CookTorranceBRDF::distribution_ggx(0.7f, 0.1f);
    TEST_ASSERT(d_off_peak < d_smooth * 0.1f, "GGX drops sharply away from alignment");
}

void test_brdf_geometry_smith() {
    std::cout << "[*] Testing CookTorranceBRDF Smith shadowing/masking..." << std::endl;

    // Perpendicular view and light (no shadowing) -> G close to 1
    const float g_perpendicular = CookTorranceBRDF::geometry_smith(1.0f, 1.0f, 0.5f);
    TEST_ASSERT(g_perpendicular > 0.6f && g_perpendicular <= 1.0f, "Normal incidence has high G");

    // Grazing angle (N.V -> 0.01) -> G drops heavily towards 0
    const float g_grazing = CookTorranceBRDF::geometry_smith(0.01f, 1.0f, 0.5f);
    TEST_ASSERT(g_grazing < 0.1f, "Grazing angle heavily shadowed");
}

void test_brdf_fresnel_schlick() {
    std::cout << "[*] Testing CookTorranceBRDF Schlick Fresnel reflectance..." << std::endl;

    const Vec3 f0(0.04f, 0.04f, 0.04f); // Typical dielectric F0

    // Direct incidence (cos_theta = 1.0) -> F = F0
    Vec3 f_direct = CookTorranceBRDF::fresnel_schlick(1.0f, f0);
    TEST_ASSERT(approx(f_direct.x, 0.04f) && approx(f_direct.y, 0.04f) && approx(f_direct.z, 0.04f), "Normal incidence F equals F0");

    // Grazing incidence (cos_theta = 0.0) -> F approaches 1.0
    Vec3 f_grazing = CookTorranceBRDF::fresnel_schlick(0.0f, f0);
    TEST_ASSERT(approx(f_grazing.x, 1.0f) && approx(f_grazing.y, 1.0f) && approx(f_grazing.z, 1.0f), "Grazing incidence F approaches 1.0");
}

void test_brdf_full_evaluation() {
    std::cout << "[*] Testing CookTorranceBRDF full photometric evaluation..." << std::endl;

    const Vec3 n(0.0f, 1.0f, 0.0f);
    const Vec3 v(0.0f, 1.0f, 0.0f);
    const Vec3 l(0.0f, 1.0f, 0.0f);
    const Vec3 albedo(0.8f, 0.8f, 0.8f);

    // Dielectric test (metallic = 0.0)
    Vec3 result_dielectric = CookTorranceBRDF::evaluate(n, v, l, albedo, 0.0f, 0.5f);
    TEST_ASSERT(result_dielectric.x > 0.0f && result_dielectric.y > 0.0f && result_dielectric.z > 0.0f, "Dielectric produces positive radiance");

    // Metallic test (metallic = 1.0)
    Vec3 result_metallic = CookTorranceBRDF::evaluate(n, v, l, albedo, 1.0f, 0.2f);
    TEST_ASSERT(result_metallic.x > 0.0f, "Metallic produces positive specular radiance");

    // Light behind surface (N.L < 0) -> zero illumination
    const Vec3 l_behind(0.0f, -1.0f, 0.0f);
    Vec3 result_behind = CookTorranceBRDF::evaluate(n, v, l_behind, albedo, 0.0f, 0.5f);
    TEST_ASSERT(result_behind.x == 0.0f && result_behind.y == 0.0f && result_behind.z == 0.0f, "Light behind surface produces zero radiance");
}

void test_glsl_shaders_validity() {
    std::cout << "[*] Testing embedded GLSL PBR shader source strings..." << std::endl;

    const char* vs = CookTorranceBRDF::PBR_VERTEX_SHADER_GLSL;
    const char* fs = CookTorranceBRDF::PBR_FRAGMENT_SHADER_GLSL;

    TEST_ASSERT(vs != nullptr && std::strlen(vs) > 100, "Vertex shader source present");
    TEST_ASSERT(fs != nullptr && std::strlen(fs) > 100, "Fragment shader source present");

    TEST_ASSERT(std::strstr(vs, "#version 450") != nullptr, "Vertex shader has GLSL 450 version tag");
    TEST_ASSERT(std::strstr(fs, "#version 450") != nullptr, "Fragment shader has GLSL 450 version tag");
    TEST_ASSERT(std::strstr(fs, "DistributionGGX") != nullptr, "Fragment shader contains GGX function");
    TEST_ASSERT(std::strstr(fs, "GeometrySmith") != nullptr, "Fragment shader contains Smith function");
    TEST_ASSERT(std::strstr(fs, "FresnelSchlick") != nullptr, "Fragment shader contains Schlick function");
}

int main() {
    std::cout << "========================================================\n"
              << "   NISABA 3D RENDERING PIPELINE: MATERIAL & BRDF TEST   \n"
              << "========================================================" << std::endl;

    test_texture2d_creation_and_sampling();
    test_pbr_material_uniforms_layout();
    test_brdf_ggx_distribution();
    test_brdf_geometry_smith();
    test_brdf_fresnel_schlick();
    test_brdf_full_evaluation();
    test_glsl_shaders_validity();

    std::cout << "\n[+] ALL TESTS COMPLETED: " << g_tests_passed << "/" << g_tests_total << " assertions passed." << std::endl;
    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

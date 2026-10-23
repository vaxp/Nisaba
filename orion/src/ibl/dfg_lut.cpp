#include "orion/ibl/dfg_lut.hpp"
#include <cmath>
#include <algorithm>

namespace orion {

static inline float radical_inverse_vdc(uint32_t bits) noexcept {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return static_cast<float>(bits) * 2.3283064365386963e-10f; // / 0x100000000
}

static inline void hammersley_2d(uint32_t i, uint32_t n, float& u, float& v) noexcept {
    u = static_cast<float>(i) / static_cast<float>(n);
    v = radical_inverse_vdc(i);
}

static inline Vec3 importance_sample_ggx(float u1, float u2, float roughness) noexcept {
    const float a = roughness * roughness;
    const float phi = 2.0f * 3.14159265358979323846f * u1;
    const float cos_theta = std::sqrt(std::clamp((1.0f - u2) / (1.0f + (a * a - 1.0f) * u2), 0.0f, 1.0f));
    const float sin_theta = std::sqrt(std::max(0.0f, 1.0f - cos_theta * cos_theta));
    return Vec3(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta);
}

static inline float geometry_smith_ggx(float n_dot_v, float n_dot_l, float roughness) noexcept {
    const float k = (roughness * roughness) * 0.5f;
    const float g_v = n_dot_v / (n_dot_v * (1.0f - k) + k);
    const float g_l = n_dot_l / (n_dot_l * (1.0f - k) + k);
    return g_v * g_l;
}

Vec3 DfgLutGenerator::integrate_dfg(float n_dot_v, float roughness, uint32_t num_samples) noexcept {
    n_dot_v = std::clamp(n_dot_v, 1e-4f, 1.0f);
    roughness = std::clamp(roughness, 1e-4f, 1.0f);
    const Vec3 V(std::sqrt(std::max(0.0f, 1.0f - n_dot_v * n_dot_v)), 0.0f, n_dot_v);

    float scale = 0.0f;
    float bias = 0.0f;
    float sheen = 0.0f;

    for (uint32_t i = 0; i < num_samples; ++i) {
        float u1 = 0.0f, u2 = 0.0f;
        hammersley_2d(i, num_samples, u1, u2);
        const Vec3 H = importance_sample_ggx(u1, u2, roughness);
        const float v_dot_h = std::max(0.0f, V.dot(H));
        const Vec3 L = H * (2.0f * v_dot_h) - V;

        const float n_dot_l = std::max(0.0f, L.z);
        const float n_dot_h = std::max(0.0f, H.z);

        if (n_dot_l > 0.0f) {
            const float G = geometry_smith_ggx(n_dot_v, n_dot_l, roughness);
            const float G_vis = (G * v_dot_h) / (n_dot_h * n_dot_v + 1e-5f);
            const float Fc = std::pow(std::clamp(1.0f - v_dot_h, 0.0f, 1.0f), 5.0f);

            scale += (1.0f - Fc) * G_vis;
            bias += Fc * G_vis;

            // Charlie sheen microfiber integration
            const float sin2_h = std::max(0.0f, 1.0f - n_dot_h * n_dot_h);
            const float charlie_d = std::pow(sin2_h, 0.5f / std::max(0.01f, roughness));
            sheen += charlie_d * G_vis;
        }
    }

    const float inv_n = 1.0f / static_cast<float>(num_samples);
    return Vec3(scale * inv_n, bias * inv_n, sheen * inv_n);
}

std::vector<float> DfgLutGenerator::generate_table(
    uint32_t width,
    uint32_t height,
    uint32_t num_samples
) noexcept {
    std::vector<float> data(width * height * 4, 0.0f);

    for (uint32_t y = 0; y < height; ++y) {
        // Vertical axis: Roughness [0, 1]
        const float roughness = std::max(0.01f, static_cast<float>(y + 1) / static_cast<float>(height));

        for (uint32_t x = 0; x < width; ++x) {
            // Horizontal axis: N.V [0, 1]
            const float n_dot_v = std::max(0.001f, static_cast<float>(x + 1) / static_cast<float>(width));

            const Vec3 dfg = integrate_dfg(n_dot_v, roughness, num_samples);

            const size_t index = (y * width + x) * 4;
            data[index + 0] = std::clamp(dfg.x, 0.0f, 1.0f); // Scale
            data[index + 1] = std::clamp(dfg.y, 0.0f, 1.0f); // Bias
            data[index + 2] = std::clamp(dfg.z, 0.0f, 1.0f); // Sheen
            data[index + 3] = 1.0f;
        }
    }

    return data;
}

Vec3 DfgLutGenerator::sample_analytical(float n_dot_v, float roughness) noexcept {
    const float nv = std::clamp(n_dot_v, 0.0f, 1.0f);
    const float r = std::clamp(roughness, 0.0f, 1.0f);

    // Analytical fit for Split-Sum GGX specular BRDF
    // At r = 0, scale = 1 - (1 - nv)^5, bias = (1 - nv)^5
    const float fc = std::pow(1.0f - nv, 5.0f);
    const float r2 = r * r;

    const float scale = std::clamp((1.0f - fc) * (1.0f - 0.75f * r) - 0.15f * r2, 0.0f, 1.0f);
    const float bias = std::clamp(fc * (1.0f - r) + r * (0.35f * (1.0f - nv) + 0.15f * (1.0f - r)), 0.0f, 1.0f);
    const float sheen = std::clamp(0.25f * (1.0f - r) * (1.0f - nv), 0.0f, 1.0f);

    return Vec3(scale, bias, sheen);
}

} // namespace orion

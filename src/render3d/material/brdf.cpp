#include "nisaba/render3d/material/brdf.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::render3d {

namespace {
constexpr float PI = 3.14159265358979323846f;
constexpr float INV_PI = 1.0f / PI;
} // namespace

float CookTorranceBRDF::distribution_ggx(float n_dot_h, float roughness) noexcept {
    const float a = roughness * roughness;
    const float a2 = a * a;
    const float n_dot_h2 = n_dot_h * n_dot_h;
    const float denom = (n_dot_h2 * (a2 - 1.0f) + 1.0f);
    return a2 / (PI * denom * denom + 1e-7f);
}

float CookTorranceBRDF::geometry_smith(float n_dot_v, float n_dot_l, float roughness) noexcept {
    const float r = roughness + 1.0f;
    const float k = (r * r) / 8.0f;
    const float g1_v = n_dot_v / (n_dot_v * (1.0f - k) + k);
    const float g1_l = n_dot_l / (n_dot_l * (1.0f - k) + k);
    return g1_v * g1_l;
}

Vec3 CookTorranceBRDF::fresnel_schlick(float cos_theta, const Vec3& f0) noexcept {
    const float factor = std::pow(std::clamp(1.0f - cos_theta, 0.0f, 1.0f), 5.0f);
    return f0 + (Vec3(1.0f, 1.0f, 1.0f) - f0) * factor;
}

Vec3 CookTorranceBRDF::evaluate(
    const Vec3& n,
    const Vec3& v,
    const Vec3& l,
    const Vec3& albedo,
    float metallic,
    float roughness
) noexcept {
    const float n_dot_l = std::max(n.dot(l), 0.0f);
    if (n_dot_l <= 0.0f) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    const float n_dot_v = std::max(n.dot(v), 1e-4f);
    const Vec3 h = (v + l).normalized();
    const float n_dot_h = std::max(n.dot(h), 0.0f);
    const float v_dot_h = std::max(v.dot(h), 0.0f);

    roughness = std::clamp(roughness, 0.04f, 1.0f);
    metallic = std::clamp(metallic, 0.0f, 1.0f);

    const Vec3 f0 = Vec3(0.04f, 0.04f, 0.04f) * (1.0f - metallic) + albedo * metallic;

    const float ndf = distribution_ggx(n_dot_h, roughness);
    const float g = geometry_smith(n_dot_v, n_dot_l, roughness);
    const Vec3 f = fresnel_schlick(v_dot_h, f0);

    const Vec3 numerator = f * (ndf * g);
    const float denominator = 4.0f * n_dot_v * n_dot_l + 1e-4f;
    const Vec3 specular = numerator / denominator;

    const Vec3 k_s = f;
    const Vec3 k_d = (Vec3(1.0f, 1.0f, 1.0f) - k_s) * (1.0f - metallic);
    const Vec3 diffuse = k_d * albedo * INV_PI;

    return (diffuse + specular) * n_dot_l;
}

} // namespace nisaba::render3d

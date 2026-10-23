#include "orion/material/brdf.hpp"
#include <cmath>
#include <algorithm>

namespace orion {

namespace {
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

float CookTorranceBRDF::distribution_charlie(float n_dot_h, float roughness) noexcept {
    const float r = std::clamp(roughness, 0.04f, 1.0f);
    const float inv_r = 1.0f / r;
    const float cos2h = n_dot_h * n_dot_h;
    const float sin2h = std::max(1.0f - cos2h, 1e-4f);
    return (2.0f + inv_r) * std::pow(sin2h, inv_r * 0.5f) / (2.0f * PI);
}

float CookTorranceBRDF::visibility_neubelt(float n_dot_v, float n_dot_l) noexcept {
    const float denom = 4.0f * (n_dot_l + n_dot_v - n_dot_l * n_dot_v);
    return (denom > 1e-5f) ? (1.0f / denom) : 0.0f;
}

float CookTorranceBRDF::visibility_kelemen(float l_dot_h) noexcept {
    const float d = l_dot_h * l_dot_h;
    return (d > 1e-5f) ? (0.25f / d) : 0.0f;
}

float CookTorranceBRDF::distribution_anisotropic(float at, float ab, float to_h, float bo_h, float no_h) noexcept {
    const float d = (to_h * to_h) / (at * at) + (bo_h * bo_h) / (ab * ab) + (no_h * no_h);
    if (d <= 1e-5f) return 0.0f;
    return 1.0f / (PI * at * ab * d * d);
}

float CookTorranceBRDF::visibility_anisotropic(
    float at, float ab, float to_v, float bo_v, float to_l, float bo_l, float no_v, float no_l
) noexcept {
    const float lambda_v = no_l * std::sqrt(at * at * to_v * to_v + ab * ab * bo_v * bo_v + no_v * no_v);
    const float lambda_l = no_v * std::sqrt(at * at * to_l * to_l + ab * ab * bo_l * bo_l + no_l * no_l);
    const float denom = 2.0f * (lambda_v + lambda_l);
    return (denom > 1e-5f) ? (1.0f / denom) : 0.0f;
}

float CookTorranceBRDF::diffuse_wrap(float n_dot_l, float w) noexcept {
    return std::clamp((n_dot_l + w) / ((1.0f + w) * (1.0f + w)), 0.0f, 1.0f);
}

Vec3 CookTorranceBRDF::evaluate_clear_coat(
    const Vec3& n, const Vec3& v, const Vec3& l, float coat_factor, float coat_roughness
) noexcept {
    const float n_dot_l = std::max(n.dot(l), 0.0f);
    if (n_dot_l <= 0.0f || coat_factor <= 1e-4f) return Vec3(0.0f, 0.0f, 0.0f);

    const Vec3 h = (v + l).normalized();
    const float n_dot_h = std::max(n.dot(h), 0.0f);
    const float l_dot_h = std::max(l.dot(h), 0.0f);

    const float d = distribution_ggx(n_dot_h, coat_roughness);
    const float vis = visibility_kelemen(l_dot_h);
    const float f = 0.04f + 0.96f * std::pow(1.0f - l_dot_h, 5.0f);

    return Vec3(1.0f, 1.0f, 1.0f) * (d * vis * f * coat_factor * n_dot_l);
}

Vec3 CookTorranceBRDF::evaluate_cloth(
    const Vec3& n, const Vec3& v, const Vec3& l, const Vec3& base_color, const Vec3& sheen_color, float sheen_roughness
) noexcept {
    const float n_dot_l = std::max(n.dot(l), 0.0f);
    const float n_dot_v = std::max(n.dot(v), 1e-4f);
    if (n_dot_l <= 0.0f) return Vec3(0.0f, 0.0f, 0.0f);

    const Vec3 h = (v + l).normalized();
    const float n_dot_h = std::max(n.dot(h), 0.0f);

    const float d = distribution_charlie(n_dot_h, sheen_roughness);
    const float vis = visibility_neubelt(n_dot_v, n_dot_l);
    const Vec3 sheen_spec = sheen_color * (d * vis);

    const float wrap_diff = diffuse_wrap(n_dot_l, 0.5f);
    const Vec3 diffuse = base_color * (wrap_diff * INV_PI);

    return (diffuse + sheen_spec) * n_dot_l;
}

Vec3 CookTorranceBRDF::evaluate_anisotropic(
    const Vec3& n, const Vec3& t, const Vec3& b, const Vec3& v, const Vec3& l,
    const Vec3& albedo, float metallic, float roughness, float anisotropy
) noexcept {
    const float n_dot_l = std::max(n.dot(l), 0.0f);
    const float n_dot_v = std::max(n.dot(v), 1e-4f);
    if (n_dot_l <= 0.0f) return Vec3(0.0f, 0.0f, 0.0f);

    const Vec3 h = (v + l).normalized();
    const float no_h = std::max(n.dot(h), 0.0f);
    const float vo_h = std::max(v.dot(h), 0.0f);

    const float at = std::max(roughness * (1.0f + anisotropy), 0.0016f);
    const float ab = std::max(roughness * (1.0f - anisotropy), 0.0016f);

    const float to_h = t.dot(h);
    const float bo_h = b.dot(h);
    const float to_v = t.dot(v);
    const float bo_v = b.dot(v);
    const float to_l = t.dot(l);
    const float bo_l = b.dot(l);

    const float d = distribution_anisotropic(at, ab, to_h, bo_h, no_h);
    const float vis = visibility_anisotropic(at, ab, to_v, bo_v, to_l, bo_l, n_dot_v, n_dot_l);

    const Vec3 f0 = Vec3(0.04f, 0.04f, 0.04f) * (1.0f - metallic) + albedo * metallic;
    const Vec3 f = fresnel_schlick(vo_h, f0);

    const Vec3 spec = f * (d * vis);
    const Vec3 k_d = (Vec3(1.0f, 1.0f, 1.0f) - f) * (1.0f - metallic);
    const Vec3 diff = k_d * albedo * INV_PI;

    return (diff + spec) * n_dot_l;
}

Vec3 CookTorranceBRDF::evaluate_subsurface(
    const Vec3& n, const Vec3& v, const Vec3& l, const Vec3& base_color, const Vec3& sss_color,
    float thickness, float sss_power
) noexcept {
    const float n_dot_l = std::max(n.dot(l), 0.0f);
    const float scatter_vo_l = std::clamp(v.dot(-l), 0.0f, 1.0f);
    const float forward_scatter = std::pow(2.0f, (scatter_vo_l - 1.0f) * sss_power);
    const float back_scatter = (n_dot_l * thickness + (1.0f - thickness)) * 0.5f;
    const float sss_factor = (back_scatter * (1.0f - forward_scatter) + forward_scatter) * (1.0f - thickness);

    const Vec3 scattered = sss_color * (sss_factor * INV_PI);
    const Vec3 diffuse = base_color * (n_dot_l * INV_PI);

    return diffuse + scattered;
}

} // namespace orion

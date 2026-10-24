#include "orion/material/brdf.hpp"
#include <cmath>
#include <algorithm>

namespace orion {

namespace {
constexpr float INV_PI = 1.0f / PI;

[[nodiscard]] inline constexpr float pow5_fast(float x) noexcept {
    const float x2 = x * x;
    return x2 * x2 * x;
}
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
    const float k = (r * r) * 0.125f;
    const float one_minus_k = 1.0f - k;
    const float g1_v = n_dot_v / (n_dot_v * one_minus_k + k);
    const float g1_l = n_dot_l / (n_dot_l * one_minus_k + k);
    return g1_v * g1_l;
}

Vec3 CookTorranceBRDF::fresnel_schlick(float cos_theta, const Vec3& f0) noexcept {
    const float factor = pow5_fast(std::clamp(1.0f - cos_theta, 0.0f, 1.0f));
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
    const float n_dot_l = n.x * l.x + n.y * l.y + n.z * l.z;
    if (n_dot_l <= 0.0f) {
        return Vec3(0.0f, 0.0f, 0.0f);
    }

    const float raw_n_dot_v = n.x * v.x + n.y * v.y + n.z * v.z;
    const float n_dot_v = (raw_n_dot_v > 1e-4f) ? raw_n_dot_v : 1e-4f;

    const float hx = v.x + l.x;
    const float hy = v.y + l.y;
    const float hz = v.z + l.z;
    const float h_len_sq = hx * hx + hy * hy + hz * hz;
    const float inv_h_len = (h_len_sq > 1e-12f) ? (1.0f / std::sqrt(h_len_sq)) : 0.0f;

    const float n_dot_h = std::max((n.x * hx + n.y * hy + n.z * hz) * inv_h_len, 0.0f);
    const float v_dot_h = std::max((v.x * hx + v.y * hy + v.z * hz) * inv_h_len, 0.0f);

    roughness = std::clamp(roughness, 0.04f, 1.0f);
    metallic = std::clamp(metallic, 0.0f, 1.0f);

    const float one_minus_m = 1.0f - metallic;
    const float f0_x = 0.04f * one_minus_m + albedo.x * metallic;
    const float f0_y = 0.04f * one_minus_m + albedo.y * metallic;
    const float f0_z = 0.04f * one_minus_m + albedo.z * metallic;

    const float a = roughness * roughness;
    const float a2 = a * a;
    const float denom_ndf = (n_dot_h * n_dot_h * (a2 - 1.0f) + 1.0f);
    const float ndf = a2 / (PI * denom_ndf * denom_ndf + 1e-7f);

    const float r = roughness + 1.0f;
    const float k = (r * r) * 0.125f;
    const float one_minus_k = 1.0f - k;
    const float g = (n_dot_v / (n_dot_v * one_minus_k + k)) * (n_dot_l / (n_dot_l * one_minus_k + k));

    const float f_factor = pow5_fast(std::clamp(1.0f - v_dot_h, 0.0f, 1.0f));
    const float fx = f0_x + (1.0f - f0_x) * f_factor;
    const float fy = f0_y + (1.0f - f0_y) * f_factor;
    const float fz = f0_z + (1.0f - f0_z) * f_factor;

    const float spec_scale = (ndf * g) / (4.0f * n_dot_v * n_dot_l + 1e-4f);
    const float diff_scale = one_minus_m * INV_PI;

    return Vec3(
        ((1.0f - fx) * diff_scale * albedo.x + fx * spec_scale) * n_dot_l,
        ((1.0f - fy) * diff_scale * albedo.y + fy * spec_scale) * n_dot_l,
        ((1.0f - fz) * diff_scale * albedo.z + fz * spec_scale) * n_dot_l
    );
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
    const float n_dot_l = n.x * l.x + n.y * l.y + n.z * l.z;
    if (n_dot_l <= 0.0f || coat_factor <= 1e-4f) return Vec3(0.0f, 0.0f, 0.0f);

    const float hx = v.x + l.x;
    const float hy = v.y + l.y;
    const float hz = v.z + l.z;
    const float h_len_sq = hx * hx + hy * hy + hz * hz;
    const float inv_h_len = (h_len_sq > 1e-12f) ? (1.0f / std::sqrt(h_len_sq)) : 0.0f;

    const float n_dot_h = std::max((n.x * hx + n.y * hy + n.z * hz) * inv_h_len, 0.0f);
    const float l_dot_h = std::max((l.x * hx + l.y * hy + l.z * hz) * inv_h_len, 0.0f);

    const float d = distribution_ggx(n_dot_h, coat_roughness);
    const float vis = visibility_kelemen(l_dot_h);
    const float f = 0.04f + 0.96f * pow5_fast(1.0f - l_dot_h);
    const float val = d * vis * f * coat_factor * n_dot_l;

    return Vec3(val, val, val);
}

Vec3 CookTorranceBRDF::evaluate_cloth(
    const Vec3& n, const Vec3& v, const Vec3& l, const Vec3& base_color, const Vec3& sheen_color, float sheen_roughness
) noexcept {
    const float n_dot_l = n.x * l.x + n.y * l.y + n.z * l.z;
    if (n_dot_l <= 0.0f) return Vec3(0.0f, 0.0f, 0.0f);
    const float raw_n_dot_v = n.x * v.x + n.y * v.y + n.z * v.z;
    const float n_dot_v = (raw_n_dot_v > 1e-4f) ? raw_n_dot_v : 1e-4f;

    const float hx = v.x + l.x;
    const float hy = v.y + l.y;
    const float hz = v.z + l.z;
    const float h_len_sq = hx * hx + hy * hy + hz * hz;
    const float inv_h_len = (h_len_sq > 1e-12f) ? (1.0f / std::sqrt(h_len_sq)) : 0.0f;
    const float n_dot_h = std::max((n.x * hx + n.y * hy + n.z * hz) * inv_h_len, 0.0f);

    const float d = distribution_charlie(n_dot_h, sheen_roughness);
    const float denom = 4.0f * (n_dot_l + n_dot_v - n_dot_l * n_dot_v);
    const float vis = (denom > 1e-5f) ? (1.0f / denom) : 0.0f;
    const float spec_factor = d * vis * n_dot_l;

    const float wrap_diff = std::clamp((n_dot_l + 0.5f) * 0.44444445f, 0.0f, 1.0f) * INV_PI * n_dot_l;

    return Vec3(
        base_color.x * wrap_diff + sheen_color.x * spec_factor,
        base_color.y * wrap_diff + sheen_color.y * spec_factor,
        base_color.z * wrap_diff + sheen_color.z * spec_factor
    );
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

Vec3 CookTorranceBRDF::evaluate_energy_compensation(
    const Vec3& f0,
    float dfg_y
) noexcept {
    const float safe_dfg = std::max(dfg_y, 1e-4f);
    const float factor = (1.0f / safe_dfg) - 1.0f;
    return Vec3(
        1.0f + f0.x * factor,
        1.0f + f0.y * factor,
        1.0f + f0.z * factor
    );
}

float CookTorranceBRDF::approximate_dfg_multiscatter(
    float n_dot_v,
    float perceptual_roughness
) noexcept {
    const float r = std::clamp(perceptual_roughness, 0.04f, 1.0f);
    const float nv = std::clamp(n_dot_v, 0.0f, 1.0f);
    const float c0 = -1.0f * r + 1.0f;
    const float a004 = std::min(r * r, std::exp2(-9.28f * nv)) * c0 + (1.0f - c0);
    return std::clamp(-1.04f * a004 + 1.04f, 0.05f, 1.0f);
}

Vec3 CookTorranceBRDF::evaluate_iridescence(
    float cos_theta,
    float thickness_nm,
    float film_ior
) noexcept {
    // Physical thin-film optical path difference (OPD) spectral interference
    const float cos_t1 = std::clamp(cos_theta, 0.0f, 1.0f);
    const float sin2_t1 = std::max(0.0f, 1.0f - cos_t1 * cos_t1);
    const float sin2_t2 = sin2_t1 / std::max(film_ior * film_ior, 1.0f);
    const float cos_t2 = std::sqrt(std::max(0.0f, 1.0f - sin2_t2));

    // Optical path difference in nanometers (with half-wave phase shift)
    const float opd = 2.0f * film_ior * thickness_nm * cos_t2;

    // Evaluate interference across RGB primary wavelengths (650nm, 530nm, 460nm)
    const float phase_r = TWO_PI * opd / 650.0f + PI;
    const float phase_g = TWO_PI * opd / 530.0f + PI;
    const float phase_b = TWO_PI * opd / 460.0f + PI;

    const float r = 0.5f + 0.5f * std::cos(phase_r);
    const float g = 0.5f + 0.5f * std::cos(phase_g);
    const float b = 0.5f + 0.5f * std::cos(phase_b);

    return Vec3(
        std::clamp(r, 0.0f, 1.0f),
        std::clamp(g, 0.0f, 1.0f),
        std::clamp(b, 0.0f, 1.0f)
    );
}

} // namespace orion

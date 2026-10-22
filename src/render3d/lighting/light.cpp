#include "nisaba/render3d/lighting/light.hpp"

namespace nisaba::render3d {

Light3D Light3D::create_directional(
    const Vec3& in_direction,
    const Vec3& in_color,
    float in_intensity
) noexcept {
    Light3D l;
    l.type = LightType::Directional;
    l.direction = in_direction.normalized();
    l.color = in_color;
    l.intensity = in_intensity;
    return l;
}

Light3D Light3D::create_point(
    const Vec3& in_position,
    float in_range,
    const Vec3& in_color,
    float in_intensity
) noexcept {
    Light3D l;
    l.type = LightType::Point;
    l.position = in_position;
    l.range = std::max(0.1f, in_range);
    l.color = in_color;
    l.intensity = in_intensity;
    return l;
}

Light3D Light3D::create_spot(
    const Vec3& in_position,
    const Vec3& in_direction,
    float in_inner_cone_rad,
    float in_outer_cone_rad,
    float in_range,
    const Vec3& in_color,
    float in_intensity
) noexcept {
    Light3D l;
    l.type = LightType::Spot;
    l.position = in_position;
    l.direction = in_direction.normalized();
    l.inner_cone_angle = std::min(in_inner_cone_rad, in_outer_cone_rad);
    l.outer_cone_angle = in_outer_cone_rad;
    l.range = std::max(0.1f, in_range);
    l.color = in_color;
    l.intensity = in_intensity;
    return l;
}

float Light3D::compute_attenuation(float distance) const noexcept {
    if (type == LightType::Directional) return 1.0f;
    if (distance <= 0.0f) return 1.0f;
    if (distance >= range) return 0.0f;

    const float d_over_r = distance / range;
    const float d4 = d_over_r * d_over_r * d_over_r * d_over_r;
    const float w = std::clamp(1.0f - d4, 0.0f, 1.0f);
    const float window = w * w;

    const float inv_sq = 1.0f / (distance * distance + 1.0f);
    return inv_sq * window;
}

float Light3D::compute_spot_falloff(const Vec3& dir_to_light) const noexcept {
    if (type != LightType::Spot) return 1.0f;

    // Spot direction points from light outward; dir_to_light points from point to light.
    const float cos_theta = (-direction).dot(dir_to_light.normalized());
    const float cos_inner = std::cos(inner_cone_angle);
    const float cos_outer = std::cos(outer_cone_angle);

    const float denom = cos_inner - cos_outer;
    if (denom > 1e-5f) {
        const float t = std::clamp((cos_theta - cos_outer) / denom, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t); // Smoothstep curve
    }
    return (cos_theta >= cos_outer) ? 1.0f : 0.0f;
}

LightUniforms Light3D::get_uniforms() const noexcept {
    LightUniforms u;

    u.color_intensity[0] = color.x;
    u.color_intensity[1] = color.y;
    u.color_intensity[2] = color.z;
    u.color_intensity[3] = intensity;

    u.position_range[0] = position.x;
    u.position_range[1] = position.y;
    u.position_range[2] = position.z;
    u.position_range[3] = range;

    u.direction_type[0] = direction.x;
    u.direction_type[1] = direction.y;
    u.direction_type[2] = direction.z;
    u.direction_type[3] = static_cast<float>(type);

    u.spot_shadow[0] = std::cos(inner_cone_angle);
    u.spot_shadow[1] = std::cos(outer_cone_angle);
    u.spot_shadow[2] = shadow_bias;
    u.spot_shadow[3] = casts_shadows ? 1.0f : 0.0f;

    return u;
}

} // namespace nisaba::render3d

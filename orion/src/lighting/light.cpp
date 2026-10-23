#include "orion/lighting/light.hpp"

namespace orion {

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

Light3D Light3D::create_sphere(
    const Vec3& in_position,
    float in_sphere_radius,
    float in_range,
    const Vec3& in_color,
    float in_intensity
) noexcept {
    Light3D l;
    l.type = LightType::Sphere;
    l.position = in_position;
    l.sphere_radius = std::max(0.001f, in_sphere_radius);
    l.range = std::max(l.sphere_radius + 0.1f, in_range);
    l.color = in_color;
    l.intensity = in_intensity;
    return l;
}

Vec3 Light3D::compute_representative_point(
    const Vec3& surface_pos,
    const Vec3& reflection_dir
) const noexcept {
    const Vec3 center_to_surface = position - surface_pos;
    const float r = sphere_radius;
    if (r <= 1e-4f) {
        return center_to_surface;
    }

    const Vec3 R = reflection_dir.normalized();
    const float t = center_to_surface.dot(R);

    if (t <= 0.0f) {
        return center_to_surface;
    }

    // Closest point along reflection ray
    const Vec3 ray_pt = R * t;
    // Vector from center to closest point on ray
    const Vec3 center_to_ray = ray_pt - center_to_surface;
    const float dist_sq = center_to_ray.length_sq();

    if (dist_sq <= r * r) {
        // Ray intersects the sphere
        return ray_pt;
    }

    const float dist = std::sqrt(dist_sq);
    // Point on sphere surface closest to ray
    return center_to_surface + center_to_ray * (r / dist);
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

GpuLightData Light3D::get_gpu_data(int32_t shadow_index) const noexcept {
    GpuLightData d;
    d.position_radius[0] = position.x;
    d.position_radius[1] = position.y;
    d.position_radius[2] = position.z;
    d.position_radius[3] = range;

    d.color_intensity[0] = color.x;
    d.color_intensity[1] = color.y;
    d.color_intensity[2] = color.z;
    d.color_intensity[3] = intensity;

    d.direction_inner[0] = direction.x;
    d.direction_inner[1] = direction.y;
    d.direction_inner[2] = direction.z;
    d.direction_inner[3] = std::cos(inner_cone_angle);

    d.parameters[0] = std::cos(outer_cone_angle);
    d.parameters[1] = static_cast<float>(type);
    d.parameters[2] = sphere_radius;
    d.parameters[3] = static_cast<float>(shadow_index);

    return d;
}

} // namespace orion

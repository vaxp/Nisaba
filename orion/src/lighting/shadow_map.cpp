#include "orion/lighting/shadow_map.hpp"
#include <algorithm>
#include <cmath>

namespace orion {

ShadowMap::ShadowMap(uint32_t w, uint32_t h)
    : width(w), height(h) {
    depth_buffer.resize(static_cast<size_t>(w) * h, 1.0f);
}

void ShadowMap::clear(float clear_depth) noexcept {
    std::fill(depth_buffer.begin(), depth_buffer.end(), clear_depth);
}

void ShadowMap::set_depth(uint32_t x, uint32_t y, float depth) noexcept {
    if (x < width && y < height) {
        depth_buffer[static_cast<size_t>(y) * width + x] = depth;
    }
}

float ShadowMap::get_depth(uint32_t x, uint32_t y) const noexcept {
    if (x < width && y < height) {
        return depth_buffer[static_cast<size_t>(y) * width + x];
    }
    return 1.0f;
}

float ShadowMap::sample_shadow_pcf(const Vec3& world_pos, float bias) const noexcept {
    const Vec3 clip = light_vp_matrix.transform_point(world_pos);

    // Convert from clip coordinates to UV [0, 1]
    const float u = clip.x * 0.5f + 0.5f;
    const float v = clip.y * 0.5f + 0.5f;
    const float z = clip.z;

    // Out of bounds is fully lit
    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f || z > 1.0f) {
        return 1.0f;
    }

    const float fx = u * static_cast<float>(width > 1 ? width - 1 : 0);
    const float fy = v * static_cast<float>(height > 1 ? height - 1 : 0);

    const uint32_t ix = static_cast<uint32_t>(fx);
    const uint32_t iy = static_cast<uint32_t>(fy);

    float shadow = 0.0f;
    for (int dy = 0; dy <= 1; ++dy) {
        for (int dx = 0; dx <= 1; ++dx) {
            const uint32_t px = std::min(ix + dx, width - 1);
            const uint32_t py = std::min(iy + dy, height - 1);
            const float d = get_depth(px, py);
            if ((z - bias) <= d) {
                shadow += 1.0f;
            }
        }
    }
    return shadow * 0.25f;
}

Mat44 ShadowMap::compute_directional_shadow_matrix(
    const Light3D& sun,
    const Camera3D& camera,
    float shadow_distance,
    uint32_t shadow_resolution
) noexcept {
    const float z_near = camera.get_near_clip();
    const float z_far = std::min(camera.get_far_clip(), shadow_distance);

    const float fov_y = camera.get_fov_y();
    const float aspect = camera.get_aspect_ratio();
    const float tan_half_fov = std::tan(fov_y * 0.5f);

    const float h_near = 2.0f * tan_half_fov * z_near;
    const float w_near = h_near * aspect;
    const float h_far = 2.0f * tan_half_fov * z_far;
    const float w_far = h_far * aspect;

    // 8 Frustum corners in camera view space
    const Vec3 corners_view[8] = {
        Vec3(-w_near * 0.5f, -h_near * 0.5f, -z_near),
        Vec3( w_near * 0.5f, -h_near * 0.5f, -z_near),
        Vec3( w_near * 0.5f,  h_near * 0.5f, -z_near),
        Vec3(-w_near * 0.5f,  h_near * 0.5f, -z_near),

        Vec3(-w_far * 0.5f, -h_far * 0.5f, -z_far),
        Vec3( w_far * 0.5f, -h_far * 0.5f, -z_far),
        Vec3( w_far * 0.5f,  h_far * 0.5f, -z_far),
        Vec3(-w_far * 0.5f,  h_far * 0.5f, -z_far)
    };

    // Transform to world space using inverse view matrix
    const Mat44 inv_view = camera.get_view_matrix().inverse_fast_rigid();
    Vec3 centroid(0.0f, 0.0f, 0.0f);
    Vec3 corners_world[8];

    for (int i = 0; i < 8; ++i) {
        corners_world[i] = inv_view.transform_point(corners_view[i]);
        centroid = centroid + corners_world[i];
    }
    centroid = centroid * (1.0f / 8.0f);

    // Light View Matrix
    const Vec3 light_dir = sun.direction.normalized();
    const Vec3 light_pos = centroid - light_dir * 100.0f;
    const Mat44 light_view = Mat44::look_at(light_pos, centroid, Vec3(0.0f, 1.0f, 0.0f));

    // Transform corners to light space
    Vec3 min_pt( 1e9f,  1e9f,  1e9f);
    Vec3 max_pt(-1e9f, -1e9f, -1e9f);

    for (int i = 0; i < 8; ++i) {
        const Vec3 pt_light = light_view.transform_point(corners_world[i]);
        min_pt = min_pt.min(pt_light);
        max_pt = max_pt.max(pt_light);
    }

    // Expand Z depth to catch shadow casters in front of the frustum
    min_pt.z -= 100.0f;
    max_pt.z += 100.0f;

    // Texel snapping to eliminate shadow edge shimmering
    const float res = static_cast<float>(shadow_resolution);
    const float units_per_texel_x = (max_pt.x - min_pt.x) / res;
    const float units_per_texel_y = (max_pt.y - min_pt.y) / res;

    if (units_per_texel_x > 1e-6f) {
        min_pt.x = std::floor(min_pt.x / units_per_texel_x) * units_per_texel_x;
        max_pt.x = std::floor(max_pt.x / units_per_texel_x) * units_per_texel_x;
    }
    if (units_per_texel_y > 1e-6f) {
        min_pt.y = std::floor(min_pt.y / units_per_texel_y) * units_per_texel_y;
        max_pt.y = std::floor(max_pt.y / units_per_texel_y) * units_per_texel_y;
    }

    // Orthographic projection for Vulkan (Z in [0, 1])
    const float dx = max_pt.x - min_pt.x;
    const float dy = max_pt.y - min_pt.y;
    const float dz = max_pt.z - min_pt.z;

    Mat44 light_proj = Mat44::identity();
    light_proj.m[0]  = (dx > 1e-6f) ? (2.0f / dx) : 1.0f;
    light_proj.m[3]  = (dx > 1e-6f) ? (-(max_pt.x + min_pt.x) / dx) : 0.0f;
    light_proj.m[5]  = (dy > 1e-6f) ? (2.0f / dy) : 1.0f;
    light_proj.m[7]  = (dy > 1e-6f) ? (-(max_pt.y + min_pt.y) / dy) : 0.0f;
    light_proj.m[10] = (dz > 1e-6f) ? (1.0f / dz) : 0.0f;
    light_proj.m[11] = (dz > 1e-6f) ? (-min_pt.z / dz) : 0.0f;
    light_proj.m[15] = 1.0f;

    return light_proj * light_view;
}

Vec3 ShadowMap::get_vogel_disk_sample(size_t index, size_t count, float phi) noexcept {
    constexpr float golden_angle = 2.39996323f; // ~137.5 degrees in radians
    const float r = std::sqrt(static_cast<float>(index) + 0.5f) / std::sqrt(static_cast<float>(count));
    const float theta = static_cast<float>(index) * golden_angle + phi;
    return Vec3(r * std::cos(theta), r * std::sin(theta), 0.0f);
}

float ShadowMap::sample_shadow_pcss(
    const Vec3& world_pos,
    const Vec3& normal,
    float light_size,
    float bias
) const noexcept {
    // Normal-bias offset prevents surface acne
    const Vec3 biased_pos = world_pos + normal * 0.012f;
    const Vec3 clip = light_vp_matrix.transform_point(biased_pos);

    const float u = clip.x * 0.5f + 0.5f;
    const float v = clip.y * 0.5f + 0.5f;
    const float z = clip.z;

    if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f || z < 0.0f || z > 1.0f) {
        return 1.0f;
    }

    const float texel_w = 1.0f / static_cast<float>(width > 1 ? width : 1);
    const float texel_h = 1.0f / static_cast<float>(height > 1 ? height : 1);

    // 1. Blocker Search
    constexpr size_t blocker_samples = 16;
    const float search_radius = std::clamp(light_size * 20.0f * z, 1.0f, 15.0f);

    float blocker_sum = 0.0f;
    int blocker_count = 0;

    for (size_t i = 0; i < blocker_samples; ++i) {
        const Vec3 disk = get_vogel_disk_sample(i, blocker_samples);
        const float sample_u = u + disk.x * search_radius * texel_w;
        const float sample_v = v + disk.y * search_radius * texel_h;

        if (sample_u >= 0.0f && sample_u <= 1.0f && sample_v >= 0.0f && sample_v <= 1.0f) {
            const uint32_t px = static_cast<uint32_t>(sample_u * static_cast<float>(width - 1));
            const uint32_t py = static_cast<uint32_t>(sample_v * static_cast<float>(height - 1));
            const float sample_d = get_depth(px, py);

            if (sample_d < (z - bias)) {
                blocker_sum += sample_d;
                blocker_count++;
            }
        }
    }

    if (blocker_count == 0) {
        return 1.0f; // No occluders in search area: fully illuminated
    }

    const float avg_blocker_depth = blocker_sum / static_cast<float>(blocker_count);

    // 2. Penumbra Estimation: w = (d_receiver - d_blocker) / d_blocker * light_size
    const float penumbra_ratio = std::max(0.0f, (z - avg_blocker_depth) / std::max(1e-4f, avg_blocker_depth));
    const float filter_radius = std::clamp(penumbra_ratio * light_size * 25.0f, 1.0f, 20.0f);

    // 3. Percentage Closer Filtering with adaptive penumbra size
    constexpr size_t pcf_samples = 24;
    float shadow_sum = 0.0f;

    for (size_t i = 0; i < pcf_samples; ++i) {
        const Vec3 disk = get_vogel_disk_sample(i, pcf_samples);
        const float sample_u = u + disk.x * filter_radius * texel_w;
        const float sample_v = v + disk.y * filter_radius * texel_h;

        if (sample_u >= 0.0f && sample_u <= 1.0f && sample_v >= 0.0f && sample_v <= 1.0f) {
            const uint32_t px = static_cast<uint32_t>(sample_u * static_cast<float>(width - 1));
            const uint32_t py = static_cast<uint32_t>(sample_v * static_cast<float>(height - 1));
            const float sample_d = get_depth(px, py);

            if ((z - bias) <= sample_d) {
                shadow_sum += 1.0f;
            }
        } else {
            shadow_sum += 1.0f;
        }
    }

    return shadow_sum / static_cast<float>(pcf_samples);
}

Mat44 ShadowMap::compute_spot_shadow_matrix(
    const Light3D& spot_light,
    float near_z
) noexcept {
    const Vec3 pos = spot_light.position;
    const Vec3 dir = spot_light.direction.normalized();
    const Vec3 up = (std::abs(dir.y) > 0.99f) ? Vec3(0.0f, 0.0f, 1.0f) : Vec3(0.0f, 1.0f, 0.0f);
    const Mat44 view = Mat44::look_at(pos, pos + dir, up);

    const float fov_y = std::clamp(spot_light.outer_cone_angle * 2.0f, 0.1f, 3.1f);
    const float far_z = std::max(near_z + 1.0f, spot_light.range);

    // Perspective projection for Vulkan/OpenGL depth range [0, 1]
    const Mat44 proj = Mat44::perspective(fov_y, 1.0f, near_z, far_z);
    return proj * view;
}

} // namespace orion

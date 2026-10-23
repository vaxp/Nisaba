#pragma once

#include "orion/math/vec3.hpp"
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace orion {


enum class LightType : uint32_t {
    Directional = 0,
    Point = 1,
    Spot = 2,
    Sphere = 3
};

/// @brief 64-byte std140 GPU uniform structure for a single light source.
struct alignas(16) GpuLightData {
    float position_radius[4]{0.0f, 0.0f, 0.0f, 20.0f};    // pos.x, pos.y, pos.z, range
    float color_intensity[4]{1.0f, 1.0f, 1.0f, 1.0f};    // color.r, color.g, color.b, intensity
    float direction_inner[4]{0.0f, -1.0f, 0.0f, 0.7071f}; // dir.x, dir.y, dir.z, cos(inner_cone)
    float parameters[4]{0.6427f, 0.0f, 0.0f, -1.0f};      // cos(outer_cone), light_type, sphere_radius, shadow_idx
};

static_assert(sizeof(GpuLightData) == 64, "GpuLightData must be exactly 64 bytes");
static_assert(alignof(GpuLightData) == 16, "GpuLightData must be 16-byte aligned");

/// @brief std140 uniform buffer block containing up to 128 active dynamic lights.
struct alignas(16) GpuLightingUboBlock {
    GpuLightData lights[128];
    uint32_t light_count{0};
    uint32_t cluster_dim_x{16};
    uint32_t cluster_dim_y{9};
    uint32_t cluster_dim_z{24};
    float cluster_z_params[4]{0.1f, 100.0f, 0.0f, 0.0f}; // near, far, log_factor, slice_factor
    float ambient_color[4]{0.1f, 0.1f, 0.1f, 1.0f};      // rgb, intensity
};

static_assert(sizeof(GpuLightingUboBlock) == 8240, "GpuLightingUboBlock must be exactly 8240 bytes");
static_assert(alignof(GpuLightingUboBlock) == 16, "GpuLightingUboBlock must be 16-byte aligned");

/// @brief Legacy 64-byte std140-compatible Uniform structure for a single light source.
struct alignas(16) LightUniforms {
    float color_intensity[4]{1.0f, 1.0f, 1.0f, 1.0f};  // RGB + Intensity
    float position_range[4]{0.0f, 0.0f, 0.0f, 20.0f};  // World Position + Range
    float direction_type[4]{0.0f, -1.0f, 0.0f, 0.0f};  // Normalized Direction + LightType
    float spot_shadow[4]{0.0f, 0.0f, 0.002f, 1.0f};    // cos(inner), cos(outer), shadow_bias, casts_shadows
};

static_assert(sizeof(LightUniforms) == 64, "LightUniforms must be exactly 64 bytes");
static_assert(alignof(LightUniforms) == 16, "LightUniforms must be 16-byte aligned");

/// @brief Analytical light source supporting directional sunlight, radial point lights,
/// conical spotlights, and spherical area lights with physically-based attenuation.
class Light3D {
public:
    LightType type{LightType::Directional};
    Vec3 color{1.0f, 1.0f, 1.0f};
    float intensity{3.0f};

    Vec3 position{0.0f, 10.0f, 0.0f};
    Vec3 direction{0.0f, -1.0f, 0.0f};
    float range{20.0f};

    float inner_cone_angle{0.785398f}; // 45 degrees in radians
    float outer_cone_angle{0.872665f}; // 50 degrees in radians

    float sphere_radius{0.0f};         // For spherical area light sources

    bool casts_shadows{true};
    float shadow_bias{0.002f};

    Light3D() = default;

    /// @brief Creates an infinitely distant directional light source (e.g. sun).
    [[nodiscard]] static Light3D create_directional(
        const Vec3& in_direction,
        const Vec3& in_color = Vec3(1.0f, 1.0f, 1.0f),
        float in_intensity = 3.0f
    ) noexcept;

    /// @brief Creates an omnidirectional point light with smooth inverse-square range cutoff.
    [[nodiscard]] static Light3D create_point(
        const Vec3& in_position,
        float in_range,
        const Vec3& in_color = Vec3(1.0f, 1.0f, 1.0f),
        float in_intensity = 10.0f
    ) noexcept;

    /// @brief Creates a directed spot light with conical penumbra falloff.
    [[nodiscard]] static Light3D create_spot(
        const Vec3& in_position,
        const Vec3& in_direction,
        float in_inner_cone_rad,
        float in_outer_cone_rad,
        float in_range,
        const Vec3& in_color = Vec3(1.0f, 1.0f, 1.0f),
        float in_intensity = 15.0f
    ) noexcept;

    /// @brief Creates a spherical area light with physically modeled representative point specular highlights.
    [[nodiscard]] static Light3D create_sphere(
        const Vec3& in_position,
        float in_sphere_radius,
        float in_range,
        const Vec3& in_color = Vec3(1.0f, 1.0f, 1.0f),
        float in_intensity = 15.0f
    ) noexcept;

    /// @brief Calculates inverse-square physical distance attenuation with smooth edge windowing.
    [[nodiscard]] float compute_attenuation(float distance) const noexcept;

    /// @brief Calculates angular spot cone penumbra multiplier [0, 1].
    [[nodiscard]] float compute_spot_falloff(const Vec3& dir_to_light) const noexcept;

    /// @brief Calculates the representative point on the sphere surface closest to the reflection ray.
    [[nodiscard]] Vec3 compute_representative_point(
        const Vec3& surface_pos,
        const Vec3& reflection_dir
    ) const noexcept;

    /// @brief Formats parameters into 64-byte std140 uniform buffer block.
    [[nodiscard]] LightUniforms get_uniforms() const noexcept;

    /// @brief Formats parameters into modern std140 GpuLightData for multi-light rendering.
    [[nodiscard]] GpuLightData get_gpu_data(int32_t shadow_index = -1) const noexcept;
};

} // namespace orion

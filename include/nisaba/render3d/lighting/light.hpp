#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace nisaba::render3d {

using Vec3 = nisaba::physics::Vec3;

enum class LightType : uint32_t {
    Directional = 0,
    Point = 1,
    Spot = 2
};

/// @brief 64-byte std140-compatible Uniform structure for a single light source.
struct alignas(16) LightUniforms {
    float color_intensity[4]{1.0f, 1.0f, 1.0f, 1.0f};  // RGB + Intensity
    float position_range[4]{0.0f, 0.0f, 0.0f, 20.0f};  // World Position + Range
    float direction_type[4]{0.0f, -1.0f, 0.0f, 0.0f};  // Normalized Direction + LightType
    float spot_shadow[4]{0.0f, 0.0f, 0.002f, 1.0f};    // cos(inner), cos(outer), shadow_bias, casts_shadows
};

static_assert(sizeof(LightUniforms) == 64, "LightUniforms must be exactly 64 bytes");
static_assert(alignof(LightUniforms) == 16, "LightUniforms must be 16-byte aligned");

/// @brief Analytical light source supporting directional sunlight, radial point lights,
/// and conical spotlights with physically-based attenuation.
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

    /// @brief Calculates inverse-square physical distance attenuation with smooth edge windowing.
    [[nodiscard]] float compute_attenuation(float distance) const noexcept;

    /// @brief Calculates angular spot cone penumbra multiplier [0, 1].
    [[nodiscard]] float compute_spot_falloff(const Vec3& dir_to_light) const noexcept;

    /// @brief Formats parameters into 64-byte std140 uniform buffer block.
    [[nodiscard]] LightUniforms get_uniforms() const noexcept;
};

} // namespace nisaba::render3d

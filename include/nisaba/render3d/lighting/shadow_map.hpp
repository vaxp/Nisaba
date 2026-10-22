#pragma once

#include "nisaba/physics/math/mat44.hpp"
#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/render3d/lighting/light.hpp"
#include "nisaba/render3d/camera/camera.hpp"
#include <vector>
#include <cstdint>

namespace nisaba::render3d {

using Mat44 = nisaba::physics::Mat44;
using Vec3 = nisaba::physics::Vec3;

/// @brief High-precision shadow map buffer and Cascaded Shadow Mapping (CSM) projection solver.
class ShadowMap {
public:
    uint32_t width{1024};
    uint32_t height{1024};
    std::vector<float> depth_buffer;

    Mat44 light_view_matrix{};
    Mat44 light_proj_matrix{};
    Mat44 light_vp_matrix{};

    ShadowMap(uint32_t w = 1024, uint32_t h = 1024);

    /// @brief Clears depth buffer to 1.0f (maximum depth).
    void clear(float clear_depth = 1.0f) noexcept;

    /// @brief Sets raw depth value at pixel (x, y).
    void set_depth(uint32_t x, uint32_t y, float depth) noexcept;

    /// @brief Gets depth at pixel (x, y).
    [[nodiscard]] float get_depth(uint32_t x, uint32_t y) const noexcept;

    /// @brief Samples shadow factor at a world-space point using 4-tap Percentage Closer Filtering (PCF).
    /// @return Visibility multiplier: 1.0 = fully lit, 0.0 = fully in shadow.
    [[nodiscard]] float sample_shadow_pcf(const Vec3& world_pos, float bias = 0.002f) const noexcept;

    /// @brief Computes stable, shimmering-free Cascaded Shadow Mapping (CSM) matrix for directional sunlight.
    /// Snaps projection bounds to shadow texel increments.
    [[nodiscard]] static Mat44 compute_directional_shadow_matrix(
        const Light3D& sun,
        const Camera3D& camera,
        float shadow_distance = 50.0f,
        uint32_t shadow_resolution = 1024
    ) noexcept;
};

} // namespace nisaba::render3d

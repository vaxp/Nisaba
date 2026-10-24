#pragma once

#include "orion/math/mat44.hpp"
#include "orion/math/vec3.hpp"
#include "orion/lighting/light.hpp"
#include "orion/camera/camera.hpp"
#include <vector>
#include <cstdint>

namespace orion {


/// @brief Configuration settings controlling the advanced shadowing pipeline (CSM, PCSS, Contact Shadows, and Bias).
struct ShadowSettings {
    static constexpr uint32_t MAX_CASCADES = 4;

    bool enable_shadows{true};
    bool enable_pcss{false};
    bool enable_contact_shadows{false};
    uint32_t cascade_count{1};           // Number of active CSM cascades (1..4)
    float cascade_split_lambda{0.65f};   // Practical split scheme blend [0=uniform, 1=logarithmic]
    float cascade_blend_fraction{0.1f};  // Smooth transition zone fraction across cascade boundaries
    float light_size{0.035f};            // Angular diameter for PCSS penumbra widening
    float contact_shadow_length{0.25f};   // World units for contact shadow search
    float contact_shadow_thickness{0.05f};// World units for contact shadow thickness
    uint32_t contact_shadow_steps{24};    // Screen-space raymarching steps
    float normal_bias{0.012f};            // Normal offset to eliminate surface acne
    float depth_bias{0.001f};             // Slope depth bias
    float filter_radius{2.5f};            // Base PCF filter radius in texels
};

/// @brief High-precision shadow map buffer and Cascaded Shadow Mapping (CSM) projection solver.
class ShadowMap {
public:
    static constexpr uint32_t MAX_CASCADES = 4;

    uint32_t width{1024};
    uint32_t height{1024};
    std::vector<float> depth_buffer;

    Mat44 light_view_matrix{};
    Mat44 light_proj_matrix{};
    Mat44 light_vp_matrix{};

    Mat44 cascade_matrices[MAX_CASCADES]{};
    float cascade_splits[MAX_CASCADES]{12.5f, 25.0f, 37.5f, 50.0f};

    ShadowSettings settings{};

    ShadowMap(uint32_t w = 1024, uint32_t h = 1024);

    /// @brief Clears depth buffer to 1.0f (maximum depth).
    void clear(float clear_depth = 1.0f) noexcept;

    /// @brief Sets raw depth value at pixel (x, y).
    void set_depth(uint32_t x, uint32_t y, float depth) noexcept;

    /// @brief Gets depth at pixel (x, y).
    [[nodiscard]] float get_depth(uint32_t x, uint32_t y) const noexcept;

    /// @brief Computes 2D Vogel disk sampling position on unit circle.
    [[nodiscard]] static Vec3 get_vogel_disk_sample(size_t index, size_t count, float phi = 0.0f) noexcept;

    /// @brief Samples shadow factor at a world-space point using 4-tap Percentage Closer Filtering (PCF).
    /// @return Visibility multiplier: 1.0 = fully lit, 0.0 = fully in shadow.
    [[nodiscard]] float sample_shadow_pcf(const Vec3& world_pos, float bias = 0.002f) const noexcept;

    /// @brief Samples Percentage Closer Soft Shadow (PCSS) with blocker search and contact hardening.
    /// @return Visibility multiplier in [0, 1] (1 = fully lit, 0 = in shadow).
    [[nodiscard]] float sample_shadow_pcss(
        const Vec3& world_pos,
        const Vec3& normal,
        float light_size = 0.035f,
        float bias = 0.002f
    ) const noexcept;

    /// @brief Computes stable, shimmering-free Cascaded Shadow Mapping (CSM) matrix for directional sunlight.
    /// Snaps projection bounds to shadow texel increments.
    [[nodiscard]] static Mat44 compute_directional_shadow_matrix(
        const Light3D& sun,
        const Camera3D& camera,
        float shadow_distance = 50.0f,
        uint32_t shadow_resolution = 1024
    ) noexcept;

    /// @brief Computes stable directional shadow projection matrix for a specific frustum sub-slice [slice_near, slice_far].
    [[nodiscard]] static Mat44 compute_directional_slice_matrix(
        const Light3D& sun,
        const Camera3D& camera,
        float slice_near,
        float slice_far,
        uint32_t shadow_resolution = 1024
    ) noexcept;

    /// @brief Computes 4-cascade split distances and texel-snapped light view-projection matrices
    /// using the Practical Split Scheme (hybrid logarithmic + uniform).
    static void compute_cascades(
        const Light3D& sun,
        const Camera3D& camera,
        float shadow_distance,
        uint32_t shadow_resolution,
        Mat44 out_matrices[MAX_CASCADES],
        float out_splits[MAX_CASCADES],
        float lambda = 0.65f
    ) noexcept;

    /// @brief Selects the optimal cascade index [0..3] for a given positive view-space depth.
    [[nodiscard]] uint32_t select_cascade(float view_depth) const noexcept;

    /// @brief Computes perspective shadow projection matrix for a spotlight.
    [[nodiscard]] static Mat44 compute_spot_shadow_matrix(
        const Light3D& spot_light,
        float near_z = 0.1f
    ) noexcept;
};

} // namespace orion

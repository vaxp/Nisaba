#pragma once

#include "orion/math/vec3.hpp"
#include <vector>
#include <cstdint>

namespace orion {


/// @brief Precomputed 2D Look-Up Table (LUT) generator for Split-Sum Cook-Torrance
/// Environment Specular BRDF integration (DFG LUT).
class DfgLutGenerator {
public:
    static constexpr uint32_t DEFAULT_WIDTH = 256;
    static constexpr uint32_t DEFAULT_HEIGHT = 256;
    static constexpr uint32_t DEFAULT_SAMPLES = 512;

    /// @brief Generates a 2D RGBA float buffer (width * height * 4 floats) storing:
    /// - Red:   Scale factor (I1) for Fresnel F0
    /// - Green: Bias factor (I2) for Fresnel F0
    /// - Blue:  Charlie sheen fabric reflection factor
    /// - Alpha: 1.0
    [[nodiscard]] static std::vector<float> generate_table(
        uint32_t width = DEFAULT_WIDTH,
        uint32_t height = DEFAULT_HEIGHT,
        uint32_t num_samples = DEFAULT_SAMPLES
    ) noexcept;

    /// @brief Evaluates Split-Sum DFG integration for a single (N.V, roughness) pair.
    [[nodiscard]] static Vec3 integrate_dfg(
        float n_dot_v,
        float roughness,
        uint32_t num_samples = DEFAULT_SAMPLES
    ) noexcept;

    /// @brief Rapid analytical polynomial approximation for real-time CPU evaluation.
    [[nodiscard]] static Vec3 sample_analytical(
        float n_dot_v,
        float roughness
    ) noexcept;
};

} // namespace orion

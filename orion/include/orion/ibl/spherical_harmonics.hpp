#pragma once

#include "orion/math/vec3.hpp"
#include <array>
#include <cmath>

namespace orion {


/// @brief 3-Band (9 coefficients) Spherical Harmonics representation for low-frequency
/// diffuse irradiance convolution and fast O(1) analytical environment lighting.
class SphericalHarmonics9 {
public:
    // 9 RGB coefficients representing order l in [0, 2]
    std::array<Vec3, 9> coefficients{};

    constexpr SphericalHarmonics9() noexcept {
        coefficients.fill(Vec3(0.0f, 0.0f, 0.0f));
    }

    /// @brief Evaluates diffuse irradiance for a surface with unit normal N using
    /// Ramamoorthi & Hanrahan analytical formulation.
    [[nodiscard]] Vec3 evaluate(const Vec3& normal) const noexcept;

    /// @brief Scales all harmonic coefficients by a scalar factor.
    void scale(float factor) noexcept;

    /// @brief Adds another spherical harmonics field.
    void add(const SphericalHarmonics9& other) noexcept;

    /// @brief Pre-configured analytical daytime sunny sky environment.
    [[nodiscard]] static SphericalHarmonics9 create_daylight(const Vec3& sun_dir) noexcept;

    /// @brief Pre-configured analytical sunset environment with warm golden and purple hues.
    [[nodiscard]] static SphericalHarmonics9 create_sunset() noexcept;

    /// @brief Pre-configured analytical neutral overcast studio lighting.
    [[nodiscard]] static SphericalHarmonics9 create_studio() noexcept;

    /// @brief Raw array access for direct std140 uniform buffer uploads.
    [[nodiscard]] const float* data() const noexcept {
        return reinterpret_cast<const float*>(coefficients.data());
    }
};

} // namespace orion

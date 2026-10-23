#pragma once

#include <algorithm>
#include <cmath>

namespace orion::physics {

enum class CombineMode : uint8_t {
    Average = 0,
    Minimum = 1,
    Maximum = 2,
    GeometricMean = 3,
    Multiply = 4
};

/// @brief Combines friction coefficients of two colliding materials.
/// Default industry standard is GeometricMean: sqrt(friction_a * friction_b).
[[nodiscard]] inline float combine_friction(
    float friction_a,
    float friction_b,
    CombineMode mode = CombineMode::GeometricMean
) noexcept {
    switch (mode) {
        case CombineMode::Average:
            return 0.5f * (friction_a + friction_b);
        case CombineMode::Minimum:
            return std::min(friction_a, friction_b);
        case CombineMode::Maximum:
            return std::max(friction_a, friction_b);
        case CombineMode::Multiply:
            return friction_a * friction_b;
        case CombineMode::GeometricMean:
        default:
            return std::sqrt(std::max(0.0f, friction_a) * std::max(0.0f, friction_b));
    }
}

/// @brief Combines restitution (elasticity) coefficients of two colliding materials.
/// Default industry standard is Maximum: max(restitution_a, restitution_b).
[[nodiscard]] inline float combine_restitution(
    float restitution_a,
    float restitution_b,
    CombineMode mode = CombineMode::Maximum
) noexcept {
    switch (mode) {
        case CombineMode::Average:
            return 0.5f * (restitution_a + restitution_b);
        case CombineMode::Minimum:
            return std::min(restitution_a, restitution_b);
        case CombineMode::Multiply:
            return restitution_a * restitution_b;
        case CombineMode::GeometricMean:
            return std::sqrt(std::max(0.0f, restitution_a) * std::max(0.0f, restitution_b));
        case CombineMode::Maximum:
        default:
            return std::max(restitution_a, restitution_b);
    }
}

} // namespace orion::physics

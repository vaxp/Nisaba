#pragma once

#include "nisaba/physics/collision/gjk.hpp"

namespace nisaba::physics {

/// @brief Result of running the Expanding Polytope Algorithm (EPA)
struct EPAResult {
    bool success{false};
    Vec3 normal{0.0f, 1.0f, 0.0f};      // Contact normal pointing from Body A towards Body B
    float penetration_depth{0.0f};      // Exact penetration depth (> 0)
    Vec3 point_a{0.0f, 0.0f, 0.0f};     // Contact point on Body A in world space
    Vec3 point_b{0.0f, 0.0f, 0.0f};     // Contact point on Body B in world space
    uint32_t iterations{0};
};

/// @brief Expanding Polytope Algorithm (EPA) for deep and shallow penetration depth calculation.
/// Expands the terminating GJK simplex polytope to find the Minimum Translational Vector (MTV).
class EPA {
public:
    static constexpr size_t MAX_VERTICES = 128;
    static constexpr size_t MAX_FACES = 256;
    static constexpr size_t MAX_EDGES = 192;

    /// @brief Runs EPA to compute exact contact normal, penetration depth, and contact points
    [[nodiscard]] static EPAResult query(
        const Shape& shape_a, const Transform3D& transform_a,
        const Shape& shape_b, const Transform3D& transform_b,
        const Simplex& gjk_simplex,
        uint32_t max_iterations = 48,
        float tolerance = 2e-3f
    ) noexcept;
};

} // namespace nisaba::physics

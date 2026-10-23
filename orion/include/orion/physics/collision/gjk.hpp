#pragma once

#include "orion/math/vec3.hpp"
#include "orion/math/transform3d.hpp"
#include "orion/physics/collision/shape.hpp"
#include <array>
#include <cstdint>
#include <algorithm>

namespace orion::physics {

/// @brief A support vertex in the Minkowski difference A - B
struct SimplexVertex {
    Vec3 point{0.0f, 0.0f, 0.0f};   // w = point_a - point_b (in Minkowski space)
    Vec3 point_a{0.0f, 0.0f, 0.0f}; // world-space support point on Shape A
    Vec3 point_b{0.0f, 0.0f, 0.0f}; // world-space support point on Shape B
};

/// @brief Simplex structure for GJK maintaining 1 to 4 vertices
struct Simplex {
    std::array<SimplexVertex, 4> vertices{};
    uint32_t count{0};

    constexpr Simplex() noexcept = default;

    void clear() noexcept {
        count = 0;
    }

    void push_front(const SimplexVertex& v) noexcept {
        for (uint32_t i = std::min(count, 3u); i > 0; --i) {
            vertices[i] = vertices[i - 1];
        }
        vertices[0] = v;
        count = std::min(count + 1, 4u);
    }

    [[nodiscard]] const SimplexVertex& operator[](size_t idx) const noexcept {
        return vertices[idx];
    }

    [[nodiscard]] SimplexVertex& operator[](size_t idx) noexcept {
        return vertices[idx];
    }
};

/// @brief Result of running the GJK algorithm
struct GJKResult {
    bool intersecting{false};
    float distance{0.0f};
    Vec3 closest_point_a{0.0f, 0.0f, 0.0f};
    Vec3 closest_point_b{0.0f, 0.0f, 0.0f};
    Vec3 separation_axis{0.0f, 1.0f, 0.0f}; // Direction pointing from B towards A
    uint32_t iterations{0};
    Simplex simplex{}; // Final simplex (tetrahedron enclosing origin if intersecting)
};

/// @brief Gilbert-Johnson-Keerthi (GJK) distance and convex intersection algorithm.
/// Computes Euclidean distance, closest witness points, or encloses the origin in a 3D tetrahedron.
class GJK {
public:
    /// @brief Evaluates support mapping in Minkowski difference A - B: S_{A-B}(d) = S_A(d) - S_B(-d)
    [[nodiscard]] static SimplexVertex support(
        const Shape& shape_a, const Transform3D& transform_a,
        const Shape& shape_b, const Transform3D& transform_b,
        const Vec3& direction
    ) noexcept;

    /// @brief Runs GJK intersection and distance query between two convex shapes
    [[nodiscard]] static GJKResult query(
        const Shape& shape_a, const Transform3D& transform_a,
        const Shape& shape_b, const Transform3D& transform_b,
        uint32_t max_iterations = 32,
        float tolerance = 1e-3f
    ) noexcept;

    /// @brief Fast boolean intersection test (early exits upon detecting origin enclosure)
    [[nodiscard]] static bool test_intersection(
        const Shape& shape_a, const Transform3D& transform_a,
        const Shape& shape_b, const Transform3D& transform_b
    ) noexcept;

private:
    /// @brief Updates simplex and chooses next search direction towards origin
    /// Returns true if origin is contained inside the tetrahedron
    static bool update_simplex(Simplex& simplex, Vec3& direction) noexcept;

    /// @brief Segment subalgorithm for simplex of 2 vertices
    static bool do_simplex2(Simplex& simplex, Vec3& direction) noexcept;

    /// @brief Triangle subalgorithm for simplex of 3 vertices
    static bool do_simplex3(Simplex& simplex, Vec3& direction) noexcept;

    /// @brief Tetrahedron subalgorithm for simplex of 4 vertices
    static bool do_simplex4(Simplex& simplex, Vec3& direction) noexcept;
};

} // namespace orion::physics

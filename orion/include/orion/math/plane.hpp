#pragma once

#include "orion/math/vec3.hpp"

namespace orion {

class alignas(16) Plane {
public:
    Vec3 normal;    // Unit normal vector pointing outward
    float distance; // Distance from origin along normal: normal . point = distance

    constexpr Plane() noexcept : normal(0.0f, 1.0f, 0.0f), distance(0.0f) {}
    constexpr Plane(const Vec3& in_normal, float in_distance) noexcept
        : normal(in_normal), distance(in_distance) {}

    [[nodiscard]] static Plane from_point_and_normal(const Vec3& point, const Vec3& in_normal) noexcept {
        const Vec3 n = in_normal.normalized();
        return Plane(n, n.dot(point));
    }

    [[nodiscard]] static Plane from_three_points(const Vec3& a, const Vec3& b, const Vec3& c) noexcept {
        const Vec3 n = (b - a).cross(c - a).normalized();
        return Plane(n, n.dot(a));
    }

    // Signed distance from point to plane
    // > 0 if point is in front of plane (half-space of normal)
    // < 0 if point is behind plane
    // = 0 if point is on the plane
    [[nodiscard]] float signed_distance(const Vec3& point) const noexcept {
        return normal.dot(point) - distance;
    }

    // Project point onto the plane surface
    [[nodiscard]] Vec3 project(const Vec3& point) const noexcept {
        return point - normal * signed_distance(point);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Plane& p) {
    os << "Plane(Normal: " << p.normal << ", Dist: " << p.distance << ")";
    return os;
}

} // namespace orion

namespace orion::physics { using orion::Plane; }

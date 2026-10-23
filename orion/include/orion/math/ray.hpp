#pragma once

#include "orion/math/vec3.hpp"

namespace orion {

class alignas(16) Ray {
public:
    Vec3 origin;
    Vec3 direction;     // Normalized direction vector
    Vec3 inv_direction; // Reciprocal (1 / direction), precomputed for fast slab intersection

    constexpr Ray() noexcept
        : origin(Vec3::zero()), direction(Vec3::forward()), inv_direction(0.0f, 0.0f, -1.0f) {}

    Ray(const Vec3& in_origin, const Vec3& in_direction) noexcept
        : origin(in_origin), direction(in_direction.normalized()) {
        inv_direction = Vec3(
            std::abs(direction.x) > EPSILON ? (1.0f / direction.x) : LARGE_FLOAT,
            std::abs(direction.y) > EPSILON ? (1.0f / direction.y) : LARGE_FLOAT,
            std::abs(direction.z) > EPSILON ? (1.0f / direction.z) : LARGE_FLOAT
        );
    }

    [[nodiscard]] Vec3 point_at(float t) const noexcept {
        return origin + direction * t;
    }
};

inline std::ostream& operator<<(std::ostream& os, const Ray& r) {
    os << "Ray(Origin: " << r.origin << ", Dir: " << r.direction << ")";
    return os;
}

} // namespace orion

namespace orion::physics { using orion::Ray; }

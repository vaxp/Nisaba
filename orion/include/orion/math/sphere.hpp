#pragma once

#include "orion/math/vec3.hpp"
#include "orion/math/aabb.hpp"

namespace orion {

class alignas(16) Sphere {
public:
    Vec3 center;
    float radius;

    constexpr Sphere() noexcept : center(Vec3::zero()), radius(0.0f) {}
    constexpr Sphere(const Vec3& in_center, float in_radius) noexcept
        : center(in_center), radius(in_radius) {}

    [[nodiscard]] float signed_distance(const Vec3& point) const noexcept {
        return (point - center).length() - radius;
    }

    [[nodiscard]] bool contains(const Vec3& point) const noexcept {
        return (point - center).length_sq() <= radius * radius;
    }

    [[nodiscard]] bool overlaps(const Sphere& other) const noexcept {
        const float r_sum = radius + other.radius;
        return (center - other.center).length_sq() <= r_sum * r_sum;
    }

    [[nodiscard]] AABB get_aabb() const noexcept {
        const Vec3 r_ext(radius, radius, radius);
        return AABB(center - r_ext, center + r_ext);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Sphere& s) {
    os << "Sphere(Center: " << s.center << ", Radius: " << s.radius << ")";
    return os;
}

} // namespace orion

namespace orion::physics { using orion::Sphere; }

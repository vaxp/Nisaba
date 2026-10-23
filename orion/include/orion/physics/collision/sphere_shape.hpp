#pragma once

#include "orion/physics/collision/shape.hpp"
#include <cmath>

namespace orion::physics {

/// @brief Analytical sphere collision shape defined by a radius.
class SphereShape final : public Shape {
public:
    float radius;

    explicit SphereShape(float in_radius) noexcept
        : radius(std::max(in_radius, EPSILON)) {}

    [[nodiscard]] ShapeType get_type() const noexcept override {
        return ShapeType::Sphere;
    }

    [[nodiscard]] bool cast_ray(
        const Ray& local_ray,
        float max_distance,
        RayCastHit& out_hit
    ) const noexcept override {
        // Sphere centered at local origin: |P|^2 = r^2, P = O + t*D
        const Vec3 oc = local_ray.origin;
        const float b = oc.dot(local_ray.direction);
        const float c = oc.dot(oc) - radius * radius;

        if (c > 0.0f && b > 0.0f) {
            return false; // Ray origin outside sphere and pointing away
        }

        const float discriminant = b * b - c;
        if (discriminant < 0.0f) {
            return false; // Ray misses sphere
        }

        float t = -b - std::sqrt(discriminant);
        if (t < 0.0f) {
            // Ray origin is inside sphere, find exit point
            t = -b + std::sqrt(discriminant);
        }

        if (t >= 0.0f && t <= max_distance) {
            out_hit.fraction = t;
            out_hit.point = local_ray.point_at(t);
            out_hit.normal = (out_hit.point / radius).normalized();
            out_hit.has_hit = true;
            return true;
        }

        return false;
    }

    [[nodiscard]] AABB get_local_bounds() const noexcept override {
        const Vec3 r(radius, radius, radius);
        return AABB(-r, r);
    }

    [[nodiscard]] AABB get_world_bounds(const Transform3D& transform) const noexcept override {
        // Spheres are invariant under rotation; bounding box is simply translated
        const Vec3 r(radius, radius, radius);
        return AABB(transform.position - r, transform.position + r);
    }

    [[nodiscard]] MassProperties get_mass_properties(float density = 1000.0f) const noexcept override {
        MassProperties mp;
        const float volume = (4.0f / 3.0f) * PI * (radius * radius * radius);
        mp.mass = density * volume;
        const float inertia = 0.4f * mp.mass * (radius * radius); // 2/5 * m * r^2
        mp.inertia_diagonal = Vec3(inertia, inertia, inertia);
        mp.inertia_rotation = Quat::identity();
        mp.center_of_mass = Vec3::zero();
        return mp;
    }

    [[nodiscard]] Vec3 get_support(const Vec3& direction) const noexcept override {
        const float len_sq = direction.length_sq();
        if (len_sq > EPSILON * EPSILON) {
            return direction * (radius / std::sqrt(len_sq));
        }
        return Vec3(0.0f, radius, 0.0f);
    }

    [[nodiscard]] float get_inner_radius() const noexcept override {
        return radius;
    }
};

} // namespace orion::physics

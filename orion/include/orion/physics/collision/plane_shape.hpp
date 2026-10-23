#pragma once

#include "orion/physics/collision/shape.hpp"
#include "orion/math/plane.hpp"

namespace orion::physics {

/// @brief Static/kinematic planar shape representing floors, walls, and infinite boundaries.
/// By convention, the local plane surface lies on the XZ plane with normal pointing along +Y.
class PlaneShape final : public Shape {
public:
    Plane plane;
    float half_extent;

    explicit PlaneShape(const Plane& in_plane = Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f), float in_half_extent = 10000.0f) noexcept
        : plane(in_plane), half_extent(in_half_extent) {}

    [[nodiscard]] ShapeType get_type() const noexcept override {
        return ShapeType::Plane;
    }

    [[nodiscard]] bool cast_ray(
        const Ray& local_ray,
        float max_distance,
        RayCastHit& out_hit
    ) const noexcept override {
        const float denom = plane.normal.dot(local_ray.direction);
        if (std::abs(denom) < EPSILON) {
            return false; // Ray parallel to plane
        }

        const float t = (plane.distance - plane.normal.dot(local_ray.origin)) / denom;
        if (t < 0.0f || t > max_distance) {
            return false;
        }

        const Vec3 hit_pt = local_ray.point_at(t);
        if (std::abs(hit_pt.x) > half_extent || std::abs(hit_pt.z) > half_extent) {
            return false; // Outside finite plane boundaries
        }

        out_hit.fraction = t;
        out_hit.point = hit_pt;
        out_hit.normal = (denom < 0.0f) ? plane.normal : -plane.normal;
        out_hit.has_hit = true;
        return true;
    }

    [[nodiscard]] AABB get_local_bounds() const noexcept override {
        const float thickness = 10.0f; // Solid slab backing
        return AABB(
            Vec3(-half_extent, -thickness, -half_extent),
            Vec3(half_extent, 0.0f, half_extent)
        );
    }

    [[nodiscard]] MassProperties get_mass_properties([[maybe_unused]] float density = 1000.0f) const noexcept override {
        // Planes are fundamentally static/immovable (infinite mass)
        MassProperties mp;
        mp.mass = 0.0f;
        mp.inertia_diagonal = Vec3::zero();
        mp.inertia_rotation = Quat::identity();
        mp.center_of_mass = Vec3::zero();
        return mp;
    }

    [[nodiscard]] Vec3 get_support(const Vec3& direction) const noexcept override {
        return Vec3(
            direction.x >= 0.0f ? half_extent : -half_extent,
            0.0f,
            direction.z >= 0.0f ? half_extent : -half_extent
        );
    }

    [[nodiscard]] float get_inner_radius() const noexcept override {
        return 0.0f;
    }
};

} // namespace orion::physics

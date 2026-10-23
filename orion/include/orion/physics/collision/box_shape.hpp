#pragma once

#include "orion/physics/collision/shape.hpp"
#include <algorithm>

namespace orion::physics {

/// @brief Analytical box collision shape defined by half-extents along each local axis.
class BoxShape final : public Shape {
public:
    Vec3 half_extents;

    explicit BoxShape(const Vec3& in_half_extents) noexcept
        : half_extents(
            std::max(in_half_extents.x, EPSILON),
            std::max(in_half_extents.y, EPSILON),
            std::max(in_half_extents.z, EPSILON)
          ) {}

    [[nodiscard]] ShapeType get_type() const noexcept override {
        return ShapeType::Box;
    }

    [[nodiscard]] bool cast_ray(
        const Ray& local_ray,
        float max_distance,
        RayCastHit& out_hit
    ) const noexcept override {
        float t_min = 0.0f;
        float t_max = max_distance;
        Vec3 hit_normal = Vec3::up();

        for (int i = 0; i < 3; ++i) {
            const float inv_d = local_ray.inv_direction[i];
            float t0 = (-half_extents[i] - local_ray.origin[i]) * inv_d;
            float t1 = ( half_extents[i] - local_ray.origin[i]) * inv_d;
            Vec3 normal_axis = Vec3::zero();
            normal_axis[i] = -1.0f;

            if (inv_d < 0.0f) {
                std::swap(t0, t1);
                normal_axis[i] = 1.0f;
            }

            if (t0 > t_min) {
                t_min = t0;
                hit_normal = normal_axis;
            }
            t_max = std::min(t_max, t1);

            if (t_min > t_max) {
                return false;
            }
        }

        if (t_min >= 0.0f && t_min <= max_distance) {
            out_hit.fraction = t_min;
            out_hit.point = local_ray.point_at(t_min);
            out_hit.normal = hit_normal;
            out_hit.has_hit = true;
            return true;
        }

        return false;
    }

    [[nodiscard]] AABB get_local_bounds() const noexcept override {
        return AABB(-half_extents, half_extents);
    }

    [[nodiscard]] MassProperties get_mass_properties(float density = 1000.0f) const noexcept override {
        MassProperties mp;
        const Vec3 extents = half_extents * 2.0f;
        const float volume = extents.x * extents.y * extents.z;
        mp.mass = density * volume;

        // Solid box inertia tensor: I_xx = (1/12) * m * (w^2 + d^2) = (1/3) * m * (hy^2 + hz^2)
        const float c = (1.0f / 3.0f) * mp.mass;
        const float hx2 = half_extents.x * half_extents.x;
        const float hy2 = half_extents.y * half_extents.y;
        const float hz2 = half_extents.z * half_extents.z;

        mp.inertia_diagonal = Vec3(
            c * (hy2 + hz2),
            c * (hx2 + hz2),
            c * (hx2 + hy2)
        );
        mp.inertia_rotation = Quat::identity();
        mp.center_of_mass = Vec3::zero();
        return mp;
    }

    [[nodiscard]] Vec3 get_support(const Vec3& direction) const noexcept override {
        return Vec3(
            direction.x >= 0.0f ? half_extents.x : -half_extents.x,
            direction.y >= 0.0f ? half_extents.y : -half_extents.y,
            direction.z >= 0.0f ? half_extents.z : -half_extents.z
        );
    }

    [[nodiscard]] float get_inner_radius() const noexcept override {
        return std::min({half_extents.x, half_extents.y, half_extents.z});
    }
};

} // namespace orion::physics

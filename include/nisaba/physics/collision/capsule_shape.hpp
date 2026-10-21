#pragma once

#include "nisaba/physics/collision/shape.hpp"
#include <cmath>

namespace nisaba::physics {

/// @brief Analytical capsule collision shape defined by a cylinder along the Y axis
/// with two hemispherical end-caps.
class CapsuleShape final : public Shape {
public:
    float half_height;
    float radius;

    CapsuleShape(float in_half_height, float in_radius) noexcept
        : half_height(std::max(in_half_height, EPSILON)),
          radius(std::max(in_radius, EPSILON)) {}

    [[nodiscard]] ShapeType get_type() const noexcept override {
        return ShapeType::Capsule;
    }

    [[nodiscard]] bool cast_ray(
        const Ray& local_ray,
        float max_distance,
        RayCastHit& out_hit
    ) const noexcept override {
        float closest_t = max_distance;
        bool hit_found = false;
        Vec3 best_point = Vec3::zero();
        Vec3 best_normal = Vec3::up();

        // 1. Ray vs cylinder section around Y: x^2 + z^2 = radius^2
        const float a = local_ray.direction.x * local_ray.direction.x + local_ray.direction.z * local_ray.direction.z;
        const float b = 2.0f * (local_ray.origin.x * local_ray.direction.x + local_ray.origin.z * local_ray.direction.z);
        const float c = local_ray.origin.x * local_ray.origin.x + local_ray.origin.z * local_ray.origin.z - radius * radius;

        if (a > EPSILON) {
            const float discriminant = b * b - 4.0f * a * c;
            if (discriminant >= 0.0f) {
                const float sqrt_d = std::sqrt(discriminant);
                float t0 = (-b - sqrt_d) / (2.0f * a);
                if (t0 < 0.0f) {
                    t0 = (-b + sqrt_d) / (2.0f * a);
                }
                if (t0 >= 0.0f && t0 < closest_t) {
                    const float y = local_ray.origin.y + t0 * local_ray.direction.y;
                    if (y >= -half_height && y <= half_height) {
                        closest_t = t0;
                        hit_found = true;
                        best_point = local_ray.point_at(t0);
                        best_normal = Vec3(best_point.x, 0.0f, best_point.z).normalized();
                    }
                }
            }
        }

        // Helper lambda for sphere cap intersection
        auto test_cap = [&](const Vec3& cap_center) {
            const Vec3 oc = local_ray.origin - cap_center;
            const float sb = oc.dot(local_ray.direction);
            const float sc = oc.dot(oc) - radius * radius;
            const float sdisc = sb * sb - sc;
            if (sdisc >= 0.0f) {
                const float ssqrt = std::sqrt(sdisc);
                float st = -sb - ssqrt;
                if (st < 0.0f) {
                    st = -sb + ssqrt;
                }
                if (st >= 0.0f && st < closest_t) {
                    const Vec3 p = local_ray.point_at(st);
                    if ((cap_center.y > 0.0f && p.y >= half_height) ||
                        (cap_center.y < 0.0f && p.y <= -half_height)) {
                        closest_t = st;
                        hit_found = true;
                        best_point = p;
                        best_normal = (p - cap_center).normalized();
                    }
                }
            }
        };

        // 2. Top cap sphere at (0, half_height, 0)
        test_cap(Vec3(0.0f, half_height, 0.0f));

        // 3. Bottom cap sphere at (0, -half_height, 0)
        test_cap(Vec3(0.0f, -half_height, 0.0f));

        if (hit_found) {
            out_hit.fraction = closest_t;
            out_hit.point = best_point;
            out_hit.normal = best_normal;
            out_hit.has_hit = true;
            return true;
        }

        return false;
    }

    [[nodiscard]] AABB get_local_bounds() const noexcept override {
        const float total_half_height = half_height + radius;
        return AABB(
            Vec3(-radius, -total_half_height, -radius),
            Vec3(radius, total_half_height, radius)
        );
    }

    [[nodiscard]] MassProperties get_mass_properties(float density = 1000.0f) const noexcept override {
        MassProperties mp;

        // Cylinder volume and mass
        const float cyl_volume = PI * (radius * radius) * (2.0f * half_height);
        const float cyl_mass = density * cyl_volume;

        // Two hemispheres volume and mass
        const float sph_volume = (4.0f / 3.0f) * PI * (radius * radius * radius);
        const float sph_mass = density * sph_volume;

        mp.mass = cyl_mass + sph_mass;

        // Cylinder moments of inertia
        const float r2 = radius * radius;
        const float h2 = half_height * half_height;

        const float i_yy_cyl = 0.5f * cyl_mass * r2;
        const float i_xz_cyl = cyl_mass * ((1.0f / 3.0f) * h2 + 0.25f * r2);

        // Hemispherical caps moments of inertia (via parallel axis theorem)
        const float i_yy_sph = 0.4f * sph_mass * r2;
        const float i_xz_sph = sph_mass * (0.4f * r2 + 0.75f * half_height * radius + h2);

        const float i_xx = i_xz_cyl + i_xz_sph;
        const float i_yy = i_yy_cyl + i_yy_sph;
        const float i_zz = i_xx;

        mp.inertia_diagonal = Vec3(i_xx, i_yy, i_zz);
        mp.inertia_rotation = Quat::identity();
        mp.center_of_mass = Vec3::zero();
        return mp;
    }

    [[nodiscard]] Vec3 get_support(const Vec3& direction) const noexcept override {
        // Capsule is the Minkowski sum of a vertical line segment and a sphere
        const Vec3 seg_support(0.0f, direction.y >= 0.0f ? half_height : -half_height, 0.0f);

        const float len_sq = direction.length_sq();
        if (len_sq > EPSILON * EPSILON) {
            return seg_support + direction * (radius / std::sqrt(len_sq));
        }
        return seg_support + Vec3(0.0f, radius, 0.0f);
    }

    [[nodiscard]] float get_inner_radius() const noexcept override {
        return radius;
    }
};

} // namespace nisaba::physics

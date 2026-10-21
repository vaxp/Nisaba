#pragma once

#include "nisaba/physics/collision/shape.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::physics {

/// @brief Analytical cylinder collision shape oriented along the local Y axis.
class CylinderShape final : public Shape {
public:
    float half_height;
    float radius;

    CylinderShape(float in_half_height, float in_radius) noexcept
        : half_height(std::max(in_half_height, EPSILON)),
          radius(std::max(in_radius, EPSILON)) {}

    [[nodiscard]] ShapeType get_type() const noexcept override {
        return ShapeType::Cylinder;
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

        // 1. Ray vs infinite cylinder around Y: x^2 + z^2 = radius^2
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

        // 2. Top planar cap at y = +half_height
        if (std::abs(local_ray.direction.y) > EPSILON) {
            const float t_top = (half_height - local_ray.origin.y) / local_ray.direction.y;
            if (t_top >= 0.0f && t_top < closest_t) {
                const Vec3 p = local_ray.point_at(t_top);
                if (p.x * p.x + p.z * p.z <= radius * radius) {
                    closest_t = t_top;
                    hit_found = true;
                    best_point = p;
                    best_normal = Vec3(0.0f, 1.0f, 0.0f);
                }
            }

            // 3. Bottom planar cap at y = -half_height
            const float t_bot = (-half_height - local_ray.origin.y) / local_ray.direction.y;
            if (t_bot >= 0.0f && t_bot < closest_t) {
                const Vec3 p = local_ray.point_at(t_bot);
                if (p.x * p.x + p.z * p.z <= radius * radius) {
                    closest_t = t_bot;
                    hit_found = true;
                    best_point = p;
                    best_normal = Vec3(0.0f, -1.0f, 0.0f);
                }
            }
        }

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
        return AABB(
            Vec3(-radius, -half_height, -radius),
            Vec3(radius, half_height, radius)
        );
    }

    [[nodiscard]] MassProperties get_mass_properties(float density = 1000.0f) const noexcept override {
        MassProperties mp;
        const float volume = PI * (radius * radius) * (2.0f * half_height);
        mp.mass = density * volume;

        // Solid cylinder moments of inertia:
        // I_yy = 0.5 * m * r^2
        // I_xx = I_zz = (1/12) * m * (3 * r^2 + 4 * h^2)
        const float r2 = radius * radius;
        const float h2 = half_height * half_height;
        const float i_yy = 0.5f * mp.mass * r2;
        const float i_xx = (1.0f / 12.0f) * mp.mass * (3.0f * r2 + 4.0f * h2);

        mp.inertia_diagonal = Vec3(i_xx, i_yy, i_xx);
        mp.inertia_rotation = Quat::identity();
        mp.center_of_mass = Vec3::zero();
        return mp;
    }

    [[nodiscard]] Vec3 get_support(const Vec3& direction) const noexcept override {
        const float y = (direction.y >= 0.0f) ? half_height : -half_height;
        const float xz_len_sq = direction.x * direction.x + direction.z * direction.z;

        if (xz_len_sq > EPSILON * EPSILON) {
            const float inv_len = radius / std::sqrt(xz_len_sq);
            return Vec3(direction.x * inv_len, y, direction.z * inv_len);
        }
        return Vec3(0.0f, y, 0.0f);
    }

    [[nodiscard]] float get_inner_radius() const noexcept override {
        return std::min(radius, half_height);
    }
};

} // namespace nisaba::physics

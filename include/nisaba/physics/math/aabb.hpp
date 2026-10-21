#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/mat44.hpp"
#include "nisaba/physics/math/transform3d.hpp"

namespace nisaba::physics {

class alignas(16) AABB {
public:
    Vec3 min_point;
    Vec3 max_point;

    // Constructors
    constexpr AABB() noexcept
        : min_point(LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT),
          max_point(-LARGE_FLOAT, -LARGE_FLOAT, -LARGE_FLOAT) {}

    constexpr AABB(const Vec3& in_min, const Vec3& in_max) noexcept
        : min_point(in_min), max_point(in_max) {}

    [[nodiscard]] static AABB from_center_half_extents(const Vec3& center, const Vec3& half_extents) noexcept {
        return AABB(center - half_extents, center + half_extents);
    }

    [[nodiscard]] bool is_valid() const noexcept {
        return min_point.x <= max_point.x &&
               min_point.y <= max_point.y &&
               min_point.z <= max_point.z;
    }

    [[nodiscard]] Vec3 center() const noexcept {
        return (min_point + max_point) * 0.5f;
    }

    [[nodiscard]] Vec3 extent() const noexcept {
        return max_point - min_point;
    }

    [[nodiscard]] Vec3 half_extent() const noexcept {
        return extent() * 0.5f;
    }

    [[nodiscard]] float volume() const noexcept {
        const Vec3 ext = extent();
        return ext.x * ext.y * ext.z;
    }

    [[nodiscard]] float surface_area() const noexcept {
        const Vec3 ext = extent();
        return 2.0f * (ext.x * ext.y + ext.y * ext.z + ext.z * ext.x);
    }

    void reset() noexcept {
        min_point = Vec3(LARGE_FLOAT, LARGE_FLOAT, LARGE_FLOAT);
        max_point = Vec3(-LARGE_FLOAT, -LARGE_FLOAT, -LARGE_FLOAT);
    }

    void encapsulate(const Vec3& point) noexcept {
        min_point = min_point.min(point);
        max_point = max_point.max(point);
    }

    void encapsulate(const AABB& other) noexcept {
        min_point = min_point.min(other.min_point);
        max_point = max_point.max(other.max_point);
    }

    void expand(float amount) noexcept {
        const Vec3 expansion(amount, amount, amount);
        min_point -= expansion;
        max_point += expansion;
    }

    [[nodiscard]] bool contains(const Vec3& point) const noexcept {
        return point.x >= min_point.x && point.x <= max_point.x &&
               point.y >= min_point.y && point.y <= max_point.y &&
               point.z >= min_point.z && point.z <= max_point.z;
    }

    [[nodiscard]] bool contains(const AABB& other) const noexcept {
        return other.min_point.x >= min_point.x && other.max_point.x <= max_point.x &&
               other.min_point.y >= min_point.y && other.max_point.y <= max_point.y &&
               other.min_point.z >= min_point.z && other.max_point.z <= max_point.z;
    }

    [[nodiscard]] bool overlaps(const AABB& other) const noexcept {
        if (max_point.x < other.min_point.x || min_point.x > other.max_point.x) return false;
        if (max_point.y < other.min_point.y || min_point.y > other.max_point.y) return false;
        if (max_point.z < other.min_point.z || min_point.z > other.max_point.z) return false;
        return true;
    }

    [[nodiscard]] AABB intersection(const AABB& other) const noexcept {
        return AABB(
            min_point.max(other.min_point),
            max_point.min(other.max_point)
        );
    }

    // Jim Arvo's algorithm: Fast transform of AABB by 4x4 matrix
    // Computes the new AABB directly from Center and Half-extents
    [[nodiscard]] AABB transform(const Mat44& m) const noexcept {
        const Vec3 c = center();
        const Vec3 h = half_extent();

        // New center
        const Vec3 new_center = m.transform_point(c);

        // New half extent: sum(|M_ij| * h_j)
        const float hx = std::abs(m(0, 0)) * h.x + std::abs(m(0, 1)) * h.y + std::abs(m(0, 2)) * h.z;
        const float hy = std::abs(m(1, 0)) * h.x + std::abs(m(1, 1)) * h.y + std::abs(m(1, 2)) * h.z;
        const float hz = std::abs(m(2, 0)) * h.x + std::abs(m(2, 1)) * h.y + std::abs(m(2, 2)) * h.z;

        const Vec3 new_half_extents(hx, hy, hz);
        return AABB(new_center - new_half_extents, new_center + new_half_extents);
    }

    /// @brief Fast direct transform of AABB by rigid Transform3D without creating a 4x4 matrix
    [[nodiscard]] AABB transform(const Transform3D& t) const noexcept {
        const Vec3 c = center();
        const Vec3 h = half_extent();

        const Vec3 new_center = t.transform_point(c);

        const float xx = t.rotation.x * t.rotation.x;
        const float yy = t.rotation.y * t.rotation.y;
        const float zz = t.rotation.z * t.rotation.z;
        const float xy = t.rotation.x * t.rotation.y;
        const float xz = t.rotation.x * t.rotation.z;
        const float yz = t.rotation.y * t.rotation.z;
        const float wx = t.rotation.w * t.rotation.x;
        const float wy = t.rotation.w * t.rotation.y;
        const float wz = t.rotation.w * t.rotation.z;

        const float r00 = 1.0f - 2.0f * (yy + zz);
        const float r01 = 2.0f * (xy - wz);
        const float r02 = 2.0f * (xz + wy);

        const float r10 = 2.0f * (xy + wz);
        const float r11 = 1.0f - 2.0f * (xx + zz);
        const float r12 = 2.0f * (yz - wx);

        const float r20 = 2.0f * (xz - wy);
        const float r21 = 2.0f * (yz + wx);
        const float r22 = 1.0f - 2.0f * (xx + yy);

        const float hx = std::abs(r00) * h.x + std::abs(r01) * h.y + std::abs(r02) * h.z;
        const float hy = std::abs(r10) * h.x + std::abs(r11) * h.y + std::abs(r12) * h.z;
        const float hz = std::abs(r20) * h.x + std::abs(r21) * h.y + std::abs(r22) * h.z;

        const Vec3 new_half_extents(hx, hy, hz);
        return AABB(new_center - new_half_extents, new_center + new_half_extents);
    }

    // Slab method (Kay-Kajiya) for ray intersection
    [[nodiscard]] bool raycast(
        const Vec3& ray_origin,
        const Vec3& ray_dir,
        float max_dist,
        float& out_t_min,
        float& out_t_max
    ) const noexcept {
        float t0 = 0.0f;
        float t1 = max_dist;

        for (int i = 0; i < 3; ++i) {
            const float inv_dir = (std::abs(ray_dir[i]) > EPSILON) ? (1.0f / ray_dir[i]) : LARGE_FLOAT;
            float t_near = (min_point[i] - ray_origin[i]) * inv_dir;
            float t_far  = (max_point[i] - ray_origin[i]) * inv_dir;

            if (t_near > t_far) {
                std::swap(t_near, t_far);
            }

            t0 = std::max(t0, t_near);
            t1 = std::min(t1, t_far);

            if (t0 > t1) {
                return false;
            }
        }

        out_t_min = t0;
        out_t_max = t1;
        return true;
    }
};

inline std::ostream& operator<<(std::ostream& os, const AABB& b) {
    os << "AABB(Min: " << b.min_point << ", Max: " << b.max_point << ")";
    return os;
}

} // namespace nisaba::physics

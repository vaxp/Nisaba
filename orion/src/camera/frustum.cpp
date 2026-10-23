#include "orion/camera/frustum.hpp"
#include <cmath>

namespace orion {

namespace {

[[nodiscard]] inline Plane normalize_plane(float a, float b, float c, float d) noexcept {
    const float len_sq = a * a + b * b + c * c;
    if (len_sq > 1e-12f) {
        const float inv_len = 1.0f / std::sqrt(len_sq);
        return Plane(Vec3(a * inv_len, b * inv_len, c * inv_len), -d * inv_len);
    }
    return Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f);
}

} // namespace

Frustum Frustum::from_view_projection(const Mat44& vp, bool vulkan_depth) noexcept {
    Frustum f;

    // Row-major matrix indexing: m[row * 4 + col]
    // Left plane: row 3 + row 0
    f.planes[static_cast<size_t>(FrustumPlaneIndex::Left)] = normalize_plane(
        vp.m[12] + vp.m[0],
        vp.m[13] + vp.m[1],
        vp.m[14] + vp.m[2],
        vp.m[15] + vp.m[3]
    );

    // Right plane: row 3 - row 0
    f.planes[static_cast<size_t>(FrustumPlaneIndex::Right)] = normalize_plane(
        vp.m[12] - vp.m[0],
        vp.m[13] - vp.m[1],
        vp.m[14] - vp.m[2],
        vp.m[15] - vp.m[3]
    );

    // Bottom plane: row 3 + row 1
    f.planes[static_cast<size_t>(FrustumPlaneIndex::Bottom)] = normalize_plane(
        vp.m[12] + vp.m[4],
        vp.m[13] + vp.m[5],
        vp.m[14] + vp.m[6],
        vp.m[15] + vp.m[7]
    );

    // Top plane: row 3 - row 1
    f.planes[static_cast<size_t>(FrustumPlaneIndex::Top)] = normalize_plane(
        vp.m[12] - vp.m[4],
        vp.m[13] - vp.m[5],
        vp.m[14] - vp.m[6],
        vp.m[15] - vp.m[7]
    );

    // Near plane:
    // Vulkan (Z in [0, 1]): row 2
    // OpenGL (Z in [-1, 1]): row 3 + row 2
    if (vulkan_depth) {
        f.planes[static_cast<size_t>(FrustumPlaneIndex::Near)] = normalize_plane(
            vp.m[8],
            vp.m[9],
            vp.m[10],
            vp.m[11]
        );
    } else {
        f.planes[static_cast<size_t>(FrustumPlaneIndex::Near)] = normalize_plane(
            vp.m[12] + vp.m[8],
            vp.m[13] + vp.m[9],
            vp.m[14] + vp.m[10],
            vp.m[15] + vp.m[11]
        );
    }

    // Far plane: row 3 - row 2
    f.planes[static_cast<size_t>(FrustumPlaneIndex::Far)] = normalize_plane(
        vp.m[12] - vp.m[8],
        vp.m[13] - vp.m[9],
        vp.m[14] - vp.m[10],
        vp.m[15] - vp.m[11]
    );

    return f;
}

bool Frustum::contains_point(const Vec3& point) const noexcept {
    for (const auto& plane : planes) {
        if (plane.signed_distance(point) < 0.0f) {
            return false;
        }
    }
    return true;
}

bool Frustum::intersects_sphere(const Vec3& center, float radius) const noexcept {
    for (const auto& plane : planes) {
        if (plane.signed_distance(center) < -radius) {
            return false;
        }
    }
    return true;
}

bool Frustum::intersects_aabb(const AABB& aabb) const noexcept {
    for (const auto& plane : planes) {
        // P-vertex: vertex of the AABB furthest in the direction of the inward plane normal
        const Vec3 p_vertex(
            plane.normal.x >= 0.0f ? aabb.max_point.x : aabb.min_point.x,
            plane.normal.y >= 0.0f ? aabb.max_point.y : aabb.min_point.y,
            plane.normal.z >= 0.0f ? aabb.max_point.z : aabb.min_point.z
        );

        if (plane.signed_distance(p_vertex) < 0.0f) {
            return false;
        }
    }
    return true;
}

} // namespace orion

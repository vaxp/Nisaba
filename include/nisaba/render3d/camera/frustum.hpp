#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/plane.hpp"
#include "nisaba/physics/math/aabb.hpp"
#include "nisaba/physics/math/mat44.hpp"
#include <array>
#include <cstdint>

namespace nisaba::render3d {

using Vec3 = nisaba::physics::Vec3;
using Plane = nisaba::physics::Plane;
using AABB = nisaba::physics::AABB;
using Mat44 = nisaba::physics::Mat44;

enum class FrustumPlaneIndex : uint8_t {
    Left = 0,
    Right = 1,
    Bottom = 2,
    Top = 3,
    Near = 4,
    Far = 5
};

/// @brief Represents a 3D camera viewing frustum bounded by 6 inward-facing planes.
/// Enables ultra-fast SIMD-compatible spatial culling for points, spheres, and AABBs.
class alignas(16) Frustum {
public:
    std::array<Plane, 6> planes;

    Frustum() noexcept = default;

    /// @brief Extracts 6 normalized frustum planes from a combined View-Projection matrix using Gribb-Hartmann.
    /// @param vp Combined View-Projection matrix (row-major).
    /// @param vulkan_depth True if clip-space depth Z is [0, 1] (Vulkan/DirectX), false if [-1, 1] (OpenGL).
    [[nodiscard]] static Frustum from_view_projection(const Mat44& vp, bool vulkan_depth = true) noexcept;

    /// @brief Fast containment test for a 3D point in world space.
    [[nodiscard]] bool contains_point(const Vec3& point) const noexcept;

    /// @brief Fast intersection test for a bounding sphere in world space.
    [[nodiscard]] bool intersects_sphere(const Vec3& center, float radius) const noexcept;

    /// @brief Fast P-vertex / N-vertex culling test for an Axis-Aligned Bounding Box (AABB).
    /// Rejects the AABB immediately if it lies entirely outside any of the 6 frustum planes.
    [[nodiscard]] bool intersects_aabb(const AABB& aabb) const noexcept;

    /// @brief Returns a specific frustum plane.
    [[nodiscard]] const Plane& get_plane(FrustumPlaneIndex index) const noexcept {
        return planes[static_cast<size_t>(index)];
    }

    [[nodiscard]] Plane& get_plane(FrustumPlaneIndex index) noexcept {
        return planes[static_cast<size_t>(index)];
    }
};

} // namespace nisaba::render3d

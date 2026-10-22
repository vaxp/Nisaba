#pragma once

#include "nisaba/render3d/geometry/mesh.hpp"

namespace nisaba::render3d {

/// @brief Procedural 3D geometry primitive factory.
/// Produces clean, manifold meshes with complete normals, tangents, UVs, and bounding boxes.
class MeshPrimitives {
public:
    /// @brief Generates an axis-aligned box/cube with separate per-face vertices for sharp shading.
    [[nodiscard]] static Mesh3D create_cube(const Vec3& size = Vec3(1.0f, 1.0f, 1.0f)) noexcept;

    /// @brief Generates a UV sphere with smooth outward-facing normals and spherical UV mapping.
    [[nodiscard]] static Mesh3D create_sphere(
        float radius = 0.5f,
        uint32_t segments = 32,
        uint32_t rings = 16
    ) noexcept;

    /// @brief Generates a flat subdivided planar mesh lying on the XZ ground plane.
    [[nodiscard]] static Mesh3D create_plane(
        float width = 10.0f,
        float depth = 10.0f,
        uint32_t sub_x = 1,
        uint32_t sub_z = 1
    ) noexcept;

    /// @brief Generates a cylinder with mantle and top/bottom circular end caps.
    [[nodiscard]] static Mesh3D create_cylinder(
        float radius = 0.5f,
        float height = 1.0f,
        uint32_t segments = 32
    ) noexcept;

    /// @brief Generates a 3D torus (donut) with smooth tubular normals.
    [[nodiscard]] static Mesh3D create_torus(
        float major_radius = 1.0f,
        float minor_radius = 0.25f,
        uint32_t radial_segments = 32,
        uint32_t tubular_segments = 16
    ) noexcept;
};

} // namespace nisaba::render3d

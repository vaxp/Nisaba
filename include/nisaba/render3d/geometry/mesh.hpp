#pragma once

#include "nisaba/physics/math/aabb.hpp"
#include "nisaba/render3d/geometry/vertex_layout.hpp"
#include "nisaba/render3d/geometry/vertex_buffer.hpp"
#include <vector>
#include <cstdint>

namespace nisaba::render3d {

using AABB = nisaba::physics::AABB;

/// @brief Represents a draw-call slice of a Mesh3D bound to a specific material.
struct SubMesh {
    uint32_t index_offset{0};
    uint32_t index_count{0};
    uint32_t vertex_offset{0};
    uint32_t material_index{0};
    AABB bounds;
};

/// @brief 4-bone skeletal skinning influence per vertex matching glTF JOINTS_0 and WEIGHTS_0.
struct VertexSkinData {
    uint16_t joints[4]{0, 0, 0, 0};
    float weights[4]{1.0f, 0.0f, 0.0f, 0.0f};
};

/// @brief Complete 3D polygonal geometry container supporting multi-submesh slicing,
/// automated normal/tangent generation, and CPU/GPU memory buffer management.
class Mesh3D {
public:
    std::vector<Vertex3D> vertices;
    std::vector<uint32_t> indices;
    std::vector<SubMesh> submeshes;
    std::vector<VertexSkinData> skin_data;
    AABB bounds;

    VertexBuffer3D vertex_buffer;
    IndexBuffer3D index_buffer;

    Mesh3D() = default;

    [[nodiscard]] bool has_skinning() const noexcept { return !skin_data.empty(); }

    /// @brief Computes local bounding box for the entire mesh and each submesh.
    void compute_bounds() noexcept;

    /// @brief Computes smooth vertex normals by accumulating weighted triangle face normals.
    void compute_normals() noexcept;

    /// @brief Computes Gram-Schmidt orthogonalized tangents and handedness sign for PBR normal mapping.
    void compute_tangents() noexcept;

    /// @brief Uploads local vertex/index arrays into VertexBuffer3D and IndexBuffer3D.
    void upload_to_buffers() noexcept;

    [[nodiscard]] const AABB& get_bounds() const noexcept { return bounds; }
    [[nodiscard]] size_t submesh_count() const noexcept { return submeshes.size(); }
    [[nodiscard]] const SubMesh& get_submesh(size_t i) const noexcept { return submeshes[i]; }
    void add_submesh(const SubMesh& sm) { submeshes.push_back(sm); }
};

} // namespace nisaba::render3d

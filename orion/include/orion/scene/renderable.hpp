#pragma once

#include "orion/math/mat44.hpp"
#include "orion/math/aabb.hpp"
#include "orion/geometry/mesh.hpp"
#include "orion/material/pbr_material.hpp"
#include "orion/animation/skeleton.hpp"
#include <memory>
#include <vector>

namespace orion {


/// @brief 3D Renderable instance combining polygonal geometry, material bindings,
/// spatial world transform, and AABB culling bounds.
class Renderable3D {
public:
    uint32_t id{0};
    std::shared_ptr<Mesh3D> mesh;
    std::vector<std::shared_ptr<PbrMaterial>> materials;
    std::shared_ptr<Skeleton> skeleton;
    Mat44 transform{Mat44::identity()};

    bool visible{true};
    bool cast_shadows{true};
    bool receive_shadows{true};

    AABB local_bounds;
    AABB world_bounds;

    Renderable3D() = default;
    explicit Renderable3D(std::shared_ptr<Mesh3D> in_mesh, std::shared_ptr<PbrMaterial> in_mat = nullptr);

    /// @brief Updates world transformation matrix and recomputes world-space bounding box.
    void set_transform(const Mat44& in_transform) noexcept;

    /// @brief Sets world position translation.
    void set_position(const Vec3& pos) noexcept;

    /// @brief Recomputes oriented world-space AABB from local mesh bounds and current transform.
    void update_world_bounds() noexcept;

    [[nodiscard]] const AABB& get_world_bounds() const noexcept { return world_bounds; }

    /// @brief Retrieves material assigned to submesh (or default material if index out of bounds).
    [[nodiscard]] std::shared_ptr<PbrMaterial> get_material(size_t submesh_idx = 0) const noexcept;

    /// @brief Assigns material to a specific submesh index.
    void set_material(size_t submesh_idx, std::shared_ptr<PbrMaterial> mat);
};

} // namespace orion

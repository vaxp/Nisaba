#pragma once

#include "nisaba/render3d/geometry/mesh.hpp"
#include "nisaba/render3d/animation/skeleton.hpp"
#include <memory>
#include <vector>

namespace nisaba::render3d {

/// @brief Connects a rigged Mesh3D to a Skeleton to evaluate dynamic skinning deformation.
class SkinnedMeshInstance {
public:
    std::shared_ptr<Mesh3D> bind_mesh;
    std::shared_ptr<Mesh3D> dynamic_mesh;
    std::shared_ptr<Skeleton> skeleton;
    bool needs_gpu_upload{false};

    SkinnedMeshInstance() = default;
    SkinnedMeshInstance(
        std::shared_ptr<Mesh3D> in_bind,
        std::shared_ptr<Mesh3D> in_dyn,
        std::shared_ptr<Skeleton> in_skel
    );

    /// @brief Performs linear blend skinning across all vertices and recalculates deformed normals.
    void update_skinning() noexcept;
};

} // namespace nisaba::render3d

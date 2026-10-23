#pragma once

#include "orion/geometry/mesh.hpp"
#include "orion/animation/skeleton.hpp"
#include <memory>
#include <vector>

namespace orion {

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

} // namespace orion

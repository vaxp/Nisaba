#pragma once

#include "nisaba/physics/math/mat44.hpp"
#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/quat.hpp"
#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace nisaba::render3d {

using Mat44 = nisaba::physics::Mat44;
using Vec3 = nisaba::physics::Vec3;
using Quat = nisaba::physics::Quat;

/// @brief Represents a single articulated bone/joint within a Skeleton hierarchy.
struct Joint {
    std::string name;
    int32_t node_index{-1};   // glTF scene node index
    int32_t parent_index{-1}; // Index of parent joint in Skeleton::joints (-1 for root joints)

    Mat44 inverse_bind_matrix{Mat44::identity()};

    // Bind pose local transform
    Vec3 bind_translation{0.0f, 0.0f, 0.0f};
    Quat bind_rotation{Quat::identity()};
    Vec3 bind_scale{1.0f, 1.0f, 1.0f};

    // Current animated local transform
    Vec3 local_translation{0.0f, 0.0f, 0.0f};
    Quat local_rotation{Quat::identity()};
    Vec3 local_scale{1.0f, 1.0f, 1.0f};

    // Computed model-space transform and final skinning palette matrix
    Mat44 model_transform{Mat44::identity()};
    Mat44 skin_matrix{Mat44::identity()};
};

/// @brief Rigged skeleton structure holding an articulated bone hierarchy and skin matrix palette.
class Skeleton {
public:
    std::string name;
    std::vector<Joint> joints;
    std::vector<Mat44> joint_matrices;

    Skeleton() = default;
    explicit Skeleton(std::string in_name) : name(std::move(in_name)) {}

    /// @brief Recomputes model-space transforms and final skinning palette matrices (model * inverseBind).
    void update_matrices() noexcept;

    /// @brief Resets all joints to their initial bind pose.
    void reset_to_bind_pose() noexcept;

    /// @brief Finds joint index by its name, returns -1 if not found.
    [[nodiscard]] int32_t find_joint_by_name(const std::string& joint_name) const noexcept;

    /// @brief Finds joint index by its scene node index, returns -1 if not found.
    [[nodiscard]] int32_t find_joint_by_node(int32_t node_idx) const noexcept;
};

} // namespace nisaba::render3d

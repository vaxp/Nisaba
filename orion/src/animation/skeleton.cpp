#include "orion/animation/skeleton.hpp"
#include <algorithm>

namespace orion {

void Skeleton::update_matrices() noexcept {
    if (joint_matrices.size() != joints.size()) {
        joint_matrices.resize(joints.size());
    }

    for (size_t i = 0; i < joints.size(); ++i) {
        auto& j = joints[i];
        const Mat44 local = Mat44::transform(j.local_translation, j.local_rotation, j.local_scale);

        if (j.parent_index >= 0 && static_cast<size_t>(j.parent_index) < joints.size()) {
            j.model_transform = joints[static_cast<size_t>(j.parent_index)].model_transform * local;
        } else {
            j.model_transform = local;
        }

        j.skin_matrix = j.model_transform * j.inverse_bind_matrix;
        joint_matrices[i] = j.skin_matrix;
    }
}

void Skeleton::reset_to_bind_pose() noexcept {
    for (auto& j : joints) {
        j.local_translation = j.bind_translation;
        j.local_rotation = j.bind_rotation;
        j.local_scale = j.bind_scale;
    }
    update_matrices();
}

int32_t Skeleton::find_joint_by_name(const std::string& joint_name) const noexcept {
    for (size_t i = 0; i < joints.size(); ++i) {
        if (joints[i].name == joint_name) {
            return static_cast<int32_t>(i);
        }
    }
    return -1;
}

int32_t Skeleton::find_joint_by_node(int32_t node_idx) const noexcept {
    if (node_idx < 0) return -1;
    for (size_t i = 0; i < joints.size(); ++i) {
        if (joints[i].node_index == node_idx) {
            return static_cast<int32_t>(i);
        }
    }
    return -1;
}

} // namespace orion

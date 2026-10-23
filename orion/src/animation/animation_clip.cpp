#include "orion/animation/animation_clip.hpp"
#include <algorithm>
#include <cmath>

namespace orion {

Vec3 AnimationChannel::sample_vec3(float time) const noexcept {
    if (vec3_values.empty()) return Vec3::zero();
    if (timestamps.empty() || time <= timestamps.front()) return vec3_values.front();
    if (time >= timestamps.back()) return vec3_values.back();

    auto it = std::upper_bound(timestamps.begin(), timestamps.end(), time);
    if (it == timestamps.begin()) return vec3_values.front();
    if (it == timestamps.end()) return vec3_values.back();

    const size_t idx1 = static_cast<size_t>(std::distance(timestamps.begin(), it));
    const size_t idx0 = idx1 - 1;

    if (interpolation == InterpolationMode::Step) {
        return vec3_values[idx0];
    }

    const float t0 = timestamps[idx0];
    const float t1 = timestamps[idx1];
    const float factor = (t1 > t0) ? ((time - t0) / (t1 - t0)) : 0.0f;

    const Vec3& v0 = vec3_values[idx0];
    const Vec3& v1 = vec3_values[idx1];
    return v0 + (v1 - v0) * factor;
}

Quat AnimationChannel::sample_quat(float time) const noexcept {
    if (quat_values.empty()) return Quat::identity();
    if (timestamps.empty() || time <= timestamps.front()) return quat_values.front();
    if (time >= timestamps.back()) return quat_values.back();

    auto it = std::upper_bound(timestamps.begin(), timestamps.end(), time);
    if (it == timestamps.begin()) return quat_values.front();
    if (it == timestamps.end()) return quat_values.back();

    const size_t idx1 = static_cast<size_t>(std::distance(timestamps.begin(), it));
    const size_t idx0 = idx1 - 1;

    if (interpolation == InterpolationMode::Step) {
        return quat_values[idx0];
    }

    const float t0 = timestamps[idx0];
    const float t1 = timestamps[idx1];
    const float factor = (t1 > t0) ? ((time - t0) / (t1 - t0)) : 0.0f;

    return Quat::slerp(quat_values[idx0], quat_values[idx1], factor);
}

void AnimationClip::apply(Skeleton& skeleton, float time) const noexcept {
    for (const auto& ch : channels) {
        int32_t j_idx = ch.target_joint;
        if (j_idx < 0) {
            j_idx = skeleton.find_joint_by_node(ch.target_node);
        }
        if (j_idx < 0 || static_cast<size_t>(j_idx) >= skeleton.joints.size()) {
            continue;
        }

        auto& joint = skeleton.joints[static_cast<size_t>(j_idx)];
        if (ch.target_path == TargetPath::Translation) {
            joint.local_translation = ch.sample_vec3(time);
        } else if (ch.target_path == TargetPath::Rotation) {
            joint.local_rotation = ch.sample_quat(time);
        } else if (ch.target_path == TargetPath::Scale) {
            joint.local_scale = ch.sample_vec3(time);
        }
    }
}

} // namespace orion

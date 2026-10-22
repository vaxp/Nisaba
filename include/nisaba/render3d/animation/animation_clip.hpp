#pragma once

#include "nisaba/render3d/animation/skeleton.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace nisaba::render3d {

enum class InterpolationMode : uint8_t {
    Step = 0,
    Linear = 1,
    CubicSpline = 2
};

enum class TargetPath : uint8_t {
    Translation = 0,
    Rotation = 1,
    Scale = 2
};

/// @brief Animation channel driving a specific transform property of a target joint.
struct AnimationChannel {
    int32_t target_node{-1};
    int32_t target_joint{-1};
    TargetPath target_path{TargetPath::Translation};
    InterpolationMode interpolation{InterpolationMode::Linear};

    std::vector<float> timestamps;
    std::vector<Vec3> vec3_values;
    std::vector<Quat> quat_values;

    [[nodiscard]] Vec3 sample_vec3(float time) const noexcept;
    [[nodiscard]] Quat sample_quat(float time) const noexcept;
};

/// @brief Skeletal animation clip containing multiple keyframe channels and playback duration.
class AnimationClip {
public:
    std::string name;
    float duration{0.0f};
    std::vector<AnimationChannel> channels;

    AnimationClip() = default;
    explicit AnimationClip(std::string in_name) : name(std::move(in_name)) {}

    /// @brief Evaluates all channels at the specified time and applies transforms to the skeleton's joints.
    void apply(Skeleton& skeleton, float time) const noexcept;
};

} // namespace nisaba::render3d

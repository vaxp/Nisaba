#pragma once

#include "orion/animation/skeleton.hpp"
#include "orion/animation/animation_clip.hpp"
#include "orion/animation/skinned_mesh.hpp"
#include <string>
#include <vector>
#include <memory>

namespace orion {

/// @brief Skeletal animation playback controller driving skeletons and deformed meshes.
class Animator {
public:
    std::shared_ptr<Skeleton> skeleton;
    std::vector<std::shared_ptr<AnimationClip>> clips;
    std::vector<std::shared_ptr<SkinnedMeshInstance>> skinned_meshes;

    int32_t current_clip_index{-1};
    float current_time{0.0f};
    float speed{1.0f};
    bool looping{true};
    bool is_playing{true};

    Animator() = default;
    explicit Animator(std::shared_ptr<Skeleton> in_skel) : skeleton(std::move(in_skel)) {}

    /// @brief Plays an animation clip by name with looping and playback speed configuration.
    void play(const std::string& clip_name, bool loop = true, float play_speed = 1.0f) noexcept;

    /// @brief Plays an animation clip by index.
    void play(size_t clip_idx, bool loop = true, float play_speed = 1.0f) noexcept;

    void stop() noexcept;
    void pause() noexcept { is_playing = false; }
    void resume() noexcept { is_playing = true; }
    void set_speed(float s) noexcept { speed = s; }
    void set_time(float t) noexcept { current_time = t; }

    /// @brief Advances animation playback time by dt seconds, updates skeleton transforms, and deforms skinned meshes.
    void update(float dt) noexcept;

    /// @brief Searches for a clip by name. Returns clip index or -1 if not found.
    [[nodiscard]] int32_t find_clip(const std::string& clip_name) const noexcept;

    [[nodiscard]] const std::string& get_current_clip_name() const noexcept;
    [[nodiscard]] float get_duration() const noexcept;
};

} // namespace orion

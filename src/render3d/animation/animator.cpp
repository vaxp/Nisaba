#include "nisaba/render3d/animation/animator.hpp"
#include <algorithm>
#include <cmath>

namespace nisaba::render3d {

static const std::string s_empty_str = "";

int32_t Animator::find_clip(const std::string& clip_name) const noexcept {
    for (size_t i = 0; i < clips.size(); ++i) {
        if (clips[i] && clips[i]->name == clip_name) {
            return static_cast<int32_t>(i);
        }
    }
    return -1;
}

void Animator::play(const std::string& clip_name, bool loop, float play_speed) noexcept {
    const int32_t idx = find_clip(clip_name);
    if (idx >= 0) {
        play(static_cast<size_t>(idx), loop, play_speed);
    }
}

void Animator::play(size_t clip_idx, bool loop, float play_speed) noexcept {
    if (clip_idx >= clips.size() || !clips[clip_idx]) return;

    if (current_clip_index != static_cast<int32_t>(clip_idx)) {
        current_clip_index = static_cast<int32_t>(clip_idx);
        current_time = 0.0f;
    }
    looping = loop;
    speed = play_speed;
    is_playing = true;
}

void Animator::stop() noexcept {
    is_playing = false;
    current_time = 0.0f;
    if (skeleton) {
        skeleton->reset_to_bind_pose();
        for (auto& sm : skinned_meshes) {
            if (sm) sm->update_skinning();
        }
    }
}

void Animator::update(float dt) noexcept {
    if (!is_playing || current_clip_index < 0 || static_cast<size_t>(current_clip_index) >= clips.size()) {
        return;
    }
    const auto& clip = clips[static_cast<size_t>(current_clip_index)];
    if (!clip || !skeleton) return;

    current_time += dt * speed;

    const float duration = clip->duration;
    if (duration > 0.0f) {
        if (looping) {
            while (current_time > duration) current_time -= duration;
            while (current_time < 0.0f) current_time += duration;
        } else {
            if (current_time >= duration) {
                current_time = duration;
                is_playing = false;
            }
        }
    }

    clip->apply(*skeleton, current_time);
    skeleton->update_matrices();

    for (auto& sm : skinned_meshes) {
        if (sm) sm->update_skinning();
    }
}

const std::string& Animator::get_current_clip_name() const noexcept {
    if (current_clip_index >= 0 && static_cast<size_t>(current_clip_index) < clips.size() && clips[static_cast<size_t>(current_clip_index)]) {
        return clips[static_cast<size_t>(current_clip_index)]->name;
    }
    return s_empty_str;
}

float Animator::get_duration() const noexcept {
    if (current_clip_index >= 0 && static_cast<size_t>(current_clip_index) < clips.size() && clips[static_cast<size_t>(current_clip_index)]) {
        return clips[static_cast<size_t>(current_clip_index)]->duration;
    }
    return 0.0f;
}

} // namespace nisaba::render3d

/// @file player.cpp
/// @brief Implementation of interactive playback controller for Lottie vector animations.

#include "nisaba/lottie/player.hpp"

#include <algorithm>
#include <cmath>

namespace nisaba::lottie {

Player::Player(std::shared_ptr<Animation> anim) {
    set_animation(std::move(anim));
}

void Player::set_animation(std::shared_ptr<Animation> anim) {
    anim_ = std::move(anim);
    if (anim_) {
        current_frame_ = anim_->in_point();
    } else {
        current_frame_ = 0.0f;
    }
}

void Player::stop() noexcept {
    is_playing_ = false;
    if (anim_) {
        current_frame_ = anim_->in_point();
    }
}

void Player::seek_frame(float frame) noexcept {
    if (!anim_) return;
    float ip = anim_->in_point();
    float op = anim_->out_point();
    current_frame_ = std::clamp(frame, ip, op);
}

void Player::seek_progress(float prog) noexcept {
    if (!anim_) return;
    float p = std::clamp(prog, 0.0f, 1.0f);
    float ip = anim_->in_point();
    float total = anim_->total_frames();
    current_frame_ = ip + p * total;
}

float Player::progress() const noexcept {
    if (!anim_ || anim_->total_frames() <= 0.0f) return 0.0f;
    float ip = anim_->in_point();
    float total = anim_->total_frames();
    return std::clamp((current_frame_ - ip) / total, 0.0f, 1.0f);
}

bool Player::advance(float dt) noexcept {
    if (!anim_ || !is_playing_) return false;
    float ip = anim_->in_point();
    float op = anim_->out_point();
    float total = anim_->total_frames();
    if (total <= 0.0f) return false;

    float frame_delta = dt * anim_->frame_rate() * speed_;
    current_frame_ += frame_delta;

    if (current_frame_ >= op) {
        if (is_looping_) {
            float excess = current_frame_ - op;
            current_frame_ = ip + std::fmod(excess, total);
        } else {
            current_frame_ = op;
            is_playing_ = false;
            return false;
        }
    } else if (current_frame_ < ip) {
        if (is_looping_) {
            float deficit = ip - current_frame_;
            current_frame_ = op - std::fmod(deficit, total);
        } else {
            current_frame_ = ip;
            is_playing_ = false;
            return false;
        }
    }
    return true;
}

void Player::render(ICanvas& canvas) const {
    if (anim_) {
        anim_->render(canvas, current_frame_);
    }
}

void Player::render(ICanvas& canvas, const Rect& dest_rect, bool preserve_aspect_ratio) const {
    if (anim_) {
        anim_->render(canvas, current_frame_, dest_rect, preserve_aspect_ratio);
    }
}

} // namespace nisaba::lottie

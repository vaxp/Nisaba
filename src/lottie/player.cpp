/// @file player.cpp
/// @brief Implementation of interactive playback controller, marker segments, and bounce mode for Lottie animations.

#include "nisaba/lottie/player.hpp"

#include <algorithm>
#include <cmath>

namespace nisaba::lottie {

Player::Player(std::shared_ptr<Animation> anim) {
    set_animation(std::move(anim));
}

void Player::set_animation(std::shared_ptr<Animation> anim) {
    anim_ = std::move(anim);
    segment_in_.reset();
    segment_out_.reset();
    direction_ = 1.0f;
    current_loops_ = 0;
    if (anim_) {
        current_frame_ = anim_->in_point();
    } else {
        current_frame_ = 0.0f;
    }
}

float Player::effective_in_point() const noexcept {
    if (!anim_) return 0.0f;
    return segment_in_.value_or(anim_->in_point());
}

float Player::effective_out_point() const noexcept {
    if (!anim_) return 0.0f;
    return segment_out_.value_or(anim_->out_point());
}

void Player::set_segment(float start_frame, float end_frame) noexcept {
    if (!anim_) return;
    if (start_frame > end_frame) std::swap(start_frame, end_frame);
    segment_in_ = std::clamp(start_frame, anim_->in_point(), anim_->out_point());
    segment_out_ = std::clamp(end_frame, anim_->in_point(), anim_->out_point());
    current_frame_ = std::clamp(current_frame_, *segment_in_, *segment_out_);
}

void Player::reset_segment() noexcept {
    segment_in_.reset();
    segment_out_.reset();
}

bool Player::play_marker(std::string_view marker_name) noexcept {
    if (!anim_) return false;
    const Marker* m = anim_->find_marker(marker_name);
    if (!m) return false;
    float start_f = m->time;
    float end_f = (m->duration > 0.0f) ? (m->time + m->duration) : anim_->out_point();
    set_segment(start_f, end_f);
    current_frame_ = effective_in_point();
    direction_ = 1.0f;
    is_playing_ = true;
    return true;
}

void Player::stop() noexcept {
    is_playing_ = false;
    direction_ = 1.0f;
    current_loops_ = 0;
    if (anim_) {
        current_frame_ = effective_in_point();
    }
}

void Player::seek_frame(float frame) noexcept {
    if (!anim_) return;
    float ip = effective_in_point();
    float op = effective_out_point();
    current_frame_ = std::clamp(frame, ip, op);
}

void Player::seek_progress(float prog) noexcept {
    if (!anim_) return;
    float p = std::clamp(prog, 0.0f, 1.0f);
    float ip = effective_in_point();
    float op = effective_out_point();
    current_frame_ = ip + p * std::max(0.0f, op - ip);
}

float Player::progress() const noexcept {
    if (!anim_) return 0.0f;
    float ip = effective_in_point();
    float op = effective_out_point();
    float total = op - ip;
    if (total <= 0.0f) return 0.0f;
    return std::clamp((current_frame_ - ip) / total, 0.0f, 1.0f);
}

bool Player::advance(float dt) {
    if (!anim_ || !is_playing_) return false;
    float ip = effective_in_point();
    float op = effective_out_point();
    float total = op - ip;
    if (total <= 0.0f) return false;

    float prev_frame = current_frame_;
    float frame_delta = dt * anim_->frame_rate() * speed_ * direction_;
    current_frame_ += frame_delta;

    // Check if any markers were crossed
    if (on_marker && !anim_->markers().empty()) {
        float f_min = std::min(prev_frame, current_frame_);
        float f_max = std::max(prev_frame, current_frame_);
        for (const auto& m : anim_->markers()) {
            if (m.time > f_min && m.time <= f_max) {
                on_marker(m.name);
            }
        }
    }

    auto check_repeat_limit = [&]() -> bool {
        current_loops_++;
        if (on_loop) on_loop(current_loops_);
        if (repeat_count_ >= 0 && current_loops_ >= repeat_count_) {
            is_playing_ = false;
            if (on_complete) on_complete();
            return false;
        }
        return true;
    };

    if (current_frame_ >= op) {
        if (loop_mode_ == LoopMode::Loop) {
            if (!check_repeat_limit()) {
                current_frame_ = op;
                return false;
            }
            float excess = current_frame_ - op;
            current_frame_ = ip + std::fmod(excess, total);
        } else if (loop_mode_ == LoopMode::Bounce) {
            if (!check_repeat_limit()) {
                current_frame_ = op;
                return false;
            }
            float excess = std::fmod(current_frame_ - op, total);
            current_frame_ = op - excess;
            direction_ = -direction_;
        } else {
            current_frame_ = op;
            is_playing_ = false;
            if (on_complete) on_complete();
            return false;
        }
    } else if (current_frame_ < ip) {
        if (loop_mode_ == LoopMode::Loop) {
            if (!check_repeat_limit()) {
                current_frame_ = ip;
                return false;
            }
            float deficit = ip - current_frame_;
            current_frame_ = op - std::fmod(deficit, total);
        } else if (loop_mode_ == LoopMode::Bounce) {
            if (!check_repeat_limit()) {
                current_frame_ = ip;
                return false;
            }
            float deficit = std::fmod(ip - current_frame_, total);
            current_frame_ = ip + deficit;
            direction_ = -direction_;
        } else {
            current_frame_ = ip;
            is_playing_ = false;
            if (on_complete) on_complete();
            return false;
        }
    }

    if (on_frame) {
        on_frame(current_frame_);
    }
    return true;
}

std::optional<std::string> Player::hit_test(float x, float y) const {
    if (!anim_) return std::nullopt;
    return anim_->hit_test(x, y, current_frame_);
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

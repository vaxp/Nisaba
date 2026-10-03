/// @file timeline.cpp
/// @brief AnimationTimeline implementation with Ticker and multi-track orchestration for Nisaba.

#include "nisaba/animation/timeline.hpp"

namespace nisaba::animation {

AnimationTimeline::AnimationTimeline() = default;

AnimationTimeline::AnimationTimeline(Duration duration_override)
    : total_duration_(duration_override), manual_duration_(true) {}

AnimationTimeline::~AnimationTimeline() {
    dispose();
}

AnimationTimeline& AnimationTimeline::add(Duration start_offset, Duration duration,
                                         std::function<void(float progress)> on_update,
                                         const Curve* curve) {
    Track track;
    track.start_ms = start_offset.count();
    track.duration_ms = duration.count();
    track.update_fn = std::move(on_update);
    track.curve = curve ? curve : &Curves::linear;

    tracks_.push_back(std::move(track));
    update_duration();
    return *this;
}

void AnimationTimeline::update_duration() {
    if (manual_duration_) return;

    int64_t max_end = 0;
    for (const auto& t : tracks_) {
        int64_t end_ms = t.start_ms + t.duration_ms;
        if (end_ms > max_end) max_end = end_ms;
    }
    total_duration_ = Duration{max_end};
}

void AnimationTimeline::apply_progress(float p) {
    progress_ = std::clamp(p, 0.0f, 1.0f);
    double total_ms = static_cast<double>(total_duration_.count());
    if (total_ms <= 0.0) {
        for (auto& track : tracks_) {
            if (track.update_fn) track.update_fn(1.0f);
        }
        return;
    }

    double current_ms = progress_ * total_ms;

    for (auto& track : tracks_) {
        if (!track.update_fn) continue;

        if (track.duration_ms <= 0) {
            float val = (current_ms >= track.start_ms) ? 1.0f : 0.0f;
            track.update_fn(val);
            continue;
        }

        constexpr double time_eps = 1e-4;
        double diff = current_ms - static_cast<double>(track.start_ms);
        if (diff <= time_eps) {
            track.update_fn(0.0f);
            continue;
        }

        double raw = diff / static_cast<double>(track.duration_ms);
        float clamped = static_cast<float>(std::clamp(raw, 0.0, 1.0));
        float curved = track.curve ? track.curve->evaluate_f(clamped) : clamped;
        track.update_fn(curved);
    }
}

void AnimationTimeline::play() {
    forward();
}

void AnimationTimeline::forward() {
    is_forward_ = true;
    is_playing_ = true;
    status_ = AnimationStatus::Forward;
    last_tick_time_ = std::chrono::steady_clock::now();
    ensure_ticker();
    notify_status_listeners();
}

void AnimationTimeline::reverse() {
    is_forward_ = false;
    is_playing_ = true;
    status_ = AnimationStatus::Reverse;
    last_tick_time_ = std::chrono::steady_clock::now();
    ensure_ticker();
    notify_status_listeners();
}

void AnimationTimeline::pause() {
    is_playing_ = false;
    stop_ticker();
    notify_status_listeners();
}

void AnimationTimeline::stop() {
    is_playing_ = false;
    stop_ticker();
    notify_status_listeners();
}

void AnimationTimeline::reset() {
    stop();
    seek(0.0f);
    status_ = AnimationStatus::Dismissed;
    notify_status_listeners();
}

void AnimationTimeline::seek(float progress) {
    apply_progress(progress);
    notify_listeners();
}

void AnimationTimeline::seek_ms(int64_t ms) {
    double total_ms = static_cast<double>(total_duration_.count());
    if (total_ms <= 0.0) {
        seek(1.0f);
    } else {
        seek(static_cast<float>(static_cast<double>(ms) / total_ms));
    }
}

bool AnimationTimeline::tick() {
    if (!is_playing_) return false;

    auto now = std::chrono::steady_clock::now();
    double dt_sec = std::chrono::duration<double>(now - last_tick_time_).count();
    last_tick_time_ = now;

    double total_sec = static_cast<double>(total_duration_.count()) / 1000.0;
    if (total_sec <= 0.0) {
        apply_progress(1.0f);
        is_playing_ = false;
        status_ = AnimationStatus::Completed;
        stop_ticker();
        notify_listeners();
        notify_status_listeners();
        return false;
    }

    double delta_p = (dt_sec * speed_) / total_sec;

    if (is_forward_) {
        progress_ += static_cast<float>(delta_p);
        if (progress_ >= 1.0f) {
            if (repeat_) {
                if (pingpong_) {
                    progress_ = 1.0f;
                    is_forward_ = false;
                    status_ = AnimationStatus::Reverse;
                } else {
                    progress_ = 0.0f;
                }
            } else {
                progress_ = 1.0f;
                is_playing_ = false;
                status_ = AnimationStatus::Completed;
                stop_ticker();
                apply_progress(progress_);
                notify_listeners();
                notify_status_listeners();
                return false;
            }
        }
    } else {
        progress_ -= static_cast<float>(delta_p);
        if (progress_ <= 0.0f) {
            if (repeat_) {
                if (pingpong_) {
                    progress_ = 0.0f;
                    is_forward_ = true;
                    status_ = AnimationStatus::Forward;
                } else {
                    progress_ = 1.0f;
                }
            } else {
                progress_ = 0.0f;
                is_playing_ = false;
                status_ = AnimationStatus::Dismissed;
                stop_ticker();
                apply_progress(progress_);
                notify_listeners();
                notify_status_listeners();
                return false;
            }
        }
    }

    apply_progress(progress_);
    notify_listeners();
    return true;
}

bool AnimationTimeline::tick_delta(std::chrono::milliseconds delta) {
    if (!is_playing_) return false;
    last_tick_time_ -= delta;
    return tick();
}

void AnimationTimeline::ensure_ticker() {
    if (ticker_id_ != 0) return;
    ticker_id_ = SchedulerBinding::instance().add_frame_callback([this] {
        tick();
    });
}

void AnimationTimeline::stop_ticker() {
    if (ticker_id_ == 0) return;
    SchedulerBinding::instance().remove_frame_callback(ticker_id_);
    ticker_id_ = 0;
}

void AnimationTimeline::add_listener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

void AnimationTimeline::add_status_listener(StatusListener listener) {
    status_listeners_.push_back(std::move(listener));
}

void AnimationTimeline::clear_listeners() noexcept {
    listeners_.clear();
    status_listeners_.clear();
}

void AnimationTimeline::dispose() noexcept {
    stop_ticker();
    listeners_.clear();
    status_listeners_.clear();
    is_playing_ = false;
}

void AnimationTimeline::notify_listeners() {
    auto snapshot = listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn();
    }
}

void AnimationTimeline::notify_status_listeners() {
    auto snapshot = status_listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn(status_);
    }
}

} // namespace nisaba::animation

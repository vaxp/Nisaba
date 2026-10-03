#pragma once

/// @file timeline.hpp
/// @brief Multi-track AnimationTimeline and Keyframe sequencer for Nisaba.
///
/// Enables choreographing complex, multi-element parallel and staggered animations
/// over a unified timeline. Supports keyframes, scrubbing, looping, and reverse.

#include "nisaba/animation/ticker.hpp"
#include "nisaba/animation/curves.hpp"
#include "nisaba/animation/tween.hpp"
#include "nisaba/animation/animation_controller.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/size.hpp"
#include "nisaba/color/color.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <algorithm>
#include <optional>
#include <utility>

namespace nisaba::animation {

// ════════════════════════════════════════════════════════════════
// Keyframe<T> — Single milestone in a Keyframe track
// ════════════════════════════════════════════════════════════════

template<typename T>
struct Keyframe {
    float        time_fraction = 0.0f;    ///< Position in track timeline [0.0, 1.0].
    T            value{};                 ///< Target value at this keyframe.
    const Curve* curve = &Curves::linear; ///< Easing curve transitioning towards this keyframe.

    Keyframe() = default;
    Keyframe(float fraction, T val, const Curve* c = &Curves::linear)
        : time_fraction(std::clamp(fraction, 0.0f, 1.0f)), value(std::move(val)), curve(c) {}
};

// ════════════════════════════════════════════════════════════════
// KeyframeSequence<T> — Multi-stop interpolated timeline
// ════════════════════════════════════════════════════════════════

template<typename T>
class KeyframeSequence {
public:
    KeyframeSequence() = default;
    explicit KeyframeSequence(std::vector<Keyframe<T>> keyframes)
        : keyframes_(std::move(keyframes)) {
        std::sort(keyframes_.begin(), keyframes_.end(), [](const auto& a, const auto& b) {
            return a.time_fraction < b.time_fraction;
        });
    }

    void add_keyframe(float fraction, T value, const Curve* curve = &Curves::linear) {
        keyframes_.emplace_back(fraction, std::move(value), curve);
        std::sort(keyframes_.begin(), keyframes_.end(), [](const auto& a, const auto& b) {
            return a.time_fraction < b.time_fraction;
        });
    }

    void addKeyframe(float fraction, T value, const Curve* curve = &Curves::linear) {
        add_keyframe(fraction, std::move(value), curve);
    }

    [[nodiscard]] T evaluate(float t) const {
        if (keyframes_.empty()) return T{};
        if (keyframes_.size() == 1 || t <= keyframes_.front().time_fraction) {
            return keyframes_.front().value;
        }
        if (t >= keyframes_.back().time_fraction) {
            return keyframes_.back().value;
        }

        // Find surrounding interval
        for (size_t i = 0; i < keyframes_.size() - 1; ++i) {
            const auto& k1 = keyframes_[i];
            const auto& k2 = keyframes_[i + 1];
            if (t >= k1.time_fraction && t <= k2.time_fraction) {
                float seg_dur = k2.time_fraction - k1.time_fraction;
                float local_t = (seg_dur > 0.0f) ? (t - k1.time_fraction) / seg_dur : 1.0f;
                local_t = std::clamp(local_t, 0.0f, 1.0f);
                Tween<T> segment_tween(k1.value, k2.value, k2.curve);
                return segment_tween.evaluate_f(local_t);
            }
        }
        return keyframes_.back().value;
    }

private:
    std::vector<Keyframe<T>> keyframes_;
};

// ════════════════════════════════════════════════════════════════
// AnimationTimeline — Orchestrator for multiple animation tracks
// ════════════════════════════════════════════════════════════════

class AnimationTimeline {
public:
    using Duration = std::chrono::milliseconds;
    using Listener = std::function<void()>;
    using StatusListener = std::function<void(AnimationStatus)>;

    AnimationTimeline();
    explicit AnimationTimeline(Duration duration_override);
    ~AnimationTimeline();

    AnimationTimeline(const AnimationTimeline&) = delete;
    AnimationTimeline& operator=(const AnimationTimeline&) = delete;

    // ── Track Composition ────────────────────────────────────────

    /// Add an animation callback track with start offset and duration
    AnimationTimeline& add(Duration start_offset, Duration duration,
                           std::function<void(float progress)> on_update,
                           const Curve* curve = &Curves::linear);

    /// Add a Tween track interpolating between typed values
    template<typename T>
    AnimationTimeline& add_tween(Duration start_offset, Duration duration,
                                 Tween<T> tween,
                                 std::function<void(const T& val)> on_update) {
        auto shared_tween = std::make_shared<Tween<T>>(std::move(tween));
        auto shared_cb = std::move(on_update);
        return add(start_offset, duration, [shared_tween, shared_cb](float p) {
            if (shared_cb) shared_cb(shared_tween->evaluate_f(p));
        }, &Curves::linear);
    }

    template<typename T>
    AnimationTimeline& addTween(Duration start_offset, Duration duration,
                                Tween<T> tween,
                                std::function<void(const T& val)> on_update) {
        return add_tween(start_offset, duration, std::move(tween), std::move(on_update));
    }

    /// Add a multi-stop Keyframe sequence track
    template<typename T>
    AnimationTimeline& add_keyframes(Duration start_offset, Duration duration,
                                     KeyframeSequence<T> sequence,
                                     std::function<void(const T& val)> on_update) {
        auto shared_seq = std::make_shared<KeyframeSequence<T>>(std::move(sequence));
        auto shared_cb = std::move(on_update);
        return add(start_offset, duration, [shared_seq, shared_cb](float p) {
            if (shared_cb) shared_cb(shared_seq->evaluate(p));
        }, &Curves::linear);
    }

    template<typename T>
    AnimationTimeline& addKeyframes(Duration start_offset, Duration duration,
                                    KeyframeSequence<T> sequence,
                                    std::function<void(const T& val)> on_update) {
        return add_keyframes(start_offset, duration, std::move(sequence), std::move(on_update));
    }

    // ── Playback Controls ────────────────────────────────────────

    void play();
    void pause();
    void stop();
    void reset();
    void forward();
    void reverse();

    /// Jump directly to normalized timeline progress in [0.0, 1.0]
    void seek(float progress);

    /// Jump directly to specific millisecond timestamp in timeline
    void seek_ms(int64_t ms);
    void seekMs(int64_t ms) { seek_ms(ms); }

    // ── Playback Configuration ───────────────────────────────────

    void set_speed(float speed) noexcept { speed_ = std::max(0.01f, speed); }
    void setSpeed(float speed) noexcept { set_speed(speed); }
    [[nodiscard]] float speed() const noexcept { return speed_; }

    void set_repeat(bool repeat) noexcept { repeat_ = repeat; }
    void setRepeat(bool repeat) noexcept { set_repeat(repeat); }
    [[nodiscard]] bool is_repeating() const noexcept { return repeat_; }
    [[nodiscard]] bool isRepeating() const noexcept { return is_repeating(); }

    void set_ping_pong(bool pingpong) noexcept { pingpong_ = pingpong; }
    void setPingPong(bool pingpong) noexcept { set_ping_pong(pingpong); }
    [[nodiscard]] bool is_ping_pong() const noexcept { return pingpong_; }
    [[nodiscard]] bool isPingPong() const noexcept { return is_ping_pong(); }

    // ── State Queries ────────────────────────────────────────────

    [[nodiscard]] Duration total_duration() const noexcept { return total_duration_; }
    [[nodiscard]] Duration totalDuration() const noexcept { return total_duration(); }
    [[nodiscard]] float progress() const noexcept { return progress_; }
    [[nodiscard]] bool is_playing() const noexcept { return is_playing_; }
    [[nodiscard]] bool isPlaying() const noexcept { return is_playing(); }
    [[nodiscard]] AnimationStatus status() const noexcept { return status_; }

    // ── Per-Frame Update (driven by Ticker) ───────────────────────

    bool tick();
    bool tick_delta(std::chrono::milliseconds delta);

    // ── Listeners ────────────────────────────────────────────────

    void add_listener(Listener listener);
    void addListener(Listener listener) { add_listener(std::move(listener)); }

    void add_status_listener(StatusListener listener);
    void addStatusListener(StatusListener listener) { add_status_listener(std::move(listener)); }

    void clear_listeners() noexcept;
    void clearListeners() noexcept { clear_listeners(); }

    void dispose() noexcept;

private:
    struct Track {
        int64_t start_ms   = 0;
        int64_t duration_ms = 0;
        std::function<void(float)> update_fn;
        const Curve* curve = &Curves::linear;
    };

    void ensure_ticker();
    void stop_ticker();
    void apply_progress(float p);
    void update_duration();
    void notify_listeners();
    void notify_status_listeners();

    std::vector<Track> tracks_;
    Duration total_duration_{0};
    bool manual_duration_ = false;

    float progress_ = 0.0f;
    float speed_    = 1.0f;
    bool is_playing_ = false;
    bool is_forward_ = true;
    bool repeat_     = false;
    bool pingpong_   = false;

    AnimationStatus status_ = AnimationStatus::Dismissed;
    std::chrono::steady_clock::time_point last_tick_time_;
    size_t ticker_id_ = 0;

    std::vector<Listener>       listeners_;
    std::vector<StatusListener> status_listeners_;
};

} // namespace nisaba::animation

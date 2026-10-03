#pragma once

/// @file animation_controller.hpp
/// @brief AnimationController — drives time-based animations producing normalized progress in [0, 1].
/// Supports forward, reverse, repeat, and ping-pong modes with callbacks and status signals.

#include "nisaba/animation/curves.hpp"
#include "nisaba/animation/signal.hpp"
#include "nisaba/animation/tween.hpp"

#include <chrono>
#include <functional>
#include <vector>
#include <algorithm>
#include <utility>

namespace nisaba::animation {

// ════════════════════════════════════════════════════════════════
// AnimationStatus
// ════════════════════════════════════════════════════════════════

enum class AnimationStatus {
    Dismissed,   ///< At beginning (value = 0, stopped)
    Forward,     ///< Running toward end (value increases)
    Reverse,     ///< Running toward beginning (value decreases)
    Completed,   ///< At end (value = 1, stopped)
};

// ════════════════════════════════════════════════════════════════
// AnimationController
// ════════════════════════════════════════════════════════════════

/// @brief Drives a time-based animation, producing values in [0, 1].
class AnimationController {
public:
    using Duration    = std::chrono::milliseconds;
    using Listener    = std::function<void()>;
    using TimePoint   = std::chrono::steady_clock::time_point;

    // ── Construction ────────────────────────────────────────────

    explicit AnimationController(Duration duration = Duration{300})
        : duration_(duration) {}

    ~AnimationController() { dispose(); }

    // Non-copyable
    AnimationController(const AnimationController&) = delete;
    AnimationController& operator=(const AnimationController&) = delete;

    // Movable
    AnimationController(AnimationController&& other) noexcept
        : value_(other.value_),
          start_value_(other.start_value_),
          status_(other.status_),
          active_(other.active_),
          repeats_(other.repeats_),
          pingpong_(other.pingpong_),
          duration_(other.duration_),
          start_time_(other.start_time_),
          listeners_(std::move(other.listeners_)),
          status_listeners_(std::move(other.status_listeners_)) {
        other.active_ = false;
    }

    AnimationController& operator=(AnimationController&& other) noexcept {
        if (this != &other) {
            dispose();
            value_ = other.value_;
            start_value_ = other.start_value_;
            status_ = other.status_;
            active_ = other.active_;
            repeats_ = other.repeats_;
            pingpong_ = other.pingpong_;
            duration_ = other.duration_;
            start_time_ = other.start_time_;
            listeners_ = std::move(other.listeners_);
            status_listeners_ = std::move(other.status_listeners_);
            other.active_ = false;
        }
        return *this;
    }

    // ── Configuration ────────────────────────────────────────────

    void set_duration(Duration d) noexcept { duration_ = d; }
    void setDuration(Duration d) noexcept { set_duration(d); }
    [[nodiscard]] Duration duration() const noexcept { return duration_; }

    void set_repeats(bool repeats) noexcept { repeats_ = repeats; }
    void setRepeats(bool repeats) noexcept { set_repeats(repeats); }
    [[nodiscard]] bool repeats() const noexcept { return repeats_; }

    void set_ping_pong(bool pingpong) noexcept { pingpong_ = pingpong; }
    void setPingPong(bool pingpong) noexcept { set_ping_pong(pingpong); }
    [[nodiscard]] bool ping_pong() const noexcept { return pingpong_; }

    // ── Control ─────────────────────────────────────────────────

    /// Start animating from current value toward end (value = 1).
    void forward() {
        status_ = AnimationStatus::Forward;
        start_time_ = std::chrono::steady_clock::now();
        start_value_ = value_;
        active_ = true;
        notify_status_listeners();
    }

    /// Start animating from current value toward beginning (value = 0).
    void reverse() {
        status_ = AnimationStatus::Reverse;
        start_time_ = std::chrono::steady_clock::now();
        start_value_ = value_;
        active_ = true;
        notify_status_listeners();
    }

    /// Repeat continuously in one direction.
    void repeat() {
        repeats_ = true;
        pingpong_ = false;
        forward();
    }

    /// Repeat continuously bouncing back and forth.
    void ping_pong_loop() {
        repeats_ = true;
        pingpong_ = true;
        forward();
    }

    /// Stop the animation at its current value.
    void stop() {
        if (!active_) return;
        active_ = false;
        if (status_ == AnimationStatus::Forward || status_ == AnimationStatus::Reverse) {
            status_ = value_ >= 1.0f ? AnimationStatus::Completed : AnimationStatus::Dismissed;
        }
        notify_status_listeners();
    }

    /// Reset to 0 and stop.
    void reset() {
        active_ = false;
        value_ = 0.0f;
        status_ = AnimationStatus::Dismissed;
        notify_listeners();
        notify_status_listeners();
    }

    /// Jump directly to a value.
    void set_value(float val) {
        value_ = std::clamp(val, 0.0f, 1.0f);
        if (value_ >= 1.0f) status_ = AnimationStatus::Completed;
        else if (value_ <= 0.0f) status_ = AnimationStatus::Dismissed;
        notify_listeners();
    }

    void setValue(float val) { set_value(val); }

    /// Animate forward if dismissed/reversed, backward if completed/forward.
    void toggle() {
        if (status_ == AnimationStatus::Forward || status_ == AnimationStatus::Completed) {
            reverse();
        } else {
            forward();
        }
    }

    // ── Per-frame tick ──────────────────────────────────────────

    /// Advance the animation by current clock time. Returns true if still running.
    bool tick() {
        if (!active_) return false;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time_).count();
        double dur_ms = static_cast<double>(duration_.count());

        double raw = dur_ms > 0.0 ? static_cast<double>(elapsed) / dur_ms : 1.0;
        raw = std::clamp(raw, 0.0, 1.0);

        if (status_ == AnimationStatus::Forward) {
            double new_val = start_value_ + raw * (1.0 - start_value_);
            value_ = static_cast<float>(std::clamp(new_val, 0.0, 1.0));
        } else {
            double new_val = start_value_ - raw * start_value_;
            value_ = static_cast<float>(std::clamp(new_val, 0.0, 1.0));
        }

        notify_listeners();

        // Check completion
        bool done = (raw >= 1.0);
        if (done) {
            if (repeats_) {
                if (pingpong_) {
                    if (status_ == AnimationStatus::Forward) {
                        reverse();
                    } else {
                        forward();
                    }
                } else {
                    reset();
                    forward();
                }
            } else {
                active_ = false;
                status_ = (status_ == AnimationStatus::Forward)
                    ? AnimationStatus::Completed
                    : AnimationStatus::Dismissed;
                notify_status_listeners();
            }
            return repeats_;
        }

        return true;
    }

    /// Advance the animation by explicit delta time in milliseconds (useful for offline tests/simulation).
    bool tick_delta(std::chrono::milliseconds delta) {
        if (!active_) return false;
        start_time_ -= delta;
        return tick();
    }

    // ── Listeners ────────────────────────────────────────────────

    void add_listener(Listener listener) {
        listeners_.push_back(std::move(listener));
    }

    void addListener(Listener listener) {
        add_listener(std::move(listener));
    }

    void add_status_listener(std::function<void(AnimationStatus)> listener) {
        status_listeners_.push_back(std::move(listener));
    }

    void addStatusListener(std::function<void(AnimationStatus)> listener) {
        add_status_listener(std::move(listener));
    }

    void clear_listeners() noexcept {
        listeners_.clear();
        status_listeners_.clear();
    }

    void clearListeners() noexcept {
        clear_listeners();
    }

    // ── Accessors ────────────────────────────────────────────────

    [[nodiscard]] float value() const noexcept { return value_; }
    [[nodiscard]] AnimationStatus status() const noexcept { return status_; }
    [[nodiscard]] bool is_animating() const noexcept { return active_; }
    [[nodiscard]] bool isAnimating() const noexcept { return is_animating(); }
    [[nodiscard]] bool is_completed() const noexcept { return status_ == AnimationStatus::Completed; }
    [[nodiscard]] bool isCompleted() const noexcept { return is_completed(); }
    [[nodiscard]] bool is_dismissed() const noexcept { return status_ == AnimationStatus::Dismissed; }
    [[nodiscard]] bool isDismissed() const noexcept { return is_dismissed(); }

    void dispose() noexcept {
        active_ = false;
        listeners_.clear();
        status_listeners_.clear();
    }

private:
    void notify_listeners() {
        for (auto& fn : listeners_) fn();
    }

    void notify_status_listeners() {
        for (auto& fn : status_listeners_) fn(status_);
    }

    float           value_       = 0.0f;
    double          start_value_ = 0.0;
    AnimationStatus status_      = AnimationStatus::Dismissed;
    bool            active_      = false;
    bool            repeats_     = false;
    bool            pingpong_    = false;

    Duration  duration_{300};
    TimePoint start_time_;

    std::vector<Listener>                             listeners_;
    std::vector<std::function<void(AnimationStatus)>> status_listeners_;
};

// ════════════════════════════════════════════════════════════════
// AnimatedValue<T> — syntactic sugar combining Controller + Tween
// ════════════════════════════════════════════════════════════════

/// @brief Combines an AnimationController with a Tween for convenience.
template<typename T>
class AnimatedValue {
public:
    AnimatedValue(T begin_val, T end_val,
                  AnimationController::Duration duration = std::chrono::milliseconds{300},
                  const Curve* curve_ptr = nullptr)
        : begin_(begin_val), end_(end_val), curve_(curve_ptr), controller_(duration) {}

    void forward()  { controller_.forward(); }
    void reverse()  { controller_.reverse(); }
    void toggle()   { controller_.toggle(); }
    void reset()    { controller_.reset(); }
    [[nodiscard]] bool is_animating() const noexcept { return controller_.is_animating(); }
    [[nodiscard]] bool isAnimating() const noexcept { return is_animating(); }

    void add_listener(AnimationController::Listener fn) { controller_.add_listener(std::move(fn)); }
    void addListener(AnimationController::Listener fn) { add_listener(std::move(fn)); }

    void dispose() noexcept { controller_.dispose(); }

    bool tick() { return controller_.tick(); }

    [[nodiscard]] T get() const {
        Tween<T> tween(begin_, end_, curve_);
        return tween.evaluate(controller_.value());
    }

    [[nodiscard]] AnimationController& controller() noexcept { return controller_; }
    [[nodiscard]] const AnimationController& controller() const noexcept { return controller_; }

private:
    T begin_{};
    T end_{};
    const Curve* curve_ = nullptr;
    AnimationController controller_;
};

} // namespace nisaba::animation

#pragma once

/// @file spring_controller.hpp
/// @brief Physics-based SpringController for fluid, interruptible UI animations in Nisaba.
/// Retargeting mid-animation preserves momentum without visual jarring.
/// Automatically sleeps when kinetic energy settles (0% idle CPU).

#include "nisaba/animation/ticker.hpp"
#include "nisaba/animation/animation_controller.hpp"
#include "nisaba/animation/spring_simulation.hpp"
#include "nisaba/animation/tween.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/size.hpp"
#include "nisaba/color/color.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace nisaba::animation {

// ════════════════════════════════════════════════════════════════
// SpringController — Ticker-driven Physics Controller
// ════════════════════════════════════════════════════════════════

class SpringController {
public:
    using Listener = std::function<void()>;
    using StatusListener = std::function<void(AnimationStatus)>;

    explicit SpringController(SpringDescription desc = Springs::smooth,
                              float initial_value = 0.0f);
    ~SpringController();

    // Non-copyable
    SpringController(const SpringController&) = delete;
    SpringController& operator=(const SpringController&) = delete;

    // ── Configuration ────────────────────────────────────────────

    void set_spring(const SpringDescription& desc);
    void setSpring(const SpringDescription& desc) { set_spring(desc); }
    [[nodiscard]] const SpringDescription& spring() const noexcept { return desc_; }

    void set_tolerance(float distance_tol, float velocity_tol = 1e-2f) noexcept;
    void setTolerance(float distance_tol, float velocity_tol = 1e-2f) noexcept {
        set_tolerance(distance_tol, velocity_tol);
    }

    // ── Interactive Control ──────────────────────────────────────

    /// Smoothly animates toward target value. If already moving, preserves
    /// current velocity so movement seamlessly redirects without jerking.
    void animate_to(float target, std::optional<float> initial_velocity = std::nullopt);
    void animateTo(float target, std::optional<float> initial_velocity = std::nullopt) {
        animate_to(target, initial_velocity);
    }

    /// Instantly snaps to value and clears velocity.
    void snap_to(float value);
    void snapTo(float value) { snap_to(value); }

    /// Stops animation at current position, resetting velocity to 0.
    void stop();

    /// Reset to 0 and stop.
    void reset();

    // ── Per-Frame Update ─────────────────────────────────────────

    /// Advance physics simulation by elapsed time.
    bool tick();

    /// Advance simulation by explicit delta (for deterministic simulation/tests).
    bool tick_delta(std::chrono::milliseconds delta);

    // ── State Queries ────────────────────────────────────────────

    [[nodiscard]] float value() const noexcept { return current_value_; }
    [[nodiscard]] float velocity() const noexcept { return current_velocity_; }
    [[nodiscard]] float target() const noexcept { return target_value_; }
    [[nodiscard]] bool is_animating() const noexcept { return is_animating_; }
    [[nodiscard]] bool isAnimating() const noexcept { return is_animating(); }
    [[nodiscard]] AnimationStatus status() const noexcept { return status_; }

    // ── Listeners ────────────────────────────────────────────────

    void add_listener(Listener listener);
    void addListener(Listener listener) { add_listener(std::move(listener)); }

    void add_status_listener(StatusListener listener);
    void addStatusListener(StatusListener listener) { add_status_listener(std::move(listener)); }

    void clear_listeners() noexcept;
    void clearListeners() noexcept { clear_listeners(); }

    // ── Cleanup ──────────────────────────────────────────────────

    void dispose() noexcept;

private:
    void ensure_ticker();
    void stop_ticker();
    void notify_listeners();
    void notify_status_listeners();

    SpringDescription desc_{Springs::smooth};
    float current_value_    = 0.0f;
    float current_velocity_ = 0.0f;
    float target_value_     = 0.0f;
    SpringSimulation  sim_;
    float distance_tol_     = 1e-3f;
    float velocity_tol_     = 1e-2f;

    bool is_animating_ = false;
    AnimationStatus status_ = AnimationStatus::Dismissed;

    std::chrono::steady_clock::time_point start_time_;
    size_t ticker_id_ = 0;

    std::vector<Listener>       listeners_;
    std::vector<StatusListener> status_listeners_;
};

// ════════════════════════════════════════════════════════════════
// SpringValue<T> — Typed Physics Animated Value
// ════════════════════════════════════════════════════════════════

/// @brief Syntactic sugar wrapping SpringController for typed properties (float, Point, Rect, Size, Color).
template<typename T>
class SpringValue {
public:
    explicit SpringValue(T initial, SpringDescription desc = Springs::smooth)
        : current_(initial), start_(initial), target_(initial), controller_(desc, 0.0f) {
        controller_.add_listener([this] {
            float p = controller_.value();
            Tween<T> tween(start_, target_, &Curves::linear);
            current_ = tween.evaluate(p);
        });
    }

    void animate_to(T target, std::optional<float> velocity = std::nullopt) {
        start_ = current_;
        target_ = target;
        controller_.snap_to(0.0f);
        controller_.animate_to(1.0f, velocity);
    }

    void animateTo(T target, std::optional<float> velocity = std::nullopt) {
        animate_to(target, velocity);
    }

    void snap_to(T val) {
        start_ = val;
        current_ = val;
        target_ = val;
        controller_.snap_to(1.0f);
    }

    void snapTo(T val) { snap_to(val); }

    [[nodiscard]] T get() const noexcept { return current_; }
    [[nodiscard]] bool is_animating() const noexcept { return controller_.is_animating(); }
    [[nodiscard]] bool isAnimating() const noexcept { return is_animating(); }

    void add_listener(SpringController::Listener l) { controller_.add_listener(std::move(l)); }
    void addListener(SpringController::Listener l) { add_listener(std::move(l)); }

    [[nodiscard]] SpringController& controller() noexcept { return controller_; }
    [[nodiscard]] const SpringController& controller() const noexcept { return controller_; }

private:
    T current_{};
    T start_{};
    T target_{};
    SpringController controller_;
};

} // namespace nisaba::animation

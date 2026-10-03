/// @file spring_controller.cpp
/// @brief SpringController implementation with Ticker binding for Nisaba.

#include "nisaba/animation/spring_controller.hpp"

namespace nisaba::animation {

SpringController::SpringController(SpringDescription desc, float initial_value)
    : desc_(desc),
      current_value_(initial_value),
      target_value_(initial_value),
      sim_(desc, initial_value, initial_value, 0.0f) {}

SpringController::~SpringController() {
    dispose();
}

void SpringController::set_spring(const SpringDescription& desc) {
    desc_ = desc;
    if (is_animating_) {
        // Retarget with new spring parameters without resetting progress
        animate_to(target_value_);
    }
}

void SpringController::set_tolerance(float distance_tol, float velocity_tol) noexcept {
    distance_tol_ = distance_tol;
    velocity_tol_ = velocity_tol;
}

void SpringController::animate_to(float target, std::optional<float> initial_velocity) {
    float start_val = current_value_;
    float start_vel = initial_velocity.value_or(current_velocity_);

    target_value_ = target;

    // Build fresh simulation from current dynamic state
    sim_ = SpringSimulation(desc_, start_val, target_value_, start_vel, distance_tol_, velocity_tol_);
    start_time_ = std::chrono::steady_clock::now();
    is_animating_ = true;

    status_ = (target_value_ >= start_val) ? AnimationStatus::Forward : AnimationStatus::Reverse;

    ensure_ticker();
    notify_status_listeners();
}

void SpringController::snap_to(float value) {
    stop_ticker();
    current_value_ = value;
    current_velocity_ = 0.0f;
    target_value_ = value;
    is_animating_ = false;
    status_ = (value >= 1.0f) ? AnimationStatus::Completed : AnimationStatus::Dismissed;
    notify_listeners();
    notify_status_listeners();
}

void SpringController::stop() {
    if (!is_animating_) return;
    stop_ticker();
    current_velocity_ = 0.0f;
    is_animating_ = false;
    notify_status_listeners();
}

void SpringController::reset() {
    snap_to(0.0f);
}

bool SpringController::tick() {
    if (!is_animating_) return false;

    auto now = std::chrono::steady_clock::now();
    double elapsed_s = std::chrono::duration<double>(now - start_time_).count();
    float t = static_cast<float>(elapsed_s);

    current_value_ = sim_.x(t);
    current_velocity_ = sim_.dx(t);

    notify_listeners();

    if (sim_.is_done(t)) {
        current_value_ = target_value_;
        current_velocity_ = 0.0f;
        is_animating_ = false;
        status_ = (target_value_ >= 1.0f) ? AnimationStatus::Completed : AnimationStatus::Dismissed;
        stop_ticker();
        notify_status_listeners();
        return false;
    }

    return true;
}

bool SpringController::tick_delta(std::chrono::milliseconds delta) {
    if (!is_animating_) return false;
    start_time_ -= delta;
    return tick();
}

void SpringController::ensure_ticker() {
    if (ticker_id_ != 0) return;
    ticker_id_ = SchedulerBinding::instance().add_frame_callback([this] {
        tick();
    });
}

void SpringController::stop_ticker() {
    if (ticker_id_ == 0) return;
    SchedulerBinding::instance().remove_frame_callback(ticker_id_);
    ticker_id_ = 0;
}

void SpringController::add_listener(Listener listener) {
    listeners_.push_back(std::move(listener));
}

void SpringController::add_status_listener(StatusListener listener) {
    status_listeners_.push_back(std::move(listener));
}

void SpringController::clear_listeners() noexcept {
    listeners_.clear();
    status_listeners_.clear();
}

void SpringController::dispose() noexcept {
    stop_ticker();
    listeners_.clear();
    status_listeners_.clear();
    is_animating_ = false;
}

void SpringController::notify_listeners() {
    auto snapshot = listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn();
    }
}

void SpringController::notify_status_listeners() {
    auto snapshot = status_listeners_;
    for (auto& fn : snapshot) {
        if (fn) fn(status_);
    }
}

} // namespace nisaba::animation

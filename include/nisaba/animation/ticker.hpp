#pragma once

/// @file ticker.hpp
/// @brief Ticker — per-frame callback system for Nisaba animations.
///
/// A Ticker fires a callback on every frame. AnimationControllers
/// register with the SchedulerBinding so they can advance their
/// progress each frame.

#include <functional>
#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>
#include <utility>

namespace nisaba::animation {

using FrameCallback = std::function<void()>;

// ════════════════════════════════════════════════════════════════
// SchedulerBinding — drives all tickers every frame
// ════════════════════════════════════════════════════════════════

/// @brief Global animation scheduler.
/// Holds a list of per-frame callbacks and fires them each frame.
class SchedulerBinding {
public:
    /// Register a callback to be called every frame.
    /// Returns an ID that can be used to cancel it.
    size_t add_frame_callback(FrameCallback cb) {
        size_t id = next_id_++;
        callbacks_.push_back({id, std::move(cb)});
        return id;
    }

    size_t addFrameCallback(FrameCallback cb) {
        return add_frame_callback(std::move(cb));
    }

    /// Remove a previously registered callback.
    void remove_frame_callback(size_t id) {
        callbacks_.erase(
            std::remove_if(callbacks_.begin(), callbacks_.end(),
                [id](const Entry& e) { return e.id == id; }),
            callbacks_.end());
    }

    void removeFrameCallback(size_t id) {
        remove_frame_callback(id);
    }

    /// Called once per frame by the application loop — fires all registered callbacks.
    void tick() {
        auto snapshot = callbacks_;
        for (auto& entry : snapshot) {
            entry.callback();
        }
    }

    /// @return true if there are any active callbacks.
    [[nodiscard]] bool has_callbacks() const noexcept { return !callbacks_.empty(); }
    [[nodiscard]] bool hasCallbacks() const noexcept { return has_callbacks(); }

    /// @return The number of currently active frame callbacks.
    [[nodiscard]] size_t ticker_count() const noexcept { return callbacks_.size(); }
    [[nodiscard]] size_t tickerCount() const noexcept { return ticker_count(); }

    /// @return Global singleton instance.
    static SchedulerBinding& instance() {
        static SchedulerBinding singleton;
        return singleton;
    }

private:
    struct Entry {
        size_t id;
        FrameCallback callback;
    };

    std::vector<Entry> callbacks_;
    size_t next_id_ = 1;
};

// ════════════════════════════════════════════════════════════════
// Ticker — a single per-frame listener
// ════════════════════════════════════════════════════════════════

/// @brief A single animation tick listener.
class Ticker {
public:
    explicit Ticker(FrameCallback callback)
        : callback_(std::move(callback)) {}

    ~Ticker() { stop(); }

    Ticker(const Ticker&) = delete;
    Ticker& operator=(const Ticker&) = delete;

    /// Start firing the callback every frame.
    void start() {
        if (active_) return;
        active_ = true;
        id_ = SchedulerBinding::instance().add_frame_callback(callback_);
    }

    /// Stop firing the callback.
    void stop() {
        if (!active_) return;
        active_ = false;
        SchedulerBinding::instance().remove_frame_callback(id_);
    }

    [[nodiscard]] bool is_active() const noexcept { return active_; }
    [[nodiscard]] bool isActive() const noexcept { return is_active(); }

private:
    FrameCallback callback_;
    size_t id_ = 0;
    bool active_ = false;
};

/// @brief Create a Ticker with the given per-frame callback.
inline std::unique_ptr<Ticker> create_ticker(FrameCallback callback) {
    return std::make_unique<Ticker>(std::move(callback));
}

inline std::unique_ptr<Ticker> createTicker(FrameCallback callback) {
    return create_ticker(std::move(callback));
}

} // namespace nisaba::animation

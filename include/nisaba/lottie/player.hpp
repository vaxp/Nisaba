#pragma once

/// @file player.hpp
/// @brief Interactive playback controller for Lottie vector animations.

#include "nisaba/lottie/animation.hpp"

#include <memory>

namespace nisaba::lottie {

class Player {
public:
    explicit Player(std::shared_ptr<Animation> anim = nullptr);
    ~Player() = default;

    void set_animation(std::shared_ptr<Animation> anim);
    [[nodiscard]] std::shared_ptr<Animation> animation() const noexcept { return anim_; }

    void play() noexcept { is_playing_ = true; }
    void pause() noexcept { is_playing_ = false; }
    void stop() noexcept;

    void set_loop(bool loop) noexcept { is_looping_ = loop; }
    [[nodiscard]] bool is_looping() const noexcept { return is_looping_; }
    [[nodiscard]] bool is_playing() const noexcept { return is_playing_; }

    void set_speed(float speed) noexcept { speed_ = speed; }
    [[nodiscard]] float speed() const noexcept { return speed_; }

    /// @brief Seek directly to a frame number.
    void seek_frame(float frame) noexcept;

    /// @brief Seek to a normalized progress [0.0, 1.0].
    void seek_progress(float progress) noexcept;

    /// @brief Current frame being displayed.
    [[nodiscard]] float current_frame() const noexcept { return current_frame_; }

    /// @brief Current normalized playback progress in [0.0, 1.0].
    [[nodiscard]] float progress() const noexcept;

    /// @brief Advance playback by delta time in seconds.
    /// @return True if animation is still active/playing.
    bool advance(float delta_time_seconds) noexcept;

    /// @brief Render current frame to canvas (1:1 scale).
    void render(ICanvas& canvas) const;

    /// @brief Render current frame fitted into destination bounds.
    void render(ICanvas& canvas, const Rect& dest_rect, bool preserve_aspect_ratio = true) const;

private:
    std::shared_ptr<Animation> anim_;
    bool is_playing_ = true;
    bool is_looping_ = true;
    float speed_ = 1.0f;
    float current_frame_ = 0.0f;
};

} // namespace nisaba::lottie

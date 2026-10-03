#pragma once

#include <cstdint>
#include <vector>
#include <functional>
#include <optional>
#include <cmath>

#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/math/rect.hpp"

namespace nisaba::damage {

/// @brief Playback looping mode for BakedAnimation.
enum class AnimationLoopMode : uint8_t {
    Loop,      ///< Wrap around: 0, 1, ..., N-1, 0, 1, ...
    PingPong,  ///< Bounce back and forth: 0, 1, ..., N-1, N-2, ..., 1, 0, ...
    Clamp      ///< Play once and hold final frame: 0, 1, ..., N-1, N-1, ...
};

/// @brief Pre-Baked Cyclic Animation Primitive (Zero-Cost Recorded Replay).
/// Pre-renders complex vector animations (curves, sine waves, trigonometric transforms)
/// into a compact cyclic frame buffer once, and replays them at runtime via pure L1/L2
/// cache blits, completely eliminating CPU math and rasterization overhead during playback.
class BakedAnimation {
public:
    BakedAnimation() = default;

    BakedAnimation(uint32_t width, uint32_t height, size_t frame_count,
                   AnimationLoopMode mode = AnimationLoopMode::Loop) {
        init(width, height, frame_count, mode);
    }

    bool init(uint32_t width, uint32_t height, size_t frame_count,
              AnimationLoopMode mode = AnimationLoopMode::Loop) {
        width_ = width;
        height_ = height;
        mode_ = mode;
        is_baked_ = false;
        frames_.clear();

        if (width == 0 || height == 0 || frame_count == 0) {
            return false;
        }

        frames_.reserve(frame_count);
        for (size_t i = 0; i < frame_count; ++i) {
            auto p = Pixmap::create(width, height);
            if (!p) {
                frames_.clear();
                return false;
            }
            frames_.push_back(std::move(*p));
        }
        return true;
    }

    /// @brief Pre-renders all frames by executing render_fn once for each frame index.
    bool bake(const std::function<void(Canvas&, size_t frame_idx)>& render_fn) {
        if (frames_.empty()) return false;

        for (size_t i = 0; i < frames_.size(); ++i) {
            Canvas canvas(frames_[i]);
            render_fn(canvas, i);
        }

        is_baked_ = true;
        return true;
    }

    /// @brief Maps any arbitrary timeline tick (frame counter / timestamp) to a valid frame index.
    [[nodiscard]] size_t tick_to_frame_index(int64_t tick) const noexcept {
        if (frames_.empty()) return 0;
        size_t n = frames_.size();

        switch (mode_) {
            case AnimationLoopMode::Loop: {
                int64_t rem = tick % static_cast<int64_t>(n);
                if (rem < 0) rem += static_cast<int64_t>(n);
                return static_cast<size_t>(rem);
            }
            case AnimationLoopMode::PingPong: {
                if (n == 1) return 0;
                int64_t cycle = static_cast<int64_t>((n - 1) * 2);
                int64_t pos = tick % cycle;
                if (pos < 0) pos += cycle;
                if (pos < static_cast<int64_t>(n)) {
                    return static_cast<size_t>(pos);
                } else {
                    return static_cast<size_t>(cycle - pos);
                }
            }
            case AnimationLoopMode::Clamp: {
                if (tick <= 0) return 0;
                if (tick >= static_cast<int64_t>(n)) return n - 1;
                return static_cast<size_t>(tick);
            }
        }
        return 0;
    }

    /// @brief Fast Zero-Cost Blit of the pre-rendered frame at the current tick.
    bool render_frame(Canvas& target, float x, float y, int64_t tick) const {
        if (!is_baked_ || frames_.empty()) return false;
        size_t idx = tick_to_frame_index(tick);
        const auto& src = frames_[idx];

        // Fast Path: Direct Scanline memcpy when canvas transform is identity
        if (target.transform().is_identity()) {
            const auto& clip_opt = target.scissor_clip();
            if (!clip_opt.has_value()) return false;
            const auto& clip = *clip_opt;

            int32_t dst_x = static_cast<int32_t>(x);
            int32_t dst_y = static_cast<int32_t>(y);

            int32_t x0 = std::max(dst_x, static_cast<int32_t>(clip.x()));
            int32_t y0 = std::max(dst_y, static_cast<int32_t>(clip.y()));
            int32_t x1 = std::min(dst_x + static_cast<int32_t>(width_), static_cast<int32_t>(clip.right()));
            int32_t y1 = std::min(dst_y + static_cast<int32_t>(height_), static_cast<int32_t>(clip.bottom()));

            if (x1 <= x0 || y1 <= y0) return true;

            uint32_t copy_width = static_cast<uint32_t>(x1 - x0);
            size_t copy_bytes = copy_width * sizeof(PremultipliedColorU8);

            auto src_ref = src.as_ref();
            auto& dst = target.pixmap();
            for (int32_t cy = y0; cy < y1; ++cy) {
                int32_t sy = cy - dst_y;
                int32_t sx = x0 - dst_x;
                const auto* src_row = src_ref.row(static_cast<size_t>(sy)) + sx;
                auto* dst_row = dst.row(static_cast<size_t>(cy)) + x0;
                std::memcpy(dst_row, src_row, copy_bytes);
            }
            return true;
        }

        target.draw_pixmap(static_cast<int32_t>(x), static_cast<int32_t>(y), src.as_ref());
        return true;
    }

    [[nodiscard]] const Pixmap* get_frame(size_t frame_idx) const noexcept {
        if (frame_idx >= frames_.size()) return nullptr;
        return &frames_[frame_idx];
    }

    [[nodiscard]] bool is_baked() const noexcept { return is_baked_; }
    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] size_t frame_count() const noexcept { return frames_.size(); }
    [[nodiscard]] AnimationLoopMode mode() const noexcept { return mode_; }

    [[nodiscard]] Rect bounds(float x, float y) const noexcept {
        auto r = Rect::from_xywh(x, y, static_cast<float>(width_), static_cast<float>(height_));
        return r ? *r : Rect{};
    }

    /// @brief Total memory consumption in bytes (width * height * 4 * frames).
    [[nodiscard]] size_t memory_footprint() const noexcept {
        return frames_.size() * (static_cast<size_t>(width_) * height_ * 4);
    }

private:
    uint32_t width_{0};
    uint32_t height_{0};
    AnimationLoopMode mode_{AnimationLoopMode::Loop};
    bool is_baked_{false};
    std::vector<Pixmap> frames_;
};

} // namespace nisaba::damage

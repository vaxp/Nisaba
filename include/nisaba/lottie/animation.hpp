#pragma once

/// @file animation.hpp
/// @brief Root Lottie animation document, asset resolver, and multi-format renderer.

#include "nisaba/lottie/layer.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/math/rect.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>

namespace nisaba::lottie {

struct Marker {
    std::string name;
    float time = 0.0f;
    float duration = 0.0f;
};

class Animation {
public:
    Animation() = default;
    ~Animation() = default;

    /// @brief Parse a Lottie animation from a JSON string or memory buffer.
    [[nodiscard]] static std::shared_ptr<Animation> load_from_data(std::string_view json_content);

    /// @brief Load and parse a Lottie animation from a file on disk.
    [[nodiscard]] static std::shared_ptr<Animation> load_from_file(const std::string& filepath);

    /// @brief Render animation at a specific frame directly to canvas (1:1 scale).
    void render(ICanvas& canvas, float frame) const;

    /// @brief Render animation at a specific frame fitted into a destination rectangle.
    void render(ICanvas& canvas, float frame, const Rect& dest_rect, bool preserve_aspect_ratio = true) const;

    // Document properties
    [[nodiscard]] std::string_view version() const noexcept { return version_; }
    [[nodiscard]] std::string_view name() const noexcept { return name_; }
    [[nodiscard]] float width() const noexcept { return width_; }
    [[nodiscard]] float height() const noexcept { return height_; }
    [[nodiscard]] float in_point() const noexcept { return in_point_; }
    [[nodiscard]] float out_point() const noexcept { return out_point_; }
    [[nodiscard]] float frame_rate() const noexcept { return frame_rate_; }

    [[nodiscard]] float total_frames() const noexcept {
        return std::max(0.0f, out_point_ - in_point_);
    }

    [[nodiscard]] float duration_seconds() const noexcept {
        return (frame_rate_ > 0.0f) ? (total_frames() / frame_rate_) : 0.0f;
    }

    [[nodiscard]] const std::vector<std::shared_ptr<LottieLayer>>& layers() const noexcept {
        return layers_;
    }

    [[nodiscard]] const std::vector<Marker>& markers() const noexcept {
        return markers_;
    }

private:
    std::string version_;
    std::string name_;
    float width_ = 0.0f;
    float height_ = 0.0f;
    float in_point_ = 0.0f;
    float out_point_ = 0.0f;
    float frame_rate_ = 60.0f;

    std::vector<std::shared_ptr<LottieLayer>> layers_;
    std::unordered_map<std::string, std::vector<std::shared_ptr<LottieLayer>>> precomp_assets_;
    std::vector<Marker> markers_;
};

} // namespace nisaba::lottie

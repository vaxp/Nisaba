#pragma once

/// @file animation.hpp
/// @brief Root Lottie & dotLottie (.lottie ZIP) animation document, asset resolver, and dynamic overrides.

#include "nisaba/lottie/layer.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/text/font_system.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <unordered_map>
#include <optional>
#include <span>

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

    /// @brief Parse a Lottie animation from a JSON string or binary .lottie (ZIP) memory buffer.
    [[nodiscard]] static std::shared_ptr<Animation> load_from_data(std::string_view content, std::string_view base_dir = "");

    /// @brief Parse a Lottie or .lottie animation from raw binary bytes.
    [[nodiscard]] static std::shared_ptr<Animation> load_from_bytes(std::span<const uint8_t> bytes, std::string_view base_dir = "");

    /// @brief Load and parse a Lottie (.json) or dotLottie (.lottie) animation from disk.
    [[nodiscard]] static std::shared_ptr<Animation> load_from_file(const std::string& filepath);

    /// @brief Render animation at a specific frame directly to canvas (1:1 scale).
    void render(ICanvas& canvas, float frame) const;

    /// @brief Render animation at a specific frame fitted into a destination rectangle.
    void render(ICanvas& canvas, float frame, const Rect& dest_rect, bool preserve_aspect_ratio = true) const;

    /// @brief Attach a FontSystem for TrueType/OpenType text shaping in TextLayers.
    void set_font_system(std::shared_ptr<text::FontSystem> fs);
    [[nodiscard]] std::shared_ptr<text::FontSystem> font_system() const noexcept { return assets_.font_system; }

    // ── Dynamic Property Overrides & Slots ────────────────────────────────

    /// @brief Override fill/stroke/solid color by keypath ("LayerName.GroupName.FillName" or "*").
    bool set_color_override(std::string_view keypath, Color color);

    /// @brief Override opacity [0..100], stroke width, or rotation by keypath.
    bool set_scalar_override(std::string_view keypath, float value);

    /// @brief Override position or scale by keypath.
    bool set_point_override(std::string_view keypath, Point value);

    /// @brief Override text string on a TextLayer by layer name (or "*" for all text layers).
    bool set_text_override(std::string_view layer_name, std::string text);

    /// @brief Override a named Slot ID ("sid") color across the entire document.
    bool set_slot_color(std::string_view slot_id, Color color);

    /// @brief Clear all dynamic runtime overrides and restore original keyframe/static values.
    void clear_overrides();

    /// @brief Hit-test point (x, y) in composition coordinates at a given frame.
    /// @return Name of the topmost hit layer/shape, or std::nullopt if nothing hit.
    [[nodiscard]] std::optional<std::string> hit_test(float x, float y, float frame) const;

    /// @brief Find a named marker in the animation document.
    [[nodiscard]] const Marker* find_marker(std::string_view marker_name) const noexcept;

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

    [[nodiscard]] const LottieAssetRegistry& assets() const noexcept {
        return assets_;
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
    LottieAssetRegistry assets_;
    std::vector<Marker> markers_;
};

} // namespace nisaba::lottie

#pragma once

/// @file baked_text.hpp
/// @brief Sovereign Pre-Rendered Vector Text Surface (BakedText) for Nisaba.
///
/// Pre-shapes, layouts, and rasterizes text into an offscreen ARGB surface for
/// instantaneous O(1) SIMD block blitting, bypassing per-frame shaping and glyph loops.

#include <memory>
#include <string_view>
#include <optional>
#include <cmath>

#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/text/attrs.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba {
class Canvas;
}

namespace nisaba::text {

class Buffer;

/// Pre-rasterized text block ready for instantaneous direct SIMD composite.
class BakedText {
public:
    BakedText() = default;

    BakedText(
        std::shared_ptr<Pixmap> pixmap,
        float width,
        float height,
        int32_t origin_x = 0,
        int32_t origin_y = 0,
        float baseline_y = 0.0f
    ) noexcept
        : pixmap_(std::move(pixmap)),
          width_(width),
          height_(height),
          origin_x_(origin_x),
          origin_y_(origin_y),
          baseline_y_(baseline_y) {}

    [[nodiscard]] bool is_valid() const noexcept {
        return pixmap_ != nullptr && pixmap_->width() > 0 && pixmap_->height() > 0;
    }

    [[nodiscard]] explicit operator bool() const noexcept {
        return is_valid();
    }

    [[nodiscard]] float width() const noexcept { return width_; }
    [[nodiscard]] float height() const noexcept { return height_; }
    [[nodiscard]] int32_t origin_x() const noexcept { return origin_x_; }
    [[nodiscard]] int32_t origin_y() const noexcept { return origin_y_; }
    [[nodiscard]] float baseline_y() const noexcept { return baseline_y_; }

    [[nodiscard]] const Pixmap* pixmap() const noexcept { return pixmap_.get(); }
    [[nodiscard]] std::shared_ptr<Pixmap> pixmap_ptr() const noexcept { return pixmap_; }
    [[nodiscard]] PixmapRef as_ref() const noexcept {
        return pixmap_ ? pixmap_->as_ref() : PixmapRef();
    }

    /// Factory: Bakes a text string with specified font configuration into a pre-rendered surface.
    static BakedText bake(
        FontSystem& font_system,
        GlyphCache& cache,
        std::string_view text,
        float font_size,
        Color color = Color::WHITE,
        Weight weight = Weight::Normal,
        bool monospace = false,
        float line_height_factor = 1.3f
    );

    /// Factory: Bakes a formatted Buffer into a pre-rendered surface.
    static BakedText from_buffer(
        Buffer& buffer,
        FontSystem& font_system,
        GlyphCache& cache,
        Color default_color = Color::WHITE
    );

private:
    std::shared_ptr<Pixmap> pixmap_{nullptr};
    float width_{0.0f};
    float height_{0.0f};
    int32_t origin_x_{0};
    int32_t origin_y_{0};
    float baseline_y_{0.0f};
};

} // namespace nisaba::text

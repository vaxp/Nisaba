#pragma once

#include <memory>
#include <optional>
#include <cstdint>
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/math/rect.hpp"

namespace nisaba {
    class Canvas;
}

namespace nisaba::svg {

class SvgDocument;

/// @brief Pre-rasterized in-memory representation of an SVG document or icon.
/// Cached at a specific resolution with optional tinting for O(1) blitting.
class BakedSvg {
public:
    BakedSvg() = default;

    /// @brief Bakes an SVG document into an anti-aliased RGBA8 pixel surface.
    static BakedSvg bake(
        const SvgDocument& doc,
        uint32_t width,
        uint32_t height,
        std::optional<Color> tint = std::nullopt
    );

    /// @brief Draws the baked SVG onto the target Canvas at (x, y) with optional opacity.
    void draw(Canvas& canvas, float x, float y, float opacity = 1.0f) const;

    /// @brief Draws the baked SVG scaled to fit dest_bounds with optional opacity.
    void draw(Canvas& canvas, const Rect& dest_bounds, float opacity = 1.0f) const;

    [[nodiscard]] bool is_valid() const noexcept { return pixmap_.has_value() && width_ > 0 && height_ > 0; }
    [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] const Pixmap& pixmap() const noexcept { return *pixmap_; }
    [[nodiscard]] std::optional<Color> tint() const noexcept { return tint_; }

private:
    std::optional<Pixmap> pixmap_{std::nullopt};
    uint32_t width_{0};
    uint32_t height_{0};
    std::optional<Color> tint_{std::nullopt};
};

} // namespace nisaba::svg

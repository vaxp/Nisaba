#pragma once

#include "nisaba/math/rect.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/border_radius.hpp"
#include <optional>

namespace nisaba {

class Path;

/// Represents a rounded rectangle with four independent corner radii.
class RRect {
public:
    Rect rect{};
    BorderRadius radii{};

    constexpr RRect() noexcept = default;
    constexpr RRect(const Rect& r, const BorderRadius& rad) noexcept
        : rect(r), radii(rad) {}

    /// Creates an RRect with uniform corner radii rx, ry (or rx, rx if ry is negative or omitted).
    static RRect from_rect_and_radius(const Rect& r, float rx, float ry = -1.0f) noexcept {
        float r_eff = (ry >= 0.0f) ? std::min(rx, ry) : rx;
        return RRect(r, BorderRadius::all(r_eff));
    }

    /// Creates an RRect from a Rect and a BorderRadius.
    static RRect from_rect_and_radii(const Rect& r, const BorderRadius& rad) noexcept {
        return RRect(r, rad);
    }

    /// Creates an RRect from xywh bounds and a BorderRadius.
    static std::optional<RRect> from_xywh(float x, float y, float w, float h, const BorderRadius& rad) noexcept {
        auto r = Rect::from_xywh(x, y, w, h);
        if (!r) return std::nullopt;
        return RRect(*r, rad);
    }

    /// Creates an RRect from ltrb coordinates and a BorderRadius.
    static std::optional<RRect> from_ltrb(float l, float t, float r, float b, const BorderRadius& rad) noexcept {
        auto rc = Rect::from_ltrb(l, t, r, b);
        if (!rc) return std::nullopt;
        return RRect(*rc, rad);
    }

    constexpr float x() const noexcept { return rect.x(); }
    constexpr float y() const noexcept { return rect.y(); }
    constexpr float width() const noexcept { return rect.width(); }
    constexpr float height() const noexcept { return rect.height(); }
    constexpr float left() const noexcept { return rect.left(); }
    constexpr float top() const noexcept { return rect.top(); }
    constexpr float right() const noexcept { return rect.right(); }
    constexpr float bottom() const noexcept { return rect.bottom(); }

    constexpr bool is_empty() const noexcept { return rect.is_empty(); }

    /// Returns a copy of this RRect with corner radii constrained according to W3C rules.
    RRect constrained() const noexcept {
        return RRect(rect, radii.constrained_for_size(rect.width(), rect.height()));
    }

    /// Checks if a point is contained inside this rounded rectangle.
    bool contains(Point p) const noexcept;
    bool contains(float px, float py) const noexcept {
        return contains(Point::from_xy(px, py));
    }

    /// Deflates the RRect by dx horizontally and dy vertically, scaling corner radii proportionally.
    RRect deflate(float dx, float dy) const noexcept;

    /// Insets each edge independently.
    RRect inset(float d_left, float d_top, float d_right, float d_bottom) const noexcept;

    /// Converts this rounded rectangle to an analytical vector Path.
    Path to_path() const;

    constexpr bool operator==(const RRect& o) const noexcept {
        return rect == o.rect && radii == o.radii;
    }
    constexpr bool operator!=(const RRect& o) const noexcept {
        return !(*this == o);
    }
};

} // namespace nisaba

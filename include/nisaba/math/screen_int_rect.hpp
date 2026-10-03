#pragma once

#include <cstdint>
#include <optional>
#include <limits>
#include "nisaba/types.hpp"
#include "nisaba/math/size.hpp"
#include "nisaba/math/rect.hpp"

namespace nisaba {

/// A screen `IntRect`.
///
/// Guarantees:
/// - X and Y are in 0..=INT32_MAX range.
/// - Width and height are in 1..=INT32_MAX range.
/// - x + width and y + height do not overflow.
class ScreenIntRect {
public:
    constexpr ScreenIntRect() noexcept
        : x_(0), y_(0), width_(LengthU32::create_unchecked(1)), height_(LengthU32::create_unchecked(1)) {}

    static constexpr std::optional<ScreenIntRect> from_xywh(uint32_t x, uint32_t y, uint32_t width, uint32_t height) noexcept {
        if (x > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) ||
            y > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) ||
            width > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) ||
            height > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
            return std::nullopt;
        }

        if (static_cast<uint64_t>(x) + width > std::numeric_limits<uint32_t>::max()) {
            return std::nullopt;
        }
        if (static_cast<uint64_t>(y) + height > std::numeric_limits<uint32_t>::max()) {
            return std::nullopt;
        }

        auto w = LengthU32::create(width);
        auto h = LengthU32::create(height);
        if (!w || !h) {
            return std::nullopt;
        }

        return ScreenIntRect(x, y, *w, *h);
    }

    static constexpr ScreenIntRect from_xywh_safe(uint32_t x, uint32_t y, LengthU32 width, LengthU32 height) noexcept {
        return ScreenIntRect(x, y, width, height);
    }

    constexpr uint32_t x() const noexcept { return x_; }
    constexpr uint32_t y() const noexcept { return y_; }
    constexpr uint32_t width() const noexcept { return width_.get(); }
    constexpr uint32_t height() const noexcept { return height_.get(); }
    constexpr LengthU32 width_safe() const noexcept { return width_; }
    constexpr LengthU32 height_safe() const noexcept { return height_; }

    constexpr uint32_t left() const noexcept { return x_; }
    constexpr uint32_t top() const noexcept { return y_; }
    constexpr uint32_t right() const noexcept { return x_ + width_.get(); }
    constexpr uint32_t bottom() const noexcept { return y_ + height_.get(); }

    IntSize size() const noexcept {
        return IntSize::from_wh_safe(width_, height_);
    }

    constexpr bool contains(const ScreenIntRect& other) const noexcept {
        return x_ <= other.x_ &&
               y_ <= other.y_ &&
               right() >= other.right() &&
               bottom() >= other.bottom();
    }

    IntRect to_int_rect() const noexcept {
        return *IntRect::from_xywh(
            static_cast<int32_t>(x_),
            static_cast<int32_t>(y_),
            width_.get(),
            height_.get()
        );
    }

    Rect to_rect() const noexcept {
        return *Rect::from_ltrb(
            static_cast<float>(x_),
            static_cast<float>(y_),
            static_cast<float>(x_ + width_.get()),
            static_cast<float>(y_ + height_.get())
        );
    }

    constexpr std::optional<ScreenIntRect> intersect(const ScreenIntRect& other) const noexcept {
        uint32_t nx0 = std::max(x_, other.x_);
        uint32_t ny0 = std::max(y_, other.y_);
        uint32_t nx1 = std::min(right(), other.right());
        uint32_t ny1 = std::min(bottom(), other.bottom());

        if (nx1 <= nx0 || ny1 <= ny0) {
            return std::nullopt;
        }

        return ScreenIntRect::from_xywh(nx0, ny0, nx1 - nx0, ny1 - ny0);
    }

    constexpr bool operator==(const ScreenIntRect& other) const noexcept = default;

private:
    constexpr ScreenIntRect(uint32_t x, uint32_t y, LengthU32 width, LengthU32 height) noexcept
        : x_(x), y_(y), width_(width), height_(height) {}

    uint32_t x_;
    uint32_t y_;
    LengthU32 width_;
    LengthU32 height_;
};

inline ScreenIntRect to_screen_int_rect(IntSize size, uint32_t x, uint32_t y) noexcept {
    return *ScreenIntRect::from_xywh(x, y, size.width(), size.height());
}

inline std::optional<ScreenIntRect> to_screen_int_rect(IntRect rect) noexcept {
    if (rect.x() < 0 || rect.y() < 0) {
        return std::nullopt;
    }
    return ScreenIntRect::from_xywh(
        static_cast<uint32_t>(rect.x()),
        static_cast<uint32_t>(rect.y()),
        rect.width(),
        rect.height()
    );
}

} // namespace nisaba

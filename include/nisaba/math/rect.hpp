#pragma once

#include <cstdint>
#include <cmath>
#include <optional>
#include <algorithm>
#include "nisaba/types.hpp"
#include "nisaba/math/scalar.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/size.hpp"
#include "nisaba/math/f32x4.hpp"

namespace nisaba {

class Rect;
class NonZeroRect;
struct Transform;

inline std::optional<float> checked_f32_sub(float a, float b) noexcept {
    double n = static_cast<double>(a) - static_cast<double>(b);
    if (n > -3.402823466e+38 && n < 3.402823466e+38) {
        return static_cast<float>(n);
    }
    return std::nullopt;
}

/// An integer rectangle.
class IntRect {
public:
    constexpr IntRect()
        : x_(0), y_(0), width_(LengthU32::create_unchecked(1)), height_(LengthU32::create_unchecked(1)) {}

    static std::optional<IntRect> from_xywh(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept {
        if (width == 0 || height == 0 || width > static_cast<uint32_t>(std::numeric_limits<int32_t>::max()) ||
            height > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
            return std::nullopt;
        }
        int64_t right = static_cast<int64_t>(x) + static_cast<int64_t>(width);
        int64_t bottom = static_cast<int64_t>(y) + static_cast<int64_t>(height);
        if (right > std::numeric_limits<int32_t>::max() || right < std::numeric_limits<int32_t>::min() ||
            bottom > std::numeric_limits<int32_t>::max() || bottom < std::numeric_limits<int32_t>::min()) {
            return std::nullopt;
        }
        return IntRect(x, y, *LengthU32::create(width), *LengthU32::create(height));
    }

    static std::optional<IntRect> from_ltrb(int32_t left, int32_t top, int32_t right, int32_t bottom) noexcept {
        int64_t w = static_cast<int64_t>(right) - static_cast<int64_t>(left);
        int64_t h = static_cast<int64_t>(bottom) - static_cast<int64_t>(top);
        if (w <= 0 || h <= 0 || w > std::numeric_limits<int32_t>::max() || h > std::numeric_limits<int32_t>::max()) {
            return std::nullopt;
        }
        return from_xywh(left, top, static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    }

    constexpr int32_t x() const noexcept { return x_; }
    constexpr int32_t y() const noexcept { return y_; }
    constexpr uint32_t width() const noexcept { return width_.get(); }
    constexpr uint32_t height() const noexcept { return height_.get(); }

    constexpr int32_t left() const noexcept { return x_; }
    constexpr int32_t top() const noexcept { return y_; }
    constexpr int32_t right() const noexcept { return x_ + static_cast<int32_t>(width_.get()); }
    constexpr int32_t bottom() const noexcept { return y_ + static_cast<int32_t>(height_.get()); }

    constexpr IntSize size() const noexcept {
        return IntSize::from_wh_safe(width_, height_);
    }

    constexpr bool contains(const IntRect& other) const noexcept {
        return x_ <= other.x_ && y_ <= other.y_ && right() >= other.right() && bottom() >= other.bottom();
    }

    std::optional<IntRect> intersect(const IntRect& other) const noexcept {
        int32_t l = std::max(x_, other.x_);
        int32_t t = std::max(y_, other.y_);
        int32_t r = std::min(right(), other.right());
        int32_t b = std::min(bottom(), other.bottom());
        return from_ltrb(l, t, r, b);
    }

    std::optional<IntRect> inset(int32_t dx, int32_t dy) const noexcept {
        return from_ltrb(left() + dx, top() + dy, right() - dx, bottom() - dy);
    }

    std::optional<IntRect> make_outset(int32_t dx, int32_t dy) const noexcept {
        int64_t l = static_cast<int64_t>(left()) - dx;
        int64_t t = static_cast<int64_t>(top()) - dy;
        int64_t r = static_cast<int64_t>(right()) + dx;
        int64_t b = static_cast<int64_t>(bottom()) + dy;
        if (l < std::numeric_limits<int32_t>::min() || t < std::numeric_limits<int32_t>::min() ||
            r > std::numeric_limits<int32_t>::max() || b > std::numeric_limits<int32_t>::max()) {
            return std::nullopt;
        }
        return from_ltrb(static_cast<int32_t>(l), static_cast<int32_t>(t), static_cast<int32_t>(r), static_cast<int32_t>(b));
    }

    std::optional<IntRect> translate(int32_t tx, int32_t ty) const noexcept {
        return from_xywh(x_ + tx, y_ + ty, width(), height());
    }

    std::optional<IntRect> translate_to(int32_t nx, int32_t ny) const noexcept {
        return from_xywh(nx, ny, width(), height());
    }

    Rect to_rect() const noexcept;

    constexpr bool operator==(const IntRect& o) const noexcept {
        return x_ == o.x_ && y_ == o.y_ && width_ == o.width_ && height_ == o.height_;
    }

    constexpr bool operator!=(const IntRect& o) const noexcept {
        return !(*this == o);
    }

private:
    constexpr IntRect(int32_t x, int32_t y, LengthU32 w, LengthU32 h) noexcept
        : x_(x), y_(y), width_(w), height_(h) {}

    int32_t x_{0};
    int32_t y_{0};
    LengthU32 width_;
    LengthU32 height_;
};

/// A floating-point rectangle.
class Rect {
public:
    constexpr Rect() noexcept = default;

    static std::optional<Rect> from_ltrb(float left, float top, float right, float bottom) noexcept {
        auto l = FiniteF32::create(left);
        auto t = FiniteF32::create(top);
        auto r = FiniteF32::create(right);
        auto b = FiniteF32::create(bottom);
        if (!l || !t || !r || !b) {
            return std::nullopt;
        }
        if (left <= right && top <= bottom) {
            if (!checked_f32_sub(right, left) || !checked_f32_sub(bottom, top)) {
                return std::nullopt;
            }
            return Rect(*l, *t, *r, *b);
        }
        return std::nullopt;
    }

    static std::optional<Rect> from_xywh(float x, float y, float w, float h) noexcept {
        return from_ltrb(x, y, x + w, y + h);
    }

    constexpr float left() const noexcept { return left_.get(); }
    constexpr float top() const noexcept { return top_.get(); }
    constexpr float right() const noexcept { return right_.get(); }
    constexpr float bottom() const noexcept { return bottom_.get(); }

    constexpr float x() const noexcept { return left_.get(); }
    constexpr float y() const noexcept { return top_.get(); }
    constexpr float width() const noexcept { return right_.get() - left_.get(); }
    constexpr float height() const noexcept { return bottom_.get() - top_.get(); }

    constexpr bool is_empty() const noexcept {
        return left_ == right_ || top_ == bottom_;
    }

    std::optional<IntRect> round() const noexcept {
        int32_t l = scalar::saturate_round(left());
        int32_t t = scalar::saturate_round(top());
        int32_t r = scalar::saturate_round(right());
        int32_t b = scalar::saturate_round(bottom());
        uint32_t w = std::max(1, r - l);
        uint32_t h = std::max(1, b - t);
        return IntRect::from_xywh(l, t, w, h);
    }

    std::optional<IntRect> round_out() const noexcept {
        int32_t l = scalar::saturate_floor(left());
        int32_t t = scalar::saturate_floor(top());
        int32_t r = scalar::saturate_ceil(right());
        int32_t b = scalar::saturate_ceil(bottom());
        uint32_t w = std::max(1, r - l);
        uint32_t h = std::max(1, b - t);
        return IntRect::from_xywh(l, t, w, h);
    }

    std::optional<Rect> intersect(const Rect& other) const noexcept {
        float l = std::max(x(), other.x());
        float t = std::max(y(), other.y());
        float r = std::min(right(), other.right());
        float b = std::min(bottom(), other.bottom());
        return from_ltrb(l, t, r, b);
    }

    std::optional<Rect> join(const Rect& other) const noexcept {
        if (other.is_empty()) return *this;
        if (is_empty()) return other;
        float l = std::min(x(), other.x());
        float t = std::min(y(), other.y());
        float r = std::max(right(), other.right());
        float b = std::max(bottom(), other.bottom());
        return from_ltrb(l, t, r, b);
    }

    static std::optional<Rect> from_points(const Point* points, size_t count) noexcept {
        if (count == 0 || !points) return std::nullopt;
        if (count == 1) {
            return Rect::from_xywh(points[0].x, points[0].y, 0.0f, 0.0f);
        }
        if (count == 2) {
            float x0 = points[0].x;
            float y0 = points[0].y;
            float x1 = points[1].x;
            float y1 = points[1].y;
            float l = (x0 < x1) ? x0 : x1;
            float r = (x0 < x1) ? x1 : x0;
            float t = (y0 < y1) ? y0 : y1;
            float b = (y0 < y1) ? y1 : y0;
            return Rect::from_ltrb(l, t, r, b);
        }

        size_t offset = 0;
        f32x4 min_v, max_v;
        if (count & 1) {
            Point pt = points[0];
            min_v = f32x4(pt.x, pt.y, pt.x, pt.y);
            max_v = min_v;
            offset = 1;
        } else {
            Point pt0 = points[0];
            Point pt1 = points[1];
            min_v = f32x4(pt0.x, pt0.y, pt1.x, pt1.y);
            max_v = min_v;
            offset = 2;
        }

        f32x4 accum;
        while (offset != count) {
            Point pt0 = points[offset + 0];
            Point pt1 = points[offset + 1];
            f32x4 xy(pt0.x, pt0.y, pt1.x, pt1.y);
            accum *= xy;
            min_v = min_v.min(xy);
            max_v = max_v.max(xy);
            offset += 2;
        }

        bool all_finite = (accum * f32x4() == f32x4());
        if (all_finite) {
            return Rect::from_ltrb(
                std::min(min_v.v[0], min_v.v[2]),
                std::min(min_v.v[1], min_v.v[3]),
                std::max(max_v.v[0], max_v.v[2]),
                std::max(max_v.v[1], max_v.v[3])
            );
        }
        return std::nullopt;
    }

    std::optional<Rect> inset(float dx, float dy) const noexcept {
        return from_ltrb(left() + dx, top() + dy, right() - dx, bottom() - dy);
    }

    std::optional<Rect> outset(float dx, float dy) const noexcept {
        return inset(-dx, -dy);
    }

    std::optional<Rect> transform(const Transform& ts) const noexcept;

    Rect bbox_transform(const NonZeroRect& bbox) const noexcept;

    std::optional<NonZeroRect> to_non_zero_rect() const noexcept;

    constexpr bool operator==(const Rect& o) const noexcept {
        return left_ == o.left_ && top_ == o.top_ && right_ == o.right_ && bottom_ == o.bottom_;
    }

    constexpr bool operator!=(const Rect& o) const noexcept {
        return !(*this == o);
    }

private:
    constexpr Rect(FiniteF32 l, FiniteF32 t, FiniteF32 r, FiniteF32 b) noexcept
        : left_(l), top_(t), right_(r), bottom_(b) {}

    FiniteF32 left_;
    FiniteF32 top_;
    FiniteF32 right_;
    FiniteF32 bottom_;

    friend class NonZeroRect;
};

/// A rectangle guaranteed to have width > 0.0 and height > 0.0.
class NonZeroRect {
public:
    static std::optional<NonZeroRect> from_ltrb(float left, float top, float right, float bottom) noexcept {
        auto l = FiniteF32::create(left);
        auto t = FiniteF32::create(top);
        auto r = FiniteF32::create(right);
        auto b = FiniteF32::create(bottom);
        if (!l || !t || !r || !b) return std::nullopt;

        if (left < right && top < bottom) {
            if (!checked_f32_sub(right, left) || !checked_f32_sub(bottom, top)) {
                return std::nullopt;
            }
            return NonZeroRect(*l, *t, *r, *b);
        }
        return std::nullopt;
    }

    static std::optional<NonZeroRect> from_xywh(float x, float y, float w, float h) noexcept {
        return from_ltrb(x, y, x + w, y + h);
    }

    constexpr float left() const noexcept { return left_.get(); }
    constexpr float top() const noexcept { return top_.get(); }
    constexpr float right() const noexcept { return right_.get(); }
    constexpr float bottom() const noexcept { return bottom_.get(); }

    constexpr float x() const noexcept { return left_.get(); }
    constexpr float y() const noexcept { return top_.get(); }
    constexpr float width() const noexcept { return right_.get() - left_.get(); }
    constexpr float height() const noexcept { return bottom_.get() - top_.get(); }

    Size size() const noexcept {
        return *Size::from_wh(width(), height());
    }

    std::optional<NonZeroRect> translate_to(float nx, float ny) const noexcept {
        return from_xywh(nx, ny, width(), height());
    }

    std::optional<NonZeroRect> transform(const Transform& ts) const noexcept;

    NonZeroRect bbox_transform(const NonZeroRect& bbox) const noexcept {
        float nx = x() * bbox.width() + bbox.x();
        float ny = y() * bbox.height() + bbox.y();
        float nw = width() * bbox.width();
        float nh = height() * bbox.height();
        return *from_xywh(nx, ny, nw, nh);
    }

    Rect to_rect() const noexcept {
        return *Rect::from_xywh(x(), y(), width(), height());
    }

    IntRect to_int_rect() const noexcept {
        return *IntRect::from_xywh(
            static_cast<int32_t>(std::floor(x())),
            static_cast<int32_t>(std::floor(y())),
            std::max(1u, static_cast<uint32_t>(std::ceil(width()))),
            std::max(1u, static_cast<uint32_t>(std::ceil(height())))
        );
    }

    constexpr bool operator==(const NonZeroRect& o) const noexcept {
        return left_ == o.left_ && top_ == o.top_ && right_ == o.right_ && bottom_ == o.bottom_;
    }

    constexpr bool operator!=(const NonZeroRect& o) const noexcept {
        return !(*this == o);
    }

private:
    constexpr NonZeroRect(FiniteF32 l, FiniteF32 t, FiniteF32 r, FiniteF32 b) noexcept
        : left_(l), top_(t), right_(r), bottom_(b) {}

    FiniteF32 left_;
    FiniteF32 top_;
    FiniteF32 right_;
    FiniteF32 bottom_;

    friend class Rect;
};

inline Rect IntRect::to_rect() const noexcept {
    return *Rect::from_ltrb(
        static_cast<float>(x_),
        static_cast<float>(y_),
        static_cast<float>(right()),
        static_cast<float>(bottom())
    );
}

inline Rect Rect::bbox_transform(const NonZeroRect& bbox) const noexcept {
    float nx = x() * bbox.width() + bbox.x();
    float ny = y() * bbox.height() + bbox.y();
    float nw = width() * bbox.width();
    float nh = height() * bbox.height();
    return *Rect::from_xywh(nx, ny, nw, nh);
}

inline std::optional<NonZeroRect> Rect::to_non_zero_rect() const noexcept {
    return NonZeroRect::from_xywh(x(), y(), width(), height());
}

} // namespace nisaba

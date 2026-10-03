#pragma once

/// @file tween.hpp
/// @brief Tween<T> — interpolates between two typed values along a Curve.
/// Specialized for Nisaba geometric and color primitives.

#include "nisaba/animation/curves.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/size.hpp"
#include "nisaba/math/transform.hpp"

#include <cmath>
#include <algorithm>

namespace nisaba::animation {

// ════════════════════════════════════════════════════════════════
// Tween<T> — typed value interpolator
// ════════════════════════════════════════════════════════════════

/// @brief Interpolates between `begin` and `end` using an easing Curve.
template<typename T>
class Tween {
public:
    T begin{};
    T end{};
    const Curve* curve = &Curves::linear;

    constexpr Tween() noexcept = default;
    constexpr Tween(T begin_val, T end_val, const Curve* curve_ptr = &Curves::linear) noexcept
        : begin(begin_val), end(end_val), curve(curve_ptr) {}

    /// Evaluate the interpolated value at progress t in [0, 1].
    [[nodiscard]] T evaluate(double t) const {
        double curved_t = curve ? curve->evaluate(t) : t;
        return lerp(begin, end, curved_t);
    }

    /// Convenience float version.
    [[nodiscard]] T evaluate_f(float t) const {
        return evaluate(static_cast<double>(t));
    }

    [[nodiscard]] T evaluateF(float t) const {
        return evaluate_f(t);
    }

private:
    // Default lerp for standard numeric types
    static T lerp(const T& a, const T& b, double t) noexcept {
        return static_cast<T>(a + (b - a) * t);
    }
};

// ── Color specialization ────────────────────────────────────────

template<>
inline Color Tween<Color>::lerp(const Color& a, const Color& b, double t) noexcept {
    float tf = static_cast<float>(t);
    float r = std::clamp(a.red() + (b.red() - a.red()) * tf, 0.0f, 1.0f);
    float g = std::clamp(a.green() + (b.green() - a.green()) * tf, 0.0f, 1.0f);
    float bl = std::clamp(a.blue() + (b.blue() - a.blue()) * tf, 0.0f, 1.0f);
    float al = std::clamp(a.alpha() + (b.alpha() - a.alpha()) * tf, 0.0f, 1.0f);
    return Color::from_rgba_unchecked(r, g, bl, al);
}

// ── Point specialization ────────────────────────────────────────

template<>
inline Point Tween<Point>::lerp(const Point& a, const Point& b, double t) noexcept {
    float tf = static_cast<float>(t);
    return Point(
        a.x + (b.x - a.x) * tf,
        a.y + (b.y - a.y) * tf
    );
}

// ── Rect specialization ─────────────────────────────────────────

template<>
inline Rect Tween<Rect>::lerp(const Rect& a, const Rect& b, double t) noexcept {
    float tf = static_cast<float>(t);
    float x = a.x() + (b.x() - a.x()) * tf;
    float y = a.y() + (b.y() - a.y()) * tf;
    float w = std::max(0.0f, a.width() + (b.width() - a.width()) * tf);
    float h = std::max(0.0f, a.height() + (b.height() - a.height()) * tf);
    auto res = Rect::from_xywh(x, y, w, h);
    return res ? *res : a;
}

// ── Size specialization ─────────────────────────────────────────

template<>
inline Size Tween<Size>::lerp(const Size& a, const Size& b, double t) noexcept {
    float tf = static_cast<float>(t);
    float w = std::max(0.001f, a.width() + (b.width() - a.width()) * tf);
    float h = std::max(0.001f, a.height() + (b.height() - a.height()) * tf);
    auto res = Size::from_wh(w, h);
    return res ? *res : a;
}

// ── Transform specialization ────────────────────────────────────

template<>
inline Transform Tween<Transform>::lerp(const Transform& a, const Transform& b, double t) noexcept {
    float tf = static_cast<float>(t);
    return Transform::from_row(
        a.sx + (b.sx - a.sx) * tf,
        a.ky + (b.ky - a.ky) * tf,
        a.kx + (b.kx - a.kx) * tf,
        a.sy + (b.sy - a.sy) * tf,
        a.tx + (b.tx - a.tx) * tf,
        a.ty + (b.ty - a.ty) * tf
    );
}

// ════════════════════════════════════════════════════════════════
// Convenient aliases
// ════════════════════════════════════════════════════════════════

using FloatTween     = Tween<float>;
using DoubleTween    = Tween<double>;
using ColorTween     = Tween<Color>;
using PointTween     = Tween<Point>;
using RectTween      = Tween<Rect>;
using SizeTween      = Tween<Size>;
using TransformTween = Tween<Transform>;

} // namespace nisaba::animation

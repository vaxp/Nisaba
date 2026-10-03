#pragma once
/// @file types.hpp
/// @brief Geometric types and insets for nisaba::backend_os.

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <limits>
#include <algorithm>

namespace nisaba::backend_os {

/// A 2D point with float coordinates.
struct Point {
    float x = 0.0f;
    float y = 0.0f;

    constexpr Point() = default;
    constexpr Point(float x, float y) : x(x), y(y) {}

    constexpr Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
    constexpr Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
    constexpr Point operator*(float s) const { return {x * s, y * s}; }
    constexpr bool  operator==(const Point&) const = default;

    [[nodiscard]] float distance(const Point& o) const {
        float dx = x - o.x, dy = y - o.y;
        return std::sqrt(dx * dx + dy * dy);
    }
};

/// A 2D size (width x height).
struct Size {
    float width  = 0.0f;
    float height = 0.0f;

    constexpr Size() = default;
    constexpr Size(float w, float h) : width(w), height(h) {}
    constexpr bool operator==(const Size&) const = default;

    [[nodiscard]] constexpr bool isEmpty() const {
        return width <= 0.0f || height <= 0.0f;
    }
    [[nodiscard]] constexpr float area() const { return width * height; }

    static constexpr Size zero() { return {0, 0}; }
    static constexpr Size infinite() {
        return {std::numeric_limits<float>::infinity(),
                std::numeric_limits<float>::infinity()};
    }
};

/// A 2D axis-aligned rectangle.
struct Rect {
    float x = 0.0f;
    float y = 0.0f;
    float width  = 0.0f;
    float height = 0.0f;

    constexpr Rect() = default;
    constexpr Rect(float x, float y, float w, float h)
        : x(x), y(y), width(w), height(h) {}

    static constexpr Rect fromLTWH(float l, float t, float w, float h) {
        return {l, t, w, h};
    }
    static constexpr Rect fromLTRB(float l, float t, float r, float b) {
        return {l, t, r - l, b - t};
    }
    static constexpr Rect fromPointSize(Point origin, Size size) {
        return {origin.x, origin.y, size.width, size.height};
    }

    [[nodiscard]] constexpr float left()   const { return x; }
    [[nodiscard]] constexpr float top()    const { return y; }
    [[nodiscard]] constexpr float right()  const { return x + width; }
    [[nodiscard]] constexpr float bottom() const { return y + height; }
    [[nodiscard]] constexpr Point topLeft()  const { return {x, y}; }
    [[nodiscard]] constexpr Point center()   const { return {x + width / 2, y + height / 2}; }
    [[nodiscard]] constexpr Size  size()     const { return {width, height}; }

    [[nodiscard]] constexpr bool contains(Point p) const {
        return p.x >= x && p.x < x + width &&
               p.y >= y && p.y < y + height;
    }

    [[nodiscard]] constexpr bool intersects(const Rect& o) const {
        return !(o.x >= right() || o.right() <= x ||
                 o.y >= bottom() || o.bottom() <= y);
    }
};

/// Edge insets for windows, safe areas, and margins.
struct EdgeInsets {
    float top    = 0.0f;
    float right  = 0.0f;
    float bottom = 0.0f;
    float left   = 0.0f;
    float start  = 0.0f;
    float end    = 0.0f;

    constexpr EdgeInsets() = default;
    constexpr EdgeInsets(float top, float right, float bottom, float left)
        : top(top), right(right), bottom(bottom), left(left) {}
    constexpr EdgeInsets(float top, float right, float bottom, float left, float start, float end)
        : top(top), right(right), bottom(bottom), left(left), start(start), end(end) {}

    static constexpr EdgeInsets all(float v) { return {v, v, v, v}; }
    static constexpr EdgeInsets symmetric(float vertical, float horizontal) {
        return {vertical, horizontal, vertical, horizontal};
    }
    static constexpr EdgeInsets only(float t = 0, float r = 0, float b = 0, float l = 0) {
        return {t, r, b, l};
    }
    static constexpr EdgeInsets fromLTRB(float left, float top, float right, float bottom) {
        return {top, right, bottom, left};
    }
    constexpr bool operator==(const EdgeInsets&) const = default;
};

}  // namespace nisaba::backend_os

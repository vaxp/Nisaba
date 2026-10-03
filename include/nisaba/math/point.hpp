#pragma once

#include <cmath>
#include <utility>
#include "nisaba/math/scalar.hpp"
#include "nisaba/math/f32x2.hpp"

namespace nisaba {

struct Point {
    float x{0.0f};
    float y{0.0f};

    constexpr Point() noexcept = default;
    constexpr Point(float x_, float y_) noexcept : x(x_), y(y_) {}

    static constexpr Point from_xy(float x, float y) noexcept {
        return Point(x, y);
    }

    static constexpr Point from_f32x2(const f32x2& r) noexcept {
        return Point(r.x(), r.y());
    }

    constexpr f32x2 to_f32x2() const noexcept {
        return f32x2(x, y);
    }

    static constexpr Point zero() noexcept {
        return Point(0.0f, 0.0f);
    }

    constexpr bool is_zero() const noexcept {
        return x == 0.0f && y == 0.0f;
    }

    /// Returns true if both x and y are finite measurable values.
    bool is_finite() const noexcept {
        return std::isfinite(x * y);
    }

    bool can_normalize() const noexcept {
        return std::isfinite(x) && std::isfinite(y) && (x != 0.0f || y != 0.0f);
    }

    bool almost_equal(const Point& other) const noexcept {
        return !(*this - other).can_normalize();
    }

    bool equals_within_tolerance(const Point& other, float tolerance) const noexcept {
        return scalar::is_nearly_zero_within_tolerance(x - other.x, tolerance) &&
               scalar::is_nearly_zero_within_tolerance(y - other.y, tolerance);
    }

    float length() const noexcept {
        float mag2 = x * x + y * y;
        if (std::isfinite(mag2)) {
            return std::sqrt(mag2);
        } else {
            double xx = static_cast<double>(x);
            double yy = static_cast<double>(y);
            return static_cast<float>(std::sqrt(xx * xx + yy * yy));
        }
    }

    constexpr float length_sqd() const noexcept {
        return dot(*this);
    }

    bool normalize() noexcept {
        return set_length_from(x, y, 1.0f);
    }

    bool set_normalize(float nx, float ny) noexcept {
        return set_length_from(nx, ny, 1.0f);
    }

    bool set_length(float len) noexcept {
        return set_length_from(x, y, len);
    }

    bool set_length_from(float nx, float ny, float len, float* orig_length = nullptr) noexcept {
        double xx = static_cast<double>(nx);
        double yy = static_cast<double>(ny);
        double dmag = std::sqrt(xx * xx + yy * yy);
        double dscale = static_cast<double>(len) / dmag;
        nx = static_cast<float>(xx * dscale);
        ny = static_cast<float>(yy * dscale);

        if (!std::isfinite(nx) || !std::isfinite(ny) || (nx == 0.0f && ny == 0.0f)) {
            *this = Point::zero();
            return false;
        }

        if (orig_length) {
            *orig_length = static_cast<float>(dmag);
        }

        *this = Point::from_xy(nx, ny);
        return true;
    }

    float distance(const Point& other) const noexcept {
        return (*this - other).length();
    }

    constexpr float distance_to_sqd(const Point& pt) const noexcept {
        float dx = x - pt.x;
        float dy = y - pt.y;
        return dx * dx + dy * dy;
    }

    constexpr float dot(const Point& other) const noexcept {
        return x * other.x + y * other.y;
    }

    constexpr float cross(const Point& other) const noexcept {
        return x * other.y - y * other.x;
    }

    void scale(float s) noexcept {
        x *= s;
        y *= s;
    }

    constexpr Point scaled(float s) const noexcept {
        return Point(x * s, y * s);
    }

    void swap_coords() noexcept {
        std::swap(x, y);
    }

    void rotate_cw() noexcept {
        swap_coords();
        x = -x;
    }

    void rotate_ccw() noexcept {
        swap_coords();
        y = -y;
    }

    constexpr Point operator-() const noexcept {
        return Point(-x, -y);
    }

    constexpr Point operator+(const Point& o) const noexcept {
        return Point(x + o.x, y + o.y);
    }

    Point& operator+=(const Point& o) noexcept {
        x += o.x;
        y += o.y;
        return *this;
    }

    constexpr Point operator-(const Point& o) const noexcept {
        return Point(x - o.x, y - o.y);
    }

    Point& operator-=(const Point& o) noexcept {
        x -= o.x;
        y -= o.y;
        return *this;
    }

    constexpr Point operator*(const Point& o) const noexcept {
        return Point(x * o.x, y * o.y);
    }

    Point& operator*=(const Point& o) noexcept {
        x *= o.x;
        y *= o.y;
        return *this;
    }

    constexpr bool operator==(const Point& o) const noexcept {
        return x == o.x && y == o.y;
    }

    constexpr bool operator!=(const Point& o) const noexcept {
        return !(*this == o);
    }
};

} // namespace nisaba

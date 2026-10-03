#pragma once

#include "nisaba/math/scalar.hpp"

namespace nisaba {

struct f32x2 {
    float v[2]{0.0f, 0.0f};

    constexpr f32x2() noexcept = default;
    constexpr f32x2(float a, float b) noexcept : v{a, b} {}

    static constexpr f32x2 splat(float x) noexcept {
        return f32x2(x, x);
    }

    constexpr float x() const noexcept { return v[0]; }
    constexpr float y() const noexcept { return v[1]; }

    f32x2 abs() const noexcept {
        return f32x2(std::abs(v[0]), std::abs(v[1]));
    }

    constexpr f32x2 min(const f32x2& other) const noexcept {
        return f32x2(scalar::pmin(v[0], other.v[0]), scalar::pmin(v[1], other.v[1]));
    }

    constexpr f32x2 max(const f32x2& other) const noexcept {
        return f32x2(scalar::pmax(v[0], other.v[0]), scalar::pmax(v[1], other.v[1]));
    }

    constexpr float max_component() const noexcept {
        return scalar::pmax(v[0], v[1]);
    }

    constexpr f32x2 operator+(const f32x2& o) const noexcept {
        return f32x2(v[0] + o.v[0], v[1] + o.v[1]);
    }

    constexpr f32x2 operator-(const f32x2& o) const noexcept {
        return f32x2(v[0] - o.v[0], v[1] - o.v[1]);
    }

    constexpr f32x2 operator*(const f32x2& o) const noexcept {
        return f32x2(v[0] * o.v[0], v[1] * o.v[1]);
    }

    constexpr f32x2 operator/(const f32x2& o) const noexcept {
        return f32x2(v[0] / o.v[0], v[1] / o.v[1]);
    }

    constexpr bool operator==(const f32x2& o) const noexcept {
        return v[0] == o.v[0] && v[1] == o.v[1];
    }

    constexpr bool operator!=(const f32x2& o) const noexcept {
        return !(*this == o);
    }
};

} // namespace nisaba

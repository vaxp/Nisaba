#pragma once

#include <algorithm>
#include "nisaba/math/scalar.hpp"

namespace nisaba {

struct alignas(16) f32x4 {
    float v[4]{0.0f, 0.0f, 0.0f, 0.0f};

    constexpr f32x4() noexcept = default;
    constexpr f32x4(float a, float b, float c, float d) noexcept : v{a, b, c, d} {}
    constexpr explicit f32x4(const float arr[4]) noexcept : v{arr[0], arr[1], arr[2], arr[3]} {}

    static constexpr f32x4 splat(float x) noexcept {
        return f32x4(x, x, x, x);
    }

    f32x4 max(const f32x4& rhs) const noexcept {
        return f32x4(
            std::max(v[0], rhs.v[0]),
            std::max(v[1], rhs.v[1]),
            std::max(v[2], rhs.v[2]),
            std::max(v[3], rhs.v[3])
        );
    }

    f32x4 min(const f32x4& rhs) const noexcept {
        return f32x4(
            std::min(v[0], rhs.v[0]),
            std::min(v[1], rhs.v[1]),
            std::min(v[2], rhs.v[2]),
            std::min(v[3], rhs.v[3])
        );
    }

    constexpr f32x4 operator+(const f32x4& rhs) const noexcept {
        return f32x4(v[0] + rhs.v[0], v[1] + rhs.v[1], v[2] + rhs.v[2], v[3] + rhs.v[3]);
    }

    f32x4& operator+=(const f32x4& rhs) noexcept {
        v[0] += rhs.v[0];
        v[1] += rhs.v[1];
        v[2] += rhs.v[2];
        v[3] += rhs.v[3];
        return *this;
    }

    constexpr f32x4 operator-(const f32x4& rhs) const noexcept {
        return f32x4(v[0] - rhs.v[0], v[1] - rhs.v[1], v[2] - rhs.v[2], v[3] - rhs.v[3]);
    }

    constexpr f32x4 operator*(const f32x4& rhs) const noexcept {
        return f32x4(v[0] * rhs.v[0], v[1] * rhs.v[1], v[2] * rhs.v[2], v[3] * rhs.v[3]);
    }

    f32x4& operator*=(const f32x4& rhs) noexcept {
        v[0] *= rhs.v[0];
        v[1] *= rhs.v[1];
        v[2] *= rhs.v[2];
        v[3] *= rhs.v[3];
        return *this;
    }

    constexpr bool operator==(const f32x4& rhs) const noexcept {
        return v[0] == rhs.v[0] && v[1] == rhs.v[1] && v[2] == rhs.v[2] && v[3] == rhs.v[3];
    }

    constexpr bool operator!=(const f32x4& rhs) const noexcept {
        return !(*this == rhs);
    }
};

} // namespace nisaba

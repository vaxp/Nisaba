#pragma once

#include <cstdint>
#include <cmath>
#include <cassert>
#include <limits>
#include "nisaba/math/scalar.hpp"

namespace nisaba {

/// A 26.6 fixed point.
using FDot6 = int32_t;

/// A 24.8 fixed point.
using FDot8 = int32_t;

/// A 16.16 fixed point.
using FDot16 = int32_t;

namespace fdot16 {

inline constexpr FDot16 HALF = (1 << 16) / 2;
inline constexpr FDot16 ONE = 1 << 16;

inline FDot16 from_f32(float x) noexcept {
    return scalar::saturate_from(x * static_cast<float>(ONE));
}

inline constexpr int32_t floor_to_i32(FDot16 x) noexcept {
    return x >> 16;
}

inline constexpr int32_t ceil_to_i32(FDot16 x) noexcept {
    return (x + ONE - 1) >> 16;
}

inline constexpr int32_t round_to_i32(FDot16 x) noexcept {
    return (x + HALF) >> 16;
}

inline constexpr FDot16 mul(FDot16 a, FDot16 b) noexcept {
    return static_cast<FDot16>((static_cast<int64_t>(a) * static_cast<int64_t>(b)) >> 16);
}

inline constexpr FDot16 div(FDot6 numer, FDot6 denom) noexcept {
    int64_t v = left_shift64(static_cast<int64_t>(numer), 16) / static_cast<int64_t>(denom);
    int64_t n = bound(static_cast<int64_t>(std::numeric_limits<int32_t>::min()),
                      v,
                      static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    return static_cast<int32_t>(n);
}

inline constexpr FDot16 fast_div(FDot6 a, FDot6 b) noexcept {
    assert((left_shift(a, 16) >> 16) == a);
    assert(b != 0);
    return left_shift(a, 16) / b;
}

} // namespace fdot16

namespace fdot6 {

inline constexpr FDot6 ONE = 64;

inline constexpr FDot6 from_i32(int32_t n) noexcept {
    assert(static_cast<int32_t>(static_cast<int16_t>(n)) == n);
    return n << 6;
}

inline constexpr FDot6 from_f32(float n) noexcept {
    return static_cast<FDot6>(n * 64.0f);
}

inline constexpr FDot6 floor(FDot6 n) noexcept {
    return n >> 6;
}

inline constexpr FDot6 ceil(FDot6 n) noexcept {
    return (n + 63) >> 6;
}

inline constexpr FDot6 round(FDot6 n) noexcept {
    return (n + 32) >> 6;
}

inline constexpr FDot16 to_fdot16(FDot6 n) noexcept {
    assert((left_shift(n, 10) >> 10) == n);
    return left_shift(n, 10);
}

inline constexpr FDot16 div(FDot6 a, FDot6 b) noexcept {
    assert(b != 0);
    if (a >= std::numeric_limits<int16_t>::min() && a <= std::numeric_limits<int16_t>::max()) {
        return left_shift(a, 16) / b;
    } else {
        return fdot16::div(a, b);
    }
}

inline constexpr bool can_convert_to_fdot16(FDot6 n) noexcept {
    constexpr int32_t max_dot6 = std::numeric_limits<int32_t>::max() >> (16 - 6);
    return (n < 0 ? -n : n) <= max_dot6;
}

inline constexpr uint8_t small_scale(uint8_t value, FDot6 dot6) noexcept {
    assert(static_cast<uint32_t>(dot6) <= 64);
    return static_cast<uint8_t>((static_cast<int32_t>(value) * dot6) >> 6);
}

} // namespace fdot6

namespace fdot8 {

inline constexpr FDot8 from_fdot16(FDot16 x) noexcept {
    return (x + 0x80) >> 8;
}

} // namespace fdot8

} // namespace nisaba

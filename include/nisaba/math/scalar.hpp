#pragma once

#include <cstdint>
#include <cmath>
#include <cstring>
#include <limits>
#include <algorithm>
#include <bit>
#include "nisaba/types.hpp"

namespace nisaba {

inline constexpr int32_t left_shift(int32_t value, int32_t shift) noexcept {
    return static_cast<int32_t>(static_cast<uint32_t>(value) << shift);
}

inline constexpr int64_t left_shift64(int64_t value, int32_t shift) noexcept {
    return static_cast<int64_t>(static_cast<uint64_t>(value) << shift);
}

template <typename T>
constexpr T bound(T min_val, T value, T max_val) noexcept {
    return std::max(min_val, std::min(value, max_val));
}

inline constexpr float SCALAR_MAX = 3.402823466e+38f;
inline constexpr float SCALAR_NEARLY_ZERO = 1.0f / 4096.0f; // 1.0 / (1 << 12)
inline constexpr float SCALAR_ROOT_2_OVER_2 = 0.707106781f;
inline constexpr float FLOAT_PI = 3.14159265f;

inline constexpr float MAX_I32_FITS_IN_F32 = 2147483520.0f;
inline constexpr float MIN_I32_FITS_IN_F32 = -MAX_I32_FITS_IN_F32;

namespace scalar {

inline constexpr float half(float x) noexcept {
    return x * 0.5f;
}

inline constexpr float ave(float a, float b) noexcept {
    return (a + b) * 0.5f;
}

inline constexpr float sqr(float x) noexcept {
    return x * x;
}

inline constexpr float invert(float x) noexcept {
    return 1.0f / x;
}

/// A safe non-panicking clamp:
/// Returns max_val for NaN or +Infinity, min_val for -Infinity.
inline float bound(float val, float min_val, float max_val) noexcept {
    if (std::isnan(val)) {
        return max_val;
    }
    if (val < min_val) {
        return min_val;
    }
    if (val > max_val) {
        return max_val;
    }
    return val;
}

// Fast approximate power function (http://www.machinedlearnings.com/2011/06/fast-approximate-logarithm-exponential.html)
inline float approx_powf(float x, float y) noexcept {
    if (x == 0.0f || x == 1.0f) {
        return x;
    }

    uint32_t x_bits = std::bit_cast<uint32_t>(x);
    float e = static_cast<float>(x_bits) * (1.0f / static_cast<float>(1 << 23));
    float m = std::bit_cast<float>((x_bits & 0x007fffffu) | 0x3f000000u);

    float log2_x = e - 124.225514990f - 1.498030302f * m - 1.725879990f / (0.3520887068f + m);

    float xy = log2_x * y;

    float f = xy - std::floor(xy);

    float a = xy + 121.274057500f;
    a -= f * 1.490129070f;
    a += 27.728023300f / (4.84252568f - f);
    a *= static_cast<float>(1 << 23);

    constexpr uint32_t inf_bits = 0x7f800000u;
    if (a < static_cast<float>(inf_bits)) {
        if (a > 0.0f) {
            return std::bit_cast<float>(static_cast<uint32_t>(std::round(a)));
        } else {
            return 0.0f;
        }
    } else {
        return std::numeric_limits<float>::infinity();
    }
}

inline bool is_nearly_equal_within_tolerance(float a, float b, float tolerance) noexcept {
    return std::abs(a - b) <= tolerance;
}

inline bool is_nearly_equal(float a, float b) noexcept {
    return is_nearly_equal_within_tolerance(a, b, SCALAR_NEARLY_ZERO);
}

inline bool is_nearly_zero_within_tolerance(float x, float tolerance) noexcept {
    return std::abs(x) <= tolerance;
}

inline bool is_nearly_zero(float x) noexcept {
    return is_nearly_zero_within_tolerance(x, SCALAR_NEARLY_ZERO);
}

/// Convert a sign-bit int into a 2's complement int.
inline int32_t sign_bit_to_2s_compliment(int32_t x) noexcept {
    if (x < 0) {
        x &= 0x7FFFFFFF;
        x = -x;
    }
    return x;
}

/// Return float as 2s complement int for comparison.
inline int32_t f32_as_2s_compliment(float x) noexcept {
    int32_t bits = 0;
    std::memcpy(&bits, &x, sizeof(float));
    return sign_bit_to_2s_compliment(bits);
}

/// Floating point comparison within ULPs tolerance.
inline bool almost_dequal_ulps(float a, float b) noexcept {
    constexpr int32_t ULPS_EPSILON = 16;
    int32_t a_bits = f32_as_2s_compliment(a);
    int32_t b_bits = f32_as_2s_compliment(b);
    return (a_bits < b_bits + ULPS_EPSILON) && (b_bits < a_bits + ULPS_EPSILON);
}

/// Return the closest integer for the given float (saturating on limits/NaN).
inline int32_t saturate_from(float x) noexcept {
    if (std::isnan(x) || x >= MAX_I32_FITS_IN_F32) {
        return static_cast<int32_t>(MAX_I32_FITS_IN_F32);
    }
    if (x <= MIN_I32_FITS_IN_F32) {
        return static_cast<int32_t>(MIN_I32_FITS_IN_F32);
    }
    return static_cast<int32_t>(x);
}

inline int32_t saturate_floor(float x) noexcept {
    return saturate_from(std::floor(x));
}

inline int32_t saturate_ceil(float x) noexcept {
    return saturate_from(std::ceil(x));
}

inline int32_t saturate_round(float x) noexcept {
    return saturate_from(std::floor(x + 0.5f));
}

/// Faster and more forgiving min/max for floating point values.
inline constexpr float pmin(float a, float b) noexcept {
    return (a < b) ? a : b;
}

inline constexpr float pmax(float a, float b) noexcept {
    return (a < b) ? b : a;
}

inline constexpr float to_radians(float degrees) noexcept {
    return degrees * (FLOAT_PI / 180.0f);
}

inline constexpr float to_degrees(float radians) noexcept {
    return radians * (180.0f / FLOAT_PI);
}

} // namespace scalar

inline NormalizedF32Exclusive NormalizedF32Exclusive::create_bounded(float val) {
    float clamped = scalar::bound(val, std::numeric_limits<float>::epsilon(), 1.0f - std::numeric_limits<float>::epsilon());
    return NormalizedF32Exclusive(clamped);
}

} // namespace nisaba

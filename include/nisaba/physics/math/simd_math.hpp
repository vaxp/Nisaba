#pragma once

#include <cmath>
#include <cstdint>
#include <algorithm>
#include <limits>

// Architecture SIMD detection
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #if defined(__SSE2__) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
        #include <immintrin.h>
        #define NISABA_PHYSICS_SIMD_SSE 1
    #endif
    #if defined(__AVX2__)
        #define NISABA_PHYSICS_SIMD_AVX2 1
    #endif
#elif defined(__ARM_NEON) || defined(__ARM_NEON__)
    #include <arm_neon.h>
    #define NISABA_PHYSICS_SIMD_NEON 1
#endif

namespace nisaba::physics {

inline constexpr float PI = 3.14159265358979323846f;
inline constexpr float TWO_PI = 6.28318530717958647692f;
inline constexpr float HALF_PI = 1.57079632679489661923f;
inline constexpr float DEG_TO_RAD = PI / 180.0f;
inline constexpr float RAD_TO_DEG = 180.0f / PI;
inline constexpr float EPSILON = 1e-6f;
inline constexpr float LARGE_FLOAT = 1e30f;

[[nodiscard]] inline constexpr float to_radians(float degrees) noexcept {
    return degrees * DEG_TO_RAD;
}

[[nodiscard]] inline constexpr float to_degrees(float radians) noexcept {
    return radians * RAD_TO_DEG;
}

[[nodiscard]] inline constexpr float clamp(float value, float min_val, float max_val) noexcept {
    return std::max(min_val, std::min(value, max_val));
}

[[nodiscard]] inline bool approx_equal(float a, float b, float epsilon = EPSILON) noexcept {
    return std::abs(a - b) <= epsilon;
}

} // namespace nisaba::physics

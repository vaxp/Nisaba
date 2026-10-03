#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <limits>



namespace nisaba::layout {

/**
 * Checks whether a floating-point scalar represents an undefined (NaN) state.
 */
template <std::floating_point T>
[[nodiscard]] constexpr bool isUndefined(T val) noexcept {
  return std::isnan(val);
}

/**
 * Checks whether a floating-point scalar has a defined (non-NaN) numerical value.
 */
template <std::floating_point T>
[[nodiscard]] constexpr bool isDefined(T val) noexcept {
  return val == val;
}

/**
 * Checks whether a floating-point value is infinite.
 */
template <std::floating_point T>
[[nodiscard]] constexpr bool isinf(T val) noexcept {
  return val == std::numeric_limits<T>::infinity() ||
         val == -std::numeric_limits<T>::infinity();
}

/**
 * Selects the maximum value, prioritizing defined values over undefined ones.
 */
template <std::floating_point T>
[[nodiscard]] constexpr T maxOrDefined(T a, T b) noexcept {
  const bool aDef = !std::isnan(a);
  const bool bDef = !std::isnan(b);
  if (aDef && bDef) {
    return a > b ? a : b;
  }
  return aDef ? a : b;
}

/**
 * Selects the minimum value, prioritizing defined values over undefined ones.
 */
template <std::floating_point T>
[[nodiscard]] constexpr T minOrDefined(T a, T b) noexcept {
  const bool aDef = !std::isnan(a);
  const bool bDef = !std::isnan(b);
  if (aDef && bDef) {
    return a < b ? a : b;
  }
  return aDef ? a : b;
}

/**
 * Compares two floating point numbers within standard layout tolerance (1e-4).
 * Treats two undefined (NaN) values as equal.
 */
[[nodiscard]] constexpr bool inexactEquals(float a, float b) noexcept {
  const bool aDef = !std::isnan(a);
  const bool bDef = !std::isnan(b);
  if (aDef && bDef) {
    const float d = a - b;
    return (d < 0.0f ? -d : d) < 0.0001f;
  }
  return !aDef && !bDef;
}

[[nodiscard]] constexpr bool inexactEquals(double a, double b) noexcept {
  const bool aDef = !std::isnan(a);
  const bool bDef = !std::isnan(b);
  if (aDef && bDef) {
    const double d = a - b;
    return (d < 0.0 ? -d : d) < 0.0001;
  }
  return !aDef && !bDef;
}

/**
 * Element-wise approximate equality for arrays.
 */
template <std::size_t N, typename T>
[[nodiscard]] constexpr bool inexactEquals(
    const std::array<T, N>& lhs,
    const std::array<T, N>& rhs) noexcept {
  for (std::size_t i = 0; i < N; ++i) {
    if (!inexactEquals(lhs[i], rhs[i])) {
      return false;
    }
  }
  return true;
}

} // namespace nisaba::layout

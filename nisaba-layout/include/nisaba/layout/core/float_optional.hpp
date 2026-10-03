#pragma once

#include <cmath>
#include <limits>
#include <algorithm>

namespace nisaba::layout {

/**
 * Sovereign C++20 optional floating-point scalar type.
 * Uses quiet NaN as sentinel state to avoid pointer/boolean overhead.
 */
class FloatOptional {
 public:
  constexpr FloatOptional() noexcept = default;
  constexpr FloatOptional(float value) noexcept : value_(value) {}

  [[nodiscard]] constexpr bool isDefined() const noexcept { return value_ == value_; }
  [[nodiscard]] constexpr bool isUndefined() const noexcept { return !(value_ == value_); }
  [[nodiscard]] constexpr float unwrap() const noexcept { return value_; }
  [[nodiscard]] constexpr float value_or(float fallback) const noexcept {
    if (value_ == value_) {
      return value_;
    }
    return fallback;
  }
  [[nodiscard]] constexpr float unwrapOrDefault(float fallback) const noexcept {
    return value_or(fallback);
  }

  [[nodiscard]] constexpr bool operator==(const FloatOptional& rhs) const noexcept {
    const bool l = isDefined();
    const bool r = rhs.isDefined();
    return (l && r) ? (value_ == rhs.value_) : (!l && !r);
  }

  [[nodiscard]] constexpr bool operator==(float rhs) const noexcept {
    return isDefined() && (value_ == rhs);
  }

  [[nodiscard]] constexpr FloatOptional operator+(FloatOptional rhs) const noexcept {
    return (isDefined() && rhs.isDefined()) ? FloatOptional{value_ + rhs.value_} : FloatOptional{};
  }

  [[nodiscard]] constexpr FloatOptional operator-(FloatOptional rhs) const noexcept {
    return (isDefined() && rhs.isDefined()) ? FloatOptional{value_ - rhs.value_} : FloatOptional{};
  }

  [[nodiscard]] constexpr bool operator<(FloatOptional rhs) const noexcept {
    return (isDefined() && rhs.isDefined()) ? (value_ < rhs.value_) : false;
  }

  [[nodiscard]] constexpr bool operator>(FloatOptional rhs) const noexcept {
    return (isDefined() && rhs.isDefined()) ? (value_ > rhs.value_) : false;
  }

  [[nodiscard]] constexpr bool operator<=(FloatOptional rhs) const noexcept {
    return *this < rhs || *this == rhs;
  }

  [[nodiscard]] constexpr bool operator>=(FloatOptional rhs) const noexcept {
    return *this > rhs || *this == rhs;
  }

 private:
  float value_{std::numeric_limits<float>::quiet_NaN()};
};

[[nodiscard]] constexpr bool operator==(float lhs, FloatOptional rhs) noexcept {
  return rhs == lhs;
}

[[nodiscard]] inline FloatOptional maxOrDefined(FloatOptional a, FloatOptional b) noexcept {
  if (a.isUndefined()) return b;
  if (b.isUndefined()) return a;
  return FloatOptional{std::max(a.unwrap(), b.unwrap())};
}

[[nodiscard]] inline bool inexactEquals(FloatOptional a, FloatOptional b) noexcept {
  if (a.isUndefined() && b.isUndefined()) return true;
  if (a.isUndefined() || b.isUndefined()) return false;
  return std::abs(a.unwrap() - b.unwrap()) < 0.0001f;
}

} // namespace nisaba::layout

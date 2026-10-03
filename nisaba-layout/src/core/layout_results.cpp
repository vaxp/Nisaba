#include <cmath>
#include <nisaba/layout/core/layout_results.hpp>

namespace nisaba::layout {

bool LayoutResults::operator==(const LayoutResults& rhs) const noexcept {
  if (direction_ != rhs.direction_ ||
      hadOverflow_ != rhs.hadOverflow_ ||
      lastOwnerDirection != rhs.lastOwnerDirection ||
      !(cachedLayout == rhs.cachedLayout) ||
      !(computedFlexBasis == rhs.computedFlexBasis)) {
    return false;
  }

  auto approxEqual = [](float a, float b) noexcept {
    const bool aNan = std::isnan(a);
    const bool bNan = std::isnan(b);
    if (aNan && bNan) return true;
    if (aNan || bNan) return false;
    return std::abs(a - b) < 0.0001f;
  };

  for (size_t i = 0; i < 4; ++i) {
    if (!approxEqual(position_[i], rhs.position_[i]) ||
        !approxEqual(margin_[i], rhs.margin_[i]) ||
        !approxEqual(border_[i], rhs.border_[i]) ||
        !approxEqual(padding_[i], rhs.padding_[i])) {
      return false;
    }
  }

  for (size_t i = 0; i < 2; ++i) {
    if (!approxEqual(dimensions_[i], rhs.dimensions_[i]) ||
        !approxEqual(measuredDimensions_[i], rhs.measuredDimensions_[i])) {
      return false;
    }
  }

  for (size_t i = 0; i < MaxCachedMeasurements; ++i) {
    if (!(cachedMeasurements[i] == rhs.cachedMeasurements[i])) {
      return false;
    }
  }

  return true;
}

} // namespace nisaba::layout

#pragma once


#include <nisaba/layout/core/comparison.hpp>
#include <nisaba/layout/core/sizing_mode.hpp>

namespace nisaba::layout {

/**
 * Entry in the layout measurement cache for short-circuiting redundant calculations.
 */
struct CachedMeasurement {
  float availableWidth{-1.0f};
  float availableHeight{-1.0f};
  SizingMode widthSizingMode{SizingMode::MaxContent};
  SizingMode heightSizingMode{SizingMode::MaxContent};

  float computedWidth{-1.0f};
  float computedHeight{-1.0f};

  [[nodiscard]] constexpr bool operator==(const CachedMeasurement& other) const noexcept {
    if (widthSizingMode != other.widthSizingMode ||
        heightSizingMode != other.heightSizingMode) {
      return false;
    }

    auto matchScalar = [](float a, float b) noexcept {
      const bool aDef = !isUndefined(a);
      const bool bDef = !isUndefined(b);
      return (!aDef && !bDef) || (aDef && bDef && a == b);
    };

    return matchScalar(availableWidth, other.availableWidth) &&
           matchScalar(availableHeight, other.availableHeight) &&
           matchScalar(computedWidth, other.computedWidth) &&
           matchScalar(computedHeight, other.computedHeight);
  }
};

} // namespace nisaba::layout

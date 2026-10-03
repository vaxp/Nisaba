#pragma once

#include <cstddef>
#include <nisaba/layout/Layout.h>
#include <nisaba/layout/core/enums.hpp>

namespace nisaba::layout {

struct SpacingOffsets {
  float leading{0.0f};
  float between{0.0f};
};

/**
 * Computes leading and between offsets for justify-content distribution (W3C CSS Flexbox §9.5).
 */
[[nodiscard]] inline SpacingOffsets calculateJustifyOffsets(
    Justify justify,
    float freeSpace,
    size_t count,
    float gap) noexcept {
  SpacingOffsets offsets{.leading = 0.0f, .between = gap};
  if (count == 0) {
    return offsets;
  }

  switch (justify) {
    case Justify::Center:
      offsets.leading = freeSpace * 0.5f;
      break;
    case Justify::FlexEnd:
      offsets.leading = freeSpace;
      break;
    case Justify::SpaceBetween:
      if (count > 1) {
        offsets.between += freeSpace / static_cast<float>(count - 1);
      }
      break;
    case Justify::SpaceEvenly: {
      const float step = freeSpace / static_cast<float>(count + 1);
      offsets.leading = step;
      offsets.between += step;
      break;
    }
    case Justify::SpaceAround: {
      const float halfStep = 0.5f * freeSpace / static_cast<float>(count);
      offsets.leading = halfStep;
      offsets.between += halfStep * 2.0f;
      break;
    }
    default:
      break;
  }
  return offsets;
}

} // namespace nisaba::layout

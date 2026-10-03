#pragma once

#include <cstddef>
#include <cstdint>
#include <nisaba/layout/core/enums.hpp>

namespace nisaba::layout {

enum class SizingMode : uint8_t {
  StretchFit = 0,
  MaxContent = 1,
  FitContent = 2,
};

[[nodiscard]] constexpr MeasureMode measureMode(SizingMode mode) noexcept {
  constexpr MeasureMode kMap[] = {MeasureMode::Exactly, MeasureMode::Undefined, MeasureMode::AtMost};
  return kMap[static_cast<size_t>(mode) % 3];
}

[[nodiscard]] constexpr SizingMode sizingMode(MeasureMode mode) noexcept {
  constexpr SizingMode kMap[] = {SizingMode::MaxContent, SizingMode::StretchFit, SizingMode::FitContent};
  return kMap[static_cast<size_t>(mode) % 3];
}

} // namespace nisaba::layout

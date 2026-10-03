#pragma once

#include <cstddef>
#include <cstdint>
#include <nisaba/layout/core/enums.hpp>

namespace nisaba::layout {

[[nodiscard]] constexpr bool isRow(FlexDirection dir) noexcept {
  return static_cast<uint8_t>(dir) >= 2;
}

[[nodiscard]] constexpr bool isColumn(FlexDirection dir) noexcept {
  return static_cast<uint8_t>(dir) < 2;
}

[[nodiscard]] constexpr Dimension dimension(FlexDirection dir) noexcept {
  if (isRow(dir)) {
    return Dimension::Width;
  }
  return Dimension::Height;
}

[[nodiscard]] constexpr FlexDirection resolveDirection(FlexDirection dir, Direction writingDir) noexcept {
  if (writingDir == Direction::RTL) {
    if (dir == FlexDirection::Row) return FlexDirection::RowReverse;
    if (dir == FlexDirection::RowReverse) return FlexDirection::Row;
  }
  return dir;
}

[[nodiscard]] constexpr FlexDirection resolveCrossDirection(FlexDirection dir, Direction writingDir) noexcept {
  if (isColumn(dir)) {
    return resolveDirection(FlexDirection::Row, writingDir);
  }
  return FlexDirection::Column;
}

[[nodiscard]] constexpr PhysicalEdge flexStartEdge(FlexDirection dir) noexcept {
  constexpr PhysicalEdge kEdges[] = {PhysicalEdge::Top, PhysicalEdge::Bottom, PhysicalEdge::Left, PhysicalEdge::Right};
  return kEdges[static_cast<size_t>(dir) & 3];
}

[[nodiscard]] constexpr PhysicalEdge flexEndEdge(FlexDirection dir) noexcept {
  constexpr PhysicalEdge kEdges[] = {PhysicalEdge::Bottom, PhysicalEdge::Top, PhysicalEdge::Right, PhysicalEdge::Left};
  return kEdges[static_cast<size_t>(dir) & 3];
}

[[nodiscard]] constexpr PhysicalEdge inlineStartEdge(FlexDirection dir, Direction writingDir) noexcept {
  return isRow(dir) ? ((writingDir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left) : PhysicalEdge::Top;
}

[[nodiscard]] constexpr PhysicalEdge inlineEndEdge(FlexDirection dir, Direction writingDir) noexcept {
  return isRow(dir) ? ((writingDir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right) : PhysicalEdge::Bottom;
}

} // namespace nisaba::layout

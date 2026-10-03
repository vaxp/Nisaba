#pragma once

#include <cstdint>
#include <type_traits>

namespace nisaba::layout {

template <typename EnumT>
constexpr auto to_underlying(EnumT e) noexcept {
  using UnderlyingType = std::underlying_type_t<EnumT>;
  return static_cast<UnderlyingType>(e);
}

// -----------------------------------------------------------------------------
// Core Flexbox & CSS Enums
// -----------------------------------------------------------------------------

enum class Align : uint8_t {
  Auto = 0,
  FlexStart = 1,
  Center = 2,
  FlexEnd = 3,
  Stretch = 4,
  Baseline = 5,
  SpaceBetween = 6,
  SpaceAround = 7,
  SpaceEvenly = 8,
  Start = 9,
  End = 10,
};

enum class BoxSizing : uint8_t {
  BorderBox = 0,
  ContentBox = 1,
};

enum class Dimension : uint8_t {
  Width = 0,
  Height = 1,
};

enum class Direction : uint8_t {
  Inherit = 0,
  LTR = 1,
  RTL = 2,
};

enum class Display : uint8_t {
  Flex = 0,
  None = 1,
  Contents = 2,
  Grid = 3,
};

enum class Edge : uint8_t {
  Left = 0,
  Top = 1,
  Right = 2,
  Bottom = 3,
  Start = 4,
  End = 5,
  Horizontal = 6,
  Vertical = 7,
  All = 8,
};

enum class PhysicalEdge : uint8_t {
  Left = 0,
  Top = 1,
  Right = 2,
  Bottom = 3,
};

enum class FlexDirection : uint8_t {
  Column = 0,
  ColumnReverse = 1,
  Row = 2,
  RowReverse = 3,
};

enum class Gutter : uint8_t {
  Column = 0,
  Row = 1,
  All = 2,
};

enum class Justify : uint8_t {
  Auto = 0,
  FlexStart = 1,
  Center = 2,
  FlexEnd = 3,
  SpaceBetween = 4,
  SpaceAround = 5,
  SpaceEvenly = 6,
  Stretch = 7,
  Start = 8,
  End = 9,
};

enum class LogLevel : uint8_t {
  Error = 0,
  Warn = 1,
  Info = 2,
  Debug = 3,
  Verbose = 4,
  Fatal = 5,
};

enum class MeasureMode : uint8_t {
  Undefined = 0,
  Exactly = 1,
  AtMost = 2,
};

enum class NodeType : uint8_t {
  Default = 0,
  Text = 1,
};

enum class Overflow : uint8_t {
  Visible = 0,
  Hidden = 1,
  Scroll = 2,
};

enum class PositionType : uint8_t {
  Static = 0,
  Relative = 1,
  Absolute = 2,
};

enum class Unit : uint8_t {
  Undefined = 0,
  Point = 1,
  Percent = 2,
  Auto = 3,
  MaxContent = 4,
  FitContent = 5,
  Stretch = 6,
};

enum class Wrap : uint8_t {
  NoWrap = 0,
  Wrap = 1,
  WrapReverse = 2,
};

// Layout engine compatibility errata flags
enum class Errata : uint32_t {
  None = 0,
  All = 0xFFFFFFFF,
};
inline constexpr Errata operator|(Errata a, Errata b) noexcept {
  return static_cast<Errata>(to_underlying(a) | to_underlying(b));
}
inline constexpr Errata operator&(Errata a, Errata b) noexcept {
  return static_cast<Errata>(to_underlying(a) & to_underlying(b));
}

enum class ExperimentalFeature : uint8_t {
  WebFlexBasis = 0,
};

} // namespace nisaba::layout

#pragma once

#include <cstdint>
#include <vector>
#include <nisaba/layout/style/value.hpp>

namespace nisaba::layout {

enum class GridLineType : uint8_t {
  Auto,
  Integer,
  Span,
};

struct GridLine {
  GridLineType type{GridLineType::Auto};
  int32_t integer{0};

  constexpr static GridLine auto_() noexcept { return {}; }
  constexpr static GridLine fromInteger(int32_t v) noexcept { return {GridLineType::Integer, v}; }
  constexpr static GridLine span(int32_t v) noexcept { return {GridLineType::Span, v}; }

  [[nodiscard]] constexpr bool isAuto() const noexcept { return type == GridLineType::Auto; }
  [[nodiscard]] constexpr bool isInteger() const noexcept { return type == GridLineType::Integer; }
  [[nodiscard]] constexpr bool isSpan() const noexcept { return type == GridLineType::Span; }

  bool operator==(const GridLine&) const = default;
};

struct GridTrackSize {
  StyleSizeLength minSizingFunction{StyleSizeLength::ofAuto()};
  StyleSizeLength maxSizingFunction{StyleSizeLength::ofAuto()};
  float baseSize{0.0f};
  float growthLimit{0.0f};
  bool infinitelyGrowable{false};

  bool operator==(const GridTrackSize&) const = default;
};

using GridTrackList = std::vector<GridTrackSize>;

struct GridStyle {
  GridTrackList templateColumns{};
  GridTrackList templateRows{};
  GridTrackList autoColumns{};
  GridTrackList autoRows{};
  GridLine columnStart{};
  GridLine columnEnd{};
  GridLine rowStart{};
  GridLine rowEnd{};

  bool operator==(const GridStyle&) const = default;
};

} // namespace nisaba::layout

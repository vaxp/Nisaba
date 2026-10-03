#pragma once

#include <array>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/float_optional.hpp>
#include <nisaba/layout/style/value.hpp>

namespace nisaba::layout {

using Edges = std::array<StyleLength, 9>;
using Gutters = std::array<StyleLength, 3>;
using Dimensions = std::array<StyleSizeLength, 2>;

/**
 * Resolves left edge value using direction-aware fallback priority:
 * Start/End (based on LTR/RTL) -> Left -> Horizontal -> All
 */
[[nodiscard]] inline StyleLength computeLeftEdge(
    const Edges& edges,
    Direction layoutDirection) noexcept {
  const Edge inlineEdge = (layoutDirection == Direction::RTL) ? Edge::End : Edge::Start;
  const Edge order[] = {inlineEdge, Edge::Left, Edge::Horizontal, Edge::All};
  for (Edge e : order) {
    const auto& len = edges[to_underlying(e)];
    if (len.isDefined()) return len;
  }
  return edges[to_underlying(Edge::All)];
}

/**
 * Resolves right edge value using direction-aware fallback priority:
 * End/Start (based on LTR/RTL) -> Right -> Horizontal -> All
 */
[[nodiscard]] inline StyleLength computeRightEdge(
    const Edges& edges,
    Direction layoutDirection) noexcept {
  const Edge inlineEdge = (layoutDirection == Direction::RTL) ? Edge::Start : Edge::End;
  const Edge order[] = {inlineEdge, Edge::Right, Edge::Horizontal, Edge::All};
  for (Edge e : order) {
    const auto& len = edges[to_underlying(e)];
    if (len.isDefined()) return len;
  }
  return edges[to_underlying(Edge::All)];
}

/**
 * Resolves top edge value: Top -> Vertical -> All
 */
[[nodiscard]] inline StyleLength computeTopEdge(const Edges& edges) noexcept {
  const Edge order[] = {Edge::Top, Edge::Vertical, Edge::All};
  for (Edge e : order) {
    const auto& len = edges[to_underlying(e)];
    if (len.isDefined()) return len;
  }
  return edges[to_underlying(Edge::All)];
}

/**
 * Resolves bottom edge value: Bottom -> Vertical -> All
 */
[[nodiscard]] inline StyleLength computeBottomEdge(const Edges& edges) noexcept {
  const Edge order[] = {Edge::Bottom, Edge::Vertical, Edge::All};
  for (Edge e : order) {
    const auto& len = edges[to_underlying(e)];
    if (len.isDefined()) return len;
  }
  return edges[to_underlying(Edge::All)];
}

/**
 * Resolves a physical edge from an Edges array.
 */
[[nodiscard]] inline StyleLength computePhysicalEdge(
    const Edges& edges,
    PhysicalEdge edge,
    Direction direction) noexcept {
  switch (edge) {
    case PhysicalEdge::Left: return computeLeftEdge(edges, direction);
    case PhysicalEdge::Top: return computeTopEdge(edges);
    case PhysicalEdge::Right: return computeRightEdge(edges, direction);
    case PhysicalEdge::Bottom: return computeBottomEdge(edges);
  }
  return StyleLength::undefined();
}

/**
 * Resolves column gutter from Gutters array with fallback to Gutter::All.
 */
[[nodiscard]] inline StyleLength computeColumnGap(const Gutters& gap) noexcept {
  const auto& col = gap[to_underlying(Gutter::Column)];
  return col.isDefined() ? col : gap[to_underlying(Gutter::All)];
}

/**
 * Resolves row gutter from Gutters array with fallback to Gutter::All.
 */
[[nodiscard]] inline StyleLength computeRowGap(const Gutters& gap) noexcept {
  const auto& row = gap[to_underlying(Gutter::Row)];
  return row.isDefined() ? row : gap[to_underlying(Gutter::All)];
}

} // namespace nisaba::layout

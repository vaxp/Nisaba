#pragma once

#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/style/box_model.hpp>

namespace nisaba::layout {

/**
 * Checks whether any horizontal insets are explicitly specified.
 */
[[nodiscard]] inline bool hasHorizontalInsets(const Edges& position) noexcept {
  constexpr Edge edges[] = {
      Edge::Left, Edge::Right, Edge::All, Edge::Horizontal, Edge::Start, Edge::End};
  for (Edge e : edges) {
    if (position[to_underlying(e)].isDefined()) {
      return true;
    }
  }
  return false;
}

/**
 * Checks whether any vertical insets are explicitly specified.
 */
[[nodiscard]] inline bool hasVerticalInsets(const Edges& position) noexcept {
  constexpr Edge edges[] = {Edge::Top, Edge::Bottom, Edge::All, Edge::Vertical};
  for (Edge e : edges) {
    if (position[to_underlying(e)].isDefined()) {
      return true;
    }
  }
  return false;
}

} // namespace nisaba::layout

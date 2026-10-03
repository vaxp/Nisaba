#pragma once

#include <nisaba/layout/core/flex_direction.hpp>
#include <nisaba/layout/core/node.hpp>

namespace nisaba::layout {

[[nodiscard]] inline Align resolveChildAlignment(
    const Node* parent,
    const Node* child) noexcept {
  const Align childSelf = child->style().alignSelf();
  const Align effective = (childSelf == Align::Auto) ? parent->style().alignItems() : childSelf;
  if (effective == Align::Baseline && isColumn(parent->style().flexDirection())) {
    return Align::FlexStart;
  }
  return effective;
}

[[nodiscard]] inline Justify resolveChildJustification(
    const Node* parent,
    const Node* child) noexcept {
  const Justify childSelf = child->style().justifySelf();
  return (childSelf == Justify::Auto) ? parent->style().justifyItems() : childSelf;
}

[[nodiscard]] constexpr Align fallbackAlignment(Align a) noexcept {
  const auto val = static_cast<uint8_t>(a);
  return (val >= 4 && val <= 8) ? Align::FlexStart : a;
}

[[nodiscard]] constexpr Justify fallbackAlignment(Justify j) noexcept {
  const auto val = static_cast<uint8_t>(j);
  return (val >= 4 && val <= 6) ? Justify::FlexStart : j;
}

} // namespace nisaba::layout

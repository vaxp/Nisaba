#pragma once

#include <nisaba/layout/core/types.hpp>
#include <nisaba/layout/core/node.hpp>

namespace nisaba::layout {

/**
 * Snaps a floating-point coordinate to the nearest physical pixel boundary
 * based on the display device point scale factor.
 */
[[nodiscard]] float alignToPixelGrid(
    double value,
    double pointScaleFactor,
    bool forceCeil,
    bool forceFloor) noexcept;

[[nodiscard]] inline float roundValueToPixelGrid(
    double value,
    double pointScaleFactor,
    bool forceCeil,
    bool forceFloor) noexcept {
  return alignToPixelGrid(value, pointScaleFactor, forceCeil, forceFloor);
}

/**
 * Recursively aligns the layout geometry of a node and its entire subtree
 * to physical pixel boundaries.
 */
void quantizeLayoutMetrics(
    nisaba::layout::Node* node,
    double absoluteLeft,
    double absoluteTop);

inline void roundLayoutResultsToPixelGrid(
    nisaba::layout::Node* node,
    double absoluteLeft,
    double absoluteTop) {
  quantizeLayoutMetrics(node, absoluteLeft, absoluteTop);
}

// Compatibility aliases
[[nodiscard]] inline float snapValueToPixelGrid(
    double value, double pointScaleFactor, bool forceCeil, bool forceFloor) noexcept {
  return alignToPixelGrid(value, pointScaleFactor, forceCeil, forceFloor);
}

inline void snapLayoutResultsToPixelGrid(
    nisaba::layout::Node* node, double absoluteLeft, double absoluteTop) {
  quantizeLayoutMetrics(node, absoluteLeft, absoluteTop);
}

} // namespace nisaba::layout

#pragma once

#include <cstdint>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/flex_direction.hpp>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/core/sizing_mode.hpp>
#include <nisaba/layout/event/event.hpp>
#include <nisaba/layout/solver/pipeline.hpp>

namespace nisaba::layout {

/**
 * Top-level entry point to calculate CSS Flexbox layout for a node hierarchy.
 */
inline void solveFlexLayout(
    Node* node,
    float ownerWidth,
    float ownerHeight,
    Direction ownerDirection) {
  LayoutPipeline::run(node, ownerWidth, ownerHeight, ownerDirection);
}

/**
 * Recursively computes layout or measurement for a node and its subtree.
 */
inline bool solveSubtreeLayout(
    Node* node,
    float availableWidth,
    float availableHeight,
    Direction ownerDirection,
    SizingMode widthSizingMode,
    SizingMode heightSizingMode,
    float ownerWidth,
    float ownerHeight,
    bool performLayout,
    LayoutPassReason reason = LayoutPassReason::kInitial) {
  return LayoutPipeline::layoutSubtree(
      node,
      availableWidth,
      availableHeight,
      ownerDirection,
      widthSizingMode,
      heightSizingMode,
      ownerWidth,
      ownerHeight,
      performLayout,
      reason);
}

// Compatibility aliases
inline void calculateLayout(
    Node* node, float ownerWidth, float ownerHeight, Direction ownerDirection) {
  solveFlexLayout(node, ownerWidth, ownerHeight, ownerDirection);
}

inline bool computeLayoutRecursive(
    Node* node, float availableWidth, float availableHeight, Direction ownerDirection,
    SizingMode widthSizingMode, SizingMode heightSizingMode, float ownerWidth, float ownerHeight,
    bool performLayout, LayoutPassReason reason = LayoutPassReason::kInitial) {
  return solveSubtreeLayout(node, availableWidth, availableHeight, ownerDirection,
      widthSizingMode, heightSizingMode, ownerWidth, ownerHeight, performLayout, reason);
}

} // namespace nisaba::layout

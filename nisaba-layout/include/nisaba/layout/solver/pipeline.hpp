#pragma once

#include <vector>
#include <cstdint>
#include <nisaba/layout/Layout.h>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/flex_direction.hpp>
#include <nisaba/layout/core/sizing_mode.hpp>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/event/event.hpp>

namespace nisaba::layout {

/**
 * Represents an individual flex item during the layout pipeline.
 */
struct FlexItem {
  Node* node{nullptr};
  float hypotheticalMainSize{0.0f};
  float hypotheticalCrossSize{0.0f};
  float flexBasis{0.0f};
  float targetMainSize{0.0f};
  float targetCrossSize{0.0f};
  float mainMarginLeading{0.0f};
  float mainMarginTrailing{0.0f};
  float crossMarginLeading{0.0f};
  float crossMarginTrailing{0.0f};
  bool isFrozen{false};
  bool hasAutoMarginMainLeading{false};
  bool hasAutoMarginMainTrailing{false};
  bool hasAutoMarginCrossLeading{false};
  bool hasAutoMarginCrossTrailing{false};
};

/**
 * Represents a single flex line containing a subset of in-flow items.
 */
struct PipelineFlexLine {
  std::vector<FlexItem> items{};
  float lineMainSize{0.0f};
  float lineCrossSize{0.0f};
  float crossOffset{0.0f};
  float totalFlexGrow{0.0f};
  float totalFlexShrinkScaled{0.0f};
  float remainingFreeSpace{0.0f};
};

/**
 * Geometric and style context for a node during layout resolution.
 */
struct LayoutContext {
  float availableWidth{Undefined};
  float availableHeight{Undefined};
  float ownerWidth{Undefined};
  float ownerHeight{Undefined};
  Direction ownerDirection{Direction::LTR};
  Direction resolvedDirection{Direction::LTR};
  FlexDirection mainAxis{FlexDirection::Row};
  FlexDirection crossAxis{FlexDirection::Column};
  bool isRow{true};
  
  // Resolved node dimensions
  float resolvedWidth{Undefined};
  float resolvedHeight{Undefined};

  // Padding & border of container
  float paddingBorderMainLeading{0.0f};
  float paddingBorderMainTrailing{0.0f};
  float paddingBorderCrossLeading{0.0f};
  float paddingBorderCrossTrailing{0.0f};
  float paddingBorderWidth{0.0f};
  float paddingBorderHeight{0.0f};
  
  // Inner content box size
  float contentBoxWidth{Undefined};
  float contentBoxHeight{Undefined};
  float contentBoxMain{Undefined};
  float contentBoxCross{Undefined};
  
  // Gaps
  float mainGap{0.0f};
  float crossGap{0.0f};
};

/**
 * Clean-Room 7-Stage CSS Flexbox Pipeline Engine.
 * Implements the W3C CSS Flexible Box Layout Module Level 1 specification.
 */
class LAYOUT_EXPORT LayoutPipeline {
 public:
  /**
   * Top-level entry point to calculate layout for a node hierarchy.
   */
  static void run(Node* node, float ownerWidth, float ownerHeight, Direction ownerDirection);

  /**
   * Computes layout or measurement for a node and its subtree recursively.
   */
  static bool layoutSubtree(
      Node* node,
      float availableWidth,
      float availableHeight,
      Direction ownerDirection,
      SizingMode widthSizingMode,
      SizingMode heightSizingMode,
      float ownerWidth,
      float ownerHeight,
      bool performLayout,
      LayoutPassReason reason = LayoutPassReason::kInitial);

 private:
  // Stage 1: Constraint & Dimension Resolution
  static LayoutContext setupContext(
      Node* node,
      float availableWidth,
      float availableHeight,
      Direction ownerDirection,
      SizingMode widthMode,
      SizingMode heightMode,
      float ownerWidth,
      float ownerHeight);

  // Stage 2: Intrinsic Sizing & Hypothetical Main Sizes
  static std::vector<FlexItem> collectAndMeasureItems(
      Node* node,
      const LayoutContext& ctx);

  // Stage 3: Line Segmentation (flex-wrap)
  static std::vector<PipelineFlexLine> segmentLines(
      Node* node,
      std::vector<FlexItem>& items,
      const LayoutContext& ctx);

  // Stage 4: Free Space Distribution (flex-grow / flex-shrink)
  static void resolveFlexibleLengths(
      PipelineFlexLine& line,
      const LayoutContext& ctx);

  // Stage 5: Cross-Axis Sizing & Stretch
  static void resolveCrossAxisSizes(
      Node* node,
      std::vector<PipelineFlexLine>& lines,
      const LayoutContext& ctx);

  // Stage 6: Alignment & Positioning
  static void positionItems(
      Node* node,
      std::vector<PipelineFlexLine>& lines,
      const LayoutContext& ctx);

  // Stage 7: Absolute Children Positioning
  static void positionAbsoluteChildren(
      Node* node,
      const LayoutContext& ctx);
};

} // namespace nisaba::layout

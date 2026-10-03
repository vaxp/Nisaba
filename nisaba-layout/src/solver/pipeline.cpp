#include <algorithm>
#include <cmath>
#include <vector>

#include <nisaba/layout/core/comparison.hpp>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/flex_direction.hpp>
#include <nisaba/layout/core/float_optional.hpp>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/core/sizing_mode.hpp>
#include <nisaba/layout/solver/axis_aligner.hpp>
#include <nisaba/layout/solver/pipeline.hpp>
#include <nisaba/layout/solver/pixel_grid.hpp>
#include <nisaba/layout/style/box_model.hpp>

namespace nisaba::layout {

namespace {

inline float resolveSizeLength(
    StyleSizeLength size,
    float reference,
    float fallback = Undefined) noexcept {
  if (size.isDefined() && !size.isAuto()) {
    auto res = size.resolve(reference);
    if (res.isDefined()) {
      return res.unwrap();
    }
  }
  return fallback;
}

} // namespace

void LayoutPipeline::run(
    Node* node,
    float ownerWidth,
    float ownerHeight,
    Direction ownerDirection) {
  if (!node) {
    return;
  }

  // Top-level layout entry
  layoutSubtree(
      node,
      ownerWidth,
      ownerHeight,
      ownerDirection,
      isUndefined(ownerWidth) ? SizingMode::MaxContent : SizingMode::StretchFit,
      isUndefined(ownerHeight) ? SizingMode::MaxContent : SizingMode::StretchFit,
      ownerWidth,
      ownerHeight,
      true,
      LayoutPassReason::kInitial);

  // Pixel grid quantization
  if (node->getConfig()) {
    const float pointScale = node->getConfig()->getPointScaleFactor();
    if (pointScale > 0.0f) {
      quantizeLayoutMetrics(node, 0.0, 0.0);
    }
  }
}

bool LayoutPipeline::layoutSubtree(
    Node* node,
    float availableWidth,
    float availableHeight,
    Direction ownerDirection,
    SizingMode widthSizingMode,
    SizingMode heightSizingMode,
    float ownerWidth,
    float ownerHeight,
    bool performLayout,
    LayoutPassReason /*reason*/) {
  if (!node) {
    return false;
  }

  // If node is display: none, collapse layout
  if (node->style().display() == Display::None) {
    node->getLayout().setDimension(Dimension::Width, 0.0f);
    node->getLayout().setDimension(Dimension::Height, 0.0f);
    node->getLayout().setPosition(PhysicalEdge::Left, 0.0f);
    node->getLayout().setPosition(PhysicalEdge::Top, 0.0f);
    return true;
  }

  // Stage 1: Setup geometric & style context
  LayoutContext ctx = setupContext(
      node,
      availableWidth,
      availableHeight,
      ownerDirection,
      widthSizingMode,
      heightSizingMode,
      ownerWidth,
      ownerHeight);

  // If leaf node with custom measurement function
  if (node->hasMeasureFunc()) {
    const auto measureModeW = (widthSizingMode == SizingMode::StretchFit)
        ? MeasureMode::Exactly
        : (isDefined(ctx.contentBoxWidth) ? MeasureMode::AtMost : MeasureMode::Undefined);
    const auto measureModeH = (heightSizingMode == SizingMode::StretchFit)
        ? MeasureMode::Exactly
        : (isDefined(ctx.contentBoxHeight) ? MeasureMode::AtMost : MeasureMode::Undefined);

    const Size measured = node->measure(
        ctx.contentBoxWidth,
        measureModeW,
        ctx.contentBoxHeight,
        measureModeH);

    float finalW = isDefined(ctx.resolvedWidth)
        ? ctx.resolvedWidth
        : (measured.width + ctx.paddingBorderWidth);
    float finalH = isDefined(ctx.resolvedHeight)
        ? ctx.resolvedHeight
        : (measured.height + ctx.paddingBorderHeight);

    node->getLayout().setDimension(Dimension::Width, finalW);
    node->getLayout().setDimension(Dimension::Height, finalH);
    node->getLayout().setDirection(ctx.resolvedDirection);
    return true;
  }

  // Stage 2: Intrinsic Sizing & Hypothetical Main Sizes
  std::vector<FlexItem> items = collectAndMeasureItems(node, ctx);

  // Stage 3: Line Segmentation (flex-wrap)
  std::vector<PipelineFlexLine> lines = segmentLines(node, items, ctx);

  // Stage 4: Free Space Distribution (flex-grow / flex-shrink)
  for (auto& line : lines) {
    resolveFlexibleLengths(line, ctx);
  }

  // Stage 5: Cross-Axis Sizing & Stretch
  resolveCrossAxisSizes(node, lines, ctx);

  // Determine container's own dimensions if unconstrained (auto / content-sized)
  float maxLineMain = 0.0f;
  float totalLinesCross = 0.0f;
  for (size_t i = 0; i < lines.size(); ++i) {
    maxLineMain = std::max(maxLineMain, lines[i].lineMainSize);
    totalLinesCross += lines[i].lineCrossSize;
    if (i + 1 < lines.size()) {
      totalLinesCross += ctx.crossGap;
    }
  }

  float finalWidth = ctx.resolvedWidth;
  float finalHeight = ctx.resolvedHeight;

  if (isUndefined(finalWidth)) {
    float contentW = ctx.isRow ? maxLineMain : totalLinesCross;
    finalWidth = contentW + ctx.paddingBorderWidth;
    float minW = resolveSizeLength(node->style().minDimension(Dimension::Width), ownerWidth, 0.0f);
    float maxW = resolveSizeLength(node->style().maxDimension(Dimension::Width), ownerWidth, 1e9f);
    finalWidth = std::clamp(finalWidth, minW, maxW);
  }

  if (isUndefined(finalHeight)) {
    float contentH = ctx.isRow ? totalLinesCross : maxLineMain;
    finalHeight = contentH + ctx.paddingBorderHeight;
    float minH = resolveSizeLength(node->style().minDimension(Dimension::Height), ownerHeight, 0.0f);
    float maxH = resolveSizeLength(node->style().maxDimension(Dimension::Height), ownerHeight, 1e9f);
    finalHeight = std::clamp(finalHeight, minH, maxH);
  }

  node->getLayout().setDimension(Dimension::Width, finalWidth);
  node->getLayout().setDimension(Dimension::Height, finalHeight);
  node->getLayout().setDirection(ctx.resolvedDirection);

  // Update content box in context after container sizing
  ctx.contentBoxWidth = std::max(0.0f, finalWidth - ctx.paddingBorderWidth);
  ctx.contentBoxHeight = std::max(0.0f, finalHeight - ctx.paddingBorderHeight);
  ctx.contentBoxMain = ctx.isRow ? ctx.contentBoxWidth : ctx.contentBoxHeight;
  ctx.contentBoxCross = ctx.isRow ? ctx.contentBoxHeight : ctx.contentBoxWidth;

  if (performLayout) {
    // Stage 6: Alignment & Positioning
    positionItems(node, lines, ctx);

    // Stage 7: Absolute Children Positioning
    positionAbsoluteChildren(node, ctx);
  }

  return true;
}

LayoutContext LayoutPipeline::setupContext(
    Node* node,
    float availableWidth,
    float availableHeight,
    Direction ownerDirection,
    SizingMode widthMode,
    SizingMode heightMode,
    float ownerWidth,
    float ownerHeight) {
  LayoutContext ctx;
  ctx.availableWidth = availableWidth;
  ctx.availableHeight = availableHeight;
  ctx.ownerWidth = ownerWidth;
  ctx.ownerHeight = ownerHeight;
  ctx.ownerDirection = ownerDirection;

  // Resolve direction
  Direction dir = node->style().direction();
  if (dir == Direction::Inherit) {
    dir = (ownerDirection != Direction::Inherit) ? ownerDirection : Direction::LTR;
  }
  ctx.resolvedDirection = dir;

  // Flex directions
  ctx.mainAxis = node->style().flexDirection();
  ctx.isRow = (ctx.mainAxis == FlexDirection::Row || ctx.mainAxis == FlexDirection::RowReverse);
  ctx.crossAxis = ctx.isRow ? FlexDirection::Column : FlexDirection::Row;

  // Resolve padding and borders using public style methods
  const float padLeft = node->style().computeInlineStartPadding(FlexDirection::Row, dir, ownerWidth);
  const float padRight = node->style().computeInlineEndPadding(FlexDirection::Row, dir, ownerWidth);
  const float padTop = node->style().computeFlexStartPadding(FlexDirection::Column, dir, ownerHeight);
  const float padBottom = node->style().computeFlexEndPadding(FlexDirection::Column, dir, ownerHeight);

  const float borderLeft = node->style().computeInlineStartBorder(FlexDirection::Row, dir);
  const float borderRight = node->style().computeInlineEndBorder(FlexDirection::Row, dir);
  const float borderTop = node->style().computeFlexStartBorder(FlexDirection::Column, dir);
  const float borderBottom = node->style().computeFlexEndBorder(FlexDirection::Column, dir);

  node->getLayout().setPadding(PhysicalEdge::Left, padLeft);
  node->getLayout().setPadding(PhysicalEdge::Right, padRight);
  node->getLayout().setPadding(PhysicalEdge::Top, padTop);
  node->getLayout().setPadding(PhysicalEdge::Bottom, padBottom);

  node->getLayout().setBorder(PhysicalEdge::Left, borderLeft);
  node->getLayout().setBorder(PhysicalEdge::Right, borderRight);
  node->getLayout().setBorder(PhysicalEdge::Top, borderTop);
  node->getLayout().setBorder(PhysicalEdge::Bottom, borderBottom);

  ctx.paddingBorderWidth = padLeft + padRight + borderLeft + borderRight;
  ctx.paddingBorderHeight = padTop + padBottom + borderTop + borderBottom;

  if (ctx.isRow) {
    const bool isRtl = (dir == Direction::RTL);
    const bool isReverse = (ctx.mainAxis == FlexDirection::RowReverse);
    const bool invertMain = isRtl ^ isReverse;
    ctx.paddingBorderMainLeading = invertMain ? (padRight + borderRight) : (padLeft + borderLeft);
    ctx.paddingBorderMainTrailing = invertMain ? (padLeft + borderLeft) : (padRight + borderRight);
    ctx.paddingBorderCrossLeading = padTop + borderTop;
    ctx.paddingBorderCrossTrailing = padBottom + borderBottom;
  } else {
    const bool isReverse = (ctx.mainAxis == FlexDirection::ColumnReverse);
    ctx.paddingBorderMainLeading = isReverse ? (padBottom + borderBottom) : (padTop + borderTop);
    ctx.paddingBorderMainTrailing = isReverse ? (padTop + borderTop) : (padBottom + borderBottom);
    const bool isRtl = (dir == Direction::RTL);
    ctx.paddingBorderCrossLeading = isRtl ? (padRight + borderRight) : (padLeft + borderLeft);
    ctx.paddingBorderCrossTrailing = isRtl ? (padLeft + borderLeft) : (padRight + borderRight);
  }

  // Resolve container dimensions respecting BoxSizing and sizingMode
  const BoxSizing boxSizing = node->style().boxSizing();
  float styleW = resolveSizeLength(node->style().dimension(Dimension::Width), ownerWidth);
  float styleH = resolveSizeLength(node->style().dimension(Dimension::Height), ownerHeight);

  if (widthMode == SizingMode::StretchFit && isDefined(availableWidth)) {
    ctx.resolvedWidth = availableWidth;
    ctx.contentBoxWidth = std::max(0.0f, availableWidth - ctx.paddingBorderWidth);
  } else if (isDefined(styleW)) {
    if (boxSizing == BoxSizing::ContentBox) {
      ctx.contentBoxWidth = styleW;
      ctx.resolvedWidth = styleW + ctx.paddingBorderWidth;
    } else {
      ctx.resolvedWidth = styleW;
      ctx.contentBoxWidth = std::max(0.0f, styleW - ctx.paddingBorderWidth);
    }
  }

  if (heightMode == SizingMode::StretchFit && isDefined(availableHeight)) {
    ctx.resolvedHeight = availableHeight;
    ctx.contentBoxHeight = std::max(0.0f, availableHeight - ctx.paddingBorderHeight);
  } else if (isDefined(styleH)) {
    if (boxSizing == BoxSizing::ContentBox) {
      ctx.contentBoxHeight = styleH;
      ctx.resolvedHeight = styleH + ctx.paddingBorderHeight;
    } else {
      ctx.resolvedHeight = styleH;
      ctx.contentBoxHeight = std::max(0.0f, styleH - ctx.paddingBorderHeight);
    }
  }

  // Min/Max bounds on container dimensions
  float minW = resolveSizeLength(node->style().minDimension(Dimension::Width), ownerWidth, 0.0f);
  float maxW = resolveSizeLength(node->style().maxDimension(Dimension::Width), ownerWidth, 1e9f);
  if (boxSizing == BoxSizing::ContentBox) {
    minW += ctx.paddingBorderWidth;
    if (maxW < 1e8f) maxW += ctx.paddingBorderWidth;
  }
  if (isDefined(ctx.resolvedWidth)) {
    ctx.resolvedWidth = std::clamp(ctx.resolvedWidth, minW, maxW);
    ctx.contentBoxWidth = std::max(0.0f, ctx.resolvedWidth - ctx.paddingBorderWidth);
  } else if (isDefined(availableWidth)) {
    ctx.contentBoxWidth = std::max(0.0f, availableWidth - ctx.paddingBorderWidth);
  }

  float minH = resolveSizeLength(node->style().minDimension(Dimension::Height), ownerHeight, 0.0f);
  float maxH = resolveSizeLength(node->style().maxDimension(Dimension::Height), ownerHeight, 1e9f);
  if (boxSizing == BoxSizing::ContentBox) {
    minH += ctx.paddingBorderHeight;
    if (maxH < 1e8f) maxH += ctx.paddingBorderHeight;
  }
  if (isDefined(ctx.resolvedHeight)) {
    ctx.resolvedHeight = std::clamp(ctx.resolvedHeight, minH, maxH);
    ctx.contentBoxHeight = std::max(0.0f, ctx.resolvedHeight - ctx.paddingBorderHeight);
  } else if (isDefined(availableHeight)) {
    ctx.contentBoxHeight = std::max(0.0f, availableHeight - ctx.paddingBorderHeight);
  }

  ctx.contentBoxMain = ctx.isRow ? ctx.contentBoxWidth : ctx.contentBoxHeight;
  ctx.contentBoxCross = ctx.isRow ? ctx.contentBoxHeight : ctx.contentBoxWidth;

  // Gaps
  ctx.mainGap = node->style().computeGapForAxis(ctx.mainAxis, ctx.contentBoxMain);
  ctx.crossGap = node->style().computeGapForAxis(ctx.crossAxis, ctx.contentBoxCross);

  return ctx;
}

std::vector<FlexItem> LayoutPipeline::collectAndMeasureItems(
    Node* node,
    const LayoutContext& ctx) {
  std::vector<FlexItem> items;

  for (Node* child : node->getChildren()) {
    if (!child || child->style().display() == Display::None) {
      continue;
    }
    if (child->style().positionType() == PositionType::Absolute) {
      continue;
    }

    FlexItem item;
    item.node = child;

    // Resolve margins using public style methods
    item.hasAutoMarginMainLeading = ctx.isRow
        ? child->style().inlineStartMarginIsAuto(ctx.mainAxis, ctx.resolvedDirection)
        : child->style().flexStartMarginIsAuto(ctx.mainAxis, ctx.resolvedDirection);
    item.hasAutoMarginMainTrailing = ctx.isRow
        ? child->style().inlineEndMarginIsAuto(ctx.mainAxis, ctx.resolvedDirection)
        : child->style().flexEndMarginIsAuto(ctx.mainAxis, ctx.resolvedDirection);
    item.hasAutoMarginCrossLeading = ctx.isRow
        ? child->style().flexStartMarginIsAuto(ctx.crossAxis, ctx.resolvedDirection)
        : child->style().inlineStartMarginIsAuto(ctx.crossAxis, ctx.resolvedDirection);
    item.hasAutoMarginCrossTrailing = ctx.isRow
        ? child->style().flexEndMarginIsAuto(ctx.crossAxis, ctx.resolvedDirection)
        : child->style().inlineEndMarginIsAuto(ctx.crossAxis, ctx.resolvedDirection);

    const float ml = child->style().computeInlineStartMargin(FlexDirection::Row, ctx.resolvedDirection, ctx.contentBoxWidth);
    const float mr = child->style().computeInlineEndMargin(FlexDirection::Row, ctx.resolvedDirection, ctx.contentBoxWidth);
    const float mt = child->style().computeFlexStartMargin(FlexDirection::Column, ctx.resolvedDirection, ctx.contentBoxHeight);
    const float mb = child->style().computeFlexEndMargin(FlexDirection::Column, ctx.resolvedDirection, ctx.contentBoxHeight);

    child->getLayout().setMargin(PhysicalEdge::Left, item.hasAutoMarginMainLeading ? 0.0f : ml);
    child->getLayout().setMargin(PhysicalEdge::Right, item.hasAutoMarginMainTrailing ? 0.0f : mr);
    child->getLayout().setMargin(PhysicalEdge::Top, item.hasAutoMarginCrossLeading ? 0.0f : mt);
    child->getLayout().setMargin(PhysicalEdge::Bottom, item.hasAutoMarginCrossTrailing ? 0.0f : mb);

    if (ctx.isRow) {
      const bool isRtl = (ctx.resolvedDirection == Direction::RTL);
      const bool isReverse = (ctx.mainAxis == FlexDirection::RowReverse);
      const bool invert = isRtl ^ isReverse;
      item.mainMarginLeading = invert ? mr : ml;
      item.mainMarginTrailing = invert ? ml : mr;
      item.crossMarginLeading = mt;
      item.crossMarginTrailing = mb;
    } else {
      const bool isReverse = (ctx.mainAxis == FlexDirection::ColumnReverse);
      item.mainMarginLeading = isReverse ? mb : mt;
      item.mainMarginTrailing = isReverse ? mt : mb;
      const bool isRtl = (ctx.resolvedDirection == Direction::RTL);
      item.crossMarginLeading = isRtl ? mr : ml;
      item.crossMarginTrailing = isRtl ? ml : mr;
    }

    // Resolve child padding and border for BoxSizing calculations
    const float childPadBorderMain = ctx.isRow
        ? (child->style().computeInlineStartPadding(FlexDirection::Row, ctx.resolvedDirection, ctx.contentBoxWidth) +
           child->style().computeInlineEndPadding(FlexDirection::Row, ctx.resolvedDirection, ctx.contentBoxWidth) +
           child->style().computeInlineStartBorder(FlexDirection::Row, ctx.resolvedDirection) +
           child->style().computeInlineEndBorder(FlexDirection::Row, ctx.resolvedDirection))
        : (child->style().computeFlexStartPadding(FlexDirection::Column, ctx.resolvedDirection, ctx.contentBoxHeight) +
           child->style().computeFlexEndPadding(FlexDirection::Column, ctx.resolvedDirection, ctx.contentBoxHeight) +
           child->style().computeFlexStartBorder(FlexDirection::Column, ctx.resolvedDirection) +
           child->style().computeFlexEndBorder(FlexDirection::Column, ctx.resolvedDirection));

    const float childPadBorderCross = ctx.isRow
        ? (child->style().computeFlexStartPadding(FlexDirection::Column, ctx.resolvedDirection, ctx.contentBoxHeight) +
           child->style().computeFlexEndPadding(FlexDirection::Column, ctx.resolvedDirection, ctx.contentBoxHeight) +
           child->style().computeFlexStartBorder(FlexDirection::Column, ctx.resolvedDirection) +
           child->style().computeFlexEndBorder(FlexDirection::Column, ctx.resolvedDirection))
        : (child->style().computeInlineStartPadding(FlexDirection::Row, ctx.resolvedDirection, ctx.contentBoxWidth) +
           child->style().computeInlineEndPadding(FlexDirection::Row, ctx.resolvedDirection, ctx.contentBoxWidth) +
           child->style().computeInlineStartBorder(FlexDirection::Row, ctx.resolvedDirection) +
           child->style().computeInlineEndBorder(FlexDirection::Row, ctx.resolvedDirection));

    // Resolve flex-basis
    const auto fb = child->style().flexBasis();
    float basis = Undefined;

    if (fb.isDefined() && !fb.isAuto()) {
      auto res = fb.resolve(ctx.contentBoxMain);
      if (res.isDefined()) {
        basis = res.unwrap();
        if (child->style().boxSizing() == BoxSizing::ContentBox) {
          basis += childPadBorderMain;
        }
      }
    }

    if (isUndefined(basis)) {
      const auto mainDim = child->style().dimension(ctx.isRow ? Dimension::Width : Dimension::Height);
      basis = resolveSizeLength(mainDim, ctx.contentBoxMain);
      if (isDefined(basis) && child->style().boxSizing() == BoxSizing::ContentBox) {
        basis += childPadBorderMain;
      }
    }

    const float flexGrow = child->style().flexGrow().unwrapOrDefault(0.0f);

    if (isUndefined(basis)) {
      if (flexGrow > 0.0f) {
        // W3C Flexbox §9.2: purely flexible items without specified size grow from 0
        basis = 0.0f;
      } else if (child->hasMeasureFunc()) {
        const auto measured = child->measure(
            ctx.contentBoxWidth,
            MeasureMode::AtMost,
            ctx.contentBoxHeight,
            MeasureMode::AtMost);
        basis = (ctx.isRow ? measured.width : measured.height) + childPadBorderMain;
      } else {
        layoutSubtree(
            child,
            Undefined,
            Undefined,
            ctx.resolvedDirection,
            SizingMode::MaxContent,
            SizingMode::MaxContent,
            ctx.resolvedWidth,
            ctx.resolvedHeight,
            false);
        basis = ctx.isRow
            ? child->getLayout().dimension(Dimension::Width)
            : child->getLayout().dimension(Dimension::Height);
      }
    }

    item.flexBasis = std::max(0.0f, basis);

    // Hypothetical main size: clamp flex-basis to min/max main dimensions
    float minMain = resolveSizeLength(
        child->style().minDimension(ctx.isRow ? Dimension::Width : Dimension::Height),
        ctx.contentBoxMain,
        0.0f);
    float maxMain = resolveSizeLength(
        child->style().maxDimension(ctx.isRow ? Dimension::Width : Dimension::Height),
        ctx.contentBoxMain,
        1e9f);
    if (child->style().boxSizing() == BoxSizing::ContentBox) {
      minMain += childPadBorderMain;
      if (maxMain < 1e8f) maxMain += childPadBorderMain;
    }
    item.hypotheticalMainSize = std::clamp(item.flexBasis, minMain, maxMain);

    // Hypothetical cross size
    const auto crossDim = child->style().dimension(ctx.isRow ? Dimension::Height : Dimension::Width);
    float crossSize = resolveSizeLength(crossDim, ctx.contentBoxCross);
    if (isDefined(crossSize) && child->style().boxSizing() == BoxSizing::ContentBox) {
      crossSize += childPadBorderCross;
    }

    if (isUndefined(crossSize)) {
      if (child->hasMeasureFunc()) {
        const auto measured = child->measure(
            ctx.contentBoxWidth,
            MeasureMode::AtMost,
            ctx.contentBoxHeight,
            MeasureMode::AtMost);
        crossSize = (ctx.isRow ? measured.height : measured.width) + childPadBorderCross;
      } else {
        layoutSubtree(
            child,
            Undefined,
            Undefined,
            ctx.resolvedDirection,
            SizingMode::MaxContent,
            SizingMode::MaxContent,
            ctx.resolvedWidth,
            ctx.resolvedHeight,
            false);
        crossSize = ctx.isRow
            ? child->getLayout().dimension(Dimension::Height)
            : child->getLayout().dimension(Dimension::Width);
      }
    }

    // Aspect ratio resolution (bidirectional)
    if (child->style().aspectRatio().isDefined()) {
      const float ar = child->style().aspectRatio().unwrap();
      if (ar > 0.0f) {
        if (ctx.isRow) {
          if ((isUndefined(item.hypotheticalMainSize) || item.hypotheticalMainSize == 0.0f) && isDefined(crossSize) && crossSize > 0.0f) {
            item.hypotheticalMainSize = crossSize * ar;
            item.flexBasis = item.hypotheticalMainSize;
          } else if (isDefined(item.hypotheticalMainSize) && item.hypotheticalMainSize > 0.0f && (isUndefined(crossSize) || crossSize == 0.0f)) {
            crossSize = item.hypotheticalMainSize / ar;
          }
        } else {
          if ((isUndefined(item.hypotheticalMainSize) || item.hypotheticalMainSize == 0.0f) && isDefined(crossSize) && crossSize > 0.0f) {
            item.hypotheticalMainSize = crossSize / ar;
            item.flexBasis = item.hypotheticalMainSize;
          } else if (isDefined(item.hypotheticalMainSize) && item.hypotheticalMainSize > 0.0f && (isUndefined(crossSize) || crossSize == 0.0f)) {
            crossSize = item.hypotheticalMainSize * ar;
          }
        }
      }
    }

    float minCross = resolveSizeLength(
        child->style().minDimension(ctx.isRow ? Dimension::Height : Dimension::Width),
        ctx.contentBoxCross,
        0.0f);
    float maxCross = resolveSizeLength(
        child->style().maxDimension(ctx.isRow ? Dimension::Height : Dimension::Width),
        ctx.contentBoxCross,
        1e9f);
    if (child->style().boxSizing() == BoxSizing::ContentBox) {
      minCross += childPadBorderCross;
      if (maxCross < 1e8f) maxCross += childPadBorderCross;
    }
    item.hypotheticalCrossSize = std::clamp(crossSize, minCross, maxCross);

    item.targetMainSize = item.hypotheticalMainSize;
    item.targetCrossSize = item.hypotheticalCrossSize;

    items.push_back(item);
  }

  return items;
}

std::vector<PipelineFlexLine> LayoutPipeline::segmentLines(
    Node* node,
    std::vector<FlexItem>& items,
    const LayoutContext& ctx) {
  std::vector<PipelineFlexLine> lines;
  if (items.empty()) {
    return lines;
  }

  const Wrap wrap = node->style().flexWrap();
  const bool canWrap = (wrap == Wrap::Wrap || wrap == Wrap::WrapReverse);

  PipelineFlexLine currentLine;
  float currentLineMain = 0.0f;

  for (auto& item : items) {
    const float outerItemMain = item.hypotheticalMainSize + item.mainMarginLeading + item.mainMarginTrailing;

    if (canWrap && !currentLine.items.empty() && isDefined(ctx.contentBoxMain)) {
      const float potentialSize = currentLineMain + ctx.mainGap + outerItemMain;
      if (potentialSize > ctx.contentBoxMain) {
        currentLine.lineMainSize = currentLineMain;
        lines.push_back(std::move(currentLine));
        currentLine = PipelineFlexLine{};
        currentLineMain = 0.0f;
      }
    }

    if (!currentLine.items.empty()) {
      currentLineMain += ctx.mainGap;
    }
    currentLineMain += outerItemMain;
    currentLine.items.push_back(item);
  }

  if (!currentLine.items.empty()) {
    currentLine.lineMainSize = currentLineMain;
    lines.push_back(std::move(currentLine));
  }

  if (wrap == Wrap::WrapReverse) {
    std::reverse(lines.begin(), lines.end());
  }

  return lines;
}

void LayoutPipeline::resolveFlexibleLengths(
    PipelineFlexLine& line,
    const LayoutContext& ctx) {
  if (line.items.empty()) {
    return;
  }

  if (isUndefined(ctx.contentBoxMain)) {
    for (auto& item : line.items) {
      item.targetMainSize = item.hypotheticalMainSize;
    }
    return;
  }

  float remainingFreeSpace = ctx.contentBoxMain - line.lineMainSize;
  line.remainingFreeSpace = remainingFreeSpace;

  if (std::abs(remainingFreeSpace) < 1e-4f) {
    for (auto& item : line.items) {
      item.targetMainSize = item.hypotheticalMainSize;
    }
    return;
  }

  // W3C Flexbox §9.7 Free Space Distribution Loop
  bool hasViolations = true;
  size_t loopCount = 0;

  while (hasViolations && loopCount < 10) {
    hasViolations = false;
    ++loopCount;

    float totalFlexGrow = 0.0f;
    float totalFlexShrinkScaled = 0.0f;

    for (const auto& item : line.items) {
      if (!item.isFrozen) {
        const float fg = item.node->style().flexGrow().unwrapOrDefault(0.0f);
        const float fs = item.node->style().flexShrink().unwrapOrDefault(Style::DefaultFlexShrink);
        totalFlexGrow += fg;
        totalFlexShrinkScaled += fs * std::max(1.0f, item.flexBasis);
      }
    }

    line.totalFlexGrow = totalFlexGrow;
    line.totalFlexShrinkScaled = totalFlexShrinkScaled;

    if (remainingFreeSpace > 0.0f && totalFlexGrow > 0.0f) {
      const float spaceToDistribute = (totalFlexGrow < 1.0f)
          ? (remainingFreeSpace * totalFlexGrow)
          : remainingFreeSpace;

      for (auto& item : line.items) {
        if (!item.isFrozen) {
          const float fg = item.node->style().flexGrow().unwrapOrDefault(0.0f);
          const float share = (fg / totalFlexGrow) * spaceToDistribute;
          float target = item.hypotheticalMainSize + share;

          float maxMain = resolveSizeLength(
              item.node->style().maxDimension(ctx.isRow ? Dimension::Width : Dimension::Height),
              ctx.contentBoxMain,
              1e9f);
          if (item.node->style().boxSizing() == BoxSizing::ContentBox) {
            float childPB = ctx.isRow ? (item.node->getLayout().padding(PhysicalEdge::Left) + item.node->getLayout().padding(PhysicalEdge::Right) + item.node->getLayout().border(PhysicalEdge::Left) + item.node->getLayout().border(PhysicalEdge::Right))
                                      : (item.node->getLayout().padding(PhysicalEdge::Top) + item.node->getLayout().padding(PhysicalEdge::Bottom) + item.node->getLayout().border(PhysicalEdge::Top) + item.node->getLayout().border(PhysicalEdge::Bottom));
            if (maxMain < 1e8f) maxMain += childPB;
          }

          if (target > maxMain) {
            item.targetMainSize = maxMain;
            item.isFrozen = true;
            hasViolations = true;
            remainingFreeSpace -= (maxMain - item.hypotheticalMainSize);
          } else {
            item.targetMainSize = target;
          }
        }
      }
    } else if (remainingFreeSpace < 0.0f && totalFlexShrinkScaled > 0.0f) {
      const float shrinkSpace = -remainingFreeSpace;

      for (auto& item : line.items) {
        if (!item.isFrozen) {
          const float fs = item.node->style().flexShrink().unwrapOrDefault(Style::DefaultFlexShrink);
          const float share = ((fs * std::max(1.0f, item.flexBasis)) / totalFlexShrinkScaled) * shrinkSpace;
          float target = item.hypotheticalMainSize - share;

          float minMain = resolveSizeLength(
              item.node->style().minDimension(ctx.isRow ? Dimension::Width : Dimension::Height),
              ctx.contentBoxMain,
              0.0f);
          if (item.node->style().boxSizing() == BoxSizing::ContentBox) {
            float childPB = ctx.isRow ? (item.node->getLayout().padding(PhysicalEdge::Left) + item.node->getLayout().padding(PhysicalEdge::Right) + item.node->getLayout().border(PhysicalEdge::Left) + item.node->getLayout().border(PhysicalEdge::Right))
                                      : (item.node->getLayout().padding(PhysicalEdge::Top) + item.node->getLayout().padding(PhysicalEdge::Bottom) + item.node->getLayout().border(PhysicalEdge::Top) + item.node->getLayout().border(PhysicalEdge::Bottom));
            minMain += childPB;
          }

          if (target < minMain) {
            item.targetMainSize = minMain;
            item.isFrozen = true;
            hasViolations = true;
            remainingFreeSpace += (item.hypotheticalMainSize - minMain);
          } else {
            item.targetMainSize = target;
          }
        }
      }
    } else {
      break;
    }
  }

  float updatedMainSize = 0.0f;
  for (size_t i = 0; i < line.items.size(); ++i) {
    if (i > 0) {
      updatedMainSize += ctx.mainGap;
    }
    updatedMainSize += line.items[i].targetMainSize + line.items[i].mainMarginLeading + line.items[i].mainMarginTrailing;
  }
  line.lineMainSize = updatedMainSize;
}

void LayoutPipeline::resolveCrossAxisSizes(
    Node* node,
    std::vector<PipelineFlexLine>& lines,
    const LayoutContext& ctx) {
  float currentCrossOffset = 0.0f;

  for (auto& line : lines) {
    float maxCross = 0.0f;

    for (auto& item : line.items) {
      Align alignSelf = item.node->style().alignSelf();
      if (alignSelf == Align::Auto) {
        alignSelf = node->style().alignItems();
      }

      const auto crossDim = item.node->style().dimension(ctx.isRow ? Dimension::Height : Dimension::Width);
      const bool hasDefiniteCross = crossDim.isDefined() && !crossDim.isAuto();

      // In single-line with definite cross size, stretch to container. In multi-line or auto, stretch to line
      float targetStretchCross = (lines.size() == 1 && isDefined(ctx.contentBoxCross)) ? ctx.contentBoxCross : line.lineCrossSize;

      if (alignSelf == Align::Stretch && !hasDefiniteCross && isDefined(targetStretchCross) && targetStretchCross > 0.0f) {
        float stretched = targetStretchCross - item.crossMarginLeading - item.crossMarginTrailing;
        float minCross = resolveSizeLength(
            item.node->style().minDimension(ctx.isRow ? Dimension::Height : Dimension::Width),
            targetStretchCross,
            0.0f);
        float maxCrossBound = resolveSizeLength(
            item.node->style().maxDimension(ctx.isRow ? Dimension::Height : Dimension::Width),
            targetStretchCross,
            1e9f);
        if (item.node->style().boxSizing() == BoxSizing::ContentBox) {
          float childPB = ctx.isRow ? (item.node->getLayout().padding(PhysicalEdge::Top) + item.node->getLayout().padding(PhysicalEdge::Bottom) + item.node->getLayout().border(PhysicalEdge::Top) + item.node->getLayout().border(PhysicalEdge::Bottom))
                                    : (item.node->getLayout().padding(PhysicalEdge::Left) + item.node->getLayout().padding(PhysicalEdge::Right) + item.node->getLayout().border(PhysicalEdge::Left) + item.node->getLayout().border(PhysicalEdge::Right));
          minCross += childPB;
          if (maxCrossBound < 1e8f) maxCrossBound += childPB;
        }
        item.targetCrossSize = std::clamp(stretched, minCross, maxCrossBound);
      }

      const float outerCross = item.targetCrossSize + item.crossMarginLeading + item.crossMarginTrailing;
      maxCross = std::max(maxCross, outerCross);
    }

    line.lineCrossSize = maxCross;
    line.crossOffset = currentCrossOffset;
    currentCrossOffset += maxCross + ctx.crossGap;
  }

  // Handle single-line cross-axis auto margins and multi-line align-content
  if (lines.size() == 1 && isDefined(ctx.contentBoxCross)) {
    // If container is taller than the single line, handle cross auto-margins or align-content
    const float freeCross = ctx.contentBoxCross - lines[0].lineCrossSize;
    if (freeCross > 0.0f) {
      bool hasCrossAutoMargins = false;
      for (const auto& item : lines[0].items) {
        if (item.hasAutoMarginCrossLeading || item.hasAutoMarginCrossTrailing) {
          hasCrossAutoMargins = true;
          break;
        }
      }
      if (hasCrossAutoMargins) {
        // Will be distributed per item in positionItems
      } else {
        const Align alignContent = node->style().alignContent();
        switch (alignContent) {
          case Align::Center:
            lines[0].crossOffset = freeCross * 0.5f;
            break;
          case Align::FlexEnd:
            lines[0].crossOffset = freeCross;
            break;
          default:
            break;
        }
      }
    }
  } else if (lines.size() > 1 && isDefined(ctx.contentBoxCross)) {
    const float totalCross = currentCrossOffset - ctx.crossGap;
    const float freeCross = ctx.contentBoxCross - totalCross;

    if (freeCross > 0.0f) {
      const Align alignContent = node->style().alignContent();
      switch (alignContent) {
        case Align::Center:
          for (auto& line : lines) {
            line.crossOffset += freeCross * 0.5f;
          }
          break;
        case Align::FlexEnd:
          for (auto& line : lines) {
            line.crossOffset += freeCross;
          }
          break;
        case Align::SpaceBetween: {
          const float step = freeCross / static_cast<float>(lines.size() - 1);
          for (size_t i = 0; i < lines.size(); ++i) {
            lines[i].crossOffset += static_cast<float>(i) * step;
          }
          break;
        }
        case Align::SpaceAround: {
          const float step = freeCross / static_cast<float>(lines.size());
          for (size_t i = 0; i < lines.size(); ++i) {
            lines[i].crossOffset += (static_cast<float>(i) + 0.5f) * step;
          }
          break;
        }
        case Align::SpaceEvenly: {
          const float step = freeCross / static_cast<float>(lines.size() + 1);
          for (size_t i = 0; i < lines.size(); ++i) {
            lines[i].crossOffset += static_cast<float>(i + 1) * step;
          }
          break;
        }
        default:
          break;
      }
    }
  }
}

void LayoutPipeline::positionItems(
    Node* node,
    std::vector<PipelineFlexLine>& lines,
    const LayoutContext& ctx) {
  for (auto& line : lines) {
    float lineFreeSpace = 0.0f;
    if (isDefined(ctx.contentBoxMain)) {
      lineFreeSpace = std::max(0.0f, ctx.contentBoxMain - line.lineMainSize);
    }

    size_t autoMarginCount = 0;
    for (const auto& item : line.items) {
      if (item.hasAutoMarginMainLeading) ++autoMarginCount;
      if (item.hasAutoMarginMainTrailing) ++autoMarginCount;
    }

    float leadingMainOffset = 0.0f;
    float betweenMainOffset = ctx.mainGap;

    if (autoMarginCount > 0) {
      const float autoMarginSpace = lineFreeSpace / static_cast<float>(autoMarginCount);
      for (auto& item : line.items) {
        if (item.hasAutoMarginMainLeading) item.mainMarginLeading = autoMarginSpace;
        if (item.hasAutoMarginMainTrailing) item.mainMarginTrailing = autoMarginSpace;
      }
    } else {
      const Justify justify = node->style().justifyContent();
      const size_t count = line.items.size();
      if (count > 0) {
        SpacingOffsets offsets = calculateJustifyOffsets(justify, lineFreeSpace, count, ctx.mainGap);
        leadingMainOffset = offsets.leading;
        betweenMainOffset = offsets.between;
      }
    }

    float currentMain = leadingMainOffset;

    for (auto& item : line.items) {
      currentMain += item.mainMarginLeading;

      Align alignSelf = item.node->style().alignSelf();
      if (alignSelf == Align::Auto) {
        alignSelf = node->style().alignItems();
      }

      // Cross-axis free space calculation (respecting container height in single-line)
      const float availableLineCross = (lines.size() == 1 && isDefined(ctx.contentBoxCross))
          ? ctx.contentBoxCross
          : line.lineCrossSize;
      const float innerCrossFree = availableLineCross - (item.targetCrossSize + item.crossMarginLeading + item.crossMarginTrailing);

      float itemCrossOffset = item.crossMarginLeading;

      if (item.hasAutoMarginCrossLeading && item.hasAutoMarginCrossTrailing) {
        if (innerCrossFree > 0.0f) {
          itemCrossOffset = innerCrossFree * 0.5f;
        }
      } else if (item.hasAutoMarginCrossLeading) {
        if (innerCrossFree > 0.0f) {
          itemCrossOffset = innerCrossFree;
        }
      } else if (item.hasAutoMarginCrossTrailing) {
        itemCrossOffset = 0.0f;
      } else if (innerCrossFree > 0.0f) {
        switch (alignSelf) {
          case Align::Center:
            itemCrossOffset += innerCrossFree * 0.5f;
            break;
          case Align::FlexEnd:
            itemCrossOffset += innerCrossFree;
            break;
          default:
            break;
        }
      }

      float x = 0.0f;
      float y = 0.0f;
      float w = 0.0f;
      float h = 0.0f;

      if (ctx.isRow) {
        w = item.targetMainSize;
        h = item.targetCrossSize;

        const bool isRtl = (ctx.resolvedDirection == Direction::RTL);
        const bool isReverse = (ctx.mainAxis == FlexDirection::RowReverse);
        if (isRtl ^ isReverse) {
          x = (node->getLayout().dimension(Dimension::Width) - ctx.paddingBorderMainTrailing) - currentMain - w;
        } else {
          x = ctx.paddingBorderMainLeading + currentMain;
        }
        y = ctx.paddingBorderCrossLeading + line.crossOffset + itemCrossOffset;
      } else {
        w = item.targetCrossSize;
        h = item.targetMainSize;

        const bool isReverse = (ctx.mainAxis == FlexDirection::ColumnReverse);
        if (isReverse) {
          y = (node->getLayout().dimension(Dimension::Height) - ctx.paddingBorderMainTrailing) - currentMain - h;
        } else {
          y = ctx.paddingBorderMainLeading + currentMain;
        }
        x = ctx.paddingBorderCrossLeading + line.crossOffset + itemCrossOffset;
      }

      item.node->getLayout().setPosition(PhysicalEdge::Left, x);
      item.node->getLayout().setPosition(PhysicalEdge::Top, y);
      item.node->getLayout().setDimension(Dimension::Width, w);
      item.node->getLayout().setDimension(Dimension::Height, h);

      // Recurse into item's children with its definite assigned dimensions
      layoutSubtree(
          item.node,
          w,
          h,
          ctx.resolvedDirection,
          SizingMode::StretchFit,
          SizingMode::StretchFit,
          node->getLayout().dimension(Dimension::Width),
          node->getLayout().dimension(Dimension::Height),
          true,
          LayoutPassReason::kInitial);

      currentMain += item.targetMainSize + item.mainMarginTrailing + betweenMainOffset;
    }
  }
}

void LayoutPipeline::positionAbsoluteChildren(
    Node* node,
    const LayoutContext& ctx) {
  const float containerW = node->getLayout().dimension(Dimension::Width);
  const float containerH = node->getLayout().dimension(Dimension::Height);

  const float borderL = node->getLayout().border(PhysicalEdge::Left);
  const float borderR = node->getLayout().border(PhysicalEdge::Right);
  const float borderT = node->getLayout().border(PhysicalEdge::Top);
  const float borderB = node->getLayout().border(PhysicalEdge::Bottom);

  const float paddingBoxW = std::max(0.0f, containerW - borderL - borderR);
  const float paddingBoxH = std::max(0.0f, containerH - borderT - borderB);

  for (Node* child : node->getChildren()) {
    if (!child || child->style().display() == Display::None) {
      continue;
    }
    if (child->style().positionType() != PositionType::Absolute) {
      continue;
    }

    const bool hasLeft = child->style().isInlineStartPositionDefined(FlexDirection::Row, ctx.resolvedDirection);
    const bool hasRight = child->style().isInlineEndPositionDefined(FlexDirection::Row, ctx.resolvedDirection);
    const bool hasTop = child->style().isFlexStartPositionDefined(FlexDirection::Column, ctx.resolvedDirection);
    const bool hasBottom = child->style().isFlexEndPositionDefined(FlexDirection::Column, ctx.resolvedDirection);

    const float posL = hasLeft ? child->style().computeInlineStartPosition(FlexDirection::Row, ctx.resolvedDirection, paddingBoxW) : Undefined;
    const float posR = hasRight ? child->style().computeInlineEndPosition(FlexDirection::Row, ctx.resolvedDirection, paddingBoxW) : Undefined;
    const float posT = hasTop ? child->style().computeFlexStartPosition(FlexDirection::Column, ctx.resolvedDirection, paddingBoxH) : Undefined;
    const float posB = hasBottom ? child->style().computeFlexEndPosition(FlexDirection::Column, ctx.resolvedDirection, paddingBoxH) : Undefined;

    // Resolve width
    float childW = resolveSizeLength(child->style().dimension(Dimension::Width), paddingBoxW);
    if (isUndefined(childW) && hasLeft && hasRight) {
      childW = std::max(0.0f, paddingBoxW - posL - posR);
    }
    if (isUndefined(childW)) {
      if (child->hasMeasureFunc()) {
        childW = child->measure(paddingBoxW, MeasureMode::AtMost, paddingBoxH, MeasureMode::AtMost).width;
      } else {
        layoutSubtree(child, paddingBoxW, paddingBoxH, ctx.resolvedDirection, SizingMode::MaxContent, SizingMode::MaxContent, containerW, containerH, false);
        childW = child->getLayout().dimension(Dimension::Width);
      }
    }

    // Resolve height
    float childH = resolveSizeLength(child->style().dimension(Dimension::Height), paddingBoxH);
    if (isUndefined(childH) && hasTop && hasBottom) {
      childH = std::max(0.0f, paddingBoxH - posT - posB);
    }
    if (isUndefined(childH)) {
      if (child->hasMeasureFunc()) {
        childH = child->measure(paddingBoxW, MeasureMode::AtMost, paddingBoxH, MeasureMode::AtMost).height;
      } else {
        layoutSubtree(child, paddingBoxW, paddingBoxH, ctx.resolvedDirection, SizingMode::MaxContent, SizingMode::MaxContent, containerW, containerH, false);
        childH = child->getLayout().dimension(Dimension::Height);
      }
    }

    const float marginL = child->style().computeInlineStartMargin(FlexDirection::Row, ctx.resolvedDirection, paddingBoxW);
    const float marginR = child->style().computeInlineEndMargin(FlexDirection::Row, ctx.resolvedDirection, paddingBoxW);
    const float marginT = child->style().computeFlexStartMargin(FlexDirection::Column, ctx.resolvedDirection, paddingBoxH);
    const float marginB = child->style().computeFlexEndMargin(FlexDirection::Column, ctx.resolvedDirection, paddingBoxH);

    // Resolve X position
    float x = borderL;
    if (hasLeft) {
      x = borderL + posL + marginL;
    } else if (hasRight) {
      x = containerW - borderR - posR - childW - marginR;
    } else {
      x = borderL + marginL;
    }

    // Resolve Y position
    float y = borderT;
    if (hasTop) {
      y = borderT + posT + marginT;
    } else if (hasBottom) {
      y = containerH - borderB - posB - childH - marginB;
    } else {
      y = borderT + marginT;
    }

    child->getLayout().setPosition(PhysicalEdge::Left, x);
    child->getLayout().setPosition(PhysicalEdge::Top, y);
    child->getLayout().setDimension(Dimension::Width, childW);
    child->getLayout().setDimension(Dimension::Height, childH);
    child->getLayout().setMargin(PhysicalEdge::Left, marginL);
    child->getLayout().setMargin(PhysicalEdge::Top, marginT);
    child->getLayout().setMargin(PhysicalEdge::Right, marginR);
    child->getLayout().setMargin(PhysicalEdge::Bottom, marginB);

    // Recurse into absolute child
    layoutSubtree(
        child,
        childW,
        childH,
        ctx.resolvedDirection,
        SizingMode::StretchFit,
        SizingMode::StretchFit,
        containerW,
        containerH,
        true,
        LayoutPassReason::kInitial);
  }
}

} // namespace nisaba::layout

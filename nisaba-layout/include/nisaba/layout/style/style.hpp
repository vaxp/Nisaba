#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

#include <nisaba/layout/Layout.h>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/float_optional.hpp>
#include <nisaba/layout/style/box_model.hpp>
#include <nisaba/layout/style/grid_props.hpp>
#include <nisaba/layout/style/value.hpp>

namespace nisaba::layout {

/**
 * Sovereign C++20 CSS Flexbox Style Specification.
 * Provides cache-friendly flat storage and direct resolution without handle pools.
 */
class LAYOUT_EXPORT Style {
 public:
  using Length = StyleLength;
  using SizeLength = StyleSizeLength;

  static constexpr float DefaultFlexGrow = 0.0f;
  static constexpr float DefaultFlexShrink = 0.0f;
  static constexpr float WebDefaultFlexShrink = 1.0f;

  // ---------------------------------------------------------------------------
  // Flex Container & Alignment Properties
  // ---------------------------------------------------------------------------
  [[nodiscard]] constexpr Direction direction() const noexcept { return direction_; }
  void setDirection(Direction d) noexcept { direction_ = d; }

  [[nodiscard]] constexpr FlexDirection flexDirection() const noexcept { return flexDirection_; }
  void setFlexDirection(FlexDirection fd) noexcept { flexDirection_ = fd; }

  [[nodiscard]] constexpr Justify justifyContent() const noexcept { return justifyContent_; }
  void setJustifyContent(Justify j) noexcept { justifyContent_ = j; }

  [[nodiscard]] constexpr Justify justifyItems() const noexcept { return justifyItems_; }
  void setJustifyItems(Justify j) noexcept { justifyItems_ = j; }

  [[nodiscard]] constexpr Justify justifySelf() const noexcept { return justifySelf_; }
  void setJustifySelf(Justify j) noexcept { justifySelf_ = j; }

  [[nodiscard]] constexpr Align alignContent() const noexcept { return alignContent_; }
  void setAlignContent(Align a) noexcept { alignContent_ = a; }

  [[nodiscard]] constexpr Align alignItems() const noexcept { return alignItems_; }
  void setAlignItems(Align a) noexcept { alignItems_ = a; }

  [[nodiscard]] constexpr Align alignSelf() const noexcept { return alignSelf_; }
  void setAlignSelf(Align a) noexcept { alignSelf_ = a; }

  [[nodiscard]] constexpr PositionType positionType() const noexcept { return positionType_; }
  void setPositionType(PositionType pt) noexcept { positionType_ = pt; }

  [[nodiscard]] constexpr Wrap flexWrap() const noexcept { return flexWrap_; }
  void setFlexWrap(Wrap w) noexcept { flexWrap_ = w; }

  [[nodiscard]] constexpr Overflow overflow() const noexcept { return overflow_; }
  void setOverflow(Overflow o) noexcept { overflow_ = o; }

  [[nodiscard]] constexpr Display display() const noexcept { return display_; }
  void setDisplay(Display d) noexcept { display_ = d; }

  [[nodiscard]] constexpr BoxSizing boxSizing() const noexcept { return boxSizing_; }
  void setBoxSizing(BoxSizing bs) noexcept { boxSizing_ = bs; }

  // ---------------------------------------------------------------------------
  // Flex Item Properties
  // ---------------------------------------------------------------------------
  [[nodiscard]] constexpr FloatOptional flex() const noexcept { return flex_; }
  void setFlex(FloatOptional f) noexcept {
    flex_ = f;
    if (f.isDefined()) {
      flexGrow_ = f;
      flexShrink_ = FloatOptional{1.0f};
      flexBasis_ = StyleSizeLength::points(0.0f);
    } else {
      flexGrow_ = FloatOptional{};
      flexShrink_ = FloatOptional{};
      flexBasis_ = StyleSizeLength::ofAuto();
    }
  }

  [[nodiscard]] constexpr FloatOptional flexGrow() const noexcept { return flexGrow_; }
  void setFlexGrow(FloatOptional fg) noexcept { flexGrow_ = fg; }

  [[nodiscard]] constexpr FloatOptional flexShrink() const noexcept { return flexShrink_; }
  void setFlexShrink(FloatOptional fs) noexcept { flexShrink_ = fs; }

  [[nodiscard]] constexpr StyleSizeLength flexBasis() const noexcept { return flexBasis_; }
  void setFlexBasis(StyleSizeLength fb) noexcept { flexBasis_ = fb; }

  // ---------------------------------------------------------------------------
  // Dimensions & Aspect Ratio
  // ---------------------------------------------------------------------------
  [[nodiscard]] constexpr StyleSizeLength dimension(Dimension dim) const noexcept {
    const size_t idx = static_cast<size_t>(dim);
    return idx < dimensions_.size() ? dimensions_[idx] : StyleSizeLength::undefined();
  }
  void setDimension(Dimension dim, StyleSizeLength val) noexcept {
    const size_t idx = static_cast<size_t>(dim);
    if (idx < dimensions_.size()) dimensions_[idx] = val;
  }

  [[nodiscard]] constexpr StyleSizeLength minDimension(Dimension dim) const noexcept {
    const size_t idx = static_cast<size_t>(dim);
    return idx < minDimensions_.size() ? minDimensions_[idx] : StyleSizeLength::undefined();
  }
  void setMinDimension(Dimension dim, StyleSizeLength val) noexcept {
    const size_t idx = static_cast<size_t>(dim);
    if (idx < minDimensions_.size()) minDimensions_[idx] = val;
  }

  [[nodiscard]] constexpr StyleSizeLength maxDimension(Dimension dim) const noexcept {
    const size_t idx = static_cast<size_t>(dim);
    return idx < maxDimensions_.size() ? maxDimensions_[idx] : StyleSizeLength::undefined();
  }
  void setMaxDimension(Dimension dim, StyleSizeLength val) noexcept {
    const size_t idx = static_cast<size_t>(dim);
    if (idx < maxDimensions_.size()) maxDimensions_[idx] = val;
  }

  [[nodiscard]] constexpr FloatOptional aspectRatio() const noexcept { return aspectRatio_; }
  void setAspectRatio(FloatOptional ar) noexcept { aspectRatio_ = ar; }

  // ---------------------------------------------------------------------------
  // Box Insets (Margin, Padding, Border, Position)
  // ---------------------------------------------------------------------------
  [[nodiscard]] constexpr StyleLength margin(Edge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < margin_.size() ? margin_[idx] : StyleLength::undefined();
  }
  void setMargin(Edge edge, StyleLength val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < margin_.size()) margin_[idx] = val;
  }

  [[nodiscard]] constexpr StyleLength position(Edge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < position_.size() ? position_[idx] : StyleLength::undefined();
  }
  void setPosition(Edge edge, StyleLength val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < position_.size()) position_[idx] = val;
  }

  [[nodiscard]] constexpr StyleLength padding(Edge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < padding_.size() ? padding_[idx] : StyleLength::undefined();
  }
  void setPadding(Edge edge, StyleLength val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < padding_.size()) padding_[idx] = val;
  }

  [[nodiscard]] constexpr StyleLength border(Edge edge) const noexcept {
    const size_t idx = static_cast<size_t>(edge);
    return idx < border_.size() ? border_[idx] : StyleLength::undefined();
  }
  void setBorder(Edge edge, StyleLength val) noexcept {
    const size_t idx = static_cast<size_t>(edge);
    if (idx < border_.size()) border_[idx] = val;
  }

  [[nodiscard]] constexpr StyleLength gap(Gutter gutter) const noexcept {
    const size_t idx = static_cast<size_t>(gutter);
    return idx < gap_.size() ? gap_[idx] : StyleLength::undefined();
  }
  void setGap(Gutter gutter, StyleLength val) noexcept {
    const size_t idx = static_cast<size_t>(gutter);
    if (idx < gap_.size()) gap_[idx] = val;
  }

  // ---------------------------------------------------------------------------
  // Geometric Edge Computations for Layout Solver
  // ---------------------------------------------------------------------------
  [[nodiscard]] StyleLength computeMargin(PhysicalEdge edge, Direction dir) const noexcept {
    return computePhysicalEdge(margin_, edge, dir);
  }

  [[nodiscard]] StyleLength computePadding(PhysicalEdge edge, Direction dir) const noexcept {
    return computePhysicalEdge(padding_, edge, dir);
  }

  [[nodiscard]] StyleLength computeBorder(PhysicalEdge edge, Direction dir) const noexcept {
    return computePhysicalEdge(border_, edge, dir);
  }

  [[nodiscard]] StyleLength computePosition(PhysicalEdge edge, Direction dir) const noexcept {
    return computePhysicalEdge(position_, edge, dir);
  }

  [[nodiscard]] float computeInlineStartPadding(FlexDirection, Direction dir, float ownerWidth) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left;
    const auto len = computePadding(edge, dir);
    return len.resolve(ownerWidth).value_or(0.0f);
  }

  [[nodiscard]] float computeInlineEndPadding(FlexDirection, Direction dir, float ownerWidth) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right;
    const auto len = computePadding(edge, dir);
    return len.resolve(ownerWidth).value_or(0.0f);
  }

  [[nodiscard]] float computeFlexStartPadding(FlexDirection, Direction dir, float ownerHeight) const noexcept {
    const auto len = computePadding(PhysicalEdge::Top, dir);
    return len.resolve(ownerHeight).value_or(0.0f);
  }

  [[nodiscard]] float computeFlexEndPadding(FlexDirection, Direction dir, float ownerHeight) const noexcept {
    const auto len = computePadding(PhysicalEdge::Bottom, dir);
    return len.resolve(ownerHeight).value_or(0.0f);
  }

  [[nodiscard]] float computeInlineStartBorder(FlexDirection, Direction dir) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left;
    const auto len = computeBorder(edge, dir);
    return len.isPoints() ? len.value().unwrap() : 0.0f;
  }

  [[nodiscard]] float computeInlineEndBorder(FlexDirection, Direction dir) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right;
    const auto len = computeBorder(edge, dir);
    return len.isPoints() ? len.value().unwrap() : 0.0f;
  }

  [[nodiscard]] float computeFlexStartBorder(FlexDirection, Direction dir) const noexcept {
    const auto len = computeBorder(PhysicalEdge::Top, dir);
    return len.isPoints() ? len.value().unwrap() : 0.0f;
  }

  [[nodiscard]] float computeFlexEndBorder(FlexDirection, Direction dir) const noexcept {
    const auto len = computeBorder(PhysicalEdge::Bottom, dir);
    return len.isPoints() ? len.value().unwrap() : 0.0f;
  }

  [[nodiscard]] float computeInlineStartMargin(FlexDirection, Direction dir, float ownerWidth) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left;
    const auto len = computeMargin(edge, dir);
    return len.isAuto() ? 0.0f : len.resolve(ownerWidth).value_or(0.0f);
  }

  [[nodiscard]] float computeInlineEndMargin(FlexDirection, Direction dir, float ownerWidth) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right;
    const auto len = computeMargin(edge, dir);
    return len.isAuto() ? 0.0f : len.resolve(ownerWidth).value_or(0.0f);
  }

  [[nodiscard]] float computeFlexStartMargin(FlexDirection, Direction dir, float ownerHeight) const noexcept {
    const auto len = computeMargin(PhysicalEdge::Top, dir);
    return len.isAuto() ? 0.0f : len.resolve(ownerHeight).value_or(0.0f);
  }

  [[nodiscard]] float computeFlexEndMargin(FlexDirection, Direction dir, float ownerHeight) const noexcept {
    const auto len = computeMargin(PhysicalEdge::Bottom, dir);
    return len.isAuto() ? 0.0f : len.resolve(ownerHeight).value_or(0.0f);
  }

  [[nodiscard]] bool isInlineStartPositionDefined(FlexDirection, Direction dir) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left;
    return computePosition(edge, dir).isDefined();
  }

  [[nodiscard]] bool isInlineEndPositionDefined(FlexDirection, Direction dir) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right;
    return computePosition(edge, dir).isDefined();
  }

  [[nodiscard]] bool isFlexStartPositionDefined(FlexDirection, Direction dir) const noexcept {
    const auto topPos = computePosition(PhysicalEdge::Top, dir);
    return topPos.isDefined();
  }

  [[nodiscard]] bool isFlexEndPositionDefined(FlexDirection, Direction dir) const noexcept {
    const auto btmPos = computePosition(PhysicalEdge::Bottom, dir);
    return btmPos.isDefined();
  }

  [[nodiscard]] float computeInlineStartPosition(FlexDirection, Direction dir, float ownerWidth) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left;
    return computePosition(edge, dir).resolve(ownerWidth).value_or(0.0f);
  }

  [[nodiscard]] float computeInlineEndPosition(FlexDirection, Direction dir, float ownerWidth) const noexcept {
    const auto edge = (dir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right;
    return computePosition(edge, dir).resolve(ownerWidth).value_or(0.0f);
  }

  [[nodiscard]] float computeFlexStartPosition(FlexDirection, Direction dir, float ownerHeight) const noexcept {
    return computePosition(PhysicalEdge::Top, dir).resolve(ownerHeight).value_or(0.0f);
  }

  [[nodiscard]] float computeFlexEndPosition(FlexDirection, Direction dir, float ownerHeight) const noexcept {
    return computePosition(PhysicalEdge::Bottom, dir).resolve(ownerHeight).value_or(0.0f);
  }

  [[nodiscard]] float computePaddingAndBorderForDimension(Direction dir, Dimension dim, float ownerSize) const noexcept {
    if (dim == Dimension::Width) {
      return computeInlineStartPadding(FlexDirection::Row, dir, ownerSize) +
             computeInlineEndPadding(FlexDirection::Row, dir, ownerSize) +
             computeInlineStartBorder(FlexDirection::Row, dir) +
             computeInlineEndBorder(FlexDirection::Row, dir);
    }
    return computeFlexStartPadding(FlexDirection::Column, dir, ownerSize) +
           computeFlexEndPadding(FlexDirection::Column, dir, ownerSize) +
           computeFlexStartBorder(FlexDirection::Column, dir) +
           computeFlexEndBorder(FlexDirection::Column, dir);
  }

  [[nodiscard]] float computeGapForAxis(FlexDirection axis, float ownerSize) const noexcept {
    const bool isCol = (axis == FlexDirection::Column || axis == FlexDirection::ColumnReverse);
    const auto gutter = isCol ? computeRowGap(gap_) : computeColumnGap(gap_);
    return gutter.resolve(ownerSize).value_or(0.0f);
  }

  [[nodiscard]] bool flexStartMarginIsAuto(FlexDirection axis, Direction dir) const noexcept {
    const bool isRow = (axis == FlexDirection::Row || axis == FlexDirection::RowReverse);
    const auto edge = isRow ? ((dir == Direction::RTL) ? PhysicalEdge::Right : PhysicalEdge::Left) : PhysicalEdge::Top;
    return computeMargin(edge, dir).isAuto();
  }

  [[nodiscard]] bool flexEndMarginIsAuto(FlexDirection axis, Direction dir) const noexcept {
    const bool isRow = (axis == FlexDirection::Row || axis == FlexDirection::RowReverse);
    const auto edge = isRow ? ((dir == Direction::RTL) ? PhysicalEdge::Left : PhysicalEdge::Right) : PhysicalEdge::Bottom;
    return computeMargin(edge, dir).isAuto();
  }

  [[nodiscard]] bool inlineStartMarginIsAuto(FlexDirection axis, Direction dir) const noexcept {
    return flexStartMarginIsAuto(axis, dir);
  }

  [[nodiscard]] bool inlineEndMarginIsAuto(FlexDirection axis, Direction dir) const noexcept {
    return flexEndMarginIsAuto(axis, dir);
  }

  bool operator==(const Style& other) const = default;

 private:
  Direction direction_{Direction::Inherit};
  FlexDirection flexDirection_{FlexDirection::Column};
  Justify justifyContent_{Justify::FlexStart};
  Justify justifyItems_{Justify::Stretch};
  Justify justifySelf_{Justify::Auto};
  Align alignContent_{Align::FlexStart};
  Align alignItems_{Align::Stretch};
  Align alignSelf_{Align::Auto};
  PositionType positionType_{PositionType::Relative};
  Wrap flexWrap_{Wrap::NoWrap};
  Overflow overflow_{Overflow::Visible};
  Display display_{Display::Flex};
  BoxSizing boxSizing_{BoxSizing::BorderBox};

  FloatOptional flex_{};
  FloatOptional flexGrow_{};
  FloatOptional flexShrink_{};
  StyleSizeLength flexBasis_{StyleSizeLength::ofAuto()};

  Edges margin_{};
  Edges position_{};
  Edges padding_{};
  Edges border_{};
  Gutters gap_{StyleLength::undefined(), StyleLength::undefined(), StyleLength::undefined()};
  Dimensions dimensions_{StyleSizeLength::ofAuto(), StyleSizeLength::ofAuto()};
  Dimensions minDimensions_{StyleSizeLength::undefined(), StyleSizeLength::undefined()};
  Dimensions maxDimensions_{StyleSizeLength::undefined(), StyleSizeLength::undefined()};
  FloatOptional aspectRatio_{};

  GridStyle grid_{};
};

} // namespace nisaba::layout

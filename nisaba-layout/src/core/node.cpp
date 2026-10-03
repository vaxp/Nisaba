#include <algorithm>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/solver/pipeline.hpp>

namespace nisaba::layout {

Node::Node() : Node(&Config::getDefault()) {}

Node::Node(const Config* config)
    : config_{config ? config : &Config::getDefault()} {
  if (config_->useWebDefaults()) {
    style_.setFlexDirection(FlexDirection::Row);
    style_.setAlignContent(Align::Stretch);
  }
}

Node::~Node() {
  if (parent_) {
    parent_->removeChild(this);
    parent_ = nullptr;
  }
  for (auto* child : children_) {
    if (child) {
      child->parent_ = nullptr;
    }
  }
  children_.clear();
}

Node::Node(Node&& other) noexcept
    : parent_{other.parent_},
      children_{std::move(other.children_)},
      style_{std::move(other.style_)},
      layout_{other.layout_},
      measureFunc_{other.measureFunc_},
      baselineFunc_{other.baselineFunc_},
      context_{other.context_},
      config_{other.config_},
      lineIndex_{other.lineIndex_},
      nodeType_{other.nodeType_},
      isDirty_{other.isDirty_},
      hasNewLayout_{other.hasNewLayout_},
      alwaysFormsContainingBlock_{other.alwaysFormsContainingBlock_},
      isReferenceBaseline_{other.isReferenceBaseline_} {
  other.parent_ = nullptr;
  for (auto* child : children_) {
    if (child) {
      child->parent_ = this;
    }
  }
}

Node& Node::operator=(Node&& other) noexcept {
  if (this != &other) {
    for (auto* child : children_) {
      if (child) child->parent_ = nullptr;
    }
    parent_ = other.parent_;
    children_ = std::move(other.children_);
    style_ = std::move(other.style_);
    layout_ = other.layout_;
    measureFunc_ = other.measureFunc_;
    baselineFunc_ = other.baselineFunc_;
    context_ = other.context_;
    config_ = other.config_;
    lineIndex_ = other.lineIndex_;
    nodeType_ = other.nodeType_;
    isDirty_ = other.isDirty_;
    hasNewLayout_ = other.hasNewLayout_;
    alwaysFormsContainingBlock_ = other.alwaysFormsContainingBlock_;
    isReferenceBaseline_ = other.isReferenceBaseline_;

    other.parent_ = nullptr;
    for (auto* child : children_) {
      if (child) child->parent_ = this;
    }
  }
  return *this;
}

void Node::insertChild(Node* child, size_t index) {
  if (!child) return;
  child->parent_ = this;
  if (index >= children_.size()) {
    children_.push_back(child);
  } else {
    children_.insert(children_.begin() + index, child);
  }
  markDirty();
}

bool Node::removeChild(Node* child) {
  if (!child) return false;
  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    (*it)->parent_ = nullptr;
    children_.erase(it);
    markDirty();
    return true;
  }
  return false;
}

void Node::removeChild(size_t index) {
  if (index < children_.size()) {
    children_[index]->parent_ = nullptr;
    children_.erase(children_.begin() + index);
    markDirty();
  }
}

void Node::clearChildren() {
  for (auto* child : children_) {
    if (child) child->parent_ = nullptr;
  }
  children_.clear();
  markDirty();
}

void Node::replaceChild(Node* child, size_t index) {
  if (index < children_.size() && child) {
    children_[index]->parent_ = nullptr;
    child->parent_ = this;
    children_[index] = child;
    markDirty();
  }
}

void Node::replaceChild(Node* oldChild, Node* newChild) {
  if (!oldChild || !newChild) return;
  auto it = std::find(children_.begin(), children_.end(), oldChild);
  if (it != children_.end()) {
    (*it)->parent_ = nullptr;
    newChild->parent_ = this;
    *it = newChild;
    markDirty();
  }
}

void Node::setChildren(const std::vector<Node*>& children) {
  clearChildren();
  for (size_t i = 0; i < children.size(); ++i) {
    insertChild(children[i], i);
  }
}

void Node::setStyle(const Style& style) {
  style_ = style;
  markDirty();
}

void Node::markDirty() {
  if (!isDirty_) {
    isDirty_ = true;
    if (parent_) {
      parent_->markDirty();
    }
  }
}

Size Node::measure(
    float availableWidth,
    MeasureMode widthMode,
    float availableHeight,
    MeasureMode heightMode) {
  if (measureFunc_) {
    return measureFunc_(this, availableWidth, widthMode, availableHeight, heightMode);
  }
  return {0.0f, 0.0f};
}

float Node::baseline(float width, float height) const {
  if (baselineFunc_) {
    return baselineFunc_(this, width, height);
  }
  return height;
}

void Node::calculateLayout(
    float availableWidth,
    float availableHeight,
    Direction ownerDirection) {
  LayoutPipeline::run(this, availableWidth, availableHeight, ownerDirection);
}

bool Node::hasDefiniteLength(Dimension dimension, float ownerSize) const noexcept {
  const auto len = style_.dimension(dimension);
  const auto res = len.resolve(ownerSize);
  return res.isDefined() && res.unwrap() >= 0.0f;
}

float Node::resolveFlexGrow() const noexcept {
  return style_.flexGrow().value_or(0.0f);
}

float Node::resolveFlexShrink() const noexcept {
  return style_.flexShrink().value_or(Style::DefaultFlexShrink);
}

bool Node::isNodeFlexible() const noexcept {
  return (style_.positionType() != PositionType::Absolute) &&
         (resolveFlexGrow() > 0.0f || resolveFlexShrink() > 0.0f);
}

Direction Node::resolveDirection(Direction ownerDirection) const noexcept {
  return (style_.direction() == Direction::Inherit) ? ownerDirection : style_.direction();
}

float Node::relativePosition(FlexDirection axis, Direction direction, float axisSize) const noexcept {
  if (style_.positionType() != PositionType::Relative) {
    return 0.0f;
  }
  const bool isRowAxis = (axis == FlexDirection::Row || axis == FlexDirection::RowReverse);
  if (isRowAxis) {
    const auto lead = style_.position(direction == Direction::RTL ? Edge::Right : Edge::Left);
    if (lead.isDefined()) return lead.resolve(axisSize).value_or(0.0f);
    const auto trail = style_.position(direction == Direction::RTL ? Edge::Left : Edge::Right);
    if (trail.isDefined()) return -trail.resolve(axisSize).value_or(0.0f);
  } else {
    const auto lead = style_.position(Edge::Top);
    if (lead.isDefined()) return lead.resolve(axisSize).value_or(0.0f);
    const auto trail = style_.position(Edge::Bottom);
    if (trail.isDefined()) return -trail.resolve(axisSize).value_or(0.0f);
  }
  return 0.0f;
}

FloatOptional Node::getResolvedDimension(
    Direction direction,
    Dimension dimension,
    float referenceLength,
    float ownerWidth) const noexcept {
  const auto dimVal = style_.dimension(dimension);
  const auto resolved = dimVal.resolve(referenceLength);
  if (style_.boxSizing() == BoxSizing::BorderBox || resolved.isUndefined()) {
    return resolved;
  }
  const float padBorder = style_.computePaddingAndBorderForDimension(direction, dimension, ownerWidth);
  return FloatOptional{resolved.unwrap() + padBorder};
}

void Node::reset() {
  clearChildren();
  style_ = Style{};
  layout_ = LayoutResults{};
  measureFunc_ = nullptr;
  baselineFunc_ = nullptr;
  context_ = nullptr;
  isDirty_ = true;
  hasNewLayout_ = true;
}

} // namespace nisaba::layout

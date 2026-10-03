#pragma once

#include <cstdint>
#include <vector>

#include <nisaba/layout/Layout.h>
#include <nisaba/layout/core/config.hpp>
#include <nisaba/layout/core/enums.hpp>
#include <nisaba/layout/core/float_optional.hpp>
#include <nisaba/layout/core/layout_results.hpp>
#include <nisaba/layout/style/style.hpp>

namespace nisaba::layout {

/**
 * Sovereign scene graph node for flexbox layout computation.
 */
class LAYOUT_EXPORT Node {
 public:
  Node();
  explicit Node(const Config* config);
  ~Node();

  Node(const Node&) = delete;
  Node& operator=(const Node&) = delete;

  Node(Node&& other) noexcept;
  Node& operator=(Node&& other) noexcept;

  // ---------------------------------------------------------------------------
  // Hierarchy Management
  // ---------------------------------------------------------------------------
  void insertChild(Node* child, size_t index);
  bool removeChild(Node* child);
  void removeChild(size_t index);
  void clearChildren();
  void replaceChild(Node* child, size_t index);
  void replaceChild(Node* oldChild, Node* newChild);

  [[nodiscard]] Node* getOwner() const noexcept { return parent_; }
  [[nodiscard]] Node* getParent() const noexcept { return parent_; }
  void setOwner(Node* owner) noexcept { parent_ = owner; }

  [[nodiscard]] Node* getChild(size_t index) const { return children_.at(index); }
  [[nodiscard]] size_t getChildCount() const noexcept { return children_.size(); }
  [[nodiscard]] const std::vector<Node*>& getChildren() const noexcept { return children_; }
  void setChildren(const std::vector<Node*>& children);

  // ---------------------------------------------------------------------------
  // Styling & Layout Results
  // ---------------------------------------------------------------------------
  [[nodiscard]] Style& style() noexcept { return style_; }
  [[nodiscard]] const Style& style() const noexcept { return style_; }
  void setStyle(const Style& style);

  [[nodiscard]] LayoutResults& getLayout() noexcept { return layout_; }
  [[nodiscard]] const LayoutResults& getLayout() const noexcept { return layout_; }
  void setLayout(const LayoutResults& layout) noexcept { layout_ = layout; }

  // ---------------------------------------------------------------------------
  // Intrinsic Measurement Callbacks
  // ---------------------------------------------------------------------------
  void setMeasureFunc(MeasureCallback fn) noexcept { measureFunc_ = fn; }
  [[nodiscard]] bool hasMeasureFunc() const noexcept {
    if (measureFunc_) return true;
    return false;
  }
  Size measure(float availableWidth, MeasureMode widthMode, float availableHeight, MeasureMode heightMode);

  void setBaselineFunc(BaselineCallback fn) noexcept { baselineFunc_ = fn; }
  [[nodiscard]] bool hasBaselineFunc() const noexcept {
    if (baselineFunc_) return true;
    return false;
  }
  float baseline(float width, float height) const;

  // ---------------------------------------------------------------------------
  // Layout Execution
  // ---------------------------------------------------------------------------
  void calculateLayout(
      float availableWidth = Undefined,
      float availableHeight = Undefined,
      Direction ownerDirection = Direction::LTR);

  // ---------------------------------------------------------------------------
  // State & Flags
  // ---------------------------------------------------------------------------
  [[nodiscard]] bool isDirty() const noexcept { return isDirty_; }
  void setDirty(bool dirty) noexcept { isDirty_ = dirty; }
  void markDirty();
  void markDirtyAndPropagate() { markDirty(); }

  [[nodiscard]] void* getContext() const noexcept { return context_; }
  void setContext(void* ctx) noexcept { context_ = ctx; }

  [[nodiscard]] const Config* getConfig() const noexcept { return config_; }
  void setConfig(Config* config) noexcept { config_ = config; }

  [[nodiscard]] bool getHasNewLayout() const noexcept { return hasNewLayout_; }
  void setHasNewLayout(bool hasNew) noexcept { hasNewLayout_ = hasNew; }

  [[nodiscard]] NodeType getNodeType() const noexcept { return nodeType_; }
  void setNodeType(NodeType type) noexcept { nodeType_ = type; }

  [[nodiscard]] bool alwaysFormsContainingBlock() const noexcept { return alwaysFormsContainingBlock_; }
  void setAlwaysFormsContainingBlock(bool forms) noexcept { alwaysFormsContainingBlock_ = forms; }

  [[nodiscard]] bool isReferenceBaseline() const noexcept { return isReferenceBaseline_; }
  void setIsReferenceBaseline(bool isRef) noexcept { isReferenceBaseline_ = isRef; }

  [[nodiscard]] size_t getLineIndex() const noexcept { return lineIndex_; }
  void setLineIndex(size_t idx) noexcept { lineIndex_ = idx; }

  // Compatibility queries
  [[nodiscard]] bool hasDefiniteLength(Dimension dimension, float ownerSize) const noexcept;
  [[nodiscard]] float resolveFlexGrow() const noexcept;
  [[nodiscard]] float resolveFlexShrink() const noexcept;
  [[nodiscard]] bool isNodeFlexible() const noexcept;
  [[nodiscard]] Direction resolveDirection(Direction ownerDirection) const noexcept;
  [[nodiscard]] float relativePosition(FlexDirection axis, Direction direction, float axisSize) const noexcept;
  void processDimensions() noexcept {}
  [[nodiscard]] Style::SizeLength getProcessedDimension(Dimension dim) const noexcept {
    return style_.dimension(dim);
  }
  [[nodiscard]] FloatOptional getResolvedDimension(
      Direction direction,
      Dimension dimension,
      float referenceLength,
      float ownerWidth) const noexcept;

  void reset();

 private:
  Node* parent_{nullptr};
  std::vector<Node*> children_{};
  Style style_{};
  LayoutResults layout_{};
  MeasureCallback measureFunc_{nullptr};
  BaselineCallback baselineFunc_{nullptr};
  void* context_{nullptr};
  const Config* config_{nullptr};
  size_t lineIndex_{0};
  NodeType nodeType_{NodeType::Default};
  bool isDirty_{true};
  bool hasNewLayout_{true};
  bool alwaysFormsContainingBlock_{false};
  bool isReferenceBaseline_{false};
};

} // namespace nisaba::layout

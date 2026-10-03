#include <cmath>
#include <nisaba/layout/core/flex_direction.hpp>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/solver/align.hpp>
#include <nisaba/layout/solver/baseline.hpp>

namespace nisaba::layout {

float computeNodeBaseline(const Node* node) {
  if (!node) return 0.0f;
  if (node->hasBaselineFunc()) {
    const float result = node->baseline(
        node->getLayout().dimension(Dimension::Width),
        node->getLayout().dimension(Dimension::Height));
    return std::isnan(result) ? 0.0f : result;
  }

  const Node* targetChild = nullptr;
  for (const auto* child : node->getChildren()) {
    if (!child || child->getLineIndex() > 0) break;
    if (child->style().positionType() == PositionType::Absolute) continue;

    if (resolveChildAlignment(node, child) == Align::Baseline || child->isReferenceBaseline()) {
      targetChild = child;
      break;
    }
    if (!targetChild) {
      targetChild = child;
    }
  }

  if (!targetChild) {
    return node->getLayout().dimension(Dimension::Height);
  }

  return computeNodeBaseline(targetChild) + targetChild->getLayout().position(PhysicalEdge::Top);
}

bool hasBaselineAlignment(const Node* node) {
  if (!node || isColumn(node->style().flexDirection())) {
    return false;
  }
  if (node->style().alignItems() == Align::Baseline) {
    return true;
  }
  for (const auto* child : node->getChildren()) {
    if (child && child->style().positionType() != PositionType::Absolute &&
        child->style().alignSelf() == Align::Baseline) {
      return true;
    }
  }
  return false;
}

} // namespace nisaba::layout

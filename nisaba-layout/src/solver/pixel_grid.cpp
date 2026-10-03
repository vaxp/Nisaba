#include <cmath>
#include <nisaba/layout/core/node.hpp>
#include <nisaba/layout/core/types.hpp>
#include <nisaba/layout/solver/pixel_grid.hpp>

namespace nisaba::layout {

float alignToPixelGrid(
    const double value,
    const double pointScaleFactor,
    const bool forceCeil,
    const bool forceFloor) noexcept {
  if (std::isnan(value) || std::isnan(pointScaleFactor) || pointScaleFactor <= 0.0) {
    return Undefined;
  }

  const double scaled = value * pointScaleFactor;
  double rounded = std::round(scaled);
  if (forceCeil) {
    rounded = std::ceil(scaled);
  } else if (forceFloor) {
    rounded = std::floor(scaled);
  }
  return static_cast<float>(rounded / pointScaleFactor);
}

void quantizeLayoutMetrics(
    Node* const node,
    const double absoluteLeft,
    const double absoluteTop) {
  if (!node) return;
  const auto factor = static_cast<double>(node->getConfig()->getPointScaleFactor());
  if (factor <= 0.0) return;

  const double nodeLeft = node->getLayout().position(PhysicalEdge::Left);
  const double nodeTop = node->getLayout().position(PhysicalEdge::Top);
  const double nodeWidth = node->getLayout().dimension(Dimension::Width);
  const double nodeHeight = node->getLayout().dimension(Dimension::Height);

  const double absL = absoluteLeft + nodeLeft;
  const double absT = absoluteTop + nodeTop;
  const double absR = absL + nodeWidth;
  const double absB = absT + nodeHeight;

  node->getLayout().setPosition(
      PhysicalEdge::Left,
      static_cast<float>(std::round(nodeLeft * factor) / factor));

  node->getLayout().setPosition(
      PhysicalEdge::Top,
      static_cast<float>(std::round(nodeTop * factor) / factor));

  const double roundedR = std::round(absR * factor);
  const double roundedL = std::round(absL * factor);
  const double roundedB = std::round(absB * factor);
  const double roundedT = std::round(absT * factor);

  node->getLayout().setDimension(
      Dimension::Width,
      static_cast<float>((roundedR - roundedL) / factor));

  node->getLayout().setDimension(
      Dimension::Height,
      static_cast<float>((roundedB - roundedT) / factor));

  for (auto* child : node->getChildren()) {
    if (child && child->getOwner() == node) {
      quantizeLayoutMetrics(child, absL, absT);
    }
  }
}

} // namespace nisaba::layout

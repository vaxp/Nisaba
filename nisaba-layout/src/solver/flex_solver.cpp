#include <nisaba/layout/solver/flex_solver.hpp>
#include <nisaba/layout/solver/pipeline.hpp>

namespace nisaba::layout {

// Explicit linkage points if needed by external consumers
void solveFlexLayoutExplicit(
    Node* node,
    float ownerWidth,
    float ownerHeight,
    Direction ownerDirection) {
  LayoutPipeline::run(node, ownerWidth, ownerHeight, ownerDirection);
}

} // namespace nisaba::layout

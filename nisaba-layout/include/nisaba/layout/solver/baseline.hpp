#pragma once

#include <nisaba/layout/Layout.h>
#include <nisaba/layout/core/node.hpp>

namespace nisaba::layout {

// Computes the baseline offset from the top edge of the node
[[nodiscard]] float computeNodeBaseline(const nisaba::layout::Node* node);

// Checks whether any children participate in baseline alignment
[[nodiscard]] bool hasBaselineAlignment(const nisaba::layout::Node* node);

} // namespace nisaba::layout

#include <nisaba/layout/event/event.hpp>

namespace nisaba::layout {

const char* LayoutPassReasonToString(LayoutPassReason value) {
  static const char* const kReasonNames[] = {
    "initial",
    "abs_layout",
    "stretch",
    "multiline_stretch",
    "flex_layout",
    "measure",
    "abs_measure",
    "flex_measure",
    "grid_layout",
  };
  const auto idx = static_cast<size_t>(value);
  if (idx < sizeof(kReasonNames) / sizeof(kReasonNames[0])) {
    return kReasonNames[idx];
  }
  return "unknown";
}

} // namespace nisaba::layout

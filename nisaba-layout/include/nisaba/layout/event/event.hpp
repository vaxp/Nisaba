#pragma once

#include <cstdint>
#include <functional>
#include <nisaba/layout/core/types.hpp>

namespace nisaba::layout {

class Node;
class Config;

enum struct LayoutPhase : int {
  kLayout = 0,
  kMeasure = 1,
  kCachedLayout = 2,
  kCachedMeasure = 3
};

using LayoutType = LayoutPhase;

enum struct LayoutPassReason : int {
  kInitial = 0,
  kAbsLayout = 1,
  kStretch = 2,
  kMultilineStretch = 3,
  kFlexLayout = 4,
  kMeasureChild = 5,
  kAbsMeasureChild = 6,
  kFlexMeasure = 7,
  kGridLayout = 8,
  COUNT
};

const char* LayoutPassReasonToString(LayoutPassReason value);

/**
 * Sovereign layout event instrumentation stub.
 */
struct LAYOUT_EXPORT Event {
  enum Type {
    NodeAllocation,
    NodeDeallocation,
    NodeLayout,
    LayoutPassStart,
    LayoutPassEnd,
    MeasureCallbackStart,
    MeasureCallbackEnd,
    NodeBaselineStart,
    NodeBaselineEnd
  };

  struct NodeAllocationData { const Config* config; };
  struct NodeDeallocationData { const Config* config; };
  struct NodeLayoutData { LayoutType layoutType; void* layoutData; };
  struct LayoutPassStartData { const void* layoutData; };
  struct LayoutPassEndData { const void* layoutData; };
  struct MeasureCallbackEndData { const void* layoutData; float width; MeasureMode widthMode; float height; MeasureMode heightMode; float layoutWidth; float layoutHeight; };
  struct NodeBaselineEndData { const void* layoutData; };

  using Subscriber = void(const Node*, Type, void*);

  template <Type E, typename DataT>
  static void publish(const Node*, const DataT&) noexcept {}

  static void subscribe(std::function<Subscriber>) noexcept {}
  static void reset() noexcept {}
};

} // namespace nisaba::layout

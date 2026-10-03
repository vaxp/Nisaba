#pragma once

#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

#ifndef LAYOUT_EXPORT
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(LAYOUT_BUILD_DLL)
#define LAYOUT_EXPORT __declspec(dllexport)
#elif defined(LAYOUT_DLL)
#define LAYOUT_EXPORT __declspec(dllimport)
#else
#define LAYOUT_EXPORT
#endif
#else
#if defined(__GNUC__) && __GNUC__ >= 4
#define LAYOUT_EXPORT __attribute__((visibility("default")))
#else
#define LAYOUT_EXPORT
#endif
#endif
#endif

namespace nisaba::layout {

/**
 * IEEE NaN representation for undefined float values in layout calculation.
 */
inline constexpr float Undefined = std::numeric_limits<float>::quiet_NaN();
inline constexpr float UndefinedValue = Undefined;
inline constexpr float LayoutUndefined = Undefined; // Compatibility alias

[[nodiscard]] inline constexpr bool isUndefined(float val) noexcept {
  return std::isnan(val);
}

[[nodiscard]] inline constexpr bool isDefined(float val) noexcept {
  return val == val;
}

/**
 * Modern 2D point representation for Nisaba Layout.
 */
struct Point {
  float x{0.0f};
  float y{0.0f};

  constexpr bool operator==(const Point&) const noexcept = default;
};

/**
 * Modern 2D size representation for Nisaba Layout.
 */
struct Size {
  float width{Undefined};
  float height{Undefined};

  constexpr bool operator==(const Size&) const noexcept = default;
};

using LayoutSize = Size; // Compatibility alias

/**
 * Modern 2D bounding rectangle.
 */
struct Rect {
  float x{0.0f};
  float y{0.0f};
  float width{0.0f};
  float height{0.0f};

  [[nodiscard]] constexpr float left() const noexcept { return x; }
  [[nodiscard]] constexpr float top() const noexcept { return y; }
  [[nodiscard]] constexpr float right() const noexcept { return x + width; }
  [[nodiscard]] constexpr float bottom() const noexcept { return y + height; }

  constexpr bool operator==(const Rect&) const noexcept = default;
};

/**
 * 4-edge layout offsets (margins, padding, border widths, insets).
 */
struct FloatEdges {
  float left{0.0f};
  float top{0.0f};
  float right{0.0f};
  float bottom{0.0f};

  constexpr bool operator==(const FloatEdges&) const noexcept = default;
};

// Forward declarations
class Node;
class Config;
enum class MeasureMode : uint8_t;
enum class LogLevel : uint8_t;

// Pure C++ Callback Types
using MeasureCallback = Size (*)(
    const Node* node,
    float width,
    MeasureMode widthMode,
    float height,
    MeasureMode heightMode);

using MinContentMeasureCallback = Size (*)(
    const Node* node,
    float width,
    MeasureMode widthMode,
    float height,
    MeasureMode heightMode);

using BaselineCallback = float (*)(const Node* node, float width, float height);
using DirtiedCallback = void (*)(const Node* node);
using LoggerCallback = int (*)(
    const Config* config,
    const Node* node,
    LogLevel level,
    const char* format,
    va_list args);
using CloneNodeCallback = Node* (*)(const Node* oldNode, const Node* owner, size_t childIndex);

// Backward compatibility typedefs for smooth transition
using LayoutMeasureFunc = MeasureCallback;
using LayoutMinContentMeasureFunc = MinContentMeasureCallback;
using LayoutBaselineFunc = BaselineCallback;
using LayoutDirtiedFunc = DirtiedCallback;
using LayoutLogger = LoggerCallback;
using LayoutCloneNodeFunc = CloneNodeCallback;
using LayoutNodeRef = Node*;
using LayoutNodeConstRef = const Node*;
using LayoutConfigRef = Config*;
using LayoutConfigConstRef = const Config*;

} // namespace nisaba::layout

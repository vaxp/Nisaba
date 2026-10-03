#pragma once

#include <cstdint>
#include <array>
#include <span>
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"

namespace nisaba {
namespace line_clipper {

inline constexpr size_t MAX_POINTS = 4;

/// Clips line src[0]..src[1] against clip rect.
/// Returns number of points written into `points` array (0 to 4).
size_t clip(
    const std::array<Point, 2>& src,
    const Rect& clip,
    bool can_cull_to_the_right,
    std::array<Point, MAX_POINTS>& points
) noexcept;

/// Intersect the line segment against the rect.
/// Returns true if there is a non-empty resulting segment stored in dst.
bool intersect(
    const std::array<Point, 2>& src,
    const Rect& clip,
    std::array<Point, 2>& dst
) noexcept;

} // namespace line_clipper
} // namespace nisaba

#pragma once

#include <span>
#include <string_view>
#include "nisaba/math/point.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/canvas/paint.hpp"

namespace nisaba {

/// Represents an positioned glyph with an 8-bit alpha mask for high-performance text rendering.
struct GlyphRun {
    Point position;
    MaskRef mask;
};

} // namespace nisaba

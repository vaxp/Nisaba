#pragma once

#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/transform4x4.hpp"
#include "nisaba/color/blend_mode.hpp"

namespace nisaba::raster {

/// Renders a source Pixmap mapped onto an arbitrary destination quadrilateral
/// [p0, p1, p2, p3] (in clockwise order) with perspective-correct scanline interpolation
/// and bilinear filtering.
void draw_pixmap_perspective_quad(
    PixmapMut dst,
    const PixmapRef& src,
    Point p0, Point p1, Point p2, Point p3,
    float opacity = 1.0f,
    BlendMode blend_mode = BlendMode::SourceOver
);

/// Renders a source Pixmap region under a full 3D Transform4x4 projective matrix
/// onto the destination surface with perspective correction and bilinear sampling.
void draw_pixmap_3d(
    PixmapMut dst,
    const PixmapRef& src,
    const Rect& src_rect,
    const Transform4x4& transform3d,
    float opacity = 1.0f,
    BlendMode blend_mode = BlendMode::SourceOver
);

} // namespace nisaba::raster

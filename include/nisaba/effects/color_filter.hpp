#pragma once

#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/color/color.hpp"

namespace nisaba::effects {

/// Fast in-place pixel-level color transformations for Pixmaps.
class ColorFilter {
public:
    /// Inverts RGB channels while preserving alpha.
    static void invert(PixmapMut& pixmap) noexcept;

    /// Converts to perceptual luminance grayscale while preserving alpha.
    static void grayscale(PixmapMut& pixmap) noexcept;

    /// Adjusts brightness in-place.
    /// factor in [-1.0, 1.0]: negative darkens towards black, positive lightens towards white.
    static void adjust_brightness(PixmapMut& pixmap, float factor) noexcept;

    /// Adjusts contrast in-place.
    /// factor > 1.0 increases contrast, factor < 1.0 decreases contrast.
    static void adjust_contrast(PixmapMut& pixmap, float factor) noexcept;

    /// Applies a warm nostalgic sepia tone.
    static void sepia(PixmapMut& pixmap) noexcept;

    /// Tints the pixmap with a solid color according to intensity [0.0, 1.0].
    static void tint(PixmapMut& pixmap, Color tint_color, float intensity) noexcept;

    /// Applies a 4x5 ColorMatrix filter in-place to the pixmap.
    static void apply_matrix(PixmapMut& pixmap, const class ColorMatrix& matrix) noexcept;
};

} // namespace nisaba::effects

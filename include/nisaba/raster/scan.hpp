#pragma once

#include "nisaba/path/path.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/raster/blitter.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba {

namespace scan {

/// Rasterizes a path into a Blitter with or without anti-aliasing.
void fill_path(
    const Path& path,
    FillRule fill_rule,
    const ScreenIntRect& clip,
    bool anti_alias,
    Blitter& blitter
);

/// Rasterizes a path into a Blitter with an inline affine transform applied
/// during rasterization, avoiding an intermediate transformed path allocation.
void fill_path(
    const Path& path,
    FillRule fill_rule,
    const ScreenIntRect& clip,
    bool anti_alias,
    Blitter& blitter,
    const Transform& transform
);

} // namespace scan

/// A solid color blitter for direct rasterization into a PixmapMut.
class SolidColorBlitter : public Blitter {
public:
    SolidColorBlitter(PixmapMut& pixmap, Color color) noexcept;

    void blit_h(uint32_t x, uint32_t y, LengthU32 width) override;
    void blit_anti_h(
        uint32_t x,
        uint32_t y,
        std::span<AlphaU8> antialias,
        std::span<AlphaRun> runs
    ) override;
    void blit_span_coverage(
        uint32_t x,
        uint32_t y,
        const uint8_t* coverage,
        uint32_t count
    ) override;
    void blit_rect(const ScreenIntRect& rect) override;

private:
    PixmapMut& pixmap_;
    PremultipliedColorU8 color_;
    ColorU8 src_color_;
};

} // namespace nisaba

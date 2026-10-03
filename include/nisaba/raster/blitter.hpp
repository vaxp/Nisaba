#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include "nisaba/types.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/math/screen_int_rect.hpp"

namespace nisaba {

using AlphaRun = std::optional<uint16_t>;

/// Describes alpha bitmaps for mask blitting.
struct MaskInfo {
    uint8_t image[2]{0, 0};
    ScreenIntRect bounds;
    uint32_t row_bytes{0};
};

/// Blitter is responsible for writing pixels into destination memory.
///
/// Coordinates passed to blit_* calls are in destination pixel space.
class Blitter {
public:
    virtual ~Blitter() = default;

    /// Blits a horizontal run of one or more pixels.
    virtual void blit_h(uint32_t x, uint32_t y, LengthU32 width) {
        (void)x; (void)y; (void)width;
    }

    /// Blits a horizontal run of antialiased pixels using sparse RLE.
    virtual void blit_anti_h(
        uint32_t x,
        uint32_t y,
        std::span<AlphaU8> antialias,
        std::span<AlphaRun> runs
    ) {
        (void)x; (void)y; (void)antialias; (void)runs;
    }

    /// Blits a horizontal span of anti-aliased pixels with dense coverage buffer.
    virtual void blit_span_coverage(
        uint32_t x,
        uint32_t y,
        const uint8_t* coverage,
        uint32_t count
    ) {
        (void)x; (void)y; (void)coverage; (void)count;
    }

    /// Blits a vertical run of pixels with a constant alpha value.
    virtual void blit_v(uint32_t x, uint32_t y, LengthU32 height, AlphaU8 alpha) {
        (void)x; (void)y; (void)height; (void)alpha;
    }

    virtual void blit_anti_h2(uint32_t x, uint32_t y, AlphaU8 alpha0, AlphaU8 alpha1) {
        (void)x; (void)y; (void)alpha0; (void)alpha1;
    }

    virtual void blit_anti_v2(uint32_t x, uint32_t y, AlphaU8 alpha0, AlphaU8 alpha1) {
        (void)x; (void)y; (void)alpha0; (void)alpha1;
    }

    /// Blits a solid rectangle one or more pixels wide.
    virtual void blit_rect(const ScreenIntRect& rect) {
        (void)rect;
    }

    /// Blits a pattern of pixels defined by a mask.
    virtual void blit_mask(const MaskInfo& mask, const ScreenIntRect& clip) {
        (void)mask; (void)clip;
    }
};

} // namespace nisaba

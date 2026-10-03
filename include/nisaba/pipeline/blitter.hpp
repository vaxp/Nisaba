#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include "nisaba/types.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/color/blend_mode.hpp"
#include "nisaba/raster/blitter.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/canvas/paint.hpp"

namespace nisaba {

class PipelineBlitter final : public Blitter {
public:
    static std::optional<PipelineBlitter> create(
        const Paint& paint,
        std::optional<SubMaskRef> mask,
        SubPixmapMut& pixmap
    );

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
    void blit_v(uint32_t x, uint32_t y, LengthU32 height, AlphaU8 alpha) override;
    void blit_anti_h2(uint32_t x, uint32_t y, AlphaU8 alpha0, AlphaU8 alpha1) override;
    void blit_anti_v2(uint32_t x, uint32_t y, AlphaU8 alpha0, AlphaU8 alpha1) override;
    void blit_rect(const ScreenIntRect& rect) override;
    void blit_mask(const MaskInfo& mask, const ScreenIntRect& clip) override;

    void blend_span(uint32_t x, uint32_t y, uint32_t len, float coverage) noexcept;
    void blend_pixel(uint32_t x, uint32_t y, float coverage) noexcept;
    void blend_pixel_impl(uint32_t x, uint32_t y, float coverage, bool apply_mask) noexcept;

private:
    PipelineBlitter(
        const Paint& paint,
        std::optional<SubMaskRef> mask,
        SubPixmapMut& pixmap,
        BlendMode blend_mode,
        std::optional<PremultipliedColorU8> memset_color
    ) noexcept;

    Paint paint_;
    std::optional<SubMaskRef> mask_;
    SubPixmapMut& pixmap_;
    BlendMode blend_mode_{BlendMode::SourceOver};
    std::optional<PremultipliedColorU8> memset_color_{};
    PremultipliedColorU8 solid_color_pm_{};
    bool is_solid_{false};
    bool pre_scale_coverage_{false};
    uint32_t lut_src_[256]{};
    uint32_t lut_inv_a_[256]{};
};

} // namespace nisaba

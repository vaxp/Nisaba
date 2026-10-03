#include "nisaba/svg/baked_svg.hpp"
#include "nisaba/svg/svg_document.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/pipeline/simd.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::svg {

BakedSvg BakedSvg::bake(
    const SvgDocument& doc,
    uint32_t width,
    uint32_t height,
    std::optional<Color> tint
) {
    if (width == 0 || height == 0) {
        return BakedSvg();
    }

    auto pm_opt = Pixmap::allocate(width, height);
    if (!pm_opt.has_value()) {
        return BakedSvg();
    }

    auto& pm = *pm_opt;
    pm.fill(Color::TRANSPARENT);

    Canvas canvas(pm);
    auto target_rect = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    if (target_rect) {
        doc.render(canvas, &(*target_rect), tint);
    }

    BakedSvg baked;
    baked.pixmap_ = std::move(pm);
    baked.width_ = width;
    baked.height_ = height;
    baked.tint_ = tint;
    return baked;
}

void BakedSvg::draw(Canvas& canvas, float x, float y, float opacity) const {
    if (!is_valid() || opacity <= 0.001f) return;

    const auto& ts = canvas.transform();
    if (ts.is_identity() && canvas.current_mask() == nullptr) {
        int32_t gx = static_cast<int32_t>(std::round(x));
        int32_t gy = static_cast<int32_t>(std::round(y));
        uint32_t sw = width_;
        uint32_t sh = height_;

        int32_t dst_w = static_cast<int32_t>(canvas.width());
        int32_t dst_h = static_cast<int32_t>(canvas.height());

        // Quick rejection
        if (gx + static_cast<int32_t>(sw) <= 0 || gy + static_cast<int32_t>(sh) <= 0 ||
            gx >= dst_w || gy >= dst_h) {
            return;
        }

        // Scissor clip bounds
        int32_t clip_left = 0;
        int32_t clip_top = 0;
        int32_t clip_right = dst_w;
        int32_t clip_bottom = dst_h;

        const auto& sc = canvas.scissor_clip();
        if (sc.has_value()) {
            clip_left = std::max(0, static_cast<int32_t>(sc->x()));
            clip_top = std::max(0, static_cast<int32_t>(sc->y()));
            clip_right = std::min(dst_w, static_cast<int32_t>(sc->right()));
            clip_bottom = std::min(dst_h, static_cast<int32_t>(sc->bottom()));
        }

        int32_t x0 = std::max(gx, clip_left);
        int32_t y0 = std::max(gy, clip_top);
        int32_t x1 = std::min(gx + static_cast<int32_t>(sw), clip_right);
        int32_t y1 = std::min(gy + static_cast<int32_t>(sh), clip_bottom);

        if (x0 >= x1 || y0 >= y1) return;

        uint32_t span_len = static_cast<uint32_t>(x1 - x0);
        int32_t src_start_x = x0 - gx;
        int32_t src_start_y = y0 - gy;

        uint8_t op_u8 = static_cast<uint8_t>(std::clamp(opacity, 0.0f, 1.0f) * 255.0f + 0.5f);
        auto& dst_pm = canvas.pixmap();
        const auto* src_pixels = pixmap_->pixels();

        for (int32_t dy = y0; dy < y1; ++dy) {
            int32_t sy = src_start_y + (dy - y0);
            auto* dst_row = dst_pm.row(static_cast<size_t>(dy)) + x0;
            const auto* src_row = src_pixels + static_cast<size_t>(sy) * sw + src_start_x;

            if (op_u8 == 255) {
                simd::blend_source_over_span(dst_row, src_row, span_len);
            } else {
                simd::blend_source_over_span_coverage(dst_row, src_row, op_u8, span_len);
            }
        }
        return;
    }

    PixmapPaint paint;
    paint.opacity = opacity;
    canvas.draw_pixmap(
        static_cast<int32_t>(std::round(x)),
        static_cast<int32_t>(std::round(y)),
        pixmap_->as_ref(),
        paint
    );
}

void BakedSvg::draw(Canvas& canvas, const Rect& dest_bounds, float opacity) const {
    if (!is_valid()) return;

    if (std::abs(dest_bounds.width() - static_cast<float>(width_)) < 0.5f &&
        std::abs(dest_bounds.height() - static_cast<float>(height_)) < 0.5f) {
        draw(canvas, dest_bounds.left(), dest_bounds.top(), opacity);
    } else {
        canvas.save();
        float sx = dest_bounds.width() / static_cast<float>(width_);
        float sy = dest_bounds.height() / static_cast<float>(height_);
        canvas.translate(dest_bounds.left(), dest_bounds.top());
        canvas.scale(sx, sy);
        draw(canvas, 0.0f, 0.0f, opacity);
        canvas.restore();
    }
}

} // namespace nisaba::svg

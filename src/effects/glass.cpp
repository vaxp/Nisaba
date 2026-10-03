#include "nisaba/effects/glass.hpp"
#include "nisaba/effects/blur.hpp"
#include "nisaba/effects/drop_shadow.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/canvas/painter.hpp"
#include "nisaba/pipeline/simd.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::effects {

void draw_glass_panel(
    PixmapMut& dst,
    const Rect& rect,
    float rx,
    float ry,
    const GlassParams& params,
    Transform transform,
    const Mask* clip_mask
) {
    if (rect.width() <= 0.0f || rect.height() <= 0.0f) return;

    // 1. Draw elevation drop shadow if enabled
    if (params.shadow.has_value()) {
        draw_round_rect_shadow(dst, rect, rx, ry, *params.shadow, transform, clip_mask);
    }

    // 2. Build rounded rectangle path
    auto rounded_opt = PathBuilder::from_rounded_rect(rect, rx, ry);
    if (!rounded_opt) return;

    const Path* rounded_path = &*rounded_opt;
    std::optional<Path> transformed;
    if (!transform.is_identity()) {
        transformed = rounded_path->transform(transform);
        if (!transformed) return;
        rounded_path = &*transformed;
    }

    auto bounds = rounded_path->bounds();
    int32_t bx = std::max(0, static_cast<int32_t>(std::floor(bounds.x())));
    int32_t by = std::max(0, static_cast<int32_t>(std::floor(bounds.y())));
    int32_t br = std::min(static_cast<int32_t>(dst.width()), static_cast<int32_t>(std::ceil(bounds.right())));
    int32_t bb = std::min(static_cast<int32_t>(dst.height()), static_cast<int32_t>(std::ceil(bounds.bottom())));

    if (br <= bx || bb <= by) return;

    int32_t bw = br - bx;
    int32_t bh = bb - by;

    // 3. Extract and blur background region (Backdrop Blur)
    int32_t pad = (params.blur_sigma > 0.0f)
        ? static_cast<int32_t>(std::ceil(params.blur_sigma * 2.0f)) + 1
        : 0;

    int32_t px0 = std::max(0, bx - pad);
    int32_t py0 = std::max(0, by - pad);
    int32_t px1 = std::min(static_cast<int32_t>(dst.width()), bx + bw + pad);
    int32_t py1 = std::min(static_cast<int32_t>(dst.height()), by + bh + pad);

    int32_t pw = px1 - px0;
    int32_t ph = py1 - py0;

    if (pw > 0 && ph > 0 && params.blur_sigma > 0.0f) {
        auto temp = Pixmap::allocate(static_cast<uint32_t>(pw), static_cast<uint32_t>(ph));
        if (temp) {
            auto temp_mut = temp->as_mut();
            for (int32_t y = 0; y < ph; ++y) {
                const auto* src_row = dst.row(static_cast<size_t>(py0 + y)) + px0;
                auto* dst_row = temp_mut.row(static_cast<size_t>(y));
                std::copy_n(src_row, static_cast<size_t>(pw), dst_row);
            }

            gaussian_blur_pixmap(temp_mut, params.blur_sigma);

            // Rasterize card mask locally to clip the blurred backdrop
            auto card_mask = Mask::allocate(static_cast<uint32_t>(bw), static_cast<uint32_t>(bh));
            if (card_mask) {
                Transform local_ts = Transform::from_translate(static_cast<float>(-bx), static_cast<float>(-by));
                card_mask->fill_path(*rounded_path, FillRule::Winding, true, local_ts);

                for (int32_t my = 0; my < bh; ++my) {
                    int32_t dy = by + my;
                    if (dy < 0 || dy >= static_cast<int32_t>(dst.height())) continue;

                    auto* dst_row = dst.row(static_cast<size_t>(dy));
                    int32_t ty = dy - py0;
                    if (ty < 0 || ty >= ph) continue;

                    const auto* blurred_row = temp_mut.row(static_cast<size_t>(ty));

                    for (int32_t mx = 0; mx < bw; ++mx) {
                        int32_t dx = bx + mx;
                        if (dx < 0 || dx >= static_cast<int32_t>(dst.width())) continue;

                        int32_t tx = dx - px0;
                        if (tx < 0 || tx >= pw) continue;

                        uint8_t cov = card_mask->get(static_cast<uint32_t>(mx), static_cast<uint32_t>(my));
                        if (cov == 0) continue;

                        if (clip_mask != nullptr) {
                            uint8_t clip_cov = clip_mask->get(static_cast<uint32_t>(dx), static_cast<uint32_t>(dy));
                            if (clip_cov == 0) continue;
                            cov = static_cast<uint8_t>((static_cast<uint32_t>(cov) * clip_cov + 128u) / 255u);
                        }

                        const auto& blurred_px = blurred_row[tx];
                        auto& target_px = dst_row[dx];

                        if (cov == 255) {
                            target_px = blurred_px;
                        } else {
                            uint32_t inv = 255u - cov;
                            target_px.r = static_cast<uint8_t>((target_px.r * inv + blurred_px.r * cov + 128u) / 255u);
                            target_px.g = static_cast<uint8_t>((target_px.g * inv + blurred_px.g * cov + 128u) / 255u);
                            target_px.b = static_cast<uint8_t>((target_px.b * inv + blurred_px.b * cov + 128u) / 255u);
                            target_px.a = static_cast<uint8_t>((target_px.a * inv + blurred_px.a * cov + 128u) / 255u);
                        }
                    }
                }
            }
        }
    }

    // 4. Fill translucent frost tint overlay
    if (params.tint_color.alpha_norm() > 0.0f) {
        Paint tint_paint(params.tint_color);
        painter::fill_path(dst, *rounded_path, tint_paint, FillRule::Winding, Transform(), clip_mask);
    }

    // 5. Stroke edge reflection border
    if (params.border_width > 0.0f && params.border_color.alpha_norm() > 0.0f) {
        Stroke stroke;
        stroke.width = params.border_width;
        painter::stroke_path(dst, *rounded_path, Paint(params.border_color), stroke, Transform(), clip_mask);
    }
}

} // namespace nisaba::effects

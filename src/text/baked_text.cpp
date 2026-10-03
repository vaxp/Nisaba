#include "nisaba/text/baked_text.hpp"
#include "nisaba/text/buffer.hpp"
#include "nisaba/canvas/canvas.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>

namespace nisaba::text {

BakedText BakedText::from_buffer(
    Buffer& buffer,
    FontSystem& font_system,
    GlyphCache& cache,
    Color default_color
) {
    buffer.shape_until_scroll(font_system);
    auto runs = buffer.layout_runs();

    if (runs.empty()) {
        return BakedText();
    }

    float min_x = std::numeric_limits<float>::max();
    float min_y = std::numeric_limits<float>::max();
    float max_x = std::numeric_limits<float>::lowest();
    float max_y = std::numeric_limits<float>::lowest();
    bool has_glyphs = false;

    float font_size = buffer.metrics().font_size;

    for (const auto& run : runs) {
        float line_baseline = run.line_top + font_size;

        for (const auto& glyph : run.glyphs) {
            if (glyph.glyph_id == 0) continue;

            const TtfFont* font = font_system.get_font(glyph.font_id);
            if (!font) continue;

            const CachedGlyph* cached = cache.get_or_render(
                *font,
                glyph.font_id,
                glyph.glyph_id,
                glyph.font_size,
                0.0f,
                0.0f
            );

            if (!cached || cached->data.empty()) continue;

            float gx = glyph.x + static_cast<float>(cached->offset_x);
            float gy = line_baseline + glyph.y + static_cast<float>(cached->offset_y);
            float gw = static_cast<float>(cached->width);
            float gh = static_cast<float>(cached->height);

            min_x = std::min(min_x, gx);
            min_y = std::min(min_y, gy);
            max_x = std::max(max_x, gx + gw);
            max_y = std::max(max_y, gy + gh);
            has_glyphs = true;
        }
    }

    if (!has_glyphs || max_x <= min_x || max_y <= min_y) {
        return BakedText();
    }

    int32_t origin_x = static_cast<int32_t>(std::floor(min_x));
    int32_t origin_y = static_cast<int32_t>(std::floor(min_y));
    uint32_t surf_w = static_cast<uint32_t>(std::ceil(max_x) - origin_x + 1);
    uint32_t surf_h = static_cast<uint32_t>(std::ceil(max_y) - origin_y + 1);

    surf_w = std::max(1u, surf_w);
    surf_h = std::max(1u, surf_h);

    auto pixmap = Pixmap::allocate(surf_w, surf_h);
    if (!pixmap) {
        return BakedText();
    }

    // Zero out transparent background
    pixmap->fill(Color::TRANSPARENT);

    // Rasterize all buffer runs into the allocated surface
    Canvas local_canvas(*pixmap);
    buffer.draw(
        local_canvas,
        cache,
        font_system,
        default_color,
        -static_cast<float>(origin_x),
        -static_cast<float>(origin_y)
    );

    float layout_w = max_x - min_x;
    float layout_h = buffer.total_height();
    float baseline_y = font_size - static_cast<float>(origin_y);

    return BakedText(
        std::make_shared<Pixmap>(std::move(*pixmap)),
        layout_w,
        layout_h,
        origin_x,
        origin_y,
        baseline_y
    );
}

BakedText BakedText::bake(
    FontSystem& font_system,
    GlyphCache& cache,
    std::string_view text,
    float font_size,
    Color color,
    Weight weight,
    bool monospace,
    float line_height_factor
) {
    if (text.empty()) {
        return BakedText();
    }

    Buffer buf(Metrics(font_size, font_size * line_height_factor));
    Attrs attrs;
    attrs.set_color(TextColor::rgba(
        static_cast<uint8_t>(color.red() * 255.0f + 0.5f),
        static_cast<uint8_t>(color.green() * 255.0f + 0.5f),
        static_cast<uint8_t>(color.blue() * 255.0f + 0.5f),
        static_cast<uint8_t>(color.alpha() * 255.0f + 0.5f)
    ));

    if (monospace) {
        attrs.set_family(Family::monospace());
    } else {
        attrs.set_family(Family::sans_serif());
    }

    attrs.set_weight(weight);
    buf.set_text(text, attrs);

    return from_buffer(buf, font_system, cache, color);
}

} // namespace nisaba::text

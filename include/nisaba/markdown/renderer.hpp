#pragma once

#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/canvas_interface.hpp"
#include "nisaba/markdown/document.hpp"
#include "nisaba/markdown/style.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/glyph_cache.hpp"

namespace nisaba::markdown {

/// High-performance renderer for Markdown documents onto CPU Canvas and GPU GpuCanvas.
class MarkdownRenderer {
public:
    MarkdownRenderer() = default;

    /// Renders a markdown document onto a software Canvas.
    static void render(
        Canvas& canvas,
        MarkdownDocument& doc,
        float x,
        float y,
        float width,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache,
        float scroll_y = 0.0f
    );

    /// Renders a markdown document onto any polymorphic ICanvas (CPU or GPU).
    static void render(
        ICanvas& canvas,
        MarkdownDocument& doc,
        float x,
        float y,
        float width,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache,
        float scroll_y = 0.0f
    );

private:
    static void render_block(
        ICanvas& canvas,
        const BlockNode& block,
        float x,
        float y,
        float width,
        const MarkdownStyle& style,
        text::FontSystem& font_system,
        text::GlyphCache& glyph_cache
    );
};

} // namespace nisaba::markdown

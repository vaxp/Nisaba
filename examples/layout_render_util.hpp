#pragma once

#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include "nisaba/nisaba.hpp"

namespace layout_example {

using LayoutEdge = nisaba::layout::Edge;

// Curated aesthetic palette for layout visualization
inline const std::vector<nisaba::Color> kItemColors = {
    nisaba::Color::from_rgba8(240, 95, 80, 230),   // Coral Red
    nisaba::Color::from_rgba8(45, 185, 175, 230),  // Teal
    nisaba::Color::from_rgba8(135, 95, 235, 230),  // Indigo
    nisaba::Color::from_rgba8(245, 180, 45, 230),  // Warm Gold
    nisaba::Color::from_rgba8(45, 195, 120, 230),  // Emerald
    nisaba::Color::from_rgba8(235, 70, 140, 230),  // Rose
    nisaba::Color::from_rgba8(80, 150, 245, 230),  // Sky Blue
    nisaba::Color::from_rgba8(175, 80, 220, 230)   // Violet
};

// Singleton Text Renderer utilizing Nisaba's Sovereign Vector Text Engine
class TextRenderer {
public:
    static TextRenderer& instance() {
        static TextRenderer renderer;
        return renderer;
    }

    void draw_text(
        nisaba::Canvas& canvas,
        std::string_view text,
        float x,
        float y,
        float font_size = 14.0f,
        nisaba::Color color = nisaba::Color::WHITE) {
        if (!initialized_) {
            init();
        }
        if (!font_loaded_) return;

        nisaba::text::Buffer buf(nisaba::text::Metrics(font_size, font_size * 1.3f));
        nisaba::text::Attrs attrs;
        attrs.set_color(nisaba::text::TextColor::rgba(
            static_cast<uint8_t>(color.red() * 255.0f),
            static_cast<uint8_t>(color.green() * 255.0f),
            static_cast<uint8_t>(color.blue() * 255.0f),
            static_cast<uint8_t>(color.alpha() * 255.0f)
        ));
        buf.set_text(text, attrs);
        buf.draw(canvas, cache_, font_system_, color, x, y);
    }

private:
    TextRenderer() = default;

    void init() {
        initialized_ = true;
        std::vector<std::string> candidates = {
            "fonts/Inter-Regular.ttf",
            "../fonts/Inter-Regular.ttf",
            "../../fonts/Inter-Regular.ttf"
        };
        for (const auto& path : candidates) {
            std::ifstream f(path, std::ios::binary);
            if (f.good()) {
                font_system_.load_font_file(path);
                font_loaded_ = true;
                break;
            }
        }
    }

    nisaba::text::FontSystem font_system_;
    nisaba::text::GlyphCache cache_;
    bool initialized_{false};
    bool font_loaded_{false};
};

inline void draw_label(
    nisaba::Canvas& canvas,
    std::string_view text,
    float x,
    float y,
    float font_size = 14.0f,
    nisaba::Color color = nisaba::Color::WHITE) {
    TextRenderer::instance().draw_text(canvas, text, x, y, font_size, color);
}

inline void draw_background(nisaba::Canvas& canvas, uint32_t width, uint32_t height) {
    std::vector<nisaba::GradientStop> stops = {
        nisaba::GradientStop::create(0.0f, nisaba::Color::from_rgba8(15, 18, 28, 255)),
        nisaba::GradientStop::create(1.0f, nisaba::Color::from_rgba8(8, 10, 16, 255))
    };
    auto grad = nisaba::LinearGradient::create(
        nisaba::Point::from_xy(0.0f, 0.0f),
        nisaba::Point::from_xy(0.0f, static_cast<float>(height)),
        stops
    );
    nisaba::Paint bg_paint;
    if (grad) {
        bg_paint.shader = nisaba::Shader(*grad);
    } else {
        bg_paint.set_color_rgba8(15, 18, 28, 255);
    }
    canvas.fill_rect(*nisaba::Rect::from_xywh(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)), bg_paint);
}

inline void render_layout_tree(
    nisaba::Canvas& canvas,
    const nisaba::layout::Node* node,
    float parentAbsLeft = 0.0f,
    float parentAbsTop = 0.0f,
    int depth = 0,
    int itemIndex = 0,
    bool showChildLabels = true) {
    if (!node) return;

    const float left = parentAbsLeft + node->getLayout().position(nisaba::layout::PhysicalEdge::Left);
    const float top = parentAbsTop + node->getLayout().position(nisaba::layout::PhysicalEdge::Top);
    const float width = node->getLayout().dimension(nisaba::layout::Dimension::Width);
    const float height = node->getLayout().dimension(nisaba::layout::Dimension::Height);

    if (depth == 0) {
        // Container box (outer frame)
        nisaba::Paint container_bg;
        container_bg.set_color_rgba8(24, 30, 46, 220);
        canvas.fill_rect(*nisaba::Rect::from_xywh(left, top, width, height), container_bg);

        nisaba::Paint container_border;
        container_border.set_color_rgba8(65, 85, 130, 255);
        nisaba::Stroke stroke(2.0f);
        canvas.stroke_rect(*nisaba::Rect::from_xywh(left, top, width, height), container_border, stroke);
    } else {
        // Child item box
        nisaba::Color fill_color = kItemColors[itemIndex % kItemColors.size()];
        nisaba::Paint item_paint;
        item_paint.set_color(fill_color);
        canvas.fill_rect(*nisaba::Rect::from_xywh(left, top, width, height), item_paint);

        // Highlight border
        nisaba::Paint border_paint;
        border_paint.set_color_rgba8(255, 255, 255, 180);
        nisaba::Stroke stroke(1.5f);
        canvas.stroke_rect(*nisaba::Rect::from_xywh(left, top, width, height), border_paint, stroke);

        // Inner shadow / accent line on top edge
        nisaba::Paint accent_paint;
        accent_paint.set_color_rgba8(255, 255, 255, 80);
        canvas.fill_rect(*nisaba::Rect::from_xywh(left + 2.0f, top + 2.0f, std::max(0.0f, width - 4.0f), 3.0f), accent_paint);

        // Vector text label inside child item
        if (showChildLabels && width >= 30.0f && height >= 20.0f) {
            std::string label = "Item " + std::to_string(itemIndex + 1);
            if (width < 60.0f) {
                label = std::to_string(itemIndex + 1);
            }
            draw_label(canvas, label, left + 8.0f, top + 6.0f, 12.0f, nisaba::Color::WHITE);

            if (width >= 70.0f && height >= 42.0f) {
                std::string dim_str = std::to_string(static_cast<int>(width)) + "x" + std::to_string(static_cast<int>(height));
                draw_label(canvas, dim_str, left + 8.0f, top + height - 18.0f, 10.0f, nisaba::Color::from_rgba8(255, 255, 255, 175));
            }
        }
    }

    int childIdx = 0;
    for (const auto* child : node->getChildren()) {
        render_layout_tree(canvas, child, left, top, depth + 1, childIdx++, showChildLabels);
    }
}

inline void print_layout_metrics(const nisaba::layout::Node* node, const std::string& name, int depth = 0) {
    if (!node) return;
    std::string indent(depth * 3, ' ');
    const float left = node->getLayout().position(nisaba::layout::PhysicalEdge::Left);
    const float top = node->getLayout().position(nisaba::layout::PhysicalEdge::Top);
    const float w = node->getLayout().dimension(nisaba::layout::Dimension::Width);
    const float h = node->getLayout().dimension(nisaba::layout::Dimension::Height);

    std::cout << indent << name << " -> [x: " << std::fixed << std::setprecision(1) << left
              << ", y: " << top << ", w: " << w << ", h: " << h << "]" << std::endl;

    int idx = 0;
    for (const auto* child : node->getChildren()) {
        print_layout_metrics(child, "Item " + std::to_string(idx++), depth + 1);
    }
}

} // namespace layout_example

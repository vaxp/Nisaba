#pragma once

#include <memory>
#include <vector>
#include <optional>
#include "nisaba/gpu/types.hpp"
#include "nisaba/gpu/context.hpp"
#include "nisaba/gpu/gpu_types.hpp"
#include "nisaba/gpu/gpu_surface.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/mesh/mesh.hpp"
#include "nisaba/effects/glass.hpp"
#include "nisaba/effects/drop_shadow.hpp"
#include "nisaba/svg/svg_document.hpp"
#include "nisaba/canvas/canvas_interface.hpp"
#include "nisaba/text/font_system.hpp"
#include "nisaba/text/buffer.hpp"
#include "nisaba/text/ttf_font.hpp"

namespace nisaba::gpu {

struct GpuCanvasState {
    nisaba::Transform transform{};
    std::optional<ScreenIntRect> scissor_clip{};
};

/// High-level, hardware-accelerated GPU Canvas bridging Nisaba 2D to the sovereign C++20 GPU engine.
class GpuCanvas : public ICanvas {
public:
    explicit GpuCanvas(std::shared_ptr<GpuSurface> surface);
    ~GpuCanvas();

    [[nodiscard]] uint32_t width() const noexcept { return surface_ ? surface_->width() : 0; }
    [[nodiscard]] uint32_t height() const noexcept { return surface_ ? surface_->height() : 0; }
    [[nodiscard]] std::shared_ptr<GpuSurface> surface() const noexcept { return surface_; }
    [[nodiscard]] Context* context() const noexcept { return context_.get(); }

    // Surface operations
    void clear(nisaba::Color color = nisaba::Color::TRANSPARENT);

    // State stack
    void save() override;
    void restore() override;
    void reset_transform() noexcept;
    void set_transform(const nisaba::Transform& ts) noexcept;
    [[nodiscard]] const nisaba::Transform& transform() const noexcept { return current_transform_; }

    void translate(float dx, float dy) noexcept override;
    void scale(float sx, float sy) noexcept override;
    void rotate(float degrees) noexcept;
    void concat(const nisaba::Transform& ts) noexcept override;

    // Clipping
    void clip_rect(const nisaba::Rect& rect);
    void clip_rect(const nisaba::ScreenIntRect& rect);
    void reset_clip() noexcept;
    [[nodiscard]] const std::optional<ScreenIntRect>& scissor_clip() const noexcept { return scissor_clip_; }

    // Fast localized clears for damage tracking
    void clear_rect(const nisaba::Rect& rect, nisaba::Color color);
    void clear_rect(const nisaba::ScreenIntRect& rect, nisaba::Color color);

    // Drawing primitives
    void fill_rect(const nisaba::Rect& rect, const nisaba::Paint& paint) override;
    void stroke_rect(const nisaba::Rect& rect, const nisaba::Paint& paint, const nisaba::Stroke& stroke);

    void fill_round_rect(const nisaba::Rect& rect, float rx, float ry, const nisaba::Paint& paint);
    void stroke_round_rect(const nisaba::Rect& rect, float rx, float ry, const nisaba::Paint& paint, const nisaba::Stroke& stroke);

    void fill_circle(float cx, float cy, float radius, const nisaba::Paint& paint);
    void stroke_circle(float cx, float cy, float radius, const nisaba::Paint& paint, const nisaba::Stroke& stroke);

    void stroke_line(nisaba::Point p0, nisaba::Point p1, const nisaba::Paint& paint, const nisaba::Stroke& stroke);
    void stroke_line(float x0, float y0, float x1, float y1, const nisaba::Paint& paint, const nisaba::Stroke& stroke);

    void fill_path(const nisaba::Path& path, const nisaba::Paint& paint, nisaba::FillRule fill_rule = nisaba::FillRule::Winding) override;
    void stroke_path(const nisaba::Path& path, const nisaba::Paint& paint, const nisaba::Stroke& stroke) override;

    void draw_pixmap(int32_t x, int32_t y, const PixmapRef& src, const PixmapPaint& paint = PixmapPaint());
    void draw_gpu_texture(int32_t x, int32_t y, const std::shared_ptr<GpuTexture>& texture, const PixmapPaint& paint = PixmapPaint());

    void draw_vertices(const nisaba::Vertices& vertices, BlendMode blend_mode = BlendMode::SourceOver, float opacity = 1.0f);
    void draw_textured_vertices(const nisaba::Vertices& vertices, const std::shared_ptr<GpuTexture>& texture, BlendMode blend_mode = BlendMode::SourceOver, float opacity = 1.0f);

    // --- Effects & Glassmorphism ---
    void draw_round_rect_shadow(const nisaba::Rect& rect, float rx, float ry, const effects::DropShadow& shadow);
    void draw_rect_shadow(const nisaba::Rect& rect, const effects::DropShadow& shadow);
    void draw_glow(const nisaba::Rect& rect, float rx, float ry, nisaba::Color glow_color, float radius);
    void draw_glass_panel(const nisaba::Rect& rect, float rx, float ry, const effects::GlassParams& params);

    // --- SVG Vector Graphics ---
    void draw_svg(const svg::SvgDocument& doc, const nisaba::Rect& dest_bounds);

    // --- Typography & Hardware Text Rendering ---
    void draw_text(std::string_view text, float x, float y, const text::Font& font, const nisaba::Paint& paint, float font_size = 16.0f);
    void draw_text(float x, float y, std::string_view text, const nisaba::Paint& paint, float font_size = 16.0f, const char* font_name = nullptr);
    void draw_text_buffer(const text::Buffer& buffer, text::FontSystem& fonts, Point pos = Point{0.0f, 0.0f}, nisaba::Color default_color = nisaba::Color::BLACK);
    void draw_text_buffer(const text::Buffer& buffer, text::FontSystem& fonts, Point pos, const nisaba::Paint& paint);

    int create_font(const char* name, const char* filename);
    int create_font_mem(const char* name, const uint8_t* data, size_t size);
    [[nodiscard]] text::FontSystem* font_system() noexcept { return context_ ? &context_->fontSystem() : nullptr; }
    [[nodiscard]] const text::FontSystem* font_system() const noexcept { return context_ ? &context_->fontSystem() : nullptr; }

    /// Flushes all pending draw operations to the GPU.
    void flush();
    void flush(const ScreenIntRect& damage_rect);
    void flush(const std::vector<ScreenIntRect>& damage_rects);

private:
    std::shared_ptr<GpuSurface> surface_;
    std::unique_ptr<Context> context_;
    bool in_frame_{false};

    nisaba::Transform current_transform_{};
    std::optional<ScreenIntRect> scissor_clip_{};
    std::vector<GpuCanvasState> state_stack_{};
    std::vector<int> temp_images_{};

    void apply_fill_paint(const nisaba::Paint& paint);
    void apply_stroke(const nisaba::Stroke& stroke, const nisaba::Paint& paint);
    void ensure_frame();
};

} // namespace nisaba::gpu

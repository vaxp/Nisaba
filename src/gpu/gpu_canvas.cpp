#include "nisaba/gpu/gpu_canvas.hpp"
#include "nisaba/gpu/gl3_renderer.hpp"
#include "nisaba/path/path_builder.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <algorithm>
#include <cmath>

namespace nisaba::gpu {


GpuCanvas::GpuCanvas(std::shared_ptr<GpuSurface> surface)
    : surface_(std::move(surface)),
      scissor_clip_(surface_ ? ScreenIntRect::from_xywh(0, 0, surface_->width(), surface_->height()) : std::nullopt) {
    if (surface_) {
        surface_->bind();
        context_ = surface_->create_context(CreateFlags::Antialias | CreateFlags::StencilStrokes);
        if (context_) {
            context_->beginFrame(static_cast<float>(surface_->width()), static_cast<float>(surface_->height()), 1.0f);
            in_frame_ = true;
        }
    }
}

GpuCanvas::~GpuCanvas() {
    flush();
    context_.reset();
}

void GpuCanvas::ensure_frame() {
    if (context_ && !in_frame_ && surface_) {
        surface_->bind();
        context_->beginFrame(static_cast<float>(surface_->width()), static_cast<float>(surface_->height()), 1.0f);
        in_frame_ = true;
    }
}

void GpuCanvas::clear(nisaba::Color color) {
    if (context_ && in_frame_) {
        context_->cancelFrame();
        in_frame_ = false;
        for (int img : temp_images_) {
            context_->deleteImage(img);
        }
        temp_images_.clear();
    }
    if (surface_) {
        surface_->bind();
        surface_->clear(color);
    }
    ensure_frame();
}

void GpuCanvas::save() {
    if (context_) context_->save();
    state_stack_.push_back(GpuCanvasState{
        .transform = current_transform_,
        .scissor_clip = scissor_clip_
    });
}

void GpuCanvas::restore() {
    if (context_) context_->restore();
    if (!state_stack_.empty()) {
        current_transform_ = state_stack_.back().transform;
        scissor_clip_ = state_stack_.back().scissor_clip;
        state_stack_.pop_back();
    }
}

void GpuCanvas::reset_transform() noexcept {
    current_transform_ = nisaba::Transform();
    if (context_) context_->resetTransform();
}

void GpuCanvas::set_transform(const nisaba::Transform& ts) noexcept {
    current_transform_ = ts;
    if (context_) {
        context_->resetTransform();
        context_->transform(ts);
    }
}

void GpuCanvas::translate(float dx, float dy) noexcept {
    current_transform_ = current_transform_.pre_translate(dx, dy);
    if (context_) context_->translate(dx, dy);
}

void GpuCanvas::scale(float sx, float sy) noexcept {
    current_transform_ = current_transform_.pre_scale(sx, sy);
    if (context_) context_->scale(sx, sy);
}

void GpuCanvas::rotate(float degrees) noexcept {
    current_transform_ = current_transform_.pre_rotate(degrees);
    if (context_) context_->rotate(degrees / 180.0f * nisaba::gpu::PI);
}

void GpuCanvas::concat(const nisaba::Transform& ts) noexcept {
    current_transform_ = current_transform_.pre_concat(ts);
    if (context_) context_->transform(ts);
}

void GpuCanvas::clip_rect(const nisaba::Rect& rect) {
    if (context_) {
        context_->scissor(rect.x(), rect.y(), rect.width(), rect.height());
    }
    float x0 = rect.left() * current_transform_.sx + current_transform_.tx;
    float x1 = rect.right() * current_transform_.sx + current_transform_.tx;
    float y0 = rect.top() * current_transform_.sy + current_transform_.ty;
    float y1 = rect.bottom() * current_transform_.sy + current_transform_.ty;
    if (x0 > x1) std::swap(x0, x1);
    if (y0 > y1) std::swap(y0, y1);

    int32_t ix0 = std::clamp(static_cast<int32_t>(std::round(x0)), 0, static_cast<int32_t>(width()));
    int32_t iy0 = std::clamp(static_cast<int32_t>(std::round(y0)), 0, static_cast<int32_t>(height()));
    int32_t ix1 = std::clamp(static_cast<int32_t>(std::round(x1)), 0, static_cast<int32_t>(width()));
    int32_t iy1 = std::clamp(static_cast<int32_t>(std::round(y1)), 0, static_cast<int32_t>(height()));

    if (ix1 > ix0 && iy1 > iy0) {
        auto new_clip = ScreenIntRect::from_xywh(
            static_cast<uint32_t>(ix0), static_cast<uint32_t>(iy0),
            static_cast<uint32_t>(ix1 - ix0), static_cast<uint32_t>(iy1 - iy0)
        );
        if (new_clip) {
            scissor_clip_ = scissor_clip_.has_value() ? scissor_clip_->intersect(*new_clip) : new_clip;
            return;
        }
    }
    scissor_clip_ = std::nullopt;
}

void GpuCanvas::clip_rect(const nisaba::ScreenIntRect& rect) {
    if (context_) {
        context_->scissorScreen(static_cast<float>(rect.x()),
                                static_cast<float>(rect.y()),
                                static_cast<float>(rect.width()),
                                static_cast<float>(rect.height()));
    }
    if (scissor_clip_.has_value()) {
        scissor_clip_ = scissor_clip_->intersect(rect);
    } else {
        scissor_clip_ = rect;
    }
}

void GpuCanvas::clear_rect(const nisaba::Rect& rect, nisaba::Color color) {
    save();
    clip_rect(rect);
    fill_rect(rect, nisaba::Paint(color));
    restore();
}

void GpuCanvas::clear_rect(const nisaba::ScreenIntRect& rect, nisaba::Color color) {
    save();
    clip_rect(rect);
    fill_rect(*nisaba::Rect::from_xywh(rect.x(), rect.y(), rect.width(), rect.height()), nisaba::Paint(color));
    restore();
}

void GpuCanvas::reset_clip() noexcept {
    if (context_) context_->resetScissor();
    if (surface_) {
        scissor_clip_ = ScreenIntRect::from_xywh(0, 0, surface_->width(), surface_->height());
    } else {
        scissor_clip_ = std::nullopt;
    }
}

void GpuCanvas::apply_fill_paint(const nisaba::Paint& paint) {
    if (!context_) return;

    if (paint.shader.type() == Shader::Type::LinearGradient) {
        const auto& lg = paint.shader.linear_gradient();
        nisaba::Point p0 = lg.start();
        nisaba::Point p1 = lg.end();
        nisaba::Color c0 = lg.stops().empty() ? nisaba::Color::BLACK : lg.stops().front().color;
        nisaba::Color c1 = lg.stops().empty() ? nisaba::Color::WHITE : lg.stops().back().color;
        context_->fillPaint(context_->linearGradient(p0.x, p0.y, p1.x, p1.y, c0, c1));
        return;
    } else if (paint.shader.type() == Shader::Type::RadialGradient) {
        const auto& rg = paint.shader.radial_gradient();
        nisaba::Point center = rg.center();
        float radius = rg.radius();
        nisaba::Color c0 = rg.stops().empty() ? nisaba::Color::BLACK : rg.stops().front().color;
        nisaba::Color c1 = rg.stops().empty() ? nisaba::Color::WHITE : rg.stops().back().color;
        context_->fillPaint(context_->radialGradient(center.x, center.y, 0.0f, radius, c0, c1));
        return;
    }

    context_->fillColor(paint.shader.solid_color());
}

void GpuCanvas::apply_stroke(const nisaba::Stroke& stroke, const nisaba::Paint& paint) {
    if (!context_) return;
    context_->strokeWidth(stroke.width);
    context_->miterLimit(stroke.miter_limit);
    context_->lineCap(stroke.line_cap);
    context_->lineJoin(stroke.line_join);

    if (paint.shader.type() == Shader::Type::LinearGradient) {
        const auto& lg = paint.shader.linear_gradient();
        nisaba::Point p0 = lg.start();
        nisaba::Point p1 = lg.end();
        nisaba::Color c0 = lg.stops().empty() ? nisaba::Color::BLACK : lg.stops().front().color;
        nisaba::Color c1 = lg.stops().empty() ? nisaba::Color::WHITE : lg.stops().back().color;
        context_->strokePaint(context_->linearGradient(p0.x, p0.y, p1.x, p1.y, c0, c1));
        return;
    } else if (paint.shader.type() == Shader::Type::RadialGradient) {
        const auto& rg = paint.shader.radial_gradient();
        nisaba::Point center = rg.center();
        float radius = rg.radius();
        nisaba::Color c0 = rg.stops().empty() ? nisaba::Color::BLACK : rg.stops().front().color;
        nisaba::Color c1 = rg.stops().empty() ? nisaba::Color::WHITE : rg.stops().back().color;
        context_->strokePaint(context_->radialGradient(center.x, center.y, 0.0f, radius, c0, c1));
        return;
    }

    context_->strokeColor(paint.shader.solid_color());
}

void GpuCanvas::fill_rect(const nisaba::Rect& rect, const nisaba::Paint& paint) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->rect(rect.x(), rect.y(), rect.width(), rect.height());
    apply_fill_paint(paint);
    context_->fill();
}

void GpuCanvas::stroke_rect(const nisaba::Rect& rect, const nisaba::Paint& paint, const nisaba::Stroke& stroke) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->rect(rect.x(), rect.y(), rect.width(), rect.height());
    apply_stroke(stroke, paint);
    context_->stroke();
}

void GpuCanvas::fill_round_rect(const nisaba::Rect& rect, float rx, float ry, const nisaba::Paint& paint) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    float r = std::min(rx, ry);
    context_->roundedRect(rect.x(), rect.y(), rect.width(), rect.height(), r);
    apply_fill_paint(paint);
    context_->fill();
}

void GpuCanvas::stroke_round_rect(const nisaba::Rect& rect, float rx, float ry, const nisaba::Paint& paint, const nisaba::Stroke& stroke) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    float r = std::min(rx, ry);
    context_->roundedRect(rect.x(), rect.y(), rect.width(), rect.height(), r);
    apply_stroke(stroke, paint);
    context_->stroke();
}

void GpuCanvas::fill_circle(float cx, float cy, float radius, const nisaba::Paint& paint) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->circle(cx, cy, radius);
    apply_fill_paint(paint);
    context_->fill();
}

void GpuCanvas::stroke_circle(float cx, float cy, float radius, const nisaba::Paint& paint, const nisaba::Stroke& stroke) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->circle(cx, cy, radius);
    apply_stroke(stroke, paint);
    context_->stroke();
}

void GpuCanvas::stroke_line(nisaba::Point p0, nisaba::Point p1, const nisaba::Paint& paint, const nisaba::Stroke& stroke) {
    stroke_line(p0.x, p0.y, p1.x, p1.y, paint, stroke);
}

void GpuCanvas::stroke_line(float x0, float y0, float x1, float y1, const nisaba::Paint& paint, const nisaba::Stroke& stroke) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->moveTo(x0, y0);
    context_->lineTo(x1, y1);
    apply_stroke(stroke, paint);
    context_->stroke();
}

void GpuCanvas::fill_path(const nisaba::Path& path, const nisaba::Paint& paint, nisaba::FillRule fill_rule) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->path(path);
    context_->pathWinding(fill_rule == FillRule::EvenOdd ? Winding::Clockwise : Winding::CounterClockwise);
    apply_fill_paint(paint);
    context_->fill();
}

void GpuCanvas::stroke_path(const nisaba::Path& path, const nisaba::Paint& paint, const nisaba::Stroke& stroke) {
    if (!context_) return;
    ensure_frame();
    context_->beginPath();
    context_->path(path);
    apply_stroke(stroke, paint);
    context_->stroke();
}

void GpuCanvas::draw_pixmap(int32_t x, int32_t y, const PixmapRef& src, const PixmapPaint& paint) {
    if (!context_ || src.width() == 0 || src.height() == 0 || !src.data()) return;
    ensure_frame();

    int img = context_->createImageRGBA(
        static_cast<int>(src.width()),
        static_cast<int>(src.height()),
        ImageFlags::ImagePremultiplied,
        reinterpret_cast<const unsigned char*>(src.data())
    );

    if (img > 0) {
        temp_images_.push_back(img);
        auto pattern = context_->imagePattern(
            static_cast<float>(x), static_cast<float>(y),
            static_cast<float>(src.width()), static_cast<float>(src.height()),
            0.0f, img, paint.opacity
        );
        context_->beginPath();
        context_->rect(static_cast<float>(x), static_cast<float>(y),
                   static_cast<float>(src.width()), static_cast<float>(src.height()));
        context_->fillPaint(pattern);
        context_->fill();
    }
}

void GpuCanvas::draw_gpu_texture(int32_t x, int32_t y, const std::shared_ptr<GpuTexture>& texture, const PixmapPaint& paint) {
    if (!context_ || !texture) return;
    ensure_frame();

    int img = context_->renderer()->createTextureFromNativeHandle(
        texture->id(),
        static_cast<int>(texture->width()),
        static_cast<int>(texture->height()),
        0
    );
    if (img <= 0) {
        auto* glRenderer = dynamic_cast<GL3Renderer*>(context_->renderer());
        if (glRenderer) {
            img = glRenderer->createImageFromHandle(
                texture->id(),
                static_cast<int>(texture->width()),
                static_cast<int>(texture->height()),
                ImageFlagsGL::ImageNoDelete
            );
        }
    }

    if (img > 0) {
        temp_images_.push_back(img);
        auto pattern = context_->imagePattern(
            static_cast<float>(x), static_cast<float>(y),
            static_cast<float>(texture->width()), static_cast<float>(texture->height()),
            0.0f, img, paint.opacity
        );
        context_->beginPath();
        context_->rect(static_cast<float>(x), static_cast<float>(y),
                   static_cast<float>(texture->width()), static_cast<float>(texture->height()));
        context_->fillPaint(pattern);
        context_->fill();
    }
}

void GpuCanvas::draw_vertices(const nisaba::Vertices& vertices, BlendMode blend_mode, float opacity) {
    (void)blend_mode; (void)opacity;
    if (!context_ || !vertices.is_valid()) return;
    ensure_frame();

    const auto& pos = vertices.positions;
    const auto& cols = vertices.colors;
    const auto& indices = vertices.indices;

    size_t num_tris = indices.empty() ? (pos.size() / 3) : (indices.size() / 3);
    for (size_t t = 0; t < num_tris; ++t) {
        uint32_t i0 = indices.empty() ? static_cast<uint32_t>(t * 3) : indices[t * 3];
        uint32_t i1 = indices.empty() ? static_cast<uint32_t>(t * 3 + 1) : indices[t * 3 + 1];
        uint32_t i2 = indices.empty() ? static_cast<uint32_t>(t * 3 + 2) : indices[t * 3 + 2];

        if (i0 < pos.size() && i1 < pos.size() && i2 < pos.size()) {
            context_->beginPath();
            context_->moveTo(pos[i0].x, pos[i0].y);
            context_->lineTo(pos[i1].x, pos[i1].y);
            context_->lineTo(pos[i2].x, pos[i2].y);
            context_->closePath();

            nisaba::Color c = !cols.empty() && i0 < cols.size() ? cols[i0] : nisaba::Color::WHITE;
            context_->fillColor(c);
            context_->fill();
        }
    }
}

void GpuCanvas::draw_textured_vertices(
    const nisaba::Vertices& vertices,
    const std::shared_ptr<GpuTexture>& texture,
    BlendMode blend_mode,
    float opacity
) {
    (void)texture;
    draw_vertices(vertices, blend_mode, opacity);
}

void GpuCanvas::draw_round_rect_shadow(const nisaba::Rect& rect, float rx, float ry, const effects::DropShadow& shadow) {
    if (!context_) return;
    ensure_frame();

    context_->save();
    float x = rect.x();
    float y = rect.y();
    float w = rect.width();
    float h = rect.height();
    float sx = x + shadow.dx;
    float sy = y + shadow.dy;
    float r = std::min(rx, ry);
    float feather = std::max(shadow.sigma, 1.0f);

    nisaba::gpu::Color col = shadow.color;
    nisaba::gpu::Color transparent = nisaba::gpu::Color(col.r, col.g, col.b, 0.0f);

    auto shadowPaint = context_->boxGradient(sx, sy, w, h, r, feather, col, transparent);

    float min_x = std::min(x, sx) - feather * 2.0f;
    float min_y = std::min(y, sy) - feather * 2.0f;
    float max_x = std::max(x + w, sx + w) + feather * 2.0f;
    float max_y = std::max(y + h, sy + h) + feather * 2.0f;

    context_->beginPath();
    context_->rect(min_x, min_y, max_x - min_x, max_y - min_y);
    context_->roundedRect(x, y, w, h, r);
    context_->pathWinding(nisaba::gpu::Winding::Hole);
    context_->fillPaint(shadowPaint);
    context_->fill();

    context_->restore();
}

void GpuCanvas::draw_rect_shadow(const nisaba::Rect& rect, const effects::DropShadow& shadow) {
    draw_round_rect_shadow(rect, 0.0f, 0.0f, shadow);
}

void GpuCanvas::draw_glow(const nisaba::Rect& rect, float rx, float ry, nisaba::Color glow_color, float radius) {
    effects::DropShadow shadow(0.0f, 0.0f, radius, glow_color);
    draw_round_rect_shadow(rect, rx, ry, shadow);
}

void GpuCanvas::draw_glass_panel(const nisaba::Rect& rect, float rx, float ry, const effects::GlassParams& params) {
    if (!context_) return;
    ensure_frame();

    float r = std::min(rx, ry);

    // 1. Drop shadow if present
    if (params.shadow.has_value()) {
        draw_round_rect_shadow(rect, rx, ry, *params.shadow);
    }

    // 2. Glass tinted background
    context_->beginPath();
    context_->roundedRect(rect.x(), rect.y(), rect.width(), rect.height(), r);
    context_->fillColor(params.tint_color);
    context_->fill();

    // 3. Crisp specular border stroke
    if (params.border_width > 0.0f && params.border_color.alpha() > 0.0f) {
        context_->beginPath();
        context_->roundedRect(rect.x() + 0.5f, rect.y() + 0.5f, rect.width() - 1.0f, rect.height() - 1.0f, r);
        context_->strokeColor(params.border_color);
        context_->strokeWidth(params.border_width);
        context_->stroke();
    }
}

void GpuCanvas::draw_svg(const svg::SvgDocument& doc, const nisaba::Rect& dest_bounds) {
    save();

    float vb_w = doc.view_box().width();
    float vb_h = doc.view_box().height();
    if (vb_w > 0.0f && vb_h > 0.0f) {
        float sx = dest_bounds.width() / vb_w;
        float sy = dest_bounds.height() / vb_h;
        float s = std::min(sx, sy);

        float tx = dest_bounds.left() + (dest_bounds.width() - vb_w * s) * 0.5f - doc.view_box().left() * s;
        float ty = dest_bounds.top() + (dest_bounds.height() - vb_h * s) * 0.5f - doc.view_box().top() * s;

        translate(tx, ty);
        scale(s, s);
    }

    for (const auto& elem : doc.elements()) {
        save();
        if (!elem.transform.is_identity()) {
            concat(elem.transform);
        }

        // 1. Fill pass
        if (elem.style.fill.has_value()) {
            nisaba::Color fill_c = *elem.style.fill;
            fill_c.apply_opacity(elem.style.opacity * elem.style.fill_opacity);
            nisaba::Paint fill_paint(fill_c);
            fill_path(elem.path, fill_paint, elem.style.fill_rule);
        }

        // 2. Stroke pass
        if (elem.style.stroke.has_value() && elem.style.stroke_width > 0.0f) {
            nisaba::Color stroke_c = *elem.style.stroke;
            stroke_c.apply_opacity(elem.style.opacity * elem.style.stroke_opacity);
            nisaba::Stroke st;
            st.width = elem.style.stroke_width;
            st.line_cap = elem.style.line_cap;
            st.line_join = elem.style.line_join;
            nisaba::Paint stroke_paint(stroke_c);
            stroke_path(elem.path, stroke_paint, st);
        }

        restore();
    }

    restore();
}

void GpuCanvas::flush() {
    if (context_ && in_frame_) {
        context_->endFrame();
        in_frame_ = false;
    }
    if (surface_) {
        surface_->resolve();
    }
    if (context_) {
        for (int img : temp_images_) {
            context_->deleteImage(img);
        }
        temp_images_.clear();
    }
}

void GpuCanvas::flush(const ScreenIntRect& damage_rect) {
    if (context_ && in_frame_) {
        context_->endFrame();
        in_frame_ = false;
    }
    if (surface_) {
        surface_->resolve(damage_rect);
    }
    if (context_) {
        for (int img : temp_images_) {
            context_->deleteImage(img);
        }
        temp_images_.clear();
    }
}

void GpuCanvas::flush(const std::vector<ScreenIntRect>& damage_rects) {
    if (context_ && in_frame_) {
        context_->endFrame();
        in_frame_ = false;
    }
    if (surface_) {
        surface_->resolve(damage_rects);
    }
    if (context_) {
        for (int img : temp_images_) {
            context_->deleteImage(img);
        }
        temp_images_.clear();
    }
}

void GpuCanvas::draw_text(std::string_view text, float x, float y, const text::Font& font, const nisaba::Paint& paint, float font_size) {
    if (!context_ || text.empty()) return;
    ensure_frame();
    apply_fill_paint(paint);
    context_->textWithFont(x, y, font, font_size, text.data(), text.data() + text.size());
}

void GpuCanvas::draw_text(float x, float y, std::string_view text, const nisaba::Paint& paint, float font_size, const char* font_name) {
    if (!context_ || text.empty()) return;
    ensure_frame();
    apply_fill_paint(paint);
    context_->fontSize(font_size);
    if (font_name && font_name[0] != '\0') {
        context_->fontFace(font_name);
    }
    context_->text(x, y, text.data(), text.data() + text.size());
}

void GpuCanvas::draw_text_buffer(const text::Buffer& buffer, text::FontSystem& fonts, Point pos, nisaba::Color default_color) {
    if (!context_) return;
    ensure_frame();
    context_->drawTextBuffer(buffer, fonts, pos, default_color);
}

void GpuCanvas::draw_text_buffer(const text::Buffer& buffer, text::FontSystem& fonts, Point pos, const nisaba::Paint& paint) {
    if (!context_) return;
    ensure_frame();
    apply_fill_paint(paint);
    context_->drawTextBuffer(buffer, fonts, pos, paint.shader.solid_color());
}

int GpuCanvas::create_font(const char* name, const char* filename) {
    return context_ ? context_->createFont(name, filename) : -1;
}

int GpuCanvas::create_font_mem(const char* name, const uint8_t* data, size_t size) {
    return context_ ? context_->createFontMem(name, const_cast<unsigned char*>(data), static_cast<int>(size), 0) : -1;
}

} // namespace nisaba::gpu

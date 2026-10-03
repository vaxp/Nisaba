#pragma once

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#include <vector>
#include <optional>
#include <memory>
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include <string_view>
#include <span>
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/painter.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/pipeline/simd.hpp"
#include "nisaba/text/glyph.hpp"
#include "nisaba/text/font_8x16.hpp"
#include "nisaba/text/baked_text.hpp"
#include "nisaba/effects/effects.hpp"
#include "nisaba/svg/svg.hpp"
#include "nisaba/math/transform4x4.hpp"
#include "nisaba/raster/perspective_raster.hpp"
#include "nisaba/mesh/mesh.hpp"
#include "nisaba/raster/mesh_raster.hpp"
#include "nisaba/canvas/canvas_interface.hpp"

namespace nisaba {

/// State saved/restored on the canvas stack.
struct CanvasState {
    Transform transform{};
    std::shared_ptr<Mask> clip_mask{};
    std::optional<ScreenIntRect> scissor_clip{};
    const Mask* external_mask{nullptr};
};

class Canvas : public ICanvas {
public:
    explicit Canvas(PixmapMut pixmap) noexcept
        : pixmap_(pixmap),
          scissor_clip_(ScreenIntRect::from_xywh(0, 0, pixmap.width(), pixmap.height())) {}

    explicit Canvas(Pixmap& pixmap) noexcept
        : pixmap_(pixmap.as_mut()),
          scissor_clip_(ScreenIntRect::from_xywh(0, 0, pixmap.width(), pixmap.height())) {}

    [[nodiscard]] uint32_t width() const noexcept { return pixmap_.width(); }
    [[nodiscard]] uint32_t height() const noexcept { return pixmap_.height(); }
    [[nodiscard]] IntSize size() const noexcept { return pixmap_.size(); }
    [[nodiscard]] PixmapMut& pixmap() noexcept { return pixmap_; }

    void save() override {
        state_stack_.push_back(CanvasState{
            .transform = current_transform_,
            .clip_mask = active_clip_mask_,
            .scissor_clip = scissor_clip_,
            .external_mask = external_mask_ptr_
        });
    }

    void restore() override {
        if (!state_stack_.empty()) {
            current_transform_ = state_stack_.back().transform;
            if (active_clip_mask_ && active_clip_mask_.use_count() == 1) {
                recycled_mask_ = std::move(active_clip_mask_);
            }
            active_clip_mask_ = std::move(state_stack_.back().clip_mask);
            scissor_clip_ = state_stack_.back().scissor_clip;
            external_mask_ptr_ = state_stack_.back().external_mask;
            state_stack_.pop_back();
        }
    }

    void reset_transform() noexcept {
        current_transform_ = Transform();
    }

    void set_transform(const Transform& ts) noexcept {
        current_transform_ = ts;
    }

    [[nodiscard]] const Transform& transform() const noexcept {
        return current_transform_;
    }

    void translate(float dx, float dy) noexcept override {
        current_transform_ = current_transform_.pre_translate(dx, dy);
    }

    void scale(float sx, float sy) noexcept override {
        current_transform_ = current_transform_.pre_scale(sx, sy);
    }

    void rotate(float degrees) noexcept {
        current_transform_ = current_transform_.pre_rotate(degrees);
    }

    void concat(const Transform& ts) noexcept override {
        current_transform_ = current_transform_.pre_concat(ts);
    }

    // --- Vector Clipping ---

    /// Intersects the active clip region with the specified path.
    void clip_path(const Path& path, FillRule fill_rule = FillRule::Winding, bool anti_alias = true) {
        Rect rect_box;
        if (std::abs(current_transform_.kx) < 1e-5f && std::abs(current_transform_.ky) < 1e-5f &&
            path.is_rect(&rect_box)) {
            clip_rect(rect_box, anti_alias);
            return;
        }

        Rect path_b = path.bounds();
        if (!current_transform_.is_identity()) {
            auto tr = path.transform(current_transform_);
            if (tr) path_b = tr->bounds();
        }
        if (path_b.width() > 0.0f && path_b.height() > 0.0f && scissor_clip_.has_value()) {
            int32_t bx0 = std::max(0, static_cast<int32_t>(std::floor(path_b.left())));
            int32_t by0 = std::max(0, static_cast<int32_t>(std::floor(path_b.top())));
            int32_t bx1 = std::min(static_cast<int32_t>(width()), static_cast<int32_t>(std::ceil(path_b.right())));
            int32_t by1 = std::min(static_cast<int32_t>(height()), static_cast<int32_t>(std::ceil(path_b.bottom())));
            if (bx1 > bx0 && by1 > by0) {
                auto br = ScreenIntRect::from_xywh(
                    static_cast<uint32_t>(bx0), static_cast<uint32_t>(by0),
                    static_cast<uint32_t>(bx1 - bx0), static_cast<uint32_t>(by1 - by0)
                );
                if (br) {
                    scissor_clip_ = scissor_clip_->intersect(*br);
                }
            } else {
                scissor_clip_ = std::nullopt;
            }
        }

        if (!active_clip_mask_) {
            if (recycled_mask_ && recycled_mask_->width() == width() && recycled_mask_->height() == height()) {
                active_clip_mask_ = std::move(recycled_mask_);
                active_clip_mask_->clear();
            } else {
                auto m = Mask::allocate(width(), height());
                if (m) {
                    active_clip_mask_ = std::make_shared<Mask>(std::move(*m));
                }
            }
            if (active_clip_mask_) {
                float cc_x, cc_y, cc_r;
                if (std::abs(current_transform_.kx) < 1e-5f && std::abs(current_transform_.ky) < 1e-5f &&
                    std::abs(current_transform_.sx - current_transform_.sy) < 1e-4f &&
                    path.is_circle(&cc_x, &cc_y, &cc_r)) {
                    float s = std::abs(current_transform_.sx);
                    active_clip_mask_->fill_circle(cc_x * current_transform_.sx + current_transform_.tx,
                                                  cc_y * current_transform_.sy + current_transform_.ty,
                                                  cc_r * s);
                } else {
                    active_clip_mask_->fill_path(path, fill_rule, anti_alias, current_transform_);
                }
            }
        } else {
            if (active_clip_mask_.use_count() > 1) {
                active_clip_mask_ = std::make_shared<Mask>(*active_clip_mask_);
            }
            active_clip_mask_->intersect_path(path, fill_rule, anti_alias, current_transform_, scissor_clip_);
        }
    }

    /// Intersects the active clip region with an axis-aligned rectangle.
    void clip_rect(const Rect& rect, bool anti_alias = true) {
        if (std::abs(current_transform_.kx) < 1e-5f && std::abs(current_transform_.ky) < 1e-5f) {
            if (!scissor_clip_.has_value()) return;

            float x0 = rect.left() * current_transform_.sx + current_transform_.tx;
            float x1 = rect.right() * current_transform_.sx + current_transform_.tx;
            float y0 = rect.top() * current_transform_.sy + current_transform_.ty;
            float y1 = rect.bottom() * current_transform_.sy + current_transform_.ty;
            if (x0 > x1) std::swap(x0, x1);
            if (y0 > y1) std::swap(y0, y1);

            int32_t ix0 = static_cast<int32_t>(std::round(x0));
            int32_t iy0 = static_cast<int32_t>(std::round(y0));
            int32_t ix1 = static_cast<int32_t>(std::round(x1));
            int32_t iy1 = static_cast<int32_t>(std::round(y1));

            ix0 = std::clamp(ix0, 0, static_cast<int32_t>(width()));
            iy0 = std::clamp(iy0, 0, static_cast<int32_t>(height()));
            ix1 = std::clamp(ix1, 0, static_cast<int32_t>(width()));
            iy1 = std::clamp(iy1, 0, static_cast<int32_t>(height()));

            if (ix1 > ix0 && iy1 > iy0) {
                auto new_clip = ScreenIntRect::from_xywh(
                    static_cast<uint32_t>(ix0), static_cast<uint32_t>(iy0),
                    static_cast<uint32_t>(ix1 - ix0), static_cast<uint32_t>(iy1 - iy0)
                );
                if (new_clip) {
                    scissor_clip_ = scissor_clip_->intersect(*new_clip);
                    return;
                }
            }
            scissor_clip_ = std::nullopt;
            return;
        }

        auto p = PathBuilder::from_rect(rect);
        clip_path(p, FillRule::Winding, anti_alias);
    }

    /// Clears the active clip mask.
    void reset_clip() noexcept {
        if (active_clip_mask_ && active_clip_mask_.use_count() == 1) {
            recycled_mask_ = std::move(active_clip_mask_);
        }
        active_clip_mask_.reset();
        scissor_clip_ = ScreenIntRect::from_xywh(0, 0, width(), height());
    }

    /// Intersects the active clip region with an arbitrary alpha/luminance coverage mask.
    void clip_mask(const Mask& mask) {
        if (!active_clip_mask_) {
            active_clip_mask_ = std::make_shared<Mask>(mask);
        } else {
            if (active_clip_mask_.use_count() > 1) {
                active_clip_mask_ = std::make_shared<Mask>(*active_clip_mask_);
            }
            active_clip_mask_->intersect_mask(mask);
        }
    }

    [[nodiscard]] const Mask* current_mask() const noexcept {
        if (active_clip_mask_) {
            return active_clip_mask_.get();
        }
        return external_mask_ptr_;
    }

    [[nodiscard]] const std::optional<ScreenIntRect>& scissor_clip() const noexcept {
        return scissor_clip_;
    }

    /// Clip directly to an integer screen rectangle.
    void clip_rect(const ScreenIntRect& rect) noexcept {
        if (scissor_clip_.has_value()) {
            scissor_clip_ = scissor_clip_->intersect(rect);
        }
    }

    /// Set or clear the active scissor clip.
    void set_scissor_clip(const std::optional<ScreenIntRect>& clip) noexcept {
        scissor_clip_ = clip;
    }


    void set_linear_blending(bool enable) noexcept { linear_blending_ = enable; }
    [[nodiscard]] bool is_linear_blending() const noexcept { return linear_blending_; }

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
    [[nodiscard]] inline Paint resolve_paint(const Paint& paint) const noexcept {
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            return p;
        }
        return paint;
    }
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

    void clear(Color color = Color::TRANSPARENT) {
        pixmap_.fill(color);
    }

    void fill_rect(const Rect& rect, const Paint& paint) override {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::fill_rect(pixmap_, rect, p, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::fill_rect(pixmap_, rect, paint, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void fill_path(const Path& path, const Paint& paint, FillRule fill_rule = FillRule::Winding) override {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::fill_path(pixmap_, path, p, fill_rule, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::fill_path(pixmap_, path, paint, fill_rule, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void stroke_path(const Path& path, const Paint& paint, const Stroke& stroke) override {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::stroke_path(pixmap_, path, p, stroke, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::stroke_path(pixmap_, path, paint, stroke, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void stroke_line(Point p0, Point p1, const Paint& paint, const Stroke& stroke) {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::stroke_line(pixmap_, p0, p1, p, stroke, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::stroke_line(pixmap_, p0, p1, paint, stroke, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void stroke_line(float x0, float y0, float x1, float y1, const Paint& paint, const Stroke& stroke) {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::stroke_line(pixmap_, Point::from_xy(x0, y0), Point::from_xy(x1, y1), p, stroke, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::stroke_line(pixmap_, Point::from_xy(x0, y0), Point::from_xy(x1, y1), paint, stroke, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void stroke_rect(const Rect& rect, const Paint& paint, const Stroke& stroke) {
        if (!scissor_clip_.has_value()) return;
        if (!stroke.dash.has_value() &&
            std::abs(current_transform_.kx) < 1e-5f && std::abs(current_transform_.ky) < 1e-5f &&
            stroke.line_join == LineJoin::Miter) {
            float sw = stroke.width;
            if (std::abs(current_transform_.sx - current_transform_.sy) < 1e-4f && current_transform_.sx > 0.0f) {
                float x0 = rect.left() * current_transform_.sx + current_transform_.tx;
                float x1 = rect.right() * current_transform_.sx + current_transform_.tx;
                float y0 = rect.top() * current_transform_.sy + current_transform_.ty;
                float y1 = rect.bottom() * current_transform_.sy + current_transform_.ty;
                auto mapped_r = Rect::from_ltrb(x0, y0, x1, y1);
                if (mapped_r) {
                    Paint p = paint;
                    if (!current_transform_.is_identity()) {
                        p.shader.transform(current_transform_);
                    }
                    if (linear_blending_ && !p.is_linear_blending()) {
                        p.set_linear_blending(true);
                    }
                    painter::stroke_rect_axis_aligned(pixmap_, *mapped_r, sw * current_transform_.sx, p, current_mask(), scissor_clip_);
                    return;
                }
            }
        }
        auto p = PathBuilder::from_rect(rect);
        stroke_path(p, paint, stroke);
    }

    void fill_round_rect(const Rect& rect, float rx, float ry, const Paint& paint) {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::fill_round_rect(pixmap_, rect, rx, ry, p, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::fill_round_rect(pixmap_, rect, rx, ry, paint, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void stroke_round_rect(const Rect& rect, float rx, float ry, const Paint& paint, const Stroke& stroke) {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::stroke_round_rect(pixmap_, rect, rx, ry, p, stroke, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::stroke_round_rect(pixmap_, rect, rx, ry, paint, stroke, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void stroke_circle(float cx, float cy, float radius, const Paint& paint, const Stroke& stroke) {
        if (!scissor_clip_.has_value()) return;
        if (radius <= 0.0f) return;

        if (stroke.dash.has_value() && current_transform_.is_identity()) {
            const auto& intervals = stroke.dash->array();
            if (!intervals.empty()) {
                float total_len = 2.0f * static_cast<float>(M_PI) * radius;
                float inv_r = 1.0f / radius;
                float dash_len = stroke.dash->interval_len().get();
                float offset = stroke.dash->offset();
                if (offset < 0.0f) {
                    offset = std::fmod(offset, dash_len) + dash_len;
                } else if (offset >= dash_len) {
                    offset = std::fmod(offset, dash_len);
                }

                size_t idx = 0;
                float accumulated = 0.0f;
                float cur_int = 0.0f;
                while (idx < intervals.size()) {
                    if (accumulated + intervals[idx] > offset) {
                        cur_int = (accumulated + intervals[idx]) - offset;
                        break;
                    }
                    accumulated += intervals[idx];
                    idx++;
                }

                float cur_dist = 0.0f;
                bool is_on = (idx % 2 == 0);
                Stroke stroke_no_dash = stroke;
                stroke_no_dash.dash = std::nullopt;

                while (cur_dist < total_len) {
                    float step = std::min(cur_int, total_len - cur_dist);
                    if (is_on && step > 0.0f) {
                        float t0 = cur_dist * inv_r;
                        float t1 = (cur_dist + step) * inv_r;
                        float dt = t1 - t0;
                        int num_steps = (dt > 0.08f) ? 2 : 1;
                        float sub_dt = dt / num_steps;
                        float cur_t = t0;
                        float p0x = cx + radius * std::cos(cur_t);
                        float p0y = cy + radius * std::sin(cur_t);
                        for (int s = 0; s < num_steps; ++s) {
                            float next_t = cur_t + sub_dt;
                            float p1x = cx + radius * std::cos(next_t);
                            float p1y = cy + radius * std::sin(next_t);
                            stroke_line(p0x, p0y, p1x, p1y, paint, stroke_no_dash);
                            p0x = p1x; p0y = p1y;
                            cur_t = next_t;
                        }
                    }
                    cur_dist += step;
                    idx = (idx + 1) % intervals.size();
                    is_on = (idx % 2 == 0);
                    cur_int = intervals[idx];
                }
                return;
            }
        }

        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::stroke_circle(pixmap_, cx, cy, radius, p, stroke, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::stroke_circle(pixmap_, cx, cy, radius, paint, stroke, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void fill_circle(float cx, float cy, float radius, const Paint& paint) {
        if (!scissor_clip_.has_value()) return;
        if (linear_blending_ && !paint.is_linear_blending()) {
            Paint p = paint;
            p.set_linear_blending(true);
            painter::fill_circle(pixmap_, cx, cy, radius, p, current_transform_, current_mask(), scissor_clip_);
        } else {
            painter::fill_circle(pixmap_, cx, cy, radius, paint, current_transform_, current_mask(), scissor_clip_);
        }
    }

    void draw_pixmap(int32_t x, int32_t y, const PixmapRef& src, const PixmapPaint& paint = PixmapPaint()) {
        if (!scissor_clip_.has_value()) return;
        painter::draw_pixmap(pixmap_, x, y, src, paint, current_transform_, current_mask(), scissor_clip_);
    }

    /// Renders a source Pixmap mapped onto an arbitrary destination quadrilateral
    /// [p0, p1, p2, p3] with perspective-correct scanline interpolation and bilinear filtering.
    void draw_pixmap_perspective(
        const PixmapRef& src,
        Point p0, Point p1, Point p2, Point p3,
        float opacity = 1.0f,
        BlendMode blend_mode = BlendMode::SourceOver
    ) {
        if (!current_transform_.is_identity()) {
            current_transform_.map_point(p0);
            current_transform_.map_point(p1);
            current_transform_.map_point(p2);
            current_transform_.map_point(p3);
        }
        raster::draw_pixmap_perspective_quad(pixmap_, src, p0, p1, p2, p3, opacity, blend_mode);
    }

    /// Renders a source Pixmap sub-region under a full 3D Transform4x4 projective matrix with perspective correction.
    void draw_pixmap_3d(
        const PixmapRef& src,
        const Rect& src_rect,
        const Transform4x4& transform3d,
        float opacity = 1.0f,
        BlendMode blend_mode = BlendMode::SourceOver
    ) {
        Transform4x4 final_t = transform3d;
        if (!current_transform_.is_identity()) {
            final_t = Transform4x4(current_transform_) * transform3d;
        }
        raster::draw_pixmap_3d(pixmap_, src, src_rect, final_t, opacity, blend_mode);
    }

    /// Renders an entire source Pixmap under a full 3D Transform4x4 projective matrix with perspective correction.
    void draw_pixmap_3d(
        const PixmapRef& src,
        const Transform4x4& transform3d,
        float opacity = 1.0f,
        BlendMode blend_mode = BlendMode::SourceOver
    ) {
        auto r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(src.width()), static_cast<float>(src.height()));
        if (r) {
            draw_pixmap_3d(src, *r, transform3d, opacity, blend_mode);
        }
    }

    // --- 2D Vertex Meshes & Freeform Gradient Meshes ---

    /// Renders a 2D mesh of colored and/or textured vertices.
    void draw_vertices(
        const Vertices& vertices,
        BlendMode blend_mode = BlendMode::SourceOver,
        float opacity = 1.0f
    ) {
        raster::draw_vertices(pixmap_, vertices, nullptr, opacity, blend_mode, current_transform_);
    }

    /// Renders a 2D mesh of textured vertices with bilinear filtering and optional vertex color modulation.
    void draw_textured_vertices(
        const Vertices& vertices,
        const PixmapRef& texture,
        BlendMode blend_mode = BlendMode::SourceOver,
        float opacity = 1.0f
    ) {
        raster::draw_vertices(pixmap_, vertices, &texture, opacity, blend_mode, current_transform_);
    }

    /// Renders a bicubic Coons Patch Freeform Gradient Mesh.
    void draw_gradient_mesh(
        const GradientMesh& mesh,
        BlendMode blend_mode = BlendMode::SourceOver,
        float opacity = 1.0f,
        uint32_t subdivision = 8
    ) {
        Vertices v = mesh.to_vertices(subdivision);
        draw_vertices(v, blend_mode, opacity);
    }

    /// Renders a single Coons Patch Freeform Gradient Mesh.
    void draw_coons_patch(
        const CoonsPatch& patch,
        BlendMode blend_mode = BlendMode::SourceOver,
        float opacity = 1.0f,
        uint32_t subdiv_u = 8,
        uint32_t subdiv_v = 8
    ) {
        Vertices v = patch.to_vertices(subdiv_u, subdiv_v);
        draw_vertices(v, blend_mode, opacity);
    }

    void set_mask(const Mask* mask) noexcept {
        external_mask_ptr_ = mask;
    }

    void apply_mask(const Mask& mask) {
        painter::apply_mask(pixmap_, mask);
    }

    // --- Visual Effects, Shadows & Glassmorphism ---

    /// Draws a path with an elevation or ambient drop shadow behind it.
    void draw_path_with_shadow(
        const Path& path,
        const Paint& paint,
        const effects::DropShadow& shadow,
        FillRule fill_rule = FillRule::Winding
    ) {
        effects::draw_path_shadow(pixmap_, path, shadow, fill_rule, current_transform_, current_mask());
        fill_path(path, paint, fill_rule);
    }

    /// Draws a rectangle with a drop shadow behind it.
    void draw_rect_with_shadow(
        const Rect& rect,
        const Paint& paint,
        const effects::DropShadow& shadow
    ) {
        effects::draw_rect_shadow(pixmap_, rect, shadow, current_transform_, current_mask());
        fill_rect(rect, paint);
    }

    /// Draws a rounded rectangle with a drop shadow behind it.
    void draw_round_rect_with_shadow(
        const Rect& rect,
        float rx,
        float ry,
        const Paint& paint,
        const effects::DropShadow& shadow
    ) {
        effects::draw_round_rect_shadow(pixmap_, rect, rx, ry, shadow, current_transform_, current_mask());
        auto p = PathBuilder::from_rounded_rect(rect, rx, ry);
        if (p) fill_path(*p, paint);
    }

    /// Draws a modern frosted glassmorphic panel (Glassmorphism) with backdrop blur,
    /// translucent frost overlay, edge reflection highlight, and drop shadow.
    void draw_glass_panel(
        const Rect& rect,
        float rx,
        float ry,
        const effects::GlassParams& params
    ) {
        effects::draw_glass_panel(pixmap_, rect, rx, ry, params, current_transform_, current_mask());
    }

    /// Applies a 3-pass fast Gaussian blur to the entire canvas surface.
    void apply_blur(float sigma) {
        effects::gaussian_blur_pixmap(pixmap_, sigma);
    }

    // --- SVG Vector Graphics ---

    /// Parses and fills an SVG path string 'd' directly.
    void draw_svg_path(
        std::string_view d,
        const Paint& paint,
        FillRule fill_rule = FillRule::Winding
    ) {
        auto p = svg::PathParser::parse(d);
        if (p) fill_path(*p, paint, fill_rule);
    }

    /// Parses and strokes an SVG path string 'd' directly.
    void stroke_svg_path(
        std::string_view d,
        const Paint& paint,
        const Stroke& stroke
    ) {
        auto p = svg::PathParser::parse(d);
        if (p) stroke_path(*p, paint, stroke);
    }

    /// Renders an SVG document, scaling to fit the target rectangle preserving aspect ratio.
    void draw_svg(const svg::SvgDocument& doc, const Rect& dest_bounds) {
        doc.render(*this, &dest_bounds);
    }

    // --- Text & Glyph Rendering ---

    /// Draws an 8-bit alpha glyph mask positioned at (pos.x, pos.y).
    void draw_glyph(Point pos, const MaskRef& glyph_mask, const Paint& paint) {
        int32_t gx = static_cast<int32_t>(std::floor(pos.x));
        int32_t gy = static_cast<int32_t>(std::floor(pos.y));
        uint32_t gw = glyph_mask.width();
        uint32_t gh = glyph_mask.height();

        if (gx + static_cast<int32_t>(gw) <= 0 || gy + static_cast<int32_t>(gh) <= 0 ||
            gx >= static_cast<int32_t>(width()) || gy >= static_cast<int32_t>(height())) {
            return;
        }

        if (!scissor_clip_.has_value()) return;
        int32_t clip_left = static_cast<int32_t>(scissor_clip_->x());
        int32_t clip_right = static_cast<int32_t>(scissor_clip_->right());
        int32_t clip_top = static_cast<int32_t>(scissor_clip_->y());
        int32_t clip_bottom = static_cast<int32_t>(scissor_clip_->bottom());

        const Mask* active_mask = current_mask();
        Paint resolved_p = resolve_paint(paint);

        for (uint32_t y = 0; y < gh; ++y) {
            int32_t dy = gy + static_cast<int32_t>(y);
            if (dy < clip_top || dy >= clip_bottom) continue;

            for (uint32_t x = 0; x < gw; ++x) {
                int32_t dx = gx + static_cast<int32_t>(x);
                if (dx < clip_left || dx >= clip_right) continue;

                uint8_t alpha = glyph_mask.get(x, y);
                if (alpha == 0) continue;

                float cov = static_cast<float>(alpha) / 255.0f;
                if (active_mask) {
                    cov *= static_cast<float>(active_mask->get(static_cast<uint32_t>(dx), static_cast<uint32_t>(dy))) / 255.0f;
                }
                if (cov <= 0.0f) continue;

                if (pixmap_.format() == PixelFormat::RGBA8888 && resolved_p.is_solid_color() && resolved_p.blend_mode == BlendMode::SourceOver) {
                    PremultipliedColorU8 sc = resolved_p.shader.solid_color().premultiply().to_color_u8();
                    uint8_t cov_u8 = static_cast<uint8_t>(cov * 255.0f + 0.5f);
                    if (resolved_p.is_linear_blending()) {
                        simd::blend_solid_source_over_linear_span(pixmap_.row(static_cast<size_t>(dy)) + dx, sc, cov_u8, 1);
                    } else {
                        simd::blend_solid_source_over_coverage(pixmap_.row(static_cast<size_t>(dy)) + dx, sc, cov_u8, 1);
                    }
                } else {
                    auto r = Rect::from_xywh(static_cast<float>(dx), static_cast<float>(dy), 1.0f, 1.0f);
                    if (r) {
                        painter::fill_rect(pixmap_, *r, resolved_p, Transform(), active_mask, scissor_clip_);
                    }
                }
            }
        }
    }

    /// Batch draws multiple positioned glyph runs.
    void draw_glyphs(std::span<const GlyphRun> glyphs, const Paint& paint) {
        for (const auto& g : glyphs) {
            draw_glyph(g.position, g.mask, paint);
        }
    }

    /// Draws a pre-rasterized BakedText surface onto Canvas with direct SIMD row blitting.
    /// This bypasses font shaping, glyph lookups, and per-pixel shader loops for O(1) text rendering.
    void draw_baked_text(const text::BakedText& baked_text, float x, float y, float opacity = 1.0f) {
        if (!baked_text.is_valid() || opacity <= 0.001f) return;
        const Pixmap* src_pixmap = baked_text.pixmap();
        if (!src_pixmap) return;

        int32_t gx = static_cast<int32_t>(std::round(x)) + baked_text.origin_x();
        int32_t gy = static_cast<int32_t>(std::round(y)) + baked_text.origin_y();
        uint32_t sw = src_pixmap->width();
        uint32_t sh = src_pixmap->height();

        int32_t dst_w = static_cast<int32_t>(width());
        int32_t dst_h = static_cast<int32_t>(height());

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

        if (scissor_clip_.has_value()) {
            clip_left = std::max(0, static_cast<int32_t>(scissor_clip_->x()));
            clip_top = std::max(0, static_cast<int32_t>(scissor_clip_->y()));
            clip_right = std::min(dst_w, static_cast<int32_t>(scissor_clip_->right()));
            clip_bottom = std::min(dst_h, static_cast<int32_t>(scissor_clip_->bottom()));
        }

        // Intersect with source rectangle
        int32_t x0 = std::max(gx, clip_left);
        int32_t y0 = std::max(gy, clip_top);
        int32_t x1 = std::min(gx + static_cast<int32_t>(sw), clip_right);
        int32_t y1 = std::min(gy + static_cast<int32_t>(sh), clip_bottom);

        if (x0 >= x1 || y0 >= y1) return;

        uint32_t span_len = static_cast<uint32_t>(x1 - x0);
        int32_t src_start_x = x0 - gx;
        int32_t src_start_y = y0 - gy;

        uint8_t op_u8 = static_cast<uint8_t>(std::clamp(opacity, 0.0f, 1.0f) * 255.0f + 0.5f);

        if (pixmap_.format() == PixelFormat::RGBA8888) {
            for (int32_t dy = y0; dy < y1; ++dy) {
                int32_t sy = src_start_y + (dy - y0);
                auto* dst_row = pixmap_.row(static_cast<size_t>(dy)) + x0;
                const auto* src_row = src_pixmap->pixels() + static_cast<size_t>(sy) * sw + src_start_x;

                if (op_u8 == 255) {
                    simd::blend_source_over_span(dst_row, src_row, span_len);
                } else {
                    simd::blend_source_over_span_coverage(dst_row, src_row, op_u8, span_len);
                }
            }
        } else {
            PixmapPaint p;
            p.opacity = opacity;
            painter::draw_pixmap(pixmap_, gx, gy, src_pixmap->as_ref(), p, Transform(), current_mask(), scissor_clip_);
        }
    }

    /// Draws a pre-rasterized BakedSvg surface onto Canvas with direct SIMD row blitting.
    /// This bypasses vector path parsing, tessellation and rasterizer pipelines for O(1) SVG rendering.
    void draw_baked_svg(const svg::BakedSvg& baked_svg, float x, float y, float opacity = 1.0f) {
        baked_svg.draw(*this, x, y, opacity);
    }

    /// Draws text using the built-in 8x16 monospace font.
    /// Perfect for DRM/KMS, embedded diagnostics, FPS counters and HUDs without any external font engine.
    void draw_text_debug(std::string_view text, float x, float y, const Paint& paint, float scale = 1.0f) {
        float cur_x = x;
        float cur_y = y;
        for (char c : text) {
            if (c == '\n') {
                cur_x = x;
                cur_y += 16.0f * scale;
                continue;
            }
            const uint8_t* glyph_data = font::get_glyph_bitmap(c);
            std::vector<uint8_t> mask_buf(8 * 16, 0);
            for (uint32_t row = 0; row < 16; ++row) {
                uint8_t row_bits = glyph_data[row];
                for (uint32_t col = 0; col < 8; ++col) {
                    if (row_bits & (1 << (7 - col))) {
                        mask_buf[row * 8 + col] = 255;
                    }
                }
            }
            auto mask_opt = MaskRef::from_bytes(mask_buf.data(), mask_buf.size(), 8, 16);
            if (mask_opt) {
                draw_glyph(Point::from_xy(cur_x, cur_y), *mask_opt, paint);
            }
            cur_x += 8.0f * scale;
        }
    }

private:
    PixmapMut pixmap_;
    std::optional<ScreenIntRect> scissor_clip_;
    Transform current_transform_{};
    std::vector<CanvasState> state_stack_{};
    std::shared_ptr<Mask> active_clip_mask_{};
    std::shared_ptr<Mask> recycled_mask_{};
    const Mask* external_mask_ptr_{nullptr};
    bool linear_blending_{false};
};

} // namespace nisaba

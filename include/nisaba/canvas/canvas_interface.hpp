#pragma once

/// @file canvas_interface.hpp
/// @brief Sovereign 2D vector drawing interface shared by CPU Canvas and GPU GpuCanvas.

#include "nisaba/math/transform.hpp"
#include "nisaba/math/transform4x4.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/border_radius.hpp"
#include "nisaba/math/rrect.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/effects/drop_shadow.hpp"

namespace nisaba {

/// @class ICanvas
/// @brief Abstract 2D vector drawing interface shared by software (Canvas) and hardware (GpuCanvas).
class ICanvas {
public:
    virtual ~ICanvas() = default;

    [[nodiscard]] virtual uint32_t canvas_width() const noexcept { return 0; }
    [[nodiscard]] virtual uint32_t canvas_height() const noexcept { return 0; }

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(float dx, float dy) = 0;
    virtual void scale(float sx, float sy) = 0;
    virtual void concat(const Transform& ts) = 0;
    [[nodiscard]] virtual Transform current_transform() const noexcept { return Transform::identity(); }

    virtual void fill_rect(const Rect& rect, const Paint& paint) = 0;
    virtual void fill_path(const Path& path, const Paint& paint, FillRule fill_rule = FillRule::Winding) = 0;
    virtual void stroke_path(const Path& path, const Paint& paint, const Stroke& stroke) = 0;

    virtual void clip_path(const Path& path, FillRule fill_rule = FillRule::Winding) {
        (void)path; (void)fill_rule;
    }
    virtual void clip_rect(const Rect& rect) {
        if (auto p = PathBuilder::from_rect(rect); !p.is_empty()) {
            clip_path(p, FillRule::Winding);
        }
    }

    virtual void draw_pixmap(int32_t x, int32_t y, const PixmapRef& src, const PixmapPaint& paint = PixmapPaint()) {
        (void)x; (void)y; (void)src; (void)paint;
    }

    virtual void draw_pixmap_3d(
        const PixmapRef& src,
        const Transform4x4& transform3d,
        float opacity = 1.0f,
        BlendMode blend_mode = BlendMode::SourceOver
    ) {
        (void)src; (void)transform3d; (void)opacity; (void)blend_mode;
    }

    virtual void fill_rrect(const RRect& rrect, const Paint& paint) {
        fill_path(rrect.to_path(), paint);
    }
    virtual void stroke_rrect(const RRect& rrect, const Paint& paint, const Stroke& stroke) {
        stroke_path(rrect.to_path(), paint, stroke);
    }
    virtual void clip_rrect(const RRect& rrect) {
        clip_path(rrect.to_path(), FillRule::Winding);
    }
};

} // namespace nisaba

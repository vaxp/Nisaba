#pragma once

/// @file canvas_interface.hpp
/// @brief Sovereign 2D vector drawing interface shared by CPU Canvas and GPU GpuCanvas.

#include "nisaba/math/transform.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/border_radius.hpp"
#include "nisaba/math/rrect.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/canvas/paint.hpp"

namespace nisaba {

/// @class ICanvas
/// @brief Abstract 2D vector drawing interface shared by software (Canvas) and hardware (GpuCanvas).
class ICanvas {
public:
    virtual ~ICanvas() = default;

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(float dx, float dy) = 0;
    virtual void scale(float sx, float sy) = 0;
    virtual void concat(const Transform& ts) = 0;

    virtual void fill_rect(const Rect& rect, const Paint& paint) = 0;
    virtual void fill_path(const Path& path, const Paint& paint, FillRule fill_rule = FillRule::Winding) = 0;
    virtual void stroke_path(const Path& path, const Paint& paint, const Stroke& stroke) = 0;
    virtual void clip_path(const Path& path, FillRule fill_rule = FillRule::Winding) {
        (void)path; (void)fill_rule;
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

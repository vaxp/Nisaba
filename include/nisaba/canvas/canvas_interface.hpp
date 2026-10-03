#pragma once

/// @file canvas_interface.hpp
/// @brief Sovereign 2D vector drawing interface shared by CPU Canvas and GPU GpuCanvas.

#include "nisaba/math/transform.hpp"
#include "nisaba/math/rect.hpp"
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
};

} // namespace nisaba

#pragma once

/// @file shape.hpp
/// @brief Vector shape items and modifiers for Lottie in Nisaba.

#include "nisaba/lottie/lottie_types.hpp"
#include "nisaba/lottie/property.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/shaders/shader.hpp"

#include <vector>
#include <memory>
#include <string>

namespace nisaba::lottie {

// Forward declarations
class RenderContext;

class LottieShapeItem {
public:
    virtual ~LottieShapeItem() = default;
    [[nodiscard]] virtual std::string_view type_name() const noexcept = 0;
    [[nodiscard]] virtual bool is_geometry() const noexcept { return false; }
    [[nodiscard]] virtual bool is_modifier() const noexcept { return false; }

    virtual void collect_geometry(float frame, Path& out_path) const { (void)frame; (void)out_path; }
    virtual void render(RenderContext& ctx, float frame) const { (void)ctx; (void)frame; }

    static std::shared_ptr<LottieShapeItem> parse(const json::JsonValue& val);
};

// Shape Group ("gr")
class ShapeGroup : public LottieShapeItem {
public:
    std::string name;
    std::vector<std::shared_ptr<LottieShapeItem>> items;
    LottieTransform transform;

    [[nodiscard]] std::string_view type_name() const noexcept override { return "gr"; }
    void render(RenderContext& ctx, float frame) const override;
    void collect_geometry(float frame, Path& out_path) const override;

    static std::shared_ptr<ShapeGroup> parse(const json::JsonValue& val);
};

// Freeform Path Shape ("sh")
class PathShape : public LottieShapeItem {
public:
    ShapeProperty shape;

    [[nodiscard]] std::string_view type_name() const noexcept override { return "sh"; }
    [[nodiscard]] bool is_geometry() const noexcept override { return true; }
    void collect_geometry(float frame, Path& out_path) const override;

    static std::shared_ptr<PathShape> parse(const json::JsonValue& val);
};

// Rectangle Shape ("rc")
class RectShape : public LottieShapeItem {
public:
    PointProperty position{Point::zero()};
    PointProperty size{Point::zero()};
    FloatProperty roundness{0.0f};

    [[nodiscard]] std::string_view type_name() const noexcept override { return "rc"; }
    [[nodiscard]] bool is_geometry() const noexcept override { return true; }
    void collect_geometry(float frame, Path& out_path) const override;

    static std::shared_ptr<RectShape> parse(const json::JsonValue& val);
};

// Ellipse Shape ("el")
class EllipseShape : public LottieShapeItem {
public:
    PointProperty position{Point::zero()};
    PointProperty size{Point::zero()};

    [[nodiscard]] std::string_view type_name() const noexcept override { return "el"; }
    [[nodiscard]] bool is_geometry() const noexcept override { return true; }
    void collect_geometry(float frame, Path& out_path) const override;

    static std::shared_ptr<EllipseShape> parse(const json::JsonValue& val);
};

// Star / Polygon Shape ("sr")
class StarShape : public LottieShapeItem {
public:
    int type = 1; // 1 = Star, 2 = Polygon
    FloatProperty points{5.0f};
    PointProperty position{Point::zero()};
    FloatProperty rotation{0.0f};
    FloatProperty inner_radius{0.0f};
    FloatProperty inner_roundness{0.0f};
    FloatProperty outer_radius{0.0f};
    FloatProperty outer_roundness{0.0f};

    [[nodiscard]] std::string_view type_name() const noexcept override { return "sr"; }
    [[nodiscard]] bool is_geometry() const noexcept override { return true; }
    void collect_geometry(float frame, Path& out_path) const override;

    static std::shared_ptr<StarShape> parse(const json::JsonValue& val);
};

// Solid Fill ("fl")
class FillShape : public LottieShapeItem {
public:
    ColorProperty color{Color::WHITE};
    FloatProperty opacity{100.0f};
    FillRule fill_rule = FillRule::NonZero;

    [[nodiscard]] std::string_view type_name() const noexcept override { return "fl"; }
    void render(RenderContext& ctx, float frame) const override;

    static std::shared_ptr<FillShape> parse(const json::JsonValue& val);
};

// Solid Stroke ("st")
class StrokeShape : public LottieShapeItem {
public:
    ColorProperty color{Color::WHITE};
    FloatProperty opacity{100.0f};
    FloatProperty width{1.0f};
    LineCap cap = LineCap::Round;
    LineJoin join = LineJoin::Round;
    float miter_limit = 4.0f;
    std::vector<FloatProperty> dashes;
    FloatProperty dash_offset{0.0f};

    [[nodiscard]] std::string_view type_name() const noexcept override { return "st"; }
    void render(RenderContext& ctx, float frame) const override;

    static std::shared_ptr<StrokeShape> parse(const json::JsonValue& val);
};

// Gradient Stop Ramp
struct GradientColorStops {
    int count = 0; // Number of color stops
    FloatProperty raw_stops;

    [[nodiscard]] std::vector<GradientStop> evaluate_stops(float frame) const;
};

// Gradient Fill ("gf")
class GradientFillShape : public LottieShapeItem {
public:
    GradientType gradient_type = GradientType::Linear;
    PointProperty start_point{Point::zero()};
    PointProperty end_point{Point::zero()};
    GradientColorStops stops;
    FloatProperty opacity{100.0f};
    FillRule fill_rule = FillRule::NonZero;

    [[nodiscard]] std::string_view type_name() const noexcept override { return "gf"; }
    void render(RenderContext& ctx, float frame) const override;

    static std::shared_ptr<GradientFillShape> parse(const json::JsonValue& val);
};

// Trim Path Modifier ("tm")
class TrimPathShape : public LottieShapeItem {
public:
    FloatProperty start{0.0f};    // [0, 100]%
    FloatProperty end{100.0f};    // [0, 100]%
    FloatProperty offset{0.0f};   // [0, 360] degrees
    TrimType trim_type = TrimType::Simultaneously;

    [[nodiscard]] std::string_view type_name() const noexcept override { return "tm"; }
    [[nodiscard]] bool is_modifier() const noexcept override { return true; }

    [[nodiscard]] Path apply_trim(const Path& in_path, float frame) const;

    static std::shared_ptr<TrimPathShape> parse(const json::JsonValue& val);
};

// Context passed through shape tree rendering
class RenderContext {
public:
    explicit RenderContext(ICanvas& c, float parent_op = 1.0f)
        : canvas(c), parent_opacity(parent_op) {}

    ICanvas& canvas;
    float parent_opacity = 1.0f;
    std::vector<std::shared_ptr<TrimPathShape>> active_trims;
    Path accumulated_path;

    void add_path(const Path& p) {
        accumulated_path.add_path(p);
    }

    [[nodiscard]] Path get_trimmed_path(float frame) const;
};

} // namespace nisaba::lottie

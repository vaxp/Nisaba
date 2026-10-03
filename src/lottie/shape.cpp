/// @file shape.cpp
/// @brief Implementation of Lottie shape primitives, geometry, and rendering.

#include "nisaba/lottie/shape.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/math/transform.hpp"

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <algorithm>

namespace nisaba::lottie {

namespace {

constexpr float KAPPA = 0.5522847498307935f;

} // anonymous namespace

// ── PathShape ─────────────────────────────────────────────────────────────

void PathShape::collect_geometry(float frame, Path& out_path) const {
    BezierData bz = shape.evaluate(frame);
    Path p = bz.to_path();
    out_path.add_path(p);
}

std::shared_ptr<PathShape> PathShape::parse(const json::JsonValue& val) {
    auto ps = std::make_shared<PathShape>();
    if (const auto* ks = val.get("ks")) {
        ps->shape = parse_shape_property(*ks);
    }
    return ps;
}

// ── RectShape ─────────────────────────────────────────────────────────────

void RectShape::collect_geometry(float frame, Path& out_path) const {
    Point pos = position.evaluate(frame);
    Point sz = size.evaluate(frame);
    float r = roundness.evaluate(frame);

    if (sz.x <= 0.0f || sz.y <= 0.0f) return;

    float x0 = pos.x - sz.x * 0.5f;
    float y0 = pos.y - sz.y * 0.5f;
    float x1 = pos.x + sz.x * 0.5f;
    float y1 = pos.y + sz.y * 0.5f;

    r = std::clamp(r, 0.0f, std::min(sz.x, sz.y) * 0.5f);

    PathBuilder builder;
    if (r <= 0.0f) {
        builder.move_to(x0, y0);
        builder.line_to(x1, y0);
        builder.line_to(x1, y1);
        builder.line_to(x0, y1);
        builder.close();
    } else {
        float ox = r * KAPPA;
        float oy = r * KAPPA;
        builder.move_to(x0 + r, y0);
        builder.line_to(x1 - r, y0);
        builder.cubic_to(x1 - r + ox, y0, x1, y0 + r - oy, x1, y0 + r);
        builder.line_to(x1, y1 - r);
        builder.cubic_to(x1, y1 - r + oy, x1 - r + ox, y1, x1 - r, y1);
        builder.line_to(x0 + r, y1);
        builder.cubic_to(x0 + r - ox, y1, x0, y1 - r + oy, x0, y1 - r);
        builder.line_to(x0, y0 + r);
        builder.cubic_to(x0, y0 + r - oy, x0 + r - ox, y0, x0 + r, y0);
        builder.close();
    }

    if (auto p = builder.finish()) {
        out_path.add_path(*p);
    }
}

std::shared_ptr<RectShape> RectShape::parse(const json::JsonValue& val) {
    auto rs = std::make_shared<RectShape>();
    if (const auto* p = val.get("p")) rs->position = parse_point_property(*p, Point::zero());
    if (const auto* s = val.get("s")) rs->size = parse_point_property(*s, Point::zero());
    if (const auto* r = val.get("r")) rs->roundness = parse_float_property(*r, 0.0f);
    return rs;
}

// ── EllipseShape ──────────────────────────────────────────────────────────

void EllipseShape::collect_geometry(float frame, Path& out_path) const {
    Point pos = position.evaluate(frame);
    Point sz = size.evaluate(frame);

    if (sz.x <= 0.0f || sz.y <= 0.0f) return;

    float rx = sz.x * 0.5f;
    float ry = sz.y * 0.5f;
    float ox = rx * KAPPA;
    float oy = ry * KAPPA;

    PathBuilder builder;
    builder.move_to(pos.x, pos.y - ry);
    builder.cubic_to(pos.x + ox, pos.y - ry, pos.x + rx, pos.y - oy, pos.x + rx, pos.y);
    builder.cubic_to(pos.x + rx, pos.y + oy, pos.x + ox, pos.y + ry, pos.x, pos.y + ry);
    builder.cubic_to(pos.x - ox, pos.y + ry, pos.x - rx, pos.y + oy, pos.x - rx, pos.y);
    builder.cubic_to(pos.x - rx, pos.y - oy, pos.x - ox, pos.y - ry, pos.x, pos.y - ry);
    builder.close();

    if (auto p = builder.finish()) {
        out_path.add_path(*p);
    }
}

std::shared_ptr<EllipseShape> EllipseShape::parse(const json::JsonValue& val) {
    auto es = std::make_shared<EllipseShape>();
    if (const auto* p = val.get("p")) es->position = parse_point_property(*p, Point::zero());
    if (const auto* s = val.get("s")) es->size = parse_point_property(*s, Point::zero());
    return es;
}

// ── StarShape ─────────────────────────────────────────────────────────────

void StarShape::collect_geometry(float frame, Path& out_path) const {
    Point pos = position.evaluate(frame);
    float num_pts = std::max(3.0f, points.evaluate(frame));
    float rot_rad = (rotation.evaluate(frame) - 90.0f) * static_cast<float>(M_PI / 180.0);
    float orad = outer_radius.evaluate(frame);
    float irad = (type == 1) ? inner_radius.evaluate(frame) : orad;

    if (orad <= 0.0f) return;

    int total_vertices = (type == 1) ? static_cast<int>(num_pts * 2.0f) : static_cast<int>(num_pts);
    float angle_step = static_cast<float>(2.0 * M_PI) / static_cast<float>(total_vertices);

    PathBuilder builder;
    for (int i = 0; i < total_vertices; ++i) {
        float r = (type == 1 && (i % 2 == 1)) ? irad : orad;
        float a = rot_rad + i * angle_step;
        float x = pos.x + r * std::cos(a);
        float y = pos.y + r * std::sin(a);
        if (i == 0) builder.move_to(x, y);
        else builder.line_to(x, y);
    }
    builder.close();

    if (auto p = builder.finish()) {
        out_path.add_path(*p);
    }
}

std::shared_ptr<StarShape> StarShape::parse(const json::JsonValue& val) {
    auto ss = std::make_shared<StarShape>();
    ss->type = val.get_int("sy", 1);
    if (const auto* pt = val.get("pt")) ss->points = parse_float_property(*pt, 5.0f);
    if (const auto* p = val.get("p")) ss->position = parse_point_property(*p, Point::zero());
    if (const auto* r = val.get("r")) ss->rotation = parse_float_property(*r, 0.0f);
    if (const auto* ir = val.get("ir")) ss->inner_radius = parse_float_property(*ir, 0.0f);
    if (const auto* is = val.get("is")) ss->inner_roundness = parse_float_property(*is, 0.0f);
    if (const auto* or_val = val.get("or")) ss->outer_radius = parse_float_property(*or_val, 0.0f);
    if (const auto* os_val = val.get("os")) ss->outer_roundness = parse_float_property(*os_val, 0.0f);
    return ss;
}

// ── FillShape ─────────────────────────────────────────────────────────────

void FillShape::render(RenderContext& ctx, float frame) const {
    Path path = ctx.get_trimmed_path(frame);
    if (path.is_empty()) return;

    Color c = color.evaluate(frame);
    float op = opacity.evaluate(frame) * 0.01f * ctx.parent_opacity;
    if (op <= 0.0f) return;

    c.apply_opacity(op);

    Paint paint;
    paint.set_color(c);

    nisaba::FillRule fr = (fill_rule == FillRule::EvenOdd)
        ? nisaba::FillRule::EvenOdd
        : nisaba::FillRule::Winding;

    ctx.canvas.fill_path(path, paint, fr);
}

std::shared_ptr<FillShape> FillShape::parse(const json::JsonValue& val) {
    auto fs = std::make_shared<FillShape>();
    if (const auto* c = val.get("c")) fs->color = parse_color_property(*c, Color::WHITE);
    if (const auto* o = val.get("o")) fs->opacity = parse_float_property(*o, 100.0f);
    fs->fill_rule = (val.get_int("r", 1) == 2) ? FillRule::EvenOdd : FillRule::NonZero;
    return fs;
}

// ── StrokeShape ───────────────────────────────────────────────────────────

void StrokeShape::render(RenderContext& ctx, float frame) const {
    Path path = ctx.get_trimmed_path(frame);
    if (path.is_empty()) return;

    float w = width.evaluate(frame);
    if (w <= 0.0f) return;

    Color c = color.evaluate(frame);
    float op = opacity.evaluate(frame) * 0.01f * ctx.parent_opacity;
    if (op <= 0.0f) return;

    c.apply_opacity(op);

    Paint paint;
    paint.set_color(c);

    Stroke stroke(w);
    switch (cap) {
        case LineCap::Butt:   stroke.line_cap = nisaba::LineCap::Butt; break;
        case LineCap::Round:  stroke.line_cap = nisaba::LineCap::Round; break;
        case LineCap::Square: stroke.line_cap = nisaba::LineCap::Square; break;
    }
    switch (join) {
        case LineJoin::Miter: stroke.line_join = nisaba::LineJoin::Miter; break;
        case LineJoin::Round: stroke.line_join = nisaba::LineJoin::Round; break;
        case LineJoin::Bevel: stroke.line_join = nisaba::LineJoin::Bevel; break;
    }
    stroke.miter_limit = miter_limit;

    if (!dashes.empty()) {
        std::vector<float> dash_intervals;
        for (const auto& d : dashes) {
            dash_intervals.push_back(d.evaluate(frame));
        }
        float doffset = dash_offset.evaluate(frame);
        stroke.dash = StrokeDash::create(std::move(dash_intervals), doffset);
    }

    ctx.canvas.stroke_path(path, paint, stroke);
}

std::shared_ptr<StrokeShape> StrokeShape::parse(const json::JsonValue& val) {
    auto ss = std::make_shared<StrokeShape>();
    if (const auto* c = val.get("c")) ss->color = parse_color_property(*c, Color::WHITE);
    if (const auto* o = val.get("o")) ss->opacity = parse_float_property(*o, 100.0f);
    if (const auto* w = val.get("w")) ss->width = parse_float_property(*w, 1.0f);
    ss->cap = static_cast<LineCap>(val.get_int("lc", 2));
    ss->join = static_cast<LineJoin>(val.get_int("lj", 2));
    ss->miter_limit = val.get_float("ml", 4.0f);

    if (const auto* d_arr = val.get("d")) {
        if (d_arr->is_array()) {
            for (const auto& d_item : d_arr->as_array()) {
                if (!d_item.is_object()) continue;
                std::string_view n = d_item.get_string("n");
                if (const auto* v = d_item.get("v")) {
                    if (n == "o") {
                        ss->dash_offset = parse_float_property(*v, 0.0f);
                    } else if (n == "d" || n == "g") {
                        ss->dashes.push_back(parse_float_property(*v, 0.0f));
                    }
                }
            }
        }
    }
    return ss;
}

// ── GradientFillShape ─────────────────────────────────────────────────────

std::vector<GradientStop> GradientColorStops::evaluate_stops(float frame) const {
    (void)frame;
    std::vector<GradientStop> stops;
    // Default 2-stop ramp
    stops.push_back(GradientStop::create(0.0f, Color::WHITE));
    stops.push_back(GradientStop::create(1.0f, Color::BLACK));
    return stops;
}

void GradientFillShape::render(RenderContext& ctx, float frame) const {
    Path path = ctx.get_trimmed_path(frame);
    if (path.is_empty()) return;

    Point s = start_point.evaluate(frame);
    Point e = end_point.evaluate(frame);
    float op = opacity.evaluate(frame) * 0.01f * ctx.parent_opacity;
    if (op <= 0.0f) return;

    auto stops_vec = stops.evaluate_stops(frame);
    for (auto& st : stops_vec) {
        Color c = st.color;
        c.apply_opacity(op);
        st = GradientStop::create(st.position.get(), c);
    }

    Paint paint;
    if (gradient_type == GradientType::Linear) {
        if (auto grad = LinearGradient::create(s, e, stops_vec)) {
            paint.shader = Shader(*grad);
        }
    } else {
        float radius = s.distance(e);
        if (auto grad = RadialGradient::create(s, radius, stops_vec)) {
            paint.shader = Shader(*grad);
        }
    }

    nisaba::FillRule fr = (fill_rule == FillRule::EvenOdd)
        ? nisaba::FillRule::EvenOdd
        : nisaba::FillRule::Winding;

    ctx.canvas.fill_path(path, paint, fr);
}

std::shared_ptr<GradientFillShape> GradientFillShape::parse(const json::JsonValue& val) {
    auto gs = std::make_shared<GradientFillShape>();
    gs->gradient_type = (val.get_int("t", 1) == 2) ? GradientType::Radial : GradientType::Linear;
    if (const auto* s = val.get("s")) gs->start_point = parse_point_property(*s, Point::zero());
    if (const auto* e = val.get("e")) gs->end_point = parse_point_property(*e, Point::zero());
    if (const auto* o = val.get("o")) gs->opacity = parse_float_property(*o, 100.0f);
    gs->fill_rule = (val.get_int("r", 1) == 2) ? FillRule::EvenOdd : FillRule::NonZero;
    return gs;
}

// ── TrimPathShape ─────────────────────────────────────────────────────────

Path TrimPathShape::apply_trim(const Path& in_path, float frame) const {
    float s = start.evaluate(frame) * 0.01f;
    float e = end.evaluate(frame) * 0.01f;
    float o = std::fmod(offset.evaluate(frame) / 360.0f, 1.0f);

    if (o < 0.0f) o += 1.0f;
    s += o;
    e += o;

    float len = e - s;
    if (std::abs(len) >= 1.0f) {
        return in_path; // Full path
    }

    s = std::fmod(s, 1.0f);
    e = std::fmod(e, 1.0f);
    if (s < 0.0f) s += 1.0f;
    if (e < 0.0f) e += 1.0f;

    if (std::abs(s - e) < 1e-4f) {
        return Path(); // Zero-length trimmed path
    }

    // High-performance DashPathEffect slicing
    // A path of length L trimmed to [s, e] can be represented as a single dash run
    float total_len = in_path.bounds().width() + in_path.bounds().height();
    if (total_len <= 0.0f) total_len = 100.0f;

    float dash_len = (e >= s) ? (e - s) * total_len : (1.0f - (s - e)) * total_len;
    float gap_len = total_len - dash_len;

    auto dash = StrokeDash::create({dash_len, gap_len}, s * total_len);
    if (!dash) return in_path;

    if (auto res = dash_path(in_path, *dash, 1.0f)) {
        return *res;
    }
    return in_path;
}

std::shared_ptr<TrimPathShape> TrimPathShape::parse(const json::JsonValue& val) {
    auto ts = std::make_shared<TrimPathShape>();
    if (const auto* s = val.get("s")) ts->start = parse_float_property(*s, 0.0f);
    if (const auto* e = val.get("e")) ts->end = parse_float_property(*e, 100.0f);
    if (const auto* o = val.get("o")) ts->offset = parse_float_property(*o, 0.0f);
    ts->trim_type = (val.get_int("m", 1) == 2) ? TrimType::Individually : TrimType::Simultaneously;
    return ts;
}

// ── ShapeGroup ────────────────────────────────────────────────────────────

void ShapeGroup::collect_geometry(float frame, Path& out_path) const {
    Path local_path;
    for (const auto& it : items) {
        if (it->is_geometry()) {
            it->collect_geometry(frame, local_path);
        }
    }
    Transform tr = transform.matrix(frame);
    local_path.apply_transform(tr);
    out_path.add_path(local_path);
}

void ShapeGroup::render(RenderContext& ctx, float frame) const {
    ctx.canvas.save();
    Transform tr = transform.matrix(frame);
    ctx.canvas.concat(tr);

    float old_opacity = ctx.parent_opacity;
    ctx.parent_opacity *= transform.eval_opacity(frame);

    // Save active trims in context
    auto old_trims = ctx.active_trims;
    for (const auto& item : items) {
        if (item->type_name() == "tm") {
            ctx.active_trims.push_back(std::static_pointer_cast<TrimPathShape>(item));
        }
    }

    // In Lottie, shape items in a group are rendered in order:
    // First geometry items are collected, and style items (fills/strokes) draw them
    Path current_geometry;
    for (const auto& item : items) {
        if (item->is_geometry()) {
            item->collect_geometry(frame, current_geometry);
        } else if (item->type_name() == "fl" || item->type_name() == "st" || item->type_name() == "gf") {
            ctx.accumulated_path = current_geometry;
            item->render(ctx, frame);
        } else if (item->type_name() == "gr") {
            item->render(ctx, frame);
        }
    }

    ctx.active_trims = old_trims;
    ctx.parent_opacity = old_opacity;
    ctx.canvas.restore();
}

std::shared_ptr<ShapeGroup> ShapeGroup::parse(const json::JsonValue& val) {
    auto grp = std::make_shared<ShapeGroup>();
    grp->name = val.get_string("nm", "");

    if (const auto* it_arr = val.get("it")) {
        if (it_arr->is_array()) {
            for (const auto& it_val : it_arr->as_array()) {
                if (!it_val.is_object()) continue;
                std::string_view ty = it_val.get_string("ty");
                if (ty == "tr") {
                    grp->transform = LottieTransform::parse(it_val);
                } else if (auto item = LottieShapeItem::parse(it_val)) {
                    grp->items.push_back(item);
                }
            }
        }
    }
    return grp;
}

// ── ShapeItem Factory ─────────────────────────────────────────────────────

std::shared_ptr<LottieShapeItem> LottieShapeItem::parse(const json::JsonValue& val) {
    if (!val.is_object()) return nullptr;
    std::string_view ty = val.get_string("ty");

    if (ty == "gr") return ShapeGroup::parse(val);
    if (ty == "sh") return PathShape::parse(val);
    if (ty == "rc") return RectShape::parse(val);
    if (ty == "el") return EllipseShape::parse(val);
    if (ty == "sr") return StarShape::parse(val);
    if (ty == "fl") return FillShape::parse(val);
    if (ty == "st") return StrokeShape::parse(val);
    if (ty == "gf") return GradientFillShape::parse(val);
    if (ty == "tm") return TrimPathShape::parse(val);

    return nullptr;
}

Path RenderContext::get_trimmed_path(float frame) const {
    Path p = accumulated_path;
    for (const auto& trim : active_trims) {
        if (trim) {
            p = trim->apply_trim(p, frame);
        }
    }
    return p;
}

} // namespace nisaba::lottie

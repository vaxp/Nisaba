/// @file property.cpp
/// @brief Implementation of animatable properties and keyframe evaluator for Lottie.

#include "nisaba/lottie/property.hpp"
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

inline float lerp(float a, float b, float t) noexcept {
    return a + t * (b - a);
}

inline Point lerp(const Point& a, const Point& b, float t) noexcept {
    return Point::from_xy(lerp(a.x, b.x, t), lerp(a.y, b.y, t));
}

inline Color lerp(const Color& a, const Color& b, float t) noexcept {
    return Color::from_rgba_unchecked(
        lerp(a.red(), b.red(), t),
        lerp(a.green(), b.green(), t),
        lerp(a.blue(), b.blue(), t),
        lerp(a.alpha(), b.alpha(), t)
    );
}

inline BezierData lerp(const BezierData& a, const BezierData& b, float t) {
    return BezierData::interpolate(a, b, t);
}

} // anonymous namespace

float solve_bezier_easing(float x, const EasingHandle& out_h, const EasingHandle& in_h) noexcept {
    x = std::clamp(x, 0.0f, 1.0f);
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;

    float x1 = std::clamp(out_h.x, 0.0f, 1.0f);
    float y1 = out_h.y;
    float x2 = std::clamp(in_h.x, 0.0f, 1.0f);
    float y2 = in_h.y;

    // Linear fast path
    if (std::abs(x1 - y1) < 1e-4f && std::abs(x2 - y2) < 1e-4f) {
        return x;
    }

    // Binary search parameter s in [0, 1] such that BezierX(s) == x
    float low = 0.0f;
    float high = 1.0f;
    float s = x;

    for (int iter = 0; iter < 12; ++iter) {
        float one_minus_s = 1.0f - s;
        float bx = 3.0f * one_minus_s * one_minus_s * s * x1 +
                   3.0f * one_minus_s * s * s * x2 +
                   s * s * s;
        if (std::abs(bx - x) < 1e-4f) break;
        if (bx < x) {
            low = s;
        } else {
            high = s;
        }
        s = 0.5f * (low + high);
    }

    float one_minus_s = 1.0f - s;
    return 3.0f * one_minus_s * one_minus_s * s * y1 +
           3.0f * one_minus_s * s * s * y2 +
           s * s * s;
}

Path BezierData::to_path() const {
    if (vertices.empty()) return Path();

    PathBuilder builder;
    builder.move_to(vertices[0].x, vertices[0].y);

    size_t count = vertices.size();
    for (size_t i = 0; i + 1 < count; ++i) {
        Point cp1 = vertices[i] + out_tangents[i];
        Point cp2 = vertices[i + 1] + in_tangents[i + 1];
        Point p2 = vertices[i + 1];
        builder.cubic_to(cp1.x, cp1.y, cp2.x, cp2.y, p2.x, p2.y);
    }

    if (closed && count > 1) {
        Point cp1 = vertices[count - 1] + out_tangents[count - 1];
        Point cp2 = vertices[0] + in_tangents[0];
        Point p0 = vertices[0];
        builder.cubic_to(cp1.x, cp1.y, cp2.x, cp2.y, p0.x, p0.y);
        builder.close();
    }

    if (auto p = builder.finish()) {
        return *p;
    }
    return Path();
}

BezierData BezierData::interpolate(const BezierData& a, const BezierData& b, float t) {
    BezierData res;
    res.closed = a.closed || b.closed;
    size_t count = std::min(a.vertices.size(), b.vertices.size());
    res.vertices.resize(count);
    res.in_tangents.resize(count);
    res.out_tangents.resize(count);

    for (size_t i = 0; i < count; ++i) {
        res.vertices[i] = lerp(a.vertices[i], b.vertices[i], t);
        res.in_tangents[i] = lerp(a.in_tangents[i], b.in_tangents[i], t);
        res.out_tangents[i] = lerp(a.out_tangents[i], b.out_tangents[i], t);
    }
    return res;
}

template <typename T>
T Property<T>::evaluate(float frame) const {
    if (!is_animated_ || keyframes_.empty()) {
        return static_value_;
    }

    if (frame <= keyframes_.front().time) {
        return keyframes_.front().start_value;
    }
    if (frame >= keyframes_.back().time) {
        return keyframes_.back().end_value;
    }

    // Binary search for surrounding keyframes
    auto it = std::lower_bound(
        keyframes_.begin(), keyframes_.end(), frame,
        [](const Keyframe<T>& kf, float f) { return kf.time < f; }
    );

    if (it == keyframes_.begin()) return it->start_value;

    const auto& prev = *(it - 1);
    const auto& curr = *it;

    if (prev.is_hold) {
        return prev.start_value;
    }

    float span = curr.time - prev.time;
    if (span <= 1e-4f) return prev.end_value;

    float tau = (frame - prev.time) / span;
    float eased = solve_bezier_easing(tau, prev.out_handle, prev.in_handle);

    return lerp(prev.start_value, prev.end_value, eased);
}

// Explicit template instantiations
template class Property<float>;
template class Property<Point>;
template class Property<Color>;
template class Property<BezierData>;

// Helpers for JSON parsing
namespace {

EasingHandle parse_easing_coord(const json::JsonValue* val, float def_x, float def_y) {
    EasingHandle h{def_x, def_y};
    if (!val) return h;

    if (val->is_object()) {
        h.x = val->get_float("x", def_x);
        h.y = val->get_float("y", def_y);
    } else if (val->is_array()) {
        const auto& arr = val->as_array();
        if (!arr.empty()) h.x = arr[0].to_float(def_x);
        if (arr.size() > 1) h.y = arr[1].to_float(def_y);
    } else if (val->is_number()) {
        h.x = val->to_float(def_x);
        h.y = val->to_float(def_y);
    }
    return h;
}

Color parse_raw_color(const json::JsonValue& val) {
    if (val.is_array()) {
        const auto& arr = val.as_array();
        float r = (arr.size() > 0) ? arr[0].to_float() : 0.0f;
        float g = (arr.size() > 1) ? arr[1].to_float() : 0.0f;
        float b = (arr.size() > 2) ? arr[2].to_float() : 0.0f;
        float a = (arr.size() > 3) ? arr[3].to_float() : 1.0f;
        if (r > 1.0f || g > 1.0f || b > 1.0f) {
            return Color::from_rgba8(
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b),
                static_cast<uint8_t>(a > 1.0f ? a : a * 255.0f)
            );
        }
        return Color::from_rgba_unchecked(r, g, b, a);
    }
    return Color::WHITE;
}

BezierData parse_raw_bezier(const json::JsonValue& val) {
    BezierData bz;
    if (!val.is_object()) return bz;

    bz.closed = val.get_bool("c", false);

    const auto& v_arr = val.get_array("v");
    const auto& i_arr = val.get_array("i");
    const auto& o_arr = val.get_array("o");

    size_t count = v_arr.size();
    bz.vertices.reserve(count);
    bz.in_tangents.reserve(count);
    bz.out_tangents.reserve(count);

    for (size_t idx = 0; idx < count; ++idx) {
        const auto& v = v_arr[idx].as_array();
        bz.vertices.push_back(Point::from_xy(
            v.size() > 0 ? v[0].to_float() : 0.0f,
            v.size() > 1 ? v[1].to_float() : 0.0f
        ));

        if (idx < i_arr.size()) {
            const auto& in_t = i_arr[idx].as_array();
            bz.in_tangents.push_back(Point::from_xy(
                in_t.size() > 0 ? in_t[0].to_float() : 0.0f,
                in_t.size() > 1 ? in_t[1].to_float() : 0.0f
            ));
        } else {
            bz.in_tangents.push_back(Point::zero());
        }

        if (idx < o_arr.size()) {
            const auto& out_t = o_arr[idx].as_array();
            bz.out_tangents.push_back(Point::from_xy(
                out_t.size() > 0 ? out_t[0].to_float() : 0.0f,
                out_t.size() > 1 ? out_t[1].to_float() : 0.0f
            ));
        } else {
            bz.out_tangents.push_back(Point::zero());
        }
    }
    return bz;
}

} // anonymous namespace

FloatProperty parse_float_property(const json::JsonValue& val, float default_val) {
    if (val.is_number()) return FloatProperty(val.to_float(default_val));
    if (!val.is_object()) return FloatProperty(default_val);

    int animated = val.get_int("a", 0);
    const auto* k_val = val.get("k");
    if (!k_val) return FloatProperty(default_val);

    if (animated == 0) {
        if (k_val->is_number()) return FloatProperty(k_val->to_float(default_val));
        if (k_val->is_array() && !k_val->as_array().empty()) {
            return FloatProperty(k_val->as_array()[0].to_float(default_val));
        }
        return FloatProperty(default_val);
    }

    FloatProperty prop;
    if (k_val->is_array()) {
        const auto& arr = k_val->as_array();
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& item = arr[i];
            if (!item.is_object()) continue;

            Keyframe<float> kf;
            kf.time = item.get_float("t", 0.0f);
            kf.is_hold = (item.get_int("h", 0) != 0);

            const auto* s = item.get("s");
            if (s && s->is_array() && !s->as_array().empty()) {
                kf.start_value = s->as_array()[0].to_float();
            } else if (s && s->is_number()) {
                kf.start_value = s->to_float();
            }

            const auto* e = item.get("e");
            if (e && e->is_array() && !e->as_array().empty()) {
                kf.end_value = e->as_array()[0].to_float();
            } else if (e && e->is_number()) {
                kf.end_value = e->to_float();
            } else if (i + 1 < arr.size() && arr[i + 1].has("s")) {
                const auto* next_s = arr[i + 1].get("s");
                if (next_s && next_s->is_array() && !next_s->as_array().empty()) {
                    kf.end_value = next_s->as_array()[0].to_float();
                }
            } else {
                kf.end_value = kf.start_value;
            }

            kf.out_handle = parse_easing_coord(item.get("o"), 0.167f, 0.167f);
            kf.in_handle = parse_easing_coord(item.get("i"), 0.833f, 0.833f);

            prop.add_keyframe(std::move(kf));
        }
    }
    return prop;
}

PointProperty parse_point_property(const json::JsonValue& val, Point default_val) {
    if (val.is_array()) {
        const auto& a = val.as_array();
        return PointProperty(Point::from_xy(
            a.size() > 0 ? a[0].to_float() : default_val.x,
            a.size() > 1 ? a[1].to_float() : default_val.y
        ));
    }
    if (!val.is_object()) return PointProperty(default_val);

    int animated = val.get_int("a", 0);
    const auto* k_val = val.get("k");
    if (!k_val) return PointProperty(default_val);

    if (animated == 0) {
        if (k_val->is_array()) {
            const auto& a = k_val->as_array();
            return PointProperty(Point::from_xy(
                a.size() > 0 ? a[0].to_float() : default_val.x,
                a.size() > 1 ? a[1].to_float() : default_val.y
            ));
        }
        return PointProperty(default_val);
    }

    PointProperty prop;
    if (k_val->is_array()) {
        const auto& arr = k_val->as_array();
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& item = arr[i];
            if (!item.is_object()) continue;

            Keyframe<Point> kf;
            kf.time = item.get_float("t", 0.0f);
            kf.is_hold = (item.get_int("h", 0) != 0);

            const auto* s = item.get("s");
            if (s && s->is_array()) {
                const auto& a = s->as_array();
                kf.start_value = Point::from_xy(
                    a.size() > 0 ? a[0].to_float() : 0.0f,
                    a.size() > 1 ? a[1].to_float() : 0.0f
                );
            }

            const auto* e = item.get("e");
            if (e && e->is_array()) {
                const auto& a = e->as_array();
                kf.end_value = Point::from_xy(
                    a.size() > 0 ? a[0].to_float() : 0.0f,
                    a.size() > 1 ? a[1].to_float() : 0.0f
                );
            } else if (i + 1 < arr.size() && arr[i + 1].has("s")) {
                const auto* next_s = arr[i + 1].get("s");
                if (next_s && next_s->is_array()) {
                    const auto& a = next_s->as_array();
                    kf.end_value = Point::from_xy(
                        a.size() > 0 ? a[0].to_float() : 0.0f,
                        a.size() > 1 ? a[1].to_float() : 0.0f
                    );
                }
            } else {
                kf.end_value = kf.start_value;
            }

            kf.out_handle = parse_easing_coord(item.get("o"), 0.167f, 0.167f);
            kf.in_handle = parse_easing_coord(item.get("i"), 0.833f, 0.833f);

            prop.add_keyframe(std::move(kf));
        }
    }
    return prop;
}

PointProperty parse_scale_property(const json::JsonValue& val, Point default_val) {
    return parse_point_property(val, default_val);
}

ColorProperty parse_color_property(const json::JsonValue& val, Color default_val) {
    if (val.is_array()) {
        return ColorProperty(parse_raw_color(val));
    }
    if (!val.is_object()) return ColorProperty(default_val);

    int animated = val.get_int("a", 0);
    const auto* k_val = val.get("k");
    if (!k_val) return ColorProperty(default_val);

    if (animated == 0) {
        return ColorProperty(parse_raw_color(*k_val));
    }

    ColorProperty prop;
    if (k_val->is_array()) {
        const auto& arr = k_val->as_array();
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& item = arr[i];
            if (!item.is_object()) continue;

            Keyframe<Color> kf;
            kf.time = item.get_float("t", 0.0f);
            kf.is_hold = (item.get_int("h", 0) != 0);

            if (const auto* s = item.get("s")) {
                kf.start_value = parse_raw_color(*s);
            }
            if (const auto* e = item.get("e")) {
                kf.end_value = parse_raw_color(*e);
            } else if (i + 1 < arr.size() && arr[i + 1].has("s")) {
                if (const auto* next_s = arr[i + 1].get("s")) {
                    kf.end_value = parse_raw_color(*next_s);
                }
            } else {
                kf.end_value = kf.start_value;
            }

            kf.out_handle = parse_easing_coord(item.get("o"), 0.167f, 0.167f);
            kf.in_handle = parse_easing_coord(item.get("i"), 0.833f, 0.833f);

            prop.add_keyframe(std::move(kf));
        }
    }
    return prop;
}

ShapeProperty parse_shape_property(const json::JsonValue& val) {
    ShapeProperty prop;
    if (!val.is_object()) return prop;

    int animated = val.get_int("a", 0);
    const auto* k_val = val.get("k");
    if (!k_val) return prop;

    if (animated == 0) {
        prop.set_static(parse_raw_bezier(*k_val));
        return prop;
    }

    if (k_val->is_array()) {
        const auto& arr = k_val->as_array();
        for (size_t i = 0; i < arr.size(); ++i) {
            const auto& item = arr[i];
            if (!item.is_object()) continue;

            Keyframe<BezierData> kf;
            kf.time = item.get_float("t", 0.0f);
            kf.is_hold = (item.get_int("h", 0) != 0);

            if (const auto* s = item.get("s")) {
                if (s->is_array() && !s->as_array().empty()) {
                    kf.start_value = parse_raw_bezier(s->as_array()[0]);
                } else if (s->is_object()) {
                    kf.start_value = parse_raw_bezier(*s);
                }
            }

            if (const auto* e = item.get("e")) {
                if (e->is_array() && !e->as_array().empty()) {
                    kf.end_value = parse_raw_bezier(e->as_array()[0]);
                } else if (e->is_object()) {
                    kf.end_value = parse_raw_bezier(*e);
                }
            } else if (i + 1 < arr.size() && arr[i + 1].has("s")) {
                const auto* next_s = arr[i + 1].get("s");
                if (next_s && next_s->is_array() && !next_s->as_array().empty()) {
                    kf.end_value = parse_raw_bezier(next_s->as_array()[0]);
                } else if (next_s && next_s->is_object()) {
                    kf.end_value = parse_raw_bezier(*next_s);
                }
            } else {
                kf.end_value = kf.start_value;
            }

            kf.out_handle = parse_easing_coord(item.get("o"), 0.167f, 0.167f);
            kf.in_handle = parse_easing_coord(item.get("i"), 0.833f, 0.833f);

            prop.add_keyframe(std::move(kf));
        }
    }
    return prop;
}

LottieTransform LottieTransform::parse(const json::JsonValue& val) {
    LottieTransform tr;
    if (!val.is_object()) return tr;

    if (val.has("a")) tr.anchor = parse_point_property(*val.get("a"), Point::zero());
    if (val.has("s")) tr.scale = parse_scale_property(*val.get("s"), Point::from_xy(100.0f, 100.0f));
    if (val.has("r")) tr.rotation = parse_float_property(*val.get("r"), 0.0f);
    else if (val.has("rz")) tr.rotation = parse_float_property(*val.get("rz"), 0.0f);
    if (val.has("o")) tr.opacity = parse_float_property(*val.get("o"), 100.0f);

    // Position or split dimensions
    if (const auto* p_val = val.get("p")) {
        if (p_val->is_object() && p_val->get_bool("s", false)) {
            // Split dimensions: x and y are independent properties
            if (p_val->has("x")) tr.x_pos = parse_float_property(*p_val->get("x"), 0.0f);
            if (p_val->has("y")) tr.y_pos = parse_float_property(*p_val->get("y"), 0.0f);
        } else {
            tr.position = parse_point_property(*p_val, Point::zero());
        }
    }

    return tr;
}

Transform LottieTransform::matrix(float frame) const {
    Point pos = position.evaluate(frame);
    if (x_pos.has_value()) pos.x = x_pos->evaluate(frame);
    if (y_pos.has_value()) pos.y = y_pos->evaluate(frame);

    Point anc = anchor.evaluate(frame);
    Point sc = scale.evaluate(frame);
    float rot_deg = rotation.evaluate(frame);

    Transform t = Transform::from_translate(pos.x, pos.y);
    if (std::abs(rot_deg) > 1e-4f) {
        t = t.pre_rotate(rot_deg);
    }
    t = t.pre_scale(sc.x * 0.01f, sc.y * 0.01f);
    t = t.pre_translate(-anc.x, -anc.y);

    return t;
}

float LottieTransform::eval_opacity(float frame) const {
    return std::clamp(opacity.evaluate(frame) * 0.01f, 0.0f, 1.0f);
}

} // namespace nisaba::lottie

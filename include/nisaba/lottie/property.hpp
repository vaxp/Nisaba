#pragma once

/// @file property.hpp
/// @brief Animatable properties and keyframe easing evaluator for Lottie in Nisaba.

#include "nisaba/lottie/lottie_types.hpp"
#include "nisaba/json/json.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/path/path.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <string>

namespace nisaba::lottie {

struct EasingHandle {
    float x = 0.0f;
    float y = 0.0f;
};

template <typename T>
struct Keyframe {
    float time = 0.0f;         // Frame number
    T start_value{};
    T end_value{};
    EasingHandle out_handle{0.167f, 0.167f}; // "o" handle leaving start
    EasingHandle in_handle{0.833f, 0.833f};  // "i" handle entering end
    bool is_hold = false;      // Hold keyframe (step)
};

// Shape/Bezier keyframe data
struct BezierData {
    std::vector<Point> vertices;
    std::vector<Point> in_tangents;   // Relative to vertex
    std::vector<Point> out_tangents;  // Relative to vertex
    bool closed = false;

    [[nodiscard]] Path to_path() const;
    [[nodiscard]] static BezierData interpolate(const BezierData& a, const BezierData& b, float t);
};

// Evaluates cubic bezier timing curve Y given normalized time X in [0, 1]
float solve_bezier_easing(float x, const EasingHandle& out_h, const EasingHandle& in_h) noexcept;

template <typename T>
class Property {
public:
    Property() = default;
    explicit Property(T static_val) : static_value_(std::move(static_val)), is_animated_(false) {}

    [[nodiscard]] bool is_animated() const noexcept { return is_animated_; }
    [[nodiscard]] const std::vector<Keyframe<T>>& keyframes() const noexcept { return keyframes_; }

    void set_static(T val) {
        static_value_ = std::move(val);
        is_animated_ = false;
        keyframes_.clear();
    }

    void add_keyframe(Keyframe<T> kf) {
        keyframes_.push_back(std::move(kf));
        is_animated_ = true;
    }

    [[nodiscard]] T evaluate(float frame) const;

private:
    T static_value_{};
    std::vector<Keyframe<T>> keyframes_;
    bool is_animated_ = false;
};

// Typed aliases
using FloatProperty = Property<float>;
using PointProperty = Property<Point>;
using ColorProperty = Property<Color>;
using ShapeProperty = Property<BezierData>;

// JSON parsing helper functions for properties
FloatProperty parse_float_property(const json::JsonValue& val, float default_val = 0.0f);
PointProperty parse_point_property(const json::JsonValue& val, Point default_val = Point::zero());
PointProperty parse_scale_property(const json::JsonValue& val, Point default_val = Point::from_xy(100.0f, 100.0f));
ColorProperty parse_color_property(const json::JsonValue& val, Color default_val = Color::WHITE);
ShapeProperty parse_shape_property(const json::JsonValue& val);

// Transform definition (anchor, position, scale, rotation, opacity, skew)
struct LottieTransform {
    PointProperty anchor{Point::zero()};
    PointProperty position{Point::zero()};
    PointProperty scale{Point::from_xy(100.0f, 100.0f)};
    FloatProperty rotation{0.0f};
    FloatProperty opacity{100.0f};

    // Split position dimensions
    std::optional<FloatProperty> x_pos;
    std::optional<FloatProperty> y_pos;

    static LottieTransform parse(const json::JsonValue& val);
    [[nodiscard]] Transform matrix(float frame) const;
    [[nodiscard]] float eval_opacity(float frame) const;
};

} // namespace nisaba::lottie

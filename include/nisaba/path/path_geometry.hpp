#pragma once

#include <cstdint>
#include <cmath>
#include <optional>
#include <algorithm>
#include <array>
#include "nisaba/types.hpp"
#include "nisaba/math/scalar.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/math/f32x2.hpp"

namespace nisaba {

enum class PathDirection {
    CW,
    CCW,
};

namespace path_geometry {

inline f32x2 interp(f32x2 v0, f32x2 v1, f32x2 t) noexcept {
    return v0 + (v1 - v0) * t;
}

inline f32x2 times_2(f32x2 value) noexcept {
    return value + value;
}

inline bool between(float a, float b, float c) noexcept {
    return (a - b) * (c - b) <= 0.0f;
}

struct QuadCoeff {
    f32x2 a;
    f32x2 b;
    f32x2 c;

    static QuadCoeff from_points(const Point points[3]) noexcept {
        f32x2 c_ = points[0].to_f32x2();
        f32x2 p1 = points[1].to_f32x2();
        f32x2 p2 = points[2].to_f32x2();
        f32x2 b_ = times_2(p1 - c_);
        f32x2 a_ = p2 - times_2(p1) + c_;
        return QuadCoeff{a_, b_, c_};
    }

    f32x2 eval(f32x2 t) const noexcept {
        return (a * t + b) * t + c;
    }
};

struct CubicCoeff {
    f32x2 a;
    f32x2 b;
    f32x2 c;
    f32x2 d;

    static CubicCoeff from_points(const Point points[4]) noexcept {
        f32x2 p0 = points[0].to_f32x2();
        f32x2 p1 = points[1].to_f32x2();
        f32x2 p2 = points[2].to_f32x2();
        f32x2 p3 = points[3].to_f32x2();
        f32x2 three = f32x2::splat(3.0f);

        return CubicCoeff{
            p3 + three * (p1 - p2) - p0,
            three * (p2 - times_2(p1) + p0),
            three * (p1 - p0),
            p0
        };
    }

    f32x2 eval(f32x2 t) const noexcept {
        return ((a * t + b) * t + c) * t + d;
    }
};

inline std::optional<NormalizedF32Exclusive> valid_unit_divide(float numer, float denom) noexcept {
    if (numer < 0.0f) {
        numer = -numer;
        denom = -denom;
    }

    if (denom == 0.0f || numer == 0.0f || numer >= denom) {
        return std::nullopt;
    }

    float r = numer / denom;
    return NormalizedF32Exclusive::create(r);
}

inline void chop_quad_at(const Point src[3], NormalizedF32Exclusive t, Point dst[5]) noexcept {
    f32x2 p0 = src[0].to_f32x2();
    f32x2 p1 = src[1].to_f32x2();
    f32x2 p2 = src[2].to_f32x2();
    f32x2 tt = f32x2::splat(t.get());

    f32x2 p01 = interp(p0, p1, tt);
    f32x2 p12 = interp(p1, p2, tt);

    dst[0] = Point::from_f32x2(p0);
    dst[1] = Point::from_f32x2(p01);
    dst[2] = Point::from_f32x2(interp(p01, p12, tt));
    dst[3] = Point::from_f32x2(p12);
    dst[4] = Point::from_f32x2(p2);
}

inline void chop_cubic_at2(const Point src[4], NormalizedF32Exclusive t, Point dst[7]) noexcept {
    f32x2 p0 = src[0].to_f32x2();
    f32x2 p1 = src[1].to_f32x2();
    f32x2 p2 = src[2].to_f32x2();
    f32x2 p3 = src[3].to_f32x2();
    f32x2 tt = f32x2::splat(t.get());

    f32x2 ab = interp(p0, p1, tt);
    f32x2 bc = interp(p1, p2, tt);
    f32x2 cd = interp(p2, p3, tt);
    f32x2 abc = interp(ab, bc, tt);
    f32x2 bcd = interp(bc, cd, tt);
    f32x2 abcd = interp(abc, bcd, tt);

    dst[0] = Point::from_f32x2(p0);
    dst[1] = Point::from_f32x2(ab);
    dst[2] = Point::from_f32x2(abc);
    dst[3] = Point::from_f32x2(abcd);
    dst[4] = Point::from_f32x2(bcd);
    dst[5] = Point::from_f32x2(cd);
    dst[6] = Point::from_f32x2(p3);
}

inline size_t find_unit_quad_roots(float a, float b, float c, NormalizedF32Exclusive roots[3]) noexcept {
    if (a == 0.0f) {
        auto r = valid_unit_divide(-c, b);
        if (r) {
            roots[0] = *r;
            return 1;
        }
        return 0;
    }

    double dr = static_cast<double>(b) * static_cast<double>(b) - 4.0 * static_cast<double>(a) * static_cast<double>(c);
    if (dr < 0.0) {
        return 0;
    }
    dr = std::sqrt(dr);
    float r = static_cast<float>(dr);
    if (!std::isfinite(r)) {
        return 0;
    }

    float q = (b < 0.0f) ? -(b - r) * 0.5f : -(b + r) * 0.5f;

    size_t roots_offset = 0;
    auto r1 = valid_unit_divide(q, a);
    if (r1) {
        roots[roots_offset++] = *r1;
    }

    auto r2 = valid_unit_divide(c, q);
    if (r2) {
        roots[roots_offset++] = *r2;
    }

    if (roots_offset == 2) {
        if (roots[0].get() > roots[1].get()) {
            std::swap(roots[0], roots[1]);
        } else if (roots[0] == roots[1]) {
            roots_offset--;
        }
    }

    return roots_offset;
}

inline std::optional<NormalizedF32Exclusive> find_quad_extrema(float a, float b, float c) noexcept {
    return valid_unit_divide(a - b, a - b - b + c);
}

inline bool is_not_monotonic(float a, float b, float c) noexcept {
    float ab = a - b;
    float bc = b - c;
    if (ab < 0.0f) {
        bc = -bc;
    }
    return ab == 0.0f || bc < 0.0f;
}

inline size_t chop_quad_at_x_extrema(const Point src[3], Point dst[5]) noexcept {
    float a = src[0].x;
    float b = src[1].x;
    float c = src[2].x;

    if (is_not_monotonic(a, b, c)) {
        auto t_value = valid_unit_divide(a - b, a - b - b + c);
        if (t_value) {
            chop_quad_at(src, *t_value, dst);
            dst[1].x = dst[2].x;
            dst[3].x = dst[2].x;
            return 1;
        }
        b = (std::abs(a - b) < std::abs(b - c)) ? a : c;
    }

    dst[0] = Point::from_xy(a, src[0].y);
    dst[1] = Point::from_xy(b, src[1].y);
    dst[2] = Point::from_xy(c, src[2].y);
    return 0;
}

inline size_t chop_quad_at_y_extrema(const Point src[3], Point dst[5]) noexcept {
    float a = src[0].y;
    float b = src[1].y;
    float c = src[2].y;

    if (is_not_monotonic(a, b, c)) {
        auto t_value = valid_unit_divide(a - b, a - b - b + c);
        if (t_value) {
            chop_quad_at(src, *t_value, dst);
            dst[1].y = dst[2].y;
            dst[3].y = dst[2].y;
            return 1;
        }
        b = (std::abs(a - b) < std::abs(b - c)) ? a : c;
    }

    dst[0] = Point::from_xy(src[0].x, a);
    dst[1] = Point::from_xy(src[1].x, b);
    dst[2] = Point::from_xy(src[2].x, c);
    return 0;
}

inline size_t find_cubic_extrema(float a, float b, float c, float d, NormalizedF32Exclusive t_values[3]) noexcept {
    float na = d - a + 3.0f * (b - c);
    float nb = 2.0f * (a - b - b + c);
    float nc = b - a;

    return find_unit_quad_roots(na, nb, nc, t_values);
}

inline void chop_cubic_at(const Point src[4], const NormalizedF32Exclusive* t_values, size_t t_count, Point dst[]) noexcept {
    if (t_count == 0) {
        dst[0] = src[0];
        dst[1] = src[1];
        dst[2] = src[2];
        dst[3] = src[3];
    } else {
        NormalizedF32Exclusive t = t_values[0];
        std::array<Point, 4> tmp{};
        const Point* current_src = src;

        size_t dst_offset = 0;
        for (size_t i = 0; i < t_count; ++i) {
            chop_cubic_at2(current_src, t, &dst[dst_offset]);
            if (i == t_count - 1) {
                break;
            }

            dst_offset += 3;
            tmp[0] = dst[dst_offset + 0];
            tmp[1] = dst[dst_offset + 1];
            tmp[2] = dst[dst_offset + 2];
            tmp[3] = dst[dst_offset + 3];
            current_src = tmp.data();

            auto n = valid_unit_divide(
                t_values[i + 1].get() - t_values[i].get(),
                1.0f - t_values[i].get()
            );

            if (n) {
                t = *n;
            } else {
                dst[dst_offset + 4] = current_src[3];
                dst[dst_offset + 5] = current_src[3];
                dst[dst_offset + 6] = current_src[3];
                break;
            }
        }
    }
}

inline size_t chop_cubic_at_x_extrema(const Point src[4], Point dst[10]) noexcept {
    std::array<NormalizedF32Exclusive, 3> t_values{};
    size_t count = find_cubic_extrema(src[0].x, src[1].x, src[2].x, src[3].x, t_values.data());

    chop_cubic_at(src, t_values.data(), count, dst);
    if (count > 0) {
        dst[2].x = dst[3].x;
        dst[4].x = dst[3].x;
        if (count == 2) {
            dst[5].x = dst[6].x;
            dst[7].x = dst[6].x;
        }
    }

    return count;
}

inline size_t chop_cubic_at_y_extrema(const Point src[4], Point dst[10]) noexcept {
    std::array<NormalizedF32Exclusive, 3> t_values{};
    size_t count = find_cubic_extrema(src[0].y, src[1].y, src[2].y, src[3].y, t_values.data());

    chop_cubic_at(src, t_values.data(), count, dst);
    if (count > 0) {
        dst[2].y = dst[3].y;
        dst[4].y = dst[3].y;
        if (count == 2) {
            dst[5].y = dst[6].y;
            dst[7].y = dst[6].y;
        }
    }

    return count;
}

inline bool chop_mono_cubic_at_x(const Point src[4], float x, Point dst[7]) noexcept {
    double d = static_cast<double>(src[0].x) - static_cast<double>(x);
    double c = 3.0 * (static_cast<double>(src[1].x) - static_cast<double>(src[0].x));
    double b = 3.0 * (static_cast<double>(src[2].x) - 2.0 * static_cast<double>(src[1].x) + static_cast<double>(src[0].x));
    double a = static_cast<double>(src[3].x) - 3.0 * static_cast<double>(src[2].x) + 3.0 * static_cast<double>(src[1].x) - static_cast<double>(src[0].x);

    double t = 0.5;
    double t_low = 0.0;
    double t_high = 1.0;
    for (int iter = 0; iter < 24; ++iter) {
        double val = ((a * t + b) * t + c) * t + d;
        if (std::abs(val) < 1e-7) break;
        if (src[3].x > src[0].x) {
            if (val < 0.0) t_low = t;
            else t_high = t;
        } else {
            if (val > 0.0) t_low = t;
            else t_high = t;
        }
        t = (t_low + t_high) * 0.5;
    }

    auto t_opt = NormalizedF32Exclusive::create(static_cast<float>(t));
    if (t_opt) {
        chop_cubic_at2(src, *t_opt, dst);
        return true;
    }
    return false;
}

inline bool chop_mono_cubic_at_y(const Point src[4], float y, Point dst[7]) noexcept {
    double d = static_cast<double>(src[0].y) - static_cast<double>(y);
    double c = 3.0 * (static_cast<double>(src[1].y) - static_cast<double>(src[0].y));
    double b = 3.0 * (static_cast<double>(src[2].y) - 2.0 * static_cast<double>(src[1].y) + static_cast<double>(src[0].y));
    double a = static_cast<double>(src[3].y) - 3.0 * static_cast<double>(src[2].y) + 3.0 * static_cast<double>(src[1].y) - static_cast<double>(src[0].y);

    double t = 0.5;
    double t_low = 0.0;
    double t_high = 1.0;
    for (int iter = 0; iter < 24; ++iter) {
        double val = ((a * t + b) * t + c) * t + d;
        if (std::abs(val) < 1e-7) break;
        if (src[3].y > src[0].y) {
            if (val < 0.0) t_low = t;
            else t_high = t;
        } else {
            if (val > 0.0) t_low = t;
            else t_high = t;
        }
        t = (t_low + t_high) * 0.5;
    }

    auto t_opt = NormalizedF32Exclusive::create(static_cast<float>(t));
    if (t_opt) {
        chop_cubic_at2(src, *t_opt, dst);
        return true;
    }
    return false;
}

inline Point eval_quad_at(const Point src[3], NormalizedF32 t) noexcept {
    return Point::from_f32x2(QuadCoeff::from_points(src).eval(f32x2::splat(t.get())));
}

inline NormalizedF32 find_quad_max_curvature(const Point src[3]) noexcept {
    float ax = src[1].x - src[0].x;
    float ay = src[1].y - src[0].y;
    float bx = src[0].x - src[1].x - src[1].x + src[2].x;
    float by = src[0].y - src[1].y - src[1].y + src[2].y;

    float numer = -(ax * bx + ay * by);
    float denom = bx * bx + by * by;
    if (denom < 0.0f) {
        numer = -numer;
        denom = -denom;
    }

    if (numer <= 0.0f) {
        return NormalizedF32::ZERO;
    }

    if (numer >= denom) {
        return NormalizedF32::ONE;
    }

    return *NormalizedF32::create(numer / denom);
}

inline Point eval_quad_tangent_at(const Point src[3], NormalizedF32 tol) noexcept {
    if ((tol == NormalizedF32::ZERO && src[0] == src[1]) || (tol == NormalizedF32::ONE && src[1] == src[2])) {
        return src[2] - src[0];
    }

    f32x2 p0 = src[0].to_f32x2();
    f32x2 p1 = src[1].to_f32x2();
    f32x2 p2 = src[2].to_f32x2();

    f32x2 b = p1 - p0;
    f32x2 a = p2 - p1 - b;
    f32x2 t = a * f32x2::splat(tol.get()) + b;

    return Point::from_f32x2(t + t);
}

inline Point eval_cubic_pos_at(const Point src[4], NormalizedF32 t) noexcept {
    return Point::from_f32x2(CubicCoeff::from_points(src).eval(f32x2::splat(t.get())));
}

inline Point eval_cubic_derivative(const Point src[4], NormalizedF32 t) noexcept {
    f32x2 p0 = src[0].to_f32x2();
    f32x2 p1 = src[1].to_f32x2();
    f32x2 p2 = src[2].to_f32x2();
    f32x2 p3 = src[3].to_f32x2();

    QuadCoeff coeff{
        p3 + f32x2::splat(3.0f) * (p1 - p2) - p0,
        times_2(p2 - times_2(p1) + p0),
        p1 - p0
    };

    return Point::from_f32x2(coeff.eval(f32x2::splat(t.get())));
}

inline Point eval_cubic_tangent_at(const Point src[4], NormalizedF32 t) noexcept {
    if ((t.get() == 0.0f && src[0] == src[1]) || (t.get() == 1.0f && src[2] == src[3])) {
        Point tangent = (t.get() == 0.0f) ? (src[2] - src[0]) : (src[3] - src[1]);
        if (tangent.x == 0.0f && tangent.y == 0.0f) {
            tangent = src[3] - src[0];
        }
        return tangent;
    } else {
        return eval_cubic_derivative(src, t);
    }
}

inline void sort_array3(NormalizedF32 array[3]) noexcept {
    if (array[0] > array[1]) std::swap(array[0], array[1]);
    if (array[1] > array[2]) std::swap(array[1], array[2]);
    if (array[0] > array[1]) std::swap(array[0], array[1]);
}

inline size_t collapse_duplicates3(NormalizedF32 array[3]) noexcept {
    size_t len = 3;
    if (array[1] == array[2]) len = 2;
    if (array[0] == array[1]) len = 1;
    return len;
}

inline size_t solve_cubic_poly(const float coeff[4], NormalizedF32 t_values[3]) noexcept {
    if (scalar::is_nearly_zero(coeff[0])) {
        NormalizedF32Exclusive tmp_t[3];
        size_t count = find_unit_quad_roots(coeff[1], coeff[2], coeff[3], tmp_t);
        for (size_t i = 0; i < count; ++i) {
            t_values[i] = tmp_t[i].to_normalized();
        }
        return count;
    }

    float inva = scalar::invert(coeff[0]);
    float a = coeff[1] * inva;
    float b = coeff[2] * inva;
    float c = coeff[3] * inva;

    float q = (a * a - b * 3.0f) / 9.0f;
    float r = (2.0f * a * a * a - 9.0f * a * b + 27.0f * c) / 54.0f;

    float q3 = q * q * q;
    float r2_minus_q3 = r * r - q3;
    float adiv3 = a / 3.0f;

    if (r2_minus_q3 < 0.0f) {
        float theta = std::acos(scalar::bound(r / std::sqrt(q3), -1.0f, 1.0f));
        float neg2_root_q = -2.0f * std::sqrt(q);

        t_values[0] = NormalizedF32::create_clamped(neg2_root_q * std::cos(theta / 3.0f) - adiv3);
        t_values[1] = NormalizedF32::create_clamped(neg2_root_q * std::cos((theta + 2.0f * FLOAT_PI) / 3.0f) - adiv3);
        t_values[2] = NormalizedF32::create_clamped(neg2_root_q * std::cos((theta - 2.0f * FLOAT_PI) / 3.0f) - adiv3);

        sort_array3(t_values);
        return collapse_duplicates3(t_values);
    } else {
        float a_ = std::abs(r) + std::sqrt(r2_minus_q3);
        a_ = std::pow(a_, 1.0f / 3.0f);
        if (r > 0.0f) a_ = -a_;
        if (a_ != 0.0f) a_ += q / a_;

        t_values[0] = NormalizedF32::create_clamped(a_ - adiv3);
        return 1;
    }
}

inline size_t find_cubic_max_curvature(const Point src[4], NormalizedF32 t_values[3]) noexcept {
    float ax = src[1].x - src[0].x;
    float ay = src[1].y - src[0].y;
    float bx = src[2].x - 2.0f * src[1].x + src[0].x;
    float by = src[2].y - 2.0f * src[1].y + src[0].y;
    float cx = src[3].x + 3.0f * (src[1].x - src[2].x) - src[0].x;
    float cy = src[3].y + 3.0f * (src[1].y - src[2].y) - src[0].y;

    float coeff[4] = {
        cx * cx + cy * cy,
        3.0f * (bx * cx + by * cy),
        2.0f * (bx * bx + by * by) + cx * ax + cy * ay,
        ax * bx + ay * by
    };

    return solve_cubic_poly(coeff, t_values);
}

inline size_t find_cubic_inflections(const Point src[4], NormalizedF32Exclusive t_values[3]) noexcept {
    float ax = src[1].x - src[0].x;
    float ay = src[1].y - src[0].y;
    float bx = src[2].x - 2.0f * src[1].x + src[0].x;
    float by = src[2].y - 2.0f * src[1].y + src[0].y;
    float cx = src[3].x + 3.0f * (src[1].x - src[2].x) - src[0].x;
    float cy = src[3].y + 3.0f * (src[1].y - src[2].y) - src[0].y;

    return find_unit_quad_roots(
        bx * cy - by * cx,
        ax * cy - ay * cx,
        ax * by - ay * bx,
        t_values
    );
}

inline bool on_same_side(const Point src[4], size_t test_index, size_t line_index) noexcept {
    Point origin = src[line_index];
    Point line = src[line_index + 1] - origin;
    float crosses[2];
    for (size_t index = 0; index < 2; ++index) {
        Point test_line = src[test_index + index] - origin;
        crosses[index] = line.cross(test_line);
    }
    return crosses[0] * crosses[1] >= 0.0f;
}

inline float calc_cubic_precision(const Point src[4]) noexcept {
    return (src[1].distance_to_sqd(src[0]) +
            src[2].distance_to_sqd(src[1]) +
            src[3].distance_to_sqd(src[2])) * 1e-8f;
}

inline std::optional<NormalizedF32Exclusive> find_cubic_cusp(const Point src[4]) noexcept {
    if (src[0] == src[1] || src[2] == src[3]) {
        return std::nullopt;
    }

    if (on_same_side(src, 0, 2) || on_same_side(src, 2, 0)) {
        return std::nullopt;
    }

    NormalizedF32 t_values[3];
    size_t count = find_cubic_max_curvature(src, t_values);
    for (size_t i = 0; i < count; ++i) {
        float t = t_values[i].get();
        if (t <= 0.0f || t >= 1.0f) {
            continue;
        }

        Point d_pt = eval_cubic_derivative(src, t_values[i]);
        float d_pt_magnitude = d_pt.length_sqd();
        float precision = calc_cubic_precision(src);
        if (d_pt_magnitude < precision) {
            return NormalizedF32Exclusive::create_bounded(t);
        }
    }

    return std::nullopt;
}

inline float subdivide_weight_value(float w) noexcept {
    return std::sqrt(0.5f + w * 0.5f);
}

struct Conic {
    Point points[3];
    float weight{1.0f};

    constexpr Conic() noexcept = default;
    constexpr Conic(Point pt0, Point pt1, Point pt2, float w) noexcept
        : points{pt0, pt1, pt2}, weight(w) {}

    static constexpr Conic from_points(const Point pts[3], float w) noexcept {
        return Conic(pts[0], pts[1], pts[2], w);
    }

    static size_t build_unit_arc(
        Point u_start,
        Point u_stop,
        PathDirection dir,
        const Transform& user_transform,
        Conic dst[5]
    ) noexcept {
        float x = u_start.dot(u_stop);
        float y = u_start.cross(u_stop);
        float abs_y = std::abs(y);

        if (abs_y <= SCALAR_NEARLY_ZERO && x > 0.0f &&
            ((y >= 0.0f && dir == PathDirection::CW) || (y <= 0.0f && dir == PathDirection::CCW))) {
            return 0;
        }

        if (dir == PathDirection::CCW) {
            y = -y;
        }

        size_t quadrant = 0;
        if (y == 0.0f) {
            quadrant = 2;
        } else if (x == 0.0f) {
            quadrant = (y > 0.0f) ? 1 : 3;
        } else {
            if (y < 0.0f) quadrant += 2;
            if ((x < 0.0f) != (y < 0.0f)) quadrant += 1;
        }

        Point quadrant_points[8] = {
            Point::from_xy(1.0f, 0.0f),
            Point::from_xy(1.0f, 1.0f),
            Point::from_xy(0.0f, 1.0f),
            Point::from_xy(-1.0f, 1.0f),
            Point::from_xy(-1.0f, 0.0f),
            Point::from_xy(-1.0f, -1.0f),
            Point::from_xy(0.0f, -1.0f),
            Point::from_xy(1.0f, -1.0f),
        };

        constexpr float QUADRANT_WEIGHT = SCALAR_ROOT_2_OVER_2;

        size_t conic_count = quadrant;
        for (size_t i = 0; i < conic_count; ++i) {
            dst[i] = Conic::from_points(&quadrant_points[i * 2], QUADRANT_WEIGHT);
        }

        Point final_pt = Point::from_xy(x, y);
        Point last_q = quadrant_points[quadrant * 2];
        float dot = last_q.dot(final_pt);

        if (dot < 1.0f) {
            Point off_curve = Point::from_xy(last_q.x + x, last_q.y + y);
            float cos_theta_over_2 = std::sqrt(scalar::half(1.0f + dot));
            off_curve.set_length(scalar::invert(cos_theta_over_2));
            if (!last_q.almost_equal(off_curve)) {
                dst[conic_count] = Conic(last_q, off_curve, final_pt, cos_theta_over_2);
                conic_count++;
            }
        }

        Transform transform = Transform::from_sin_cos(u_start.y, u_start.x);
        if (dir == PathDirection::CCW) {
            transform = transform.pre_scale(1.0f, -1.0f);
        }
        transform = transform.post_concat(user_transform);

        for (size_t i = 0; i < conic_count; ++i) {
            transform.map_points(dst[i].points, 3);
        }

        return conic_count;
    }

    std::optional<uint8_t> compute_quad_pow2(float tolerance) const noexcept {
        if (tolerance < 0.0f || !std::isfinite(tolerance)) {
            return std::nullopt;
        }
        if (!points[0].is_finite() || !points[1].is_finite() || !points[2].is_finite()) {
            return std::nullopt;
        }

        constexpr size_t MAX_CONIC_TO_QUAD_POW2 = 4;
        float a = weight - 1.0f;
        float k = a / (4.0f * (2.0f + a));
        float x = k * (points[0].x - 2.0f * points[1].x + points[2].x);
        float y = k * (points[0].y - 2.0f * points[1].y + points[2].y);

        float error = std::sqrt(x * x + y * y);
        uint8_t pow2 = 0;
        for (size_t i = 0; i < MAX_CONIC_TO_QUAD_POW2; ++i) {
            if (error <= tolerance) break;
            error *= 0.25f;
            pow2++;
        }
        return std::max<uint8_t>(pow2, 1);
    }

    std::pair<Conic, Conic> chop() const noexcept {
        f32x2 scale = f32x2::splat(scalar::invert(1.0f + weight));
        float new_w = subdivide_weight_value(weight);

        f32x2 p0 = points[0].to_f32x2();
        f32x2 p1 = points[1].to_f32x2();
        f32x2 p2 = points[2].to_f32x2();
        f32x2 ww = f32x2::splat(weight);

        f32x2 wp1 = ww * p1;
        f32x2 m = (p0 + times_2(wp1) + p2) * scale * f32x2::splat(0.5f);
        Point m_pt = Point::from_f32x2(m);
        if (!m_pt.is_finite()) {
            double w_d = static_cast<double>(weight);
            double w_2 = w_d * 2.0;
            double scale_half = 1.0 / (1.0 + w_d) * 0.5;
            m_pt.x = static_cast<float>((static_cast<double>(points[0].x) +
                                         w_2 * static_cast<double>(points[1].x) +
                                         static_cast<double>(points[2].x)) * scale_half);
            m_pt.y = static_cast<float>((static_cast<double>(points[0].y) +
                                         w_2 * static_cast<double>(points[1].y) +
                                         static_cast<double>(points[2].y)) * scale_half);
        }

        return {
            Conic(points[0], Point::from_f32x2((p0 + wp1) * scale), m_pt, new_w),
            Conic(m_pt, Point::from_f32x2((wp1 + p2) * scale), points[2], new_w)
        };
    }

    static size_t subdivide(const Conic& src, Point* points, uint8_t level) noexcept {
        if (level == 0) {
            points[0] = src.points[1];
            points[1] = src.points[2];
            return 2;
        } else {
            auto [dst0, dst1] = src.chop();

            float start_y = src.points[0].y;
            float end_y = src.points[2].y;
            if (between(start_y, src.points[1].y, end_y)) {
                float mid_y = dst0.points[2].y;
                if (!between(start_y, mid_y, end_y)) {
                    float closer_y = (std::abs(mid_y - start_y) < std::abs(mid_y - end_y)) ? start_y : end_y;
                    dst0.points[2].y = closer_y;
                    dst1.points[0].y = closer_y;
                }

                if (!between(start_y, dst0.points[1].y, dst0.points[2].y)) {
                    dst0.points[1].y = start_y;
                }

                if (!between(dst1.points[0].y, dst1.points[1].y, end_y)) {
                    dst1.points[1].y = end_y;
                }
            }

            size_t count0 = subdivide(dst0, points, level - 1);
            size_t count1 = subdivide(dst1, points + count0, level - 1);
            return count0 + count1;
        }
    }

    uint8_t chop_into_quads_pow2(uint8_t pow2, Point points[64]) const noexcept {
        points[0] = points[0]; // will be assigned below
        points[0] = this->points[0];
        subdivide(*this, points + 1, pow2);

        size_t quad_count = 1u << pow2;
        size_t pt_count = 2 * quad_count + 1;
        bool has_non_finite = false;
        for (size_t i = 0; i < pt_count; ++i) {
            if (!points[i].is_finite()) {
                has_non_finite = true;
                break;
            }
        }
        if (has_non_finite) {
            for (size_t i = 1; i < pt_count - 1; ++i) {
                points[i] = this->points[1];
            }
        }

        return static_cast<uint8_t>(quad_count);
    }
};

struct AutoConicToQuads {
    Point points[64];
    uint8_t len{0};

    static std::optional<AutoConicToQuads> compute(Point pt0, Point pt1, Point pt2, float weight) noexcept {
        Conic conic(pt0, pt1, pt2, weight);
        auto pow2 = conic.compute_quad_pow2(0.25f);
        if (!pow2) return std::nullopt;

        AutoConicToQuads result;
        result.len = conic.chop_into_quads_pow2(*pow2, result.points);
        return result;
    }
};

} // namespace path_geometry
} // namespace nisaba

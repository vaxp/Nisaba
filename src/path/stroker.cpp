#include "nisaba/path/stroker.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/path_geometry.hpp"
#include "nisaba/path/path_ops.hpp"

namespace nisaba {

namespace {

struct SwappableBuilders {
    PathBuilder* inner;
    PathBuilder* outer;

    void swap() noexcept {
        std::swap(inner, outer);
    }
};

using CapProc = void (*)(Point pivot, Point normal, Point stop, const PathBuilder* other_path, PathBuilder& path);
using JoinProc = void (*)(
    Point before_unit_normal,
    Point pivot,
    Point after_unit_normal,
    float radius,
    float inv_miter_limit,
    bool prev_is_line,
    bool curr_is_line,
    SwappableBuilders builders
);

enum class ReductionType {
    Point,
    Line,
    Quad,
    Degenerate,
    Degenerate2,
    Degenerate3,
};

enum class StrokeType : int32_t {
    Outer = 1,
    Inner = -1,
};

enum class ResultType {
    Split,
    Degenerate,
    Quad,
};

enum class IntersectRayType {
    CtrlPt,
    ResultType,
};

constexpr size_t QUAD_RECURSIVE_LIMIT = 3;
constexpr int32_t RECURSIVE_LIMITS[4] = {5 * 3, 26 * 3, 11 * 3, 11 * 3};

void butt_capper(Point, Point, Point stop, const PathBuilder*, PathBuilder& path) {
    path.line_to(stop.x, stop.y);
}

void round_capper(Point pivot, Point normal, Point stop, const PathBuilder*, PathBuilder& path) {
    Point parallel = normal;
    parallel.rotate_cw();

    Point apex = pivot + parallel;
    constexpr float k = 0.5522847498f;

    Point p0 = pivot + normal;
    Point p1 = p0 + parallel.scaled(k);
    Point p2 = apex + normal.scaled(k);
    path.cubic_to(p1.x, p1.y, p2.x, p2.y, apex.x, apex.y);

    Point q1 = apex - normal.scaled(k);
    Point q2 = stop + parallel.scaled(k);
    path.cubic_to(q1.x, q1.y, q2.x, q2.y, stop.x, stop.y);
}

void square_capper(Point pivot, Point normal, Point stop, const PathBuilder* other_path, PathBuilder& path) {
    Point parallel = normal;
    parallel.rotate_cw();

    if (other_path) {
        path.set_last_point(Point::from_xy(
            pivot.x + normal.x + parallel.x,
            pivot.y + normal.y + parallel.y
        ));
        path.line_to(
            pivot.x - normal.x + parallel.x,
            pivot.y - normal.y + parallel.y
        );
    } else {
        path.line_to(
            pivot.x + normal.x + parallel.x,
            pivot.y + normal.y + parallel.y
        );
        path.line_to(
            pivot.x - normal.x + parallel.x,
            pivot.y - normal.y + parallel.y
        );
        path.line_to(stop.x, stop.y);
    }
}

inline CapProc cap_factory(LineCap cap) noexcept {
    switch (cap) {
        case LineCap::Butt: return butt_capper;
        case LineCap::Round: return round_capper;
        case LineCap::Square: return square_capper;
    }
    return butt_capper;
}

inline bool is_clockwise(Point before, Point after) noexcept {
    return before.x * after.y > before.y * after.x;
}

enum class AngleType {
    Nearly180,
    Sharp,
    Shallow,
    NearlyLine,
};

inline AngleType dot_to_angle_type(float dot) noexcept {
    if (dot >= 0.0f) {
        if (scalar::is_nearly_zero(1.0f - dot)) {
            return AngleType::NearlyLine;
        } else {
            return AngleType::Shallow;
        }
    } else {
        if (scalar::is_nearly_zero(1.0f + dot)) {
            return AngleType::Nearly180;
        } else {
            return AngleType::Sharp;
        }
    }
}

inline void handle_inner_join(Point pivot, Point after, PathBuilder& inner) {
    inner.line_to(pivot.x, pivot.y);
    inner.line_to(pivot.x - after.x, pivot.y - after.y);
}

void bevel_joiner(
    Point before_unit_normal,
    Point pivot,
    Point after_unit_normal,
    float radius,
    float,
    bool,
    bool,
    SwappableBuilders builders
) {
    Point after = after_unit_normal.scaled(radius);

    if (!is_clockwise(before_unit_normal, after_unit_normal)) {
        builders.swap();
        after = -after;
    }

    builders.outer->line_to(pivot.x + after.x, pivot.y + after.y);
    handle_inner_join(pivot, after, *builders.inner);
}

void round_joiner(
    Point before_unit_normal,
    Point pivot,
    Point after_unit_normal,
    float radius,
    float,
    bool,
    bool,
    SwappableBuilders builders
) {
    float dot_prod = before_unit_normal.dot(after_unit_normal);
    AngleType angle_type = dot_to_angle_type(dot_prod);

    if (angle_type == AngleType::NearlyLine) {
        return;
    }

    Point before = before_unit_normal;
    Point after = after_unit_normal;
    PathDirection dir = PathDirection::CW;

    if (!is_clockwise(before, after)) {
        builders.swap();
        before = -before;
        after = -after;
        dir = PathDirection::CCW;
    }

    Transform ts = Transform::from_row(radius, 0.0f, 0.0f, radius, pivot.x, pivot.y);
    path_geometry::Conic conics[5];
    size_t conics_count = path_geometry::Conic::build_unit_arc(before, after, dir, ts, conics);
    if (conics_count > 0) {
        for (size_t i = 0; i < conics_count; ++i) {
            builders.outer->conic_to(conics[i].points[1].x, conics[i].points[1].y, conics[i].points[2].x, conics[i].points[2].y, conics[i].weight);
        }
        after.scale(radius);
        handle_inner_join(pivot, after, *builders.inner);
    }
}

void miter_joiner_inner(
    Point before_unit_normal,
    Point pivot,
    Point after_unit_normal,
    float radius,
    float inv_miter_limit,
    bool miter_clip,
    bool prev_is_line,
    bool curr_is_line,
    SwappableBuilders builders
) {
    auto do_blunt_or_clipped = [&](
        SwappableBuilders b,
        Point piv,
        float rad,
        bool prev_line,
        bool curr_line,
        Point bef,
        Point m,
        Point aft
    ) {
        aft.scale(rad);
        if (miter_clip) {
            m.normalize();
            float cos_beta = bef.dot(m);
            float sin_beta = bef.cross(m);
            float x = (std::abs(sin_beta) <= SCALAR_NEARLY_ZERO) ? (1.0f / inv_miter_limit) : (((1.0f / inv_miter_limit) - cos_beta) / sin_beta);
            bef.scale(rad);
            Point before_tangent = bef;
            before_tangent.rotate_cw();
            Point after_tangent = aft;
            after_tangent.rotate_ccw();

            Point c1 = piv + bef + before_tangent.scaled(x);
            Point c2 = piv + aft + after_tangent.scaled(x);

            if (prev_line) {
                b.outer->set_last_point(c1);
            } else {
                b.outer->line_to(c1.x, c1.y);
            }
            b.outer->line_to(c2.x, c2.y);
        }

        if (!curr_line) {
            b.outer->line_to(piv.x + aft.x, piv.y + aft.y);
        }
        handle_inner_join(piv, aft, *b.inner);
    };

    auto do_miter = [&](
        SwappableBuilders b,
        Point piv,
        float rad,
        bool prev_line,
        bool curr_line,
        Point m,
        Point aft
    ) {
        aft.scale(rad);
        if (prev_line) {
            b.outer->set_last_point(Point::from_xy(piv.x + m.x, piv.y + m.y));
        } else {
            b.outer->line_to(piv.x + m.x, piv.y + m.y);
        }
        if (!curr_line) {
            b.outer->line_to(piv.x + aft.x, piv.y + aft.y);
        }
        handle_inner_join(piv, aft, *b.inner);
    };

    float dot_prod = before_unit_normal.dot(after_unit_normal);
    AngleType angle_type = dot_to_angle_type(dot_prod);
    Point before = before_unit_normal;
    Point after = after_unit_normal;
    Point mid;

    if (angle_type == AngleType::NearlyLine) return;

    if (angle_type == AngleType::Nearly180) {
        curr_is_line = false;
        mid = (after - before).scaled(radius * 0.5f);
        do_blunt_or_clipped(builders, pivot, radius, prev_is_line, curr_is_line, before, mid, after);
        return;
    }

    bool ccw = !is_clockwise(before, after);
    if (ccw) {
        builders.swap();
        before = -before;
        after = -after;
    }

    if (dot_prod == 0.0f && inv_miter_limit <= SCALAR_ROOT_2_OVER_2) {
        mid = (before + after).scaled(radius);
        do_miter(builders, pivot, radius, prev_is_line, curr_is_line, mid, after);
        return;
    }

    if (angle_type == AngleType::Sharp) {
        mid = Point::from_xy(after.y - before.y, before.x - after.x);
        if (ccw) mid = -mid;
    } else {
        mid = Point::from_xy(before.x + after.x, before.y + after.y);
    }

    float sin_half_angle = std::sqrt(scalar::half(1.0f + dot_prod));
    if (sin_half_angle < inv_miter_limit) {
        curr_is_line = false;
        do_blunt_or_clipped(builders, pivot, radius, prev_is_line, curr_is_line, before, mid, after);
        return;
    }

    mid.set_length(radius / sin_half_angle);
    do_miter(builders, pivot, radius, prev_is_line, curr_is_line, mid, after);
}

void miter_joiner(
    Point before_unit_normal, Point pivot, Point after_unit_normal,
    float radius, float inv_miter_limit, bool prev_is_line, bool curr_is_line,
    SwappableBuilders builders
) {
    miter_joiner_inner(before_unit_normal, pivot, after_unit_normal, radius, inv_miter_limit, false, prev_is_line, curr_is_line, builders);
}

void miter_clip_joiner(
    Point before_unit_normal, Point pivot, Point after_unit_normal,
    float radius, float inv_miter_limit, bool prev_is_line, bool curr_is_line,
    SwappableBuilders builders
) {
    miter_joiner_inner(before_unit_normal, pivot, after_unit_normal, radius, inv_miter_limit, true, prev_is_line, curr_is_line, builders);
}

inline JoinProc join_factory(LineJoin join) noexcept {
    switch (join) {
        case LineJoin::Miter: return miter_joiner;
        case LineJoin::MiterClip: return miter_clip_joiner;
        case LineJoin::Round: return round_joiner;
        case LineJoin::Bevel: return bevel_joiner;
    }
    return miter_joiner;
}

inline bool set_normal_unit_normal(
    Point before, Point after, float scale, float radius,
    Point& normal, Point& unit_normal
) noexcept {
    if (!unit_normal.set_normalize((after.x - before.x) * scale, (after.y - before.y) * scale)) {
        return false;
    }
    unit_normal.rotate_ccw();
    normal = unit_normal.scaled(radius);
    return true;
}

inline bool set_normal_unit_normal2(
    Point vec, float radius,
    Point& normal, Point& unit_normal
) noexcept {
    if (!unit_normal.set_normalize(vec.x, vec.y)) {
        return false;
    }
    unit_normal.rotate_ccw();
    normal = unit_normal.scaled(radius);
    return true;
}

inline bool degenerate_vector(Point v) noexcept {
    return !v.can_normalize();
}

inline float pt_to_line(Point pt, Point line_start, Point line_end) noexcept {
    Point dxy = line_end - line_start;
    Point ab0 = pt - line_start;
    float numer = dxy.dot(ab0);
    float denom = dxy.dot(dxy);
    float t = numer / denom;
    if (t >= 0.0f && t <= 1.0f) {
        Point hit = Point::from_xy(
            line_start.x * (1.0f - t) + line_end.x * t,
            line_start.y * (1.0f - t) + line_end.y * t
        );
        return hit.distance_to_sqd(pt);
    } else {
        return pt.distance_to_sqd(line_start);
    }
}

inline bool quad_in_line(const Point quad[3]) noexcept {
    float pt_max = -1.0f;
    size_t outer1 = 0;
    size_t outer2 = 0;
    for (size_t index = 0; index < 2; ++index) {
        for (size_t inner = index + 1; inner < 3; ++inner) {
            Point test_diff = quad[inner] - quad[index];
            float test_max = std::max(std::abs(test_diff.x), std::abs(test_diff.y));
            if (pt_max < test_max) {
                outer1 = index;
                outer2 = inner;
                pt_max = test_max;
            }
        }
    }

    size_t mid = outer1 ^ outer2 ^ 3;
    constexpr float CURVATURE_SLOP = 0.000005f;
    float line_slop = pt_max * pt_max * CURVATURE_SLOP;
    return pt_to_line(quad[mid], quad[outer1], quad[outer2]) <= line_slop;
}

inline std::pair<Point, ReductionType> check_quad_linear(const Point quad[3]) noexcept {
    bool degenerate_ab = degenerate_vector(quad[1] - quad[0]);
    bool degenerate_bc = degenerate_vector(quad[2] - quad[1]);
    if (degenerate_ab && degenerate_bc) {
        return {Point::zero(), ReductionType::Point};
    }
    if (degenerate_ab || degenerate_bc) {
        return {Point::zero(), ReductionType::Line};
    }
    if (!quad_in_line(quad)) {
        return {Point::zero(), ReductionType::Quad};
    }
    NormalizedF32 t = path_geometry::find_quad_max_curvature(quad);
    if (t == NormalizedF32::ZERO || t == NormalizedF32::ONE) {
        return {Point::zero(), ReductionType::Line};
    }
    return {path_geometry::eval_quad_at(quad, t), ReductionType::Degenerate};
}

inline size_t intersect_quad_ray(const Point line[2], const Point quad[3], NormalizedF32Exclusive roots[3]) noexcept {
    Point vec = line[1] - line[0];
    float r[3];
    for (size_t n = 0; n < 3; ++n) {
        r[n] = (quad[n].y - line[0].y) * vec.x - (quad[n].x - line[0].x) * vec.y;
    }
    float a = r[2] + r[0] - 2.0f * r[1];
    float b = -(r[1] - r[0]);
    float c = r[0];
    return path_geometry::find_unit_quad_roots(a, 2.0f * b, c, roots);
}

inline bool points_within_dist(Point near_pt, Point far_pt, float limit) noexcept {
    return near_pt.distance_to_sqd(far_pt) <= limit * limit;
}

inline bool sharp_angle(const Point quad[3]) noexcept {
    Point smaller = quad[1] - quad[0];
    Point larger = quad[1] - quad[2];
    float smaller_len = smaller.length_sqd();
    float larger_len = larger.length_sqd();
    if (smaller_len > larger_len) {
        std::swap(smaller, larger);
        larger_len = smaller_len;
    }
    if (!smaller.set_length(larger_len)) {
        return false;
    }
    return smaller.dot(larger) > 0.0f;
}

inline bool pt_in_quad_bounds(const Point quad[3], Point pt, float inv_res_scale) noexcept {
    float x_min = std::min({quad[0].x, quad[1].x, quad[2].x});
    if (pt.x + inv_res_scale < x_min) return false;
    float x_max = std::max({quad[0].x, quad[1].x, quad[2].x});
    if (pt.x - inv_res_scale > x_max) return false;
    float y_min = std::min({quad[0].y, quad[1].y, quad[2].y});
    if (pt.y + inv_res_scale < y_min) return false;
    float y_max = std::max({quad[0].y, quad[1].y, quad[2].y});
    if (pt.y - inv_res_scale > y_max) return false;
    return true;
}

inline bool cubic_in_line(const Point cubic[4]) noexcept {
    float pt_max = -1.0f;
    size_t outer1 = 0;
    size_t outer2 = 0;
    for (size_t index = 0; index < 3; ++index) {
        for (size_t inner = index + 1; inner < 4; ++inner) {
            Point test_diff = cubic[inner] - cubic[index];
            float test_max = std::max(std::abs(test_diff.x), std::abs(test_diff.y));
            if (pt_max < test_max) {
                outer1 = index;
                outer2 = inner;
                pt_max = test_max;
            }
        }
    }
    size_t mid1 = (1 + (2 >> outer2)) >> outer1;
    size_t mid2 = outer1 ^ outer2 ^ mid1;
    float line_slop = pt_max * pt_max * 0.00001f;
    return pt_to_line(cubic[mid1], cubic[outer1], cubic[outer2]) <= line_slop &&
           pt_to_line(cubic[mid2], cubic[outer1], cubic[outer2]) <= line_slop;
}

ReductionType check_cubic_linear(
    const Point cubic[4],
    Point reduction[3],
    Point* tangent_pt
) noexcept {
    bool degenerate_ab = degenerate_vector(cubic[1] - cubic[0]);
    bool degenerate_bc = degenerate_vector(cubic[2] - cubic[1]);
    bool degenerate_cd = degenerate_vector(cubic[3] - cubic[2]);
    if (degenerate_ab && degenerate_bc && degenerate_cd) {
        return ReductionType::Point;
    }
    if ((degenerate_ab ? 1 : 0) + (degenerate_bc ? 1 : 0) + (degenerate_cd ? 1 : 0) == 2) {
        return ReductionType::Line;
    }
    if (!cubic_in_line(cubic)) {
        if (tangent_pt) {
            *tangent_pt = degenerate_ab ? cubic[2] : cubic[1];
        }
        return ReductionType::Quad;
    }

    NormalizedF32 t_values[3];
    size_t count = path_geometry::find_cubic_max_curvature(cubic, t_values);
    size_t r_count = 0;
    for (size_t i = 0; i < count; ++i) {
        float t = t_values[i].get();
        if (t <= 0.0f || t >= 1.0f) continue;
        reduction[r_count] = path_geometry::eval_cubic_pos_at(cubic, t_values[i]);
        if (reduction[r_count] != cubic[0] && reduction[r_count] != cubic[3]) {
            r_count++;
        }
    }

    switch (r_count) {
        case 0: return ReductionType::Line;
        case 1: return ReductionType::Degenerate;
        case 2: return ReductionType::Degenerate2;
        case 3: return ReductionType::Degenerate3;
        default: return ReductionType::Line;
    }
}

struct QuadConstruct {
    Point quad[3];
    Point tangent_start;
    Point tangent_end;
    NormalizedF32 start_t{NormalizedF32::ZERO};
    NormalizedF32 mid_t{NormalizedF32::ZERO};
    NormalizedF32 end_t{NormalizedF32::ZERO};
    bool start_set{false};
    bool end_set{false};
    bool opposite_tangents{false};

    bool init(NormalizedF32 start, NormalizedF32 end) noexcept {
        start_t = start;
        mid_t = NormalizedF32::create_clamped(scalar::half(start.get() + end.get()));
        end_t = end;
        start_set = false;
        end_set = false;
        return start_t < mid_t && mid_t < end_t;
    }

    bool init_with_start(const QuadConstruct& parent) noexcept {
        if (!init(parent.start_t, parent.mid_t)) return false;
        quad[0] = parent.quad[0];
        tangent_start = parent.tangent_start;
        start_set = true;
        return true;
    }

    bool init_with_end(const QuadConstruct& parent) noexcept {
        if (!init(parent.mid_t, parent.end_t)) return false;
        quad[2] = parent.quad[2];
        tangent_end = parent.tangent_end;
        end_set = true;
        return true;
    }
};

} // anonymous namespace

struct PathStroker::Impl {
    float radius{0.0f};
    float inv_miter_limit{0.0f};
    float res_scale{1.0f};
    float inv_res_scale{1.0f};
    float inv_res_scale_squared{1.0f};

    Point first_normal;
    Point prev_normal;
    Point first_unit_normal;
    Point prev_unit_normal;

    Point first_pt;
    Point prev_pt;
    Point first_outer_pt;
    size_t first_outer_pt_index_in_contour{0};
    int32_t segment_count{-1};
    bool prev_is_line{false};

    CapProc capper{butt_capper};
    JoinProc joiner{miter_joiner};

    PathBuilder inner;
    PathBuilder outer;
    PathBuilder cusper;
    Path cached_path;

    StrokeType stroke_type{StrokeType::Outer};
    int32_t recursion_depth{0};
    bool found_tangents{false};
    bool join_completed{false};

    SwappableBuilders builders() noexcept {
        return SwappableBuilders{&inner, &outer};
    }

    void move_to(Point p) {
        if (segment_count > 0) {
            finish_contour(false, false);
        }
        segment_count = 0;
        first_pt = p;
        prev_pt = p;
        join_completed = false;
    }

    void line_to(Point p, const PathSegmentsIter* iter) {
        bool teeny_line = prev_pt.equals_within_tolerance(p, SCALAR_NEARLY_ZERO * inv_res_scale);
        if (capper == butt_capper && teeny_line) return;
        if (teeny_line && (join_completed || (iter && iter->next_verb().has_value()))) return;

        Point normal, unit_normal;
        if (!pre_join_to(p, true, normal, unit_normal)) return;

        outer.line_to(p.x + normal.x, p.y + normal.y);
        inner.line_to(p.x - normal.x, p.y - normal.y);
        post_join_to(p, normal, unit_normal);
    }

    void quad_to(Point p1, Point p2) {
        Point quad[3] = {prev_pt, p1, p2};
        auto [reduction, reduction_type] = check_quad_linear(quad);
        if (reduction_type == ReductionType::Point || reduction_type == ReductionType::Line) {
            line_to(p2, nullptr);
            return;
        }
        if (reduction_type == ReductionType::Degenerate) {
            line_to(reduction, nullptr);
            auto save_joiner = joiner;
            joiner = round_joiner;
            line_to(p2, nullptr);
            joiner = save_joiner;
            return;
        }

        Point normal_ab, unit_ab, normal_bc, unit_bc;
        if (!pre_join_to(p1, false, normal_ab, unit_ab)) {
            line_to(p2, nullptr);
            return;
        }

        QuadConstruct quad_points;
        init_quad(StrokeType::Outer, NormalizedF32::ZERO, NormalizedF32::ONE, quad_points);
        quad_stroke(quad, quad_points);
        init_quad(StrokeType::Inner, NormalizedF32::ZERO, NormalizedF32::ONE, quad_points);
        quad_stroke(quad, quad_points);

        bool ok = set_normal_unit_normal(quad[1], quad[2], res_scale, radius, normal_bc, unit_bc);
        if (!ok) {
            normal_bc = normal_ab;
            unit_bc = unit_ab;
        }
        post_join_to(p2, normal_bc, unit_bc);
    }

    void adaptive_cubic_to_quads(const Point c[4], int depth) {
        auto lerp = [](Point a, Point b, float t) noexcept {
            return Point::from_xy(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t);
        };

        Point c1 = lerp(c[0], c[1], 0.75f);
        Point c2 = lerp(c[3], c[2], 0.75f);
        Point pm = lerp(c1, c2, 0.5f);

        Point cubic_mid = Point::from_xy(
            (c[0].x + c[3].x) * 0.125f + (c[1].x + c[2].x) * 0.375f,
            (c[0].y + c[3].y) * 0.125f + (c[1].y + c[2].y) * 0.375f
        );
        float err_sq = pm.distance_to_sqd(cubic_mid);

        float tol_sq = std::max(0.75f, inv_res_scale_squared);
        if (err_sq <= tol_sq || depth >= 2) {
            quad_to(c1, pm);
            quad_to(c2, c[3]);
        } else {
            Point ab = lerp(c[0], c[1], 0.5f);
            Point bc = lerp(c[1], c[2], 0.5f);
            Point cd = lerp(c[2], c[3], 0.5f);
            Point abbc = lerp(ab, bc, 0.5f);
            Point bccd = lerp(bc, cd, 0.5f);
            Point mid = lerp(abbc, bccd, 0.5f);

            Point left[4]  = {c[0], ab, abbc, mid};
            Point right[4] = {mid, bccd, cd, c[3]};

            adaptive_cubic_to_quads(left, depth + 1);
            adaptive_cubic_to_quads(right, depth + 1);
        }
    }

    void cubic_to(Point pt1, Point pt2, Point pt3) {
        Point cubic[4] = {prev_pt, pt1, pt2, pt3};
        Point reduction[3];
        Point tangent_pt;
        ReductionType reduction_type = check_cubic_linear(cubic, reduction, &tangent_pt);
        if (reduction_type == ReductionType::Point || reduction_type == ReductionType::Line) {
            line_to(pt3, nullptr);
            return;
        }
        if (reduction_type >= ReductionType::Degenerate && reduction_type <= ReductionType::Degenerate3) {
            line_to(reduction[0], nullptr);
            auto save_joiner = joiner;
            joiner = round_joiner;
            if (reduction_type >= ReductionType::Degenerate2) {
                line_to(reduction[1], nullptr);
            }
            if (reduction_type == ReductionType::Degenerate3) {
                line_to(reduction[2], nullptr);
            }
            line_to(pt3, nullptr);
            joiner = save_joiner;
            return;
        }

        auto cusp = path_geometry::find_cubic_cusp(cubic);
        if (cusp) {
            Point cusp_loc = path_geometry::eval_cubic_pos_at(cubic, cusp->to_normalized());
            cusper.push_circle(cusp_loc.x, cusp_loc.y, radius);
        }

        adaptive_cubic_to_quads(cubic, 0);
    }

    void finish_contour(bool close, bool curr_is_line) {
        if (segment_count > 0) {
            if (close) {
                joiner(prev_unit_normal, prev_pt, first_unit_normal, radius, inv_miter_limit, prev_is_line, curr_is_line, builders());
                outer.close();
                auto pt = inner.last_point().value_or(Point::zero());
                outer.move_to(pt.x, pt.y);
                outer.reverse_path_to(&inner);
                outer.close();
            } else {
                auto pt = inner.last_point().value_or(Point::zero());
                const PathBuilder* other_path = curr_is_line ? &inner : nullptr;
                capper(prev_pt, prev_normal, pt, other_path, outer);
                outer.reverse_path_to(&inner);

                const PathBuilder* other_path_start = prev_is_line ? &inner : nullptr;
                capper(first_pt, -first_normal, first_outer_pt, other_path_start, outer);
                outer.close();
            }

            if (!cusper.is_empty()) {
                outer.push_path_builder(&cusper);
                cusper.clear();
            }
        }
        inner.clear();
        segment_count = -1;
        first_outer_pt_index_in_contour = outer.points().size();
    }

    bool pre_join_to(Point p, bool curr_is_line, Point& normal, Point& unit_normal) {
        float prev_x = prev_pt.x;
        float prev_y = prev_pt.y;

        if (!set_normal_unit_normal(prev_pt, p, res_scale, radius, normal, unit_normal)) {
            if (capper == butt_capper) return false;
            normal = Point::from_xy(radius, 0.0f);
            unit_normal = Point::from_xy(1.0f, 0.0f);
        }

        if (segment_count == 0) {
            first_normal = normal;
            first_unit_normal = unit_normal;
            first_outer_pt = Point::from_xy(prev_x + normal.x, prev_y + normal.y);
            outer.move_to(first_outer_pt.x, first_outer_pt.y);
            inner.move_to(prev_x - normal.x, prev_y - normal.y);
        } else {
            joiner(prev_unit_normal, prev_pt, unit_normal, radius, inv_miter_limit, prev_is_line, curr_is_line, builders());
        }
        prev_is_line = curr_is_line;
        return true;
    }

    void post_join_to(Point p, Point normal, Point unit_normal) noexcept {
        join_completed = true;
        prev_pt = p;
        prev_unit_normal = unit_normal;
        prev_normal = normal;
        segment_count++;
    }

    void init_quad(StrokeType st, NormalizedF32 start, NormalizedF32 end, QuadConstruct& qp) noexcept {
        stroke_type = st;
        found_tangents = false;
        qp.init(start, end);
    }

    bool quad_stroke(const Point quad[3], QuadConstruct& qp) {
        ResultType rt = compare_quad_quad(quad, qp);
        if (rt == ResultType::Quad) {
            PathBuilder& path = (stroke_type == StrokeType::Outer) ? outer : inner;
            path.quad_to(qp.quad[1].x, qp.quad[1].y, qp.quad[2].x, qp.quad[2].y);
            return true;
        }
        if (rt == ResultType::Degenerate) {
            add_degenerate_line(qp);
            return true;
        }

        recursion_depth++;
        if (recursion_depth > RECURSIVE_LIMITS[QUAD_RECURSIVE_LIMIT]) return false;

        QuadConstruct half;
        half.init_with_start(qp);
        if (!quad_stroke(quad, half)) return false;
        half.init_with_end(qp);
        if (!quad_stroke(quad, half)) return false;

        recursion_depth--;
        return true;
    }

    ResultType compare_quad_quad(const Point quad[3], QuadConstruct& qp) {
        if (!qp.start_set) {
            Point quad_start_pt;
            quad_perp_ray(quad, qp.start_t, quad_start_pt, qp.quad[0], &qp.tangent_start);
            qp.start_set = true;
        }
        if (!qp.end_set) {
            Point quad_end_pt;
            quad_perp_ray(quad, qp.end_t, quad_end_pt, qp.quad[2], &qp.tangent_end);
            qp.end_set = true;
        }

        ResultType rt = intersect_ray(IntersectRayType::CtrlPt, qp);
        if (rt != ResultType::Quad) return rt;

        Point ray0, ray1;
        quad_perp_ray(quad, qp.mid_t, ray1, ray0, nullptr);
        Point ray[2] = {ray0, ray1};
        return stroke_close_enough(qp.quad, ray, qp);
    }

    void quad_perp_ray(const Point quad[3], NormalizedF32 t, Point& tp, Point& on_p, Point* tangent) const noexcept {
        tp = path_geometry::eval_quad_at(quad, t);
        Point dxy = path_geometry::eval_quad_tangent_at(quad, t);
        if (dxy.is_zero()) {
            dxy = quad[2] - quad[0];
        }
        set_ray_points(tp, dxy, on_p, tangent);
    }

    void set_ray_points(Point tp, Point dxy, Point& on_p, Point* tangent) const noexcept {
        if (!dxy.set_length(radius)) {
            dxy = Point::from_xy(radius, 0.0f);
        }
        float axis_flip = static_cast<float>(static_cast<int32_t>(stroke_type));
        on_p.x = tp.x + axis_flip * dxy.y;
        on_p.y = tp.y - axis_flip * dxy.x;
        if (tangent) {
            tangent->x = on_p.x + dxy.x;
            tangent->y = on_p.y + dxy.y;
        }
    }

    void add_degenerate_line(const QuadConstruct& qp) {
        PathBuilder& path = (stroke_type == StrokeType::Outer) ? outer : inner;
        path.line_to(qp.quad[2].x, qp.quad[2].y);
    }

    ResultType stroke_close_enough(const Point stroke[3], const Point ray[2], QuadConstruct& qp) const noexcept {
        auto half = NormalizedF32::create_clamped(0.5f);
        Point stroke_mid = path_geometry::eval_quad_at(stroke, half);
        if (points_within_dist(ray[0], stroke_mid, inv_res_scale)) {
            if (sharp_angle(qp.quad)) return ResultType::Split;
            return ResultType::Quad;
        }
        if (!pt_in_quad_bounds(stroke, ray[0], inv_res_scale)) {
            return ResultType::Split;
        }

        NormalizedF32Exclusive roots[3];
        size_t r_count = intersect_quad_ray(ray, stroke, roots);
        if (r_count != 1) return ResultType::Split;

        Point quad_pt = path_geometry::eval_quad_at(stroke, roots[0].to_normalized());
        float error = inv_res_scale * (1.0f - std::abs(roots[0].get() - 0.5f) * 2.0f);
        if (points_within_dist(ray[0], quad_pt, error)) {
            if (sharp_angle(qp.quad)) return ResultType::Split;
            return ResultType::Quad;
        }
        return ResultType::Split;
    }

    ResultType intersect_ray(IntersectRayType irt, QuadConstruct& qp) const noexcept {
        Point start = qp.quad[0];
        Point end = qp.quad[2];
        Point a_len = qp.tangent_start - start;
        Point b_len = qp.tangent_end - end;

        float denom = a_len.cross(b_len);
        if (denom == 0.0f || !std::isfinite(denom)) {
            qp.opposite_tangents = a_len.dot(b_len) < 0.0f;
            return ResultType::Degenerate;
        }

        qp.opposite_tangents = false;
        Point ab0 = start - end;
        float numer_a = b_len.cross(ab0);
        float numer_b = a_len.cross(ab0);

        if ((numer_a >= 0.0f) == (numer_b >= 0.0f)) {
            float dist1 = pt_to_line(start, end, qp.tangent_end);
            float dist2 = pt_to_line(end, start, qp.tangent_start);
            if (std::max(dist1, dist2) <= inv_res_scale_squared) {
                return ResultType::Degenerate;
            }
            return ResultType::Split;
        }

        numer_a /= denom;
        if (numer_a > numer_a - 1.0f) {
            if (irt == IntersectRayType::CtrlPt) {
                qp.quad[1].x = start.x * (1.0f - numer_a) + qp.tangent_start.x * numer_a;
                qp.quad[1].y = start.y * (1.0f - numer_a) + qp.tangent_start.y * numer_a;
            }
            return ResultType::Quad;
        }

        qp.opposite_tangents = a_len.dot(b_len) < 0.0f;
        return ResultType::Degenerate;
    }

    bool cubic_stroke(const Point cubic[4], QuadConstruct& qp) {
        if (!found_tangents) {
            ResultType rt = tangents_meet(cubic, qp);
            if (rt != ResultType::Quad) {
                bool ok = points_within_dist(qp.quad[0], qp.quad[2], inv_res_scale);
                if ((rt == ResultType::Degenerate || ok) && cubic_mid_on_line(cubic, qp)) {
                    add_degenerate_line(qp);
                    return true;
                }
            } else {
                found_tangents = true;
            }
        }

        if (found_tangents) {
            ResultType rt = compare_quad_cubic(cubic, qp);
            if (rt == ResultType::Quad) {
                PathBuilder& path = (stroke_type == StrokeType::Outer) ? outer : inner;
                path.quad_to(qp.quad[1].x, qp.quad[1].y, qp.quad[2].x, qp.quad[2].y);
                return true;
            }
            if (rt == ResultType::Degenerate) {
                if (!qp.opposite_tangents) {
                    add_degenerate_line(qp);
                    return true;
                }
            }
        }

        if (!std::isfinite(qp.quad[2].x) || !std::isfinite(qp.quad[2].y)) {
            return false;
        }

        recursion_depth++;
        if (recursion_depth > RECURSIVE_LIMITS[found_tangents ? 1 : 0]) {
            return false;
        }

        QuadConstruct half;
        if (!half.init_with_start(qp)) {
            add_degenerate_line(qp);
            recursion_depth--;
            return true;
        }
        if (!cubic_stroke(cubic, half)) return false;

        if (!half.init_with_end(qp)) {
            add_degenerate_line(qp);
            recursion_depth--;
            return true;
        }
        if (!cubic_stroke(cubic, half)) return false;

        recursion_depth--;
        return true;
    }

    ResultType tangents_meet(const Point cubic[4], QuadConstruct& qp) {
        cubic_quad_ends(cubic, qp);
        return intersect_ray(IntersectRayType::ResultType, qp);
    }

    bool cubic_mid_on_line(const Point cubic[4], QuadConstruct& qp) const noexcept {
        Point stroke_mid;
        Point cubic_mid_pt;
        cubic_perp_ray(cubic, qp.mid_t, cubic_mid_pt, stroke_mid, nullptr);
        float dist = pt_to_line(stroke_mid, qp.quad[0], qp.quad[2]);
        return dist < inv_res_scale_squared;
    }

    ResultType compare_quad_cubic(const Point cubic[4], QuadConstruct& qp) {
        cubic_quad_ends(cubic, qp);
        ResultType rt = intersect_ray(IntersectRayType::CtrlPt, qp);
        if (rt != ResultType::Quad) return rt;

        Point ray0, ray1;
        cubic_perp_ray(cubic, qp.mid_t, ray1, ray0, nullptr);
        Point ray[2] = {ray0, ray1};
        return stroke_close_enough(qp.quad, ray, qp);
    }

    void cubic_quad_ends(const Point cubic[4], QuadConstruct& qp) {
        if (!qp.start_set) {
            Point cubic_start_pt;
            cubic_perp_ray(cubic, qp.start_t, cubic_start_pt, qp.quad[0], &qp.tangent_start);
            qp.start_set = true;
        }
        if (!qp.end_set) {
            Point cubic_end_pt;
            cubic_perp_ray(cubic, qp.end_t, cubic_end_pt, qp.quad[2], &qp.tangent_end);
            qp.end_set = true;
        }
    }

    void cubic_perp_ray(const Point cubic[4], NormalizedF32 t, Point& t_pt, Point& on_pt, Point* tangent) const noexcept {
        t_pt = path_geometry::eval_cubic_pos_at(cubic, t);
        Point dxy = path_geometry::eval_cubic_tangent_at(cubic, t);

        Point chopped[7];
        if (dxy.x == 0.0f && dxy.y == 0.0f) {
            const Point* c_points = cubic;
            if (scalar::is_nearly_zero(t.get())) {
                dxy = cubic[2] - cubic[0];
            } else if (scalar::is_nearly_zero(1.0f - t.get())) {
                dxy = cubic[3] - cubic[1];
            } else {
                auto t_ex = NormalizedF32Exclusive::create(t.get());
                if (t_ex) {
                    path_geometry::chop_cubic_at2(cubic, *t_ex, chopped);
                    dxy = chopped[3] - chopped[2];
                    if (dxy.x == 0.0f && dxy.y == 0.0f) {
                        dxy = chopped[3] - chopped[1];
                        c_points = chopped;
                    }
                }
            }
            if (dxy.x == 0.0f && dxy.y == 0.0f) {
                dxy = c_points[3] - c_points[0];
            }
        }

        set_ray_points(t_pt, dxy, on_pt, tangent);
    }

    void set_cubic_end_normal(
        const Point cubic[4], Point normal_ab, Point unit_normal_ab,
        Point& normal_cd, Point& unit_normal_cd
    ) {
        Point ab = cubic[1] - cubic[0];
        Point cd = cubic[3] - cubic[2];
        bool degenerate_ab = degenerate_vector(ab);
        bool degenerate_cb = degenerate_vector(cd);

        if (degenerate_ab && degenerate_cb) {
            normal_cd = normal_ab;
            unit_normal_cd = unit_normal_ab;
            return;
        }
        if (degenerate_ab) {
            ab = cubic[2] - cubic[0];
            degenerate_ab = degenerate_vector(ab);
        }
        if (degenerate_cb) {
            cd = cubic[3] - cubic[1];
            degenerate_cb = degenerate_vector(cd);
        }
        if (degenerate_ab || degenerate_cb) {
            normal_cd = normal_ab;
            unit_normal_cd = unit_normal_ab;
            return;
        }
        set_normal_unit_normal2(cd, radius, normal_cd, unit_normal_cd);
    }
};

PathStroker::PathStroker() : impl_(std::make_unique<Impl>()) {}
PathStroker::~PathStroker() = default;
PathStroker::PathStroker(PathStroker&&) noexcept = default;
PathStroker& PathStroker::operator=(PathStroker&&) noexcept = default;

float PathStroker::compute_resolution_scale(const Transform& ts) noexcept {
    float sx = Point::from_xy(ts.sx, ts.kx).length();
    float sy = Point::from_xy(ts.ky, ts.sy).length();
    if (std::isfinite(sx) && std::isfinite(sy)) {
        float scale = std::max(sx, sy);
        if (scale > 0.0f) return scale;
    }
    return 1.0f;
}

const Path* PathStroker::stroke_fast(const Path& path, const Stroke& stroke, float resolution_scale) {
    if (stroke.dash) {
        auto dashed = dash_path_fast(path, *stroke.dash, resolution_scale);
        if (!dashed) return nullptr;
        Stroke no_dash_stroke = stroke;
        no_dash_stroke.dash = std::nullopt;
        return this->stroke_fast(*dashed, no_dash_stroke, resolution_scale);
    }

    auto width_opt = NonZeroPositiveF32::create(stroke.width);
    if (!width_opt) return nullptr;

    float inv_miter_limit = 0.0f;
    LineJoin line_join = stroke.line_join;
    if (line_join == LineJoin::Miter) {
        if (stroke.miter_limit <= 1.0f) {
            line_join = LineJoin::Bevel;
        } else {
            inv_miter_limit = scalar::invert(stroke.miter_limit);
        }
    }
    if (line_join == LineJoin::MiterClip) {
        inv_miter_limit = scalar::invert(stroke.miter_limit);
    }

    impl_->res_scale = resolution_scale;
    impl_->inv_res_scale = scalar::invert(resolution_scale * 1.5f);
    impl_->inv_res_scale_squared = scalar::sqr(impl_->inv_res_scale);
    impl_->radius = scalar::half(width_opt->get());
    impl_->inv_miter_limit = inv_miter_limit;

    impl_->first_normal = Point::zero();
    impl_->prev_normal = Point::zero();
    impl_->first_unit_normal = Point::zero();
    impl_->prev_unit_normal = Point::zero();
    impl_->first_pt = Point::zero();
    impl_->prev_pt = Point::zero();
    impl_->first_outer_pt = Point::zero();
    impl_->first_outer_pt_index_in_contour = 0;
    impl_->segment_count = -1;
    impl_->prev_is_line = false;

    impl_->capper = cap_factory(stroke.line_cap);
    impl_->joiner = join_factory(line_join);

    impl_->inner.clear();
    impl_->outer.clear();
    impl_->cusper.clear();
    impl_->inner.reserve(64, 64);
    impl_->outer.reserve(64, 64);
    impl_->stroke_type = StrokeType::Outer;
    impl_->recursion_depth = 0;
    impl_->found_tangents = false;
    impl_->join_completed = false;

    bool last_segment_is_line = false;
    auto iter = path.segments();
    iter.set_auto_close(true);

    while (auto seg = iter.next()) {
        switch (seg->type) {
            case PathSegment::Type::MoveTo:
                impl_->move_to(seg->p0);
                break;
            case PathSegment::Type::LineTo:
                impl_->line_to(seg->p0, &iter);
                last_segment_is_line = true;
                break;
            case PathSegment::Type::QuadTo:
                impl_->quad_to(seg->p0, seg->p1);
                last_segment_is_line = false;
                break;
            case PathSegment::Type::CubicTo:
                impl_->cubic_to(seg->p0, seg->p1, seg->p2);
                last_segment_is_line = false;
                break;
            case PathSegment::Type::Close:
                if (stroke.line_cap != LineCap::Butt) {
                    if (impl_->segment_count == 0) {
                        impl_->line_to(impl_->first_pt, nullptr);
                        last_segment_is_line = true;
                        continue;
                    }
                }
                impl_->finish_contour(true, last_segment_is_line);
                break;
        }
    }

    impl_->finish_contour(false, last_segment_is_line);
    impl_->outer.finish_into(impl_->cached_path);
    if (impl_->cached_path.is_empty()) return nullptr;
    return &impl_->cached_path;
}

std::optional<Path> PathStroker::stroke(const Path& path, const Stroke& stroke, float resolution_scale) {
    if (const Path* fast_path = stroke_fast(path, stroke, resolution_scale)) {
        return *fast_path;
    }
    return std::nullopt;
}

std::optional<Path> Path::stroke_to_fill(const Stroke& stroke, float resolution_scale) const {
    return stroke_path(*this, stroke, resolution_scale);
}

std::optional<Path> Path::stroke(const Stroke& stroke, float resolution_scale) const {
    return stroke_to_fill(stroke, resolution_scale);
}

std::optional<Path> Path::stroke_to_fill_clean(const Stroke& stroke, float resolution_scale) const {
    auto outlined = stroke_to_fill(stroke, resolution_scale);
    if (!outlined) return std::nullopt;
    Path empty;
    auto unified = outlined->unite(empty);
    if (unified && !unified->is_empty()) {
        return unified;
    }
    return outlined;
}

} // namespace nisaba

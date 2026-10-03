#include "nisaba/path/path_ops.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/path_geometry.hpp"
#include <vector>
#include <cmath>
#include <algorithm>
#include <array>
#include <limits>

namespace nisaba {

namespace {

constexpr double EPSILON = 1e-5;
constexpr float POINT_TOLERANCE = 1e-3f;

inline Point scale_point(Point p, float s) noexcept {
    return Point::from_xy(p.x * s, p.y * s);
}

inline bool rects_intersect(const Rect& a, const Rect& b) noexcept {
    auto is = a.intersect(b);
    return is.has_value() && !is->is_empty();
}

enum class SegmentKind : uint8_t {
    Line,
    Quad,
    Cubic,
};

struct OpSegment {
    SegmentKind kind{SegmentKind::Line};
    Point pts[4];
    Rect bounds{};

    Point start_point() const noexcept { return pts[0]; }
    Point end_point() const noexcept {
        switch (kind) {
            case SegmentKind::Line: return pts[1];
            case SegmentKind::Quad: return pts[2];
            case SegmentKind::Cubic: return pts[3];
        }
        return pts[0];
    }

    void update_bounds() noexcept {
        Point min_p = pts[0];
        Point max_p = pts[0];
        int count = (kind == SegmentKind::Line) ? 2 : (kind == SegmentKind::Quad ? 3 : 4);
        for (int i = 1; i < count; ++i) {
            min_p.x = std::min(min_p.x, pts[i].x);
            min_p.y = std::min(min_p.y, pts[i].y);
            max_p.x = std::max(max_p.x, pts[i].x);
            max_p.y = std::max(max_p.y, pts[i].y);
        }
        auto r = Rect::from_ltrb(min_p.x - 0.01f, min_p.y - 0.01f, max_p.x + 0.01f, max_p.y + 0.01f);
        if (r) bounds = *r;
    }

    Point eval(double t) const noexcept {
        float tf = static_cast<float>(t);
        float one_minus_t = 1.0f - tf;
        switch (kind) {
            case SegmentKind::Line:
                return Point::from_xy(
                    pts[0].x * one_minus_t + pts[1].x * tf,
                    pts[0].y * one_minus_t + pts[1].y * tf
                );
            case SegmentKind::Quad: {
                float a = one_minus_t * one_minus_t;
                float b = 2.0f * one_minus_t * tf;
                float c = tf * tf;
                return Point::from_xy(
                    pts[0].x * a + pts[1].x * b + pts[2].x * c,
                    pts[0].y * a + pts[1].y * b + pts[2].y * c
                );
            }
            case SegmentKind::Cubic: {
                float a = one_minus_t * one_minus_t * one_minus_t;
                float b = 3.0f * one_minus_t * one_minus_t * tf;
                float c = 3.0f * one_minus_t * tf * tf;
                float d = tf * tf * tf;
                return Point::from_xy(
                    pts[0].x * a + pts[1].x * b + pts[2].x * c + pts[3].x * d,
                    pts[0].y * a + pts[1].y * b + pts[2].y * c + pts[3].y * d
                );
            }
        }
        return pts[0];
    }

    Point derivative(double t) const noexcept {
        float tf = static_cast<float>(t);
        float one_minus_t = 1.0f - tf;
        switch (kind) {
            case SegmentKind::Line:
                return pts[1] - pts[0];
            case SegmentKind::Quad:
                return scale_point(pts[1] - pts[0], 2.0f * one_minus_t) + scale_point(pts[2] - pts[1], 2.0f * tf);
            case SegmentKind::Cubic: {
                float a = 3.0f * one_minus_t * one_minus_t;
                float b = 6.0f * one_minus_t * tf;
                float c = 3.0f * tf * tf;
                return scale_point(pts[1] - pts[0], a) + scale_point(pts[2] - pts[1], b) + scale_point(pts[3] - pts[2], c);
            }
        }
        return Point();
    }

    Point normal(double t) const noexcept {
        Point d = derivative(t);
        float len = std::hypot(d.x, d.y);
        if (len < 1e-6f) return Point::from_xy(0.0f, 1.0f);
        // For CW contour in screen coordinates (+Y down): interior normal is (-dy, dx)
        return Point::from_xy(-d.y / len, d.x / len);
    }

    OpSegment reversed() const noexcept {
        OpSegment res;
        res.kind = kind;
        switch (kind) {
            case SegmentKind::Line:
                res.pts[0] = pts[1];
                res.pts[1] = pts[0];
                break;
            case SegmentKind::Quad:
                res.pts[0] = pts[2];
                res.pts[1] = pts[1];
                res.pts[2] = pts[0];
                break;
            case SegmentKind::Cubic:
                res.pts[0] = pts[3];
                res.pts[1] = pts[2];
                res.pts[2] = pts[1];
                res.pts[3] = pts[0];
                break;
        }
        res.bounds = bounds;
        return res;
    }

    void chop_at(double t, OpSegment& left, OpSegment& right) const noexcept {
        auto t_norm = NormalizedF32Exclusive::create(static_cast<float>(t));
        if (!t_norm) {
            left = *this;
            right = *this;
            return;
        }
        left.kind = kind;
        right.kind = kind;

        switch (kind) {
            case SegmentKind::Line: {
                Point mid = eval(t);
                left.pts[0] = pts[0];
                left.pts[1] = mid;
                right.pts[0] = mid;
                right.pts[1] = pts[1];
                break;
            }
            case SegmentKind::Quad: {
                Point dst[5];
                path_geometry::chop_quad_at(pts, *t_norm, dst);
                left.pts[0] = dst[0];
                left.pts[1] = dst[1];
                left.pts[2] = dst[2];
                right.pts[0] = dst[2];
                right.pts[1] = dst[3];
                right.pts[2] = dst[4];
                break;
            }
            case SegmentKind::Cubic: {
                Point dst[7];
                path_geometry::chop_cubic_at2(pts, *t_norm, dst);
                left.pts[0] = dst[0];
                left.pts[1] = dst[1];
                left.pts[2] = dst[2];
                left.pts[3] = dst[3];
                right.pts[0] = dst[3];
                right.pts[1] = dst[4];
                right.pts[2] = dst[5];
                right.pts[3] = dst[6];
                break;
            }
        }
        left.update_bounds();
        right.update_bounds();
    }
};

std::vector<OpSegment> decompose_path(const Path& path) {
    std::vector<OpSegment> segments;
    PathSegmentsIter iter(path);
    Point last_move{0.0f, 0.0f};
    Point last_point{0.0f, 0.0f};
    bool in_contour = false;

    while (auto seg = iter.next()) {
        switch (seg->type) {
            case PathSegment::Type::MoveTo:
                if (in_contour && (last_point.x != last_move.x || last_point.y != last_move.y)) {
                    OpSegment s;
                    s.kind = SegmentKind::Line;
                    s.pts[0] = last_point;
                    s.pts[1] = last_move;
                    s.update_bounds();
                    segments.push_back(s);
                }
                last_move = seg->p0;
                last_point = seg->p0;
                in_contour = true;
                break;
            case PathSegment::Type::LineTo: {
                OpSegment s;
                s.kind = SegmentKind::Line;
                s.pts[0] = last_point;
                s.pts[1] = seg->p0;
                s.update_bounds();
                segments.push_back(s);
                last_point = seg->p0;
                break;
            }
            case PathSegment::Type::QuadTo: {
                OpSegment s;
                s.kind = SegmentKind::Quad;
                s.pts[0] = last_point;
                s.pts[1] = seg->p0;
                s.pts[2] = seg->p1;
                s.update_bounds();
                segments.push_back(s);
                last_point = seg->p1;
                break;
            }
            case PathSegment::Type::CubicTo: {
                OpSegment s;
                s.kind = SegmentKind::Cubic;
                s.pts[0] = last_point;
                s.pts[1] = seg->p0;
                s.pts[2] = seg->p1;
                s.pts[3] = seg->p2;
                s.update_bounds();
                segments.push_back(s);
                last_point = seg->p2;
                break;
            }
            case PathSegment::Type::Close:
                if (in_contour && (last_point.x != last_move.x || last_point.y != last_move.y)) {
                    OpSegment s;
                    s.kind = SegmentKind::Line;
                    s.pts[0] = last_point;
                    s.pts[1] = last_move;
                    s.update_bounds();
                    segments.push_back(s);
                }
                last_point = last_move;
                in_contour = false;
                break;
        }
    }
    if (in_contour && (last_point.x != last_move.x || last_point.y != last_move.y)) {
        OpSegment s;
        s.kind = SegmentKind::Line;
        s.pts[0] = last_point;
        s.pts[1] = last_move;
        s.update_bounds();
        segments.push_back(s);
    }
    return segments;
}

void solve_quadratic_unit(double a, double b, double c, std::vector<double>& roots) {
    if (std::abs(a) < 1e-9) {
        if (std::abs(b) > 1e-9) {
            double r = -c / b;
            if (r >= -1e-4 && r <= 1.0 + 1e-4) roots.push_back(std::clamp(r, 0.0, 1.0));
        }
        return;
    }
    double disc = b * b - 4.0 * a * c;
    if (disc < 0.0) return;
    double sqrt_disc = std::sqrt(disc);
    double r1 = (-b - sqrt_disc) / (2.0 * a);
    double r2 = (-b + sqrt_disc) / (2.0 * a);
    if (r1 >= -1e-4 && r1 <= 1.0 + 1e-4) roots.push_back(std::clamp(r1, 0.0, 1.0));
    if (r2 >= -1e-4 && r2 <= 1.0 + 1e-4 && std::abs(r2 - r1) > EPSILON) roots.push_back(std::clamp(r2, 0.0, 1.0));
}

void solve_cubic_unit(double a, double b, double c, double d, std::vector<double>& roots) {
    if (std::abs(a) < 1e-9) {
        solve_quadratic_unit(b, c, d, roots);
        return;
    }
    double p = (3.0 * a * c - b * b) / (3.0 * a * a);
    double q = (2.0 * b * b * b - 9.0 * a * b * c + 27.0 * a * a * d) / (27.0 * a * a * a);
    double offset = b / (3.0 * a);

    double delta = (q * q / 4.0) + (p * p * p / 27.0);

    if (delta > 1e-11) {
        double sqrt_d = std::sqrt(delta);
        double u = -q / 2.0 + sqrt_d;
        double v = -q / 2.0 - sqrt_d;
        u = (u >= 0.0) ? std::cbrt(u) : -std::cbrt(-u);
        v = (v >= 0.0) ? std::cbrt(v) : -std::cbrt(-v);
        double r = u + v - offset;
        if (r >= -1e-4 && r <= 1.0 + 1e-4) roots.push_back(std::clamp(r, 0.0, 1.0));
    } else if (std::abs(delta) <= 1e-11) {
        double u = (q >= 0.0) ? -std::cbrt(q / 2.0) : std::cbrt(-q / 2.0);
        double r1 = 2.0 * u - offset;
        double r2 = -u - offset;
        if (r1 >= -1e-4 && r1 <= 1.0 + 1e-4) roots.push_back(std::clamp(r1, 0.0, 1.0));
        if (r2 >= -1e-4 && r2 <= 1.0 + 1e-4 && std::abs(r2 - r1) > EPSILON) roots.push_back(std::clamp(r2, 0.0, 1.0));
    } else {
        double m = 2.0 * std::sqrt(-p / 3.0);
        double theta = std::acos(std::clamp(-q / (2.0 * std::sqrt(-p * p * p / 27.0)), -1.0, 1.0));
        constexpr double TWO_PI = 6.283185307179586;
        for (int k = 0; k < 3; ++k) {
            double r = m * std::cos((theta + k * TWO_PI) / 3.0) - offset;
            if (r >= -1e-4 && r <= 1.0 + 1e-4) {
                double clamped_r = std::clamp(r, 0.0, 1.0);
                bool exists = false;
                for (double existing : roots) {
                    if (std::abs(existing - clamped_r) < EPSILON) { exists = true; break; }
                }
                if (!exists) roots.push_back(clamped_r);
            }
        }
    }
}

bool intersect_line_line(const OpSegment& s1, const OpSegment& s2, double& t1, double& t2) {
    double x1 = s1.pts[0].x, y1 = s1.pts[0].y;
    double x2 = s1.pts[1].x, y2 = s1.pts[1].y;
    double x3 = s2.pts[0].x, y3 = s2.pts[0].y;
    double x4 = s2.pts[1].x, y4 = s2.pts[1].y;

    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-9) return false;

    double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

    if (t > EPSILON && t < 1.0 - EPSILON && u > EPSILON && u < 1.0 - EPSILON) {
        t1 = t;
        t2 = u;
        return true;
    }
    return false;
}

bool check_collinear_lines(
    const OpSegment& s1, const OpSegment& s2,
    std::vector<double>& splits1, std::vector<double>& splits2
) {
    double x1 = s1.pts[0].x, y1 = s1.pts[0].y;
    double x2 = s1.pts[1].x, y2 = s1.pts[1].y;
    double dx1 = x2 - x1, dy1 = y2 - y1;
    double len1_sq = dx1 * dx1 + dy1 * dy1;
    if (len1_sq < 1e-9) return false;

    double x3 = s2.pts[0].x, y3 = s2.pts[0].y;
    double x4 = s2.pts[1].x, y4 = s2.pts[1].y;
    double dx2 = x4 - x3, dy2 = y4 - y3;
    double len2_sq = dx2 * dx2 + dy2 * dy2;
    if (len2_sq < 1e-9) return false;

    // Check if parallel
    double cross = dx1 * dy2 - dy1 * dx2;
    if (std::abs(cross) > 1e-5) return false;

    // Check if collinear
    double cross_p = dx1 * (y3 - y1) - dy1 * (x3 - x1);
    if (std::abs(cross_p) > 1e-4) return false;

    // Project s2 endpoints onto s1
    double t3 = ((x3 - x1) * dx1 + (y3 - y1) * dy1) / len1_sq;
    double t4 = ((x4 - x1) * dx1 + (y4 - y1) * dy1) / len1_sq;
    if (t3 > EPSILON && t3 < 1.0 - EPSILON) splits1.push_back(t3);
    if (t4 > EPSILON && t4 < 1.0 - EPSILON) splits1.push_back(t4);

    // Project s1 endpoints onto s2
    double s1_p0 = ((x1 - x3) * dx2 + (y1 - y3) * dy2) / len2_sq;
    double s1_p1 = ((x2 - x3) * dx2 + (y2 - y3) * dy2) / len2_sq;
    if (s1_p0 > EPSILON && s1_p0 < 1.0 - EPSILON) splits2.push_back(s1_p0);
    if (s1_p1 > EPSILON && s1_p1 < 1.0 - EPSILON) splits2.push_back(s1_p1);

    return true;
}

void intersect_line_curve(const OpSegment& line, const OpSegment& curve, std::vector<std::pair<double, double>>& hits) {
    double lx0 = line.pts[0].x, ly0 = line.pts[0].y;
    double lx1 = line.pts[1].x, ly1 = line.pts[1].y;
    double dx = lx1 - lx0;
    double dy = ly1 - ly0;
    double len_sq = dx * dx + dy * dy;
    if (len_sq < 1e-9) return;

    double a_line = -dy;
    double b_line = dx;
    double c_line = -(a_line * lx0 + b_line * ly0);

    std::vector<double> curve_ts;
    if (curve.kind == SegmentKind::Quad) {
        double x0 = curve.pts[0].x, y0 = curve.pts[0].y;
        double x1 = curve.pts[1].x, y1 = curve.pts[1].y;
        double x2 = curve.pts[2].x, y2 = curve.pts[2].y;

        double c_quad_a = a_line * (x2 - 2.0 * x1 + x0) + b_line * (y2 - 2.0 * y1 + y0);
        double c_quad_b = a_line * 2.0 * (x1 - x0) + b_line * 2.0 * (y1 - y0);
        double c_quad_c = a_line * x0 + b_line * y0 + c_line;
        solve_quadratic_unit(c_quad_a, c_quad_b, c_quad_c, curve_ts);
    } else if (curve.kind == SegmentKind::Cubic) {
        double x0 = curve.pts[0].x, y0 = curve.pts[0].y;
        double x1 = curve.pts[1].x, y1 = curve.pts[1].y;
        double x2 = curve.pts[2].x, y2 = curve.pts[2].y;
        double x3 = curve.pts[3].x, y3 = curve.pts[3].y;

        double ca = a_line * (x3 - 3.0 * x2 + 3.0 * x1 - x0) + b_line * (y3 - 3.0 * y2 + 3.0 * y1 - y0);
        double cb = a_line * (3.0 * x2 - 6.0 * x1 + 3.0 * x0) + b_line * (3.0 * y2 - 6.0 * y1 + 3.0 * y0);
        double cc = a_line * (3.0 * x1 - 3.0 * x0) + b_line * (3.0 * y1 - 3.0 * y0);
        double cd = a_line * x0 + b_line * y0 + c_line;
        solve_cubic_unit(ca, cb, cc, cd, curve_ts);
    }

    for (double tc : curve_ts) {
        Point p = curve.eval(tc);
        double proj = ((p.x - lx0) * dx + (p.y - ly0) * dy) / len_sq;
        if (proj > EPSILON && proj < 1.0 - EPSILON) {
            hits.emplace_back(proj, tc);
        }
    }
}

void intersect_curves_recursive(
    const OpSegment& s1, double t1_0, double t1_1,
    const OpSegment& s2, double t2_0, double t2_1,
    int depth,
    std::vector<std::pair<double, double>>& hits
) {
    if (!rects_intersect(s1.bounds, s2.bounds)) return;

    if (depth > 12 || (s1.bounds.width() < 1e-3f && s1.bounds.height() < 1e-3f &&
                       s2.bounds.width() < 1e-3f && s2.bounds.height() < 1e-3f)) {
        double m1 = (t1_0 + t1_1) * 0.5;
        double m2 = (t2_0 + t2_1) * 0.5;
        if (m1 > EPSILON && m1 < 1.0 - EPSILON && m2 > EPSILON && m2 < 1.0 - EPSILON) {
            hits.emplace_back(m1, m2);
        }
        return;
    }

    OpSegment s1_l, s1_r, s2_l, s2_r;
    s1.chop_at(0.5, s1_l, s1_r);
    s2.chop_at(0.5, s2_l, s2_r);
    double m1 = (t1_0 + t1_1) * 0.5;
    double m2 = (t2_0 + t2_1) * 0.5;

    intersect_curves_recursive(s1_l, t1_0, m1, s2_l, t2_0, m2, depth + 1, hits);
    intersect_curves_recursive(s1_l, t1_0, m1, s2_r, m2, t2_1, depth + 1, hits);
    intersect_curves_recursive(s1_r, m1, t1_1, s2_l, t2_0, m2, depth + 1, hits);
    intersect_curves_recursive(s1_r, m1, t1_1, s2_r, m2, t2_1, depth + 1, hits);
}

int compute_winding_at_point(Point p, const std::vector<OpSegment>& segs) {
    // Tiny irrational vertical perturbation avoids ray directly grazing vertices and horizontal edges
    double py = p.y + 0.000137;
    int winding = 0;
    for (const auto& s : segs) {
        if (s.bounds.bottom() < py || s.bounds.top() > py || s.bounds.right() < p.x) {
            continue;
        }

        if (s.kind == SegmentKind::Line) {
            double y0 = s.pts[0].y;
            double y1 = s.pts[1].y;
            if ((y0 <= py && y1 > py) || (y1 <= py && y0 > py)) {
                double t = (py - y0) / (y1 - y0);
                double x = s.pts[0].x + t * (s.pts[1].x - s.pts[0].x);
                if (x > p.x) {
                    winding += (y1 > y0) ? 1 : -1;
                }
            }
        } else {
            std::vector<double> roots;
            if (s.kind == SegmentKind::Quad) {
                double a = s.pts[2].y - 2.0 * s.pts[1].y + s.pts[0].y;
                double b = 2.0 * (s.pts[1].y - s.pts[0].y);
                double c = s.pts[0].y - py;
                solve_quadratic_unit(a, b, c, roots);
            } else if (s.kind == SegmentKind::Cubic) {
                double a = s.pts[3].y - 3.0 * s.pts[2].y + 3.0 * s.pts[1].y - s.pts[0].y;
                double b = 3.0 * s.pts[2].y - 6.0 * s.pts[1].y + 3.0 * s.pts[0].y;
                double c = 3.0 * (s.pts[1].y - s.pts[0].y);
                double d = s.pts[0].y - py;
                solve_cubic_unit(a, b, c, d, roots);
            }
            for (double t : roots) {
                Point pt = s.eval(t);
                if (pt.x > p.x) {
                    Point d = s.derivative(t);
                    if (d.y > 1e-6f) winding += 1;
                    else if (d.y < -1e-6f) winding -= 1;
                }
            }
        }
    }
    return winding;
}

bool is_inside(Point p, const std::vector<OpSegment>& segs, FillRule rule) {
    int w = compute_winding_at_point(p, segs);
    return (rule == FillRule::Winding) ? (w != 0) : ((w & 1) != 0);
}

} // namespace

std::optional<Path> path_op(
    const Path& path_a,
    const Path& path_b,
    PathOp op,
    FillRule fill_rule_a,
    FillRule fill_rule_b
) {
    // 1. Fast path for empty operands
    if (path_a.is_empty() && path_b.is_empty()) return Path();
    if (path_a.is_empty()) {
        switch (op) {
            case PathOp::Union:
            case PathOp::Xor:
                return path_b;
            case PathOp::Difference:
            case PathOp::Intersect:
                return Path();
        }
    }
    if (path_b.is_empty()) {
        switch (op) {
            case PathOp::Union:
            case PathOp::Difference:
            case PathOp::Xor:
                return path_a;
            case PathOp::Intersect:
                return Path();
        }
    }

    // 2. Fast path for disjoint bounding boxes
    if (!rects_intersect(path_a.bounds(), path_b.bounds())) {
        switch (op) {
            case PathOp::Union:
            case PathOp::Xor: {
                Path res = path_a;
                res.add_path(path_b);
                return res;
            }
            case PathOp::Difference:
                return path_a;
            case PathOp::Intersect:
                return Path();
        }
    }

    // 3. Decompose paths into atomic segments
    std::vector<OpSegment> segs_a = decompose_path(path_a);
    std::vector<OpSegment> segs_b = decompose_path(path_b);

    if (segs_a.empty()) return path_b;
    if (segs_b.empty()) return path_a;

    std::vector<std::vector<double>> splits_a(segs_a.size());
    std::vector<std::vector<double>> splits_b(segs_b.size());

    // 4. Find all intersections between A and B
    for (size_t i = 0; i < segs_a.size(); ++i) {
        for (size_t j = 0; j < segs_b.size(); ++j) {
            if (!rects_intersect(segs_a[i].bounds, segs_b[j].bounds)) continue;

            if (segs_a[i].kind == SegmentKind::Line && segs_b[j].kind == SegmentKind::Line) {
                // Check collinearity
                if (!check_collinear_lines(segs_a[i], segs_b[j], splits_a[i], splits_b[j])) {
                    double t1 = 0.0, t2 = 0.0;
                    if (intersect_line_line(segs_a[i], segs_b[j], t1, t2)) {
                        splits_a[i].push_back(t1);
                        splits_b[j].push_back(t2);
                    }
                }
            } else if (segs_a[i].kind == SegmentKind::Line) {
                std::vector<std::pair<double, double>> hits;
                intersect_line_curve(segs_a[i], segs_b[j], hits);
                for (const auto& [t1, t2] : hits) {
                    if (t1 > EPSILON && t1 < 1.0 - EPSILON) splits_a[i].push_back(t1);
                    if (t2 > EPSILON && t2 < 1.0 - EPSILON) splits_b[j].push_back(t2);
                }
            } else if (segs_b[j].kind == SegmentKind::Line) {
                std::vector<std::pair<double, double>> hits;
                intersect_line_curve(segs_b[j], segs_a[i], hits);
                for (const auto& [t2, t1] : hits) {
                    if (t1 > EPSILON && t1 < 1.0 - EPSILON) splits_a[i].push_back(t1);
                    if (t2 > EPSILON && t2 < 1.0 - EPSILON) splits_b[j].push_back(t2);
                }
            } else {
                std::vector<std::pair<double, double>> hits;
                intersect_curves_recursive(segs_a[i], 0.0, 1.0, segs_b[j], 0.0, 1.0, 0, hits);
                for (const auto& [t1, t2] : hits) {
                    if (t1 > EPSILON && t1 < 1.0 - EPSILON) splits_a[i].push_back(t1);
                    if (t2 > EPSILON && t2 < 1.0 - EPSILON) splits_b[j].push_back(t2);
                }
            }
        }
    }

    // 5. Subdivide segments at intersection parameters
    auto subdivide = [](const std::vector<OpSegment>& original, std::vector<std::vector<double>>& splits) {
        std::vector<OpSegment> result;
        for (size_t i = 0; i < original.size(); ++i) {
            auto& s_list = splits[i];
            std::sort(s_list.begin(), s_list.end());
            std::vector<double> unique_splits;
            for (double t : s_list) {
                if (t <= EPSILON || t >= 1.0 - EPSILON) continue;
                if (unique_splits.empty() || (t - unique_splits.back()) > EPSILON) {
                    unique_splits.push_back(t);
                }
            }

            if (unique_splits.empty()) {
                result.push_back(original[i]);
            } else {
                OpSegment current = original[i];
                double prev_t = 0.0;
                for (double t : unique_splits) {
                    double local_t = (t - prev_t) / (1.0 - prev_t);
                    if (local_t <= EPSILON || local_t >= 1.0 - EPSILON) continue;
                    OpSegment left, right;
                    current.chop_at(local_t, left, right);
                    result.push_back(left);
                    current = right;
                    prev_t = t;
                }
                result.push_back(current);
            }
        }
        return result;
    };

    std::vector<OpSegment> atomic_a = subdivide(segs_a, splits_a);
    std::vector<OpSegment> atomic_b = subdivide(segs_b, splits_b);

    // 6. Classify and filter segments according to boolean operation
    std::vector<OpSegment> kept_segments;

    for (const auto& s : atomic_a) {
        Point mid = s.eval(0.5);
        Point norm = s.normal(0.5);
        Point p_in = mid + scale_point(norm, 0.005f);
        Point p_out = mid - scale_point(norm, 0.005f);
        bool in_b_in = is_inside(p_in, segs_b, fill_rule_b);
        bool in_b_out = is_inside(p_out, segs_b, fill_rule_b);

        switch (op) {
            case PathOp::Union:
                if (!in_b_out) kept_segments.push_back(s);
                break;
            case PathOp::Difference:
                if (!in_b_in) kept_segments.push_back(s);
                break;
            case PathOp::Intersect:
                if (in_b_in) kept_segments.push_back(s);
                break;
            case PathOp::Xor:
                if (!in_b_in && !in_b_out) kept_segments.push_back(s);
                else if (in_b_in && in_b_out) kept_segments.push_back(s.reversed());
                break;
        }
    }

    for (const auto& s : atomic_b) {
        Point mid = s.eval(0.5);
        Point norm = s.normal(0.5);
        Point p_in = mid + scale_point(norm, 0.005f);
        Point p_out = mid - scale_point(norm, 0.005f);
        bool in_a_in = is_inside(p_in, segs_a, fill_rule_a);
        bool in_a_out = is_inside(p_out, segs_a, fill_rule_a);

        switch (op) {
            case PathOp::Union:
                if (!in_a_out) kept_segments.push_back(s);
                break;
            case PathOp::Difference:
                if (in_a_out) kept_segments.push_back(s.reversed());
                break;
            case PathOp::Intersect:
                if (in_a_in) kept_segments.push_back(s);
                break;
            case PathOp::Xor:
                if (!in_a_in && !in_a_out) kept_segments.push_back(s);
                else if (in_a_in && in_a_out) kept_segments.push_back(s.reversed());
                break;
        }
    }

    if (kept_segments.empty()) {
        return Path();
    }

    // Deduplicate identical coincident segments
    std::vector<OpSegment> filtered_kept;
    for (size_t i = 0; i < kept_segments.size(); ++i) {
        bool duplicate = false;
        Point p0_i = kept_segments[i].start_point();
        Point p1_i = kept_segments[i].end_point();

        for (size_t j = 0; j < filtered_kept.size(); ++j) {
            Point p0_j = filtered_kept[j].start_point();
            Point p1_j = filtered_kept[j].end_point();
            if (std::hypot(p0_i.x - p0_j.x, p0_i.y - p0_j.y) < POINT_TOLERANCE * 5.0f &&
                std::hypot(p1_i.x - p1_j.x, p1_i.y - p1_j.y) < POINT_TOLERANCE * 5.0f) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            filtered_kept.push_back(kept_segments[i]);
        }
    }
    kept_segments = std::move(filtered_kept);

    // 7. Topological chaining of kept segments into closed contours
    PathBuilder builder;
    std::vector<bool> used(kept_segments.size(), false);
    size_t remaining = kept_segments.size();

    while (remaining > 0) {
        size_t start_idx = 0;
        while (start_idx < kept_segments.size() && used[start_idx]) {
            start_idx++;
        }
        if (start_idx >= kept_segments.size()) break;

        size_t curr_idx = start_idx;
        used[curr_idx] = true;
        remaining--;

        Point contour_start = kept_segments[curr_idx].start_point();
        builder.move_to(contour_start);

        while (true) {
            const auto& cur_seg = kept_segments[curr_idx];
            switch (cur_seg.kind) {
                case SegmentKind::Line:
                    builder.line_to(cur_seg.pts[1]);
                    break;
                case SegmentKind::Quad:
                    builder.quad_to(cur_seg.pts[1], cur_seg.pts[2]);
                    break;
                case SegmentKind::Cubic:
                    builder.cubic_to(cur_seg.pts[1], cur_seg.pts[2], cur_seg.pts[3]);
                    break;
            }

            Point tip = cur_seg.end_point();

            float dist_to_start = std::hypot(tip.x - contour_start.x, tip.y - contour_start.y);
            if (dist_to_start <= POINT_TOLERANCE * 10.0f) {
                builder.close();
                break;
            }

            size_t best_next = kept_segments.size();
            float best_dist = std::numeric_limits<float>::max();

            for (size_t k = 0; k < kept_segments.size(); ++k) {
                if (used[k]) continue;
                Point p_start = kept_segments[k].start_point();
                float d = std::hypot(tip.x - p_start.x, tip.y - p_start.y);
                if (d < best_dist) {
                    best_dist = d;
                    best_next = k;
                }
            }

            if (best_next < kept_segments.size() && best_dist <= POINT_TOLERANCE * 50.0f) {
                curr_idx = best_next;
                used[curr_idx] = true;
                remaining--;
            } else {
                builder.close();
                break;
            }
        }
    }

    return builder.finish();
}

std::optional<Path> Path::op(const Path& other, PathOp operation, FillRule fill_rule_a, FillRule fill_rule_b) const {
    return path_op(*this, other, operation, fill_rule_a, fill_rule_b);
}

std::optional<Path> Path::unite(const Path& other) const {
    return path_union(*this, other);
}

std::optional<Path> Path::difference(const Path& other) const {
    return path_difference(*this, other);
}

std::optional<Path> Path::intersect(const Path& other) const {
    return path_intersect(*this, other);
}

std::optional<Path> Path::xor_op(const Path& other) const {
    return path_xor(*this, other);
}

} // namespace nisaba

#include "nisaba/path/path_measure.hpp"
#include "nisaba/path/path_geometry.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba {

namespace {

constexpr uint32_t MAX_T_VALUE = 0x3FFFFFFF;
constexpr float MAX_T_RECIPROCAL = 1.0f / static_cast<float>(MAX_T_VALUE);

inline uint32_t t_span_big_enough(uint32_t t_span) noexcept {
    return t_span >> 10;
}

inline float interp_safe(float a, float b, float t) noexcept {
    return a + (b - a) * t;
}

inline bool cheap_dist_exceeds_limit(Point pt, float x, float y, float tolerance) noexcept {
    float dist = std::max(std::abs(x - pt.x), std::abs(y - pt.y));
    return dist > tolerance;
}

inline bool quad_too_curvy(Point p0, Point p1, Point p2, float tolerance) noexcept {
    float dx = scalar::half(p1.x) - scalar::half(scalar::half(p0.x + p2.x));
    float dy = scalar::half(p1.y) - scalar::half(scalar::half(p0.y + p2.y));
    float dist = std::max(std::abs(dx), std::abs(dy));
    return dist > tolerance;
}

inline bool cubic_too_curvy(Point p0, Point p1, Point p2, Point p3, float tolerance) noexcept {
    bool n0 = cheap_dist_exceeds_limit(
        p1,
        interp_safe(p0.x, p3.x, 1.0f / 3.0f),
        interp_safe(p0.y, p3.y, 1.0f / 3.0f),
        tolerance
    );
    bool n1 = cheap_dist_exceeds_limit(
        p2,
        interp_safe(p0.x, p3.x, 2.0f / 3.0f),
        interp_safe(p0.y, p3.y, 2.0f / 3.0f),
        tolerance
    );
    return n0 || n1;
}

inline float interp_scalar(float a, float b, NormalizedF32 t) noexcept {
    return a + (b - a) * t.get();
}

float compute_line_seg(PathMeasure::Contour& contour, Point p0, Point p1, float distance, size_t point_index) {
    float d = p0.distance(p1);
    float prev_d = distance;
    distance += d;
    if (distance > prev_d) {
        contour.segments.push_back(PathMeasure::Segment{distance, point_index, MAX_T_VALUE, PathMeasure::SegmentType::Line});
    }
    return distance;
}

float compute_quad_segs(
    PathMeasure::Contour& contour,
    Point p0, Point p1, Point p2,
    float distance, uint32_t min_t, uint32_t max_t,
    size_t point_index, float tolerance
) {
    if (t_span_big_enough(max_t - min_t) != 0 && quad_too_curvy(p0, p1, p2, tolerance)) {
        Point tmp[5];
        uint32_t half_t = (min_t + max_t) >> 1;
        Point src[3] = {p0, p1, p2};
        path_geometry::chop_quad_at(src, NormalizedF32Exclusive::HALF, tmp);

        distance = compute_quad_segs(contour, tmp[0], tmp[1], tmp[2], distance, min_t, half_t, point_index, tolerance);
        distance = compute_quad_segs(contour, tmp[2], tmp[3], tmp[4], distance, half_t, max_t, point_index, tolerance);
    } else {
        float d = p0.distance(p2);
        float prev_d = distance;
        distance += d;
        if (distance > prev_d) {
            contour.segments.push_back(PathMeasure::Segment{distance, point_index, max_t, PathMeasure::SegmentType::Quad});
        }
    }
    return distance;
}

float compute_cubic_segs(
    PathMeasure::Contour& contour,
    Point p0, Point p1, Point p2, Point p3,
    float distance, uint32_t min_t, uint32_t max_t,
    size_t point_index, float tolerance
) {
    if (t_span_big_enough(max_t - min_t) != 0 && cubic_too_curvy(p0, p1, p2, p3, tolerance)) {
        Point tmp[7];
        uint32_t half_t = (min_t + max_t) >> 1;
        Point src[4] = {p0, p1, p2, p3};
        path_geometry::chop_cubic_at2(src, NormalizedF32Exclusive::HALF, tmp);

        distance = compute_cubic_segs(contour, tmp[0], tmp[1], tmp[2], tmp[3], distance, min_t, half_t, point_index, tolerance);
        distance = compute_cubic_segs(contour, tmp[3], tmp[4], tmp[5], tmp[6], distance, half_t, max_t, point_index, tolerance);
    } else {
        float d = p0.distance(p3);
        float prev_d = distance;
        distance += d;
        if (distance > prev_d) {
            contour.segments.push_back(PathMeasure::Segment{distance, point_index, max_t, PathMeasure::SegmentType::Cubic});
        }
    }
    return distance;
}

int32_t find_segment(const std::vector<PathMeasure::Segment>& base, float key) noexcept {
    if (base.empty()) return 0;
    uint32_t lo = 0;
    uint32_t hi = static_cast<uint32_t>(base.size() - 1);

    while (lo < hi) {
        uint32_t mid = (hi + lo) >> 1;
        if (base[mid].distance < key) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (base[hi].distance < key) {
        hi += 1;
        hi = ~hi;
    } else if (key < base[hi].distance) {
        hi = ~hi;
    }

    return static_cast<int32_t>(hi);
}

void compute_pos_tan(
    const Point* points,
    PathMeasure::SegmentType seg_kind,
    NormalizedF32 t,
    Point* pos,
    Point* tangent
) noexcept {
    switch (seg_kind) {
        case PathMeasure::SegmentType::Line: {
            if (pos) {
                pos->x = interp_scalar(points[0].x, points[1].x, t);
                pos->y = interp_scalar(points[0].y, points[1].y, t);
            }
            if (tangent) {
                if (!tangent->set_normalize(points[1].x - points[0].x, points[1].y - points[0].y)) {
                    *tangent = Point::from_xy(1.0f, 0.0f);
                }
            }
            break;
        }
        case PathMeasure::SegmentType::Quad: {
            if (pos) {
                *pos = path_geometry::eval_quad_at(points, t);
            }
            if (tangent) {
                *tangent = path_geometry::eval_quad_tangent_at(points, t);
                if (!tangent->normalize()) {
                    *tangent = Point::from_xy(1.0f, 0.0f);
                }
            }
            break;
        }
        case PathMeasure::SegmentType::Cubic: {
            if (pos) {
                *pos = path_geometry::eval_cubic_pos_at(points, t);
            }
            if (tangent) {
                *tangent = path_geometry::eval_cubic_tangent_at(points, t);
                if (!tangent->normalize()) {
                    *tangent = Point::from_xy(1.0f, 0.0f);
                }
            }
            break;
        }
    }
}

} // anonymous namespace

void PathMeasure::segment_to_path(
    const Point* points,
    PathMeasure::SegmentType seg_kind,
    NormalizedF32 start_t,
    NormalizedF32 stop_t,
    Path& dst
) noexcept {
    if (start_t == stop_t) return;

    switch (seg_kind) {
        case PathMeasure::SegmentType::Line: {
            dst.verbs_.push_back(PathVerb::Line);
            if (stop_t == NormalizedF32::ONE) {
                dst.points_.push_back(points[1]);
            } else {
                dst.points_.push_back(Point::from_xy(
                    interp_scalar(points[0].x, points[1].x, stop_t),
                    interp_scalar(points[0].y, points[1].y, stop_t)
                ));
            }
            break;
        }
        case PathMeasure::SegmentType::Quad: {
            Point tmp0[5];
            Point tmp1[5];
            if (start_t == NormalizedF32::ZERO) {
                if (stop_t == NormalizedF32::ONE) {
                    dst.verbs_.push_back(PathVerb::Quad);
                    dst.points_.push_back(points[1]);
                    dst.points_.push_back(points[2]);
                } else {
                    auto stop_ex = NormalizedF32Exclusive::create_bounded(stop_t.get());
                    path_geometry::chop_quad_at(points, stop_ex, tmp0);
                    dst.verbs_.push_back(PathVerb::Quad);
                    dst.points_.push_back(tmp0[1]);
                    dst.points_.push_back(tmp0[2]);
                }
            } else {
                auto start_ex = NormalizedF32Exclusive::create_bounded(start_t.get());
                path_geometry::chop_quad_at(points, start_ex, tmp0);
                if (stop_t == NormalizedF32::ONE) {
                    dst.verbs_.push_back(PathVerb::Quad);
                    dst.points_.push_back(tmp0[3]);
                    dst.points_.push_back(tmp0[4]);
                } else {
                    float new_t = (stop_t.get() - start_t.get()) / (1.0f - start_t.get());
                    auto new_t_ex = NormalizedF32Exclusive::create_bounded(new_t);
                    path_geometry::chop_quad_at(tmp0 + 2, new_t_ex, tmp1);
                    dst.verbs_.push_back(PathVerb::Quad);
                    dst.points_.push_back(tmp1[1]);
                    dst.points_.push_back(tmp1[2]);
                }
            }
            break;
        }
        case PathMeasure::SegmentType::Cubic: {
            Point tmp0[7];
            Point tmp1[7];
            if (start_t == NormalizedF32::ZERO) {
                if (stop_t == NormalizedF32::ONE) {
                    dst.verbs_.push_back(PathVerb::Cubic);
                    dst.points_.push_back(points[1]);
                    dst.points_.push_back(points[2]);
                    dst.points_.push_back(points[3]);
                } else {
                    auto stop_ex = NormalizedF32Exclusive::create_bounded(stop_t.get());
                    path_geometry::chop_cubic_at2(points, stop_ex, tmp0);
                    dst.verbs_.push_back(PathVerb::Cubic);
                    dst.points_.push_back(tmp0[1]);
                    dst.points_.push_back(tmp0[2]);
                    dst.points_.push_back(tmp0[3]);
                }
            } else {
                auto start_ex = NormalizedF32Exclusive::create_bounded(start_t.get());
                path_geometry::chop_cubic_at2(points, start_ex, tmp0);
                if (stop_t == NormalizedF32::ONE) {
                    dst.verbs_.push_back(PathVerb::Cubic);
                    dst.points_.push_back(tmp0[4]);
                    dst.points_.push_back(tmp0[5]);
                    dst.points_.push_back(tmp0[6]);
                } else {
                    float new_t = (stop_t.get() - start_t.get()) / (1.0f - start_t.get());
                    auto new_t_ex = NormalizedF32Exclusive::create_bounded(new_t);
                    path_geometry::chop_cubic_at2(tmp0 + 3, new_t_ex, tmp1);
                    dst.verbs_.push_back(PathVerb::Cubic);
                    dst.points_.push_back(tmp1[1]);
                    dst.points_.push_back(tmp1[2]);
                    dst.points_.push_back(tmp1[3]);
                }
            }
            break;
        }
    }
}

float PathMeasure::Segment::scalar_t() const noexcept {
    return static_cast<float>(t_value) * MAX_T_RECIPROCAL;
}

std::optional<std::pair<size_t, NormalizedF32>> PathMeasure::Contour::distance_to_segment(float distance) const noexcept {
    if (segments.empty()) return std::nullopt;

    int32_t index = find_segment(segments, distance);
    index ^= (index >> 31);
    size_t idx = static_cast<size_t>(index);
    if (idx >= segments.size()) {
        idx = segments.size() - 1;
    }
    const Segment& seg = segments[idx];

    float start_t = 0.0f;
    float start_d = 0.0f;
    if (idx > 0) {
        start_d = segments[idx - 1].distance;
        if (segments[idx - 1].point_index == seg.point_index) {
            start_t = segments[idx - 1].scalar_t();
        }
    }

    float t = seg.scalar_t();
    float denom = seg.distance - start_d;
    if (denom > 1e-6f) {
        t = start_t + (seg.scalar_t() - start_t) * (distance - start_d) / denom;
    }
    t = std::clamp(t, 0.0f, 1.0f);
    return std::make_pair(idx, NormalizedF32::create_clamped(t));
}

PathMeasure::PathMeasure(const Path& path, bool force_closed, float res_scale) {
    set_path(path, force_closed, res_scale);
}

PathMeasure::PathMeasure(const Path* path, bool force_closed, float res_scale) {
    set_path(path, force_closed, res_scale);
}

void PathMeasure::set_path(const Path& path, bool force_closed, float res_scale) {
    build_contours(path, force_closed, res_scale);
}

void PathMeasure::set_path(const Path* path, bool force_closed, float res_scale) {
    if (path) {
        build_contours(*path, force_closed, res_scale);
    } else {
        reset();
    }
}

void PathMeasure::reset() noexcept {
    contours_.clear();
    current_contour_idx_ = 0;
}

float PathMeasure::length() const noexcept {
    if (current_contour_idx_ >= contours_.size()) {
        return 0.0f;
    }
    return contours_[current_contour_idx_].length;
}

bool PathMeasure::is_closed() const noexcept {
    if (current_contour_idx_ >= contours_.size()) {
        return false;
    }
    return contours_[current_contour_idx_].is_closed;
}

bool PathMeasure::next_contour() {
    if (contours_.empty()) return false;
    if (current_contour_idx_ + 1 < contours_.size()) {
        current_contour_idx_++;
        return true;
    }
    current_contour_idx_ = contours_.size();
    return false;
}

bool PathMeasure::get_pos_tan(float distance, Point* position, Point* tangent) const noexcept {
    if (current_contour_idx_ >= contours_.size()) return false;
    const auto& contour = contours_[current_contour_idx_];
    if (contour.length <= 0.0f || contour.segments.empty()) return false;
    if (!std::isfinite(distance)) return false;

    float d = std::clamp(distance, 0.0f, contour.length);

    auto seg_pair = contour.distance_to_segment(d);
    if (!seg_pair) return false;
    auto [seg_idx, t] = *seg_pair;
    const auto& seg = contour.segments[seg_idx];

    compute_pos_tan(contour.points.data() + seg.point_index, seg.kind, t, position, tangent);
    return true;
}

bool PathMeasure::get_segment(float start_d, float stop_d, Path* dst, bool start_with_move_to) const {
    if (!dst) return false;
    if (current_contour_idx_ >= contours_.size()) return false;
    const auto& contour = contours_[current_contour_idx_];
    if (contour.length <= 0.0f || contour.segments.empty()) return false;
    if (!std::isfinite(start_d) || !std::isfinite(stop_d)) return false;

    if (start_d < 0.0f) start_d = 0.0f;
    if (stop_d > contour.length) stop_d = contour.length;
    if (!(start_d < stop_d)) return false;

    auto start_pair = contour.distance_to_segment(start_d);
    auto stop_pair = contour.distance_to_segment(stop_d);
    if (!start_pair || !stop_pair) return false;

    auto [seg_index, start_t] = *start_pair;
    auto [stop_seg_index, stop_t] = *stop_pair;
    Segment seg = contour.segments[seg_index];
    Segment stop_seg = contour.segments[stop_seg_index];

    Point p;
    compute_pos_tan(contour.points.data() + seg.point_index, seg.kind, start_t, &p, nullptr);

    if (start_with_move_to || dst->verbs_.empty()) {
        dst->verbs_.push_back(PathVerb::Move);
        dst->points_.push_back(p);
    } else {
        if (dst->points_.back() != p) {
            dst->verbs_.push_back(PathVerb::Line);
            dst->points_.push_back(p);
        }
    }

    if (seg.point_index == stop_seg.point_index) {
        segment_to_path(contour.points.data() + seg.point_index, seg.kind, start_t, stop_t, *dst);
    } else {
        size_t new_seg_index = seg_index;
        while (true) {
            segment_to_path(contour.points.data() + seg.point_index, seg.kind, start_t, NormalizedF32::ONE, *dst);

            size_t old_point_index = seg.point_index;
            while (true) {
                new_seg_index++;
                if (new_seg_index >= contour.segments.size() || contour.segments[new_seg_index].point_index != old_point_index) {
                    break;
                }
            }
            if (new_seg_index >= contour.segments.size()) break;
            seg = contour.segments[new_seg_index];

            start_t = NormalizedF32::ZERO;
            if (seg.point_index >= stop_seg.point_index) {
                break;
            }
        }

        segment_to_path(contour.points.data() + seg.point_index, seg.kind, NormalizedF32::ZERO, stop_t, *dst);
    }

    if (auto b = Rect::from_points(dst->points_.data(), dst->points_.size())) {
        dst->bounds_ = *b;
    }
    return true;
}

void PathMeasure::build_contours(const Path& path, bool force_closed, float res_scale) {
    contours_.clear();
    current_contour_idx_ = 0;
    if (path.is_empty()) return;

    float scale = (res_scale <= 0.0f) ? 1.0f : res_scale;
    float tolerance = 0.5f * scalar::invert(scale);
    // Upper bound tolerance to guarantee sub-millimeter geometric accuracy
    tolerance = std::min(tolerance, 0.25f);

    auto iter = path.segments();
    while (true) {
        Contour contour;
        contour.segments.reserve(64);
        contour.points.reserve(128);

        size_t point_index = 0;
        float distance = 0.0f;
        bool have_seen_close = false;
        Point prev_p = Point::zero();
        bool has_verbs = false;

        while (auto seg = iter.next()) {
            has_verbs = true;
            switch (seg->type) {
                case PathSegment::Type::MoveTo: {
                    contour.points.push_back(seg->p0);
                    prev_p = seg->p0;
                    break;
                }
                case PathSegment::Type::LineTo: {
                    float prev_d = distance;
                    distance = compute_line_seg(contour, prev_p, seg->p0, distance, point_index);
                    if (distance > prev_d) {
                        contour.points.push_back(seg->p0);
                        point_index++;
                    }
                    prev_p = seg->p0;
                    break;
                }
                case PathSegment::Type::QuadTo: {
                    float prev_d = distance;
                    distance = compute_quad_segs(
                        contour, prev_p, seg->p0, seg->p1, distance, 0, MAX_T_VALUE, point_index, tolerance
                    );
                    if (distance > prev_d) {
                        contour.points.push_back(seg->p0);
                        contour.points.push_back(seg->p1);
                        point_index += 2;
                    }
                    prev_p = seg->p1;
                    break;
                }
                case PathSegment::Type::CubicTo: {
                    float prev_d = distance;
                    distance = compute_cubic_segs(
                        contour, prev_p, seg->p0, seg->p1, seg->p2, distance, 0, MAX_T_VALUE, point_index, tolerance
                    );
                    if (distance > prev_d) {
                        contour.points.push_back(seg->p0);
                        contour.points.push_back(seg->p1);
                        contour.points.push_back(seg->p2);
                        point_index += 3;
                    }
                    prev_p = seg->p2;
                    break;
                }
                case PathSegment::Type::Close: {
                    have_seen_close = true;
                    break;
                }
            }

            if (iter.next_verb() == PathVerb::Move) {
                break;
            }
        }

        if (!has_verbs) break;

        if ((have_seen_close || force_closed) && !contour.points.empty()) {
            Point first_pt = contour.points[0];
            if (prev_p != first_pt) {
                float prev_d = distance;
                distance = compute_line_seg(contour, prev_p, first_pt, distance, point_index);
                if (distance > prev_d) {
                    contour.points.push_back(first_pt);
                    point_index++;
                }
            }
            contour.is_closed = true;
        } else {
            contour.is_closed = have_seen_close;
        }

        contour.length = distance;

        if (distance > 0.0f && !contour.segments.empty()) {
            contours_.push_back(std::move(contour));
        }

        if (!iter.next_verb().has_value()) {
            break;
        }
    }
}

} // namespace nisaba

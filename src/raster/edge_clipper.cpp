#include "nisaba/raster/edge_clipper.hpp"
#include "nisaba/raster/line_clipper.hpp"
#include "nisaba/path/path_geometry.hpp"
#include "nisaba/math/scalar.hpp"
#include <algorithm>
#include <cassert>

namespace nisaba {

namespace {

inline bool quick_reject(const Rect& bounds, const Rect& clip) noexcept {
    return bounds.top() >= clip.bottom() || bounds.bottom() <= clip.top();
}

inline bool sort_increasing_y(const Point* src, Point* dst, size_t count) noexcept {
    if (src[0].y > src[count - 1].y) {
        for (size_t i = 0; i < count; ++i) {
            dst[i] = src[count - 1 - i];
        }
        return true;
    } else {
        for (size_t i = 0; i < count; ++i) {
            dst[i] = src[i];
        }
        return false;
    }
}

inline bool chop_mono_quad_at(float c0, float c1, float c2, float target, NormalizedF32Exclusive* t) noexcept {
    float a = c0 - c1 - c1 + c2;
    float b = 2.0f * (c1 - c0);
    float c = c0 - target;

    std::array<NormalizedF32Exclusive, 2> roots{};
    size_t count = path_geometry::find_unit_quad_roots(a, b, c, roots.data());
    if (count != 0) {
        *t = roots[0];
        return true;
    }
    return false;
}

inline bool chop_mono_quad_at_x(const Point* pts, float x, NormalizedF32Exclusive* t) noexcept {
    return chop_mono_quad_at(pts[0].x, pts[1].x, pts[2].x, x, t);
}

inline bool chop_mono_quad_at_y(const Point* pts, float y, NormalizedF32Exclusive* t) noexcept {
    return chop_mono_quad_at(pts[0].y, pts[1].y, pts[2].y, y, t);
}

inline void chop_quad_in_y(const Rect& clip, Point* pts) noexcept {
    NormalizedF32Exclusive t = NormalizedF32Exclusive::create_unchecked(0.5f);
    std::array<Point, 5> tmp{};

    if (pts[0].y < clip.top()) {
        if (chop_mono_quad_at_y(pts, clip.top(), &t)) {
            path_geometry::chop_quad_at(pts, t, tmp.data());
            tmp[2] = Point(tmp[2].x, clip.top());
            tmp[3] = Point(tmp[3].x, std::max(tmp[3].y, clip.top()));

            pts[0] = tmp[2];
            pts[1] = tmp[3];
        } else {
            for (size_t i = 0; i < 3; ++i) {
                if (pts[i].y < clip.top()) {
                    pts[i] = Point(pts[i].x, clip.top());
                }
            }
        }
    }

    if (pts[2].y > clip.bottom()) {
        if (chop_mono_quad_at_y(pts, clip.bottom(), &t)) {
            path_geometry::chop_quad_at(pts, t, tmp.data());
            tmp[1] = Point(tmp[1].x, std::min(tmp[1].y, clip.bottom()));
            tmp[2] = Point(tmp[2].x, clip.bottom());

            pts[1] = tmp[1];
            pts[2] = tmp[2];
        } else {
            for (size_t i = 0; i < 3; ++i) {
                if (pts[i].y > clip.bottom()) {
                    pts[i] = Point(pts[i].x, clip.bottom());
                }
            }
        }
    }
}

inline bool too_big_for_reliable_float_math(const Rect& r) noexcept {
    constexpr float limit = static_cast<float>(1 << 22);
    return r.left() < -limit || r.top() < -limit || r.right() > limit || r.bottom() > limit;
}

inline NormalizedF32Exclusive mono_cubic_closest_t(const float* src, float x) noexcept {
    float t = 0.5f;
    float last_t = 0.0f;
    float best_t = t;
    float step = 0.25f;
    float d = src[0];
    float a = src[3] + 3.0f * (src[1] - src[2]) - d;
    float b = 3.0f * (src[2] - src[1] - src[1] + d);
    float c = 3.0f * (src[1] - d);
    x -= d;
    float closest = SCALAR_MAX;
    while (true) {
        float loc = ((a * t + b) * t + c) * t;
        float dist = std::abs(loc - x);
        if (closest > dist) {
            closest = dist;
            best_t = t;
        }

        last_t = t;
        t += (loc < x) ? step : -step;
        step *= 0.5f;

        if (!(closest > 0.25f && last_t != t)) {
            break;
        }
    }

    auto res = NormalizedF32Exclusive::create(best_t);
    return res ? *res : NormalizedF32Exclusive::create_unchecked(0.5f);
}

inline void chop_mono_cubic_at_x(const Point* src, float x, Point* dst) noexcept {
    if (path_geometry::chop_mono_cubic_at_x(src, x, dst)) {
        return;
    }
    float src_vals[4] = {src[0].x, src[1].x, src[2].x, src[3].x};
    path_geometry::chop_cubic_at2(src, mono_cubic_closest_t(src_vals, x), dst);
}

inline void chop_mono_cubic_at_y(const Point* src, float y, Point* dst) noexcept {
    if (path_geometry::chop_mono_cubic_at_y(src, y, dst)) {
        return;
    }
    float src_vals[4] = {src[0].y, src[1].y, src[2].y, src[3].y};
    path_geometry::chop_cubic_at2(src, mono_cubic_closest_t(src_vals, y), dst);
}

inline void chop_cubic_in_y(const Rect& clip, Point* pts) noexcept {
    if (pts[0].y < clip.top()) {
        std::array<Point, 7> tmp{};
        chop_mono_cubic_at_y(pts, clip.top(), tmp.data());

        if (tmp[3].y < clip.top() && tmp[4].y < clip.top() && tmp[5].y < clip.top()) {
            std::array<Point, 4> tmp2 = {tmp[3], tmp[4], tmp[5], tmp[6]};
            chop_mono_cubic_at_y(tmp2.data(), clip.top(), tmp.data());
        }

        tmp[3] = Point(tmp[3].x, clip.top());
        tmp[4] = Point(tmp[4].x, std::max(tmp[4].y, clip.top()));

        pts[0] = tmp[3];
        pts[1] = tmp[4];
        pts[2] = tmp[5];
    }

    if (pts[3].y > clip.bottom()) {
        std::array<Point, 7> tmp{};
        chop_mono_cubic_at_y(pts, clip.bottom(), tmp.data());
        tmp[3] = Point(tmp[3].x, clip.bottom());
        tmp[2] = Point(tmp[2].x, std::min(tmp[2].y, clip.bottom()));

        pts[1] = tmp[1];
        pts[2] = tmp[2];
        pts[3] = tmp[3];
    }
}

} // namespace

void EdgeClipper::push_vline(float x, float y0, float y1, bool reverse, std::vector<PathEdge>& out) {
    if (reverse) {
        std::swap(y0, y1);
    }
    out.push_back(PathEdge::line(Point(x, y0), Point(x, y1)));
}

void EdgeClipper::push_quad(const Point* pts, bool reverse, std::vector<PathEdge>& out) {
    if (reverse) {
        out.push_back(PathEdge::quad(pts[2], pts[1], pts[0]));
    } else {
        out.push_back(PathEdge::quad(pts[0], pts[1], pts[2]));
    }
}

void EdgeClipper::push_cubic(const Point* pts, bool reverse, std::vector<PathEdge>& out) {
    if (reverse) {
        out.push_back(PathEdge::cubic(pts[3], pts[2], pts[1], pts[0]));
    } else {
        out.push_back(PathEdge::cubic(pts[0], pts[1], pts[2], pts[3]));
    }
}

bool EdgeClipper::clip_line(Point p0, Point p1, std::vector<PathEdge>& out) {
    size_t start_len = out.size();
    std::array<Point, line_clipper::MAX_POINTS> pts{};
    size_t count = line_clipper::clip({p0, p1}, clip_, can_cull_to_the_right_, pts);
    if (count > 1) {
        for (size_t i = 0; i < count - 1; ++i) {
            out.push_back(PathEdge::line(pts[i], pts[i + 1]));
        }
    }
    return out.size() > start_len;
}

void EdgeClipper::clip_mono_quad(const Point* src, std::vector<PathEdge>& out) {
    std::array<Point, 3> pts{};
    bool reverse = sort_increasing_y(src, pts.data(), 3);

    if (pts[2].y <= clip_.top() || pts[0].y >= clip_.bottom()) {
        return;
    }

    chop_quad_in_y(clip_, pts.data());

    if (pts[0].x > pts[2].x) {
        std::swap(pts[0], pts[2]);
        reverse = !reverse;
    }

    if (pts[2].x <= clip_.left()) {
        push_vline(clip_.left(), pts[0].y, pts[2].y, reverse, out);
        return;
    }

    if (pts[0].x >= clip_.right()) {
        if (!can_cull_to_the_right_) {
            push_vline(clip_.right(), pts[0].y, pts[2].y, reverse, out);
        }
        return;
    }

    NormalizedF32Exclusive t = NormalizedF32Exclusive::create_unchecked(0.5f);
    std::array<Point, 5> tmp{};

    if (pts[0].x < clip_.left()) {
        if (chop_mono_quad_at_x(pts.data(), clip_.left(), &t)) {
            path_geometry::chop_quad_at(pts.data(), t, tmp.data());
            push_vline(clip_.left(), tmp[0].y, tmp[2].y, reverse, out);
            tmp[2] = Point(clip_.left(), tmp[2].y);
            tmp[3] = Point(std::max(tmp[3].x, clip_.left()), tmp[3].y);

            pts[0] = tmp[2];
            pts[1] = tmp[3];
        } else {
            push_vline(clip_.left(), pts[0].y, pts[2].y, reverse, out);
            return;
        }
    }

    if (pts[2].x > clip_.right()) {
        if (chop_mono_quad_at_x(pts.data(), clip_.right(), &t)) {
            path_geometry::chop_quad_at(pts.data(), t, tmp.data());
            tmp[1] = Point(std::min(tmp[1].x, clip_.right()), tmp[1].y);
            tmp[2] = Point(clip_.right(), tmp[2].y);

            push_quad(tmp.data(), reverse, out);
            push_vline(clip_.right(), tmp[2].y, tmp[4].y, reverse, out);
        } else {
            pts[1] = Point(std::min(pts[1].x, clip_.right()), pts[1].y);
            pts[2] = Point(std::min(pts[2].x, clip_.right()), pts[2].y);
            push_quad(pts.data(), reverse, out);
        }
    } else {
        push_quad(pts.data(), reverse, out);
    }
}

bool EdgeClipper::clip_quad(Point p0, Point p1, Point p2, std::vector<PathEdge>& out) {
    size_t start_len = out.size();
    std::array<Point, 3> pts = {p0, p1, p2};
    auto bounds = Rect::from_points(pts.data(), 3);
    if (!bounds) return false;

    if (!quick_reject(*bounds, clip_)) {
        std::array<Point, 5> mono_y{};
        size_t count_y = path_geometry::chop_quad_at_y_extrema(pts.data(), mono_y.data());
        for (size_t y = 0; y <= count_y; ++y) {
            std::array<Point, 5> mono_x{};
            const Point* y_points = &mono_y[y * 2];
            size_t count_x = path_geometry::chop_quad_at_x_extrema(y_points, mono_x.data());
            for (size_t x = 0; x <= count_x; ++x) {
                const Point* x_points = &mono_x[x * 2];
                clip_mono_quad(x_points, out);
            }
        }
    }

    return out.size() > start_len;
}

void EdgeClipper::clip_mono_cubic(const Point* src, std::vector<PathEdge>& out) {
    std::array<Point, 4> pts{};
    bool reverse = sort_increasing_y(src, pts.data(), 4);

    if (pts[3].y <= clip_.top() || pts[0].y >= clip_.bottom()) {
        return;
    }

    chop_cubic_in_y(clip_, pts.data());

    if (pts[0].x > pts[3].x) {
        std::swap(pts[0], pts[3]);
        std::swap(pts[1], pts[2]);
        reverse = !reverse;
    }

    if (pts[3].x <= clip_.left()) {
        push_vline(clip_.left(), pts[0].y, pts[3].y, reverse, out);
        return;
    }

    if (pts[0].x >= clip_.right()) {
        if (!can_cull_to_the_right_) {
            push_vline(clip_.right(), pts[0].y, pts[3].y, reverse, out);
        }
        return;
    }

    if (pts[0].x < clip_.left()) {
        std::array<Point, 7> tmp{};
        chop_mono_cubic_at_x(pts.data(), clip_.left(), tmp.data());
        push_vline(clip_.left(), tmp[0].y, tmp[3].y, reverse, out);

        tmp[3] = Point(clip_.left(), tmp[3].y);
        tmp[4] = Point(std::max(tmp[4].x, clip_.left()), tmp[4].y);

        pts[0] = tmp[3];
        pts[1] = tmp[4];
        pts[2] = tmp[5];
    }

    if (pts[3].x > clip_.right()) {
        std::array<Point, 7> tmp{};
        chop_mono_cubic_at_x(pts.data(), clip_.right(), tmp.data());
        tmp[3] = Point(clip_.right(), tmp[3].y);
        tmp[2] = Point(std::min(tmp[2].x, clip_.right()), tmp[2].y);

        push_cubic(tmp.data(), reverse, out);
        push_vline(clip_.right(), tmp[3].y, tmp[6].y, reverse, out);
    } else {
        push_cubic(pts.data(), reverse, out);
    }
}

bool EdgeClipper::clip_cubic(Point p0, Point p1, Point p2, Point p3, std::vector<PathEdge>& out) {
    size_t start_len = out.size();
    std::array<Point, 4> pts = {p0, p1, p2, p3};
    auto bounds = Rect::from_points(pts.data(), 4);
    if (!bounds) return false;

    if (bounds->bottom() > clip_.top() && bounds->top() < clip_.bottom()) {
        if (too_big_for_reliable_float_math(*bounds)) {
            return clip_line(p0, p3, out);
        } else {
            std::array<Point, 10> mono_y{};
            size_t count_y = path_geometry::chop_cubic_at_y_extrema(pts.data(), mono_y.data());
            for (size_t y = 0; y <= count_y; ++y) {
                std::array<Point, 10> mono_x{};
                const Point* y_points = &mono_y[y * 3];
                size_t count_x = path_geometry::chop_cubic_at_x_extrema(y_points, mono_x.data());
                for (size_t x = 0; x <= count_x; ++x) {
                    const Point* x_points = &mono_x[x * 3];
                    clip_mono_cubic(x_points, out);
                }
            }
        }
    }

    return out.size() > start_len;
}

std::optional<PathEdge> EdgeClipperIter::next() {
    while (true) {
        if (current_idx_ < current_edges_.size()) {
            return current_edges_[current_idx_++];
        }

        current_edges_.clear();
        current_idx_ = 0;

        auto next_edge = edge_iter_.next();
        if (!next_edge) {
            return std::nullopt;
        }

        switch (next_edge->type) {
            case PathEdge::Type::LineTo:
                clipper_.clip_line(next_edge->points[0], next_edge->points[1], current_edges_);
                break;
            case PathEdge::Type::QuadTo:
                clipper_.clip_quad(next_edge->points[0], next_edge->points[1], next_edge->points[2], current_edges_);
                break;
            case PathEdge::Type::CubicTo:
                clipper_.clip_cubic(next_edge->points[0], next_edge->points[1], next_edge->points[2], next_edge->points[3], current_edges_);
                break;
        }
    }
}

} // namespace nisaba

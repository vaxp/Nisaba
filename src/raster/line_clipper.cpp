#include "nisaba/raster/line_clipper.hpp"
#include "nisaba/math/scalar.hpp"
#include <algorithm>
#include <cassert>

namespace nisaba {
namespace line_clipper {

namespace {

inline bool is_between_unsorted(float value, float limit0, float limit1) noexcept {
    if (limit0 < limit1) {
        return limit0 <= value && value <= limit1;
    } else {
        return limit1 <= value && value <= limit0;
    }
}

inline float pin_unsorted_f32(float value, float limit0, float limit1) noexcept {
    if (limit1 < limit0) std::swap(limit0, limit1);
    if (value < limit0) return limit0;
    if (value > limit1) return limit1;
    return value;
}

inline double pin_unsorted_f64(double value, double limit0, double limit1) noexcept {
    if (limit1 < limit0) std::swap(limit0, limit1);
    if (value < limit0) return limit0;
    if (value > limit1) return limit1;
    return value;
}

inline float sect_with_horizontal(const std::array<Point, 2>& src, float y) noexcept {
    float dy = src[1].y - src[0].y;
    if (scalar::is_nearly_zero(dy)) {
        return scalar::ave(src[0].x, src[1].x);
    } else {
        double x0 = static_cast<double>(src[0].x);
        double y0 = static_cast<double>(src[0].y);
        double x1 = static_cast<double>(src[1].x);
        double y1 = static_cast<double>(src[1].y);
        double result = x0 + (static_cast<double>(y) - y0) * (x1 - x0) / (y1 - y0);
        return static_cast<float>(pin_unsorted_f64(result, x0, x1));
    }
}

inline float sect_with_vertical(const std::array<Point, 2>& src, float x) noexcept {
    float dx = src[1].x - src[0].x;
    if (scalar::is_nearly_zero(dx)) {
        return scalar::ave(src[0].y, src[1].y);
    } else {
        double x0 = static_cast<double>(src[0].x);
        double y0 = static_cast<double>(src[0].y);
        double x1 = static_cast<double>(src[1].x);
        double y1 = static_cast<double>(src[1].y);
        double result = y0 + (static_cast<double>(x) - x0) * (y1 - y0) / (x1 - x0);
        return static_cast<float>(result);
    }
}

inline float sect_clamp_with_vertical(const std::array<Point, 2>& src, float x) noexcept {
    float y = sect_with_vertical(src, x);
    return pin_unsorted_f32(y, src[0].y, src[1].y);
}

inline bool nested_lt(float a, float b, float dim) noexcept {
    return a <= b && (a < b || dim > 0.0f);
}

inline bool contains_no_empty_check(const Rect& outer, const Rect& inner) noexcept {
    return outer.left() <= inner.left() &&
           outer.top() <= inner.top() &&
           outer.right() >= inner.right() &&
           outer.bottom() >= inner.bottom();
}

} // namespace

size_t clip(
    const std::array<Point, 2>& src,
    const Rect& clip,
    bool can_cull_to_the_right,
    std::array<Point, MAX_POINTS>& points
) noexcept {
    size_t index0 = src[0].y < src[1].y ? 0 : 1;
    size_t index1 = src[0].y < src[1].y ? 1 : 0;

    // Check if we're completely clipped out in Y
    if (src[index1].y <= clip.top()) return 0;
    if (src[index0].y >= clip.bottom()) return 0;

    std::array<Point, 2> tmp = src;

    if (src[index0].y < clip.top()) {
        tmp[index0] = Point(sect_with_horizontal(src, clip.top()), clip.top());
        assert(is_between_unsorted(tmp[index0].x, src[0].x, src[1].x));
    }

    if (tmp[index1].y > clip.bottom()) {
        tmp[index1] = Point(sect_with_horizontal(src, clip.bottom()), clip.bottom());
        assert(is_between_unsorted(tmp[index1].x, src[0].x, src[1].x));
    }

    std::array<Point, MAX_POINTS> result_storage{};
    size_t line_count = 1;
    bool reverse = false;

    if (src[0].x < src[1].x) {
        index0 = 0;
        index1 = 1;
        reverse = false;
    } else {
        index0 = 1;
        index1 = 0;
        reverse = true;
    }

    const Point* result_pts = nullptr;

    if (tmp[index1].x <= clip.left()) {
        tmp[0] = Point(clip.left(), tmp[0].y);
        tmp[1] = Point(clip.left(), tmp[1].y);
        reverse = false;
        result_pts = tmp.data();
    } else if (tmp[index0].x >= clip.right()) {
        if (can_cull_to_the_right) return 0;
        tmp[0] = Point(clip.right(), tmp[0].y);
        tmp[1] = Point(clip.right(), tmp[1].y);
        reverse = false;
        result_pts = tmp.data();
    } else {
        size_t offset = 0;

        if (tmp[index0].x < clip.left()) {
            result_storage[offset] = Point(clip.left(), tmp[index0].y);
            offset += 1;
            result_storage[offset] = Point(clip.left(), sect_clamp_with_vertical(tmp, clip.left()));
            assert(is_between_unsorted(result_storage[offset].y, tmp[0].y, tmp[1].y));
        } else {
            result_storage[offset] = tmp[index0];
        }
        offset += 1;

        if (tmp[index1].x > clip.right()) {
            result_storage[offset] = Point(clip.right(), sect_clamp_with_vertical(tmp, clip.right()));
            assert(is_between_unsorted(result_storage[offset].y, tmp[0].y, tmp[1].y));
            offset += 1;
            result_storage[offset] = Point(clip.right(), tmp[index1].y);
        } else {
            result_storage[offset] = tmp[index1];
        }

        line_count = offset;
        result_pts = result_storage.data();
    }

    if (reverse) {
        for (size_t i = 0; i <= line_count; ++i) {
            points[line_count - i] = result_pts[i];
        }
    } else {
        for (size_t i = 0; i <= line_count; ++i) {
            points[i] = result_pts[i];
        }
    }

    return line_count + 1;
}

bool intersect(
    const std::array<Point, 2>& src,
    const Rect& clip,
    std::array<Point, 2>& dst
) noexcept {
    auto bounds = Rect::from_ltrb(
        std::min(src[0].x, src[1].x),
        std::min(src[0].y, src[1].y),
        std::max(src[0].x, src[1].x),
        std::max(src[0].y, src[1].y)
    );

    if (bounds) {
        if (contains_no_empty_check(clip, *bounds)) {
            dst = src;
            return true;
        }

        if (nested_lt(bounds->right(), clip.left(), bounds->width()) ||
            nested_lt(clip.right(), bounds->left(), bounds->width()) ||
            nested_lt(bounds->bottom(), clip.top(), bounds->height()) ||
            nested_lt(clip.bottom(), bounds->top(), bounds->height())) {
            return false;
        }
    }

    size_t index0 = src[0].y < src[1].y ? 0 : 1;
    size_t index1 = src[0].y < src[1].y ? 1 : 0;

    std::array<Point, 2> tmp = src;

    if (tmp[index0].y < clip.top()) {
        tmp[index0] = Point(sect_with_horizontal(src, clip.top()), clip.top());
    }
    if (tmp[index1].y > clip.bottom()) {
        tmp[index1] = Point(sect_with_horizontal(src, clip.bottom()), clip.bottom());
    }

    index0 = tmp[0].x < tmp[1].x ? 0 : 1;
    index1 = tmp[0].x < tmp[1].x ? 1 : 0;

    if (tmp[index1].x <= clip.left() || tmp[index0].x >= clip.right()) {
        if (tmp[0].x != tmp[1].x || tmp[0].x < clip.left() || tmp[0].x > clip.right()) {
            return false;
        }
    }

    if (tmp[index0].x < clip.left()) {
        tmp[index0] = Point(clip.left(), sect_with_vertical(src, clip.left()));
    }
    if (tmp[index1].x > clip.right()) {
        tmp[index1] = Point(clip.right(), sect_with_vertical(src, clip.right()));
    }

    dst = tmp;
    return true;
}

} // namespace line_clipper
} // namespace nisaba

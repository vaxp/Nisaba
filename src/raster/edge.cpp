#include "nisaba/raster/edge.hpp"
#include <algorithm>
#include <cassert>
#include <bit>

namespace nisaba {

namespace {

inline FDot6 compute_dy(FDot6 top, FDot6 y0) noexcept {
    return left_shift(top, 6) + 32 - y0;
}

inline int32_t cheap_distance(FDot6 dx, FDot6 dy) noexcept {
    dx = std::abs(dx);
    dy = std::abs(dy);
    return (dx > dy) ? (dx + (dy >> 1)) : (dy + (dx >> 1));
}

inline int32_t diff_to_shift(FDot6 dx, FDot6 dy, int32_t shift_aa) noexcept {
    int32_t dist = cheap_distance(dx, dy);
    dist = (dist + (1 << (2 + shift_aa))) >> (3 + shift_aa);
    int32_t lz = (dist == 0) ? 32 : std::countl_zero(static_cast<uint32_t>(dist));
    return (32 - lz) >> 1;
}

inline FDot16 fdot6_to_fixed_div2(FDot6 value) noexcept {
    return left_shift(value, 16 - 6 - 1);
}

inline int32_t fdot6_up_shift(FDot6 x, int32_t up_shift) noexcept {
    assert((left_shift(x, up_shift) >> up_shift) == x);
    return left_shift(x, up_shift);
}

inline FDot6 cubic_delta_from_line(FDot6 a, FDot6 b, FDot6 c, FDot6 d) noexcept {
    int32_t one_third = ((a * 8 - b * 15 + 6 * c + d) * 19) >> 9;
    int32_t two_third = ((a + 6 * b - c * 15 + d * 8) * 19) >> 9;
    return std::max(std::abs(one_third), std::abs(two_third));
}

} // namespace

std::optional<LineEdge> LineEdge::create(Point p0, Point p1, int32_t shift) noexcept {
    float scale = static_cast<float>(1 << (shift + 6));
    int32_t x0 = static_cast<int32_t>(p0.x * scale);
    int32_t y0 = static_cast<int32_t>(p0.y * scale);
    int32_t x1 = static_cast<int32_t>(p1.x * scale);
    int32_t y1 = static_cast<int32_t>(p1.y * scale);

    int8_t winding = 1;
    if (y0 > y1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
        winding = -1;
    }

    int32_t top = fdot6::round(y0);
    int32_t bottom = fdot6::round(y1);

    if (top == bottom) return std::nullopt;

    int32_t slope = fdot6::div(x1 - x0, y1 - y0);
    int32_t dy = compute_dy(top, y0);

    LineEdge edge;
    edge.prev = 0;
    edge.next = 0;
    edge.x = fdot6::to_fdot16(x0 + fdot16::mul(slope, dy));
    edge.dx = slope;
    edge.first_y = top;
    edge.last_y = bottom - 1;
    edge.winding = winding;

    return edge;
}

bool LineEdge::update(FDot16 x0, FDot16 y0, FDot16 x1, FDot16 y1) noexcept {
    assert(winding == 1 || winding == -1);

    y0 >>= 10;
    y1 >>= 10;

    assert(y0 <= y1);

    int32_t top = fdot6::round(y0);
    int32_t bottom = fdot6::round(y1);

    if (top == bottom) return false;

    x0 >>= 10;
    x1 >>= 10;

    int32_t slope = fdot6::div(x1 - x0, y1 - y0);
    int32_t dy = compute_dy(top, y0);

    x = fdot6::to_fdot16(x0 + fdot16::mul(slope, dy));
    dx = slope;
    first_y = top;
    last_y = bottom - 1;

    return true;
}

std::optional<QuadraticEdge> QuadraticEdge::create(const Point* points, int32_t shift) noexcept {
    float scale = static_cast<float>(1 << (shift + 6));
    int32_t x0 = static_cast<int32_t>(points[0].x * scale);
    int32_t y0 = static_cast<int32_t>(points[0].y * scale);
    int32_t x1 = static_cast<int32_t>(points[1].x * scale);
    int32_t y1 = static_cast<int32_t>(points[1].y * scale);
    int32_t x2 = static_cast<int32_t>(points[2].x * scale);
    int32_t y2 = static_cast<int32_t>(points[2].y * scale);

    int8_t winding = 1;
    if (y0 > y2) {
        std::swap(x0, x2);
        std::swap(y0, y2);
        winding = -1;
    }
    assert(y0 <= y1 && y1 <= y2);

    int32_t top = fdot6::round(y0);
    int32_t bottom = fdot6::round(y2);

    if (top == bottom) return std::nullopt;

    int32_t dx = (left_shift(x1, 1) - x0 - x2) >> 2;
    int32_t dy = (left_shift(y1, 1) - y0 - y2) >> 2;
    shift = diff_to_shift(dx, dy, shift);
    assert(shift >= 0);

    if (shift == 0) {
        shift = 1;
    } else if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    int8_t curve_count = static_cast<int8_t>(1 << shift);
    uint8_t curve_shift = static_cast<uint8_t>(shift - 1);

    int32_t a = fdot6_to_fixed_div2(x0 - x1 - x1 + x2);
    int32_t b = fdot6::to_fdot16(x1 - x0);

    FDot16 qx = fdot6::to_fdot16(x0);
    FDot16 qdx = b + (a >> shift);
    FDot16 qddx = a >> (shift - 1);

    a = fdot6_to_fixed_div2(y0 - y1 - y1 + y2);
    b = fdot6::to_fdot16(y1 - y0);

    FDot16 qy = fdot6::to_fdot16(y0);
    FDot16 qdy = b + (a >> shift);
    FDot16 qddy = a >> (shift - 1);

    FDot16 q_last_x = fdot6::to_fdot16(x2);
    FDot16 q_last_y = fdot6::to_fdot16(y2);

    QuadraticEdge quad;
    quad.line.prev = 0;
    quad.line.next = 0;
    quad.line.x = 0;
    quad.line.dx = 0;
    quad.line.first_y = 0;
    quad.line.last_y = 0;
    quad.line.winding = winding;
    quad.curve_count = curve_count;
    quad.curve_shift = curve_shift;
    quad.qx = qx;
    quad.qy = qy;
    quad.qdx = qdx;
    quad.qdy = qdy;
    quad.qddx = qddx;
    quad.qddy = qddy;
    quad.q_last_x = q_last_x;
    quad.q_last_y = q_last_y;

    if (quad.update()) {
        return quad;
    }
    return std::nullopt;
}

bool QuadraticEdge::update() noexcept {
    bool success = false;
    int8_t count = curve_count;
    FDot16 oldx = qx;
    FDot16 oldy = qy;
    FDot16 dx_val = qdx;
    FDot16 dy_val = qdy;
    FDot16 newx = 0;
    FDot16 newy = 0;
    uint8_t shift = curve_shift;

    assert(count > 0);

    while (true) {
        count -= 1;
        if (count > 0) {
            newx = oldx + (dx_val >> shift);
            dx_val += qddx;
            newy = oldy + (dy_val >> shift);
            dy_val += qddy;
        } else {
            newx = q_last_x;
            newy = q_last_y;
        }
        success = line.update(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;

        if (count == 0 || success) {
            break;
        }
    }

    qx = newx;
    qy = newy;
    qdx = dx_val;
    qdy = dy_val;
    curve_count = count;

    return success;
}

std::optional<CubicEdge> CubicEdge::create(const Point* points, int32_t shift) noexcept {
    float scale = static_cast<float>(1 << (shift + 6));
    int32_t x0 = static_cast<int32_t>(points[0].x * scale);
    int32_t y0 = static_cast<int32_t>(points[0].y * scale);
    int32_t x1 = static_cast<int32_t>(points[1].x * scale);
    int32_t y1 = static_cast<int32_t>(points[1].y * scale);
    int32_t x2 = static_cast<int32_t>(points[2].x * scale);
    int32_t y2 = static_cast<int32_t>(points[2].y * scale);
    int32_t x3 = static_cast<int32_t>(points[3].x * scale);
    int32_t y3 = static_cast<int32_t>(points[3].y * scale);

    int8_t winding = 1;
    if (y0 > y3) {
        std::swap(x0, x3);
        std::swap(x1, x2);
        std::swap(y0, y3);
        std::swap(y1, y2);
        winding = -1;
    }

    int32_t top = fdot6::round(y0);
    int32_t bot = fdot6::round(y3);

    if (top == bot) return std::nullopt;

    int32_t dx = cubic_delta_from_line(x0, x1, x2, x3);
    int32_t dy = cubic_delta_from_line(y0, y1, y2, y3);
    shift = diff_to_shift(dx, dy, 2) + 1;

    assert(shift > 0);
    if (shift > MAX_COEFF_SHIFT) {
        shift = MAX_COEFF_SHIFT;
    }

    int32_t up_shift = 6;
    int32_t down_shift = shift + up_shift - 10;
    if (down_shift < 0) {
        down_shift = 0;
        up_shift = 10 - shift;
    }

    int8_t curve_count = static_cast<int8_t>(left_shift(-1, shift));
    uint8_t curve_shift = static_cast<uint8_t>(shift);
    uint8_t dshift = static_cast<uint8_t>(down_shift);

    int32_t b = fdot6_up_shift(3 * (x1 - x0), up_shift);
    int32_t c = fdot6_up_shift(3 * (x0 - x1 - x1 + x2), up_shift);
    int32_t d = fdot6_up_shift(x3 + 3 * (x1 - x2) - x0, up_shift);

    FDot16 cx = fdot6::to_fdot16(x0);
    FDot16 cdx = b + (c >> shift) + (d >> (2 * shift));
    FDot16 cddx = 2 * c + ((3 * d) >> (shift - 1));
    FDot16 cdddx = (3 * d) >> (shift - 1);

    b = fdot6_up_shift(3 * (y1 - y0), up_shift);
    c = fdot6_up_shift(3 * (y0 - y1 - y1 + y2), up_shift);
    d = fdot6_up_shift(y3 + 3 * (y1 - y2) - y0, up_shift);

    FDot16 cy = fdot6::to_fdot16(y0);
    FDot16 cdy = b + (c >> shift) + (d >> (2 * shift));
    FDot16 cddy = 2 * c + ((3 * d) >> (shift - 1));
    FDot16 cdddy = (3 * d) >> (shift - 1);

    FDot16 c_last_x = fdot6::to_fdot16(x3);
    FDot16 c_last_y = fdot6::to_fdot16(y3);

    CubicEdge cubic;
    cubic.line.prev = 0;
    cubic.line.next = 0;
    cubic.line.x = 0;
    cubic.line.dx = 0;
    cubic.line.first_y = 0;
    cubic.line.last_y = 0;
    cubic.line.winding = winding;
    cubic.curve_count = curve_count;
    cubic.curve_shift = curve_shift;
    cubic.dshift = dshift;
    cubic.cx = cx;
    cubic.cy = cy;
    cubic.cdx = cdx;
    cubic.cdy = cdy;
    cubic.cddx = cddx;
    cubic.cddy = cddy;
    cubic.cdddx = cdddx;
    cubic.cdddy = cdddy;
    cubic.c_last_x = c_last_x;
    cubic.c_last_y = c_last_y;

    if (cubic.update()) {
        return cubic;
    }
    return std::nullopt;
}

bool CubicEdge::update() noexcept {
    bool success = false;
    int8_t count = curve_count;
    FDot16 oldx = cx;
    FDot16 oldy = cy;
    FDot16 newx = 0;
    FDot16 newy = 0;
    uint8_t ddshift = curve_shift;
    uint8_t dshift_val = dshift;

    assert(count < 0);

    while (true) {
        count += 1;
        if (count < 0) {
            newx = oldx + (cdx >> dshift_val);
            cdx += cddx >> ddshift;
            cddx += cdddx;

            newy = oldy + (cdy >> dshift_val);
            cdy += cddy >> ddshift;
            cddy += cdddy;
        } else {
            newx = c_last_x;
            newy = c_last_y;
        }

        if (newy < oldy) {
            newy = oldy;
        }

        success = line.update(oldx, oldy, newx, newy);
        oldx = newx;
        oldy = newy;

        if (count == 0 || success) {
            break;
        }
    }

    cx = newx;
    cy = newy;
    curve_count = count;

    return success;
}

} // namespace nisaba

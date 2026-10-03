#include "nisaba/math/transform4x4.hpp"
#include <cmath>
#include <numbers>

namespace nisaba {

namespace {

constexpr float DEG_TO_RAD = std::numbers::pi_v<float> / 180.0f;

} // namespace

Transform4x4 Transform4x4::from_rotate_x(float angle_degrees) noexcept {
    float rad = angle_degrees * DEG_TO_RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    return Transform4x4(
        1.0f, 0.0f,  0.0f, 0.0f,
        0.0f,    c,    -s, 0.0f,
        0.0f,    s,     c, 0.0f,
        0.0f, 0.0f,  0.0f, 1.0f
    );
}

Transform4x4 Transform4x4::from_rotate_y(float angle_degrees) noexcept {
    float rad = angle_degrees * DEG_TO_RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    return Transform4x4(
           c, 0.0f,     s, 0.0f,
        0.0f, 1.0f,  0.0f, 0.0f,
          -s, 0.0f,     c, 0.0f,
        0.0f, 0.0f,  0.0f, 1.0f
    );
}

Transform4x4 Transform4x4::from_rotate_z(float angle_degrees) noexcept {
    float rad = angle_degrees * DEG_TO_RAD;
    float c = std::cos(rad);
    float s = std::sin(rad);
    return Transform4x4(
           c,   -s,  0.0f, 0.0f,
           s,    c,  0.0f, 0.0f,
        0.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 1.0f
    );
}

Transform4x4 Transform4x4::from_camera_orbit(
    float pivot_x, float pivot_y,
    float rot_x_deg, float rot_y_deg, float rot_z_deg,
    float perspective_distance
) noexcept {
    auto t_to_origin = from_translate(-pivot_x, -pivot_y, 0.0f);
    auto rx = from_rotate_x(rot_x_deg);
    auto ry = from_rotate_y(rot_y_deg);
    auto rz = from_rotate_z(rot_z_deg);
    auto p = from_perspective(perspective_distance);
    auto t_back = from_translate(pivot_x, pivot_y, 0.0f);

    // Compound transformation: T_back * P * Rz * Ry * Rx * T_to_origin
    return t_back * (p * (rz * (ry * (rx * t_to_origin))));
}

Transform4x4 Transform4x4::operator*(const Transform4x4& o) const noexcept {
    Transform4x4 res;
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            res.m[r * 4 + c] =
                m[r * 4 + 0] * o.m[0 * 4 + c] +
                m[r * 4 + 1] * o.m[1 * 4 + c] +
                m[r * 4 + 2] * o.m[2 * 4 + c] +
                m[r * 4 + 3] * o.m[3 * 4 + c];
        }
    }
    return res;
}

std::optional<Transform4x4> Transform4x4::invert() const noexcept {
    float s0 = m[0] * m[5] - m[4] * m[1];
    float s1 = m[0] * m[6] - m[4] * m[2];
    float s2 = m[0] * m[7] - m[4] * m[3];
    float s3 = m[1] * m[6] - m[5] * m[2];
    float s4 = m[1] * m[7] - m[5] * m[3];
    float s5 = m[2] * m[7] - m[6] * m[3];

    float c5 = m[10] * m[15] - m[14] * m[11];
    float c4 = m[9]  * m[15] - m[13] * m[11];
    float c3 = m[9]  * m[14] - m[13] * m[10];
    float c2 = m[8]  * m[15] - m[12] * m[11];
    float c1 = m[8]  * m[14] - m[12] * m[10];
    float c0 = m[8]  * m[13] - m[12] * m[9];

    float det = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    if (std::abs(det) < 1e-12f) {
        return std::nullopt;
    }

    float inv_det = 1.0f / det;

    Transform4x4 b;
    b.m[0]  = ( m[5] * c5 - m[6] * c4 + m[7] * c3) * inv_det;
    b.m[1]  = (-m[1] * c5 + m[2] * c4 - m[3] * c3) * inv_det;
    b.m[2]  = ( m[13] * s5 - m[14] * s4 + m[15] * s3) * inv_det;
    b.m[3]  = (-m[9]  * s5 + m[10] * s4 - m[11] * s3) * inv_det;

    b.m[4]  = (-m[4] * c5 + m[6] * c2 - m[7] * c1) * inv_det;
    b.m[5]  = ( m[0] * c5 - m[2] * c2 + m[3] * c1) * inv_det;
    b.m[6]  = (-m[12] * s5 + m[14] * s2 - m[15] * s1) * inv_det;
    b.m[7]  = ( m[8]  * s5 - m[10] * s2 + m[11] * s1) * inv_det;

    b.m[8]  = ( m[4] * c4 - m[5] * c2 + m[7] * c0) * inv_det;
    b.m[9]  = (-m[0] * c4 + m[1] * c2 - m[3] * c0) * inv_det;
    b.m[10] = ( m[12] * s4 - m[13] * s2 + m[15] * s0) * inv_det;
    b.m[11] = (-m[8]  * s4 + m[9]  * s2 - m[11] * s0) * inv_det;

    b.m[12] = (-m[4] * c3 + m[5] * c1 - m[6] * c0) * inv_det;
    b.m[13] = ( m[0] * c3 - m[1] * c1 + m[2] * c0) * inv_det;
    b.m[14] = (-m[12] * s3 + m[13] * s1 - m[14] * s0) * inv_det;
    b.m[15] = ( m[8]  * s3 - m[9]  * s1 + m[10] * s0) * inv_det;

    return b;
}

Point Transform4x4::map_point(Point p) const noexcept {
    float x = m[0] * p.x + m[1] * p.y + m[3];
    float y = m[4] * p.x + m[5] * p.y + m[7];
    float w = m[12] * p.x + m[13] * p.y + m[15];

    if (std::abs(w) > 1e-7f) {
        float inv_w = 1.0f / w;
        return Point(x * inv_w, y * inv_w);
    }
    return Point(x, y);
}

std::optional<Point> Transform4x4::map_point_checked(Point p) const noexcept {
    float w = m[12] * p.x + m[13] * p.y + m[15];
    if (w <= 1e-6f) {
        return std::nullopt;
    }
    float x = m[0] * p.x + m[1] * p.y + m[3];
    float y = m[4] * p.x + m[5] * p.y + m[7];
    float inv_w = 1.0f / w;
    return Point(x * inv_w, y * inv_w);
}

Point Transform4x4::map_point3d(float px, float py, float pz) const noexcept {
    float x = m[0] * px + m[1] * py + m[2] * pz + m[3];
    float y = m[4] * px + m[5] * py + m[6] * pz + m[7];
    float w = m[12] * px + m[13] * py + m[14] * pz + m[15];

    if (std::abs(w) > 1e-7f) {
        float inv_w = 1.0f / w;
        return Point(x * inv_w, y * inv_w);
    }
    return Point(x, y);
}

std::optional<Transform4x4> Transform4x4::from_rect_to_quad(
    const Rect& src,
    Point p0, Point p1, Point p2, Point p3
) noexcept {
    float w = src.width();
    float h = src.height();
    if (w <= 0.0f || h <= 0.0f) {
        return std::nullopt;
    }

    float x0 = p0.x, y0 = p0.y;
    float x1 = p1.x, y1 = p1.y;
    float x2 = p2.x, y2 = p2.y;
    float x3 = p3.x, y3 = p3.y;

    float dx1 = x1 - x2;
    float dx2 = x3 - x2;
    float sx = x0 - x1 + x2 - x3;
    float dy1 = y1 - y2;
    float dy2 = y3 - y2;
    float sy = y0 - y1 + y2 - y3;

    float a11 = 0.0f, a12 = 0.0f, a13 = x0;
    float a21 = 0.0f, a22 = 0.0f, a23 = y0;
    float a31 = 0.0f, a32 = 0.0f, a33 = 1.0f;

    if (std::abs(sx) < 1e-6f && std::abs(sy) < 1e-6f) {
        // Affine mapping (parallelogram)
        a11 = x1 - x0;
        a12 = x3 - x0;
        a21 = y1 - y0;
        a22 = y3 - y0;
    } else {
        // Full projective mapping
        float det = dx1 * dy2 - dx2 * dy1;
        if (std::abs(det) < 1e-9f) {
            return std::nullopt;
        }

        a31 = (sx * dy2 - sy * dx2) / det;
        a32 = (dx1 * sy - dy1 * sx) / det;
        a11 = x1 - x0 + a31 * x1;
        a12 = x3 - x0 + a32 * x3;
        a21 = y1 - y0 + a31 * y1;
        a22 = y3 - y0 + a32 * y3;
    }

    // Now compose with map from src rect to unit square:
    // u' = (u - u0) / w, v' = (v - v0) / h
    float u0 = src.left();
    float v0 = src.top();

    float h00 = a11 / w;
    float h01 = a12 / h;
    float h02 = a13 - (a11 * u0 / w) - (a12 * v0 / h);

    float h10 = a21 / w;
    float h11 = a22 / h;
    float h12 = a23 - (a21 * u0 / w) - (a22 * v0 / h);

    float h20 = a31 / w;
    float h21 = a32 / h;
    float h22 = a33 - (a31 * u0 / w) - (a32 * v0 / h);

    return Transform4x4(
        h00,  h01,  0.0f, h02,
        h10,  h11,  0.0f, h12,
        0.0f, 0.0f, 1.0f, 0.0f,
        h20,  h21,  0.0f, h22
    );
}

} // namespace nisaba

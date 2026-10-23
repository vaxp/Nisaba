#pragma once

#include "orion/math/vec3.hpp"
#include "orion/math/quat.hpp"
#include <cstring>

namespace orion {

class alignas(16) Mat44 {
public:
    // Row-major: m[row * 4 + col]
    alignas(16) float m[16];

    // Constructors
    constexpr Mat44() noexcept : m{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    } {}

    constexpr explicit Mat44(const float values[16]) noexcept : m{
        values[0],  values[1],  values[2],  values[3],
        values[4],  values[5],  values[6],  values[7],
        values[8],  values[9],  values[10], values[11],
        values[12], values[13], values[14], values[15]
    } {}

    constexpr Mat44(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33
    ) noexcept : m{
        m00, m01, m02, m03,
        m10, m11, m12, m13,
        m20, m21, m22, m23,
        m30, m31, m32, m33
    } {}

    // Element access
    [[nodiscard]] constexpr float operator()(size_t row, size_t col) const noexcept {
        return m[row * 4 + col];
    }

    [[nodiscard]] constexpr float& operator()(size_t row, size_t col) noexcept {
        return m[row * 4 + col];
    }

    // Static factories
    [[nodiscard]] static constexpr Mat44 identity() noexcept {
        return Mat44();
    }

    [[nodiscard]] static constexpr Mat44 zeros() noexcept {
        return Mat44(
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f
        );
    }

    [[nodiscard]] static Mat44 translation(const Vec3& t) noexcept {
        return Mat44(
            1.0f, 0.0f, 0.0f, t.x,
            0.0f, 1.0f, 0.0f, t.y,
            0.0f, 0.0f, 1.0f, t.z,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    [[nodiscard]] static Mat44 scaling(const Vec3& s) noexcept {
        return Mat44(
            s.x,  0.0f, 0.0f, 0.0f,
            0.0f, s.y,  0.0f, 0.0f,
            0.0f, 0.0f, s.z,  0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    [[nodiscard]] static Mat44 rotation(const Quat& q) noexcept {
        const float xx = q.x * q.x;
        const float yy = q.y * q.y;
        const float zz = q.z * q.z;
        const float xy = q.x * q.y;
        const float xz = q.x * q.z;
        const float yz = q.y * q.z;
        const float wx = q.w * q.x;
        const float wy = q.w * q.y;
        const float wz = q.w * q.z;

        return Mat44(
            1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz),        2.0f * (xz + wy),        0.0f,
            2.0f * (xy + wz),        1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx),        0.0f,
            2.0f * (xz - wy),        2.0f * (yz + wx),        1.0f - 2.0f * (xx + yy), 0.0f,
            0.0f,                    0.0f,                    0.0f,                    1.0f
        );
    }

    [[nodiscard]] static Mat44 transform(const Vec3& translation, const Quat& rotation, const Vec3& scale = Vec3::one()) noexcept {
        Mat44 rot = Mat44::rotation(rotation);

        // Apply scale to columns
        rot.m[0]  *= scale.x; rot.m[1]  *= scale.y; rot.m[2]  *= scale.z;
        rot.m[4]  *= scale.x; rot.m[5]  *= scale.y; rot.m[6]  *= scale.z;
        rot.m[8]  *= scale.x; rot.m[9]  *= scale.y; rot.m[10] *= scale.z;

        // Apply translation
        rot.m[3]  = translation.x;
        rot.m[7]  = translation.y;
        rot.m[11] = translation.z;

        return rot;
    }

    // Matrix multiplication (SIMD accelerated on SSE/NEON)
    [[nodiscard]] Mat44 operator*(const Mat44& rhs) const noexcept {
#if defined(NISABA_SIMD_SSE)
        Mat44 out;
        const __m128 b0 = _mm_load_ps(&rhs.m[0]);
        const __m128 b1 = _mm_load_ps(&rhs.m[4]);
        const __m128 b2 = _mm_load_ps(&rhs.m[8]);
        const __m128 b3 = _mm_load_ps(&rhs.m[12]);

        for (int i = 0; i < 4; ++i) {
            const __m128 a0 = _mm_set1_ps(m[i * 4 + 0]);
            const __m128 a1 = _mm_set1_ps(m[i * 4 + 1]);
            const __m128 a2 = _mm_set1_ps(m[i * 4 + 2]);
            const __m128 a3 = _mm_set1_ps(m[i * 4 + 3]);

            const __m128 row = _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(a0, b0), _mm_mul_ps(a1, b1)),
                _mm_add_ps(_mm_mul_ps(a2, b2), _mm_mul_ps(a3, b3))
            );
            _mm_store_ps(&out.m[i * 4], row);
        }
        return out;
#elif defined(NISABA_SIMD_NEON)
        Mat44 out;
        const float32x4_t b0 = vld1q_f32(&rhs.m[0]);
        const float32x4_t b1 = vld1q_f32(&rhs.m[4]);
        const float32x4_t b2 = vld1q_f32(&rhs.m[8]);
        const float32x4_t b3 = vld1q_f32(&rhs.m[12]);

        for (int i = 0; i < 4; ++i) {
            float32x4_t row = vmulq_n_f32(b0, m[i * 4 + 0]);
            row = vmlaq_n_f32(row, b1, m[i * 4 + 1]);
            row = vmlaq_n_f32(row, b2, m[i * 4 + 2]);
            row = vmlaq_n_f32(row, b3, m[i * 4 + 3]);
            vst1q_f32(&out.m[i * 4], row);
        }
        return out;
#else
        Mat44 out;
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                out.m[r * 4 + c] =
                    m[r * 4 + 0] * rhs.m[0 * 4 + c] +
                    m[r * 4 + 1] * rhs.m[1 * 4 + c] +
                    m[r * 4 + 2] * rhs.m[2 * 4 + c] +
                    m[r * 4 + 3] * rhs.m[3 * 4 + c];
            }
        }
        return out;
#endif
    }

    // Affine transformation of points (w = 1)
    [[nodiscard]] Vec3 transform_point(const Vec3& p) const noexcept {
        const float rx = m[0] * p.x + m[1] * p.y + m[2] * p.z + m[3];
        const float ry = m[4] * p.x + m[5] * p.y + m[6] * p.z + m[7];
        const float rz = m[8] * p.x + m[9] * p.y + m[10] * p.z + m[11];
        const float rw = m[12] * p.x + m[13] * p.y + m[14] * p.z + m[15];

        if (std::abs(rw - 1.0f) > EPSILON && std::abs(rw) > EPSILON) {
            const float inv_w = 1.0f / rw;
            return Vec3(rx * inv_w, ry * inv_w, rz * inv_w);
        }
        return Vec3(rx, ry, rz);
    }

    // Direction vector transformation (w = 0, ignores translation)
    [[nodiscard]] Vec3 transform_vector(const Vec3& v) const noexcept {
        return Vec3(
            m[0] * v.x + m[1] * v.y + m[2] * v.z,
            m[4] * v.x + m[5] * v.y + m[6] * v.z,
            m[8] * v.x + m[9] * v.y + m[10] * v.z
        );
    }

    // Transpose
    [[nodiscard]] Mat44 transpose() const noexcept {
        return Mat44(
            m[0], m[4], m[8],  m[12],
            m[1], m[5], m[9],  m[13],
            m[2], m[6], m[10], m[14],
            m[3], m[7], m[11], m[15]
        );
    }

    // Determinant
    [[nodiscard]] float determinant() const noexcept {
        const float s0 = m[0] * m[5] - m[1] * m[4];
        const float s1 = m[0] * m[6] - m[2] * m[4];
        const float s2 = m[0] * m[7] - m[3] * m[4];
        const float s3 = m[1] * m[6] - m[2] * m[5];
        const float s4 = m[1] * m[7] - m[3] * m[5];
        const float s5 = m[2] * m[7] - m[3] * m[6];

        const float c5 = m[10] * m[15] - m[11] * m[14];
        const float c4 = m[9]  * m[15] - m[11] * m[13];
        const float c3 = m[9]  * m[14] - m[10] * m[13];
        const float c2 = m[8]  * m[15] - m[11] * m[12];
        const float c1 = m[8]  * m[14] - m[10] * m[12];
        const float c0 = m[8]  * m[13] - m[9]  * m[12];

        return s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
    }

    // General Inverse
    [[nodiscard]] Mat44 inverse(bool& success) const noexcept {
        const float det = determinant();
        if (std::abs(det) < EPSILON) {
            success = false;
            return identity();
        }

        const float inv_det = 1.0f / det;
        Mat44 inv;

        inv.m[0]  = (m[5] * (m[10] * m[15] - m[11] * m[14]) - m[9] * (m[6] * m[15] - m[7] * m[14]) + m[13] * (m[6] * m[11] - m[7] * m[10])) * inv_det;
        inv.m[1]  = -(m[1] * (m[10] * m[15] - m[11] * m[14]) - m[9] * (m[2] * m[15] - m[3] * m[14]) + m[13] * (m[2] * m[11] - m[3] * m[10])) * inv_det;
        inv.m[2]  = (m[1] * (m[6] * m[15] - m[7] * m[14]) - m[5] * (m[2] * m[15] - m[3] * m[14]) + m[13] * (m[2] * m[7] - m[3] * m[6])) * inv_det;
        inv.m[3]  = -(m[1] * (m[6] * m[11] - m[7] * m[10]) - m[5] * (m[2] * m[11] - m[3] * m[10]) + m[9] * (m[2] * m[7] - m[3] * m[6])) * inv_det;

        inv.m[4]  = -(m[4] * (m[10] * m[15] - m[11] * m[14]) - m[8] * (m[6] * m[15] - m[7] * m[14]) + m[12] * (m[6] * m[11] - m[7] * m[10])) * inv_det;
        inv.m[5]  = (m[0] * (m[10] * m[15] - m[11] * m[14]) - m[8] * (m[2] * m[15] - m[3] * m[14]) + m[12] * (m[2] * m[11] - m[3] * m[10])) * inv_det;
        inv.m[6]  = -(m[0] * (m[6] * m[15] - m[7] * m[14]) - m[4] * (m[2] * m[15] - m[3] * m[14]) + m[12] * (m[2] * m[7] - m[3] * m[6])) * inv_det;
        inv.m[7]  = (m[0] * (m[6] * m[11] - m[7] * m[10]) - m[4] * (m[2] * m[11] - m[3] * m[10]) + m[8] * (m[2] * m[7] - m[3] * m[6])) * inv_det;

        inv.m[8]  = (m[4] * (m[9] * m[15] - m[11] * m[13]) - m[8] * (m[5] * m[15] - m[7] * m[13]) + m[12] * (m[5] * m[11] - m[7] * m[9])) * inv_det;
        inv.m[9]  = -(m[0] * (m[9] * m[15] - m[11] * m[13]) - m[8] * (m[1] * m[15] - m[3] * m[13]) + m[12] * (m[1] * m[11] - m[3] * m[9])) * inv_det;
        inv.m[10] = (m[0] * (m[5] * m[15] - m[7] * m[13]) - m[4] * (m[1] * m[15] - m[3] * m[13]) + m[12] * (m[1] * m[7] - m[3] * m[5])) * inv_det;
        inv.m[11] = -(m[0] * (m[5] * m[11] - m[7] * m[9]) - m[4] * (m[1] * m[11] - m[3] * m[9]) + m[8] * (m[1] * m[7] - m[3] * m[5])) * inv_det;

        inv.m[12] = -(m[4] * (m[9] * m[14] - m[10] * m[13]) - m[8] * (m[5] * m[14] - m[6] * m[13]) + m[12] * (m[5] * m[10] - m[6] * m[9])) * inv_det;
        inv.m[13] = (m[0] * (m[9] * m[14] - m[10] * m[13]) - m[8] * (m[1] * m[14] - m[2] * m[13]) + m[12] * (m[1] * m[10] - m[2] * m[9])) * inv_det;
        inv.m[14] = -(m[0] * (m[5] * m[14] - m[6] * m[13]) - m[4] * (m[1] * m[14] - m[2] * m[13]) + m[12] * (m[1] * m[6] - m[2] * m[5])) * inv_det;
        inv.m[15] = (m[0] * (m[5] * m[10] - m[6] * m[9]) - m[4] * (m[1] * m[10] - m[2] * m[9]) + m[8] * (m[1] * m[6] - m[2] * m[5])) * inv_det;

        success = true;
        return inv;
    }

    // Fast inverse for rigid-body transforms (Orthogonal 3x3 rotation + translation)
    // T^-1 = [ R^T  -R^T * t ]
    //        [ 0         1   ]
    [[nodiscard]] Mat44 inverse_fast_rigid() const noexcept {
        // Transposed upper-left 3x3
        const float r00 = m[0], r01 = m[4], r02 = m[8];
        const float r10 = m[1], r11 = m[5], r12 = m[9];
        const float r20 = m[2], r21 = m[6], r22 = m[10];

        // Translation vector
        const float tx = m[3], ty = m[7], tz = m[11];

        // -R^T * t
        const float inv_tx = -(r00 * tx + r01 * ty + r02 * tz);
        const float inv_ty = -(r10 * tx + r11 * ty + r12 * tz);
        const float inv_tz = -(r20 * tx + r21 * ty + r22 * tz);

        return Mat44(
            r00, r01, r02, inv_tx,
            r10, r11, r12, inv_ty,
            r20, r21, r22, inv_tz,
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    [[nodiscard]] Vec3 get_translation() const noexcept {
        return Vec3(m[3], m[7], m[11]);
    }

    [[nodiscard]] static Mat44 look_at(const Vec3& eye, const Vec3& target, const Vec3& up) noexcept {
        const Vec3 f = (target - eye).normalized();
        const Vec3 s = f.cross(up).normalized();
        const Vec3 u = s.cross(f);

        return Mat44(
            s.x,  s.y,  s.z,  -s.dot(eye),
            u.x,  u.y,  u.z,  -u.dot(eye),
            -f.x, -f.y, -f.z, f.dot(eye),
            0.0f, 0.0f, 0.0f, 1.0f
        );
    }

    [[nodiscard]] static Mat44 perspective(float fov_y_rad, float aspect, float z_near, float z_far) noexcept {
        const float tan_half_fov = std::tan(fov_y_rad * 0.5f);
        const float f = 1.0f / tan_half_fov;

        Mat44 res = Mat44::zeros();
        res.m[0]  = f / aspect;
        res.m[5]  = f;
        res.m[10] = -(z_far + z_near) / (z_far - z_near);
        res.m[11] = -(2.0f * z_far * z_near) / (z_far - z_near);
        res.m[14] = -1.0f;
        return res;
    }

    [[nodiscard]] static Mat44 orthographic(float left, float right, float bottom, float top, float z_near, float z_far) noexcept {
        const float dx = right - left;
        const float dy = top - bottom;
        const float dz = z_far - z_near;

        Mat44 res = Mat44::zeros();
        res.m[0]  = (dx > 1e-6f) ? (2.0f / dx) : 1.0f;
        res.m[3]  = (dx > 1e-6f) ? (-(right + left) / dx) : 0.0f;
        res.m[5]  = (dy > 1e-6f) ? (2.0f / dy) : 1.0f;
        res.m[7]  = (dy > 1e-6f) ? (-(top + bottom) / dy) : 0.0f;
        res.m[10] = (dz > 1e-6f) ? (1.0f / dz) : 0.0f;
        res.m[11] = (dz > 1e-6f) ? (-z_near / dz) : 0.0f;
        res.m[15] = 1.0f;
        return res;
    }
};

} // namespace orion

namespace orion::physics { using orion::Mat44; }

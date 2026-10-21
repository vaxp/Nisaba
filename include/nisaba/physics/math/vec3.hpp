#pragma once

#include "nisaba/physics/math/simd_math.hpp"
#include <iostream>

namespace nisaba::physics {

class alignas(16) Vec3 {
public:
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
    union {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        __m128 m_simd;
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        float32x4_t m_simd;
#endif
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        float v[4];
    };
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

    // Constructors
    constexpr Vec3() noexcept : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    constexpr Vec3(float in_x, float in_y, float in_z, float in_w = 0.0f) noexcept : x(in_x), y(in_y), z(in_z), w(in_w) {}
    constexpr explicit Vec3(float splat) noexcept : x(splat), y(splat), z(splat), w(0.0f) {}

#if defined(NISABA_PHYSICS_SIMD_SSE)
    Vec3(__m128 simd) noexcept : m_simd(simd) {}
#elif defined(NISABA_PHYSICS_SIMD_NEON)
    Vec3(float32x4_t simd) noexcept : m_simd(simd) {}
#endif

    // Element access
    [[nodiscard]] constexpr float operator[](size_t index) const noexcept {
        return v[index];
    }

    [[nodiscard]] constexpr float& operator[](size_t index) noexcept {
        return v[index];
    }

    // Static direction helpers
    [[nodiscard]] static constexpr Vec3 zero() noexcept { return Vec3(0.0f, 0.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 one() noexcept { return Vec3(1.0f, 1.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec3 unit_x() noexcept { return Vec3(1.0f, 0.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 unit_y() noexcept { return Vec3(0.0f, 1.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 unit_z() noexcept { return Vec3(0.0f, 0.0f, 1.0f); }
    [[nodiscard]] static constexpr Vec3 up() noexcept { return Vec3(0.0f, 1.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 down() noexcept { return Vec3(0.0f, -1.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 left() noexcept { return Vec3(-1.0f, 0.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 right() noexcept { return Vec3(1.0f, 0.0f, 0.0f); }
    [[nodiscard]] static constexpr Vec3 forward() noexcept { return Vec3(0.0f, 0.0f, -1.0f); }
    [[nodiscard]] static constexpr Vec3 back() noexcept { return Vec3(0.0f, 0.0f, 1.0f); }

    // Arithmetic operators
    [[nodiscard]] Vec3 operator+(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_add_ps(m_simd, rhs.m_simd));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vaddq_f32(m_simd, rhs.m_simd));
#else
        return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
#endif
    }

    Vec3& operator+=(const Vec3& rhs) noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        m_simd = _mm_add_ps(m_simd, rhs.m_simd);
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        m_simd = vaddq_f32(m_simd, rhs.m_simd);
#else
        x += rhs.x; y += rhs.y; z += rhs.z;
#endif
        return *this;
    }

    [[nodiscard]] Vec3 operator-(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_sub_ps(m_simd, rhs.m_simd));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vsubq_f32(m_simd, rhs.m_simd));
#else
        return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
#endif
    }

    Vec3& operator-=(const Vec3& rhs) noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        m_simd = _mm_sub_ps(m_simd, rhs.m_simd);
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        m_simd = vsubq_f32(m_simd, rhs.m_simd);
#else
        x -= rhs.x; y -= rhs.y; z -= rhs.z;
#endif
        return *this;
    }

    [[nodiscard]] Vec3 operator-() const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_sub_ps(_mm_setzero_ps(), m_simd));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vnegq_f32(m_simd));
#else
        return Vec3(-x, -y, -z);
#endif
    }

    [[nodiscard]] Vec3 operator*(float scalar) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_mul_ps(m_simd, _mm_set1_ps(scalar)));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vmulq_n_f32(m_simd, scalar));
#else
        return Vec3(x * scalar, y * scalar, z * scalar);
#endif
    }

    Vec3& operator*=(float scalar) noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        m_simd = _mm_mul_ps(m_simd, _mm_set1_ps(scalar));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        m_simd = vmulq_n_f32(m_simd, scalar);
#else
        x *= scalar; y *= scalar; z *= scalar;
#endif
        return *this;
    }

    [[nodiscard]] Vec3 operator*(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_mul_ps(m_simd, rhs.m_simd));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vmulq_f32(m_simd, rhs.m_simd));
#else
        return Vec3(x * rhs.x, y * rhs.y, z * rhs.z);
#endif
    }

    [[nodiscard]] Vec3 operator/(float scalar) const noexcept {
        const float inv = 1.0f / scalar;
        return *this * inv;
    }

    Vec3& operator/=(float scalar) noexcept {
        const float inv = 1.0f / scalar;
        return *this *= inv;
    }

    [[nodiscard]] bool approx_equal(const Vec3& rhs, float tol = EPSILON) const noexcept {
        return nisaba::physics::approx_equal(x, rhs.x, tol) &&
               nisaba::physics::approx_equal(y, rhs.y, tol) &&
               nisaba::physics::approx_equal(z, rhs.z, tol);
    }

    [[nodiscard]] bool operator==(const Vec3& rhs) const noexcept {
        return approx_equal(rhs);
    }

    [[nodiscard]] bool operator!=(const Vec3& rhs) const noexcept {
        return !(*this == rhs);
    }

    // Vector operations
    [[nodiscard]] float dot(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        __m128 mul = _mm_mul_ps(m_simd, rhs.m_simd);
        __m128 shuf = _mm_shuffle_ps(mul, mul, _MM_SHUFFLE(2, 3, 0, 1));
        __m128 sums = _mm_add_ps(mul, shuf);
        shuf = _mm_movehl_ps(shuf, sums);
        sums = _mm_add_ss(sums, shuf);
        return _mm_cvtss_f32(sums);
#else
        return x * rhs.x + y * rhs.y + z * rhs.z;
#endif
    }

    [[nodiscard]] Vec3 cross(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        // a_yzx
        __m128 a_yzx = _mm_shuffle_ps(m_simd, m_simd, _MM_SHUFFLE(3, 0, 2, 1));
        // b_zxy
        __m128 b_zxy = _mm_shuffle_ps(rhs.m_simd, rhs.m_simd, _MM_SHUFFLE(3, 1, 0, 2));
        // a_zxy
        __m128 a_zxy = _mm_shuffle_ps(m_simd, m_simd, _MM_SHUFFLE(3, 1, 0, 2));
        // b_yzx
        __m128 b_yzx = _mm_shuffle_ps(rhs.m_simd, rhs.m_simd, _MM_SHUFFLE(3, 0, 2, 1));
        return Vec3(_mm_sub_ps(_mm_mul_ps(a_yzx, b_zxy), _mm_mul_ps(a_zxy, b_yzx)));
#else
        return Vec3(
            y * rhs.z - z * rhs.y,
            z * rhs.x - x * rhs.z,
            x * rhs.y - y * rhs.x
        );
#endif
    }

    [[nodiscard]] float length_sq() const noexcept {
        return dot(*this);
    }

    [[nodiscard]] float length() const noexcept {
        return std::sqrt(length_sq());
    }

    [[nodiscard]] float inv_length() const noexcept {
        const float lsq = length_sq();
        if (lsq > EPSILON * EPSILON) {
            return 1.0f / std::sqrt(lsq);
        }
        return 0.0f;
    }

    [[nodiscard]] Vec3 normalized() const noexcept {
        const float inv_l = inv_length();
        return *this * inv_l;
    }

    Vec3& normalize() noexcept {
        const float inv_l = inv_length();
        *this *= inv_l;
        return *this;
    }

    [[nodiscard]] bool is_normalized(float epsilon = 1e-4f) const noexcept {
        return std::abs(length_sq() - 1.0f) <= epsilon;
    }

    [[nodiscard]] bool is_zero(float epsilon = EPSILON) const noexcept {
        return length_sq() <= epsilon * epsilon;
    }

    [[nodiscard]] Vec3 min(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_min_ps(m_simd, rhs.m_simd));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vminq_f32(m_simd, rhs.m_simd));
#else
        return Vec3(std::min(x, rhs.x), std::min(y, rhs.y), std::min(z, rhs.z));
#endif
    }

    [[nodiscard]] Vec3 max(const Vec3& rhs) const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        return Vec3(_mm_max_ps(m_simd, rhs.m_simd));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vmaxq_f32(m_simd, rhs.m_simd));
#else
        return Vec3(std::max(x, rhs.x), std::max(y, rhs.y), std::max(z, rhs.z));
#endif
    }

    [[nodiscard]] Vec3 abs() const noexcept {
#if defined(NISABA_PHYSICS_SIMD_SSE)
        __m128 mask = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff));
        return Vec3(_mm_and_ps(m_simd, mask));
#elif defined(NISABA_PHYSICS_SIMD_NEON)
        return Vec3(vabsq_f32(m_simd));
#else
        return Vec3(std::abs(x), std::abs(y), std::abs(z));
#endif
    }

    [[nodiscard]] Vec3 reflect(const Vec3& normal) const noexcept {
        return *this - normal * (2.0f * this->dot(normal));
    }

    [[nodiscard]] Vec3 project_onto(const Vec3& target) const noexcept {
        const float denom = target.dot(target);
        if (denom > EPSILON * EPSILON) {
            return target * (this->dot(target) / denom);
        }
        return Vec3::zero();
    }

    [[nodiscard]] static Vec3 lerp(const Vec3& a, const Vec3& b, float t) noexcept {
        return a + (b - a) * t;
    }

    [[nodiscard]] static float distance(const Vec3& a, const Vec3& b) noexcept {
        return (a - b).length();
    }

    [[nodiscard]] static float distance_sq(const Vec3& a, const Vec3& b) noexcept {
        return (a - b).length_sq();
    }

    /// @brief Computes an exact orthonormal basis (t1, t2) perpendicular to this unit vector.
    /// Uses Duff et al.'s branchless algorithm (zero square roots, zero branching).
    void get_orthonormal_basis(Vec3& out_t1, Vec3& out_t2) const noexcept {
        const float sign = std::copysign(1.0f, z);
        const float a = -1.0f / (sign + z);
        const float b = x * y * a;
        out_t1 = Vec3(1.0f + sign * x * x * a, sign * b, -sign * x);
        out_t2 = Vec3(b, sign + y * y * a, -y);
    }
};

[[nodiscard]] inline Vec3 operator*(float scalar, const Vec3& vec) noexcept {
    return vec * scalar;
}

inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
    os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

} // namespace nisaba::physics

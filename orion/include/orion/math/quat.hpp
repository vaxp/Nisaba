#pragma once

#include "orion/math/vec3.hpp"

namespace orion {

class alignas(16) Quat {
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
    constexpr Quat() noexcept : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
    constexpr Quat(float in_x, float in_y, float in_z, float in_w) noexcept : x(in_x), y(in_y), z(in_z), w(in_w) {}

#if defined(NISABA_PHYSICS_SIMD_SSE)
    Quat(__m128 simd) noexcept : m_simd(simd) {}
#elif defined(NISABA_PHYSICS_SIMD_NEON)
    Quat(float32x4_t simd) noexcept : m_simd(simd) {}
#endif

    // Static identity
    [[nodiscard]] static constexpr Quat identity() noexcept {
        return Quat(0.0f, 0.0f, 0.0f, 1.0f);
    }

    // Factory methods
    [[nodiscard]] static Quat from_axis_angle(const Vec3& axis, float angle_rad) noexcept {
        const float half_angle = angle_rad * 0.5f;
        const float s = std::sin(half_angle);
        const float c = std::cos(half_angle);
        const Vec3 norm_axis = axis.normalized();
        return Quat(norm_axis.x * s, norm_axis.y * s, norm_axis.z * s, c);
    }

    [[nodiscard]] static Quat from_euler(float pitch_rad, float yaw_rad, float roll_rad) noexcept {
        // Roll (X), Pitch (Y), Yaw (Z) or ZYX intrinsic rotation
        const float hp = pitch_rad * 0.5f;
        const float hy = yaw_rad * 0.5f;
        const float hr = roll_rad * 0.5f;

        const float sp = std::sin(hp);
        const float cp = std::cos(hp);
        const float sy = std::sin(hy);
        const float cy = std::cos(hy);
        const float sr = std::sin(hr);
        const float cr = std::cos(hr);

        return Quat(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
    }

    [[nodiscard]] static Quat from_two_vectors(const Vec3& u, const Vec3& v) noexcept {
        const float norm_u_norm_v = std::sqrt(u.length_sq() * v.length_sq());
        float real_part = norm_u_norm_v + u.dot(v);
        Vec3 w;

        if (real_part < 1e-6f * norm_u_norm_v) {
            // Vectors are pointing in opposite directions (180 deg)
            real_part = 0.0f;
            w = std::abs(u.x) > std::abs(u.z) ? Vec3(-u.y, u.x, 0.0f) : Vec3(0.0f, -u.z, u.y);
        } else {
            w = u.cross(v);
        }

        return Quat(w.x, w.y, w.z, real_part).normalized();
    }

    // Hamilton product: q1 * q2
    [[nodiscard]] Quat operator*(const Quat& rhs) const noexcept {
        return Quat(
            w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
            w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
            w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
            w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z
        );
    }

    Quat& operator*=(const Quat& rhs) noexcept {
        *this = *this * rhs;
        return *this;
    }

    [[nodiscard]] Quat operator*(float scalar) const noexcept {
        return Quat(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    [[nodiscard]] Quat operator+(const Quat& rhs) const noexcept {
        return Quat(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
    }

    [[nodiscard]] bool approx_equal(const Quat& rhs, float tol = EPSILON) const noexcept {
        const bool same_sign = orion::approx_equal(x, rhs.x, tol) &&
                               orion::approx_equal(y, rhs.y, tol) &&
                               orion::approx_equal(z, rhs.z, tol) &&
                               orion::approx_equal(w, rhs.w, tol);
        const bool opp_sign = orion::approx_equal(x, -rhs.x, tol) &&
                              orion::approx_equal(y, -rhs.y, tol) &&
                              orion::approx_equal(z, -rhs.z, tol) &&
                              orion::approx_equal(w, -rhs.w, tol);
        return same_sign || opp_sign;
    }

    [[nodiscard]] bool operator==(const Quat& rhs) const noexcept {
        return approx_equal(rhs);
    }

    [[nodiscard]] bool operator!=(const Quat& rhs) const noexcept {
        return !(*this == rhs);
    }

    // Conjugate & Inverse
    [[nodiscard]] Quat conjugate() const noexcept {
        return Quat(-x, -y, -z, w);
    }

    [[nodiscard]] Quat inverse() const noexcept {
        const float lsq = length_sq();
        if (lsq > EPSILON * EPSILON) {
            const float inv_lsq = 1.0f / lsq;
            return Quat(-x * inv_lsq, -y * inv_lsq, -z * inv_lsq, w * inv_lsq);
        }
        return identity();
    }

    [[nodiscard]] float dot(const Quat& rhs) const noexcept {
        return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w;
    }

    [[nodiscard]] float length_sq() const noexcept {
        return dot(*this);
    }

    [[nodiscard]] float length() const noexcept {
        return std::sqrt(length_sq());
    }

    [[nodiscard]] Quat normalized() const noexcept {
        const float lsq = length_sq();
        if (lsq > EPSILON * EPSILON) {
            const float inv_l = 1.0f / std::sqrt(lsq);
            return Quat(x * inv_l, y * inv_l, z * inv_l, w * inv_l);
        }
        return identity();
    }

    Quat& normalize() noexcept {
        *this = normalized();
        return *this;
    }

    // Vector rotation: Rodrigues formula
    // v' = v + 2 * q_v x (q_v x v + q_w * v)
    [[nodiscard]] Vec3 rotate_vec(const Vec3& vec) const noexcept {
        const Vec3 q_v(x, y, z);
        const Vec3 t = 2.0f * q_v.cross(vec);
        return vec + (w * t) + q_v.cross(t);
    }

    // Inverse vector rotation (equivalent to rotating by conjugate / inverse)
    [[nodiscard]] Vec3 inverse_rotate_vec(const Vec3& vec) const noexcept {
        const Vec3 q_v(-x, -y, -z);
        const Vec3 t = 2.0f * q_v.cross(vec);
        return vec + (w * t) + q_v.cross(t);
    }

    [[nodiscard]] Vec3 operator*(const Vec3& vec) const noexcept {
        return rotate_vec(vec);
    }

    // Spherical Linear Interpolation (SLERP)
    [[nodiscard]] static Quat slerp(const Quat& from, Quat to, float t) noexcept {
        float cos_theta = from.dot(to);

        // If negative dot, take opposite quaternion to choose shortest path
        if (cos_theta < 0.0f) {
            to = Quat(-to.x, -to.y, -to.z, -to.w);
            cos_theta = -cos_theta;
        }

        // If very close, use simple normalized linear interpolation (NLERP)
        if (cos_theta > 0.9995f) {
            return Quat(
                from.x + (to.x - from.x) * t,
                from.y + (to.y - from.y) * t,
                from.z + (to.z - from.z) * t,
                from.w + (to.w - from.w) * t
            ).normalized();
        }

        const float theta = std::acos(cos_theta);
        const float sin_theta = std::sin(theta);
        const float factor_from = std::sin((1.0f - t) * theta) / sin_theta;
        const float factor_to = std::sin(t * theta) / sin_theta;

        return Quat(
            from.x * factor_from + to.x * factor_to,
            from.y * factor_from + to.y * factor_to,
            from.z * factor_from + to.z * factor_to,
            from.w * factor_from + to.w * factor_to
        ).normalized();
    }

    // Exact exponential map integration of angular velocity over time step dt
    // Handles large dt and high angular speeds with zero truncation drift
    [[nodiscard]] Quat integrate(const Vec3& angular_velocity, float dt) const noexcept {
        const float w_len = angular_velocity.length();
        if (w_len > EPSILON) {
            const float angle = w_len * dt;
            const Quat delta_q = Quat::from_axis_angle(angular_velocity * (1.0f / w_len), angle);
            return (delta_q * (*this)).normalized();
        }
        return *this;
    }

    void to_euler(float& out_pitch, float& out_yaw, float& out_roll) const noexcept {
        // Roll (x-axis rotation)
        const float sinr_cosp = 2.0f * (w * x + y * z);
        const float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
        out_roll = std::atan2(sinr_cosp, cosr_cosp);

        // Pitch (y-axis rotation)
        const float sinp = 2.0f * (w * y - z * x);
        if (std::abs(sinp) >= 1.0f) {
            out_pitch = std::copysign(HALF_PI, sinp); // use 90 degrees if out of range
        } else {
            out_pitch = std::asin(sinp);
        }

        // Yaw (z-axis rotation)
        const float siny_cosp = 2.0f * (w * z + x * y);
        const float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
        out_yaw = std::atan2(siny_cosp, cosy_cosp);
    }
};

inline std::ostream& operator<<(std::ostream& os, const Quat& q) {
    os << "Quat(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
    return os;
}

} // namespace orion

namespace orion::physics { using orion::Quat; }

#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/quat.hpp"
#include "nisaba/physics/math/mat44.hpp"

namespace nisaba::physics {

/// @brief Represents a 3D Rigid Body Transformation in SE(3) space (Translation + Rotation)
/// Size is exactly 32 bytes, aligned to 16 bytes for optimal cache-line density and SIMD performance.
class alignas(16) Transform3D {
public:
    Vec3 position;
    Quat rotation;

    constexpr Transform3D() noexcept
        : position(Vec3::zero()), rotation(Quat::identity()) {}

    constexpr Transform3D(const Vec3& in_pos, const Quat& in_rot) noexcept
        : position(in_pos), rotation(in_rot) {}

    [[nodiscard]] static constexpr Transform3D identity() noexcept {
        return Transform3D(Vec3::zero(), Quat::identity());
    }

    [[nodiscard]] static Transform3D from_translation(const Vec3& in_pos) noexcept {
        return Transform3D(in_pos, Quat::identity());
    }

    [[nodiscard]] static Transform3D from_rotation(const Quat& in_rot) noexcept {
        return Transform3D(Vec3::zero(), in_rot);
    }

    /// @brief Transform a local point into parent/world space: p' = R * p + t
    [[nodiscard]] Vec3 transform_point(const Vec3& p) const noexcept {
        return rotation.rotate_vec(p) + position;
    }

    /// @brief Transform a direction vector into parent/world space (ignores translation): v' = R * v
    [[nodiscard]] Vec3 transform_vector(const Vec3& v) const noexcept {
        return rotation.rotate_vec(v);
    }

    /// @brief Transform a world point back into local space: p = R^-1 * (p' - t)
    [[nodiscard]] Vec3 inverse_transform_point(const Vec3& p) const noexcept {
        return rotation.inverse_rotate_vec(p - position);
    }

    /// @brief Transform a world vector back into local space: v = R^-1 * v'
    [[nodiscard]] Vec3 inverse_transform_vector(const Vec3& v) const noexcept {
        return rotation.inverse_rotate_vec(v);
    }

    /// @brief Compose two transforms: T_composed = T_this * T_rhs
    /// Point transformation: T_this(T_rhs(p)) = R_this * (R_rhs * p + t_rhs) + t_this
    ///                       = (R_this * R_rhs) * p + (R_this * t_rhs + t_this)
    [[nodiscard]] Transform3D operator*(const Transform3D& rhs) const noexcept {
        return Transform3D(
            position + rotation.rotate_vec(rhs.position),
            (rotation * rhs.rotation).normalized()
        );
    }

    /// @brief Fast exact inverse of the rigid transformation: T^-1 = (R^-1 * (-t), R^-1)
    [[nodiscard]] Transform3D inverse() const noexcept {
        const Quat inv_rot = rotation.inverse();
        return Transform3D(inv_rot.rotate_vec(-position), inv_rot);
    }

    /// @brief Convert this rigid transform to a 4x4 matrix
    [[nodiscard]] Mat44 to_mat44() const noexcept {
        return Mat44::transform(position, rotation);
    }

    /// @brief Extract rigid position and rotation from a 4x4 matrix
    [[nodiscard]] static Transform3D from_mat44(const Mat44& m) noexcept {
        const Vec3 pos = m.get_translation();

        // Extract 3x3 rotation columns and normalize them to remove scale
        Vec3 c0(m(0, 0), m(1, 0), m(2, 0));
        Vec3 c1(m(0, 1), m(1, 1), m(2, 1));
        Vec3 c2(m(0, 2), m(1, 2), m(2, 2));

        c0 = c0.normalized();
        c1 = c1.normalized();
        c2 = c2.normalized();

        const float r00 = c0.x, r01 = c1.x, r02 = c2.x;
        const float r10 = c0.y, r11 = c1.y, r12 = c2.y;
        const float r20 = c0.z, r21 = c1.z, r22 = c2.z;

        const float trace = r00 + r11 + r22;
        Quat rot;

        if (trace > 0.0f) {
            const float s = 0.5f / std::sqrt(trace + 1.0f);
            rot.w = 0.25f / s;
            rot.x = (r21 - r12) * s;
            rot.y = (r02 - r20) * s;
            rot.z = (r10 - r01) * s;
        } else if (r00 > r11 && r00 > r22) {
            const float s = 2.0f * std::sqrt(1.0f + r00 - r11 - r22);
            rot.w = (r21 - r12) / s;
            rot.x = 0.25f * s;
            rot.y = (r01 + r10) / s;
            rot.z = (r02 + r20) / s;
        } else if (r11 > r22) {
            const float s = 2.0f * std::sqrt(1.0f + r11 - r00 - r22);
            rot.w = (r02 - r20) / s;
            rot.x = (r01 + r10) / s;
            rot.y = 0.25f * s;
            rot.z = (r12 + r21) / s;
        } else {
            const float s = 2.0f * std::sqrt(1.0f + r22 - r00 - r11);
            rot.w = (r10 - r01) / s;
            rot.x = (r02 + r20) / s;
            rot.y = (r12 + r21) / s;
            rot.z = 0.25f * s;
        }

        return Transform3D(pos, rot.normalized());
    }

    [[nodiscard]] bool approx_equal(const Transform3D& other, float tol = EPSILON) const noexcept {
        return position.approx_equal(other.position, tol) && rotation.approx_equal(other.rotation, tol);
    }

    [[nodiscard]] bool operator==(const Transform3D& other) const noexcept {
        return approx_equal(other);
    }

    [[nodiscard]] bool operator!=(const Transform3D& other) const noexcept {
        return !(*this == other);
    }
};

} // namespace nisaba::physics

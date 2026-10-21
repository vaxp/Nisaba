#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/quat.hpp"
#include "nisaba/physics/math/simd_math.hpp"

namespace nisaba::physics {

/// @brief Represents rigid body mass distribution properties in principal axes space.
/// By storing principal moments along diagonal (Ixx, Iyy, Izz) and principal orientation (Quat),
/// we eliminate full 3x3 inertia tensor matrix inverses entirely during simulation.
struct MassProperties {
    float mass = 1.0f;
    Vec3 inertia_diagonal = Vec3(1.0f, 1.0f, 1.0f);
    Quat inertia_rotation = Quat::identity();
    Vec3 center_of_mass = Vec3::zero();

    /// @brief Scale the inertia tensor to match a target mass uniformly
    void scale_to_mass(float new_mass) noexcept {
        if (mass > EPSILON && new_mass > 0.0f) {
            const float ratio = new_mass / mass;
            inertia_diagonal *= ratio;
            mass = new_mass;
        }
    }

    /// @brief Calculate inverse moments of inertia along principal axes
    [[nodiscard]] Vec3 calculate_inv_inertia_diagonal() const noexcept {
        return Vec3(
            inertia_diagonal.x > EPSILON ? 1.0f / inertia_diagonal.x : 0.0f,
            inertia_diagonal.y > EPSILON ? 1.0f / inertia_diagonal.y : 0.0f,
            inertia_diagonal.z > EPSILON ? 1.0f / inertia_diagonal.z : 0.0f
        );
    }
};

} // namespace nisaba::physics

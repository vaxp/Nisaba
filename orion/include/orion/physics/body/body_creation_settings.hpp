#pragma once

#include "orion/math/vec3.hpp"
#include "orion/math/quat.hpp"
#include "orion/physics/body/motion_type.hpp"
#include "orion/physics/collision/shape.hpp"
#include <cstdint>

namespace orion::physics {

/// @brief Configuration descriptor used to instantiate physics bodies.
struct BodyCreationSettings {
    Vec3 position = Vec3::zero();
    Quat rotation = Quat::identity();
    Vec3 linear_velocity = Vec3::zero();
    Vec3 angular_velocity = Vec3::zero();

    const Shape* shape = nullptr;
    MotionType motion_type = MotionType::Dynamic;

    float friction = 0.2f;
    float restitution = 0.0f;
    float density = 1000.0f;
    float override_mass = 0.0f; // If > 0, overrides mass calculated from shape density

    float linear_damping = 0.05f;
    float angular_damping = 0.05f;
    float gravity_factor = 1.0f;

    uint16_t object_layer = 0;
    uint8_t broad_phase_layer = 0;
    uint64_t user_data = 0;
    bool is_sensor = false;

    constexpr BodyCreationSettings() noexcept = default;

    BodyCreationSettings(
        const Shape* in_shape,
        const Vec3& in_position,
        const Quat& in_rotation = Quat::identity(),
        MotionType in_motion_type = MotionType::Dynamic
    ) noexcept
        : position(in_position),
          rotation(in_rotation),
          shape(in_shape),
          motion_type(in_motion_type) {}
};

} // namespace orion::physics

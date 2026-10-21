#pragma once

#include <cstdint>

namespace nisaba::physics {

/// @brief Determines how a body behaves in the physics simulation.
enum class MotionType : uint8_t {
    Static = 0,    ///< Immovable body (zero velocity, no MotionProperties allocated, infinite mass).
    Kinematic = 1, ///< Animated/scripted body with velocity, unaffected by forces/collisions.
    Dynamic = 2    ///< Fully simulated rigid body responding to forces, gravity, and impulses.
};

/// @brief Bitmask for degrees of freedom restriction (e.g. 2D physics or locking axes).
enum class AllowedDOFs : uint8_t {
    None = 0,
    TranslationX = 1 << 0,
    TranslationY = 1 << 1,
    TranslationZ = 1 << 2,
    RotationX    = 1 << 3,
    RotationY    = 1 << 4,
    RotationZ    = 1 << 5,
    All          = 0x3F,
    Plane2D      = TranslationX | TranslationY | RotationZ
};

} // namespace nisaba::physics

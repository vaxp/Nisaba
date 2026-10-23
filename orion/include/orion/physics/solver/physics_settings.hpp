#pragma once

#include "orion/math/vec3.hpp"
#include <cstdint>

namespace orion::physics {

/// @brief Global simulation parameters and solver tuning settings.
struct PhysicsSettings {
    Vec3 gravity = Vec3(0.0f, -9.81f, 0.0f);   ///< Gravity acceleration vector in m/s^2
    uint32_t num_velocity_iterations = 10;     ///< Number of Gauss-Seidel solver velocity iterations
    float baumgarte_factor = 0.2f;             ///< Baumgarte stabilization factor for position error (0.1 - 0.3)
    float penetration_slop = 0.005f;           ///< Penetration tolerance before Baumgarte bias applies (5 mm)
    float restitution_threshold = 1.0f;        ///< Relative velocity threshold below which restitution is suppressed (prevents bouncing jitter)
    bool enable_warm_starting = true;          ///< Retain previous frame impulses to dramatically accelerate convergence
    float max_penetration_bias = 10.0f;        ///< Maximum bias velocity clamp to avoid explosive projection (m/s)

    // --- Sleeping & Island Settings ---
    bool enable_sleeping = true;               ///< Enable automatic island-based sleep when motion ceases
    float time_before_sleep = 0.5f;            ///< Time in seconds an island must remain stationary before sleeping
    float linear_sleep_threshold_sq = 0.04f;   ///< Linear velocity magnitude squared threshold ((0.2 m/s)^2)
    float angular_sleep_threshold_sq = 0.04f;  ///< Angular velocity magnitude squared threshold ((0.2 rad/s)^2)

    constexpr PhysicsSettings() noexcept = default;
};

} // namespace orion::physics

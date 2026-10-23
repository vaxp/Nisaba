#pragma once

#include "orion/math/vec3.hpp"
#include <cstdint>

namespace orion::physics {

/// @brief Represents a single contact point between two colliding bodies.
struct ContactPoint {
    Vec3 position_on_a = Vec3::zero();  ///< Contact position in world space on surface of Body A
    Vec3 position_on_b = Vec3::zero();  ///< Contact position in world space on surface of Body B
    Vec3 normal = Vec3::unit_y();       ///< Contact normal pointing from Body A to Body B
    float penetration_depth = 0.0f;     ///< Penetration depth (positive when bodies overlap)
    uint32_t feature_id = 0;            ///< Unique identifier of the feature (used for warm-start cache)

    // Warm-starting cached impulses (used in Phase 4 sequential impulse solver)
    float normal_impulse = 0.0f;
    float tangent_impulse[2] = {0.0f, 0.0f};

    constexpr ContactPoint() noexcept = default;

    ContactPoint(
        const Vec3& in_pos_a,
        const Vec3& in_pos_b,
        const Vec3& in_normal,
        float in_depth,
        uint32_t in_feature_id = 0
    ) noexcept
        : position_on_a(in_pos_a),
          position_on_b(in_pos_b),
          normal(in_normal),
          penetration_depth(in_depth),
          feature_id(in_feature_id) {}
};

} // namespace orion::physics

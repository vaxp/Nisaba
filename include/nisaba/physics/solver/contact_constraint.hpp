#pragma once

#include "nisaba/physics/body/body.hpp"
#include "nisaba/physics/collision/contact_manifold.hpp"
#include "nisaba/physics/solver/physics_settings.hpp"
#include <cstdint>

namespace nisaba::physics {

/// @brief Precomputed constraint data used by the Sequential Impulse Velocity Solver.
struct ContactConstraint {
    struct Point {
        Vec3 r_a = Vec3::zero();               ///< Vector from Body A center of mass to contact point
        Vec3 r_b = Vec3::zero();               ///< Vector from Body B center of mass to contact point
        float eff_mass_normal = 0.0f;          ///< Effective mass along normal: 1 / (J_n * M^-1 * J_n^T)
        float eff_mass_t1 = 0.0f;              ///< Effective mass along tangent 1
        float eff_mass_t2 = 0.0f;              ///< Effective mass along tangent 2
        float velocity_bias = 0.0f;            ///< Baumgarte position error correction + restitution bias
        float normal_impulse = 0.0f;           ///< Accumulated normal impulse (lambda_n >= 0)
        float tangent_impulse[2] = {0.0f, 0.0f};///< Accumulated friction impulses (lambda_t1, lambda_t2)
        uint32_t feature_id = 0;               ///< Feature ID for matching across frames
    };

    Body* body_a = nullptr;
    Body* body_b = nullptr;
    Vec3 normal = Vec3::unit_y();              ///< Contact normal pointing from Body A to Body B
    Vec3 tangent1 = Vec3::unit_x();            ///< First tangent vector (perpendicular to normal)
    Vec3 tangent2 = Vec3::unit_z();            ///< Second tangent vector (perpendicular to normal and tangent1)
    float friction = 0.0f;
    float restitution = 0.0f;

    Point points[ContactManifold::MAX_POINTS];
    uint32_t num_points = 0;

    /// @brief Precompute effective masses and velocity biases for this contact constraint
    void setup(const ContactManifold& manifold, Body* in_body_a, Body* in_body_b, float dt, const PhysicsSettings& settings) noexcept {
        body_a = in_body_a;
        body_b = in_body_b;
        normal = manifold.normal;
        normal.get_orthonormal_basis(tangent1, tangent2);
        friction = manifold.combined_friction;
        restitution = manifold.combined_restitution;
        num_points = manifold.num_points;

        const float inv_m_a = body_a->get_inv_mass();
        const float inv_m_b = body_b->get_inv_mass();

        const float inv_dt = dt > EPSILON ? 1.0f / dt : 0.0f;

        for (uint32_t i = 0; i < num_points; ++i) {
            const ContactPoint& cp = manifold.points[i];
            Point& pt = points[i];

            pt.r_a = cp.position_on_a - body_a->get_position();
            pt.r_b = cp.position_on_b - body_b->get_position();
            pt.feature_id = cp.feature_id;

            // Retain warm starting impulses if enabled
            if (settings.enable_warm_starting) {
                pt.normal_impulse = cp.normal_impulse;
                pt.tangent_impulse[0] = cp.tangent_impulse[0];
                pt.tangent_impulse[1] = cp.tangent_impulse[1];
            } else {
                pt.normal_impulse = 0.0f;
                pt.tangent_impulse[0] = 0.0f;
                pt.tangent_impulse[1] = 0.0f;
            }

            // Normal effective mass: K_n = 1/m_a + 1/m_b + ((I_a^-1 * (r_a x n)) x r_a + (I_b^-1 * (r_b x n)) x r_b) . n
            const Vec3 rn_a = pt.r_a.cross(normal);
            const Vec3 rn_b = pt.r_b.cross(normal);
            const Vec3 inv_i_rn_a = body_a->multiply_inverse_inertia(rn_a);
            const Vec3 inv_i_rn_b = body_b->multiply_inverse_inertia(rn_b);
            const float k_normal = inv_m_a + inv_m_b + inv_i_rn_a.dot(rn_a) + inv_i_rn_b.dot(rn_b);
            pt.eff_mass_normal = k_normal > EPSILON ? 1.0f / k_normal : 0.0f;

            // Tangent 1 effective mass
            const Vec3 rt1_a = pt.r_a.cross(tangent1);
            const Vec3 rt1_b = pt.r_b.cross(tangent1);
            const Vec3 inv_i_rt1_a = body_a->multiply_inverse_inertia(rt1_a);
            const Vec3 inv_i_rt1_b = body_b->multiply_inverse_inertia(rt1_b);
            const float k_t1 = inv_m_a + inv_m_b + inv_i_rt1_a.dot(rt1_a) + inv_i_rt1_b.dot(rt1_b);
            pt.eff_mass_t1 = k_t1 > EPSILON ? 1.0f / k_t1 : 0.0f;

            // Tangent 2 effective mass
            const Vec3 rt2_a = pt.r_a.cross(tangent2);
            const Vec3 rt2_b = pt.r_b.cross(tangent2);
            const Vec3 inv_i_rt2_a = body_a->multiply_inverse_inertia(rt2_a);
            const Vec3 inv_i_rt2_b = body_b->multiply_inverse_inertia(rt2_b);
            const float k_t2 = inv_m_a + inv_m_b + inv_i_rt2_a.dot(rt2_a) + inv_i_rt2_b.dot(rt2_b);
            pt.eff_mass_t2 = k_t2 > EPSILON ? 1.0f / k_t2 : 0.0f;

            // Velocity bias calculation:
            // 1. Baumgarte position stabilization: bias_pos = (beta / dt) * max(0, depth - slop)
            const float penetration_error = std::max(0.0f, cp.penetration_depth - settings.penetration_slop);
            float bias = std::min(settings.max_penetration_bias, settings.baumgarte_factor * inv_dt * penetration_error);

            // 2. Restitution bias: if relative velocity is significant, add bounce velocity
            const Vec3 rel_vel = body_b->get_point_velocity(cp.position_on_b) - body_a->get_point_velocity(cp.position_on_a);
            const float vn = rel_vel.dot(normal);
            if (vn < -settings.restitution_threshold) {
                bias = std::max(bias, -restitution * vn);
            }

            pt.velocity_bias = bias;
        }
    }
};

} // namespace nisaba::physics

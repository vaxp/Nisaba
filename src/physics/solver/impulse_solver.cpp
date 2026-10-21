#include "nisaba/physics/solver/impulse_solver.hpp"
#include <algorithm>

namespace nisaba::physics {

void ImpulseSolver::apply_warm_starting(ContactConstraint* constraints, size_t count) noexcept {
    for (size_t c = 0; c < count; ++c) {
        ContactConstraint& cc = constraints[c];
        Body* b_a = cc.body_a;
        Body* b_b = cc.body_b;

        for (uint32_t i = 0; i < cc.num_points; ++i) {
            ContactConstraint::Point& pt = cc.points[i];

            // J = lambda_n * n + lambda_t1 * t1 + lambda_t2 * t2
            const Vec3 impulse = cc.normal * pt.normal_impulse +
                                 cc.tangent1 * pt.tangent_impulse[0] +
                                 cc.tangent2 * pt.tangent_impulse[1];

            // Body A receives -J at r_a; Body B receives +J at r_b
            b_a->apply_impulse_at_point(-impulse, b_a->get_position() + pt.r_a);
            b_b->apply_impulse_at_point( impulse, b_b->get_position() + pt.r_b);
        }
    }
}

void ImpulseSolver::solve_velocity_iteration(ContactConstraint* constraints, size_t count) noexcept {
    for (size_t c = 0; c < count; ++c) {
        ContactConstraint& cc = constraints[c];
        Body* b_a = cc.body_a;
        Body* b_b = cc.body_b;

        const Vec3 pos_a = b_a->get_position();
        const Vec3 pos_b = b_b->get_position();

        // 1. Solve Friction Constraints First
        if (cc.friction > EPSILON) {
            for (uint32_t i = 0; i < cc.num_points; ++i) {
                ContactConstraint::Point& pt = cc.points[i];

                const Vec3 v_a = b_a->get_point_velocity(pos_a + pt.r_a);
                const Vec3 v_b = b_b->get_point_velocity(pos_b + pt.r_b);
                const Vec3 v_rel = v_b - v_a;

                // Maximum Coulomb friction force: lambda_max = mu * lambda_n
                const float max_friction = cc.friction * pt.normal_impulse;

                // --- Tangent 1 ---
                const float vt1 = v_rel.dot(cc.tangent1);
                const float delta_lambda_t1 = -pt.eff_mass_t1 * vt1;
                const float old_lambda_t1 = pt.tangent_impulse[0];
                pt.tangent_impulse[0] = std::clamp(old_lambda_t1 + delta_lambda_t1, -max_friction, max_friction);
                const float applied_t1 = pt.tangent_impulse[0] - old_lambda_t1;

                const Vec3 impulse_t1 = cc.tangent1 * applied_t1;
                b_a->apply_impulse_at_point(-impulse_t1, pos_a + pt.r_a);
                b_b->apply_impulse_at_point( impulse_t1, pos_b + pt.r_b);

                // --- Tangent 2 ---
                const Vec3 v_a2 = b_a->get_point_velocity(pos_a + pt.r_a);
                const Vec3 v_b2 = b_b->get_point_velocity(pos_b + pt.r_b);
                const Vec3 v_rel2 = v_b2 - v_a2;

                const float vt2 = v_rel2.dot(cc.tangent2);
                const float delta_lambda_t2 = -pt.eff_mass_t2 * vt2;
                const float old_lambda_t2 = pt.tangent_impulse[1];
                pt.tangent_impulse[1] = std::clamp(old_lambda_t2 + delta_lambda_t2, -max_friction, max_friction);
                const float applied_t2 = pt.tangent_impulse[1] - old_lambda_t2;

                const Vec3 impulse_t2 = cc.tangent2 * applied_t2;
                b_a->apply_impulse_at_point(-impulse_t2, pos_a + pt.r_a);
                b_b->apply_impulse_at_point( impulse_t2, pos_b + pt.r_b);
            }
        }

        // 2. Solve Non-Penetration Normal Constraints
        for (uint32_t i = 0; i < cc.num_points; ++i) {
            ContactConstraint::Point& pt = cc.points[i];

            const Vec3 v_a = b_a->get_point_velocity(pos_a + pt.r_a);
            const Vec3 v_b = b_b->get_point_velocity(pos_b + pt.r_b);
            const Vec3 v_rel = v_b - v_a;

            const float vn = v_rel.dot(cc.normal);

            // Compute delta impulse: delta_lambda = -m_eff * (v_n - bias)
            const float delta_lambda_n = -pt.eff_mass_normal * (vn - pt.velocity_bias);

            // Unilateral clamp: accumulated normal impulse cannot be negative (no sticking/pulling)
            const float old_lambda_n = pt.normal_impulse;
            pt.normal_impulse = std::max(0.0f, old_lambda_n + delta_lambda_n);
            const float applied_n = pt.normal_impulse - old_lambda_n;

            // Apply impulse along normal
            const Vec3 impulse_n = cc.normal * applied_n;
            b_a->apply_impulse_at_point(-impulse_n, pos_a + pt.r_a);
            b_b->apply_impulse_at_point( impulse_n, pos_b + pt.r_b);
        }
    }
}

void ImpulseSolver::solve_velocity_constraints(
    ContactConstraint* constraints,
    size_t count,
    uint32_t num_iterations
) noexcept {
    for (uint32_t iter = 0; iter < num_iterations; ++iter) {
        solve_velocity_iteration(constraints, count);
    }
}

} // namespace nisaba::physics

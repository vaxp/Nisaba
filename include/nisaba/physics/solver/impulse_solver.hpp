#pragma once

#include "nisaba/physics/solver/contact_constraint.hpp"

namespace nisaba::physics {

/// @brief Projected Gauss-Seidel Sequential Impulse Velocity Solver.
/// Solves unilateral non-penetration constraints and Coulomb friction cones.
class ImpulseSolver {
public:
    /// @brief Applies cached impulses from previous time step for rapid convergence
    static void apply_warm_starting(ContactConstraint* constraints, size_t count) noexcept;

    /// @brief Solves all velocity constraints across N Gauss-Seidel iterations
    static void solve_velocity_constraints(
        ContactConstraint* constraints,
        size_t count,
        uint32_t num_iterations
    ) noexcept;

private:
    static void solve_velocity_iteration(ContactConstraint* constraints, size_t count) noexcept;
};

} // namespace nisaba::physics

/// @file spring_simulation.cpp
/// @brief Analytical damped harmonic oscillator physics implementation for Nisaba.

#include "nisaba/animation/spring_simulation.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::animation {

SpringSimulation::SpringSimulation(const SpringDescription& desc,
                                   float start,
                                   float end,
                                   float velocity,
                                   float distance_tolerance,
                                   float velocity_tolerance)
    : desc_(desc),
      start_(start),
      end_(end),
      initial_velocity_(velocity),
      distance_tolerance_(std::max(1e-5f, distance_tolerance)),
      velocity_tolerance_(std::max(1e-4f, velocity_tolerance)) {
    solve();
}

void SpringSimulation::solve() {
    float m = std::max(1e-4f, desc_.mass);
    float k = std::max(1e-4f, desc_.stiffness);
    float c = std::max(0.0f, desc_.damping);

    omega0_ = std::sqrt(k / m);
    zeta_ = c / (2.0f * std::sqrt(k * m));

    float x0 = start_ - end_;
    float v0 = initial_velocity_;

    constexpr float eps = 1e-4f;

    if (zeta_ < 1.0f - eps) {
        // ── Underdamped Case ──────────────────────────────────────
        solution_type_ = SolutionType::Underdamped;
        omega_d_ = omega0_ * std::sqrt(1.0f - zeta_ * zeta_);
        c1_ = x0;
        c2_ = (v0 + zeta_ * omega0_ * x0) / omega_d_;

        // Decay envelope rate is zeta * omega0
        float decay_rate = std::max(1e-2f, zeta_ * omega0_);
        float max_ampl = std::max(std::sqrt(c1_ * c1_ + c2_ * c2_), 0.1f);
        estimated_duration_ = std::clamp(-std::log(distance_tolerance_ / max_ampl) / decay_rate, 0.1f, 3.0f);

    } else if (zeta_ > 1.0f + eps) {
        // ── Overdamped Case ───────────────────────────────────────
        solution_type_ = SolutionType::Overdamped;
        float gamma = omega0_ * std::sqrt(zeta_ * zeta_ - 1.0f);
        r1_ = -zeta_ * omega0_ + gamma;
        r2_ = -zeta_ * omega0_ - gamma;

        c2_ = (v0 - r1_ * x0) / (r2_ - r1_);
        c1_ = x0 - c2_;

        // Slower decay root is r1
        float decay_rate = std::max(1e-2f, std::abs(r1_));
        estimated_duration_ = std::clamp(-std::log(distance_tolerance_ / std::max(std::abs(x0), 0.1f)) / decay_rate, 0.1f, 3.0f);

    } else {
        // ── Critically Damped Case ────────────────────────────────
        solution_type_ = SolutionType::CriticallyDamped;
        c1_ = x0;
        c2_ = v0 + omega0_ * x0;

        float decay_rate = std::max(1e-2f, omega0_);
        estimated_duration_ = std::clamp(-std::log(distance_tolerance_ / std::max(std::abs(x0), 0.1f)) / decay_rate, 0.1f, 3.0f);
    }
}

float SpringSimulation::x(float t) const {
    if (t <= 0.0f) return start_;

    float y = 0.0f;
    switch (solution_type_) {
        case SolutionType::Underdamped: {
            float decay = std::exp(-zeta_ * omega0_ * t);
            float cos_term = std::cos(omega_d_ * t);
            float sin_term = std::sin(omega_d_ * t);
            y = decay * (c1_ * cos_term + c2_ * sin_term);
            break;
        }
        case SolutionType::CriticallyDamped: {
            float decay = std::exp(-omega0_ * t);
            y = decay * (c1_ + c2_ * t);
            break;
        }
        case SolutionType::Overdamped: {
            y = c1_ * std::exp(r1_ * t) + c2_ * std::exp(r2_ * t);
            break;
        }
    }

    // Settled snap to avoid micro floating-point wobbles
    if (std::abs(y) < distance_tolerance_ && std::abs(dx(t)) < velocity_tolerance_) {
        return end_;
    }

    return end_ + y;
}

float SpringSimulation::dx(float t) const {
    if (t <= 0.0f) return initial_velocity_;

    switch (solution_type_) {
        case SolutionType::Underdamped: {
            float decay = std::exp(-zeta_ * omega0_ * t);
            float cos_term = std::cos(omega_d_ * t);
            float sin_term = std::sin(omega_d_ * t);
            float y = decay * (c1_ * cos_term + c2_ * sin_term);
            float dy_no_decay = -c1_ * omega_d_ * sin_term + c2_ * omega_d_ * cos_term;
            return -zeta_ * omega0_ * y + decay * dy_no_decay;
        }
        case SolutionType::CriticallyDamped: {
            float decay = std::exp(-omega0_ * t);
            float y = decay * (c1_ + c2_ * t);
            return -omega0_ * y + c2_ * decay;
        }
        case SolutionType::Overdamped: {
            return c1_ * r1_ * std::exp(r1_ * t) + c2_ * r2_ * std::exp(r2_ * t);
        }
    }
    return 0.0f;
}

bool SpringSimulation::is_done(float t) const {
    if (t <= 0.0f) return false;
    float current_x = x(t);
    float current_v = dx(t);
    return (std::abs(current_x - end_) < distance_tolerance_) &&
           (std::abs(current_v) < velocity_tolerance_);
}

} // namespace nisaba::animation

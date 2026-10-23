#pragma once

#include "orion/math/vec3.hpp"
#include "orion/math/quat.hpp"
#include "orion/physics/collision/mass_properties.hpp"
#include <algorithm>
#include <cstdint>

namespace orion::physics {

/// @brief Contains dynamic motion properties: velocities, inverse mass, inertia, and forces.
/// Separated from Body so static bodies (which never move or collide dynamically)
/// do not allocate or occupy cache lines for motion state.
/// Layout: Exactly 128 bytes (two 64-byte CPU cache lines), aligned to 16 bytes.
class alignas(16) MotionProperties {
public:
    static constexpr uint32_t INACTIVE_INDEX = 0xFFFFFFFF;

    MotionProperties() noexcept = default;

    // --- Velocity Getters & Setters ---
    [[nodiscard]] const Vec3& get_linear_velocity() const noexcept {
        return m_linear_velocity;
    }

    void set_linear_velocity(const Vec3& v) noexcept {
        m_linear_velocity = v;
        clamp_linear_velocity();
    }

    [[nodiscard]] const Vec3& get_angular_velocity() const noexcept {
        return m_angular_velocity;
    }

    void set_angular_velocity(const Vec3& w) noexcept {
        m_angular_velocity = w;
        clamp_angular_velocity();
    }

    // --- Mass & Inertia ---
    [[nodiscard]] float get_inv_mass() const noexcept {
        return m_inv_mass;
    }

    void set_inv_mass(float inv_mass) noexcept {
        m_inv_mass = std::max(0.0f, inv_mass);
    }

    [[nodiscard]] float get_mass() const noexcept {
        return m_inv_mass > EPSILON ? 1.0f / m_inv_mass : 0.0f;
    }

    void set_mass(float mass) noexcept {
        m_inv_mass = mass > EPSILON ? 1.0f / mass : 0.0f;
    }

    [[nodiscard]] const Vec3& get_inv_inertia_diagonal() const noexcept {
        return m_inv_inertia_diagonal;
    }

    [[nodiscard]] const Quat& get_inertia_rotation() const noexcept {
        return m_inertia_rotation;
    }

    void set_mass_properties(const MassProperties& mp) noexcept {
        set_mass(mp.mass);
        m_inv_inertia_diagonal = mp.calculate_inv_inertia_diagonal();
        m_inertia_rotation = mp.inertia_rotation;
    }

    /// @brief Multiplies an angular world-space vector by the body's world-space inverse inertia tensor.
    /// Uses principal axes decomposition: v_world = Q * (D^-1 * (Q^-1 * v))
    /// where Q = body_rotation * inertia_rotation.
    [[nodiscard]] Vec3 multiply_world_inv_inertia_by_vector(const Quat& body_rotation, const Vec3& v) const noexcept {
        const Quat q_total = body_rotation * m_inertia_rotation;
        const Vec3 v_local = q_total.inverse_rotate_vec(v);
        const Vec3 v_inv_inertia(
            v_local.x * m_inv_inertia_diagonal.x,
            v_local.y * m_inv_inertia_diagonal.y,
            v_local.z * m_inv_inertia_diagonal.z
        );
        return q_total.rotate_vec(v_inv_inertia);
    }

    // --- Forces & Torques ---
    [[nodiscard]] const Vec3& get_accumulated_force() const noexcept {
        return m_accumulated_force;
    }

    [[nodiscard]] const Vec3& get_accumulated_torque() const noexcept {
        return m_accumulated_torque;
    }

    void add_force(const Vec3& force) noexcept {
        m_accumulated_force += force;
    }

    void add_torque(const Vec3& torque) noexcept {
        m_accumulated_torque += torque;
    }

    void add_force_at_point(const Vec3& force, const Vec3& point, const Vec3& center_of_mass) noexcept {
        m_accumulated_force += force;
        m_accumulated_torque += (point - center_of_mass).cross(force);
    }

    void reset_forces() noexcept {
        m_accumulated_force = Vec3::zero();
        m_accumulated_torque = Vec3::zero();
    }

    // --- Impulses ---
    void apply_linear_impulse(const Vec3& impulse) noexcept {
        m_linear_velocity += impulse * m_inv_mass;
        clamp_linear_velocity();
    }

    void apply_angular_impulse(const Vec3& impulse, const Quat& body_rotation) noexcept {
        m_angular_velocity += multiply_world_inv_inertia_by_vector(body_rotation, impulse);
        clamp_angular_velocity();
    }

    void apply_impulse_at_point(
        const Vec3& impulse,
        const Vec3& point,
        const Vec3& center_of_mass,
        const Quat& body_rotation
    ) noexcept {
        apply_linear_impulse(impulse);
        apply_angular_impulse((point - center_of_mass).cross(impulse), body_rotation);
    }

    /// @brief Integrates accumulated forces, gravity, and torques into velocities with damping.
    void integrate_velocities(const Vec3& gravity, float dt, const Quat& body_rotation) noexcept {
        if (m_inv_mass <= 0.0f) return;

        // Linear velocity integration: v += dt * (F * inv_mass + g * gravity_factor)
        const Vec3 linear_accel = m_accumulated_force * m_inv_mass + gravity * m_gravity_factor;
        m_linear_velocity += linear_accel * dt;

        // Linear damping: v *= max(0, 1 - damping * dt)
        const float lin_damp = std::max(0.0f, 1.0f - m_linear_damping * dt);
        m_linear_velocity *= lin_damp;
        clamp_linear_velocity();

        // Angular velocity integration: w += dt * (I^-1 * tau)
        const Vec3 angular_accel = multiply_world_inv_inertia_by_vector(body_rotation, m_accumulated_torque);
        m_angular_velocity += angular_accel * dt;

        // Angular damping: w *= max(0, 1 - damping * dt)
        const float ang_damp = std::max(0.0f, 1.0f - m_angular_damping * dt);
        m_angular_velocity *= ang_damp;
        clamp_angular_velocity();

        reset_forces();
    }

    // --- Damping & Limits ---
    [[nodiscard]] float get_linear_damping() const noexcept { return m_linear_damping; }
    void set_linear_damping(float damping) noexcept { m_linear_damping = std::max(0.0f, damping); }

    [[nodiscard]] float get_angular_damping() const noexcept { return m_angular_damping; }
    void set_angular_damping(float damping) noexcept { m_angular_damping = std::max(0.0f, damping); }

    [[nodiscard]] float get_max_linear_velocity() const noexcept { return m_max_linear_velocity; }
    void set_max_linear_velocity(float max_v) noexcept { m_max_linear_velocity = std::max(0.0f, max_v); }

    [[nodiscard]] float get_max_angular_velocity() const noexcept { return m_max_angular_velocity; }
    void set_max_angular_velocity(float max_w) noexcept { m_max_angular_velocity = std::max(0.0f, max_w); }

    [[nodiscard]] float get_gravity_factor() const noexcept { return m_gravity_factor; }
    void set_gravity_factor(float factor) noexcept { m_gravity_factor = factor; }

    [[nodiscard]] uint32_t get_island_index() const noexcept { return m_island_index; }
    void set_island_index(uint32_t index) noexcept { m_island_index = index; }

    [[nodiscard]] uint32_t get_active_index() const noexcept { return m_active_index; }
    void set_active_index(uint32_t index) noexcept { m_active_index = index; }

    void clamp_linear_velocity() noexcept {
        const float len_sq = m_linear_velocity.length_sq();
        if (len_sq > m_max_linear_velocity * m_max_linear_velocity && len_sq > EPSILON * EPSILON) {
            m_linear_velocity *= (m_max_linear_velocity / std::sqrt(len_sq));
        }
    }

    void clamp_angular_velocity() noexcept {
        const float len_sq = m_angular_velocity.length_sq();
        if (len_sq > m_max_angular_velocity * m_max_angular_velocity && len_sq > EPSILON * EPSILON) {
            m_angular_velocity *= (m_max_angular_velocity / std::sqrt(len_sq));
        }
    }

private:
    // --- CACHE LINE 1 (64 bytes): Kinematic State & Inverse Inertia ---
    Vec3 m_linear_velocity = Vec3::zero();          // 16 bytes
    Vec3 m_angular_velocity = Vec3::zero();         // 16 bytes
    Vec3 m_inv_inertia_diagonal = Vec3::zero();     // 16 bytes
    Quat m_inertia_rotation = Quat::identity();     // 16 bytes

    // --- CACHE LINE 2 (64 bytes): Accumulators, Mass, Damping, Limits ---
    Vec3 m_accumulated_force = Vec3::zero();        // 16 bytes
    Vec3 m_accumulated_torque = Vec3::zero();       // 16 bytes
    float m_inv_mass = 1.0f;                        // 4 bytes
    float m_linear_damping = 0.05f;                 // 4 bytes
    float m_angular_damping = 0.05f;                // 4 bytes
    float m_max_linear_velocity = 500.0f;           // 4 bytes
    float m_max_angular_velocity = 100.0f;          // 4 bytes
    float m_gravity_factor = 1.0f;                  // 4 bytes
    uint32_t m_island_index = INACTIVE_INDEX;       // 4 bytes
    uint32_t m_active_index = INACTIVE_INDEX;       // 4 bytes
};

static_assert(sizeof(MotionProperties) == 128, "MotionProperties must be exactly 128 bytes (2 cache lines)");
static_assert(alignof(MotionProperties) == 16, "MotionProperties must be 16-byte aligned");

} // namespace orion::physics

#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/quat.hpp"
#include "nisaba/physics/math/aabb.hpp"
#include "nisaba/physics/math/transform3d.hpp"
#include "nisaba/physics/body/body_id.hpp"
#include "nisaba/physics/body/motion_type.hpp"
#include "nisaba/physics/body/motion_properties.hpp"
#include "nisaba/physics/body/body_creation_settings.hpp"
#include "nisaba/physics/collision/shape.hpp"
#include <cstdint>

namespace nisaba::physics {

/// @brief Represents a rigid body in the physics simulation.
/// Designed for maximum cache efficiency:
/// - Cache Line 1 (64 bytes): Pure spatial query state (position, rotation, AABB bounds).
///   Broad-phase queries and culling touch ONLY this cache line.
/// - Cache Line 2 (64 bytes): Simulation metadata, pointers, materials, ID, flags.
/// Total size: exactly 128 bytes, aligned to 16 bytes.
class alignas(16) Body {
public:
    enum Flags : uint8_t {
        FLAG_IS_SENSOR        = 1 << 0,
        FLAG_IS_ACTIVE        = 1 << 1,
        FLAG_IN_BROAD_PHASE   = 1 << 2
    };

    Body() noexcept = default;

    /// @brief Initialize the body from creation settings.
    /// @param settings Descriptor containing shape, initial transform, material.
    /// @param optional_mp Pre-allocated MotionProperties for dynamic/kinematic bodies.
    /// Static bodies must pass nullptr.
    void init(const BodyCreationSettings& settings, MotionProperties* optional_mp = nullptr) noexcept {
        m_position = settings.position;
        m_rotation = settings.rotation;
        m_shape = settings.shape;
        m_motion_type = settings.motion_type;
        m_friction = settings.friction;
        m_restitution = settings.restitution;
        m_object_layer = settings.object_layer;
        if (m_object_layer == 0 && m_motion_type != MotionType::Static) {
            m_object_layer = 1; // Default to Moving layer
        }
        m_broad_phase_layer = settings.broad_phase_layer;
        m_user_data = settings.user_data;
        m_flags = 0;
        if (settings.is_sensor) m_flags |= FLAG_IS_SENSOR;
        if (m_motion_type != MotionType::Static) m_flags |= FLAG_IS_ACTIVE;

        m_motion_properties = optional_mp;
        if (m_motion_properties && m_shape && m_motion_type == MotionType::Dynamic) {
            MassProperties mp = m_shape->get_mass_properties(settings.density);
            if (settings.override_mass > 0.0f) {
                mp.scale_to_mass(settings.override_mass);
            }
            m_motion_properties->set_mass_properties(mp);
            m_motion_properties->set_linear_damping(settings.linear_damping);
            m_motion_properties->set_angular_damping(settings.angular_damping);
            m_motion_properties->set_gravity_factor(settings.gravity_factor);
            m_motion_properties->set_linear_velocity(settings.linear_velocity);
            m_motion_properties->set_angular_velocity(settings.angular_velocity);
        } else if (m_motion_properties && m_motion_type == MotionType::Kinematic) {
            m_motion_properties->set_inv_mass(0.0f);
            m_motion_properties->set_linear_velocity(settings.linear_velocity);
            m_motion_properties->set_angular_velocity(settings.angular_velocity);
        }

        update_bounds();
    }

    // --- Spatial State (Cache Line 1) ---
    [[nodiscard]] const Vec3& get_position() const noexcept {
        return m_position;
    }

    void set_position(const Vec3& p) noexcept {
        m_position = p;
        update_bounds();
    }

    [[nodiscard]] const Quat& get_rotation() const noexcept {
        return m_rotation;
    }

    void set_rotation(const Quat& q) noexcept {
        m_rotation = q;
        update_bounds();
    }

    void set_position_and_rotation(const Vec3& p, const Quat& q) noexcept {
        m_position = p;
        m_rotation = q;
        update_bounds();
    }

    [[nodiscard]] Transform3D get_transform() const noexcept {
        return Transform3D(m_position, m_rotation);
    }

    [[nodiscard]] const AABB& get_bounds() const noexcept {
        return m_bounds;
    }

    void update_bounds() noexcept {
        if (m_shape) {
            m_bounds = m_shape->get_world_bounds(get_transform());
        } else {
            m_bounds = AABB(m_position, m_position);
        }
    }

    // --- Shape & Motion Types (Cache Line 2) ---
    [[nodiscard]] const Shape* get_shape() const noexcept {
        return m_shape;
    }

    void set_shape(const Shape* shape) noexcept {
        m_shape = shape;
        update_bounds();
    }

    [[nodiscard]] MotionType get_motion_type() const noexcept {
        return m_motion_type;
    }

    [[nodiscard]] bool is_static() const noexcept {
        return m_motion_type == MotionType::Static;
    }

    [[nodiscard]] bool is_kinematic() const noexcept {
        return m_motion_type == MotionType::Kinematic;
    }

    [[nodiscard]] bool is_dynamic() const noexcept {
        return m_motion_type == MotionType::Dynamic;
    }

    [[nodiscard]] MotionProperties* get_motion_properties() noexcept {
        return m_motion_properties;
    }

    [[nodiscard]] const MotionProperties* get_motion_properties() const noexcept {
        return m_motion_properties;
    }

    // --- Velocities & Kinematics forwarding ---
    [[nodiscard]] Vec3 get_linear_velocity() const noexcept {
        return m_motion_properties ? m_motion_properties->get_linear_velocity() : Vec3::zero();
    }

    void set_linear_velocity(const Vec3& v) noexcept {
        if (m_motion_properties) {
            m_motion_properties->set_linear_velocity(v);
            if (v.length_sq() > 1e-6f) wake_up();
        }
    }

    [[nodiscard]] Vec3 get_angular_velocity() const noexcept {
        return m_motion_properties ? m_motion_properties->get_angular_velocity() : Vec3::zero();
    }

    void set_angular_velocity(const Vec3& w) noexcept {
        if (m_motion_properties) {
            m_motion_properties->set_angular_velocity(w);
            if (w.length_sq() > 1e-6f) wake_up();
        }
    }

    void apply_impulse(const Vec3& impulse) noexcept {
        if (m_motion_properties && is_dynamic()) {
            wake_up();
            m_motion_properties->apply_linear_impulse(impulse);
        }
    }

    void apply_impulse_at_point(const Vec3& impulse, const Vec3& point) noexcept {
        if (m_motion_properties && is_dynamic()) {
            wake_up();
            m_motion_properties->apply_impulse_at_point(impulse, point, m_position, m_rotation);
        }
    }

    void add_force(const Vec3& force) noexcept {
        if (m_motion_properties && is_dynamic()) {
            wake_up();
            m_motion_properties->add_force(force);
        }
    }

    void add_force_at_point(const Vec3& force, const Vec3& point) noexcept {
        if (m_motion_properties && is_dynamic()) {
            wake_up();
            m_motion_properties->add_force_at_point(force, point, m_position);
        }
    }

    [[nodiscard]] float get_inv_mass() const noexcept {
        return (m_motion_properties && is_dynamic()) ? m_motion_properties->get_inv_mass() : 0.0f;
    }

    [[nodiscard]] Vec3 get_point_velocity(const Vec3& world_point) const noexcept {
        if (!m_motion_properties) return Vec3::zero();
        return m_motion_properties->get_linear_velocity() +
               m_motion_properties->get_angular_velocity().cross(world_point - m_position);
    }

    [[nodiscard]] Vec3 multiply_inverse_inertia(const Vec3& torque) const noexcept {
        if (!m_motion_properties || !is_dynamic()) return Vec3::zero();
        return m_motion_properties->multiply_world_inv_inertia_by_vector(m_rotation, torque);
    }

    void integrate_positions(float dt) noexcept {
        if (is_static() || !m_motion_properties) return;
        m_position += m_motion_properties->get_linear_velocity() * dt;
        m_rotation = m_rotation.integrate(m_motion_properties->get_angular_velocity(), dt);
        update_bounds();
    }

    // --- Material & Identifiers ---
    [[nodiscard]] float get_friction() const noexcept { return m_friction; }
    void set_friction(float f) noexcept { m_friction = std::max(0.0f, f); }

    [[nodiscard]] float get_restitution() const noexcept { return m_restitution; }
    void set_restitution(float r) noexcept { m_restitution = std::clamp(r, 0.0f, 1.0f); }

    [[nodiscard]] BodyID get_id() const noexcept { return m_id; }
    void set_id(BodyID id) noexcept { m_id = id; }

    [[nodiscard]] uint16_t get_object_layer() const noexcept { return m_object_layer; }
    void set_object_layer(uint16_t layer) noexcept { m_object_layer = layer; }

    [[nodiscard]] uint8_t get_broad_phase_layer() const noexcept { return m_broad_phase_layer; }
    void set_broad_phase_layer(uint8_t layer) noexcept { m_broad_phase_layer = layer; }

    [[nodiscard]] uint64_t get_user_data() const noexcept { return m_user_data; }
    void set_user_data(uint64_t data) noexcept { m_user_data = data; }

    [[nodiscard]] bool is_sensor() const noexcept { return (m_flags & FLAG_IS_SENSOR) != 0; }
    void set_is_sensor(bool sensor) noexcept {
        if (sensor) m_flags |= FLAG_IS_SENSOR;
        else m_flags &= ~FLAG_IS_SENSOR;
    }

    [[nodiscard]] bool is_active() const noexcept { return (m_flags & FLAG_IS_ACTIVE) != 0; }
    void set_is_active(bool active) noexcept {
        if (active) m_flags |= FLAG_IS_ACTIVE;
        else m_flags &= ~FLAG_IS_ACTIVE;
    }

    [[nodiscard]] bool is_sleeping() const noexcept {
        return is_dynamic() && !is_active();
    }

    [[nodiscard]] float get_sleep_timer() const noexcept { return m_sleep_timer; }
    void set_sleep_timer(float t) noexcept { m_sleep_timer = t; }
    void reset_sleep_timer() noexcept { m_sleep_timer = 0.0f; }

    void wake_up() noexcept {
        if (is_dynamic()) {
            if (!is_active()) {
                set_is_active(true);
                m_sleep_timer = 0.0f;
            }
        }
    }

    void put_to_sleep() noexcept {
        if (is_dynamic()) {
            set_is_active(false);
            if (m_motion_properties) {
                m_motion_properties->set_linear_velocity(Vec3::zero());
                m_motion_properties->set_angular_velocity(Vec3::zero());
            }
        }
    }

private:
    // --- CACHE LINE 1 (64 bytes): Spatial & Broadphase Bounds ---
    Vec3 m_position = Vec3::zero();                 // 16 bytes
    Quat m_rotation = Quat::identity();             // 16 bytes
    AABB m_bounds;                                  // 32 bytes

    // --- CACHE LINE 2 (64 bytes): Pointers, Materials, Layers, Flags ---
    const Shape* m_shape = nullptr;                 // 8 bytes
    MotionProperties* m_motion_properties = nullptr;// 8 bytes (null for Static!)
    uint64_t m_user_data = 0;                       // 8 bytes
    float m_friction = 0.2f;                        // 4 bytes
    float m_restitution = 0.0f;                     // 4 bytes
    BodyID m_id;                                    // 4 bytes
    float m_sleep_timer = 0.0f;                     // 4 bytes
    uint16_t m_object_layer = 0;                    // 2 bytes
    uint8_t m_broad_phase_layer = 0;                // 1 byte
    MotionType m_motion_type = MotionType::Dynamic; // 1 byte
    uint8_t m_flags = 0;                            // 1 byte
    uint8_t m_padding[19] = {0};                    // 19 bytes padding to reach exactly 64 bytes for Cache Line 2 (Total = 128 bytes)
};

static_assert(sizeof(Body) == 128, "Body must be exactly 128 bytes (2 cache lines)");
static_assert(alignof(Body) == 16, "Body must be 16-byte aligned");

} // namespace nisaba::physics

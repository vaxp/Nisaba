#include "nisaba/physics/scene/physics_scene.hpp"

namespace nisaba::physics {

PhysicsScene::PhysicsScene(const PhysicsSettings& in_settings) noexcept
    : m_settings(in_settings) {
    m_body_slots.reserve(128);
    m_free_slots.reserve(64);
    m_active_manifolds.reserve(256);
    m_active_constraints.reserve(256);
    m_previous_constraints.reserve(256);
    m_active_pairs.reserve(512);
}

PhysicsScene::~PhysicsScene() = default;

PhysicsScene::PhysicsScene(PhysicsScene&&) noexcept = default;
PhysicsScene& PhysicsScene::operator=(PhysicsScene&&) noexcept = default;

BodyID PhysicsScene::create_body(const BodyCreationSettings& settings) {
    uint32_t slot_idx = 0;
    if (!m_free_slots.empty()) {
        slot_idx = m_free_slots.back();
        m_free_slots.pop_back();
    } else {
        slot_idx = static_cast<uint32_t>(m_body_slots.size());
        m_body_slots.emplace_back();
    }

    BodySlot& slot = m_body_slots[slot_idx];
    slot.is_alive = true;

    // Allocate MotionProperties only for non-static bodies
    MotionProperties* mp = nullptr;
    if (settings.motion_type != MotionType::Static) {
        if (!slot.motion_properties) {
            slot.motion_properties = std::make_unique<MotionProperties>();
        }
        mp = slot.motion_properties.get();
    } else {
        slot.motion_properties.reset();
    }

    slot.body.init(settings, mp);
    const BodyID id(slot_idx, slot.sequence);
    slot.body.set_id(id);

    m_broad_phase.add_body(id, slot.body.get_bounds(), slot.body.get_object_layer());

    return id;
}

void PhysicsScene::destroy_body(BodyID id) {
    if (!id.is_valid()) return;

    const uint32_t idx = id.index();
    if (idx >= m_body_slots.size()) return;

    BodySlot& slot = m_body_slots[idx];
    if (!slot.is_alive || slot.sequence != id.sequence()) return;

    slot.is_alive = false;
    slot.sequence = (slot.sequence + 1) & BodyID::SEQUENCE_MASK;
    if (slot.sequence == 0) slot.sequence = 1; // Avoid 0
    slot.motion_properties.reset();
    m_free_slots.push_back(idx);

    m_broad_phase.remove_body(id);
}

Body* PhysicsScene::get_body(BodyID id) noexcept {
    if (!id.is_valid()) return nullptr;

    const uint32_t idx = id.index();
    if (idx >= m_body_slots.size()) return nullptr;

    BodySlot& slot = m_body_slots[idx];
    if (!slot.is_alive || slot.sequence != id.sequence()) return nullptr;

    return &slot.body;
}

const Body* PhysicsScene::get_body(BodyID id) const noexcept {
    if (!id.is_valid()) return nullptr;

    const uint32_t idx = id.index();
    if (idx >= m_body_slots.size()) return nullptr;

    const BodySlot& slot = m_body_slots[idx];
    if (!slot.is_alive || slot.sequence != id.sequence()) return nullptr;

    return &slot.body;
}

size_t PhysicsScene::get_body_count() const noexcept {
    size_t count = 0;
    for (const auto& slot : m_body_slots) {
        if (slot.is_alive) count++;
    }
    return count;
}

Body* PhysicsScene::get_body_by_slot(uint32_t slot_idx) noexcept {
    if (slot_idx >= m_body_slots.size()) return nullptr;
    BodySlot& slot = m_body_slots[slot_idx];
    if (!slot.is_alive) return nullptr;
    return &slot.body;
}

const Body* PhysicsScene::get_body_by_slot(uint32_t slot_idx) const noexcept {
    if (slot_idx >= m_body_slots.size()) return nullptr;
    const BodySlot& slot = m_body_slots[slot_idx];
    if (!slot.is_alive) return nullptr;
    return &slot.body;
}

void PhysicsScene::wake_up_body(BodyID id) noexcept {
    m_island_builder.wake_up_island_containing(*this, id);
}

bool PhysicsScene::is_body_sleeping(BodyID id) const noexcept {
    const Body* b = get_body(id);
    return b ? b->is_sleeping() : false;
}

void PhysicsScene::step(float dt) noexcept {
    if (dt <= 0.0f) return;

    const size_t total_slots = m_body_slots.size();

    // 1. Integrate Velocities (Gravity & External Forces) - ONLY for ACTIVE dynamic bodies
    for (size_t i = 0; i < total_slots; ++i) {
        BodySlot& slot = m_body_slots[i];
        if (slot.is_alive && slot.body.is_dynamic() && slot.body.is_active() && slot.motion_properties) {
            slot.motion_properties->integrate_velocities(m_settings.gravity, dt, slot.body.get_rotation());
        }
    }

    // 2. Broad-Phase Query & Narrow-Phase Collision Detection
    m_active_manifolds.clear();
    m_broad_phase.find_colliding_pairs(m_layer_filter, m_active_pairs);

    for (const auto& pair : m_active_pairs) {
        Body* b_a = get_body(pair.body_a);
        Body* b_b = get_body(pair.body_b);
        if (!b_a || !b_b) continue;

        const bool a_active = !b_a->is_static() && b_a->is_active();
        const bool b_active = !b_b->is_static() && b_b->is_active();

        // At least one body must be an active moving body (dynamic or kinematic)
        if (!a_active && !b_active) {
            continue;
        }

        ContactManifold manifold;
        if (NarrowPhase::collide_bodies(*b_a, *b_b, manifold)) {
            m_active_manifolds.push_back(manifold);
        }
    }

    // 3. Build Islands & Cluster Contact Manifolds (Wakes up sleeping bodies contacted by active bodies)
    m_island_builder.build_islands(*this, m_settings);

    // 4. Precompute Constraints & Transfer Warm-Starting Impulses
    std::swap(m_previous_constraints, m_active_constraints);
    m_active_constraints.clear();
    m_active_constraints.resize(m_active_manifolds.size());

    for (size_t m = 0; m < m_active_manifolds.size(); ++m) {
        ContactManifold& manifold = m_active_manifolds[m];
        Body* b_a = get_body(manifold.body_a);
        Body* b_b = get_body(manifold.body_b);

        // Find persistent contact match from previous frame
        if (m_settings.enable_warm_starting) {
            for (const auto& prev_c : m_previous_constraints) {
                if (prev_c.body_a == b_a && prev_c.body_b == b_b) {
                    for (uint32_t i = 0; i < manifold.num_points; ++i) {
                        for (uint32_t j = 0; j < prev_c.num_points; ++j) {
                            if (manifold.points[i].feature_id == prev_c.points[j].feature_id) {
                                manifold.points[i].normal_impulse = prev_c.points[j].normal_impulse;
                                manifold.points[i].tangent_impulse[0] = prev_c.points[j].tangent_impulse[0];
                                manifold.points[i].tangent_impulse[1] = prev_c.points[j].tangent_impulse[1];
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }

        m_active_constraints[m].setup(manifold, b_a, b_b, dt, m_settings);
    }

    // 5. Warm Starting
    if (m_settings.enable_warm_starting && !m_active_constraints.empty()) {
        ImpulseSolver::apply_warm_starting(m_active_constraints.data(), m_active_constraints.size());
    }

    // 6. Velocity Constraint Solver (Gauss-Seidel)
    if (!m_active_constraints.empty()) {
        ImpulseSolver::solve_velocity_constraints(
            m_active_constraints.data(),
            m_active_constraints.size(),
            m_settings.num_velocity_iterations
        );
    }

    // 7. Integrate Positions & Orientations (Symplectic Euler) & Update Broad-Phase for ACTIVE bodies
    for (size_t i = 0; i < total_slots; ++i) {
        BodySlot& slot = m_body_slots[i];
        if (slot.is_alive && !slot.body.is_static() && slot.body.is_active()) {
            slot.body.integrate_positions(dt);
            const Vec3 displacement = slot.body.get_linear_velocity() * dt;
            m_broad_phase.update_body(slot.body.get_id(), slot.body.get_bounds(), displacement);
        }
    }

    // 8. Island-based Sleep Evaluation
    m_island_builder.update_sleeping(*this, dt, m_settings);
}

bool PhysicsScene::cast_ray(
    const Ray& ray,
    float max_distance,
    RayCastResult& out_result
) const noexcept {
    out_result.has_hit = false;
    out_result.distance = max_distance;

    m_broad_phase.cast_ray(ray, max_distance, [&](BodyID id, float broadphase_t) {
        if (broadphase_t > out_result.distance) {
            return;
        }

        const Body* b = get_body(id);
        if (!b) return;

        const Shape* s = b->get_shape();
        if (!s) return;

        const Vec3 local_orig = b->get_rotation().inverse_rotate_vec(ray.origin - b->get_position());
        const Vec3 local_dir = b->get_rotation().inverse_rotate_vec(ray.direction);
        const Ray local_ray(local_orig, local_dir);

        RayCastHit hit;
        if (s->cast_ray(local_ray, out_result.distance, hit)) {
            if (hit.has_hit && hit.fraction < out_result.distance) {
                out_result.distance = hit.fraction;
                out_result.has_hit = true;
                out_result.body_id = id;
                out_result.hit_point = b->get_position() + b->get_rotation().rotate_vec(hit.point);
                out_result.hit_normal = b->get_rotation().rotate_vec(hit.normal).normalized();
            }
        }
    });

    return out_result.has_hit;
}

} // namespace nisaba::physics


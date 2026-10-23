#pragma once

#include "orion/physics/body/body.hpp"
#include "orion/physics/body/motion_properties.hpp"
#include "orion/physics/collision/narrow_phase.hpp"
#include "orion/physics/solver/physics_settings.hpp"
#include "orion/physics/solver/contact_constraint.hpp"
#include "orion/physics/solver/impulse_solver.hpp"
#include "orion/physics/broadphase/broad_phase.hpp"
#include "orion/physics/scene/island.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

namespace orion::physics {

/// @brief Master 3D Physics Scene and simulation coordinator.
/// Manages body lifecycle, collision dispatch, constraint formulation, and time stepping.
class PhysicsScene {
public:
    explicit PhysicsScene(const PhysicsSettings& in_settings = PhysicsSettings()) noexcept;
    ~PhysicsScene();

    PhysicsScene(const PhysicsScene&) = delete;
    PhysicsScene& operator=(const PhysicsScene&) = delete;
    PhysicsScene(PhysicsScene&&) noexcept;
    PhysicsScene& operator=(PhysicsScene&&) noexcept;

    // --- Body Lifecycle ---
    [[nodiscard]] BodyID create_body(const BodyCreationSettings& settings);
    void destroy_body(BodyID id);

    [[nodiscard]] Body* get_body(BodyID id) noexcept;
    [[nodiscard]] const Body* get_body(BodyID id) const noexcept;
    [[nodiscard]] size_t get_body_count() const noexcept;

    [[nodiscard]] size_t get_body_slot_count() const noexcept { return m_body_slots.size(); }
    [[nodiscard]] Body* get_body_by_slot(uint32_t slot_idx) noexcept;
    [[nodiscard]] const Body* get_body_by_slot(uint32_t slot_idx) const noexcept;

    void wake_up_body(BodyID id) noexcept;
    [[nodiscard]] bool is_body_sleeping(BodyID id) const noexcept;

    // --- Broad-Phase Access ---
    [[nodiscard]] BroadPhase& get_broad_phase() noexcept { return m_broad_phase; }
    [[nodiscard]] const BroadPhase& get_broad_phase() const noexcept { return m_broad_phase; }

    // --- Island & Sleeping Access ---
    [[nodiscard]] IslandBuilder& get_island_builder() noexcept { return m_island_builder; }
    [[nodiscard]] const IslandBuilder& get_island_builder() const noexcept { return m_island_builder; }

    // --- Simulation Settings ---
    [[nodiscard]] const PhysicsSettings& get_settings() const noexcept { return m_settings; }
    [[nodiscard]] PhysicsSettings& get_settings() noexcept { return m_settings; }
    void set_gravity(const Vec3& g) noexcept { m_settings.gravity = g; }

    // --- Simulation Time Step ---
    /// @brief Advances the physics simulation by time step dt (e.g. 1/60s).
    /// Executes velocity integration, broadphase/narrowphase collision, island building,
    /// sequential impulse solving, position integration, and sleep management.
    void step(float dt) noexcept;

    // --- Raycasting & Spatial Scene Queries ---
    struct RayCastResult {
        BodyID body_id;
        Vec3 hit_point = Vec3::zero();
        Vec3 hit_normal = Vec3::up();
        float distance = LARGE_FLOAT;
        bool has_hit = false;
    };

    /// @brief Casts a ray through the entire scene, accelerated via 4-way SIMD BVH traversal.
    /// @param ray The world-space ray.
    /// @param max_distance Maximum ray travel distance.
    /// @param out_result Contains the closest hit body ID, world-space hit point, normal, and distance.
    /// @return True if any body was hit within max_distance, false otherwise.
    [[nodiscard]] bool cast_ray(
        const Ray& ray,
        float max_distance,
        RayCastResult& out_result
    ) const noexcept;

    // --- Inspection & Diagnostics ---
    [[nodiscard]] const std::vector<ContactManifold>& get_active_manifolds() const noexcept { return m_active_manifolds; }
    [[nodiscard]] const std::vector<ContactConstraint>& get_active_constraints() const noexcept { return m_active_constraints; }

private:
    struct BodySlot {
        alignas(16) Body body;
        std::unique_ptr<MotionProperties> motion_properties;
        uint8_t sequence = 1;
        bool is_alive = false;
    };

    PhysicsSettings m_settings;
    std::vector<BodySlot> m_body_slots;
    std::vector<uint32_t> m_free_slots;

    // Spatial partitioning & broad-phase
    BroadPhase m_broad_phase;
    ObjectLayerPairFilter m_layer_filter;
    std::vector<BodyPair> m_active_pairs;

    // Island clustering & sleep engine
    IslandBuilder m_island_builder;

    // Reusable buffers to guarantee zero steady-state heap allocations per tick
    std::vector<ContactManifold> m_active_manifolds;
    std::vector<ContactConstraint> m_active_constraints;
    std::vector<ContactConstraint> m_previous_constraints;
};

} // namespace orion::physics

#pragma once

#include "nisaba/physics/scene/physics_scene.hpp"
#include "nisaba/render3d/scene/renderable.hpp"
#include "nisaba/render3d/scene/scene.hpp"
#include "nisaba/render3d/material/pbr_material.hpp"
#include <memory>
#include <vector>
#include <unordered_map>

namespace nisaba::render3d {

using BodyID = nisaba::physics::BodyID;
using PhysicsScene = nisaba::physics::PhysicsScene;
using Quat = nisaba::physics::Quat;

/// @brief Binding record pairing a rigid body simulation entity to a visual renderable.
struct BodyRenderBinding {
    BodyID body_id;
    std::shared_ptr<Renderable3D> renderable;
    Vec3 local_offset{Vec3::zero()};
    Quat local_rotation{Quat::identity()};
    Vec3 local_scale{Vec3::one()};

    // Sub-frame interpolation state
    Vec3 prev_position{Vec3::zero()};
    Quat prev_rotation{Quat::identity()};
    bool has_prev{false};
};

/// @brief Real-time synchronization bridge coordinating Nisaba's 3D Physics simulation
/// with the 3D PBR Rendering pipeline.
/// Provides automatic primitive mesh generation matching physical collision shapes,
/// transform propagation, and sub-frame temporal interpolation.
class PhysicsRenderBridge {
public:
    PhysicsRenderBridge() = default;
    ~PhysicsRenderBridge() = default;

    /// @brief Registers an explicit binding between a physics body and a visual renderable.
    void bind(
        BodyID body_id,
        std::shared_ptr<Renderable3D> renderable,
        const Vec3& local_offset = Vec3::zero(),
        const Quat& local_rotation = Quat::identity(),
        const Vec3& local_scale = Vec3::one()
    );

    /// @brief Unregisters a physics body from visual synchronization.
    void unbind(BodyID body_id);

    /// @brief Clears all registered bindings.
    void clear() noexcept;

    /// @brief Retrieves the renderable bound to a specific physics body.
    [[nodiscard]] std::shared_ptr<Renderable3D> get_renderable(BodyID body_id) const noexcept;

    /// @brief Checks if a physics body is currently bound.
    [[nodiscard]] bool has_binding(BodyID body_id) const noexcept;

    /// @brief Returns the total number of active bindings.
    [[nodiscard]] size_t binding_count() const noexcept { return m_bindings.size(); }

    /// @brief Automatically inspects the physics body shape (Box, Sphere, Cylinder, Plane)
    /// and constructs a matching procedural Renderable3D, initializes its world transform,
    /// registers the binding, and optionally attaches it to a Scene3D.
    [[nodiscard]] std::shared_ptr<Renderable3D> create_and_bind(
        const PhysicsScene& physics_scene,
        BodyID body_id,
        std::shared_ptr<PbrMaterial> material = nullptr,
        Scene3D* render_scene = nullptr
    );

    /// @brief Snapshots current physics transforms as historical state for sub-frame interpolation.
    void record_physics_step(const PhysicsScene& physics_scene) noexcept;

    /// @brief Synchronizes all registered renderables with current physics body transforms.
    /// @param physics_scene Source physics simulation scene.
    /// @param alpha Temporal interpolation factor in [0, 1] (1.0 = current step, < 1.0 = lerp/slerp from prev).
    void sync(const PhysicsScene& physics_scene, float alpha = 1.0f) noexcept;

private:
    std::unordered_map<uint32_t, BodyRenderBinding> m_bindings;
};

} // namespace nisaba::render3d

#pragma once

#include "nisaba/physics/body/body.hpp"
#include "nisaba/physics/collision/contact_manifold.hpp"
#include "nisaba/physics/collision/collision_combine.hpp"
#include "nisaba/physics/collision/sphere_shape.hpp"
#include "nisaba/physics/collision/box_shape.hpp"
#include "nisaba/physics/collision/plane_shape.hpp"
#include "nisaba/physics/collision/capsule_shape.hpp"

namespace nisaba::physics {

/// @brief Narrow-phase collision detection engine.
/// Provides high-precision analytical contact generation for primitive shapes.
class NarrowPhase {
public:
    /// @brief Sphere vs Sphere analytical collision
    static bool collide_sphere_vs_sphere(
        const Body& body_a,
        const SphereShape& sphere_a,
        const Body& body_b,
        const SphereShape& sphere_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief Sphere vs Plane analytical collision
    static bool collide_sphere_vs_plane(
        const Body& body_a,
        const SphereShape& sphere_a,
        const Body& body_b,
        const PlaneShape& plane_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief Sphere vs Box analytical collision
    static bool collide_sphere_vs_box(
        const Body& body_a,
        const SphereShape& sphere_a,
        const Body& body_b,
        const BoxShape& box_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief Box vs Plane analytical collision
    static bool collide_box_vs_plane(
        const Body& body_a,
        const BoxShape& box_a,
        const Body& body_b,
        const PlaneShape& plane_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief Box vs Box analytical OBB Separating Axis Theorem (SAT) collision
    static bool collide_box_vs_box(
        const Body& body_a,
        const BoxShape& box_a,
        const Body& body_b,
        const BoxShape& box_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief Capsule vs Plane analytical collision
    static bool collide_capsule_vs_plane(
        const Body& body_a,
        const CapsuleShape& capsule_a,
        const Body& body_b,
        const PlaneShape& plane_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief Sphere vs Capsule analytical collision
    static bool collide_sphere_vs_capsule(
        const Body& body_a,
        const SphereShape& sphere_a,
        const Body& body_b,
        const CapsuleShape& capsule_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief General convex vs convex collision using GJK and EPA
    static bool collide_convex_vs_convex(
        const Body& body_a,
        const Body& body_b,
        ContactManifold& out_manifold
    ) noexcept;

    /// @brief General dispatcher: Tests collision between any two bodies with analytical shapes
    static bool collide_bodies(
        const Body& body_a,
        const Body& body_b,
        ContactManifold& out_manifold
    ) noexcept;
};

} // namespace nisaba::physics

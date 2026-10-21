#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/physics/math/aabb.hpp"
#include "nisaba/physics/math/transform3d.hpp"
#include "nisaba/physics/collision/mass_properties.hpp"
#include "nisaba/physics/math/ray.hpp"
#include <cstdint>

namespace nisaba::physics {

/// @brief Raycast hit information for collision shapes and broad-phase queries.
struct RayCastHit {
    Vec3 point = Vec3::zero();       ///< Hit point in local coordinates
    Vec3 normal = Vec3::up();        ///< Surface normal at hit point in local coordinates
    float fraction = 0.0f;           ///< Distance along ray to hit point
    bool has_hit = false;            ///< True if an intersection occurred
};

enum class ShapeType : uint8_t {
    Sphere = 0,
    Box = 1,
    Capsule = 2,
    Plane = 3,
    Cylinder = 4,
    ConvexHull = 5
};

/// @brief Polymorphic base class for all geometric collision shapes.
/// Provides analytical bounds, mass properties, and support functions for narrow-phase collision.
class Shape {
public:
    virtual ~Shape() = default;

    /// @brief Fast RTTI identifier without dynamic_cast
    [[nodiscard]] virtual ShapeType get_type() const noexcept = 0;

    /// @brief Casts a ray against the shape in its local space.
    /// @param local_ray The ray in shape local coordinates.
    /// @param max_distance Maximum ray travel distance.
    /// @param out_hit Populated with local hit point, normal, and distance if hit occurs.
    /// @return True if ray intersects the shape within max_distance, false otherwise.
    [[nodiscard]] virtual bool cast_ray(
        const Ray& local_ray,
        float max_distance,
        RayCastHit& out_hit
    ) const noexcept = 0;

    /// @brief Local axis-aligned bounding box of the shape in its own reference frame
    [[nodiscard]] virtual AABB get_local_bounds() const noexcept = 0;

    /// @brief World axis-aligned bounding box under a rigid transform
    [[nodiscard]] virtual AABB get_world_bounds(const Transform3D& transform) const noexcept {
        return get_local_bounds().transform(transform);
    }

    /// @brief Computes analytical mass, principal moments of inertia, and center of mass
    /// @param density Material density in kg/m^3 (default 1000 kg/m^3 for water/standard objects)
    [[nodiscard]] virtual MassProperties get_mass_properties(float density = 1000.0f) const noexcept = 0;

    /// @brief Support mapping function S(v) = argmax_{p in Shape} (p . v)
    /// Core primitive for GJK, EPA, and analytical contact generation.
    [[nodiscard]] virtual Vec3 get_support(const Vec3& direction) const noexcept = 0;

    /// @brief Inner radius of the shape for conservative culling and tunneling prevention
    [[nodiscard]] virtual float get_inner_radius() const noexcept = 0;

    /// @brief Whether this shape is convex (all analytical primitives in Phase 2 are convex)
    [[nodiscard]] virtual bool is_convex() const noexcept {
        return true;
    }
};

} // namespace nisaba::physics

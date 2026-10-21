#pragma once

#include "nisaba/physics/collision/contact_point.hpp"
#include "nisaba/physics/body/body_id.hpp"
#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace nisaba::physics {

/// @brief Represents the contact manifold surface between two colliding bodies.
/// Caps contact points at 4 (a quad), which provides maximal rotational stability
/// without redundant constraints or solver instability.
struct ContactManifold {
    static constexpr size_t MAX_POINTS = 4;

    BodyID body_a;
    BodyID body_b;
    Vec3 normal = Vec3::unit_y(); ///< Normal pointing from Body A to Body B
    float combined_friction = 0.0f;
    float combined_restitution = 0.0f;

    ContactPoint points[MAX_POINTS];
    uint32_t num_points = 0;

    constexpr ContactManifold() noexcept = default;

    ContactManifold(BodyID in_a, BodyID in_b, const Vec3& in_normal = Vec3::unit_y()) noexcept
        : body_a(in_a), body_b(in_b), normal(in_normal) {}

    [[nodiscard]] bool is_empty() const noexcept {
        return num_points == 0;
    }

    void clear() noexcept {
        num_points = 0;
    }

    /// @brief Add a contact point to the manifold. If capacity is exceeded,
    /// replaces the least significant point while maximizing quadrilateral area.
    void add_point(const ContactPoint& pt) noexcept {
        if (num_points < MAX_POINTS) {
            points[num_points++] = pt;
            return;
        }

        // Capacity full: buffer all 5 points and prune down to 4 maximal area points
        ContactPoint temp[MAX_POINTS + 1];
        for (size_t i = 0; i < MAX_POINTS; ++i) {
            temp[i] = points[i];
        }
        temp[MAX_POINTS] = pt;

        prune_points_internal(temp, MAX_POINTS + 1);
    }

    /// @brief Prune an arbitrary set of candidate points down to at most 4 maximal area points.
    void set_points_pruned(const ContactPoint* candidates, size_t count) noexcept {
        if (count <= MAX_POINTS) {
            num_points = static_cast<uint32_t>(count);
            for (size_t i = 0; i < count; ++i) {
                points[i] = candidates[i];
            }
            return;
        }

        prune_points_internal(candidates, count);
    }

    /// @brief Swaps bodies A and B, inverting the normal and swapping contact endpoints
    [[nodiscard]] ContactManifold swap_bodies() const noexcept {
        ContactManifold swapped(body_b, body_a, -normal);
        swapped.combined_friction = combined_friction;
        swapped.combined_restitution = combined_restitution;
        swapped.num_points = num_points;

        for (uint32_t i = 0; i < num_points; ++i) {
            swapped.points[i] = ContactPoint(
                points[i].position_on_b,
                points[i].position_on_a,
                -points[i].normal,
                points[i].penetration_depth,
                points[i].feature_id
            );
            swapped.points[i].normal_impulse = points[i].normal_impulse;
            swapped.points[i].tangent_impulse[0] = points[i].tangent_impulse[0];
            swapped.points[i].tangent_impulse[1] = points[i].tangent_impulse[1];
        }
        return swapped;
    }

private:
    /// @brief Selects 4 points that maximize the 2D projected contact polygon area.
    void prune_points_internal(const ContactPoint* candidates, size_t count) noexcept {
        if (count == 0) {
            num_points = 0;
            return;
        }

        // 1. Point 1: Deepest penetration point
        size_t p1 = 0;
        float max_depth = candidates[0].penetration_depth;
        for (size_t i = 1; i < count; ++i) {
            if (candidates[i].penetration_depth > max_depth) {
                max_depth = candidates[i].penetration_depth;
                p1 = i;
            }
        }

        // 2. Point 2: Furthest point from Point 1 in the contact plane
        size_t p2 = p1;
        float max_dist_sq = -1.0f;
        for (size_t i = 0; i < count; ++i) {
            if (i == p1) continue;
            const float d_sq = (candidates[i].position_on_a - candidates[p1].position_on_a).length_sq();
            if (d_sq > max_dist_sq) {
                max_dist_sq = d_sq;
                p2 = i;
            }
        }

        if (p2 == p1 || count <= 2) {
            // Degenerate collinear/single point
            num_points = 0;
            points[num_points++] = candidates[p1];
            if (p2 != p1 && count > 1) points[num_points++] = candidates[p2];
            return;
        }

        // Line segment vector from p1 to p2
        const Vec3 seg = candidates[p2].position_on_a - candidates[p1].position_on_a;
        // Direction perpendicular to segment and normal in the contact plane
        const Vec3 perp = seg.cross(normal);

        // 3. Points 3 & 4: Furthest points on positive and negative sides of segment
        size_t p3 = p1;
        size_t p4 = p1;
        float min_proj = 0.0f;
        float max_proj = 0.0f;

        for (size_t i = 0; i < count; ++i) {
            if (i == p1 || i == p2) continue;
            const Vec3 diff = candidates[i].position_on_a - candidates[p1].position_on_a;
            const float proj = perp.dot(diff);
            if (proj < min_proj) {
                min_proj = proj;
                p3 = i;
            } else if (proj > max_proj) {
                max_proj = proj;
                p4 = i;
            }
        }

        num_points = 0;
        points[num_points++] = candidates[p1];
        points[num_points++] = candidates[p2];
        if (p3 != p1) points[num_points++] = candidates[p3];
        if (p4 != p1 && p4 != p3) points[num_points++] = candidates[p4];
    }
};

} // namespace nisaba::physics

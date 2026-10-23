#pragma once

#include "orion/physics/collision/shape.hpp"
#include <vector>
#include <algorithm>
#include <cassert>

namespace orion::physics {

/// @brief Arbitrary 3D convex hull shape defined by a point cloud / vertex set.
/// Evaluates extreme points via support mapping for GJK/EPA narrow-phase queries.
class ConvexHullShape final : public Shape {
public:
    std::vector<Vec3> vertices;
    AABB local_bounds;
    Vec3 centroid{0.0f, 0.0f, 0.0f};
    struct FacePlane {
        Vec3 normal;
        float distance = 0.0f; // normal . point = distance
    };
    std::vector<FacePlane> face_planes;

    explicit ConvexHullShape(std::vector<Vec3> in_vertices)
        : vertices(std::move(in_vertices)) {
        recompute_bounds();
    }

    [[nodiscard]] ShapeType get_type() const noexcept override {
        return ShapeType::ConvexHull;
    }

    [[nodiscard]] bool cast_ray(
        const Ray& local_ray,
        float max_distance,
        RayCastHit& out_hit
    ) const noexcept override {
        if (face_planes.empty()) {
            float t0 = 0.0f;
            float t1 = max_distance;
            if (local_bounds.raycast(local_ray.origin, local_ray.direction, max_distance, t0, t1)) {
                out_hit.fraction = t0;
                out_hit.point = local_ray.point_at(t0);
                out_hit.normal = Vec3::up();
                out_hit.has_hit = true;
                return true;
            }
            return false;
        }

        float t_enter = 0.0f;
        float t_exit = max_distance;
        Vec3 enter_normal = Vec3::up();

        for (const auto& plane : face_planes) {
            const float denom = plane.normal.dot(local_ray.direction);
            const float dist = plane.distance - plane.normal.dot(local_ray.origin);

            if (std::abs(denom) < EPSILON) {
                if (dist < 0.0f) {
                    return false; // Ray is parallel and outside this face plane
                }
            } else if (denom > 0.0f) {
                const float t = dist / denom;
                t_exit = std::min(t_exit, t);
            } else {
                const float t = dist / denom;
                if (t > t_enter) {
                    t_enter = t;
                    enter_normal = plane.normal;
                }
            }

            if (t_enter > t_exit) {
                return false;
            }
        }

        if (t_enter >= 0.0f && t_enter <= max_distance) {
            out_hit.fraction = t_enter;
            out_hit.point = local_ray.point_at(t_enter);
            out_hit.normal = enter_normal;
            out_hit.has_hit = true;
            return true;
        }

        return false;
    }

    [[nodiscard]] AABB get_local_bounds() const noexcept override {
        return local_bounds;
    }

    [[nodiscard]] MassProperties get_mass_properties(float density = 1000.0f) const noexcept override {
        MassProperties mp;
        const Vec3 extents = local_bounds.extent();
        // Convex hull volume approximated conservatively as ~60% of bounding box
        const float bbox_vol = extents.x * extents.y * extents.z;
        const float volume = std::max(bbox_vol * 0.6f, EPSILON);
        mp.mass = density * volume;

        const float c = (1.0f / 12.0f) * mp.mass;
        const float ex2 = extents.x * extents.x;
        const float ey2 = extents.y * extents.y;
        const float ez2 = extents.z * extents.z;

        mp.inertia_diagonal = Vec3(
            c * (ey2 + ez2),
            c * (ex2 + ez2),
            c * (ex2 + ey2)
        );
        mp.inertia_rotation = Quat::identity();
        mp.center_of_mass = centroid;
        return mp;
    }

    [[nodiscard]] Vec3 get_support(const Vec3& direction) const noexcept override {
        if (vertices.empty()) {
            return Vec3::zero();
        }

        Vec3 best_vertex = vertices[0];
        float best_dot = best_vertex.dot(direction);

        for (size_t i = 1; i < vertices.size(); ++i) {
            const float d = vertices[i].dot(direction);
            if (d > best_dot) {
                best_dot = d;
                best_vertex = vertices[i];
            }
        }
        return best_vertex;
    }

    [[nodiscard]] float get_inner_radius() const noexcept override {
        const Vec3 half_extents = local_bounds.half_extent();
        return std::min({half_extents.x, half_extents.y, half_extents.z});
    }

private:
    void recompute_bounds() noexcept {
        if (vertices.empty()) {
            local_bounds = AABB(Vec3(-EPSILON, -EPSILON, -EPSILON), Vec3(EPSILON, EPSILON, EPSILON));
            centroid = Vec3::zero();
            return;
        }

        Vec3 min_v = vertices[0];
        Vec3 max_v = vertices[0];
        Vec3 sum = vertices[0];

        for (size_t i = 1; i < vertices.size(); ++i) {
            min_v = min_v.min(vertices[i]);
            max_v = max_v.max(vertices[i]);
            sum = sum + vertices[i];
        }

        local_bounds = AABB(min_v, max_v);
        centroid = sum * (1.0f / static_cast<float>(vertices.size()));

        face_planes.clear();
        const size_t n_verts = vertices.size();
        if (n_verts >= 3 && n_verts <= 64) {
            for (size_t i = 0; i < n_verts; ++i) {
                for (size_t j = i + 1; j < n_verts; ++j) {
                    for (size_t k = j + 1; k < n_verts; ++k) {
                        const Vec3 e1 = vertices[j] - vertices[i];
                        const Vec3 e2 = vertices[k] - vertices[i];
                        Vec3 n = e1.cross(e2);
                        if (n.length_sq() < 1e-6f) continue;
                        n = n.normalized();
                        if (n.dot(centroid - vertices[i]) > 0.0f) {
                            n = -n;
                        }
                        const float d = n.dot(vertices[i]);

                        bool is_hull_face = true;
                        for (size_t m = 0; m < n_verts; ++m) {
                            if (n.dot(vertices[m]) > d + 1e-3f) {
                                is_hull_face = false;
                                break;
                            }
                        }

                        if (is_hull_face) {
                            bool duplicate = false;
                            for (const auto& existing : face_planes) {
                                if (existing.normal.dot(n) > 0.999f && std::abs(existing.distance - d) < 1e-3f) {
                                    duplicate = true;
                                    break;
                                }
                            }
                            if (!duplicate) {
                                face_planes.push_back({n, d});
                            }
                        }
                    }
                }
            }
        }
    }
};

} // namespace orion::physics

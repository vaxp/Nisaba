#include "orion/physics/collision/narrow_phase.hpp"
#include "orion/physics/collision/gjk.hpp"
#include "orion/physics/collision/epa.hpp"
#include <cmath>

namespace orion::physics {

bool NarrowPhase::collide_sphere_vs_sphere(
    const Body& body_a,
    const SphereShape& sphere_a,
    const Body& body_b,
    const SphereShape& sphere_b,
    ContactManifold& out_manifold
) noexcept {
    // Fast AABB overlap rejection
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Vec3 pos_a = body_a.get_position();
    const Vec3 pos_b = body_b.get_position();
    const Vec3 delta = pos_b - pos_a;
    const float dist_sq = delta.length_sq();
    const float radius_sum = sphere_a.radius + sphere_b.radius;

    if (dist_sq > radius_sum * radius_sum) {
        return false;
    }

    Vec3 normal;
    float depth;
    if (dist_sq > EPSILON * EPSILON) {
        const float dist = std::sqrt(dist_sq);
        normal = delta / dist;
        depth = radius_sum - dist;
    } else {
        // Degenerate concentric spheres: choose arbitrary vertical separation
        normal = Vec3::unit_y();
        depth = radius_sum;
    }

    const Vec3 pt_on_a = pos_a + normal * sphere_a.radius;
    const Vec3 pt_on_b = pos_b - normal * sphere_b.radius;

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), normal);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.add_point(ContactPoint(pt_on_a, pt_on_b, normal, depth, 0));

    return true;
}

bool NarrowPhase::collide_sphere_vs_plane(
    const Body& body_a,
    const SphereShape& sphere_a,
    const Body& body_b,
    const PlaneShape& plane_b,
    ContactManifold& out_manifold
) noexcept {
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_plane = body_b.get_transform();
    // Plane local normal points +Y by convention
    const Vec3 plane_normal_world = t_plane.rotation.rotate_vec(plane_b.plane.normal).normalized();
    const Vec3 plane_point_world = t_plane.transform_point(plane_b.plane.normal * plane_b.plane.distance);

    const Vec3 sphere_pos = body_a.get_position();
    const float signed_dist = plane_normal_world.dot(sphere_pos - plane_point_world);

    if (signed_dist > sphere_a.radius) {
        return false;
    }

    const float depth = sphere_a.radius - signed_dist;
    // Normal points from Body A (sphere) into Body B (plane)
    const Vec3 normal = -plane_normal_world;

    const Vec3 pt_on_a = sphere_pos + normal * sphere_a.radius;
    const Vec3 pt_on_b = sphere_pos - plane_normal_world * signed_dist;

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), normal);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.add_point(ContactPoint(pt_on_a, pt_on_b, normal, depth, 0));

    return true;
}

bool NarrowPhase::collide_sphere_vs_box(
    const Body& body_a,
    const SphereShape& sphere_a,
    const Body& body_b,
    const BoxShape& box_b,
    ContactManifold& out_manifold
) noexcept {
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_box = body_b.get_transform();
    const Vec3 sphere_pos_local = t_box.inverse_transform_point(body_a.get_position());
    const Vec3 h = box_b.half_extents;

    // Closest point on box in local space
    const Vec3 closest_local(
        std::clamp(sphere_pos_local.x, -h.x, h.x),
        std::clamp(sphere_pos_local.y, -h.y, h.y),
        std::clamp(sphere_pos_local.z, -h.z, h.z)
    );

    const Vec3 delta_local = sphere_pos_local - closest_local;
    const float dist_sq = delta_local.length_sq();

    Vec3 normal_local;
    float depth = 0.0f;
    Vec3 pt_box_local = closest_local;

    if (dist_sq > EPSILON * EPSILON) {
        // Sphere center is OUTSIDE box
        if (dist_sq > sphere_a.radius * sphere_a.radius) {
            return false;
        }
        const float dist = std::sqrt(dist_sq);
        // Normal from box to sphere in local space
        const Vec3 n_box_to_sphere = delta_local / dist;
        // Normal pointing from Body A (sphere) to Body B (box)
        normal_local = -n_box_to_sphere;
        depth = sphere_a.radius - dist;
    } else {
        // Sphere center is INSIDE box: find closest face to push out along
        const float dx = h.x - std::abs(sphere_pos_local.x);
        const float dy = h.y - std::abs(sphere_pos_local.y);
        const float dz = h.z - std::abs(sphere_pos_local.z);

        if (dx <= dy && dx <= dz) {
            const float sign = sphere_pos_local.x >= 0.0f ? 1.0f : -1.0f;
            normal_local = Vec3(-sign, 0.0f, 0.0f);
            depth = sphere_a.radius + dx;
            pt_box_local.x = sign * h.x;
        } else if (dy <= dz) {
            const float sign = sphere_pos_local.y >= 0.0f ? 1.0f : -1.0f;
            normal_local = Vec3(0.0f, -sign, 0.0f);
            depth = sphere_a.radius + dy;
            pt_box_local.y = sign * h.y;
        } else {
            const float sign = sphere_pos_local.z >= 0.0f ? 1.0f : -1.0f;
            normal_local = Vec3(0.0f, 0.0f, -sign);
            depth = sphere_a.radius + dz;
            pt_box_local.z = sign * h.z;
        }
    }

    const Vec3 normal_world = t_box.transform_vector(normal_local).normalized();
    const Vec3 pt_on_b = t_box.transform_point(pt_box_local);
    const Vec3 pt_on_a = pt_on_b - normal_world * depth;

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), normal_world);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.add_point(ContactPoint(pt_on_a, pt_on_b, normal_world, depth, 0));

    return true;
}

bool NarrowPhase::collide_box_vs_plane(
    const Body& body_a,
    const BoxShape& box_a,
    const Body& body_b,
    const PlaneShape& plane_b,
    ContactManifold& out_manifold
) noexcept {
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_box = body_a.get_transform();
    const Transform3D t_plane = body_b.get_transform();

    const Vec3 plane_normal_world = t_plane.rotation.rotate_vec(plane_b.plane.normal).normalized();
    const Vec3 plane_point_world = t_plane.transform_point(plane_b.plane.normal * plane_b.plane.distance);

    // Normal points from Body A (box) into Body B (plane)
    const Vec3 normal = -plane_normal_world;

    const Vec3 h = box_a.half_extents;
    const Vec3 local_corners[8] = {
        Vec3(-h.x, -h.y, -h.z),
        Vec3( h.x, -h.y, -h.z),
        Vec3(-h.x,  h.y, -h.z),
        Vec3( h.x,  h.y, -h.z),
        Vec3(-h.x, -h.y,  h.z),
        Vec3( h.x, -h.y,  h.z),
        Vec3(-h.x,  h.y,  h.z),
        Vec3( h.x,  h.y,  h.z)
    };

    ContactPoint candidates[8];
    size_t candidate_count = 0;

    for (size_t i = 0; i < 8; ++i) {
        const Vec3 corner_world = t_box.transform_point(local_corners[i]);
        const float signed_dist = plane_normal_world.dot(corner_world - plane_point_world);

        if (signed_dist <= EPSILON) {
            const float depth = -signed_dist;
            const Vec3 pt_on_b = corner_world - plane_normal_world * signed_dist;
            candidates[candidate_count++] = ContactPoint(
                corner_world,
                pt_on_b,
                normal,
                depth,
                static_cast<uint32_t>(i)
            );
        }
    }

    if (candidate_count == 0) {
        return false;
    }

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), normal);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.set_points_pruned(candidates, candidate_count);

    return true;
}

static size_t clip_polygon_against_plane(
    const Vec3* in_pts,
    size_t in_count,
    const Vec3& plane_normal,
    float plane_d,
    Vec3* out_pts
) noexcept {
    if (in_count == 0) return 0;
    size_t out_count = 0;

    Vec3 s = in_pts[in_count - 1];
    float s_dist = plane_normal.dot(s) - plane_d;

    for (size_t i = 0; i < in_count; ++i) {
        Vec3 p = in_pts[i];
        float p_dist = plane_normal.dot(p) - plane_d;

        if (p_dist <= 0.0f) {
            if (s_dist > 0.0f) {
                float denom = s_dist - p_dist;
                float t = denom > EPSILON ? s_dist / denom : 0.0f;
                if (out_count < 16) {
                    out_pts[out_count++] = s + (p - s) * t;
                }
            }
            if (out_count < 16) {
                out_pts[out_count++] = p;
            }
        } else {
            if (s_dist <= 0.0f) {
                float denom = s_dist - p_dist;
                float t = denom > EPSILON ? s_dist / denom : 0.0f;
                if (out_count < 16) {
                    out_pts[out_count++] = s + (p - s) * t;
                }
            }
        }
        s = p;
        s_dist = p_dist;
    }
    return out_count;
}

bool NarrowPhase::collide_box_vs_box(
    const Body& body_a,
    const BoxShape& box_a,
    const Body& body_b,
    const BoxShape& box_b,
    ContactManifold& out_manifold
) noexcept {
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_a = body_a.get_transform();
    const Transform3D t_b = body_b.get_transform();

    const Vec3 pos_a = t_a.position;
    const Vec3 pos_b = t_b.position;
    const Vec3 delta = pos_b - pos_a;

    // Local axes in world coordinates
    const Vec3 u_a[3] = {
        t_a.rotation.rotate_vec(Vec3::unit_x()),
        t_a.rotation.rotate_vec(Vec3::unit_y()),
        t_a.rotation.rotate_vec(Vec3::unit_z())
    };
    const Vec3 u_b[3] = {
        t_b.rotation.rotate_vec(Vec3::unit_x()),
        t_b.rotation.rotate_vec(Vec3::unit_y()),
        t_b.rotation.rotate_vec(Vec3::unit_z())
    };

    const Vec3 h_a = box_a.half_extents;
    const Vec3 h_b = box_b.half_extents;

    float min_overlap = 1e30f;
    Vec3 best_axis = Vec3::unit_y();
    int best_axis_id = 1; // Default to Y axis

    auto test_axis = [&](const Vec3& axis, int id, float bias = 1.0f) -> bool {
        const float l_sq = axis.length_sq();
        if (l_sq < 1e-6f) return true; // Degenerate axis, skip

        const Vec3 l = axis / std::sqrt(l_sq);

        const float ra = h_a.x * std::abs(l.dot(u_a[0])) +
                         h_a.y * std::abs(l.dot(u_a[1])) +
                         h_a.z * std::abs(l.dot(u_a[2]));

        const float rb = h_b.x * std::abs(l.dot(u_b[0])) +
                         h_b.y * std::abs(l.dot(u_b[1])) +
                         h_b.z * std::abs(l.dot(u_b[2]));

        const float s = std::abs(delta.dot(l));
        const float overlap = (ra + rb) - s;

        if (overlap <= 0.0f) {
            return false; // Separating axis exists!
        }

        const float biased_overlap = overlap * bias;
        if (biased_overlap < min_overlap) {
            min_overlap = biased_overlap;
            best_axis = l;
            best_axis_id = id;
        }
        return true;
    };

    // 1. Box A Face Axes
    if (!test_axis(u_a[0], 0)) return false;
    if (!test_axis(u_a[1], 1)) return false;
    if (!test_axis(u_a[2], 2)) return false;

    // 2. Box B Face Axes
    if (!test_axis(u_b[0], 3)) return false;
    if (!test_axis(u_b[1], 4)) return false;
    if (!test_axis(u_b[2], 5)) return false;

    // 3. Edge-Edge Cross Axes (bias edge axes with 1.05 to prefer face contacts)
    const float edge_bias = 1.05f;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            if (!test_axis(u_a[i].cross(u_b[j]), 6 + i * 3 + j, edge_bias)) {
                return false;
            }
        }
    }

    // Ensure best_axis points from Box A towards Box B
    if (delta.dot(best_axis) < 0.0f) {
        best_axis = -best_axis;
    }

    // Decide Reference and Incident bodies
    bool a_is_reference = true;
    if (best_axis_id >= 3 && best_axis_id < 6) {
        a_is_reference = false;
    } else if (best_axis_id >= 6) {
        float max_dot_a = 0.0f;
        for (int i = 0; i < 3; ++i) {
            max_dot_a = std::max(max_dot_a, std::abs(u_a[i].dot(best_axis)));
        }
        float max_dot_b = 0.0f;
        for (int i = 0; i < 3; ++i) {
            max_dot_b = std::max(max_dot_b, std::abs(u_b[i].dot(best_axis)));
        }
        a_is_reference = (max_dot_a >= max_dot_b);
    }

    const Vec3* u_ref = a_is_reference ? u_a : u_b;
    const Vec3* u_inc = a_is_reference ? u_b : u_a;
    const Vec3 h_ref = a_is_reference ? h_a : h_b;
    const Vec3 h_inc = a_is_reference ? h_b : h_a;
    const Vec3 pos_ref = a_is_reference ? pos_a : pos_b;
    const Vec3 pos_inc = a_is_reference ? pos_b : pos_a;

    // Normal pointing OUT of reference body towards incident body
    const Vec3 n_ref = a_is_reference ? best_axis : -best_axis;

    // Find reference face axis (most aligned with n_ref)
    int ref_axis = 0;
    float best_ref_dot = -1.0f;
    float ref_sign = 1.0f;
    for (int i = 0; i < 3; ++i) {
        float d = u_ref[i].dot(n_ref);
        if (std::abs(d) > best_ref_dot) {
            best_ref_dot = std::abs(d);
            ref_axis = i;
            ref_sign = d >= 0.0f ? 1.0f : -1.0f;
        }
    }
    const Vec3 ref_normal = u_ref[ref_axis] * ref_sign;
    const float h_ref_val = ref_axis == 0 ? h_ref.x : (ref_axis == 1 ? h_ref.y : h_ref.z);
    const Vec3 ref_center = pos_ref + ref_normal * h_ref_val;

    // Find incident face (outward normal MOST OPPOSITE to ref_normal)
    int inc_axis = 0;
    float best_inc_dot = 1e30f;
    float inc_sign = 1.0f;
    for (int i = 0; i < 3; ++i) {
        float d1 = u_inc[i].dot(ref_normal);
        if (d1 < best_inc_dot) {
            best_inc_dot = d1;
            inc_axis = i;
            inc_sign = 1.0f;
        }
        float d2 = (-u_inc[i]).dot(ref_normal);
        if (d2 < best_inc_dot) {
            best_inc_dot = d2;
            inc_axis = i;
            inc_sign = -1.0f;
        }
    }

    const Vec3 inc_normal = u_inc[inc_axis] * inc_sign;
    const float h_inc_val = inc_axis == 0 ? h_inc.x : (inc_axis == 1 ? h_inc.y : h_inc.z);
    const Vec3 inc_center = pos_inc + inc_normal * h_inc_val;

    const int inc_a = (inc_axis + 1) % 3;
    const int inc_b = (inc_axis + 2) % 3;
    const float h_inc_a = inc_a == 0 ? h_inc.x : (inc_a == 1 ? h_inc.y : h_inc.z);
    const float h_inc_b = inc_b == 0 ? h_inc.x : (inc_b == 1 ? h_inc.y : h_inc.z);

    // 4 corners of incident face in CCW order
    Vec3 poly[16];
    poly[0] = inc_center - u_inc[inc_a] * h_inc_a - u_inc[inc_b] * h_inc_b;
    poly[1] = inc_center + u_inc[inc_a] * h_inc_a - u_inc[inc_b] * h_inc_b;
    poly[2] = inc_center + u_inc[inc_a] * h_inc_a + u_inc[inc_b] * h_inc_b;
    poly[3] = inc_center - u_inc[inc_a] * h_inc_a + u_inc[inc_b] * h_inc_b;
    size_t poly_count = 4;

    // Clip against the 4 side planes of the reference face
    const int ref_a = (ref_axis + 1) % 3;
    const int ref_b = (ref_axis + 2) % 3;
    const float h_ref_a = ref_a == 0 ? h_ref.x : (ref_a == 1 ? h_ref.y : h_ref.z);
    const float h_ref_b = ref_b == 0 ? h_ref.x : (ref_b == 1 ? h_ref.y : h_ref.z);

    const struct { Vec3 n; float d; } side_planes[4] = {
        {  u_ref[ref_a],  u_ref[ref_a].dot(pos_ref) + h_ref_a },
        { -u_ref[ref_a], -u_ref[ref_a].dot(pos_ref) + h_ref_a },
        {  u_ref[ref_b],  u_ref[ref_b].dot(pos_ref) + h_ref_b },
        { -u_ref[ref_b], -u_ref[ref_b].dot(pos_ref) + h_ref_b }
    };

    Vec3 clipped[16];
    for (int p = 0; p < 4; ++p) {
        poly_count = clip_polygon_against_plane(poly, poly_count, side_planes[p].n, side_planes[p].d, clipped);
        for (size_t k = 0; k < poly_count; ++k) poly[k] = clipped[k];
    }

    // Keep points behind the reference face
    ContactPoint candidates[16];
    size_t candidate_count = 0;

    for (size_t i = 0; i < poly_count; ++i) {
        const float dist = ref_normal.dot(poly[i] - ref_center);
        if (dist <= 0.0f) {
            const float depth = -dist;
            const Vec3 pt_on_ref = poly[i] - ref_normal * dist;
            const Vec3 pt_on_inc = poly[i];

            const Vec3 pt_a = a_is_reference ? pt_on_ref : pt_on_inc;
            const Vec3 pt_b = a_is_reference ? pt_on_inc : pt_on_ref;

            candidates[candidate_count++] = ContactPoint(
                pt_a,
                pt_b,
                best_axis, // Points from A to B
                depth,
                static_cast<uint32_t>(i)
            );
        }
    }

    if (candidate_count == 0) {
        const float dist = ref_normal.dot(inc_center - ref_center);
        const float depth = std::max(0.0f, -dist);
        const Vec3 pt_on_ref = inc_center - ref_normal * dist;
        const Vec3 pt_a = a_is_reference ? pt_on_ref : inc_center;
        const Vec3 pt_b = a_is_reference ? inc_center : pt_on_ref;
        candidates[candidate_count++] = ContactPoint(pt_a, pt_b, best_axis, depth, 0);
    }

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), best_axis);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.set_points_pruned(candidates, candidate_count);

    return true;
}

bool NarrowPhase::collide_capsule_vs_plane(
    const Body& body_a,
    const CapsuleShape& capsule_a,
    const Body& body_b,
    const PlaneShape& plane_b,
    ContactManifold& out_manifold
) noexcept {
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_cap = body_a.get_transform();
    const Transform3D t_plane = body_b.get_transform();

    const Vec3 plane_normal_world = t_plane.rotation.rotate_vec(plane_b.plane.normal).normalized();
    const Vec3 plane_point_world = t_plane.transform_point(plane_b.plane.normal * plane_b.plane.distance);

    // Normal points from Body A (capsule) into Body B (plane)
    const Vec3 normal = -plane_normal_world;

    const float h = capsule_a.half_height;
    const float r = capsule_a.radius;

    const Vec3 local_ends[2] = {
        Vec3(0.0f, -h, 0.0f),
        Vec3(0.0f,  h, 0.0f)
    };

    ContactPoint candidates[2];
    size_t candidate_count = 0;

    for (size_t i = 0; i < 2; ++i) {
        const Vec3 end_world = t_cap.transform_point(local_ends[i]);
        const float signed_dist = plane_normal_world.dot(end_world - plane_point_world);

        if (signed_dist <= r) {
            const float depth = r - signed_dist;
            const Vec3 pt_on_a = end_world - plane_normal_world * r;
            const Vec3 pt_on_b = end_world - plane_normal_world * signed_dist;
            candidates[candidate_count++] = ContactPoint(
                pt_on_a,
                pt_on_b,
                normal,
                depth,
                static_cast<uint32_t>(i)
            );
        }
    }

    if (candidate_count == 0) {
        return false;
    }

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), normal);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    for (size_t i = 0; i < candidate_count; ++i) {
        out_manifold.add_point(candidates[i]);
    }

    return true;
}

bool NarrowPhase::collide_sphere_vs_capsule(
    const Body& body_a,
    const SphereShape& sphere_a,
    const Body& body_b,
    const CapsuleShape& capsule_b,
    ContactManifold& out_manifold
) noexcept {
    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_cap = body_b.get_transform();
    const Vec3 sphere_pos_local = t_cap.inverse_transform_point(body_a.get_position());

    const float h = capsule_b.half_height;
    const Vec3 closest_segment_local(0.0f, std::clamp(sphere_pos_local.y, -h, h), 0.0f);

    const Vec3 delta_local = closest_segment_local - sphere_pos_local;
    const float dist_sq = delta_local.length_sq();
    const float radius_sum = sphere_a.radius + capsule_b.radius;

    if (dist_sq > radius_sum * radius_sum) {
        return false;
    }

    Vec3 normal_local;
    float depth = 0.0f;

    if (dist_sq > EPSILON * EPSILON) {
        const float dist = std::sqrt(dist_sq);
        normal_local = delta_local / dist;
        depth = radius_sum - dist;
    } else {
        normal_local = Vec3(1.0f, 0.0f, 0.0f);
        depth = radius_sum;
    }

    const Vec3 normal_world = t_cap.transform_vector(normal_local).normalized();
    const Vec3 closest_world = t_cap.transform_point(closest_segment_local);
    const Vec3 sphere_pos_world = body_a.get_position();

    const Vec3 pt_on_a = sphere_pos_world + normal_world * sphere_a.radius;
    const Vec3 pt_on_b = closest_world - normal_world * capsule_b.radius;

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), normal_world);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.add_point(ContactPoint(pt_on_a, pt_on_b, normal_world, depth, 0));

    return true;
}

bool NarrowPhase::collide_bodies(
    const Body& body_a,
    const Body& body_b,
    ContactManifold& out_manifold
) noexcept {
    const Shape* s_a = body_a.get_shape();
    const Shape* s_b = body_b.get_shape();
    if (!s_a || !s_b) return false;

    const ShapeType type_a = s_a->get_type();
    const ShapeType type_b = s_b->get_type();

    // 1. Sphere vs Sphere
    if (type_a == ShapeType::Sphere && type_b == ShapeType::Sphere) {
        return collide_sphere_vs_sphere(
            body_a, static_cast<const SphereShape&>(*s_a),
            body_b, static_cast<const SphereShape&>(*s_b),
            out_manifold
        );
    }

    // 2. Sphere vs Plane
    if (type_a == ShapeType::Sphere && type_b == ShapeType::Plane) {
        return collide_sphere_vs_plane(
            body_a, static_cast<const SphereShape&>(*s_a),
            body_b, static_cast<const PlaneShape&>(*s_b),
            out_manifold
        );
    }
    if (type_a == ShapeType::Plane && type_b == ShapeType::Sphere) {
        if (collide_sphere_vs_plane(
            body_b, static_cast<const SphereShape&>(*s_b),
            body_a, static_cast<const PlaneShape&>(*s_a),
            out_manifold
        )) {
            out_manifold = out_manifold.swap_bodies();
            return true;
        }
        return false;
    }

    // 3. Sphere vs Box
    if (type_a == ShapeType::Sphere && type_b == ShapeType::Box) {
        return collide_sphere_vs_box(
            body_a, static_cast<const SphereShape&>(*s_a),
            body_b, static_cast<const BoxShape&>(*s_b),
            out_manifold
        );
    }
    if (type_a == ShapeType::Box && type_b == ShapeType::Sphere) {
        if (collide_sphere_vs_box(
            body_b, static_cast<const SphereShape&>(*s_b),
            body_a, static_cast<const BoxShape&>(*s_a),
            out_manifold
        )) {
            out_manifold = out_manifold.swap_bodies();
            return true;
        }
        return false;
    }

    // 4. Box vs Plane
    if (type_a == ShapeType::Box && type_b == ShapeType::Plane) {
        return collide_box_vs_plane(
            body_a, static_cast<const BoxShape&>(*s_a),
            body_b, static_cast<const PlaneShape&>(*s_b),
            out_manifold
        );
    }
    if (type_a == ShapeType::Plane && type_b == ShapeType::Box) {
        if (collide_box_vs_plane(
            body_b, static_cast<const BoxShape&>(*s_b),
            body_a, static_cast<const PlaneShape&>(*s_a),
            out_manifold
        )) {
            out_manifold = out_manifold.swap_bodies();
            return true;
        }
        return false;
    }

    // 5. Capsule vs Plane
    if (type_a == ShapeType::Capsule && type_b == ShapeType::Plane) {
        return collide_capsule_vs_plane(
            body_a, static_cast<const CapsuleShape&>(*s_a),
            body_b, static_cast<const PlaneShape&>(*s_b),
            out_manifold
        );
    }
    if (type_a == ShapeType::Plane && type_b == ShapeType::Capsule) {
        if (collide_capsule_vs_plane(
            body_b, static_cast<const CapsuleShape&>(*s_b),
            body_a, static_cast<const PlaneShape&>(*s_a),
            out_manifold
        )) {
            out_manifold = out_manifold.swap_bodies();
            return true;
        }
        return false;
    }

    // 6. Sphere vs Capsule
    if (type_a == ShapeType::Sphere && type_b == ShapeType::Capsule) {
        return collide_sphere_vs_capsule(
            body_a, static_cast<const SphereShape&>(*s_a),
            body_b, static_cast<const CapsuleShape&>(*s_b),
            out_manifold
        );
    }
    if (type_a == ShapeType::Capsule && type_b == ShapeType::Sphere) {
        if (collide_sphere_vs_capsule(
            body_b, static_cast<const SphereShape&>(*s_b),
            body_a, static_cast<const CapsuleShape&>(*s_a),
            out_manifold
        )) {
            out_manifold = out_manifold.swap_bodies();
            return true;
        }
        return false;
    }

    // 7. Box vs Box
    if (type_a == ShapeType::Box && type_b == ShapeType::Box) {
        return collide_box_vs_box(
            body_a, static_cast<const BoxShape&>(*s_a),
            body_b, static_cast<const BoxShape&>(*s_b),
            out_manifold
        );
    }

    // 8. General Convex vs Convex via GJK + EPA
    return collide_convex_vs_convex(body_a, body_b, out_manifold);
}

bool NarrowPhase::collide_convex_vs_convex(
    const Body& body_a,
    const Body& body_b,
    ContactManifold& out_manifold
) noexcept {
    const Shape* s_a = body_a.get_shape();
    const Shape* s_b = body_b.get_shape();
    if (!s_a || !s_b) return false;
    if (!s_a->is_convex() || !s_b->is_convex()) return false;
    if (s_a->get_type() == ShapeType::Plane || s_b->get_type() == ShapeType::Plane) return false;

    if (!body_a.get_bounds().overlaps(body_b.get_bounds())) {
        return false;
    }

    const Transform3D t_a = body_a.get_transform();
    const Transform3D t_b = body_b.get_transform();

    const GJKResult gjk = GJK::query(*s_a, t_a, *s_b, t_b);
    if (!gjk.intersecting) {
        return false;
    }

    const EPAResult epa = EPA::query(*s_a, t_a, *s_b, t_b, gjk.simplex);
    if (!epa.success || epa.penetration_depth < EPSILON) {
        return false;
    }

    out_manifold = ContactManifold(body_a.get_id(), body_b.get_id(), epa.normal);
    out_manifold.combined_friction = combine_friction(body_a.get_friction(), body_b.get_friction());
    out_manifold.combined_restitution = combine_restitution(body_a.get_restitution(), body_b.get_restitution());
    out_manifold.add_point(ContactPoint(epa.point_a, epa.point_b, epa.normal, epa.penetration_depth, 0));

    return true;
}

} // namespace orion::physics

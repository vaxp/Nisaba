#include "nisaba/physics/collision/gjk.hpp"
#include <cmath>

namespace nisaba::physics {

namespace {

[[nodiscard]] inline Vec3 pick_orthogonal(const Vec3& v) noexcept {
    if (std::abs(v.x) > 0.5f || std::abs(v.y) > 0.5f) {
        return Vec3(-v.y, v.x, 0.0f).normalized();
    }
    return Vec3(0.0f, -v.z, v.y).normalized();
}

void compute_simplex_closest_points(
    const Simplex& simplex,
    Vec3& out_p_a,
    Vec3& out_p_b,
    Vec3& out_closest
) noexcept {
    if (simplex.count == 1) {
        out_closest = simplex[0].point;
        out_p_a = simplex[0].point_a;
        out_p_b = simplex[0].point_b;
        return;
    }

    if (simplex.count == 2) {
        const SimplexVertex& a = simplex[0];
        const SimplexVertex& b = simplex[1];
        const Vec3 ab = b.point - a.point;
        const float len_sq = ab.length_sq();

        float t = 0.0f;
        if (len_sq > 1e-10f) {
            t = std::clamp(-a.point.dot(ab) / len_sq, 0.0f, 1.0f);
        }
        const float u = 1.0f - t;
        out_closest = a.point * u + b.point * t;
        out_p_a = a.point_a * u + b.point_a * t;
        out_p_b = a.point_b * u + b.point_b * t;
        return;
    }

    if (simplex.count == 3) {
        const SimplexVertex& a = simplex[0];
        const SimplexVertex& b = simplex[1];
        const SimplexVertex& c = simplex[2];

        const Vec3 v0 = b.point - a.point;
        const Vec3 v1 = c.point - a.point;
        const Vec3 n = v0.cross(v1);
        const float n_len_sq = n.length_sq();

        if (n_len_sq > 1e-10f) {
            const float plane_d = a.point.dot(n);
            const Vec3 p = n * (plane_d / n_len_sq);
            const Vec3 v2 = p - a.point;

            const float d00 = v0.dot(v0);
            const float d01 = v0.dot(v1);
            const float d11 = v1.dot(v1);
            const float d20 = v2.dot(v0);
            const float d21 = v2.dot(v1);

            const float denom = d00 * d11 - d01 * d01;
            if (std::abs(denom) > 1e-10f) {
                float v_b = (d11 * d20 - d01 * d21) / denom;
                float w_c = (d00 * d21 - d01 * d20) / denom;
                float u_a = 1.0f - v_b - w_c;

                u_a = std::clamp(u_a, 0.0f, 1.0f);
                v_b = std::clamp(v_b, 0.0f, 1.0f);
                w_c = std::clamp(w_c, 0.0f, 1.0f);
                const float sum = u_a + v_b + w_c;
                if (sum > 1e-6f) {
                    u_a /= sum;
                    v_b /= sum;
                    w_c /= sum;
                }

                out_closest = a.point * u_a + b.point * v_b + c.point * w_c;
                out_p_a = a.point_a * u_a + b.point_a * v_b + c.point_a * w_c;
                out_p_b = a.point_b * u_a + b.point_b * v_b + c.point_b * w_c;
                return;
            }
        }

        const float third = 1.0f / 3.0f;
        out_closest = (a.point + b.point + c.point) * third;
        out_p_a = (a.point_a + b.point_a + c.point_a) * third;
        out_p_b = (a.point_b + b.point_b + c.point_b) * third;
        return;
    }

    // Fallback: 4 vertices (interior of tetrahedron)
    const float quarter = 0.25f;
    out_closest = Vec3::zero();
    out_p_a = (simplex[0].point_a + simplex[1].point_a + simplex[2].point_a + simplex[3].point_a) * quarter;
    out_p_b = (simplex[0].point_b + simplex[1].point_b + simplex[2].point_b + simplex[3].point_b) * quarter;
}

} // namespace

SimplexVertex GJK::support(
    const Shape& shape_a, const Transform3D& transform_a,
    const Shape& shape_b, const Transform3D& transform_b,
    const Vec3& direction
) noexcept {
    const Vec3 local_dir_a = transform_a.inverse_transform_vector(direction);
    const Vec3 local_pt_a = shape_a.get_support(local_dir_a);
    const Vec3 world_pt_a = transform_a.transform_point(local_pt_a);

    const Vec3 local_dir_b = transform_b.inverse_transform_vector(-direction);
    const Vec3 local_pt_b = shape_b.get_support(local_dir_b);
    const Vec3 world_pt_b = transform_b.transform_point(local_pt_b);

    return SimplexVertex{
        world_pt_a - world_pt_b,
        world_pt_a,
        world_pt_b
    };
}

bool GJK::do_simplex2(Simplex& simplex, Vec3& direction) noexcept {
    const SimplexVertex& a = simplex[0];
    const SimplexVertex& b = simplex[1];

    const Vec3 ab = b.point - a.point;
    const Vec3 ao = -a.point;

    if (ab.dot(ao) > 0.0f) {
        Vec3 perp = (ab.cross(ao)).cross(ab);
        if (perp.length_sq() < EPSILON * EPSILON) {
            perp = pick_orthogonal(ab);
        }
        direction = perp;
        return false;
    }

    simplex.count = 1;
    direction = ao;
    return false;
}

bool GJK::do_simplex3(Simplex& simplex, Vec3& direction) noexcept {
    const SimplexVertex& a = simplex[0];
    const SimplexVertex& b = simplex[1];
    const SimplexVertex& c = simplex[2];

    const Vec3 ab = b.point - a.point;
    const Vec3 ac = c.point - a.point;
    const Vec3 ao = -a.point;
    const Vec3 abc = ab.cross(ac);

    // Normal across edge AB pointing away from C
    const Vec3 ab_out = ab.cross(abc);
    if (ab_out.dot(ao) > 0.0f) {
        if (ab.dot(ao) > 0.0f) {
            simplex.count = 2;
            simplex.vertices[0] = a;
            simplex.vertices[1] = b;
            Vec3 perp = (ab.cross(ao)).cross(ab);
            if (perp.length_sq() < EPSILON * EPSILON) perp = ab_out;
            direction = perp;
            return false;
        }
        simplex.count = 1;
        simplex.vertices[0] = a;
        direction = ao;
        return false;
    }

    // Normal across edge AC pointing away from B
    const Vec3 ac_out = abc.cross(ac);
    if (ac_out.dot(ao) > 0.0f) {
        if (ac.dot(ao) > 0.0f) {
            simplex.count = 2;
            simplex.vertices[0] = a;
            simplex.vertices[1] = c;
            Vec3 perp = (ac.cross(ao)).cross(ac);
            if (perp.length_sq() < EPSILON * EPSILON) perp = ac_out;
            direction = perp;
            return false;
        }
        simplex.count = 1;
        simplex.vertices[0] = a;
        direction = ao;
        return false;
    }

    // Inside triangle ABC
    if (abc.dot(ao) > 0.0f) {
        direction = abc;
        return false;
    }

    simplex.vertices[1] = c;
    simplex.vertices[2] = b;
    direction = -abc;
    return false;
}

bool GJK::do_simplex4(Simplex& simplex, Vec3& direction) noexcept {
    const SimplexVertex a = simplex[0];
    const SimplexVertex b = simplex[1];
    const SimplexVertex c = simplex[2];
    const SimplexVertex d = simplex[3];

    const Vec3 ab = b.point - a.point;
    const Vec3 ac = c.point - a.point;
    const Vec3 ad = d.point - a.point;
    const Vec3 ao = -a.point;

    // Face ABC outward normal
    const Vec3 abc = ab.cross(ac);
    const bool abc_inverts = (abc.dot(ad) > 0.0f);
    const Vec3 out_abc = abc_inverts ? -abc : abc;
    if (out_abc.dot(ao) > 1e-5f) {
        simplex.count = 3;
        simplex.vertices[0] = a;
        if (abc_inverts) {
            simplex.vertices[1] = c;
            simplex.vertices[2] = b;
        } else {
            simplex.vertices[1] = b;
            simplex.vertices[2] = c;
        }
        direction = out_abc;
        return false;
    }

    // Face ACD outward normal
    const Vec3 acd = ac.cross(ad);
    const bool acd_inverts = (acd.dot(ab) > 0.0f);
    const Vec3 out_acd = acd_inverts ? -acd : acd;
    if (out_acd.dot(ao) > 1e-5f) {
        simplex.count = 3;
        simplex.vertices[0] = a;
        if (acd_inverts) {
            simplex.vertices[1] = d;
            simplex.vertices[2] = c;
        } else {
            simplex.vertices[1] = c;
            simplex.vertices[2] = d;
        }
        direction = out_acd;
        return false;
    }

    // Face ADB outward normal
    const Vec3 adb = ad.cross(ab);
    const bool adb_inverts = (adb.dot(ac) > 0.0f);
    const Vec3 out_adb = adb_inverts ? -adb : adb;
    if (out_adb.dot(ao) > 1e-5f) {
        simplex.count = 3;
        simplex.vertices[0] = a;
        if (adb_inverts) {
            simplex.vertices[1] = b;
            simplex.vertices[2] = d;
        } else {
            simplex.vertices[1] = d;
            simplex.vertices[2] = b;
        }
        direction = out_adb;
        return false;
    }

    // Origin is inside the tetrahedron!
    return true;
}

bool GJK::update_simplex(Simplex& simplex, Vec3& direction) noexcept {
    switch (simplex.count) {
        case 2: return do_simplex2(simplex, direction);
        case 3: return do_simplex3(simplex, direction);
        case 4: return do_simplex4(simplex, direction);
        default: return false;
    }
}

GJKResult GJK::query(
    const Shape& shape_a, const Transform3D& transform_a,
    const Shape& shape_b, const Transform3D& transform_b,
    uint32_t max_iterations,
    float tolerance
) noexcept {
    GJKResult result;
    result.intersecting = false;

    // Initial search direction from center of A to center of B
    Vec3 initial_dir = transform_b.position - transform_a.position;
    if (initial_dir.length_sq() < EPSILON * EPSILON) {
        initial_dir = Vec3(1.0f, 0.0f, 0.0f);
    }

    Simplex simplex;
    SimplexVertex v0 = support(shape_a, transform_a, shape_b, transform_b, initial_dir);
    simplex.push_front(v0);

    Vec3 closest = v0.point;
    float best_dist = closest.length();
    Vec3 best_closest = closest;
    Simplex best_simplex = simplex;

    for (uint32_t iter = 0; iter < max_iterations; ++iter) {
        result.iterations = iter + 1;

        const float dist_sq = closest.length_sq();
        if (dist_sq < tolerance * tolerance) {
            // Origin is on or enclosed in simplex
            if (simplex.count == 4) {
                result.intersecting = true;
                result.distance = 0.0f;
                result.simplex = simplex;
                return result;
            }
        }

        const float dist = std::sqrt(dist_sq);
        if (dist < best_dist) {
            best_dist = dist;
            best_closest = closest;
            best_simplex = simplex;
        }

        Vec3 direction;
        if (dist >= tolerance) {
            direction = -closest;
        } else {
            // Lower-dimensional simplex containing origin; search orthogonally to expand
            if (simplex.count == 1) {
                direction = Vec3(1.0f, 0.0f, 0.0f);
            } else if (simplex.count == 2) {
                direction = pick_orthogonal(simplex[1].point - simplex[0].point);
            } else { // count == 3
                Vec3 ab = simplex[1].point - simplex[0].point;
                Vec3 ac = simplex[2].point - simplex[0].point;
                Vec3 n = ab.cross(ac);
                if (n.length_sq() < EPSILON * EPSILON) {
                    n = pick_orthogonal(ab);
                }
                direction = n.normalized();
            }
        }

        const SimplexVertex w = support(shape_a, transform_a, shape_b, transform_b, direction);

        if (dist >= tolerance) {
            // Duality gap: progress made towards origin along search direction
            const float progress = (closest - w.point).dot(closest) / dist;
            if (progress <= tolerance) {
                result.intersecting = false;
                result.distance = best_dist;
                result.separation_axis = (best_dist > EPSILON) ? (best_closest / best_dist) : direction.normalized();
                result.simplex = best_simplex;
                compute_simplex_closest_points(best_simplex, result.closest_point_a, result.closest_point_b, closest);
                return result;
            }
        } else {
            // Expanding intersecting simplex; check if support reaches across
            if (w.point.dot(direction) < 0.0f) {
                // Cannot enclose origin; shapes are touching at surface boundary
                result.intersecting = true;
                result.distance = 0.0f;
                result.simplex = simplex;
                return result;
            }
        }

        // Duplicate vertex check to avoid infinite loops near boundary
        for (uint32_t i = 0; i < simplex.count; ++i) {
            if ((w.point - simplex[i].point).length_sq() < 1e-8f) {
                if (dist < tolerance) {
                    result.intersecting = true;
                    result.distance = 0.0f;
                    result.simplex = simplex;
                    return result;
                }
                result.intersecting = false;
                result.distance = best_dist;
                result.separation_axis = (best_dist > EPSILON) ? (best_closest / best_dist) : direction.normalized();
                result.simplex = best_simplex;
                compute_simplex_closest_points(best_simplex, result.closest_point_a, result.closest_point_b, closest);
                return result;
            }
        }

        simplex.push_front(w);

        Vec3 unused_dir;
        if (update_simplex(simplex, unused_dir)) {
            // Origin enclosed inside tetrahedron!
            result.intersecting = true;
            result.distance = 0.0f;
            result.simplex = simplex;
            return result;
        }

        // Compute updated closest point on the reduced simplex
        compute_simplex_closest_points(simplex, result.closest_point_a, result.closest_point_b, closest);
        const float new_dist = closest.length();
        if (new_dist < best_dist) {
            best_dist = new_dist;
            best_closest = closest;
            best_simplex = simplex;
        }
    }

    result.intersecting = false;
    result.simplex = best_simplex;
    result.distance = best_dist;
    result.separation_axis = (best_dist > EPSILON) ? (best_closest / best_dist) : Vec3(0.0f, 1.0f, 0.0f);
    compute_simplex_closest_points(best_simplex, result.closest_point_a, result.closest_point_b, closest);
    return result;
}

bool GJK::test_intersection(
    const Shape& shape_a, const Transform3D& transform_a,
    const Shape& shape_b, const Transform3D& transform_b
) noexcept {
    Vec3 direction = transform_b.position - transform_a.position;
    if (direction.length_sq() < EPSILON * EPSILON) {
        direction = Vec3(1.0f, 0.0f, 0.0f);
    }

    Simplex simplex;
    SimplexVertex v0 = support(shape_a, transform_a, shape_b, transform_b, direction);
    simplex.push_front(v0);

    direction = -v0.point;
    if (direction.length_sq() < EPSILON * EPSILON) {
        direction = Vec3(0.0f, 1.0f, 0.0f);
    }

    for (uint32_t iter = 0; iter < 32; ++iter) {
        if (direction.length_sq() < 1e-8f) {
            return true;
        }

        SimplexVertex w = support(shape_a, transform_a, shape_b, transform_b, direction);
        if (w.point.dot(direction) < 0.0f) {
            return false;
        }

        simplex.push_front(w);

        if (update_simplex(simplex, direction)) {
            return true;
        }
    }

    return false;
}

} // namespace nisaba::physics

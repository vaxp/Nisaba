#include "nisaba/physics/collision/epa.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::physics {

namespace {

struct EPAFace {
    uint32_t v[3];
    Vec3 normal;
    float distance;
    bool alive;
};

struct EPAEdge {
    uint32_t a;
    uint32_t b;
};

[[nodiscard]] inline Vec3 pick_orthogonal(const Vec3& v) noexcept {
    if (std::abs(v.x) > 0.5f || std::abs(v.y) > 0.5f) {
        return Vec3(-v.y, v.x, 0.0f).normalized();
    }
    return Vec3(0.0f, -v.z, v.y).normalized();
}

} // namespace

EPAResult EPA::query(
    const Shape& shape_a, const Transform3D& transform_a,
    const Shape& shape_b, const Transform3D& transform_b,
    const Simplex& gjk_simplex,
    uint32_t max_iterations,
    float tolerance
) noexcept {
    EPAResult result;
    result.success = false;

    SimplexVertex vertices[MAX_VERTICES];
    size_t vertex_count = 0;

    for (uint32_t i = 0; i < gjk_simplex.count && vertex_count < MAX_VERTICES; ++i) {
        bool dup = false;
        for (size_t j = 0; j < vertex_count; ++j) {
            if ((vertices[j].point - gjk_simplex[i].point).length_sq() < 1e-6f) {
                dup = true;
                break;
            }
        }
        if (!dup) {
            vertices[vertex_count++] = gjk_simplex[i];
        }
    }

    EPAFace faces[MAX_FACES];
    size_t face_count = 0;
    Vec3 centroid = Vec3::zero();

    auto make_face = [&](uint32_t a, uint32_t b, uint32_t c) {
        if (face_count >= MAX_FACES) return;

        Vec3 va = vertices[a].point;
        Vec3 vb = vertices[b].point;
        Vec3 vc = vertices[c].point;

        Vec3 n = (vb - va).cross(vc - va);
        float len = n.length();
        if (len < 1e-7f) return;

        n = n / len;
        // Ensure outward normal points away from polytope centroid
        if (n.dot(va - centroid) < 0.0f) {
            n = -n;
            std::swap(b, c);
        }

        float d = n.dot(va);

        faces[face_count++] = EPAFace{
            {a, b, c},
            n,
            d,
            true
        };
    };

    bool initial_polytope_valid = false;

    // Check if GJK provided a valid 4-simplex that encloses origin
    if (vertex_count == 4) {
        Vec3 da = vertices[1].point - vertices[0].point;
        Vec3 db = vertices[2].point - vertices[0].point;
        Vec3 dc = vertices[3].point - vertices[0].point;
        float volume = std::abs(da.dot(db.cross(dc)));

        if (volume > 1e-6f) {
            centroid = (vertices[0].point + vertices[1].point + vertices[2].point + vertices[3].point) * 0.25f;

            make_face(0, 1, 2);
            make_face(0, 2, 3);
            make_face(0, 3, 1);
            make_face(1, 3, 2);

            bool encloses_origin = (face_count == 4);
            for (size_t i = 0; i < face_count; ++i) {
                if (faces[i].distance <= 1e-4f) {
                    encloses_origin = false;
                    break;
                }
            }

            if (encloses_origin) {
                initial_polytope_valid = true;
            }
        }
    }

    if (!initial_polytope_valid) {
        // Build an enclosing bipyramid (5 vertices, 6 faces) around the origin
        face_count = 0;

        if (vertex_count == 0) {
            vertices[0] = GJK::support(shape_a, transform_a, shape_b, transform_b, Vec3(1.0f, 0.0f, 0.0f));
            vertex_count = 1;
        }

        if (vertex_count == 1) {
            Vec3 dir = -vertices[0].point;
            if (dir.length_sq() < 1e-6f) dir = Vec3(-1.0f, 0.0f, 0.0f);
            vertices[1] = GJK::support(shape_a, transform_a, shape_b, transform_b, dir.normalized());
            vertex_count = 2;
        }

        if (vertex_count >= 2) {
            Vec3 ab = vertices[1].point - vertices[0].point;
            Vec3 perp = pick_orthogonal(ab);
            SimplexVertex v2 = GJK::support(shape_a, transform_a, shape_b, transform_b, perp);
            if (((v2.point - vertices[0].point).cross(ab)).length_sq() < 1e-6f) {
                v2 = GJK::support(shape_a, transform_a, shape_b, transform_b, -perp);
            }
            vertices[2] = v2;
            vertex_count = 3;
        }

        // Triangle vertices 0, 1, 2 are non-collinear
        Vec3 e1 = vertices[1].point - vertices[0].point;
        Vec3 e2 = vertices[2].point - vertices[0].point;
        Vec3 tri_normal = e1.cross(e2);
        if (tri_normal.length_sq() < 1e-6f) {
            tri_normal = Vec3(0.0f, 0.0f, 1.0f);
        } else {
            tri_normal = tri_normal.normalized();
        }

        // 4th and 5th vertices along +tri_normal and -tri_normal
        vertices[3] = GJK::support(shape_a, transform_a, shape_b, transform_b, tri_normal);
        vertices[4] = GJK::support(shape_a, transform_a, shape_b, transform_b, -tri_normal);
        vertex_count = 5;

        centroid = (vertices[0].point + vertices[1].point + vertices[2].point +
                    vertices[3].point + vertices[4].point) * 0.2f;

        // Top 3 faces connecting to vertex 3 (+tri_normal)
        make_face(0, 1, 3);
        make_face(1, 2, 3);
        make_face(2, 0, 3);

        // Bottom 3 faces connecting to vertex 4 (-tri_normal)
        make_face(1, 0, 4);
        make_face(2, 1, 4);
        make_face(0, 2, 4);
    }

    if (face_count == 0) {
        return result;
    }

    size_t best_face = 0;

    for (uint32_t iter = 0; iter < max_iterations; ++iter) {
        result.iterations = iter + 1;

        // Find alive face closest to origin
        best_face = MAX_FACES;
        float min_dist = 1e30f;
        for (size_t i = 0; i < face_count; ++i) {
            if (faces[i].alive && faces[i].distance < min_dist) {
                min_dist = faces[i].distance;
                best_face = i;
            }
        }

        if (best_face == MAX_FACES) {
            break;
        }

        const Vec3 face_normal = faces[best_face].normal;
        const SimplexVertex w = GJK::support(shape_a, transform_a, shape_b, transform_b, face_normal);

        const float w_dist = w.point.dot(face_normal);
        if (w_dist - min_dist <= tolerance) {
            // Polytope reached boundary of Minkowski difference along face_normal
            break;
        }

        // Check if vertex or face pool is approaching capacity
        if (vertex_count >= MAX_VERTICES || face_count + 16 >= MAX_FACES) {
            break;
        }

        bool dup = false;
        for (size_t i = 0; i < vertex_count; ++i) {
            if ((w.point - vertices[i].point).length_sq() < 1e-8f) {
                dup = true;
                break;
            }
        }
        if (dup) {
            break;
        }

        const uint32_t w_idx = static_cast<uint32_t>(vertex_count++);
        vertices[w_idx] = w;

        // Collect horizon edges from all faces visible from w
        EPAEdge edges[MAX_EDGES];
        size_t edge_count = 0;

        for (size_t i = 0; i < face_count; ++i) {
            if (!faces[i].alive) continue;

            if (faces[i].normal.dot(w.point - vertices[faces[i].v[0]].point) > 0.0f) {
                faces[i].alive = false;

                const uint32_t f_v[3] = {faces[i].v[0], faces[i].v[1], faces[i].v[2]};
                for (int e = 0; e < 3; ++e) {
                    const uint32_t u = f_v[e];
                    const uint32_t v = f_v[(e + 1) % 3];

                    // Remove edge if opposite already present (internal edge)
                    bool opposite_found = false;
                    for (size_t k = 0; k < edge_count; ++k) {
                        if (edges[k].a == v && edges[k].b == u) {
                            edges[k] = edges[--edge_count];
                            opposite_found = true;
                            break;
                        }
                    }

                    if (!opposite_found && edge_count < MAX_EDGES) {
                        edges[edge_count++] = EPAEdge{u, v};
                    }
                }
            }
        }

        // Construct new faces connecting horizon edges to w
        for (size_t i = 0; i < edge_count; ++i) {
            make_face(edges[i].a, edges[i].b, w_idx);
        }
    }

    if (best_face >= face_count || !faces[best_face].alive) {
        // Fallback: find any alive face with minimum distance
        float min_dist = 1e30f;
        for (size_t i = 0; i < face_count; ++i) {
            if (faces[i].alive && faces[i].distance < min_dist) {
                min_dist = faces[i].distance;
                best_face = i;
            }
        }
    }

    if (best_face >= face_count) {
        return result;
    }

    const EPAFace& f = faces[best_face];
    const SimplexVertex& v0 = vertices[f.v[0]];
    const SimplexVertex& v1 = vertices[f.v[1]];
    const SimplexVertex& v2 = vertices[f.v[2]];

    result.normal = f.normal;
    result.penetration_depth = f.distance;

    // Project origin onto closest face plane
    const Vec3 p = result.normal * result.penetration_depth;

    // Barycentric coordinates of p on (v0, v1, v2)
    const Vec3 edge0 = v1.point - v0.point;
    const Vec3 edge1 = v2.point - v0.point;
    const Vec3 v0_to_p = p - v0.point;

    const float d00 = edge0.dot(edge0);
    const float d01 = edge0.dot(edge1);
    const float d11 = edge1.dot(edge1);
    const float d20 = v0_to_p.dot(edge0);
    const float d21 = v0_to_p.dot(edge1);

    const float denom = d00 * d11 - d01 * d01;
    float l0 = 1.0f / 3.0f, l1 = 1.0f / 3.0f, l2 = 1.0f / 3.0f;

    if (std::abs(denom) > 1e-10f) {
        l1 = (d11 * d20 - d01 * d21) / denom;
        l2 = (d00 * d21 - d01 * d20) / denom;
        l0 = 1.0f - l1 - l2;

        l0 = std::clamp(l0, 0.0f, 1.0f);
        l1 = std::clamp(l1, 0.0f, 1.0f);
        l2 = std::clamp(l2, 0.0f, 1.0f);
        const float sum = l0 + l1 + l2;
        if (sum > 1e-6f) {
            l0 /= sum;
            l1 /= sum;
            l2 /= sum;
        }
    }

    result.point_a = v0.point_a * l0 + v1.point_a * l1 + v2.point_a * l2;
    result.point_b = v0.point_b * l0 + v1.point_b * l1 + v2.point_b * l2;
    result.success = true;

    return result;
}

} // namespace nisaba::physics

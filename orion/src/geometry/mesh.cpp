#include "orion/geometry/mesh.hpp"
#include <cmath>

namespace orion {

void Mesh3D::compute_bounds() noexcept {
    bounds.reset();
    if (vertices.empty()) return;

    for (const auto& v : vertices) {
        bounds.encapsulate(v.position);
    }

    if (submeshes.empty() && !indices.empty()) {
        SubMesh sm;
        sm.index_offset = 0;
        sm.index_count = static_cast<uint32_t>(indices.size());
        sm.vertex_offset = 0;
        sm.material_index = 0;
        sm.bounds = bounds;
        submeshes.push_back(sm);
    } else {
        for (auto& sm : submeshes) {
            sm.bounds.reset();
            const uint32_t end_idx = sm.index_offset + sm.index_count;
            for (uint32_t i = sm.index_offset; i < end_idx && i < indices.size(); ++i) {
                const uint32_t v_idx = sm.vertex_offset + indices[i];
                if (v_idx < vertices.size()) {
                    sm.bounds.encapsulate(vertices[v_idx].position);
                }
            }
        }
    }
}

void Mesh3D::compute_normals() noexcept {
    if (vertices.empty() || indices.empty()) return;

    for (auto& v : vertices) {
        v.normal = Vec3(0.0f, 0.0f, 0.0f);
    }

    const size_t tri_count = indices.size() / 3;
    for (size_t t = 0; t < tri_count; ++t) {
        const uint32_t i0 = indices[t * 3 + 0];
        const uint32_t i1 = indices[t * 3 + 1];
        const uint32_t i2 = indices[t * 3 + 2];

        if (i0 >= vertices.size() || i1 >= vertices.size() || i2 >= vertices.size()) continue;

        const Vec3& p0 = vertices[i0].position;
        const Vec3& p1 = vertices[i1].position;
        const Vec3& p2 = vertices[i2].position;

        const Vec3 face_norm = (p1 - p0).cross(p2 - p0);
        vertices[i0].normal = vertices[i0].normal + face_norm;
        vertices[i1].normal = vertices[i1].normal + face_norm;
        vertices[i2].normal = vertices[i2].normal + face_norm;
    }

    for (auto& v : vertices) {
        if (v.normal.length_sq() > 1e-12f) {
            v.normal = v.normal.normalized();
        } else {
            v.normal = Vec3(0.0f, 1.0f, 0.0f);
        }
    }
}

void Mesh3D::compute_tangents() noexcept {
    if (vertices.empty() || indices.empty()) return;

    const size_t v_count = vertices.size();
    std::vector<Vec3> tan1(v_count, Vec3(0.0f, 0.0f, 0.0f));
    std::vector<Vec3> tan2(v_count, Vec3(0.0f, 0.0f, 0.0f));

    const size_t tri_count = indices.size() / 3;
    for (size_t t = 0; t < tri_count; ++t) {
        const uint32_t i0 = indices[t * 3 + 0];
        const uint32_t i1 = indices[t * 3 + 1];
        const uint32_t i2 = indices[t * 3 + 2];

        if (i0 >= v_count || i1 >= v_count || i2 >= v_count) continue;

        const auto& v0 = vertices[i0];
        const auto& v1 = vertices[i1];
        const auto& v2 = vertices[i2];

        const Vec3 e1 = v1.position - v0.position;
        const Vec3 e2 = v2.position - v0.position;

        const float du1 = v1.uv[0] - v0.uv[0];
        const float dv1 = v1.uv[1] - v0.uv[1];
        const float du2 = v2.uv[0] - v0.uv[0];
        const float dv2 = v2.uv[1] - v0.uv[1];

        const float det = du1 * dv2 - du2 * dv1;
        if (std::abs(det) > 1e-10f) {
            const float r = 1.0f / det;
            const Vec3 sdir(
                (dv2 * e1.x - dv1 * e2.x) * r,
                (dv2 * e1.y - dv1 * e2.y) * r,
                (dv2 * e1.z - dv1 * e2.z) * r
            );
            const Vec3 tdir(
                (du1 * e2.x - du2 * e1.x) * r,
                (du1 * e2.y - du2 * e1.y) * r,
                (du1 * e2.z - du2 * e1.z) * r
            );

            tan1[i0] = tan1[i0] + sdir;
            tan1[i1] = tan1[i1] + sdir;
            tan1[i2] = tan1[i2] + sdir;

            tan2[i0] = tan2[i0] + tdir;
            tan2[i1] = tan2[i1] + tdir;
            tan2[i2] = tan2[i2] + tdir;
        }
    }

    for (size_t i = 0; i < v_count; ++i) {
        const Vec3& n = vertices[i].normal;
        const Vec3& t = tan1[i];

        if (t.length_sq() > 1e-12f) {
            // Gram-Schmidt orthogonalize: t_ortho = (t - n * (n . t)).normalized()
            Vec3 t_ortho = (t - n * n.dot(t));
            if (t_ortho.length_sq() > 1e-12f) {
                t_ortho = t_ortho.normalized();
            } else {
                t_ortho = Vec3(1.0f, 0.0f, 0.0f);
            }

            // Calculate handedness sign
            const float w = (n.cross(t).dot(tan2[i]) < 0.0f) ? -1.0f : 1.0f;
            vertices[i].tangent[0] = t_ortho.x;
            vertices[i].tangent[1] = t_ortho.y;
            vertices[i].tangent[2] = t_ortho.z;
            vertices[i].tangent[3] = w;
        } else {
            // Default tangent perpendicular to normal
            Vec3 c1 = n.cross(Vec3(0.0f, 0.0f, 1.0f));
            Vec3 c2 = n.cross(Vec3(0.0f, 1.0f, 0.0f));
            Vec3 t_fallback = (c1.length_sq() > c2.length_sq()) ? c1.normalized() : c2.normalized();
            vertices[i].tangent[0] = t_fallback.x;
            vertices[i].tangent[1] = t_fallback.y;
            vertices[i].tangent[2] = t_fallback.z;
            vertices[i].tangent[3] = 1.0f;
        }
    }
}

void Mesh3D::upload_to_buffers() noexcept {
    vertex_buffer.set_vertices(vertices);
    index_buffer.set_indices(indices);
}

} // namespace orion

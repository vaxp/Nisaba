#include "orion/scene/renderer.hpp"
#include <algorithm>
#include <cmath>

namespace orion {

namespace {

/// @brief Computes upper-left 3x3 inverse-transpose (cofactor matrix) in ~20 FLOPs
/// instead of a full 4x4 matrix inversion + transpose.
[[nodiscard]] inline Mat44 compute_fast_normal_matrix(const Mat44& m) noexcept {
    const float a00 = m.m[0], a01 = m.m[1], a02 = m.m[2];
    const float a10 = m.m[4], a11 = m.m[5], a12 = m.m[6];
    const float a20 = m.m[8], a21 = m.m[9], a22 = m.m[10];

    const float c00 = a11 * a22 - a12 * a21;
    const float c01 = a12 * a20 - a10 * a22;
    const float c02 = a10 * a21 - a11 * a20;

    const float det = a00 * c00 + a01 * c01 + a02 * c02;
    if (std::abs(det) < 1e-8f) {
        return Mat44::identity();
    }
    const float inv_det = 1.0f / det;

    Mat44 out = Mat44::identity();
    out.m[0]  = c00 * inv_det;
    out.m[1]  = c01 * inv_det;
    out.m[2]  = c02 * inv_det;
    out.m[4]  = (a02 * a21 - a01 * a22) * inv_det;
    out.m[5]  = (a00 * a22 - a02 * a20) * inv_det;
    out.m[6]  = (a01 * a20 - a00 * a21) * inv_det;
    out.m[8]  = (a01 * a12 - a02 * a11) * inv_det;
    out.m[9]  = (a02 * a10 - a00 * a12) * inv_det;
    out.m[10] = (a00 * a11 - a01 * a10) * inv_det;
    return out;
}

} // namespace

Renderer3D::Renderer3D(uint32_t shadow_res)
    : shadow_map(shadow_res, shadow_res) {
    draw_queue.reserve(256);
    m_visible_renderables.reserve(256);
}

void Renderer3D::prepare_frame(const View3D& view) noexcept {
    draw_queue.clear();
    last_stats.reset();

    if (!view.camera || !view.scene) {
        return;
    }

    // 1. Frustum Culling Pass (reuses pre-allocated m_visible_renderables)
    view.scene->gather_visible_renderables(view.camera->get_frustum(), m_visible_renderables);

    last_stats.total_objects = static_cast<uint32_t>(view.scene->renderables.size());
    last_stats.visible_objects = static_cast<uint32_t>(m_visible_renderables.size());
    last_stats.culled_objects = last_stats.total_objects - last_stats.visible_objects;

    // 2. Cascaded Shadow Mapping (CSM) Setup
    if (view.shadows_enabled) {
        for (const auto& light : view.scene->lights) {
            if (light.type == LightType::Directional && light.casts_shadows) {
                shadow_map.light_vp_matrix = ShadowMap::compute_directional_shadow_matrix(
                    light, *view.camera, view.shadow_distance, shadow_map.width
                );
                ShadowMap::compute_cascades(
                    light, *view.camera, view.shadow_distance, shadow_map.width,
                    shadow_map.cascade_matrices, shadow_map.cascade_splits,
                    shadow_map.settings.cascade_split_lambda
                );
                break;
            }
        }
    }

    // 3. 64-Bit Draw Command Generation
    const Vec3 cam_pos = view.camera->get_position();

    for (const auto* r : m_visible_renderables) {
        if (!r || !r->mesh) continue;

        const Vec3 center = r->world_bounds.center();
        const float dist = (center - cam_pos).length();
        const Mat44 norm_mat = compute_fast_normal_matrix(r->transform);

        const size_t sm_count = r->mesh->submeshes.empty() ? 1 : r->mesh->submeshes.size();

        for (size_t s = 0; s < sm_count; ++s) {
            const auto mat_ptr = r->get_material(s);
            const PbrMaterial* mat = mat_ptr.get();

            const uint16_t mat_id = static_cast<uint16_t>(reinterpret_cast<uintptr_t>(mat) & 0xFFFF);
            const bool is_trans = mat ? mat->is_transparent() : false;
            const RenderPassID pass_id = is_trans ? RenderPassID::Transparent : RenderPassID::Opaque;
            const uint32_t prim_id = r->id;

            const uint64_t key = DrawSortKey64::encode(
                pass_id, mat_id, dist, is_trans, prim_id
            );

            DrawCommand64 cmd;
            cmd.sort_key = key;
            cmd.renderable = r;
            cmd.mesh = r->mesh.get();
            cmd.submesh_index = static_cast<uint32_t>(s);
            cmd.material = mat;
            cmd.world_transform = r->transform;
            cmd.normal_matrix = norm_mat;
            cmd.distance_to_camera = dist;

            draw_queue.push_back(cmd);

            if (is_trans) {
                last_stats.transparent_draw_calls++;
            } else {
                last_stats.opaque_draw_calls++;
            }

            if (!r->mesh->submeshes.empty()) {
                last_stats.total_triangles += r->mesh->submeshes[s].index_count / 3;
            } else {
                last_stats.total_triangles += static_cast<uint32_t>(r->mesh->indices.size() / 3);
            }
        }
    }

    // 4. 64-Bit Sorting Pass
    // Front-to-back for opaque (Early-Z kill maximization)
    // Back-to-front for transparent (Alpha blending correctness)
    std::sort(draw_queue.begin(), draw_queue.end());

    last_stats.total_draw_calls = static_cast<uint32_t>(draw_queue.size());
    for (size_t i = 0; i < draw_queue.size(); ) {
        size_t batch = 1;
        while (i + batch < draw_queue.size() && batch < 128 &&
               draw_queue[i + batch].mesh == draw_queue[i].mesh &&
               draw_queue[i + batch].submesh_index == draw_queue[i].submesh_index &&
               draw_queue[i + batch].material == draw_queue[i].material &&
               DrawSortKey64::decode_pass(draw_queue[i + batch].sort_key) == DrawSortKey64::decode_pass(draw_queue[i].sort_key)) {
            ++batch;
        }
        if (batch > 1) {
            last_stats.instanced_batches++;
        }
        i += batch;
    }
}

void Renderer3D::execute_frame(const View3D& view) noexcept {
    if (draw_queue.empty()) return;

    // Simulate shadow pass
    if (view.shadows_enabled) {
        shadow_map.clear(1.0f);
    }
}

void Renderer3D::render(const View3D& view) noexcept {
    prepare_frame(view);
    execute_frame(view);
}

} // namespace orion

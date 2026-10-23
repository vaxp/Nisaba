#include "orion/scene/renderer.hpp"
#include <algorithm>

namespace orion {

Renderer3D::Renderer3D(uint32_t shadow_res)
    : shadow_map(shadow_res, shadow_res) {}

void Renderer3D::prepare_frame(const View3D& view) noexcept {
    draw_queue.clear();
    last_stats.reset();

    if (!view.camera || !view.scene) {
        return;
    }

    // 1. Frustum Culling Pass
    std::vector<const Renderable3D*> visible_renderables;
    view.scene->gather_visible_renderables(view.camera->get_frustum(), visible_renderables);

    last_stats.total_objects = static_cast<uint32_t>(view.scene->renderables.size());
    last_stats.visible_objects = static_cast<uint32_t>(visible_renderables.size());
    last_stats.culled_objects = last_stats.total_objects - last_stats.visible_objects;

    // 2. Cascaded Shadow Mapping (CSM) Setup
    if (view.shadows_enabled) {
        for (const auto& light : view.scene->lights) {
            if (light.type == LightType::Directional && light.casts_shadows) {
                shadow_map.light_vp_matrix = ShadowMap::compute_directional_shadow_matrix(
                    light, *view.camera, view.shadow_distance, shadow_map.width
                );
                break;
            }
        }
    }

    // 3. 64-Bit Draw Command Generation
    const Vec3 cam_pos = view.camera->get_position();

    for (const auto* r : visible_renderables) {
        if (!r || !r->mesh) continue;

        const Vec3 center = r->world_bounds.center();
        const float dist = (center - cam_pos).length();

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

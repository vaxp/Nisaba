#include "nisaba/render3d/gpu/gpu_backend3d.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

namespace nisaba::render3d {

GpuBackend3D::GpuBackend3D(uint32_t shadow_resolution)
    : m_shadow_pass(shadow_resolution) {
    init_shaders();
}

GpuBackend3D::~GpuBackend3D() = default;

void GpuBackend3D::init_shaders() {
    m_pbr_shader = GlShaderProgram::create_pbr_shader();
}

void GpuBackend3D::render_view(
    const View3D& view,
    uint32_t target_fbo,
    bool clear_buffers
) {
    GlStateGuard3D guard;

    if (!view.camera || !view.scene) {
        return;
    }

    // 1. CPU Culling & 64-Bit Draw Queue Sorting
    m_cpu_renderer.prepare_frame(view);

    // 2. Cascaded Shadow Depth Pre-Pass
    if (view.shadows_enabled && m_cpu_renderer.shadow_map.light_vp_matrix.determinant() != 0.0f) {
        m_shadow_pass.begin(m_cpu_renderer.shadow_map.light_vp_matrix);
        for (const auto& r_ptr : view.scene->renderables) {
            if (r_ptr && r_ptr->visible && r_ptr->cast_shadows && r_ptr->mesh) {
                m_shadow_pass.render_caster(m_resource_cache, *r_ptr->mesh, r_ptr->transform);
            }
        }
        m_shadow_pass.end();
    }

    // 3. Main Forward PBR Hardware Pass
    glBindFramebuffer(GL_FRAMEBUFFER, target_fbo);

    const auto& vp = view.viewport;
    glViewport(
        static_cast<GLint>(vp.x),
        static_cast<GLint>(vp.y),
        static_cast<GLsizei>(vp.width),
        static_cast<GLsizei>(vp.height)
    );

    if (clear_buffers) {
        glClearColor(view.clear_color.x, view.clear_color.y, view.clear_color.z, view.clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    m_resource_cache.update_camera_ubo(
        view.camera->get_view_matrix(),
        view.camera->get_projection_matrix(),
        view.camera->get_position()
    );

    if (!m_pbr_shader || !m_pbr_shader->is_valid()) {
        return;
    }

    m_pbr_shader->bind();

    // Setup Directional Light Uniforms
    Vec3 light_dir(0.0f, -1.0f, 0.0f);
    Vec3 light_col(1.0f, 1.0f, 1.0f);
    float light_intensity = 3.0f;

    for (const auto& l : view.scene->lights) {
        if (l.type == LightType::Directional) {
            light_dir = l.direction;
            light_col = l.color;
            light_intensity = l.intensity;
            break;
        }
    }

    m_pbr_shader->set_uniform("u_lightDirection", light_dir);
    m_pbr_shader->set_uniform("u_lightColor", light_col);
    m_pbr_shader->set_uniform("u_lightIntensity", light_intensity);
    m_pbr_shader->set_uniform("u_ambientColor", view.scene->ambient_color);
    m_pbr_shader->set_uniform("u_lightSpaceMatrix", m_cpu_renderer.shadow_map.light_vp_matrix);

    m_pbr_shader->set_uniform("u_baseColorTexture", 0);
    m_pbr_shader->set_uniform("u_metallicRoughnessTexture", 1);
    m_pbr_shader->set_uniform("u_normalTexture", 2);
    m_pbr_shader->set_uniform("u_occlusionTexture", 3);
    m_pbr_shader->set_uniform("u_emissiveTexture", 4);
    m_pbr_shader->set_uniform("u_shadowMap", 5);

    // Bind Shadow Map to Texture Unit 5
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_shadow_pass.depth_texture());

    // Execute 64-Bit Sorted Draw Stream
    RenderPassID current_pass = static_cast<RenderPassID>(-1);
    const PbrMaterial* current_material = nullptr;
    GLuint current_vao = 0;

    for (const auto& cmd : m_cpu_renderer.draw_queue) {
        if (!cmd.mesh) continue;

        const RenderPassID pass_id = DrawSortKey64::decode_pass(cmd.sort_key);
        if (pass_id != current_pass) {
            current_pass = pass_id;
            if (current_pass == RenderPassID::Opaque) {
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_TRUE);
                glDepthFunc(GL_LEQUAL);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glDisable(GL_BLEND);
            } else {
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }

        // Per-draw model matrix
        m_pbr_shader->set_uniform("u_model", cmd.world_transform);
        bool inv_ok = false;
        m_pbr_shader->set_uniform("u_normalMatrix", cmd.world_transform.inverse(inv_ok).transpose());

        // Material binding update
        if (cmd.material != current_material) {
            current_material = cmd.material;
            if (current_material) {
                m_resource_cache.update_material_ubo(current_material->get_uniforms());

                auto base_th = current_material->base_color_texture ? m_resource_cache.get_or_upload_texture(*current_material->base_color_texture) : GpuTextureHandle{};
                auto mr_th = current_material->metallic_roughness_texture ? m_resource_cache.get_or_upload_texture(*current_material->metallic_roughness_texture) : GpuTextureHandle{};
                auto norm_th = current_material->normal_texture ? m_resource_cache.get_or_upload_texture(*current_material->normal_texture) : GpuTextureHandle{};
                auto occ_th = current_material->occlusion_texture ? m_resource_cache.get_or_upload_texture(*current_material->occlusion_texture) : GpuTextureHandle{};
                auto em_th = current_material->emissive_texture ? m_resource_cache.get_or_upload_texture(*current_material->emissive_texture) : GpuTextureHandle{};

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, base_th.is_valid ? base_th.texture_id : m_resource_cache.default_white_texture());

                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, mr_th.is_valid ? mr_th.texture_id : m_resource_cache.default_white_texture());

                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, norm_th.is_valid ? norm_th.texture_id : m_resource_cache.default_normal_texture());

                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, occ_th.is_valid ? occ_th.texture_id : m_resource_cache.default_white_texture());

                glActiveTexture(GL_TEXTURE4);
                glBindTexture(GL_TEXTURE_2D, em_th.is_valid ? em_th.texture_id : m_resource_cache.default_black_texture());
            }
        }

        // Mesh VAO binding
        auto mesh_handle = m_resource_cache.get_or_upload_mesh(*cmd.mesh);
        if (!mesh_handle.is_valid || mesh_handle.vao == 0) continue;

        if (mesh_handle.vao != current_vao) {
            current_vao = mesh_handle.vao;
            glBindVertexArray(current_vao);
        }

        // Draw Call Dispatch
        if (cmd.submesh_index < cmd.mesh->submeshes.size()) {
            const auto& sm = cmd.mesh->submeshes[cmd.submesh_index];
            if (mesh_handle.ebo != 0) {
                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(sm.index_count),
                    GL_UNSIGNED_INT,
                    reinterpret_cast<const void*>(static_cast<uintptr_t>(sm.index_offset * sizeof(uint32_t)))
                );
            } else {
                glDrawArrays(GL_TRIANGLES, static_cast<GLint>(sm.vertex_offset), static_cast<GLsizei>(sm.index_count));
            }
        } else {
            if (mesh_handle.ebo != 0) {
                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(mesh_handle.index_count),
                    GL_UNSIGNED_INT,
                    nullptr
                );
            } else {
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(mesh_handle.index_count));
            }
        }
    }

    if (current_vao != 0) {
        glBindVertexArray(0);
    }
    m_pbr_shader->unbind();

    // Cleanly unbind textures and restore default active texture unit 0 for 2D UI / HUD overlay
    for (int t = 5; t >= 0; --t) {
        glActiveTexture(GL_TEXTURE0 + t);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

} // namespace nisaba::render3d

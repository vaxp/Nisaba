#include "orion/gpu/gpu_backend3d.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <chrono>
#include <iostream>

namespace orion {

GpuBackend3D::GpuBackend3D(uint32_t shadow_resolution)
    : m_shadow_pass(shadow_resolution) {
    init_shaders();
}

GpuBackend3D::~GpuBackend3D() {
    cleanup_hdr_framebuffer();
}

void GpuBackend3D::cleanup_hdr_framebuffer() noexcept {
    if (m_hdr_color_tex != 0) {
        glDeleteTextures(1, &m_hdr_color_tex);
        m_hdr_color_tex = 0;
    }
    if (m_hdr_depth_rbo != 0) {
        glDeleteRenderbuffers(1, &m_hdr_depth_rbo);
        m_hdr_depth_rbo = 0;
    }
    if (m_hdr_fbo != 0) {
        glDeleteFramebuffers(1, &m_hdr_fbo);
        m_hdr_fbo = 0;
    }
    m_hdr_width = 0;
    m_hdr_height = 0;
}

void GpuBackend3D::ensure_hdr_framebuffer(uint32_t width, uint32_t height) {
    if (m_hdr_width == width && m_hdr_height == height && m_hdr_fbo != 0) {
        return;
    }

    cleanup_hdr_framebuffer();

    m_hdr_width = std::max(width, 1u);
    m_hdr_height = std::max(height, 1u);

    glGenFramebuffers(1, &m_hdr_fbo);
    glGenTextures(1, &m_hdr_color_tex);
    glGenRenderbuffers(1, &m_hdr_depth_rbo);

    glBindTexture(GL_TEXTURE_2D, m_hdr_color_tex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA16F,
        static_cast<GLsizei>(m_hdr_width),
        static_cast<GLsizei>(m_hdr_height),
        0, GL_RGBA, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindRenderbuffer(GL_RENDERBUFFER, m_hdr_depth_rbo);
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
        static_cast<GLsizei>(m_hdr_width),
        static_cast<GLsizei>(m_hdr_height)
    );

    glBindFramebuffer(GL_FRAMEBUFFER, m_hdr_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_hdr_color_tex, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_hdr_depth_rbo);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void GpuBackend3D::init_shaders() {
    m_pbr_shader = GlShaderProgram::create_pbr_shader();
    m_advanced_pbr_shader = GlShaderProgram::create_advanced_pbr_shader();
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

    // 2.5 Scene Depth Pre-Pass (Enables Screen-Space Contact Shadows & GTAO)
    const auto& vp = view.viewport;
    const auto& s_settings = view.shadow_settings;
    const bool need_scene_depth = (view.shadows_enabled && s_settings.enable_contact_shadows) ||
                                  (view.post_process.enabled && (view.post_process.gtao.enabled || view.post_process.fog.enabled));
    if (need_scene_depth) {
        const Mat44 view_proj = view.camera->get_view_projection_matrix();
        m_depth_pass.begin(static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height), view_proj);
        for (const auto& cmd : m_cpu_renderer.draw_queue) {
            if (!cmd.mesh) continue;
            const RenderPassID pass_id = DrawSortKey64::decode_pass(cmd.sort_key);
            if (pass_id == RenderPassID::Opaque) {
                m_depth_pass.render_object(m_resource_cache, *cmd.mesh, cmd.world_transform);
            }
        }
        m_depth_pass.end();
    }

    const bool use_postprocess = view.post_process.enabled &&
                                 (view.post_process.bloom.enabled ||
                                  view.post_process.gtao.enabled ||
                                  view.post_process.fog.enabled ||
                                  view.post_process.grading.enabled);

    uint32_t active_fbo = target_fbo;
    if (use_postprocess) {
        ensure_hdr_framebuffer(static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height));
        active_fbo = m_hdr_fbo;
    }

    // 3. Main Forward PBR Hardware Pass
    glBindFramebuffer(GL_FRAMEBUFFER, active_fbo);

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

    const bool need_advanced = (view.scene->environment_map != nullptr) ||
                               (view.shadows_enabled && (s_settings.enable_pcss || s_settings.enable_contact_shadows)) ||
                               (view.scene->lights.size() > 1) ||
                               use_postprocess;
    bool has_advanced_material = false;
    if (!need_advanced) {
        for (const auto& cmd : m_cpu_renderer.draw_queue) {
            if (cmd.material && cmd.material->shading_model != ShadingModel::Standard) {
                has_advanced_material = true;
                break;
            }
        }
    }
    const bool use_advanced = need_advanced || has_advanced_material;
    GlShaderProgram* active_shader = (use_advanced && m_advanced_pbr_shader) ? m_advanced_pbr_shader.get() : m_pbr_shader.get();

    if (!active_shader || !active_shader->is_valid()) {
        return;
    }

    active_shader->bind();

    // Setup Directional Light Uniforms (fallback)
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

    active_shader->set_uniform("u_lightDirection", light_dir);
    active_shader->set_uniform("u_lightColor", light_col);
    active_shader->set_uniform("u_lightIntensity", light_intensity);
    active_shader->set_uniform("u_ambientColor", view.scene->ambient_color);
    active_shader->set_uniform("u_lightSpaceMatrix", m_cpu_renderer.shadow_map.light_vp_matrix);

    active_shader->set_uniform("u_baseColorTexture", 0);
    active_shader->set_uniform("u_metallicRoughnessTexture", 1);
    active_shader->set_uniform("u_normalTexture", 2);
    active_shader->set_uniform("u_occlusionTexture", 3);
    active_shader->set_uniform("u_emissiveTexture", 4);
    active_shader->set_uniform("u_shadowMap", 5);

    // Bind Shadow Map to Texture Unit 5
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, m_shadow_pass.depth_texture());

    if (use_advanced) {
        // Build and upload std140 Lighting Uniform Buffer Block
        GpuLightingUboBlock lighting_block{};
        const size_t light_count = std::min(view.scene->lights.size(), size_t(128));
        lighting_block.light_count = static_cast<uint32_t>(light_count);

        for (size_t i = 0; i < light_count; ++i) {
            lighting_block.lights[i] = view.scene->lights[i].get_gpu_data(i == 0 ? 0 : -1);
        }

        lighting_block.ambient_color[0] = view.scene->ambient_color.x;
        lighting_block.ambient_color[1] = view.scene->ambient_color.y;
        lighting_block.ambient_color[2] = view.scene->ambient_color.z;
        lighting_block.ambient_color[3] = 1.0f;

        m_resource_cache.update_lighting_ubo(lighting_block);

        active_shader->set_uniform("u_dfgLut", 6);
        active_shader->set_uniform("u_environmentMap", 7);

        // Bind DFG LUT to Texture Unit 6
        uint32_t dfg_lut = m_resource_cache.get_or_create_dfg_lut();
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, dfg_lut);

        // Bind Environment Map to Texture Unit 7 (Cube Map)
        if (view.scene->environment_map) {
            uint32_t env_tex = m_resource_cache.get_or_upload_environment_map(*view.scene->environment_map);
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, env_tex);

            active_shader->set_uniform("u_hasEnvironmentMap", 1);
            active_shader->set_uniform("u_envIntensity", view.scene->environment_intensity);
            for (int i = 0; i < 9; ++i) {
                std::string sh_name = "u_sh[" + std::to_string(i) + "]";
                active_shader->set_uniform(sh_name, view.scene->sh_irradiance.coefficients[i]);
            }
        } else {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            active_shader->set_uniform("u_hasEnvironmentMap", 0);
            active_shader->set_uniform("u_envIntensity", 1.0f);
            for (int i = 0; i < 9; ++i) {
                std::string sh_name = "u_sh[" + std::to_string(i) + "]";
                active_shader->set_uniform(sh_name, Vec3(0.0f, 0.0f, 0.0f));
            }
        }

        // Bind Scene Depth Texture to Texture Unit 8 for Screen-Space Contact Shadows (if active)
        active_shader->set_uniform("u_sceneDepthTexture", 8);
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, need_scene_depth ? m_depth_pass.depth_texture() : 0);

        active_shader->set_uniform("u_lightSize", s_settings.light_size);
        active_shader->set_uniform("u_enablePcss", s_settings.enable_pcss ? 1 : 0);
        active_shader->set_uniform("u_enableContactShadows", (view.shadows_enabled && s_settings.enable_contact_shadows) ? 1 : 0);
        active_shader->set_uniform("u_contactShadowRayLength", s_settings.contact_shadow_length);
        active_shader->set_uniform("u_contactShadowThickness", s_settings.contact_shadow_thickness);
        active_shader->set_uniform("u_contactShadowSteps", static_cast<int>(s_settings.contact_shadow_steps));
        active_shader->set_uniform("u_outputHdr", use_postprocess ? 1 : 0);
    }

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
        active_shader->set_uniform("u_model", cmd.world_transform);
        bool inv_ok = false;
        active_shader->set_uniform("u_normalMatrix", cmd.world_transform.inverse(inv_ok).transpose());

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
    active_shader->unbind();

    // Cleanly unbind textures and restore default active texture unit 0 for 2D UI / HUD overlay
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    for (int t = 6; t >= 0; --t) {
        glActiveTexture(GL_TEXTURE0 + t);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);

    // 4. Post-Processing Pipeline Pass
    if (use_postprocess) {
        const Light3D* sun_ptr = nullptr;
        for (const auto& l : view.scene->lights) {
            if (l.type == LightType::Directional) {
                sun_ptr = &l;
                break;
            }
        }
        m_postprocess.settings = view.post_process;
        m_postprocess.render(
            m_hdr_color_tex,
            m_depth_pass.depth_texture(),
            static_cast<uint32_t>(vp.width),
            static_cast<uint32_t>(vp.height),
            *view.camera,
            sun_ptr,
            target_fbo
        );
    }
}

} // namespace orion

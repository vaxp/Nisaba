#include "orion/gpu/gpu_backend3d.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <chrono>
#include <iostream>

namespace orion {

GpuBackend3D::GpuBackend3D(uint32_t shadow_resolution)
    : m_shadow_pass(shadow_resolution) {
    init_shaders();
    glGenVertexArrays(1, &m_skybox_vao);
}

GpuBackend3D::~GpuBackend3D() {
    cleanup_hdr_framebuffer();
    if (m_skybox_vao != 0) {
        glDeleteVertexArrays(1, &m_skybox_vao);
        m_skybox_vao = 0;
    }
}

void GpuBackend3D::cleanup_hdr_framebuffer() noexcept {
    if (m_refraction_tex != 0) {
        glDeleteTextures(1, &m_refraction_tex);
        m_refraction_tex = 0;
    }
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
    glGenTextures(1, &m_refraction_tex);
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

    glBindTexture(GL_TEXTURE_2D, m_refraction_tex);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA16F,
        static_cast<GLsizei>(m_hdr_width),
        static_cast<GLsizei>(m_hdr_height),
        0, GL_RGBA, GL_FLOAT, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
    if (m_advanced_pbr_shader && m_advanced_pbr_shader->is_valid()) {
        for (int i = 0; i < 9; ++i) {
            std::string sh_name = "u_sh[" + std::to_string(i) + "]";
            m_loc_sh[i] = m_advanced_pbr_shader->get_uniform_location(sh_name);
        }
        for (int c = 0; c < 4; ++c) {
            std::string cm_name = "u_cascadeMatrices[" + std::to_string(c) + "]";
            m_loc_cascade_matrices[c] = m_advanced_pbr_shader->get_uniform_location(cm_name);
        }
    }

    static constexpr const char* s_skybox_vert = R"(#version 330 core
out vec3 v_worldDir;
uniform mat4 u_invViewProjNoTrans;
void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    vec4 clip = vec4(x, y, 1.0, 1.0);
    vec4 world = u_invViewProjNoTrans * clip;
    v_worldDir = world.xyz / max(world.w, 1e-5);
    gl_Position = vec4(x, y, 0.999999, 1.0);
}
)";
    static constexpr const char* s_skybox_frag = R"(#version 330 core
in vec3 v_worldDir;
out vec4 FragColor;
uniform samplerCube u_environmentMap;
uniform float u_envIntensity;
uniform float u_skyboxBlur;
uniform int u_outputHdr;
void main() {
    vec3 dir = normalize(v_worldDir);
    vec3 col = textureLod(u_environmentMap, dir, u_skyboxBlur * 4.0).rgb * u_envIntensity;
    if (u_outputHdr == 0) {
        float a = 2.51; float b = 0.03; float c = 2.43; float d = 0.59; float e = 0.14;
        col = clamp((col * (a * col + b)) / (col * (c * col + d) + e), 0.0, 1.0);
        col = pow(col, vec3(1.0 / 2.2));
    }
    FragColor = vec4(col, 1.0);
}
)";
    m_skybox_shader = std::make_unique<GlShaderProgram>();
    m_skybox_shader->compile_and_link(s_skybox_vert, s_skybox_frag);
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
    const auto& s_settings = view.shadow_settings;
    const uint32_t cascade_count = std::clamp(s_settings.cascade_count, 1u, 4u);
    if (view.shadows_enabled && m_cpu_renderer.shadow_map.light_vp_matrix.determinant() != 0.0f) {
        if (cascade_count > 1) {
            m_shadow_pass.begin_atlas();
            for (uint32_t c = 0; c < cascade_count; ++c) {
                m_shadow_pass.begin_cascade(c, m_cpu_renderer.shadow_map.cascade_matrices[c]);
                for (const auto& r_ptr : view.scene->renderables) {
                    if (r_ptr && r_ptr->visible && r_ptr->cast_shadows && r_ptr->mesh) {
                        m_shadow_pass.render_caster(m_resource_cache, *r_ptr->mesh, r_ptr->transform);
                    }
                }
            }
            m_shadow_pass.end();
        } else {
            m_shadow_pass.begin(m_cpu_renderer.shadow_map.light_vp_matrix);
            for (const auto& r_ptr : view.scene->renderables) {
                if (r_ptr && r_ptr->visible && r_ptr->cast_shadows && r_ptr->mesh) {
                    m_shadow_pass.render_caster(m_resource_cache, *r_ptr->mesh, r_ptr->transform);
                }
            }
            m_shadow_pass.end();
        }
    }

    // 2.5 Scene Depth Pre-Pass (Enables Screen-Space Contact Shadows, GTAO, SSR, DoF, TAA)
    const auto& vp = view.viewport;
    const bool need_scene_depth = (view.shadows_enabled && s_settings.enable_contact_shadows) ||
                                  (view.post_process.enabled && (
                                      view.post_process.gtao.enabled ||
                                      view.post_process.fog.enabled ||
                                      view.post_process.ssr.enabled ||
                                      view.post_process.dof.enabled ||
                                      view.post_process.anti_aliasing.enable_taa));
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

    bool has_refraction_draw = false;
    bool has_advanced_material = false;
    bool has_skinning_or_morph = false;
    for (const auto& cmd : m_cpu_renderer.draw_queue) {
        if (cmd.material) {
            if (cmd.material->shading_model != ShadingModel::Standard ||
                cmd.material->iridescence > 0.001f ||
                cmd.material->transmission > 0.001f) {
                has_advanced_material = true;
            }
            if (cmd.material->transmission > 0.001f) {
                has_refraction_draw = true;
            }
        }
        if (cmd.renderable && cmd.renderable->skeleton && !cmd.renderable->skeleton->joint_matrices.empty()) {
            has_skinning_or_morph = true;
        }
        if (cmd.mesh && !cmd.mesh->morph_targets.empty()) {
            has_skinning_or_morph = true;
        }
    }

    const bool use_postprocess = view.post_process.enabled &&
                                 (view.post_process.bloom.enabled ||
                                  view.post_process.gtao.enabled ||
                                  view.post_process.fog.enabled ||
                                  view.post_process.ssr.enabled ||
                                  view.post_process.dof.enabled ||
                                  view.post_process.anti_aliasing.enable_fxaa ||
                                  view.post_process.anti_aliasing.enable_taa ||
                                  view.post_process.grading.enabled);

    uint32_t active_fbo = target_fbo;
    if (use_postprocess || has_refraction_draw) {
        ensure_hdr_framebuffer(static_cast<uint32_t>(vp.width), static_cast<uint32_t>(vp.height));
        if (use_postprocess) {
            active_fbo = m_hdr_fbo;
        }
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
                               (view.shadows_enabled && (s_settings.enable_pcss || s_settings.enable_contact_shadows || cascade_count > 1)) ||
                               (view.scene->lights.size() > 1) ||
                               has_skinning_or_morph ||
                               use_postprocess;
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

        // Cascaded Shadow Atlas Uniforms
        active_shader->set_uniform("u_cascadeCount", static_cast<int>(cascade_count));
        if (cascade_count > 1) {
            for (int c = 0; c < 4; ++c) {
                GlShaderProgram::set_uniform_mat4(m_loc_cascade_matrices[c], m_cpu_renderer.shadow_map.cascade_matrices[c]);
            }
            active_shader->set_uniform(
                "u_cascadeSplits",
                m_cpu_renderer.shadow_map.cascade_splits[0],
                m_cpu_renderer.shadow_map.cascade_splits[1],
                m_cpu_renderer.shadow_map.cascade_splits[2],
                m_cpu_renderer.shadow_map.cascade_splits[3]
            );
        }

        // Froxel Clustered Forward Light Culling (activated when > 8 analytical lights are active)
        const bool use_clusters = (light_count > 8);
        active_shader->set_uniform("u_useClusterGrid", use_clusters ? 1 : 0);
        active_shader->set_uniform("u_clusterCellsTex", 9);
        active_shader->set_uniform("u_clusterIndicesTex", 10);
        if (use_clusters) {
            m_cluster_grid.build_frustum_grid(
                view.camera->get_fov_y(),
                view.camera->get_aspect_ratio(),
                view.camera->get_near_clip(),
                view.camera->get_far_clip()
            );
            m_cluster_grid.cull_lights(view.scene->lights, view.camera->get_view_matrix());
            m_resource_cache.update_cluster_textures(m_cluster_grid);

            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, m_resource_cache.cluster_cells_texture());
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, m_resource_cache.cluster_indices_texture());
            active_shader->set_uniform(
                "u_clusterParams",
                view.camera->get_near_clip(),
                m_cluster_grid.log_factor(),
                std::max(vp.width, 1.0f),
                std::max(vp.height, 1.0f)
            );
        } else {
            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE10);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        active_shader->set_uniform("u_refractionTexture", 11);
        active_shader->set_uniform("u_hasRefraction", 0);

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
                GlShaderProgram::set_uniform_vec3(m_loc_sh[i], view.scene->sh_irradiance.coefficients[i]);
            }
        } else {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            active_shader->set_uniform("u_hasEnvironmentMap", 0);
            active_shader->set_uniform("u_envIntensity", 1.0f);
            for (int i = 0; i < 9; ++i) {
                GlShaderProgram::set_uniform_vec3(m_loc_sh[i], Vec3(0.0f, 0.0f, 0.0f));
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

    // Execute 64-Bit Sorted Draw Stream with Automatic Hardware Instancing & GPU Skinning
    RenderPassID current_pass = static_cast<RenderPassID>(-1);
    const PbrMaterial* current_material = nullptr;
    GLuint current_vao = 0;
    bool refraction_captured = false;

    const size_t queue_size = m_cpu_renderer.draw_queue.size();
    for (size_t idx = 0; idx < queue_size; ) {
        const auto& cmd = m_cpu_renderer.draw_queue[idx];
        if (!cmd.mesh) {
            ++idx;
            continue;
        }

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
                // Capture opaque scene into mipmapped refraction texture before transparent pass
                if (has_refraction_draw && !refraction_captured && use_advanced && m_refraction_tex != 0) {
                    glActiveTexture(GL_TEXTURE11);
                    glBindTexture(GL_TEXTURE_2D, m_refraction_tex);
                    glCopyTexSubImage2D(
                        GL_TEXTURE_2D, 0, 0, 0,
                        static_cast<GLint>(vp.x), static_cast<GLint>(vp.y),
                        static_cast<GLsizei>(vp.width), static_cast<GLsizei>(vp.height)
                    );
                    glGenerateMipmap(GL_TEXTURE_2D);
                    active_shader->set_uniform("u_hasRefraction", 1);
                    refraction_captured = true;
                }
                glEnable(GL_DEPTH_TEST);
                glDepthMask(GL_FALSE);
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }

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
        if (!mesh_handle.is_valid || mesh_handle.vao == 0) {
            ++idx;
            continue;
        }

        if (mesh_handle.vao != current_vao) {
            current_vao = mesh_handle.vao;
            glBindVertexArray(current_vao);
        }

        // Check for Hardware GPU Skinning
        const bool has_skin = (cmd.renderable && cmd.renderable->skeleton && !cmd.renderable->skeleton->joint_matrices.empty());
        GlShaderProgram::set_uniform_int(active_shader->loc_has_skinning(), has_skin ? 1 : 0);
        if (has_skin) {
            m_resource_cache.update_skinning_ubo(*cmd.renderable->skeleton);
        }

        // Check how many consecutive commands can be hardware-instanced together (up to 128)
        size_t batch_count = 1;
        if (!has_skin && pass_id == RenderPassID::Opaque) {
            while (idx + batch_count < queue_size && batch_count < 128) {
                const auto& next_cmd = m_cpu_renderer.draw_queue[idx + batch_count];
                if (next_cmd.mesh != cmd.mesh ||
                    next_cmd.submesh_index != cmd.submesh_index ||
                    next_cmd.material != cmd.material ||
                    DrawSortKey64::decode_pass(next_cmd.sort_key) != pass_id ||
                    (next_cmd.renderable && next_cmd.renderable->skeleton && !next_cmd.renderable->skeleton->joint_matrices.empty())) {
                    break;
                }
                ++batch_count;
            }
        }

        if (batch_count > 1) {
            GlShaderProgram::set_uniform_int(active_shader->loc_is_instanced(), 1);
            GpuInstancingUboBlock inst_block{};
            const Mat44* first_cols = &cmd.world_transform;
            (void)first_cols;
            for (size_t b = 0; b < batch_count; ++b) {
                const auto& bcmd = m_cpu_renderer.draw_queue[idx + b];
                // Transpose row-major Mat44 into column-major std140 mat4 for GLSL UBO
                const Mat44 wt = bcmd.world_transform.transpose();
                const Mat44 nt = bcmd.normal_matrix.transpose();
                for (int m = 0; m < 16; ++m) {
                    inst_block.instances[b].model[m] = wt.m[m];
                    inst_block.instances[b].normal_matrix[m] = nt.m[m];
                }
            }
            m_resource_cache.update_instancing_ubo(inst_block.instances, batch_count);

            if (cmd.submesh_index < cmd.mesh->submeshes.size()) {
                const auto& sm = cmd.mesh->submeshes[cmd.submesh_index];
                if (mesh_handle.ebo != 0) {
                    glDrawElementsInstanced(
                        GL_TRIANGLES,
                        static_cast<GLsizei>(sm.index_count),
                        GL_UNSIGNED_INT,
                        reinterpret_cast<const void*>(static_cast<uintptr_t>(sm.index_offset * sizeof(uint32_t))),
                        static_cast<GLsizei>(batch_count)
                    );
                } else {
                    glDrawArraysInstanced(
                        GL_TRIANGLES,
                        static_cast<GLint>(sm.vertex_offset),
                        static_cast<GLsizei>(sm.index_count),
                        static_cast<GLsizei>(batch_count)
                    );
                }
            } else {
                if (mesh_handle.ebo != 0) {
                    glDrawElementsInstanced(
                        GL_TRIANGLES,
                        static_cast<GLsizei>(mesh_handle.index_count),
                        GL_UNSIGNED_INT,
                        nullptr,
                        static_cast<GLsizei>(batch_count)
                    );
                } else {
                    glDrawArraysInstanced(
                        GL_TRIANGLES,
                        0,
                        static_cast<GLsizei>(mesh_handle.index_count),
                        static_cast<GLsizei>(batch_count)
                    );
                }
            }
            idx += batch_count;
        } else {
            GlShaderProgram::set_uniform_int(active_shader->loc_is_instanced(), 0);
            GlShaderProgram::set_uniform_mat4(active_shader->loc_model(), cmd.world_transform);
            GlShaderProgram::set_uniform_mat4(active_shader->loc_normal_matrix(), cmd.normal_matrix);

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
            ++idx;
        }
    }

    if (current_vao != 0) {
        glBindVertexArray(0);
    }
    active_shader->unbind();

    // Optional Skybox Pass (renders at depth = 1.0 where no opaque geometry wrote)
    if (view.scene->render_skybox && view.scene->environment_map && m_skybox_shader && m_skybox_shader->is_valid()) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);

        m_skybox_shader->bind();
        Mat44 view_no_trans = view.camera->get_view_matrix();
        view_no_trans.m[12] = 0.0f;
        view_no_trans.m[13] = 0.0f;
        view_no_trans.m[14] = 0.0f;
        bool inv_ok = false;
        Mat44 inv_vp_no_trans = (view.camera->get_projection_matrix() * view_no_trans).inverse(inv_ok);

        m_skybox_shader->set_uniform("u_invViewProjNoTrans", inv_vp_no_trans);
        m_skybox_shader->set_uniform("u_environmentMap", 7);
        m_skybox_shader->set_uniform("u_envIntensity", view.scene->environment_intensity);
        m_skybox_shader->set_uniform("u_skyboxBlur", view.scene->skybox_blur);
        m_skybox_shader->set_uniform("u_outputHdr", use_postprocess ? 1 : 0);

        uint32_t env_tex = m_resource_cache.get_or_upload_environment_map(*view.scene->environment_map);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, env_tex);

        glBindVertexArray(m_skybox_vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
        m_skybox_shader->unbind();
    }

    // Cleanly unbind textures and restore default active texture unit 0 for 2D UI / HUD overlay
    for (int t = 11; t >= 8; --t) {
        glActiveTexture(GL_TEXTURE0 + t);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
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

#pragma once

#include "orion/scene/view.hpp"
#include "orion/scene/renderer.hpp"
#include "orion/gpu/gl_shader_program.hpp"
#include "orion/gpu/gl_state_guard.hpp"
#include "orion/gpu/gpu_resource_cache.hpp"
#include "orion/gpu/gpu_shadow_pass.hpp"
#include "orion/gpu/gpu_scene_depth_pass.hpp"
#include "orion/lighting/cluster_grid.hpp"
#include "orion/postprocess/postprocess_pipeline.hpp"
#include <memory>
#include <cstdint>

namespace orion {

/// @brief Sovereign GPU hardware 3D PBR rendering backend for Nisaba.
/// Consumes 64-bit sorted draw commands, executes multi-pass cascaded shadow depth writes,
/// manages std140 uniform blocks, binds hardware textures, and dispatches Early-Z PBR passes.
class GpuBackend3D {
public:
    explicit GpuBackend3D(uint32_t shadow_resolution = 2048);
    ~GpuBackend3D();

    GpuBackend3D(const GpuBackend3D&) = delete;
    GpuBackend3D& operator=(const GpuBackend3D&) = delete;

    /// @brief Executes complete hardware rendering of a 3D view into a destination framebuffer.
    /// @param view 3D View defining camera, scene, lights, and viewport.
    /// @param target_fbo Target OpenGL framebuffer ID (0 = default window backbuffer).
    /// @param clear_buffers True if target buffer should be cleared before drawing.
    void render_view(
        const View3D& view,
        uint32_t target_fbo = 0,
        bool clear_buffers = true
    );

    [[nodiscard]] GpuResourceCache& resource_cache() noexcept { return m_resource_cache; }
    [[nodiscard]] GpuShadowPass& shadow_pass() noexcept { return m_shadow_pass; }
    [[nodiscard]] GpuSceneDepthPass& scene_depth_pass() noexcept { return m_depth_pass; }
    [[nodiscard]] PostProcessPipeline& postprocess_pipeline() noexcept { return m_postprocess; }
    [[nodiscard]] const RenderStats& last_stats() const noexcept { return m_cpu_renderer.get_stats(); }

    /// @brief Dynamically updates vertex buffer on GPU for animated or deformed meshes.
    void update_mesh_buffer(const Mesh3D& mesh) noexcept { m_resource_cache.update_mesh_buffer(mesh); }

private:
    Renderer3D m_cpu_renderer;
    ClusterGrid m_cluster_grid{16, 9, 24};
    GpuResourceCache m_resource_cache;
    GpuShadowPass m_shadow_pass;
    GpuSceneDepthPass m_depth_pass;
    PostProcessPipeline m_postprocess;
    std::unique_ptr<GlShaderProgram> m_pbr_shader;
    std::unique_ptr<GlShaderProgram> m_advanced_pbr_shader;
    std::unique_ptr<GlShaderProgram> m_skybox_shader;
    uint32_t m_skybox_vao{0};
    int32_t m_loc_sh[9]{-1, -1, -1, -1, -1, -1, -1, -1, -1};
    int32_t m_loc_cascade_matrices[4]{-1, -1, -1, -1};

    uint32_t m_hdr_fbo{0};
    uint32_t m_hdr_color_tex{0};
    uint32_t m_hdr_depth_rbo{0};
    uint32_t m_refraction_tex{0};
    uint32_t m_hdr_width{0};
    uint32_t m_hdr_height{0};

    void ensure_hdr_framebuffer(uint32_t width, uint32_t height);
    void cleanup_hdr_framebuffer() noexcept;
    void init_shaders();
};

} // namespace orion

#pragma once

#include "nisaba/render3d/scene/view.hpp"
#include "nisaba/render3d/scene/renderer.hpp"
#include "nisaba/render3d/gpu/gl_shader_program.hpp"
#include "nisaba/render3d/gpu/gl_state_guard.hpp"
#include "nisaba/render3d/gpu/gpu_resource_cache.hpp"
#include "nisaba/render3d/gpu/gpu_shadow_pass.hpp"
#include <memory>
#include <cstdint>

namespace nisaba::render3d {

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
    [[nodiscard]] const RenderStats& last_stats() const noexcept { return m_cpu_renderer.get_stats(); }

    /// @brief Dynamically updates vertex buffer on GPU for animated or deformed meshes.
    void update_mesh_buffer(const Mesh3D& mesh) noexcept { m_resource_cache.update_mesh_buffer(mesh); }

private:
    Renderer3D m_cpu_renderer;
    GpuResourceCache m_resource_cache;
    GpuShadowPass m_shadow_pass;
    std::unique_ptr<GlShaderProgram> m_pbr_shader;

    void init_shaders();
};

} // namespace nisaba::render3d

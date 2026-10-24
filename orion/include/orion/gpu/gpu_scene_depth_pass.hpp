#pragma once

#include "orion/gpu/gl_shader_program.hpp"
#include "orion/gpu/gpu_resource_cache.hpp"
#include "orion/geometry/mesh.hpp"
#include "orion/math/mat44.hpp"
#include <memory>
#include <cstdint>

namespace orion {


/// @brief Sovereign GPU Scene Depth Pre-Pass.
/// Generates a full-resolution screen-space depth buffer to enable:
/// 1. High-fidelity Screen-Space Contact Shadows (SSCS)
/// 2. Early-Z rejection optimization for complex forward PBR pixel shaders
/// 3. Zero-overhead Ground-Truth Ambient Occlusion (GTAO) and Temporal Reprojection.
class GpuSceneDepthPass {
public:
    GpuSceneDepthPass();
    ~GpuSceneDepthPass();

    GpuSceneDepthPass(const GpuSceneDepthPass&) = delete;
    GpuSceneDepthPass& operator=(const GpuSceneDepthPass&) = delete;

    /// @brief Configures target viewport resolution and binds the depth prepass framebuffer.
    void begin(uint32_t width, uint32_t height, const Mat44& view_projection);

    /// @brief Renders an opaque mesh into the scene depth buffer.
    void render_object(
        GpuResourceCache& cache,
        const Mesh3D& mesh,
        const Mat44& world_transform
    );

    /// @brief Concludes depth prepass and unbinds the framebuffer.
    void end();

    [[nodiscard]] uint32_t depth_texture() const noexcept { return m_depth_texture; }
    [[nodiscard]] uint32_t width() const noexcept { return m_width; }
    [[nodiscard]] uint32_t height() const noexcept { return m_height; }
    [[nodiscard]] bool is_valid() const noexcept { return m_depth_texture != 0; }

private:
    uint32_t m_width{0};
    uint32_t m_height{0};
    uint32_t m_fbo{0};
    uint32_t m_depth_texture{0};
    Mat44 m_view_projection{Mat44::identity()};

    std::unique_ptr<GlShaderProgram> m_depth_shader;
    int m_loc_view_proj{-1};
    int m_loc_model{-1};

    void ensure_capacity(uint32_t width, uint32_t height);
    void cleanup() noexcept;
};

} // namespace orion

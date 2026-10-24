#pragma once

#include "orion/gpu/gl_shader_program.hpp"
#include "orion/gpu/gpu_resource_cache.hpp"
#include "orion/geometry/mesh.hpp"
#include "orion/math/mat44.hpp"
#include <cstdint>

namespace orion {


/// @brief Sovereign GPU hardware shadow depth pass.
/// Manages high-precision offscreen depth attachments, front-face culling to eliminate
/// Peter-Panning, and light-space depth rendering.
class GpuShadowPass {
public:
    explicit GpuShadowPass(uint32_t resolution = 2048);
    ~GpuShadowPass();

    GpuShadowPass(const GpuShadowPass&) = delete;
    GpuShadowPass& operator=(const GpuShadowPass&) = delete;

    /// @brief Prepares shadow framebuffer and configures front-face culling for single-map depth write.
    void begin(const Mat44& light_space_matrix);

    /// @brief Begins a multi-cascade shadow atlas pass (2x2 quadrant layout), clearing the full depth buffer.
    void begin_atlas();

    /// @brief Configures viewport quadrant [0..3] and light-space matrix for a specific cascade within the atlas.
    void begin_cascade(uint32_t cascade_index, const Mat44& cascade_vp_matrix);

    /// @brief Renders a shadow caster mesh into the light depth texture.
    void render_caster(
        GpuResourceCache& cache,
        const Mesh3D& mesh,
        const Mat44& world_transform
    );

    /// @brief Concludes shadow pass and unbinds the shadow framebuffer.
    void end();

    [[nodiscard]] uint32_t depth_texture() const noexcept { return m_depth_texture; }
    [[nodiscard]] uint32_t resolution() const noexcept { return m_resolution; }
    [[nodiscard]] const Mat44& light_space_matrix() const noexcept { return m_light_space_matrix; }

private:
    uint32_t m_resolution{2048};
    uint32_t m_fbo{0};
    uint32_t m_depth_texture{0};
    Mat44 m_light_space_matrix{Mat44::identity()};

    std::unique_ptr<GlShaderProgram> m_depth_shader;
    int m_loc_light_space{-1};
    int m_loc_model{-1};

    void init_framebuffer();
    void cleanup() noexcept;
};

} // namespace orion

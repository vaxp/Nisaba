#pragma once

#include "nisaba/render3d/gpu/gl_shader_program.hpp"
#include "nisaba/render3d/gpu/gpu_resource_cache.hpp"
#include "nisaba/render3d/geometry/mesh.hpp"
#include "nisaba/physics/math/mat44.hpp"
#include <cstdint>

namespace nisaba::render3d {

using Mat44 = nisaba::physics::Mat44;

/// @brief Sovereign GPU hardware shadow depth pass.
/// Manages high-precision offscreen depth attachments, front-face culling to eliminate
/// Peter-Panning, and light-space depth rendering.
class GpuShadowPass {
public:
    explicit GpuShadowPass(uint32_t resolution = 2048);
    ~GpuShadowPass();

    GpuShadowPass(const GpuShadowPass&) = delete;
    GpuShadowPass& operator=(const GpuShadowPass&) = delete;

    /// @brief Prepares shadow framebuffer and configures front-face culling for depth write.
    void begin(const Mat44& light_space_matrix);

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

    void init_framebuffer();
    void cleanup() noexcept;
};

} // namespace nisaba::render3d

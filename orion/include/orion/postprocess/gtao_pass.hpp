#pragma once

#include "orion/postprocess/postprocess_settings.hpp"
#include "orion/gpu/gl_shader_program.hpp"
#include "orion/math/mat44.hpp"
#include "orion/math/vec3.hpp"
#include <memory>
#include <cstdint>

namespace orion {


/// @brief Sovereign GPU Ground Truth Ambient Occlusion (GTAO) Pass.
/// Computes physical cosine-weighted horizon angle integrals directly from the scene depth buffer,
/// followed by an edge-preserving bilateral spatial reconstruction filter.
class GtaoPass {
public:
    GtaoPass();
    ~GtaoPass();

    GtaoPass(const GtaoPass&) = delete;
    GtaoPass& operator=(const GtaoPass&) = delete;

    /// @brief Evaluates GTAO horizon angles and executes bilateral spatial reconstruction.
    /// @param scene_depth_texture Input scene depth texture ID.
    /// @param width Screen viewport width.
    /// @param height Screen viewport height.
    /// @param proj Camera projection matrix.
    /// @param settings GTAO radius, slices, steps, and sharpness settings.
    void render(
        uint32_t scene_depth_texture,
        uint32_t width,
        uint32_t height,
        const Mat44& proj,
        const GtaoSettings& settings
    );

    [[nodiscard]] uint32_t ao_texture() const noexcept { return m_blurred_ao_tex; }
    [[nodiscard]] bool is_valid() const noexcept { return m_blurred_ao_tex != 0; }

private:
    uint32_t m_width{0};
    uint32_t m_height{0};

    uint32_t m_raw_ao_fbo{0};
    uint32_t m_raw_ao_tex{0};

    uint32_t m_blurred_ao_fbo{0};
    uint32_t m_blurred_ao_tex{0};

    uint32_t m_fullscreen_vao{0};

    std::unique_ptr<GlShaderProgram> m_gtao_shader;
    std::unique_ptr<GlShaderProgram> m_bilateral_shader;

    void ensure_buffers(uint32_t width, uint32_t height);
    void cleanup() noexcept;
    void init_shaders();
    void render_quad() const noexcept;
};

} // namespace orion

#pragma once

#include "orion/postprocess/postprocess_settings.hpp"
#include "orion/postprocess/bloom_pass.hpp"
#include "orion/postprocess/gtao_pass.hpp"
#include "orion/gpu/gl_shader_program.hpp"
#include "orion/camera/camera.hpp"
#include "orion/lighting/light.hpp"
#include <memory>
#include <cstdint>

namespace orion {

/// @brief Sovereign High-End Cinematic Post-Processing Pipeline for Nisaba.
/// Coordinates full-pipeline render passes:
/// 1. Ground Truth Ambient Occlusion (GTAO) with bilateral edge preservation
/// 2. 5-Stage 13-tap/9-tap Progressive Pyramid Bloom
/// 3. Volumetric Height Fog with Beer-Lambert extinction and sun in-scattering
/// 4. ACES Filmic Tone Mapping, photographic exposure, contrast S-curves, and color grading.
class PostProcessPipeline {
public:
    PostProcessPipelineSettings settings{};

    PostProcessPipeline();
    ~PostProcessPipeline();

    PostProcessPipeline(const PostProcessPipeline&) = delete;
    PostProcessPipeline& operator=(const PostProcessPipeline&) = delete;

    /// @brief Executes complete post-processing chain into destination framebuffer.
    /// @param src_hdr_texture Input scene HDR color texture ID.
    /// @param scene_depth_texture Input scene depth texture ID.
    /// @param width Viewport width in pixels.
    /// @param height Viewport height in pixels.
    /// @param camera View camera.
    /// @param sun Directional sunlight for atmospheric in-scattering.
    /// @param dst_fbo Destination target framebuffer ID (0 = backbuffer).
    void render(
        uint32_t src_hdr_texture,
        uint32_t scene_depth_texture,
        uint32_t width,
        uint32_t height,
        const Camera3D& camera,
        const Light3D* sun = nullptr,
        uint32_t dst_fbo = 0
    );

    [[nodiscard]] BloomPass& bloom_pass() noexcept { return m_bloom; }
    [[nodiscard]] GtaoPass& gtao_pass() noexcept { return m_gtao; }

private:
    BloomPass m_bloom;
    GtaoPass m_gtao;

    uint32_t m_fullscreen_vao{0};
    std::unique_ptr<GlShaderProgram> m_composite_shader;

    void init_shaders();
    void render_quad() const noexcept;
};

} // namespace orion

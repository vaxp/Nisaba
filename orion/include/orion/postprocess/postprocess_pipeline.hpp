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

    /// @brief CPU-accessible tone mapping evaluation for unit testing and software verification.
    [[nodiscard]] static Vec3 evaluate_tone_mapping(const Vec3& hdr_color, ToneMappingOperator op) noexcept;

private:
    BloomPass m_bloom;
    GtaoPass m_gtao;

    uint32_t m_fullscreen_vao{0};
    std::unique_ptr<GlShaderProgram> m_composite_shader;
    std::unique_ptr<GlShaderProgram> m_fxaa_taa_shader;

    uint32_t m_ldr_fbo{0};
    uint32_t m_ldr_tex{0};
    uint32_t m_taa_history_fbo{0};
    uint32_t m_taa_history_tex{0};
    uint32_t m_aa_width{0};
    uint32_t m_aa_height{0};
    bool m_taa_history_valid{false};
    Mat44 m_prev_view_proj{Mat44::identity()};

    void init_shaders();
    void ensure_aa_buffers(uint32_t width, uint32_t height);
    void cleanup_aa_buffers() noexcept;
    void render_quad() const noexcept;
};

} // namespace orion

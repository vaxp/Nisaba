#pragma once

#include "orion/postprocess/postprocess_settings.hpp"
#include "orion/gpu/gl_shader_program.hpp"
#include <vector>
#include <memory>
#include <cstdint>

namespace orion {

struct BloomMipLevel {
    uint32_t fbo{0};
    uint32_t texture{0};
    uint32_t width{0};
    uint32_t height{0};
};

/// @brief Sovereign GPU 5-Stage Progressive Downsample / Upsample Bloom Pyramid.
/// Implements 13-tap dual-filtered downsampling with Karis partial averaging to completely
/// eliminate specular fireflies, and 9-tap tent upsampling with weighted additive accumulation.
class BloomPass {
public:
    static constexpr size_t NUM_BLOOM_MIPS = 5;

    BloomPass();
    ~BloomPass();

    BloomPass(const BloomPass&) = delete;
    BloomPass& operator=(const BloomPass&) = delete;

    /// @brief Executes bloom extraction, 5-stage downsampling pyramid, and progressive upsampling blend.
    /// @param src_hdr_texture Input HDR color texture ID.
    /// @param width Screen viewport width.
    /// @param height Screen viewport height.
    /// @param settings Bloom threshold, knee, intensity, and scatter parameters.
    void render(
        uint32_t src_hdr_texture,
        uint32_t width,
        uint32_t height,
        const BloomSettings& settings
    );

    /// @brief Returns the final accumulated bloom texture ID ready for composition.
    [[nodiscard]] uint32_t bloom_texture() const noexcept {
        return m_mips.empty() ? 0 : m_mips[0].texture;
    }

    [[nodiscard]] bool is_valid() const noexcept {
        return !m_mips.empty() && m_mips[0].texture != 0;
    }

private:
    uint32_t m_base_width{0};
    uint32_t m_base_height{0};
    std::vector<BloomMipLevel> m_mips;

    uint32_t m_fullscreen_vao{0};

    std::unique_ptr<GlShaderProgram> m_downsample_shader;
    std::unique_ptr<GlShaderProgram> m_upsample_shader;

    void ensure_mips(uint32_t width, uint32_t height);
    void cleanup() noexcept;
    void init_shaders();
    void render_quad() const noexcept;
};

} // namespace orion

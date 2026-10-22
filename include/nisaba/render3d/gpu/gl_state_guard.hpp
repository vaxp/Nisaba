#pragma once

#include <cstdint>

namespace nisaba::render3d {

/// @brief Sovereign RAII OpenGL 3.3 / ES 3.0+ Pipeline State Guard.
/// Captures all relevant rasterizer, depth, culling, blending, and buffer binding states
/// upon creation, and restores them unconditionally upon destruction.
/// Guarantees zero state pollution between 3D PBR rendering and Nisaba's 2D vector canvas.
class GlStateGuard3D {
public:
    GlStateGuard3D();
    ~GlStateGuard3D();

    GlStateGuard3D(const GlStateGuard3D&) = delete;
    GlStateGuard3D& operator=(const GlStateGuard3D&) = delete;

    /// @brief Explicitly restores the captured OpenGL state prior to destruction.
    void restore() noexcept;

private:
    bool m_restored{false};

    // Shader & Buffer Bindings
    int32_t m_prev_program{0};
    int32_t m_prev_vao{0};
    int32_t m_prev_vbo{0};
    int32_t m_prev_ebo{0};
    int32_t m_prev_fbo{0};

    // Depth Pipeline
    uint8_t m_prev_depth_test{0};
    uint8_t m_prev_depth_mask{0};
    int32_t m_prev_depth_func{0};

    // Face Culling Pipeline
    uint8_t m_prev_cull_face{0};
    int32_t m_prev_cull_mode{0};
    int32_t m_prev_front_face{0};

    // Color Blending Pipeline
    uint8_t m_prev_blend{0};
    int32_t m_prev_blend_src_rgb{0};
    int32_t m_prev_blend_dst_rgb{0};
    int32_t m_prev_blend_src_alpha{0};
    int32_t m_prev_blend_dst_alpha{0};
    int32_t m_prev_blend_eq_rgb{0};
    int32_t m_prev_blend_eq_alpha{0};

    // Scissor & Viewport
    uint8_t m_prev_scissor_test{0};
    int32_t m_prev_scissor_box[4]{0, 0, 0, 0};
    int32_t m_prev_viewport[4]{0, 0, 0, 0};

    // Texture Unit
    int32_t m_prev_active_texture{0};
};

} // namespace nisaba::render3d

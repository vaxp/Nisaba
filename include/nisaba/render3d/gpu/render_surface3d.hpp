#pragma once

#include "nisaba/render3d/gpu/gpu_backend3d.hpp"
#include <cstdint>
#include <memory>

namespace nisaba::render3d {

/// @brief Sovereign GPU Render Surface supporting zero-copy 3D PBR scene rendering
/// combined with 2D vector HUD / UI overlay compositing.
class RenderSurface3D {
public:
    RenderSurface3D(uint32_t width, uint32_t height);
    ~RenderSurface3D();

    RenderSurface3D(const RenderSurface3D&) = delete;
    RenderSurface3D& operator=(const RenderSurface3D&) = delete;

    /// @brief Resizes surface attachments if dimensions changed.
    void resize(uint32_t width, uint32_t height);

    /// @brief Renders 3D View directly into this surface's hardware framebuffer.
    void render_3d(GpuBackend3D& backend, const View3D& view, bool clear_first = true);

    /// @brief Reads surface color buffer into CPU host memory (RGBA8888).
    void read_pixels(void* out_rgba) const;

    [[nodiscard]] uint32_t fbo() const noexcept { return m_fbo; }
    [[nodiscard]] uint32_t color_texture() const noexcept { return m_color_texture; }
    [[nodiscard]] uint32_t width() const noexcept { return m_width; }
    [[nodiscard]] uint32_t height() const noexcept { return m_height; }

private:
    uint32_t m_width{0};
    uint32_t m_height{0};

    uint32_t m_fbo{0};
    uint32_t m_color_texture{0};
    uint32_t m_depth_stencil_rb{0};

    void init_surface();
    void cleanup() noexcept;
};

} // namespace nisaba::render3d

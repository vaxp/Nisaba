#pragma once

#include "nisaba/compositor/color.hpp"
#include "nisaba/compositor/damage_ring.hpp"
#include "nisaba/compositor/dmabuf_texture.hpp"
#include "nisaba/compositor/dual_kawase.hpp"
#include "nisaba/compositor/egl_context.hpp"
#include "nisaba/compositor/matrix3.hpp"
#include "nisaba/compositor/render_pass.hpp"
#include "nisaba/compositor/render_target.hpp"
#include "nisaba/compositor/renderer.hpp"
#include "nisaba/compositor/scene_tree.hpp"

#include <memory>
#include <vector>

namespace nisaba::compositor {

/**
 * @brief High-level sovereign Compositor Canvas for modern Wayland window managers.
 *
 * Combines low-level zero-copy GPU window composition with Nisaba's vector graphics
 * features (dock panels, rounded rectangles, dropshadows, and frosted glass effects).
 */
class CompositorCanvas {
public:
    static std::unique_ptr<CompositorCanvas> create(
        std::shared_ptr<EglContext> egl = nullptr,
        std::shared_ptr<Renderer> renderer = nullptr);

    CompositorCanvas(
        std::shared_ptr<EglContext> egl,
        std::shared_ptr<Renderer> renderer);

    ~CompositorCanvas();

    CompositorCanvas(const CompositorCanvas&) = delete;
    CompositorCanvas& operator=(const CompositorCanvas&) = delete;

    /**
     * @brief Begins a compositor rendering frame targeting an FBO or hardware scanout target.
     */
    bool beginFrame(
        std::shared_ptr<RenderTarget> target,
        wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL);

    /**
     * @brief Sets the active damage region to restrict rendering commands.
     */
    void setDamage(const Region* damage);

    /**
     * @brief Clears the current frame target with the specified background color.
     */
    void clear(const Color& color);

    /**
     * @brief Renders a Wayland client surface texture (DMA-BUF or SHM).
     */
    void drawWindowSurface(
        const Texture& texture,
        const RectI& destBox,
        const RectF& srcBox = RectF{0.0, 0.0, 1.0, 1.0},
        float alpha = 1.0f,
        wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL);

    /**
     * @brief Draws a solid or translucent colored rectangle.
     */
    void drawRect(const RectI& box, const Color& color);

    /**
     * @brief Applies a Dual Kawase frosted glass blur to a sub-region or backdrop.
     */
    void drawFrostedGlass(
        const Texture& source,
        const RectI& destBox,
        const RectI& srcBox = RectI{0, 0, 0, 0},
        const DualKawaseConfig& config = {},
        const Texture* maskTexture = nullptr);

    /**
     * @brief Renders a complete scene graph tree.
     */
    void drawScene(const SceneTree& tree);

    /**
     * @brief Finalizes and submits all accumulated commands to the GPU.
     */
    bool endFrame();

    [[nodiscard]] const std::shared_ptr<Renderer>& renderer() const noexcept { return m_renderer; }
    [[nodiscard]] const std::shared_ptr<EglContext>& egl() const noexcept { return m_egl; }

private:
    std::shared_ptr<EglContext> m_egl;
    std::shared_ptr<Renderer> m_renderer;
    std::unique_ptr<RenderPass> m_currentPass;
    const Region* m_currentDamage{nullptr};
};

} // namespace nisaba::compositor

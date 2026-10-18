#pragma once

#include "nisaba/compositor/color.hpp"
#include "nisaba/compositor/geometry.hpp"
#include "nisaba/compositor/matrix3.hpp"
#include "nisaba/compositor/render_target.hpp"
#include "nisaba/compositor/shader.hpp"
#include "nisaba/compositor/texture.hpp"
#include "nisaba/compositor/dual_kawase.hpp"

#include <memory>
#include <optional>

namespace nisaba::compositor {

/**
 * @brief Sovereign RenderPass interface accumulating 2D compositing drawing commands
 * and submitting them efficiently to GPU, completely decoupled from any graphics API.
 */
class IRenderPass {
public:
    virtual ~IRenderPass() = default;

    /**
     * @brief Clears the entire target buffer with specified color.
     */
    virtual void clear(const Color& color) const noexcept = 0;

    /**
     * @brief Renders a solid color rectangle with optional region clipping.
     */
    virtual void renderRect(
        const RectI& box,
        const Color& color,
        const Region* clip = nullptr) const = 0;

    /**
     * @brief Renders a textured quad with source sub-box, alpha, and surface transform.
     */
    virtual void renderTexture(
        const ITexture& texture,
        const RectI& destBox,
        const RectF& srcBox = RectF{0.0, 0.0, 1.0, 1.0},
        float alpha = 1.0f,
        wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL,
        const Region* clip = nullptr) const = 0;

    /**
     * @brief Submits the render pass commands to the graphics device.
     */
    virtual bool submit() = 0;

    [[nodiscard]] virtual RenderBackendType backendType() const noexcept = 0;

    /**
     * @brief Native API handle escape hatch for custom shader nodes and extensions.
     */
    [[nodiscard]] virtual void* nativeHandle() noexcept = 0;
};

/**
 * @brief Sovereign OpenGL ES implementation of IRenderPass.
 */
class RenderPass : public IRenderPass {
public:
    static std::unique_ptr<RenderPass> begin(
        std::shared_ptr<RenderTarget> target,
        std::shared_ptr<ShaderManager> shaders,
        wl_output_transform outputTransform = WL_OUTPUT_TRANSFORM_FLIPPED_180);

    RenderPass(
        std::shared_ptr<RenderTarget> target,
        std::shared_ptr<ShaderManager> shaders,
        wl_output_transform outputTransform);

    ~RenderPass() override;

    RenderPass(const RenderPass&) = delete;
    RenderPass& operator=(const RenderPass&) = delete;

    void clear(const Color& color) const noexcept override;

    void renderRect(
        const RectI& box,
        const Color& color,
        const Region* clip = nullptr) const override;

    void renderTexture(
        const ITexture& texture,
        const RectI& destBox,
        const RectF& srcBox = RectF{0.0, 0.0, 1.0, 1.0},
        float alpha = 1.0f,
        wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL,
        const Region* clip = nullptr) const override;

    void renderTexture(
        const Texture& texture,
        const RectI& destBox,
        const RectF& srcBox = RectF{0.0, 0.0, 1.0, 1.0},
        float alpha = 1.0f,
        wl_output_transform transform = WL_OUTPUT_TRANSFORM_NORMAL,
        const Region* clip = nullptr) const;

    void renderBlur(
        DualKawaseBlur& blurEngine,
        const Texture& source,
        const RectI& destBox,
        const RectI& srcBox = RectI{0, 0, 0, 0},
        const DualKawaseConfig& config = {},
        const Texture* maskTexture = nullptr);

    bool submit() override;

    [[nodiscard]] RenderBackendType backendType() const noexcept override {
        return RenderBackendType::OpenGL;
    }

    [[nodiscard]] void* nativeHandle() noexcept override {
        return m_target.get();
    }

    [[nodiscard]] const std::shared_ptr<RenderTarget>& target() const noexcept { return m_target; }
    [[nodiscard]] const Matrix3& projection() const noexcept { return m_projection; }

private:
    void renderQuads(
        const RectI& box,
        const Region* clip,
        GLint posAttrib) const;

    std::shared_ptr<RenderTarget> m_target;
    std::shared_ptr<ShaderManager> m_shaders;
    Matrix3 m_projection;
    bool m_submitted{false};
};

} // namespace nisaba::compositor

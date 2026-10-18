#pragma once

#include "nisaba/compositor/drm_formats.hpp"
#include "nisaba/compositor/geometry.hpp"
#include "nisaba/compositor/buffer.hpp"

#include <memory>
#include <string_view>
#include <wayland-server-protocol.h>

namespace nisaba::compositor {

class IRenderPass;
class IRenderTarget;

/**
 * @brief Memory allocator interface for allocating displayable buffers.
 */
class IAllocator {
public:
    virtual ~IAllocator() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    /**
     * @brief Allocates a hardware buffer matching the given format and modifier constraints.
     */
    [[nodiscard]] virtual std::shared_ptr<IBuffer> allocate(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        uint64_t modifier) = 0;
};

/**
 * @brief External graphics engine interface for Compositor.
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    /**
     * @brief Queries formats and modifiers supported by the rendering engine.
     */
    [[nodiscard]] virtual const DrmFormatSet& supportedFormats() const noexcept = 0;

    /**
     * @brief Imports an external DMA-BUF into the rendering engine as a texture/render target.
     */
    [[nodiscard]] virtual std::shared_ptr<IBuffer> importBuffer(const DmaBufAttributes& attrs) = 0;

    /**
     * @brief Begins a rendering pass targeted at the specified output buffer.
     */
    virtual bool beginFrame(IBuffer& target, const RectI& damage) = 0;

    /**
     * @brief Finalizes and submits the frame to the GPU.
     */
    virtual bool endFrame() = 0;

    /**
     * @brief Begins a rendering pass targeted at the specified render target.
     */
    [[nodiscard]] virtual std::unique_ptr<IRenderPass> beginPass(
        std::shared_ptr<IRenderTarget> target,
        wl_output_transform outputTransform = WL_OUTPUT_TRANSFORM_NORMAL) = 0;
};

} // namespace nisaba::compositor

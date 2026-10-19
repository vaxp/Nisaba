#pragma once

#include <cstdint>
#include <memory>
#include "nisaba/gpu/gpu_types.hpp"
#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::gpu {

class GpuTexture {
public:
    static std::shared_ptr<GpuTexture> create(
        std::shared_ptr<GpuDevice> device,
        uint32_t width,
        uint32_t height,
        GpuTextureFormat format = GpuTextureFormat::RGBA8,
        GpuTextureFilter filter = GpuTextureFilter::Linear,
        GpuTextureWrap wrap = GpuTextureWrap::Clamp
    );

    static std::shared_ptr<GpuTexture> create_from_pixmap(
        std::shared_ptr<GpuDevice> device,
        const PixmapRef& pixmap,
        GpuTextureFilter filter = GpuTextureFilter::Linear,
        GpuTextureWrap wrap = GpuTextureWrap::Clamp
    );

    /// Wraps an existing native GPU texture handle (e.g. from compositor or client DMA-BUF) without copying.
    static std::shared_ptr<GpuTexture> from_native_handle(
        std::shared_ptr<GpuDevice> device,
        uint32_t texture_id,
        uint32_t width,
        uint32_t height,
        GpuTextureFormat format = GpuTextureFormat::RGBA8,
        bool owns_texture = false
    );

    ~GpuTexture();

    [[nodiscard]] uint32_t id() const noexcept { return texture_id_; }
    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] GpuTextureFormat format() const noexcept { return format_; }
    [[nodiscard]] bool owns_texture() const noexcept { return owns_texture_; }

    void bind(uint32_t unit = 0);
    void update(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* pixels);

private:
    GpuTexture(
        std::shared_ptr<GpuDevice> device,
        uint32_t texture_id,
        uint32_t width,
        uint32_t height,
        GpuTextureFormat format,
        bool owns_texture = true
    );

    std::shared_ptr<GpuDevice> device_;
    uint32_t texture_id_{0};
    uint32_t width_{0};
    uint32_t height_{0};
    GpuTextureFormat format_{GpuTextureFormat::RGBA8};
    bool owns_texture_{true};
};

} // namespace nisaba::gpu

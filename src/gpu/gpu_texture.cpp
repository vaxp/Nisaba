#include "nisaba/gpu/gpu_texture.hpp"

namespace nisaba::gpu {

GpuTexture::GpuTexture(
    std::shared_ptr<GpuDevice> device,
    uint32_t texture_id,
    uint32_t width,
    uint32_t height,
    GpuTextureFormat format
) : device_(std::move(device)),
    texture_id_(texture_id),
    width_(width),
    height_(height),
    format_(format) {}

GpuTexture::~GpuTexture() {
    if (device_ && texture_id_ != 0) {
        device_->delete_texture(texture_id_);
    }
}

std::shared_ptr<GpuTexture> GpuTexture::create(
    std::shared_ptr<GpuDevice> device,
    uint32_t width,
    uint32_t height,
    GpuTextureFormat format,
    GpuTextureFilter filter,
    GpuTextureWrap wrap
) {
    if (!device || width == 0 || height == 0) return nullptr;

    uint32_t tex_id = device->create_texture();
    if (tex_id == 0) return nullptr;

    device->bind_texture(0, tex_id);
    device->upload_texture_image_2d(width, height, nullptr, format, filter, wrap);

    return std::shared_ptr<GpuTexture>(new GpuTexture(device, tex_id, width, height, format));
}

std::shared_ptr<GpuTexture> GpuTexture::create_from_pixmap(
    std::shared_ptr<GpuDevice> device,
    const PixmapRef& pixmap,
    GpuTextureFilter filter,
    GpuTextureWrap wrap
) {
    if (!device || pixmap.width() == 0 || pixmap.height() == 0) return nullptr;

    uint32_t tex_id = device->create_texture();
    if (tex_id == 0) return nullptr;

    device->bind_texture(0, tex_id);
    device->upload_texture_image_2d(
        pixmap.width(), pixmap.height(),
        pixmap.data(),
        GpuTextureFormat::RGBA8,
        filter,
        wrap
    );

    return std::shared_ptr<GpuTexture>(new GpuTexture(device, tex_id, pixmap.width(), pixmap.height(), GpuTextureFormat::RGBA8));
}

void GpuTexture::bind(uint32_t unit) {
    if (device_ && texture_id_ != 0) {
        device_->bind_texture(unit, texture_id_);
    }
}

void GpuTexture::update(uint32_t x, uint32_t y, uint32_t width, uint32_t height, const void* pixels) {
    if (device_ && texture_id_ != 0 && pixels) {
        device_->bind_texture(0, texture_id_);
        device_->update_texture_sub_image_2d(x, y, width, height, pixels);
    }
}

} // namespace nisaba::gpu

#include "nisaba/compositor/gbm_allocator.hpp"
#include "nisaba/compositor/log.hpp"

namespace nisaba::compositor {

std::unique_ptr<GbmAllocator> GbmAllocator::create(std::shared_ptr<GbmDevice> device) {
    if (!device || !device->raw()) {
        logError("GbmAllocator::create: Invalid GbmDevice");
        return nullptr;
    }
    return std::make_unique<GbmAllocator>(std::move(device));
}

std::unique_ptr<GbmAllocator> GbmAllocator::createAuto() {
    auto device = GbmDevice::createAuto();
    if (!device) return nullptr;
    return create(std::move(device));
}

GbmAllocator::GbmAllocator(std::shared_ptr<GbmDevice> device)
    : m_device(std::move(device)) {}

GbmAllocator::~GbmAllocator() = default;

std::string_view GbmAllocator::name() const noexcept {
    return "Nisaba-Sovereign-GBM-Allocator";
}

std::shared_ptr<IBuffer> GbmAllocator::allocate(
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint64_t modifier) {
    return allocateBuffer(width, height, format, modifier);
}

std::shared_ptr<GbmBuffer> GbmAllocator::allocateBuffer(
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint64_t modifier) {
    if (!m_device || !m_device->raw() || width == 0 || height == 0) {
        return nullptr;
    }

    struct gbm_device* gbm = m_device->raw();
    struct gbm_bo* bo = nullptr;

    if (modifier != DRM_FORMAT_MOD_INVALID) {
        bo = gbm_bo_create_with_modifiers(gbm, width, height, format, &modifier, 1);
    }

    if (!bo) {
        uint32_t usage = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
        if (modifier == DRM_FORMAT_MOD_LINEAR) {
            usage |= GBM_BO_USE_LINEAR;
        }
        bo = gbm_bo_create(gbm, width, height, format, usage);
    }

    if (!bo) {
        logError("GbmAllocator: Failed to allocate GBM buffer {}x{} (format 0x{:08x})",
            width, height, format);
        return nullptr;
    }

    return GbmBuffer::create(m_device, bo);
}

std::shared_ptr<GbmBuffer> GbmAllocator::allocateWithModifiers(
    uint32_t width,
    uint32_t height,
    uint32_t format,
    std::span<const uint64_t> modifiers) {
    if (!m_device || !m_device->raw() || width == 0 || height == 0) {
        return nullptr;
    }

    struct gbm_device* gbm = m_device->raw();
    struct gbm_bo* bo = nullptr;

    if (!modifiers.empty()) {
        bo = gbm_bo_create_with_modifiers(
            gbm,
            width,
            height,
            format,
            modifiers.data(),
            static_cast<unsigned int>(modifiers.size()));
    }

    if (!bo) {
        uint32_t usage = GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING;
        bo = gbm_bo_create(gbm, width, height, format, usage);
    }

    if (!bo) {
        logError("GbmAllocator: Failed to allocate GBM buffer with modifiers {}x{}", width, height);
        return nullptr;
    }

    return GbmBuffer::create(m_device, bo);
}

} // namespace nisaba::compositor

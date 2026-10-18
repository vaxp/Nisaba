#include "nisaba/compositor/swapchain.hpp"
#include "nisaba/compositor/log.hpp"

#include <algorithm>

namespace nisaba::compositor {

std::shared_ptr<Swapchain> Swapchain::create(
    std::shared_ptr<GbmAllocator> allocator,
    std::shared_ptr<EglContext> egl,
    int width,
    int height,
    uint32_t format,
    size_t capacity) {
    if (!allocator || !egl || width <= 0 || height <= 0 || capacity == 0) {
        logError("Swapchain::create invalid parameters (width={}, height={}, cap={})", width, height, capacity);
        return nullptr;
    }

    return std::make_shared<Swapchain>(
        std::move(allocator),
        std::move(egl),
        width,
        height,
        format,
        capacity
    );
}

Swapchain::Swapchain(
    std::shared_ptr<GbmAllocator> allocator,
    std::shared_ptr<EglContext> egl,
    int width,
    int height,
    uint32_t format,
    size_t capacity)
    : m_allocator(std::move(allocator)),
      m_egl(std::move(egl)),
      m_width(width),
      m_height(height),
      m_format(format),
      m_capacity(capacity) {
    m_slots.resize(m_capacity);
}

Swapchain::~Swapchain() {
    m_slots.clear();
}

std::shared_ptr<RenderTarget> Swapchain::acquire() {
    Slot* freeSlot = nullptr;

    // 1. Look for already allocated but unacquired slot
    for (auto& slot : m_slots) {
        if (!slot.acquired) {
            if (slot.target) {
                slot.acquired = true;
                return slot.target;
            }
            if (!freeSlot) {
                freeSlot = &slot;
            }
        }
    }

    if (!freeSlot) {
        logWarn("Swapchain exhausted: all {} slots are currently acquired", m_capacity);
        return nullptr;
    }

    // 2. Allocate new hardware buffer for slot
    auto buffer = m_allocator->allocateBuffer(m_width, m_height, m_format, DRM_FORMAT_MOD_INVALID);
    if (!buffer) {
        buffer = m_allocator->allocateBuffer(m_width, m_height, m_format, DRM_FORMAT_MOD_LINEAR);
    }

    if (!buffer) {
        logError("Swapchain failed to allocate hardware GBM buffer {}x{}", m_width, m_height);
        return nullptr;
    }

    auto target = RenderTarget::fromGbmBuffer(m_egl, buffer);
    if (!target) {
        logError("Swapchain failed to create RenderTarget from GBM buffer");
        return nullptr;
    }

    freeSlot->buffer = buffer;
    freeSlot->target = target;
    freeSlot->acquired = true;

    return target;
}

void Swapchain::release(const RenderTarget* target) noexcept {
    if (!target) return;

    for (auto& slot : m_slots) {
        if (slot.target.get() == target) {
            slot.acquired = false;
            return;
        }
    }
}

void Swapchain::resize(int width, int height) {
    if (m_width == width && m_height == height) return;

    m_width = width;
    m_height = height;

    m_slots.clear();
    m_slots.resize(m_capacity);
}

} // namespace nisaba::compositor

#pragma once

#include "nisaba/compositor/egl_context.hpp"
#include "nisaba/compositor/gbm_allocator.hpp"
#include "nisaba/compositor/render_target.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace nisaba::compositor {

/**
 * @brief Sovereign hardware output swapchain managing multi-buffer DRM KMS scanout pools.
 */
class Swapchain {
public:
    static constexpr size_t kDefaultCapacity = 3;

    static std::shared_ptr<Swapchain> create(
        std::shared_ptr<GbmAllocator> allocator,
        std::shared_ptr<EglContext> egl,
        int width,
        int height,
        uint32_t format = DRM_FORMAT_XRGB8888,
        size_t capacity = kDefaultCapacity);

    Swapchain(
        std::shared_ptr<GbmAllocator> allocator,
        std::shared_ptr<EglContext> egl,
        int width,
        int height,
        uint32_t format,
        size_t capacity);

    ~Swapchain();

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    /**
     * @brief Acquires the next available scanout render target for drawing.
     */
    [[nodiscard]] std::shared_ptr<RenderTarget> acquire();

    /**
     * @brief Marks a previously acquired render target as released/presented.
     */
    void release(const RenderTarget* target) noexcept;

    /**
     * @brief Resizes swapchain, invalidating old buffers when output mode changes.
     */
    void resize(int width, int height);

    [[nodiscard]] int width() const noexcept { return m_width; }
    [[nodiscard]] int height() const noexcept { return m_height; }
    [[nodiscard]] uint32_t format() const noexcept { return m_format; }
    [[nodiscard]] size_t capacity() const noexcept { return m_capacity; }

private:
    struct Slot {
        std::shared_ptr<GbmBuffer> buffer;
        std::shared_ptr<RenderTarget> target;
        bool acquired{false};
    };

    std::shared_ptr<GbmAllocator> m_allocator;
    std::shared_ptr<EglContext> m_egl;
    int m_width{0};
    int m_height{0};
    uint32_t m_format{DRM_FORMAT_XRGB8888};
    size_t m_capacity{kDefaultCapacity};
    std::vector<Slot> m_slots;
};

} // namespace nisaba::compositor

#pragma once

#include "nisaba/compositor/gbm_buffer.hpp"
#include "nisaba/compositor/gbm_device.hpp"
#include "nisaba/compositor/renderer_interface.hpp"

#include <span>
#include <vector>

namespace nisaba::compositor {

/**
 * @brief Sovereign hardware buffer memory allocator using Linux GBM subsystem.
 */
class GbmAllocator : public IAllocator {
public:
    static std::unique_ptr<GbmAllocator> create(std::shared_ptr<GbmDevice> device);
    static std::unique_ptr<GbmAllocator> createAuto();

    explicit GbmAllocator(std::shared_ptr<GbmDevice> device);
    ~GbmAllocator() override;

    GbmAllocator(const GbmAllocator&) = delete;
    GbmAllocator& operator=(const GbmAllocator&) = delete;

    [[nodiscard]] std::string_view name() const noexcept override;

    [[nodiscard]] std::shared_ptr<IBuffer> allocate(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        uint64_t modifier) override;

    [[nodiscard]] std::shared_ptr<GbmBuffer> allocateBuffer(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        uint64_t modifier);

    [[nodiscard]] std::shared_ptr<GbmBuffer> allocateWithModifiers(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        std::span<const uint64_t> modifiers);

    [[nodiscard]] const std::shared_ptr<GbmDevice>& device() const noexcept { return m_device; }

private:
    std::shared_ptr<GbmDevice> m_device;
};

} // namespace nisaba::compositor

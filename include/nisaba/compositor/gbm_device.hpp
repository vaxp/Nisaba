#pragma once

#include "nisaba/compositor/fd.hpp"

#include <gbm.h>
#include <memory>
#include <string_view>

namespace nisaba::compositor {

/**
 * @brief Manages the hardware GBM (Generic Buffer Management) device context.
 */
class GbmDevice {
public:
    /**
     * @brief Creates a GBM device wrapping an existing open DRM file descriptor.
     */
    static std::shared_ptr<GbmDevice> create(int drmFd);

    /**
     * @brief Automatically discovers and opens the primary DRM render node or card.
     */
    static std::shared_ptr<GbmDevice> createAuto();

    GbmDevice(UniqueFd drmFd, struct gbm_device* gbmDevice);
    ~GbmDevice();

    GbmDevice(const GbmDevice&) = delete;
    GbmDevice& operator=(const GbmDevice&) = delete;

    [[nodiscard]] int fd() const noexcept { return m_fd.get(); }
    [[nodiscard]] struct gbm_device* raw() const noexcept { return m_device; }

    /**
     * @brief Checks if a pixel format is supported for specific usage flags.
     */
    [[nodiscard]] bool isFormatSupported(uint32_t format, uint32_t usageFlags) const noexcept;

private:
    UniqueFd m_fd;
    struct gbm_device* m_device{nullptr};
};

} // namespace nisaba::compositor

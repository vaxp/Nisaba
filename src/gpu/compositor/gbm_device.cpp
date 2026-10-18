#include "nisaba/compositor/gbm_device.hpp"
#include "nisaba/compositor/log.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <array>

namespace nisaba::compositor {

std::shared_ptr<GbmDevice> GbmDevice::create(int drmFd) {
    if (drmFd < 0) {
        logError("GbmDevice::create: Invalid DRM file descriptor");
        return nullptr;
    }

    int dupFd = fcntl(drmFd, F_DUPFD_CLOEXEC, 0);
    if (dupFd < 0) {
        logError("GbmDevice::create: Failed to duplicate DRM fd: {}", strerror(errno));
        return nullptr;
    }

    struct gbm_device* device = gbm_create_device(dupFd);
    if (!device) {
        logError("GbmDevice::create: Failed to create gbm_device from fd {}", dupFd);
        close(dupFd);
        return nullptr;
    }

    logInfo("GBM device initialized successfully on DRM fd {}", dupFd);
    return std::make_shared<GbmDevice>(UniqueFd(dupFd), device);
}

std::shared_ptr<GbmDevice> GbmDevice::createAuto() {
    constexpr std::array<const char*, 4> candidatePaths = {
        "/dev/dri/renderD128",
        "/dev/dri/renderD129",
        "/dev/dri/card0",
        "/dev/dri/card1",
    };

    for (const char* path : candidatePaths) {
        int fd = open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) continue;

        struct gbm_device* device = gbm_create_device(fd);
        if (!device) {
            close(fd);
            continue;
        }

        logInfo("Auto-discovered GBM hardware device on '{}' (fd {})", path, fd);
        return std::make_shared<GbmDevice>(UniqueFd(fd), device);
    }

    logError("GbmDevice::createAuto: Failed to open any DRM/GBM device");
    return nullptr;
}

GbmDevice::GbmDevice(UniqueFd drmFd, struct gbm_device* gbmDevice)
    : m_fd(std::move(drmFd)), m_device(gbmDevice) {}

GbmDevice::~GbmDevice() {
    if (m_device) {
        gbm_device_destroy(m_device);
        m_device = nullptr;
    }
}

bool GbmDevice::isFormatSupported(uint32_t format, uint32_t usageFlags) const noexcept {
    if (!m_device) return false;
    return gbm_device_is_format_supported(m_device, format, usageFlags) != 0;
}

} // namespace nisaba::compositor

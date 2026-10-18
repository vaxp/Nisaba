#pragma once

#include "nisaba/compositor/drm_formats.hpp"
#include "nisaba/compositor/fd.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace nisaba::compositor {

/**
 * @brief Representation of a Linux DMA-BUF memory plane.
 */
struct DmaBufPlane {
    UniqueFd fd;
    uint32_t stride{0};
    uint32_t offset{0};
};

/**
 * @brief Zero-copy DMA-BUF memory handle shared across GPU, DRM, and Wayland clients.
 */
struct DmaBufAttributes {
    uint32_t width{0};
    uint32_t height{0};
    uint32_t format{DRM_FORMAT_INVALID};
    uint64_t modifier{DRM_FORMAT_MOD_INVALID};
    std::vector<DmaBufPlane> planes;
};

/**
 * @brief Abstract representation of a graphic buffer (GPU Texture, Framebuffer, SHM).
 */
class IBuffer {
public:
    virtual ~IBuffer() = default;

    [[nodiscard]] virtual uint32_t width() const noexcept = 0;
    [[nodiscard]] virtual uint32_t height() const noexcept = 0;
    [[nodiscard]] virtual uint32_t format() const noexcept = 0;
    [[nodiscard]] virtual uint64_t modifier() const noexcept = 0;

    /**
     * @brief Exports buffer as a zero-copy DMA-BUF attributes handle if supported.
     */
    [[nodiscard]] virtual std::optional<DmaBufAttributes> exportDmaBuf() const = 0;
};

} // namespace nisaba::compositor

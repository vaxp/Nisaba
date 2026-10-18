#pragma once

#include "nisaba/compositor/buffer.hpp"
#include <gbm.h>
#include <memory>

namespace nisaba::compositor {

class GbmDevice;

/**
 * @brief Hardware graphics buffer allocated via Linux GBM.
 */
class GbmBuffer : public IBuffer {
public:
    static std::shared_ptr<GbmBuffer> create(
        std::shared_ptr<GbmDevice> device,
        struct gbm_bo* bo);

    GbmBuffer(std::shared_ptr<GbmDevice> device, struct gbm_bo* bo);
    ~GbmBuffer() override;

    GbmBuffer(const GbmBuffer&) = delete;
    GbmBuffer& operator=(const GbmBuffer&) = delete;

    [[nodiscard]] uint32_t width() const noexcept override { return m_width; }
    [[nodiscard]] uint32_t height() const noexcept override { return m_height; }
    [[nodiscard]] uint32_t format() const noexcept override { return m_format; }
    [[nodiscard]] uint64_t modifier() const noexcept override { return m_modifier; }
    [[nodiscard]] uint32_t stride() const noexcept { return m_stride; }
    [[nodiscard]] uint32_t planeCount() const noexcept { return m_planeCount; }

    [[nodiscard]] struct gbm_bo* raw() const noexcept { return m_bo; }
    [[nodiscard]] const std::shared_ptr<GbmDevice>& device() const noexcept { return m_device; }

    [[nodiscard]] std::optional<DmaBufAttributes> exportDmaBuf() const override;

private:
    std::shared_ptr<GbmDevice> m_device;
    struct gbm_bo* m_bo{nullptr};

    uint32_t m_width{0};
    uint32_t m_height{0};
    uint32_t m_format{0};
    uint64_t m_modifier{DRM_FORMAT_MOD_INVALID};
    uint32_t m_stride{0};
    uint32_t m_planeCount{1};
};

} // namespace nisaba::compositor

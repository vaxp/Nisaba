#include "nisaba/compositor/gbm_buffer.hpp"
#include "nisaba/compositor/gbm_device.hpp"
#include "nisaba/compositor/log.hpp"

#include <unistd.h>

namespace nisaba::compositor {

std::shared_ptr<GbmBuffer> GbmBuffer::create(
    std::shared_ptr<GbmDevice> device,
    struct gbm_bo* bo) {
    if (!device || !bo) return nullptr;
    return std::make_shared<GbmBuffer>(std::move(device), bo);
}

GbmBuffer::GbmBuffer(std::shared_ptr<GbmDevice> device, struct gbm_bo* bo)
    : m_device(std::move(device)), m_bo(bo) {
    if (m_bo) {
        m_width = gbm_bo_get_width(m_bo);
        m_height = gbm_bo_get_height(m_bo);
        m_format = gbm_bo_get_format(m_bo);
        m_modifier = gbm_bo_get_modifier(m_bo);
        m_stride = gbm_bo_get_stride(m_bo);
        int planes = gbm_bo_get_plane_count(m_bo);
        m_planeCount = (planes > 0) ? static_cast<uint32_t>(planes) : 1;
    }
}

GbmBuffer::~GbmBuffer() {
    if (m_bo) {
        gbm_bo_destroy(m_bo);
        m_bo = nullptr;
    }
}

std::optional<DmaBufAttributes> GbmBuffer::exportDmaBuf() const {
    if (!m_bo) return std::nullopt;

    DmaBufAttributes attrs;
    attrs.width = m_width;
    attrs.height = m_height;
    attrs.format = m_format;
    attrs.modifier = m_modifier;

    uint32_t count = m_planeCount;
    if (count == 0 || count > 4) return std::nullopt;

    attrs.planes.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        int fd = gbm_bo_get_fd_for_plane(m_bo, static_cast<int>(i));
        if (fd < 0) {
            logError("GbmBuffer::exportDmaBuf: Failed to get fd for plane {}", i);
            return std::nullopt;
        }

        uint32_t stride = gbm_bo_get_stride_for_plane(m_bo, static_cast<int>(i));
        uint32_t offset = gbm_bo_get_offset(m_bo, static_cast<int>(i));

        attrs.planes.push_back(DmaBufPlane{
            .fd = UniqueFd(fd),
            .stride = stride,
            .offset = offset,
        });
    }

    return attrs;
}

} // namespace nisaba::compositor

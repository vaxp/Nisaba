#pragma once

#include "nisaba/compositor/texture.hpp"

namespace nisaba::compositor {

/**
 * @brief GPU texture populated from CPU shared memory (wl_shm).
 */
class ShmTexture : public Texture {
public:
    static std::shared_ptr<ShmTexture> create(
        uint32_t width,
        uint32_t height,
        uint32_t format,
        bool hasAlpha,
        const void* initialPixels = nullptr,
        uint32_t stride = 0);

    ShmTexture(
        GLuint id,
        uint32_t width,
        uint32_t height,
        uint32_t format,
        bool hasAlpha);

    ~ShmTexture() override = default;

    /**
     * @brief Uploads damaged sub-rectangle from client memory into GPU texture.
     */
    bool updateDamage(const RectI& damage, const void* pixels, uint32_t stride);
};

} // namespace nisaba::compositor

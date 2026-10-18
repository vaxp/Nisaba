#pragma once

#include "nisaba/compositor/egl_image.hpp"
#include "nisaba/compositor/texture.hpp"

namespace nisaba::compositor {

/**
 * @brief Zero-copy GPU texture binding an imported hardware DMA-BUF.
 */
class DmaBufTexture : public Texture {
public:
    static std::shared_ptr<DmaBufTexture> create(
        std::shared_ptr<EglImage> image,
        uint32_t width,
        uint32_t height,
        uint32_t format,
        bool hasAlpha,
        GLenum target = GL_TEXTURE_2D);

    DmaBufTexture(
        std::shared_ptr<EglImage> image,
        GLuint id,
        GLenum target,
        uint32_t width,
        uint32_t height,
        uint32_t format,
        bool hasAlpha);

    ~DmaBufTexture() override = default;

    [[nodiscard]] const std::shared_ptr<EglImage>& image() const noexcept { return m_image; }

private:
    std::shared_ptr<EglImage> m_image;
};

} // namespace nisaba::compositor

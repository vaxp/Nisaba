#pragma once

#include "nisaba/compositor/buffer.hpp"
#include "nisaba/compositor/egl_context.hpp"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <memory>

namespace nisaba::compositor {

/**
 * @brief Zero-copy hardware EGLImage wrapper imported from Linux DMA-BUF.
 */
class EglImage {
public:
    static std::shared_ptr<EglImage> createFromDmaBuf(
        std::shared_ptr<EglContext> context,
        const DmaBufAttributes& attrs);

    EglImage(std::shared_ptr<EglContext> context, EGLImageKHR image);
    ~EglImage();

    EglImage(const EglImage&) = delete;
    EglImage& operator=(const EglImage&) = delete;

    [[nodiscard]] EGLImageKHR raw() const noexcept { return m_image; }
    [[nodiscard]] const std::shared_ptr<EglContext>& context() const noexcept { return m_context; }

private:
    std::shared_ptr<EglContext> m_context;
    EGLImageKHR m_image{EGL_NO_IMAGE_KHR};
};

} // namespace nisaba::compositor

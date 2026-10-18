#include "nisaba/compositor/egl_image.hpp"
#include "nisaba/compositor/log.hpp"

#include <vector>

namespace nisaba::compositor {

namespace {

typedef EGLImageKHR (*PFNEGLCREATEIMAGEKHRPROC)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (*PFNEGLDESTROYIMAGEKHRPROC)(EGLDisplay dpy, EGLImageKHR image);

} // namespace

std::shared_ptr<EglImage> EglImage::createFromDmaBuf(
    std::shared_ptr<EglContext> context,
    const DmaBufAttributes& attrs) {
    if (!context || context->display() == EGL_NO_DISPLAY || attrs.planes.empty()) {
        logError("EglImage::createFromDmaBuf: Invalid context or empty planes");
        return nullptr;
    }

    auto createImage = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(
        context->getProcAddress("eglCreateImageKHR"));
    if (!createImage) {
        logError("EglImage::createFromDmaBuf: eglCreateImageKHR not available");
        return nullptr;
    }

    bool useModifiers = (attrs.modifier != DRM_FORMAT_MOD_INVALID) &&
                        context->hasExtension("EGL_EXT_image_dma_buf_import_modifiers");

    auto buildAttribs = [&](bool includeModifiers) {
        std::vector<EGLint> attribs;
        attribs.reserve(64);

        attribs.push_back(EGL_WIDTH);
        attribs.push_back(static_cast<EGLint>(attrs.width));
        attribs.push_back(EGL_HEIGHT);
        attribs.push_back(static_cast<EGLint>(attrs.height));
        attribs.push_back(EGL_LINUX_DRM_FOURCC_EXT);
        attribs.push_back(static_cast<EGLint>(attrs.format));

        // Plane 0
        attribs.push_back(EGL_DMA_BUF_PLANE0_FD_EXT);
        attribs.push_back(attrs.planes[0].fd.get());
        attribs.push_back(EGL_DMA_BUF_PLANE0_OFFSET_EXT);
        attribs.push_back(static_cast<EGLint>(attrs.planes[0].offset));
        attribs.push_back(EGL_DMA_BUF_PLANE0_PITCH_EXT);
        attribs.push_back(static_cast<EGLint>(attrs.planes[0].stride));
        if (includeModifiers) {
            attribs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.modifier & 0xFFFFFFFF));
            attribs.push_back(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.modifier >> 32));
        }

        // Plane 1
        if (attrs.planes.size() > 1) {
            attribs.push_back(EGL_DMA_BUF_PLANE1_FD_EXT);
            attribs.push_back(attrs.planes[1].fd.get());
            attribs.push_back(EGL_DMA_BUF_PLANE1_OFFSET_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.planes[1].offset));
            attribs.push_back(EGL_DMA_BUF_PLANE1_PITCH_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.planes[1].stride));
            if (includeModifiers) {
                attribs.push_back(EGL_DMA_BUF_PLANE1_MODIFIER_LO_EXT);
                attribs.push_back(static_cast<EGLint>(attrs.modifier & 0xFFFFFFFF));
                attribs.push_back(EGL_DMA_BUF_PLANE1_MODIFIER_HI_EXT);
                attribs.push_back(static_cast<EGLint>(attrs.modifier >> 32));
            }
        }

        // Plane 2
        if (attrs.planes.size() > 2) {
            attribs.push_back(EGL_DMA_BUF_PLANE2_FD_EXT);
            attribs.push_back(attrs.planes[2].fd.get());
            attribs.push_back(EGL_DMA_BUF_PLANE2_OFFSET_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.planes[2].offset));
            attribs.push_back(EGL_DMA_BUF_PLANE2_PITCH_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.planes[2].stride));
            if (includeModifiers) {
                attribs.push_back(EGL_DMA_BUF_PLANE2_MODIFIER_LO_EXT);
                attribs.push_back(static_cast<EGLint>(attrs.modifier & 0xFFFFFFFF));
                attribs.push_back(EGL_DMA_BUF_PLANE2_MODIFIER_HI_EXT);
                attribs.push_back(static_cast<EGLint>(attrs.modifier >> 32));
            }
        }

        // Plane 3
        if (attrs.planes.size() > 3) {
            attribs.push_back(EGL_DMA_BUF_PLANE3_FD_EXT);
            attribs.push_back(attrs.planes[3].fd.get());
            attribs.push_back(EGL_DMA_BUF_PLANE3_OFFSET_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.planes[3].offset));
            attribs.push_back(EGL_DMA_BUF_PLANE3_PITCH_EXT);
            attribs.push_back(static_cast<EGLint>(attrs.planes[3].stride));
            if (includeModifiers) {
                attribs.push_back(EGL_DMA_BUF_PLANE3_MODIFIER_LO_EXT);
                attribs.push_back(static_cast<EGLint>(attrs.modifier & 0xFFFFFFFF));
                attribs.push_back(EGL_DMA_BUF_PLANE3_MODIFIER_HI_EXT);
                attribs.push_back(static_cast<EGLint>(attrs.modifier >> 32));
            }
        }

        attribs.push_back(EGL_NONE);
        return attribs;
    };

    auto attribs = buildAttribs(useModifiers);
    EGLImageKHR image = createImage(
        context->display(),
        EGL_NO_CONTEXT,
        EGL_LINUX_DMA_BUF_EXT,
        nullptr,
        attribs.data());

    if (image == EGL_NO_IMAGE_KHR && useModifiers) {
        auto fallbackAttribs = buildAttribs(false);
        image = createImage(
            context->display(),
            EGL_NO_CONTEXT,
            EGL_LINUX_DMA_BUF_EXT,
            nullptr,
            fallbackAttribs.data());
    }

    if (image == EGL_NO_IMAGE_KHR) {
        logError("EglImage::createFromDmaBuf: Failed to create EGLImage (error 0x{:04x})", eglGetError());
        return nullptr;
    }

    return std::make_shared<EglImage>(std::move(context), image);
}

EglImage::EglImage(std::shared_ptr<EglContext> context, EGLImageKHR image)
    : m_context(std::move(context)), m_image(image) {}

EglImage::~EglImage() {
    if (m_image != EGL_NO_IMAGE_KHR && m_context && m_context->display() != EGL_NO_DISPLAY) {
        auto destroyImage = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(
            m_context->getProcAddress("eglDestroyImageKHR"));
        if (destroyImage) {
            destroyImage(m_context->display(), m_image);
        }
        m_image = EGL_NO_IMAGE_KHR;
    }
}

} // namespace nisaba::compositor

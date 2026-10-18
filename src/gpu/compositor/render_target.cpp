#include "nisaba/compositor/render_target.hpp"
#include "nisaba/compositor/log.hpp"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace nisaba::compositor {

std::shared_ptr<RenderTarget> RenderTarget::createDefault(int width, int height) {
    return std::make_shared<RenderTarget>(width, height, 0, 0, false);
}

std::shared_ptr<RenderTarget> RenderTarget::fromTexture(std::shared_ptr<Texture> texture) {
    if (!texture || !texture->id()) {
        logError("Cannot create RenderTarget from null or invalid texture");
        return nullptr;
    }

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    if (!fbo) {
        logError("Failed to generate OpenGL framebuffer");
        return nullptr;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id(), 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        logError("Texture RenderTarget FBO incomplete: status 0x{:x}", status);
        glDeleteFramebuffers(1, &fbo);
        return nullptr;
    }

    return std::make_shared<RenderTarget>(texture->width(), texture->height(), fbo, 0, true);
}

std::shared_ptr<RenderTarget> RenderTarget::fromGbmBuffer(
    std::shared_ptr<EglContext> context,
    std::shared_ptr<GbmBuffer> buffer) {
    if (!context || !buffer) {
        logError("Cannot create RenderTarget with null EglContext or GbmBuffer");
        return nullptr;
    }

    auto dmaAttrs = buffer->exportDmaBuf();
    if (!dmaAttrs) {
        logError("Failed to export DMA-BUF attributes from GbmBuffer");
        return nullptr;
    }

    auto image = EglImage::createFromDmaBuf(context, *dmaAttrs);
    if (!image) {
        logError("Failed to import DMA-BUF into EGLImage for RenderTarget");
        return nullptr;
    }

    typedef void (*PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)(GLenum target, GLeglImageOES image);
    auto glEGLImageTargetRenderbufferStorageOES =
        reinterpret_cast<PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC>(
            context->getProcAddress("glEGLImageTargetRenderbufferStorageOES"));

    if (!glEGLImageTargetRenderbufferStorageOES) {
        logError("Extension glEGLImageTargetRenderbufferStorageOES is unavailable");
        return nullptr;
    }

    GLuint rbo = 0;
    glGenRenderbuffers(1, &rbo);
    if (!rbo) {
        logError("Failed to generate renderbuffer for hardware scanout target");
        return nullptr;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glEGLImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, image->raw());
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    if (!fbo) {
        logError("Failed to generate framebuffer for hardware scanout target");
        glDeleteRenderbuffers(1, &rbo);
        return nullptr;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        logError("Hardware RenderTarget FBO incomplete: status 0x{:x}", status);
        glDeleteFramebuffers(1, &fbo);
        glDeleteRenderbuffers(1, &rbo);
        return nullptr;
    }

    return std::make_shared<RenderTarget>(
        buffer->width(),
        buffer->height(),
        fbo,
        rbo,
        true,
        image,
        buffer
    );
}

RenderTarget::RenderTarget(
    int width,
    int height,
    GLuint fbo,
    GLuint rbo,
    bool ownsResources,
    std::shared_ptr<EglImage> image,
    std::shared_ptr<GbmBuffer> buffer)
    : m_width(width),
      m_height(height),
      m_fbo(fbo),
      m_rbo(rbo),
      m_ownsResources(ownsResources),
      m_image(std::move(image)),
      m_buffer(std::move(buffer)) {}

RenderTarget::~RenderTarget() {
    if (m_ownsResources) {
        if (m_fbo) {
            glDeleteFramebuffers(1, &m_fbo);
            m_fbo = 0;
        }
        if (m_rbo) {
            glDeleteRenderbuffers(1, &m_rbo);
            m_rbo = 0;
        }
    }
}

void RenderTarget::bind() const noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void RenderTarget::unbind() noexcept {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace nisaba::compositor

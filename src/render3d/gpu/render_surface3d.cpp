#include "nisaba/render3d/gpu/render_surface3d.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>

namespace nisaba::render3d {

RenderSurface3D::RenderSurface3D(uint32_t width, uint32_t height)
    : m_width(width), m_height(height) {
    init_surface();
}

RenderSurface3D::~RenderSurface3D() {
    cleanup();
}

void RenderSurface3D::init_surface() {
    cleanup();

    if (m_width == 0 || m_height == 0) return;

    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_color_texture);
    glGenRenderbuffers(1, &m_depth_stencil_rb);

    glBindTexture(GL_TEXTURE_2D, m_color_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        static_cast<GLsizei>(m_width),
        static_cast<GLsizei>(m_height),
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindRenderbuffer(GL_RENDERBUFFER, m_depth_stencil_rb);
    glRenderbufferStorage(
        GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
        static_cast<GLsizei>(m_width),
        static_cast<GLsizei>(m_height)
    );

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depth_stencil_rb);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[RenderSurface3D] Framebuffer incomplete! Status: " << status << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RenderSurface3D::cleanup() noexcept {
    if (m_depth_stencil_rb != 0) {
        glDeleteRenderbuffers(1, &m_depth_stencil_rb);
        m_depth_stencil_rb = 0;
    }
    if (m_color_texture != 0) {
        glDeleteTextures(1, &m_color_texture);
        m_color_texture = 0;
    }
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

void RenderSurface3D::resize(uint32_t width, uint32_t height) {
    if (m_width == width && m_height == height) return;
    m_width = width;
    m_height = height;
    init_surface();
}

void RenderSurface3D::render_3d(GpuBackend3D& backend, const View3D& view, bool clear_first) {
    backend.render_view(view, m_fbo, clear_first);
}

void RenderSurface3D::read_pixels(void* out_rgba) const {
    if (!out_rgba || m_fbo == 0) return;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glReadPixels(
        0, 0,
        static_cast<GLsizei>(m_width),
        static_cast<GLsizei>(m_height),
        GL_RGBA, GL_UNSIGNED_BYTE, out_rgba
    );
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace nisaba::render3d

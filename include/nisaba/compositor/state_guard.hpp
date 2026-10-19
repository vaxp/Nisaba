#pragma once

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>

namespace nisaba::compositor {

/**
 * @brief Sovereign RAII OpenGL State Guard.
 *
 * Automatically captures and restores critical OpenGL pipeline state when
 * transitioning between the low-level zero-copy compositor pass and the
 * high-level vector graphics canvas (GpuCanvas).
 */
class GlStateGuard {
public:
    GlStateGuard() {
        glGetIntegerv(GL_CURRENT_PROGRAM, &m_prevProgram);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_prevVbo);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_prevEbo);

        typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint array);
        static auto pfnBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(eglGetProcAddress("glBindVertexArray"));
        if (!pfnBindVertexArray) {
            pfnBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(eglGetProcAddress("glBindVertexArrayOES"));
        }

#ifndef GL_VERTEX_ARRAY_BINDING
#define GL_VERTEX_ARRAY_BINDING 0x85B5
#endif
        if (pfnBindVertexArray) {
            glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_prevVao);
        }

        glGetIntegerv(GL_BLEND_SRC_RGB, &m_prevBlendSrcRgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &m_prevBlendDstRgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_prevBlendSrcAlpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &m_prevBlendDstAlpha);
        m_prevBlend = glIsEnabled(GL_BLEND);

        m_prevScissor = glIsEnabled(GL_SCISSOR_TEST);
        glGetIntegerv(GL_SCISSOR_BOX, m_prevScissorBox);

        m_prevDepth = glIsEnabled(GL_DEPTH_TEST);
        m_prevStencil = glIsEnabled(GL_STENCIL_TEST);
        glGetIntegerv(GL_VIEWPORT, m_prevViewport);
        glGetIntegerv(GL_ACTIVE_TEXTURE, &m_prevActiveTexture);
        glGetBooleanv(GL_COLOR_WRITEMASK, m_prevColorMask);
    }

    ~GlStateGuard() {
        restore();
    }

    GlStateGuard(const GlStateGuard&) = delete;
    GlStateGuard& operator=(const GlStateGuard&) = delete;

    void restore() noexcept {
        if (m_restored) return;
        m_restored = true;

        glUseProgram(static_cast<GLuint>(m_prevProgram));

        typedef void (*PFNGLBINDVERTEXARRAYPROC)(GLuint array);
        static auto pfnBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(eglGetProcAddress("glBindVertexArray"));
        if (!pfnBindVertexArray) {
            pfnBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(eglGetProcAddress("glBindVertexArrayOES"));
        }
        if (pfnBindVertexArray) {
            pfnBindVertexArray(static_cast<GLuint>(m_prevVao));
        }

        glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(m_prevVbo));
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(m_prevEbo));

        if (m_prevBlend) {
            glEnable(GL_BLEND);
            glBlendFuncSeparate(m_prevBlendSrcRgb, m_prevBlendDstRgb,
                                m_prevBlendSrcAlpha, m_prevBlendDstAlpha);
        } else {
            glDisable(GL_BLEND);
        }

        if (m_prevScissor) {
            glEnable(GL_SCISSOR_TEST);
            glScissor(m_prevScissorBox[0], m_prevScissorBox[1],
                      m_prevScissorBox[2], m_prevScissorBox[3]);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }

        if (m_prevDepth) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (m_prevStencil) glEnable(GL_STENCIL_TEST); else glDisable(GL_STENCIL_TEST);

        glViewport(m_prevViewport[0], m_prevViewport[1],
                   m_prevViewport[2], m_prevViewport[3]);
        glActiveTexture(static_cast<GLenum>(m_prevActiveTexture));
        glColorMask(m_prevColorMask[0], m_prevColorMask[1],
                    m_prevColorMask[2], m_prevColorMask[3]);
    }

private:
    GLint m_prevProgram{0};
    GLint m_prevVao{0};
    GLint m_prevVbo{0};
    GLint m_prevEbo{0};
    GLint m_prevBlendSrcRgb{0};
    GLint m_prevBlendDstRgb{0};
    GLint m_prevBlendSrcAlpha{0};
    GLint m_prevBlendDstAlpha{0};
    GLboolean m_prevBlend{GL_FALSE};
    GLboolean m_prevScissor{GL_FALSE};
    GLint m_prevScissorBox[4]{0, 0, 0, 0};
    GLboolean m_prevDepth{GL_FALSE};
    GLboolean m_prevStencil{GL_FALSE};
    GLint m_prevViewport[4]{0, 0, 0, 0};
    GLint m_prevActiveTexture{GL_TEXTURE0};
    GLboolean m_prevColorMask[4]{GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE};
    bool m_restored{false};
};

} // namespace nisaba::compositor

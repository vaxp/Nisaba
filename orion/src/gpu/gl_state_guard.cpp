#include "orion/gpu/gl_state_guard.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

namespace orion {

GlStateGuard3D::GlStateGuard3D() {
    glGetIntegerv(GL_CURRENT_PROGRAM, &m_prev_program);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &m_prev_vao);
    glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &m_prev_vbo);
    glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &m_prev_ebo);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &m_prev_fbo);

    m_prev_depth_test = glIsEnabled(GL_DEPTH_TEST) ? 1 : 0;
    GLboolean dmask = GL_TRUE;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &dmask);
    m_prev_depth_mask = dmask ? 1 : 0;
    glGetIntegerv(GL_DEPTH_FUNC, &m_prev_depth_func);

    m_prev_cull_face = glIsEnabled(GL_CULL_FACE) ? 1 : 0;
    glGetIntegerv(GL_CULL_FACE_MODE, &m_prev_cull_mode);
    glGetIntegerv(GL_FRONT_FACE, &m_prev_front_face);

    m_prev_blend = glIsEnabled(GL_BLEND) ? 1 : 0;
    glGetIntegerv(GL_BLEND_SRC_RGB, &m_prev_blend_src_rgb);
    glGetIntegerv(GL_BLEND_DST_RGB, &m_prev_blend_dst_rgb);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &m_prev_blend_src_alpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &m_prev_blend_dst_alpha);
    glGetIntegerv(GL_BLEND_EQUATION_RGB, &m_prev_blend_eq_rgb);
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &m_prev_blend_eq_alpha);

    m_prev_scissor_test = glIsEnabled(GL_SCISSOR_TEST) ? 1 : 0;
    glGetIntegerv(GL_SCISSOR_BOX, m_prev_scissor_box);
    glGetIntegerv(GL_VIEWPORT, m_prev_viewport);

    glGetIntegerv(GL_ACTIVE_TEXTURE, &m_prev_active_texture);
}

GlStateGuard3D::~GlStateGuard3D() {
    restore();
}

void GlStateGuard3D::restore() noexcept {
    if (m_restored) return;
    m_restored = true;

    glUseProgram(static_cast<GLuint>(m_prev_program));
    glBindVertexArray(static_cast<GLuint>(m_prev_vao));
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<GLuint>(m_prev_vbo));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint>(m_prev_ebo));
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(m_prev_fbo));

    if (m_prev_depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthMask(m_prev_depth_mask ? GL_TRUE : GL_FALSE);
    glDepthFunc(static_cast<GLenum>(m_prev_depth_func));

    if (m_prev_cull_face) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glCullFace(static_cast<GLenum>(m_prev_cull_mode));
    glFrontFace(static_cast<GLenum>(m_prev_front_face));

    if (m_prev_blend) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    glBlendFuncSeparate(
        static_cast<GLenum>(m_prev_blend_src_rgb),
        static_cast<GLenum>(m_prev_blend_dst_rgb),
        static_cast<GLenum>(m_prev_blend_src_alpha),
        static_cast<GLenum>(m_prev_blend_dst_alpha)
    );
    glBlendEquationSeparate(
        static_cast<GLenum>(m_prev_blend_eq_rgb),
        static_cast<GLenum>(m_prev_blend_eq_alpha)
    );

    if (m_prev_scissor_test) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(m_prev_scissor_box[0], m_prev_scissor_box[1], m_prev_scissor_box[2], m_prev_scissor_box[3]);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }

    glViewport(m_prev_viewport[0], m_prev_viewport[1], m_prev_viewport[2], m_prev_viewport[3]);
    glActiveTexture(static_cast<GLenum>(m_prev_active_texture));
}

} // namespace orion

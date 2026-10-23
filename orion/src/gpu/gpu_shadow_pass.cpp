#include "orion/gpu/gpu_shadow_pass.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

namespace orion {

GpuShadowPass::GpuShadowPass(uint32_t resolution)
    : m_resolution(resolution) {
    init_framebuffer();
    m_depth_shader = GlShaderProgram::create_shadow_depth_shader();
}

GpuShadowPass::~GpuShadowPass() {
    cleanup();
}

void GpuShadowPass::init_framebuffer() {
    cleanup();

    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_depth_texture);

    glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
        static_cast<GLsizei>(m_resolution),
        static_cast<GLsizei>(m_resolution),
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    const float border_color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

    // No color buffer write
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GpuShadowPass::cleanup() noexcept {
    if (m_depth_texture != 0) {
        glDeleteTextures(1, &m_depth_texture);
        m_depth_texture = 0;
    }
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

void GpuShadowPass::begin(const Mat44& light_space_matrix) {
    m_light_space_matrix = light_space_matrix;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, static_cast<GLsizei>(m_resolution), static_cast<GLsizei>(m_resolution));
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Front-face culling & hardware slope bias eliminates shadow acne and Peter-Panning
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);

    if (m_depth_shader && m_depth_shader->is_valid()) {
        m_depth_shader->bind();
        m_depth_shader->set_uniform("u_lightSpaceMatrix", m_light_space_matrix);
    }
}

void GpuShadowPass::render_caster(
    GpuResourceCache& cache,
    const Mesh3D& mesh,
    const Mat44& world_transform
) {
    if (!m_depth_shader || !m_depth_shader->is_valid()) return;

    m_depth_shader->set_uniform("u_model", world_transform);

    auto handle = cache.get_or_upload_mesh(mesh);
    if (!handle.is_valid || handle.vao == 0) return;

    glBindVertexArray(handle.vao);
    if (handle.ebo != 0) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(handle.index_count), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(handle.index_count));
    }
    glBindVertexArray(0);
}

void GpuShadowPass::end() {
    if (m_depth_shader && m_depth_shader->is_valid()) {
        m_depth_shader->unbind();
    }

    glDisable(GL_POLYGON_OFFSET_FILL);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace orion

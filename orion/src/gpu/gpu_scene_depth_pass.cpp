#include "orion/gpu/gpu_scene_depth_pass.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

namespace orion {

GpuSceneDepthPass::GpuSceneDepthPass() {
    m_depth_shader = GlShaderProgram::create_shadow_depth_shader();
    if (m_depth_shader && m_depth_shader->is_valid()) {
        m_loc_view_proj = m_depth_shader->get_uniform_location("u_lightSpaceMatrix");
        m_loc_model = m_depth_shader->get_uniform_location("u_model");
    }
}

GpuSceneDepthPass::~GpuSceneDepthPass() {
    cleanup();
}

void GpuSceneDepthPass::cleanup() noexcept {
    if (m_depth_texture != 0) {
        glDeleteTextures(1, &m_depth_texture);
        m_depth_texture = 0;
    }
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_width = 0;
    m_height = 0;
}

void GpuSceneDepthPass::ensure_capacity(uint32_t width, uint32_t height) {
    if (m_width == width && m_height == height && m_fbo != 0) {
        return;
    }

    cleanup();

    m_width = std::max(width, 1u);
    m_height = std::max(height, 1u);

    glGenFramebuffers(1, &m_fbo);
    glGenTextures(1, &m_depth_texture);

    glBindTexture(GL_TEXTURE_2D, m_depth_texture);
    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
        static_cast<GLsizei>(m_width),
        static_cast<GLsizei>(m_height),
        0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth_texture, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GpuSceneDepthPass::begin(uint32_t width, uint32_t height, const Mat44& view_projection) {
    ensure_capacity(width, height);
    m_view_projection = view_projection;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    if (m_depth_shader && m_depth_shader->is_valid()) {
        m_depth_shader->bind();
        if (m_loc_view_proj >= 0) {
            glUniformMatrix4fv(m_loc_view_proj, 1, GL_TRUE, m_view_projection.m);
        } else {
            m_depth_shader->set_uniform("u_lightSpaceMatrix", m_view_projection);
        }
    }
}

void GpuSceneDepthPass::render_object(
    GpuResourceCache& cache,
    const Mesh3D& mesh,
    const Mat44& world_transform
) {
    if (!m_depth_shader || !m_depth_shader->is_valid()) return;

    if (m_loc_model >= 0) {
        glUniformMatrix4fv(m_loc_model, 1, GL_TRUE, world_transform.m);
    } else {
        m_depth_shader->set_uniform("u_model", world_transform);
    }

    auto handle = cache.get_or_upload_mesh(mesh);
    if (!handle.is_valid || handle.vao == 0) return;

    glBindVertexArray(handle.vao);
    if (handle.ebo != 0) {
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(handle.index_count), GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(handle.index_count));
    }
}

void GpuSceneDepthPass::end() {
    glBindVertexArray(0);
    if (m_depth_shader) {
        m_depth_shader->unbind();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

} // namespace orion

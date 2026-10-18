#include "nisaba/compositor/texture.hpp"

namespace nisaba::compositor {

Texture::Texture(GLuint id, GLenum target, uint32_t width, uint32_t height, uint32_t format, bool hasAlpha)
    : m_id(id), m_target(target), m_width(width), m_height(height), m_format(format), m_hasAlpha(hasAlpha) {}

Texture::~Texture() {
    if (m_id != 0) {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}

void Texture::bind(uint32_t textureUnit) const {
    if (m_id != 0) {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(m_target, m_id);
    }
}

void Texture::unbind(uint32_t textureUnit) const {
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    glBindTexture(m_target, 0);
}

} // namespace nisaba::compositor

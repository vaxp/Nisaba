#include "nisaba/compositor/solid_texture.hpp"
#include "nisaba/compositor/log.hpp"

#include <drm_fourcc.h>
#include <algorithm>

namespace nisaba::compositor {

std::shared_ptr<SolidColorTexture> SolidColorTexture::create(
    float r,
    float g,
    float b,
    float a) {
    GLuint texId = 0;
    glGenTextures(1, &texId);
    if (texId == 0) return nullptr;

    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    uint8_t pixel[4] = {
        static_cast<uint8_t>(std::clamp(r, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(g, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(b, 0.0f, 1.0f) * 255.0f),
        static_cast<uint8_t>(std::clamp(a, 0.0f, 1.0f) * 255.0f),
    };

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        1,
        1,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixel);

    glBindTexture(GL_TEXTURE_2D, 0);

    return std::make_shared<SolidColorTexture>(texId, r, g, b, a);
}

SolidColorTexture::SolidColorTexture(GLuint id, float r, float g, float b, float a)
    : Texture(id, GL_TEXTURE_2D, 1, 1, DRM_FORMAT_ARGB8888, a < 1.0f),
      m_r(r), m_g(g), m_b(b), m_a(a) {}

} // namespace nisaba::compositor

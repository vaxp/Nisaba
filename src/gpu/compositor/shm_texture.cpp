#include "nisaba/compositor/shm_texture.hpp"
#include "nisaba/compositor/log.hpp"

#include <drm_fourcc.h>
#include <vector>

namespace nisaba::compositor {

namespace {

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT 0x80E1
#endif

#ifndef GL_UNPACK_ROW_LENGTH_EXT
#define GL_UNPACK_ROW_LENGTH_EXT 0x0CF2
#define GL_UNPACK_SKIP_ROWS_EXT 0x0CF3
#define GL_UNPACK_SKIP_PIXELS_EXT 0x0CF4
#endif

GLenum determineGlFormat(uint32_t format) {
    if (format == 0 /* WL_SHM_FORMAT_ARGB8888 */ || format == DRM_FORMAT_ARGB8888 ||
        format == 1 /* WL_SHM_FORMAT_XRGB8888 */ || format == DRM_FORMAT_XRGB8888) {
        return GL_BGRA_EXT;
    }
    if (format == DRM_FORMAT_XBGR8888 || format == DRM_FORMAT_ABGR8888) {
        return GL_RGBA;
    }
    return GL_RGBA;
}

} // namespace

std::shared_ptr<ShmTexture> ShmTexture::create(
    uint32_t width,
    uint32_t height,
    uint32_t format,
    bool hasAlpha,
    const void* initialPixels,
    uint32_t /*stride*/) {
    if (width == 0 || height == 0) return nullptr;

    GLuint texId = 0;
    glGenTextures(1, &texId);
    if (texId == 0) {
        logError("ShmTexture::create: Failed to generate GL texture");
        return nullptr;
    }

    glBindTexture(GL_TEXTURE_2D, texId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum glFormat = determineGlFormat(format);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        glFormat,
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        0,
        glFormat,
        GL_UNSIGNED_BYTE,
        initialPixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    return std::make_shared<ShmTexture>(texId, width, height, format, hasAlpha);
}

ShmTexture::ShmTexture(
    GLuint id,
    uint32_t width,
    uint32_t height,
    uint32_t format,
    bool hasAlpha)
    : Texture(id, GL_TEXTURE_2D, width, height, format, hasAlpha) {}

bool ShmTexture::updateDamage(const RectI& damage, const void* pixels, uint32_t stride) {
    if (m_id == 0 || !pixels) return false;
    if (damage.width <= 0 || damage.height <= 0) return true;

    glBindTexture(GL_TEXTURE_2D, m_id);

    GLenum glFormat = determineGlFormat(m_format);

    if (stride > 0 && damage.width > 0) {
        glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, static_cast<GLint>(stride / 4));
        glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, static_cast<GLint>(damage.x));
        glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, static_cast<GLint>(damage.y));
    }

    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        damage.x,
        damage.y,
        damage.width,
        damage.height,
        glFormat,
        GL_UNSIGNED_BYTE,
        pixels);

    if (stride > 0 && damage.width > 0) {
        glPixelStorei(GL_UNPACK_ROW_LENGTH_EXT, 0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS_EXT, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS_EXT, 0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    return true;
}

} // namespace nisaba::compositor

#pragma once

#include "nisaba/compositor/geometry.hpp"
#include "nisaba/compositor/texture_interface.hpp"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cstdint>
#include <memory>

namespace nisaba::compositor {

/**
 * @brief Base GPU texture abstraction for compositor, implementing ITexture for OpenGL ES.
 */
class Texture : public ITexture {
public:
    Texture(GLuint id, GLenum target, uint32_t width, uint32_t height, uint32_t format, bool hasAlpha);
    ~Texture() override;

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    [[nodiscard]] GLuint id() const noexcept { return m_id; }
    [[nodiscard]] GLenum target() const noexcept { return m_target; }
    [[nodiscard]] uint32_t width() const noexcept override { return m_width; }
    [[nodiscard]] uint32_t height() const noexcept override { return m_height; }
    [[nodiscard]] uint32_t format() const noexcept override { return m_format; }
    [[nodiscard]] bool hasAlpha() const noexcept override { return m_hasAlpha; }
    [[nodiscard]] RenderBackendType backendType() const noexcept override { return RenderBackendType::OpenGL; }
    [[nodiscard]] void* nativeHandle() const noexcept override {
        return reinterpret_cast<void*>(static_cast<uintptr_t>(m_id));
    }

    void bind(uint32_t textureUnit = 0) const;
    void unbind(uint32_t textureUnit = 0) const;

protected:
    GLuint m_id{0};
    GLenum m_target{GL_TEXTURE_2D};
    uint32_t m_width{0};
    uint32_t m_height{0};
    uint32_t m_format{0};
    bool m_hasAlpha{false};
};

} // namespace nisaba::compositor

#pragma once

#include "nisaba/compositor/color.hpp"
#include "nisaba/compositor/matrix3.hpp"

#include <GLES2/gl2.h>
#include <memory>
#include <string>
#include <string_view>

namespace nisaba::compositor {

/**
 * @brief Sovereign OpenGL ES Shader object.
 */
class Shader {
public:
    static std::unique_ptr<Shader> compile(GLenum type, std::string_view source);

    Shader(GLenum type, GLuint id);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&& other) noexcept;
    Shader& operator=(Shader&& other) noexcept;

    [[nodiscard]] GLuint id() const noexcept { return m_id; }
    [[nodiscard]] GLenum type() const noexcept { return m_type; }

private:
    GLenum m_type{0};
    GLuint m_id{0};
};

/**
 * @brief Base class for linked OpenGL ES shader programs.
 */
class ShaderProgram {
public:
    static std::unique_ptr<ShaderProgram> create(std::string_view vertSource, std::string_view fragSource);

    ShaderProgram(GLuint programId);
    virtual ~ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
    ShaderProgram(ShaderProgram&& other) noexcept;
    ShaderProgram& operator=(ShaderProgram&& other) noexcept;

    void use() const noexcept;

    [[nodiscard]] GLuint id() const noexcept { return m_program; }
    [[nodiscard]] GLuint release() noexcept {
        GLuint prog = m_program;
        m_program = 0;
        return prog;
    }
    [[nodiscard]] GLint getUniformLocation(const char* name) const noexcept;
    [[nodiscard]] GLint getAttribLocation(const char* name) const noexcept;

protected:
    GLuint m_program{0};
};

/**
 * @brief Solid color box shader program.
 */
class SolidQuadProgram : public ShaderProgram {
public:
    static std::unique_ptr<SolidQuadProgram> create();
    SolidQuadProgram(GLuint programId);

    void setProjection(const Matrix3& proj) const noexcept;
    void setColor(const Color& color) const noexcept;

    [[nodiscard]] GLint posAttrib() const noexcept { return m_posAttrib; }

private:
    GLint m_projLoc{-1};
    GLint m_colorLoc{-1};
    GLint m_posAttrib{-1};
};

/**
 * @brief Texture quad shader program supporting 2D textures or external OES.
 */
class TextureQuadProgram : public ShaderProgram {
public:
    enum class Target {
        Rgba,
        Rgbx,
        ExternalOes,
        ExternalOesRgbx
    };

    static std::unique_ptr<TextureQuadProgram> create(Target target);
    TextureQuadProgram(GLuint programId, Target target);

    void setProjection(const Matrix3& proj) const noexcept;
    void setTextureProjection(const Matrix3& texProj) const noexcept;
    void setAlpha(float alpha) const noexcept;
    void setTextureUnit(int unit) const noexcept;

    [[nodiscard]] GLint posAttrib() const noexcept { return m_posAttrib; }
    [[nodiscard]] Target target() const noexcept { return m_target; }

private:
    Target m_target;
    GLint m_projLoc{-1};
    GLint m_texProjLoc{-1};
    GLint m_texLoc{-1};
    GLint m_alphaLoc{-1};
    GLint m_posAttrib{-1};
};

/**
 * @brief Manager that caches and provides access to compiled compositor shaders.
 */
class ShaderManager {
public:
    static std::shared_ptr<ShaderManager> create(bool supportExternalOes = true);

    ShaderManager(
        std::unique_ptr<SolidQuadProgram> solid,
        std::unique_ptr<TextureQuadProgram> texRgba,
        std::unique_ptr<TextureQuadProgram> texRgbx,
        std::unique_ptr<TextureQuadProgram> texExternal,
        std::unique_ptr<TextureQuadProgram> texExternalRgbx);

    ~ShaderManager() = default;

    [[nodiscard]] const SolidQuadProgram& solidQuad() const noexcept { return *m_solidQuad; }
    [[nodiscard]] const TextureQuadProgram& textureRgba() const noexcept { return *m_textureRgba; }
    [[nodiscard]] const TextureQuadProgram& textureRgbx() const noexcept { return *m_textureRgbx; }
    [[nodiscard]] const TextureQuadProgram* textureExternal() const noexcept { return m_textureExternal.get(); }
    [[nodiscard]] const TextureQuadProgram* textureExternalRgbx() const noexcept { return m_textureExternalRgbx.get(); }

private:
    std::unique_ptr<SolidQuadProgram> m_solidQuad;
    std::unique_ptr<TextureQuadProgram> m_textureRgba;
    std::unique_ptr<TextureQuadProgram> m_textureRgbx;
    std::unique_ptr<TextureQuadProgram> m_textureExternal;
    std::unique_ptr<TextureQuadProgram> m_textureExternalRgbx;
};

} // namespace nisaba::compositor

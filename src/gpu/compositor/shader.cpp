#include "nisaba/compositor/shader.hpp"
#include "nisaba/compositor/log.hpp"

#include <vector>

namespace nisaba::compositor {

namespace {

constexpr std::string_view kSolidQuadVert = R"glsl(
attribute vec2 pos;
uniform mat3 proj;

void main() {
    vec3 pos3 = vec3(pos, 1.0);
    gl_Position = vec4(pos3 * proj, 1.0);
}
)glsl";

constexpr std::string_view kSolidQuadFrag = R"glsl(
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

uniform vec4 color;

void main() {
    gl_FragColor = color;
}
)glsl";

constexpr std::string_view kTextureQuadVert = R"glsl(
attribute vec2 pos;
uniform mat3 proj;
uniform mat3 tex_proj;
varying vec2 v_texcoord;

void main() {
    vec3 pos3 = vec3(pos, 1.0);
    gl_Position = vec4(pos3 * proj, 1.0);
    v_texcoord = (pos3 * tex_proj).xy;
}
)glsl";

constexpr std::string_view kTextureQuadRgbaFrag = R"glsl(
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

varying vec2 v_texcoord;
uniform sampler2D tex;
uniform float alpha;

void main() {
    gl_FragColor = texture2D(tex, v_texcoord) * alpha;
}
)glsl";

constexpr std::string_view kTextureQuadRgbxFrag = R"glsl(
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

varying vec2 v_texcoord;
uniform sampler2D tex;
uniform float alpha;

void main() {
    gl_FragColor = vec4(texture2D(tex, v_texcoord).rgb, 1.0) * alpha;
}
)glsl";

constexpr std::string_view kTextureQuadExternalFrag = R"glsl(
#extension GL_OES_EGL_image_external : require

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

varying vec2 v_texcoord;
uniform samplerExternalOES tex;
uniform float alpha;

void main() {
    gl_FragColor = texture2D(tex, v_texcoord) * alpha;
}
)glsl";

constexpr std::string_view kTextureQuadExternalRgbxFrag = R"glsl(
#extension GL_OES_EGL_image_external : require

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

varying vec2 v_texcoord;
uniform samplerExternalOES tex;
uniform float alpha;

void main() {
    gl_FragColor = vec4(texture2D(tex, v_texcoord).rgb, 1.0) * alpha;
}
)glsl";

} // namespace

// ==========================================
// Shader Implementation
// ==========================================

std::unique_ptr<Shader> Shader::compile(GLenum type, std::string_view source) {
    GLuint id = glCreateShader(type);
    if (!id) {
        logError("Failed to create OpenGL ES shader of type 0x{:x}", type);
        return nullptr;
    }

    const char* srcPtr = source.data();
    GLint srcLen = static_cast<GLint>(source.size());
    glShaderSource(id, 1, &srcPtr, &srcLen);
    glCompileShader(id);

    GLint status = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint logLen = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(static_cast<size_t>(logLen > 1 ? logLen : 256));
        glGetShaderInfoLog(id, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
        logError("Shader compilation failed (type 0x{:x}): {}", type, infoLog.data());
        glDeleteShader(id);
        return nullptr;
    }

    return std::make_unique<Shader>(type, id);
}

Shader::Shader(GLenum type, GLuint id)
    : m_type(type), m_id(id) {}

Shader::~Shader() {
    if (m_id) {
        glDeleteShader(m_id);
    }
}

Shader::Shader(Shader&& other) noexcept
    : m_type(other.m_type), m_id(other.m_id) {
    other.m_id = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_id) glDeleteShader(m_id);
        m_type = other.m_type;
        m_id = other.m_id;
        other.m_id = 0;
    }
    return *this;
}

// ==========================================
// ShaderProgram Implementation
// ==========================================

std::unique_ptr<ShaderProgram> ShaderProgram::create(std::string_view vertSource, std::string_view fragSource) {
    auto vert = Shader::compile(GL_VERTEX_SHADER, vertSource);
    if (!vert) return nullptr;

    auto frag = Shader::compile(GL_FRAGMENT_SHADER, fragSource);
    if (!frag) return nullptr;

    GLuint prog = glCreateProgram();
    if (!prog) {
        logError("Failed to create OpenGL ES program");
        return nullptr;
    }

    glAttachShader(prog, vert->id());
    glAttachShader(prog, frag->id());
    glLinkProgram(prog);

    GLint status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(static_cast<size_t>(logLen > 1 ? logLen : 256));
        glGetProgramInfoLog(prog, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
        logError("Program linking failed: {}", infoLog.data());
        glDeleteProgram(prog);
        return nullptr;
    }

    return std::make_unique<ShaderProgram>(prog);
}

ShaderProgram::ShaderProgram(GLuint programId)
    : m_program(programId) {}

ShaderProgram::~ShaderProgram() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : m_program(other.m_program) {
    other.m_program = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
    if (this != &other) {
        if (m_program) glDeleteProgram(m_program);
        m_program = other.m_program;
        other.m_program = 0;
    }
    return *this;
}

void ShaderProgram::use() const noexcept {
    glUseProgram(m_program);
}

GLint ShaderProgram::getUniformLocation(const char* name) const noexcept {
    return glGetUniformLocation(m_program, name);
}

GLint ShaderProgram::getAttribLocation(const char* name) const noexcept {
    return glGetAttribLocation(m_program, name);
}

// ==========================================
// SolidQuadProgram Implementation
// ==========================================

std::unique_ptr<SolidQuadProgram> SolidQuadProgram::create() {
    auto base = ShaderProgram::create(kSolidQuadVert, kSolidQuadFrag);
    if (!base) return nullptr;

    GLuint id = base->release();
    return std::make_unique<SolidQuadProgram>(id);
}

SolidQuadProgram::SolidQuadProgram(GLuint programId)
    : ShaderProgram(programId) {
    m_projLoc = getUniformLocation("proj");
    m_colorLoc = getUniformLocation("color");
    m_posAttrib = getAttribLocation("pos");
}

void SolidQuadProgram::setProjection(const Matrix3& proj) const noexcept {
    if (m_projLoc >= 0) {
        glUniformMatrix3fv(m_projLoc, 1, GL_FALSE, proj.data());
    }
}

void SolidQuadProgram::setColor(const Color& color) const noexcept {
    if (m_colorLoc >= 0) {
        glUniform4f(m_colorLoc, color.r, color.g, color.b, color.a);
    }
}

// ==========================================
// TextureQuadProgram Implementation
// ==========================================

std::unique_ptr<TextureQuadProgram> TextureQuadProgram::create(Target target) {
    std::string_view fragSrc;
    switch (target) {
        case Target::Rgba:
            fragSrc = kTextureQuadRgbaFrag;
            break;
        case Target::Rgbx:
            fragSrc = kTextureQuadRgbxFrag;
            break;
        case Target::ExternalOes:
            fragSrc = kTextureQuadExternalFrag;
            break;
        case Target::ExternalOesRgbx:
            fragSrc = kTextureQuadExternalRgbxFrag;
            break;
    }

    auto vert = Shader::compile(GL_VERTEX_SHADER, kTextureQuadVert);
    if (!vert) return nullptr;

    auto frag = Shader::compile(GL_FRAGMENT_SHADER, fragSrc);
    if (!frag) return nullptr;

    GLuint prog = glCreateProgram();
    if (!prog) return nullptr;

    glAttachShader(prog, vert->id());
    glAttachShader(prog, frag->id());
    glLinkProgram(prog);

    GLint status = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (!status) {
        GLint logLen = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
        std::vector<char> infoLog(static_cast<size_t>(logLen > 1 ? logLen : 256));
        glGetProgramInfoLog(prog, static_cast<GLsizei>(infoLog.size()), nullptr, infoLog.data());
        logError("Texture program link failed: {}", infoLog.data());
        glDeleteProgram(prog);
        return nullptr;
    }

    return std::make_unique<TextureQuadProgram>(prog, target);
}

TextureQuadProgram::TextureQuadProgram(GLuint programId, Target target)
    : ShaderProgram(programId), m_target(target) {
    m_projLoc = getUniformLocation("proj");
    m_texProjLoc = getUniformLocation("tex_proj");
    m_texLoc = getUniformLocation("tex");
    m_alphaLoc = getUniformLocation("alpha");
    m_posAttrib = getAttribLocation("pos");
}

void TextureQuadProgram::setProjection(const Matrix3& proj) const noexcept {
    if (m_projLoc >= 0) {
        glUniformMatrix3fv(m_projLoc, 1, GL_FALSE, proj.data());
    }
}

void TextureQuadProgram::setTextureProjection(const Matrix3& texProj) const noexcept {
    if (m_texProjLoc >= 0) {
        glUniformMatrix3fv(m_texProjLoc, 1, GL_FALSE, texProj.data());
    }
}

void TextureQuadProgram::setAlpha(float alpha) const noexcept {
    if (m_alphaLoc >= 0) {
        glUniform1f(m_alphaLoc, alpha);
    }
}

void TextureQuadProgram::setTextureUnit(int unit) const noexcept {
    if (m_texLoc >= 0) {
        glUniform1i(m_texLoc, unit);
    }
}

// ==========================================
// ShaderManager Implementation
// ==========================================

std::shared_ptr<ShaderManager> ShaderManager::create(bool supportExternalOes) {
    auto solid = SolidQuadProgram::create();
    if (!solid) {
        logError("Failed to compile SolidQuadProgram");
        return nullptr;
    }

    auto texRgba = TextureQuadProgram::create(TextureQuadProgram::Target::Rgba);
    if (!texRgba) {
        logError("Failed to compile TextureQuadProgram (RGBA)");
        return nullptr;
    }

    auto texRgbx = TextureQuadProgram::create(TextureQuadProgram::Target::Rgbx);
    if (!texRgbx) {
        logError("Failed to compile TextureQuadProgram (RGBX)");
        return nullptr;
    }

    std::unique_ptr<TextureQuadProgram> texExternal = nullptr;
    std::unique_ptr<TextureQuadProgram> texExternalRgbx = nullptr;
    if (supportExternalOes) {
        texExternal = TextureQuadProgram::create(TextureQuadProgram::Target::ExternalOes);
        if (!texExternal) {
            logWarn("External OES texture shader not supported by GL platform, falling back");
        }
        texExternalRgbx = TextureQuadProgram::create(TextureQuadProgram::Target::ExternalOesRgbx);
        if (!texExternalRgbx) {
            logWarn("External OES RGBX texture shader not supported by GL platform, falling back");
        }
    }

    return std::make_shared<ShaderManager>(
        std::move(solid),
        std::move(texRgba),
        std::move(texRgbx),
        std::move(texExternal),
        std::move(texExternalRgbx)
    );
}

ShaderManager::ShaderManager(
    std::unique_ptr<SolidQuadProgram> solid,
    std::unique_ptr<TextureQuadProgram> texRgba,
    std::unique_ptr<TextureQuadProgram> texRgbx,
    std::unique_ptr<TextureQuadProgram> texExternal,
    std::unique_ptr<TextureQuadProgram> texExternalRgbx)
    : m_solidQuad(std::move(solid)),
      m_textureRgba(std::move(texRgba)),
      m_textureRgbx(std::move(texRgbx)),
      m_textureExternal(std::move(texExternal)),
      m_textureExternalRgbx(std::move(texExternalRgbx)) {}

} // namespace nisaba::compositor

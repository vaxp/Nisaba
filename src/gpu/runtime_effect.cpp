#include "nisaba/gpu/runtime_effect.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <sstream>
#include <regex>
#include <iostream>
#include <algorithm>

namespace nisaba::gpu {

RuntimeEffect::~RuntimeEffect() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
    if (vert_shader_ != 0) {
        glDeleteShader(vert_shader_);
        vert_shader_ = 0;
    }
    if (frag_shader_ != 0) {
        glDeleteShader(frag_shader_);
        frag_shader_ = 0;
    }
}

const RuntimeEffect::Uniform* RuntimeEffect::findUniform(const char* name) const noexcept {
    if (!name) return nullptr;
    for (const auto& u : uniforms_) {
        if (u.name == name) return &u;
    }
    return nullptr;
}

RuntimeEffect::Result RuntimeEffect::MakeForShader(std::string_view sksl) {
    if (sksl.empty()) {
        return Result{nullptr, "Empty shader source string."};
    }

    std::string src(sksl);
    std::vector<Uniform> uniforms;
    size_t current_offset = 0;

    // Regex to detect "uniform <type> <name>;"
    // Matches standard GLSL and SkSL variable definitions
    std::regex uniform_regex(R"(uniform\s+([A-Za-z0-9_]+)\s+([A-Za-z0-9_]+)\s*;)");
    auto words_begin = std::sregex_iterator(src.begin(), src.end(), uniform_regex);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string type_str = match[1].str();
        std::string name_str = match[2].str();

        Uniform u;
        u.name = name_str;
        u.offset = current_offset;

        if (type_str == "float" || type_str == "half") {
            u.type = Uniform::Type::Float;
            u.sizeInBytes = 4;
        } else if (type_str == "vec2" || type_str == "float2" || type_str == "half2") {
            u.type = Uniform::Type::Float2;
            u.sizeInBytes = 8;
        } else if (type_str == "vec3" || type_str == "float3" || type_str == "half3") {
            u.type = Uniform::Type::Float3;
            u.sizeInBytes = 12;
        } else if (type_str == "vec4" || type_str == "float4" || type_str == "half4") {
            u.type = Uniform::Type::Float4;
            u.sizeInBytes = 16;
        } else if (type_str == "mat3" || type_str == "float3x3" || type_str == "half3x3") {
            u.type = Uniform::Type::Mat3;
            u.sizeInBytes = 36;
        } else if (type_str == "mat4" || type_str == "float4x4" || type_str == "half4x4") {
            u.type = Uniform::Type::Mat4;
            u.sizeInBytes = 64;
        } else if (type_str == "int") {
            u.type = Uniform::Type::Int;
            u.sizeInBytes = 4;
        } else {
            // Default 4 bytes for unrecognized uniform scalar
            u.type = Uniform::Type::Float;
            u.sizeInBytes = 4;
        }

        current_offset += u.sizeInBytes;
        uniforms.push_back(u);
    }

    // Determine main() signature: 1 argument vs 2 arguments
    bool has_color_in = false;
    std::regex main_2args_regex(R"(\b(vec4|half4|float4)\s+main\s*\(\s*[^,)]+\s*,\s*[^)]+\))");
    if (std::regex_search(src, main_2args_regex)) {
        has_color_in = true;
    }

    // Replace "vec4 main(" with "vec4 user_main("
    std::regex main_rename_regex(R"(\b(vec4|half4|float4)\s+main\s*\()");
    std::string transformed_code = std::regex_replace(src, main_rename_regex, "$1 user_main(");

    // Build complete Vertex Shader
    static const char* kVertSource =
        "#version 150 core\n"
        "uniform vec2 viewSize;\n"
        "in vec2 vertex;\n"
        "in vec2 tcoord;\n"
        "in vec4 color;\n"
        "out vec2 fpos;\n"
        "out vec2 ftcoord;\n"
        "out vec4 fcolor;\n"
        "void main(void) {\n"
        "    fpos = vertex;\n"
        "    ftcoord = tcoord;\n"
        "    fcolor = color;\n"
        "    gl_Position = vec4(2.0 * vertex.x / viewSize.x - 1.0, 1.0 - 2.0 * vertex.y / viewSize.y, 0.0, 1.0);\n"
        "}\n";

    // Build complete Fragment Shader with GLSL 150 header and user code wrapper
    std::ostringstream frag_ss;
    frag_ss << "#version 150 core\n"
            << "precision highp float;\n\n"
            << "#define float2 vec2\n"
            << "#define float3 vec3\n"
            << "#define float4 vec4\n"
            << "#define float2x2 mat2\n"
            << "#define float3x3 mat3\n"
            << "#define float4x4 mat4\n"
            << "#define half float\n"
            << "#define half2 vec2\n"
            << "#define half3 vec3\n"
            << "#define half4 vec4\n"
            << "#define half2x2 mat2\n"
            << "#define half3x3 mat3\n"
            << "#define half4x4 mat4\n\n"
            << "uniform vec2 viewSize;\n"
            << "uniform mat3 u_localMatrix;\n\n"
            << "in vec2 fpos;\n"
            << "in vec2 ftcoord;\n"
            << "in vec4 fcolor;\n"
            << "out vec4 outColor;\n\n"
            << transformed_code << "\n\n"
            << "void main(void) {\n"
            << "    vec2 localCoord = (u_localMatrix * vec3(fpos, 1.0)).xy;\n";

    if (has_color_in) {
        frag_ss << "    outColor = user_main(localCoord, fcolor);\n";
    } else {
        frag_ss << "    outColor = user_main(localCoord);\n";
    }
    frag_ss << "}\n";

    std::string kFragSource = frag_ss.str();

    // Compile Vertex Shader
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    const char* v_ptr = kVertSource;
    glShaderSource(vert, 1, &v_ptr, nullptr);
    glCompileShader(vert);

    GLint status = 0;
    glGetShaderiv(vert, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        GLsizei len = 0;
        glGetShaderInfoLog(vert, sizeof(log), &len, log);
        glDeleteShader(vert);
        return Result{nullptr, std::string("Vertex shader compile error: ") + log};
    }

    // Compile Fragment Shader
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f_ptr = kFragSource.c_str();
    glShaderSource(frag, 1, &f_ptr, nullptr);
    glCompileShader(frag);

    glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[2048];
        GLsizei len = 0;
        glGetShaderInfoLog(frag, sizeof(log), &len, log);
        glDeleteShader(vert);
        glDeleteShader(frag);
        return Result{nullptr, std::string("Fragment shader compile error: ") + log};
    }

    // Link Program
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);

    glBindAttribLocation(prog, 0, "vertex");
    glBindAttribLocation(prog, 1, "tcoord");
    glBindAttribLocation(prog, 2, "color");

    glLinkProgram(prog);
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        GLsizei len = 0;
        glGetProgramInfoLog(prog, sizeof(log), &len, log);
        glDeleteShader(vert);
        glDeleteShader(frag);
        glDeleteProgram(prog);
        return Result{nullptr, std::string("Program link error: ") + log};
    }

    // Query uniform locations
    int loc_mat = glGetUniformLocation(prog, "u_localMatrix");
    int loc_view = glGetUniformLocation(prog, "viewSize");

    for (auto& u : uniforms) {
        u.location = glGetUniformLocation(prog, u.name.c_str());
    }

    // Construct RuntimeEffect
    std::shared_ptr<RuntimeEffect> effect(new RuntimeEffect());
    effect->program_ = prog;
    effect->vert_shader_ = vert;
    effect->frag_shader_ = frag;
    effect->uniforms_ = std::move(uniforms);
    effect->uniform_size_ = current_offset;
    effect->loc_local_mat_ = loc_mat;
    effect->loc_view_size_ = loc_view;

    return Result{effect, ""};
}

void RuntimeShader::bind(const float* viewSize) const {
    if (!effect_ || effect_->programId() == 0) return;

    glUseProgram(effect_->programId());

    if (effect_->viewSizeLoc() >= 0 && viewSize) {
        glUniform2fv(effect_->viewSizeLoc(), 1, viewSize);
    }

    if (effect_->localMatrixLoc() >= 0) {
        Transform2D invLocal;
        if (!local_matrix_.inverse(invLocal)) {
            invLocal = Transform2D::identity();
        }

        // Column-major 3x3 matrix for OpenGL:
        // [ m0 m2 m4 ]
        // [ m1 m3 m5 ]
        // [  0  0  1 ]
        float m3[9] = {
            invLocal[0], invLocal[1], 0.0f,
            invLocal[2], invLocal[3], 0.0f,
            invLocal[4], invLocal[5], 1.0f
        };
        glUniformMatrix3fv(effect_->localMatrixLoc(), 1, GL_FALSE, m3);
    }

    for (const auto& u : effect_->uniforms()) {
        if (u.location < 0 || u.offset + u.sizeInBytes > uniform_data_.size()) continue;
        const uint8_t* ptr = uniform_data_.data() + u.offset;

        switch (u.type) {
            case RuntimeEffect::Uniform::Type::Float:
                glUniform1f(u.location, *reinterpret_cast<const float*>(ptr));
                break;
            case RuntimeEffect::Uniform::Type::Float2:
                glUniform2fv(u.location, 1, reinterpret_cast<const float*>(ptr));
                break;
            case RuntimeEffect::Uniform::Type::Float3:
                glUniform3fv(u.location, 1, reinterpret_cast<const float*>(ptr));
                break;
            case RuntimeEffect::Uniform::Type::Float4:
                glUniform4fv(u.location, 1, reinterpret_cast<const float*>(ptr));
                break;
            case RuntimeEffect::Uniform::Type::Mat3:
                glUniformMatrix3fv(u.location, 1, GL_FALSE, reinterpret_cast<const float*>(ptr));
                break;
            case RuntimeEffect::Uniform::Type::Mat4:
                glUniformMatrix4fv(u.location, 1, GL_FALSE, reinterpret_cast<const float*>(ptr));
                break;
            case RuntimeEffect::Uniform::Type::Int:
                glUniform1i(u.location, *reinterpret_cast<const int*>(ptr));
                break;
        }
    }
}

RuntimeShaderBuilder::RuntimeShaderBuilder(std::shared_ptr<RuntimeEffect> effect)
    : effect_(std::move(effect)) {
    if (effect_) {
        uniform_data_.resize(effect_->uniformSize(), 0);
    }
}

void RuntimeShaderBuilder::writeUniformData(size_t offset, const void* data, size_t size) {
    if (offset + size <= uniform_data_.size() && data != nullptr) {
        std::memcpy(uniform_data_.data() + offset, data, size);
    }
}

std::shared_ptr<Shader> RuntimeShaderBuilder::makeShader(const Transform2D* localMatrix) {
    Transform2D lm = localMatrix ? *localMatrix : Transform2D::identity();
    return std::make_shared<RuntimeShader>(effect_, uniform_data_, lm);
}

std::shared_ptr<Shader> RuntimeShaderBuilder::makeShader(const nisaba::Transform* localMatrix) {
    Transform2D lm = localMatrix ? Transform2D(*localMatrix) : Transform2D::identity();
    return std::make_shared<RuntimeShader>(effect_, uniform_data_, lm);
}

} // namespace nisaba::gpu

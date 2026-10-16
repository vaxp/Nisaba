#include "nisaba/gpu/runtime_effect.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <cctype>
#include <cstring>
#include <iostream>
#include <algorithm>

namespace nisaba::gpu {

namespace {
static GLuint s_shared_vert_shader = 0;

static GLuint get_shared_vertex_shader() {
    if (s_shared_vert_shader == 0) {
        static const char* kVertSource =
            "#version 150 core\n"
            "uniform vec2 viewSize;\n"
            "uniform mat3 u_localMatrix;\n"
            "in vec2 vertex;\n"
            "in vec2 tcoord;\n"
            "in vec4 color;\n"
            "out vec2 fpos;\n"
            "out vec2 ftcoord;\n"
            "out vec4 fcolor;\n"
            "out vec2 v_localCoord;\n"
            "void main(void) {\n"
            "    fpos = vertex;\n"
            "    ftcoord = tcoord;\n"
            "    fcolor = color;\n"
            "    v_localCoord = (u_localMatrix * vec3(vertex, 1.0)).xy;\n"
            "    gl_Position = vec4(2.0 * vertex.x / viewSize.x - 1.0, 1.0 - 2.0 * vertex.y / viewSize.y, 0.0, 1.0);\n"
            "}\n";
        s_shared_vert_shader = glCreateShader(GL_VERTEX_SHADER);
        const char* v_ptr = kVertSource;
        glShaderSource(s_shared_vert_shader, 1, &v_ptr, nullptr);
        glCompileShader(s_shared_vert_shader);
    }
    return s_shared_vert_shader;
}
} // namespace

RuntimeEffect::~RuntimeEffect() {
    if (program_ != 0) {
        glDeleteProgram(program_);
        program_ = 0;
    }
    if (vert_shader_ != 0 && vert_shader_ != s_shared_vert_shader) {
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

    const size_t n = sksl.size();
    std::string transformed_code;
    transformed_code.reserve(n + 64);

    std::vector<Uniform> uniforms;
    uniforms.reserve(16);
    size_t current_offset = 0;
    bool has_color_in = false;

    auto skip_whitespace_and_comments = [&](size_t& idx) {
        while (idx < n) {
            if (std::isspace(static_cast<unsigned char>(sksl[idx]))) {
                idx++;
            } else if (idx + 1 < n && sksl[idx] == '/' && sksl[idx + 1] == '/') {
                idx += 2;
                while (idx < n && sksl[idx] != '\n') idx++;
            } else if (idx + 1 < n && sksl[idx] == '/' && sksl[idx + 1] == '*') {
                idx += 2;
                while (idx + 1 < n && !(sksl[idx] == '*' && sksl[idx + 1] == '/')) idx++;
                if (idx + 1 < n) idx += 2;
            } else {
                break;
            }
        }
    };

    auto is_ident_start = [](char c) {
        return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
    };
    auto is_ident_char = [](char c) {
        return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
    };

    size_t i = 0;
    std::string_view last_ident;

    while (i < n) {
        // Fast comment preservation
        if (i + 1 < n && sksl[i] == '/' && sksl[i + 1] == '/') {
            size_t start = i;
            i += 2;
            while (i < n && sksl[i] != '\n') i++;
            transformed_code.append(sksl.substr(start, i - start));
            continue;
        }
        if (i + 1 < n && sksl[i] == '/' && sksl[i + 1] == '*') {
            size_t start = i;
            i += 2;
            while (i + 1 < n && !(sksl[i] == '*' && sksl[i + 1] == '/')) i++;
            if (i + 1 < n) i += 2;
            transformed_code.append(sksl.substr(start, i - start));
            continue;
        }

        if (is_ident_start(sksl[i])) {
            size_t start = i;
            while (i < n && is_ident_char(sksl[i])) i++;
            std::string_view ident = sksl.substr(start, i - start);

            if (ident == "uniform") {
                // Look ahead for `<type> <name>;`
                size_t p = i;
                skip_whitespace_and_comments(p);
                if (p < n && is_ident_start(sksl[p])) {
                    size_t type_start = p;
                    while (p < n && is_ident_char(sksl[p])) p++;
                    std::string_view type_str = sksl.substr(type_start, p - type_start);

                    skip_whitespace_and_comments(p);
                    if (p < n && is_ident_start(sksl[p])) {
                        size_t name_start = p;
                        while (p < n && is_ident_char(sksl[p])) p++;
                        std::string_view name_str = sksl.substr(name_start, p - name_start);

                        skip_whitespace_and_comments(p);
                        if (p < n && sksl[p] == ';') {
                            Uniform u;
                            u.name = std::string(name_str);
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
                                u.type = Uniform::Type::Float;
                                u.sizeInBytes = 4;
                            }

                            current_offset += u.sizeInBytes;
                            uniforms.push_back(u);
                        }
                    }
                }
                transformed_code.append(ident);
            } else if (ident == "main") {
                // Check if preceded by return type (vec4, half4, float4) and followed by '('
                size_t p = i;
                skip_whitespace_and_comments(p);
                if (p < n && sksl[p] == '(') {
                    bool valid_ret = (last_ident == "vec4" || last_ident == "half4" || last_ident == "float4");
                    if (valid_ret) {
                        size_t arg_start = p + 1;
                        int paren_depth = 1;
                        size_t k = arg_start;
                        while (k < n && paren_depth > 0) {
                            if (sksl[k] == '(') paren_depth++;
                            else if (sksl[k] == ')') paren_depth--;
                            else if (sksl[k] == ',' && paren_depth == 1) has_color_in = true;
                            k++;
                        }
                        transformed_code.append("user_main");
                    } else {
                        transformed_code.append(ident);
                    }
                } else {
                    transformed_code.append(ident);
                }
            } else {
                transformed_code.append(ident);
            }

            last_ident = ident;
            continue;
        }

        transformed_code.push_back(sksl[i]);
        i++;
    }


    // Build complete Fragment Shader with GLSL 150 header and zero ALU matrix overhead
    std::string kFragSource;
    kFragSource.reserve(transformed_code.size() + 512);
    kFragSource +=
        "#version 150 core\n"
        "precision highp float;\n\n"
        "#define float2 vec2\n"
        "#define float3 vec3\n"
        "#define float4 vec4\n"
        "#define float2x2 mat2\n"
        "#define float3x3 mat3\n"
        "#define float4x4 mat4\n"
        "#define half float\n"
        "#define half2 vec2\n"
        "#define half3 vec3\n"
        "#define half4 vec4\n"
        "#define half2x2 mat2\n"
        "#define half3x3 mat3\n"
        "#define half4x4 mat4\n\n"
        "uniform vec2 viewSize;\n\n"
        "in vec2 fpos;\n"
        "in vec2 ftcoord;\n"
        "in vec4 fcolor;\n"
        "in vec2 v_localCoord;\n"
        "out vec4 outColor;\n\n";

    kFragSource += transformed_code;
    kFragSource += "\n\nvoid main(void) {\n";
    if (has_color_in) {
        kFragSource += "    outColor = user_main(v_localCoord, fcolor);\n";
    } else {
        kFragSource += "    outColor = user_main(v_localCoord);\n";
    }
    kFragSource += "}\n";

    // Use shared pre-compiled Vertex Shader
    GLuint vert = get_shared_vertex_shader();

    // Compile Fragment Shader
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    const char* f_ptr = kFragSource.c_str();
    glShaderSource(frag, 1, &f_ptr, nullptr);
    glCompileShader(frag);

    GLint status = 0;
    glGetShaderiv(frag, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[2048];
        GLsizei len = 0;
        glGetShaderInfoLog(frag, sizeof(log), &len, log);
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

RuntimeShader::RuntimeShader(std::shared_ptr<RuntimeEffect> effect,
                             std::vector<uint8_t> uniformData,
                             Transform2D localMatrix)
    : effect_(std::move(effect)),
      uniform_data_(std::move(uniformData)),
      local_matrix_(localMatrix) {
    Transform2D invLocal;
    if (!local_matrix_.inverse(invLocal)) {
        invLocal = Transform2D::identity();
    }
    inv_m3_[0] = invLocal[0]; inv_m3_[1] = invLocal[1]; inv_m3_[2] = 0.0f;
    inv_m3_[3] = invLocal[2]; inv_m3_[4] = invLocal[3]; inv_m3_[5] = 0.0f;
    inv_m3_[6] = invLocal[4]; inv_m3_[7] = invLocal[5]; inv_m3_[8] = 1.0f;
}

void RuntimeShader::bind(const float* viewSize) const {
    if (!effect_ || effect_->programId() == 0) return;

    glUseProgram(effect_->programId());

    if (effect_->viewSizeLoc() >= 0 && viewSize) {
        glUniform2fv(effect_->viewSizeLoc(), 1, viewSize);
    }

    if (effect_->localMatrixLoc() >= 0) {
        glUniformMatrix3fv(effect_->localMatrixLoc(), 1, GL_FALSE, inv_m3_);
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

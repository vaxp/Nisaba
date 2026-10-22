#include "nisaba/render3d/gpu/gl_shader_program.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <vector>

namespace nisaba::render3d {

GlShaderProgram::GlShaderProgram() = default;

GlShaderProgram::~GlShaderProgram() {
    cleanup();
}

GlShaderProgram::GlShaderProgram(GlShaderProgram&& rhs) noexcept
    : m_program_id(rhs.m_program_id),
      m_is_valid(rhs.m_is_valid),
      m_log(std::move(rhs.m_log)),
      m_uniform_cache(std::move(rhs.m_uniform_cache)) {
    rhs.m_program_id = 0;
    rhs.m_is_valid = false;
}

GlShaderProgram& GlShaderProgram::operator=(GlShaderProgram&& rhs) noexcept {
    if (this != &rhs) {
        cleanup();
        m_program_id = rhs.m_program_id;
        m_is_valid = rhs.m_is_valid;
        m_log = std::move(rhs.m_log);
        m_uniform_cache = std::move(rhs.m_uniform_cache);
        rhs.m_program_id = 0;
        rhs.m_is_valid = false;
    }
    return *this;
}

void GlShaderProgram::cleanup() noexcept {
    if (m_program_id != 0) {
        glDeleteProgram(m_program_id);
        m_program_id = 0;
    }
    m_is_valid = false;
    m_uniform_cache.clear();
}

static GLuint compile_shader_stage(GLenum stage, std::string_view source, std::string& out_log) {
    GLuint shader = glCreateShader(stage);
    const char* src_ptr = source.data();
    const GLint src_len = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &src_ptr, &src_len);
    glCompileShader(shader);

    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint log_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 1) {
            std::vector<char> info_log(log_len);
            glGetShaderInfoLog(shader, log_len, nullptr, info_log.data());
            out_log += std::string(info_log.data()) + "\n";
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool GlShaderProgram::compile_and_link(std::string_view vertex_src, std::string_view fragment_src) {
    cleanup();
    m_log.clear();

    GLuint vert_shader = compile_shader_stage(GL_VERTEX_SHADER, vertex_src, m_log);
    if (!vert_shader) {
        m_log = "Vertex Shader Compile Error:\n" + m_log;
        return false;
    }

    GLuint frag_shader = compile_shader_stage(GL_FRAGMENT_SHADER, fragment_src, m_log);
    if (!frag_shader) {
        glDeleteShader(vert_shader);
        m_log = "Fragment Shader Compile Error:\n" + m_log;
        return false;
    }

    m_program_id = glCreateProgram();
    glAttachShader(m_program_id, vert_shader);
    glAttachShader(m_program_id, frag_shader);
    glLinkProgram(m_program_id);

    GLint linked = 0;
    glGetProgramiv(m_program_id, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint log_len = 0;
        glGetProgramiv(m_program_id, GL_INFO_LOG_LENGTH, &log_len);
        if (log_len > 1) {
            std::vector<char> info_log(log_len);
            glGetProgramInfoLog(m_program_id, log_len, nullptr, info_log.data());
            m_log = "Program Link Error:\n" + std::string(info_log.data());
        }
        glDeleteShader(vert_shader);
        glDeleteShader(frag_shader);
        cleanup();
        return false;
    }

    glDetachShader(m_program_id, vert_shader);
    glDetachShader(m_program_id, frag_shader);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    m_is_valid = true;
    return true;
}

void GlShaderProgram::bind() const noexcept {
    if (m_program_id != 0) {
        glUseProgram(m_program_id);
    }
}

void GlShaderProgram::unbind() const noexcept {
    glUseProgram(0);
}

int GlShaderProgram::get_uniform_location(std::string_view name) {
    if (!m_is_valid) return -1;
    std::string str_name(name);
    auto it = m_uniform_cache.find(str_name);
    if (it != m_uniform_cache.end()) {
        return it->second;
    }
    int loc = glGetUniformLocation(m_program_id, str_name.c_str());
    m_uniform_cache[str_name] = loc;
    return loc;
}

bool GlShaderProgram::set_uniform(std::string_view name, float value) {
    int loc = get_uniform_location(name);
    if (loc < 0) return false;
    glUniform1f(loc, value);
    return true;
}

bool GlShaderProgram::set_uniform(std::string_view name, int value) {
    int loc = get_uniform_location(name);
    if (loc < 0) return false;
    glUniform1i(loc, value);
    return true;
}

bool GlShaderProgram::set_uniform(std::string_view name, const Vec3& v) {
    int loc = get_uniform_location(name);
    if (loc < 0) return false;
    glUniform3f(loc, v.x, v.y, v.z);
    return true;
}

bool GlShaderProgram::set_uniform(std::string_view name, float x, float y, float z, float w) {
    int loc = get_uniform_location(name);
    if (loc < 0) return false;
    glUniform4f(loc, x, y, z, w);
    return true;
}

bool GlShaderProgram::set_uniform(std::string_view name, const Mat44& mat, bool transpose) {
    int loc = get_uniform_location(name);
    if (loc < 0) return false;
    // Nisaba Mat44 is row-major. Setting transpose = GL_TRUE maps it to column-major shader expectation.
    glUniformMatrix4fv(loc, 1, transpose ? GL_FALSE : GL_TRUE, mat.m);
    return true;
}

bool GlShaderProgram::set_uniform_block_binding(std::string_view block_name, uint32_t binding_point) {
    if (!m_is_valid) return false;
    std::string str_name(block_name);
    GLuint block_index = glGetUniformBlockIndex(m_program_id, str_name.c_str());
    if (block_index == GL_INVALID_INDEX) {
        return false;
    }
    glUniformBlockBinding(m_program_id, block_index, binding_point);
    return true;
}

std::unique_ptr<GlShaderProgram> GlShaderProgram::create_pbr_shader() {
    static constexpr const char* s_vert = R"(#version 330 core
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord0;
layout(location = 4) in vec2 inTexCoord1;

layout(std140) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 cameraPosition;
} camera;

uniform mat4 u_model;
uniform mat4 u_normalMatrix;
uniform mat4 u_lightSpaceMatrix;

out vec3 v_worldPos;
out vec3 v_normal;
out vec4 v_tangent;
out vec2 v_uv0;
out vec4 v_fragPosLightSpace;

void main() {
    vec4 worldPos = u_model * vec4(inPosition, 1.0);
    v_worldPos = worldPos.xyz;
    v_normal = normalize((u_normalMatrix * vec4(inNormal, 0.0)).xyz);
    v_tangent = vec4(normalize((u_normalMatrix * vec4(inTangent.xyz, 0.0)).xyz), inTangent.w);
    v_uv0 = inTexCoord0;
    v_fragPosLightSpace = u_lightSpaceMatrix * worldPos;
    gl_Position = camera.viewProjection * worldPos;
}
)";

    static constexpr const char* s_frag = R"(#version 330 core
in vec3 v_worldPos;
in vec3 v_normal;
in vec4 v_tangent;
in vec2 v_uv0;
in vec4 v_fragPosLightSpace;

layout(std140) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 cameraPosition;
} camera;

layout(std140) uniform MaterialUBO {
    vec4 baseColorFactor;
    vec3 emissiveFactor;
    float normalScale;
    float metallicFactor;
    float roughnessFactor;
    float occlusionStrength;
    float alphaCutoff;
    uint textureFlags;
    uint alphaMode;
    uint doubleSided;
    float pad;
} material;

uniform sampler2D u_baseColorTexture;
uniform sampler2D u_metallicRoughnessTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_occlusionTexture;
uniform sampler2D u_emissiveTexture;
uniform sampler2D u_shadowMap;

uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform float u_lightIntensity;
uniform vec3 u_ambientColor;

out vec4 FragColor;

const float PI = 3.14159265358979323846;

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (PI * denom * denom + 1e-7);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 0.0;
    float currentDepth = projCoords.z;
    float bias = max(0.008 * (1.0 - dot(normal, lightDir)), 0.001);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap, 0));
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    return shadow / 9.0;
}

void main() {
    vec4 baseColor = material.baseColorFactor;
    if ((material.textureFlags & 1u) != 0u) {
        vec4 texSample = texture(u_baseColorTexture, v_uv0);
        // glTF baseColorTexture is sRGB: linearize for physically accurate lighting calculation
        texSample.rgb = pow(texSample.rgb, vec3(2.2));
        baseColor *= texSample;
    }
    if (material.alphaMode == 1u && baseColor.a < material.alphaCutoff) {
        discard;
    }

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if ((material.textureFlags & 2u) != 0u) {
        vec4 mrSample = texture(u_metallicRoughnessTexture, v_uv0);
        roughness *= mrSample.g;
        metallic *= mrSample.b;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    vec3 N = normalize(v_normal);
    if ((material.textureFlags & 4u) != 0u) {
        vec3 normalMap = texture(u_normalTexture, v_uv0).rgb * 2.0 - 1.0;
        normalMap.xy *= material.normalScale;
        vec3 T = v_tangent.xyz - N * dot(v_tangent.xyz, N);
        if (dot(T, T) > 1e-6) {
            T = normalize(T);
            vec3 B = normalize(cross(N, T) * (v_tangent.w < 0.0 ? -1.0 : 1.0));
            mat3 TBN = mat3(T, B, N);
            N = normalize(TBN * normalMap);
        } else {
            // Screen-space derivative TBN reconstruction when tangents are omitted
            vec3 dp1 = dFdx(v_worldPos);
            vec3 dp2 = dFdy(v_worldPos);
            vec2 duv1 = dFdx(v_uv0);
            vec2 duv2 = dFdy(v_uv0);
            vec3 dp2perp = cross(dp2, N);
            vec3 dp1perp = cross(N, dp1);
            vec3 T_est = dp2perp * duv1.x + dp1perp * duv2.x;
            vec3 B_est = dp2perp * duv1.y + dp1perp * duv2.y;
            float invmax = inversesqrt(max(dot(T_est, T_est), dot(B_est, B_est)));
            if (!isinf(invmax) && !isnan(invmax)) {
                mat3 TBN = mat3(T_est * invmax, B_est * invmax, N);
                N = normalize(TBN * normalMap);
            }
        }
    }

    // Toksvig / Geometric normal variance anti-aliasing:
    // Filters specular footprint using normal screen derivatives to eliminate floor sparkles/glitter
    vec3 dN_dx = dFdx(N);
    vec3 dN_dy = dFdy(N);
    float variance = max(dot(dN_dx, dN_dx), dot(dN_dy, dN_dy));
    float kernelRoughness = clamp(2.0 * variance, 0.0, 1.0);
    float filtered_roughness = sqrt(clamp(roughness * roughness + kernelRoughness, 0.0016, 1.0));

    vec3 V = normalize(camera.cameraPosition - v_worldPos);
    vec3 L = normalize(-u_lightDirection);
    vec3 H = normalize(V + L);

    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float NDF = DistributionGGX(N, H, filtered_roughness);
    float G = GeometrySmith(N, V, L, filtered_roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    float NdotL = max(dot(N, L), 0.0);

    float shadow = calculateShadow(v_fragPosLightSpace, N, L);
    vec3 directLight = (kD * baseColor.rgb / PI + specular) * u_lightColor * u_lightIntensity * NdotL * (1.0 - shadow);

    float ao = 1.0;
    if ((material.textureFlags & 8u) != 0u) {
        ao = mix(1.0, texture(u_occlusionTexture, v_uv0).r, material.occlusionStrength);
    }

    // Hemispherical ambient bounce (soft skylight from above, warm bounce from floor)
    vec3 skyAmbient = u_ambientColor * 1.25;
    vec3 groundAmbient = u_ambientColor * vec3(0.70, 0.65, 0.55);
    float hemiFactor = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 envAmbient = mix(groundAmbient, skyAmbient, hemiFactor);
    vec3 ambient = envAmbient * mix(baseColor.rgb, F0, metallic) * ao;

    vec3 emissive = material.emissiveFactor;
    if ((material.textureFlags & 16u) != 0u) {
        vec3 emTex = texture(u_emissiveTexture, v_uv0).rgb;
        emTex = pow(emTex, vec3(2.2));
        emissive *= emTex;
    }

    vec3 hdrColor = ambient + directLight + emissive;

    // ACES Filmic Tone Mapping (Narkowicz 2015)
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    vec3 ldrColor = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e), 0.0, 1.0);

    // Gamma correction to sRGB for display output
    vec3 finalColor = pow(ldrColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, baseColor.a);
}
)";

    auto program = std::make_unique<GlShaderProgram>();
    if (!program->compile_and_link(s_vert, s_frag)) {
        std::cerr << "[GlShaderProgram] PBR Shader compilation failed: " << program->log() << std::endl;
        return nullptr;
    }

    // Assign uniform block binding points
    program->set_uniform_block_binding("CameraUBO", 0);
    program->set_uniform_block_binding("MaterialUBO", 2);

    // Default texture units
    program->bind();
    program->set_uniform("u_baseColorTexture", 0);
    program->set_uniform("u_metallicRoughnessTexture", 1);
    program->set_uniform("u_normalTexture", 2);
    program->set_uniform("u_occlusionTexture", 3);
    program->set_uniform("u_emissiveTexture", 4);
    program->set_uniform("u_shadowMap", 5);
    program->unbind();

    return program;
}

std::unique_ptr<GlShaderProgram> GlShaderProgram::create_shadow_depth_shader() {
    static constexpr const char* s_vert = R"(#version 330 core
layout(location = 0) in vec3 inPosition;

uniform mat4 u_lightSpaceMatrix;
uniform mat4 u_model;

void main() {
    gl_Position = u_lightSpaceMatrix * u_model * vec4(inPosition, 1.0);
}
)";

    static constexpr const char* s_frag = R"(#version 330 core
void main() {
    // Hardware writes gl_FragDepth automatically
}
)";

    auto program = std::make_unique<GlShaderProgram>();
    if (!program->compile_and_link(s_vert, s_frag)) {
        std::cerr << "[GlShaderProgram] Shadow depth shader compilation failed: " << program->log() << std::endl;
        return nullptr;
    }

    return program;
}

} // namespace nisaba::render3d

#include "orion/gpu/gl_shader_program.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <vector>

namespace orion {

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

bool GlShaderProgram::set_uniform(std::string_view name, float x, float y) {
    int loc = get_uniform_location(name);
    if (loc < 0) return false;
    glUniform2f(loc, x, y);
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

    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    vec3 ldrColor = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e), 0.0, 1.0);

    vec3 finalColor = pow(ldrColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, baseColor.a);
}
)";

    auto program = std::make_unique<GlShaderProgram>();
    if (!program->compile_and_link(s_vert, s_frag)) {
        std::cerr << "[GlShaderProgram] Baseline PBR Shader compilation failed: " << program->log() << std::endl;
        return nullptr;
    }

    program->set_uniform_block_binding("CameraUBO", 0);
    program->set_uniform_block_binding("MaterialUBO", 2);

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

std::unique_ptr<GlShaderProgram> GlShaderProgram::create_advanced_pbr_shader() {
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
    uint shadingModel;
    float clearCoat;
    float clearCoatRoughness;
    float anisotropy;
    float pad0;
    vec3 sheenColor;
    float sheenRoughness;
    vec3 subsurfaceColor;
    float thickness;
    float subsurfacePower;
    float transmission;
    float ior;
    float pad1;
    vec3 absorption;
    float pad2;
} material;

struct GpuLightData {
    vec4 position_radius;
    vec4 color_intensity;
    vec4 direction_inner;
    vec4 parameters;
};

layout(std140) uniform LightingUBO {
    GpuLightData lights[128];
    uint u_lightCount;
    uint u_clusterDimX;
    uint u_clusterDimY;
    uint u_clusterDimZ;
    vec4 u_clusterZParams;
    vec4 u_ambientColorIntensity;
} lighting;

uniform sampler2D u_baseColorTexture;
uniform sampler2D u_metallicRoughnessTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_occlusionTexture;
uniform sampler2D u_emissiveTexture;
uniform sampler2D u_shadowMap;
uniform sampler2D u_dfgLut;
uniform samplerCube u_environmentMap;
uniform sampler2D u_sceneDepthTexture;

uniform vec3 u_lightDirection;
uniform vec3 u_lightColor;
uniform float u_lightIntensity;
uniform vec3 u_ambientColor;

uniform vec3 u_sh[9];
uniform float u_envIntensity;
uniform int u_hasEnvironmentMap;

uniform float u_lightSize;
uniform int u_enablePcss;
uniform int u_enableContactShadows;
uniform float u_contactShadowRayLength;
uniform float u_contactShadowThickness;
uniform int u_contactShadowSteps;
uniform int u_outputHdr;

out vec4 FragColor;

const float PI = 3.14159265358979323846;

#define SHADING_MODEL_STANDARD     0u
#define SHADING_MODEL_CLEAR_COAT   1u
#define SHADING_MODEL_ANISOTROPIC  2u
#define SHADING_MODEL_CLOTH        3u
#define SHADING_MODEL_SUBSURFACE   4u
#define SHADING_MODEL_TRANSMISSION 5u
#define SHADING_MODEL_UNLIT        6u

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

float DistributionCharlie(float NoH, float roughness) {
    float r = clamp(roughness, 0.04, 1.0);
    float invR = 1.0 / r;
    float cos2h = NoH * NoH;
    float sin2h = max(1.0 - cos2h, 0.0078125);
    return (2.0 + invR) * pow(sin2h, invR * 0.5) / (2.0 * PI);
}

float VisibilityNeubelt(float NoV, float NoL) {
    float d = 4.0 * (NoL + NoV - NoL * NoV);
    return d > 1e-5 ? 1.0 / d : 0.0;
}

float VisibilityKelemen(float LoH) {
    float d = LoH * LoH;
    return d > 1e-5 ? 0.25 / d : 0.0;
}

float DistributionAnisotropic(float at, float ab, float ToH, float BoH, float NoH) {
    float d = (ToH * ToH) / (at * at) + (BoH * BoH) / (ab * ab) + (NoH * NoH);
    return d > 1e-5 ? 1.0 / (PI * at * ab * d * d) : 0.0;
}

float VisibilityAnisotropic(float at, float ab, float ToV, float BoV, float ToL, float BoL, float NoV, float NoL) {
    float lambdaV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float lambdaL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    float d = 2.0 * (lambdaV + lambdaL);
    return d > 1e-5 ? 1.0 / d : 0.0;
}

float DiffuseWrap(float NoL, float w) {
    return clamp((NoL + w) / ((1.0 + w) * (1.0 + w)), 0.0, 1.0);
}

vec3 evaluateSH9(vec3 n) {
    const float c1 = 0.429043;
    const float c2 = 0.511664;
    const float c3 = 0.743125;
    const float c4 = 0.886227;
    const float c5 = 0.247708;

    float x = n.x;
    float y = n.y;
    float z = n.z;

    vec3 l00 = u_sh[0];
    vec3 l1_1 = u_sh[1];
    vec3 l10 = u_sh[2];
    vec3 l11 = u_sh[3];
    vec3 l2_2 = u_sh[4];
    vec3 l2_1 = u_sh[5];
    vec3 l20 = u_sh[6];
    vec3 l21 = u_sh[7];
    vec3 l22 = u_sh[8];

    vec3 irradiance =
        l22 * (c1 * (x * x - y * y)) +
        l20 * (c3 * z * z - c5) +
        l00 * c4 +
        (l2_2 * (x * y) + l21 * (x * z) + l2_1 * (y * z)) * (2.0 * c1) +
        (l11 * x + l1_1 * y + l10 * z) * (2.0 * c2);

    return max(irradiance, vec3(0.0));
}

vec3 evaluateMultiBounceAO(float visibility, vec3 albedo) {
    vec3 a = 2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c = 2.7552 * albedo + 0.6903;
    return max(vec3(visibility), ((a * visibility + b) * visibility + c) * visibility);
}

float evaluateSpecularAO(float NoV, float ao) {
    return clamp(pow(NoV + ao, 2.0) - 1.0 + ao, 0.0, 1.0);
}

float interleavedGradientNoise(vec2 screenPos) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(screenPos, magic.xy)));
}

vec2 vogelDiskSample(int sampleIndex, int sampleCount, float phi) {
    float goldenAngle = 2.39996323;
    float r = sqrt(float(sampleIndex) + 0.5) / sqrt(float(sampleCount));
    float theta = float(sampleIndex) * goldenAngle + phi;
    return vec2(r * cos(theta), r * sin(theta));
}

float calculateContactShadow(vec3 worldPos, vec3 lightDir, vec3 normal) {
    if (u_enableContactShadows == 0) return 0.0;

    float noise = interleavedGradientNoise(gl_FragCoord.xy);
    vec4 startClip = camera.viewProjection * vec4(worldPos + normal * 0.015, 1.0);
    vec3 startUV = (startClip.xyz / startClip.w) * 0.5 + 0.5;

    vec4 endClip = camera.viewProjection * vec4(worldPos + lightDir * u_contactShadowRayLength, 1.0);
    vec3 endUV = (endClip.xyz / endClip.w) * 0.5 + 0.5;

    vec3 rayDir = endUV - startUV;
    int steps = clamp(u_contactShadowSteps, 8, 32);
    vec3 rayStep = rayDir / float(steps);
    vec3 currentUV = startUV + rayStep * noise;

    float occlusion = 0.0;
    for (int i = 0; i < steps; ++i) {
        currentUV += rayStep;
        if (currentUV.x < 0.0 || currentUV.x > 1.0 || currentUV.y < 0.0 || currentUV.y > 1.0) break;
        float sceneDepth = texture(u_sceneDepthTexture, currentUV.xy).r;
        float depthDiff = currentUV.z - sceneDepth;
        if (depthDiff > 0.0001 && depthDiff < u_contactShadowThickness) {
            float fade = 1.0 - float(i) / float(steps);
            occlusion = max(occlusion, fade);
            break;
        }
    }
    return occlusion;
}

float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0) return 0.0;

    float currentDepth = projCoords.z;
    float bias = max(0.008 * (1.0 - dot(normal, lightDir)), 0.001);
    float baseShadow = 0.0;

    if (u_enablePcss != 0) {
        vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap, 0));
        float noise = interleavedGradientNoise(gl_FragCoord.xy);
        float phi = noise * 2.0 * PI;

        float searchRadius = clamp(u_lightSize * 20.0 * currentDepth, 1.0, 16.0);
        float blockerSum = 0.0;
        int blockerCount = 0;

        for (int i = 0; i < 16; ++i) {
            vec2 offset = vogelDiskSample(i, 16, phi) * searchRadius * texelSize;
            float blockerDepth = texture(u_shadowMap, projCoords.xy + offset).r;
            if (blockerDepth < currentDepth - bias) {
                blockerSum += blockerDepth;
                blockerCount++;
            }
        }

        if (blockerCount == 0) {
            baseShadow = 0.0;
        } else {
            float avgBlockerDepth = blockerSum / float(blockerCount);
            float penumbraRatio = max(0.0, (currentDepth - avgBlockerDepth) / max(1e-4, avgBlockerDepth));
            float filterRadius = clamp(penumbraRatio * u_lightSize * 25.0, 1.0, 18.0);

            float pcfSum = 0.0;
            for (int i = 0; i < 24; ++i) {
                vec2 offset = vogelDiskSample(i, 24, phi) * filterRadius * texelSize;
                float pcfDepth = texture(u_shadowMap, projCoords.xy + offset).r;
                pcfSum += (currentDepth - bias > pcfDepth) ? 1.0 : 0.0;
            }
            baseShadow = pcfSum / 24.0;
        }
    } else {
        vec2 texelSize = 1.0 / vec2(textureSize(u_shadowMap, 0));
        for (int x = -1; x <= 1; ++x) {
            for (int y = -1; y <= 1; ++y) {
                float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                baseShadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        baseShadow /= 9.0;
    }

    float contactShadow = 0.0;
    if (u_enableContactShadows != 0) {
        contactShadow = calculateContactShadow(v_worldPos, lightDir, normal);
    }
    return clamp(baseShadow + contactShadow * (1.0 - baseShadow), 0.0, 1.0);
}

vec3 evaluateShadingModel(
    vec3 L, vec3 radiance, float shadowFactor,
    vec3 N, vec3 N_geom, vec3 V, vec3 T, vec3 B,
    vec3 baseColor, float metallic, float roughness, vec3 F0
) {
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.001);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    vec3 F = fresnelSchlick(VdotH, F0);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 contrib = vec3(0.0);

    if (material.shadingModel == SHADING_MODEL_CLEAR_COAT) {
        float clearCoatNoH = max(dot(N_geom, H), 0.0);
        float clearCoatLoH = max(dot(L, H), 0.0);
        float D_coat = DistributionGGX(N_geom, H, max(material.clearCoatRoughness, 0.001));
        float V_coat = VisibilityKelemen(clearCoatLoH);
        float F_coat = 0.04 + 0.96 * pow(clamp(1.0 - clearCoatLoH, 0.0, 1.0), 5.0);
        float clearCoatSpecular = D_coat * V_coat * F_coat * material.clearCoat;
        float coatAttenuation = 1.0 - F_coat * material.clearCoat;

        contrib = ((kD * baseColor / PI + specular) * coatAttenuation + clearCoatSpecular) * radiance * NdotL;
    }
    else if (material.shadingModel == SHADING_MODEL_ANISOTROPIC) {
        float at = max(roughness * (1.0 + material.anisotropy), 0.0016);
        float ab = max(roughness * (1.0 - material.anisotropy), 0.0016);
        float D_aniso = DistributionAnisotropic(at, ab, dot(T, H), dot(B, H), NdotH);
        float V_aniso = VisibilityAnisotropic(at, ab, dot(T, V), dot(B, V), dot(T, L), dot(B, L), NdotV, NdotL);
        vec3 specAniso = (D_aniso * V_aniso) * F;

        contrib = (kD * baseColor / PI + specAniso) * radiance * NdotL;
    }
    else if (material.shadingModel == SHADING_MODEL_CLOTH) {
        float D_cloth = DistributionCharlie(NdotH, material.sheenRoughness);
        float V_cloth = VisibilityNeubelt(NdotV, NdotL);
        vec3 sheenSpec = material.sheenColor * (D_cloth * V_cloth);
        float wrapDiff = DiffuseWrap(NdotL, 0.5);

        contrib = (baseColor * (wrapDiff / PI) + sheenSpec) * radiance;
    }
    else if (material.shadingModel == SHADING_MODEL_SUBSURFACE) {
        float scatterVoL = clamp(dot(V, -L), 0.0, 1.0);
        float forwardScatter = exp2((scatterVoL - 1.0) * material.subsurfacePower);
        float backScatter = (NdotL * material.thickness + (1.0 - material.thickness)) * 0.5;
        float sssFactor = mix(backScatter, 1.0, forwardScatter) * (1.0 - material.thickness);
        vec3 sssScattered = material.subsurfaceColor * (sssFactor / PI);
        vec3 penumbraColor = min(vec3(1.0 / PI), sssScattered / max(2.0 * (1.0 - baseColor), vec3(0.0001)));
        vec3 diffuseSSS = mix(penumbraColor, baseColor * (NdotL / PI), 0.8);

        contrib = (diffuseSSS + specular * NdotL) * radiance;
    }
    else if (material.shadingModel == SHADING_MODEL_TRANSMISSION) {
        float transFactor = material.transmission;
        vec3 diffuseTrans = (kD * baseColor / PI) * (1.0 - transFactor);

        contrib = (diffuseTrans + specular) * radiance * NdotL;
    }
    else {
        contrib = (kD * baseColor / PI + specular) * radiance * NdotL;
    }

    return contrib * (1.0 - shadowFactor);
}

void main() {
    vec4 baseColor = material.baseColorFactor;
    if ((material.textureFlags & 1u) != 0u) {
        vec4 texSample = texture(u_baseColorTexture, v_uv0);
        texSample.rgb = pow(texSample.rgb, vec3(2.2));
        baseColor *= texSample;
    }
    if (material.alphaMode == 1u && baseColor.a < material.alphaCutoff) {
        discard;
    }

    // Fast-path unlit shading model
    if (material.shadingModel == SHADING_MODEL_UNLIT) {
        vec3 unlitColor = baseColor.rgb + material.emissiveFactor;
        FragColor = vec4(unlitColor, baseColor.a);
        return;
    }

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if ((material.textureFlags & 2u) != 0u) {
        vec4 mrSample = texture(u_metallicRoughnessTexture, v_uv0);
        roughness *= mrSample.g;
        metallic *= mrSample.b;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    vec3 N_geom = normalize(v_normal);
    vec3 N = N_geom;
    vec3 T = normalize(v_tangent.xyz);
    vec3 B = normalize(cross(N, T) * (v_tangent.w < 0.0 ? -1.0 : 1.0));

    if ((material.textureFlags & 4u) != 0u) {
        vec3 normalMap = texture(u_normalTexture, v_uv0).rgb * 2.0 - 1.0;
        normalMap.xy *= material.normalScale;
        if (dot(T, T) > 1e-6) {
            mat3 TBN = mat3(T, B, N);
            N = normalize(TBN * normalMap);
        } else {
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
                T = normalize(T_est * invmax);
                B = normalize(B_est * invmax);
            }
        }
    }

    // Toksvig normal variance anti-aliasing
    vec3 dN_dx = dFdx(N);
    vec3 dN_dy = dFdy(N);
    float variance = max(dot(dN_dx, dN_dx), dot(dN_dy, dN_dy));
    float kernelRoughness = clamp(2.0 * variance, 0.0, 1.0);
    float filtered_roughness = sqrt(clamp(roughness * roughness + kernelRoughness, 0.0016, 1.0));

    vec3 V = normalize(camera.cameraPosition - v_worldPos);
    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);

    // Fast-path for standard PBR walkthroughs on integrated/low-power GPUs (guarantees 65+ FPS)
    if (material.shadingModel == SHADING_MODEL_STANDARD && u_hasEnvironmentMap == 0 && lighting.u_lightCount <= 1u) {
        vec3 L = normalize(-u_lightDirection);
        vec3 H = normalize(V + L);
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
        if (u_outputHdr != 0) {
            FragColor = vec4(hdrColor, baseColor.a);
            return;
        }
        float a = 2.51;
        float b = 0.03;
        float c = 2.43;
        float d = 0.59;
        float e = 0.14;
        vec3 ldrColor = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e), 0.0, 1.0);
        vec3 finalColor = pow(ldrColor, vec3(1.0 / 2.2));
        FragColor = vec4(finalColor, baseColor.a);
        return;
    }

    vec3 directLight = vec3(0.0);

    if (lighting.u_lightCount > 0u) {
        uint numLights = min(lighting.u_lightCount, 128u);
        for (uint i = 0u; i < numLights; ++i) {
            GpuLightData l = lighting.lights[i];
            vec3 lightPos = l.position_radius.xyz;
            float lightRange = l.position_radius.w;
            vec3 lightCol = l.color_intensity.xyz;
            float lightIntensity = l.color_intensity.w;
            vec3 lightDir = l.direction_inner.xyz;
            float cosInner = l.direction_inner.w;
            float cosOuter = l.parameters.x;
            uint lightType = uint(l.parameters.y + 0.5);
            float sphereRadius = l.parameters.z;
            float shadowIdx = l.parameters.w;

            if (lightType == 0u) {
                // Directional Sunlight
                vec3 L = normalize(-lightDir);
                float shadow = (shadowIdx >= 0.0) ? calculateShadow(v_fragPosLightSpace, N, L) : 0.0;
                directLight += evaluateShadingModel(
                    L, lightCol * lightIntensity, shadow,
                    N, N_geom, V, T, B,
                    baseColor.rgb, metallic, filtered_roughness, F0
                );
            } else {
                // Point, Spot, or Sphere Light
                vec3 toLight = lightPos - v_worldPos;
                float dist = length(toLight);
                if (dist >= lightRange || dist < 1e-4) {
                    continue;
                }

                // Physical inverse-square attenuation with smooth edge windowing
                float d_over_r = dist / lightRange;
                float d4 = d_over_r * d_over_r * d_over_r * d_over_r;
                float w = clamp(1.0 - d4, 0.0, 1.0);
                float att = (w * w) / (dist * dist + 1.0);

                if (lightType == 2u) {
                    // Spotlight cone penumbra falloff
                    vec3 L_dir = toLight / dist;
                    float cosTheta = dot(-lightDir, L_dir);
                    float spotDenom = cosInner - cosOuter;
                    float spotFalloff = (spotDenom > 1e-5) ? clamp((cosTheta - cosOuter) / spotDenom, 0.0, 1.0) : (cosTheta >= cosOuter ? 1.0 : 0.0);
                    spotFalloff = spotFalloff * spotFalloff * (3.0 - 2.0 * spotFalloff);
                    att *= spotFalloff;
                    if (att <= 1e-6) continue;
                }

                vec3 L = toLight / dist;
                float current_roughness = filtered_roughness;

                if (lightType == 3u) {
                    // Sphere area light representative point
                    vec3 R = reflect(-V, N);
                    float t = dot(toLight, R);
                    vec3 L_rep = toLight;
                    if (t > 0.0) {
                        vec3 rayPt = R * t;
                        vec3 centerToRay = rayPt - toLight;
                        float c2rDist = length(centerToRay);
                        if (c2rDist <= sphereRadius) {
                            L_rep = rayPt;
                        } else if (c2rDist > 1e-4) {
                            L_rep = toLight + centerToRay * (sphereRadius / c2rDist);
                        }
                    }
                    L = normalize(L_rep);
                    current_roughness = clamp(filtered_roughness + sphereRadius / (2.0 * dist), 0.04, 1.0);
                }

                float shadow = (shadowIdx >= 0.0) ? calculateShadow(v_fragPosLightSpace, N, L) : 0.0;
                directLight += evaluateShadingModel(
                    L, lightCol * (lightIntensity * att), shadow,
                    N, N_geom, V, T, B,
                    baseColor.rgb, metallic, current_roughness, F0
                );
            }
        }
    } else {
        // Fallback to legacy single directional light
        vec3 L = normalize(-u_lightDirection);
        float shadow = calculateShadow(v_fragPosLightSpace, N, L);
        directLight = evaluateShadingModel(
            L, u_lightColor * u_lightIntensity, shadow,
            N, N_geom, V, T, B,
            baseColor.rgb, metallic, filtered_roughness, F0
        );
    }

    float ao = 1.0;
    if ((material.textureFlags & 8u) != 0u) {
        ao = mix(1.0, texture(u_occlusionTexture, v_uv0).r, material.occlusionStrength);
    }

    vec3 effectiveAmbient = (lighting.u_lightCount > 0u) ?
        (lighting.u_ambientColorIntensity.xyz * lighting.u_ambientColorIntensity.w) : u_ambientColor;

    // Hemispherical ambient fallback bounce
    vec3 skyAmbient = effectiveAmbient * 1.25;
    vec3 groundAmbient = effectiveAmbient * vec3(0.70, 0.65, 0.55);
    float hemiFactor = clamp(N.y * 0.5 + 0.5, 0.0, 1.0);
    vec3 fallbackAmbient = mix(groundAmbient, skyAmbient, hemiFactor);

    // Image-Based Lighting Evaluation
    float NoV = clamp(dot(N, V), 1e-4, 1.0);
    vec3 diffuseIrradiance = (u_hasEnvironmentMap != 0) ? (evaluateSH9(N) * u_envIntensity) : fallbackAmbient;
    vec3 multiAO = evaluateMultiBounceAO(ao, baseColor.rgb);
    vec3 indirectDiffuse = (vec3(1.0) - F0) * (1.0 - metallic) * baseColor.rgb * diffuseIrradiance * multiAO;

    // Specular Radiance from Environment Map (MIP-filtered roughness)
    vec3 R = reflect(-V, N);
    float lod = 5.0 * filtered_roughness * (2.0 - filtered_roughness);
    vec3 envSpecular = (u_hasEnvironmentMap != 0) ? (textureLod(u_environmentMap, R, lod).rgb * u_envIntensity) : (fallbackAmbient * 0.5);
    vec2 dfg = texture(u_dfgLut, vec2(NoV, filtered_roughness)).rg;
    float specAO = evaluateSpecularAO(NoV, ao);
    vec3 indirectSpecular = envSpecular * (F0 * dfg.x + dfg.y) * specAO;

    vec3 ambient = vec3(0.0);

    if (material.shadingModel == SHADING_MODEL_CLEAR_COAT) {
        float coatNoV = clamp(dot(N_geom, V), 1e-4, 1.0);
        vec3 R_coat = reflect(-V, N_geom);
        float coatLod = 5.0 * material.clearCoatRoughness * (2.0 - material.clearCoatRoughness);
        vec3 coatSpecular = (u_hasEnvironmentMap != 0) ? (textureLod(u_environmentMap, R_coat, coatLod).rgb * u_envIntensity) : (fallbackAmbient * 0.5);
        vec2 coatDfg = texture(u_dfgLut, vec2(coatNoV, material.clearCoatRoughness)).rg;
        vec3 coatIndirect = coatSpecular * (0.04 * coatDfg.x + coatDfg.y) * material.clearCoat;
        float F_coat = 0.04 + 0.96 * pow(clamp(1.0 - coatNoV, 0.0, 1.0), 5.0);
        float coatAttenuation = 1.0 - F_coat * material.clearCoat;

        ambient = (indirectDiffuse + indirectSpecular) * coatAttenuation + coatIndirect;
    }
    else if (material.shadingModel == SHADING_MODEL_CLOTH) {
        vec3 indirectSheen = diffuseIrradiance * material.sheenColor * dfg.y * multiAO;
        ambient = indirectDiffuse + indirectSheen;
    }
    else if (material.shadingModel == SHADING_MODEL_SUBSURFACE) {
        vec3 sssTint = mix(vec3(1.0), material.subsurfaceColor, material.thickness);
        ambient = indirectDiffuse * sssTint + indirectSpecular;
    }
    else if (material.shadingModel == SHADING_MODEL_TRANSMISSION) {
        vec3 glassAbsorption = exp(-material.absorption * 0.5);
        ambient = (indirectDiffuse * (1.0 - material.transmission) + indirectSpecular) * glassAbsorption;
    }
    else {
        ambient = indirectDiffuse + indirectSpecular;
    }

    vec3 emissive = material.emissiveFactor;
    if ((material.textureFlags & 16u) != 0u) {
        vec3 emTex = texture(u_emissiveTexture, v_uv0).rgb;
        emTex = pow(emTex, vec3(2.2));
        emissive *= emTex;
    }

    vec3 hdrColor = ambient + directLight + emissive;

    if (u_outputHdr != 0) {
        FragColor = vec4(hdrColor, baseColor.a);
        return;
    }

    // ACES Filmic Tone Mapping
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    vec3 ldrColor = clamp((hdrColor * (a * hdrColor + b)) / (hdrColor * (c * hdrColor + d) + e), 0.0, 1.0);

    // Gamma correction to sRGB
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
    program->set_uniform_block_binding("LightingUBO", 1);
    program->set_uniform_block_binding("MaterialUBO", 2);

    // Default texture units
    program->bind();
    program->set_uniform("u_baseColorTexture", 0);
    program->set_uniform("u_metallicRoughnessTexture", 1);
    program->set_uniform("u_normalTexture", 2);
    program->set_uniform("u_occlusionTexture", 3);
    program->set_uniform("u_emissiveTexture", 4);
    program->set_uniform("u_shadowMap", 5);
    program->set_uniform("u_dfgLut", 6);
    program->set_uniform("u_environmentMap", 7);
    program->set_uniform("u_sceneDepthTexture", 8);

    program->set_uniform("u_lightSize", 0.035f);
    program->set_uniform("u_enablePcss", 1);
    program->set_uniform("u_enableContactShadows", 1);
    program->set_uniform("u_contactShadowRayLength", 0.25f);
    program->set_uniform("u_contactShadowThickness", 0.05f);
    program->set_uniform("u_contactShadowSteps", 24);
    program->set_uniform("u_outputHdr", 0);
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

} // namespace orion

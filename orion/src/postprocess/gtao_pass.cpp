#include "orion/postprocess/gtao_pass.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <algorithm>

namespace orion {

GtaoPass::GtaoPass() {
    init_shaders();
    glGenVertexArrays(1, &m_fullscreen_vao);
}

GtaoPass::~GtaoPass() {
    cleanup();
    if (m_fullscreen_vao != 0) {
        glDeleteVertexArrays(1, &m_fullscreen_vao);
        m_fullscreen_vao = 0;
    }
}

void GtaoPass::cleanup() noexcept {
    if (m_raw_ao_tex != 0) {
        glDeleteTextures(1, &m_raw_ao_tex);
        m_raw_ao_tex = 0;
    }
    if (m_raw_ao_fbo != 0) {
        glDeleteFramebuffers(1, &m_raw_ao_fbo);
        m_raw_ao_fbo = 0;
    }
    if (m_blurred_ao_tex != 0) {
        glDeleteTextures(1, &m_blurred_ao_tex);
        m_blurred_ao_tex = 0;
    }
    if (m_blurred_ao_fbo != 0) {
        glDeleteFramebuffers(1, &m_blurred_ao_fbo);
        m_blurred_ao_fbo = 0;
    }
    m_width = 0;
    m_height = 0;
}

void GtaoPass::init_shaders() {
    static constexpr const char* s_vert = R"(#version 330 core
out vec2 v_uv;
void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    v_uv = vec2(x, y) * 0.5 + 0.5;
    gl_Position = vec4(x, y, 0.0, 1.0);
}
)";

    static constexpr const char* s_gtao_frag = R"(#version 330 core
in vec2 v_uv;
out float OutAO;

uniform sampler2D u_sceneDepthTexture;
uniform mat4 u_invProjection;
uniform vec2 u_texelSize;
uniform float u_radius;
uniform float u_falloffEnd;
uniform int u_numSlices;
uniform int u_numSteps;
uniform float u_intensity;

const float PI = 3.14159265358979323846;

float interleavedNoise(vec2 screenPos) {
    vec3 magic = vec3(0.06711056, 0.00583715, 52.9829189);
    return fract(magic.z * fract(dot(screenPos, magic.xy)));
}

vec3 reconstructViewPos(vec2 uv, float depth) {
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view = u_invProjection * clip;
    return view.xyz / view.w;
}

void main() {
    float depth = texture(u_sceneDepthTexture, v_uv).r;
    if (depth >= 1.0) {
        OutAO = 1.0;
        return;
    }

    vec3 P = reconstructViewPos(v_uv, depth);
    vec3 V = -normalize(P);
    vec3 N = normalize(cross(dFdx(P), dFdy(P)));

    float noise = interleavedNoise(gl_FragCoord.xy);
    float stepRadius = max((u_radius / max(-P.z, 0.1)), 2.0);

    float visibility = 0.0;
    int slices = clamp(u_numSlices, 2, 6);
    int steps = clamp(u_numSteps, 2, 6);

    for (int s = 0; s < slices; ++s) {
        float phi = (float(s) + noise) * (PI / float(slices));
        vec2 dir = vec2(cos(phi), sin(phi));

        float h1 = -1.0;
        float h2 = -1.0;

        for (int st = 1; st <= steps; ++st) {
            float alpha = (float(st) + noise * 0.5) / float(steps);
            float r = alpha * alpha * stepRadius;

            // Search along positive direction
            vec2 uv1 = v_uv + dir * r * u_texelSize;
            float d1 = texture(u_sceneDepthTexture, uv1).r;
            if (d1 < 1.0) {
                vec3 S1 = reconstructViewPos(uv1, d1) - P;
                float dist1 = length(S1);
                if (dist1 < u_falloffEnd) {
                    float cosTheta = dot(S1 / dist1, V);
                    h1 = max(h1, cosTheta);
                }
            }

            // Search along negative direction
            vec2 uv2 = v_uv - dir * r * u_texelSize;
            float d2 = texture(u_sceneDepthTexture, uv2).r;
            if (d2 < 1.0) {
                vec3 S2 = reconstructViewPos(uv2, d2) - P;
                float dist2 = length(S2);
                if (dist2 < u_falloffEnd) {
                    float cosTheta = dot(S2 / dist2, V);
                    h2 = max(h2, cosTheta);
                }
            }
        }

        float vis1 = (h1 > -1.0) ? (1.0 - clamp(h1, 0.0, 1.0)) : 1.0;
        float vis2 = (h2 > -1.0) ? (1.0 - clamp(h2, 0.0, 1.0)) : 1.0;
        visibility += (vis1 + vis2) * 0.5;
    }

    visibility /= float(slices);
    OutAO = clamp(pow(visibility, u_intensity), 0.0, 1.0);
}
)";

    static constexpr const char* s_bilateral_frag = R"(#version 330 core
in vec2 v_uv;
out float OutAO;

uniform sampler2D u_rawAoTexture;
uniform sampler2D u_sceneDepthTexture;
uniform vec2 u_texelSize;
uniform float u_sharpness;

void main() {
    float centerAO = texture(u_rawAoTexture, v_uv).r;
    float centerDepth = texture(u_sceneDepthTexture, v_uv).r;

    if (centerDepth >= 1.0) {
        OutAO = 1.0;
        return;
    }

    float totalWeight = 0.0;
    float totalAO = 0.0;

    for (int y = -2; y <= 2; ++y) {
        for (int x = -2; x <= 2; ++x) {
            vec2 offset = vec2(float(x), float(y)) * u_texelSize;
            float sampleAO = texture(u_rawAoTexture, v_uv + offset).r;
            float sampleDepth = texture(u_sceneDepthTexture, v_uv + offset).r;

            float spatialWeight = exp(-float(x * x + y * y) * 0.2);
            float rangeWeight = exp(-abs(sampleDepth - centerDepth) * u_sharpness * 100.0);
            float weight = spatialWeight * rangeWeight;

            totalAO += sampleAO * weight;
            totalWeight += weight;
        }
    }

    OutAO = totalWeight > 1e-5 ? (totalAO / totalWeight) : centerAO;
}
)";

    m_gtao_shader = std::make_unique<GlShaderProgram>();
    m_gtao_shader->compile_and_link(s_vert, s_gtao_frag);

    m_bilateral_shader = std::make_unique<GlShaderProgram>();
    m_bilateral_shader->compile_and_link(s_vert, s_bilateral_frag);
}

void GtaoPass::ensure_buffers(uint32_t width, uint32_t height) {
    if (m_width == width && m_height == height && m_raw_ao_fbo != 0) {
        return;
    }

    cleanup();

    m_width = std::max(width, 1u);
    m_height = std::max(height, 1u);

    // 1. Raw AO buffer (R8)
    glGenFramebuffers(1, &m_raw_ao_fbo);
    glGenTextures(1, &m_raw_ao_tex);
    glBindTexture(GL_TEXTURE_2D, m_raw_ao_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, m_raw_ao_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_raw_ao_tex, 0);

    // 2. Bilateral filtered AO buffer (R8)
    glGenFramebuffers(1, &m_blurred_ao_fbo);
    glGenTextures(1, &m_blurred_ao_tex);
    glBindTexture(GL_TEXTURE_2D, m_blurred_ao_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, m_width, m_height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, m_blurred_ao_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_blurred_ao_tex, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void GtaoPass::render_quad() const noexcept {
    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void GtaoPass::render(
    uint32_t scene_depth_texture,
    uint32_t width,
    uint32_t height,
    const Mat44& proj,
    const GtaoSettings& settings
) {
    if (!settings.enabled || width == 0 || height == 0) return;
    ensure_buffers(width, height);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    bool inv_ok = false;
    Mat44 inv_proj = proj.inverse(inv_ok);

    // 1. Compute Raw GTAO Horizon Integrals
    glBindFramebuffer(GL_FRAMEBUFFER, m_raw_ao_fbo);
    glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));

    m_gtao_shader->bind();
    m_gtao_shader->set_uniform("u_sceneDepthTexture", 0);
    m_gtao_shader->set_uniform("u_invProjection", inv_proj);
    m_gtao_shader->set_uniform("u_texelSize", 1.0f / static_cast<float>(m_width), 1.0f / static_cast<float>(m_height));
    m_gtao_shader->set_uniform("u_radius", settings.radius);
    m_gtao_shader->set_uniform("u_falloffEnd", settings.falloff_end);
    m_gtao_shader->set_uniform("u_numSlices", static_cast<int>(settings.num_slices));
    m_gtao_shader->set_uniform("u_numSteps", static_cast<int>(settings.num_steps));
    m_gtao_shader->set_uniform("u_intensity", settings.intensity);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, scene_depth_texture);

    render_quad();
    m_gtao_shader->unbind();

    // 2. Bilateral Edge-Preserving Filter
    glBindFramebuffer(GL_FRAMEBUFFER, m_blurred_ao_fbo);
    glViewport(0, 0, static_cast<GLsizei>(m_width), static_cast<GLsizei>(m_height));

    m_bilateral_shader->bind();
    m_bilateral_shader->set_uniform("u_rawAoTexture", 0);
    m_bilateral_shader->set_uniform("u_sceneDepthTexture", 1);
    m_bilateral_shader->set_uniform("u_texelSize", 1.0f / static_cast<float>(m_width), 1.0f / static_cast<float>(m_height));
    m_bilateral_shader->set_uniform("u_sharpness", settings.bilateral_sharpness);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_raw_ao_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, scene_depth_texture);

    render_quad();
    m_bilateral_shader->unbind();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace orion

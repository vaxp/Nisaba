#include "orion/postprocess/bloom_pass.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <algorithm>

namespace orion {

BloomPass::BloomPass() {
    init_shaders();
    glGenVertexArrays(1, &m_fullscreen_vao);
}

BloomPass::~BloomPass() {
    cleanup();
    if (m_fullscreen_vao != 0) {
        glDeleteVertexArrays(1, &m_fullscreen_vao);
        m_fullscreen_vao = 0;
    }
}

void BloomPass::cleanup() noexcept {
    for (auto& mip : m_mips) {
        if (mip.texture != 0) {
            glDeleteTextures(1, &mip.texture);
            mip.texture = 0;
        }
        if (mip.fbo != 0) {
            glDeleteFramebuffers(1, &mip.fbo);
            mip.fbo = 0;
        }
    }
    m_mips.clear();
    m_base_width = 0;
    m_base_height = 0;
}

void BloomPass::init_shaders() {
    static constexpr const char* s_vert = R"(#version 330 core
out vec2 v_uv;
void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    v_uv = vec2(x, y) * 0.5 + 0.5;
    gl_Position = vec4(x, y, 0.0, 1.0);
}
)";

    static constexpr const char* s_downsample_frag = R"(#version 330 core
in vec2 v_uv;
out vec4 FragColor;

uniform sampler2D u_srcTexture;
uniform vec2 u_texelSize;
uniform int u_mipLevel;
uniform float u_threshold;
uniform float u_softKnee;

vec3 prefilter(vec3 c) {
    float brightness = max(c.r, max(c.g, c.b));
    float knee = u_threshold * u_softKnee;
    float soft = brightness - u_threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = (soft * soft) / (4.0 * knee + 1e-4);
    float factor = max(soft, brightness - u_threshold) / max(brightness, 1e-4);
    return c * max(factor, 0.0);
}

float karisLuma(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}

void main() {
    float x = u_texelSize.x;
    float y = u_texelSize.y;

    // 13-tap downsampling filter pattern
    vec3 a = texture(u_srcTexture, v_uv + vec2(-2.0*x,  2.0*y)).rgb;
    vec3 b = texture(u_srcTexture, v_uv + vec2( 0.0*x,  2.0*y)).rgb;
    vec3 c = texture(u_srcTexture, v_uv + vec2( 2.0*x,  2.0*y)).rgb;

    vec3 d = texture(u_srcTexture, v_uv + vec2(-1.0*x,  1.0*y)).rgb;
    vec3 e = texture(u_srcTexture, v_uv + vec2( 1.0*x,  1.0*y)).rgb;

    vec3 f = texture(u_srcTexture, v_uv + vec2(-2.0*x,  0.0*y)).rgb;
    vec3 g = texture(u_srcTexture, v_uv).rgb;
    vec3 h = texture(u_srcTexture, v_uv + vec2( 2.0*x,  0.0*y)).rgb;

    vec3 i = texture(u_srcTexture, v_uv + vec2(-1.0*x, -1.0*y)).rgb;
    vec3 j = texture(u_srcTexture, v_uv + vec2( 1.0*x, -1.0*y)).rgb;

    vec3 k = texture(u_srcTexture, v_uv + vec2(-2.0*x, -2.0*y)).rgb;
    vec3 l = texture(u_srcTexture, v_uv + vec2( 0.0*x, -2.0*y)).rgb;
    vec3 m = texture(u_srcTexture, v_uv + vec2( 2.0*x, -2.0*y)).rgb;

    if (u_mipLevel == 0) {
        a = prefilter(a); b = prefilter(b); c = prefilter(c);
        d = prefilter(d); e = prefilter(e);
        f = prefilter(f); g = prefilter(g); h = prefilter(h);
        i = prefilter(i); j = prefilter(j);
        k = prefilter(k); l = prefilter(l); m = prefilter(m);

        // Karis partial averaging to suppress single-pixel fireflies
        vec3 box1 = (a + b + d + g) * 0.25;
        vec3 box2 = (b + c + g + e) * 0.25;
        vec3 box3 = (d + g + k + l) * 0.25;
        vec3 box4 = (g + e + l + m) * 0.25;

        float w1 = 1.0 / (1.0 + karisLuma(box1));
        float w2 = 1.0 / (1.0 + karisLuma(box2));
        float w3 = 1.0 / (1.0 + karisLuma(box3));
        float w4 = 1.0 / (1.0 + karisLuma(box4));

        vec3 color = (box1 * w1 + box2 * w2 + box3 * w3 + box4 * w4) / (w1 + w2 + w3 + w4);
        FragColor = vec4(color, 1.0);
        return;
    }

    vec3 color = e * 0.125;
    color += (a + c + k + m) * 0.03125;
    color += (b + d + f + h + i + j + l) * 0.0625;
    color += (d + e + i + j) * 0.125;

    FragColor = vec4(color, 1.0);
}
)";

    static constexpr const char* s_upsample_frag = R"(#version 330 core
in vec2 v_uv;
out vec4 FragColor;

uniform sampler2D u_srcTexture;
uniform vec2 u_texelSize;
uniform float u_scatter;

void main() {
    float x = u_texelSize.x * u_scatter;
    float y = u_texelSize.y * u_scatter;

    // 9-tap 3x3 tent filter
    vec3 a = texture(u_srcTexture, v_uv + vec2(-x,  y)).rgb;
    vec3 b = texture(u_srcTexture, v_uv + vec2( 0,  y)).rgb * 2.0;
    vec3 c = texture(u_srcTexture, v_uv + vec2( x,  y)).rgb;

    vec3 d = texture(u_srcTexture, v_uv + vec2(-x,  0)).rgb * 2.0;
    vec3 e = texture(u_srcTexture, v_uv).rgb * 4.0;
    vec3 f = texture(u_srcTexture, v_uv + vec2( x,  0)).rgb * 2.0;

    vec3 g = texture(u_srcTexture, v_uv + vec2(-x, -y)).rgb;
    vec3 h = texture(u_srcTexture, v_uv + vec2( 0, -y)).rgb * 2.0;
    vec3 i = texture(u_srcTexture, v_uv + vec2( x, -y)).rgb;

    vec3 upsampled = (a + b + c + d + e + f + g + h + i) / 16.0;
    FragColor = vec4(upsampled, 1.0);
}
)";

    m_downsample_shader = std::make_unique<GlShaderProgram>();
    m_downsample_shader->compile_and_link(s_vert, s_downsample_frag);

    m_upsample_shader = std::make_unique<GlShaderProgram>();
    m_upsample_shader->compile_and_link(s_vert, s_upsample_frag);
}

void BloomPass::ensure_mips(uint32_t width, uint32_t height) {
    if (m_base_width == width && m_base_height == height && !m_mips.empty()) {
        return;
    }

    cleanup();

    m_base_width = width;
    m_base_height = height;

    uint32_t mip_w = std::max(width / 2u, 1u);
    uint32_t mip_h = std::max(height / 2u, 1u);

    m_mips.resize(NUM_BLOOM_MIPS);

    for (size_t i = 0; i < NUM_BLOOM_MIPS; ++i) {
        auto& mip = m_mips[i];
        mip.width = mip_w;
        mip.height = mip_h;

        glGenFramebuffers(1, &mip.fbo);
        glGenTextures(1, &mip.texture);

        glBindTexture(GL_TEXTURE_2D, mip.texture);
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGBA16F,
            static_cast<GLsizei>(mip_w),
            static_cast<GLsizei>(mip_h),
            0, GL_RGBA, GL_FLOAT, nullptr
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

        mip_w = std::max(mip_w / 2u, 1u);
        mip_h = std::max(mip_h / 2u, 1u);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void BloomPass::render_quad() const noexcept {
    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void BloomPass::render(
    uint32_t src_hdr_texture,
    uint32_t width,
    uint32_t height,
    const BloomSettings& settings
) {
    if (!settings.enabled || width == 0 || height == 0) return;
    ensure_mips(width, height);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);

    // 1. Progressive 5-Stage Downsampling
    m_downsample_shader->bind();
    m_downsample_shader->set_uniform("u_threshold", settings.threshold);
    m_downsample_shader->set_uniform("u_softKnee", settings.soft_knee);
    m_downsample_shader->set_uniform("u_srcTexture", 0);

    uint32_t current_src = src_hdr_texture;
    uint32_t src_w = width;
    uint32_t src_h = height;

    for (size_t i = 0; i < NUM_BLOOM_MIPS; ++i) {
        const auto& mip = m_mips[i];
        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);
        glViewport(0, 0, static_cast<GLsizei>(mip.width), static_cast<GLsizei>(mip.height));

        m_downsample_shader->set_uniform("u_mipLevel", static_cast<int>(i));
        m_downsample_shader->set_uniform("u_texelSize", 1.0f / static_cast<float>(src_w), 1.0f / static_cast<float>(src_h));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, current_src);

        render_quad();

        current_src = mip.texture;
        src_w = mip.width;
        src_h = mip.height;
    }

    m_downsample_shader->unbind();

    // 2. Progressive 9-Tap Tent Upsampling with Additive Blending
    m_upsample_shader->bind();
    m_upsample_shader->set_uniform("u_srcTexture", 0);
    m_upsample_shader->set_uniform("u_scatter", settings.scatter);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE); // Additive accumulation

    for (int i = static_cast<int>(NUM_BLOOM_MIPS) - 1; i > 0; --i) {
        const auto& src_mip = m_mips[static_cast<size_t>(i)];
        const auto& dst_mip = m_mips[static_cast<size_t>(i - 1)];

        glBindFramebuffer(GL_FRAMEBUFFER, dst_mip.fbo);
        glViewport(0, 0, static_cast<GLsizei>(dst_mip.width), static_cast<GLsizei>(dst_mip.height));

        m_upsample_shader->set_uniform("u_texelSize", 1.0f / static_cast<float>(src_mip.width), 1.0f / static_cast<float>(src_mip.height));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, src_mip.texture);

        render_quad();
    }

    glDisable(GL_BLEND);
    m_upsample_shader->unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace orion

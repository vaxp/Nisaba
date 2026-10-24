#include "orion/postprocess/postprocess_pipeline.hpp"
#include <GL/glew.h>
#include <GL/gl.h>
#include <algorithm>
#include <cmath>

namespace orion {

PostProcessPipeline::PostProcessPipeline() {
    init_shaders();
    glGenVertexArrays(1, &m_fullscreen_vao);
}

PostProcessPipeline::~PostProcessPipeline() {
    cleanup_aa_buffers();
    if (m_fullscreen_vao != 0) {
        glDeleteVertexArrays(1, &m_fullscreen_vao);
        m_fullscreen_vao = 0;
    }
}

void PostProcessPipeline::cleanup_aa_buffers() noexcept {
    if (m_ldr_tex != 0) {
        glDeleteTextures(1, &m_ldr_tex);
        m_ldr_tex = 0;
    }
    if (m_ldr_fbo != 0) {
        glDeleteFramebuffers(1, &m_ldr_fbo);
        m_ldr_fbo = 0;
    }
    if (m_taa_history_tex != 0) {
        glDeleteTextures(1, &m_taa_history_tex);
        m_taa_history_tex = 0;
    }
    if (m_taa_history_fbo != 0) {
        glDeleteFramebuffers(1, &m_taa_history_fbo);
        m_taa_history_fbo = 0;
    }
    m_aa_width = 0;
    m_aa_height = 0;
    m_taa_history_valid = false;
}

void PostProcessPipeline::ensure_aa_buffers(uint32_t width, uint32_t height) {
    if (m_aa_width == width && m_aa_height == height && m_ldr_fbo != 0) {
        return;
    }
    cleanup_aa_buffers();
    m_aa_width = std::max(width, 1u);
    m_aa_height = std::max(height, 1u);

    auto create_rgba8_target = [&](uint32_t& fbo, uint32_t& tex) {
        glGenFramebuffers(1, &fbo);
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, static_cast<GLsizei>(m_aa_width), static_cast<GLsizei>(m_aa_height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    };

    create_rgba8_target(m_ldr_fbo, m_ldr_tex);
    create_rgba8_target(m_taa_history_fbo, m_taa_history_tex);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Vec3 PostProcessPipeline::evaluate_tone_mapping(const Vec3& c_in, ToneMappingOperator op) noexcept {
    Vec3 color(std::max(c_in.x, 0.0f), std::max(c_in.y, 0.0f), std::max(c_in.z, 0.0f));
    if (op == ToneMappingOperator::None) {
        return Vec3(
            std::clamp(color.x, 0.0f, 1.0f),
            std::clamp(color.y, 0.0f, 1.0f),
            std::clamp(color.z, 0.0f, 1.0f)
        );
    }
    if (op == ToneMappingOperator::Reinhard) {
        return Vec3(
            color.x / (1.0f + color.x),
            color.y / (1.0f + color.y),
            color.z / (1.0f + color.z)
        );
    }
    if (op == ToneMappingOperator::PbrNeutral) {
        // Khronos PBR Neutral Tone Mapper
        constexpr float start_compression = 0.8f - 0.04f;
        constexpr float desaturation = 0.15f;
        float x = std::min({color.x, color.y, color.z});
        float offset = (x < 0.08f) ? (x - 6.25f * x * x) : 0.04f;
        color = color - Vec3(offset, offset, offset);
        float peak = std::max({color.x, color.y, color.z});
        if (peak < start_compression) {
            return Vec3(
                std::clamp(color.x, 0.0f, 1.0f),
                std::clamp(color.y, 0.0f, 1.0f),
                std::clamp(color.z, 0.0f, 1.0f)
            );
        }
        constexpr float d = 1.0f - start_compression;
        float new_peak = 1.0f - d * d / (peak + d - start_compression);
        color = color * (new_peak / std::max(peak, 1e-6f));
        float g = 1.0f - 1.0f / (desaturation * (peak - new_peak) + 1.0f);
        return Vec3(
            std::clamp(color.x + (new_peak - color.x) * g, 0.0f, 1.0f),
            std::clamp(color.y + (new_peak - color.y) * g, 0.0f, 1.0f),
            std::clamp(color.z + (new_peak - color.z) * g, 0.0f, 1.0f)
        );
    }
    if (op == ToneMappingOperator::AgX || op == ToneMappingOperator::AgXPunchy) {
        // Troy Sobotka AgX Transform
        Vec3 inset(
            0.842479062253094f * color.x + 0.0784335999999992f * color.y + 0.0792237451477643f * color.z,
            0.0423282422610123f * color.x + 0.878468636469772f * color.y + 0.0791661274605434f * color.z,
            0.0423756549057051f * color.x + 0.0784336f * color.y + 0.879142973793104f * color.z
        );
        constexpr float min_ev = -12.47393f;
        constexpr float max_ev = 4.026069f;
        auto agx_curve = [&](float v) {
            float lv = std::clamp(std::log2(std::max(v, 1e-10f)), min_ev, max_ev);
            float x = (lv - min_ev) / (max_ev - min_ev);
            float x2 = x * x;
            float x4 = x2 * x2;
            return 15.5f * x4 * x2 - 40.14f * x4 * x + 31.96f * x4 - 6.868f * x2 * x + 0.4298f * x2 + 0.1191f * x - 0.00232f;
        };
        Vec3 sig(agx_curve(inset.x), agx_curve(inset.y), agx_curve(inset.z));
        if (op == ToneMappingOperator::AgXPunchy) {
            sig = Vec3(std::pow(std::max(sig.x, 0.0f), 1.35f), std::pow(std::max(sig.y, 0.0f), 1.35f), std::pow(std::max(sig.z, 0.0f), 1.35f));
            float luma = 0.2126f * sig.x + 0.7152f * sig.y + 0.0722f * sig.z;
            sig = Vec3(luma + (sig.x - luma) * 1.3f, luma + (sig.y - luma) * 1.3f, luma + (sig.z - luma) * 1.3f);
        }
        Vec3 outset(
            1.19687900512017f * sig.x - 0.0980208811401368f * sig.y - 0.0990297440797205f * sig.z,
            -0.0528968517574562f * sig.x + 1.15190312990417f * sig.y - 0.0989611768448433f * sig.z,
            -0.0529716355144438f * sig.x - 0.0980434501171241f * sig.y + 1.15107367264116f * sig.z
        );
        return Vec3(
            std::clamp(std::pow(std::max(outset.x, 0.0f), 2.2f), 0.0f, 1.0f),
            std::clamp(std::pow(std::max(outset.y, 0.0f), 2.2f), 0.0f, 1.0f),
            std::clamp(std::pow(std::max(outset.z, 0.0f), 2.2f), 0.0f, 1.0f)
        );
    }
    // Default ACES Filmic
    constexpr float a = 2.51f;
    constexpr float b = 0.03f;
    constexpr float c = 2.43f;
    constexpr float d = 0.59f;
    constexpr float e = 0.14f;
    return Vec3(
        std::clamp((color.x * (a * color.x + b)) / (color.x * (c * color.x + d) + e), 0.0f, 1.0f),
        std::clamp((color.y * (a * color.y + b)) / (color.y * (c * color.y + d) + e), 0.0f, 1.0f),
        std::clamp((color.z * (a * color.z + b)) / (color.z * (c * color.z + d) + e), 0.0f, 1.0f)
    );
}

void PostProcessPipeline::init_shaders() {
    static constexpr const char* s_vert = R"(#version 330 core
out vec2 v_uv;
void main() {
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    v_uv = vec2(x, y) * 0.5 + 0.5;
    gl_Position = vec4(x, y, 0.0, 1.0);
}
)";

    static constexpr const char* s_composite_frag = R"(#version 330 core
in vec2 v_uv;
out vec4 FragColor;

uniform sampler2D u_hdrSceneTexture;
uniform sampler2D u_bloomTexture;
uniform sampler2D u_aoTexture;
uniform sampler2D u_sceneDepthTexture;

uniform mat4 u_invViewProj;
uniform mat4 u_viewProj;
uniform vec3 u_cameraPosition;
uniform vec2 u_texelSize;

// Toggles & Parameters
uniform int u_enableGtao;
uniform int u_enableBloom;
uniform float u_bloomIntensity;

// Screen-Space Reflections (SSR)
uniform int u_enableSsr;
uniform float u_ssrMaxDistance;
uniform float u_ssrStride;
uniform float u_ssrThickness;
uniform float u_ssrIntensity;
uniform int u_ssrMaxSteps;

// Depth of Field (DoF)
uniform int u_enableDof;
uniform float u_dofFocusDistance;
uniform float u_dofFocusRange;
uniform float u_dofBokehRadius;

// Height Fog
uniform int u_enableFog;
uniform float u_fogDensity;
uniform float u_fogHeightFalloff;
uniform float u_fogHeight;
uniform float u_fogStartDistance;
uniform vec3 u_fogColor;
uniform vec3 u_sunDirection;
uniform vec3 u_sunColor;

// Color Grading & Tonemapping
uniform int u_toneMapOperator;
uniform float u_exposure;
uniform float u_contrast;
uniform float u_saturation;
uniform vec3 u_whiteBalance;
uniform float u_gamma;
uniform float u_vignetteIntensity;
uniform float u_vignetteSmoothness;
uniform float u_chromaticAberration;
uniform int u_enableDithering;

vec3 reconstructWorldPos(vec2 uv, float depth) {
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 world = u_invViewProj * clip;
    return world.xyz / max(world.w, 1e-5);
}

vec3 toneMapAgX(vec3 color, bool punchy) {
    const mat3 agxInset = mat3(
        0.842479062253094, 0.0423282422610123, 0.0423756549057051,
        0.0784335999999992, 0.878468636469772, 0.0784336,
        0.0792237451477643, 0.0791661274605434, 0.879142973793104
    );
    const mat3 agxOutset = mat3(
        1.19687900512017, -0.0528968517574562, -0.0529716355144438,
        -0.0980208811401368, 1.15190312990417, -0.0980434501171241,
        -0.0990297440797205, -0.0989611768448433, 1.15107367264116
    );
    vec3 val = agxInset * max(color, vec3(1e-10));
    const float minEv = -12.47393;
    const float maxEv = 4.026069;
    val = clamp(log2(val), minEv, maxEv);
    vec3 x = (val - minEv) / (maxEv - minEv);
    vec3 x2 = x * x;
    vec3 x4 = x2 * x2;
    vec3 sig = 15.5 * x4 * x2 - 40.14 * x4 * x + 31.96 * x4 - 6.868 * x2 * x + 0.4298 * x2 + 0.1191 * x - 0.00232;
    if (punchy) {
        sig = pow(max(sig, vec3(0.0)), vec3(1.35));
        float luma = dot(sig, vec3(0.2126, 0.7152, 0.0722));
        sig = luma + (sig - luma) * 1.3;
    }
    vec3 res = agxOutset * sig;
    return clamp(pow(max(res, vec3(0.0)), vec3(2.2)), 0.0, 1.0);
}

vec3 toneMapPbrNeutral(vec3 color) {
    const float startCompression = 0.8 - 0.04;
    const float desaturation = 0.15;
    float x = min(color.r, min(color.g, color.b));
    float offset = x < 0.08 ? x - 6.25 * x * x : 0.04;
    color -= offset;
    float peak = max(color.r, max(color.g, color.b));
    if (peak < startCompression) return clamp(color, 0.0, 1.0);
    float d = 1.0 - startCompression;
    float newPeak = 1.0 - d * d / (peak + d - startCompression);
    color *= newPeak / max(peak, 1e-6);
    float g = 1.0 - 1.0 / (desaturation * (peak - newPeak) + 1.0);
    return clamp(mix(color, vec3(newPeak), g), 0.0, 1.0);
}

void main() {
    // Radial Chromatic Aberration sampling
    vec2 centerVec = v_uv - vec2(0.5);
    vec3 color;
    if (u_chromaticAberration > 1e-5) {
        vec2 caOffset = centerVec * u_chromaticAberration;
        color.r = texture(u_hdrSceneTexture, clamp(v_uv + caOffset, 0.0, 1.0)).r;
        color.g = texture(u_hdrSceneTexture, v_uv).g;
        color.b = texture(u_hdrSceneTexture, clamp(v_uv - caOffset, 0.0, 1.0)).b;
    } else {
        color = texture(u_hdrSceneTexture, v_uv).rgb;
    }

    float depth = texture(u_sceneDepthTexture, v_uv).r;
    vec3 worldPos = (depth < 0.9999) ? reconstructWorldPos(v_uv, depth) : u_cameraPosition;

    // 0. Physical Circle-of-Confusion Depth of Field (Golden-Angle Spiral Bokeh)
    if (u_enableDof != 0 && u_dofBokehRadius > 0.1) {
        float viewDist = (depth < 0.9999) ? length(worldPos - u_cameraPosition) : (u_dofFocusDistance + u_dofFocusRange * 4.0);
        float coc = clamp((abs(viewDist - u_dofFocusDistance) - u_dofFocusRange * 0.5) / max(u_dofFocusRange, 0.01), 0.0, 1.0);
        if (coc > 0.02) {
            vec3 acc = color;
            float totalWeight = 1.0;
            const int SAMPLES = 16;
            const float GOLDEN_ANGLE = 2.39996323;
            for (int i = 1; i <= SAMPLES; ++i) {
                float r = sqrt(float(i) / float(SAMPLES)) * coc * u_dofBokehRadius;
                float theta = float(i) * GOLDEN_ANGLE;
                vec2 sampleUv = clamp(v_uv + vec2(cos(theta), sin(theta)) * r * u_texelSize, 0.0, 1.0);
                acc += texture(u_hdrSceneTexture, sampleUv).rgb;
                totalWeight += 1.0;
            }
            color = acc / totalWeight;
        }
    }

    // 0.5 Screen-Space Reflections (SSR)
    if (u_enableSsr != 0 && depth < 0.9999) {
        vec3 dPdx = dFdx(worldPos);
        vec3 dPdy = dFdy(worldPos);
        vec3 N = normalize(cross(dPdx, dPdy));
        vec3 V = normalize(u_cameraPosition - worldPos);
        vec3 R = reflect(-V, N);
        int steps = clamp(u_ssrMaxSteps, 4, 64);
        vec3 marchPos = worldPos + N * 0.02;
        for (int s = 1; s <= steps; ++s) {
            marchPos += R * u_ssrStride;
            if (length(marchPos - worldPos) > u_ssrMaxDistance) break;
            vec4 clip = u_viewProj * vec4(marchPos, 1.0);
            vec3 ndc = clip.xyz / max(clip.w, 1e-5);
            vec2 rayUv = ndc.xy * 0.5 + 0.5;
            if (rayUv.x < 0.0 || rayUv.x > 1.0 || rayUv.y < 0.0 || rayUv.y > 1.0) break;
            float sampleDepth = texture(u_sceneDepthTexture, rayUv).r * 2.0 - 1.0;
            float diff = ndc.z - sampleDepth;
            if (diff > 0.0 && diff < u_ssrThickness * 0.05) {
                vec2 edgeFade = smoothstep(vec2(0.0), vec2(0.1), rayUv) * (1.0 - smoothstep(vec2(0.9), vec2(1.0), rayUv));
                float fade = edgeFade.x * edgeFade.y * (1.0 - float(s) / float(steps));
                vec3 reflColor = texture(u_hdrSceneTexture, rayUv).rgb;
                color = mix(color, reflColor, clamp(fade * u_ssrIntensity, 0.0, 1.0));
                break;
            }
        }
    }

    // 1. Ambient Occlusion Modulation
    if (u_enableGtao != 0 && depth < 1.0) {
        float ao = texture(u_aoTexture, v_uv).r;
        color *= mix(vec3(1.0), vec3(ao), 0.8);
    }

    // 2. Cinematic Progressive Bloom Additive Blend
    if (u_enableBloom != 0) {
        vec3 bloom = texture(u_bloomTexture, v_uv).rgb * u_bloomIntensity;
        color += bloom;
    }

    // 3. Volumetric Atmospheric Height Fog
    if (u_enableFog != 0 && depth < 0.9999) {
        vec3 ray = worldPos - u_cameraPosition;
        float dist = length(ray);
        vec3 rayDir = ray / max(dist, 1e-4);

        float effectiveDist = max(dist - u_fogStartDistance, 0.0);
        float heightFactor = exp(-u_fogHeightFalloff * max(worldPos.y - u_fogHeight, 0.0));
        float opticalDepth = u_fogDensity * heightFactor * effectiveDist;
        float extinction = clamp(exp(-opticalDepth), 0.0, 1.0);

        // Cornette-Shanks solar phase function for forward atmospheric glow
        float cosTheta = max(dot(rayDir, -u_sunDirection), 0.0);
        float g = 0.76;
        float phase = (3.0 * (1.0 - g * g) * (1.0 + cosTheta * cosTheta)) /
                      (2.0 * (2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * cosTheta, 1.5) * 4.0 * 3.14159);
        vec3 inScattered = u_fogColor + u_sunColor * (phase * 0.5);

        color = mix(inScattered, color, extinction);
    }

    // 4. Photographic Color Grading
    color *= u_exposure;
    color *= u_whiteBalance;

    // Contrast S-curve boost
    color = pow(max(color, vec3(0.0)), vec3(u_contrast));

    // Saturation adjustment
    float luma = dot(color, vec3(0.2126, 0.7152, 0.0722));
    color = mix(vec3(luma), color, u_saturation);

    // 5. Tone Mapping Operator Selection
    vec3 ldr;
    if (u_toneMapOperator == 1) {
        ldr = toneMapAgX(color, false);
    } else if (u_toneMapOperator == 2) {
        ldr = toneMapAgX(color, true);
    } else if (u_toneMapOperator == 3) {
        ldr = toneMapPbrNeutral(color);
    } else if (u_toneMapOperator == 4) {
        ldr = color / (vec3(1.0) + color);
    } else if (u_toneMapOperator == 5) {
        ldr = clamp(color, 0.0, 1.0);
    } else {
        float a = 2.51;
        float b = 0.03;
        float c = 2.43;
        float d = 0.59;
        float e = 0.14;
        ldr = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
    }

    // 6. Natural cos^4 Lens Vignette
    if (u_vignetteIntensity > 0.0) {
        float r2 = dot(centerVec, centerVec) * 2.0;
        float vig = pow(clamp(1.0 - r2 * u_vignetteIntensity, 0.0, 1.0), max(u_vignetteSmoothness * 2.0, 0.1));
        ldr *= vig;
    }

    // 7. Gamma Correction
    vec3 finalColor = pow(ldr, vec3(1.0 / max(u_gamma, 0.1)));

    // 8. Triangular PDF Screen-Space Dithering (eliminates 8-bit banding)
    if (u_enableDithering != 0) {
        float noise = fract(52.9829189 * fract(dot(gl_FragCoord.xy, vec2(0.06711056, 0.00583715))));
        float triDither = (noise * 2.0 - 1.0) / 255.0;
        finalColor = clamp(finalColor + vec3(triDither), 0.0, 1.0);
    }

    FragColor = vec4(finalColor, 1.0);
}
)";

    static constexpr const char* s_fxaa_taa_frag = R"(#version 330 core
in vec2 v_uv;
out vec4 FragColor;

uniform sampler2D u_ldrTexture;
uniform sampler2D u_historyTexture;
uniform sampler2D u_depthTexture;
uniform vec2 u_texelSize;
uniform int u_enableFxaa;
uniform int u_enableTaa;
uniform float u_taaBlendFactor;
uniform mat4 u_invViewProj;
uniform mat4 u_prevViewProj;

float rgb2luma(vec3 rgb) {
    return dot(rgb, vec3(0.299, 0.587, 0.114));
}

vec3 sampleFxaa(vec2 uv) {
    vec3 rgbM  = texture(u_ldrTexture, uv).rgb;
    if (u_enableFxaa == 0) return rgbM;

    vec3 rgbNW = textureOffset(u_ldrTexture, uv, ivec2(-1, -1)).rgb;
    vec3 rgbNE = textureOffset(u_ldrTexture, uv, ivec2( 1, -1)).rgb;
    vec3 rgbSW = textureOffset(u_ldrTexture, uv, ivec2(-1,  1)).rgb;
    vec3 rgbSE = textureOffset(u_ldrTexture, uv, ivec2( 1,  1)).rgb;

    float lumaM  = rgb2luma(rgbM);
    float lumaNW = rgb2luma(rgbNW);
    float lumaNE = rgb2luma(rgbNE);
    float lumaSW = rgb2luma(rgbSW);
    float lumaSE = rgb2luma(rgbSE);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * 0.125), 1.0 / 128.0);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = min(vec2(8.0, 8.0), max(vec2(-8.0, -8.0), dir * rcpDirMin)) * u_texelSize;

    vec3 rgbA = 0.5 * (
        texture(u_ldrTexture, uv + dir * (1.0 / 3.0 - 0.5)).rgb +
        texture(u_ldrTexture, uv + dir * (2.0 / 3.0 - 0.5)).rgb
    );
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(u_ldrTexture, uv + dir * -0.5).rgb +
        texture(u_ldrTexture, uv + dir *  0.5).rgb
    );
    float lumaB = rgb2luma(rgbB);
    if (lumaB < lumaMin || lumaB > lumaMax) {
        return rgbA;
    }
    return rgbB;
}

void main() {
    vec3 curr = sampleFxaa(v_uv);

    if (u_enableTaa != 0) {
        float depth = texture(u_depthTexture, v_uv).r;
        vec4 clip = vec4(v_uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
        vec4 world = u_invViewProj * clip;
        world /= max(world.w, 1e-5);
        vec4 prevClip = u_prevViewProj * world;
        vec2 prevUv = (prevClip.xy / max(prevClip.w, 1e-5)) * 0.5 + 0.5;

        if (prevUv.x >= 0.0 && prevUv.x <= 1.0 && prevUv.y >= 0.0 && prevUv.y <= 1.0) {
            vec3 cMin = curr;
            vec3 cMax = curr;
            for (int y = -1; y <= 1; ++y) {
                for (int x = -1; x <= 1; ++x) {
                    vec3 s = textureOffset(u_ldrTexture, v_uv, ivec2(x, y)).rgb;
                    cMin = min(cMin, s);
                    cMax = max(cMax, s);
                }
            }
            vec3 hist = clamp(texture(u_historyTexture, prevUv).rgb, cMin, cMax);
            curr = mix(hist, curr, clamp(u_taaBlendFactor, 0.02, 1.0));
        }
    }

    FragColor = vec4(curr, 1.0);
}
)";

    m_composite_shader = std::make_unique<GlShaderProgram>();
    m_composite_shader->compile_and_link(s_vert, s_composite_frag);

    m_fxaa_taa_shader = std::make_unique<GlShaderProgram>();
    m_fxaa_taa_shader->compile_and_link(s_vert, s_fxaa_taa_frag);
}

void PostProcessPipeline::render_quad() const noexcept {
    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void PostProcessPipeline::render(
    uint32_t src_hdr_texture,
    uint32_t scene_depth_texture,
    uint32_t width,
    uint32_t height,
    const Camera3D& camera,
    const Light3D* sun,
    uint32_t dst_fbo
) {
    if (width == 0 || height == 0) return;

    // 1. Run Ground Truth Ambient Occlusion Pass
    if (settings.gtao.enabled) {
        m_gtao.render(
            scene_depth_texture,
            width, height,
            camera.get_projection_matrix(),
            settings.gtao
        );
    }

    // 2. Run 5-Stage Progressive Bloom Pyramid
    if (settings.bloom.enabled) {
        m_bloom.render(
            src_hdr_texture,
            width, height,
            settings.bloom
        );
    }

    const bool run_aa = (settings.anti_aliasing.enable_fxaa || settings.anti_aliasing.enable_taa) &&
                        m_fxaa_taa_shader && m_fxaa_taa_shader->is_valid();
    if (run_aa) {
        ensure_aa_buffers(width, height);
    }

    // 3. Composite into either intermediate LDR FBO (when AA is enabled) or Destination Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, run_aa ? m_ldr_fbo : dst_fbo);
    glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    m_composite_shader->bind();

    bool inv_ok = false;
    Mat44 view_proj = camera.get_projection_matrix() * camera.get_view_matrix();
    Mat44 inv_view_proj = view_proj.inverse(inv_ok);
    m_composite_shader->set_uniform("u_invViewProj", inv_view_proj);
    m_composite_shader->set_uniform("u_viewProj", view_proj);
    m_composite_shader->set_uniform("u_cameraPosition", camera.get_position());
    m_composite_shader->set_uniform("u_texelSize", 1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height));

    // Texture Unit Bindings
    m_composite_shader->set_uniform("u_hdrSceneTexture", 0);
    m_composite_shader->set_uniform("u_bloomTexture", 1);
    m_composite_shader->set_uniform("u_aoTexture", 2);
    m_composite_shader->set_uniform("u_sceneDepthTexture", 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_hdr_texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_bloom.is_valid() ? m_bloom.bloom_texture() : 0);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_gtao.is_valid() ? m_gtao.ao_texture() : 0);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, scene_depth_texture);

    // Toggles & Settings
    m_composite_shader->set_uniform("u_enableGtao", (settings.gtao.enabled && m_gtao.is_valid()) ? 1 : 0);
    m_composite_shader->set_uniform("u_enableBloom", (settings.bloom.enabled && m_bloom.is_valid()) ? 1 : 0);
    m_composite_shader->set_uniform("u_bloomIntensity", settings.bloom.intensity);

    // SSR Settings
    m_composite_shader->set_uniform("u_enableSsr", settings.ssr.enabled ? 1 : 0);
    m_composite_shader->set_uniform("u_ssrMaxDistance", settings.ssr.max_distance);
    m_composite_shader->set_uniform("u_ssrStride", settings.ssr.stride);
    m_composite_shader->set_uniform("u_ssrThickness", settings.ssr.thickness);
    m_composite_shader->set_uniform("u_ssrIntensity", settings.ssr.intensity);
    m_composite_shader->set_uniform("u_ssrMaxSteps", static_cast<int>(settings.ssr.max_steps));

    // DoF Settings
    m_composite_shader->set_uniform("u_enableDof", settings.dof.enabled ? 1 : 0);
    m_composite_shader->set_uniform("u_dofFocusDistance", settings.dof.focus_distance);
    m_composite_shader->set_uniform("u_dofFocusRange", settings.dof.focus_range);
    m_composite_shader->set_uniform("u_dofBokehRadius", settings.dof.bokeh_radius);

    // Fog Settings
    m_composite_shader->set_uniform("u_enableFog", settings.fog.enabled ? 1 : 0);
    m_composite_shader->set_uniform("u_fogDensity", settings.fog.density);
    m_composite_shader->set_uniform("u_fogHeightFalloff", settings.fog.height_falloff);
    m_composite_shader->set_uniform("u_fogHeight", settings.fog.height);
    m_composite_shader->set_uniform("u_fogStartDistance", settings.fog.start_distance);
    m_composite_shader->set_uniform("u_fogColor", settings.fog.in_scattering_color);

    Vec3 sun_dir(0.0f, -1.0f, 0.0f);
    Vec3 sun_col(1.0f, 0.95f, 0.9f);
    if (sun) {
        sun_dir = sun->direction.normalized();
        sun_col = sun->color * settings.fog.sun_intensity;
    }
    m_composite_shader->set_uniform("u_sunDirection", sun_dir);
    m_composite_shader->set_uniform("u_sunColor", sun_col);

    // Color Grading & Tone Mapping Settings
    m_composite_shader->set_uniform("u_toneMapOperator", static_cast<int>(settings.grading.tone_mapping));
    m_composite_shader->set_uniform("u_exposure", settings.grading.exposure);
    m_composite_shader->set_uniform("u_contrast", settings.grading.contrast);
    m_composite_shader->set_uniform("u_saturation", settings.grading.saturation);
    m_composite_shader->set_uniform("u_whiteBalance", settings.grading.white_balance);
    m_composite_shader->set_uniform("u_gamma", settings.grading.gamma);
    m_composite_shader->set_uniform("u_vignetteIntensity", settings.grading.vignette_intensity);
    m_composite_shader->set_uniform("u_vignetteSmoothness", settings.grading.vignette_smoothness);
    m_composite_shader->set_uniform("u_chromaticAberration", settings.grading.chromatic_aberration);
    m_composite_shader->set_uniform("u_enableDithering", settings.grading.enable_dithering ? 1 : 0);

    render_quad();

    m_composite_shader->unbind();

    // 4. Optional FXAA 3.11 & Temporal Anti-Aliasing (TAA) Pass
    if (run_aa) {
        glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
        glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));

        m_fxaa_taa_shader->bind();
        m_fxaa_taa_shader->set_uniform("u_ldrTexture", 0);
        m_fxaa_taa_shader->set_uniform("u_historyTexture", 1);
        m_fxaa_taa_shader->set_uniform("u_depthTexture", 2);
        m_fxaa_taa_shader->set_uniform("u_texelSize", 1.0f / static_cast<float>(width), 1.0f / static_cast<float>(height));
        m_fxaa_taa_shader->set_uniform("u_enableFxaa", settings.anti_aliasing.enable_fxaa ? 1 : 0);
        m_fxaa_taa_shader->set_uniform("u_enableTaa", (settings.anti_aliasing.enable_taa && m_taa_history_valid) ? 1 : 0);
        m_fxaa_taa_shader->set_uniform("u_taaBlendFactor", settings.anti_aliasing.taa_blend_factor);
        m_fxaa_taa_shader->set_uniform("u_invViewProj", inv_view_proj);
        m_fxaa_taa_shader->set_uniform("u_prevViewProj", m_prev_view_proj);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_ldr_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_taa_history_tex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, scene_depth_texture);

        render_quad();

        // Update TAA history buffer for next frame if TAA is active
        if (settings.anti_aliasing.enable_taa) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_taa_history_fbo);
            render_quad();
            glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
            m_taa_history_valid = true;
            m_prev_view_proj = view_proj;
        }

        m_fxaa_taa_shader->unbind();
    }

    // Clean unbinds
    for (int t = 3; t >= 0; --t) {
        glActiveTexture(GL_TEXTURE0 + t);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

} // namespace orion

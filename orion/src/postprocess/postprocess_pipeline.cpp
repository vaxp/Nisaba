#include "orion/postprocess/postprocess_pipeline.hpp"
#include <GL/glew.h>
#include <GL/gl.h>

namespace orion {

PostProcessPipeline::PostProcessPipeline() {
    init_shaders();
    glGenVertexArrays(1, &m_fullscreen_vao);
}

PostProcessPipeline::~PostProcessPipeline() {
    if (m_fullscreen_vao != 0) {
        glDeleteVertexArrays(1, &m_fullscreen_vao);
        m_fullscreen_vao = 0;
    }
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
uniform vec3 u_cameraPosition;

// Toggles & Parameters
uniform int u_enableGtao;
uniform int u_enableBloom;
uniform float u_bloomIntensity;

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
uniform float u_exposure;
uniform float u_contrast;
uniform float u_saturation;
uniform vec3 u_whiteBalance;
uniform float u_gamma;

vec3 reconstructWorldPos(vec2 uv, float depth) {
    vec4 clip = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 world = u_invViewProj * clip;
    return world.xyz / max(world.w, 1e-5);
}

void main() {
    vec3 color = texture(u_hdrSceneTexture, v_uv).rgb;
    float depth = texture(u_sceneDepthTexture, v_uv).r;

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
        vec3 worldPos = reconstructWorldPos(v_uv, depth);
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

    // 5. ACES Filmic Tone Mapping (Academy Color Encoding System fit)
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    vec3 ldr = clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);

    // 6. Gamma Correction
    vec3 finalColor = pow(ldr, vec3(1.0 / max(u_gamma, 0.1)));

    FragColor = vec4(finalColor, 1.0);
}
)";

    m_composite_shader = std::make_unique<GlShaderProgram>();
    m_composite_shader->compile_and_link(s_vert, s_composite_frag);
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

    // 3. Final Composite into Destination Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);
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
    m_composite_shader->set_uniform("u_cameraPosition", camera.get_position());

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

    // Color Grading Settings
    m_composite_shader->set_uniform("u_exposure", settings.grading.exposure);
    m_composite_shader->set_uniform("u_contrast", settings.grading.contrast);
    m_composite_shader->set_uniform("u_saturation", settings.grading.saturation);
    m_composite_shader->set_uniform("u_whiteBalance", settings.grading.white_balance);
    m_composite_shader->set_uniform("u_gamma", settings.grading.gamma);

    render_quad();

    m_composite_shader->unbind();

    // Clean unbinds
    for (int t = 3; t >= 0; --t) {
        glActiveTexture(GL_TEXTURE0 + t);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

} // namespace orion

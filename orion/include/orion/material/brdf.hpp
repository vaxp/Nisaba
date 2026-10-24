#pragma once

#include "orion/math/vec3.hpp"

namespace orion {


/// @brief Analytical Cook-Torrance Microfacet BRDF evaluation and GLSL shader code repository.
class CookTorranceBRDF {
public:
    /// @brief Trowbridge-Reitz GGX Normal Distribution Function (NDF).
    [[nodiscard]] static float distribution_ggx(float n_dot_h, float roughness) noexcept;

    /// @brief Smith geometry attenuation model with Schlick-GGX masking-shadowing.
    [[nodiscard]] static float geometry_smith(float n_dot_v, float n_dot_l, float roughness) noexcept;

    /// @brief Schlick approximation for Fresnel reflectance.
    [[nodiscard]] static Vec3 fresnel_schlick(float cos_theta, const Vec3& f0) noexcept;

    /// @brief Evaluates full Cook-Torrance BRDF for given surface normal, view, and light directions.
    /// @param n Normalized surface normal.
    /// @param v Normalized vector towards the eye/camera.
    /// @param l Normalized vector towards the light source.
    /// @param albedo Base diffuse/specular surface color.
    /// @param metallic Metallic parameter [0, 1].
    /// @param roughness Perceptual roughness parameter [0.04, 1].
    [[nodiscard]] static Vec3 evaluate(
        const Vec3& n,
        const Vec3& v,
        const Vec3& l,
        const Vec3& albedo,
        float metallic,
        float roughness
    ) noexcept;

    /// @brief Charlie micro-fiber sheen distribution for cloth and fabrics.
    [[nodiscard]] static float distribution_charlie(float n_dot_h, float roughness) noexcept;

    /// @brief Neubelt visibility for cloth fabrics.
    [[nodiscard]] static float visibility_neubelt(float n_dot_v, float n_dot_l) noexcept;

    /// @brief Kelemen specular visibility for clear coat layers.
    [[nodiscard]] static float visibility_kelemen(float l_dot_h) noexcept;

    /// @brief Anisotropic GGX distribution for brushed metals and hair.
    [[nodiscard]] static float distribution_anisotropic(float at, float ab, float to_h, float bo_h, float no_h) noexcept;

    /// @brief Anisotropic correlated Smith visibility.
    [[nodiscard]] static float visibility_anisotropic(float at, float ab, float to_v, float bo_v, float to_l, float bo_l, float no_v, float no_l) noexcept;

    /// @brief Energy-conserving wrap diffuse for porous/cloth surfaces.
    [[nodiscard]] static float diffuse_wrap(float n_dot_l, float w = 0.5f) noexcept;

    /// @brief Evaluates full clear coat layer specular contribution.
    [[nodiscard]] static Vec3 evaluate_clear_coat(
        const Vec3& n,
        const Vec3& v,
        const Vec3& l,
        float coat_factor,
        float coat_roughness
    ) noexcept;

    /// @brief Evaluates cloth/velvet BRDF incorporating Charlie sheen and wrap diffuse.
    [[nodiscard]] static Vec3 evaluate_cloth(
        const Vec3& n,
        const Vec3& v,
        const Vec3& l,
        const Vec3& base_color,
        const Vec3& sheen_color,
        float sheen_roughness
    ) noexcept;

    /// @brief Evaluates anisotropic metallic BRDF.
    [[nodiscard]] static Vec3 evaluate_anisotropic(
        const Vec3& n,
        const Vec3& t,
        const Vec3& b,
        const Vec3& v,
        const Vec3& l,
        const Vec3& albedo,
        float metallic,
        float roughness,
        float anisotropy
    ) noexcept;

    /// @brief Evaluates subsurface scattering approximation (forward + back scatter + penumbra).
    [[nodiscard]] static Vec3 evaluate_subsurface(
        const Vec3& n,
        const Vec3& v,
        const Vec3& l,
        const Vec3& base_color,
        const Vec3& sss_color,
        float thickness,
        float sss_power = 12.0f
    ) noexcept;

    /// E_comp = 1.0 + f0 * (1.0 / dfg_y - 1.0).
    [[nodiscard]] static Vec3 evaluate_energy_compensation(
        const Vec3& f0,
        float dfg_y
    ) noexcept;

    /// @brief Analytical polynomial approximation of split-sum DFG.y for multiple-scattering energy conservation.
    [[nodiscard]] static float approximate_dfg_multiscatter(
        float n_dot_v,
        float perceptual_roughness
    ) noexcept;

    [[nodiscard]] static Vec3 evaluate_iridescence(
        float cos_theta,
        float thickness_nm = 380.0f,
        float film_ior = 1.3f
    ) noexcept;

    // --- Embedded Sovereign GLSL 450 Shaders ---
    static constexpr const char* PBR_VERTEX_SHADER_GLSL = R"(#version 450
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inTexCoord0;
layout(location = 4) in vec2 inTexCoord1;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 cameraPosition;
} camera;

layout(push_constant) uniform ModelPushConstants {
    mat4 model;
    mat4 normalMatrix;
} object;

layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outTangent;
layout(location = 3) out vec2 outUV0;

void main() {
    vec4 worldPos = object.model * vec4(inPosition, 1.0);
    outWorldPos = worldPos.xyz;
    outNormal = normalize((object.normalMatrix * vec4(inNormal, 0.0)).xyz);
    outTangent = vec4(normalize((object.normalMatrix * vec4(inTangent.xyz, 0.0)).xyz), inTangent.w);
    outUV0 = inTexCoord0;
    gl_Position = camera.viewProjection * worldPos;
}
)";

    static constexpr const char* PBR_FRAGMENT_SHADER_GLSL = R"(#version 450
layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 inTangent;
layout(location = 3) in vec2 inUV0;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 view;
    mat4 projection;
    mat4 viewProjection;
    vec3 cameraPosition;
} camera;

layout(set = 1, binding = 0) uniform MaterialUBO {
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

layout(set = 1, binding = 1) uniform sampler2D baseColorTexture;
layout(set = 1, binding = 2) uniform sampler2D metallicRoughnessTexture;
layout(set = 1, binding = 3) uniform sampler2D normalTexture;
layout(set = 1, binding = 4) uniform sampler2D occlusionTexture;
layout(set = 1, binding = 5) uniform sampler2D emissiveTexture;

layout(location = 0) out vec4 outColor;

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
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec4 baseColor = material.baseColorFactor;
    if ((material.textureFlags & 1u) != 0u) {
        baseColor *= texture(baseColorTexture, inUV0);
    }
    if (material.alphaMode == 1u && baseColor.a < material.alphaCutoff) {
        discard;
    }

    float metallic = material.metallicFactor;
    float roughness = material.roughnessFactor;
    if ((material.textureFlags & 2u) != 0u) {
        vec4 mr = texture(metallicRoughnessTexture, inUV0);
        roughness *= mr.g;
        metallic *= mr.b;
    }
    roughness = clamp(roughness, 0.04, 1.0);

    vec3 N = normalize(inNormal);
    if ((material.textureFlags & 4u) != 0u) {
        vec3 normalMap = texture(normalTexture, inUV0).rgb * 2.0 - 1.0;
        normalMap.xy *= material.normalScale;
        vec3 T = normalize(inTangent.xyz);
        vec3 B = cross(N, T) * inTangent.w;
        mat3 TBN = mat3(T, B, N);
        N = normalize(TBN * normalMap);
    }

    vec3 V = normalize(camera.cameraPosition - inWorldPos);
    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metallic);

    // Directional sunlight simulation
    vec3 L = normalize(vec3(0.5, 1.0, 0.3));
    vec3 H = normalize(V + L);
    vec3 lightRadiance = vec3(3.0);

    float NdotL = max(dot(N, L), 0.0);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL + 0.0001;
    vec3 specular = numerator / denominator;

    vec3 kS = F;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 diffuse = kD * baseColor.rgb / PI;

    vec3 directLight = (diffuse + specular) * lightRadiance * NdotL;
    vec3 ambient = vec3(0.03) * baseColor.rgb;
    if ((material.textureFlags & 8u) != 0u) {
        ambient *= texture(occlusionTexture, inUV0).r * material.occlusionStrength;
    }

    vec3 emissive = material.emissiveFactor;
    if ((material.textureFlags & 16u) != 0u) {
        emissive *= texture(emissiveTexture, inUV0).rgb;
    }

    vec3 finalColor = ambient + directLight + emissive;
    // ACES Film tone mapping
    finalColor = (finalColor * (2.51 * finalColor + 0.03)) / (finalColor * (2.43 * finalColor + 0.59) + 0.14);
    outColor = vec4(clamp(finalColor, 0.0, 1.0), baseColor.a);
}
)";
};

using BRDF = CookTorranceBRDF;
using Brdf = CookTorranceBRDF;

} // namespace orion

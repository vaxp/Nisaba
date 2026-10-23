#pragma once

#include "orion/math/vec3.hpp"
#include "orion/material/texture.hpp"
#include <memory>
#include <cstdint>

namespace orion {


enum class AlphaMode : uint8_t {
    Opaque = 0,
    Mask = 1,
    Blend = 2
};

enum class ShadingModel : uint32_t {
    Standard = 0,
    ClearCoat = 1,
    Anisotropic = 2,
    Cloth = 3,
    Subsurface = 4,
    Transmission = 5,
    Unlit = 6
};

enum TextureFlags : uint32_t {
    HAS_NO_TEXTURES            = 0,
    HAS_BASE_COLOR_MAP         = 1 << 0,
    HAS_METALLIC_ROUGHNESS_MAP = 1 << 1,
    HAS_NORMAL_MAP             = 1 << 2,
    HAS_OCCLUSION_MAP          = 1 << 3,
    HAS_EMISSIVE_MAP           = 1 << 4
};

/// @brief 144-byte std140-compatible Uniform Buffer Object block for GPU PBR fragment shaders.
/// Houses core PBR factors alongside specialized parameters for ClearCoat, Anisotropic, Cloth, Subsurface, and Transmission.
struct alignas(16) PbrMaterialUniforms {
    float base_color[4]{1.0f, 1.0f, 1.0f, 1.0f};           // 0..16: RGBA base color factor (vec4)
    float emissive[3]{0.0f, 0.0f, 0.0f};                    // 16..28: RGB emissive factor (vec3)
    float normal_scale{1.0f};                               // 28..32: Tangent-space normal intensity
    float metallic{0.0f};                                   // 32..36: Metallic factor [0, 1]
    float roughness{0.5f};                                  // 36..40: Perceptual roughness factor [0.04, 1]
    float occlusion_strength{1.0f};                         // 40..44: Ambient occlusion multiplier
    float alpha_cutoff{0.5f};                               // 44..48: Alpha test cutoff threshold
    uint32_t texture_flags{0};                              // 48..52: Bitmask of bound textures
    uint32_t alpha_mode{0};                                 // 52..56: 0=Opaque, 1=Mask, 2=Blend
    uint32_t double_sided{0};                               // 56..60: 1 if two-sided lighting
    uint32_t shading_model{0};                              // 60..64: 0=Standard, 1=ClearCoat, 2=Aniso, 3=Cloth, 4=SSS, 5=Trans, 6=Unlit
    float clear_coat{0.0f};                                 // 64..68: Clear coat intensity [0, 1]
    float clear_coat_roughness{0.04f};                      // 68..72: Clear coat layer roughness [0.001, 1]
    float anisotropy{0.0f};                                 // 72..76: Anisotropy strength [-1, 1]
    float pad0{0.0f};                                       // 76..80: Alignment padding
    float sheen_color[3]{0.0f, 0.0f, 0.0f};                 // 80..92: Sheen specular tint (vec3)
    float sheen_roughness{0.2f};                            // 92..96: Sheen roughness [0.04, 1]
    float subsurface_color[3]{1.0f, 1.0f, 1.0f};            // 96..108: Subsurface scattering tint (vec3)
    float thickness{0.5f};                                  // 108..112: Material volumetric thickness [0, 1]
    float subsurface_power{12.0f};                          // 112..116: Subsurface forward scattering power
    float transmission{0.0f};                               // 116..120: Optical transmission factor [0, 1]
    float ior{1.5f};                                        // 120..124: Index of refraction
    float pad1{0.0f};                                       // 124..128: Alignment padding
    float absorption[3]{1.0f, 1.0f, 1.0f};                  // 128..140: Medium volumetric absorption color (vec3)
    float pad2{0.0f};                                       // 140..144: Alignment padding
};

static_assert(sizeof(PbrMaterialUniforms) == 144, "PbrMaterialUniforms must be exactly 144 bytes");
static_assert(alignof(PbrMaterialUniforms) == 16, "PbrMaterialUniforms must be 16-byte aligned");

/// @brief Physically Based Rendering (PBR) multi-model material definition.
class PbrMaterial {
public:
    ShadingModel shading_model{ShadingModel::Standard};

    Vec4 base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
    float metallic_factor{0.0f};
    float roughness_factor{0.5f};
    Vec3 emissive_factor{0.0f, 0.0f, 0.0f};
    float normal_scale{1.0f};
    float occlusion_strength{1.0f};
    float alpha_cutoff{0.5f};
    AlphaMode alpha_mode{AlphaMode::Opaque};
    bool double_sided{false};

    // Clear Coat parameters
    float clear_coat{0.0f};
    float clear_coat_roughness{0.04f};

    // Anisotropy parameters
    float anisotropy{0.0f};

    // Cloth & Sheen parameters
    Vec3 sheen_color{0.0f, 0.0f, 0.0f};
    float sheen_roughness{0.2f};

    // Subsurface Scattering parameters
    Vec3 subsurface_color{1.0f, 1.0f, 1.0f};
    float thickness{0.5f};
    float subsurface_power{12.0f};

    // Transmission & Optical Refraction parameters
    float transmission{0.0f};
    float ior{1.5f};
    Vec3 absorption{1.0f, 1.0f, 1.0f};

    std::shared_ptr<Texture2D> base_color_texture;
    std::shared_ptr<Texture2D> metallic_roughness_texture;
    std::shared_ptr<Texture2D> normal_texture;
    std::shared_ptr<Texture2D> occlusion_texture;
    std::shared_ptr<Texture2D> emissive_texture;

    PbrMaterial() = default;

    /// @brief Computes active texture bitmask.
    [[nodiscard]] uint32_t compute_texture_flags() const noexcept;

    /// @brief Packs all material parameters into a 144-byte GPU uniform structure.
    [[nodiscard]] PbrMaterialUniforms get_uniforms() const noexcept;

    /// @brief True if material requires back-to-front rendering with alpha blending.
    [[nodiscard]] bool is_transparent() const noexcept {
        return alpha_mode == AlphaMode::Blend || transmission > 0.001f || base_color_factor.w < 0.999f;
    }

    /// @brief Creates a default shiny dielectric white material.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_default_dielectric() {
        auto m = std::make_shared<PbrMaterial>();
        m->base_color_factor = Vec4(0.9f, 0.9f, 0.9f, 1.0f);
        m->metallic_factor = 0.0f;
        m->roughness_factor = 0.3f;
        return m;
    }

    /// @brief Creates a polished metallic gold material.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_gold() {
        auto m = std::make_shared<PbrMaterial>();
        m->base_color_factor = Vec4(1.0f, 0.766f, 0.336f, 1.0f);
        m->metallic_factor = 1.0f;
        m->roughness_factor = 0.15f;
        return m;
    }

    /// @brief Creates an automotive clear-coat lacquered material.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_clear_coat(
        const Vec4& color = Vec4(0.85f, 0.1f, 0.1f, 1.0f),
        float coat = 1.0f,
        float coat_roughness = 0.04f
    ) {
        auto m = std::make_shared<PbrMaterial>();
        m->shading_model = ShadingModel::ClearCoat;
        m->base_color_factor = color;
        m->metallic_factor = 0.2f;
        m->roughness_factor = 0.4f;
        m->clear_coat = coat;
        m->clear_coat_roughness = coat_roughness;
        return m;
    }

    /// @brief Creates a brushed metal anisotropic material.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_anisotropic(
        const Vec4& color = Vec4(0.9f, 0.9f, 0.92f, 1.0f),
        float anisotropy = 0.85f,
        float roughness = 0.25f
    ) {
        auto m = std::make_shared<PbrMaterial>();
        m->shading_model = ShadingModel::Anisotropic;
        m->base_color_factor = color;
        m->metallic_factor = 0.95f;
        m->roughness_factor = roughness;
        m->anisotropy = anisotropy;
        return m;
    }

    /// @brief Creates a rich cloth/velvet fabric material.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_cloth(
        const Vec4& base_color = Vec4(0.2f, 0.08f, 0.35f, 1.0f),
        const Vec3& sheen_color = Vec3(0.85f, 0.6f, 0.95f),
        float sheen_roughness = 0.3f
    ) {
        auto m = std::make_shared<PbrMaterial>();
        m->shading_model = ShadingModel::Cloth;
        m->base_color_factor = base_color;
        m->metallic_factor = 0.0f;
        m->roughness_factor = 0.6f;
        m->sheen_color = sheen_color;
        m->sheen_roughness = sheen_roughness;
        return m;
    }

    /// @brief Creates a translucent subsurface scattering material (skin, wax, marble).
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_subsurface(
        const Vec4& base_color = Vec4(0.92f, 0.75f, 0.65f, 1.0f),
        const Vec3& sss_color = Vec3(0.85f, 0.25f, 0.15f),
        float thickness = 0.35f
    ) {
        auto m = std::make_shared<PbrMaterial>();
        m->shading_model = ShadingModel::Subsurface;
        m->base_color_factor = base_color;
        m->metallic_factor = 0.0f;
        m->roughness_factor = 0.3f;
        m->subsurface_color = sss_color;
        m->thickness = thickness;
        m->subsurface_power = 12.0f;
        return m;
    }

    /// @brief Creates a physically refractive glass/liquid material.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_glass(
        float ior = 1.52f,
        const Vec3& absorption = Vec3(0.95f, 0.98f, 1.0f)
    ) {
        auto m = std::make_shared<PbrMaterial>();
        m->shading_model = ShadingModel::Transmission;
        m->base_color_factor = Vec4(1.0f, 1.0f, 1.0f, 0.1f);
        m->metallic_factor = 0.0f;
        m->roughness_factor = 0.05f;
        m->transmission = 1.0f;
        m->ior = ior;
        m->absorption = absorption;
        m->alpha_mode = AlphaMode::Blend;
        return m;
    }

    /// @brief Creates an unlit self-illuminated material for VFX and UI elements.
    [[nodiscard]] static std::shared_ptr<PbrMaterial> create_unlit(
        const Vec4& color = Vec4(1.0f, 1.0f, 1.0f, 1.0f),
        const Vec3& emissive = Vec3(0.0f, 0.0f, 0.0f)
    ) {
        auto m = std::make_shared<PbrMaterial>();
        m->shading_model = ShadingModel::Unlit;
        m->base_color_factor = color;
        m->emissive_factor = emissive;
        return m;
    }
};

} // namespace orion

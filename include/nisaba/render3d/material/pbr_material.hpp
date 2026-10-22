#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include "nisaba/render3d/material/texture.hpp"
#include <memory>
#include <cstdint>

namespace nisaba::render3d {

using Vec3 = nisaba::physics::Vec3;

enum class AlphaMode : uint8_t {
    Opaque = 0,
    Mask = 1,
    Blend = 2
};

enum TextureFlags : uint32_t {
    HAS_NO_TEXTURES            = 0,
    HAS_BASE_COLOR_MAP         = 1 << 0,
    HAS_METALLIC_ROUGHNESS_MAP = 1 << 1,
    HAS_NORMAL_MAP             = 1 << 2,
    HAS_OCCLUSION_MAP          = 1 << 3,
    HAS_EMISSIVE_MAP           = 1 << 4
};

/// @brief 64-byte std140-compatible Uniform Buffer Object block for GPU PBR fragment shaders.
struct alignas(16) PbrMaterialUniforms {
    float base_color[4]{1.0f, 1.0f, 1.0f, 1.0f};           // 16 bytes: RGBA base color factor
    float emissive[3]{0.0f, 0.0f, 0.0f};                    // 12 bytes: RGB emissive factor
    float normal_scale{1.0f};                               // 4 bytes: Tangent-space normal intensity
    float metallic{0.0f};                                   // 4 bytes: Metallic factor [0, 1]
    float roughness{0.5f};                                  // 4 bytes: Perceptual roughness factor [0.04, 1]
    float occlusion_strength{1.0f};                         // 4 bytes: Ambient occlusion multiplier
    float alpha_cutoff{0.5f};                               // 4 bytes: Alpha test cutoff threshold
    uint32_t texture_flags{0};                              // 4 bytes: Bitmask of bound textures
    uint32_t alpha_mode{0};                                 // 4 bytes: 0=Opaque, 1=Mask, 2=Blend
    uint32_t double_sided{0};                               // 4 bytes: 1 if two-sided lighting
    float padding{0.0f};                                    // 4 bytes: 16-byte alignment pad
};

static_assert(sizeof(PbrMaterialUniforms) == 64, "PbrMaterialUniforms must be exactly 64 bytes");
static_assert(alignof(PbrMaterialUniforms) == 16, "PbrMaterialUniforms must be 16-byte aligned");

/// @brief Physically Based Rendering (PBR) metallic-roughness material definition.
class PbrMaterial {
public:
    Vec4 base_color_factor{1.0f, 1.0f, 1.0f, 1.0f};
    float metallic_factor{0.0f};
    float roughness_factor{0.5f};
    Vec3 emissive_factor{0.0f, 0.0f, 0.0f};
    float normal_scale{1.0f};
    float occlusion_strength{1.0f};
    float alpha_cutoff{0.5f};
    AlphaMode alpha_mode{AlphaMode::Opaque};
    bool double_sided{false};

    std::shared_ptr<Texture2D> base_color_texture;
    std::shared_ptr<Texture2D> metallic_roughness_texture;
    std::shared_ptr<Texture2D> normal_texture;
    std::shared_ptr<Texture2D> occlusion_texture;
    std::shared_ptr<Texture2D> emissive_texture;

    PbrMaterial() = default;

    /// @brief Computes active texture bitmask.
    [[nodiscard]] uint32_t compute_texture_flags() const noexcept;

    /// @brief Packs all material parameters into a 64-byte GPU uniform structure.
    [[nodiscard]] PbrMaterialUniforms get_uniforms() const noexcept;

    /// @brief True if material requires back-to-front rendering with alpha blending.
    [[nodiscard]] bool is_transparent() const noexcept {
        return alpha_mode == AlphaMode::Blend || base_color_factor.w < 0.999f;
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
};

} // namespace nisaba::render3d

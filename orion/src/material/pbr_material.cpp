#include "orion/material/pbr_material.hpp"

namespace orion {

uint32_t PbrMaterial::compute_texture_flags() const noexcept {
    uint32_t flags = HAS_NO_TEXTURES;
    if (base_color_texture)         flags |= HAS_BASE_COLOR_MAP;
    if (metallic_roughness_texture) flags |= HAS_METALLIC_ROUGHNESS_MAP;
    if (normal_texture)             flags |= HAS_NORMAL_MAP;
    if (occlusion_texture)          flags |= HAS_OCCLUSION_MAP;
    if (emissive_texture)           flags |= HAS_EMISSIVE_MAP;
    return flags;
}

PbrMaterialUniforms PbrMaterial::get_uniforms() const noexcept {
    PbrMaterialUniforms u;

    u.base_color[0] = base_color_factor.x;
    u.base_color[1] = base_color_factor.y;
    u.base_color[2] = base_color_factor.z;
    u.base_color[3] = base_color_factor.w;

    u.emissive[0] = emissive_factor.x;
    u.emissive[1] = emissive_factor.y;
    u.emissive[2] = emissive_factor.z;
    u.normal_scale = normal_scale;

    u.metallic = metallic_factor;
    u.roughness = roughness_factor;
    u.occlusion_strength = occlusion_strength;
    u.alpha_cutoff = alpha_cutoff;

    u.texture_flags = compute_texture_flags();
    u.alpha_mode = static_cast<uint32_t>(alpha_mode);
    u.double_sided = double_sided ? 1u : 0u;
    u.shading_model = static_cast<uint32_t>(shading_model);

    u.clear_coat = clear_coat;
    u.clear_coat_roughness = clear_coat_roughness;
    u.anisotropy = anisotropy;
    u.pad0 = iridescence;

    u.sheen_color[0] = sheen_color.x;
    u.sheen_color[1] = sheen_color.y;
    u.sheen_color[2] = sheen_color.z;
    u.sheen_roughness = sheen_roughness;

    u.subsurface_color[0] = subsurface_color.x;
    u.subsurface_color[1] = subsurface_color.y;
    u.subsurface_color[2] = subsurface_color.z;
    u.thickness = thickness;

    u.subsurface_power = subsurface_power;
    u.transmission = transmission;
    u.ior = ior;
    u.pad1 = iridescence_thickness;

    u.absorption[0] = absorption.x;
    u.absorption[1] = absorption.y;
    u.absorption[2] = absorption.z;
    u.pad2 = thin_walled ? - std::max(0.001f, dispersion) : dispersion;

    return u;
}

} // namespace orion

#include "orion/lighting/lighting_manager.hpp"
#include "orion/material/brdf.hpp"
#include <algorithm>
#include <cmath>

namespace orion {

LightingManager::LightingManager()
    : m_cluster_grid(16, 9, 24) {
    m_lights.reserve(128);
}

void LightingManager::clear() noexcept {
    m_lights.clear();
}

void LightingManager::add_light(const Light3D& light) {
    m_lights.push_back(light);
}

void LightingManager::set_ambient_light(const Vec3& color, float intensity) noexcept {
    m_ambient_color = color;
    m_ambient_intensity = std::max(0.0f, intensity);
}

void LightingManager::prepare_frame(const Camera3D& camera, float aspect_ratio) noexcept {
    m_cluster_grid.build_frustum_grid(
        camera.get_fov_y(),
        aspect_ratio,
        camera.get_near_clip(),
        camera.get_far_clip()
    );
    m_cluster_grid.cull_lights(m_lights, camera.get_view_matrix());
}

GpuLightingUboBlock LightingManager::generate_gpu_ubo() const noexcept {
    GpuLightingUboBlock ubo{};
    const size_t count = std::min(m_lights.size(), MAX_GPU_LIGHTS);
    ubo.light_count = static_cast<uint32_t>(count);

    for (size_t i = 0; i < count; ++i) {
        ubo.lights[i] = m_lights[i].get_gpu_data(i == 0 ? 0 : -1);
    }

    ubo.cluster_dim_x = m_cluster_grid.dim_x();
    ubo.cluster_dim_y = m_cluster_grid.dim_y();
    ubo.cluster_dim_z = m_cluster_grid.dim_z();

    ubo.cluster_z_params[0] = m_cluster_grid.z_near();
    ubo.cluster_z_params[1] = m_cluster_grid.z_far();
    ubo.cluster_z_params[2] = std::log(m_cluster_grid.z_far() / m_cluster_grid.z_near());
    ubo.cluster_z_params[3] = static_cast<float>(m_cluster_grid.dim_z());

    ubo.ambient_color[0] = m_ambient_color.x;
    ubo.ambient_color[1] = m_ambient_color.y;
    ubo.ambient_color[2] = m_ambient_color.z;
    ubo.ambient_color[3] = m_ambient_intensity;

    return ubo;
}

Vec3 LightingManager::evaluate_lighting_cpu(
    const Vec3& world_pos,
    const Vec3& normal,
    const Vec3& view_pos,
    const PbrMaterial& material
) const noexcept {
    if (material.shading_model == ShadingModel::Unlit) {
        return material.base_color_factor.xyz() + material.emissive_factor;
    }

    const Vec3 V = (view_pos - world_pos).normalized();
    const Vec3 N = normal.normalized();

    Vec3 accumulated_direct(0.0f, 0.0f, 0.0f);

    for (const auto& light : m_lights) {
        Vec3 L(0.0f, 0.0f, 0.0f);
        float attenuation = 1.0f;

        if (light.type == LightType::Directional) {
            L = (-light.direction).normalized();
        } else {
            const Vec3 to_light = light.position - world_pos;
            const float dist = to_light.length();
            if (dist >= light.range || dist < 1e-4f) {
                continue;
            }

            attenuation = light.compute_attenuation(dist);
            if (attenuation <= 1e-6f) {
                continue;
            }

            if (light.type == LightType::Spot) {
                attenuation *= light.compute_spot_falloff(to_light);
                if (attenuation <= 1e-6f) {
                    continue;
                }
            }

            if (light.type == LightType::Sphere) {
                const Vec3 R = (-V).reflect(N);
                const Vec3 rep_point = light.compute_representative_point(world_pos, R);
                L = rep_point.normalized();
            } else {
                L = to_light / dist;
            }
        }

        const float NdotL = std::max(N.dot(L), 0.0f);
        if (NdotL <= 1e-6f) {
            continue;
        }

        const Vec3 light_radiance = light.color * (light.intensity * attenuation * NdotL);

        // Evaluate BRDF according to active ShadingModel
        Vec3 direct_contrib(0.0f, 0.0f, 0.0f);

        if (material.shading_model == ShadingModel::ClearCoat) {
            Vec3 coat = Brdf::evaluate_clear_coat(
                N, V, L,
                material.clear_coat,
                material.clear_coat_roughness
            );
            Vec3 base = Brdf::evaluate(
                N, V, L,
                material.base_color_factor.xyz(),
                material.metallic_factor,
                material.roughness_factor
            );
            direct_contrib = base + coat;
        } else if (material.shading_model == ShadingModel::Cloth) {
            direct_contrib = Brdf::evaluate_cloth(
                N, V, L,
                material.base_color_factor.xyz(),
                material.sheen_color,
                material.sheen_roughness
            );
        } else if (material.shading_model == ShadingModel::Subsurface) {
            direct_contrib = Brdf::evaluate_subsurface(
                N, V, L,
                material.base_color_factor.xyz(),
                material.subsurface_color,
                material.thickness,
                material.subsurface_power
            );
        } else {
            // Standard GGX
            direct_contrib = Brdf::evaluate(
                N, V, L,
                material.base_color_factor.xyz(),
                material.metallic_factor,
                material.roughness_factor
            );
        }

        accumulated_direct = accumulated_direct + direct_contrib * light_radiance;
    }

    const Vec3 ambient = material.base_color_factor.xyz() * (m_ambient_color * m_ambient_intensity);
    return accumulated_direct + ambient + material.emissive_factor;
}

} // namespace orion

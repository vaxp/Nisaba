#pragma once

#include "orion/lighting/light.hpp"
#include "orion/lighting/cluster_grid.hpp"
#include "orion/material/pbr_material.hpp"
#include "orion/camera/camera.hpp"
#include <vector>
#include <memory>
#include <cstdint>

namespace orion {

/// @brief High-performance manager orchestrating dynamic multi-light scenes,
/// clustered froxel spatial partitioning, CPU analytical evaluation, and std140 GPU UBO synchronization.
class LightingManager {
public:
    static constexpr size_t MAX_GPU_LIGHTS = 128;

    LightingManager();
    ~LightingManager() = default;

    /// @brief Clears all active dynamic lights.
    void clear() noexcept;

    /// @brief Adds a light source to the manager.
    void add_light(const Light3D& light);

    /// @brief Sets the scene global ambient light color and intensity.
    void set_ambient_light(const Vec3& color, float intensity = 1.0f) noexcept;

    /// @brief Prepares and synchronizes lighting for the current view frame (culls lights into cluster grid).
    void prepare_frame(const Camera3D& camera, float aspect_ratio) noexcept;

    /// @brief Generates the 8240-byte std140 uniform block for GPU shaders.
    [[nodiscard]] GpuLightingUboBlock generate_gpu_ubo() const noexcept;

    /// @brief Evaluates analytical direct lighting on CPU for a given surface point and material.
    [[nodiscard]] Vec3 evaluate_lighting_cpu(
        const Vec3& world_pos,
        const Vec3& normal,
        const Vec3& view_pos,
        const PbrMaterial& material
    ) const noexcept;

    [[nodiscard]] const std::vector<Light3D>& get_lights() const noexcept { return m_lights; }
    [[nodiscard]] const ClusterGrid& get_cluster_grid() const noexcept { return m_cluster_grid; }
    [[nodiscard]] ClusterGrid& get_cluster_grid() noexcept { return m_cluster_grid; }
    [[nodiscard]] const Vec3& ambient_color() const noexcept { return m_ambient_color; }
    [[nodiscard]] float ambient_intensity() const noexcept { return m_ambient_intensity; }

private:
    std::vector<Light3D> m_lights;
    Vec3 m_ambient_color{0.1f, 0.1f, 0.12f};
    float m_ambient_intensity{1.0f};
    ClusterGrid m_cluster_grid;
};

} // namespace orion

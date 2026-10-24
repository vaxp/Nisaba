#pragma once

#include "orion/scene/renderable.hpp"
#include "orion/lighting/light.hpp"
#include "orion/ibl/spherical_harmonics.hpp"
#include "orion/ibl/environment_map.hpp"
#include "orion/camera/frustum.hpp"
#include <vector>
#include <memory>

namespace orion {

/// @brief 3D World container aggregating spatial renderables, analytical lights,
/// IBL environment radiance maps, and SIMD frustum visibility queries.
class Scene3D {
public:
    std::vector<std::shared_ptr<Renderable3D>> renderables;
    std::vector<Light3D> lights;

    Vec3 ambient_color{0.03f, 0.03f, 0.03f};
    float ambient_intensity{1.0f};

    std::shared_ptr<EnvironmentMap> environment_map;
    SphericalHarmonics9 sh_irradiance;
    float environment_intensity{1.0f};
    bool render_skybox{false};
    float skybox_blur{0.0f};

    Scene3D() = default;

    /// @brief Sets the environment map and computes/assigns the corresponding SH9 irradiance.
    void set_environment(std::shared_ptr<EnvironmentMap> env, float intensity = 1.0f);

    /// @brief Adds a renderable entity to the scene.
    void add_renderable(std::shared_ptr<Renderable3D> renderable);

    /// @brief Removes a renderable entity from the scene.
    void remove_renderable(const std::shared_ptr<Renderable3D>& renderable);

    /// @brief Adds an analytical light to the scene.
    void add_light(const Light3D& light);

    /// @brief Clears all entities and lights from the scene.
    void clear() noexcept;

    /// @brief Performs SIMD frustum culling, extracting only visible renderables.
    void gather_visible_renderables(
        const Frustum& frustum,
        std::vector<const Renderable3D*>& out_visible
    ) const noexcept;

    [[nodiscard]] size_t renderable_count() const noexcept { return renderables.size(); }
    [[nodiscard]] size_t light_count() const noexcept { return lights.size(); }
};

} // namespace orion

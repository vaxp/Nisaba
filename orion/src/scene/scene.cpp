#include "orion/scene/scene.hpp"
#include <algorithm>

namespace orion {

void Scene3D::set_environment(std::shared_ptr<EnvironmentMap> env, float intensity) {
    environment_map = std::move(env);
    environment_intensity = std::max(0.0f, intensity);
    if (environment_map) {
        sh_irradiance = environment_map->compute_sh9();
    }
}

void Scene3D::add_renderable(std::shared_ptr<Renderable3D> renderable) {
    if (renderable) {
        renderables.push_back(std::move(renderable));
    }
}

void Scene3D::remove_renderable(const std::shared_ptr<Renderable3D>& renderable) {
    auto it = std::remove(renderables.begin(), renderables.end(), renderable);
    renderables.erase(it, renderables.end());
}

void Scene3D::add_light(const Light3D& light) {
    lights.push_back(light);
}

void Scene3D::clear() noexcept {
    renderables.clear();
    lights.clear();
}

void Scene3D::gather_visible_renderables(
    const Frustum& frustum,
    std::vector<const Renderable3D*>& out_visible
) const noexcept {
    out_visible.clear();
    for (const auto& r : renderables) {
        if (!r || !r->visible || !r->mesh) continue;
        if (frustum.intersects_aabb(r->world_bounds)) {
            out_visible.push_back(r.get());
        }
    }
}

} // namespace orion

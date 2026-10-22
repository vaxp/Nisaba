#include "nisaba/render3d/scene/view.hpp"

namespace nisaba::render3d {

View3D::View3D(std::shared_ptr<Camera3D> in_cam, std::shared_ptr<Scene3D> in_scene)
    : camera(std::move(in_cam)), scene(std::move(in_scene)) {}

void View3D::set_viewport(float x, float y, float w, float h) noexcept {
    viewport = {x, y, w, h};
    if (camera && h > 0.0f) {
        camera->set_aspect_ratio(w / h);
    }
}

} // namespace nisaba::render3d

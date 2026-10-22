#pragma once

#include "nisaba/render3d/camera/camera.hpp"
#include "nisaba/render3d/scene/scene.hpp"
#include <memory>

namespace nisaba::render3d {

enum class ToneMapping : uint8_t {
    None = 0,
    Reinhard = 1,
    ACES = 2
};

struct Viewport3D {
    float x{0.0f};
    float y{0.0f};
    float width{800.0f};
    float height{600.0f};
};

/// @brief Represents a viewing configuration linking a Camera3D to a Scene3D with viewport and post-processing.
class View3D {
public:
    std::shared_ptr<Camera3D> camera;
    std::shared_ptr<Scene3D> scene;
    Viewport3D viewport;

    Vec4 clear_color{0.05f, 0.05f, 0.08f, 1.0f};
    ToneMapping tone_mapping{ToneMapping::ACES};
    float exposure{1.0f};

    bool shadows_enabled{true};
    float shadow_distance{50.0f};

    View3D() = default;
    View3D(std::shared_ptr<Camera3D> in_cam, std::shared_ptr<Scene3D> in_scene);

    void set_camera(std::shared_ptr<Camera3D> in_cam) { camera = std::move(in_cam); }
    void set_scene(std::shared_ptr<Scene3D> in_scene) { scene = std::move(in_scene); }
    void set_viewport(float x, float y, float w, float h) noexcept;
    void set_clear_color(const Vec4& c) noexcept { clear_color = c; }
};

} // namespace nisaba::render3d

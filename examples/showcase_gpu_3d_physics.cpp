#include <iostream>
#include <vector>
#include <cmath>

#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/render3d/render3d.hpp"
#include "nisaba/physics/scene/physics_scene.hpp"
#include "nisaba/physics/collision/box_shape.hpp"
#include "nisaba/physics/collision/sphere_shape.hpp"
#include "nisaba/physics/collision/cylinder_shape.hpp"
#include "nisaba/physics/collision/plane_shape.hpp"
#include "nisaba/image/image_io.hpp"

using namespace nisaba;
using namespace nisaba::render3d;
using namespace nisaba::physics;
using nisaba::gpu::GpuDevice;

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D Real-Time PBR & Physics GPU Showcase" << std::endl;
    std::cout << "==========================================================" << std::endl;

    auto device = GpuDevice::create();
    if (!device || !device->is_valid()) {
        std::cerr << "Hardware GPU device not available on this platform." << std::endl;
        return 1;
    }

    std::cout << "[GPU] Active Hardware Device: " << device->renderer_name()
              << " (" << device->version_name() << ")" << std::endl;
    device->make_current();

    const uint32_t width = 1920;
    const uint32_t height = 1080;

    RenderSurface3D surface(width, height);
    GpuBackend3D gpu_backend(2048); // 2048x2048 Cascaded Shadow Map

    // 1. Setup Physics Simulation
    PhysicsSettings phys_settings;
    phys_settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene physics_scene(phys_settings);

    auto render_scene = std::make_shared<Scene3D>();
    PhysicsRenderBridge bridge;

    // Materials
    auto gold_mat = PbrMaterial::create_gold();
    gold_mat->metallic_factor = 0.85f;
    gold_mat->roughness_factor = 0.25f;

    auto red_mat = std::make_shared<PbrMaterial>();
    red_mat->base_color_factor = Vec4(0.85f, 0.15f, 0.15f, 1.0f);
    red_mat->metallic_factor = 0.3f;
    red_mat->roughness_factor = 0.25f;

    auto blue_mat = std::make_shared<PbrMaterial>();
    blue_mat->base_color_factor = Vec4(0.15f, 0.35f, 0.85f, 1.0f);
    blue_mat->metallic_factor = 0.5f;
    blue_mat->roughness_factor = 0.2f;

    auto emerald_mat = std::make_shared<PbrMaterial>();
    emerald_mat->base_color_factor = Vec4(0.1f, 0.75f, 0.3f, 1.0f);
    emerald_mat->metallic_factor = 0.1f;
    emerald_mat->roughness_factor = 0.35f;

    auto floor_mat = std::make_shared<PbrMaterial>();
    floor_mat->base_color_factor = Vec4(0.35f, 0.35f, 0.4f, 1.0f);
    floor_mat->metallic_factor = 0.05f;
    floor_mat->roughness_factor = 0.6f;

    // A. Static Ground Floor Plane
    auto floor_shape = std::make_shared<PlaneShape>();
    BodyCreationSettings floor_settings;
    floor_settings.position = Vec3(0.0f, 0.0f, 0.0f);
    floor_settings.shape = floor_shape.get();
    floor_settings.motion_type = MotionType::Static;
    BodyID floor_id = physics_scene.create_body(floor_settings);
    auto r_floor = bridge.create_and_bind(physics_scene, floor_id, floor_mat, render_scene.get());
    (void)r_floor;

    // B. Stacking Pyramid of Dynamic Cubes
    auto box_shape = std::make_shared<BoxShape>(Vec3(0.75f, 0.75f, 0.75f));
    for (int y = 0; y < 4; ++y) {
        for (int x = -y; x <= y; x += 2) {
            BodyCreationSettings b_set;
            b_set.position = Vec3(static_cast<float>(x) * 0.9f, 0.8f + static_cast<float>(y) * 1.6f, 0.0f);
            b_set.shape = box_shape.get();
            b_set.motion_type = MotionType::Dynamic;
            b_set.friction = 0.6f;
            b_set.restitution = 0.2f;

            BodyID bid = physics_scene.create_body(b_set);
            auto mat = (y % 2 == 0) ? red_mat : blue_mat;
            auto r_box = bridge.create_and_bind(physics_scene, bid, mat, render_scene.get());
            (void)r_box;
        }
    }

    // C. Dynamic Spheres Falling from Sky
    auto sphere_shape = std::make_shared<SphereShape>(1.2f);
    BodyCreationSettings s_set;
    s_set.position = Vec3(-0.5f, 14.0f, 0.5f);
    s_set.shape = sphere_shape.get();
    s_set.motion_type = MotionType::Dynamic;
    s_set.restitution = 0.5f;
    BodyID sid = physics_scene.create_body(s_set);
    auto r_sphere = bridge.create_and_bind(physics_scene, sid, gold_mat, render_scene.get());
    (void)r_sphere;

    // D. Dynamic Cylinders
    auto cyl_shape = std::make_shared<CylinderShape>(1.0f, 0.6f);
    BodyCreationSettings c_set;
    c_set.position = Vec3(3.5f, 8.0f, -1.0f);
    c_set.shape = cyl_shape.get();
    c_set.motion_type = MotionType::Dynamic;
    BodyID cid = physics_scene.create_body(c_set);
    auto r_cyl = bridge.create_and_bind(physics_scene, cid, emerald_mat, render_scene.get());
    (void)r_cyl;

    // 2. Setup Lighting & Sun
    Light3D sun = Light3D::create_directional(
        Vec3(-0.4f, -1.0f, -0.6f).normalized(),
        Vec3(1.0f, 0.96f, 0.88f),
        5.5f
    );
    render_scene->add_light(sun);
    render_scene->ambient_color = Vec3(0.12f, 0.12f, 0.16f);

    // 3. Setup Orbit Camera
    auto camera = std::make_shared<Camera3D>();
    camera->set_look_at(Vec3(0.0f, 3.8f, 9.2f), Vec3(0.0f, 1.6f, 0.0f), Vec3::up());
    camera->set_perspective(48.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

    View3D view;
    view.set_scene(render_scene);
    view.set_camera(camera);
    view.set_viewport(0, 0, width, height);
    view.set_clear_color(Vec4(0.04f, 0.05f, 0.08f, 1.0f)); // Elegant dark obsidian
    view.shadows_enabled = true;
    view.shadow_distance = 40.0f;

    std::cout << "[Physics] Stepping rigid body dynamics and synchronizing transforms..." << std::endl;
    // Step simulation for 90 frames (1.5 seconds)
    const float dt = 1.0f / 60.0f;
    for (int frame = 0; frame < 90; ++frame) {
        physics_scene.step(dt);
        bridge.sync(physics_scene);
    }

    std::cout << "[GPU] Rendering 3D PBR scene with CSM hardware shadows..." << std::endl;
    surface.render_3d(gpu_backend, view, true);

    const auto& stats = gpu_backend.last_stats();
    std::cout << "[GPU] Frame Statistics:" << std::endl;
    std::cout << "  - Total Objects:        " << stats.total_objects << std::endl;
    std::cout << "  - Visible Objects:      " << stats.visible_objects << std::endl;
    std::cout << "  - Total Draw Calls:     " << stats.total_draw_calls << std::endl;
    std::cout << "  - Total Triangles:      " << stats.total_triangles << std::endl;

    // Read back and save showcase image
    std::vector<uint8_t> pixels(width * height * 4);
    surface.read_pixels(pixels.data());

    // OpenGL glReadPixels returns bottom-to-top, flip vertically for standard PNG saving
    std::vector<uint8_t> flipped(width * height * 4);
    for (uint32_t y = 0; y < height; ++y) {
        const uint32_t src_row = (height - 1 - y) * width * 4;
        const uint32_t dst_row = y * width * 4;
        std::memcpy(&flipped[dst_row], &pixels[src_row], width * 4);
    }

    auto sz = IntSize::from_wh(width, height);
    if (!sz) {
        std::cerr << "[Showcase] Invalid image dimensions!" << std::endl;
        return 1;
    }
    PixmapRef pref(flipped.data(), flipped.size(), *sz, PixelFormat::RGBA8888);
    const std::string out_path = "showcase/showcase_gpu_3d_physics.png";
    bool saved = image::save_image_file(pref, out_path);
    if (saved) {
        std::cout << "[Showcase] Successfully saved rendered 3D PBR frame to: " << out_path << std::endl;
    } else {
        std::cerr << "[Showcase] Failed to save output image file: " << out_path << std::endl;
    }

    std::cout << "\n>>> SHOWCASE GPU 3D PHYSICS COMPLETE! <<<" << std::endl;
    return 0;
}

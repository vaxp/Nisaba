#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>

#include "nisaba/gpu/gpu_device.hpp"
#include "orion/orion.hpp"
#include "nisaba/image/image_io.hpp"

using namespace nisaba;
using namespace orion;
using nisaba::gpu::GpuDevice;

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "Nisaba Sovereign 3D Multi-Model PBR Gallery Showcase" << std::endl;
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
    GpuBackend3D gpu_backend(2048);

    auto scene = std::make_shared<Scene3D>();

    // 1. Static Ground Plane
    auto floor_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_plane(30.0f, 20.0f, 10, 10));
    auto floor_mat = std::make_shared<PbrMaterial>();
    floor_mat->base_color_factor = Vec4(0.04f, 0.045f, 0.055f, 1.0f);
    floor_mat->metallic_factor = 0.05f;
    floor_mat->roughness_factor = 0.35f;

    auto r_floor = std::make_shared<Renderable3D>(floor_mesh, floor_mat);
    scene->add_renderable(r_floor);

    // Shared sphere geometry
    auto sphere_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_sphere(0.85f, 64, 32));

    // 2. Material 1: Clear Coat (Lacquered Automotive Ruby)
    auto coat_mat = PbrMaterial::create_clear_coat(Vec4(0.85f, 0.05f, 0.08f, 1.0f), 1.0f, 0.03f);
    coat_mat->metallic_factor = 0.15f;
    coat_mat->roughness_factor = 0.35f;
    auto r_coat = std::make_shared<Renderable3D>(sphere_mesh, coat_mat);
    r_coat->set_transform(Mat44::translation(Vec3(-5.0f, 0.95f, 0.0f)));
    scene->add_renderable(r_coat);

    // 3. Material 2: Anisotropic (Brushed Titanium)
    auto aniso_mat = PbrMaterial::create_anisotropic(Vec4(0.92f, 0.92f, 0.95f, 1.0f), 0.85f, 0.2f);
    auto r_aniso = std::make_shared<Renderable3D>(sphere_mesh, aniso_mat);
    r_aniso->set_transform(Mat44::translation(Vec3(-3.0f, 0.95f, 0.0f)));
    scene->add_renderable(r_aniso);

    // 4. Material 3: Standard PBR (Polished Royal Gold)
    auto gold_mat = PbrMaterial::create_gold();
    gold_mat->roughness_factor = 0.15f;
    auto r_gold = std::make_shared<Renderable3D>(sphere_mesh, gold_mat);
    r_gold->set_transform(Mat44::translation(Vec3(-1.0f, 0.95f, 0.0f)));
    scene->add_renderable(r_gold);

    // 5. Material 4: Cloth / Velvet (Imperial Amethyst with Charlie Sheen)
    auto cloth_mat = PbrMaterial::create_cloth(
        Vec4(0.25f, 0.06f, 0.45f, 1.0f),
        Vec3(0.9f, 0.7f, 1.0f),
        0.35f
    );
    auto r_cloth = std::make_shared<Renderable3D>(sphere_mesh, cloth_mat);
    r_cloth->set_transform(Mat44::translation(Vec3(1.0f, 0.95f, 0.0f)));
    scene->add_renderable(r_cloth);

    // 6. Material 5: Subsurface Scattering (Alabaster Jade Marble)
    auto sss_mat = PbrMaterial::create_subsurface(
        Vec4(0.9f, 0.78f, 0.7f, 1.0f),
        Vec3(0.95f, 0.35f, 0.2f),
        0.4f
    );
    auto r_sss = std::make_shared<Renderable3D>(sphere_mesh, sss_mat);
    r_sss->set_transform(Mat44::translation(Vec3(3.0f, 0.95f, 0.0f)));
    scene->add_renderable(r_sss);

    // 7. Material 6: Transmission (Emerald Optical Glass)
    auto glass_mat = PbrMaterial::create_glass(1.54f, Vec3(0.2f, 0.9f, 0.4f));
    glass_mat->base_color_factor = Vec4(0.7f, 0.98f, 0.85f, 0.2f);
    glass_mat->roughness_factor = 0.04f;
    auto r_glass = std::make_shared<Renderable3D>(sphere_mesh, glass_mat);
    r_glass->set_transform(Mat44::translation(Vec3(5.0f, 0.95f, 0.0f)));
    scene->add_renderable(r_glass);

    // 8. Material 7: Unlit (Cyan Hologram Torus)
    auto torus_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_torus(1.1f, 0.15f, 32, 16));
    auto unlit_mat = PbrMaterial::create_unlit(Vec4(0.0f, 0.8f, 1.0f, 1.0f), Vec3(0.1f, 0.85f, 1.0f));
    auto r_unlit = std::make_shared<Renderable3D>(torus_mesh, unlit_mat);
    r_unlit->set_transform(Mat44::translation(Vec3(0.0f, 3.2f, -1.0f)) * Mat44::rotation(Quat::from_axis_angle(Vec3(1.0f, 0.0f, 0.0f), 0.4f)));
    scene->add_renderable(r_unlit);

    // 8.5 Setup Sovereign Image-Based Lighting (IBL) Atmospheric Dome
    auto sky_env = EnvironmentMap::create_procedural_sky(
        Vec3(-0.45f, 0.9f, 0.55f).normalized(),
        Vec3(1.0f, 0.98f, 0.92f),
        4.8f,
        256
    );
    scene->set_environment(sky_env, 1.25f);

    // 9. Setup Lighting (Sun + Clustered Dynamic Lights)
    Light3D sun = Light3D::create_directional(
        Vec3(-0.45f, -0.9f, -0.55f).normalized(),
        Vec3(1.0f, 0.98f, 0.92f),
        3.0f
    );
    scene->add_light(sun);

    // Dynamic Sphere Area Light (Warm Amber) over titanium
    Light3D sphere_light = Light3D::create_sphere(
        Vec3(-3.0f, 2.8f, 1.0f),
        0.4f, // 40cm sphere radius
        6.0f,
        Vec3(1.0f, 0.75f, 0.4f),
        15.0f
    );
    scene->add_light(sphere_light);

    // Dynamic Point Light (Cyan) hovering above velvet
    Light3D point_light = Light3D::create_point(
        Vec3(1.0f, 2.4f, 1.2f),
        6.0f,
        Vec3(0.1f, 0.8f, 1.0f),
        15.0f
    );
    scene->add_light(point_light);

    // Dynamic Spot Light (Magenta) beaming down on glass
    Light3D spot_light = Light3D::create_spot(
        Vec3(5.0f, 4.0f, 0.5f),
        Vec3(0.0f, -1.0f, -0.1f),
        0.35f, 0.6f, // inner/outer cone
        8.0f,
        Vec3(1.0f, 0.2f, 0.8f),
        20.0f
    );
    scene->add_light(spot_light);

    scene->ambient_color = Vec3(0.08f, 0.09f, 0.12f);

    // 10. Setup Camera
    auto camera = std::make_shared<Camera3D>();
    camera->set_look_at(Vec3(0.0f, 3.6f, 9.8f), Vec3(0.0f, 1.3f, 0.0f), Vec3::up());
    camera->set_perspective(45.0f, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);

    View3D view;
    view.set_scene(scene);
    view.set_camera(camera);
    view.set_viewport(0, 0, width, height);
    view.set_clear_color(Vec4(0.02f, 0.025f, 0.035f, 1.0f));
    view.shadows_enabled = true;
    view.shadow_distance = 40.0f;
    view.shadow_settings.enable_pcss = true;
    view.shadow_settings.enable_contact_shadows = true;

    // 10.5 Configure Sovereign Cinematic Post-Processing Pipeline
    view.post_process.enabled = true;
    view.post_process.bloom.enabled = true;
    view.post_process.bloom.threshold = 1.0f;
    view.post_process.bloom.soft_knee = 0.5f;
    view.post_process.bloom.intensity = 0.35f;
    view.post_process.gtao.enabled = true;
    view.post_process.gtao.radius = 1.6f;
    view.post_process.gtao.intensity = 1.1f;
    view.post_process.fog.enabled = true;
    view.post_process.fog.density = 0.005f;
    view.post_process.fog.height_falloff = 0.15f;
    view.post_process.fog.height = 0.0f;
    view.post_process.fog.start_distance = 8.0f;
    view.post_process.grading.enabled = true;
    view.post_process.grading.exposure = 1.05f;
    view.post_process.grading.contrast = 1.05f;
    view.post_process.grading.saturation = 1.05f;

    std::cout << "[GPU] Rendering 3D Multi-Model PBR Scene with Cinematic Post-Processing..." << std::endl;
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
    const std::string out_path = "showcase/showcase_advanced_pbr.png";
    bool saved = image::save_image_file(pref, out_path);
    if (saved) {
        std::cout << "[Showcase] Successfully saved rendered frame to: " << out_path << std::endl;
    } else {
        std::cerr << "[Showcase] Failed to save output image file: " << out_path << std::endl;
        return 1;
    }

    std::cout << "\n>>> SHOWCASE ADVANCED PBR COMPLETE! <<<" << std::endl;
    return 0;
}

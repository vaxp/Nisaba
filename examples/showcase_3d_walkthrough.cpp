#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <algorithm>
#include <filesystem>
#include <cstring>
#include <cstdio>

#include "orion/orion.hpp"
#include "orion/physics/physics.hpp"
#include "orion/gltf/gltf_loader.hpp"
#include "orion/gpu/gpu_backend3d.hpp"
#include "orion/camera/orbit_controller.hpp"

#include "nisaba/gpu/context.hpp"
#include "nisaba/gpu/gl3_renderer.hpp"
#include "nisaba/canvas/pixmap.hpp"

#ifdef NISABA_GLEW
#   include <GL/glew.h>
#else
#   include <GL/gl.h>
#endif

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/window.hpp"
#include <EGL/egl.h>

using namespace nisaba;
using namespace orion;

namespace {

constexpr int DEFAULT_WIDTH  = 1600;
constexpr int DEFAULT_HEIGHT = 900;

static const char* resolveFont(const char* name) {
    static char resolvedPaths[4][512];
    static int nextIdx = 0;
    char* outBuf = resolvedPaths[nextIdx++ % 4];

    const char* prefixes[] = {
        "fonts/",
        "nisaba/fonts/",
        "../fonts/",
        "../../fonts/",
        "../../../fonts/"
    };

    for (const auto& prefix : prefixes) {
        std::snprintf(outBuf, 512, "%s%s", prefix, name);
        FILE* f = std::fopen(outBuf, "rb");
        if (f) {
            std::fclose(f);
            return outBuf;
        }
    }
    return name;
}

// ─────────────────────────────────────────────────────────────────
// First-Person Walk & Free-Fly Camera Controller
// ─────────────────────────────────────────────────────────────────
struct FirstPersonCamera {
    Vec3 position{ -5.5f, 1.8f, -0.1f };
    float yaw{ 0.0f };      // Radians: 0.0 = looking down +X
    float pitch{ 0.14f };   // Radians: clamped between -1.50 and +1.50
    float move_speed{ 6.5f }; // meters per second
    float look_sensitivity{ 0.0035f };
    bool fly_mode{ true };

    [[nodiscard]] Vec3 forward() const noexcept {
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);
        const float cp = std::cos(pitch);
        const float sp = std::sin(pitch);
        return Vec3(cy * cp, sp, sy * cp).normalized();
    }

    [[nodiscard]] Vec3 forward_horizontal() const noexcept {
        const float cy = std::cos(yaw);
        const float sy = std::sin(yaw);
        return Vec3(cy, 0.0f, sy).normalized();
    }

    [[nodiscard]] Vec3 right() const noexcept {
        return forward().cross(Vec3::up()).normalized();
    }

    void update(Camera3D& camera) const noexcept {
        camera.set_look_at(position, position + forward(), Vec3::up());
    }
};

enum class CameraMode {
    ThirdPersonPlayer,
    FirstPersonWalk,
    Orbit
};

struct PlayerCharacter {
    Vec3 position{0.0f, 0.0f, 0.0f};
    float facing_yaw{0.0f};
    float model_facing_offset{0.0f};
    float move_speed{6.5f};
    float sprint_speed{16.0f};
    float velocity_y{0.0f};
    bool is_grounded{true};
    float ground_y{0.0f};
    float model_scale{1.15f};

    // Third-person chase camera
    float cam_yaw{0.0f};
    float cam_pitch{0.25f};
    float cam_distance{4.5f};
    float cam_target_height{1.35f};
};

} // anonymous namespace

int main(int argc, char** argv) {
    std::cout << "==========================================================\n"
              << "Nisaba Sovereign 3D Real-Time Interactive Walkthrough\n"
              << "Map & Model Explorer — Native Desktop Hardware Window\n"
              << "==========================================================\n";

    // 1. Resolve 3D Model File Path
    std::string model_path = "third_party/models/Sponza/sponza.glb";
    bool headless = false;
    bool sim_walk = false;
    int target_frames = -1;
    std::string snapshot_path = "showcase/showcase_walkthrough.png";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--headless") {
            headless = true;
        } else if (arg == "--walk") {
            sim_walk = true;
        } else if (arg == "--frames" && i + 1 < argc) {
            target_frames = std::atoi(argv[++i]);
        } else if (arg == "--snapshot" && i + 1 < argc) {
            snapshot_path = argv[++i];
        } else if (arg.rfind("--", 0) != 0) {
            model_path = arg;
        }
    }

    if (!std::filesystem::exists(model_path)) {
        // Fallback search
        const std::vector<std::string> fallbacks = {
            "third_party/models/Sponza/sponza.glb",
            "third_party/models/Lantern/Lantern.glb",
            "third_party/models/nativeStaticScene.glb"
        };
        for (const auto& fb : fallbacks) {
            if (std::filesystem::exists(fb)) {
                model_path = fb;
                break;
            }
        }
    }

    std::cout << "[Model] Target Asset: " << model_path << "\n";
    if (!std::filesystem::exists(model_path)) {
        std::cerr << "[Error] Asset file does not exist: " << model_path << "\n";
        return 1;
    }

    // 2. Initialize Native Linux Platform Subsystem
    auto plat_res = nisaba::backend_os::Platform::create();
    if (!plat_res.isOk()) {
        std::cerr << "[Error] Failed to initialize Nisaba native platform: "
                  << plat_res.error().message << "\n";
        return 1;
    }
    auto platform = std::move(plat_res.value());

    // 3. Create Hardware-Accelerated Native Window
    nisaba::backend_os::WindowConfig win_cfg;
    win_cfg.title = "Nisaba Sovereign 3D Engine — Interactive Map Walkthrough";
    win_cfg.width = DEFAULT_WIDTH;
    win_cfg.height = DEFAULT_HEIGHT;
    win_cfg.resizable = true;
    win_cfg.vsync = false;
    win_cfg.transparent = false;

    auto win_res = nisaba::backend_os::Window::create(*platform, win_cfg);
    if (!win_res.isOk()) {
        std::cerr << "[Error] Failed to create Nisaba native window: "
                  << win_res.error().message << "\n";
        return 1;
    }
    auto window = std::move(win_res.value());
    window->makeCurrent();

    EGLDisplay egl_dpy = eglGetCurrentDisplay();
    if (egl_dpy != EGL_NO_DISPLAY) {
        eglSwapInterval(egl_dpy, 0); // Start uncapped for raw performance
    }

#ifdef NISABA_GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK && err != GLEW_ERROR_NO_GLX_DISPLAY) {
        std::fprintf(stderr, "GLEW notice: %s\n", (const char*)glewGetErrorString(err));
    }
    while (glGetError() != GL_NO_ERROR) {}
#endif

    std::cout << "[GPU] Active Hardware Device: " << glGetString(GL_RENDERER)
              << " (" << glGetString(GL_VERSION) << ")\n";

    // 4. Create Nisaba 2D GPU HUD Context for UI Overlay Fusion
    auto hud_ctx = nisaba::gpu::createContextGL3(nisaba::gpu::CreateFlags::Antialias);
    int font_sans = hud_ctx ? hud_ctx->createFont("sans", resolveFont("Inter-Regular.ttf")) : -1;
    if (font_sans == -1 && hud_ctx) font_sans = hud_ctx->createFont("sans", resolveFont("NotoSans-Regular.ttf"));
    int font_mono = hud_ctx ? hud_ctx->createFont("mono", resolveFont("FiraMono-Medium.ttf")) : -1;
    int font_arabic = hud_ctx ? hud_ctx->createFont("arabic", resolveFont("NotoSansArabic.ttf")) : -1;
    if (font_sans != -1 && font_arabic != -1 && hud_ctx) hud_ctx->addFallbackFontId(font_sans, font_arabic);
    if (font_mono != -1 && font_arabic != -1 && hud_ctx) hud_ctx->addFallbackFontId(font_mono, font_arabic);

    // 5. Ingest 3D Model Asset via Sovereign glTF Loader
    std::cout << "[Model] Ingesting glTF asset hierarchy...\n";
    auto load_result = GltfLoader::load_from_file(model_path);
    if (!load_result.success || !load_result.scene) {
        std::cerr << "[Error] glTF loading failed: " << load_result.error_message << "\n";
        return 1;
    }

    size_t total_verts = 0;
    size_t total_tris = 0;
    Vec3 min_pt( 1e9f,  1e9f,  1e9f);
    Vec3 max_pt(-1e9f, -1e9f, -1e9f);

    for (const auto& r : load_result.renderables) {
        if (!r || !r->mesh) continue;
        const auto& mesh = *r->mesh;
        total_verts += mesh.vertices.size();
        total_tris += mesh.indices.size() / 3;

        for (const auto& v : mesh.vertices) {
            const Vec3 p = r->transform.transform_point(v.position);
            min_pt = min_pt.min(p);
            max_pt = max_pt.max(p);
        }
    }

    const Vec3 center = (min_pt + max_pt) * 0.5f;
    const Vec3 extents = max_pt - min_pt;
    float radius = std::max({extents.x, extents.y, extents.z}) * 0.5f;
    if (radius < 1e-4f) radius = 1.0f;

    std::cout << "  - Total Meshes:        " << load_result.meshes.size() << "\n"
              << "  - Total Materials:     " << load_result.materials.size() << "\n"
              << "  - Total Textures:      " << load_result.textures.size() << "\n"
              << "  - Total Geometry:      " << total_verts << " vertices, " << total_tris << " triangles\n"
              << "  - Scene Center:        (" << center.x << ", " << center.y << ", " << center.z << ")\n"
              << "  - Scene Extents:       (" << extents.x << ", " << extents.y << ", " << extents.z << ")\n"
              << "  - Bounding Radius:     " << radius << "\n";

    // 6. Setup Lighting & Atmospheric Environment
    auto scene = load_result.scene;
    scene->lights.clear();

    const bool is_sponza = (model_path.find("sponza") != std::string::npos ||
                            model_path.find("Sponza") != std::string::npos);
    const bool is_village = (model_path.find("village") != std::string::npos ||
                             model_path.find("Village") != std::string::npos);
    const bool is_city = (model_path.find("city") != std::string::npos ||
                          model_path.find("City") != std::string::npos);
    const bool is_castle = (model_path.find("castle") != std::string::npos ||
                            model_path.find("Castle") != std::string::npos);
    const bool is_environment = is_sponza || is_village || is_city || is_castle || (radius > 5.0f) ||
                                (model_path.find("dungeon") != std::string::npos) ||
                                (model_path.find("Dungeon") != std::string::npos) ||
                                (model_path.find("hallway") != std::string::npos) ||
                                (model_path.find("house") != std::string::npos);

    if (is_environment) {
        // Natural sunlight pouring across the environment
        Light3D sun_light = Light3D::create_directional(
            Vec3(-0.35f, -0.88f, 0.32f).normalized(),
            Vec3(1.0f, 0.98f, 0.94f),
            is_sponza ? 3.8f : 5.0f
        );
        scene->add_light(sun_light);
        scene->ambient_color = is_sponza ? Vec3(0.55f, 0.58f, 0.65f) : Vec3(0.40f, 0.42f, 0.48f);
    } else {
        // Three-point cinematic lighting for standalone models
        Light3D key = Light3D::create_directional(
            Vec3(-0.6f, -0.7f, -0.5f).normalized(),
            Vec3(1.0f, 0.98f, 0.94f),
            4.8f
        );
        scene->add_light(key);
        Light3D fill = Light3D::create_directional(
            Vec3(0.7f, -0.3f, 0.4f).normalized(),
            Vec3(0.6f, 0.75f, 0.95f),
            1.8f
        );
        scene->add_light(fill);
        scene->ambient_color = Vec3(0.35f, 0.38f, 0.45f);
    }

    // 7. Initialize Player Character & Camera Controllers
    std::string character_path = "third_party/models/Characters/Xbot.glb";
    if (!std::filesystem::exists(character_path)) {
        character_path = "third_party/models/Characters/Soldier.glb";
    }
    if (!std::filesystem::exists(character_path)) {
        character_path = "third_party/models/Characters/CesiumMan.glb";
    }
    std::vector<std::shared_ptr<Renderable3D>> player_renderables;
    std::vector<Mat44> player_base_transforms;
    std::shared_ptr<Animator> player_animator;
    bool has_player = false;
    PlayerCharacter player;

    const bool is_xbot = (character_path.find("Xbot") != std::string::npos);
    if (is_xbot) {
        player.model_scale = 1.0f;
        player.model_facing_offset = 0.0f;
    }

    if (std::filesystem::exists(character_path) && is_environment) {
        auto char_load = GltfLoader::load_from_file(character_path);
        if (char_load.success && char_load.scene) {
            has_player = true;
            for (auto& r : char_load.renderables) {
                if (r) {
                    player_base_transforms.push_back(r->transform);
                    scene->add_renderable(r);
                    player_renderables.push_back(r);
                }
            }
            player_animator = char_load.animator;
            std::cout << "[Player] 3D Controllable Character Loaded: " << character_path
                      << " (" << player_renderables.size() << " sub-renderables)\n";
            if (player_animator) {
                std::cout << "[Player Animation] Skeletal Animator Active: "
                          << player_animator->clips.size() << " clips, "
                          << player_animator->skinned_meshes.size() << " skinned mesh instances.\n";
                for (size_t i = 0; i < player_animator->clips.size(); ++i) {
                    std::cout << "  - Clip " << i << ": \"" << player_animator->clips[i]->name
                              << "\" (Duration: " << player_animator->clips[i]->duration << "s)\n";
                }

                // Start immediately in idle animation
                int32_t idle_idx = player_animator->find_clip("idle");
                if (idle_idx < 0) idle_idx = player_animator->find_clip("Idle");
                if (idle_idx >= 0) {
                    player_animator->play(static_cast<size_t>(idle_idx), true, 1.0f);
                }
            }
        }
    }

    FirstPersonCamera fps_cam;
    OrbitCameraController orbit_cam;

    if (is_sponza) {
        player.ground_y = 0.0f;
        player.position = Vec3(-5.5f, player.ground_y, -0.1f);
        player.facing_yaw = 1.5707963f; // Look toward +X down long arcade
        player.cam_yaw = -1.5707963f;   // Chase camera behind player
        player.cam_pitch = 0.22f;
        player.cam_distance = 3.6f;

        fps_cam.position = Vec3(-5.5f, 1.8f, -0.1f);
        fps_cam.yaw = 0.0f;
        fps_cam.pitch = 0.12f;
        fps_cam.move_speed = 7.0f;

        orbit_cam.target = center;
        orbit_cam.distance = radius * 1.5f;
        orbit_cam.yaw = 0.7f;
        orbit_cam.pitch = 0.35f;
    } else if (is_village) {
        player.ground_y = min_pt.y;
        player.position = Vec3(0.0f, player.ground_y, 22.0f);
        player.facing_yaw = 3.14159265f; // Face forward towards village houses
        player.cam_yaw = 0.0f;           // Camera behind looking toward -Z
        player.cam_pitch = 0.22f;
        player.cam_distance = 4.5f;

        fps_cam.position = Vec3(0.0f, min_pt.y + 1.8f, 22.0f);
        fps_cam.yaw = -1.5707963f;
        fps_cam.pitch = 0.05f;
        fps_cam.move_speed = 10.0f;

        orbit_cam.target = center;
        orbit_cam.distance = radius * 1.6f;
        orbit_cam.yaw = 0.8f;
        orbit_cam.pitch = 0.4f;
    } else if (is_city) {
        player.ground_y = 0.0f; // Highway street tarmac level
        player.position = Vec3(0.0f, player.ground_y, 16.0f);
        player.facing_yaw = 3.14159265f; // Face forward down the cyber avenue
        player.cam_yaw = 0.0f;           // Chase camera behind looking along road
        player.cam_pitch = 0.22f;
        player.cam_distance = 5.2f;
        player.cam_target_height = 1.6f;
        player.model_scale = 1.35f;

        fps_cam.position = Vec3(0.0f, 1.8f, 16.0f);
        fps_cam.yaw = -1.5707963f;
        fps_cam.pitch = 0.05f;
        fps_cam.move_speed = 14.0f;

        orbit_cam.target = center;
        orbit_cam.distance = radius * 1.5f;
        orbit_cam.yaw = 0.5f;
        orbit_cam.pitch = 0.3f;
    } else if (is_environment) {
        player.ground_y = min_pt.y;
        player.position = Vec3(center.x, player.ground_y, center.z + std::min(radius * 0.25f, 15.0f));
        player.facing_yaw = 3.14159265f;
        player.cam_yaw = 0.0f;
        player.cam_pitch = 0.22f;
        player.cam_distance = 5.0f;

        fps_cam.position = Vec3(center.x, min_pt.y + 1.75f, center.z + std::min(radius * 0.3f, 12.0f));
        fps_cam.yaw = -1.5707963f;
        fps_cam.pitch = 0.05f;
        fps_cam.move_speed = std::clamp(radius * 0.15f, 5.0f, 25.0f);

        orbit_cam.target = center;
        orbit_cam.distance = radius * 1.8f;
        orbit_cam.yaw = 0.5f;
        orbit_cam.pitch = 0.3f;
    } else {
        fps_cam.position = center + Vec3(0.0f, radius * 0.4f, radius * 2.2f);
        fps_cam.yaw = -1.5707963f; // Look toward -Z
        fps_cam.pitch = 0.0f;
        fps_cam.move_speed = radius * 1.5f;

        orbit_cam.target = center;
        orbit_cam.distance = radius * 2.2f;
        orbit_cam.yaw = 0.5f;
        orbit_cam.pitch = 0.25f;
    }

    CameraMode current_cam_mode = (has_player && is_environment)
        ? CameraMode::ThirdPersonPlayer
        : (is_environment ? CameraMode::FirstPersonWalk : CameraMode::Orbit);

    auto camera = std::make_shared<Camera3D>();

    // -----------------------------------------------------------------
    // 8. Sovereign 3D Physics Simulation & Environment Colliders
    // -----------------------------------------------------------------
    orion::physics::PhysicsSettings phys_settings;
    phys_settings.gravity = orion::Vec3(0.0f, -14.0f, 0.0f);
    orion::physics::PhysicsScene physics_scene(phys_settings);
    PhysicsRenderBridge physics_bridge;

    // Retain collision shapes in memory
    std::vector<std::shared_ptr<orion::physics::Shape>> collision_shapes;

    auto add_static_box = [&](const Vec3& pos, const Vec3& half_size) {
        auto shape = std::make_shared<orion::physics::BoxShape>(
            orion::Vec3(half_size.x, half_size.y, half_size.z)
        );
        collision_shapes.push_back(shape);
        orion::physics::BodyCreationSettings b_set;
        b_set.position = orion::Vec3(pos.x, pos.y, pos.z);
        b_set.shape = shape.get();
        b_set.motion_type = orion::physics::MotionType::Static;
        b_set.friction = 0.5f;
        (void)physics_scene.create_body(b_set);
    };

    auto add_static_cylinder = [&](const Vec3& pos, float half_height, float radius_val) {
        auto shape = std::make_shared<orion::physics::CylinderShape>(half_height, radius_val);
        collision_shapes.push_back(shape);
        orion::physics::BodyCreationSettings b_set;
        b_set.position = orion::Vec3(pos.x, pos.y, pos.z);
        b_set.shape = shape.get();
        b_set.motion_type = orion::physics::MotionType::Static;
        b_set.friction = 0.5f;
        (void)physics_scene.create_body(b_set);
    };

    if (is_sponza) {
        // Ground floor
        add_static_box(Vec3(0.0f, -0.5f, 0.0f), Vec3(25.0f, 0.5f, 15.0f));

        // Perimeter Walls
        // North Wall (back lion relief)
        add_static_box(Vec3(12.2f, 4.0f, 0.0f), Vec3(0.6f, 6.0f, 8.0f));
        // South Wall (front entrance)
        add_static_box(Vec3(-13.2f, 4.0f, 0.0f), Vec3(0.6f, 6.0f, 8.0f));
        // East Wall (behind banners)
        add_static_box(Vec3(0.0f, 4.0f, 5.4f), Vec3(14.0f, 6.0f, 0.6f));
        // West Wall (behind banners)
        add_static_box(Vec3(0.0f, 4.0f, -5.9f), Vec3(14.0f, 6.0f, 0.6f));

        // Columns / Pillars lining the central arcade
        const float pillar_x_coords[] = { -9.8f, -5.9f, -2.0f, 1.9f, 5.8f, 9.7f };
        for (float px : pillar_x_coords) {
            add_static_cylinder(Vec3(px, 2.5f, -1.95f), 3.0f, 0.38f);
            add_static_cylinder(Vec3(px, 2.5f,  1.95f), 3.0f, 0.38f);
        }

        // Add Dynamic Physics Rigid Bodies along the hallway (interactive pushable objects!)
        auto gold_mat = PbrMaterial::create_gold();
        gold_mat->metallic_factor = 0.90f;
        gold_mat->roughness_factor = 0.20f;

        auto red_mat = std::make_shared<PbrMaterial>();
        red_mat->base_color_factor = Vec4(0.85f, 0.15f, 0.15f, 1.0f);
        red_mat->metallic_factor = 0.2f;
        red_mat->roughness_factor = 0.3f;

        auto blue_mat = std::make_shared<PbrMaterial>();
        blue_mat->base_color_factor = Vec4(0.15f, 0.35f, 0.85f, 1.0f);
        blue_mat->metallic_factor = 0.4f;
        blue_mat->roughness_factor = 0.25f;

        auto emerald_mat = std::make_shared<PbrMaterial>();
        emerald_mat->base_color_factor = Vec4(0.1f, 0.75f, 0.3f, 1.0f);
        emerald_mat->metallic_factor = 0.1f;
        emerald_mat->roughness_factor = 0.3f;

        // Dynamic Crate 1
        auto crate_shape1 = std::make_shared<orion::physics::BoxShape>(orion::Vec3(0.35f, 0.35f, 0.35f));
        collision_shapes.push_back(crate_shape1);
        orion::physics::BodyCreationSettings c1_set;
        c1_set.position = orion::Vec3(-2.8f, 0.6f, -0.6f);
        c1_set.shape = crate_shape1.get();
        c1_set.motion_type = orion::physics::MotionType::Dynamic;
        c1_set.override_mass = 12.0f;
        c1_set.friction = 0.5f;
        c1_set.restitution = 0.15f;
        auto c1_id = physics_scene.create_body(c1_set);
        (void)physics_bridge.create_and_bind(physics_scene, c1_id, red_mat, scene.get());

        // Dynamic Crate 2
        auto crate_shape2 = std::make_shared<orion::physics::BoxShape>(orion::Vec3(0.35f, 0.35f, 0.35f));
        collision_shapes.push_back(crate_shape2);
        orion::physics::BodyCreationSettings c2_set;
        c2_set.position = orion::Vec3(1.5f, 0.6f, 0.7f);
        c2_set.shape = crate_shape2.get();
        c2_set.motion_type = orion::physics::MotionType::Dynamic;
        c2_set.override_mass = 12.0f;
        c2_set.friction = 0.5f;
        c2_set.restitution = 0.15f;
        auto c2_id = physics_scene.create_body(c2_set);
        (void)physics_bridge.create_and_bind(physics_scene, c2_id, blue_mat, scene.get());

        // Dynamic Gold Ball
        auto ball_shape = std::make_shared<orion::physics::SphereShape>(0.42f);
        collision_shapes.push_back(ball_shape);
        orion::physics::BodyCreationSettings b_set;
        b_set.position = orion::Vec3(-0.5f, 0.8f, 0.5f);
        b_set.shape = ball_shape.get();
        b_set.motion_type = orion::physics::MotionType::Dynamic;
        b_set.override_mass = 8.0f;
        b_set.friction = 0.3f;
        b_set.restitution = 0.65f;
        auto b_id = physics_scene.create_body(b_set);
        (void)physics_bridge.create_and_bind(physics_scene, b_id, gold_mat, scene.get());

        // Dynamic Emerald Cylinder
        auto cyl_shape = std::make_shared<orion::physics::CylinderShape>(0.45f, 0.30f);
        collision_shapes.push_back(cyl_shape);
        orion::physics::BodyCreationSettings cyl_set;
        cyl_set.position = orion::Vec3(3.8f, 0.8f, -0.5f);
        cyl_set.shape = cyl_shape.get();
        cyl_set.motion_type = orion::physics::MotionType::Dynamic;
        cyl_set.override_mass = 15.0f;
        cyl_set.friction = 0.45f;
        cyl_set.restitution = 0.2f;
        auto cyl_id = physics_scene.create_body(cyl_set);
        (void)physics_bridge.create_and_bind(physics_scene, cyl_id, emerald_mat, scene.get());

        std::cout << "[Physics] Sponza Palace Colliders Initialized:\n"
                  << "  - 4 Perimeter Walls & Ground Plane\n"
                  << "  - 12 Arcade Stone Column Colliders\n"
                  << "  - 4 Dynamic Pushable Interactive Rigid Bodies\n";
    } else {
        // Fallback ground and boundaries for any other map
        add_static_box(Vec3(center.x, min_pt.y - 0.5f, center.z), Vec3(radius * 2.0f, 0.5f, radius * 2.0f));
    }

    // 9. Hardware 3D GPU Backend
    GpuBackend3D gpu_backend(2048); // 2048x2048 Cascaded Shadow Map

    // 9. Input State Tracking
    bool key_w = false, key_a = false, key_s = false, key_d = false;
    bool key_space = false, key_c = false, key_shift = false;
    bool key_left = false, key_right = false, key_up = false, key_down = false;

    bool mouse_dragging = false;
    float last_mouse_x = 0.0f;
    float last_mouse_y = 0.0f;

    bool running = true;
    bool vsync_enabled = false;
    bool show_hud = true;

    window->onClose().connect([&]() {
        running = false;
    });

    auto handle_key = [&](int key, bool pressed) {
        if (key == 'w' || key == 'W') key_w = pressed;
        else if (key == 'a' || key == 'A') key_a = pressed;
        else if (key == 's' || key == 'S') key_s = pressed;
        else if (key == 'd' || key == 'D') key_d = pressed;
        else if (key == ' ' || key == 32) key_space = pressed;
        else if (key == 'c' || key == 'C') key_c = pressed;
        else if (key == 0xffe1 || key == 0xffe2 || key == 65505 || key == 65506) key_shift = pressed;
        else if (key == 0xff51 || key == 65361) key_left = pressed;
        else if (key == 0xff52 || key == 65362) key_up = pressed;
        else if (key == 0xff53 || key == 65363) key_right = pressed;
        else if (key == 0xff54 || key == 65364) key_down = pressed;
    };

    platform->onKeyDown().connect([&](int key, int) {
        if (key == 27 || key == 0xff1b || key == 'q' || key == 'Q') {
            running = false;
        } else if (key == 0xff09 || key == 9) { // Tab: cycle camera mode
            if (has_player) {
                if (current_cam_mode == CameraMode::ThirdPersonPlayer) current_cam_mode = CameraMode::FirstPersonWalk;
                else if (current_cam_mode == CameraMode::FirstPersonWalk) current_cam_mode = CameraMode::Orbit;
                else current_cam_mode = CameraMode::ThirdPersonPlayer;
            } else {
                current_cam_mode = (current_cam_mode == CameraMode::FirstPersonWalk)
                    ? CameraMode::Orbit
                    : CameraMode::FirstPersonWalk;
            }
            std::cout << "[*] Camera Mode Switched: "
                      << (current_cam_mode == CameraMode::ThirdPersonPlayer ? "THIRD-PERSON CHARACTER [GAME MODE]" :
                          current_cam_mode == CameraMode::FirstPersonWalk ? "FIRST-PERSON WALK/FLY" : "ORBIT INSPECTION")
                      << "\n";
        } else if (key == 'v' || key == 'V') {
            vsync_enabled = !vsync_enabled;
            EGLDisplay dpy = eglGetCurrentDisplay();
            if (dpy != EGL_NO_DISPLAY) {
                eglSwapInterval(dpy, vsync_enabled ? 1 : 0);
            }
            std::cout << "[*] VSync toggled: " << (vsync_enabled ? "ON (60Hz)" : "OFF (UNCAPPED RAW)") << "\n";
        } else if (key == 'h' || key == 'H') {
            show_hud = !show_hud;
        } else if (key == 'f' || key == 'F') {
            fps_cam.fly_mode = !fps_cam.fly_mode;
            std::cout << "[*] Walk/Fly Mode: " << (fps_cam.fly_mode ? "FREE 3D FLIGHT" : "GROUND LOCKED WALK") << "\n";
        }
        handle_key(key, true);
    });

    platform->onKeyUp().connect([&](int key, int) {
        handle_key(key, false);
    });

    platform->onMouseDown().connect([&](float x, float y, int btn) {
        if (btn == 1 || btn == 3) {
            mouse_dragging = true;
            last_mouse_x = x;
            last_mouse_y = y;
        }
    });

    platform->onMouseUp().connect([&](float, float, int btn) {
        if (btn == 1 || btn == 3) {
            mouse_dragging = false;
        }
    });

    platform->onMouseMove().connect([&](float x, float y) {
        if (mouse_dragging) {
            float dx = x - last_mouse_x;
            float dy = y - last_mouse_y;
            last_mouse_x = x;
            last_mouse_y = y;

            if (current_cam_mode == CameraMode::ThirdPersonPlayer) {
                player.cam_yaw += dx * 0.004f;
                player.cam_pitch = std::clamp(player.cam_pitch - dy * 0.003f, -0.3f, 1.2f);
            } else if (current_cam_mode == CameraMode::FirstPersonWalk) {
                fps_cam.yaw += dx * fps_cam.look_sensitivity;
                fps_cam.pitch = std::clamp(fps_cam.pitch - dy * fps_cam.look_sensitivity, -1.50f, 1.50f);
            } else {
                orbit_cam.orbit(dx * 0.005f, -dy * 0.005f);
            }
        }
    });

    platform->onScroll().connect([&](float, float dy) {
        if (current_cam_mode == CameraMode::ThirdPersonPlayer) {
            player.cam_distance = std::clamp(player.cam_distance - dy * 0.5f, 1.5f, 25.0f);
        } else if (current_cam_mode == CameraMode::FirstPersonWalk) {
            fps_cam.move_speed = std::clamp(fps_cam.move_speed + dy * 0.8f, 1.0f, 80.0f);
        } else {
            orbit_cam.zoom(-dy * (radius * 0.1f));
        }
    });

    std::cout << "\n==========================================================\n"
              << "Nisaba 3D Interactive Walkthrough Window Live!\n"
              << "Controls:\n"
              << "  [W/A/S/D]        : Walk Forward / Strafe Left / Back / Right\n"
              << "  [Space / C]      : Fly Upward / Downward\n"
              << "  [Mouse Drag]     : Look around (Yaw / Pitch free view)\n"
              << "  [Arrow Keys]     : Turn / Tilt camera\n"
              << "  [Shift]          : Sprint / Fast Boost (3x Speed)\n"
              << "  [Scroll Wheel]   : Increase / Decrease walking speed\n"
              << "  [Tab]            : Toggle First-Person Walk / Orbit Inspection\n"
              << "  [F]              : Toggle Free-Fly vs Ground-Locked Walk\n"
              << "  [V]              : Toggle VSync (Uncapped vs 60Hz)\n"
              << "  [H]              : Toggle HUD visibility\n"
              << "  [ESC / Q]        : Close Window & Quit\n"
              << "==========================================================\n\n";

    double start_time = platform->getTime();
    double prev_time = start_time;
    int frame_count = 0;

    // Stable rolling frame accumulator (updates every 200ms to eliminate jitter)
    double fps_accum_time = 0.0;
    int fps_accum_frames = 0;
    double displayed_fps = 60.0;
    double displayed_ms = 16.6;
    char perf_str[128];
    std::snprintf(perf_str, sizeof(perf_str), "%.1f FPS (%.2f ms) | VSync: %s",
                  displayed_fps, displayed_ms, vsync_enabled ? "ON (60Hz)" : "OFF (UNCAPPED)");

    // Main Interactive Render Loop
    while (running) {
        if (!platform->pollEvents()) {
            break;
        }

        double cur_time = platform->getTime();
        float dt = static_cast<float>(cur_time - prev_time);
        if (dt < 0.0001f) dt = 0.0001f;
        if (dt > 0.1f) dt = 0.1f;
        prev_time = cur_time;
        float time_sec = static_cast<float>(cur_time - start_time);
        (void)time_sec;

        fps_accum_time += static_cast<double>(dt);
        fps_accum_frames++;
        if (fps_accum_time >= 0.20) {
            displayed_fps = static_cast<double>(fps_accum_frames) / fps_accum_time;
            displayed_ms = (fps_accum_time / static_cast<double>(fps_accum_frames)) * 1000.0;
            std::snprintf(perf_str, sizeof(perf_str), "%.1f FPS (%.2f ms) | VSync: %s",
                          displayed_fps, displayed_ms, vsync_enabled ? "ON (60Hz)" : "OFF (UNCAPPED)");
            fps_accum_time = 0.0;
            fps_accum_frames = 0;
        }

        // Update Camera & Character Movement Physics
        if (current_cam_mode == CameraMode::ThirdPersonPlayer) {
            for (auto& r : player_renderables) {
                if (r) r->visible = true;
            }

            // Arrow keys rotate chase camera yaw/pitch
            if (key_left)  player.cam_yaw += 2.0f * dt;
            if (key_right) player.cam_yaw -= 2.0f * dt;
            if (key_up)    player.cam_pitch = std::clamp(player.cam_pitch + 1.4f * dt, -0.3f, 1.3f);
            if (key_down)  player.cam_pitch = std::clamp(player.cam_pitch - 1.4f * dt, -0.3f, 1.3f);

            // Compute camera horizontal forward & right
            const float cy = std::cos(player.cam_yaw);
            const float sy = std::sin(player.cam_yaw);
            const Vec3 cam_fwd(-sy, 0.0f, -cy);
            const Vec3 cam_rgt(cy, 0.0f, -sy);

            // WASD character movement relative to camera orientation
            const bool move_forward = key_w || (sim_walk && frame_count >= 5);
            Vec3 move_dir(0.0f, 0.0f, 0.0f);
            if (move_forward) move_dir += cam_fwd;
            if (key_s) move_dir -= cam_fwd;
            if (key_d) move_dir += cam_rgt;
            if (key_a) move_dir -= cam_rgt;

            // Advance Physics Simulation and Synchronize Renderable Transforms
            const float sim_dt = std::min(dt, 0.033f);
            physics_scene.step(sim_dt);
            physics_bridge.sync(physics_scene);

            float move_len_sq = move_dir.x * move_dir.x + move_dir.z * move_dir.z;
            if (move_len_sq > 0.0001f) {
                float inv_len = 1.0f / std::sqrt(move_len_sq);
                move_dir.x *= inv_len;
                move_dir.z *= inv_len;

                float speed = key_shift ? player.sprint_speed : player.move_speed;
                Vec3 target_vel = move_dir * speed;
                Vec3 delta_pos = target_vel * dt;

                // Collision Detection & Sweep-and-Slide Against All Physics Bodies
                const float char_radius = 0.38f;
                const float char_half_h = 0.85f;
                Vec3 new_pos = player.position + delta_pos;
                Vec3 p_center(new_pos.x, new_pos.y + 0.90f, new_pos.z);

                // Multi-pass collision relaxation
                for (int iter = 0; iter < 3; ++iter) {
                    bool collided = false;
                    for (size_t s_i = 0; s_i < physics_scene.get_body_slot_count(); ++s_i) {
                        const auto* body = physics_scene.get_body_by_slot(static_cast<uint32_t>(s_i));
                        if (!body || !body->get_shape()) continue;

                        const auto* shape = body->get_shape();
                        const auto b_pos = body->get_position();

                        if (shape->get_type() == orion::physics::ShapeType::Box) {
                            const auto* box = static_cast<const orion::physics::BoxShape*>(shape);
                            const auto h = box->half_extents;
                            float cx = std::clamp(p_center.x, b_pos.x - h.x, b_pos.x + h.x);
                            float cy = std::clamp(p_center.y, b_pos.y - h.y, b_pos.y + h.y);
                            float cz = std::clamp(p_center.z, b_pos.z - h.z, b_pos.z + h.z);

                            if (std::abs(p_center.y - cy) < char_half_h) {
                                float dx = p_center.x - cx;
                                float dz = p_center.z - cz;
                                float dist_sq = dx * dx + dz * dz;
                                if (dist_sq < char_radius * char_radius) {
                                    float dist = std::sqrt(dist_sq);
                                    Vec3 normal = (dist > 1e-4f) ? Vec3(dx / dist, 0.0f, dz / dist) : Vec3(1.0f, 0.0f, 0.0f);
                                    float pen = char_radius - dist;
                                    new_pos.x += normal.x * (pen + 0.002f);
                                    new_pos.z += normal.z * (pen + 0.002f);
                                    p_center.x = new_pos.x;
                                    p_center.z = new_pos.z;
                                    collided = true;

                                    if (body->is_dynamic()) {
                                        auto* mut_body = physics_scene.get_body_by_slot(static_cast<uint32_t>(s_i));
                                        if (mut_body) {
                                            mut_body->apply_impulse(orion::Vec3(target_vel.x, 0.4f, target_vel.z) * (18.0f * dt));
                                        }
                                    }
                                }
                            }
                        } else if (shape->get_type() == orion::physics::ShapeType::Cylinder ||
                                   shape->get_type() == orion::physics::ShapeType::Sphere) {
                            float obj_radius = 0.40f;
                            float obj_half_h = 3.0f;
                            if (shape->get_type() == orion::physics::ShapeType::Cylinder) {
                                const auto* cyl = static_cast<const orion::physics::CylinderShape*>(shape);
                                obj_radius = cyl->radius;
                                obj_half_h = cyl->half_height;
                            } else {
                                const auto* sph = static_cast<const orion::physics::SphereShape*>(shape);
                                obj_radius = sph->radius;
                                obj_half_h = sph->radius;
                            }

                            if (std::abs(p_center.y - b_pos.y) < (char_half_h + obj_half_h)) {
                                float dx = p_center.x - b_pos.x;
                                float dz = p_center.z - b_pos.z;
                                float dist_sq = dx * dx + dz * dz;
                                float min_dist = char_radius + obj_radius;
                                if (dist_sq < min_dist * min_dist) {
                                    float dist = std::sqrt(dist_sq);
                                    Vec3 normal = (dist > 1e-4f) ? Vec3(dx / dist, 0.0f, dz / dist) : Vec3(1.0f, 0.0f, 0.0f);
                                    float pen = min_dist - dist;
                                    new_pos.x += normal.x * (pen + 0.002f);
                                    new_pos.z += normal.z * (pen + 0.002f);
                                    p_center.x = new_pos.x;
                                    p_center.z = new_pos.z;
                                    collided = true;

                                    if (body->is_dynamic()) {
                                        auto* mut_body = physics_scene.get_body_by_slot(static_cast<uint32_t>(s_i));
                                        if (mut_body) {
                                            mut_body->apply_impulse(orion::Vec3(target_vel.x, 0.6f, target_vel.z) * (20.0f * dt));
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (!collided) break;
                }
                player.position.x = new_pos.x;
                player.position.z = new_pos.z;

                // Smoothly orient character to face movement direction
                float target_yaw = std::atan2(move_dir.x, move_dir.z) + player.model_facing_offset;
                float diff = target_yaw - player.facing_yaw;
                constexpr float pi_val = 3.1415926535f;
                constexpr float two_pi_val = 2.0f * pi_val;
                while (diff > pi_val) diff -= two_pi_val;
                while (diff < -pi_val) diff += two_pi_val;
                player.facing_yaw += diff * std::min(14.0f * dt, 1.0f);
            } else {
                // When stationary, still push player out if an object rolls into player
                Vec3 p_center(player.position.x, player.position.y + 0.90f, player.position.z);
                const float char_radius = 0.38f;
                const float char_half_h = 0.85f;
                for (size_t s_i = 0; s_i < physics_scene.get_body_slot_count(); ++s_i) {
                    const auto* body = physics_scene.get_body_by_slot(static_cast<uint32_t>(s_i));
                    if (!body || !body->get_shape() || !body->is_dynamic()) continue;
                    const auto b_pos = body->get_position();
                    float dx = p_center.x - b_pos.x;
                    float dz = p_center.z - b_pos.z;
                    float dist_sq = dx * dx + dz * dz;
                    float min_dist = char_radius + 0.40f;
                    if (dist_sq < min_dist * min_dist && std::abs(p_center.y - b_pos.y) < char_half_h + 0.40f) {
                        float dist = std::sqrt(dist_sq);
                        Vec3 normal = (dist > 1e-4f) ? Vec3(dx / dist, 0.0f, dz / dist) : Vec3(1.0f, 0.0f, 0.0f);
                        player.position.x += normal.x * (min_dist - dist);
                        player.position.z += normal.z * (min_dist - dist);
                    }
                }
            }

            // Jump & Ground Gravity physics
            if (key_space && player.is_grounded) {
                player.velocity_y = 6.8f;
                player.is_grounded = false;
            }
            if (!player.is_grounded) {
                player.velocity_y -= 18.0f * dt;
                player.position.y += player.velocity_y * dt;
                if (player.position.y <= player.ground_y) {
                    player.position.y = player.ground_y;
                    player.velocity_y = 0.0f;
                    player.is_grounded = true;
                }
            }

            // Update 3D Character Model Transform (Rotation + Translation + Scale)
            Mat44 root_xform = Mat44::transform(
                player.position,
                Quat::from_axis_angle(Vec3::up(), player.facing_yaw),
                Vec3(player.model_scale, player.model_scale, player.model_scale)
            );
            for (size_t i = 0; i < player_renderables.size(); ++i) {
                if (player_renderables[i]) {
                    Mat44 base = (i < player_base_transforms.size()) ? player_base_transforms[i] : Mat44::identity();
                    player_renderables[i]->set_transform(root_xform * base);
                }
            }

            // Skeletal Animation Playback & Real-Time GPU Mesh Deformation
            if (player_animator) {
                const bool is_moving = (move_len_sq > 0.0001f);
                if (is_moving) {
                    int32_t target_idx = -1;
                    float anim_speed = 1.0f;
                    if (key_shift) {
                        target_idx = player_animator->find_clip("run");
                        if (target_idx < 0) target_idx = player_animator->find_clip("Run");
                        if (target_idx >= 0) {
                            anim_speed = 1.0f;
                        } else {
                            target_idx = player_animator->find_clip("walk");
                            if (target_idx < 0) target_idx = player_animator->find_clip("Walk");
                            anim_speed = 2.0f;
                        }
                    } else {
                        target_idx = player_animator->find_clip("walk");
                        if (target_idx < 0) target_idx = player_animator->find_clip("Walk");
                        anim_speed = 1.0f;
                    }

                    if (target_idx >= 0) {
                        if (player_animator->current_clip_index != target_idx) {
                            player_animator->play(static_cast<size_t>(target_idx), true, anim_speed);
                        } else {
                            player_animator->set_speed(anim_speed);
                            player_animator->resume();
                        }
                    } else {
                        player_animator->set_speed(key_shift ? 2.2f : 1.0f);
                        player_animator->resume();
                    }
                    player_animator->update(dt);
                } else {
                    int32_t idle_idx = player_animator->find_clip("idle");
                    if (idle_idx < 0) idle_idx = player_animator->find_clip("Idle");
                    if (idle_idx >= 0) {
                        if (player_animator->current_clip_index != idle_idx) {
                            player_animator->play(static_cast<size_t>(idle_idx), true, 1.0f);
                        } else {
                            player_animator->set_speed(1.0f);
                            player_animator->resume();
                        }
                        player_animator->update(dt);
                    } else {
                        // For single-clip models (e.g. CesiumMan), cycle slowly or keep stance
                        player_animator->set_speed(0.3f);
                        player_animator->resume();
                        player_animator->update(dt * 0.3f);
                    }
                }

                // Dynamically stream deformed vertex positions and normals to GPU VBO
                for (const auto& sm : player_animator->skinned_meshes) {
                    if (sm && sm->needs_gpu_upload && sm->dynamic_mesh) {
                        gpu_backend.update_mesh_buffer(*sm->dynamic_mesh);
                        sm->needs_gpu_upload = false;
                    }
                }
            }

            // Position chase camera behind character looking at target point
            Vec3 target = player.position + Vec3(0.0f, player.cam_target_height, 0.0f);
            const float cp = std::cos(player.cam_pitch);
            const float sp = std::sin(player.cam_pitch);
            Vec3 eye = target + Vec3(player.cam_distance * cp * sy,
                                     player.cam_distance * sp,
                                     player.cam_distance * cp * cy);
            camera->set_look_at(eye, target, Vec3::up());

        } else if (current_cam_mode == CameraMode::FirstPersonWalk) {
            for (auto& r : player_renderables) {
                if (r) r->visible = false;
            }

            float speed = key_shift ? (fps_cam.move_speed * 3.0f) : fps_cam.move_speed;
            Vec3 fwd = fps_cam.fly_mode ? fps_cam.forward() : fps_cam.forward_horizontal();
            Vec3 right = fps_cam.right();

            if (key_w) fps_cam.position += fwd * (speed * dt);
            if (key_s) fps_cam.position -= fwd * (speed * dt);
            if (key_d) fps_cam.position += right * (speed * dt);
            if (key_a) fps_cam.position -= right * (speed * dt);
            if (key_space) fps_cam.position += Vec3::up() * (speed * dt);
            if (key_c) fps_cam.position -= Vec3::up() * (speed * dt);

            if (key_left)  fps_cam.yaw -= 1.8f * dt;
            if (key_right) fps_cam.yaw += 1.8f * dt;
            if (key_up)    fps_cam.pitch = std::clamp(fps_cam.pitch + 1.4f * dt, -1.50f, 1.50f);
            if (key_down)  fps_cam.pitch = std::clamp(fps_cam.pitch - 1.4f * dt, -1.50f, 1.50f);

            fps_cam.update(*camera);
        } else {
            for (auto& r : player_renderables) {
                if (r) r->visible = true;
            }
            orbit_cam.update(*camera);
        }

        auto win_sz = window->getSize();
        auto fb_sz = window->getDrawableSize();
        int win_w = static_cast<int>(win_sz.width);
        int win_h = static_cast<int>(win_sz.height);
        int fb_w = static_cast<int>(fb_sz.width);
        int fb_h = static_cast<int>(fb_sz.height);
        if (win_w <= 0 || win_h <= 0 || fb_w <= 0 || fb_h <= 0) continue;
        float dpr = static_cast<float>(fb_w) / static_cast<float>(win_w);

        // Configure 3D Camera Projection
        camera->set_perspective(
            60.0f * (3.1415926535f / 180.0f), // 60 deg wide FOV for immersive walkthrough
            static_cast<float>(fb_w) / static_cast<float>(fb_h),
            0.1f,
            radius * 12.0f,
            false // OpenGL [-1, 1] depth
        );

        View3D view;
        view.set_scene(scene);
        view.set_camera(camera);
        view.set_viewport(0, 0, fb_w, fb_h);
        Vec4 clear_col = is_sponza ? Vec4(0.92f, 0.95f, 0.99f, 1.0f) :
                         is_village ? Vec4(0.65f, 0.78f, 0.92f, 1.0f) :
                         is_city ? Vec4(0.08f, 0.05f, 0.06f, 1.0f) :
                         Vec4(0.04f, 0.05f, 0.08f, 1.0f);
        view.set_clear_color(clear_col);
        view.shadows_enabled = true;
        view.shadow_distance = radius * 8.0f;

        // Render 3D Scene Directly to Native Window Screen (FBO 0)
        gpu_backend.render_view(view, 0, true);

        const auto& stats = gpu_backend.last_stats();

        // 10. Render Sleek 2D UI / HUD Overlay Fusion
        if (show_hud && hud_ctx) {
            hud_ctx->beginFrame(win_w, win_h, dpr);

            // Subtle Crosshair in Center
            float mid_x = win_w * 0.5f;
            float mid_y = win_h * 0.5f;
            hud_ctx->beginPath();
            hud_ctx->moveTo(mid_x - 8.0f, mid_y);
            hud_ctx->lineTo(mid_x + 8.0f, mid_y);
            hud_ctx->moveTo(mid_x, mid_y - 8.0f);
            hud_ctx->lineTo(mid_x, mid_y + 8.0f);
            hud_ctx->strokeColor(nisaba::gpu::Color::rgba(255, 255, 255, 120));
            hud_ctx->strokeWidth(1.5f);
            hud_ctx->stroke();

            // Top-Left Glassmorphism Card: Engine & Model Info
            {
                float card_x = 24.0f;
                float card_y = 24.0f;
                float card_w = 420.0f;
                float card_h = 100.0f;

                hud_ctx->beginPath();
                hud_ctx->roundedRect(card_x, card_y, card_w, card_h, 12.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(15, 23, 42, 210));
                hud_ctx->fill();

                hud_ctx->beginPath();
                hud_ctx->roundedRect(card_x, card_y, card_w, card_h, 12.0f);
                hud_ctx->strokeColor(nisaba::gpu::Color::rgba(56, 189, 248, 140));
                hud_ctx->strokeWidth(1.2f);
                hud_ctx->stroke();

                hud_ctx->fontSize(16.0f);
                hud_ctx->fontFace("sans");
                hud_ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(255, 255, 255, 255));
                hud_ctx->text(card_x + 18.0f, card_y + 14.0f, "NISABA 3D HARDWARE WALKTHROUGH");

                hud_ctx->fontSize(13.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(56, 189, 248, 255));
                char model_label[256];
                std::snprintf(model_label, sizeof(model_label), "Map: %s",
                              std::filesystem::path(model_path).filename().string().c_str());
                hud_ctx->text(card_x + 18.0f, card_y + 40.0f, model_label);

                hud_ctx->fontSize(12.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(203, 213, 225, 230));
                char geom_label[256];
                std::snprintf(geom_label, sizeof(geom_label),
                              "Triangles: %zu | Vertices: %zu | Draw Calls: %u",
                              total_tris, total_verts, stats.total_draw_calls);
                hud_ctx->text(card_x + 18.0f, card_y + 64.0f, geom_label);
            }

            // Top-Right Glassmorphism Card: Real-Time Performance
            {
                float card_w = 340.0f;
                float card_h = 100.0f;
                float card_x = win_w - card_w - 24.0f;
                float card_y = 24.0f;

                hud_ctx->beginPath();
                hud_ctx->roundedRect(card_x, card_y, card_w, card_h, 12.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(15, 23, 42, 210));
                hud_ctx->fill();

                hud_ctx->beginPath();
                hud_ctx->roundedRect(card_x, card_y, card_w, card_h, 12.0f);
                hud_ctx->strokeColor(nisaba::gpu::Color::rgba(16, 185, 129, 140));
                hud_ctx->strokeWidth(1.2f);
                hud_ctx->stroke();

                hud_ctx->fontSize(15.0f);
                hud_ctx->fontFace("sans");
                hud_ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Top);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(16, 185, 129, 255));
                hud_ctx->text(card_x + 18.0f, card_y + 14.0f, "LIVE GPU PERFORMANCE");

                hud_ctx->fontSize(13.0f);
                hud_ctx->fontFace("sans");
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(248, 250, 252, 255));
                hud_ctx->text(card_x + 18.0f, card_y + 40.0f, perf_str);

                char mode_str[128];
                std::snprintf(mode_str, sizeof(mode_str), "Mode: %s [Tab] | Physics: ACTIVE",
                    current_cam_mode == CameraMode::ThirdPersonPlayer ? "3D PLAYER (GAME MODE)" :
                    current_cam_mode == CameraMode::FirstPersonWalk   ? "FIRST PERSON WALK" : "ORBIT INSPECT");
                hud_ctx->fontSize(12.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(203, 213, 225, 230));
                hud_ctx->text(card_x + 18.0f, card_y + 64.0f, mode_str);
            }

            // Bottom-Left Coordinates Pill
            {
                float pill_x = 24.0f;
                float pill_y = win_h - 70.0f;
                float pill_w = 420.0f;
                float pill_h = 44.0f;

                hud_ctx->beginPath();
                hud_ctx->roundedRect(pill_x, pill_y, pill_w, pill_h, 8.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(15, 23, 42, 200));
                hud_ctx->fill();

                char pos_str[256];
                if (current_cam_mode == CameraMode::ThirdPersonPlayer) {
                    std::snprintf(pos_str, sizeof(pos_str), "Player: (%.1f, %.1f, %.1f) | Facing: %.0f deg | %s",
                                  player.position.x, player.position.y, player.position.z,
                                  player.facing_yaw * (180.0f / 3.14159265f),
                                  key_shift ? "SPRINT" : "WALK");
                } else if (current_cam_mode == CameraMode::FirstPersonWalk) {
                    std::snprintf(pos_str, sizeof(pos_str), "Pos: (%.2f, %.2f, %.2f) | Speed: %.1f m/s",
                                  fps_cam.position.x, fps_cam.position.y, fps_cam.position.z,
                                  key_shift ? (fps_cam.move_speed * 3.0f) : fps_cam.move_speed);
                } else {
                    std::snprintf(pos_str, sizeof(pos_str), "Target: (%.1f, %.1f, %.1f) | Dist: %.1f",
                                  orbit_cam.target.x, orbit_cam.target.y, orbit_cam.target.z,
                                  orbit_cam.distance);
                }

                hud_ctx->fontSize(12.0f);
                hud_ctx->fontFace("sans");
                hud_ctx->textAlign(nisaba::gpu::Align::Left | nisaba::gpu::Align::Middle);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(226, 232, 240, 240));
                hud_ctx->text(pill_x + 16.0f, pill_y + 22.0f, pos_str);
            }

            // Bottom-Center Navigation Controls Bar
            {
                float bar_w = 840.0f;
                float bar_h = 44.0f;
                float bar_x = (win_w - bar_w) * 0.5f;
                float bar_y = win_h - 70.0f;

                hud_ctx->beginPath();
                hud_ctx->roundedRect(bar_x, bar_y, bar_w, bar_h, 22.0f);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(15, 23, 42, 220));
                hud_ctx->fill();

                hud_ctx->beginPath();
                hud_ctx->roundedRect(bar_x, bar_y, bar_w, bar_h, 22.0f);
                hud_ctx->strokeColor(nisaba::gpu::Color::rgba(148, 163, 184, 90));
                hud_ctx->strokeWidth(1.0f);
                hud_ctx->stroke();

                hud_ctx->fontSize(12.0f);
                hud_ctx->fontFace("sans");
                hud_ctx->textAlign(nisaba::gpu::Align::Center | nisaba::gpu::Align::Middle);
                hud_ctx->fillColor(nisaba::gpu::Color::rgba(226, 232, 240, 240));
                if (current_cam_mode == CameraMode::ThirdPersonPlayer) {
                    hud_ctx->text(bar_x + bar_w * 0.5f, bar_y + 22.0f,
                        "[W/A/S/D] Move  |  [Mouse Drag] Orbit  |  [Space] Jump  |  [Shift] Sprint  |  [Tab] Switch Mode  |  [V] VSync  |  [ESC] Quit");
                } else {
                    hud_ctx->text(bar_x + bar_w * 0.5f, bar_y + 22.0f,
                        "[W/A/S/D] Walk  |  [Mouse Drag] Look  |  [Space/C] Up/Down  |  [Shift] Sprint  |  [Tab] Switch Mode  |  [V] VSync  |  [ESC] Quit");
                }
            }

            hud_ctx->endFrame();
        }

        // Snapshot handler for headless testing or capture
        const bool should_take_snapshot = headless && (target_frames > 0 ? (frame_count >= target_frames - 1) : (frame_count >= 5));
        if (should_take_snapshot) {
            std::vector<uint8_t> pixels(fb_w * fb_h * 4);
            glReadPixels(0, 0, fb_w, fb_h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            std::vector<uint8_t> flipped(fb_w * fb_h * 4);
            int stride = fb_w * 4;
            for (int y = 0; y < fb_h; ++y) {
                std::memcpy(&flipped[y * stride], &pixels[(fb_h - 1 - y) * stride], stride);
            }
            auto sz = IntSize::from_wh(static_cast<uint32_t>(fb_w), static_cast<uint32_t>(fb_h));
            if (sz) {
                auto pix = Pixmap::from_vec(std::move(flipped), *sz, PixelFormat::RGBA8888);
                if (pix) {
                    std::filesystem::path p(snapshot_path);
                    if (p.has_parent_path()) {
                        std::filesystem::create_directories(p.parent_path());
                    }
                    pix->save_png(snapshot_path);
                    std::cout << "[+] Saved walkthrough frame snapshot to: " << snapshot_path << "\n";
                }
            }
            break;
        }

        window->swapBuffers();
        frame_count++;

        if (target_frames > 0 && frame_count >= target_frames) {
            break;
        }
    }

    std::cout << "Walkthrough completed. Total frames: " << frame_count << "\n";
    window.reset();
    platform.reset();
    return 0;
}

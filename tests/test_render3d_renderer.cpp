#include "nisaba/render3d/render3d.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace nisaba::render3d;
using namespace nisaba::physics;

static int g_tests_passed = 0;
static int g_tests_total = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        g_tests_total++; \
        if (!(cond)) { \
            std::cerr << "[-] FAILED: " << msg << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            assert(false && msg); \
        } else { \
            g_tests_passed++; \
        } \
    } while(0)

void test_64bit_sort_key_encoding_and_decoding() {
    std::cout << "[*] Testing 64-bit sort key bitfield encoding/decoding..." << std::endl;

    const RenderPassID pass = RenderPassID::Opaque;
    const uint16_t mat_id = 0xBEEF;
    const float depth = 125.5f;
    const uint32_t prim_id = 0xABCDE;

    uint64_t key = DrawSortKey64::encode(pass, mat_id, depth, false, prim_id);

    TEST_ASSERT(DrawSortKey64::decode_pass(key) == pass, "Decoded pass ID matches");
    TEST_ASSERT(DrawSortKey64::decode_material_id(key) == mat_id, "Decoded material ID matches");
    TEST_ASSERT(DrawSortKey64::decode_primitive_id(key) == prim_id, "Decoded primitive ID matches");
}

void test_64bit_sorting_order() {
    std::cout << "[*] Testing 64-bit Early-Z front-to-back & alpha back-to-front sorting..." << std::endl;

    // 1. Opaque near (depth = 5.0)
    uint64_t key_op_near = DrawSortKey64::encode(RenderPassID::Opaque, 1, 5.0f, false, 10);
    // 2. Opaque far (depth = 30.0)
    uint64_t key_op_far  = DrawSortKey64::encode(RenderPassID::Opaque, 1, 30.0f, false, 20);

    // 3. Transparent near (depth = 5.0)
    uint64_t key_tr_near = DrawSortKey64::encode(RenderPassID::Transparent, 2, 5.0f, true, 30);
    // 4. Transparent far (depth = 30.0)
    uint64_t key_tr_far  = DrawSortKey64::encode(RenderPassID::Transparent, 2, 30.0f, true, 40);

    // Opaque must sort Front-to-Back (smallest depth first) for Early-Z optimization
    TEST_ASSERT(key_op_near < key_op_far, "Opaque near must sort before opaque far (Front-to-Back)");

    // Opaque passes must always execute BEFORE transparent passes
    TEST_ASSERT(key_op_far < key_tr_far, "All opaque commands must sort before any transparent command");

    // Transparent must sort Back-to-Front (farthest depth first) for correct alpha blending
    TEST_ASSERT(key_tr_far < key_tr_near, "Transparent far must sort before transparent near (Back-to-Front)");
}

void test_renderable_and_scene_graph() {
    std::cout << "[*] Testing Renderable3D transforms and Scene3D culling queries..." << std::endl;

    auto mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cube(Vec3(2.0f, 2.0f, 2.0f)));
    auto mat = PbrMaterial::create_default_dielectric();

    auto r = std::make_shared<Renderable3D>(mesh, mat);
    r->id = 42;
    r->set_position(Vec3(0.0f, 0.0f, 10.0f));

    const AABB& wb = r->get_world_bounds();
    TEST_ASSERT(wb.min_point.z >= 8.9f && wb.max_point.z <= 11.1f, "World bounds translate to Z=10");

    Scene3D scene;
    scene.add_renderable(r);
    TEST_ASSERT(scene.renderable_count() == 1, "Scene has 1 renderable");

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)); // Looking at +Z
    cam.set_perspective(60.0f * (3.14159265f / 180.0f), 1.0f, 0.1f, 50.0f);

    std::vector<const Renderable3D*> visible;
    scene.gather_visible_renderables(cam.get_frustum(), visible);
    TEST_ASSERT(visible.size() == 1, "Renderable in front of camera is gathered");

    // Move camera to look away (-Z)
    cam.set_look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
    scene.gather_visible_renderables(cam.get_frustum(), visible);
    TEST_ASSERT(visible.empty(), "Renderable behind camera is culled");
}

void test_renderer_end_to_end_frame_pipeline() {
    std::cout << "[*] Testing Renderer3D end-to-end frame batching and queue sorting..." << std::endl;

    auto cube_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_cube(Vec3(1.0f, 1.0f, 1.0f)));
    auto sphere_mesh = std::make_shared<Mesh3D>(MeshPrimitives::create_sphere(1.0f, 16, 8));

    auto opaque_mat = PbrMaterial::create_gold();
    auto trans_mat = std::make_shared<PbrMaterial>();
    trans_mat->base_color_factor = Vec4(1.0f, 1.0f, 1.0f, 0.5f);
    trans_mat->alpha_mode = AlphaMode::Blend;

    // Cube 1: Opaque at (0, 0, 5) -> dist 5
    auto obj1 = std::make_shared<Renderable3D>(cube_mesh, opaque_mat);
    obj1->id = 1;
    obj1->set_position(Vec3(0.0f, 0.0f, 5.0f));

    // Cube 2: Opaque at (0, 0, 15) -> dist 15
    auto obj2 = std::make_shared<Renderable3D>(cube_mesh, opaque_mat);
    obj2->id = 2;
    obj2->set_position(Vec3(0.0f, 0.0f, 15.0f));

    // Sphere: Transparent at (0, 0, 10) -> dist 10
    auto obj3 = std::make_shared<Renderable3D>(sphere_mesh, trans_mat);
    obj3->id = 3;
    obj3->set_position(Vec3(0.0f, 0.0f, 10.0f));

    // Cube 3: Far away at (0, 0, 200) -> beyond camera far clip (100) -> should be culled
    auto obj4 = std::make_shared<Renderable3D>(cube_mesh, opaque_mat);
    obj4->id = 4;
    obj4->set_position(Vec3(0.0f, 0.0f, 200.0f));

    auto scene = std::make_shared<Scene3D>();
    scene->add_renderable(obj1);
    scene->add_renderable(obj2);
    scene->add_renderable(obj3);
    scene->add_renderable(obj4);

    auto sun = Light3D::create_directional(Vec3(0.5f, -1.0f, 0.3f));
    scene->add_light(sun);

    auto cam = std::make_shared<Camera3D>();
    cam->set_look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)); // Looking at +Z
    cam->set_perspective(60.0f * (3.14159265f / 180.0f), 1.0f, 0.1f, 50.0f);

    View3D view(cam, scene);
    view.set_viewport(0, 0, 800, 600);

    Renderer3D renderer;
    renderer.prepare_frame(view);

    const auto& stats = renderer.get_stats();
    TEST_ASSERT(stats.total_objects == 4, "Total objects is 4");
    TEST_ASSERT(stats.visible_objects == 3, "Visible objects is 3 (obj4 culled)");
    TEST_ASSERT(stats.culled_objects == 1, "Culled objects is 1");
    TEST_ASSERT(stats.opaque_draw_calls == 2, "2 Opaque draw calls");
    TEST_ASSERT(stats.transparent_draw_calls == 1, "1 Transparent draw call");
    TEST_ASSERT(stats.total_draw_calls == 3, "3 Total draw calls");

    const auto& queue = renderer.get_draw_queue();
    TEST_ASSERT(queue.size() == 3, "Draw queue has 3 sorted commands");

    // Verify ordering in the sorted draw queue:
    // Index 0: Opaque Cube 1 (distance 5 - front)
    TEST_ASSERT(queue[0].renderable->id == 1, "Queue[0] is nearest opaque cube (Front-to-back)");
    // Index 1: Opaque Cube 2 (distance 15 - farther)
    TEST_ASSERT(queue[1].renderable->id == 2, "Queue[1] is farther opaque cube (Front-to-back)");
    // Index 2: Transparent Sphere (distance 10)
    TEST_ASSERT(queue[2].renderable->id == 3, "Queue[2] is transparent sphere (Opaque before transparent)");
}

int main() {
    std::cout << "========================================================\n"
              << "   NISABA 3D RENDERING PIPELINE: SCENE & RENDERER TEST \n"
              << "========================================================" << std::endl;

    test_64bit_sort_key_encoding_and_decoding();
    test_64bit_sorting_order();
    test_renderable_and_scene_graph();
    test_renderer_end_to_end_frame_pipeline();

    std::cout << "\n[+] ALL TESTS COMPLETED: " << g_tests_passed << "/" << g_tests_total << " assertions passed." << std::endl;
    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

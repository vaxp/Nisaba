#include "nisaba/physics/physics.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

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

// ============================================================================
// Test 1: Full Scene Population & SIMD Raycasting
// ============================================================================
void test_scene_population_and_raycasting() {
    std::cout << "[*] Testing PhysicsScene multi-shape population and SIMD raycasting..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene scene(settings);

    // 1. Static Ground Plane at y = 0
    PlaneShape ground_shape(Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f), 100.0f);
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, 0.0f, 0.0f);
    BodyID ground_id = scene.create_body(ground_cs);
    TEST_ASSERT(ground_id.is_valid(), "Ground plane created");

    // 2. Sphere at (0, 5, 0), radius 1.0
    SphereShape sphere_shape(1.0f);
    BodyCreationSettings sphere_cs;
    sphere_cs.shape = &sphere_shape;
    sphere_cs.motion_type = MotionType::Dynamic;
    sphere_cs.position = Vec3(0.0f, 5.0f, 0.0f);
    BodyID sphere_id = scene.create_body(sphere_cs);
    TEST_ASSERT(sphere_id.is_valid(), "Sphere created");

    // 3. Box at (10, 5, 0), half-extents (1, 1, 1)
    BoxShape box_shape(Vec3(1.0f, 1.0f, 1.0f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(10.0f, 5.0f, 0.0f);
    BodyID box_id = scene.create_body(box_cs);
    TEST_ASSERT(box_id.is_valid(), "Box created");

    // 4. Capsule at (-10, 5, 0), half-height 1.0, radius 0.5 (height spans y = [3.5, 6.5])
    CapsuleShape capsule_shape(1.0f, 0.5f);
    BodyCreationSettings capsule_cs;
    capsule_cs.shape = &capsule_shape;
    capsule_cs.motion_type = MotionType::Dynamic;
    capsule_cs.position = Vec3(-10.0f, 5.0f, 0.0f);
    BodyID capsule_id = scene.create_body(capsule_cs);
    TEST_ASSERT(capsule_id.is_valid(), "Capsule created");

    // 5. Cylinder at (0, 5, 10), half-height 1.0, radius 0.5 (top cap at y = 6.0)
    CylinderShape cylinder_shape(1.0f, 0.5f);
    BodyCreationSettings cylinder_cs;
    cylinder_cs.shape = &cylinder_shape;
    cylinder_cs.motion_type = MotionType::Dynamic;
    cylinder_cs.position = Vec3(0.0f, 5.0f, 10.0f);
    BodyID cylinder_id = scene.create_body(cylinder_cs);
    TEST_ASSERT(cylinder_id.is_valid(), "Cylinder created");

    // 6. Convex Hull Pyramid at (0, 5, -10)
    std::vector<Vec3> pyramid_verts = {
        Vec3(-1.0f, -1.0f, -1.0f),
        Vec3( 1.0f, -1.0f, -1.0f),
        Vec3( 1.0f, -1.0f,  1.0f),
        Vec3(-1.0f, -1.0f,  1.0f),
        Vec3( 0.0f,  1.0f,  0.0f)
    };
    ConvexHullShape hull_shape(pyramid_verts);
    BodyCreationSettings hull_cs;
    hull_cs.shape = &hull_shape;
    hull_cs.motion_type = MotionType::Dynamic;
    hull_cs.position = Vec3(0.0f, 5.0f, -10.0f);
    BodyID hull_id = scene.create_body(hull_cs);
    TEST_ASSERT(hull_id.is_valid(), "Convex hull pyramid created");

    TEST_ASSERT(scene.get_body_count() == 6, "Scene contains 6 bodies");

    // --- Raycast Tests ---
    PhysicsScene::RayCastResult hit;

    // Ray 1: Downward onto Sphere from (0, 10, 0)
    Ray ray_sphere(Vec3(0.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
    bool sphere_hit = scene.cast_ray(ray_sphere, 20.0f, hit);
    TEST_ASSERT(sphere_hit, "Ray hit sphere");
    TEST_ASSERT(hit.body_id == sphere_id, "Hit body is the sphere");
    TEST_ASSERT(std::abs(hit.hit_point.y - 6.0f) < 0.01f, "Sphere hit at top apex y = 6.0");
    TEST_ASSERT(std::abs(hit.hit_normal.y - 1.0f) < 0.01f, "Sphere hit normal points up (+Y)");
    TEST_ASSERT(std::abs(hit.distance - 4.0f) < 0.01f, "Sphere ray travel distance is exactly 4.0m");

    // Ray 2: Downward onto Box from (10, 10, 0)
    Ray ray_box(Vec3(10.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
    bool box_hit = scene.cast_ray(ray_box, 20.0f, hit);
    TEST_ASSERT(box_hit, "Ray hit box");
    TEST_ASSERT(hit.body_id == box_id, "Hit body is the box");
    TEST_ASSERT(std::abs(hit.hit_point.y - 6.0f) < 0.01f, "Box hit at top face y = 6.0");
    TEST_ASSERT(std::abs(hit.hit_normal.y - 1.0f) < 0.01f, "Box top face normal is +Y");
    TEST_ASSERT(std::abs(hit.distance - 4.0f) < 0.01f, "Box ray distance is 4.0m");

    // Ray 3: Downward onto Capsule from (-10, 10, 0)
    Ray ray_capsule(Vec3(-10.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
    bool capsule_hit = scene.cast_ray(ray_capsule, 20.0f, hit);
    TEST_ASSERT(capsule_hit, "Ray hit capsule");
    TEST_ASSERT(hit.body_id == capsule_id, "Hit body is capsule");
    // Capsule top apex is at y = 5.0 + 1.0 (half-height) + 0.5 (radius) = 6.5
    TEST_ASSERT(std::abs(hit.hit_point.y - 6.5f) < 0.01f, "Capsule hit at top hemisphere apex y = 6.5");
    TEST_ASSERT(std::abs(hit.hit_normal.y - 1.0f) < 0.01f, "Capsule apex normal points +Y");
    TEST_ASSERT(std::abs(hit.distance - 3.5f) < 0.01f, "Capsule ray distance is 3.5m");

    // Ray 4: Downward onto Cylinder from (0, 10, 10)
    Ray ray_cylinder(Vec3(0.0f, 10.0f, 10.0f), Vec3(0.0f, -1.0f, 0.0f));
    bool cyl_hit = scene.cast_ray(ray_cylinder, 20.0f, hit);
    TEST_ASSERT(cyl_hit, "Ray hit cylinder");
    TEST_ASSERT(hit.body_id == cylinder_id, "Hit body is cylinder");
    // Cylinder top flat disc is at y = 5.0 + 1.0 = 6.0
    TEST_ASSERT(std::abs(hit.hit_point.y - 6.0f) < 0.01f, "Cylinder hit on top flat cap y = 6.0");
    TEST_ASSERT(std::abs(hit.hit_normal.y - 1.0f) < 0.01f, "Cylinder top cap normal is +Y");
    TEST_ASSERT(std::abs(hit.distance - 4.0f) < 0.01f, "Cylinder ray distance is 4.0m");

    // Ray 5: Downward onto Convex Hull Pyramid from (0, 10, -10)
    Ray ray_hull(Vec3(0.0f, 10.0f, -10.0f), Vec3(0.0f, -1.0f, 0.0f));
    bool hull_hit = scene.cast_ray(ray_hull, 20.0f, hit);
    TEST_ASSERT(hull_hit, "Ray hit convex hull pyramid");
    TEST_ASSERT(hit.body_id == hull_id, "Hit body is convex hull");
    // Pyramid tip is at local y = 1.0 => world y = 6.0
    TEST_ASSERT(std::abs(hit.hit_point.y - 6.0f) < 0.05f, "Pyramid apex hit near y = 6.0");

    // Ray 6: Ray that misses everything (directed into empty void)
    Ray ray_miss(Vec3(50.0f, 50.0f, 50.0f), Vec3(1.0f, 1.0f, 1.0f));
    bool miss = scene.cast_ray(ray_miss, 100.0f, hit);
    TEST_ASSERT(!miss, "Ray in empty space returns no hit");
}

// ============================================================================
// Test 2: Dynamic Multi-Shape Simulation & Resting Equilibrium
// ============================================================================
void test_dynamic_multi_shape_simulation() {
    std::cout << "[*] Testing dynamic multi-shape cascade and stacking equilibrium..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.enable_warm_starting = true;
    settings.enable_sleeping = true;
    settings.time_before_sleep = 0.3f; // 0.3 seconds before sleep
    PhysicsScene scene(settings);

    // Static Ground at y = -1.0
    BoxShape ground_shape(Vec3(50.0f, 1.0f, 50.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    (void)scene.create_body(ground_cs);

    // Dynamic Sphere dropping from y = 1.0 (radius 0.5, resting y will be 0.5)
    SphereShape sphere_shape(0.5f);
    BodyCreationSettings sphere_cs;
    sphere_cs.shape = &sphere_shape;
    sphere_cs.motion_type = MotionType::Dynamic;
    sphere_cs.position = Vec3(0.0f, 1.0f, 0.0f);
    BodyID sphere_id = scene.create_body(sphere_cs);

    // Dynamic Box dropping from y = 2.0 (half-extents 0.5, resting y = 0.5) at x = 5.0
    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(5.0f, 2.0f, 0.0f);
    BodyID box_id = scene.create_body(box_cs);

    // Step simulation for 90 frames (1.5 seconds)
    for (int i = 0; i < 90; ++i) {
        scene.step(1.0f / 60.0f);
    }

    Body* sphere = scene.get_body(sphere_id);
    Body* box = scene.get_body(box_id);

    TEST_ASSERT(sphere != nullptr && box != nullptr, "Bodies exist");

    // Both bodies must have settled on the ground (y ≈ 0.5)
    TEST_ASSERT(std::abs(sphere->get_position().y - 0.5f) < 0.05f, "Sphere resting safely on ground plane");
    TEST_ASSERT(std::abs(box->get_position().y - 0.5f) < 0.05f, "Box resting safely on ground plane");

    // Both bodies must have gone to sleep after 1.5s (> 0.3s sleep threshold)
    TEST_ASSERT(sphere->is_sleeping(), "Sphere successfully transitioned to quiescent sleep");
    TEST_ASSERT(box->is_sleeping(), "Box successfully transitioned to quiescent sleep");
    TEST_ASSERT(scene.get_island_builder().get_active_island_count() == 0, "Zero active simulation islands at equilibrium");
    TEST_ASSERT(scene.get_island_builder().get_sleeping_island_count() == 2, "Two distinct sleeping islands formed");

    // Verify zero drifting over an additional 120 frames (2.0 full seconds of sleeping simulation)
    const Vec3 sleep_sphere_pos = sphere->get_position();
    const Vec3 sleep_box_pos = box->get_position();

    for (int i = 0; i < 120; ++i) {
        scene.step(1.0f / 60.0f);
    }

    TEST_ASSERT((sphere->get_position() - sleep_sphere_pos).length_sq() == 0.0f, "Sphere has 0.0f CPU drift while sleeping");
    TEST_ASSERT((box->get_position() - sleep_box_pos).length_sq() == 0.0f, "Box has 0.0f CPU drift while sleeping");
}

// ============================================================================
// Test 3: Kinetic Impact Wakeup & Chain Reaction
// ============================================================================
void test_kinetic_impact_wakeup_chain() {
    std::cout << "[*] Testing kinetic impact wakeup and island re-activation..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.enable_sleeping = true;
    settings.time_before_sleep = 0.2f;
    PhysicsScene scene(settings);

    // Ground
    BoxShape ground_shape(Vec3(50.0f, 1.0f, 50.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    (void)scene.create_body(ground_cs);

    // Target Box resting on ground
    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    BodyID box_id = scene.create_body(box_cs);

    // Let box fall asleep (30 frames = 0.5s > 0.2s)
    for (int i = 0; i < 30; ++i) {
        scene.step(1.0f / 60.0f);
    }

    Body* target_box = scene.get_body(box_id);
    TEST_ASSERT(target_box->is_sleeping(), "Target box is asleep");

    // Shoot a high-speed projectile sphere at the sleeping target box from x = -5.0
    SphereShape bullet_shape(0.3f);
    BodyCreationSettings bullet_cs;
    bullet_cs.shape = &bullet_shape;
    bullet_cs.motion_type = MotionType::Dynamic;
    bullet_cs.position = Vec3(-5.0f, 0.5f, 0.0f);
    bullet_cs.linear_velocity = Vec3(20.0f, 0.0f, 0.0f); // 20 m/s horizontal speed
    bullet_cs.override_mass = 2.0f;
    BodyID bullet_id = scene.create_body(bullet_cs);

    TEST_ASSERT(scene.get_body(bullet_id)->is_active(), "Bullet is active");
    TEST_ASSERT(target_box->is_sleeping(), "Target is sleeping as bullet approaches");

    // Advance until impact
    bool impact_occurred = false;
    for (int i = 0; i < 30; ++i) {
        scene.step(1.0f / 60.0f);
        if (!target_box->is_sleeping()) {
            impact_occurred = true;
            break;
        }
    }

    TEST_ASSERT(impact_occurred, "Bullet impact triggered automatic wake-up of sleeping target");
    TEST_ASSERT(target_box->is_active(), "Target box is active after impact");
    TEST_ASSERT(target_box->get_linear_velocity().x > 0.01f, "Target box acquired forward momentum from projectile");
    TEST_ASSERT(scene.get_island_builder().get_active_island_count() >= 1, "Active island created containing interacting bodies");
}

int main() {
    std::cout << "==========================================================" << std::endl;
    std::cout << "   Nisaba Sovereign 3D Physics Scene Integration Test     " << std::endl;
    std::cout << "==========================================================" << std::endl;

    test_scene_population_and_raycasting();
    test_dynamic_multi_shape_simulation();
    test_kinetic_impact_wakeup_chain();

    std::cout << "==========================================================" << std::endl;
    std::cout << "[+] ALL INTEGRATION TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "==========================================================" << std::endl;

    return 0;
}

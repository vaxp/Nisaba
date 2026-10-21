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
// Test 1: Disjoint-Set Union & Clustering
// ============================================================================
void test_island_clustering() {
    std::cout << "[*] Testing IslandBuilder independent cluster partitioning..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3::zero(); // Zero gravity for pure geometric clustering test
    PhysicsScene scene(settings);

    SphereShape sphere_shape(1.0f);

    // Cluster 1: Two intersecting dynamic spheres around X = -10
    // Sphere A at (-10.5, 0, 0), Sphere B at (-9.5, 0, 0). Overlap = 1.0.
    BodyCreationSettings cs1;
    cs1.shape = &sphere_shape;
    cs1.motion_type = MotionType::Dynamic;
    cs1.position = Vec3(-10.5f, 0.0f, 0.0f);
    BodyID id_1a = scene.create_body(cs1);

    BodyCreationSettings cs2;
    cs2.shape = &sphere_shape;
    cs2.motion_type = MotionType::Dynamic;
    cs2.position = Vec3(-9.5f, 0.0f, 0.0f);
    BodyID id_1b = scene.create_body(cs2);

    // Cluster 2: Two intersecting dynamic spheres around X = +10
    // Sphere C at (+9.5, 0, 0), Sphere D at (+10.5, 0, 0). Overlap = 1.0.
    BodyCreationSettings cs3;
    cs3.shape = &sphere_shape;
    cs3.motion_type = MotionType::Dynamic;
    cs3.position = Vec3(9.5f, 0.0f, 0.0f);
    BodyID id_2a = scene.create_body(cs3);

    BodyCreationSettings cs4;
    cs4.shape = &sphere_shape;
    cs4.motion_type = MotionType::Dynamic;
    cs4.position = Vec3(10.5f, 0.0f, 0.0f);
    BodyID id_2b = scene.create_body(cs4);

    // Step 1 frame
    scene.step(1.0f / 60.0f);

    const IslandBuilder& ib = scene.get_island_builder();
    TEST_ASSERT(ib.get_active_island_count() == 2, "Exactly 2 independent islands formed");

    // Verify bodies are properly grouped
    const auto& islands = ib.get_islands();
    TEST_ASSERT(islands.size() == 2, "Island vector size is 2");

    bool cluster1_found = false;
    bool cluster2_found = false;

    for (const auto& island : islands) {
        TEST_ASSERT(island.bodies.size() == 2, "Each island contains exactly 2 bodies");
        TEST_ASSERT(island.manifold_indices.size() == 1, "Each island contains 1 contact manifold");

        bool has_1a = (island.bodies[0] == id_1a || island.bodies[1] == id_1a);
        bool has_1b = (island.bodies[0] == id_1b || island.bodies[1] == id_1b);
        bool has_2a = (island.bodies[0] == id_2a || island.bodies[1] == id_2a);
        bool has_2b = (island.bodies[0] == id_2b || island.bodies[1] == id_2b);

        if (has_1a && has_1b) {
            cluster1_found = true;
            TEST_ASSERT(!has_2a && !has_2b, "Cluster 1 does not contain cluster 2 bodies");
        } else if (has_2a && has_2b) {
            cluster2_found = true;
            TEST_ASSERT(!has_1a && !has_1b, "Cluster 2 does not contain cluster 1 bodies");
        }
    }

    TEST_ASSERT(cluster1_found && cluster2_found, "Both independent clusters were recognized");
}

// ============================================================================
// Test 2: Static Body Sharing Without Merging
// ============================================================================
void test_static_body_sharing() {
    std::cout << "[*] Testing static body sharing without island merging..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene scene(settings);

    // Static Ground: large box at y = -1.0
    BoxShape ground_shape(Vec3(50.0f, 1.0f, 50.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    BodyID ground_id = scene.create_body(ground_cs);
    TEST_ASSERT(ground_id.is_valid(), "Static ground created");

    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));

    // Dynamic Box 1 at X = -10, resting on ground (ground top is y = 0, box half-h = 0.5 => y = 0.5)
    BodyCreationSettings box1_cs;
    box1_cs.shape = &box_shape;
    box1_cs.motion_type = MotionType::Dynamic;
    box1_cs.position = Vec3(-10.0f, 0.49f, 0.0f); // slight overlap to form contact
    BodyID box1_id = scene.create_body(box1_cs);
    TEST_ASSERT(box1_id.is_valid(), "Box 1 created");

    // Dynamic Box 2 at X = +10, resting on the same ground
    BodyCreationSettings box2_cs;
    box2_cs.shape = &box_shape;
    box2_cs.motion_type = MotionType::Dynamic;
    box2_cs.position = Vec3(10.0f, 0.49f, 0.0f); // slight overlap to form contact
    BodyID box2_id = scene.create_body(box2_cs);
    TEST_ASSERT(box2_id.is_valid(), "Box 2 created");

    scene.step(1.0f / 60.0f);

    const IslandBuilder& ib = scene.get_island_builder();
    // Static bodies must NOT merge Box 1 and Box 2 into one island!
    TEST_ASSERT(ib.get_active_island_count() == 2, "Static ground does not merge independent stacks into one island");

    for (const auto& island : ib.get_islands()) {
        TEST_ASSERT(island.bodies.size() == 1, "Each island contains exactly 1 dynamic body");
        TEST_ASSERT(island.manifold_indices.size() == 1, "Each island contains 1 contact manifold with ground");
    }
}

// ============================================================================
// Test 3: Quiescent Body Sleep Transition
// ============================================================================
void test_quiescent_sleep_transition() {
    std::cout << "[*] Testing quiescent body sleep transition..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.enable_sleeping = true;
    settings.time_before_sleep = 0.3f; // 0.3 seconds before sleeping
    PhysicsScene scene(settings);

    BoxShape ground_shape(Vec3(20.0f, 1.0f, 20.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    (void)scene.create_body(ground_cs);

    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f); // Resting directly on ground
    box_cs.linear_velocity = Vec3::zero();
    box_cs.angular_velocity = Vec3::zero();
    BodyID box_id = scene.create_body(box_cs);

    Body* box = scene.get_body(box_id);
    TEST_ASSERT(box->is_active(), "Box initially active");
    TEST_ASSERT(!box->is_sleeping(), "Box not initially sleeping");

    // Simulate for 24 steps (24 * (1/60s) = 0.40s > 0.30s sleep threshold)
    for (int i = 0; i < 24; ++i) {
        scene.step(1.0f / 60.0f);
    }

    TEST_ASSERT(!box->is_active(), "Box is inactive after sustained quiescence");
    TEST_ASSERT(box->is_sleeping(), "Box is sleeping");
    TEST_ASSERT(scene.is_body_sleeping(box_id), "Scene confirms body is sleeping");
    TEST_ASSERT(scene.get_island_builder().get_sleeping_island_count() == 1, "Sleeping island count is 1");
    TEST_ASSERT(scene.get_island_builder().get_active_island_count() == 0, "Active island count is 0");
    TEST_ASSERT(box->get_linear_velocity().length_sq() == 0.0f, "Linear velocity is zeroed on sleep");
    TEST_ASSERT(box->get_angular_velocity().length_sq() == 0.0f, "Angular velocity is zeroed on sleep");
}

// ============================================================================
// Test 4: Sleeping Stability & Zero CPU Drifting
// ============================================================================
void test_sleeping_stability() {
    std::cout << "[*] Testing sleeping stability and zero drift..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.enable_sleeping = true;
    settings.time_before_sleep = 0.2f;
    PhysicsScene scene(settings);

    BoxShape ground_shape(Vec3(20.0f, 1.0f, 20.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    (void)scene.create_body(ground_cs);

    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    BodyID box_id = scene.create_body(box_cs);

    // Let it settle and sleep (30 steps = 0.5s > 0.2s)
    for (int i = 0; i < 30; ++i) {
        scene.step(1.0f / 60.0f);
    }

    Body* box = scene.get_body(box_id);
    TEST_ASSERT(box->is_sleeping(), "Body successfully put to sleep");

    const Vec3 sleep_position = box->get_position();
    const Quat sleep_rotation = box->get_rotation();

    // Step for 120 more frames (2 full seconds) while asleep
    for (int i = 0; i < 120; ++i) {
        scene.step(1.0f / 60.0f);
    }

    TEST_ASSERT(box->is_sleeping(), "Body remains asleep with no perturbations");
    TEST_ASSERT((box->get_position() - sleep_position).length_sq() == 0.0f, "Position has zero drift while asleep");
    TEST_ASSERT(std::abs(box->get_rotation().dot(sleep_rotation) - 1.0f) < 1e-6f, "Rotation has zero drift while asleep");
}

// ============================================================================
// Test 5: Collision Wakeup Propagation
// ============================================================================
void test_collision_wakeup_propagation() {
    std::cout << "[*] Testing collision wakeup propagation..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.enable_sleeping = true;
    settings.time_before_sleep = 0.2f;
    PhysicsScene scene(settings);

    BoxShape ground_shape(Vec3(20.0f, 1.0f, 20.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    (void)scene.create_body(ground_cs);

    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    BodyID box_id = scene.create_body(box_cs);

    // Let the resting box fall asleep
    for (int i = 0; i < 30; ++i) {
        scene.step(1.0f / 60.0f);
    }

    Body* box = scene.get_body(box_id);
    TEST_ASSERT(box->is_sleeping(), "Box is sleeping on ground");

    // Drop a dynamic sphere directly onto the sleeping box from y = 3.0
    SphereShape sphere_shape(0.5f);
    BodyCreationSettings sphere_cs;
    sphere_cs.shape = &sphere_shape;
    sphere_cs.motion_type = MotionType::Dynamic;
    sphere_cs.position = Vec3(0.0f, 3.0f, 0.0f);
    sphere_cs.override_mass = 5.0f; // Heavy projectile
    BodyID sphere_id = scene.create_body(sphere_cs);

    // Before collision: sphere is active, box is sleeping
    TEST_ASSERT(scene.get_body(sphere_id)->is_active(), "Dropped sphere is active");
    TEST_ASSERT(box->is_sleeping(), "Box is still sleeping as sphere falls");

    // Step until impact (approx 0.5s = 30 steps)
    bool collision_occurred = false;
    for (int i = 0; i < 60; ++i) {
        scene.step(1.0f / 60.0f);
        if (!box->is_sleeping()) {
            collision_occurred = true;
            break;
        }
    }

    TEST_ASSERT(collision_occurred, "Sleeping box was woken up upon impact by falling sphere");
    TEST_ASSERT(box->is_active(), "Box is active after collision");
    TEST_ASSERT(!box->is_sleeping(), "Box is not sleeping after collision");
    TEST_ASSERT(scene.get_island_builder().get_active_island_count() >= 1, "Active island contains the collision system");
}

// ============================================================================
// Test 6: External Impulse & Manual Wakeup
// ============================================================================
void test_external_impulse_wakeup() {
    std::cout << "[*] Testing external impulse & manual wakeup..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.enable_sleeping = true;
    settings.time_before_sleep = 0.2f;
    PhysicsScene scene(settings);

    BoxShape ground_shape(Vec3(20.0f, 1.0f, 20.0f));
    BodyCreationSettings ground_cs;
    ground_cs.shape = &ground_shape;
    ground_cs.motion_type = MotionType::Static;
    ground_cs.position = Vec3(0.0f, -1.0f, 0.0f);
    (void)scene.create_body(ground_cs);

    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));
    BodyCreationSettings box_cs;
    box_cs.shape = &box_shape;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    box_cs.override_mass = 1.0f;
    BodyID box_id = scene.create_body(box_cs);

    // Settle to sleep
    for (int i = 0; i < 30; ++i) {
        scene.step(1.0f / 60.0f);
    }

    Body* box = scene.get_body(box_id);
    TEST_ASSERT(box->is_sleeping(), "Box is sleeping");

    // 1. Manual scene.wake_up_body
    scene.wake_up_body(box_id);
    TEST_ASSERT(box->is_active(), "scene.wake_up_body activates the body");
    TEST_ASSERT(!box->is_sleeping(), "Body is no longer sleeping");

    // Put it back to sleep manually
    box->put_to_sleep();
    TEST_ASSERT(box->is_sleeping(), "Body put back to sleep");

    // 2. Applying an impulse must auto-wake the body
    box->apply_impulse(Vec3(0.0f, 10.0f, 0.0f));
    TEST_ASSERT(box->is_active(), "apply_impulse automatically wakes up sleeping body");
    TEST_ASSERT(!box->is_sleeping(), "Body is no longer sleeping after impulse");
    TEST_ASSERT(box->get_linear_velocity().y > 0.0f, "Impulse successfully applied");

    // Step simulation: body must ascend dynamically
    scene.step(1.0f / 60.0f);
    TEST_ASSERT(box->get_position().y > 0.5f, "Body ascended after upward impulse");
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "   Nisaba Sovereign 3D Physics Island Test Suite   " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_island_clustering();
    test_static_body_sharing();
    test_quiescent_sleep_transition();
    test_sleeping_stability();
    test_collision_wakeup_propagation();
    test_external_impulse_wakeup();

    std::cout << "=================================================" << std::endl;
    std::cout << "[+] ALL TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "=================================================" << std::endl;

    return 0;
}

#include "orion/physics/physics.hpp"
#include <cassert>
#include <iostream>
#include <cmath>

using namespace orion;
using namespace orion::physics;

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

void test_freefall_kinematics() {
    std::cout << "[*] Testing Free-Fall Symplectic Euler Kinematics..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene scene(settings);

    SphereShape sphere_shape(1.0f);

    BodyCreationSettings bcs;
    bcs.motion_type = MotionType::Dynamic;
    bcs.position = Vec3(0.0f, 100.0f, 0.0f);
    bcs.shape = &sphere_shape;
    bcs.override_mass = 2.0f;
    bcs.linear_damping = 0.0f;
    bcs.angular_damping = 0.0f;

    BodyID id = scene.create_body(bcs);
    TEST_ASSERT(id.is_valid(), "Dynamic body created successfully");

    const float dt = 1.0f / 60.0f;
    const int num_steps = 60; // 1 second of simulation

    for (int i = 0; i < num_steps; ++i) {
        scene.step(dt);
    }

    Body* body = scene.get_body(id);
    TEST_ASSERT(body != nullptr, "Body exists after 60 steps");

    // Symplectic Euler exact velocity after N steps: v_y = -N * g * dt
    const float expected_vy = -60.0f * 9.81f * dt; // -9.81 m/s
    const float actual_vy = body->get_linear_velocity().y;
    TEST_ASSERT(approx_equal(actual_vy, expected_vy, 0.05f), "Velocity matches symplectic Euler -9.81 m/s");

    // Symplectic Euler position: y_N = y_0 - g * dt^2 * N * (N + 1) / 2
    const float expected_y = 100.0f - 9.81f * (dt * dt) * (60.0f * 61.0f / 2.0f); // ~95.013 m
    const float actual_y = body->get_position().y;
    TEST_ASSERT(approx_equal(actual_y, expected_y, 0.1f), "Position matches symplectic Euler ~95.01 m");

    std::cout << "  [+] Free-fall kinematics verified (v_y = " << actual_vy << ", y = " << actual_y << ")." << std::endl;
}

void test_elastic_vs_inelastic_bounce() {
    std::cout << "[*] Testing Collision Restitution (Elastic vs Inelastic Bounce)..." << std::endl;

    // --- Case A: Inelastic Impact (e = 0.0) ---
    {
        PhysicsSettings settings;
        settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
        PhysicsScene scene(settings);

        PlaneShape floor_shape(Plane(Vec3::unit_y(), 0.0f));
        SphereShape ball_shape(1.0f);

        // Floor at y = 0
        BodyCreationSettings floor_cs;
        floor_cs.motion_type = MotionType::Static;
        floor_cs.position = Vec3(0.0f, 0.0f, 0.0f);
        floor_cs.shape = &floor_shape;
        floor_cs.restitution = 0.0f;
        [[maybe_unused]] auto fid = scene.create_body(floor_cs);

        // Inelastic sphere dropped from y = 3.0, radius = 1.0
        BodyCreationSettings ball_cs;
        ball_cs.motion_type = MotionType::Dynamic;
        ball_cs.position = Vec3(0.0f, 3.0f, 0.0f);
        ball_cs.shape = &ball_shape;
        ball_cs.override_mass = 1.0f;
        ball_cs.restitution = 0.0f;
        BodyID ball_id = scene.create_body(ball_cs);

        const float dt = 1.0f / 60.0f;
        for (int i = 0; i < 90; ++i) { // 1.5 seconds
            scene.step(dt);
        }

        Body* ball = scene.get_body(ball_id);
        TEST_ASSERT(ball != nullptr, "Ball exists");
        TEST_ASSERT(approx_equal(ball->get_position().y, 1.0f, 0.05f), "Inelastic ball rests at y = 1.0");
        TEST_ASSERT(std::abs(ball->get_linear_velocity().y) < 0.1f, "Inelastic ball velocity is zeroed at rest");
    }

    // --- Case B: Elastic Bounce (e = 0.8) ---
    {
        PhysicsSettings settings;
        settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
        PhysicsScene scene(settings);

        PlaneShape floor_shape(Plane(Vec3::unit_y(), 0.0f));
        SphereShape ball_shape(1.0f);

        BodyCreationSettings floor_cs;
        floor_cs.motion_type = MotionType::Static;
        floor_cs.position = Vec3::zero();
        floor_cs.shape = &floor_shape;
        floor_cs.restitution = 0.8f;
        [[maybe_unused]] auto fid = scene.create_body(floor_cs);

        BodyCreationSettings ball_cs;
        ball_cs.motion_type = MotionType::Dynamic;
        ball_cs.position = Vec3(0.0f, 4.0f, 0.0f);
        ball_cs.shape = &ball_shape;
        ball_cs.override_mass = 1.0f;
        ball_cs.restitution = 0.8f;
        BodyID ball_id = scene.create_body(ball_cs);

        const float dt = 1.0f / 60.0f;
        bool bounced_upward = false;
        float max_rebound_y = 1.0f;

        for (int i = 0; i < 90; ++i) {
            scene.step(dt);
            Body* ball = scene.get_body(ball_id);
            if (ball->get_linear_velocity().y > 0.5f) {
                bounced_upward = true;
            }
            max_rebound_y = std::max(max_rebound_y, ball->get_position().y);
        }

        TEST_ASSERT(bounced_upward, "Elastic sphere bounced upward with positive vy");
        TEST_ASSERT(max_rebound_y > 2.0f, "Elastic sphere reached rebound height > 2.0 m");
    }

    std::cout << "  [+] Restitution and bounce behavior verified." << std::endl;
}

void test_horizontal_friction_dissipation() {
    std::cout << "[*] Testing Coulomb Friction Cone & Kinetic Dissipation..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene scene(settings);

    PlaneShape floor_shape(Plane(Vec3::unit_y(), 0.0f));
    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));

    // Floor with friction mu = 0.5
    BodyCreationSettings floor_cs;
    floor_cs.motion_type = MotionType::Static;
    floor_cs.position = Vec3::zero();
    floor_cs.shape = &floor_shape;
    floor_cs.friction = 0.5f;
    [[maybe_unused]] auto fid = scene.create_body(floor_cs);

    // Box resting on floor (y = 0.5, half-extent = 0.5), moving right at vx = 8.0 m/s
    BodyCreationSettings box_cs;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    box_cs.shape = &box_shape;
    box_cs.override_mass = 10.0f;
    box_cs.friction = 0.5f;
    box_cs.linear_velocity = Vec3(8.0f, 0.0f, 0.0f);
    box_cs.linear_damping = 0.0f;
    box_cs.angular_damping = 0.0f;

    BodyID box_id = scene.create_body(box_cs);
    const float dt = 1.0f / 60.0f;

    // Simulate for 120 steps (2 seconds)
    float prev_vx = 8.0f;
    bool decelerated = true;

    for (int i = 0; i < 120; ++i) {
        scene.step(dt);
        Body* b = scene.get_body(box_id);
        float current_vx = b->get_linear_velocity().x;

        if (current_vx > prev_vx + 0.01f) {
            decelerated = false;
        }
        prev_vx = current_vx;
    }

    Body* box = scene.get_body(box_id);
    TEST_ASSERT(decelerated, "Box monotonically decelerated under friction");
    TEST_ASSERT(std::abs(box->get_linear_velocity().x) < 0.05f, "Friction brought sliding box to complete stop");
    TEST_ASSERT(box->get_linear_velocity().x >= -0.01f, "Friction did not reverse sliding direction");

    std::cout << "  [+] Friction dissipation verified (final vx = " << box->get_linear_velocity().x << ")." << std::endl;
}

void test_sphere_stack_equilibrium() {
    std::cout << "[*] Testing Sphere-on-Sphere Floor Stack Equilibrium..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    PhysicsScene scene(settings);

    PlaneShape floor_shape(Plane(Vec3::unit_y(), 0.0f));
    SphereShape s1_shape(0.5f);
    SphereShape s2_shape(0.5f);

    // Ground plane at y = 0
    BodyCreationSettings floor_cs;
    floor_cs.motion_type = MotionType::Static;
    floor_cs.shape = &floor_shape;
    floor_cs.friction = 0.5f;
    [[maybe_unused]] auto fid = scene.create_body(floor_cs);

    // Bottom sphere: radius = 0.5, pos = (0, 0.5, 0)
    BodyCreationSettings s1_cs;
    s1_cs.motion_type = MotionType::Dynamic;
    s1_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    s1_cs.shape = &s1_shape;
    s1_cs.override_mass = 1.0f;
    s1_cs.friction = 0.5f;
    BodyID s1_id = scene.create_body(s1_cs);

    // Top sphere: radius = 0.5, pos = (0, 1.5, 0)
    BodyCreationSettings s2_cs;
    s2_cs.motion_type = MotionType::Dynamic;
    s2_cs.position = Vec3(0.0f, 1.5f, 0.0f);
    s2_cs.shape = &s2_shape;
    s2_cs.override_mass = 1.0f;
    s2_cs.friction = 0.5f;
    BodyID s2_id = scene.create_body(s2_cs);

    const float dt = 1.0f / 60.0f;
    for (int i = 0; i < 120; ++i) {
        scene.step(dt);
    }

    Body* b1 = scene.get_body(s1_id);
    Body* b2 = scene.get_body(s2_id);

    TEST_ASSERT(b1 != nullptr && b2 != nullptr, "Both spheres alive");
    TEST_ASSERT(approx_equal(b1->get_position().y, 0.5f, 0.05f), "Bottom sphere rests at y = 0.5");
    TEST_ASSERT(approx_equal(b2->get_position().y, 1.5f, 0.05f), "Top sphere rests at y = 1.5");
    TEST_ASSERT(std::abs(b1->get_linear_velocity().y) < 0.05f, "Bottom sphere vy near 0");
    TEST_ASSERT(std::abs(b2->get_linear_velocity().y) < 0.05f, "Top sphere vy near 0");

    std::cout << "  [+] Sphere stack resting equilibrium verified." << std::endl;
}

void test_box_stack_equilibrium() {
    std::cout << "[*] Testing 3-Box Floor Resting Stack Equilibrium (The Ultimate Stability Test)..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.num_velocity_iterations = 10;
    settings.enable_warm_starting = true;
    PhysicsScene scene(settings);

    PlaneShape floor_shape(Plane(Vec3::unit_y(), 0.0f));
    BoxShape box1_shape(Vec3(0.5f, 0.5f, 0.5f));
    BoxShape box2_shape(Vec3(0.5f, 0.5f, 0.5f));
    BoxShape box3_shape(Vec3(0.5f, 0.5f, 0.5f));

    // Ground plane at y = 0
    BodyCreationSettings floor_cs;
    floor_cs.motion_type = MotionType::Static;
    floor_cs.shape = &floor_shape;
    floor_cs.friction = 0.6f;
    floor_cs.restitution = 0.0f;
    [[maybe_unused]] auto fid = scene.create_body(floor_cs);

    // Box 1 (Bottom): size 1x1x1 (half-extents 0.5), resting at y = 0.5
    BodyCreationSettings b1_cs;
    b1_cs.motion_type = MotionType::Dynamic;
    b1_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    b1_cs.shape = &box1_shape;
    b1_cs.override_mass = 5.0f;
    b1_cs.friction = 0.6f;
    b1_cs.restitution = 0.0f;
    BodyID b1_id = scene.create_body(b1_cs);

    // Box 2 (Middle): resting on Box 1 at y = 1.5
    BodyCreationSettings b2_cs;
    b2_cs.motion_type = MotionType::Dynamic;
    b2_cs.position = Vec3(0.0f, 1.5f, 0.0f);
    b2_cs.shape = &box2_shape;
    b2_cs.override_mass = 5.0f;
    b2_cs.friction = 0.6f;
    b2_cs.restitution = 0.0f;
    BodyID b2_id = scene.create_body(b2_cs);

    // Box 3 (Top): resting on Box 2 at y = 2.5
    BodyCreationSettings b3_cs;
    b3_cs.motion_type = MotionType::Dynamic;
    b3_cs.position = Vec3(0.0f, 2.5f, 0.0f);
    b3_cs.shape = &box3_shape;
    b3_cs.override_mass = 5.0f;
    b3_cs.friction = 0.6f;
    b3_cs.restitution = 0.0f;
    BodyID b3_id = scene.create_body(b3_cs);

    // Run simulation for 180 steps at 60Hz (3.0 seconds)
    const float dt = 1.0f / 60.0f;
    for (int step = 0; step < 180; ++step) {
        scene.step(dt);
    }

    Body* b1 = scene.get_body(b1_id);
    Body* b2 = scene.get_body(b2_id);
    Body* b3 = scene.get_body(b3_id);

    TEST_ASSERT(b1 != nullptr && b2 != nullptr && b3 != nullptr, "All 3 stacked boxes are alive");

    // 1. Position bounds: boxes must remain stably stacked without sinking into floor
    const float y1 = b1->get_position().y;
    const float y2 = b2->get_position().y;
    const float y3 = b3->get_position().y;

    TEST_ASSERT(approx_equal(y1, 0.50f, 0.04f), "Box 1 (bottom) resting altitude y ≈ 0.50 m");
    TEST_ASSERT(approx_equal(y2, 1.50f, 0.05f), "Box 2 (middle) resting altitude y ≈ 1.50 m");
    TEST_ASSERT(approx_equal(y3, 2.50f, 0.06f), "Box 3 (top) resting altitude y ≈ 2.50 m");

    // 2. Horizontal drift must remain negligible (< 0.05 m)
    TEST_ASSERT(std::abs(b1->get_position().x) < 0.05f, "Box 1 has zero horizontal drift x");
    TEST_ASSERT(std::abs(b1->get_position().z) < 0.05f, "Box 1 has zero horizontal drift z");
    TEST_ASSERT(std::abs(b2->get_position().x) < 0.05f, "Box 2 has zero horizontal drift x");
    TEST_ASSERT(std::abs(b2->get_position().z) < 0.05f, "Box 2 has zero horizontal drift z");
    TEST_ASSERT(std::abs(b3->get_position().x) < 0.05f, "Box 3 has zero horizontal drift x");
    TEST_ASSERT(std::abs(b3->get_position().z) < 0.05f, "Box 3 has zero horizontal drift z");

    // 3. Velocities must be settled to static rest
    TEST_ASSERT(b1->get_linear_velocity().length() < 0.05f, "Box 1 linear velocity settled to 0");
    TEST_ASSERT(b2->get_linear_velocity().length() < 0.05f, "Box 2 linear velocity settled to 0");
    TEST_ASSERT(b3->get_linear_velocity().length() < 0.05f, "Box 3 linear velocity settled to 0");

    TEST_ASSERT(b1->get_angular_velocity().length() < 0.05f, "Box 1 angular velocity settled to 0");
    TEST_ASSERT(b2->get_angular_velocity().length() < 0.05f, "Box 2 angular velocity settled to 0");
    TEST_ASSERT(b3->get_angular_velocity().length() < 0.05f, "Box 3 angular velocity settled to 0");

    std::cout << "  [+] 3-Box Floor Stack Equilibrium Verified:" << std::endl;
    std::cout << "      Box 1 y = " << y1 << " (error: " << std::abs(y1 - 0.5f) << " m)" << std::endl;
    std::cout << "      Box 2 y = " << y2 << " (error: " << std::abs(y2 - 1.5f) << " m)" << std::endl;
    std::cout << "      Box 3 y = " << y3 << " (error: " << std::abs(y3 - 2.5f) << " m)" << std::endl;
}

void test_newtonian_impulse_balance() {
    std::cout << "[*] Testing Newtonian Impulse Balance (sum(lambda_n) == M * g * dt)..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3(0.0f, -9.81f, 0.0f);
    settings.num_velocity_iterations = 10;
    settings.enable_warm_starting = true;
    settings.enable_sleeping = false;
    PhysicsScene scene(settings);

    PlaneShape floor_shape(Plane(Vec3::unit_y(), 0.0f));
    BoxShape box_shape(Vec3(0.5f, 0.5f, 0.5f));

    BodyCreationSettings floor_cs;
    floor_cs.motion_type = MotionType::Static;
    floor_cs.shape = &floor_shape;
    [[maybe_unused]] auto fid = scene.create_body(floor_cs);

    const float mass = 8.0f;
    BodyCreationSettings box_cs;
    box_cs.motion_type = MotionType::Dynamic;
    box_cs.position = Vec3(0.0f, 0.5f, 0.0f);
    box_cs.shape = &box_shape;
    box_cs.override_mass = mass;
    [[maybe_unused]] auto bid = scene.create_body(box_cs);

    const float dt = 1.0f / 60.0f;

    // Settle for 60 frames
    for (int i = 0; i < 60; ++i) {
        scene.step(dt);
    }

    // After settling, sum of accumulated impulses across contact points must balance gravity impulse
    const auto& constraints = scene.get_active_constraints();
    TEST_ASSERT(constraints.size() == 1, "Exactly 1 contact constraint active for single resting box");

    float total_normal_impulse = 0.0f;
    for (uint32_t p = 0; p < constraints[0].num_points; ++p) {
        total_normal_impulse += constraints[0].points[p].normal_impulse;
    }

    // Expected impulse J = F_gravity * dt = (m * g) * dt
    const float expected_impulse = mass * 9.81f * dt;
    TEST_ASSERT(approx_equal(total_normal_impulse, expected_impulse, 0.08f),
                "Total normal impulse equals gravity weight impulse J = m * g * dt");

    std::cout << "  [+] Newtonian balance verified: total lambda_n = " << total_normal_impulse
              << " N*s (expected: " << expected_impulse << " N*s)." << std::endl;
}

void test_spinning_box_angular_integration() {
    std::cout << "[*] Testing 3D Rotational Kinematics & Angular Damping..." << std::endl;

    PhysicsSettings settings;
    settings.gravity = Vec3::zero(); // Zero gravity
    PhysicsScene scene(settings);

    BoxShape box_shape(Vec3(1.0f, 2.0f, 3.0f));

    BodyCreationSettings bcs;
    bcs.motion_type = MotionType::Dynamic;
    bcs.position = Vec3::zero();
    bcs.shape = &box_shape;
    bcs.override_mass = 5.0f;
    bcs.angular_velocity = Vec3(0.0f, 10.0f, 0.0f); // 10 rad/s around Y
    bcs.angular_damping = 0.1f; // 10% damping per second

    BodyID id = scene.create_body(bcs);
    const float dt = 1.0f / 60.0f;

    for (int i = 0; i < 60; ++i) { // 1.0 second
        scene.step(dt);
    }

    Body* b = scene.get_body(id);
    TEST_ASSERT(b != nullptr, "Spinning body exists");

    // Quaternion must remain unit length
    const float quat_norm = b->get_rotation().length();
    TEST_ASSERT(approx_equal(quat_norm, 1.0f, 0.001f), "Rotation quaternion remains strictly normalized");

    // Position in zero gravity must stay at origin
    TEST_ASSERT(b->get_position().length() < 0.001f, "Center of mass position stationary during pure rotation");

    // Angular velocity must have attenuated smoothly under damping
    const float final_omega_y = b->get_angular_velocity().y;
    TEST_ASSERT(final_omega_y > 8.0f && final_omega_y < 10.0f, "Angular velocity smoothly damped");

    std::cout << "  [+] Rotational kinematics verified (final omega_y = " << final_omega_y << " rad/s)." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "  Nisaba Sovereign 3D Impulse Solver Tests       " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_freefall_kinematics();
    test_elastic_vs_inelastic_bounce();
    test_horizontal_friction_dissipation();
    test_sphere_stack_equilibrium();
    test_box_stack_equilibrium();
    test_newtonian_impulse_balance();
    test_spinning_box_angular_integration();

    std::cout << "=================================================" << std::endl;
    std::cout << "[+] ALL TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}

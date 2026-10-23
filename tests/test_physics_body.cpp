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

void test_memory_layout() {
    std::cout << "[*] Testing Body & MotionProperties Cache-Line Alignment..." << std::endl;

    TEST_ASSERT(sizeof(Body) == 128, "Body must be exactly 128 bytes (2 cache lines)");
    TEST_ASSERT(alignof(Body) == 16, "Body must be 16-byte aligned");

    TEST_ASSERT(sizeof(MotionProperties) == 128, "MotionProperties must be exactly 128 bytes (2 cache lines)");
    TEST_ASSERT(alignof(MotionProperties) == 16, "MotionProperties must be 16-byte aligned");

    TEST_ASSERT(sizeof(BodyID) == 4, "BodyID must be 4 bytes");

    std::cout << "  [+] Memory layout verified." << std::endl;
}

void test_body_id() {
    std::cout << "[*] Testing BodyID Handles..." << std::endl;

    BodyID default_id;
    TEST_ASSERT(!default_id.is_valid(), "Default BodyID is invalid");

    BodyID id1(42, 5);
    TEST_ASSERT(id1.is_valid(), "Constructed BodyID is valid");
    TEST_ASSERT(id1.index() == 42, "BodyID index == 42");
    TEST_ASSERT(id1.sequence() == 5, "BodyID sequence == 5");

    BodyID id2(42, 6);
    TEST_ASSERT(id1 != id2, "Different sequence numbers produce distinct BodyIDs");
    TEST_ASSERT(id1 < id2, "BodyID comparison operator");

    std::cout << "  [+] BodyID verified." << std::endl;
}

void test_sphere_shape() {
    std::cout << "[*] Testing SphereShape Analytics..." << std::endl;

    const float radius = 2.0f;
    SphereShape sphere(radius);
    TEST_ASSERT(sphere.get_type() == ShapeType::Sphere, "ShapeType is Sphere");
    TEST_ASSERT(sphere.radius == radius, "Sphere radius preserved");
    TEST_ASSERT(sphere.get_inner_radius() == radius, "Sphere inner radius == radius");

    // Local bounds
    AABB local_bounds = sphere.get_local_bounds();
    TEST_ASSERT(local_bounds.min_point == Vec3(-radius, -radius, -radius), "Sphere local bounds min");
    TEST_ASSERT(local_bounds.max_point == Vec3(radius, radius, radius), "Sphere local bounds max");

    // World bounds under rotation & translation
    Transform3D t(Vec3(10.0f, 20.0f, 30.0f), Quat::from_axis_angle(Vec3::unit_z(), HALF_PI));
    AABB world_bounds = sphere.get_world_bounds(t);
    TEST_ASSERT(world_bounds.center() == Vec3(10.0f, 20.0f, 30.0f), "Sphere world bounds center");
    TEST_ASSERT(world_bounds.half_extent() == Vec3(radius, radius, radius), "Sphere world bounds extents unchanged by rotation");

    // Mass properties
    const float density = 1000.0f;
    MassProperties mp = sphere.get_mass_properties(density);
    const float expected_volume = (4.0f / 3.0f) * PI * (radius * radius * radius);
    const float expected_mass = density * expected_volume;
    const float expected_inertia = 0.4f * expected_mass * (radius * radius);

    TEST_ASSERT(approx_equal(mp.mass, expected_mass), "Sphere analytical mass");
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.x, expected_inertia), "Sphere inertia Ixx");
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.y, expected_inertia), "Sphere inertia Iyy");
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.z, expected_inertia), "Sphere inertia Izz");

    // Support function
    Vec3 sup_x = sphere.get_support(Vec3(5.0f, 0.0f, 0.0f));
    TEST_ASSERT(sup_x == Vec3(radius, 0.0f, 0.0f), "Sphere support along +X");

    Vec3 sup_diag = sphere.get_support(Vec3(1.0f, 1.0f, 0.0f));
    const float inv_sqrt2 = 1.0f / std::sqrt(2.0f);
    TEST_ASSERT(approx_equal(sup_diag.x, radius * inv_sqrt2) && approx_equal(sup_diag.y, radius * inv_sqrt2), "Sphere support diagonal");

    std::cout << "  [+] SphereShape verified." << std::endl;
}

void test_box_shape() {
    std::cout << "[*] Testing BoxShape Analytics..." << std::endl;

    Vec3 half_extents(1.0f, 2.0f, 3.0f);
    BoxShape box(half_extents);
    TEST_ASSERT(box.get_type() == ShapeType::Box, "ShapeType is Box");
    TEST_ASSERT(box.get_inner_radius() == 1.0f, "Box inner radius is min half-extent");

    // Local bounds
    AABB local_bounds = box.get_local_bounds();
    TEST_ASSERT(local_bounds.min_point == -half_extents, "Box local bounds min");
    TEST_ASSERT(local_bounds.max_point == half_extents, "Box local bounds max");

    // Mass properties
    const float density = 1000.0f;
    MassProperties mp = box.get_mass_properties(density);
    const float expected_volume = 8.0f * (1.0f * 2.0f * 3.0f); // 48 m^3
    const float expected_mass = density * expected_volume;      // 48,000 kg
    TEST_ASSERT(approx_equal(mp.mass, expected_mass), "Box mass calculation");

    // Ixx = (1/3) * m * (hy^2 + hz^2) = (1/3) * 48000 * (4 + 9) = 16000 * 13 = 208,000
    // Iyy = (1/3) * m * (hx^2 + hz^2) = (1/3) * 48000 * (1 + 9) = 16000 * 10 = 160,000
    // Izz = (1/3) * m * (hx^2 + hy^2) = (1/3) * 48000 * (1 + 4) = 16000 * 5  = 80,000
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.x, 208000.0f), "Box Ixx inertia");
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.y, 160000.0f), "Box Iyy inertia");
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.z, 80000.0f), "Box Izz inertia");

    // Support function
    Vec3 sup = box.get_support(Vec3(1.0f, -0.5f, 2.0f));
    TEST_ASSERT(sup == Vec3(1.0f, -2.0f, 3.0f), "Box support function octant selection");

    // World bounds transformed by 90 deg rotation around Z
    Transform3D t_rot(Vec3::zero(), Quat::from_axis_angle(Vec3::unit_z(), HALF_PI));
    AABB world_b = box.get_world_bounds(t_rot);
    // After 90 deg rotation around Z: half_extents X and Y swap!
    TEST_ASSERT(approx_equal(world_b.half_extent().x, 2.0f), "Rotated Box bounds X == 2.0");
    TEST_ASSERT(approx_equal(world_b.half_extent().y, 1.0f), "Rotated Box bounds Y == 1.0");
    TEST_ASSERT(approx_equal(world_b.half_extent().z, 3.0f), "Rotated Box bounds Z == 3.0");

    std::cout << "  [+] BoxShape verified." << std::endl;
}

void test_capsule_shape() {
    std::cout << "[*] Testing CapsuleShape Analytics..." << std::endl;

    const float half_height = 2.0f;
    const float radius = 1.0f;
    CapsuleShape capsule(half_height, radius);
    TEST_ASSERT(capsule.get_type() == ShapeType::Capsule, "ShapeType is Capsule");
    TEST_ASSERT(capsule.get_inner_radius() == radius, "Capsule inner radius");

    // Local bounds
    AABB b = capsule.get_local_bounds();
    TEST_ASSERT(b.min_point == Vec3(-radius, -(half_height + radius), -radius), "Capsule local min bounds");
    TEST_ASSERT(b.max_point == Vec3(radius, half_height + radius, radius), "Capsule local max bounds");

    // Mass properties
    MassProperties mp = capsule.get_mass_properties(1000.0f);
    TEST_ASSERT(mp.mass > 0.0f, "Capsule mass > 0");
    TEST_ASSERT(mp.inertia_diagonal.x > 0.0f && mp.inertia_diagonal.y > 0.0f, "Capsule inertia > 0");
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.x, mp.inertia_diagonal.z), "Capsule symmetry Ixx == Izz");

    // Support function
    Vec3 sup_up = capsule.get_support(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(sup_up == Vec3(0.0f, half_height + radius, 0.0f), "Capsule top tip support");

    Vec3 sup_down = capsule.get_support(Vec3(0.0f, -1.0f, 0.0f));
    TEST_ASSERT(sup_down == Vec3(0.0f, -(half_height + radius), 0.0f), "Capsule bottom tip support");

    std::cout << "  [+] CapsuleShape verified." << std::endl;
}

void test_plane_shape() {
    std::cout << "[*] Testing PlaneShape..." << std::endl;

    Plane floor_plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f);
    PlaneShape plane(floor_plane, 500.0f);
    TEST_ASSERT(plane.get_type() == ShapeType::Plane, "ShapeType is Plane");

    MassProperties mp = plane.get_mass_properties();
    TEST_ASSERT(mp.mass == 0.0f, "Plane has 0 mass (infinite static mass)");

    Vec3 sup = plane.get_support(Vec3(1.0f, 0.0f, -1.0f));
    TEST_ASSERT(approx_equal(sup.x, 500.0f) && approx_equal(sup.z, -500.0f), "Plane boundary support");

    std::cout << "  [+] PlaneShape verified." << std::endl;
}

void test_static_body() {
    std::cout << "[*] Testing Static Body & Allocation Splitting..." << std::endl;

    BoxShape floor_box(Vec3(50.0f, 1.0f, 50.0f));
    BodyCreationSettings settings(
        &floor_box,
        Vec3(0.0f, -1.0f, 0.0f),
        Quat::identity(),
        MotionType::Static
    );
    settings.friction = 0.5f;
    settings.restitution = 0.1f;

    Body floor_body;
    floor_body.init(settings, nullptr); // Static bodies pass nullptr for MotionProperties!

    TEST_ASSERT(floor_body.is_static(), "Body is static");
    TEST_ASSERT(!floor_body.is_dynamic(), "Body is not dynamic");
    TEST_ASSERT(floor_body.get_motion_properties() == nullptr, "Static body allocates ZERO MotionProperties");
    TEST_ASSERT(floor_body.get_linear_velocity() == Vec3::zero(), "Static body linear velocity is 0");
    TEST_ASSERT(floor_body.get_angular_velocity() == Vec3::zero(), "Static body angular velocity is 0");

    // Applying impulses or forces to static bodies is safely ignored
    floor_body.apply_impulse(Vec3(100.0f, 0.0f, 0.0f));
    TEST_ASSERT(floor_body.get_linear_velocity() == Vec3::zero(), "Static body ignores impulses");

    floor_body.add_force(Vec3(0.0f, 1000.0f, 0.0f));
    TEST_ASSERT(floor_body.get_linear_velocity() == Vec3::zero(), "Static body ignores forces");

    // Bounds check
    AABB bounds = floor_body.get_bounds();
    TEST_ASSERT(bounds.min_point == Vec3(-50.0f, -2.0f, -50.0f), "Floor body min bounds");
    TEST_ASSERT(bounds.max_point == Vec3(50.0f, 0.0f, 50.0f), "Floor body max bounds");

    std::cout << "  [+] Static body verified." << std::endl;
}

void test_dynamic_body_and_impulses() {
    std::cout << "[*] Testing Dynamic Body & Impulse Dynamics..." << std::endl;

    SphereShape ball_shape(1.0f);
    BodyCreationSettings settings(
        &ball_shape,
        Vec3(0.0f, 10.0f, 0.0f),
        Quat::identity(),
        MotionType::Dynamic
    );
    settings.override_mass = 10.0f; // 10 kg
    settings.friction = 0.3f;
    settings.restitution = 0.8f;

    MotionProperties mp;
    Body ball_body;
    ball_body.init(settings, &mp);

    TEST_ASSERT(ball_body.is_dynamic(), "Body is dynamic");
    TEST_ASSERT(ball_body.get_motion_properties() == &mp, "Body holds MotionProperties pointer");
    TEST_ASSERT(approx_equal(mp.get_mass(), 10.0f), "Mass is 10 kg");
    TEST_ASSERT(approx_equal(mp.get_inv_mass(), 0.1f), "Inv mass is 0.1 kg^-1");

    // Test central linear impulse: J = (0, 50, 0) N*s on mass 10 kg -> delta_v = 5 m/s
    ball_body.apply_impulse(Vec3(0.0f, 50.0f, 0.0f));
    TEST_ASSERT(approx_equal(ball_body.get_linear_velocity().y, 5.0f), "Linear impulse v = J / m = 50 / 10 = 5 m/s");
    TEST_ASSERT(ball_body.get_angular_velocity() == Vec3::zero(), "Central impulse produces no angular velocity");

    // Test eccentric impulse at point (0, 11, 0) (top of sphere, r = (0, 1, 0))
    // J = (10, 0, 0) along +X.
    // Torque impulse tau = r x J = (0, 1, 0) x (10, 0, 0) = (0, 0, -10) N*s*m.
    // Sphere moment of inertia I = 0.4 * m * r^2 = 0.4 * 10 * 1 = 4 kg*m^2.
    // Angular velocity change delta_w = tau / I = -10 / 4 = -2.5 rad/s around Z.
    ball_body.apply_impulse_at_point(Vec3(10.0f, 0.0f, 0.0f), Vec3(0.0f, 11.0f, 0.0f));
    // Linear velocity increased by 10/10 = 1 m/s along X
    TEST_ASSERT(approx_equal(ball_body.get_linear_velocity().x, 1.0f), "Eccentric impulse linear velocity delta");
    TEST_ASSERT(approx_equal(ball_body.get_angular_velocity().z, -2.5f), "Eccentric impulse angular velocity delta w = -2.5 rad/s");

    // Test force accumulation at point
    ball_body.add_force_at_point(Vec3(100.0f, 0.0f, 0.0f), Vec3(0.0f, 11.0f, 0.0f));
    TEST_ASSERT(approx_equal(mp.get_accumulated_force().x, 100.0f), "Accumulated force");
    TEST_ASSERT(approx_equal(mp.get_accumulated_torque().z, -100.0f), "Accumulated torque tau = r x F");

    mp.reset_forces();
    TEST_ASSERT(mp.get_accumulated_force() == Vec3::zero(), "Forces reset");
    TEST_ASSERT(mp.get_accumulated_torque() == Vec3::zero(), "Torques reset");

    // Test get_inv_mass & point velocity
    TEST_ASSERT(approx_equal(ball_body.get_inv_mass(), 0.1f), "Body get_inv_mass matches");
    Vec3 pt_vel = ball_body.get_point_velocity(Vec3(0.0f, 11.0f, 0.0f));
    // pt_vel = v + w x (0, 1, 0)
    Vec3 expected_pt_vel = ball_body.get_linear_velocity() + ball_body.get_angular_velocity().cross(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(pt_vel.approx_equal(expected_pt_vel), "Body get_point_velocity matches kinematics");

    // Test velocity integration under gravity: ball dropping from y = 10 m
    mp.set_linear_velocity(Vec3::zero());
    mp.set_angular_velocity(Vec3::zero());
    const Vec3 gravity(0.0f, -9.81f, 0.0f);
    const float dt = 0.1f;
    mp.integrate_velocities(gravity, dt, ball_body.get_rotation());
    // v_y should be approx -9.81 * 0.1 = -0.981 m/s (factoring small damping)
    TEST_ASSERT(mp.get_linear_velocity().y < -0.97f && mp.get_linear_velocity().y > -0.99f, "Velocity integrated under gravity");

    // Test position integration: p_y should decrease
    const float old_y = ball_body.get_position().y;
    ball_body.integrate_positions(dt);
    TEST_ASSERT(ball_body.get_position().y < old_y, "Position integrated with velocity");

    std::cout << "  [+] Dynamic body & impulses verified." << std::endl;
}

void test_world_inverse_inertia() {
    std::cout << "[*] Testing World Space Inverse Inertia Decomposition..." << std::endl;

    // Asymmetric box: half extents (1, 2, 3) -> distinct Ixx, Iyy, Izz
    BoxShape box(Vec3(1.0f, 2.0f, 3.0f));
    MassProperties mp = box.get_mass_properties(1000.0f);

    MotionProperties motion;
    motion.set_mass_properties(mp);

    // Identity rotation: world inverse inertia along X equals inv_inertia_diagonal.x
    Vec3 inv_w_x = motion.multiply_world_inv_inertia_by_vector(Quat::identity(), Vec3::unit_x());
    TEST_ASSERT(approx_equal(inv_w_x.x, motion.get_inv_inertia_diagonal().x), "World inv inertia matches diagonal on identity rotation");

    // Rotate body by 90 degrees around Z axis: X and Y principal axes swap!
    Quat rot_z90 = Quat::from_axis_angle(Vec3::unit_z(), HALF_PI);
    Vec3 inv_w_rot = motion.multiply_world_inv_inertia_by_vector(rot_z90, Vec3::unit_x());
    // In world coordinates, unit_x now maps to local +Y, so it should scale by inv_inertia_diagonal.y!
    TEST_ASSERT(approx_equal(inv_w_rot.x, motion.get_inv_inertia_diagonal().y), "World inv inertia rotates correctly under 90 deg rotation");

    std::cout << "  [+] World space inverse inertia verified." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "   Nisaba Sovereign 3D Physics Body Test Suite   " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_memory_layout();
    test_body_id();
    test_sphere_shape();
    test_box_shape();
    test_capsule_shape();
    test_plane_shape();
    test_static_body();
    test_dynamic_body_and_impulses();
    test_world_inverse_inertia();

    std::cout << "=================================================" << std::endl;
    std::cout << "[+] ALL TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}

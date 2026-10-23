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

void test_combine_rules() {
    std::cout << "[*] Testing Collision Combining Rules..." << std::endl;

    // Friction combining
    float f1 = 0.2f, f2 = 0.8f;
    float geom_f = combine_friction(f1, f2, CombineMode::GeometricMean); // sqrt(0.16) = 0.4
    TEST_ASSERT(approx_equal(geom_f, 0.4f), "Geometric mean friction");

    float min_f = combine_friction(f1, f2, CombineMode::Minimum);
    TEST_ASSERT(approx_equal(min_f, 0.2f), "Minimum friction");

    float max_f = combine_friction(f1, f2, CombineMode::Maximum);
    TEST_ASSERT(approx_equal(max_f, 0.8f), "Maximum friction");

    // Restitution combining
    float r1 = 0.1f, r2 = 0.9f;
    float max_r = combine_restitution(r1, r2, CombineMode::Maximum);
    TEST_ASSERT(approx_equal(max_r, 0.9f), "Maximum restitution (default)");

    float avg_r = combine_restitution(r1, r2, CombineMode::Average);
    TEST_ASSERT(approx_equal(avg_r, 0.5f), "Average restitution");

    std::cout << "  [+] Combine rules verified." << std::endl;
}

void test_contact_manifold_basics() {
    std::cout << "[*] Testing ContactManifold & Pruning..." << std::endl;

    BodyID id_a(1, 1);
    BodyID id_b(2, 1);
    ContactManifold manifold(id_a, id_b, Vec3::unit_y());

    TEST_ASSERT(manifold.is_empty(), "New manifold is empty");
    TEST_ASSERT(manifold.num_points == 0, "Initial num_points == 0");

    // Add 1st point
    ContactPoint cp1(Vec3(0, 0, 0), Vec3(0, -0.1f, 0), Vec3::unit_y(), 0.1f, 101);
    manifold.add_point(cp1);
    TEST_ASSERT(manifold.num_points == 1, "1 point added");
    TEST_ASSERT(manifold.points[0].penetration_depth == 0.1f, "Point depth matches");

    // Add 2nd, 3rd, 4th points
    manifold.add_point(ContactPoint(Vec3(1, 0, 0), Vec3(1, -0.1f, 0), Vec3::unit_y(), 0.1f, 102));
    manifold.add_point(ContactPoint(Vec3(0, 0, 1), Vec3(0, -0.1f, 1), Vec3::unit_y(), 0.1f, 103));
    manifold.add_point(ContactPoint(Vec3(1, 0, 1), Vec3(1, -0.1f, 1), Vec3::unit_y(), 0.1f, 104));
    TEST_ASSERT(manifold.num_points == 4, "4 points added (capacity reached)");

    // Add 5th point (should prune and retain exactly 4 points)
    manifold.add_point(ContactPoint(Vec3(0.5f, 0, 0.5f), Vec3(0.5f, -0.1f, 0.5f), Vec3::unit_y(), 0.05f, 105));
    TEST_ASSERT(manifold.num_points == 4, "Pruned back to exactly 4 points");

    // Test swap_bodies
    ContactManifold swapped = manifold.swap_bodies();
    TEST_ASSERT(swapped.body_a == id_b && swapped.body_b == id_a, "Bodies swapped");
    TEST_ASSERT(swapped.normal == -Vec3::unit_y(), "Normal inverted in swapped manifold");
    TEST_ASSERT(swapped.num_points == 4, "Point count preserved");
    TEST_ASSERT(swapped.points[0].position_on_a == manifold.points[0].position_on_b, "Contact endpoints swapped");

    std::cout << "  [+] ContactManifold verified." << std::endl;
}

void test_sphere_sphere_collision() {
    std::cout << "[*] Testing Analytical Sphere vs Sphere Collision..." << std::endl;

    SphereShape s1(1.0f);
    SphereShape s2(2.0f);

    Body b1, b2;
    BodyCreationSettings c1(&s1, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    c1.friction = 0.4f; c1.restitution = 0.5f;
    BodyCreationSettings c2(&s2, Vec3(5.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    c2.friction = 0.4f; c2.restitution = 0.5f;
    b1.init(c1); b1.set_id(BodyID(1, 0));
    b2.init(c2); b2.set_id(BodyID(2, 0));

    // Case 1: Separated (distance = 5 > 1 + 2 = 3)
    ContactManifold manifold;
    bool hit = NarrowPhase::collide_bodies(b1, b2, manifold);
    TEST_ASSERT(!hit, "Separated spheres do not collide");
    TEST_ASSERT(manifold.is_empty(), "Manifold is empty for separated spheres");

    // Case 2: Overlapping (move b2 to (2.5, 0, 0), distance = 2.5 < 3.0, depth = 0.5)
    b2.set_position(Vec3(2.5f, 0.0f, 0.0f));
    hit = NarrowPhase::collide_bodies(b1, b2, manifold);
    TEST_ASSERT(hit, "Overlapping spheres collide");
    TEST_ASSERT(manifold.num_points == 1, "Sphere-sphere produces exactly 1 contact point");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.5f), "Sphere-sphere penetration depth == 0.5");
    // Normal points from b1 to b2 along +X
    TEST_ASSERT(manifold.normal == Vec3(1.0f, 0.0f, 0.0f), "Sphere-sphere normal points along +X");
    TEST_ASSERT(approx_equal(manifold.points[0].position_on_a.x, 1.0f), "Contact point on sphere A at x=1.0");
    TEST_ASSERT(approx_equal(manifold.points[0].position_on_b.x, 0.5f), "Contact point on sphere B at x=0.5");
    TEST_ASSERT(approx_equal(manifold.combined_friction, 0.4f), "Combined friction matches");
    TEST_ASSERT(approx_equal(manifold.combined_restitution, 0.5f), "Combined restitution matches");

    std::cout << "  [+] Sphere vs Sphere verified." << std::endl;
}

void test_sphere_plane_collision() {
    std::cout << "[*] Testing Analytical Sphere vs Plane Collision..." << std::endl;

    SphereShape sphere(1.0f);
    PlaneShape plane(Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f), 500.0f); // Horizontal floor at y=0

    Body ball, floor;
    BodyCreationSettings c_ball(&sphere, Vec3(0.0f, 2.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    BodyCreationSettings c_floor(&plane, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Static);
    ball.init(c_ball); ball.set_id(BodyID(10, 0));
    floor.init(c_floor); floor.set_id(BodyID(20, 0));

    // Case 1: Ball above floor (y = 2.0 > radius 1.0)
    ContactManifold manifold;
    bool hit = NarrowPhase::collide_bodies(ball, floor, manifold);
    TEST_ASSERT(!hit, "Sphere above floor does not collide");

    // Case 2: Ball penetrating floor (y = 0.7, radius = 1.0 -> depth = 0.3)
    ball.set_position(Vec3(0.0f, 0.7f, 0.0f));
    hit = NarrowPhase::collide_bodies(ball, floor, manifold);
    TEST_ASSERT(hit, "Sphere penetrating floor collides");
    TEST_ASSERT(manifold.num_points == 1, "Sphere-plane produces 1 contact point");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.3f), "Sphere-plane depth == 0.3");
    // Normal points from sphere into floor: (0, -1, 0)
    TEST_ASSERT(manifold.normal == Vec3(0.0f, -1.0f, 0.0f), "Sphere-plane normal points down into floor");
    TEST_ASSERT(approx_equal(manifold.points[0].position_on_a.y, -0.3f), "Contact point on sphere lowest point y = -0.3");
    TEST_ASSERT(approx_equal(manifold.points[0].position_on_b.y, 0.0f), "Contact point on floor surface y = 0.0");

    // Case 3: Commutative test (floor vs ball)
    ContactManifold swapped_manifold;
    bool swapped_hit = NarrowPhase::collide_bodies(floor, ball, swapped_manifold);
    TEST_ASSERT(swapped_hit, "Plane vs Sphere collides via swap");
    TEST_ASSERT(swapped_manifold.normal == Vec3(0.0f, 1.0f, 0.0f), "Plane vs Sphere normal points up out of floor");
    TEST_ASSERT(approx_equal(swapped_manifold.points[0].penetration_depth, 0.3f), "Swapped depth matches");

    std::cout << "  [+] Sphere vs Plane verified." << std::endl;
}

void test_sphere_box_collision() {
    std::cout << "[*] Testing Analytical Sphere vs Box Collision..." << std::endl;

    SphereShape sphere(0.5f);
    BoxShape box(Vec3(1.0f, 1.0f, 1.0f)); // Box [-1, 1] in all axes

    Body ball, cube;
    BodyCreationSettings c_ball(&sphere, Vec3(2.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    BodyCreationSettings c_cube(&box, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Static);
    ball.init(c_ball); ball.set_id(BodyID(1, 0));
    cube.init(c_cube); cube.set_id(BodyID(2, 0));

    // Case 1: Ball separated from face (x = 2.0 > 1.0 + 0.5 = 1.5)
    ContactManifold manifold;
    TEST_ASSERT(!NarrowPhase::collide_bodies(ball, cube, manifold), "Separated sphere-box");

    // Case 2: Ball penetrating +X face (x = 1.3 -> depth = 1.5 - 1.3 = 0.2)
    ball.set_position(Vec3(1.3f, 0.0f, 0.0f));
    bool hit = NarrowPhase::collide_bodies(ball, cube, manifold);
    TEST_ASSERT(hit, "Sphere penetrating box face collides");
    TEST_ASSERT(manifold.num_points == 1, "Sphere-box produces 1 contact point");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.2f), "Sphere-box penetration depth == 0.2");
    // Normal points from ball to cube (-X)
    TEST_ASSERT(manifold.normal == Vec3(-1.0f, 0.0f, 0.0f), "Sphere-box normal points along -X into cube");
    TEST_ASSERT(approx_equal(manifold.points[0].position_on_b.x, 1.0f), "Contact point on cube face x = 1.0");

    // Case 3: Ball penetrating box corner (x = 1.2, y = 1.2, z = 0)
    // Closest point on box is (1.0, 1.0, 0.0). Distance = sqrt(0.04 + 0.04) = sqrt(0.08) approx 0.2828.
    // Depth = 0.5 - 0.2828 = 0.2172 > 0.
    ball.set_position(Vec3(1.2f, 1.2f, 0.0f));
    hit = NarrowPhase::collide_bodies(ball, cube, manifold);
    TEST_ASSERT(hit, "Sphere penetrating box corner collides");
    const float expected_dist = std::sqrt(0.08f);
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.5f - expected_dist), "Corner penetration depth matches");
    TEST_ASSERT(approx_equal(manifold.points[0].position_on_b.x, 1.0f) && approx_equal(manifold.points[0].position_on_b.y, 1.0f), "Contact point on cube corner (1, 1, 0)");

    std::cout << "  [+] Sphere vs Box verified." << std::endl;
}

void test_box_plane_collision() {
    std::cout << "[*] Testing Analytical Box vs Plane Collision & Manifold Generation..." << std::endl;

    BoxShape box(Vec3(1.0f, 1.0f, 1.0f)); // 2x2x2 cube
    PlaneShape plane(Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f), 500.0f); // Floor at y=0

    Body cube, floor;
    BodyCreationSettings c_cube(&box, Vec3(0.0f, 5.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    BodyCreationSettings c_floor(&plane, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Static);
    cube.init(c_cube); cube.set_id(BodyID(10, 0));
    floor.init(c_floor); floor.set_id(BodyID(20, 0));

    // Case 1: Cube floating high above floor
    ContactManifold manifold;
    TEST_ASSERT(!NarrowPhase::collide_bodies(cube, floor, manifold), "Floating cube does not collide with floor");

    // Case 2: Cube resting flat on floor (bottom face y = 0.8 -> penetration depth = 1.0 - 0.8 = 0.2)
    cube.set_position(Vec3(0.0f, 0.8f, 0.0f));
    bool hit = NarrowPhase::collide_bodies(cube, floor, manifold);
    TEST_ASSERT(hit, "Flat cube penetrating floor collides");
    // A flat box on a plane generates exactly 4 contact points (the 4 bottom face corners)!
    TEST_ASSERT(manifold.num_points == 4, "Flat box on plane generates exactly 4 contact points (a quad manifold)");
    TEST_ASSERT(manifold.normal == Vec3(0.0f, -1.0f, 0.0f), "Box-plane normal points down into floor");
    for (uint32_t i = 0; i < manifold.num_points; ++i) {
        TEST_ASSERT(approx_equal(manifold.points[i].penetration_depth, 0.2f), "All 4 quad contact points have depth 0.2");
        TEST_ASSERT(approx_equal(manifold.points[i].position_on_b.y, 0.0f), "All 4 contact points touch floor y=0");
    }

    // Case 3: Cube tilted so only 1 corner penetrates floor
    // Rotate 45 deg around Z and 45 deg around X (corner pointing down)
    Quat tilt = Quat::from_axis_angle(Vec3(1.0f, 0.0f, 1.0f).normalized(), 0.6f);
    cube.set_position_and_rotation(Vec3(0.0f, 1.5f, 0.0f), tilt);
    hit = NarrowPhase::collide_bodies(cube, floor, manifold);
    TEST_ASSERT(hit, "Tilted cube penetrates floor");
    TEST_ASSERT(manifold.num_points == 1, "Cube on single corner generates exactly 1 contact point");

    std::cout << "  [+] Box vs Plane verified." << std::endl;
}

void test_capsule_plane_collision() {
    std::cout << "[*] Testing Analytical Capsule vs Plane Collision..." << std::endl;

    CapsuleShape capsule(2.0f, 1.0f); // half_height = 2.0, radius = 1.0
    PlaneShape plane(Plane(Vec3(0.0f, 1.0f, 0.0f), 0.0f), 500.0f);

    Body cap_body, floor;
    BodyCreationSettings c_cap(&capsule, Vec3(0.0f, 10.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    BodyCreationSettings c_floor(&plane, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Static);
    cap_body.init(c_cap); cap_body.set_id(BodyID(10, 0));
    floor.init(c_floor); floor.set_id(BodyID(20, 0));

    // Case 1: Vertical capsule resting on bottom tip (bottom endpoint y = -2, bottom tip y = -3)
    // Position at y = 2.7 -> bottom tip at y = 2.7 - 3.0 = -0.3 (depth = 0.3)
    cap_body.set_position(Vec3(0.0f, 2.7f, 0.0f));
    ContactManifold manifold;
    bool hit = NarrowPhase::collide_bodies(cap_body, floor, manifold);
    TEST_ASSERT(hit, "Vertical capsule penetrating floor collides");
    TEST_ASSERT(manifold.num_points == 1, "Vertical capsule produces 1 contact point on tip");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.3f), "Vertical capsule depth == 0.3");

    // Case 2: Horizontal capsule lying flat on floor (rotated 90 deg around Z)
    // Capsule core segment along X. Radius = 1.0. Position y = 0.8 -> depth = 0.2 at both endpoints!
    Quat rot_z90 = Quat::from_axis_angle(Vec3::unit_z(), HALF_PI);
    cap_body.set_position_and_rotation(Vec3(0.0f, 0.8f, 0.0f), rot_z90);
    hit = NarrowPhase::collide_bodies(cap_body, floor, manifold);
    TEST_ASSERT(hit, "Horizontal capsule flat on floor collides");
    TEST_ASSERT(manifold.num_points == 2, "Horizontal capsule produces 2 contact points (both ends)");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.2f), "End 1 depth == 0.2");
    TEST_ASSERT(approx_equal(manifold.points[1].penetration_depth, 0.2f), "End 2 depth == 0.2");

    std::cout << "  [+] Capsule vs Plane verified." << std::endl;
}

void test_sphere_capsule_collision() {
    std::cout << "[*] Testing Analytical Sphere vs Capsule Collision..." << std::endl;

    SphereShape sphere(1.0f);
    CapsuleShape capsule(2.0f, 1.0f); // cylinder y in [-2, 2], radius 1.0

    Body ball, cap;
    BodyCreationSettings c_ball(&sphere, Vec3(5.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    BodyCreationSettings c_cap(&capsule, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Static);
    ball.init(c_ball); ball.set_id(BodyID(1, 0));
    cap.init(c_cap); cap.set_id(BodyID(2, 0));

    // Case 1: Ball separated (x = 5.0 > 1.0 + 1.0 = 2.0)
    ContactManifold manifold;
    TEST_ASSERT(!NarrowPhase::collide_bodies(ball, cap, manifold), "Separated sphere-capsule");

    // Case 2: Ball touching capsule cylinder mid-section (x = 1.7, y = 0, z = 0)
    // Distance to core segment is 1.7. Radius sum = 1 + 1 = 2. Depth = 2.0 - 1.7 = 0.3.
    ball.set_position(Vec3(1.7f, 0.0f, 0.0f));
    bool hit = NarrowPhase::collide_bodies(ball, cap, manifold);
    TEST_ASSERT(hit, "Sphere penetrating capsule cylinder section collides");
    TEST_ASSERT(manifold.num_points == 1, "Sphere-capsule produces 1 contact point");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.3f), "Sphere-capsule depth == 0.3");

    // Case 3: Ball touching capsule hemisphere cap (x = 0, y = 3.5, z = 0)
    // Capsule top endpoint is (0, 2, 0). Distance from (0, 3.5, 0) is 1.5. Depth = 2.0 - 1.5 = 0.5.
    ball.set_position(Vec3(0.0f, 3.5f, 0.0f));
    hit = NarrowPhase::collide_bodies(ball, cap, manifold);
    TEST_ASSERT(hit, "Sphere penetrating capsule top hemispherical cap collides");
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.5f), "Cap depth == 0.5");

    std::cout << "  [+] Sphere vs Capsule verified." << std::endl;
}

void test_box_box_collision() {
    std::cout << "[*] Testing Analytical Box vs Box Collision (SAT & Sutherland-Hodgman Face Clipping)..." << std::endl;

    BoxShape box_shape(Vec3(1.0f, 1.0f, 1.0f));

    BodyCreationSettings cs_a(&box_shape, Vec3::zero());
    Body box_a;
    box_a.init(cs_a);
    box_a.set_id(BodyID(1, 1));

    // Case 1: Disjoint Boxes
    BodyCreationSettings cs_b_separated(&box_shape, Vec3(3.5f, 0.0f, 0.0f));
    Body box_b_separated;
    box_b_separated.init(cs_b_separated);
    box_b_separated.set_id(BodyID(2, 1));

    ContactManifold manifold;
    bool hit = NarrowPhase::collide_bodies(box_a, box_b_separated, manifold);
    TEST_ASSERT(!hit, "Disjoint boxes along X axis return false");

    // Case 2: Face-to-Face Stacking Collision along Y axis
    // Box A at (0, 0, 0), Box B at (0, 1.8, 0). Half-extents are 1.0. Overlap depth = 2.0 - 1.8 = 0.2.
    BodyCreationSettings cs_b_stacked(&box_shape, Vec3(0.0f, 1.8f, 0.0f));
    Body box_b_stacked;
    box_b_stacked.init(cs_b_stacked);
    box_b_stacked.set_id(BodyID(3, 1));

    hit = NarrowPhase::collide_bodies(box_a, box_b_stacked, manifold);
    TEST_ASSERT(hit, "Overlapping stacked boxes return true");
    TEST_ASSERT(manifold.num_points == 4, "Face-to-face box collision produces exactly 4 quad contact points");
    TEST_ASSERT(approx_equal(manifold.normal.y, 1.0f, 0.01f), "Collision normal points upward +Y");

    for (uint32_t i = 0; i < manifold.num_points; ++i) {
        TEST_ASSERT(approx_equal(manifold.points[i].penetration_depth, 0.2f, 0.05f),
                    "Each contact point has expected penetration depth ~0.2");
        TEST_ASSERT(std::abs(manifold.points[i].position_on_a.y - 1.0f) < 0.05f,
                    "Points on Box A lie on top face y = 1.0");
        TEST_ASSERT(std::abs(manifold.points[i].position_on_b.y - 0.8f) < 0.05f,
                    "Points on Box B lie on bottom face y = 0.8");
    }

    // Case 3: Rotated Box (45 degrees around Y)
    const Quat rot_y45 = Quat::from_axis_angle(Vec3::unit_y(), 0.785398f);
    BodyCreationSettings cs_b_rot(&box_shape, Vec3(0.0f, 1.8f, 0.0f), rot_y45);
    Body box_b_rot;
    box_b_rot.init(cs_b_rot);
    box_b_rot.set_id(BodyID(4, 1));

    hit = NarrowPhase::collide_bodies(box_a, box_b_rot, manifold);
    TEST_ASSERT(hit, "Overlapping rotated boxes return true");
    TEST_ASSERT(manifold.num_points >= 4, "Rotated box face contact generates clipped manifold");
    TEST_ASSERT(approx_equal(manifold.normal.y, 1.0f, 0.05f), "Rotated contact normal points along +Y");

    std::cout << "  [+] Box vs Box verified (4-point quad manifold generated)." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "  Nisaba Sovereign 3D Contact & Manifold Tests   " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_combine_rules();
    test_contact_manifold_basics();
    test_sphere_sphere_collision();
    test_sphere_plane_collision();
    test_sphere_box_collision();
    test_box_plane_collision();
    test_capsule_plane_collision();
    test_sphere_capsule_collision();
    test_box_box_collision();

    std::cout << "=================================================" << std::endl;
    std::cout << "[+] ALL TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}

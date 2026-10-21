#include "nisaba/physics/physics.hpp"
#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>

using namespace nisaba::physics;

static int g_assert_count = 0;
#define TEST_ASSERT(cond) do { \
    if (!(cond)) { \
        std::cerr << "Assertion FAILED: " #cond " at " __FILE__ ":" << __LINE__ << std::endl; \
        std::abort(); \
    } \
    ++g_assert_count; \
} while(0)

static bool approx_equal_vec3(const Vec3& a, const Vec3& b, float eps = 1e-3f) {
    return nisaba::physics::approx_equal(a.x, b.x, eps) &&
           nisaba::physics::approx_equal(a.y, b.y, eps) &&
           nisaba::physics::approx_equal(a.z, b.z, eps);
}

void test_cylinder_shape() {
    std::cout << "Testing CylinderShape..." << std::endl;

    CylinderShape cylinder(1.0f, 0.5f); // h = 1.0, r = 0.5
    TEST_ASSERT(cylinder.get_type() == ShapeType::Cylinder);
    TEST_ASSERT(approx_equal(cylinder.half_height, 1.0f));
    TEST_ASSERT(approx_equal(cylinder.radius, 0.5f));
    TEST_ASSERT(cylinder.is_convex());

    // Local bounds
    AABB bounds = cylinder.get_local_bounds();
    TEST_ASSERT(approx_equal_vec3(bounds.min_point, Vec3(-0.5f, -1.0f, -0.5f)));
    TEST_ASSERT(approx_equal_vec3(bounds.max_point, Vec3(0.5f, 1.0f, 0.5f)));

    // Mass properties
    MassProperties mp = cylinder.get_mass_properties(1000.0f);
    // V = pi * r^2 * 2h = pi * 0.25 * 2 = 0.5 * pi ~ 1.570796 kg
    float expected_mass = 1000.0f * PI * 0.25f * 2.0f;
    TEST_ASSERT(approx_equal(mp.mass, expected_mass, 1.0f));
    TEST_ASSERT(mp.inertia_diagonal.x > 0.0f);
    TEST_ASSERT(mp.inertia_diagonal.y > 0.0f);
    TEST_ASSERT(approx_equal(mp.inertia_diagonal.x, mp.inertia_diagonal.z));

    // Support function tests
    Vec3 sup_up = cylinder.get_support(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(approx_equal(sup_up.y, 1.0f));

    Vec3 sup_down = cylinder.get_support(Vec3(0.0f, -1.0f, 0.0f));
    TEST_ASSERT(approx_equal(sup_down.y, -1.0f));

    Vec3 sup_right = cylinder.get_support(Vec3(1.0f, 0.0f, 0.0f));
    TEST_ASSERT(approx_equal(sup_right.x, 0.5f));
    TEST_ASSERT(approx_equal(sup_right.y, 1.0f)); // direction.y >= 0 -> +h
    TEST_ASSERT(approx_equal(sup_right.z, 0.0f));

    Vec3 sup_diag = cylinder.get_support(Vec3(1.0f, -1.0f, 1.0f));
    TEST_ASSERT(approx_equal(sup_diag.y, -1.0f));
    float r_diag = std::sqrt(sup_diag.x * sup_diag.x + sup_diag.z * sup_diag.z);
    TEST_ASSERT(approx_equal(r_diag, 0.5f));
}

void test_convex_hull_shape() {
    std::cout << "Testing ConvexHullShape..." << std::endl;

    // Construct a 3D pyramid with square base in XZ and apex along +Y
    std::vector<Vec3> pyramid_verts = {
        Vec3(-1.0f, 0.0f, -1.0f),
        Vec3( 1.0f, 0.0f, -1.0f),
        Vec3( 1.0f, 0.0f,  1.0f),
        Vec3(-1.0f, 0.0f,  1.0f),
        Vec3( 0.0f,  2.0f,  0.0f) // apex
    };

    ConvexHullShape hull(pyramid_verts);
    TEST_ASSERT(hull.get_type() == ShapeType::ConvexHull);
    TEST_ASSERT(hull.is_convex());
    TEST_ASSERT(hull.vertices.size() == 5);

    AABB bounds = hull.get_local_bounds();
    TEST_ASSERT(approx_equal_vec3(bounds.min_point, Vec3(-1.0f, 0.0f, -1.0f)));
    TEST_ASSERT(approx_equal_vec3(bounds.max_point, Vec3(1.0f, 2.0f, 1.0f)));

    // Support tests
    Vec3 sup_apex = hull.get_support(Vec3(0.0f, 1.0f, 0.0f));
    TEST_ASSERT(approx_equal_vec3(sup_apex, Vec3(0.0f, 2.0f, 0.0f)));

    Vec3 sup_corner = hull.get_support(Vec3(1.0f, -1.0f, 1.0f));
    TEST_ASSERT(approx_equal_vec3(sup_corner, Vec3(1.0f, 0.0f, 1.0f)));

    MassProperties mp = hull.get_mass_properties(1000.0f);
    TEST_ASSERT(mp.mass > 0.0f);
    TEST_ASSERT(mp.inertia_diagonal.x > 0.0f);
}

void test_gjk_disjoint_shapes() {
    std::cout << "Testing GJK with disjoint shapes..." << std::endl;

    // 1. Two disjoint spheres
    SphereShape s1(1.0f);
    SphereShape s2(1.0f);
    Transform3D t1 = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D t2 = Transform3D::from_translation(Vec3(5.0f, 0.0f, 0.0f));

    GJKResult res_spheres = GJK::query(s1, t1, s2, t2);
    TEST_ASSERT(!res_spheres.intersecting);
    // Center distance is 5.0, radii sum is 2.0, exact surface distance is 3.0
    TEST_ASSERT(approx_equal(res_spheres.distance, 3.0f, 1e-2f));
    TEST_ASSERT(approx_equal_vec3(res_spheres.closest_point_a, Vec3(1.0f, 0.0f, 0.0f), 1e-2f));
    TEST_ASSERT(approx_equal_vec3(res_spheres.closest_point_b, Vec3(4.0f, 0.0f, 0.0f), 1e-2f));

    // 2. Disjoint boxes along Y
    BoxShape b1(Vec3(1.0f, 1.0f, 1.0f));
    BoxShape b2(Vec3(1.0f, 1.0f, 1.0f));
    Transform3D tb1 = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D tb2 = Transform3D::from_translation(Vec3(0.0f, 4.0f, 0.0f));

    GJKResult res_boxes = GJK::query(b1, tb1, b2, tb2);
    TEST_ASSERT(!res_boxes.intersecting);
    // Distance should be 4.0 - 2.0 = 2.0
    TEST_ASSERT(approx_equal(res_boxes.distance, 2.0f, 1e-2f));
    TEST_ASSERT(approx_equal(res_boxes.closest_point_a.y, 1.0f, 1e-2f));
    TEST_ASSERT(approx_equal(res_boxes.closest_point_b.y, 3.0f, 1e-2f));

    // 3. Cylinder and Box separated along Z
    CylinderShape cyl(1.0f, 0.5f);
    BoxShape box(Vec3(0.5f, 0.5f, 0.5f));
    Transform3D t_cyl = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D t_box = Transform3D::from_translation(Vec3(0.0f, 0.0f, 3.0f));

    GJKResult res_cyl_box = GJK::query(cyl, t_cyl, box, t_box);
    TEST_ASSERT(!res_cyl_box.intersecting);
    std::cout << "res_cyl_box dist=" << res_cyl_box.distance << ", pt_a=("
              << res_cyl_box.closest_point_a.x << "," << res_cyl_box.closest_point_a.y << "," << res_cyl_box.closest_point_a.z << ") pt_b=("
              << res_cyl_box.closest_point_b.x << "," << res_cyl_box.closest_point_b.y << "," << res_cyl_box.closest_point_b.z << ")" << std::endl;
    // Distance should be 3.0 - 0.5 - 0.5 = 2.0
    TEST_ASSERT(approx_equal(res_cyl_box.distance, 2.0f, 1e-2f));
}

void test_gjk_intersection() {
    std::cout << "Testing GJK intersection detection..." << std::endl;

    // Overlapping spheres
    SphereShape s1(1.0f);
    SphereShape s2(1.0f);
    Transform3D t1 = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D t2 = Transform3D::from_translation(Vec3(1.5f, 0.0f, 0.0f));

    GJKResult res1 = GJK::query(s1, t1, s2, t2);
    TEST_ASSERT(res1.intersecting);
    TEST_ASSERT(res1.simplex.count >= 2);
    TEST_ASSERT(GJK::test_intersection(s1, t1, s2, t2));

    // Overlapping Capsule and Box
    CapsuleShape cap(1.0f, 0.5f);
    BoxShape box(Vec3(1.0f, 1.0f, 1.0f));
    Transform3D tc = Transform3D::from_translation(Vec3(0.0f, 1.0f, 0.0f));
    Transform3D tb = Transform3D::from_translation(Vec3(0.5f, 0.5f, 0.0f));

    GJKResult res2 = GJK::query(cap, tc, box, tb);
    TEST_ASSERT(res2.intersecting);
    TEST_ASSERT(GJK::test_intersection(cap, tc, box, tb));

    // Concentric shapes
    Transform3D t_origin = Transform3D::identity();
    TEST_ASSERT(GJK::test_intersection(s1, t_origin, box, t_origin));
}

void test_epa_sphere_vs_sphere() {
    std::cout << "Testing EPA Sphere vs Sphere penetration..." << std::endl;

    SphereShape s1(1.0f);
    SphereShape s2(1.0f);
    Transform3D t1 = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D t2 = Transform3D::from_translation(Vec3(1.6f, 0.0f, 0.0f));

    // Analytical expectation:
    // Distance = 1.6, radius sum = 2.0 => penetration depth = 0.4
    // Normal from Sphere 1 towards Sphere 2: (1.0, 0.0, 0.0)
    // Contact point 1: (1.0, 0.0, 0.0)
    // Contact point 2: (0.6, 0.0, 0.0)
    GJKResult gjk = GJK::query(s1, t1, s2, t2);
    TEST_ASSERT(gjk.intersecting);

    EPAResult epa = EPA::query(s1, t1, s2, t2, gjk.simplex);
    TEST_ASSERT(epa.success);
    TEST_ASSERT(approx_equal(epa.penetration_depth, 0.4f, 1e-2f));
    TEST_ASSERT(approx_equal_vec3(epa.normal, Vec3(1.0f, 0.0f, 0.0f), 0.08f));
    TEST_ASSERT(approx_equal_vec3(epa.point_a, Vec3(1.0f, 0.0f, 0.0f), 0.08f));
    TEST_ASSERT(approx_equal_vec3(epa.point_b, Vec3(0.6f, 0.0f, 0.0f), 0.08f));
}

void test_epa_box_vs_box() {
    std::cout << "Testing EPA Box vs Box penetration..." << std::endl;

    BoxShape b1(Vec3(1.0f, 1.0f, 1.0f));
    BoxShape b2(Vec3(1.0f, 1.0f, 1.0f));
    Transform3D t1 = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D t2 = Transform3D::from_translation(Vec3(0.0f, 1.75f, 0.0f));

    // Overlap along Y: 1.0 + 1.0 - 1.75 = 0.25
    GJKResult gjk = GJK::query(b1, t1, b2, t2);
    TEST_ASSERT(gjk.intersecting);

    EPAResult epa = EPA::query(b1, t1, b2, t2, gjk.simplex);
    TEST_ASSERT(epa.success);
    TEST_ASSERT(approx_equal(epa.penetration_depth, 0.25f, 1e-2f));
    TEST_ASSERT(approx_equal_vec3(epa.normal, Vec3(0.0f, 1.0f, 0.0f), 1e-2f));
    TEST_ASSERT(approx_equal(epa.point_a.y, 1.0f, 1e-2f));
    TEST_ASSERT(approx_equal(epa.point_b.y, 0.75f, 1e-2f));
}

void test_epa_cylinder_vs_box() {
    std::cout << "Testing EPA Cylinder vs Box penetration..." << std::endl;

    CylinderShape cyl(1.0f, 0.5f); // h = 1.0, r = 0.5
    BoxShape box(Vec3(0.5f, 0.5f, 0.5f));
    Transform3D t_cyl = Transform3D::from_translation(Vec3(0.0f, 0.0f, 0.0f));
    Transform3D t_box = Transform3D::from_translation(Vec3(0.85f, 0.0f, 0.0f));

    // Cylinder extends to x = 0.5, box extends left to x = 0.85 - 0.5 = 0.35
    // Overlap = 0.5 - 0.35 = 0.15
    GJKResult gjk = GJK::query(cyl, t_cyl, box, t_box);
    TEST_ASSERT(gjk.intersecting);

    EPAResult epa = EPA::query(cyl, t_cyl, box, t_box, gjk.simplex);
    TEST_ASSERT(epa.success);
    TEST_ASSERT(approx_equal(epa.penetration_depth, 0.15f, 1e-2f));
    TEST_ASSERT(approx_equal_vec3(epa.normal, Vec3(1.0f, 0.0f, 0.0f), 2e-2f));
}

void test_epa_convex_hull_pyramid_vs_sphere() {
    std::cout << "Testing EPA ConvexHull Pyramid vs Sphere..." << std::endl;

    std::vector<Vec3> pyramid = {
        Vec3(-1.0f, 0.0f, -1.0f),
        Vec3( 1.0f, 0.0f, -1.0f),
        Vec3( 1.0f, 0.0f,  1.0f),
        Vec3(-1.0f, 0.0f,  1.0f),
        Vec3( 0.0f,  2.0f,  0.0f) // apex at y = 2
    };
    ConvexHullShape hull(pyramid);
    SphereShape sphere(0.5f);

    Transform3D t_hull = Transform3D::identity();
    Transform3D t_sph = Transform3D::from_translation(Vec3(0.0f, 2.3f, 0.0f));

    // Sphere bottom is at y = 2.3 - 0.5 = 1.8. Apex is at y = 2.0. Overlap = 0.2.
    GJKResult gjk = GJK::query(hull, t_hull, sphere, t_sph);
    TEST_ASSERT(gjk.intersecting);

    EPAResult epa = EPA::query(hull, t_hull, sphere, t_sph, gjk.simplex);
    TEST_ASSERT(epa.success);
    TEST_ASSERT(approx_equal(epa.penetration_depth, 0.2f, 1e-2f));
    TEST_ASSERT(approx_equal_vec3(epa.normal, Vec3(0.0f, 1.0f, 0.0f), 0.05f));
}

void test_narrow_phase_general_convex_fallback() {
    std::cout << "Testing NarrowPhase general convex integration..." << std::endl;

    // Body A: Capsule
    CapsuleShape capsule(1.0f, 0.5f);
    Body body_a;
    BodyCreationSettings cs_a(&capsule, Vec3(0.0f, 0.0f, 0.0f), Quat::identity(), MotionType::Dynamic);
    body_a.init(cs_a);
    body_a.set_id(BodyID(1, 0));

    // Body B: Box (Capsule vs Box is handled via GJK/EPA fallback)
    BoxShape box(Vec3(0.5f, 0.5f, 0.5f));
    Body body_b;
    BodyCreationSettings cs_b(&box, Vec3(0.85f, 0.0f, 0.0f), Quat::identity(), MotionType::Static);
    body_b.init(cs_b);
    body_b.set_id(BodyID(2, 0));

    ContactManifold manifold;
    bool collided = NarrowPhase::collide_bodies(body_a, body_b, manifold);
    TEST_ASSERT(collided);
    TEST_ASSERT(manifold.num_points == 1);
    TEST_ASSERT(manifold.body_a == BodyID(1, 0));
    TEST_ASSERT(manifold.body_b == BodyID(2, 0));
    TEST_ASSERT(approx_equal_vec3(manifold.normal, Vec3(1.0f, 0.0f, 0.0f), 2e-2f));
    TEST_ASSERT(approx_equal(manifold.points[0].penetration_depth, 0.15f, 1e-2f));

    // Now move body B far away -> should not collide
    body_b.set_position(Vec3(5.0f, 0.0f, 0.0f));
    ContactManifold manifold_far;
    bool collided_far = NarrowPhase::collide_bodies(body_a, body_b, manifold_far);
    TEST_ASSERT(!collided_far);
}

void test_scene_simulation_with_gjk_epa() {
    std::cout << "Testing PhysicsScene simulation with GJK/EPA..." << std::endl;

    PhysicsScene scene;

    // Static Box table at y = -0.5 with half-extents (5, 0.5, 5) => top face at y = 0.0
    BoxShape floor_shape(Vec3(5.0f, 0.5f, 5.0f));
    BodyCreationSettings floor_cs;
    floor_cs.shape = &floor_shape;
    floor_cs.position = Vec3(0.0f, -0.5f, 0.0f);
    floor_cs.motion_type = MotionType::Static;
    BodyID floor_id = scene.create_body(floor_cs);
    TEST_ASSERT(floor_id.is_valid());

    // Dynamic Cylinder at y = 2.0, radius 0.5, half-height 0.5 (bottom at y = 1.5)
    CylinderShape cyl_shape(0.5f, 0.5f);
    BodyCreationSettings cyl_cs;
    cyl_cs.shape = &cyl_shape;
    cyl_cs.position = Vec3(0.0f, 2.0f, 0.0f);
    cyl_cs.motion_type = MotionType::Dynamic;
    cyl_cs.override_mass = 10.0f;
    BodyID cyl_id = scene.create_body(cyl_cs);
    TEST_ASSERT(cyl_id.is_valid());

    // Step the physics simulation for 1 second (60 ticks @ 60Hz)
    const float dt = 1.0f / 60.0f;
    for (int i = 0; i < 60; ++i) {
        scene.step(dt);
    }

    const Body* cyl_body = scene.get_body(cyl_id);
    TEST_ASSERT(cyl_body != nullptr);

    // The cylinder should have fallen under gravity and rested on top of the floor
    // Floor top is y = 0.0. Cylinder half-height is 0.5, so center should be around y = 0.5!
    std::cout << "Final cylinder Y position: " << cyl_body->get_position().y << std::endl;
    TEST_ASSERT(approx_equal(cyl_body->get_position().y, 0.5f, 0.05f));
    TEST_ASSERT(cyl_body->get_linear_velocity().length() < 0.2f);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Nisaba Phase 6: GJK & EPA Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_cylinder_shape();
    test_convex_hull_shape();
    test_gjk_disjoint_shapes();
    test_gjk_intersection();
    test_epa_sphere_vs_sphere();
    test_epa_box_vs_box();
    test_epa_cylinder_vs_box();
    test_epa_convex_hull_pyramid_vs_sphere();
    test_narrow_phase_general_convex_fallback();
    test_scene_simulation_with_gjk_epa();

    std::cout << "========================================" << std::endl;
    std::cout << "Phase 6 GJK/EPA test passed successfully with " << g_assert_count << " assertions!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

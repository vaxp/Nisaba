#include "orion/orion.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>
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

static constexpr float TEST_EPSILON = 1e-4f;

static bool approx(float a, float b, float eps = TEST_EPSILON) {
    return std::abs(a - b) <= eps;
}

void test_frustum_gribb_hartmann() {
    std::cout << "[*] Testing Frustum Gribb-Hartmann plane extraction..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    cam.set_perspective(60.0f * (3.14159265358979323846f / 180.0f), 1.0f, 1.0f, 100.0f, true);

    const Frustum& f = cam.get_frustum();

    // Verify all 6 plane normals are unit vectors
    for (size_t i = 0; i < 6; ++i) {
        const Plane& p = f.planes[i];
        const float len = p.normal.length();
        TEST_ASSERT(approx(len, 1.0f), "Frustum plane normal must be normalized");
    }

    // Near plane should point towards -Z (camera looking from +Z to 0)
    const Plane& near_p = f.get_plane(FrustumPlaneIndex::Near);
    TEST_ASSERT(near_p.normal.z < -0.9f, "Near plane normal must face into frustum (-Z direction)");

    // Far plane should point towards +Z
    const Plane& far_p = f.get_plane(FrustumPlaneIndex::Far);
    TEST_ASSERT(far_p.normal.z > 0.9f, "Far plane normal must face into frustum (+Z direction)");
}

void test_frustum_point_containment() {
    std::cout << "[*] Testing Frustum point containment..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    cam.set_perspective(60.0f * (3.14159265358979323846f / 180.0f), 1.0f, 1.0f, 100.0f, true);

    const Frustum& f = cam.get_frustum();

    // Points inside frustum (between Z=9 and Z=-90, around X=0, Y=0)
    TEST_ASSERT(f.contains_point(Vec3(0.0f, 0.0f, 5.0f)), "Center point at Z=5 must be inside frustum");
    TEST_ASSERT(f.contains_point(Vec3(0.0f, 0.0f, 0.0f)), "Look-at target at Z=0 must be inside frustum");
    TEST_ASSERT(f.contains_point(Vec3(0.0f, 0.0f, -50.0f)), "Mid-depth point at Z=-50 must be inside frustum");

    // Point behind camera (Z > 10)
    TEST_ASSERT(!f.contains_point(Vec3(0.0f, 0.0f, 15.0f)), "Point behind camera must be culled");

    // Point in front of camera but closer than near plane (Z=9.5 is distance 0.5 from camera, near clip is 1.0)
    TEST_ASSERT(!f.contains_point(Vec3(0.0f, 0.0f, 9.5f)), "Point closer than near clip must be culled");

    // Point beyond far plane (cam at Z=10, far clip=100 -> cutoff is Z=-90)
    TEST_ASSERT(!f.contains_point(Vec3(0.0f, 0.0f, -100.0f)), "Point beyond far clip must be culled");

    // Points way off to left, right, top, bottom
    TEST_ASSERT(!f.contains_point(Vec3(100.0f, 0.0f, 5.0f)), "Point far to right must be culled");
    TEST_ASSERT(!f.contains_point(Vec3(-100.0f, 0.0f, 5.0f)), "Point far to left must be culled");
    TEST_ASSERT(!f.contains_point(Vec3(0.0f, 100.0f, 5.0f)), "Point far above must be culled");
    TEST_ASSERT(!f.contains_point(Vec3(0.0f, -100.0f, 5.0f)), "Point far below must be culled");
}

void test_frustum_sphere_intersection() {
    std::cout << "[*] Testing Frustum sphere culling..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    cam.set_perspective(60.0f * (3.14159265358979323846f / 180.0f), 1.0f, 1.0f, 100.0f, true);

    const Frustum& f = cam.get_frustum();

    // Sphere entirely inside
    TEST_ASSERT(f.intersects_sphere(Vec3(0.0f, 0.0f, 5.0f), 1.0f), "Sphere centered at (0,0,5) must intersect");

    // Sphere straddling near plane (center at Z=9.5, radius 1.0, near plane at Z=9.0)
    TEST_ASSERT(f.intersects_sphere(Vec3(0.0f, 0.0f, 9.5f), 1.0f), "Sphere straddling near plane must intersect");

    // Sphere straddling left border
    TEST_ASSERT(f.intersects_sphere(Vec3(-4.0f, 0.0f, 5.0f), 3.0f), "Sphere overlapping side plane must intersect");

    // Sphere entirely outside behind camera
    TEST_ASSERT(!f.intersects_sphere(Vec3(0.0f, 0.0f, 20.0f), 2.0f), "Sphere behind camera must be culled");

    // Sphere entirely outside beyond far plane
    TEST_ASSERT(!f.intersects_sphere(Vec3(0.0f, 0.0f, -150.0f), 5.0f), "Sphere beyond far plane must be culled");
}

void test_frustum_aabb_intersection() {
    std::cout << "[*] Testing Frustum AABB culling (P-vertex / N-vertex)..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    cam.set_perspective(60.0f * (3.14159265358979323846f / 180.0f), 1.0f, 1.0f, 100.0f, true);

    const Frustum& f = cam.get_frustum();

    // AABB inside
    AABB box_inside(Vec3(-1.0f, -1.0f, 4.0f), Vec3(1.0f, 1.0f, 6.0f));
    TEST_ASSERT(f.intersects_aabb(box_inside), "AABB fully inside must intersect");

    // AABB crossing near plane
    AABB box_crossing_near(Vec3(-1.0f, -1.0f, 8.5f), Vec3(1.0f, 1.0f, 9.5f));
    TEST_ASSERT(f.intersects_aabb(box_crossing_near), "AABB crossing near plane must intersect");

    // AABB completely behind camera
    AABB box_behind(Vec3(-1.0f, -1.0f, 12.0f), Vec3(1.0f, 1.0f, 15.0f));
    TEST_ASSERT(!f.intersects_aabb(box_behind), "AABB behind camera must be culled");

    // AABB completely beyond far plane
    AABB box_far(Vec3(-1.0f, -1.0f, -150.0f), Vec3(1.0f, 1.0f, -120.0f));
    TEST_ASSERT(!f.intersects_aabb(box_far), "AABB beyond far plane must be culled");

    // AABB completely to the left
    AABB box_left(Vec3(-50.0f, -1.0f, 0.0f), Vec3(-30.0f, 1.0f, 2.0f));
    TEST_ASSERT(!f.intersects_aabb(box_left), "AABB to left must be culled");

    // Giant AABB enclosing the entire view frustum
    AABB box_giant(Vec3(-200.0f, -200.0f, -200.0f), Vec3(200.0f, 200.0f, 200.0f));
    TEST_ASSERT(f.intersects_aabb(box_giant), "Giant AABB enclosing frustum must intersect");
}

void test_camera_look_at_and_vectors() {
    std::cout << "[*] Testing Camera3D look_at and orientation vectors..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 2.0f, 10.0f), Vec3(0.0f, 2.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

    Vec3 f = cam.get_forward();
    Vec3 r = cam.get_right();
    Vec3 u = cam.get_camera_up();

    TEST_ASSERT(approx(f.x, 0.0f) && approx(f.y, 0.0f) && approx(f.z, -1.0f), "Forward vector should point down -Z");
    TEST_ASSERT(approx(r.x, 1.0f) && approx(r.y, 0.0f) && approx(r.z, 0.0f), "Right vector should point along +X");
    TEST_ASSERT(approx(u.x, 0.0f) && approx(u.y, 1.0f) && approx(u.z, 0.0f), "Camera up vector should point along +Y");

    // Check View matrix transform
    const Mat44& v = cam.get_view_matrix();
    // Eye position should map to origin in view space
    Vec3 eye_view = v.transform_point(Vec3(0.0f, 2.0f, 10.0f));
    TEST_ASSERT(approx(eye_view.x, 0.0f) && approx(eye_view.y, 0.0f) && approx(eye_view.z, 0.0f), "Eye maps to origin in view space");

    // Target position should map to (0, 0, -10) in view space
    Vec3 target_view = v.transform_point(Vec3(0.0f, 2.0f, 0.0f));
    TEST_ASSERT(approx(target_view.x, 0.0f) && approx(target_view.y, 0.0f) && approx(target_view.z, -10.0f), "Target maps to (0, 0, -10)");
}

void test_camera_perspective_vulkan() {
    std::cout << "[*] Testing Camera3D Vulkan perspective depth mapping..." << std::endl;

    Camera3D cam;
    const float fov = 60.0f * (3.14159265358979323846f / 180.0f);
    cam.set_perspective(fov, 16.0f / 9.0f, 0.1f, 100.0f, true);
    cam.set_look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));

    const Mat44& vp = cam.get_view_projection_matrix();

    // Point on near plane (0, 0, -0.1) should map to NDC Z = 0.0
    Vec3 near_pt = vp.transform_point(Vec3(0.0f, 0.0f, -0.1f));
    TEST_ASSERT(approx(near_pt.z, 0.0f, 1e-3f), "Near plane should map to Vulkan NDC Z = 0.0");

    // Point on far plane (0, 0, -100.0) should map to NDC Z = 1.0
    Vec3 far_pt = vp.transform_point(Vec3(0.0f, 0.0f, -100.0f));
    TEST_ASSERT(approx(far_pt.z, 1.0f, 1e-3f), "Far plane should map to Vulkan NDC Z = 1.0");

    // Point midway (0, 0, -10.0) should map to 0 < Z < 1
    Vec3 mid_pt = vp.transform_point(Vec3(0.0f, 0.0f, -10.0f));
    TEST_ASSERT(mid_pt.z > 0.0f && mid_pt.z < 1.0f, "Mid point NDC Z must be strictly in (0, 1)");
}

void test_camera_orthographic() {
    std::cout << "[*] Testing Camera3D orthographic projection..." << std::endl;

    Camera3D cam;
    cam.set_orthographic(-10.0f, 10.0f, -5.0f, 5.0f, 1.0f, 50.0f, true);
    cam.set_look_at(Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, 1.0f, 0.0f));

    const Mat44& vp = cam.get_view_projection_matrix();

    // Point at top-right on near plane (10, 5, -1) -> NDC (1, 1, 0)
    Vec3 tr_near = vp.transform_point(Vec3(10.0f, 5.0f, -1.0f));
    TEST_ASSERT(approx(tr_near.x, 1.0f) && approx(tr_near.y, 1.0f) && approx(tr_near.z, 0.0f), "Top-right near should map to (1, 1, 0)");

    // Point at bottom-left on far plane (-10, -5, -50) -> NDC (-1, -1, 1)
    Vec3 bl_far = vp.transform_point(Vec3(-10.0f, -5.0f, -50.0f));
    TEST_ASSERT(approx(bl_far.x, -1.0f) && approx(bl_far.y, -1.0f) && approx(bl_far.z, 1.0f), "Bottom-left far should map to (-1, -1, 1)");
}

void test_camera_picking_ray() {
    std::cout << "[*] Testing Camera3D screen unprojection and picking ray..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 10.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
    cam.set_perspective(60.0f * (3.14159265358979323846f / 180.0f), 1.0f, 0.1f, 100.0f, true);

    // Screen center on 800x800 viewport
    Ray center_ray = cam.screen_point_to_ray(400.0f, 400.0f, 800.0f, 800.0f);
    TEST_ASSERT(approx(center_ray.origin.z, 10.0f), "Center ray origin Z matches camera position");
    TEST_ASSERT(approx(center_ray.direction.x, 0.0f) && approx(center_ray.direction.y, 0.0f) && approx(center_ray.direction.z, -1.0f), "Center ray direction points directly at -Z");

    // Right edge (screen_x = 800, screen_y = 400) -> ray direction must have positive X
    Ray right_ray = cam.screen_point_to_ray(800.0f, 400.0f, 800.0f, 800.0f);
    TEST_ASSERT(right_ray.direction.x > 0.1f, "Right-edge ray must have positive X direction");
    TEST_ASSERT(approx(right_ray.direction.y, 0.0f), "Right-edge ray should have near-zero Y direction");

    // Top edge (screen_x = 400, screen_y = 0) -> ray direction must have positive Y
    Ray top_ray = cam.screen_point_to_ray(400.0f, 0.0f, 800.0f, 800.0f);
    TEST_ASSERT(top_ray.direction.y > 0.1f, "Top-edge ray must have positive Y direction");
    TEST_ASSERT(approx(top_ray.direction.x, 0.0f), "Top-edge ray should have near-zero X direction");
}

void test_camera_caching_and_invalidation() {
    std::cout << "[*] Testing Camera3D matrix lazy caching and dirty flags..." << std::endl;

    Camera3D cam;
    cam.set_look_at(Vec3(0.0f, 0.0f, 5.0f), Vec3(0.0f, 0.0f, 0.0f));

    const Mat44 v1_copy = cam.get_view_matrix();
    const Mat44& v1_ref = cam.get_view_matrix();
    const Mat44& v2_ref = cam.get_view_matrix();
    // Pointers to internal buffer should be identical
    TEST_ASSERT(&v1_ref == &v2_ref, "Repeated get_view_matrix calls must return identical cached reference");

    // Change position
    cam.set_position(Vec3(0.0f, 0.0f, 15.0f));
    const Mat44& v3 = cam.get_view_matrix();
    TEST_ASSERT(v3(2, 3) != v1_copy(2, 3), "Changing position must invalidate view matrix cache");
}

void test_orbit_controller() {
    std::cout << "[*] Testing OrbitCameraController..." << std::endl;

    OrbitCameraController orbit(Vec3(0.0f, 0.0f, 0.0f), 10.0f, 0.0f, 0.0f);

    Vec3 eye = orbit.compute_eye_position();
    TEST_ASSERT(approx(eye.x, 0.0f) && approx(eye.y, 0.0f) && approx(eye.z, 10.0f), "Initial eye at (0, 0, 10)");

    // Orbit 90 degrees yaw (pi/2)
    const float pi_half = 3.14159265358979323846f * 0.5f;
    orbit.orbit(pi_half, 0.0f);
    eye = orbit.compute_eye_position();
    TEST_ASSERT(approx(eye.x, 10.0f) && approx(eye.y, 0.0f) && approx(eye.z, 0.0f), "90 deg yaw moves eye to (+10, 0, 0)");

    // Zoom in
    orbit.zoom(-3.0f);
    TEST_ASSERT(approx(orbit.distance, 7.0f), "Zoom -3 reduces distance to 7");

    // Panning
    Camera3D cam;
    orbit.update(cam);
    orbit.pan(2.0f, 1.0f, cam);
    TEST_ASSERT(orbit.target.length() > 0.1f, "Panning must translate target position");

    // Pitch clamping (cannot flip upside down)
    orbit.orbit(0.0f, 10.0f); // large upward pitch
    TEST_ASSERT(orbit.pitch <= orbit.max_pitch, "Pitch must be clamped to safe maximum");
}

int main() {
    std::cout << "========================================================\n"
              << "   NISABA 3D RENDERING PIPELINE: CAMERA & FRUSTUM TEST  \n"
              << "========================================================" << std::endl;

    test_frustum_gribb_hartmann();
    test_frustum_point_containment();
    test_frustum_sphere_intersection();
    test_frustum_aabb_intersection();
    test_camera_look_at_and_vectors();
    test_camera_perspective_vulkan();
    test_camera_orthographic();
    test_camera_picking_ray();
    test_camera_caching_and_invalidation();
    test_orbit_controller();

    std::cout << "\n[+] ALL TESTS COMPLETED: " << g_tests_passed << "/" << g_tests_total << " assertions passed." << std::endl;
    return (g_tests_passed == g_tests_total) ? 0 : 1;
}

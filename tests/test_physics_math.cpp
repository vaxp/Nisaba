#include "orion/math/math.hpp"
#include <cassert>
#include <iostream>
#include <iomanip>

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

void test_vec3() {
    std::cout << "[*] Testing Vec3 (16-byte SIMD)..." << std::endl;

    TEST_ASSERT(sizeof(Vec3) == 16, "Vec3 size must be exactly 16 bytes");
    TEST_ASSERT(alignof(Vec3) == 16, "Vec3 must be 16-byte aligned");

    Vec3 a(1.0f, 2.0f, 3.0f);
    Vec3 b(4.0f, 5.0f, 6.0f);

    // Addition
    Vec3 add = a + b;
    TEST_ASSERT(approx_equal(add.x, 5.0f) && approx_equal(add.y, 7.0f) && approx_equal(add.z, 9.0f), "Vec3 addition");

    // Subtraction
    Vec3 sub = b - a;
    TEST_ASSERT(approx_equal(sub.x, 3.0f) && approx_equal(sub.y, 3.0f) && approx_equal(sub.z, 3.0f), "Vec3 subtraction");

    // Scalar mul & div
    Vec3 scaled = a * 2.0f;
    TEST_ASSERT(approx_equal(scaled.x, 2.0f) && approx_equal(scaled.y, 4.0f) && approx_equal(scaled.z, 6.0f), "Vec3 scalar mul");

    Vec3 div = scaled / 2.0f;
    TEST_ASSERT(div == a, "Vec3 scalar div");

    // Negation
    Vec3 neg = -a;
    TEST_ASSERT(approx_equal(neg.x, -1.0f) && approx_equal(neg.y, -2.0f) && approx_equal(neg.z, -3.0f), "Vec3 negation");

    // Dot product
    float dot_val = a.dot(b); // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
    TEST_ASSERT(approx_equal(dot_val, 32.0f), "Vec3 dot product");

    // Orthogonal dot product = 0
    TEST_ASSERT(approx_equal(Vec3::unit_x().dot(Vec3::unit_y()), 0.0f), "Vec3 orthogonal dot");

    // Cross product
    Vec3 cross_xy = Vec3::unit_x().cross(Vec3::unit_y());
    TEST_ASSERT(cross_xy == Vec3::unit_z(), "Cross product X x Y = Z");

    Vec3 cross_yz = Vec3::unit_y().cross(Vec3::unit_z());
    TEST_ASSERT(cross_yz == Vec3::unit_x(), "Cross product Y x Z = X");

    Vec3 cross_zx = Vec3::unit_z().cross(Vec3::unit_x());
    TEST_ASSERT(cross_zx == Vec3::unit_y(), "Cross product Z x X = Y");

    // Anti-symmetry: A x B == -(B x A)
    Vec3 cross_ab = a.cross(b);
    Vec3 cross_ba = b.cross(a);
    TEST_ASSERT(cross_ab == -cross_ba, "Cross product anti-symmetry");

    // Length & normalization
    Vec3 v340(3.0f, 4.0f, 0.0f);
    TEST_ASSERT(approx_equal(v340.length_sq(), 25.0f), "Vec3 length_sq");
    TEST_ASSERT(approx_equal(v340.length(), 5.0f), "Vec3 length");

    Vec3 norm = v340.normalized();
    TEST_ASSERT(norm.is_normalized(), "Vec3 normalized status");
    TEST_ASSERT(approx_equal(norm.x, 0.6f) && approx_equal(norm.y, 0.8f) && approx_equal(norm.z, 0.0f), "Vec3 normalized values");

    // Min / Max / Abs
    Vec3 neg_v(-5.0f, 3.0f, -10.0f);
    Vec3 abs_v = neg_v.abs();
    TEST_ASSERT(approx_equal(abs_v.x, 5.0f) && approx_equal(abs_v.y, 3.0f) && approx_equal(abs_v.z, 10.0f), "Vec3 abs");

    Vec3 v_min = a.min(b);
    TEST_ASSERT(v_min == a, "Vec3 min");

    Vec3 v_max = a.max(b);
    TEST_ASSERT(v_max == b, "Vec3 max");

    // Reflection: ball bouncing on horizontal floor (normal = (0, 1, 0))
    Vec3 incoming(1.0f, -1.0f, 0.0f);
    Vec3 normal(0.0f, 1.0f, 0.0f);
    Vec3 reflected = incoming.reflect(normal);
    TEST_ASSERT(approx_equal(reflected.x, 1.0f) && approx_equal(reflected.y, 1.0f) && approx_equal(reflected.z, 0.0f), "Vec3 reflection");

    // Distance
    TEST_ASSERT(approx_equal(Vec3::distance(Vec3(0, 0, 0), Vec3(0, 10, 0)), 10.0f), "Vec3 distance");

    // Duff et al. Orthonormal basis
    Vec3 normal_up = Vec3::unit_y();
    Vec3 t1, t2;
    normal_up.get_orthonormal_basis(t1, t2);
    TEST_ASSERT(approx_equal(t1.length(), 1.0f), "Tangent 1 is unit length");
    TEST_ASSERT(approx_equal(t2.length(), 1.0f), "Tangent 2 is unit length");
    TEST_ASSERT(approx_equal(normal_up.dot(t1), 0.0f), "Normal and t1 are orthogonal");
    TEST_ASSERT(approx_equal(normal_up.dot(t2), 0.0f), "Normal and t2 are orthogonal");
    TEST_ASSERT(approx_equal(t1.dot(t2), 0.0f), "t1 and t2 are orthogonal");

    std::cout << "  [+] Vec3 passed." << std::endl;
}

void test_quat() {
    std::cout << "[*] Testing Quat (Quaternions & Rotations)..." << std::endl;

    TEST_ASSERT(sizeof(Quat) == 16, "Quat size must be exactly 16 bytes");
    TEST_ASSERT(alignof(Quat) == 16, "Quat must be 16-byte aligned");

    Quat id = Quat::identity();
    TEST_ASSERT(approx_equal(id.x, 0.0f) && approx_equal(id.y, 0.0f) && approx_equal(id.z, 0.0f) && approx_equal(id.w, 1.0f), "Quat identity");

    // Rotate (1, 0, 0) by 90 degrees around Z axis -> (0, 1, 0)
    Quat q_rot_z90 = Quat::from_axis_angle(Vec3::unit_z(), HALF_PI);
    Vec3 rotated_x = q_rot_z90.rotate_vec(Vec3::unit_x());
    TEST_ASSERT(approx_equal(rotated_x.x, 0.0f) && approx_equal(rotated_x.y, 1.0f) && approx_equal(rotated_x.z, 0.0f), "Rotate X around Z by 90 deg");

    // Rotate (0, 1, 0) by 90 degrees around X axis -> (0, 0, 1)
    Quat q_rot_x90 = Quat::from_axis_angle(Vec3::unit_x(), HALF_PI);
    Vec3 rotated_y = q_rot_x90.rotate_vec(Vec3::unit_y());
    TEST_ASSERT(approx_equal(rotated_y.x, 0.0f) && approx_equal(rotated_y.y, 0.0f) && approx_equal(rotated_y.z, 1.0f), "Rotate Y around X by 90 deg");

    // Rotate (0, 0, 1) by 90 degrees around Y axis -> (1, 0, 0)
    Quat q_rot_y90 = Quat::from_axis_angle(Vec3::unit_y(), HALF_PI);
    Vec3 rotated_z = q_rot_y90.rotate_vec(Vec3::unit_z());
    TEST_ASSERT(approx_equal(rotated_z.x, 1.0f) && approx_equal(rotated_z.y, 0.0f) && approx_equal(rotated_z.z, 0.0f), "Rotate Z around Y by 90 deg");

    // Composition: rotate by 90 deg around Z, then by 90 deg around Z again = 180 deg around Z
    Quat q_rot_z180 = q_rot_z90 * q_rot_z90;
    Vec3 rotated_x180 = q_rot_z180.rotate_vec(Vec3::unit_x());
    TEST_ASSERT(approx_equal(rotated_x180.x, -1.0f) && approx_equal(rotated_x180.y, 0.0f), "Composition 90 + 90 = 180 deg");

    // Inverse and conjugate properties: q * q^-1 == identity
    Quat q_inv = q_rot_z90.inverse();
    Quat q_res = q_rot_z90 * q_inv;
    TEST_ASSERT(approx_equal(q_res.w, 1.0f) && approx_equal(q_res.x, 0.0f), "q * q^-1 == I");

    // Rotating back by inverse returns original vector
    Vec3 back_x = q_inv.rotate_vec(rotated_x);
    TEST_ASSERT(back_x == Vec3::unit_x(), "Rotate back with inverse");

    // SLERP interpolation: 0 deg to 90 deg at t=0.5 -> 45 deg
    Quat q_45 = Quat::slerp(id, q_rot_z90, 0.5f);
    Vec3 rot_45 = q_45.rotate_vec(Vec3::unit_x());
    const float expected_45 = std::sqrt(0.5f);
    TEST_ASSERT(approx_equal(rot_45.x, expected_45) && approx_equal(rot_45.y, expected_45), "SLERP 45 deg");

    // Angular velocity integration
    Vec3 omega(0.0f, 0.0f, PI); // 180 deg/s around Z
    float dt = 0.5f; // should rotate by 90 deg
    Quat integrated_q = id.integrate(omega, dt);
    Vec3 int_rot = integrated_q.rotate_vec(Vec3::unit_x());
    TEST_ASSERT(approx_equal(int_rot.x, 0.0f) && approx_equal(int_rot.y, 1.0f), "Angular velocity integration");

    std::cout << "  [+] Quat passed." << std::endl;
}

void test_mat44() {
    std::cout << "[*] Testing Mat44 (4x4 Transform Matrices)..." << std::endl;

    TEST_ASSERT(sizeof(Mat44) == 64, "Mat44 size must be exactly 64 bytes");
    TEST_ASSERT(alignof(Mat44) == 16, "Mat44 must be 16-byte aligned");

    Mat44 id = Mat44::identity();
    Vec3 p(10.0f, 20.0f, 30.0f);
    TEST_ASSERT(id.transform_point(p) == p, "Identity transforms point to itself");
    TEST_ASSERT(id.transform_vector(p) == p, "Identity transforms vector to itself");

    // Translation matrix
    Mat44 trans = Mat44::translation(Vec3(5.0f, -3.0f, 2.0f));
    Vec3 p_trans = trans.transform_point(p);
    TEST_ASSERT(approx_equal(p_trans.x, 15.0f) && approx_equal(p_trans.y, 17.0f) && approx_equal(p_trans.z, 32.0f), "Point translation");

    // Transform vector ignores translation
    Vec3 v_trans = trans.transform_vector(p);
    TEST_ASSERT(v_trans == p, "Vector transform ignores translation");

    // Scaling matrix
    Mat44 scale = Mat44::scaling(Vec3(2.0f, 0.5f, -1.0f));
    Vec3 p_scale = scale.transform_point(p);
    TEST_ASSERT(approx_equal(p_scale.x, 20.0f) && approx_equal(p_scale.y, 10.0f) && approx_equal(p_scale.z, -30.0f), "Point scaling");

    // Rotation from Quaternion
    Quat q_rot = Quat::from_axis_angle(Vec3::unit_z(), HALF_PI);
    Mat44 rot = Mat44::rotation(q_rot);
    Vec3 p_rot = rot.transform_point(Vec3::unit_x());
    TEST_ASSERT(approx_equal(p_rot.x, 0.0f) && approx_equal(p_rot.y, 1.0f) && approx_equal(p_rot.z, 0.0f), "Mat44 rotation from Quat");

    // Determinant of identity is 1.0
    TEST_ASSERT(approx_equal(id.determinant(), 1.0f), "Identity determinant == 1");

    // Matrix Multiplication & Inversion
    Mat44 compound = trans * rot;
    bool inv_success = false;
    Mat44 inv_compound = compound.inverse(inv_success);
    TEST_ASSERT(inv_success, "General inverse success");

    Mat44 prod = compound * inv_compound;
    TEST_ASSERT(approx_equal(prod(0, 0), 1.0f) && approx_equal(prod(1, 1), 1.0f) && approx_equal(prod(2, 2), 1.0f) && approx_equal(prod(3, 3), 1.0f), "M * M^-1 == I diagonal");
    TEST_ASSERT(approx_equal(prod(0, 1), 0.0f) && approx_equal(prod(0, 3), 0.0f), "M * M^-1 == I off-diagonal");

    // Fast Rigid Inverse
    Mat44 rigid_inv = compound.inverse_fast_rigid();
    Mat44 rigid_prod = compound * rigid_inv;
    TEST_ASSERT(approx_equal(rigid_prod(0, 0), 1.0f) && approx_equal(rigid_prod(1, 1), 1.0f) && approx_equal(rigid_prod(2, 2), 1.0f), "Fast rigid inverse M * M^-1 == I");
    TEST_ASSERT(approx_equal(rigid_prod(0, 3), 0.0f) && approx_equal(rigid_prod(1, 3), 0.0f) && approx_equal(rigid_prod(2, 3), 0.0f), "Fast rigid inverse translation == 0");

    std::cout << "  [+] Mat44 passed." << std::endl;
}

void test_aabb() {
    std::cout << "[*] Testing AABB (Bounding Volumes & Slab Raycast)..." << std::endl;

    TEST_ASSERT(sizeof(AABB) == 32, "AABB size must be exactly 32 bytes");
    TEST_ASSERT(alignof(AABB) == 16, "AABB must be 16-byte aligned");

    AABB box(Vec3(-2.0f, -3.0f, -4.0f), Vec3(2.0f, 3.0f, 4.0f));
    TEST_ASSERT(box.is_valid(), "AABB validity");
    TEST_ASSERT(box.center() == Vec3::zero(), "AABB center");
    TEST_ASSERT(box.extent() == Vec3(4.0f, 6.0f, 8.0f), "AABB extent");
    TEST_ASSERT(box.half_extent() == Vec3(2.0f, 3.0f, 4.0f), "AABB half_extent");
    TEST_ASSERT(approx_equal(box.volume(), 4.0f * 6.0f * 8.0f), "AABB volume == 192");

    // Point containment
    TEST_ASSERT(box.contains(Vec3(0.0f, 0.0f, 0.0f)), "AABB contains origin");
    TEST_ASSERT(box.contains(Vec3(2.0f, 3.0f, 4.0f)), "AABB contains boundary point");
    TEST_ASSERT(!box.contains(Vec3(2.1f, 0.0f, 0.0f)), "AABB rejects outside point");

    // Overlaps
    AABB box2(Vec3(1.0f, 1.0f, 1.0f), Vec3(5.0f, 5.0f, 5.0f));
    TEST_ASSERT(box.overlaps(box2), "Overlapping AABBs");

    AABB disjoint_box(Vec3(10.0f, 10.0f, 10.0f), Vec3(12.0f, 12.0f, 12.0f));
    TEST_ASSERT(!box.overlaps(disjoint_box), "Disjoint AABBs do not overlap");

    // Encapsulation
    AABB dynamic_box;
    TEST_ASSERT(!dynamic_box.is_valid(), "Default AABB is inverted / empty");
    dynamic_box.encapsulate(Vec3(1.0f, 2.0f, 3.0f));
    dynamic_box.encapsulate(Vec3(-1.0f, -2.0f, -3.0f));
    TEST_ASSERT(dynamic_box.min_point == Vec3(-1.0f, -2.0f, -3.0f), "Encapsulate sets min");
    TEST_ASSERT(dynamic_box.max_point == Vec3(1.0f, 2.0f, 3.0f), "Encapsulate sets max");

    // Transform AABB by rotation & translation (Jim Arvo's algorithm)
    Quat rot_z90 = Quat::from_axis_angle(Vec3::unit_z(), HALF_PI);
    Mat44 trans = Mat44::transform(Vec3(10.0f, 0.0f, 0.0f), rot_z90);
    AABB transformed_box = box.transform(trans);
    TEST_ASSERT(transformed_box.is_valid(), "Transformed AABB is valid");
    TEST_ASSERT(approx_equal(transformed_box.center().x, 10.0f), "Transformed AABB center translated");
    // Since width was 4 (x) and height was 6 (y), rotating 90 deg swaps them: new width=6, new height=4
    TEST_ASSERT(approx_equal(transformed_box.extent().x, 6.0f), "Transformed AABB extent X swapped with Y");
    TEST_ASSERT(approx_equal(transformed_box.extent().y, 4.0f), "Transformed AABB extent Y swapped with X");

    // Raycast (Slab Method)
    // Ray pointing straight at the center of box from -10 on Z axis
    Vec3 ray_origin(0.0f, 0.0f, -10.0f);
    Vec3 ray_dir(0.0f, 0.0f, 1.0f);
    float t_min = 0.0f, t_max = 0.0f;
    bool hit = box.raycast(ray_origin, ray_dir, 100.0f, t_min, t_max);
    TEST_ASSERT(hit, "Ray hits AABB");
    // Box bounds on Z are [-4, 4], ray starts at -10, so hit occurs at t = 6 (-10 + 6 = -4) and exits at t = 14
    TEST_ASSERT(approx_equal(t_min, 6.0f), "Ray entrance t_min == 6.0");
    TEST_ASSERT(approx_equal(t_max, 14.0f), "Ray exit t_max == 14.0");

    // Ray pointing away from box
    Vec3 miss_dir(0.0f, 0.0f, -1.0f);
    bool miss = box.raycast(ray_origin, miss_dir, 100.0f, t_min, t_max);
    TEST_ASSERT(!miss, "Ray pointing away misses AABB");

    // Ray starting inside box
    Vec3 inside_origin(0.0f, 0.0f, 0.0f);
    bool inside_hit = box.raycast(inside_origin, ray_dir, 100.0f, t_min, t_max);
    TEST_ASSERT(inside_hit, "Ray starting inside AABB hits");
    TEST_ASSERT(approx_equal(t_min, 0.0f), "Inside ray t_min clamped to 0.0");
    TEST_ASSERT(approx_equal(t_max, 4.0f), "Inside ray t_max exit == 4.0");

    std::cout << "  [+] AABB passed." << std::endl;
}

void test_ray() {
    std::cout << "[*] Testing Ray..." << std::endl;
    Ray ray(Vec3(0.0f, 10.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
    TEST_ASSERT(ray.origin == Vec3(0.0f, 10.0f, 0.0f), "Ray origin");
    TEST_ASSERT(ray.direction == Vec3(0.0f, -1.0f, 0.0f), "Ray direction");
    TEST_ASSERT(ray.point_at(10.0f) == Vec3::zero(), "Ray point_at hit origin");
    std::cout << "  [+] Ray passed." << std::endl;
}

void test_plane() {
    std::cout << "[*] Testing Plane..." << std::endl;
    Plane floor(Vec3(0.0f, 1.0f, 0.0f), 0.0f);
    TEST_ASSERT(approx_equal(floor.signed_distance(Vec3(0.0f, 5.0f, 0.0f)), 5.0f), "Point 5m above floor has +5 signed distance");
    TEST_ASSERT(approx_equal(floor.signed_distance(Vec3(0.0f, -3.0f, 0.0f)), -3.0f), "Point 3m below floor has -3 signed distance");
    TEST_ASSERT(approx_equal(floor.signed_distance(Vec3(10.0f, 0.0f, 20.0f)), 0.0f), "Point on floor has 0 signed distance");

    Vec3 projected = floor.project(Vec3(10.0f, 5.0f, 20.0f));
    TEST_ASSERT(projected == Vec3(10.0f, 0.0f, 20.0f), "Project point onto floor");

    Plane p3 = Plane::from_three_points(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 0, 1));
    TEST_ASSERT(approx_equal(std::abs(p3.normal.y), 1.0f), "Plane normal from 3 points is vertical");
    std::cout << "  [+] Plane passed." << std::endl;
}

void test_sphere() {
    std::cout << "[*] Testing Sphere..." << std::endl;
    Sphere sphere(Vec3(0.0f, 2.0f, 0.0f), 1.0f);
    TEST_ASSERT(sphere.contains(Vec3(0.0f, 2.0f, 0.0f)), "Sphere contains its center");
    TEST_ASSERT(sphere.contains(Vec3(0.0f, 3.0f, 0.0f)), "Sphere contains surface point");
    TEST_ASSERT(!sphere.contains(Vec3(0.0f, 3.5f, 0.0f)), "Sphere rejects outside point");
    TEST_ASSERT(approx_equal(sphere.signed_distance(Vec3(0.0f, 5.0f, 0.0f)), 2.0f), "Sphere signed distance to point at y=5 is 2.0");

    Sphere other_sphere(Vec3(0.0f, 3.5f, 0.0f), 1.0f);
    TEST_ASSERT(sphere.overlaps(other_sphere), "Overlapping spheres");

    AABB s_aabb = sphere.get_aabb();
    TEST_ASSERT(s_aabb.min_point == Vec3(-1.0f, 1.0f, -1.0f), "Sphere AABB min");
    TEST_ASSERT(s_aabb.max_point == Vec3(1.0f, 3.0f, 1.0f), "Sphere AABB max");
    std::cout << "  [+] Sphere passed." << std::endl;
}

void test_transform3d() {
    std::cout << "[*] Testing Transform3D (SE(3) Rigid Kinematics)..." << std::endl;

    TEST_ASSERT(sizeof(Transform3D) == 32, "Transform3D size must be exactly 32 bytes");
    TEST_ASSERT(alignof(Transform3D) == 16, "Transform3D must be 16-byte aligned");

    Transform3D id = Transform3D::identity();
    Vec3 p(1.0f, 2.0f, 3.0f);
    Vec3 v(0.0f, 1.0f, 0.0f);

    TEST_ASSERT(id.transform_point(p) == p, "Identity Transform3D preserves point");
    TEST_ASSERT(id.transform_vector(v) == v, "Identity Transform3D preserves vector");

    // Pure translation
    Transform3D t_trans = Transform3D::from_translation(Vec3(10.0f, -5.0f, 20.0f));
    Vec3 p_trans = t_trans.transform_point(p);
    TEST_ASSERT(approx_equal(p_trans.x, 11.0f) && approx_equal(p_trans.y, -3.0f) && approx_equal(p_trans.z, 23.0f), "Transform3D point translation");
    TEST_ASSERT(t_trans.transform_vector(v) == v, "Transform3D vector ignores translation");

    // Pure rotation (90 deg around Z)
    Quat q_rot = Quat::from_axis_angle(Vec3::unit_z(), HALF_PI);
    Transform3D t_rot = Transform3D::from_rotation(q_rot);
    Vec3 p_rot = t_rot.transform_point(Vec3::unit_x());
    TEST_ASSERT(approx_equal(p_rot.x, 0.0f) && approx_equal(p_rot.y, 1.0f) && approx_equal(p_rot.z, 0.0f), "Transform3D point rotation");

    // Compound: Rotation + Translation
    Transform3D compound(Vec3(10.0f, 20.0f, 30.0f), q_rot);
    Vec3 p_comp = compound.transform_point(Vec3::unit_x());
    // unit_x rotated 90 deg around Z becomes (0, 1, 0), plus translation (10, 20, 30) = (10, 21, 30)
    TEST_ASSERT(approx_equal(p_comp.x, 10.0f) && approx_equal(p_comp.y, 21.0f) && approx_equal(p_comp.z, 30.0f), "Transform3D compound transform");

    // Inverse transform
    Transform3D comp_inv = compound.inverse();
    Vec3 p_back = comp_inv.transform_point(p_comp);
    TEST_ASSERT(p_back == Vec3::unit_x(), "Transform3D inverse restores original point");

    Vec3 v_rot = compound.transform_vector(Vec3::unit_x());
    Vec3 v_back = comp_inv.transform_vector(v_rot);
    TEST_ASSERT(v_back == Vec3::unit_x(), "Transform3D inverse restores original vector");

    // Composition operator*
    Transform3D t1(Vec3(5.0f, 0.0f, 0.0f), Quat::from_axis_angle(Vec3::unit_y(), HALF_PI));
    Transform3D t2(Vec3(0.0f, 0.0f, -5.0f), Quat::from_axis_angle(Vec3::unit_y(), HALF_PI));
    Transform3D t_composed = t1 * t2;
    Vec3 p_comp_chain = t1.transform_point(t2.transform_point(p));
    Vec3 p_comp_op = t_composed.transform_point(p);
    TEST_ASSERT(p_comp_chain.approx_equal(p_comp_op), "Transform3D operator* matches chained transforms");

    // Conversion to Mat44
    Mat44 m = compound.to_mat44();
    Vec3 p_m = m.transform_point(Vec3::unit_x());
    TEST_ASSERT(p_m.approx_equal(p_comp), "Transform3D to_mat44 matches transform_point");

    // Conversion from Mat44
    Transform3D t_reconstructed = Transform3D::from_mat44(m);
    TEST_ASSERT(t_reconstructed.position.approx_equal(compound.position), "from_mat44 reconstructed position");
    TEST_ASSERT(t_reconstructed.rotation.approx_equal(compound.rotation), "from_mat44 reconstructed rotation");

    // AABB transformed by Transform3D vs Mat44
    AABB box(Vec3(-1.0f, -2.0f, -3.0f), Vec3(1.0f, 2.0f, 3.0f));
    AABB aabb_m = box.transform(m);
    AABB aabb_t = box.transform(compound);
    TEST_ASSERT(aabb_m.min_point.approx_equal(aabb_t.min_point), "AABB transform by Transform3D matches Mat44 min");
    TEST_ASSERT(aabb_m.max_point.approx_equal(aabb_t.max_point), "AABB transform by Transform3D matches Mat44 max");

    std::cout << "  [+] Transform3D passed." << std::endl;
}

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "   Nisaba Sovereign 3D Physics Math Test Suite   " << std::endl;
    std::cout << "=================================================" << std::endl;

    test_vec3();
    test_quat();
    test_mat44();
    test_transform3d();
    test_aabb();
    test_ray();
    test_plane();
    test_sphere();

    std::cout << "=================================================" << std::endl;
    std::cout << "[+] ALL TESTS PASSED: " << g_tests_passed << "/" << g_tests_total << " assertions verified!" << std::endl;
    std::cout << "=================================================" << std::endl;
    return 0;
}


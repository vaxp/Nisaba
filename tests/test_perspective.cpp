#include <iostream>
#include <cmath>
#include <cassert>
#include "nisaba/nisaba.hpp"

using namespace nisaba;

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_CASE(name) void name()
#define RUN_TEST(name) do { \
    std::cout << "[RUN]  " << #name << "..." << std::flush; \
    int prev_failed = g_tests_failed; \
    name(); \
    if (g_tests_failed == prev_failed) { \
        std::cout << " [PASSED]" << std::endl; \
        g_tests_passed++; \
    } else { \
        std::cout << " [FAILED]" << std::endl; \
    } \
} while(0)

#define EXPECT_TRUE(expr) do { \
    if (!(expr)) { \
        std::cerr << "\n  FAILED: " << #expr << " is false at " << __FILE__ << ":" << __LINE__ << std::endl; \
        g_tests_failed++; \
    } \
} while(0)

#define EXPECT_FALSE(expr) EXPECT_TRUE(!(expr))

#define EXPECT_EQ(a, b) do { \
    if (!((a) == (b))) { \
        std::cerr << "\n  FAILED: " << #a << " == " << #b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        g_tests_failed++; \
    } \
} while(0)

#define EXPECT_NEAR(a, b, eps) do { \
    if (std::abs((a) - (b)) > (eps)) { \
        std::cerr << "\n  FAILED: |" << #a << " - " << #b << "| (" << std::abs((a) - (b)) << ") <= " << #eps << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        g_tests_failed++; \
    } \
} while(0)

TEST_CASE(test_identity_and_basic_transforms) {
    auto id = Transform4x4::identity();
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            float expected = (r == c) ? 1.0f : 0.0f;
            EXPECT_NEAR(id.m[r * 4 + c], expected, 1e-6f);
        }
    }

    Point p(10.0f, 20.0f);
    Point p_mapped = id.map_point(p);
    EXPECT_NEAR(p_mapped.x, 10.0f, 1e-5f);
    EXPECT_NEAR(p_mapped.y, 20.0f, 1e-5f);

    auto t = Transform4x4::from_translate(15.0f, -5.0f, 30.0f);
    Point pt = t.map_point(p);
    EXPECT_NEAR(pt.x, 25.0f, 1e-5f);
    EXPECT_NEAR(pt.y, 15.0f, 1e-5f);

    auto s = Transform4x4::from_scale(2.0f, 3.0f, 4.0f);
    Point ps = s.map_point(p);
    EXPECT_NEAR(ps.x, 20.0f, 1e-5f);
    EXPECT_NEAR(ps.y, 60.0f, 1e-5f);
}

TEST_CASE(test_3d_rotations) {
    // 90 degree rotation around Z axis: (1, 0) -> (0, 1)
    auto rz90 = Transform4x4::from_rotate_z(90.0f);
    Point pz = rz90.map_point(Point(1.0f, 0.0f));
    EXPECT_NEAR(pz.x, 0.0f, 1e-5f);
    EXPECT_NEAR(pz.y, 1.0f, 1e-5f);

    // 90 degree rotation around X: (0, 1, 0) -> (0, 0, 1)
    auto rx90 = Transform4x4::from_rotate_x(90.0f);
    Point px = rx90.map_point3d(0.0f, 1.0f, 0.0f);
    EXPECT_NEAR(px.x, 0.0f, 1e-5f);
    EXPECT_NEAR(px.y, 0.0f, 1e-5f); // y becomes 0, z becomes 1

    // 90 degree rotation around Y: (1, 0, 0) -> (0, 0, -1)
    auto ry90 = Transform4x4::from_rotate_y(90.0f);
    Point py = ry90.map_point3d(1.0f, 0.0f, 0.0f);
    EXPECT_NEAR(py.x, 0.0f, 1e-5f);
    EXPECT_NEAR(py.y, 0.0f, 1e-5f);
}

TEST_CASE(test_matrix_multiply_and_invert) {
    auto t = Transform4x4::from_translate(50.0f, 100.0f, 10.0f);
    auto r = Transform4x4::from_rotate_z(45.0f);
    auto s = Transform4x4::from_scale(2.0f, 2.0f, 1.0f);

    auto m = t * r * s;
    auto inv_opt = m.invert();
    EXPECT_TRUE(inv_opt.has_value());

    auto prod = m * (*inv_opt);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float expected = (row == col) ? 1.0f : 0.0f;
            EXPECT_NEAR(prod.m[row * 4 + col], expected, 1e-4f);
        }
    }
}

TEST_CASE(test_perspective_projection) {
    float d = 500.0f;
    auto persp = Transform4x4::from_perspective(d);
    EXPECT_TRUE(persp.has_perspective());

    // Point at z = 0 should have w = 1
    Point p0 = persp.map_point3d(100.0f, 50.0f, 0.0f);
    EXPECT_NEAR(p0.x, 100.0f, 1e-5f);
    EXPECT_NEAR(p0.y, 50.0f, 1e-5f);

    // Point pushed back into screen at z = -500:
    // W = 1 - (-500 / 500) = 2
    // Screen coords should be halved (100 / 2 = 50, 50 / 2 = 25)
    Point p_back = persp.map_point3d(100.0f, 50.0f, -500.0f);
    EXPECT_NEAR(p_back.x, 50.0f, 1e-4f);
    EXPECT_NEAR(p_back.y, 25.0f, 1e-4f);
}

TEST_CASE(test_from_rect_to_quad_homography) {
    auto src_rect = Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f);
    EXPECT_TRUE(src_rect.has_value());

    // Map square to trapezoid (perspective tilt):
    // Top edge narrows (p0=(20, 10), p1=(80, 10))
    // Bottom edge wide (p2=(100, 90), p3=(0, 90))
    Point q0(20.0f, 10.0f);
    Point q1(80.0f, 10.0f);
    Point q2(100.0f, 90.0f);
    Point q3(0.0f, 90.0f);

    auto h_opt = Transform4x4::from_rect_to_quad(*src_rect, q0, q1, q2, q3);
    EXPECT_TRUE(h_opt.has_value());

    // Verify all 4 corners map precisely
    Point m0 = h_opt->map_point(Point(0.0f, 0.0f));
    EXPECT_NEAR(m0.x, q0.x, 1e-3f);
    EXPECT_NEAR(m0.y, q0.y, 1e-3f);

    Point m1 = h_opt->map_point(Point(100.0f, 0.0f));
    EXPECT_NEAR(m1.x, q1.x, 1e-3f);
    EXPECT_NEAR(m1.y, q1.y, 1e-3f);

    Point m2 = h_opt->map_point(Point(100.0f, 100.0f));
    EXPECT_NEAR(m2.x, q2.x, 1e-3f);
    EXPECT_NEAR(m2.y, q2.y, 1e-3f);

    Point m3 = h_opt->map_point(Point(0.0f, 100.0f));
    EXPECT_NEAR(m3.x, q3.x, 1e-3f);
    EXPECT_NEAR(m3.y, q3.y, 1e-3f);
}

TEST_CASE(test_rasterize_perspective_quad) {
    // Create destination pixmap (100x100, clear transparent)
    auto dst = Pixmap::allocate(100, 100);
    EXPECT_TRUE(dst.has_value());
    dst->fill(Color::TRANSPARENT);

    // Create a 20x20 solid red source pixmap
    auto src = Pixmap::allocate(20, 20);
    EXPECT_TRUE(src.has_value());
    src->fill(Color::from_rgba8(255, 0, 0, 255));

    // Map onto destination quad:
    Point p0(30.0f, 20.0f);
    Point p1(70.0f, 20.0f);
    Point p2(80.0f, 80.0f);
    Point p3(20.0f, 80.0f);

    raster::draw_pixmap_perspective_quad(
        dst->as_mut(),
        src->as_ref(),
        p0, p1, p2, p3,
        1.0f,
        BlendMode::SourceOver
    );

    // Center pixel (50, 50) must be inside the quad and red
    auto center_px = dst->pixel(50, 50);
    EXPECT_TRUE(center_px.has_value());
    EXPECT_NEAR(center_px->red(), 255, 1);
    EXPECT_NEAR(center_px->alpha(), 255, 1);

    // Outside pixels should remain transparent
    auto outside_px = dst->pixel(5, 5);
    EXPECT_TRUE(outside_px.has_value());
    EXPECT_EQ(outside_px->alpha(), 0);
}

TEST_CASE(test_canvas_draw_pixmap_3d) {
    auto dst = Pixmap::allocate(200, 200);
    EXPECT_TRUE(dst.has_value());
    dst->fill(Color::TRANSPARENT);

    Canvas canvas(*dst);

    // Create a small 40x40 blue texture
    auto src = Pixmap::allocate(40, 40);
    EXPECT_TRUE(src.has_value());
    src->fill(Color::from_rgba8(0, 100, 255, 255));

    // 3D camera orbit transform: center at (100, 100), tilted around Y by 30 deg
    auto t3d = Transform4x4::from_camera_orbit(100.0f, 100.0f, 15.0f, 25.0f, 0.0f, 600.0f);
    // Translate source to center
    t3d = t3d * Transform4x4::from_translate(80.0f, 80.0f, 0.0f);

    canvas.draw_pixmap_3d(src->as_ref(), t3d, 0.9f, BlendMode::SourceOver);

    // Pixel around center (100, 100) must have been colored
    auto px = dst->pixel(100, 100);
    EXPECT_TRUE(px.has_value());
    EXPECT_TRUE(px->alpha() > 200);
    EXPECT_TRUE(px->blue() > 180);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Nisaba 3D Perspective & Spatial Transform Tests " << std::endl;
    std::cout << "========================================" << std::endl;

    RUN_TEST(test_identity_and_basic_transforms);
    RUN_TEST(test_3d_rotations);
    RUN_TEST(test_matrix_multiply_and_invert);
    RUN_TEST(test_perspective_projection);
    RUN_TEST(test_from_rect_to_quad_homography);
    RUN_TEST(test_rasterize_perspective_quad);
    RUN_TEST(test_canvas_draw_pixmap_3d);

    std::cout << "========================================" << std::endl;
    std::cout << " Total Passed: " << g_tests_passed << std::endl;
    std::cout << " Total Failed: " << g_tests_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}

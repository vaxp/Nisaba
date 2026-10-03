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

TEST_CASE(test_vertices_triangle_modes) {
    // 1. Independent Triangles (6 points -> 2 triangles)
    std::vector<Point> pts6 = {
        Point(0, 0), Point(10, 0), Point(0, 10),
        Point(10, 0), Point(10, 10), Point(0, 10)
    };
    auto v_tri = Vertices::create_triangles(pts6);
    EXPECT_TRUE(v_tri.is_valid());
    EXPECT_EQ(v_tri.triangle_count(), 2u);

    auto [a0, a1, a2] = v_tri.get_triangle_indices(0);
    EXPECT_EQ(a0, 0u); EXPECT_EQ(a1, 1u); EXPECT_EQ(a2, 2u);
    auto [b0, b1, b2] = v_tri.get_triangle_indices(1);
    EXPECT_EQ(b0, 3u); EXPECT_EQ(b1, 4u); EXPECT_EQ(b2, 5u);

    // 2. Triangle Strip (4 points -> 2 triangles)
    std::vector<Point> pts4 = {
        Point(0, 0), Point(0, 10), Point(10, 0), Point(10, 10)
    };
    auto v_strip = Vertices::create_triangle_strip(pts4);
    EXPECT_TRUE(v_strip.is_valid());
    EXPECT_EQ(v_strip.triangle_count(), 2u);

    // 3. Triangle Fan (5 points -> 3 triangles)
    std::vector<Point> pts5 = {
        Point(5, 5), Point(0, 0), Point(10, 0), Point(10, 10), Point(0, 10)
    };
    auto v_fan = Vertices::create_triangle_fan(pts5);
    EXPECT_TRUE(v_fan.is_valid());
    EXPECT_EQ(v_fan.triangle_count(), 3u);
}

TEST_CASE(test_vertices_grid_builder) {
    // 3x3 grid (9 points -> 4 quads -> 8 triangles)
    std::vector<Point> pts;
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            pts.push_back(Point(static_cast<float>(x * 10), static_cast<float>(y * 10)));
        }
    }

    auto grid = Vertices::create_grid(3, 3, pts);
    EXPECT_TRUE(grid.is_valid());
    EXPECT_EQ(grid.triangle_count(), 8u);

    Rect b = grid.bounds();
    EXPECT_NEAR(b.left(), 0.0f, 1e-4f);
    EXPECT_NEAR(b.top(), 0.0f, 1e-4f);
    EXPECT_NEAR(b.right(), 20.0f, 1e-4f);
    EXPECT_NEAR(b.bottom(), 20.0f, 1e-4f);
}

TEST_CASE(test_coons_patch_evaluation) {
    CoonsPatch patch;
    // Define a 100x100 square Coons patch with straight boundary curves
    float one_third = 100.0f / 3.0f;
    float two_thirds = 200.0f / 3.0f;
    patch.top[0] = Point(0, 0);          patch.top[1] = Point(one_third, 0); patch.top[2] = Point(two_thirds, 0); patch.top[3] = Point(100, 0);
    patch.right[0] = Point(100, 0);      patch.right[1] = Point(100, one_third); patch.right[2] = Point(100, two_thirds); patch.right[3] = Point(100, 100);
    patch.bottom[0] = Point(0, 100);     patch.bottom[1] = Point(one_third, 100); patch.bottom[2] = Point(two_thirds, 100); patch.bottom[3] = Point(100, 100);
    patch.left[0] = Point(0, 0);         patch.left[1] = Point(0, one_third); patch.left[2] = Point(0, two_thirds); patch.left[3] = Point(0, 100);

    patch.color_top_left = Color::from_rgba8(255, 0, 0, 255);      // Red
    patch.color_top_right = Color::from_rgba8(0, 255, 0, 255);     // Green
    patch.color_bottom_left = Color::from_rgba8(0, 0, 255, 255);    // Blue
    patch.color_bottom_right = Color::from_rgba8(255, 255, 0, 255); // Yellow

    // Verify 4 corners evaluate to exact positions
    Point p_tl = patch.evaluate_position(0.0f, 0.0f);
    EXPECT_NEAR(p_tl.x, 0.0f, 1e-4f);
    EXPECT_NEAR(p_tl.y, 0.0f, 1e-4f);

    Point p_tr = patch.evaluate_position(1.0f, 0.0f);
    EXPECT_NEAR(p_tr.x, 100.0f, 1e-4f);
    EXPECT_NEAR(p_tr.y, 0.0f, 1e-4f);

    Point p_br = patch.evaluate_position(1.0f, 1.0f);
    EXPECT_NEAR(p_br.x, 100.0f, 1e-4f);
    EXPECT_NEAR(p_br.y, 100.0f, 1e-4f);

    Point p_bl = patch.evaluate_position(0.0f, 1.0f);
    EXPECT_NEAR(p_bl.x, 0.0f, 1e-4f);
    EXPECT_NEAR(p_bl.y, 100.0f, 1e-4f);

    // Center position should be at (50, 50)
    Point p_center = patch.evaluate_position(0.5f, 0.5f);
    EXPECT_NEAR(p_center.x, 50.0f, 1e-4f);
    EXPECT_NEAR(p_center.y, 50.0f, 1e-4f);

    // Color at top-left should be pure red
    Color c_tl = patch.evaluate_color(0.0f, 0.0f);
    EXPECT_NEAR(c_tl.red(), 1.0f, 1e-4f);
    EXPECT_NEAR(c_tl.green(), 0.0f, 1e-4f);
    EXPECT_NEAR(c_tl.blue(), 0.0f, 1e-4f);

    // Tessellation test
    Vertices mesh_v = patch.to_vertices(4, 4);
    EXPECT_TRUE(mesh_v.is_valid());
    EXPECT_EQ(mesh_v.triangle_count(), 32u); // 4x4 quads * 2 = 32 triangles
}

TEST_CASE(test_gouraud_shaded_triangle_raster) {
    auto dst = Pixmap::allocate(100, 100);
    EXPECT_TRUE(dst.has_value());
    dst->fill(Color::TRANSPARENT);

    // Triangle: (20, 20) Pure Red, (80, 20) Pure Green, (50, 80) Pure Blue
    Point p0(20.0f, 20.0f); Color c0 = Color::from_rgba8(255, 0, 0, 255);
    Point p1(80.0f, 20.0f); Color c1 = Color::from_rgba8(0, 255, 0, 255);
    Point p2(50.0f, 80.0f); Color c2 = Color::from_rgba8(0, 0, 255, 255);

    raster::draw_colored_triangle(dst->as_mut(), p0, p1, p2, c0, c1, c2);

    // Top-left corner near p0 should be heavily red
    auto px_red = dst->pixel(25, 25);
    EXPECT_TRUE(px_red.has_value());
    EXPECT_TRUE(px_red->red() > 190);

    // Top-right corner near p1 should be heavily green
    auto px_green = dst->pixel(72, 25);
    EXPECT_TRUE(px_green.has_value());
    EXPECT_TRUE(px_green->green() > 190);

    // Bottom corner near p2 should be heavily blue
    auto px_blue = dst->pixel(50, 72);
    EXPECT_TRUE(px_blue.has_value());
    EXPECT_TRUE(px_blue->blue() > 190);

    // Center pixel (50, 40) must have a blend of all 3 channels
    auto px_center = dst->pixel(50, 40);
    EXPECT_TRUE(px_center.has_value());
    EXPECT_TRUE(px_center->alpha() > 250);
    EXPECT_TRUE(px_center->red() > 50);
    EXPECT_TRUE(px_center->green() > 50);
    EXPECT_TRUE(px_center->blue() > 50);

    // Outside pixel should remain transparent
    auto px_out = dst->pixel(5, 5);
    EXPECT_TRUE(px_out.has_value());
    EXPECT_EQ(px_out->alpha(), 0);
}

TEST_CASE(test_textured_triangle_raster) {
    auto dst = Pixmap::allocate(100, 100);
    EXPECT_TRUE(dst.has_value());
    dst->fill(Color::TRANSPARENT);

    // Create a 20x20 yellow texture
    auto tex = Pixmap::allocate(20, 20);
    EXPECT_TRUE(tex.has_value());
    tex->fill(Color::from_rgba8(255, 255, 0, 255));

    Point p0(10.0f, 10.0f); Point uv0(0.0f, 0.0f);
    Point p1(90.0f, 10.0f); Point uv1(20.0f, 0.0f);
    Point p2(50.0f, 90.0f); Point uv2(10.0f, 20.0f);

    raster::draw_textured_triangle(dst->as_mut(), tex->as_ref(), p0, p1, p2, uv0, uv1, uv2);

    // Center pixel should be yellow
    auto px = dst->pixel(50, 40);
    EXPECT_TRUE(px.has_value());
    EXPECT_NEAR(px->red(), 255, 1);
    EXPECT_NEAR(px->green(), 255, 1);
    EXPECT_NEAR(px->blue(), 0, 1);
}

TEST_CASE(test_canvas_draw_vertices_and_gradient_mesh) {
    auto dst = Pixmap::allocate(200, 200);
    EXPECT_TRUE(dst.has_value());
    dst->fill(Color::TRANSPARENT);

    Canvas canvas(*dst);

    // Build a Coons patch
    CoonsPatch patch;
    patch.top[0] = Point(20, 20);    patch.top[1] = Point(60, 20);    patch.top[2] = Point(120, 20);   patch.top[3] = Point(180, 20);
    patch.right[0] = Point(180, 20); patch.right[1] = Point(180, 60); patch.right[2] = Point(180, 120); patch.right[3] = Point(180, 180);
    patch.bottom[0] = Point(20, 180);patch.bottom[1] = Point(60, 180);patch.bottom[2] = Point(120, 180);patch.bottom[3] = Point(180, 180);
    patch.left[0] = Point(20, 20);   patch.left[1] = Point(20, 60);   patch.left[2] = Point(20, 120);  patch.left[3] = Point(20, 180);

    patch.color_top_left = Color::from_rgba8(255, 0, 128, 255);
    patch.color_top_right = Color::from_rgba8(0, 229, 255, 255);
    patch.color_bottom_right = Color::from_rgba8(0, 255, 136, 255);
    patch.color_bottom_left = Color::from_rgba8(255, 170, 0, 255);

    canvas.draw_coons_patch(patch, BlendMode::SourceOver, 1.0f, 6, 6);

    // Verify center pixel has been shaded
    auto px_center = dst->pixel(100, 100);
    EXPECT_TRUE(px_center.has_value());
    EXPECT_TRUE(px_center->alpha() > 250);
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Nisaba 2D Vertex Mesh & Gradient Mesh Tests " << std::endl;
    std::cout << "========================================" << std::endl;

    RUN_TEST(test_vertices_triangle_modes);
    RUN_TEST(test_vertices_grid_builder);
    RUN_TEST(test_coons_patch_evaluation);
    RUN_TEST(test_gouraud_shaded_triangle_raster);
    RUN_TEST(test_textured_triangle_raster);
    RUN_TEST(test_canvas_draw_vertices_and_gradient_mesh);

    std::cout << "========================================" << std::endl;
    std::cout << " Total Passed: " << g_tests_passed << std::endl;
    std::cout << " Total Failed: " << g_tests_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed == 0 ? 0 : 1;
}

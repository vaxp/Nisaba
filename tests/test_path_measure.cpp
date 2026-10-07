#include <iostream>
#include <cmath>
#include <cassert>
#include "nisaba/nisaba.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/path_measure.hpp"

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

TEST_CASE(test_empty_and_degenerate_paths) {
    Path empty_path;
    PathMeasure pm(empty_path, false);
    EXPECT_EQ(pm.length(), 0.0f);
    EXPECT_EQ(pm.contour_count(), 0u);
    EXPECT_FALSE(pm.is_closed());
    EXPECT_FALSE(pm.next_contour());

    Point pos, tan;
    EXPECT_FALSE(pm.get_pos_tan(0.0f, &pos, &tan));

    Path dst;
    EXPECT_FALSE(pm.get_segment(0.0f, 10.0f, &dst));

    // Degenerate Move-only path
    PathBuilder pb;
    pb.move_to(50.0f, 50.0f);
    auto move_only = pb.finish();
    // Path with only Move has verbs.size() == 1, finish() returns std::nullopt
    EXPECT_FALSE(move_only.has_value());

    // Path with multiple MoveTos but no actual lines
    PathBuilder pb2;
    pb2.move_to(10.0f, 10.0f);
    pb2.move_to(20.0f, 20.0f);
    auto multi_move = pb2.finish();
    EXPECT_FALSE(multi_move.has_value());
}

TEST_CASE(test_straight_line_measure) {
    PathBuilder pb;
    pb.move_to(0.0f, 0.0f);
    pb.line_to(100.0f, 0.0f);
    auto path = pb.finish();
    EXPECT_TRUE(path.has_value());

    PathMeasure pm(*path, false);
    EXPECT_EQ(pm.contour_count(), 1u);
    EXPECT_NEAR(pm.length(), 100.0f, 1e-4f);
    EXPECT_FALSE(pm.is_closed());

    // Evaluate pos and tan along the line
    Point pos, tan;
    EXPECT_TRUE(pm.get_pos_tan(50.0f, &pos, &tan));
    EXPECT_NEAR(pos.x, 50.0f, 1e-3f);
    EXPECT_NEAR(pos.y, 0.0f, 1e-3f);
    EXPECT_NEAR(tan.x, 1.0f, 1e-4f);
    EXPECT_NEAR(tan.y, 0.0f, 1e-4f);

    // Quarter and three-quarters
    EXPECT_TRUE(pm.get_pos_tan(25.0f, &pos, &tan));
    EXPECT_NEAR(pos.x, 25.0f, 1e-3f);
    EXPECT_NEAR(pos.y, 0.0f, 1e-3f);

    EXPECT_TRUE(pm.get_pos_tan(75.0f, &pos, &tan));
    EXPECT_NEAR(pos.x, 75.0f, 1e-3f);
    EXPECT_NEAR(pos.y, 0.0f, 1e-3f);

    // Sub-segment extraction
    Path dst;
    EXPECT_TRUE(pm.get_segment(20.0f, 80.0f, &dst, true));
    PathMeasure sub_pm(dst, false);
    EXPECT_NEAR(sub_pm.length(), 60.0f, 1e-3f);

    Point sub_p0, sub_p1;
    EXPECT_TRUE(sub_pm.get_pos_tan(0.0f, &sub_p0, nullptr));
    EXPECT_TRUE(sub_pm.get_pos_tan(60.0f, &sub_p1, nullptr));
    EXPECT_NEAR(sub_p0.x, 20.0f, 1e-3f);
    EXPECT_NEAR(sub_p1.x, 80.0f, 1e-3f);
}

TEST_CASE(test_diagonal_line_measure) {
    PathBuilder pb;
    pb.move_to(10.0f, 20.0f);
    // 30, 40 triangle => hypotenuse = 50
    pb.line_to(40.0f, 60.0f);
    auto path = pb.finish();
    EXPECT_TRUE(path.has_value());

    PathMeasure pm(*path, false);
    EXPECT_NEAR(pm.length(), 50.0f, 1e-4f);

    Point pos, tan;
    EXPECT_TRUE(pm.get_pos_tan(25.0f, &pos, &tan));
    EXPECT_NEAR(pos.x, 25.0f, 1e-3f);
    EXPECT_NEAR(pos.y, 40.0f, 1e-3f);
    EXPECT_NEAR(tan.x, 0.6f, 1e-4f);
    EXPECT_NEAR(tan.y, 0.8f, 1e-4f);
    EXPECT_NEAR(tan.length(), 1.0f, 1e-4f);
}

TEST_CASE(test_closed_and_force_closed_triangle) {
    // Right triangle with legs 30 and 40, hypotenuse 50
    // Open path: (0,0) -> (30,0) -> (30,40)
    PathBuilder pb;
    pb.move_to(0.0f, 0.0f);
    pb.line_to(30.0f, 0.0f);
    pb.line_to(30.0f, 40.0f);
    auto open_path = pb.finish();
    EXPECT_TRUE(open_path.has_value());

    // Without force_closed
    PathMeasure pm_open(*open_path, false);
    EXPECT_NEAR(pm_open.length(), 70.0f, 1e-3f);
    EXPECT_FALSE(pm_open.is_closed());

    // With force_closed = true
    PathMeasure pm_closed(*open_path, true);
    EXPECT_NEAR(pm_closed.length(), 120.0f, 1e-3f);
    EXPECT_TRUE(pm_closed.is_closed());

    // Position at d = 70 should be (30, 40)
    Point pos;
    EXPECT_TRUE(pm_closed.get_pos_tan(70.0f, &pos, nullptr));
    EXPECT_NEAR(pos.x, 30.0f, 1e-3f);
    EXPECT_NEAR(pos.y, 40.0f, 1e-3f);

    // Position at midpoint of hypotenuse (d = 70 + 25 = 95) should be (15, 20)
    EXPECT_TRUE(pm_closed.get_pos_tan(95.0f, &pos, nullptr));
    EXPECT_NEAR(pos.x, 15.0f, 1e-3f);
    EXPECT_NEAR(pos.y, 20.0f, 1e-3f);
}

TEST_CASE(test_circle_perimeter_accuracy) {
    const float radius = 100.0f;
    const float cx = 150.0f;
    const float cy = 150.0f;
    auto circle_opt = PathBuilder::from_circle(cx, cy, radius);
    EXPECT_TRUE(circle_opt.has_value());

    PathMeasure pm(*circle_opt, true, 1.0f);
    EXPECT_EQ(pm.contour_count(), 1u);
    EXPECT_TRUE(pm.is_closed());

    const float expected_circumference = 2.0f * static_cast<float>(M_PI) * radius;
    // Approximating circle via 4 cubic Béziers has an analytical error < 0.03%
    EXPECT_NEAR(pm.length(), expected_circumference, 0.5f);

    // Verify points around the circle lie on (x - cx)^2 + (y - cy)^2 = R^2
    const float total_len = pm.length();
    for (int i = 0; i < 16; ++i) {
        float d = total_len * (static_cast<float>(i) / 16.0f);
        Point pos, tan;
        EXPECT_TRUE(pm.get_pos_tan(d, &pos, &tan));

        float dx = pos.x - cx;
        float dy = pos.y - cy;
        float dist_from_center = std::sqrt(dx * dx + dy * dy);
        EXPECT_NEAR(dist_from_center, radius, 0.5f);

        // Tangent must be unit length
        EXPECT_NEAR(tan.length(), 1.0f, 1e-4f);

        // Tangent must be orthogonal to radius vector (dot product == 0)
        float dot_prod = (dx * tan.x + dy * tan.y) / radius;
        EXPECT_NEAR(dot_prod, 0.0f, 1e-2f);
    }
}

TEST_CASE(test_spinner_subsegment_extraction) {
    // Tests animated spinner widget use-case (ENKI Spinner / ProgressRing)
    const float radius = 50.0f;
    auto circle_opt = PathBuilder::from_circle(100.0f, 100.0f, radius);
    EXPECT_TRUE(circle_opt.has_value());

    PathMeasure pm(*circle_opt, true);
    const float full_len = pm.length();

    // Extract a 90-degree arc (one quarter of circle)
    float start_d = full_len * 0.25f;
    float stop_d = full_len * 0.50f;
    Path arc_segment;
    EXPECT_TRUE(pm.get_segment(start_d, stop_d, &arc_segment, true));

    PathMeasure arc_pm(arc_segment, false);
    EXPECT_NEAR(arc_pm.length(), stop_d - start_d, 0.5f);

    // Extract multiple consecutive segments with start_with_move_to = false
    Path combined;
    EXPECT_TRUE(pm.get_segment(0.0f, full_len * 0.25f, &combined, true));
    EXPECT_TRUE(pm.get_segment(full_len * 0.25f, full_len * 0.50f, &combined, false));

    PathMeasure comb_pm(combined, false);
    EXPECT_NEAR(comb_pm.length(), full_len * 0.50f, 0.5f);
}

TEST_CASE(test_multi_contour_navigation) {
    PathBuilder pb;
    // Contour 0: square 50x50, closed => perimeter = 200
    pb.move_to(0.0f, 0.0f);
    pb.line_to(50.0f, 0.0f);
    pb.line_to(50.0f, 50.0f);
    pb.line_to(0.0f, 50.0f);
    pb.close();

    // Contour 1: horizontal line from (100, 100) to (250, 100) => length = 150
    pb.move_to(100.0f, 100.0f);
    pb.line_to(250.0f, 100.0f);

    // Contour 2: vertical line from (300, 0) to (300, 80) => length = 80
    pb.move_to(300.0f, 0.0f);
    pb.line_to(300.0f, 80.0f);

    auto multi_path = pb.finish();
    EXPECT_TRUE(multi_path.has_value());

    PathMeasure pm(*multi_path, false);
    EXPECT_EQ(pm.contour_count(), 3u);

    // Contour 0
    EXPECT_EQ(pm.current_contour_index(), 0u);
    EXPECT_NEAR(pm.length(), 200.0f, 1e-3f);
    EXPECT_TRUE(pm.is_closed());

    // Advance to Contour 1
    EXPECT_TRUE(pm.next_contour());
    EXPECT_EQ(pm.current_contour_index(), 1u);
    EXPECT_NEAR(pm.length(), 150.0f, 1e-3f);
    EXPECT_FALSE(pm.is_closed());

    // Advance to Contour 2
    EXPECT_TRUE(pm.next_contour());
    EXPECT_EQ(pm.current_contour_index(), 2u);
    EXPECT_NEAR(pm.length(), 80.0f, 1e-3f);
    EXPECT_FALSE(pm.is_closed());

    // Advance past end
    EXPECT_FALSE(pm.next_contour());
    EXPECT_EQ(pm.length(), 0.0f);
    EXPECT_FALSE(pm.is_closed());
}

TEST_CASE(test_quadratic_and_cubic_bezier_measure) {
    // Symmetric quadratic Bézier: (0,0) -> (50, 100) -> (100, 0)
    PathBuilder pb_quad;
    pb_quad.move_to(0.0f, 0.0f);
    pb_quad.quad_to(50.0f, 100.0f, 100.0f, 0.0f);
    auto quad_path = pb_quad.finish();
    EXPECT_TRUE(quad_path.has_value());

    PathMeasure quad_pm(*quad_path, false);
    float quad_len = quad_pm.length();
    // Straight chord is 100, control point is at y=100 so length > 100
    EXPECT_TRUE(quad_len > 100.0f);

    // Apex at distance L/2 must be near x = 50, tangent horizontal (1, 0)
    Point apex_pos, apex_tan;
    EXPECT_TRUE(quad_pm.get_pos_tan(quad_len * 0.5f, &apex_pos, &apex_tan));
    EXPECT_NEAR(apex_pos.x, 50.0f, 0.5f);
    EXPECT_NEAR(apex_tan.x, 1.0f, 1e-2f);
    EXPECT_NEAR(apex_tan.y, 0.0f, 1e-2f);

    // Symmetric cubic Bézier: (0,0) -> (0, 100) -> (100, 100) -> (100, 0)
    PathBuilder pb_cubic;
    pb_cubic.move_to(0.0f, 0.0f);
    pb_cubic.cubic_to(0.0f, 100.0f, 100.0f, 100.0f, 100.0f, 0.0f);
    auto cubic_path = pb_cubic.finish();
    EXPECT_TRUE(cubic_path.has_value());

    PathMeasure cubic_pm(*cubic_path, false);
    float cubic_len = cubic_pm.length();
    EXPECT_TRUE(cubic_len > 150.0f);

    Point c_apex, c_tan;
    EXPECT_TRUE(cubic_pm.get_pos_tan(cubic_len * 0.5f, &c_apex, &c_tan));
    EXPECT_NEAR(c_apex.x, 50.0f, 0.5f);
    EXPECT_NEAR(c_tan.x, 1.0f, 1e-2f);
    EXPECT_NEAR(c_tan.y, 0.0f, 1e-2f);
}

TEST_CASE(test_skia_camel_case_compatibility) {
    PathBuilder pb;
    pb.move_to(0.0f, 0.0f);
    pb.line_to(200.0f, 0.0f);
    auto path = pb.finish();
    EXPECT_TRUE(path.has_value());

    PathMeasure pm;
    pm.setPath(&*path, false);
    EXPECT_NEAR(pm.getLength(), 200.0f, 1e-4f);
    EXPECT_FALSE(pm.isClosed());

    Point pos, tan;
    EXPECT_TRUE(pm.getPosTan(100.0f, &pos, &tan));
    EXPECT_NEAR(pos.x, 100.0f, 1e-3f);
    EXPECT_NEAR(tan.x, 1.0f, 1e-4f);

    Path dst;
    EXPECT_TRUE(pm.getSegment(50.0f, 150.0f, &dst, true));
    PathMeasure sub;
    sub.setPath(dst, false);
    EXPECT_NEAR(sub.getLength(), 100.0f, 1e-3f);

    EXPECT_FALSE(pm.nextContour());
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Nisaba PathMeasure Automated Tests    " << std::endl;
    std::cout << "========================================" << std::endl;

    RUN_TEST(test_empty_and_degenerate_paths);
    RUN_TEST(test_straight_line_measure);
    RUN_TEST(test_diagonal_line_measure);
    RUN_TEST(test_closed_and_force_closed_triangle);
    RUN_TEST(test_circle_perimeter_accuracy);
    RUN_TEST(test_spinner_subsegment_extraction);
    RUN_TEST(test_multi_contour_navigation);
    RUN_TEST(test_quadratic_and_cubic_bezier_measure);
    RUN_TEST(test_skia_camel_case_compatibility);

    std::cout << "========================================" << std::endl;
    std::cout << "  Results: " << g_tests_passed << " Passed, "
              << g_tests_failed << " Failed" << std::endl;
    std::cout << "========================================" << std::endl;

    return g_tests_failed > 0 ? 1 : 0;
}

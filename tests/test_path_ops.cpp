#include <iostream>
#include <cmath>
#include <cassert>
#include "nisaba/nisaba.hpp"
#include "nisaba/path/path_ops.hpp"

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

TEST_CASE(test_empty_paths) {
    Path empty;
    auto r = Rect::from_xywh(10, 10, 50, 50);
    EXPECT_TRUE(r.has_value());
    Path rect = PathBuilder::from_rect(*r);

    // Union
    auto u1 = path_union(empty, rect);
    EXPECT_TRUE(u1.has_value());
    EXPECT_FALSE(u1->is_empty());
    EXPECT_NEAR(u1->bounds().width(), 50.0f, 1e-3f);

    auto u2 = path_union(rect, empty);
    EXPECT_TRUE(u2.has_value());
    EXPECT_FALSE(u2->is_empty());

    // Intersect
    auto i1 = path_intersect(empty, rect);
    EXPECT_TRUE(i1.has_value());
    EXPECT_TRUE(i1->is_empty());

    // Difference
    auto d1 = path_difference(rect, empty);
    EXPECT_TRUE(d1.has_value());
    EXPECT_FALSE(d1->is_empty());
    EXPECT_NEAR(d1->bounds().width(), 50.0f, 1e-3f);

    auto d2 = path_difference(empty, rect);
    EXPECT_TRUE(d2.has_value());
    EXPECT_TRUE(d2->is_empty());

    // Xor
    auto x1 = path_xor(empty, rect);
    EXPECT_TRUE(x1.has_value());
    EXPECT_FALSE(x1->is_empty());
}

TEST_CASE(test_disjoint_rectangles) {
    auto r1 = Rect::from_xywh(0, 0, 10, 10);
    auto r2 = Rect::from_xywh(30, 30, 10, 10);
    Path p1 = PathBuilder::from_rect(*r1);
    Path p2 = PathBuilder::from_rect(*r2);

    auto u = path_union(p1, p2);
    EXPECT_TRUE(u.has_value());
    EXPECT_FALSE(u->is_empty());
    EXPECT_NEAR(u->bounds().left(), 0.0f, 1e-3f);
    EXPECT_NEAR(u->bounds().right(), 40.0f, 1e-3f);

    auto i = path_intersect(p1, p2);
    EXPECT_TRUE(i.has_value());
    EXPECT_TRUE(i->is_empty());

    auto d = path_difference(p1, p2);
    EXPECT_TRUE(d.has_value());
    EXPECT_FALSE(d->is_empty());
    EXPECT_NEAR(d->bounds().width(), 10.0f, 1e-3f);
}

TEST_CASE(test_overlapping_rectangles) {
    // A: [0, 0, 20, 20]
    // B: [10, 0, 20, 20]
    auto r1 = Rect::from_xywh(0, 0, 20, 20);
    auto r2 = Rect::from_xywh(10, 0, 20, 20);
    Path p1 = PathBuilder::from_rect(*r1);
    Path p2 = PathBuilder::from_rect(*r2);

    // Union should cover [0, 0] to [30, 20] -> width 30, height 20
    auto u = path_union(p1, p2);
    EXPECT_TRUE(u.has_value());
    EXPECT_FALSE(u->is_empty());
    EXPECT_NEAR(u->bounds().left(), 0.0f, 0.1f);
    EXPECT_NEAR(u->bounds().right(), 30.0f, 0.1f);
    EXPECT_NEAR(u->bounds().height(), 20.0f, 0.1f);

    // Intersect should cover [10, 0] to [20, 20] -> width 10, height 20
    auto i = path_intersect(p1, p2);
    EXPECT_TRUE(i.has_value());
    EXPECT_FALSE(i->is_empty());
    EXPECT_NEAR(i->bounds().left(), 10.0f, 0.1f);
    EXPECT_NEAR(i->bounds().right(), 20.0f, 0.1f);
    EXPECT_NEAR(i->bounds().height(), 20.0f, 0.1f);

    // Difference p1 - p2 should cover [0, 0] to [10, 20] -> width 10, height 20
    auto d1 = path_difference(p1, p2);
    EXPECT_TRUE(d1.has_value());
    EXPECT_FALSE(d1->is_empty());
    EXPECT_NEAR(d1->bounds().left(), 0.0f, 0.1f);
    EXPECT_NEAR(d1->bounds().right(), 10.0f, 0.1f);

    // Difference p2 - p1 should cover [20, 0] to [30, 20] -> width 10, height 20
    auto d2 = path_difference(p2, p1);
    EXPECT_TRUE(d2.has_value());
    EXPECT_FALSE(d2->is_empty());
    EXPECT_NEAR(d2->bounds().left(), 20.0f, 0.1f);
    EXPECT_NEAR(d2->bounds().right(), 30.0f, 0.1f);
}

TEST_CASE(test_nested_rectangles) {
    // Outer: [0, 0, 100, 100]
    // Inner: [25, 25, 50, 50]
    auto r_out = Rect::from_xywh(0, 0, 100, 100);
    auto r_in = Rect::from_xywh(25, 25, 50, 50);
    Path p_out = PathBuilder::from_rect(*r_out);
    Path p_in = PathBuilder::from_rect(*r_in);

    // Intersect of nested: should be inner rect
    auto i = path_intersect(p_out, p_in);
    EXPECT_TRUE(i.has_value());
    EXPECT_FALSE(i->is_empty());
    EXPECT_NEAR(i->bounds().left(), 25.0f, 0.1f);
    EXPECT_NEAR(i->bounds().width(), 50.0f, 0.1f);

    // Union of nested: should be outer rect
    auto u = path_union(p_out, p_in);
    EXPECT_TRUE(u.has_value());
    EXPECT_FALSE(u->is_empty());
    EXPECT_NEAR(u->bounds().left(), 0.0f, 0.1f);
    EXPECT_NEAR(u->bounds().width(), 100.0f, 0.1f);

    // Difference: outer with inner subtracted (hole)
    auto d = path_difference(p_out, p_in);
    EXPECT_TRUE(d.has_value());
    EXPECT_FALSE(d->is_empty());
    EXPECT_NEAR(d->bounds().width(), 100.0f, 0.1f);
}

TEST_CASE(test_circle_and_rectangle) {
    // Circle centered at (50, 50) with radius 30 (bounds [20, 20, 80, 80])
    auto c = PathBuilder::from_circle(50.0f, 50.0f, 30.0f);
    EXPECT_TRUE(c.has_value());

    // Rect [50, 50, 50, 50] (lower-right quadrant of circle and beyond)
    auto r = Rect::from_xywh(50.0f, 50.0f, 50.0f, 50.0f);
    Path p_rect = PathBuilder::from_rect(*r);

    auto inter = path_intersect(*c, p_rect);
    EXPECT_TRUE(inter.has_value());
    EXPECT_FALSE(inter->is_empty());
    // The intersection should be contained within [50, 50] to [80, 80]
    EXPECT_NEAR(inter->bounds().left(), 50.0f, 0.2f);
    EXPECT_NEAR(inter->bounds().top(), 50.0f, 0.2f);
    EXPECT_NEAR(inter->bounds().right(), 80.0f, 0.2f);
    EXPECT_NEAR(inter->bounds().bottom(), 80.0f, 0.2f);

    auto diff = path_difference(*c, p_rect);
    EXPECT_TRUE(diff.has_value());
    EXPECT_FALSE(diff->is_empty());
}

TEST_CASE(test_two_overlapping_circles) {
    auto c1 = PathBuilder::from_circle(40.0f, 50.0f, 30.0f);
    auto c2 = PathBuilder::from_circle(60.0f, 50.0f, 30.0f);
    EXPECT_TRUE(c1.has_value() && c2.has_value());

    auto u = path_union(*c1, *c2);
    EXPECT_TRUE(u.has_value());
    EXPECT_FALSE(u->is_empty());
    EXPECT_NEAR(u->bounds().left(), 10.0f, 0.2f);
    EXPECT_NEAR(u->bounds().right(), 90.0f, 0.2f);

    auto inter = path_intersect(*c1, *c2);
    EXPECT_TRUE(inter.has_value());
    EXPECT_FALSE(inter->is_empty());
    // Overlapping lens between 30 and 70
    EXPECT_NEAR(inter->bounds().left(), 30.0f, 0.2f);
    EXPECT_NEAR(inter->bounds().right(), 70.0f, 0.2f);

    auto crescent = path_difference(*c1, *c2);
    EXPECT_TRUE(crescent.has_value());
    EXPECT_FALSE(crescent->is_empty());
}

TEST_CASE(test_path_member_methods) {
    auto r1 = Rect::from_xywh(0, 0, 20, 20);
    auto r2 = Rect::from_xywh(10, 0, 20, 20);
    Path p1 = PathBuilder::from_rect(*r1);
    Path p2 = PathBuilder::from_rect(*r2);

    auto u = p1.unite(p2);
    EXPECT_TRUE(u.has_value());
    EXPECT_NEAR(u->bounds().width(), 30.0f, 0.1f);

    auto i = p1.intersect(p2);
    EXPECT_TRUE(i.has_value());
    EXPECT_NEAR(i->bounds().width(), 10.0f, 0.1f);

    auto d = p1.difference(p2);
    EXPECT_TRUE(d.has_value());
    EXPECT_NEAR(d->bounds().width(), 10.0f, 0.1f);

    auto x = p1.xor_op(p2);
    EXPECT_TRUE(x.has_value());
    EXPECT_FALSE(x->is_empty());
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Nisaba PathOps Boolean Operations Test" << std::endl;
    std::cout << "========================================" << std::endl;

    RUN_TEST(test_empty_paths);
    RUN_TEST(test_disjoint_rectangles);
    RUN_TEST(test_overlapping_rectangles);
    RUN_TEST(test_nested_rectangles);
    RUN_TEST(test_circle_and_rectangle);
    RUN_TEST(test_two_overlapping_circles);
    RUN_TEST(test_path_member_methods);

    std::cout << "========================================" << std::endl;
    std::cout << "Results: " << g_tests_passed << " passed, " << g_tests_failed << " failed." << std::endl;
    return g_tests_failed == 0 ? 0 : 1;
}

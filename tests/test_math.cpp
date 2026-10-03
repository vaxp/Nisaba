#include <iostream>
#include <cmath>
#include <cassert>
#include <string>
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

TEST_CASE(test_scalar_bound) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    float inf = std::numeric_limits<float>::infinity();
    float eps = std::numeric_limits<float>::epsilon();

    EXPECT_EQ(scalar::bound(nan, 0.0f, 1.0f), 1.0f);
    EXPECT_EQ(scalar::bound(inf, 0.0f, 1.0f), 1.0f);
    EXPECT_EQ(scalar::bound(-inf, 0.0f, 1.0f), 0.0f);
    EXPECT_EQ(scalar::bound(eps, 0.0f, 1.0f), eps);
    EXPECT_EQ(scalar::bound(0.5f, 0.0f, 1.0f), 0.5f);
    EXPECT_EQ(scalar::bound(-1.0f, 0.0f, 1.0f), 0.0f);
    EXPECT_EQ(scalar::bound(2.0f, 0.0f, 1.0f), 1.0f);
}

TEST_CASE(test_point_math) {
    Point p = Point::from_xy(3.0f, 4.0f);
    EXPECT_EQ(p.length(), 5.0f);
    EXPECT_EQ(p.length_sqd(), 25.0f);

    Point p2 = Point::from_xy(0.0f, 0.0f);
    EXPECT_TRUE(p2.is_zero());

    Point p_norm = p;
    EXPECT_TRUE(p_norm.normalize());
    EXPECT_NEAR(p_norm.length(), 1.0f, 1e-6f);
    EXPECT_NEAR(p_norm.x, 0.6f, 1e-6f);
    EXPECT_NEAR(p_norm.y, 0.8f, 1e-6f);

    EXPECT_EQ(p.dot(Point::from_xy(2.0f, 1.0f)), 3.0f * 2.0f + 4.0f * 1.0f);
    EXPECT_EQ(p.cross(Point::from_xy(2.0f, 1.0f)), 3.0f * 1.0f - 4.0f * 2.0f);

    Point pr = p;
    pr.rotate_cw();
    EXPECT_EQ(pr.x, -4.0f);
    EXPECT_EQ(pr.y, 3.0f);
}

TEST_CASE(test_int_size) {
    EXPECT_FALSE(IntSize::from_wh(0, 0).has_value());
    EXPECT_FALSE(IntSize::from_wh(1, 0).has_value());
    EXPECT_FALSE(IntSize::from_wh(0, 1).has_value());

    auto s = IntSize::from_wh(3, 4);
    EXPECT_TRUE(s.has_value());
    EXPECT_EQ(s->width(), 3u);
    EXPECT_EQ(s->height(), 4u);
}

TEST_CASE(test_int_rect) {
    EXPECT_FALSE(IntRect::from_xywh(0, 0, 0, 0).has_value());
    EXPECT_FALSE(IntRect::from_xywh(0, 0, 1, 0).has_value());
    EXPECT_FALSE(IntRect::from_xywh(0, 0, 0, 1).has_value());

    // Intersection
    auto r1 = IntRect::from_xywh(1, 2, 3, 4).value();
    auto r2 = IntRect::from_xywh(11, 12, 13, 14).value();
    EXPECT_FALSE(r1.intersect(r2).has_value());

    auto r3 = IntRect::from_xywh(1, 2, 30, 40).value();
    auto r4 = IntRect::from_xywh(11, 12, 13, 14).value();
    EXPECT_EQ(r3.intersect(r4), r4);

    auto r5 = IntRect::from_xywh(1, 2, 30, 40).value();
    auto r6 = IntRect::from_xywh(11, 12, 50, 60).value();
    auto r_inter = r5.intersect(r6);
    EXPECT_TRUE(r_inter.has_value());
    EXPECT_EQ(r_inter.value(), IntRect::from_xywh(11, 12, 20, 30).value());
}

TEST_CASE(test_rect) {
    float nan = std::numeric_limits<float>::quiet_NaN();
    EXPECT_FALSE(Rect::from_ltrb(10.0f, 10.0f, 5.0f, 10.0f).has_value());
    EXPECT_FALSE(Rect::from_ltrb(10.0f, 10.0f, 10.0f, 5.0f).has_value());
    EXPECT_FALSE(Rect::from_ltrb(nan, 10.0f, 10.0f, 10.0f).has_value());

    auto rect = Rect::from_ltrb(10.0f, 20.0f, 30.0f, 40.0f).value();
    EXPECT_EQ(rect.left(), 10.0f);
    EXPECT_EQ(rect.top(), 20.0f);
    EXPECT_EQ(rect.right(), 30.0f);
    EXPECT_EQ(rect.bottom(), 40.0f);
    EXPECT_EQ(rect.width(), 20.0f);
    EXPECT_EQ(rect.height(), 20.0f);

    auto rect2 = Rect::from_ltrb(1.0f, 2.0f, 3.0f, 4.0f).value();
    EXPECT_EQ(rect2.transform(Transform::identity()).value(), rect2);

    auto ts_scale = Transform::from_scale(1.0f, 2.0f);
    EXPECT_EQ(rect2.transform(ts_scale).value(), Rect::from_ltrb(1.0f, 4.0f, 3.0f, 8.0f).value());

    auto ts_skew = Transform::from_skew(1.0f, 0.0f);
    EXPECT_EQ(rect2.transform(ts_skew).value(), Rect::from_ltrb(3.0f, 2.0f, 7.0f, 4.0f).value());

    auto ts_skew_y = Transform::from_skew(0.0f, 2.0f);
    EXPECT_EQ(rect2.transform(ts_skew_y).value(), Rect::from_ltrb(1.0f, 4.0f, 3.0f, 10.0f).value());
}

TEST_CASE(test_transform) {
    EXPECT_EQ(Transform::identity(), Transform::from_row(1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Transform::from_scale(1.0f, 2.0f), Transform::from_row(1.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.0f));
    EXPECT_EQ(Transform::from_skew(2.0f, 3.0f), Transform::from_row(1.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f));
    EXPECT_EQ(Transform::from_translate(5.0f, 6.0f), Transform::from_row(1.0f, 0.0f, 0.0f, 1.0f, 5.0f, 6.0f));

    Transform ts = Transform::from_scale(2.0f, 3.0f);
    EXPECT_FALSE(ts.is_identity());
    EXPECT_TRUE(ts.is_scale());
    EXPECT_FALSE(ts.is_skew());
    EXPECT_FALSE(ts.is_translate());
    EXPECT_TRUE(ts.is_scale_translate());
    EXPECT_TRUE(ts.has_scale());
    EXPECT_FALSE(ts.has_skew());
    EXPECT_FALSE(ts.has_translate());

    Transform ts_skew = Transform::from_skew(2.0f, 3.0f);
    EXPECT_FALSE(ts_skew.is_identity());
    EXPECT_FALSE(ts_skew.is_scale());
    EXPECT_TRUE(ts_skew.is_skew());
    EXPECT_FALSE(ts_skew.is_translate());
    EXPECT_FALSE(ts_skew.is_scale_translate());
    EXPECT_FALSE(ts_skew.has_scale());
    EXPECT_TRUE(ts_skew.has_skew());
    EXPECT_FALSE(ts_skew.has_translate());

    // Concat tests from nisaba
    Transform c1 = Transform::from_row(1.2f, 3.4f, -5.6f, -7.8f, 1.2f, 3.4f);
    c1 = c1.pre_scale(2.0f, -4.0f);
    EXPECT_NEAR(c1.sx, 2.4f, 1e-5f);
    EXPECT_NEAR(c1.ky, 6.8f, 1e-5f);
    EXPECT_NEAR(c1.kx, 22.4f, 1e-5f);
    EXPECT_NEAR(c1.sy, 31.2f, 1e-5f);
    EXPECT_NEAR(c1.tx, 1.2f, 1e-5f);
    EXPECT_NEAR(c1.ty, 3.4f, 1e-5f);

    Transform c2 = Transform::from_row(1.2f, 3.4f, -5.6f, -7.8f, 1.2f, 3.4f);
    c2 = c2.post_scale(2.0f, -4.0f);
    EXPECT_NEAR(c2.sx, 2.4f, 1e-5f);
    EXPECT_NEAR(c2.ky, -13.6f, 1e-5f);
    EXPECT_NEAR(c2.kx, -11.2f, 1e-5f);
    EXPECT_NEAR(c2.sy, 31.2f, 1e-5f);
    EXPECT_NEAR(c2.tx, 2.4f, 1e-5f);
    EXPECT_NEAR(c2.ty, -13.6f, 1e-5f);

    // Invert test
    Transform inv_test = Transform::from_row(1.0f, 2.0f, 0.0f, 1.0f, 10.0f, 20.0f);
    auto inv = inv_test.invert();
    EXPECT_TRUE(inv.has_value());
    Point pt = Point::from_xy(15.0f, 25.0f);
    Point orig = pt;
    inv_test.map_point(pt);
    inv->map_point(pt);
    EXPECT_NEAR(pt.x, orig.x, 1e-5f);
    EXPECT_NEAR(pt.y, orig.y, 1e-5f);
}

int main() {
    std::cout << "=== Running Nisaba Math Core Unit Tests ===" << std::endl;
    RUN_TEST(test_scalar_bound);
    RUN_TEST(test_point_math);
    RUN_TEST(test_int_size);
    RUN_TEST(test_int_rect);
    RUN_TEST(test_rect);
    RUN_TEST(test_transform);

    std::cout << "\nTest Summary: " << g_tests_passed << " passed, " << g_tests_failed << " failed." << std::endl;
    return g_tests_failed == 0 ? 0 : 1;
}

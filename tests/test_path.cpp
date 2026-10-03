#include <iostream>
#include <cmath>
#include <cassert>
#include "nisaba/nisaba.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"

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

TEST_CASE(test_cubic_eval) {
    Point src[4] = {
        Point::from_xy(30.0f, 40.0f),
        Point::from_xy(30.0f, 40.0f),
        Point::from_xy(171.0f, 45.0f),
        Point::from_xy(180.0f, 155.0f),
    };

    Point p = path_geometry::eval_cubic_pos_at(src, NormalizedF32::ZERO);
    EXPECT_EQ(p, Point::from_xy(30.0f, 40.0f));

    Point t = path_geometry::eval_cubic_tangent_at(src, NormalizedF32::ZERO);
    EXPECT_EQ(t, Point::from_xy(141.0f, 5.0f));
}

TEST_CASE(test_cubic_curvature) {
    Point src[4] = {
        Point::from_xy(20.0f, 160.0f),
        Point::from_xy(20.0001f, 160.0f),
        Point::from_xy(160.0f, 20.0f),
        Point::from_xy(160.0001f, 20.0f),
    };

    NormalizedF32 t_values[3] = {NormalizedF32::ZERO, NormalizedF32::ZERO, NormalizedF32::ZERO};
    size_t count = path_geometry::find_cubic_max_curvature(src, t_values);

    EXPECT_EQ(count, 3u);
    EXPECT_EQ(t_values[0], NormalizedF32::ZERO);
    EXPECT_NEAR(t_values[1].get(), 0.5f, 1e-4f);
    EXPECT_EQ(t_values[2], NormalizedF32::ONE);
}

TEST_CASE(test_path_builder_rect) {
    auto rect = Rect::from_xywh(10.0f, 20.0f, 30.0f, 40.0f).value();
    Path path = PathBuilder::from_rect(rect);

    EXPECT_EQ(path.len(), 5u);
    EXPECT_EQ(path.bounds(), rect);

    auto iter = path.segments();
    auto s1 = iter.next();
    EXPECT_TRUE(s1.has_value());
    EXPECT_EQ(s1->type, PathSegment::Type::MoveTo);
    EXPECT_EQ(s1->p0, Point::from_xy(10.0f, 20.0f));

    auto s2 = iter.next();
    EXPECT_TRUE(s2.has_value());
    EXPECT_EQ(s2->type, PathSegment::Type::LineTo);
    EXPECT_EQ(s2->p0, Point::from_xy(40.0f, 20.0f));

    auto s3 = iter.next();
    EXPECT_TRUE(s3.has_value());
    EXPECT_EQ(s3->type, PathSegment::Type::LineTo);
    EXPECT_EQ(s3->p0, Point::from_xy(40.0f, 60.0f));

    auto s4 = iter.next();
    EXPECT_TRUE(s4.has_value());
    EXPECT_EQ(s4->type, PathSegment::Type::LineTo);
    EXPECT_EQ(s4->p0, Point::from_xy(10.0f, 60.0f));

    auto s5 = iter.next();
    EXPECT_TRUE(s5.has_value());
    EXPECT_EQ(s5->type, PathSegment::Type::Close);

    EXPECT_FALSE(iter.next().has_value());
}

TEST_CASE(test_path_builder_circle) {
    auto path = PathBuilder::from_circle(100.0f, 100.0f, 50.0f);
    EXPECT_TRUE(path.has_value());

    EXPECT_NEAR(path->bounds().left(), 50.0f, 1e-4f);
    EXPECT_NEAR(path->bounds().top(), 50.0f, 1e-4f);
    EXPECT_NEAR(path->bounds().right(), 150.0f, 1e-4f);
    EXPECT_NEAR(path->bounds().bottom(), 150.0f, 1e-4f);

    auto tight = path->compute_tight_bounds();
    EXPECT_TRUE(tight.has_value());
    EXPECT_NEAR(tight->left(), 50.0f, 1.0f);
    EXPECT_NEAR(tight->right(), 150.0f, 1.0f);
}

TEST_CASE(test_path_transform) {
    PathBuilder pb;
    pb.move_to(0.0f, 0.0f);
    pb.line_to(10.0f, 0.0f);
    pb.line_to(10.0f, 10.0f);
    pb.close();
    auto path = pb.finish().value();

    auto ts = Transform::from_translate(5.0f, 10.0f);
    auto transformed = path.transform(ts);
    EXPECT_TRUE(transformed.has_value());
    EXPECT_NEAR(transformed->bounds().left(), 5.0f, 1e-5f);
    EXPECT_NEAR(transformed->bounds().top(), 10.0f, 1e-5f);
    EXPECT_NEAR(transformed->bounds().right(), 15.0f, 1e-5f);
    EXPECT_NEAR(transformed->bounds().bottom(), 20.0f, 1e-5f);
}

int main() {
    std::cout << "=== Running Nisaba Path & Geometry Unit Tests ===" << std::endl;
    RUN_TEST(test_cubic_eval);
    RUN_TEST(test_cubic_curvature);
    RUN_TEST(test_path_builder_rect);
    RUN_TEST(test_path_builder_circle);
    RUN_TEST(test_path_transform);

    std::cout << "\nTest Summary: " << g_tests_passed << " passed, " << g_tests_failed << " failed." << std::endl;
    return g_tests_failed == 0 ? 0 : 1;
}

#include <iostream>
#include <cmath>
#include <cassert>
#include "nisaba/nisaba.hpp"
#include "nisaba/math/border_radius.hpp"
#include "nisaba/math/rrect.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"

#ifdef NISABA_HAS_GPU
#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/gpu/gpu_surface.hpp"
#include "nisaba/gpu/gpu_canvas.hpp"
#endif

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
        std::cerr << "\n  FAILED: |" << #a << " - " << #b << "| <= " << #eps \
                  << " (got " << (a) << " vs " << (b) << ", diff " << std::abs((a) - (b)) << ") at " \
                  << __FILE__ << ":" << __LINE__ << std::endl; \
        g_tests_failed++; \
    } \
} while(0)

// -----------------------------------------------------------------------------
// Test 1: BorderRadius Constructors, Factories, and Predicates
// -----------------------------------------------------------------------------
TEST_CASE(test_border_radius_constructors_and_properties) {
    BorderRadius b_all = BorderRadius::all(16.0f);
    EXPECT_TRUE(b_all.is_uniform());
    EXPECT_FALSE(b_all.is_zero());
    EXPECT_NEAR(b_all.top_left, 16.0f, 1e-4f);
    EXPECT_NEAR(b_all.top_right, 16.0f, 1e-4f);
    EXPECT_NEAR(b_all.bottom_right, 16.0f, 1e-4f);
    EXPECT_NEAR(b_all.bottom_left, 16.0f, 1e-4f);

    BorderRadius b_vert = BorderRadius::vertical(10.0f, 20.0f);
    EXPECT_FALSE(b_vert.is_uniform());
    EXPECT_NEAR(b_vert.top_left, 10.0f, 1e-4f);
    EXPECT_NEAR(b_vert.top_right, 10.0f, 1e-4f);
    EXPECT_NEAR(b_vert.bottom_right, 20.0f, 1e-4f);
    EXPECT_NEAR(b_vert.bottom_left, 20.0f, 1e-4f);

    BorderRadius b_horiz = BorderRadius::horizontal(8.0f, 18.0f);
    EXPECT_FALSE(b_horiz.is_uniform());
    EXPECT_NEAR(b_horiz.top_left, 8.0f, 1e-4f);
    EXPECT_NEAR(b_horiz.top_right, 18.0f, 1e-4f);
    EXPECT_NEAR(b_horiz.bottom_right, 18.0f, 1e-4f);
    EXPECT_NEAR(b_horiz.bottom_left, 8.0f, 1e-4f);

    BorderRadius b_only = BorderRadius::only(5.0f, 10.0f, 15.0f, 20.0f);
    EXPECT_FALSE(b_only.is_uniform());
    EXPECT_FALSE(b_only.is_zero());
    EXPECT_NEAR(b_only.top_left, 5.0f, 1e-4f);
    EXPECT_NEAR(b_only.top_right, 10.0f, 1e-4f);
    EXPECT_NEAR(b_only.bottom_right, 15.0f, 1e-4f);
    EXPECT_NEAR(b_only.bottom_left, 20.0f, 1e-4f);

    BorderRadius b_scaled = b_only.scaled_by(2.0f);
    EXPECT_NEAR(b_scaled.top_left, 10.0f, 1e-4f);
    EXPECT_NEAR(b_scaled.top_right, 20.0f, 1e-4f);
    EXPECT_NEAR(b_scaled.bottom_right, 30.0f, 1e-4f);
    EXPECT_NEAR(b_scaled.bottom_left, 40.0f, 1e-4f);

    BorderRadius b_zero;
    EXPECT_TRUE(b_zero.is_zero());
    EXPECT_TRUE(b_zero.is_uniform());
}

// -----------------------------------------------------------------------------
// Test 2: W3C Corner Constraint Scaling Algorithm
// -----------------------------------------------------------------------------
TEST_CASE(test_w3c_corner_scaling_constraint) {
    // Case A: Radii comfortably fit within rectangle (no scaling needed)
    BorderRadius b1 = BorderRadius::only(10.0f, 20.0f, 15.0f, 25.0f);
    BorderRadius c1 = b1.constrained_for_size(100.0f, 80.0f);
    EXPECT_NEAR(c1.top_left, 10.0f, 1e-4f);
    EXPECT_NEAR(c1.top_right, 20.0f, 1e-4f);
    EXPECT_NEAR(c1.bottom_right, 15.0f, 1e-4f);
    EXPECT_NEAR(c1.bottom_left, 25.0f, 1e-4f);

    // Case B: Top adjacent radii exceed width (TL=80 + TR=80 = 160 > 100)
    // Scale factor should be 100 / 160 = 0.625
    BorderRadius b2(80.0f, 80.0f, 10.0f, 10.0f);
    BorderRadius c2 = b2.constrained_for_size(100.0f, 200.0f);
    EXPECT_NEAR(c2.top_left, 50.0f, 1e-4f);
    EXPECT_NEAR(c2.top_right, 50.0f, 1e-4f);
    EXPECT_NEAR(c2.bottom_right, 6.25f, 1e-4f);
    EXPECT_NEAR(c2.bottom_left, 6.25f, 1e-4f);
    // Adjacent sums must not exceed bounds
    EXPECT_TRUE(c2.top_left + c2.top_right <= 100.001f);

    // Case C: Vertical adjacent radii exceed height (TL=60 + BL=60 = 120 > 60)
    // Scale factor should be 60 / 120 = 0.5
    BorderRadius b3(60.0f, 10.0f, 10.0f, 60.0f);
    BorderRadius c3 = b3.constrained_for_size(200.0f, 60.0f);
    EXPECT_NEAR(c3.top_left, 30.0f, 1e-4f);
    EXPECT_NEAR(c3.top_right, 5.0f, 1e-4f);
    EXPECT_NEAR(c3.bottom_right, 5.0f, 1e-4f);
    EXPECT_NEAR(c3.bottom_left, 30.0f, 1e-4f);
    EXPECT_TRUE(c3.top_left + c3.bottom_left <= 60.001f);

    // Case D: Zero or negative dimensions
    BorderRadius c_zero = b1.constrained_for_size(0.0f, 50.0f);
    EXPECT_TRUE(c_zero.is_zero());
}

// -----------------------------------------------------------------------------
// Test 3: RRect Creation, Queries, and Properties
// -----------------------------------------------------------------------------
TEST_CASE(test_rrect_creation_and_bounds) {
    auto r_opt = Rect::from_xywh(10.0f, 20.0f, 120.0f, 80.0f);
    EXPECT_TRUE(r_opt.has_value());
    Rect r = *r_opt;

    BorderRadius rad(12.0f, 16.0f, 20.0f, 24.0f);
    RRect rr(r, rad);

    EXPECT_NEAR(rr.x(), 10.0f, 1e-4f);
    EXPECT_NEAR(rr.y(), 20.0f, 1e-4f);
    EXPECT_NEAR(rr.width(), 120.0f, 1e-4f);
    EXPECT_NEAR(rr.height(), 80.0f, 1e-4f);
    EXPECT_NEAR(rr.left(), 10.0f, 1e-4f);
    EXPECT_NEAR(rr.top(), 20.0f, 1e-4f);
    EXPECT_NEAR(rr.right(), 130.0f, 1e-4f);
    EXPECT_NEAR(rr.bottom(), 100.0f, 1e-4f);
    EXPECT_FALSE(rr.is_empty());

    auto rr_xywh = RRect::from_xywh(10.0f, 20.0f, 120.0f, 80.0f, rad);
    EXPECT_TRUE(rr_xywh.has_value());
    EXPECT_EQ(rr, *rr_xywh);

    RRect rr_uniform = RRect::from_rect_and_radius(r, 14.0f);
    EXPECT_TRUE(rr_uniform.radii.is_uniform());
    EXPECT_NEAR(rr_uniform.radii.top_left, 14.0f, 1e-4f);
}

// -----------------------------------------------------------------------------
// Test 4: Precise Analytical Point Containment (contains)
// -----------------------------------------------------------------------------
TEST_CASE(test_rrect_contains) {
    // 100x100 rectangle at origin (0, 0)
    // Only top-left is rounded with radius 30. Other 3 corners are sharp (0).
    auto r = *Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f);
    RRect rr(r, BorderRadius::only(30.0f, 0.0f, 0.0f, 0.0f));

    // Center point: definitely inside
    EXPECT_TRUE(rr.contains(Point::from_xy(50.0f, 50.0f)));
    EXPECT_TRUE(rr.contains(50.0f, 50.0f));

    // Points outside rectangle bounding box
    EXPECT_FALSE(rr.contains(-1.0f, 50.0f));
    EXPECT_FALSE(rr.contains(101.0f, 50.0f));
    EXPECT_FALSE(rr.contains(50.0f, -1.0f));
    EXPECT_FALSE(rr.contains(50.0f, 101.0f));

    // Sharp corners: points close to corners are INSIDE
    EXPECT_TRUE(rr.contains(99.0f, 1.0f));   // Near sharp Top-Right
    EXPECT_TRUE(rr.contains(99.0f, 99.0f));  // Near sharp Bottom-Right
    EXPECT_TRUE(rr.contains(1.0f, 99.0f));   // Near sharp Bottom-Left

    // Top-Left rounded corner arc center is at (30, 30), radius = 30
    // Test point (2, 2): distance to (30, 30) is sqrt(28^2 + 28^2) ≈ 39.6 > 30 -> OUTSIDE
    EXPECT_FALSE(rr.contains(2.0f, 2.0f));

    // Test point (5, 5): distance is sqrt(25^2 + 25^2) ≈ 35.35 > 30 -> OUTSIDE
    EXPECT_FALSE(rr.contains(5.0f, 5.0f));

    // Test point (20, 20): distance is sqrt(10^2 + 10^2) ≈ 14.14 < 30 -> INSIDE
    EXPECT_TRUE(rr.contains(20.0f, 20.0f));

    // Test point (29, 29): distance is sqrt(1^2 + 1^2) ≈ 1.41 < 30 -> INSIDE
    EXPECT_TRUE(rr.contains(29.0f, 29.0f));
}

// -----------------------------------------------------------------------------
// Test 5: Deflate and Inset Transformations
// -----------------------------------------------------------------------------
TEST_CASE(test_rrect_deflate_and_inset) {
    auto r = *Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f);
    RRect rr(r, BorderRadius::all(20.0f));

    RRect def = rr.deflate(10.0f, 10.0f);
    EXPECT_NEAR(def.left(), 10.0f, 1e-4f);
    EXPECT_NEAR(def.top(), 10.0f, 1e-4f);
    EXPECT_NEAR(def.width(), 80.0f, 1e-4f);
    EXPECT_NEAR(def.height(), 80.0f, 1e-4f);
    // Radius should scale proportionally with dimension: (100 - 20) / 100 = 0.8 -> 20 * 0.8 = 16
    EXPECT_NEAR(def.radii.top_left, 16.0f, 1e-4f);

    RRect ins = rr.inset(5.0f, 10.0f, 15.0f, 20.0f);
    EXPECT_NEAR(ins.left(), 5.0f, 1e-4f);
    EXPECT_NEAR(ins.top(), 10.0f, 1e-4f);
    EXPECT_NEAR(ins.right(), 85.0f, 1e-4f);
    EXPECT_NEAR(ins.bottom(), 80.0f, 1e-4f);
}

// -----------------------------------------------------------------------------
// Test 6: Asymmetric Vector Path Generation via PathBuilder
// -----------------------------------------------------------------------------
TEST_CASE(test_rrect_to_path_asymmetric) {
    auto r = *Rect::from_xywh(10.0f, 10.0f, 200.0f, 100.0f);
    // Asymmetric: TL=20, TR=0, BR=30, BL=10
    BorderRadius radii(20.0f, 0.0f, 30.0f, 10.0f);
    RRect rr(r, radii);

    Path p = rr.to_path();
    EXPECT_FALSE(p.is_empty());
    EXPECT_TRUE(p.verbs().size() > 5);

    // First verb must be Move
    EXPECT_TRUE(p.verbs().front() == PathVerb::Move);
    // Last verb must be Close
    EXPECT_TRUE(p.verbs().back() == PathVerb::Close);

    // Verify bounds of generated path match the rectangle
    Rect b = p.bounds();
    EXPECT_NEAR(b.left(), 10.0f, 0.5f);
    EXPECT_NEAR(b.top(), 10.0f, 0.5f);
    EXPECT_NEAR(b.right(), 210.0f, 0.5f);
    EXPECT_NEAR(b.bottom(), 110.0f, 0.5f);

    // Count cubic curves: corners with radius > 0 should have cubic segments
    int cubic_count = 0;
    for (auto v : p.verbs()) {
        if (v == PathVerb::Cubic) cubic_count++;
    }
    // Three corners have radius > 0 (TL=20, BR=30, BL=10), so exactly 3 cubics
    EXPECT_EQ(cubic_count, 3);
}

// -----------------------------------------------------------------------------
// Test 7: Zero Radius Degenerates Gracefully to Standard Rect Path
// -----------------------------------------------------------------------------
TEST_CASE(test_rrect_zero_radius) {
    auto r = *Rect::from_xywh(0.0f, 0.0f, 50.0f, 50.0f);
    RRect rr(r, BorderRadius::all(0.0f));

    Path p = rr.to_path();
    EXPECT_FALSE(p.is_empty());

    int cubic_count = 0;
    for (auto v : p.verbs()) {
        if (v == PathVerb::Cubic) cubic_count++;
    }
    // Zero rounded corners -> 0 cubics, pure lines
    EXPECT_EQ(cubic_count, 0);

    Rect b = p.bounds();
    EXPECT_NEAR(b.width(), 50.0f, 1e-4f);
    EXPECT_NEAR(b.height(), 50.0f, 1e-4f);
}

// -----------------------------------------------------------------------------
// Test 8: Hardware GPU Rendering with fill_rrect and clip_rrect
// -----------------------------------------------------------------------------
TEST_CASE(test_rrect_gpu_rendering) {
#ifdef NISABA_HAS_GPU
    auto device = GpuDevice::create();
    if (!device || !device->is_valid()) {
        std::cout << " (GPU Device unavailable, skipping hardware pass) ";
        return;
    }

    constexpr int W = 200;
    constexpr int H = 200;
    auto surface = GpuSurface::create(device, W, H);
    if (!surface) {
        std::cout << " (GpuSurface unavailable, skipping hardware pass) ";
        return;
    }

    GpuCanvas canvas(surface);
    canvas.clear_rect(*Rect::from_xywh(0.0f, 0.0f, W, H), Color::BLACK);

    // 1. Draw Asymmetric RRect Fill (Analytical fast-path ShaderRRect)
    auto r_box = *Rect::from_xywh(20.0f, 20.0f, 160.0f, 120.0f);
    BorderRadius rad(30.0f, 10.0f, 25.0f, 0.0f);
    canvas.fill_rrect(r_box, rad, Paint(Color::from_rgba8(0, 229, 255, 255)));

    // 2. Stroke RRect
    canvas.stroke_rrect(r_box, rad, Paint(Color::from_rgba8(255, 215, 0, 255)), Stroke(2.0f));

    // 3. Stencil Clip RRect
    canvas.save();
    auto clip_box = *Rect::from_xywh(40.0f, 40.0f, 120.0f, 80.0f);
    canvas.clip_rrect(RRect(clip_box, BorderRadius::all(15.0f)));
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, W, H), Paint(Color::from_rgba8(255, 0, 128, 255)));
    canvas.restore();

    canvas.flush();

    // Verify rendered pixels
    auto pixmap = surface->to_pixmap();
    EXPECT_TRUE(pixmap != nullptr);
    if (pixmap) {
        // Sample center of clipped fill: should be magenta
        Color center_col = pixmap->get_pixel(100, 80);
        EXPECT_TRUE(center_col.alpha() > 0.0f);

        // Sample outside initial RRect: should be clear/black
        Color outside_col = pixmap->get_pixel(5, 5);
        EXPECT_NEAR(outside_col.alpha(), 0.0f, 0.05f);
    }
#endif
}

// -----------------------------------------------------------------------------
// Main Test Runner
// -----------------------------------------------------------------------------
int main() {
    std::cout << "========================================================\n"
              << "Nisaba Sovereign Engine — RRect & BorderRadius Unit Tests\n"
              << "========================================================\n";

    RUN_TEST(test_border_radius_constructors_and_properties);
    RUN_TEST(test_w3c_corner_scaling_constraint);
    RUN_TEST(test_rrect_creation_and_bounds);
    RUN_TEST(test_rrect_contains);
    RUN_TEST(test_rrect_deflate_and_inset);
    RUN_TEST(test_rrect_to_path_asymmetric);
    RUN_TEST(test_rrect_zero_radius);
    RUN_TEST(test_rrect_gpu_rendering);

    std::cout << "========================================================\n";
    std::cout << "Tests Summary: " << g_tests_passed << " Passed, "
              << g_tests_failed << " Failed.\n";
    std::cout << "========================================================\n";

    return (g_tests_failed == 0) ? 0 : 1;
}

#include <iostream>
#include <cassert>
#include "nisaba/nisaba.hpp"

using namespace nisaba;

void test_line_clipper() {
    Rect clip = *Rect::from_ltrb(10.0f, 10.0f, 50.0f, 50.0f);
    std::array<Point, 2> line = {Point(0.0f, 25.0f), Point(60.0f, 25.0f)};
    std::array<Point, line_clipper::MAX_POINTS> clipped{};

    size_t count = line_clipper::clip(line, clip, false, clipped);
    assert(count >= 2);
    assert(clipped[0].x >= 10.0f && clipped[0].x <= 50.0f);

    std::cout << "[PASS] Line clipper tests passed\n";
}

void test_alpha_runs() {
    LengthU32 width = LengthU32::create_unchecked(100);
    AlphaRuns runs(width);
    assert(runs.is_empty());

    // Add a run: x=10, start=128, middle=20 (coverage=255), stop=64
    runs.add(10, 128, 20, 64, 255, 0);
    assert(!runs.is_empty());

    std::cout << "[PASS] AlphaRuns tests passed\n";
}

void test_raster_triangle_and_circle() {
    constexpr uint32_t W = 64;
    constexpr uint32_t H = 64;
    auto pm = Pixmap::allocate(W, H);
    assert(pm.has_value());

    // 1. Draw a filled green triangle with Anti-Aliasing
    PathBuilder pb;
    pb.move_to(32.0f, 10.0f);
    pb.line_to(54.0f, 50.0f);
    pb.line_to(10.0f, 50.0f);
    pb.close();
    auto triangle_path = pb.finish();
    assert(triangle_path.has_value());

    auto clip_rect = to_screen_int_rect(pm->size(), 0, 0);
    auto mut_view = pm->as_mut();
    SolidColorBlitter blitter(mut_view, Color::from_rgba8(0, 255, 0, 255)); // solid green

    scan::fill_path(*triangle_path, FillRule::Winding, clip_rect, true, blitter);

    // Center of triangle (around 32, 35) must be green
    auto center_px = pm->pixel(32, 35);
    assert(center_px.has_value());
    assert(center_px->green() == 255);
    assert(center_px->alpha() == 255);

    // Corner (0, 0) outside triangle must remain transparent
    auto corner_px = pm->pixel(0, 0);
    assert(corner_px.has_value());
    assert(corner_px->alpha() == 0);

    // Save triangle
    pm->save_bmp("test_triangle_aa.bmp");

    // 2. Clear and draw stroked circle with AA
    pm->fill(Color::TRANSPARENT);
    auto circle_path = PathBuilder::from_circle(32.0f, 32.0f, 20.0f);
    assert(circle_path.has_value());

    Stroke stroke;
    stroke.width = 4.0f;
    auto stroked_circle = stroke_path(*circle_path, stroke);
    assert(stroked_circle.has_value());

    SolidColorBlitter red_blitter(mut_view, Color::from_rgba8(255, 0, 0, 255)); // solid red
    scan::fill_path(*stroked_circle, FillRule::Winding, clip_rect, true, red_blitter);

    // On the circle ring (e.g. at 32, 12 which is 32 - 20) must be red
    auto ring_px = pm->pixel(32, 12);
    assert(ring_px.has_value());
    assert(ring_px->red() > 200); // anti-aliased red
    assert(ring_px->alpha() > 200);

    // Save stroked circle
    pm->save_bmp("test_stroked_circle_aa.bmp");

    std::cout << "[PASS] Path rasterization (triangle + stroked circle) passed\n";
}

int main() {
    std::cout << "Running Nisaba Scanline Rasterizer tests...\n";
    test_line_clipper();
    test_alpha_runs();
    test_raster_triangle_and_circle();
    std::cout << "All Scanline Rasterizer tests passed successfully!\n";
    return 0;
}

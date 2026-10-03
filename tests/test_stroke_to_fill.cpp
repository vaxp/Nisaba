#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/stroker.hpp"
#include "nisaba/path/dash.hpp"
#include "nisaba/path/path_ops.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/paint.hpp"

using namespace nisaba;

void test_straight_line_outline() {
    std::cout << "[TEST] Straight line stroke_to_fill (Butt & Square caps)... ";

    PathBuilder pb;
    pb.move_to(10.0f, 50.0f);
    pb.line_to(90.0f, 50.0f);
    auto line_path = pb.finish();
    assert(line_path.has_value());

    // 1. Butt cap, width 10 -> rectangle [10, 45, 90, 55]
    Stroke s_butt;
    s_butt.width = 10.0f;
    s_butt.line_cap = LineCap::Butt;

    auto filled_butt = line_path->stroke_to_fill(s_butt);
    assert(filled_butt.has_value());
    assert(!filled_butt->is_empty());
    auto b_butt = filled_butt->compute_tight_bounds();
    assert(b_butt.has_value());
    assert(std::abs(b_butt->left() - 10.0f) < 0.5f);
    assert(std::abs(b_butt->right() - 90.0f) < 0.5f);
    assert(std::abs(b_butt->top() - 45.0f) < 0.5f);
    assert(std::abs(b_butt->bottom() - 55.0f) < 0.5f);

    // 2. Square cap, width 10 -> expands by 5 on ends -> [5, 45, 95, 55]
    Stroke s_square;
    s_square.width = 10.0f;
    s_square.line_cap = LineCap::Square;

    auto filled_square = line_path->stroke_to_fill(s_square);
    assert(filled_square.has_value());
    auto b_sq = filled_square->compute_tight_bounds();
    assert(b_sq.has_value());
    assert(std::abs(b_sq->left() - 5.0f) < 0.5f);
    assert(std::abs(b_sq->right() - 95.0f) < 0.5f);

    std::cout << "PASSED\n";
}

void test_corner_joins_outline() {
    std::cout << "[TEST] Corner joins (Miter, Bevel, Round) stroke_to_fill... ";

    PathBuilder pb;
    pb.move_to(10.0f, 10.0f);
    pb.line_to(50.0f, 10.0f);
    pb.line_to(50.0f, 50.0f);
    auto corner = pb.finish();
    assert(corner.has_value());

    // Miter join
    Stroke s_miter;
    s_miter.width = 8.0f;
    s_miter.line_join = LineJoin::Miter;
    auto filled_miter = corner->stroke_to_fill(s_miter);
    assert(filled_miter.has_value());
    assert(filled_miter->len() >= 6);

    // Bevel join
    Stroke s_bevel;
    s_bevel.width = 8.0f;
    s_bevel.line_join = LineJoin::Bevel;
    auto filled_bevel = corner->stroke_to_fill(s_bevel);
    assert(filled_bevel.has_value());

    // Round join
    Stroke s_round;
    s_round.width = 8.0f;
    s_round.line_join = LineJoin::Round;
    auto filled_round = corner->stroke_to_fill(s_round);
    assert(filled_round.has_value());

    std::cout << "PASSED\n";
}

void test_dashed_stroke_to_fill() {
    std::cout << "[TEST] Dashed stroke to fill multi-contour generation... ";

    PathBuilder pb;
    pb.move_to(0.0f, 50.0f);
    pb.line_to(100.0f, 50.0f);
    auto line = pb.finish();
    assert(line.has_value());

    std::vector<float> intervals = {10.0f, 10.0f}; // 10 on, 10 off
    auto dash_opt = StrokeDash::create(intervals, 0.0f);
    assert(dash_opt.has_value());

    Stroke s_dash;
    s_dash.width = 6.0f;
    s_dash.dash = *dash_opt;

    auto dashed_fill = line->stroke_to_fill(s_dash);
    assert(dashed_fill.has_value());

    // Count Move verbs to verify multiple dash segments generated
    size_t move_count = 0;
    for (auto v : dashed_fill->verbs()) {
        if (v == PathVerb::Move) ++move_count;
    }
    assert(move_count >= 4 && "Dashed stroke must produce multiple closed contours!");

    std::cout << "PASSED (Generated " << move_count << " dash contours)\n";
}

void test_pathops_on_stroked_path() {
    std::cout << "[TEST] Boolean operations (PathOps) on stroked outline path... ";

    // Path 1: Stroked diagonal line with width 20
    PathBuilder pb1;
    pb1.move_to(10.0f, 10.0f);
    pb1.line_to(90.0f, 90.0f);
    auto diag = pb1.finish();
    assert(diag.has_value());

    Stroke s;
    s.width = 20.0f;
    auto diag_outlined = diag->stroke_to_fill(s);
    assert(diag_outlined.has_value());

    // Path 2: Solid square in the middle [30, 30, 70, 70]
    PathBuilder pb2;
    pb2.move_to(30.0f, 30.0f);
    pb2.line_to(70.0f, 30.0f);
    pb2.line_to(70.0f, 70.0f);
    pb2.line_to(30.0f, 70.0f);
    pb2.close();
    auto square = pb2.finish();
    assert(square.has_value());

    // Intersect stroked line outline with square
    auto intersected = diag_outlined->intersect(*square);
    assert(intersected.has_value());
    assert(!intersected->is_empty());

    // Tight bounds of intersection must be contained within [30, 30, 70, 70]
    auto bounds = intersected->compute_tight_bounds();
    assert(bounds.has_value());
    assert(bounds->left() >= 29.5f && bounds->right() <= 70.5f);
    assert(bounds->top() >= 29.5f && bounds->bottom() <= 70.5f);

    // Difference: subtract square from stroked line
    auto diff = diag_outlined->difference(*square);
    assert(diff.has_value());
    assert(!diff->is_empty());

    std::cout << "PASSED\n";
}

void test_render_equivalence() {
    std::cout << "[TEST] Visual raster equivalence between stroke_path and fill(stroke_to_fill)... ";

    auto pm1 = Pixmap::allocate(100, 100);
    auto pm2 = Pixmap::allocate(100, 100);
    assert(pm1.has_value() && pm2.has_value());

    Canvas c1(*pm1);
    Canvas c2(*pm2);
    c1.clear(Color::WHITE);
    c2.clear(Color::WHITE);

    PathBuilder pb;
    pb.move_to(20.0f, 20.0f);
    pb.quad_to(80.0f, 20.0f, 80.0f, 80.0f);
    auto path = pb.finish();
    assert(path.has_value());

    Stroke stroke;
    stroke.width = 12.0f;
    stroke.line_cap = LineCap::Round;
    stroke.line_join = LineJoin::Round;

    Paint paint(Color::from_rgba8(255, 0, 0, 255));
    paint.anti_alias = true;

    // Method 1: direct stroke
    c1.stroke_path(*path, paint, stroke);

    // Method 2: stroke_to_fill then fill
    auto outlined = path->stroke_to_fill(stroke);
    assert(outlined.has_value());
    c2.fill_path(*outlined, paint);

    // Verify both produced painted pixels with very close match
    size_t p1_count = 0;
    size_t p2_count = 0;
    for (uint32_t y = 0; y < 100; ++y) {
        for (uint32_t x = 0; x < 100; ++x) {
            if (pm1->pixel(x, y)->red() > 200 && pm1->pixel(x, y)->green() < 50) ++p1_count;
            if (pm2->pixel(x, y)->red() > 200 && pm2->pixel(x, y)->green() < 50) ++p2_count;
        }
    }
    assert(p1_count > 200);
    assert(p2_count > 200);
    // Differences between direct stroker and fill_path(outlined) should be negligible (< 5%)
    assert(std::abs(static_cast<int>(p1_count) - static_cast<int>(p2_count)) < 50);

    std::cout << "PASSED (p1: " << p1_count << " px, p2: " << p2_count << " px)\n";
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Nisaba Stroke-to-Fill (Outlining) Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    test_straight_line_outline();
    test_corner_joins_outline();
    test_dashed_stroke_to_fill();
    test_pathops_on_stroked_path();
    test_render_equivalence();

    std::cout << "========================================" << std::endl;
    std::cout << " All Stroke-to-Fill Tests PASSED!       " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}

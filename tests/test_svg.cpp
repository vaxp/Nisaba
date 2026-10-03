#include <cassert>
#include <iostream>
#include <cmath>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::svg;

void test_path_parser_basic() {
    std::cout << "[Test] SVG PathParser basic commands (M, L, H, V, Z)..." << std::endl;

    // Absolute
    auto p1 = PathParser::parse("M 10 20 L 50 20 L 50 60 L 10 60 Z");
    assert(p1.has_value());
    assert(!p1->is_empty());
    auto b1 = p1->bounds();
    assert(std::abs(b1.left() - 10.0f) < 1e-4f);
    assert(std::abs(b1.right() - 50.0f) < 1e-4f);
    assert(std::abs(b1.top() - 20.0f) < 1e-4f);
    assert(std::abs(b1.bottom() - 60.0f) < 1e-4f);

    // Horizontal and vertical
    auto p2 = PathParser::parse("M 0 0 H 100 V 50 H 0 Z");
    assert(p2.has_value());
    auto b2 = p2->bounds();
    assert(std::abs(b2.width() - 100.0f) < 1e-4f);
    assert(std::abs(b2.height() - 50.0f) < 1e-4f);

    // Relative commands
    auto p3 = PathParser::parse("m 10 10 h 20 v 20 h -20 z");
    assert(p3.has_value());
    auto b3 = p3->bounds();
    assert(std::abs(b3.left() - 10.0f) < 1e-4f);
    assert(std::abs(b3.right() - 30.0f) < 1e-4f);

    // Implicit lineTo
    auto p4 = PathParser::parse("M 0 0 10 10 20 20 30 10 Z");
    assert(p4.has_value());
    assert(p4->verbs().size() == 5); // Move, Line, Line, Line, Close

    std::cout << "  Basic path parser verified." << std::endl;
}

void test_path_parser_curves() {
    std::cout << "[Test] SVG PathParser curves (C, S, Q, T)..." << std::endl;

    // Cubic and smooth cubic
    auto p_cubic = PathParser::parse("M 0 0 C 10 20 30 20 40 0 S 70 -20 80 0");
    assert(p_cubic.has_value());
    assert(!p_cubic->is_empty());

    // Quadratic and smooth quad
    auto p_quad = PathParser::parse("M 0 0 Q 20 40 40 0 T 80 0");
    assert(p_quad.has_value());
    assert(!p_quad->is_empty());

    std::cout << "  Bézier curves verified." << std::endl;
}

void test_path_parser_arcs() {
    std::cout << "[Test] SVG PathParser elliptical arc (A/a)..." << std::endl;

    // Semi-circle arc
    auto p_arc = PathParser::parse("M 10 50 A 40 40 0 0 1 90 50");
    assert(p_arc.has_value());
    assert(!p_arc->is_empty());

    auto b = p_arc->bounds();
    assert(b.left() >= 9.0f && b.right() <= 91.0f);

    std::cout << "  Elliptical arc verified." << std::endl;
}

void test_svg_document() {
    std::cout << "[Test] SvgDocument parsing and elements..." << std::endl;

    const std::string xml = R"(
        <svg viewBox="0 0 200 200" width="200" height="200">
            <!-- Background rectangle -->
            <rect x="10" y="10" width="180" height="180" rx="15" fill="#1e2433" stroke="#00e5ff" stroke-width="2"/>
            <!-- Sensor Circle -->
            <circle cx="100" cy="100" r="40" fill="#ff0055" opacity="0.8"/>
            <!-- Custom Path -->
            <path d="M 80 100 L 100 70 L 120 100 Z" fill="#ffffff"/>
            <!-- Line -->
            <line x1="20" y1="190" x2="180" y2="190" stroke="#ffffff" stroke-width="1"/>
        </svg>
    )";

    auto doc = SvgDocument::parse(xml);
    assert(doc.has_value());
    assert(doc->view_box().width() == 200.0f);
    assert(doc->view_box().height() == 200.0f);
    assert(doc->elements().size() == 4); // rect, circle, path, line

    // Verify first element (rect with rounded corners)
    const auto& elem0 = doc->elements()[0];
    assert(elem0.style.fill.has_value());
    assert(elem0.style.stroke.has_value());
    assert(elem0.style.stroke_width == 2.0f);

    std::cout << "  SvgDocument parsed " << doc->elements().size() << " elements successfully." << std::endl;
}

void test_svg_rendering() {
    std::cout << "[Test] SvgDocument Canvas rendering..." << std::endl;

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::BLACK);

    const std::string svg_code = R"(
        <svg viewBox="0 0 50 50">
            <rect x="10" y="10" width="30" height="30" fill="#00ff00"/>
        </svg>
    )";

    auto doc = SvgDocument::parse(svg_code);
    assert(doc.has_value());

    auto target = Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f);
    assert(target.has_value());
    canvas.draw_svg(*doc, *target);

    // The center at (50, 50) must be Green (#00ff00)
    auto p_center = pix->as_ref().pixel(50, 50);
    assert(p_center.has_value());
    assert(p_center->green() == 255);
    assert(p_center->red() == 0);
    assert(p_center->blue() == 0);

    // The corner (5, 5) should remain background Black
    auto p_corner = pix->as_ref().pixel(5, 5);
    assert(p_corner.has_value());
    assert(p_corner->green() == 0);

    std::cout << "  SvgDocument rendered successfully to Canvas." << std::endl;
}

int main() {
    std::cout << "=== Running Nisaba Sovereign SVG Renderer Unit Tests ===" << std::endl;
    test_path_parser_basic();
    test_path_parser_curves();
    test_path_parser_arcs();
    test_svg_document();
    test_svg_rendering();
    std::cout << "All SVG unit tests passed successfully!" << std::endl;
    return 0;
}

#include <cassert>
#include <iostream>
#include <cmath>
#include <string>
#include "nisaba/nisaba.hpp"
#include "nisaba/svg/svg_document.hpp"
#include "nisaba/image/png.hpp"

using namespace nisaba;
using namespace nisaba::svg;

namespace {

std::string base64_encode(std::span<const uint8_t> data) {
    static const char tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);
    size_t i = 0;
    while (i < data.size()) {
        size_t rem = data.size() - i;
        uint32_t b0 = data[i++];
        uint32_t b1 = (rem > 1) ? data[i++] : 0;
        uint32_t b2 = (rem > 2) ? data[i++] : 0;
        uint32_t triple = (b0 << 16) | (b1 << 8) | b2;

        out.push_back(tbl[(triple >> 18) & 0x3F]);
        out.push_back(tbl[(triple >> 12) & 0x3F]);
        out.push_back((rem > 1) ? tbl[(triple >> 6) & 0x3F] : '=');
        out.push_back((rem > 2) ? tbl[triple & 0x3F] : '=');
    }
    return out;
}

} // namespace

void test_svg_linear_gradient() {
    std::cout << "[Test] SVG Linear Gradient (fill=\"url(#id)\")..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <linearGradient id="gradLinear" x1="0%" y1="0%" x2="100%" y2="0%">
                    <stop offset="0%" stop-color="#ff0000" />
                    <stop offset="100%" stop-color="#0000ff" />
                </linearGradient>
            </defs>
            <rect x="0" y="0" width="100" height="100" fill="url(#gradLinear)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->gradients().count("gradLinear") == 1);
    const auto& grad = doc->gradients().at("gradLinear");
    assert(grad.kind == SvgGradient::Kind::Linear);
    assert(grad.stops.size() == 2);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    // Sample left edge: predominantly red
    auto p_left = pix->as_ref().pixel(5, 50);
    assert(p_left.has_value());
    assert(p_left->red() > 200);
    assert(p_left->blue() < 50);

    // Sample right edge: predominantly blue
    auto p_right = pix->as_ref().pixel(95, 50);
    assert(p_right.has_value());
    assert(p_right->blue() > 200);
    assert(p_right->red() < 50);

    // Sample center: mixture of red and blue (purple)
    auto p_center = pix->as_ref().pixel(50, 50);
    assert(p_center.has_value());
    assert(p_center->red() > 80 && p_center->blue() > 80);

    std::cout << "  SVG Linear Gradient rendered accurately." << std::endl;
}

void test_svg_radial_gradient() {
    std::cout << "[Test] SVG Radial Gradient..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <radialGradient id="gradRadial" cx="50%" cy="50%" r="50%">
                    <stop offset="0%" stop-color="#ffff00" />
                    <stop offset="100%" stop-color="#000000" />
                </radialGradient>
            </defs>
            <circle cx="50" cy="50" r="50" fill="url(#gradRadial)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->gradients().count("gradRadial") == 1);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    // Center pixel (50, 50) must be Yellow (Red + Green, low Blue)
    auto p_center = pix->as_ref().pixel(50, 50);
    assert(p_center.has_value());
    assert(p_center->red() > 200);
    assert(p_center->green() > 200);
    assert(p_center->blue() < 40);

    // Periphery near edge (e.g. 50, 95) must be darker than center
    auto p_edge = pix->as_ref().pixel(50, 95);
    assert(p_edge.has_value());
    assert(p_edge->red() < 100);

    std::cout << "  SVG Radial Gradient rendered accurately." << std::endl;
}

void test_svg_gradient_inheritance() {
    std::cout << "[Test] SVG Gradient inheritance (href=\"#parent\")..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <linearGradient id="baseGrad">
                    <stop offset="0%" stop-color="#00ff00" />
                    <stop offset="100%" stop-color="#000000" />
                </linearGradient>
                <linearGradient id="childGrad" href="#baseGrad" x1="0%" y1="0%" x2="100%" y2="0%" />
            </defs>
            <rect x="0" y="0" width="100" height="100" fill="url(#childGrad)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->gradients().count("childGrad") == 1);
    const auto& child = doc->gradients().at("childGrad");
    assert(child.stops.size() == 2);
    assert(child.stops[0].color.green() > 0.9f);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    auto p_start = pix->as_ref().pixel(5, 50);
    assert(p_start.has_value());
    assert(p_start->green() > 200);

    std::cout << "  SVG Gradient inheritance verified." << std::endl;
}

void test_svg_defs_and_use() {
    std::cout << "[Test] SVG <defs> and <use> elements..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <rect id="sampleTile" x="0" y="0" width="20" height="20" fill="#00ff00" />
            </defs>
            <!-- (0, 0) should remain empty because sampleTile is in <defs> -->
            <use href="#sampleTile" x="30" y="30" />
            <use xlink:href="#sampleTile" x="70" y="70" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->defined_elements().count("sampleTile") == 1);
    assert(doc->elements().size() == 2); // Two instantiated <use> elements

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    // Defs element itself at (10, 10) must be transparent
    auto p_defs = pix->as_ref().pixel(10, 10);
    assert(p_defs.has_value() && p_defs->alpha() == 0);

    // First use at (30+10, 30+10) = (40, 40) must be green
    auto p_use1 = pix->as_ref().pixel(40, 40);
    assert(p_use1.has_value());
    assert(p_use1->green() == 255);
    assert(p_use1->red() == 0);

    // Second use at (70+10, 70+10) = (80, 80) must be green
    auto p_use2 = pix->as_ref().pixel(80, 80);
    assert(p_use2.has_value());
    assert(p_use2->green() == 255);

    std::cout << "  SVG <defs> and <use> verified." << std::endl;
}

void test_svg_clip_path() {
    std::cout << "[Test] SVG <clipPath> (clip-path=\"url(#id)\")..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <clipPath id="centerHole">
                    <circle cx="50" cy="50" r="25" />
                </clipPath>
            </defs>
            <rect x="0" y="0" width="100" height="100" fill="#00ffff" clip-path="url(#centerHole)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->clip_paths().count("centerHole") == 1);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    // Center (50, 50) is inside the circle clip: must be Cyan (#00ffff)
    auto p_center = pix->as_ref().pixel(50, 50);
    assert(p_center.has_value());
    assert(p_center->green() == 255 && p_center->blue() == 255);

    // Outside the circle (10, 10) must remain transparent
    auto p_outside = pix->as_ref().pixel(10, 10);
    assert(p_outside.has_value());
    assert(p_outside->alpha() == 0);

    std::cout << "  SVG <clipPath> verified." << std::endl;
}

void test_svg_mask() {
    std::cout << "[Test] SVG <mask> (mask=\"url(#id)\")..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <mask id="fadeHole">
                    <rect x="0" y="0" width="100" height="100" fill="#ffffff" />
                    <circle cx="50" cy="50" r="20" fill="#000000" />
                </mask>
            </defs>
            <rect x="0" y="0" width="100" height="100" fill="#ff00ff" mask="url(#fadeHole)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->masks().count("fadeHole") == 1);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    // Center (50, 50) has black in the mask -> masked out (alpha near 0)
    auto p_center = pix->as_ref().pixel(50, 50);
    assert(p_center.has_value());
    assert(p_center->alpha() == 0);

    // Outer corner (10, 10) has white in the mask -> solid Magenta (#ff00ff)
    auto p_corner = pix->as_ref().pixel(10, 10);
    assert(p_corner.has_value());
    assert(p_corner->red() == 255 && p_corner->blue() == 255);
    assert(p_corner->alpha() == 255);

    std::cout << "  SVG <mask> luminance masking verified." << std::endl;
}

void test_svg_image_data_uri() {
    std::cout << "[Test] SVG <image> with base64 Data URI..." << std::endl;

    // Create a 4x4 pure solid Orange image and encode to PNG
    auto src_pix = Pixmap::allocate(4, 4);
    assert(src_pix.has_value());
    src_pix->fill(Color::from_rgba8(255, 128, 0, 255));

    auto png_bytes = image::encode_png(src_pix->as_ref(), 1);
    assert(png_bytes.has_value());

    std::string b64 = base64_encode(*png_bytes);
    std::string data_uri = "data:image/png;base64," + b64;

    std::string svg_xml = "<svg viewBox=\"0 0 100 100\" width=\"100\" height=\"100\">\n";
    svg_xml += "  <image href=\"" + data_uri + "\" x=\"20\" y=\"20\" width=\"60\" height=\"60\" />\n";
    svg_xml += "</svg>";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->elements().size() == 1);
    const auto& elem = doc->elements()[0];
    assert(elem.type == SvgElementType::Image);
    assert(elem.image != nullptr);
    assert(elem.image->width() == 4 && elem.image->height() == 4);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);

    doc->render(canvas);

    // Sample inside image bounds (50, 50) -> must be Orange (255, 128, 0)
    auto p_inside = pix->as_ref().pixel(50, 50);
    assert(p_inside.has_value());
    assert(p_inside->red() == 255);
    assert(p_inside->green() >= 120 && p_inside->green() <= 136);
    assert(p_inside->blue() == 0);

    // Sample outside image bounds (10, 10) -> transparent
    auto p_outside = pix->as_ref().pixel(10, 10);
    assert(p_outside.has_value() && p_outside->alpha() == 0);

    std::cout << "  SVG <image> base64 Data URI decoding and rendering verified." << std::endl;
}

void test_svg_groups_and_cascaded_transforms() {
    std::cout << "[Test] SVG <g> groups and cascaded styles/transforms..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <g transform="translate(20, 20)" fill="#0000ff" opacity="0.5">
                <rect x="0" y="0" width="40" height="40" />
            </g>
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->elements().size() == 1);
    const auto& elem = doc->elements()[0];
    assert(elem.style.fill.has_value());
    assert(elem.style.fill->blue() > 0.9f);
    assert(elem.style.opacity == 0.5f);

    auto pix = Pixmap::allocate(100, 100);
    assert(pix.has_value());
    Canvas canvas(*pix);
    canvas.clear(Color::BLACK);

    doc->render(canvas);

    // Inside rect (30, 30) must be blended blue over black with alpha = 0.5
    auto p = pix->as_ref().pixel(30, 30);
    assert(p.has_value());
    assert(p->blue() > 100);

    std::cout << "  SVG <g> groups verified." << std::endl;
}

int main() {
    std::cout << "=== Running Nisaba Stage 3 SVG Completeness Unit Tests ===" << std::endl;
    test_svg_linear_gradient();
    test_svg_radial_gradient();
    test_svg_gradient_inheritance();
    test_svg_defs_and_use();
    test_svg_clip_path();
    test_svg_mask();
    test_svg_image_data_uri();
    test_svg_groups_and_cascaded_transforms();
    std::cout << "All Nisaba Stage 3 SVG Completeness tests passed successfully!" << std::endl;
    return 0;
}

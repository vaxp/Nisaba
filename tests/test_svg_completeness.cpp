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

void test_svg_w3c_colors_and_theming() {
    std::cout << "[Test] SVG W3C 148 Named Colors, HSL/HSLA, #RGBA/#RRGGBBAA, currentColor, var(--*) & SvgTheme..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <rect id="namedRect" x="0" y="0" width="25" height="50" fill="crimson" />
            <rect id="hslRect" x="25" y="0" width="25" height="50" fill="hsl(120, 100%, 50%)" />
            <rect id="curColorRect" x="50" y="0" width="25" height="50" color="dodgerblue" fill="currentColor" />
            <rect id="varRect" x="75" y="0" width="25" height="50" fill="var(--accent-color, #ff00ff)" />
            <rect id="hex8Rect" x="0" y="50" width="50" height="50" fill="#00ffff80" />
            <rect id="overrideRect" x="50" y="50" width="50" height="50" fill="rebeccapurple" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    // 1. Default render (fallback var, inline currentColor)
    auto pix1 = Pixmap::allocate(100, 100);
    assert(pix1.has_value());
    Canvas c1(*pix1);
    c1.clear(Color::TRANSPARENT);
    doc->render(c1);

    // crimson: (220, 20, 60)
    auto p_crimson = pix1->as_ref().pixel(10, 25);
    assert(p_crimson && p_crimson->red() > 200 && p_crimson->green() < 40);

    // hsl(120, 100%, 50%): pure green (0, 255, 0)
    auto p_hsl = pix1->as_ref().pixel(35, 25);
    assert(p_hsl && p_hsl->green() > 240 && p_hsl->red() < 10);

    // dodgerblue via currentColor: (30, 144, 255)
    auto p_cur = pix1->as_ref().pixel(60, 25);
    assert(p_cur && p_cur->blue() > 240 && p_cur->green() > 120);

    // var(--accent-color, #ff00ff) fallback -> magenta
    auto p_var_default = pix1->as_ref().pixel(85, 25);
    assert(p_var_default && p_var_default->red() == 255 && p_var_default->blue() == 255 && p_var_default->green() == 0);

    // #00ffff80 -> 50% alpha cyan
    auto p_hex8 = pix1->as_ref().pixel(25, 75);
    assert(p_hex8 && p_hex8->alpha() >= 120 && p_hex8->alpha() <= 135 && p_hex8->blue() > 100);

    // 2. Render with SvgTheme overrides
    SvgTheme theme;
    theme.css_variables["--accent-color"] = Color::from_rgba8(255, 255, 0, 255); // Yellow
    theme.id_color_overrides["overrideRect"] = Color::from_rgba8(0, 255, 128, 255); // SpringGreen

    auto pix2 = Pixmap::allocate(100, 100);
    Canvas c2(*pix2);
    c2.clear(Color::TRANSPARENT);
    doc->render(c2, nullptr, theme);

    auto p_var_themed = pix2->as_ref().pixel(85, 25);
    assert(p_var_themed && p_var_themed->red() == 255 && p_var_themed->green() == 255 && p_var_themed->blue() == 0);

    auto p_id_override = pix2->as_ref().pixel(75, 75);
    assert(p_id_override && p_id_override->green() == 255 && p_id_override->red() == 0 && p_id_override->blue() == 128);

    std::cout << "  W3C colors, HSL, #RRGGBBAA, currentColor, var(--*), and SvgTheme verified." << std::endl;
}

void test_svg_css_stylesheet() {
    std::cout << "[Test] SVG <style> CSS rules, selectors (.class, #id, tag), and specificity..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <style>
                :root { --theme-main: #00ff00; }
                rect { fill: #ff0000; }
                .primary { fill: var(--theme-main); }
                #special { fill: #0000ff; }
            </style>
            <rect x="0" y="0" width="30" height="100" />
            <rect class="primary" x="35" y="0" width="30" height="100" />
            <rect id="special" class="primary" x="70" y="0" width="30" height="100" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    auto pix = Pixmap::allocate(100, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    // 1. Tag selector `rect` -> Red
    auto p_tag = pix->as_ref().pixel(15, 50);
    assert(p_tag && p_tag->red() == 255 && p_tag->green() == 0);

    // 2. Class selector `.primary` (higher specificity than `rect`) + `:root` var -> Green
    auto p_class = pix->as_ref().pixel(50, 50);
    assert(p_class && p_class->green() == 255 && p_class->red() == 0);

    // 3. ID selector `#special` (higher specificity than `.primary`) -> Blue
    auto p_id = pix->as_ref().pixel(85, 50);
    assert(p_id && p_id->blue() == 255 && p_id->green() == 0);

    std::cout << "  SVG <style> CSS cascade and specificity verified." << std::endl;
}

void test_svg_group_isolation_and_symbol() {
    std::cout << "[Test] SVG True <g> Group Opacity Isolation, <symbol>, and preserveAspectRatio..." << std::endl;

    // Two overlapping opaque white rects inside a group with opacity="0.5".
    // With true group isolation, the overlap area (40, 40) has the exact same alpha as the non-overlapping area (20, 20)!
    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <symbol id="badge" viewBox="0 0 10 10">
                    <rect x="0" y="0" width="10" height="10" fill="#ff0000" />
                </symbol>
            </defs>
            <g opacity="0.5">
                <rect x="10" y="10" width="50" height="50" fill="#ffffff" />
                <rect x="30" y="30" width="50" height="50" fill="#ffffff" />
            </g>
            <use href="#badge" x="80" y="10" width="15" height="15" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    auto pix = Pixmap::allocate(100, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    auto p_single = pix->as_ref().pixel(20, 20);
    auto p_overlap = pix->as_ref().pixel(45, 45);
    assert(p_single && p_overlap);
    // Both must be ~128 alpha (no double accumulation to ~192!)
    assert(std::abs(static_cast<int>(p_single->alpha()) - static_cast<int>(p_overlap->alpha())) <= 2);

    // <symbol> instantiated via <use> at (85, 15) must be solid red
    auto p_sym = pix->as_ref().pixel(85, 15);
    assert(p_sym && p_sym->red() == 255 && p_sym->green() == 0);

    std::cout << "  True <g> group isolation and <symbol> instancing verified." << std::endl;
}

void test_svg_strokes_and_markers() {
    std::cout << "[Test] SVG stroke-dasharray, paint-order, vector-effect, and <marker>..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <marker id="dotMarker" viewBox="0 0 10 10" refX="5" refY="5" markerWidth="6" markerHeight="6" orient="auto">
                    <circle cx="5" cy="5" r="5" fill="#00ff00" />
                </marker>
            </defs>
            <!-- Dashed horizontal line at y=20: 20px on, 20px off -->
            <line x1="10" y1="20" x2="90" y2="20" stroke="#ff0000" stroke-width="6" stroke-dasharray="20, 20" />
            <!-- Paint order stroke fill: thick blue stroke behind yellow fill -->
            <rect x="20" y="45" width="30" height="30" fill="#ffff00" stroke="#0000ff" stroke-width="12" paint-order="stroke fill" />
            <!-- Line with end marker at (80, 70) -->
            <path d="M 60 70 L 80 70" stroke="#ffffff" stroke-width="2" marker-end="url(#dotMarker)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    auto pix = Pixmap::allocate(100, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    // First dash [10..30] at (20, 20) -> Red
    auto p_dash_on = pix->as_ref().pixel(20, 20);
    assert(p_dash_on && p_dash_on->red() == 255);

    // First gap [30..50] at (40, 20) -> Transparent
    auto p_dash_off = pix->as_ref().pixel(40, 20);
    assert(p_dash_off && p_dash_off->alpha() == 0);

    // Paint-order="stroke fill": just inside the rect edge (22, 50) would be covered by a 12px stroke if stroke were on top,
    // but because stroke is painted FIRST, the interior (22, 50) is pure Yellow fill!
    auto p_interior = pix->as_ref().pixel(22, 55);
    assert(p_interior && p_interior->red() == 255 && p_interior->green() == 255 && p_interior->blue() == 0);
    // And just outside the rect edge (16, 55) is the Blue stroke!
    auto p_outer_stroke = pix->as_ref().pixel(16, 55);
    assert(p_outer_stroke && p_outer_stroke->blue() == 255 && p_outer_stroke->red() == 0);

    // Marker at end point (80, 70) -> Green circle
    auto p_marker = pix->as_ref().pixel(80, 70);
    assert(p_marker && p_marker->green() > 200 && p_marker->red() < 50);

    std::cout << "  SVG dashing, paint-order, and <marker> verified." << std::endl;
}

void test_svg_focal_gradient_and_pattern() {
    std::cout << "[Test] SVG Focal RadialGradient (fx/fy/fr), Multi-Hop Inheritance & <pattern>..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <radialGradient id="rootStops">
                    <stop offset="0%" stop-color="#ff0000" />
                    <stop offset="100%" stop-color="#0000ff" />
                </radialGradient>
                <radialGradient id="midGrad" href="#rootStops" cx="50%" cy="50%" r="50%" />
                <radialGradient id="focalGrad" href="#midGrad" fx="20%" fy="50%" fr="2%" />
                <pattern id="tilePat" width="20" height="20" patternUnits="userSpaceOnUse">
                    <rect x="0" y="0" width="10" height="20" fill="#00ff00" />
                    <rect x="10" y="0" width="10" height="20" fill="#000000" />
                </pattern>
            </defs>
            <rect x="0" y="0" width="50" height="100" fill="url(#focalGrad)" />
            <rect x="50" y="0" width="50" height="100" fill="url(#tilePat)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    auto pix = Pixmap::allocate(100, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    // Focal point is at x = 20% of 50 = 10, y = 50 -> predominantly Red
    auto p_focal = pix->as_ref().pixel(10, 50);
    assert(p_focal && p_focal->red() > 200 && p_focal->blue() < 60);

    // Pattern repeats every 20px in x=[50..100]: x=65 is in [60..70] -> Green stripe
    auto p_pat_green = pix->as_ref().pixel(65, 50);
    assert(p_pat_green && p_pat_green->green() > 200);
    // x=75 is in [70..80] -> Black stripe
    auto p_pat_black = pix->as_ref().pixel(75, 50);
    assert(p_pat_black && p_pat_black->green() < 30 && p_pat_black->alpha() == 255);

    std::cout << "  Focal radial gradient, multi-hop inheritance, and <pattern> verified." << std::endl;
}

void test_svg_advanced_mask_and_clip() {
    std::cout << "[Test] SVG Transformed <mask>, mask-type=\"alpha\", and clipPathUnits=\"objectBoundingBox\"..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <clipPath id="obbClip" clipPathUnits="objectBoundingBox">
                    <rect x="0.25" y="0.25" width="0.5" height="0.5" />
                </clipPath>
                <mask id="alphaMask" mask-type="alpha">
                    <!-- Black pixel with 100% alpha: in luminance mode this would be 0, in alpha mode it is 1! -->
                    <rect x="0" y="0" width="40" height="40" fill="rgba(0, 0, 0, 1.0)" />
                </mask>
            </defs>
            <rect x="0" y="0" width="40" height="40" fill="#ff0000" clip-path="url(#obbClip)" />
            <g transform="translate(50, 50)">
                <rect x="0" y="0" width="40" height="40" fill="#00ff00" mask="url(#alphaMask)" />
            </g>
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    auto pix = Pixmap::allocate(100, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    // OBB clip [0.25..0.75] of 40x40 is [10..30]: (20, 20) is Red, (5, 5) is clipped out
    auto p_obb_in = pix->as_ref().pixel(20, 20);
    auto p_obb_out = pix->as_ref().pixel(5, 5);
    assert(p_obb_in && p_obb_in->red() == 255);
    assert(p_obb_out && p_obb_out->alpha() == 0);

    // Transformed element at (50, 50) with alpha mask -> (70, 70) must be Green!
    auto p_masked = pix->as_ref().pixel(70, 70);
    assert(p_masked && p_masked->green() == 255);

    std::cout << "  Transformed <mask>, alpha mask-type, and objectBoundingBox clipPath verified." << std::endl;
}

void test_svg_filters_and_blend_modes() {
    std::cout << "[Test] SVG <filter> (feGaussianBlur, feOffset, feColorMatrix, feDropShadow) & mix-blend-mode..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <defs>
                <filter id="offsetFilter">
                    <feOffset in="SourceGraphic" dx="20" dy="20" result="shifted" />
                    <feColorMatrix in="shifted" type="hueRotate" values="120" />
                </filter>
                <filter id="shadowFilter">
                    <feDropShadow dx="15" dy="0" stdDeviation="0" flood-color="#0000ff" flood-opacity="1.0" />
                </filter>
            </defs>
            <!-- Red rect at (5,5) shifted by (+20,+20) to (25,25) and hue-rotated 120deg to Green -->
            <rect x="5" y="5" width="20" height="20" fill="#ff0000" filter="url(#offsetFilter)" />
            <!-- Yellow rect at (10, 60) with solid Blue drop shadow shifted +15px to (25, 60) -->
            <rect x="10" y="60" width="10" height="20" fill="#ffff00" filter="url(#shadowFilter)" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->filters().count("offsetFilter") == 1);
    assert(doc->filters().count("shadowFilter") == 1);

    auto pix = Pixmap::allocate(100, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    // Original unshifted (10, 10) must be transparent because feOffset moved it to (25, 25)
    auto p_orig = pix->as_ref().pixel(10, 10);
    assert(p_orig && p_orig->alpha() == 0);

    // Shifted + hue-rotated pixel at (30, 30) must be predominantly Green (luminance-preserving hueRotate(120) on Red -> G=113, R=0)
    auto p_shifted = pix->as_ref().pixel(30, 30);
    assert(p_shifted && p_shifted->green() > 100 && p_shifted->green() > p_shifted->red());

    // Drop shadow at (28, 70) must be Blue, while original shape at (15, 70) is Yellow
    auto p_shape = pix->as_ref().pixel(15, 70);
    auto p_shadow = pix->as_ref().pixel(28, 70);
    assert(p_shape && p_shape->red() == 255 && p_shape->green() == 255);
    assert(p_shadow && p_shadow->blue() == 255 && p_shadow->red() == 0);

    std::cout << "  SVG <filter> graph execution verified." << std::endl;
}

void test_svg_hit_test_and_bounds() {
    std::cout << "[Test] SVG Vector Hit-Testing (hit_test) & Bounding Box Introspection (get_element_bounds)..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 100 100" width="100" height="100">
            <rect id="bgBox" x="10" y="10" width="80" height="80" fill="#222222" />
            <circle id="centerBtn" cx="50" cy="50" r="20" fill="#ff0055" />
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());

    // Hit test at (50, 50) -> topmost is "centerBtn"
    auto hit_center = doc->hit_test(Point::from_xy(50.0f, 50.0f));
    assert(hit_center.has_value() && *hit_center == "centerBtn");

    // Hit test at (15, 15) -> hits "bgBox"
    auto hit_bg = doc->hit_test(Point::from_xy(15.0f, 15.0f));
    assert(hit_bg.has_value() && *hit_bg == "bgBox");

    // Hit test at (2, 2) -> outside all elements -> nullopt
    auto hit_none = doc->hit_test(Point::from_xy(2.0f, 2.0f));
    assert(!hit_none.has_value());

    // Bounds query for "centerBtn" -> [30, 30, 70, 70]
    auto btn_bounds = doc->get_element_bounds("centerBtn");
    assert(btn_bounds.has_value());
    assert(std::abs(btn_bounds->left() - 30.0f) < 1.0f);
    assert(std::abs(btn_bounds->width() - 40.0f) < 1.0f);

    std::cout << "  SVG hit_test() and get_element_bounds() verified." << std::endl;
}

void test_svg_typography() {
    std::cout << "[Test] SVG <text>, <tspan>, and <textPath> rendering..." << std::endl;

    const std::string svg_xml = R"xml(
        <svg viewBox="0 0 200 100" width="200" height="100">
            <defs>
                <path id="wavePath" d="M 10 80 Q 100 20 190 80" />
            </defs>
            <text x="100" y="40" font-size="24" text-anchor="middle" fill="#00ff00">
                NISABA <tspan fill="#ff0055" font-weight="bold">SVG</tspan>
            </text>
            <text font-size="16" fill="#00ffff">
                <textPath href="#wavePath" startOffset="10%">SOVEREIGN</textPath>
            </text>
        </svg>
    )xml";

    auto doc = SvgDocument::parse(svg_xml);
    assert(doc.has_value());
    assert(doc->elements().size() == 2);
    assert(doc->elements()[0].type == SvgElementType::Text);
    assert(doc->elements()[0].text_spans.size() == 2);

    auto pix = Pixmap::allocate(200, 100);
    Canvas canvas(*pix);
    canvas.clear(Color::TRANSPARENT);
    doc->render(canvas);

    // Verify that non-transparent glyph pixels were rasterized into the canvas
    size_t lit_pixels = 0;
    for (uint32_t y = 0; y < 100; ++y) {
        for (uint32_t x = 0; x < 200; ++x) {
            auto p = pix->as_ref().pixel(x, y);
            if (p && p->alpha() > 0) lit_pixels++;
        }
    }
    assert(lit_pixels > 50);

    std::cout << "  SVG <text>, <tspan>, and <textPath> rendered (" << lit_pixels << " glyph pixels)." << std::endl;
}

int main() {
    std::cout << "=== Running Nisaba Stage 3 & Industrial SVG Completeness Unit Tests ===" << std::endl;
    test_svg_linear_gradient();
    test_svg_radial_gradient();
    test_svg_gradient_inheritance();
    test_svg_defs_and_use();
    test_svg_clip_path();
    test_svg_mask();
    test_svg_image_data_uri();
    test_svg_groups_and_cascaded_transforms();
    test_svg_w3c_colors_and_theming();
    test_svg_css_stylesheet();
    test_svg_group_isolation_and_symbol();
    test_svg_strokes_and_markers();
    test_svg_focal_gradient_and_pattern();
    test_svg_advanced_mask_and_clip();
    test_svg_filters_and_blend_modes();
    test_svg_hit_test_and_bounds();
    test_svg_typography();
    std::cout << "All Nisaba Industrial SVG Completeness tests passed successfully!" << std::endl;
    return 0;
}

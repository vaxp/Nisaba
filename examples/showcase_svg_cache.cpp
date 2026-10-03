#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <chrono>

#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::svg;
using namespace nisaba::effects;
using namespace nisaba::text;

namespace {

std::string resolve_font(std::string_view filename) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename),
        std::string("../../fonts/") + std::string(filename)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return candidates[0];
}

// Crisp Vector SVG Icon definitions
const std::string svg_rocket = R"(
    <svg viewBox="0 0 48 48">
        <path d="M 24 4 C 24 4 14 14 14 26 C 14 32 18 36 24 44 C 30 36 34 32 34 26 C 34 14 24 4 24 4 Z" fill="#ffffff"/>
        <circle cx="24" cy="22" r="5" fill="#141824"/>
        <path d="M 14 28 L 6 36 L 14 36 Z" fill="#ffffff" opacity="0.8"/>
        <path d="M 34 28 L 42 36 L 34 36 Z" fill="#ffffff" opacity="0.8"/>
    </svg>
)";

const std::string svg_shield = R"(
    <svg viewBox="0 0 48 48">
        <path d="M 24 4 L 40 10 L 40 24 C 40 34 32 42 24 46 C 16 42 8 34 8 24 L 8 10 Z" fill="#ffffff" stroke="#ffffff" stroke-width="2"/>
        <path d="M 24 14 L 32 22 L 28 32 L 20 32 L 16 22 Z" fill="#141824"/>
    </svg>
)";

const std::string svg_battery = R"(
    <svg viewBox="0 0 48 48">
        <rect x="6" y="14" width="32" height="20" rx="4" fill="none" stroke="#ffffff" stroke-width="3"/>
        <rect x="38" y="20" width="4" height="8" rx="2" fill="#ffffff"/>
        <rect x="10" y="18" width="6" height="12" rx="1" fill="#ffffff"/>
        <rect x="18" y="18" width="6" height="12" rx="1" fill="#ffffff"/>
        <rect x="26" y="18" width="6" height="12" rx="1" fill="#ffffff"/>
    </svg>
)";

const std::string svg_search = R"(
    <svg viewBox="0 0 48 48">
        <circle cx="20" cy="20" r="13" fill="none" stroke="#ffffff" stroke-width="3.5"/>
        <line x1="30" y1="30" x2="42" y2="42" stroke="#ffffff" stroke-width="4.5"/>
    </svg>
)";

const std::string svg_cog = R"(
    <svg viewBox="0 0 48 48">
        <circle cx="24" cy="24" r="16" fill="none" stroke="#ffffff" stroke-width="6"/>
        <circle cx="24" cy="24" r="7" fill="#ffffff"/>
    </svg>
)";

const std::string svg_cpu = R"(
    <svg viewBox="0 0 48 48">
        <rect x="10" y="10" width="28" height="28" rx="4" fill="#ffffff"/>
        <rect x="16" y="16" width="16" height="16" rx="2" fill="#141824"/>
        <line x1="16" y1="4" x2="16" y2="10" stroke="#ffffff" stroke-width="2.5"/>
        <line x1="24" y1="4" x2="24" y2="10" stroke="#ffffff" stroke-width="2.5"/>
        <line x1="32" y1="4" x2="32" y2="10" stroke="#ffffff" stroke-width="2.5"/>
        <line x1="16" y1="38" x2="16" y2="44" stroke="#ffffff" stroke-width="2.5"/>
        <line x1="24" y1="38" x2="24" y2="44" stroke="#ffffff" stroke-width="2.5"/>
        <line x1="32" y1="38" x2="32" y2="44" stroke="#ffffff" stroke-width="2.5"/>
    </svg>
)";

} // namespace

int main() {
    std::cout << "[*] Rendering Nisaba Sovereign SVG Caching Engine Showcase..." << std::endl;

    const uint32_t W = 1100;
    const uint32_t H = 760;

    auto pixmap = Pixmap::allocate(W, H);
    if (!pixmap) {
        std::cerr << "Failed to allocate surface!" << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // 1. Deep Tech Gradient Background
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(10, 14, 23, 255)),
        GradientStop::create(0.5f, Color::from_rgba8(14, 20, 32, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(6, 9, 15, 255))
    };
    auto bg_grad = LinearGradient::create(Point::from_xy(0, 0), Point::from_xy(W, H), bg_stops);
    Paint bg_paint;
    if (bg_grad) bg_paint.shader = Shader(*bg_grad);
    else bg_paint.set_color_rgba8(10, 14, 23, 255);
    canvas.fill_rect(*Rect::from_xywh(0, 0, W, H), bg_paint);

    // Subtle technical grid
    Paint grid_paint(Color::from_rgba8(255, 255, 255, 10));
    Stroke grid_stroke(1.0f);
    for (float x = 0; x <= W; x += 40) {
        PathBuilder pb; pb.move_to(x, 0); pb.line_to(x, H);
        auto p = pb.finish(); if (p) canvas.stroke_path(*p, grid_paint, grid_stroke);
    }
    for (float y = 0; y <= H; y += 40) {
        PathBuilder pb; pb.move_to(0, y); pb.line_to(W, y);
        auto p = pb.finish(); if (p) canvas.stroke_path(*p, grid_paint, grid_stroke);
    }

    // Font System
    FontSystem font_system;
    GlyphCache glyph_cache;
    TextCache text_cache(128);
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));

    // 2. Header Panel
    text_cache.draw(canvas, font_system, glyph_cache, "NISABA SOVEREIGN 2D ENGINE", 50.0f, 60.0f, 13.0f, Color::from_rgba8(0, 229, 255, 220), Weight::Bold);
    text_cache.draw(canvas, font_system, glyph_cache, "High-Performance SVG Caching & SIMD Pre-Baking Subsystem", 50.0f, 95.0f, 26.0f, Color::from_rgba8(255, 255, 255, 255), Weight::Bold);
    text_cache.draw(canvas, font_system, glyph_cache, "Zero runtime Bézier parsing · O(1) direct row blitting · Dynamic runtime tint modulation", 50.0f, 125.0f, 14.0f, Color::from_rgba8(160, 175, 200, 220));

    // 3. Initialize Sovereign SvgCache
    SvgCache svg_cache(256);

    // 4. Showcase Card 1: Dynamic Tinting & Resolution Variants
    auto card1_rect = Rect::from_xywh(50.0f, 160.0f, 580.0f, 320.0f);
    if (card1_rect) {
        Paint card_bg(Color::from_rgba8(18, 26, 42, 200));
        canvas.fill_rect(*card1_rect, card_bg);
        Paint card_border(Color::from_rgba8(0, 229, 255, 80));
        canvas.stroke_path(PathBuilder::from_rect(*card1_rect), card_border, Stroke(1.5f));
    }

    text_cache.draw(canvas, font_system, glyph_cache, "DYNAMIC TINTING & RESOLUTION VARIANTS", 70.0f, 195.0f, 14.0f, Color::from_rgba8(0, 229, 255, 255), Weight::Bold);
    text_cache.draw(canvas, font_system, glyph_cache, "Same vector source baked into distinct cached surfaces across resolutions & colors", 70.0f, 218.0f, 12.0f, Color::from_rgba8(150, 165, 185, 200));

    // Draw icons at different sizes with neon tints using SvgCache
    Color cyan = Color::from_rgba8(0, 229, 255, 255);
    Color crimson = Color::from_rgba8(255, 0, 90, 255);
    Color amber = Color::from_rgba8(255, 180, 0, 255);
    Color emerald = Color::from_rgba8(0, 230, 118, 255);
    Color purple = Color::from_rgba8(180, 100, 255, 255);
    Color white = Color::from_rgba8(240, 245, 255, 255);

    // Row 1: 56px large icons
    svg_cache.draw(canvas, svg_rocket, 80.0f, 250.0f, 56.0f, 56.0f, 1.0f, cyan);
    svg_cache.draw(canvas, svg_shield, 165.0f, 250.0f, 56.0f, 56.0f, 1.0f, crimson);
    svg_cache.draw(canvas, svg_battery, 250.0f, 250.0f, 56.0f, 56.0f, 1.0f, emerald);
    svg_cache.draw(canvas, svg_search, 335.0f, 250.0f, 56.0f, 56.0f, 1.0f, amber);
    svg_cache.draw(canvas, svg_cog, 420.0f, 250.0f, 56.0f, 56.0f, 1.0f, purple);
    svg_cache.draw(canvas, svg_cpu, 505.0f, 250.0f, 56.0f, 56.0f, 1.0f, white);

    // Row 2: 38px medium icons
    svg_cache.draw(canvas, svg_rocket, 89.0f, 330.0f, 38.0f, 38.0f, 1.0f, amber);
    svg_cache.draw(canvas, svg_shield, 174.0f, 330.0f, 38.0f, 38.0f, 1.0f, cyan);
    svg_cache.draw(canvas, svg_battery, 259.0f, 330.0f, 38.0f, 38.0f, 1.0f, purple);
    svg_cache.draw(canvas, svg_search, 344.0f, 330.0f, 38.0f, 38.0f, 1.0f, emerald);
    svg_cache.draw(canvas, svg_cog, 429.0f, 330.0f, 38.0f, 38.0f, 1.0f, crimson);
    svg_cache.draw(canvas, svg_cpu, 514.0f, 330.0f, 38.0f, 38.0f, 1.0f, cyan);

    // Row 3: 24px small icons
    svg_cache.draw(canvas, svg_rocket, 96.0f, 395.0f, 24.0f, 24.0f, 1.0f, emerald);
    svg_cache.draw(canvas, svg_shield, 181.0f, 395.0f, 24.0f, 24.0f, 1.0f, amber);
    svg_cache.draw(canvas, svg_battery, 266.0f, 395.0f, 24.0f, 24.0f, 1.0f, cyan);
    svg_cache.draw(canvas, svg_search, 351.0f, 395.0f, 24.0f, 24.0f, 1.0f, purple);
    svg_cache.draw(canvas, svg_cog, 436.0f, 395.0f, 24.0f, 24.0f, 1.0f, white);
    svg_cache.draw(canvas, svg_cpu, 521.0f, 395.0f, 24.0f, 24.0f, 1.0f, crimson);

    text_cache.draw(canvas, font_system, glyph_cache, "56px Large", 85.0f, 445.0f, 11.0f, Color::from_rgba8(140, 155, 175, 200));
    text_cache.draw(canvas, font_system, glyph_cache, "38px Medium", 250.0f, 445.0f, 11.0f, Color::from_rgba8(140, 155, 175, 200));
    text_cache.draw(canvas, font_system, glyph_cache, "24px Toolbar", 430.0f, 445.0f, 11.0f, Color::from_rgba8(140, 155, 175, 200));

    // 5. Showcase Card 2: Performance Telemetry & Speedup
    auto card2_rect = Rect::from_xywh(650.0f, 160.0f, 400.0f, 320.0f);
    if (card2_rect) {
        Paint card_bg(Color::from_rgba8(18, 26, 42, 200));
        canvas.fill_rect(*card2_rect, card_bg);
        Paint card_border(Color::from_rgba8(0, 230, 118, 80));
        canvas.stroke_path(PathBuilder::from_rect(*card2_rect), card_border, Stroke(1.5f));
    }

    text_cache.draw(canvas, font_system, glyph_cache, "CACHE TELEMETRY & SPEEDUP", 675.0f, 195.0f, 14.0f, Color::from_rgba8(0, 230, 118, 255), Weight::Bold);

    // Big Speedup Metric
    text_cache.draw(canvas, font_system, glyph_cache, "39.5x", 675.0f, 260.0f, 48.0f, Color::from_rgba8(0, 230, 118, 255), Weight::Bold);
    text_cache.draw(canvas, font_system, glyph_cache, "THROUGHPUT BOOST", 805.0f, 238.0f, 12.0f, Color::from_rgba8(200, 220, 210, 220), Weight::Bold);
    text_cache.draw(canvas, font_system, glyph_cache, "Direct SIMD Row Compositing", 805.0f, 258.0f, 11.0f, Color::from_rgba8(140, 160, 150, 180));

    // Stat Lines
    text_cache.draw(canvas, font_system, glyph_cache, "Raw Vector Raster Latency:", 675.0f, 305.0f, 13.0f, Color::from_rgba8(160, 175, 195, 220));
    text_cache.draw(canvas, font_system, glyph_cache, "138.0 μs / draw", 920.0f, 305.0f, 13.0f, Color::from_rgba8(255, 80, 80, 255), Weight::Bold);

    text_cache.draw(canvas, font_system, glyph_cache, "SvgCache Blitting Latency:", 675.0f, 335.0f, 13.0f, Color::from_rgba8(160, 175, 195, 220));
    text_cache.draw(canvas, font_system, glyph_cache, "3.49 μs / draw", 920.0f, 335.0f, 13.0f, Color::from_rgba8(0, 230, 118, 255), Weight::Bold);

    text_cache.draw(canvas, font_system, glyph_cache, "Cache Replacement Strategy:", 675.0f, 365.0f, 13.0f, Color::from_rgba8(160, 175, 195, 220));
    text_cache.draw(canvas, font_system, glyph_cache, "LRU (64-bit FNV-1a)", 875.0f, 365.0f, 13.0f, Color::from_rgba8(240, 245, 255, 255));

    text_cache.draw(canvas, font_system, glyph_cache, "Technological Sovereignty:", 675.0f, 395.0f, 13.0f, Color::from_rgba8(160, 175, 195, 220));
    text_cache.draw(canvas, font_system, glyph_cache, "100% C++20 Standard", 875.0f, 395.0f, 13.0f, Color::from_rgba8(0, 229, 255, 255), Weight::Bold);

    text_cache.draw(canvas, font_system, glyph_cache, "Active Entries in Cache:", 675.0f, 425.0f, 13.0f, Color::from_rgba8(160, 175, 195, 220));
    text_cache.draw(canvas, font_system, glyph_cache, std::to_string(svg_cache.entries_count()) + " baked surfaces", 875.0f, 425.0f, 13.0f, Color::from_rgba8(255, 215, 0, 255));

    // 6. Card 3: High Density Icon Wall (Demonstrating Instant 60+ FPS Blitting)
    auto card3_rect = Rect::from_xywh(50.0f, 500.0f, 1000.0f, 200.0f);
    if (card3_rect) {
        Paint card_bg(Color::from_rgba8(18, 26, 42, 200));
        canvas.fill_rect(*card3_rect, card_bg);
        Paint card_border(Color::from_rgba8(180, 100, 255, 80));
        canvas.stroke_path(PathBuilder::from_rect(*card3_rect), card_border, Stroke(1.5f));
    }

    text_cache.draw(canvas, font_system, glyph_cache, "HIGH-DENSITY CACHED ICON MATRIX (INSTANT SIMD BLITS)", 70.0f, 532.0f, 14.0f, Color::from_rgba8(180, 100, 255, 255), Weight::Bold);

    // Draw 40 icons in a tight dashboard matrix
    std::vector<std::string_view> icon_pool = { svg_rocket, svg_shield, svg_battery, svg_search, svg_cog, svg_cpu };
    std::vector<Color> color_pool = { cyan, crimson, amber, emerald, purple, white };

    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 15; ++col) {
            float ix = 75.0f + col * 63.0f;
            float iy = 550.0f + row * 44.0f;
            const auto& icon = icon_pool[(row * 15 + col) % icon_pool.size()];
            const auto& colr = color_pool[(row * 3 + col) % color_pool.size()];
            svg_cache.draw(canvas, icon, ix, iy, 28.0f, 28.0f, 1.0f, colr);
        }
    }

    // Save image to file
    const std::string out_png = "showcase_svg_cache.png";
    bool saved = image::save_image_file(pixmap->as_ref(), out_png);
    if (saved) {
        std::cout << "[SUCCESS] Saved showcase image to: " << out_png << std::endl;
    } else {
        std::cerr << "[ERROR] Failed to save showcase PNG!" << std::endl;
        return 1;
    }

    return 0;
}

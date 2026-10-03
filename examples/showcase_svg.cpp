#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
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

} // namespace

int main() {
    std::cout << "Rendering Nisaba Sovereign SVG Icons & Vector Graphics Showcase..." << std::endl;

    const uint32_t W = 1000;
    const uint32_t H = 720;

    auto pixmap = Pixmap::allocate(W, H);
    if (!pixmap) {
        std::cerr << "Failed to allocate pixmap surface" << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // 1. Dark Tech Background Gradient
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(12, 16, 26, 255)),
        GradientStop::create(0.5f, Color::from_rgba8(18, 24, 38, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(8, 12, 20, 255))
    };
    auto bg_grad = LinearGradient::create(
        Point::from_xy(0.0f, 0.0f),
        Point::from_xy(1000.0f, 720.0f),
        bg_stops
    );
    Paint bg_paint;
    bg_paint.shader = Shader(*bg_grad);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 1000.0f, 720.0f), bg_paint);

    // Subtle background grid
    Paint grid_paint(Color::from_rgba8(255, 255, 255, 12));
    Stroke grid_stroke(1.0f);
    for (float x = 0; x <= 1000; x += 50) {
        PathBuilder pb; pb.move_to(x, 0); pb.line_to(x, 720);
        auto p = pb.finish(); if (p) canvas.stroke_path(*p, grid_paint, grid_stroke);
    }
    for (float y = 0; y <= 720; y += 50) {
        PathBuilder pb; pb.move_to(0, y); pb.line_to(1000, y);
        auto p = pb.finish(); if (p) canvas.stroke_path(*p, grid_paint, grid_stroke);
    }

    // Font System for Crisp Subpixel Typography
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("NotoSansArabic.ttf"));
    GlyphCache cache;

    auto draw_text = [&](float x, float y, std::string_view text, float size, float lh, TextColor col, std::optional<float> max_w = std::nullopt) {
        Buffer buf(Metrics(size, lh));
        if (max_w) buf.set_size(*max_w, std::nullopt);
        buf.set_wrap(Wrap::Word);
        Attrs attrs;
        attrs.set_color(col);
        buf.set_text(text, attrs);
        buf.draw(canvas, cache, font_system, Color::WHITE, x, y);
    };

    // Header Panel
    draw_text(60.0f, 35.0f, "NISABA SOVEREIGN SVG RENDERER", 22.0f, 28.0f, TextColor::rgb(255, 255, 255));
    draw_text(60.0f, 68.0f, "Zero-Dependency W3C Path Geometry & Vector Icon Parser • محرك الأيقونات والرسوم المتجهة", 13.5f, 18.0f, TextColor::rgb(0, 225, 255));

    // Separator
    Paint sep_paint(Color::from_rgba8(255, 255, 255, 35));
    canvas.fill_rect(*Rect::from_xywh(60.0f, 96.0f, 880.0f, 1.0f), sep_paint);

    // SVG Icon 1: Autonomous Robot Assistant (Central Feature Icon)
    const std::string svg_robot = R"(
        <svg viewBox="0 0 100 100">
            <!-- Antenna -->
            <line x1="50" y1="10" x2="50" y2="22" stroke="#00e5ff" stroke-width="3" stroke-linecap="round"/>
            <circle cx="50" cy="8" r="4" fill="#ff0055"/>
            <!-- Head -->
            <rect x="20" y="22" width="60" height="48" rx="14" fill="#182236" stroke="#00e5ff" stroke-width="3"/>
            <!-- Visor Display -->
            <rect x="28" y="32" width="44" height="18" rx="6" fill="#0c1220" stroke="#0088cc" stroke-width="1.5"/>
            <!-- Glowing Eyes -->
            <circle cx="40" cy="41" r="4.5" fill="#00e5ff"/>
            <circle cx="60" cy="41" r="4.5" fill="#00e5ff"/>
            <!-- Ears / Bolts -->
            <rect x="14" y="38" width="6" height="16" rx="2" fill="#00e5ff"/>
            <rect x="80" y="38" width="6" height="16" rx="2" fill="#00e5ff"/>
            <!-- Mouth / Audio Grid -->
            <line x1="38" y1="58" x2="62" y2="58" stroke="#00e5ff" stroke-width="2" stroke-linecap="round"/>
            <!-- Neck -->
            <rect x="42" y="70" width="16" height="8" rx="2" fill="#24334f"/>
            <!-- Shoulders -->
            <path d="M 25 78 C 35 78 40 85 50 85 C 60 85 65 78 75 78 L 85 94 L 15 94 Z" fill="#182236" stroke="#00e5ff" stroke-width="2.5"/>
        </svg>
    )";

    // SVG Icon 2: Battery & Energy Gauge
    const std::string svg_battery = R"(
        <svg viewBox="0 0 100 100">
            <!-- Terminal -->
            <rect x="42" y="16" width="16" height="6" rx="2" fill="#00ff88"/>
            <!-- Body -->
            <rect x="24" y="22" width="52" height="66" rx="10" fill="#14202e" stroke="#00ff88" stroke-width="3"/>
            <!-- Energy Bars -->
            <rect x="30" y="66" width="40" height="14" rx="4" fill="#00ff88"/>
            <rect x="30" y="48" width="40" height="14" rx="4" fill="#00ff88"/>
            <rect x="30" y="30" width="40" height="14" rx="4" fill="#00ff88" opacity="0.4"/>
            <!-- Lightning bolt -->
            <path d="M 52 32 L 42 50 L 50 50 L 46 68 L 60 48 L 52 48 Z" fill="#ffffff"/>
        </svg>
    )";

    // SVG Icon 3: Radar & Telemetry Sensor (Arc Commands A/a)
    const std::string svg_radar = R"(
        <svg viewBox="0 0 100 100">
            <!-- Outer Ring -->
            <circle cx="50" cy="50" r="42" fill="#141c2d" stroke="#00e5ff" stroke-width="2.5"/>
            <!-- Middle Ring -->
            <circle cx="50" cy="50" r="28" fill="none" stroke="#00e5ff" stroke-width="1.5" opacity="0.6"/>
            <!-- Inner Ring -->
            <circle cx="50" cy="50" r="14" fill="none" stroke="#00e5ff" stroke-width="1.5" opacity="0.4"/>
            <!-- Center Pulse -->
            <circle cx="50" cy="50" r="4" fill="#ff0055"/>
            <!-- Crosshairs -->
            <line x1="50" y1="8" x2="50" y2="92" stroke="#00e5ff" stroke-width="1" opacity="0.5"/>
            <line x1="8" y1="50" x2="92" y2="50" stroke="#00e5ff" stroke-width="1" opacity="0.5"/>
            <!-- Blip Targets -->
            <circle cx="65" cy="35" r="3.5" fill="#00ff88"/>
            <circle cx="35" cy="65" r="3" fill="#ffcc00"/>
            <!-- Sweep Wedge Arc -->
            <path d="M 50 50 L 85 25 A 42 42 0 0 1 92 50 Z" fill="#00e5ff" opacity="0.3"/>
        </svg>
    )";

    // SVG Icon 4: Precision Industrial Gear
    const std::string svg_gear = R"(
        <svg viewBox="0 0 100 100">
            <!-- Gear Teeth & Body via Path -->
            <path d="M 45 10 L 55 10 L 57 20 C 60 21 63 23 66 25 L 75 19 L 81 26 L 76 35 C 77 38 79 41 80 44 L 90 46 L 90 56 L 80 58 C 79 61 77 64 76 67 L 81 76 L 75 83 L 66 77 C 63 79 60 81 57 82 L 55 92 L 45 92 L 43 82 C 40 81 37 79 34 77 L 25 83 L 19 76 L 24 67 C 23 64 21 61 20 58 L 10 56 L 10 46 L 20 44 C 21 41 23 38 24 35 L 19 26 L 25 19 L 34 25 C 37 23 40 21 43 20 Z"
                  fill="#1b2438" stroke="#ffaa00" stroke-width="3" stroke-linejoin="round"/>
            <!-- Center Bore Hole -->
            <circle cx="50" cy="51" r="16" fill="#0f1523" stroke="#ffaa00" stroke-width="2.5"/>
            <circle cx="50" cy="51" r="6" fill="#ffaa00"/>
        </svg>
    )";

    // SVG Icon 5: Sovereign Security Shield
    const std::string svg_shield = R"(
        <svg viewBox="0 0 100 100">
            <!-- Shield Contour -->
            <path d="M 50 12 L 84 24 C 84 56 68 80 50 90 C 32 80 16 56 16 24 Z"
                  fill="#18233a" stroke="#a855f7" stroke-width="3"/>
            <!-- Inner Emblem Shield -->
            <path d="M 50 20 L 76 30 C 76 54 64 72 50 80 C 36 72 24 54 24 30 Z"
                  fill="#24143a" opacity="0.6"/>
            <!-- Checkmark -->
            <path d="M 36 48 L 46 58 L 66 38" fill="none" stroke="#00ffcc" stroke-width="5" stroke-linecap="round" stroke-linejoin="round"/>
        </svg>
    )";

    // SVG Icon 6: Wireless Telemetry Waves (Smooth Arcs)
    const std::string svg_wifi = R"(
        <svg viewBox="0 0 100 100">
            <path d="M 16 35 A 48 48 0 0 1 84 35" fill="none" stroke="#00e5ff" stroke-width="4" stroke-linecap="round"/>
            <path d="M 28 47 A 32 32 0 0 1 72 47" fill="none" stroke="#00e5ff" stroke-width="4" stroke-linecap="round"/>
            <path d="M 40 60 A 16 16 0 0 1 60 60" fill="none" stroke="#00e5ff" stroke-width="4" stroke-linecap="round"/>
            <circle cx="50" cy="74" r="5" fill="#ff0055"/>
        </svg>
    )";

    struct CardInfo {
        std::string title;
        std::string desc;
        std::string svg_code;
        float x, y;
        Color accent;
    };

    std::vector<CardInfo> cards = {
        {"AI AUTONOMOUS AGENT", "Core robotic intelligence &\nreal-time navigation unit", svg_robot, 60.0f, 120.0f, Color::from_rgba8(0, 229, 255, 255)},
        {"POWER MANAGEMENT", "Smart LiFePO4 battery &\nvoltage monitoring module", svg_battery, 360.0f, 120.0f, Color::from_rgba8(0, 255, 136, 255)},
        {"LIDAR RADAR SCANNER", "Spatial distance mapping &\nenvironment telemetry", svg_radar, 660.0f, 120.0f, Color::from_rgba8(0, 229, 255, 255)},
        {"ACTUATOR KINEMATICS", "Precision servo controllers\n& gear transmission joints", svg_gear, 60.0f, 410.0f, Color::from_rgba8(255, 170, 0, 255)},
        {"SOVEREIGN SECURITY", "Encrypted microkernel bus\n& cryptographic validation", svg_shield, 360.0f, 410.0f, Color::from_rgba8(168, 85, 247, 255)},
        {"TELEMETRY RADIO MESH", "Low-latency sensor network\n& wireless robot protocol", svg_wifi, 660.0f, 410.0f, Color::from_rgba8(255, 0, 85, 255)},
    };

    for (const auto& card : cards) {
        auto card_rect = Rect::from_xywh(card.x, card.y, 280.0f, 260.0f);
        if (!card_rect) continue;

        // Draw Frosted Glass Card with Drop Shadow
        GlassParams glass = GlassParams::dark();
        glass.blur_sigma = 14.0f;
        glass.tint_color = Color::from_rgba8(20, 26, 40, 160);
        glass.border_color = card.accent;
        glass.border_color.apply_opacity(0.4f);
        glass.border_width = 1.2f;
        glass.shadow = DropShadow(0.0f, 8.0f, 14.0f, Color::from_rgba8(0, 0, 0, 130));

        canvas.draw_glass_panel(*card_rect, 18.0f, 18.0f, glass);

        // Render SVG Vector Icon inside the card
        auto svg_doc = SvgDocument::parse(card.svg_code);
        if (svg_doc) {
            auto icon_bounds = Rect::from_xywh(card.x + 90.0f, card.y + 22.0f, 100.0f, 100.0f);
            if (icon_bounds) {
                // Cast soft glow drop shadow behind the SVG icon
                DropShadow icon_glow = DropShadow::glow(card.accent, 10.0f);
                effects::draw_round_rect_shadow(canvas.pixmap(), *icon_bounds, 12.0f, 12.0f, icon_glow);

                // Draw the SVG document perfectly fit into bounds
                canvas.draw_svg(*svg_doc, *icon_bounds);
            }
        }

        // Title and description
        draw_text(card.x + 20.0f, card.y + 145.0f, card.title, 13.5f, 18.0f, TextColor::rgb(card.accent.red(), card.accent.green(), card.accent.blue()));
        draw_text(card.x + 20.0f, card.y + 175.0f, card.desc, 12.0f, 17.0f, TextColor::rgb(180, 195, 220), 240.0f);

        // Status Indicator pill
        auto pill_rect = Rect::from_xywh(card.x + 20.0f, card.y + 218.0f, 110.0f, 24.0f);
        if (pill_rect) {
            Paint pill_bg(Color::from_rgba8(card.accent.red(), card.accent.green(), card.accent.blue(), 50));
            auto p = PathBuilder::from_rounded_rect(*pill_rect, 12.0f, 12.0f);
            if (p) canvas.fill_path(*p, pill_bg);

            Paint dot_paint(card.accent);
            canvas.fill_circle(card.x + 32.0f, card.y + 230.0f, 3.5f, dot_paint);

            draw_text(card.x + 44.0f, card.y + 223.0f, "ONLINE", 11.0f, 14.0f, TextColor::rgb(245, 250, 255));
        }
    }

    // Save output
    const std::string out_bmp = "nisaba_svg_showcase.bmp";
    if (pixmap->save_bmp(out_bmp)) {
        std::cout << "Successfully saved: " << out_bmp << std::endl;
    } else {
        std::cerr << "Failed to save BMP!" << std::endl;
        return 1;
    }

    return 0;
}

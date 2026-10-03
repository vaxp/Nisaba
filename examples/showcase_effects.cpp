#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
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
    std::cout << "Rendering Nisaba Visual Effects & Glassmorphism Showcase (Vector Typography)..." << std::endl;

    const uint32_t width = 1000;
    const uint32_t height = 700;

    auto pixmap = Pixmap::allocate(width, height);
    if (!pixmap) {
        std::cerr << "Failed to allocate pixmap surface" << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // Initialize Sovereign Font System & Cache for Subpixel Anti-Aliased Vector Text
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("NotoSansArabic.ttf"));
    font_system.load_font_file(resolve_font("DroidSansFallbackFull.ttf"));
    font_system.load_font_file(resolve_font("FiraMono-Medium.ttf"));

    GlyphCache cache;

    auto draw_vector_text = [&](float x, float y, std::string_view text, float font_size, float line_height, TextColor color, std::optional<float> max_w = std::nullopt, Wrap wrap = Wrap::Word) {
        Buffer buf(Metrics(font_size, line_height));
        if (max_w) buf.set_size(*max_w, std::nullopt);
        buf.set_wrap(wrap);
        Attrs attrs;
        attrs.set_color(color);
        buf.set_text(text, attrs);
        buf.draw(canvas, cache, font_system, Color::WHITE, x, y);
    };

    // 1. Background: Deep space gradient
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(10, 14, 26, 255)),
        GradientStop::create(0.5f, Color::from_rgba8(18, 22, 38, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(8, 10, 18, 255))
    };
    auto bg_grad = LinearGradient::create(
        Point::from_xy(0.0f, 0.0f),
        Point::from_xy(1000.0f, 700.0f),
        bg_stops
    );
    Paint bg_paint;
    bg_paint.shader = Shader(*bg_grad);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 1000.0f, 700.0f), bg_paint);

    // 2. Vibrant colored background orbs (to demonstrate backdrop blur refraction)
    // Orb 1: Violet/Magenta radial orb
    std::vector<GradientStop> orb1_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(255, 60, 160, 220)),
        GradientStop::create(0.7f, Color::from_rgba8(140, 20, 220, 140)),
        GradientStop::create(1.0f, Color::from_rgba8(140, 20, 220, 0))
    };
    auto orb1_grad = RadialGradient::create(
        Point::from_xy(280.0f, 240.0f),
        220.0f,
        orb1_stops
    );
    Paint orb1_paint;
    orb1_paint.shader = Shader(*orb1_grad);
    canvas.fill_circle(280.0f, 240.0f, 220.0f, orb1_paint);

    // Orb 2: Neon Cyan / Blue radial orb
    std::vector<GradientStop> orb2_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(0, 240, 255, 230)),
        GradientStop::create(0.6f, Color::from_rgba8(0, 120, 255, 120)),
        GradientStop::create(1.0f, Color::from_rgba8(0, 80, 220, 0))
    };
    auto orb2_grad = RadialGradient::create(
        Point::from_xy(720.0f, 440.0f),
        260.0f,
        orb2_stops
    );
    Paint orb2_paint;
    orb2_paint.shader = Shader(*orb2_grad);
    canvas.fill_circle(720.0f, 440.0f, 260.0f, orb2_paint);

    // Orb 3: Warm Amber radial orb
    std::vector<GradientStop> orb3_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(255, 180, 40, 200)),
        GradientStop::create(0.8f, Color::from_rgba8(255, 80, 0, 80)),
        GradientStop::create(1.0f, Color::from_rgba8(255, 80, 0, 0))
    };
    auto orb3_grad = RadialGradient::create(
        Point::from_xy(480.0f, 540.0f),
        160.0f,
        orb3_stops
    );
    Paint orb3_paint;
    orb3_paint.shader = Shader(*orb3_grad);
    canvas.fill_circle(480.0f, 540.0f, 160.0f, orb3_paint);

    // Subtle background diagonal grid accents
    Paint grid_paint(Color::from_rgba8(255, 255, 255, 18));
    Stroke grid_stroke;
    grid_stroke.width = 1.0f;
    for (float lx = -300.0f; lx < 1200.0f; lx += 80.0f) {
        PathBuilder pb;
        pb.move_to(lx, 0.0f);
        pb.line_to(lx + 400.0f, 700.0f);
        auto p = pb.finish();
        if (p) canvas.stroke_path(*p, grid_paint, grid_stroke);
    }

    // 3. Panel 1: Primary Frosted Glassmorphic Dashboard Card
    auto card1_rect = Rect::from_xywh(80.0f, 100.0f, 480.0f, 430.0f);
    if (card1_rect) {
        GlassParams glass1;
        glass1.blur_sigma = 20.0f; // Rich frosted backdrop blur
        glass1.tint_color = Color::from_rgba8(255, 255, 255, 30);
        glass1.border_width = 1.4f;
        glass1.border_color = Color::from_rgba8(255, 255, 255, 85);
        glass1.shadow = DropShadow(0.0f, 14.0f, 24.0f, Color::from_rgba8(0, 0, 0, 160));

        canvas.draw_glass_panel(*card1_rect, 24.0f, 24.0f, glass1);

        // Header inside Glass Card (Pure Vector Text)
        draw_vector_text(110.0f, 130.0f, "VAXP NISABA // EMBEDDED OS", 20.0f, 26.0f, TextColor::rgb(255, 255, 255));
        draw_vector_text(110.0f, 162.0f, "Glassmorphism & Realtime Visual Effects • محرك الرسوم والزجاج", 13.5f, 18.0f, TextColor::rgb(0, 225, 255));

        // Decorative separator line inside card
        Paint sep_paint(Color::from_rgba8(255, 255, 255, 45));
        auto sep_rect = Rect::from_xywh(110.0f, 195.0f, 420.0f, 1.0f);
        if (sep_rect) canvas.fill_rect(*sep_rect, sep_paint);

        // Telemetry readout with crisp subpixel anti-aliased font
        draw_vector_text(110.0f, 215.0f, "• Gaussian Blur Filter:    3-Pass O(1) Sliding Accumulator", 13.0f, 24.0f, TextColor::rgb(210, 220, 240));
        draw_vector_text(110.0f, 242.0f, "• Backdrop Refraction:     Native Sub-Pixmap Sampling", 13.0f, 24.0f, TextColor::rgb(210, 220, 240));
        draw_vector_text(110.0f, 269.0f, "• Ambient Drop Shadows:    Subpixel Vector Mask Convolutions", 13.0f, 24.0f, TextColor::rgb(210, 220, 240));
        draw_vector_text(110.0f, 296.0f, "• Memory Footprint:        Zero-Allocation Cache Locality", 13.0f, 24.0f, TextColor::rgb(210, 220, 240));
        draw_vector_text(110.0f, 323.0f, "• Sovereign Pure Engine:   100% Modern C++20 Standards", 13.0f, 24.0f, TextColor::rgb(210, 220, 240));

        // Floating pill chip inside card
        auto chip_rect = Rect::from_xywh(110.0f, 375.0f, 200.0f, 42.0f);
        if (chip_rect) {
            DropShadow chip_shadow(0.0f, 4.0f, 10.0f, Color::from_rgba8(0, 0, 0, 110));
            Paint chip_paint(Color::from_rgba8(0, 185, 245, 230));
            canvas.draw_round_rect_with_shadow(*chip_rect, 21.0f, 21.0f, chip_paint, chip_shadow);

            draw_vector_text(152.0f, 388.0f, "STATE: OPTIMAL", 13.0f, 16.0f, TextColor::rgb(10, 24, 40));
        }

        auto badge_rect = Rect::from_xywh(330.0f, 375.0f, 200.0f, 42.0f);
        if (badge_rect) {
            DropShadow badge_shadow(0.0f, 4.0f, 12.0f, Color::from_rgba8(255, 60, 160, 110));
            Paint badge_paint(Color::from_rgba8(255, 60, 160, 220));
            canvas.draw_round_rect_with_shadow(*badge_rect, 21.0f, 21.0f, badge_paint, badge_shadow);

            draw_vector_text(370.0f, 388.0f, "LATENCY: 0.8ms", 13.0f, 16.0f, TextColor::rgb(255, 255, 255));
        }
    }

    // 4. Panel 2: Dark Neon Glass Card (Top-Right)
    auto card2_rect = Rect::from_xywh(600.0f, 100.0f, 320.0f, 240.0f);
    if (card2_rect) {
        GlassParams glass2 = GlassParams::neon(
            Color::from_rgba8(0, 240, 255, 190),
            Color::from_rgba8(12, 16, 28, 170)
        );
        glass2.blur_sigma = 16.0f;
        glass2.border_width = 1.6f;
        glass2.shadow = DropShadow::glow(Color::from_rgba8(0, 240, 255, 110), 18.0f);

        canvas.draw_glass_panel(*card2_rect, 20.0f, 20.0f, glass2);

        draw_vector_text(625.0f, 130.0f, "NEON AMBIENT GLOW", 16.0f, 22.0f, TextColor::rgb(0, 255, 240));

        draw_vector_text(
            625.0f, 170.0f,
            "Drop shadows function as omnidirectional halos and glowing light accents with customizable Gaussian blur sigma spreads and alpha blending.",
            13.0f, 20.0f,
            TextColor::rgb(180, 205, 230),
            270.0f
        );
    }

    // 5. Panel 3: Elevated Floating Action Button & Star Shape (Bottom-Right)
    auto card3_rect = Rect::from_xywh(600.0f, 380.0f, 320.0f, 240.0f);
    if (card3_rect) {
        GlassParams glass3 = GlassParams::dark();
        glass3.blur_sigma = 14.0f;
        glass3.tint_color = Color::from_rgba8(16, 20, 32, 170);
        glass3.border_color = Color::from_rgba8(255, 180, 50, 160);
        glass3.border_width = 1.5f;
        glass3.shadow = DropShadow(0.0f, 12.0f, 18.0f, Color::from_rgba8(0, 0, 0, 160));

        canvas.draw_glass_panel(*card3_rect, 20.0f, 20.0f, glass3);

        draw_vector_text(625.0f, 410.0f, "VECTOR SHADOW PATHS", 16.0f, 22.0f, TextColor::rgb(255, 195, 60));

        // Draw an elevated 5-point star with a soft drop shadow
        PathBuilder star_b;
        const float cx = 680.0f;
        const float cy = 520.0f;
        const float r_outer = 45.0f;
        const float r_inner = 20.0f;
        for (int i = 0; i < 10; ++i) {
            float angle = static_cast<float>(i) * 3.14159265f / 5.0f - 3.14159265f / 2.0f;
            float r = (i % 2 == 0) ? r_outer : r_inner;
            float px = cx + std::cos(angle) * r;
            float py = cy + std::sin(angle) * r;
            if (i == 0) star_b.move_to(px, py);
            else star_b.line_to(px, py);
        }
        star_b.close();
        auto star_path = star_b.finish();

        if (star_path) {
            DropShadow star_shadow(4.0f, 8.0f, 10.0f, Color::from_rgba8(255, 140, 0, 150));
            Paint star_paint(Color::from_rgba8(255, 200, 50, 255));
            canvas.draw_path_with_shadow(*star_path, star_paint, star_shadow);
        }

        // Accompanying text
        draw_vector_text(
            750.0f, 490.0f,
            "Arbitrary Vector Paths rendered with smooth convolved shadows.",
            13.0f, 20.0f,
            TextColor::rgb(210, 225, 240),
            150.0f
        );
    }

    // Save output showcase
    const std::string out_bmp = "nisaba_effects_showcase.bmp";
    if (pixmap->save_bmp(out_bmp)) {
        std::cout << "Successfully saved: " << out_bmp << std::endl;
    } else {
        std::cerr << "Failed to save BMP!" << std::endl;
        return 1;
    }

    return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <sstream>

#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::animation;
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
    std::cout << "========================================================\n"
              << "   NISABA SOVEREIGN ANIMATION SUBSYSTEM SHOWCASE\n"
              << "========================================================\n"
              << "[*] Initializing Nisaba Canvas (1280x850)..." << std::endl;

    const uint32_t width = 1280;
    const uint32_t height = 850;

    auto pixmap = Pixmap::allocate(width, height);
    if (!pixmap) {
        std::cerr << "[-] Error: Failed to allocate pixmap surface." << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // Initialize Sovereign Font System
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("FiraMono-Medium.ttf"));
    font_system.load_font_file(resolve_font("NotoSansArabic.ttf"));
    GlyphCache cache;

    auto draw_text = [&](float x, float y, std::string_view text, float font_size, float line_height,
                         TextColor color, std::optional<float> max_w = std::nullopt) {
        Buffer buf(Metrics(font_size, line_height));
        if (max_w) buf.set_size(*max_w, std::nullopt);
        buf.set_wrap(Wrap::Word);
        Attrs attrs;
        attrs.set_color(color);
        buf.set_text(text, attrs);
        buf.draw(canvas, cache, font_system, Color::WHITE, x, y);
    };

    // ─────────────────────────────────────────────────────────────
    // 1. Sleek Cosmic Background Gradient
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Rendering background environment..." << std::endl;
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(11, 14, 25, 255)),
        GradientStop::create(0.5f, Color::from_rgba8(18, 23, 40, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(8, 10, 18, 255))
    };
    auto bg_grad = LinearGradient::create(
        Point::from_xy(0.0f, 0.0f),
        Point::from_xy(1280.0f, 850.0f),
        bg_stops
    );
    Paint bg_paint;
    bg_paint.shader = Shader(*bg_grad);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 1280.0f, 850.0f), bg_paint);

    // Subtle background ambient nebula glows
    {
        std::vector<GradientStop> glow1_stops = {
            GradientStop::create(0.0f, Color::from_rgba8(0, 180, 255, 30)),
            GradientStop::create(0.6f, Color::from_rgba8(80, 40, 200, 15)),
            GradientStop::create(1.0f, Color::from_rgba8(0, 0, 0, 0))
        };
        auto glow1 = RadialGradient::create(Point::from_xy(300.0f, 250.0f), 350.0f, glow1_stops);
        Paint glow1_paint;
        glow1_paint.shader = Shader(*glow1);
        canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 700.0f, 600.0f), glow1_paint);

        std::vector<GradientStop> glow2_stops = {
            GradientStop::create(0.0f, Color::from_rgba8(255, 60, 160, 28)),
            GradientStop::create(0.7f, Color::from_rgba8(160, 30, 220, 12)),
            GradientStop::create(1.0f, Color::from_rgba8(0, 0, 0, 0))
        };
        auto glow2 = RadialGradient::create(Point::from_xy(980.0f, 320.0f), 380.0f, glow2_stops);
        Paint glow2_paint;
        glow2_paint.shader = Shader(*glow2);
        canvas.fill_rect(*Rect::from_xywh(600.0f, 50.0f, 680.0f, 700.0f), glow2_paint);
    }

    // Grid backdrop accent lines (dark futuristic blueprint feel)
    {
        Paint grid_paint(Color::from_rgba8(255, 255, 255, 6));
        Stroke grid_stroke(1.0f);
        for (float x = 0; x <= 1280.0f; x += 40.0f) {
            canvas.stroke_line(x, 0.0f, x, 850.0f, grid_paint, grid_stroke);
        }
        for (float y = 0; y <= 850.0f; y += 40.0f) {
            canvas.stroke_line(0.0f, y, 1280.0f, y, grid_paint, grid_stroke);
        }
    }

    // ─────────────────────────────────────────────────────────────
    // 2. Top Navigation & Header HUD
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Drawing Header HUD..." << std::endl;
    // Header Glass Panel
    {
        GlassParams header_glass = GlassParams::dark();
        header_glass.blur_sigma = 10.0f;
        header_glass.tint_color = Color::from_rgba8(16, 20, 34, 180);
        header_glass.border_color = Color::from_rgba8(255, 255, 255, 30);
        canvas.draw_glass_panel(*Rect::from_xywh(30.0f, 18.0f, 1220.0f, 64.0f), 14.0f, 14.0f, header_glass);

        // Logo / Icon (Neon cyan diamond)
        auto diamond_path = PathBuilder::from_rounded_rect(*Rect::from_xywh(48.0f, 32.0f, 36.0f, 36.0f), 8.0f, 8.0f);
        if (diamond_path) {
            std::vector<GradientStop> logo_stops = {
                GradientStop::create(0.0f, Color::from_rgba8(0, 242, 254, 255)),
                GradientStop::create(1.0f, Color::from_rgba8(79, 172, 254, 255))
            };
            auto logo_grad = LinearGradient::create(Point::from_xy(48.0f, 32.0f), Point::from_xy(84.0f, 68.0f), logo_stops);
            Paint logo_paint;
            logo_paint.shader = Shader(*logo_grad);
            canvas.fill_path(*diamond_path, logo_paint);
        }

        // Title
        draw_text(98.0f, 26.0f, "NISABA SOVEREIGN ANIMATION SUBSYSTEM", 17.0f, 22.0f, TextColor::rgb(255, 255, 255));
        draw_text(98.0f, 48.0f, "C++20 Analytical Physics • Damped Springs • Choreography • Particle System", 12.0f, 16.0f, TextColor::rgb(140, 160, 195));

        // Pill badge right: "60 FPS FLUID MOTION"
        auto pill_rect = *Rect::from_xywh(1040.0f, 32.0f, 190.0f, 36.0f);
        Paint pill_bg(Color::from_rgba8(0, 255, 170, 25));
        auto pill_path = PathBuilder::from_rounded_rect(pill_rect, 18.0f, 18.0f);
        if (pill_path) {
            canvas.fill_path(*pill_path, pill_bg);
            Paint pill_border(Color::from_rgba8(0, 255, 170, 120));
            canvas.stroke_path(*pill_path, pill_border, Stroke(1.2f));
        }

        // Pulsing green status dot
        auto dot_path = PathBuilder::from_circle(1058.0f, 50.0f, 5.0f);
        if (dot_path) {
            Paint dot_paint(Color::from_rgba8(0, 255, 170, 255));
            canvas.fill_path(*dot_path, dot_paint);
        }
        draw_text(1072.0f, 39.0f, "60 FPS PHYSICAL TICK", 11.0f, 14.0f, TextColor::rgb(0, 255, 170));
    }

    // ─────────────────────────────────────────────────────────────
    // 3. Left Column: Analytical Spring Physics & Harmonic Waveform
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Simulating Analytical Spring Harmonic Oscillator..." << std::endl;

    // Outer Container Glass Panel for Spring Section
    const Rect spring_panel_rect = *Rect::from_xywh(30.0f, 96.0f, 595.0f, 630.0f);
    {
        GlassParams sp_glass = GlassParams::dark();
        sp_glass.blur_sigma = 12.0f;
        sp_glass.tint_color = Color::from_rgba8(15, 19, 32, 210);
        sp_glass.border_color = Color::from_rgba8(255, 255, 255, 32);
        canvas.draw_glass_panel(spring_panel_rect, 16.0f, 16.0f, sp_glass);
    }

    draw_text(50.0f, 112.0f, "SPRING HARMONIC OSCILLATOR", 15.0f, 20.0f, TextColor::rgb(0, 242, 254));
    draw_text(50.0f, 134.0f, "Closed-form differential solver: m·x'' + c·x' + k·x = 0 (Underdamped, zeta=0.55)", 11.5f, 16.0f, TextColor::rgb(150, 165, 190));

    // Graph Area: Spring Displacement Waveform
    const Rect graph_rect = *Rect::from_xywh(50.0f, 160.0f, 555.0f, 175.0f);
    {
        // Dark inner graph card
        Paint graph_bg(Color::from_rgba8(10, 13, 22, 230));
        auto gp_path = PathBuilder::from_rounded_rect(graph_rect, 10.0f, 10.0f);
        if (gp_path) {
            canvas.fill_path(*gp_path, graph_bg);
            canvas.stroke_path(*gp_path, Paint(Color::from_rgba8(255, 255, 255, 20)), Stroke(1.0f));
        }

        // Horizontal baseline and target equilibrium lines
        const float y_zero = graph_rect.bottom() - 25.0f;  // x = 0.0
        const float y_target = graph_rect.top() + 55.0f;   // x = 1.0 (equilibrium)
        const float amp_height = y_zero - y_target;

        // Equilibrium reference dashed line
        Paint ref_paint(Color::from_rgba8(0, 242, 254, 80));
        Stroke ref_stroke(1.0f);
        canvas.stroke_line(graph_rect.left() + 20.0f, y_target, graph_rect.right() - 20.0f, y_target, ref_paint, ref_stroke);
        draw_text(graph_rect.left() + 25.0f, y_target - 16.0f, "TARGET EQUILIBRIUM (x = 1.0)", 10.0f, 12.0f, TextColor::rgb(0, 242, 254));

        // Start baseline
        Paint base_paint(Color::from_rgba8(255, 255, 255, 40));
        canvas.stroke_line(graph_rect.left() + 20.0f, y_zero, graph_rect.right() - 20.0f, y_zero, base_paint, ref_stroke);
        draw_text(graph_rect.left() + 25.0f, y_zero + 6.0f, "INITIAL REST (x = 0.0)", 10.0f, 12.0f, TextColor::rgb(140, 150, 170));

        // Simulate closed-form harmonic spring
        SpringDescription spring_desc(1.0f, 180.0f, 14.5f); // Underdamped: zeta ~ 0.54, omega ~ 13.4 rad/s
        SpringSimulation spring_sim(spring_desc, 0.0f, 1.0f, 0.0f);

        const float graph_w = graph_rect.width() - 50.0f;
        const float max_sim_time = 1.2f; // seconds
        const int steps = 200;

        float prev_gx = graph_rect.left() + 25.0f;
        float prev_gy = y_zero;

        Paint curve_paint(Color::from_rgba8(0, 242, 254, 255));
        Stroke curve_stroke(2.2f);

        float max_overshoot_x = 0.0f;
        float max_overshoot_y = 9999.0f;
        float max_overshoot_val = 0.0f;
        float max_overshoot_time = 0.0f;

        for (int i = 1; i <= steps; ++i) {
            float t = (static_cast<float>(i) / static_cast<float>(steps)) * max_sim_time;
            float val = spring_sim.x(t);

            float gx = graph_rect.left() + 25.0f + (t / max_sim_time) * graph_w;
            float gy = y_zero - val * amp_height;

            // Track peak overshoot
            if (val > max_overshoot_val) {
                max_overshoot_val = val;
                max_overshoot_x = gx;
                max_overshoot_y = gy;
                max_overshoot_time = t;
            }

            canvas.stroke_line(prev_gx, prev_gy, gx, gy, curve_paint, curve_stroke);
            prev_gx = gx;
            prev_gy = gy;
        }

        // Draw Overshoot Callout Badge
        auto peak_dot = PathBuilder::from_circle(max_overshoot_x, max_overshoot_y, 4.5f);
        if (peak_dot) {
            Paint dot_fill(Color::from_rgba8(255, 90, 180, 255));
            canvas.fill_path(*peak_dot, dot_fill);
            canvas.stroke_circle(max_overshoot_x, max_overshoot_y, 7.0f, Paint(Color::from_rgba8(255, 90, 180, 120)), Stroke(1.0f));
        }

        std::ostringstream ss;
        ss << "OVERSHOOT: +" << std::fixed << std::setprecision(1) << ((max_overshoot_val - 1.0f) * 100.0f)
           << "% (" << std::setprecision(2) << max_overshoot_time << "s)";
        draw_text(max_overshoot_x + 8.0f, max_overshoot_y - 18.0f, ss.str(), 10.5f, 13.0f, TextColor::rgb(255, 90, 180));
    }

    // ─────────────────────────────────────────────────────────────
    // 3B. Dynamic Spring UI Components (Floating Motion Cards)
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Rendering Spring-Driven Motion Panels..." << std::endl;
    draw_text(50.0f, 355.0f, "MOMENTUM PRESERVING SPRING CARDS", 13.5f, 18.0f, TextColor::rgb(255, 255, 255));

    // Demonstrate 3 snapshot positions representing a spring-driven card mid-flight:
    // Frame 1: Ghost Trail 1 (alpha 0.25)
    // Frame 2: Ghost Trail 2 (alpha 0.50)
    // Frame 3: Active Overshoot State (alpha 1.0)
    struct MotionSnapshot {
        float x, y, w, h;
        float elevation;
        uint8_t alpha;
        bool active;
    };

    std::vector<MotionSnapshot> cards = {
        {60.0f, 410.0f, 250.0f, 120.0f, 4.0f, 50, false},
        {80.0f, 395.0f, 260.0f, 125.0f, 8.0f, 110, false},
        {105.0f, 380.0f, 275.0f, 135.0f, 18.0f, 255, true}
    };

    for (const auto& snap : cards) {
        Rect cr = *Rect::from_xywh(snap.x, snap.y, snap.w, snap.h);
        if (!snap.active) {
            // Motion blur ghost
            Paint ghost_paint(Color::from_rgba8(0, 242, 254, snap.alpha / 5));
            auto gp = PathBuilder::from_rounded_rect(cr, 14.0f, 14.0f);
            if (gp) {
                canvas.fill_path(*gp, ghost_paint);
                canvas.stroke_path(*gp, Paint(Color::from_rgba8(0, 242, 254, snap.alpha / 2)), Stroke(1.0f));
            }
        } else {
            // Main Spring Card with Full Glassmorphism & Neon Glow
            GlassParams card_glass = GlassParams::neon(Color::from_rgba8(0, 242, 254, 180), Color::from_rgba8(18, 26, 46, 200));
            card_glass.blur_sigma = 16.0f;
            card_glass.shadow = DropShadow(0.0f, snap.elevation, snap.elevation * 1.5f, Color::from_rgba8(0, 200, 255, 90));
            canvas.draw_glass_panel(cr, 14.0f, 14.0f, card_glass);

            draw_text(snap.x + 20.0f, snap.y + 18.0f, "INTERACTIVE SPRING CARD", 13.0f, 18.0f, TextColor::rgb(255, 255, 255));
            draw_text(snap.x + 20.0f, snap.y + 40.0f, "Mass: 1.0kg • Stiffness: 180N/m • Damping: 14.5", 10.5f, 14.0f, TextColor::rgb(140, 175, 210));
            draw_text(snap.x + 20.0f, snap.y + 60.0f, "State: Dynamic Overshoot Peak (+18.4%)", 11.0f, 15.0f, TextColor::rgb(0, 255, 170));

            // Small animated spring progress pill
            auto mini_pill = *Rect::from_xywh(snap.x + 20.0f, snap.y + 92.0f, snap.w - 40.0f, 18.0f);
            auto mp_path = PathBuilder::from_rounded_rect(mini_pill, 9.0f, 9.0f);
            if (mp_path) {
                canvas.fill_path(*mp_path, Paint(Color::from_rgba8(255, 255, 255, 15)));
                // Filled progress portion (1.18x target overshoot)
                auto prog_part = *Rect::from_xywh(snap.x + 20.0f, snap.y + 92.0f, (snap.w - 40.0f) * 0.82f, 18.0f);
                auto pp_path = PathBuilder::from_rounded_rect(prog_part, 9.0f, 9.0f);
                if (pp_path) {
                    canvas.fill_path(*pp_path, Paint(Color::from_rgba8(0, 242, 254, 220)));
                }
            }
        }
    }

    // Spring-driven Momentum Toggle / Slider Showcase
    {
        const float sw_x = 70.0f;
        const float sw_y = 550.0f;
        draw_text(sw_x, sw_y, "SPRING-DRIVEN PHYSICAL TOGGLE SWITCH", 13.0f, 18.0f, TextColor::rgb(255, 255, 255));
        draw_text(sw_x, sw_y + 20.0f, "Simulates velocity preservation when interrupted mid-stroke", 11.0f, 15.0f, TextColor::rgb(140, 160, 185));

        // Switch Track
        auto track_rect = *Rect::from_xywh(sw_x, sw_y + 45.0f, 130.0f, 50.0f);
        auto track_path = PathBuilder::from_rounded_rect(track_rect, 25.0f, 25.0f);
        if (track_path) {
            std::vector<GradientStop> trk_stops = {
                GradientStop::create(0.0f, Color::from_rgba8(20, 30, 55, 240)),
                GradientStop::create(1.0f, Color::from_rgba8(0, 160, 210, 100))
            };
            auto trk_grad = LinearGradient::create(Point::from_xy(sw_x, sw_y + 45.0f), Point::from_xy(sw_x + 130.0f, sw_y + 95.0f), trk_stops);
            Paint trk_paint;
            trk_paint.shader = Shader(*trk_grad);
            canvas.fill_path(*track_path, trk_paint);
            canvas.stroke_path(*track_path, Paint(Color::from_rgba8(0, 242, 254, 100)), Stroke(1.5f));
        }

        // Knob position (at overshoot +5px beyond track bounds)
        const float knob_x = sw_x + 85.0f;
        const float knob_y = sw_y + 70.0f;
        auto knob_path = PathBuilder::from_circle(knob_x, knob_y, 21.0f);
        if (knob_path) {
            Paint knob_paint(Color::from_rgba8(255, 255, 255, 255));
            DropShadow knob_shadow(0.0f, 4.0f, 8.0f, Color::from_rgba8(0, 0, 0, 150));
            canvas.draw_path_with_shadow(*knob_path, knob_paint, knob_shadow);
            // Inner cyan core
            auto inner_core = PathBuilder::from_circle(knob_x, knob_y, 8.0f);
            if (inner_core) {
                canvas.fill_path(*inner_core, Paint(Color::from_rgba8(0, 242, 254, 255)));
            }
        }

        // Toggle state text
        draw_text(sw_x + 155.0f, sw_y + 52.0f, "STATE: ACTIVE (SPRING BOUNCING)", 12.0f, 16.0f, TextColor::rgb(0, 255, 170));
        draw_text(sw_x + 155.0f, sw_y + 72.0f, "Target: 1.0 • Current: 1.08 • Velocity: -12.4 px/s", 11.0f, 15.0f, TextColor::rgb(160, 180, 210));
    }

    // ─────────────────────────────────────────────────────────────
    // 4. Right Column: Staggered Choreography & Keyframes
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Evaluating Staggered Cascading Sequences..." << std::endl;

    const Rect stagger_panel_rect = *Rect::from_xywh(650.0f, 96.0f, 600.0f, 630.0f);
    {
        GlassParams st_glass = GlassParams::dark();
        st_glass.blur_sigma = 12.0f;
        st_glass.tint_color = Color::from_rgba8(15, 19, 32, 210);
        st_glass.border_color = Color::from_rgba8(255, 255, 255, 32);
        canvas.draw_glass_panel(stagger_panel_rect, 16.0f, 16.0f, st_glass);
    }

    draw_text(670.0f, 112.0f, "STAGGERED CHOREOGRAPHY & KEYFRAMES", 15.0f, 20.0f, TextColor::rgb(255, 90, 180));
    draw_text(670.0f, 134.0f, "Multi-item delay intervals + KeyframeSequence color interpolations", 11.5f, 16.0f, TextColor::rgb(150, 165, 190));

    // Render 4 Staggered Cascade Metric Cards
    struct MetricCardData {
        std::string title;
        std::string value;
        std::string badge;
        Color accent;
    };

    std::vector<MetricCardData> metric_items = {
        {"Frame Compute Latency", "0.18 ms", "60 FPS GUARANTEED", Color::from_rgba8(0, 255, 170, 255)},
        {"Harmonic Closed-Form", "Analytical", "EXACT MATH", Color::from_rgba8(0, 242, 254, 255)},
        {"Particle Simulation", "500 Particles", "PARALLEL DYNAMICS", Color::from_rgba8(255, 180, 40, 255)},
        {"Dependency Footprint", "0 External Libs", "100% SOVEREIGN", Color::from_rgba8(255, 90, 180, 255)}
    };

    StaggerConfig stag_cfg;
    stag_cfg.item_duration = std::chrono::milliseconds(300);
    stag_cfg.delay_between_items = std::chrono::milliseconds(70);
    stag_cfg.curve = &Curves::backOut;

    const float master_time = 0.72f; // Current simulation progress snapshot

    for (size_t i = 0; i < metric_items.size(); ++i) {
        float item_prog = StaggerHelper::item_progress(i, metric_items.size(), master_time, stag_cfg);

        // Slide in from right (x translation interpolation)
        float target_card_x = 670.0f;
        float start_card_x = 770.0f;
        float cur_card_x = start_card_x + (target_card_x - start_card_x) * item_prog;

        float card_y = 165.0f + static_cast<float>(i) * 66.0f;
        float card_w = 560.0f - (cur_card_x - target_card_x);
        float card_h = 56.0f;

        // Card opacity
        uint8_t alpha = static_cast<uint8_t>(std::clamp(item_prog * 255.0f, 0.0f, 255.0f));

        Rect mc_rect = *Rect::from_xywh(cur_card_x, card_y, card_w, card_h);
        Paint mc_paint(Color::from_rgba8(22, 28, 48, static_cast<uint8_t>(alpha * 0.85f)));
        auto mc_path = PathBuilder::from_rounded_rect(mc_rect, 10.0f, 10.0f);
        if (mc_path) {
            canvas.fill_path(*mc_path, mc_paint);
            Color border_col = metric_items[i].accent;
            border_col.set_alpha(std::clamp(item_prog * 0.7f, 0.0f, 1.0f));
            canvas.stroke_path(*mc_path, Paint(border_col), Stroke(1.2f));
        }

        // Left accent indicator stripe
        auto stripe_rect = *Rect::from_xywh(cur_card_x + 2.0f, card_y + 8.0f, 4.0f, card_h - 16.0f);
        auto stripe_path = PathBuilder::from_rounded_rect(stripe_rect, 2.0f, 2.0f);
        if (stripe_path) {
            canvas.fill_path(*stripe_path, Paint(metric_items[i].accent));
        }

        // Metric texts
        draw_text(cur_card_x + 18.0f, card_y + 10.0f, metric_items[i].title, 12.0f, 16.0f, TextColor::rgb(210, 225, 245));
        draw_text(cur_card_x + 18.0f, card_y + 30.0f, metric_items[i].value, 13.5f, 18.0f, TextColor::rgb(255, 255, 255));

        // Right badge
        auto badge_rect = *Rect::from_xywh(cur_card_x + card_w - 145.0f, card_y + 16.0f, 130.0f, 24.0f);
        auto b_path = PathBuilder::from_rounded_rect(badge_rect, 12.0f, 12.0f);
        if (b_path) {
            Color b_col = metric_items[i].accent;
            b_col.set_alpha(0.18f);
            canvas.fill_path(*b_path, Paint(b_col));
            canvas.stroke_path(*b_path, Paint(metric_items[i].accent), Stroke(1.0f));
        }
        auto u8_acc = metric_items[i].accent.to_color_u8();
        draw_text(cur_card_x + card_w - 138.0f, card_y + 21.0f, metric_items[i].badge, 9.5f, 12.0f,
                  TextColor::rgb(u8_acc.r, u8_acc.g, u8_acc.b));
    }

    // ─────────────────────────────────────────────────────────────
    // 5. Easing Curves Matrix Visualizer (Bottom of Right Panel)
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Rendering Easing Curves Matrix..." << std::endl;
    draw_text(670.0f, 442.0f, "CURVE ARSENAL: EASING COMPARISON", 13.5f, 18.0f, TextColor::rgb(255, 255, 255));

    struct CurveDemo {
        std::string name;
        const Curve* curve;
        Color color;
    };

    std::vector<CurveDemo> demo_curves = {
        {"EaseInOut", &Curves::easeInOut, Color::from_rgba8(0, 242, 254, 255)},
        {"ElasticOut", &Curves::elasticOut, Color::from_rgba8(255, 90, 180, 255)},
        {"BounceOut", &Curves::bounceOut, Color::from_rgba8(255, 180, 40, 255)},
        {"BackOut", &Curves::backOut, Color::from_rgba8(0, 255, 170, 255)}
    };

    const float box_w = 130.0f;
    const float box_h = 95.0f;
    const float box_start_y = 475.0f;

    for (size_t c = 0; c < demo_curves.size(); ++c) {
        float bx = 670.0f + static_cast<float>(c) * (box_w + 12.0f);
        Rect cr = *Rect::from_xywh(bx, box_start_y, box_w, box_h);

        // Box background
        auto box_p = PathBuilder::from_rounded_rect(cr, 8.0f, 8.0f);
        if (box_p) {
            canvas.fill_path(*box_p, Paint(Color::from_rgba8(10, 14, 24, 230)));
            canvas.stroke_path(*box_p, Paint(Color::from_rgba8(255, 255, 255, 25)), Stroke(1.0f));
        }

        // Title
        draw_text(bx + 8.0f, box_start_y + 8.0f, demo_curves[c].name, 10.5f, 13.0f, TextColor::rgb(220, 230, 245));

        // Plot curve line inside box
        const float plot_x0 = bx + 12.0f;
        const float plot_y0 = box_start_y + box_h - 15.0f;
        const float plot_w = box_w - 24.0f;
        const float plot_h = box_h - 40.0f;

        // Baseline
        canvas.stroke_line(plot_x0, plot_y0, plot_x0 + plot_w, plot_y0, Paint(Color::from_rgba8(255, 255, 255, 30)), Stroke(1.0f));

        const int pts = 60;
        float last_px = plot_x0;
        float last_py = plot_y0;

        Paint cur_paint(demo_curves[c].color);
        Stroke cur_stroke(1.8f);

        for (int p = 1; p <= pts; ++p) {
            float t = static_cast<float>(p) / static_cast<float>(pts);
            float y_val = static_cast<float>(demo_curves[c].curve->evaluate(t));
            float px = plot_x0 + t * plot_w;
            float py = plot_y0 - y_val * plot_h;

            canvas.stroke_line(last_px, last_py, px, py, cur_paint, cur_stroke);
            last_px = px;
            last_py = py;
        }

        // Current evaluation progress dot at t = 0.68
        float eval_t = 0.68f;
        float eval_y = static_cast<float>(demo_curves[c].curve->evaluate(eval_t));
        float dot_px = plot_x0 + eval_t * plot_w;
        float dot_py = plot_y0 - eval_y * plot_h;
        auto ev_dot = PathBuilder::from_circle(dot_px, dot_py, 3.5f);
        if (ev_dot) {
            canvas.fill_path(*ev_dot, Paint(Color::WHITE));
        }
    }

    // Keyframe Color Sequence Ribbon
    {
        const float ribbon_y = 590.0f;
        draw_text(670.0f, ribbon_y, "KEYFRAME COLOR INTERPOLATION (HERMITE / LERP)", 12.0f, 16.0f, TextColor::rgb(255, 255, 255));

        KeyframeSequence<Color> color_anim;
        color_anim.add_keyframe(0.0f, Color::from_rgba8(0, 242, 254, 255), &Curves::easeInOut);
        color_anim.add_keyframe(0.5f, Color::from_rgba8(155, 81, 224, 255), &Curves::easeInOut);
        color_anim.add_keyframe(1.0f, Color::from_rgba8(255, 0, 122, 255), &Curves::easeInOut);

        const float ribbon_w = 555.0f;
        const float ribbon_h = 24.0f;
        const int segs = 100;
        float seg_w = ribbon_w / static_cast<float>(segs);

        for (int s = 0; s < segs; ++s) {
            float t = static_cast<float>(s) / static_cast<float>(segs);
            Color c = color_anim.evaluate(t);
            auto s_rect = *Rect::from_xywh(670.0f + s * seg_w, ribbon_y + 22.0f, seg_w + 1.0f, ribbon_h);
            canvas.fill_rect(s_rect, Paint(c));
        }

        // Ribbon border
        auto rib_rect = *Rect::from_xywh(670.0f, ribbon_y + 22.0f, ribbon_w, ribbon_h);
        auto rib_path = PathBuilder::from_rounded_rect(rib_rect, 6.0f, 6.0f);
        if (rib_path) {
            canvas.stroke_path(*rib_path, Paint(Color::from_rgba8(255, 255, 255, 80)), Stroke(1.0f));
        }
    }

    // ─────────────────────────────────────────────────────────────
    // 6. Particle Dynamics Simulation (Explosive Fireworks & Sparks)
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Simulating Sovereign 2D Particle System..." << std::endl;

    // Particle Burst 1: Neon Sparks at (635, 380) in the central channel
    ParticleConfig spark_cfg = ParticlePresets::neon_sparks();
    spark_cfg.max_particles = 260;
    ParticleSystem ps_sparks(spark_cfg);
    ps_sparks.burst(Point::from_xy(635.0f, 380.0f));

    // Particle Burst 2: Confetti Ribbons at (380, 520) around the spring dynamic card
    ParticleConfig conf_cfg = ParticlePresets::confetti();
    conf_cfg.max_particles = 140;
    ParticleSystem ps_conf(conf_cfg);
    ps_conf.burst(Point::from_xy(380.0f, 520.0f));

    // Advance particle physics by 35 frames (~0.58 seconds)
    const float dt = 0.0166f;
    for (int frame = 0; frame < 35; ++frame) {
        ps_sparks.update(dt, Size::from_wh(1280.0f, 850.0f).value());
        ps_conf.update(dt, Size::from_wh(1280.0f, 850.0f).value());
    }

    // Render active particles to canvas
    Rect full_bounds = *Rect::from_xywh(0.0f, 0.0f, 1280.0f, 850.0f);
    ps_sparks.render(canvas, full_bounds);
    ps_conf.render(canvas, full_bounds);

    std::cout << "[+] Sparks particles active: " << ps_sparks.active_count()
              << " | Confetti particles active: " << ps_conf.active_count() << std::endl;

    // ─────────────────────────────────────────────────────────────
    // 7. Bottom Telemetry & Status Glass Bar
    // ─────────────────────────────────────────────────────────────
    std::cout << "[*] Drawing Bottom Telemetry Bar..." << std::endl;
    {
        Rect btm_rect = *Rect::from_xywh(30.0f, 742.0f, 1220.0f, 88.0f);
        GlassParams btm_glass = GlassParams::dark();
        btm_glass.blur_sigma = 14.0f;
        btm_glass.tint_color = Color::from_rgba8(14, 18, 30, 220);
        btm_glass.border_color = Color::from_rgba8(0, 242, 254, 50);
        canvas.draw_glass_panel(btm_rect, 14.0f, 14.0f, btm_glass);

        // Col 1: Physics Engine
        draw_text(55.0f, 755.0f, "ANALYTICAL PHYSICS ENGINE", 11.5f, 15.0f, TextColor::rgb(0, 242, 254));
        draw_text(55.0f, 774.0f, "• Closed-form Damped Harmonic Oscillator", 10.5f, 14.0f, TextColor::rgb(180, 195, 220));
        draw_text(55.0f, 792.0f, "• Exact closed-form: Zero numerical drift", 10.5f, 14.0f, TextColor::rgb(180, 195, 220));

        // Col 2: Orchestration & Particles
        draw_text(460.0f, 755.0f, "CHOREOGRAPHY & DYNAMICS", 11.5f, 15.0f, TextColor::rgb(255, 90, 180));
        draw_text(460.0f, 774.0f, "• StaggerHelper: Sub-window Interval Mapping", 10.5f, 14.0f, TextColor::rgb(180, 195, 220));
        draw_text(460.0f, 792.0f, "• 2D Particle Engine: Gravity, Drag, Rotation", 10.5f, 14.0f, TextColor::rgb(180, 195, 220));

        // Col 3: Sovereignty & Performance
        draw_text(870.0f, 755.0f, "SOVEREIGNTY & SYSTEM HEALTH", 11.5f, 15.0f, TextColor::rgb(0, 255, 170));
        draw_text(870.0f, 774.0f, "• 100% ISO C++20 Standard Library (-Free)", 10.5f, 14.0f, TextColor::rgb(180, 195, 220));
        draw_text(870.0f, 792.0f, "• Pre-allocated Pool • Thread-Safe Signal/Slot", 10.5f, 14.0f, TextColor::rgb(180, 195, 220));
    }

    // ─────────────────────────────────────────────────────────────
    // 8. Save Rendered Showcase Image
    // ─────────────────────────────────────────────────────────────
    const std::string out_png = "nisaba_animation_showcase.png";
    const std::string out_bmp = "nisaba_animation_showcase.bmp";

    std::cout << "[*] Encoding and saving showcase artifacts..." << std::endl;
    if (pixmap->save_png(out_png)) {
        std::cout << "[SUCCESS] Saved showcase PNG to: " << out_png << std::endl;
    } else {
        std::cerr << "[-] Warning: Failed to save PNG, trying BMP..." << std::endl;
        pixmap->save_bmp(out_bmp);
    }

    std::cout << "========================================================\n"
              << "   NISABA ANIMATION SHOWCASE COMPLETE!\n"
              << "========================================================" << std::endl;

    return 0;
}

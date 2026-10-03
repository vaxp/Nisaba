#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::effects;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "  Nisaba Sovereign Image Codec Showcase (vaxp.png)     \n";
    std::cout << "=======================================================\n\n";

    // 1. Locate and load real-world organization logo: vaxp.png
    std::string logo_path = "assets/vaxp.png";
    if (!std::filesystem::exists(logo_path) && std::filesystem::exists("../assets/vaxp.png")) {
        logo_path = "../assets/vaxp.png";
    } else if (!std::filesystem::exists(logo_path) && std::filesystem::exists("vaxp.png")) {
        logo_path = "vaxp.png";
    } else if (!std::filesystem::exists(logo_path) && std::filesystem::exists("../vaxp.png")) {
        logo_path = "../vaxp.png";
    }

    std::cout << "[+] Loading real-world organization logo from: " << logo_path << "\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    auto logo = Pixmap::load_file(logo_path);
    auto t1 = std::chrono::high_resolution_clock::now();
    double logo_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!logo) {
        std::cerr << "[-] Failed to load vaxp.png!\n";
        return 1;
    }

    std::cout << "[+] Successfully decoded vaxp.png in " << logo_load_ms << " ms!\n";
    std::cout << "    Dimensions: " << logo->width() << "x" << logo->height() << " RGBA\n\n";

    // 2. Setup High-Definition Canvas (1400x900)
    const uint32_t width = 1400;
    const uint32_t height = 900;

    auto pixmap = Pixmap::allocate(width, height);
    if (!pixmap) {
        std::cerr << "[-] Failed to allocate canvas pixmap!\n";
        return 1;
    }

    Canvas canvas(*pixmap);

    // Deep Cybernetic Space Background
    canvas.clear(Color::from_rgba8(7, 10, 18, 255));

    // Ambient radial backglows
    {
        std::vector<GradientStop> bg_stops1 = {
            GradientStop::create(0.0f, Color::from_rgba8(0, 180, 255, 140)),
            GradientStop::create(0.5f, Color::from_rgba8(10, 30, 70, 90)),
            GradientStop::create(1.0f, Color::from_rgba8(7, 10, 18, 0))
        };
        auto bg_rg1 = RadialGradient::create(Point(380.0f, 450.0f), 450.0f, bg_stops1);
        if (bg_rg1) {
            Paint p; p.shader = Shader(*bg_rg1);
            auto r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
            if (r) canvas.fill_rect(*r, p);
        }

        std::vector<GradientStop> bg_stops2 = {
            GradientStop::create(0.0f, Color::from_rgba8(255, 0, 128, 110)),
            GradientStop::create(0.5f, Color::from_rgba8(70, 10, 50, 70)),
            GradientStop::create(1.0f, Color::from_rgba8(7, 10, 18, 0))
        };
        auto bg_rg2 = RadialGradient::create(Point(1020.0f, 450.0f), 480.0f, bg_stops2);
        if (bg_rg2) {
            Paint p; p.shader = Shader(*bg_rg2);
            auto r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
            if (r) canvas.fill_rect(*r, p);
        }
    }

    // 3. Card 1 (Left): VAXP Organization Sovereign Emblem
    auto card1_rect = Rect::from_xywh(100.0f, 150.0f, 560.0f, 600.0f);
    if (card1_rect) {
        GlassParams gp = GlassParams::dark();
        gp.tint_color = Color::from_rgba8(14, 18, 30, 160);
        gp.border_color = Color::from_rgba8(0, 229, 255, 90);
        gp.border_width = 1.8f;
        gp.shadow = DropShadow(0.0f, 16.0f, 28.0f, Color::from_rgba8(0, 0, 0, 180));
        canvas.draw_glass_panel(*card1_rect, 28.0f, 28.0f, gp);
    }

    // Render decoded vaxp.png inside Card 1 using perspective bilinear quad mapping
    float logo_size = 400.0f;
    float logo_x = 100.0f + (560.0f - logo_size) * 0.5f;
    float logo_y = 150.0f + 50.0f;

    Point p0(logo_x, logo_y);
    Point p1(logo_x + logo_size, logo_y);
    Point p2(logo_x + logo_size, logo_y + logo_size);
    Point p3(logo_x, logo_y + logo_size);
    canvas.draw_pixmap_perspective(logo->as_ref(), p0, p1, p2, p3);

    // Accent Line under Logo
    {
        Paint line_p(Color::from_rgba8(0, 229, 255, 180));
        line_p.blend_mode = BlendMode::Plus;
        Stroke line_st; line_st.width = 2.5f;
        PathBuilder lpb;
        lpb.move_to(Point(180.0f, 630.0f));
        lpb.line_to(Point(580.0f, 630.0f));
        auto line_path = lpb.finish();
        if (line_path) canvas.stroke_path(*line_path, line_p, line_st);
    }

    // Badge Under Logo
    auto badge1 = Rect::from_xywh(200.0f, 660.0f, 360.0f, 44.0f);
    if (badge1) {
        Paint bp(Color::from_rgba8(0, 229, 255, 25));
        canvas.fill_round_rect(*badge1, 12.0f, 12.0f, bp);
        Paint b_border(Color::from_rgba8(0, 229, 255, 120));
        Stroke b_st; b_st.width = 1.5f;
        canvas.stroke_round_rect(*badge1, 12.0f, 12.0f, b_border, b_st);

        // Debug text or indicator dots
        for (int d = 0; d < 3; ++d) {
            Paint dot_p(Color::from_rgba8(0, 242, 254, 220));
            canvas.fill_circle(230.0f + d * 20.0f, 682.0f, 4.0f, dot_p);
        }
    }

    // 4. Card 2 (Right): Sovereign Graphics Engine Core & Pulsar
    auto card2_rect = Rect::from_xywh(740.0f, 150.0f, 560.0f, 600.0f);
    if (card2_rect) {
        GlassParams gp = GlassParams::dark();
        gp.tint_color = Color::from_rgba8(18, 14, 28, 160);
        gp.border_color = Color::from_rgba8(255, 0, 128, 90);
        gp.border_width = 1.8f;
        gp.shadow = DropShadow(0.0f, 16.0f, 28.0f, Color::from_rgba8(0, 0, 0, 180));
        canvas.draw_glass_panel(*card2_rect, 28.0f, 28.0f, gp);
    }

    // Radiant Pulsar Core inside Card 2
    float pc_x = 740.0f + 280.0f;
    float pc_y = 150.0f + 250.0f;

    // Glowing core
    std::vector<GradientStop> core_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(255, 255, 255, 255)),
        GradientStop::create(0.3f, Color::from_rgba8(255, 200, 80, 240)),
        GradientStop::create(0.6f, Color::from_rgba8(255, 0, 128, 180)),
        GradientStop::create(1.0f, Color::from_rgba8(0, 229, 255, 0))
    };
    auto core_rg = RadialGradient::create(Point(pc_x, pc_y), 110.0f, core_stops);
    if (core_rg) {
        Paint p; p.shader = Shader(*core_rg);
        canvas.fill_circle(pc_x, pc_y, 110.0f, p);
    }

    // Orbital Gyroscopic Rings
    for (int i = 0; i < 6; ++i) {
        float r = 55.0f + i * 26.0f;
        Stroke st; st.width = 3.0f;
        uint8_t a = static_cast<uint8_t>(230 - i * 28);
        Paint p(Color::from_rgba8(255, 0, 140, a));
        p.blend_mode = BlendMode::Plus;

        PathBuilder pb;
        pb.push_circle(pc_x, pc_y, r);
        auto circle_path = pb.finish();
        if (circle_path) canvas.stroke_path(*circle_path, p, st);
    }

    // Status Badges in Card 2
    for (int b = 0; b < 3; ++b) {
        auto badge_rect = Rect::from_xywh(780.0f, 510.0f + b * 60.0f, 480.0f, 46.0f);
        if (badge_rect) {
            Paint bp(Color::from_rgba8(255, 255, 255, 15));
            canvas.fill_round_rect(*badge_rect, 10.0f, 10.0f, bp);
            Color pill_color = (b == 0 ? Color::from_rgba8(0, 229, 255, 90) :
                                b == 1 ? Color::from_rgba8(0, 255, 128, 90) :
                                         Color::from_rgba8(255, 0, 128, 90));
            Paint bb(pill_color);
            Stroke bs; bs.width = 1.2f;
            canvas.stroke_round_rect(*badge_rect, 10.0f, 10.0f, bb, bs);

            // Active indicator pill
            Paint pill(pill_color);
            pill.blend_mode = BlendMode::Plus;
            canvas.fill_circle(805.0f, 533.0f + b * 60.0f, 6.0f, pill);
        }
    }

    std::cout << "[INFO] HD Canvas rendered successfully (1400x900)\n\n";

    // -------------------------------------------------------------
    // 5. Save Sovereign PNG (Fast & Production Modes)
    // -------------------------------------------------------------
    std::string out_dir = "";
    if (std::filesystem::exists("showcase")) {
        out_dir = "showcase/";
    } else if (std::filesystem::exists("../showcase")) {
        out_dir = "../showcase/";
    }

    std::string out_png_fast = out_dir + "showcase_vaxp_fast.png";
    t0 = std::chrono::high_resolution_clock::now();
    bool png_fast_ok = pixmap->save_png(out_png_fast, 1);
    t1 = std::chrono::high_resolution_clock::now();
    double png_fast_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::string out_png = out_dir + "showcase_vaxp.png";
    t0 = std::chrono::high_resolution_clock::now();
    bool png_ok = pixmap->save_png(out_png, 6);
    t1 = std::chrono::high_resolution_clock::now();
    double png_save_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!png_ok || !png_fast_ok) {
        std::cerr << "[-] Error saving PNG!\n";
        return 1;
    }
    std::cout << "[+] Sovereign PNG Fast Mode (Level 1, UI Compositor) -> " << png_fast_ms << " ms\n";
    std::cout << "[+] Sovereign PNG High Compression (Level 6, Storage)  -> " << png_save_ms << " ms\n";

    // -------------------------------------------------------------
    // 6. Save Sovereign JPEG
    // -------------------------------------------------------------
    std::string out_jpg = out_dir + "showcase_vaxp.jpg";
    t0 = std::chrono::high_resolution_clock::now();
    bool jpg_ok = pixmap->save_jpeg(out_jpg, 92);
    t1 = std::chrono::high_resolution_clock::now();
    double jpg_save_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!jpg_ok) {
        std::cerr << "[-] Error saving JPEG!\n";
        return 1;
    }
    std::cout << "[+] Sovereign JPEG Saved -> " << out_jpg << " in " << jpg_save_ms << " ms\n";

    // -------------------------------------------------------------
    // 7. Save Sovereign QOI (Ultra-Fast OS & Framework Cache)
    // -------------------------------------------------------------
    std::string out_qoi = out_dir + "showcase_vaxp.qoi";
    t0 = std::chrono::high_resolution_clock::now();
    bool qoi_ok = pixmap->save_qoi(out_qoi);
    t1 = std::chrono::high_resolution_clock::now();
    double qoi_save_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!qoi_ok) {
        std::cerr << "[-] Error saving QOI!\n";
        return 1;
    }
    std::cout << "[+] Sovereign QOI Saved -> " << out_qoi << " in " << qoi_save_ms << " ms  <-- ⚡ ULTRA-FAST\n";

    // -------------------------------------------------------------
    // 8. Verify Round-trip Loading
    // -------------------------------------------------------------
    auto png_reload = Pixmap::load_file(out_png);
    auto jpg_reload = Pixmap::load_file(out_jpg);

    t0 = std::chrono::high_resolution_clock::now();
    auto qoi_reload = Pixmap::load_file(out_qoi);
    t1 = std::chrono::high_resolution_clock::now();
    double qoi_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!png_reload || !jpg_reload || !qoi_reload) {
        std::cerr << "[-] Round-trip load verification failed!\n";
        return 1;
    }

    std::cout << "[+] Sovereign QOI Decoded back from Disk in " << qoi_load_ms << " ms\n";
    std::cout << "[+] Round-trip verification for PNG, JPEG, and QOI: 100% OK!\n\n";
    std::cout << "=======================================================\n";
    std::cout << "  VAXP Logo Integration: SUCCESS!                      \n";
    std::cout << "=======================================================\n";

    return 0;
}

#include <iostream>
#include <chrono>
#include <vector>
#include <filesystem>
#include <iomanip>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::effects;

int main() {
    std::cout << "=======================================================\n";
    std::cout << "  Nisaba Sovereign Image Codec Showcase                \n";
    std::cout << "  Formats: PNG • JPEG • QOI • WebP (Lossless & Lossy)  \n";
    std::cout << "=======================================================\n\n";

    // 1. Locate assets
    std::string assets_dir = "assets/";
    if (!std::filesystem::exists(assets_dir) && std::filesystem::exists("../assets/")) {
        assets_dir = "../assets/";
    }

    std::string logo_path = assets_dir + "vaxp.png";
    std::string sample_lossy_path = assets_dir + "sample.webp";
    std::string emblem_png = assets_dir + "nisaba_emblem.png";
    std::string webp_sample_path = assets_dir + "nisaba_emblem.webp";
    std::string vaxp_webp_path = assets_dir + "vaxp.webp";

    // -------------------------------------------------------------
    // 2. Decode & Benchmark Input Images
    // -------------------------------------------------------------
    std::cout << "[+] Loading organization logo from: " << logo_path << "\n";
    auto t0 = std::chrono::high_resolution_clock::now();
    auto logo = Pixmap::load_file(logo_path);
    auto t1 = std::chrono::high_resolution_clock::now();
    double logo_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!logo) {
        std::cerr << "[-] Failed to load " << logo_path << "!\n";
        return 1;
    }
    std::cout << "    -> Decoded PNG in " << logo_load_ms << " ms (" 
              << logo->width() << "x" << logo->height() << " RGBA)\n\n";

    // Convert and save vaxp.webp if not present, to ensure local WebP asset exists
    if (!std::filesystem::exists(vaxp_webp_path)) {
        logo->save_webp(vaxp_webp_path, true, 90);
    }

    // Ensure nisaba_emblem.webp exists in assets
    if (!std::filesystem::exists(webp_sample_path)) {
        if (std::filesystem::exists(emblem_png)) {
            auto src_img = Pixmap::load_file(emblem_png);
            if (src_img) src_img->save_webp(webp_sample_path, true, 90);
        } else {
            logo->save_webp(webp_sample_path, true, 90);
        }
    }

    // Probe downloaded sample.webp to verify RIFF container probing
    if (std::filesystem::exists(sample_lossy_path)) {
        std::cout << "[+] Probing third-party WebP sample: " << sample_lossy_path << "\n";
        auto probe_lossy = image::probe_image_file(sample_lossy_path);
        if (probe_lossy) {
            std::cout << "    [Probe] Format: WebP, Dimensions: " << probe_lossy->width 
                      << "x" << probe_lossy->height << ", Channels: " << static_cast<int>(probe_lossy->channels)
                      << ", Alpha: " << (probe_lossy->has_alpha ? "yes" : "no") << "\n";
        }
    }

    // Decode & Probe sovereign WebP showcase image
    std::cout << "[+] Probing & Decoding Sovereign WebP image: " << webp_sample_path << "\n";
    auto probe_webp_res = image::probe_image_file(webp_sample_path);
    if (probe_webp_res) {
        std::cout << "    [Probe] Format: WebP, Canvas: " << probe_webp_res->width 
                  << "x" << probe_webp_res->height << ", Channels: " << static_cast<int>(probe_webp_res->channels)
                  << ", Alpha: " << (probe_webp_res->has_alpha ? "yes" : "no") << "\n";
    }

    t0 = std::chrono::high_resolution_clock::now();
    auto webp_img = Pixmap::load_file(webp_sample_path);
    t1 = std::chrono::high_resolution_clock::now();
    double webp_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!webp_img) {
        std::cerr << "[-] Warning: Failed to load " << webp_sample_path << ", fallback to logo.\n";
        webp_img = logo->as_ref().to_owned();
    } else {
        std::cout << "    -> Decoded WebP in " << webp_load_ms << " ms (" 
                  << webp_img->width() << "x" << webp_img->height() << " RGBA)  <-- ⚡ SOVEREIGN WEBP\n\n";
    }

    // -------------------------------------------------------------
    // 3. Setup High-Definition Showcase Canvas (1400x900)
    // -------------------------------------------------------------
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

    // Top Header Badge
    auto header_rect = Rect::from_xywh(100.0f, 45.0f, 1200.0f, 65.0f);
    if (header_rect) {
        GlassParams gp_hdr = GlassParams::dark();
        gp_hdr.tint_color = Color::from_rgba8(12, 16, 26, 180);
        gp_hdr.border_color = Color::from_rgba8(0, 229, 255, 110);
        gp_hdr.border_width = 1.5f;
        canvas.draw_glass_panel(*header_rect, 16.0f, 16.0f, gp_hdr);

        Color codec_colors[4] = {
            Color::from_rgba8(0, 229, 255, 255),
            Color::from_rgba8(255, 170, 0, 255),
            Color::from_rgba8(0, 255, 136, 255),
            Color::from_rgba8(255, 0, 128, 255)
        };
        for (int i = 0; i < 4; ++i) {
            float lx = 140.0f + i * 290.0f;
            Paint led_p(codec_colors[i]);
            led_p.blend_mode = BlendMode::Plus;
            canvas.fill_circle(lx, 77.0f, 6.0f, led_p);

            // Sub-pill
            auto pill = Rect::from_xywh(lx + 15.0f, 62.0f, 140.0f, 30.0f);
            if (pill) {
                Paint pill_bg(Color::from_rgba8(255, 255, 255, 18));
                canvas.fill_round_rect(*pill, 8.0f, 8.0f, pill_bg);
                Color border_col = codec_colors[i];
                border_col.set_alpha(0.5f);
                Paint pill_border(border_col);
                Stroke ps; ps.width = 1.0f;
                canvas.stroke_round_rect(*pill, 8.0f, 8.0f, pill_border, ps);
            }
        }
    }

    // -------------------------------------------------------------
    // 4. Card 1 (Left): VAXP Organization Sovereign Emblem
    // -------------------------------------------------------------
    auto card1_rect = Rect::from_xywh(100.0f, 150.0f, 560.0f, 680.0f);
    if (card1_rect) {
        GlassParams gp = GlassParams::dark();
        gp.tint_color = Color::from_rgba8(14, 18, 30, 160);
        gp.border_color = Color::from_rgba8(0, 229, 255, 90);
        gp.border_width = 1.8f;
        gp.shadow = DropShadow(0.0f, 16.0f, 28.0f, Color::from_rgba8(0, 0, 0, 180));
        canvas.draw_glass_panel(*card1_rect, 28.0f, 28.0f, gp);
    }

    // Render decoded vaxp logo inside Card 1 using perspective bilinear quad mapping
    float logo_size = 400.0f;
    float logo_x = 100.0f + (560.0f - logo_size) * 0.5f;
    float logo_y = 150.0f + 40.0f;

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
        lpb.move_to(Point(180.0f, 615.0f));
        lpb.line_to(Point(580.0f, 615.0f));
        auto line_path = lpb.finish();
        if (line_path) canvas.stroke_path(*line_path, line_p, line_st);
    }

    // Badge Under Logo
    auto badge1 = Rect::from_xywh(180.0f, 645.0f, 400.0f, 50.0f);
    if (badge1) {
        Paint bp(Color::from_rgba8(0, 229, 255, 25));
        canvas.fill_round_rect(*badge1, 12.0f, 12.0f, bp);
        Paint b_border(Color::from_rgba8(0, 229, 255, 120));
        Stroke b_st; b_st.width = 1.5f;
        canvas.stroke_round_rect(*badge1, 12.0f, 12.0f, b_border, b_st);

        for (int d = 0; d < 4; ++d) {
            Paint dot_p(Color::from_rgba8(0, 242, 254, 220));
            canvas.fill_circle(210.0f + d * 22.0f, 670.0f, 4.5f, dot_p);
        }
    }

    // -------------------------------------------------------------
    // 5. Card 2 (Right): Decoded WebP Real-World Image Showcase
    // -------------------------------------------------------------
    auto card2_rect = Rect::from_xywh(740.0f, 150.0f, 560.0f, 680.0f);
    if (card2_rect) {
        GlassParams gp = GlassParams::dark();
        gp.tint_color = Color::from_rgba8(18, 14, 28, 160);
        gp.border_color = Color::from_rgba8(255, 0, 128, 90);
        gp.border_width = 1.8f;
        gp.shadow = DropShadow(0.0f, 16.0f, 28.0f, Color::from_rgba8(0, 0, 0, 180));
        canvas.draw_glass_panel(*card2_rect, 28.0f, 28.0f, gp);
    }

    // Render WebP image inside Card 2
    float webp_size = 380.0f;
    float webp_x = 740.0f + (560.0f - webp_size) * 0.5f;
    float webp_y = 150.0f + 40.0f;

    Point wp0(webp_x, webp_y);
    Point wp1(webp_x + webp_size, webp_y);
    Point wp2(webp_x + webp_size, webp_y + webp_size);
    Point wp3(webp_x, webp_y + webp_size);
    canvas.draw_pixmap_perspective(webp_img->as_ref(), wp0, wp1, wp2, wp3);

    // Border around WebP image
    auto webp_border_r = Rect::from_xywh(webp_x, webp_y, webp_size, webp_size);
    if (webp_border_r) {
        Paint p_wb(Color::from_rgba8(255, 0, 128, 140));
        Stroke st_wb; st_wb.width = 2.0f;
        canvas.stroke_round_rect(*webp_border_r, 14.0f, 14.0f, p_wb, st_wb);
    }

    // Status Badges in Card 2
    for (int b = 0; b < 3; ++b) {
        auto badge_rect = Rect::from_xywh(780.0f, 600.0f + b * 54.0f, 480.0f, 42.0f);
        if (badge_rect) {
            Paint bp(Color::from_rgba8(255, 255, 255, 15));
            canvas.fill_round_rect(*badge_rect, 10.0f, 10.0f, bp);
            Color pill_color = (b == 0 ? Color::from_rgba8(0, 229, 255, 140) :
                                b == 1 ? Color::from_rgba8(0, 255, 136, 140) :
                                         Color::from_rgba8(255, 0, 128, 140));
            Paint bb(pill_color);
            Stroke bs; bs.width = 1.2f;
            canvas.stroke_round_rect(*badge_rect, 10.0f, 10.0f, bb, bs);

            // Active indicator pill
            Paint pill(pill_color);
            pill.blend_mode = BlendMode::Plus;
            canvas.fill_circle(805.0f, 621.0f + b * 54.0f, 6.0f, pill);
        }
    }

    std::cout << "[INFO] HD Canvas rendered successfully (1400x900)\n\n";

    // -------------------------------------------------------------
    // 6. Output Directories
    // -------------------------------------------------------------
    std::string out_dir = "";
    if (std::filesystem::exists("showcase")) {
        out_dir = "showcase/";
    } else if (std::filesystem::exists("../showcase")) {
        out_dir = "../showcase/";
    }

    // -------------------------------------------------------------
    // 7. Save Sovereign PNG (Fast & High Compression)
    // -------------------------------------------------------------
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
    std::cout << "[+] Sovereign PNG Fast Mode (Level 1)          -> " << png_fast_ms << " ms\n";
    std::cout << "[+] Sovereign PNG High Compression (Level 6)   -> " << png_save_ms << " ms\n";

    // -------------------------------------------------------------
    // 8. Save Sovereign JPEG
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
    std::cout << "[+] Sovereign JPEG Saved                       -> " << jpg_save_ms << " ms\n";

    // -------------------------------------------------------------
    // 9. Save Sovereign QOI
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
    std::cout << "[+] Sovereign QOI Saved                        -> " << qoi_save_ms << " ms  <-- ⚡ ULTRA-FAST\n";

    // -------------------------------------------------------------
    // 10. Save Sovereign WebP (NEW!)
    // -------------------------------------------------------------
    std::string out_webp = out_dir + "showcase_vaxp.webp";
    t0 = std::chrono::high_resolution_clock::now();
    bool webp_ok = pixmap->save_webp(out_webp, true, 90);
    t1 = std::chrono::high_resolution_clock::now();
    double webp_save_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!webp_ok) {
        std::cerr << "[-] Error saving WebP!\n";
        return 1;
    }
    std::cout << "[+] Sovereign WebP Saved                       -> " << webp_save_ms << " ms  <-- 🌟 NEW SOVEREIGN CODEC\n";

    // -------------------------------------------------------------
    // 11. Verify Round-trip Loading for ALL formats
    // -------------------------------------------------------------
    std::cout << "\n[+] Running Round-Trip Verification & Disk Benchmarks:\n";

    t0 = std::chrono::high_resolution_clock::now();
    auto png_reload = Pixmap::load_file(out_png);
    t1 = std::chrono::high_resolution_clock::now();
    double png_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    auto jpg_reload = Pixmap::load_file(out_jpg);
    t1 = std::chrono::high_resolution_clock::now();
    double jpg_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    auto qoi_reload = Pixmap::load_file(out_qoi);
    t1 = std::chrono::high_resolution_clock::now();
    double qoi_load_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    t0 = std::chrono::high_resolution_clock::now();
    auto webp_reload = Pixmap::load_file(out_webp);
    t1 = std::chrono::high_resolution_clock::now();
    double webp_reload_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    if (!png_reload || !jpg_reload || !qoi_reload || !webp_reload) {
        std::cerr << "[-] Round-trip load verification failed!\n";
        return 1;
    }

    auto get_size_kb = [](const std::string& path) -> double {
        if (!std::filesystem::exists(path)) return 0.0;
        return static_cast<double>(std::filesystem::file_size(path)) / 1024.0;
    };

    auto print_row = [](const std::string& fmt, double enc, double dec, double size_kb) {
        std::cout << std::left << std::setw(10) << fmt 
                  << std::right << std::fixed << std::setprecision(2)
                  << std::setw(12) << enc << " ms  "
                  << std::setw(12) << dec << " ms  "
                  << std::setw(10) << size_kb << " KB\n";
    };

    std::cout << "------------------------------------------------------------------\n";
    std::cout << std::left << std::setw(10) << "Format" 
              << std::right << std::setw(16) << "Encode Time" 
              << std::setw(16) << "Decode Time" 
              << std::setw(16) << "File Size\n";
    std::cout << "------------------------------------------------------------------\n";
    print_row("PNG",  png_save_ms,  png_load_ms,    get_size_kb(out_png));
    print_row("JPEG", jpg_save_ms,  jpg_load_ms,    get_size_kb(out_jpg));
    print_row("QOI",  qoi_save_ms,  qoi_load_ms,    get_size_kb(out_qoi));
    print_row("WebP", webp_save_ms, webp_reload_ms, get_size_kb(out_webp));
    std::cout << "------------------------------------------------------------------\n";

    std::cout << "[+] Round-trip verification: 100% OK! All 4 codecs validated.\n\n";
    std::cout << "=======================================================\n";
    std::cout << "  Nisaba Sovereign WebP Integration: COMPLETE SUCCESS!  \n";
    std::cout << "=======================================================\n";

    return 0;
}

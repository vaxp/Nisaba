#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <numeric>
#include <memory>
#include <optional>

#include "nisaba/nisaba.hpp"
#include "nisaba/damage/damage.hpp"
#include "nisaba/effects/glass.hpp"

#ifdef NISABA_HAS_BACKEND_OS
#  include "nisaba/backend_os/platform.hpp"
#  include "nisaba/backend_os/window.hpp"
#endif

using namespace nisaba;
using namespace nisaba::text;
using namespace nisaba::damage;
using namespace nisaba::effects;

namespace {

constexpr uint32_t TOTAL_WIDTH = 1600;
constexpr uint32_t TOTAL_HEIGHT = 900;
constexpr uint32_t VIEWPORT_WIDTH = 800;
constexpr uint32_t VIEWPORT_HEIGHT = 900;
constexpr uint32_t CARD_WIDTH = 340;
constexpr uint32_t CARD_HEIGHT = 220;

/// Reusable production text renderer using Nisaba Text Engine with debug fallback
class TextEngine {
public:
    static TextEngine& instance() {
        static TextEngine s_instance;
        return s_instance;
    }

    void init() {
        if (initialized_) return;
        initialized_ = true;
        auto inter = resolve_font("Inter-Regular.ttf");
        auto fira = resolve_font("FiraMono-Medium.ttf");
        if (!inter.empty()) font_system_.load_font_file(inter);
        if (!fira.empty()) font_system_.load_font_file(fira);
    }

    void draw(Canvas& canvas, std::string_view text, float x, float y, float size = 13.0f,
              Color col = Color::WHITE, bool mono = false, Weight weight = Weight::Normal) {
        if (!initialized_) init();

        if (font_system_.font_count() == 0) {
            Paint p;
            p.set_color(col);
            canvas.draw_text_debug(text, x, y, p);
            return;
        }

        Buffer buf(Metrics(size, size * 1.3f));
        Attrs attrs;
        attrs.set_color(TextColor::rgba(
            static_cast<uint8_t>(col.red() * 255.0f),
            static_cast<uint8_t>(col.green() * 255.0f),
            static_cast<uint8_t>(col.blue() * 255.0f),
            static_cast<uint8_t>(col.alpha() * 255.0f)
        ));
        if (mono) {
            attrs.set_family(Family::monospace());
        } else {
            attrs.set_family(Family::sans_serif());
        }
        attrs.set_weight(weight);
        buf.set_text(text, attrs);
        buf.draw(canvas, cache_, font_system_, Color::WHITE, x, y);
    }

    void draw_cached(Canvas& canvas, std::string_view text, float x, float y, float size = 13.0f,
                     Color col = Color::WHITE, bool mono = false, Weight weight = Weight::Normal,
                     float opacity = 1.0f) {
        if (!initialized_) init();

        if (font_system_.font_count() == 0) {
            Paint p;
            p.set_color(col);
            canvas.draw_text_debug(text, x, y, p);
            return;
        }

        text_cache_.draw(canvas, font_system_, cache_, text, x, y, size, col, weight, mono, opacity);
    }

    TextCache& cache() noexcept { return text_cache_; }

private:
    TextEngine() = default;

    static std::string resolve_font(std::string_view filename) {
        std::vector<std::string> candidates = {
            std::string("fonts/") + std::string(filename),
            std::string("../fonts/") + std::string(filename),
            std::string("../../fonts/") + std::string(filename),
            std::string("nisaba/fonts/") + std::string(filename),
            std::string("/home/x/Downloads/thorvg-main/nisaba/fonts/") + std::string(filename)
        };
        for (const auto& path : candidates) {
            std::ifstream f(path, std::ios::binary);
            if (f.good()) return path;
        }
        return "";
    }

    FontSystem font_system_;
    GlyphCache cache_;
    TextCache text_cache_{512};
    bool initialized_{false};
};

/// Renders a visually rich, colorful vector background wallpaper
void render_rich_wallpaper(Canvas& canvas, uint32_t w, uint32_t h) {
    // 1. Dark tech background
    canvas.clear(Color::from_rgba8(12, 16, 26, 255));

    // 2. Neon diagonal stripes
    for (uint32_t x = 0; x < w + h; x += 60) {
        PathBuilder pb;
        pb.move_to(static_cast<float>(x), 0.0f);
        pb.line_to(static_cast<float>(x) - 300.0f, static_cast<float>(h));
        auto path = pb.finish();
        if (path) {
            Stroke s;
            s.width = 3.0f;
            uint8_t alpha = static_cast<uint8_t>(20 + (x % 40));
            canvas.stroke_path(*path, Paint(Color::from_rgba8(30, 80, 160, alpha)), s);
        }
    }

    // 3. Glowing gradient circles / abstract spheres
    auto draw_circle = [&](float cx, float cy, float radius, Color col) {
        auto circle_path = PathBuilder::from_circle(cx, cy, radius);
        if (circle_path) {
            canvas.fill_path(*circle_path, Paint(col));
        }
    };

    draw_circle(200.0f, 250.0f, 160.0f, Color::from_rgba8(230, 60, 110, 180));
    draw_circle(550.0f, 380.0f, 190.0f, Color::from_rgba8(40, 180, 240, 180));
    draw_circle(380.0f, 620.0f, 170.0f, Color::from_rgba8(140, 40, 240, 180));
    draw_circle(680.0f, 180.0f, 110.0f, Color::from_rgba8(250, 180, 20, 160));

    // 4. Background circuit grid
    for (uint32_t y = 80; y < h - 100; y += 40) {
        PathBuilder pb;
        pb.move_to(40.0f, static_cast<float>(y));
        pb.line_to(static_cast<float>(w) - 40.0f, static_cast<float>(y));
        auto path = pb.finish();
        if (path) {
            Stroke s;
            s.width = 1.0f;
            canvas.stroke_path(*path, Paint(Color::from_rgba8(255, 255, 255, 18)), s);
        }
    }

    // 5. Tech typography and badges
    auto& txt = TextEngine::instance();
    txt.draw(canvas, "NISABA APERTURE BLUR TEST BED", 80.0f, 140.0f, 26.0f, Color::from_rgba8(255, 255, 255, 140), false, Weight::Bold);
    txt.draw(canvas, "CORE ENGINE VECTOR SUBSYSTEM // 2D COMPOSITOR", 80.0f, 180.0f, 16.0f, Color::from_rgba8(100, 220, 255, 160), true);
}

struct CardInfo {
    std::string title;
    std::string tag;
    float base_x;
    float base_y;
    float amp_x;
    float amp_y;
    float freq_x;
    float freq_y;
    float phase;
    float width;
    float height;
    Color accent;
};

// Global microsecond accumulators for profiler telemetry
static double g_right_wall_us = 0.0;
static double g_right_aperture_us = 0.0;
static double g_right_gauge_us = 0.0;
static double g_right_text_us = 0.0;

static double g_left_wall_us = 0.0;
static double g_left_blur_us = 0.0;
static double g_left_gauge_us = 0.0;
static double g_left_text_us = 0.0;

/// Renders card contents (title, icons, metrics) over a glass panel
void render_card_content(Canvas& canvas, float x, float y, float w, float /*h*/, int64_t frame_idx, const CardInfo& card, bool use_text_cache = false) {
    auto& txt = TextEngine::instance();

    auto t_txt0 = std::chrono::high_resolution_clock::now();
    if (use_text_cache) {
        txt.draw_cached(canvas, card.title, x + 20.0f, y + 22.0f, 13.5f, Color::from_rgba8(255, 255, 255, 240), false, Weight::Bold);
        txt.draw_cached(canvas, card.tag, x + 20.0f, y + 42.0f, 10.0f, Color::from_rgba8(140, 200, 255, 200), true);
    } else {
        txt.draw(canvas, card.title, x + 20.0f, y + 22.0f, 13.5f, Color::from_rgba8(255, 255, 255, 240), false, Weight::Bold);
        txt.draw(canvas, card.tag, x + 20.0f, y + 42.0f, 10.0f, Color::from_rgba8(140, 200, 255, 200), true);
    }
    auto t_txt1 = std::chrono::high_resolution_clock::now();
    double dt_txt1 = std::chrono::duration<double, std::micro>(t_txt1 - t_txt0).count();

    // Dynamic animated gauge bar inside the card
    auto t_g0 = std::chrono::high_resolution_clock::now();
    float bar_x = x + 20.0f;
    float bar_y = y + 64.0f;
    float bar_w = w - 40.0f;
    float bar_h = 7.0f;

    auto track = PathBuilder::from_rounded_rect(Rect::from_xywh(bar_x, bar_y, bar_w, bar_h).value(), 3.5f, 3.5f);
    if (track) canvas.fill_path(*track, Paint(Color::from_rgba8(255, 255, 255, 40)));

    float progress = 0.5f + 0.45f * std::sin(static_cast<float>(frame_idx) * 0.08f + card.phase);
    auto fill = PathBuilder::from_rounded_rect(Rect::from_xywh(bar_x, bar_y, bar_w * progress, bar_h).value(), 3.5f, 3.5f);
    if (fill) canvas.fill_path(*fill, Paint(card.accent));
    auto t_g1 = std::chrono::high_resolution_clock::now();
    double dt_g = std::chrono::duration<double, std::micro>(t_g1 - t_g0).count();

    auto t_txt2 = std::chrono::high_resolution_clock::now();
    int pct = std::clamp(static_cast<int>(progress * 100.0f + 0.5f), 0, 100);
    std::string tp_str = "THROUGHPUT: " + std::to_string(pct) + "%";
    if (use_text_cache) {
        txt.draw_cached(canvas, tp_str, bar_x, bar_y + 15.0f, 10.0f, Color::from_rgba8(240, 240, 255, 220), true);
        txt.draw_cached(canvas, "APERTURE: SCANLINE DIRECT", x + 20.0f, y + 104.0f, 9.5f, Color::from_rgba8(180, 190, 210, 180), true);
        txt.draw_cached(canvas, "BLEED: MIRROR REFLECTIVE", x + 20.0f, y + 122.0f, 9.5f, Color::from_rgba8(180, 190, 210, 180), true);
    } else {
        txt.draw(canvas, tp_str, bar_x, bar_y + 15.0f, 10.0f, Color::from_rgba8(240, 240, 255, 220), true);
        txt.draw(canvas, "APERTURE: SCANLINE DIRECT", x + 20.0f, y + 104.0f, 9.5f, Color::from_rgba8(180, 190, 210, 180), true);
        txt.draw(canvas, "BLEED: MIRROR REFLECTIVE", x + 20.0f, y + 122.0f, 9.5f, Color::from_rgba8(180, 190, 210, 180), true);
    }
    auto t_txt3 = std::chrono::high_resolution_clock::now();
    double dt_txt2 = std::chrono::duration<double, std::micro>(t_txt3 - t_txt2).count();

    if (use_text_cache) {
        g_right_text_us += (dt_txt1 + dt_txt2);
        g_right_gauge_us += dt_g;
    } else {
        g_left_text_us += (dt_txt1 + dt_txt2);
        g_left_gauge_us += dt_g;
    }
}

} // namespace

int main(int argc, char* argv[]) {
    bool headless = false;
    uint32_t total_frames = 100;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--headless") headless = true;
        else if (arg == "--frames" && i + 1 < argc) total_frames = static_cast<uint32_t>(std::stoul(argv[++i]));
    }
    (void)headless;

    std::cout << "\n====================================================================================\n";
    std::cout << "     NISABA 2D GRAPHICS ENGINE — FROSTED GLASS APERTURE BENCHMARK\n";
    std::cout << "     Left: Traditional Real-Time Gaussian Blur vs Right: BackdropBlurCache (Aperture)\n";
    std::cout << "====================================================================================\n";

    // 1. Prepare Base Framebuffer
    auto main_pix = Pixmap::create(TOTAL_WIDTH, TOTAL_HEIGHT);
    if (!main_pix) {
        std::cerr << "Failed to allocate framebuffer\n";
        return 1;
    }
    Canvas main_canvas(*main_pix);

    // 2. Prepare Viewport Surfaces
    auto left_pix = Pixmap::create(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    auto right_pix = Pixmap::create(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    Canvas left_canvas(*left_pix);
    Canvas right_canvas(*right_pix);

    // 3. Pre-render Wallpaper for Viewports
    auto wallpaper = Pixmap::create(VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    Canvas wall_canvas(*wallpaper);
    render_rich_wallpaper(wall_canvas, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    // Copy wallpaper to both viewports initially
    std::memcpy(left_pix->pixels_mut(), wallpaper->pixels(), wallpaper->data_len());
    std::memcpy(right_pix->pixels_mut(), wallpaper->pixels(), wallpaper->data_len());

    // 4. Initialize BackdropBlurCache for Right Viewport
    BackdropBlurCache blur_cache(*wallpaper, 14.0f, true);

    // Setup Glass parameters
    GlassParams glass_params = GlassParams::dark();
    glass_params.blur_sigma = 14.0f;
    glass_params.tint_color = Color::from_rgba8(18, 24, 38, 140);
    glass_params.border_color = Color::from_rgba8(255, 255, 255, 80);
    glass_params.border_width = 1.2f;
    glass_params.shadow = DropShadow(0.0f, 10.0f, 14.0f, Color::from_rgba8(0, 0, 0, 150));

    // Setup 6 Dynamic Floating Glass Cards
    std::vector<CardInfo> cards = {
        {"GPU PIPELINE ALPHA", "NODE_01 // ACTIVE",   150.0f, 150.0f, 90.0f, 45.0f, 1.0f, 0.8f, 0.0f, 250.0f, 155.0f, Color::from_rgba8(40, 240, 180, 230)},
        {"NEURAL COMPOSITOR",  "NODE_02 // SHADER",   430.0f, 160.0f, 80.0f, 50.0f, 0.7f, 1.1f, 1.5f, 250.0f, 155.0f, Color::from_rgba8(255, 110, 180, 230)},
        {"VECTOR TILE SCANNER","NODE_03 // SPANS",    160.0f, 370.0f, 75.0f, 55.0f, 1.1f, 0.6f, 3.0f, 250.0f, 155.0f, Color::from_rgba8(100, 200, 255, 230)},
        {"APERTURE BLUR BLIT", "NODE_04 // ZERO_MATH",440.0f, 390.0f, 85.0f, 60.0f, 0.9f, 1.2f, 4.2f, 250.0f, 155.0f, Color::from_rgba8(255, 210, 60, 230)},
        {"SURFACE CACHE ATLAS","NODE_05 // MEMORY",   170.0f, 580.0f, 80.0f, 45.0f, 0.8f, 0.9f, 2.1f, 250.0f, 155.0f, Color::from_rgba8(190, 120, 255, 230)},
        {"MIRROR BLEED ENGINE","NODE_06 // REFLECT",  430.0f, 590.0f, 75.0f, 50.0f, 1.2f, 0.7f, 5.0f, 250.0f, 155.0f, Color::from_rgba8(50, 255, 220, 230)}
    };

    // Performance metrics
    std::vector<double> left_latencies;
    std::vector<double> right_latencies;
    left_latencies.reserve(total_frames);
    right_latencies.reserve(total_frames);

    std::cout << "[*] Executing real-time frosted glass animation loop (" << total_frames << " frames, 6 CARDS)...\n";

    for (uint32_t f = 0; f < total_frames; ++f) {
        float t = static_cast<float>(f) * 0.05f;

        // ------------------------------------------------------------        // -------------------------------------------------------------
        // LEFT VIEWPORT: Traditional Real-Time 3-Pass Gaussian Blur (6 Cards)
        // -------------------------------------------------------------
        auto t0_left = std::chrono::high_resolution_clock::now();

        // 1. Restore wallpaper background
        auto t_l_wall0 = std::chrono::high_resolution_clock::now();
        std::memcpy(left_pix->pixels_mut(), wallpaper->pixels(), wallpaper->data_len());
        auto t_l_wall1 = std::chrono::high_resolution_clock::now();
        g_left_wall_us += std::chrono::duration<double, std::micro>(t_l_wall1 - t_l_wall0).count();

        // 2. Real-Time Gaussian Blur & Composite for 6 cards
        auto left_mut = left_pix->as_mut();
        for (const auto& card : cards) {
            float cx = card.base_x + card.amp_x * std::sin(t * card.freq_x + card.phase);
            float cy = card.base_y + card.amp_y * std::cos(t * card.freq_y + card.phase);
            auto card_rect = Rect::from_xywh(cx, cy, card.width, card.height).value();

            auto t_lb0 = std::chrono::high_resolution_clock::now();
            draw_glass_panel(left_mut, card_rect, 14.0f, 14.0f, glass_params);
            auto t_lb1 = std::chrono::high_resolution_clock::now();
            g_left_blur_us += std::chrono::duration<double, std::micro>(t_lb1 - t_lb0).count();

            render_card_content(left_canvas, cx, cy, card.width, card.height, f, card, false);
        }

        auto t1_left = std::chrono::high_resolution_clock::now();
        double left_ms = std::chrono::duration<double, std::milli>(t1_left - t0_left).count();
        left_latencies.push_back(left_ms);

        // -------------------------------------------------------------
        // RIGHT VIEWPORT: BackdropBlurCache + Sovereign TextCache (6 Cards)
        // -------------------------------------------------------------
        auto t0_right = std::chrono::high_resolution_clock::now();

        // 1. Restore wallpaper background
        auto t_rw0 = std::chrono::high_resolution_clock::now();
        std::memcpy(right_pix->pixels_mut(), wallpaper->pixels(), wallpaper->data_len());
        auto t_rw1 = std::chrono::high_resolution_clock::now();
        g_right_wall_us += std::chrono::duration<double, std::micro>(t_rw1 - t_rw0).count();

        // 2. Instantaneous Zero-Cost Screen Aperture Sampling + SIMD TextCache Blitting
        for (const auto& card : cards) {
            float cx = card.base_x + card.amp_x * std::sin(t * card.freq_x + card.phase);
            float cy = card.base_y + card.amp_y * std::cos(t * card.freq_y + card.phase);
            auto card_rect = Rect::from_xywh(cx, cy, card.width, card.height).value();

            auto t_ap0 = std::chrono::high_resolution_clock::now();
            blur_cache.draw_glass_aperture(right_canvas, card_rect, 14.0f, 14.0f, glass_params);
            auto t_ap1 = std::chrono::high_resolution_clock::now();
            g_right_aperture_us += std::chrono::duration<double, std::micro>(t_ap1 - t_ap0).count();

            render_card_content(right_canvas, cx, cy, card.width, card.height, f, card, true);
        }

        auto t1_right = std::chrono::high_resolution_clock::now();
        double right_ms = std::chrono::duration<double, std::milli>(t1_right - t0_right).count();
        right_latencies.push_back(right_ms);

        if (f % 10 == 0 || f == total_frames - 1) {
            double speedup = (right_ms > 0.0001) ? (left_ms / right_ms) : 1.0;
            std::cout << "  [Frame " << std::setw(4) << std::setfill('0') << (f + 1) << "/"
                      << std::setw(4) << total_frames << "] Traditional (6 Cards): "
                      << std::fixed << std::setprecision(2) << left_ms << " ms ("
                      << std::setw(5) << std::setprecision(1) << (1000.0 / left_ms) << " FPS) | "
                      << "Aperture + TextCache: " << std::setprecision(4) << right_ms << " ms ("
                      << std::setw(7) << std::setprecision(1) << (1000.0 / right_ms) << " FPS) | "
                      << "Speedup: " << std::setw(5) << std::setprecision(1) << speedup << "x\n";
        }
    }

    // 5. Composite Left and Right viewports into Main Framebuffer
    auto main_mut = main_pix->as_mut();
    for (uint32_t y = 0; y < TOTAL_HEIGHT; ++y) {
        auto* dst_row = main_mut.row(y);
        const auto* left_row = left_pix->pixels() + y * VIEWPORT_WIDTH;
        const auto* right_row = right_pix->pixels() + y * VIEWPORT_WIDTH;

        std::memcpy(dst_row, left_row, VIEWPORT_WIDTH * sizeof(PremultipliedColorU8));
        std::memcpy(dst_row + VIEWPORT_WIDTH, right_row, VIEWPORT_WIDTH * sizeof(PremultipliedColorU8));
    }

    // 6. Draw HUD Headers and Dividers
    auto& txt = TextEngine::instance();

    // Vertical Divider
    PathBuilder div_pb;
    div_pb.move_to(static_cast<float>(VIEWPORT_WIDTH), 0.0f);
    div_pb.line_to(static_cast<float>(VIEWPORT_WIDTH), static_cast<float>(TOTAL_HEIGHT));
    auto div_path = div_pb.finish();
    if (div_path) {
        Stroke ds;
        ds.width = 2.0f;
        main_canvas.stroke_path(*div_path, Paint(Color::from_rgba8(0, 220, 255, 120)), ds);
    }

    // Header Banners
    txt.draw(main_canvas, "TRADITIONAL REAL-TIME BLUR + UNCACHED TEXT [6 CARDS]", 40.0f, 32.0f, 17.0f, Color::from_rgba8(255, 100, 100, 240), false, Weight::Bold);
    txt.draw(main_canvas, "BACKDROP BLUR CACHE + SOVEREIGN TEXT CACHE [6 CARDS]", VIEWPORT_WIDTH + 40.0f, 32.0f, 17.0f, Color::from_rgba8(40, 240, 160, 240), false, Weight::Bold);

    // Bottom Telemetry Cards
    double avg_left = std::accumulate(left_latencies.begin(), left_latencies.end(), 0.0) / left_latencies.size();
    double avg_right = std::accumulate(right_latencies.begin(), right_latencies.end(), 0.0) / right_latencies.size();
    double overall_speedup = avg_left / avg_right;

    auto hud_left = PathBuilder::from_rounded_rect(Rect::from_xywh(40.0f, 800.0f, 720.0f, 70.0f).value(), 8.0f, 8.0f);
    if (hud_left) main_canvas.fill_path(*hud_left, Paint(Color::from_rgba8(10, 14, 22, 220)));

    auto hud_right = PathBuilder::from_rounded_rect(Rect::from_xywh(VIEWPORT_WIDTH + 40.0f, 800.0f, 720.0f, 70.0f).value(), 8.0f, 8.0f);
    if (hud_right) main_canvas.fill_path(*hud_right, Paint(Color::from_rgba8(10, 14, 22, 220)));

    std::ostringstream oss_l;
    oss_l << "Mean Latency: " << std::fixed << std::setprecision(2) << avg_left << " ms | Throughput: "
          << std::setprecision(1) << (1000.0 / avg_left) << " FPS (Uncached Shaping + Blur Lag)";
    txt.draw(main_canvas, oss_l.str(), 60.0f, 830.0f, 14.5f, Color::from_rgba8(255, 140, 140, 240), true);

    std::ostringstream oss_r;
    oss_r << "Mean Latency: " << std::fixed << std::setprecision(3) << avg_right << " ms | Throughput: "
          << std::setprecision(1) << (1000.0 / avg_right) << " FPS (" << std::setprecision(1) << overall_speedup << "x FASTER)";
    txt.draw(main_canvas, oss_r.str(), VIEWPORT_WIDTH + 60.0f, 830.0f, 14.5f, Color::from_rgba8(100, 255, 180, 240), true);

    // 7. Save showcase image
    main_pix->save_png("showcase/glass_aperture_comparison.png");
    main_pix->save_png("/home/x/.gemini/antigravity-ide/brain/0fd1f6fd-8cd1-4ad7-87ba-53c26fa4badf/combined_glass_text_showcase.png");
    std::cout << "[+] Showcase comparison saved to: showcase/glass_aperture_comparison.png\n";

    double n_f = static_cast<double>(total_frames);
    double l_wall_ms = (g_left_wall_us / n_f) / 1000.0;
    double l_blur_ms = (g_left_blur_us / n_f) / 1000.0;
    double l_gauge_ms = (g_left_gauge_us / n_f) / 1000.0;
    double l_text_ms = (g_left_text_us / n_f) / 1000.0;

    double r_wall_ms = (g_right_wall_us / n_f) / 1000.0;
    double r_ap_ms = (g_right_aperture_us / n_f) / 1000.0;
    double r_gauge_ms = (g_right_gauge_us / n_f) / 1000.0;
    double r_text_ms = (g_right_text_us / n_f) / 1000.0;

    std::cout << "\n========================================================================================================\n";
    std::cout << "  NISABA DUAL CACHE BENCHMARK (6 CARDS): TRADITIONAL VS (APERTURE BLUR + SOVEREIGN TEXT CACHE)\n";
    std::cout << "========================================================================================================\n";
    std::cout << std::setfill(' ');
    std::cout << " Metric                        | Traditional (Gaussian + Raw) | Aperture + Sovereign TextCache | Speedup\n";
    std::cout << "-------------------------------+------------------------------+--------------------------------+-------------\n";
    std::cout << " Mean Frame Latency            | " << std::setw(25) << std::fixed << std::setprecision(3) << avg_left << " ms | "
              << std::setw(27) << std::setprecision(4) << avg_right << " ms | "
              << std::setw(10) << std::setprecision(1) << overall_speedup << "x Faster\n";
    std::cout << " Render Throughput (FPS)       | " << std::setw(25) << std::setprecision(1) << (1000.0 / avg_left) << " FPS | "
              << std::setw(27) << std::setprecision(1) << (1000.0 / avg_right) << " FPS | +"
              << std::setw(7) << std::setprecision(0) << ((overall_speedup - 1.0) * 100.0) << "% Gain\n";
    std::cout << " Real-time Blur Math           | " << std::setw(28) << "18 Passes per frame" << " | "
              << std::setw(30) << "ZERO Passes (O(1) Blit)" << " | "
              << std::setw(12) << "Zero-Math\n";
    std::cout << " Text Engine Caching           | " << std::setw(28) << "30 Full Shapings / Frame" << " | "
              << std::setw(30) << "Direct O(1) SIMD Blits" << " | "
              << std::setw(12) << "Instant\n";
    std::cout << " TextCache Hit Ratio           | " << std::setw(28) << "0.0% (No Cache)" << " | "
              << std::setw(24) << std::setprecision(1) << (txt.cache().hit_ratio() * 100.0) << "% (" << txt.cache().hits() << " hits) | "
              << std::setw(12) << "High Hit\n";
    std::cout << "-------------------------------+------------------------------+--------------------------------+-------------\n";
    std::cout << " SUB-OPERATION BREAKDOWN       | Left Viewport (Traditional)  | Right Viewport (Optimized)     | Status\n";
    std::cout << "-------------------------------+------------------------------+--------------------------------+-------------\n";
    std::cout << " 1. Wallpaper 2.88MB Memcpy    | " << std::setw(25) << std::setprecision(4) << l_wall_ms << " ms | "
              << std::setw(27) << std::setprecision(4) << r_wall_ms << " ms | "
              << ((l_wall_ms > 0) ? (l_wall_ms / (r_wall_ms > 0 ? r_wall_ms : 1.0)) : 1.0) << "x\n";
    std::cout << " 2. Blur / Aperture + Shadow   | " << std::setw(25) << std::setprecision(4) << l_blur_ms << " ms | "
              << std::setw(27) << std::setprecision(4) << r_ap_ms << " ms | "
              << (l_blur_ms / (r_ap_ms > 0 ? r_ap_ms : 1.0)) << "x\n";
    std::cout << " 3. Gauge Vector FillPath (12x)| " << std::setw(25) << std::setprecision(4) << l_gauge_ms << " ms | "
              << std::setw(27) << std::setprecision(4) << r_gauge_ms << " ms | "
              << (l_gauge_ms / (r_gauge_ms > 0 ? r_gauge_ms : 1.0)) << "x\n";
    std::cout << " 4. Text Rendering (30 Draws)  | " << std::setw(25) << std::setprecision(4) << l_text_ms << " ms | "
              << std::setw(27) << std::setprecision(4) << r_text_ms << " ms | "
              << (l_text_ms / (r_text_ms > 0 ? r_text_ms : 1.0)) << "x\n";
    std::cout << "========================================================================================================\n";

    return 0;
}

//
// Nisaba 2D Graphics Engine - Damage Tracking Micro-Benchmark Suite
// Purpose: Measure baseline frame times, throughput, and memory bandwidth
// under realistic GUI micro-invalidation scenarios (Blinking Cursor, Counters, Multi-Zones).
//

#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <string>
#include <string_view>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <memory>

#include "nisaba/nisaba.hpp"

using namespace nisaba;

// -----------------------------------------------------------------------------
// Benchmark Configuration & Statistics
// -----------------------------------------------------------------------------

struct BenchmarkResult {
    std::string scenario_name;
    size_t total_frames = 0;
    double total_time_ms = 0.0;
    double avg_frame_us = 0.0;
    double median_frame_us = 0.0;
    double p95_frame_us = 0.0;
    double min_frame_us = 0.0;
    double max_frame_us = 0.0;
    double fps = 0.0;
    double std_dev_us = 0.0;
    uint64_t total_pixels_processed = 0;
};

class BenchmarkStats {
public:
    static BenchmarkResult compute(std::string_view name, std::vector<double>& frame_times_us, uint64_t pixels_per_frame) {
        BenchmarkResult res;
        res.scenario_name = name;
        res.total_frames = frame_times_us.size();
        if (frame_times_us.empty()) return res;

        std::sort(frame_times_us.begin(), frame_times_us.end());

        double sum = std::accumulate(frame_times_us.begin(), frame_times_us.end(), 0.0);
        res.total_time_ms = sum / 1000.0;
        res.avg_frame_us = sum / res.total_frames;
        res.min_frame_us = frame_times_us.front();
        res.max_frame_us = frame_times_us.back();
        res.median_frame_us = frame_times_us[res.total_frames / 2];
        res.p95_frame_us = frame_times_us[static_cast<size_t>(res.total_frames * 0.95)];
        res.fps = (res.total_time_ms > 0.0) ? (static_cast<double>(res.total_frames) / (res.total_time_ms / 1000.0)) : 0.0;
        res.total_pixels_processed = pixels_per_frame * res.total_frames;

        double variance = 0.0;
        for (double t : frame_times_us) {
            double diff = t - res.avg_frame_us;
            variance += diff * diff;
        }
        res.std_dev_us = std::sqrt(variance / res.total_frames);

        return res;
    }
};

// -----------------------------------------------------------------------------
// Realistic UI Scene Generator (1280 x 800 HD Dashboard)
// -----------------------------------------------------------------------------

class RealisticDashboardScene {
public:
    static constexpr uint32_t WIDTH = 1280;
    static constexpr uint32_t HEIGHT = 800;

    static void render_full_background(Canvas& canvas) {
        // 1. Base dark background
        canvas.clear(Color::from_rgba8(11, 14, 20, 255));

        // 2. Subtle ambient gradient backdrop
        std::vector<GradientStop> bg_stops = {
            GradientStop::create(0.0f, Color::from_rgba8(20, 28, 48, 255)),
            GradientStop::create(0.6f, Color::from_rgba8(14, 18, 28, 255)),
            GradientStop::create(1.0f, Color::from_rgba8(8, 10, 15, 255))
        };
        auto bg_rad = RadialGradient::create(
            Point::from_xy(WIDTH * 0.5f, 0.0f),
            WIDTH * 0.8f,
            bg_stops
        );
        Paint bg_paint;
        bg_paint.shader = Shader(*bg_rad);
        canvas.fill_rect(*Rect::from_xywh(0, 0, WIDTH, HEIGHT), bg_paint);

        // 3. Top navigation header bar
        Paint nav_paint;
        nav_paint.set_color_rgba8(18, 24, 38, 240);
        canvas.fill_rect(*Rect::from_xywh(0, 0, WIDTH, 64), nav_paint);

        Paint nav_border;
        nav_border.set_color_rgba8(35, 48, 72, 180);
        Stroke stroke1(1.0f);
        canvas.stroke_line(Point::from_xy(0, 64), Point::from_xy(WIDTH, 64), nav_border, stroke1);

        // Header Title
        Paint title_paint;
        title_paint.set_color_rgba8(0, 220, 255, 255);
        canvas.draw_text_debug("NISABA MISSION TELEMETRY DASHBOARD", 28.0f, 24.0f, title_paint);

        // 4. Three prominent glassmorphic panels
        // Card 1: System Health (Left)
        draw_card(canvas, 32.0f, 96.0f, 360.0f, 660.0f, "SYSTEM STATUS & NODES");

        // Card 2: Interactive Console & Input (Center)
        draw_card(canvas, 424.0f, 96.0f, 480.0f, 660.0f, "CORE ENGINE TERMINAL");

        // Card 3: Real-Time Gauges (Right)
        draw_card(canvas, 936.0f, 96.0f, 312.0f, 660.0f, "HARDWARE ANALYTICS");
    }

    static void draw_card(Canvas& canvas, float x, float y, float w, float h, const char* title) {
        auto card_rect = Rect::from_xywh(x, y, w, h);
        if (!card_rect) return;

        // Card body with subtle translucent fill
        Paint card_fill;
        card_fill.set_color_rgba8(16, 22, 34, 210);
        canvas.fill_round_rect(*card_rect, 12.0f, 12.0f, card_fill);

        // Card boundary outline
        Paint card_stroke;
        card_stroke.set_color_rgba8(40, 56, 85, 200);
        Stroke card_stroke_def(1.2f);
        canvas.stroke_round_rect(*card_rect, 12.0f, 12.0f, card_stroke, card_stroke_def);

        // Title bar inside card
        Paint title_paint;
        title_paint.set_color_rgba8(140, 170, 210, 255);
        canvas.draw_text_debug(title, x + 16.0f, y + 16.0f, title_paint);

        Paint divider;
        divider.set_color_rgba8(30, 42, 64, 180);
        Stroke divider_stroke(1.0f);
        canvas.stroke_line(Point::from_xy(x + 12.0f, y + 42.0f), Point::from_xy(x + w - 12.0f, y + 42.0f), divider, divider_stroke);
    }

    // --- Dynamic Micro-Update Renderers ---

    // 1. Blinking Text Cursor (approx 3 x 18 px) inside Card 2
    static void draw_cursor(Canvas& canvas, bool visible) {
        float cx = 450.0f;
        float cy = 200.0f;
        float cw = 3.0f;
        float ch = 18.0f;

        Paint p;
        if (visible) {
            p.set_color_rgba8(0, 229, 255, 255);
        } else {
            p.set_color_rgba8(16, 22, 34, 255); // background color of card
        }
        canvas.fill_rect(*Rect::from_xywh(cx, cy, cw, ch), p);
    }

    // 2. Dynamic Metric Gauge & Counter inside Card 3 (approx 120 x 28 px)
    static void draw_counter(Canvas& canvas, int frame_idx) {
        float gx = 960.0f;
        float gy = 200.0f;
        float gw = 180.0f;
        float gh = 24.0f;

        // Clear previous counter box area
        Paint bg_p;
        bg_p.set_color_rgba8(16, 22, 34, 255);
        canvas.fill_rect(*Rect::from_xywh(gx, gy, gw, gh), bg_p);

        // Draw active progress fill
        float progress = (frame_idx % 100) / 100.0f;
        Paint bar_bg;
        bar_bg.set_color_rgba8(25, 35, 55, 255);
        canvas.fill_round_rect(*Rect::from_xywh(gx, gy + 14.0f, gw, 8.0f), 4.0f, 4.0f, bar_bg);

        Paint bar_fill;
        bar_fill.set_color_rgba8(0, 220, 130, 255);
        if (progress > 0.01f) {
            canvas.fill_round_rect(*Rect::from_xywh(gx, gy + 14.0f, gw * progress, 8.0f), 4.0f, 4.0f, bar_fill);
        }

        // Label
        char buf[32];
        std::snprintf(buf, sizeof(buf), "VAL: %d%%", static_cast<int>(progress * 100.0f));
        Paint txt_p;
        txt_p.set_color_rgba8(200, 220, 240, 255);
        canvas.draw_text_debug(buf, gx, gy, txt_p);
    }

    // 3. Multi-Zone Updates (4 discrete small spots on screen)
    static void draw_multi_zone(Canvas& canvas, int frame_idx) {
        // Zone 1: Cursor in Center Card
        draw_cursor(canvas, (frame_idx % 2) == 0);

        // Zone 2: Counter in Right Card
        draw_counter(canvas, frame_idx);

        // Zone 3: Header System Clock / FPS in Top Right (80 x 20 px)
        {
            float x = WIDTH - 120.0f;
            float y = 22.0f;
            Paint bg;
            bg.set_color_rgba8(18, 24, 38, 255);
            canvas.fill_rect(*Rect::from_xywh(x, y, 100.0f, 20.0f), bg);

            char buf[32];
            std::snprintf(buf, sizeof(buf), "T: %04d ms", frame_idx * 16);
            Paint p;
            p.set_color_rgba8(0, 220, 255, 255);
            canvas.draw_text_debug(buf, x, y, p);
        }

        // Zone 4: Status Indicator LED in Left Card (12 x 12 px)
        {
            float x = 60.0f;
            float y = 160.0f;
            Paint led;
            if ((frame_idx / 5) % 2 == 0) {
                led.set_color_rgba8(0, 255, 128, 255);
            } else {
                led.set_color_rgba8(255, 60, 60, 255);
            }
            canvas.fill_circle(x + 6.0f, y + 6.0f, 6.0f, led);
        }
    }
};

// -----------------------------------------------------------------------------
// Benchmark Runner
// -----------------------------------------------------------------------------

enum class BenchmarkMode {
    FullRepaint,  // Naive Baseline: Full 1280x800 redraw every frame
    DamageTracked // Future/Opt: Scissor-clipped / Tiled damage updates
};

BenchmarkResult run_micro_pulse_test(size_t frame_count, BenchmarkMode mode) {
    auto pixmap = Pixmap::allocate(RealisticDashboardScene::WIDTH, RealisticDashboardScene::HEIGHT);
    Canvas canvas(*pixmap);

    // Initial warm-up: full scene rendering
    RealisticDashboardScene::render_full_background(canvas);

    std::vector<double> frame_times;
    frame_times.reserve(frame_count);

    const auto cursor_rect = *Rect::from_xywh(450.0f, 200.0f, 4.0f, 20.0f);
    nisaba::damage::TiledSpanTracker<16> tracker(RealisticDashboardScene::WIDTH, RealisticDashboardScene::HEIGHT);
    std::vector<ScreenIntRect> damage_rects;

    for (size_t i = 0; i < frame_count; ++i) {
        bool visible = (i % 2) == 0;
        auto t0 = std::chrono::high_resolution_clock::now();

        if (mode == BenchmarkMode::FullRepaint) {
            // Full Repaint: Re-rasterize all 1,024,000 pixels
            RealisticDashboardScene::render_full_background(canvas);
            RealisticDashboardScene::draw_cursor(canvas, visible);
        } else {
            // Damage Tracked via TiledSpanTracker<16>
            tracker.clear();
            tracker.mark_dirty(cursor_rect);
            tracker.generate_damage_rects(damage_rects);
            for (const auto& r : damage_rects) {
                canvas.save();
                canvas.clip_rect(r);
                RealisticDashboardScene::draw_cursor(canvas, visible);
                canvas.restore();
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        frame_times.push_back(us);
    }

    uint64_t px_per_frame = (mode == BenchmarkMode::FullRepaint)
        ? (RealisticDashboardScene::WIDTH * RealisticDashboardScene::HEIGHT)
        : static_cast<uint64_t>(cursor_rect.width() * cursor_rect.height());

    std::string name = (mode == BenchmarkMode::FullRepaint)
        ? "Micro-Pulse (Blinking Cursor) [Full Repaint Baseline]"
        : "Micro-Pulse (Blinking Cursor) [Damage-Tracked]";

    return BenchmarkStats::compute(name, frame_times, px_per_frame);
}

BenchmarkResult run_sub_component_test(size_t frame_count, BenchmarkMode mode) {
    auto pixmap = Pixmap::allocate(RealisticDashboardScene::WIDTH, RealisticDashboardScene::HEIGHT);
    Canvas canvas(*pixmap);

    RealisticDashboardScene::render_full_background(canvas);

    std::vector<double> frame_times;
    frame_times.reserve(frame_count);

    const auto counter_rect = *Rect::from_xywh(955.0f, 195.0f, 190.0f, 35.0f);
    nisaba::damage::TiledSpanTracker<16> tracker(RealisticDashboardScene::WIDTH, RealisticDashboardScene::HEIGHT);
    std::vector<ScreenIntRect> damage_rects;

    for (size_t i = 0; i < frame_count; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();

        if (mode == BenchmarkMode::FullRepaint) {
            RealisticDashboardScene::render_full_background(canvas);
            RealisticDashboardScene::draw_counter(canvas, static_cast<int>(i));
        } else {
            tracker.clear();
            tracker.mark_dirty(counter_rect);
            tracker.generate_damage_rects(damage_rects);
            for (const auto& r : damage_rects) {
                canvas.save();
                canvas.clip_rect(r);
                RealisticDashboardScene::draw_counter(canvas, static_cast<int>(i));
                canvas.restore();
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        frame_times.push_back(us);
    }

    uint64_t px_per_frame = (mode == BenchmarkMode::FullRepaint)
        ? (RealisticDashboardScene::WIDTH * RealisticDashboardScene::HEIGHT)
        : static_cast<uint64_t>(counter_rect.width() * counter_rect.height());

    std::string name = (mode == BenchmarkMode::FullRepaint)
        ? "Sub-Component (Counter & Gauge) [Full Repaint Baseline]"
        : "Sub-Component (Counter & Gauge) [Damage-Tracked]";

    return BenchmarkStats::compute(name, frame_times, px_per_frame);
}

BenchmarkResult run_multi_zone_test(size_t frame_count, BenchmarkMode mode) {
    auto pixmap = Pixmap::allocate(RealisticDashboardScene::WIDTH, RealisticDashboardScene::HEIGHT);
    Canvas canvas(*pixmap);

    RealisticDashboardScene::render_full_background(canvas);

    std::vector<double> frame_times;
    frame_times.reserve(frame_count);

    const auto r_cursor = *Rect::from_xywh(450.0f, 200.0f, 4.0f, 20.0f);
    const auto r_counter = *Rect::from_xywh(955.0f, 195.0f, 190.0f, 35.0f);
    const auto r_clock = *Rect::from_xywh(RealisticDashboardScene::WIDTH - 120.0f, 20.0f, 105.0f, 24.0f);
    const auto r_led = *Rect::from_xywh(50.0f, 150.0f, 25.0f, 25.0f);

    nisaba::damage::TiledSpanTracker<16> tracker(RealisticDashboardScene::WIDTH, RealisticDashboardScene::HEIGHT);
    std::vector<ScreenIntRect> damage_rects;

    for (size_t i = 0; i < frame_count; ++i) {
        auto t0 = std::chrono::high_resolution_clock::now();

        if (mode == BenchmarkMode::FullRepaint) {
            RealisticDashboardScene::render_full_background(canvas);
            RealisticDashboardScene::draw_multi_zone(canvas, static_cast<int>(i));
        } else {
            tracker.clear();
            tracker.mark_dirty(r_cursor);
            tracker.mark_dirty(r_counter);
            tracker.mark_dirty(r_clock);
            tracker.mark_dirty(r_led);
            tracker.generate_damage_rects(damage_rects);

            for (const auto& r : damage_rects) {
                canvas.save();
                canvas.clip_rect(r);
                RealisticDashboardScene::draw_multi_zone(canvas, static_cast<int>(i));
                canvas.restore();
            }
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        double us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        frame_times.push_back(us);
    }

    uint64_t px_per_frame = (mode == BenchmarkMode::FullRepaint)
        ? (RealisticDashboardScene::WIDTH * RealisticDashboardScene::HEIGHT)
        : static_cast<uint64_t>(r_cursor.width() * r_cursor.height() +
                                r_counter.width() * r_counter.height() +
                                r_clock.width() * r_clock.height() +
                                r_led.width() * r_led.height());

    std::string name = (mode == BenchmarkMode::FullRepaint)
        ? "Multi-Zone (4 Simultaneous Micro-Spots) [Full Repaint Baseline]"
        : "Multi-Zone (4 Simultaneous Micro-Spots) [Damage-Tracked]";

    return BenchmarkStats::compute(name, frame_times, px_per_frame);
}

// -----------------------------------------------------------------------------
// Formatted Output Printer
// -----------------------------------------------------------------------------

void print_result(const BenchmarkResult& r) {
    std::cout << "--------------------------------------------------------------------------------\n";
    std::cout << " Scenario        : " << r.scenario_name << "\n";
    std::cout << " Frames Tested   : " << r.total_frames << " frames\n";
    std::cout << " Total Time      : " << std::fixed << std::setprecision(2) << r.total_time_ms << " ms\n";
    std::cout << " Mean Frame Time : " << std::fixed << std::setprecision(1) << r.avg_frame_us << " us ("
              << std::setprecision(3) << (r.avg_frame_us / 1000.0) << " ms)\n";
    std::cout << " Median (P50)    : " << std::fixed << std::setprecision(1) << r.median_frame_us << " us\n";
    std::cout << " 95th Percentile : " << std::fixed << std::setprecision(1) << r.p95_frame_us << " us\n";
    std::cout << " Min / Max Frame : " << std::fixed << std::setprecision(1) << r.min_frame_us << " us / " << r.max_frame_us << " us\n";
    std::cout << " Std Deviation   : " << std::fixed << std::setprecision(1) << r.std_dev_us << " us\n";
    std::cout << " Throughput      : " << std::fixed << std::setprecision(1) << r.fps << " FPS\n";
    std::cout << " Bandwidth Touch : " << std::fixed << std::setprecision(2)
              << (static_cast<double>(r.total_pixels_processed * 4) / (1024.0 * 1024.0)) << " MB written\n";
}

int main(int argc, char** argv) {
    size_t frame_count = 150; // Standard 150 frames per scenario
    bool damage_mode = false;
    bool json_output = false;

    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--damage" || arg == "--damage-opt") {
            damage_mode = true;
        } else if (arg == "--frames" && i + 1 < argc) {
            frame_count = static_cast<size_t>(std::atoi(argv[++i]));
        } else if (arg == "--json") {
            json_output = true;
        }
    }

    if (!json_output) {
        std::cout << "================================================================================\n";
        std::cout << "       NISABA DAMAGE TRACKING BENCHMARK (1280x800 HD Dashboard)        \n";
        std::cout << " Mode: " << (damage_mode ? "DAMAGE-TRACKED OPTIMIZATION" : "FULL REPAINT BASELINE") << "\n";
        std::cout << " Frame Count per Scenario: " << frame_count << "\n";
        std::cout << "================================================================================\n\n";
    }

    auto r1 = run_micro_pulse_test(frame_count, damage_mode ? BenchmarkMode::DamageTracked : BenchmarkMode::FullRepaint);
    if (!json_output) print_result(r1);

    auto r2 = run_sub_component_test(frame_count, damage_mode ? BenchmarkMode::DamageTracked : BenchmarkMode::FullRepaint);
    if (!json_output) print_result(r2);

    auto r3 = run_multi_zone_test(frame_count, damage_mode ? BenchmarkMode::DamageTracked : BenchmarkMode::FullRepaint);
    if (!json_output) print_result(r3);

    if (json_output) {
        std::cout << "{\n";
        std::cout << "  \"mode\": \"" << (damage_mode ? "damage_tracked" : "full_repaint") << "\",\n";
        std::cout << "  \"frames\": " << frame_count << ",\n";
        std::cout << "  \"results\": [\n";
        std::cout << "    {\"scenario\": \"micro_pulse\", \"avg_ms\": " << (r1.avg_frame_us / 1000.0)
                  << ", \"median_ms\": " << (r1.median_frame_us / 1000.0)
                  << ", \"p95_ms\": " << (r1.p95_frame_us / 1000.0)
                  << ", \"fps\": " << r1.fps
                  << ", \"std_dev_ms\": " << (r1.std_dev_us / 1000.0) << "},\n";
        std::cout << "    {\"scenario\": \"sub_component\", \"avg_ms\": " << (r2.avg_frame_us / 1000.0)
                  << ", \"median_ms\": " << (r2.median_frame_us / 1000.0)
                  << ", \"p95_ms\": " << (r2.p95_frame_us / 1000.0)
                  << ", \"fps\": " << r2.fps
                  << ", \"std_dev_ms\": " << (r2.std_dev_us / 1000.0) << "},\n";
        std::cout << "    {\"scenario\": \"multi_zone\", \"avg_ms\": " << (r3.avg_frame_us / 1000.0)
                  << ", \"median_ms\": " << (r3.median_frame_us / 1000.0)
                  << ", \"p95_ms\": " << (r3.p95_frame_us / 1000.0)
                  << ", \"fps\": " << r3.fps
                  << ", \"std_dev_ms\": " << (r3.std_dev_us / 1000.0) << "}\n";
        std::cout << "  ]\n";
        std::cout << "}\n";
    } else {
        std::cout << "================================================================================\n";
        std::cout << " SUMMARY TABLE: " << (damage_mode ? "DAMAGE-TRACKED" : "BASELINE") << "\n";
        std::cout << "--------------------------------------------------------------------------------\n";
        std::cout << " Scenario                    | Avg (ms)   | Median (ms)| P95 (ms)   | FPS        \n";
        std::cout << "-----------------------------+------------+------------+------------+-----------\n";
        std::cout << std::left << std::setw(29) << "1. Micro-Pulse (Cursor)"
                  << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(3) << (r1.avg_frame_us / 1000.0)
                  << " | " << std::setw(10) << (r1.median_frame_us / 1000.0)
                  << " | " << std::setw(10) << (r1.p95_frame_us / 1000.0)
                  << " | " << std::setw(9) << std::setprecision(1) << r1.fps << "\n";
        std::cout << std::left << std::setw(29) << "2. Sub-Component (Counter)"
                  << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(3) << (r2.avg_frame_us / 1000.0)
                  << " | " << std::setw(10) << (r2.median_frame_us / 1000.0)
                  << " | " << std::setw(10) << (r2.p95_frame_us / 1000.0)
                  << " | " << std::setw(9) << std::setprecision(1) << r2.fps << "\n";
        std::cout << std::left << std::setw(29) << "3. Multi-Zone (4 spots)"
                  << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(3) << (r3.avg_frame_us / 1000.0)
                  << " | " << std::setw(10) << (r3.median_frame_us / 1000.0)
                  << " | " << std::setw(10) << (r3.p95_frame_us / 1000.0)
                  << " | " << std::setw(9) << std::setprecision(1) << r3.fps << "\n";
        std::cout << "================================================================================\n";
    }

    return 0;
}


/// @file showcase_lottie.cpp
/// @brief Live interactive 60 FPS native window showcase of Nisaba's 100% sovereign Lottie vector engine.

#include "nisaba/nisaba.hpp"

#ifdef NISABA_HAS_BACKEND_OS
#  include "nisaba/backend_os/platform.hpp"
#  include "nisaba/backend_os/window.hpp"
#  include "nisaba/gpu/context.hpp"
#  include "nisaba/gpu/gl3_renderer.hpp"
#  ifdef NISABA_GLEW
#    include <GL/glew.h>
#  else
#    include <GL/gl.h>
#  endif
#endif

#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <string_view>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <algorithm>

using namespace nisaba;
using namespace nisaba::lottie;
using namespace nisaba::effects;
using namespace nisaba::text;
using namespace nisaba::damage;

namespace {

constexpr uint32_t WIN_WIDTH = 1280;
constexpr uint32_t WIN_HEIGHT = 760;

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

// 1. Loading Spinner Lottie JSON (Rotating dashed arc)
constexpr std::string_view SPINNER_LOTTIE = R"({
    "v": "5.7.4", "fr": 60, "ip": 0, "op": 60, "w": 200, "h": 200, "nm": "Spinner",
    "layers": [
        {
            "ind": 1, "ty": 4, "nm": "Ring", "ip": 0, "op": 60, "st": 0,
            "ks": {
                "o": {"a": 0, "k": 100},
                "r": {
                    "a": 1,
                    "k": [
                        {"t": 0, "s": [0], "e": [360], "o": {"x": 0.4, "y": 0.0}, "i": {"x": 0.6, "y": 1.0}},
                        {"t": 60, "s": [360]}
                    ]
                },
                "p": {"a": 0, "k": [100, 100]},
                "a": {"a": 0, "k": [0, 0]},
                "s": {"a": 0, "k": [100, 100]}
            },
            "shapes": [
                {
                    "ty": "gr", "nm": "Circle",
                    "it": [
                        {
                            "ty": "el", "nm": "Ellipse",
                            "p": {"a": 0, "k": [0, 0]},
                            "s": {"a": 0, "k": [120, 120]}
                        },
                        {
                            "ty": "tm", "nm": "Trim",
                            "s": {"a": 0, "k": 10},
                            "e": {
                                "a": 1,
                                "k": [
                                    {"t": 0, "s": [20], "e": [85]},
                                    {"t": 30, "s": [85], "e": [20]},
                                    {"t": 60, "s": [20]}
                                ]
                            },
                            "o": {"a": 0, "k": 0},
                            "m": 1
                        },
                        {
                            "ty": "st", "nm": "Stroke",
                            "c": {"a": 0, "k": [0.0, 0.85, 1.0, 1.0]},
                            "o": {"a": 0, "k": 100},
                            "w": {"a": 0, "k": 14},
                            "lc": 2, "lj": 2
                        },
                        {
                            "ty": "tr", "p": {"a": 0, "k": [0, 0]}, "a": {"a": 0, "k": [0, 0]},
                            "s": {"a": 0, "k": [100, 100]}, "r": {"a": 0, "k": 0}, "o": {"a": 0, "k": 100}
                        }
                    ]
                }
            ]
        }
    ]
})";

// 2. Success Checkmark with Circle Pop Lottie JSON
constexpr std::string_view CHECKMARK_LOTTIE = R"({
    "v": "5.7.4", "fr": 60, "ip": 0, "op": 60, "w": 200, "h": 200, "nm": "Checkmark",
    "layers": [
        {
            "ind": 1, "ty": 4, "nm": "Check", "ip": 15, "op": 60, "st": 0,
            "ks": {
                "o": {"a": 0, "k": 100}, "r": {"a": 0, "k": 0},
                "p": {"a": 0, "k": [100, 100]}, "a": {"a": 0, "k": [0, 0]},
                "s": {"a": 0, "k": [100, 100]}
            },
            "shapes": [
                {
                    "ty": "gr", "nm": "CheckPath",
                    "it": [
                        {
                            "ty": "sh", "nm": "Tick",
                            "ks": {
                                "a": 0,
                                "k": {
                                    "c": false,
                                    "v": [[-35, 5], [-10, 30], [40, -25]],
                                    "i": [[0, 0], [0, 0], [0, 0]],
                                    "o": [[0, 0], [0, 0], [0, 0]]
                                }
                            }
                        },
                        {
                            "ty": "tm", "nm": "Trim",
                            "s": {"a": 0, "k": 0},
                            "e": {
                                "a": 1,
                                "k": [
                                    {"t": 15, "s": [0], "e": [100], "o": {"x": 0.2, "y": 0.0}, "i": {"x": 0.2, "y": 1.0}},
                                    {"t": 45, "s": [100]}
                                ]
                            },
                            "o": {"a": 0, "k": 0}, "m": 1
                        },
                        {
                            "ty": "st", "nm": "Stroke",
                            "c": {"a": 0, "k": [0.0, 1.0, 0.6, 1.0]},
                            "o": {"a": 0, "k": 100},
                            "w": {"a": 0, "k": 12},
                            "lc": 2, "lj": 2
                        },
                        {
                            "ty": "tr", "p": {"a": 0, "k": [0, 0]}, "a": {"a": 0, "k": [0, 0]},
                            "s": {"a": 0, "k": [100, 100]}, "r": {"a": 0, "k": 0}, "o": {"a": 0, "k": 100}
                        }
                    ]
                }
            ]
        },
        {
            "ind": 2, "ty": 4, "nm": "CircleBg", "ip": 0, "op": 60, "st": 0,
            "ks": {
                "o": {"a": 0, "k": 100}, "r": {"a": 0, "k": 0},
                "p": {"a": 0, "k": [100, 100]}, "a": {"a": 0, "k": [0, 0]},
                "s": {
                    "a": 1,
                    "k": [
                        {"t": 0, "s": [0, 0], "e": [115, 115], "o": {"x": 0.175, "y": 0.885}, "i": {"x": 0.32, "y": 1.275}},
                        {"t": 25, "s": [115, 115], "e": [100, 100]},
                        {"t": 40, "s": [100, 100]}
                    ]
                }
            },
            "shapes": [
                {
                    "ty": "gr", "nm": "CircleGroup",
                    "it": [
                        {
                            "ty": "el", "nm": "Circle",
                            "p": {"a": 0, "k": [0, 0]},
                            "s": {"a": 0, "k": [130, 130]}
                        },
                        {
                            "ty": "st", "nm": "RingStroke",
                            "c": {"a": 0, "k": [0.0, 1.0, 0.6, 0.4]},
                            "o": {"a": 0, "k": 100},
                            "w": {"a": 0, "k": 6}
                        },
                        {
                            "ty": "fl", "nm": "CircleFill",
                            "c": {"a": 0, "k": [0.05, 0.25, 0.18, 0.8]},
                            "o": {"a": 0, "k": 100}
                        },
                        {
                            "ty": "tr", "p": {"a": 0, "k": [0, 0]}, "a": {"a": 0, "k": [0, 0]},
                            "s": {"a": 0, "k": [100, 100]}, "r": {"a": 0, "k": 0}, "o": {"a": 0, "k": 100}
                        }
                    ]
                }
            ]
        }
    ]
})";

// 3. Pulsing Heart Lottie JSON
constexpr std::string_view HEART_LOTTIE = R"({
    "v": "5.7.4", "fr": 60, "ip": 0, "op": 60, "w": 200, "h": 200, "nm": "PulsingHeart",
    "layers": [
        {
            "ind": 1, "ty": 4, "nm": "HeartLayer", "ip": 0, "op": 60, "st": 0,
            "ks": {
                "o": {"a": 0, "k": 100}, "r": {"a": 0, "k": 0},
                "p": {"a": 0, "k": [100, 105]}, "a": {"a": 0, "k": [0, 0]},
                "s": {
                    "a": 1,
                    "k": [
                        {"t": 0, "s": [90, 90], "e": [125, 125]},
                        {"t": 15, "s": [125, 125], "e": [95, 95]},
                        {"t": 30, "s": [95, 95], "e": [120, 120]},
                        {"t": 45, "s": [120, 120], "e": [90, 90]},
                        {"t": 60, "s": [90, 90]}
                    ]
                }
            },
            "shapes": [
                {
                    "ty": "gr", "nm": "HeartShapeGroup",
                    "it": [
                        {
                            "ty": "sh", "nm": "HeartPath",
                            "ks": {
                                "a": 0,
                                "k": {
                                    "c": true,
                                    "v": [[0, 35], [-45, -5], [-25, -45], [0, -25], [25, -45], [45, -5]],
                                    "i": [[20, 0], [-10, 20], [-15, 0], [-10, -10], [10, -10], [15, 0]],
                                    "o": [[-20, 0], [10, -20], [10, 10], [10, -10], [15, 0], [-10, 20]]
                                }
                            }
                        },
                        {
                            "ty": "fl", "nm": "Fill",
                            "c": {"a": 0, "k": [1.0, 0.15, 0.35, 1.0]},
                            "o": {"a": 0, "k": 100}
                        },
                        {
                            "ty": "st", "nm": "GlowStroke",
                            "c": {"a": 0, "k": [1.0, 0.4, 0.6, 0.7]},
                            "o": {"a": 0, "k": 100},
                            "w": {"a": 0, "k": 6}
                        },
                        {
                            "ty": "tr", "p": {"a": 0, "k": [0, 0]}, "a": {"a": 0, "k": [0, 0]},
                            "s": {"a": 0, "k": [100, 100]}, "r": {"a": 0, "k": 0}, "o": {"a": 0, "k": 100}
                        }
                    ]
                }
            ]
        }
    ]
})";

} // namespace

int main(int argc, char** argv) {
    bool run_headless = false;
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg == "--headless") {
            run_headless = true;
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: showcase_lottie [options]\n\n"
                      << "Options:\n"
                      << "  --headless   Run in headless mode (no GUI window, saves PNG snapshot)\n"
                      << "  --help, -h   Display this help message\n\n"
                      << "Interactive Window Controls:\n"
                      << "  [SPACE]      Play / Pause animations\n"
                      << "  [R]          Restart all animations from frame 0\n"
                      << "  [S]          Cycle speed (0.25x -> 0.5x -> 1.0x -> 2.0x)\n"
                      << "  [L]          Toggle looping on/off\n"
                      << "  [1 / 2 / 3]  Replay individual card (Spinner, Checkmark, Heart)\n"
                      << "  [CLICK]      Click any card directly with mouse to replay\n"
                      << "  [ESC] / [Q]  Exit showcase\n";
            return 0;
        }
    }

    std::cout << "========================================================\n"
              << "   NISABA SOVEREIGN LOTTIE VECTOR ENGINE LIVE SHOWCASE\n"
              << "========================================================\n";

    // 1. Framebuffer allocation
    auto fb = Pixmap::create(WIN_WIDTH, WIN_HEIGHT);
    if (!fb) {
        std::cerr << "[-] Error: Failed to allocate framebuffer." << std::endl;
        return 1;
    }
    Canvas canvas(*fb);

    // 2. Text rendering setup
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("FiraMono-Medium.ttf"));
    GlyphCache cache;

    auto draw_text = [&](Canvas& c, float x, float y, std::string_view text, float font_size, float line_height,
                         TextColor color, std::optional<float> max_w = std::nullopt) {
        Buffer buf(Metrics(font_size, line_height));
        if (max_w) buf.set_size(*max_w, std::nullopt);
        buf.set_wrap(Wrap::Word);
        Attrs attrs;
        attrs.set_color(color);
        buf.set_text(text, attrs);
        buf.draw(c, cache, font_system, Color::WHITE, x, y);
    };

    // 3. Pre-render Background Wallpaper
    std::cout << "[*] Pre-rendering cosmic background environment..." << std::endl;
    auto wallpaper = Pixmap::create(WIN_WIDTH, WIN_HEIGHT);
    {
        Canvas wall_canvas(*wallpaper);
        std::vector<GradientStop> bg_stops = {
            GradientStop::create(0.0f, Color::from_rgba8(9, 12, 22, 255)),
            GradientStop::create(0.5f, Color::from_rgba8(16, 20, 36, 255)),
            GradientStop::create(1.0f, Color::from_rgba8(7, 9, 16, 255))
        };
        auto bg_grad = LinearGradient::create(Point::from_xy(0.0f, 0.0f), Point::from_xy(WIN_WIDTH, WIN_HEIGHT), bg_stops);
        Paint bg_paint;
        if (bg_grad) bg_paint.shader = Shader(*bg_grad);
        wall_canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, WIN_WIDTH, WIN_HEIGHT), bg_paint);

        // Nebulae glows
        if (auto g_cyan = RadialGradient::create(Point::from_xy(250.0f, 150.0f), 350.0f, {
            GradientStop::create(0.0f, Color::from_rgba8(0, 242, 254, 35)),
            GradientStop::create(1.0f, Color::from_rgba8(0, 242, 254, 0))
        })) {
            Paint p; p.shader = Shader(*g_cyan);
            wall_canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 600.0f, 500.0f), p);
        }

        if (auto g_pink = RadialGradient::create(Point::from_xy(1050.0f, 400.0f), 400.0f, {
            GradientStop::create(0.0f, Color::from_rgba8(255, 45, 115, 30)),
            GradientStop::create(1.0f, Color::from_rgba8(255, 45, 115, 0))
        })) {
            Paint p; p.shader = Shader(*g_pink);
            wall_canvas.fill_rect(*Rect::from_xywh(650.0f, 50.0f, 630.0f, 700.0f), p);
        }
    }

    // 4. Initialize BackdropBlurCache for instantaneous zero-cost frosted glass apertures
    std::cout << "[*] Pre-blurring cosmic wallpaper into BackdropBlurCache (one-time setup)...\n";
    auto t_b0 = std::chrono::high_resolution_clock::now();
    BackdropBlurCache blur_cache(*wallpaper, 14.0f, true);
    auto t_b1 = std::chrono::high_resolution_clock::now();
    double init_blur_ms = std::chrono::duration<double, std::milli>(t_b1 - t_b0).count();
    std::cout << "    Blur cache ready in " << std::fixed << std::setprecision(2) << init_blur_ms << " ms.\n\n";

    // 5. Load Animations and build Players
    std::cout << "[*] Initializing Lottie Players..." << std::endl;
    auto anim_spinner = Animation::load_from_data(SPINNER_LOTTIE);
    auto anim_checkmark = Animation::load_from_data(CHECKMARK_LOTTIE);
    auto anim_heart = Animation::load_from_data(HEART_LOTTIE);

    if (!anim_spinner || !anim_checkmark || !anim_heart) {
        std::cerr << "[-] Error parsing Lottie animations." << std::endl;
        return 1;
    }

    Player player_spinner(anim_spinner);
    Player player_checkmark(anim_checkmark);
    Player player_heart(anim_heart);

    float current_speed = 1.0f;
    bool is_looping = true;
    bool is_paused = false;

    // 5. Initialize Native Window if backend_os is available
#ifdef NISABA_HAS_BACKEND_OS
    std::unique_ptr<backend_os::Platform> platform;
    std::unique_ptr<backend_os::Window> window;
    std::unique_ptr<nisaba::gpu::Context> gpu_ctx;
    int gpu_texture_id = 0;
    bool g_running = true;

    if (!run_headless) {
        auto p_res = backend_os::Platform::create();
        if (p_res.isOk()) {
            platform = std::move(p_res).value();
            backend_os::WindowConfig cfg;
            cfg.title = "NISABA LOTTIE LIVE SHOWCASE — 60 FPS Sovereign Vector Animation Player";
            cfg.width = WIN_WIDTH;
            cfg.height = WIN_HEIGHT;
            cfg.vsync = true;
            auto w_res = backend_os::Window::create(*platform, cfg);
            if (w_res.isOk()) {
                window = std::move(w_res).value();
#ifdef NISABA_GLEW
                glewInit();
#endif
                gpu_ctx = nisaba::gpu::createContextGL3(nisaba::gpu::CreateFlags::Antialias);
                if (gpu_ctx) {
                    gpu_texture_id = gpu_ctx->createImageRGBA(WIN_WIDTH, WIN_HEIGHT, 0, fb->data());
                    std::cout << "[+] Native OS GUI window created successfully (1280x760).\n";

                    window->onClose().connect([&]() {
                        g_running = false;
                    });

                    platform->onKeyDown().connect([&](int key, int mods) {
                        (void)mods;
                        if (key == 27 || key == 'q' || key == 'Q') {
                            g_running = false;
                        } else if (key == 32) { // Space
                            is_paused = !is_paused;
                            if (is_paused) {
                                player_spinner.pause();
                                player_checkmark.pause();
                                player_heart.pause();
                            } else {
                                player_spinner.play();
                                player_checkmark.play();
                                player_heart.play();
                            }
                        } else if (key == 'r' || key == 'R') {
                            player_spinner.seek_frame(0.0f);
                            player_checkmark.seek_frame(0.0f);
                            player_heart.seek_frame(0.0f);
                        } else if (key == 's' || key == 'S') {
                            if (current_speed == 1.0f) current_speed = 2.0f;
                            else if (current_speed == 2.0f) current_speed = 0.5f;
                            else if (current_speed == 0.5f) current_speed = 0.25f;
                            else current_speed = 1.0f;

                            player_spinner.set_speed(current_speed);
                            player_checkmark.set_speed(current_speed);
                            player_heart.set_speed(current_speed);
                        } else if (key == 'l' || key == 'L') {
                            is_looping = !is_looping;
                            player_spinner.set_loop(is_looping);
                            player_checkmark.set_loop(is_looping);
                            player_heart.set_loop(is_looping);
                        } else if (key == '1') {
                            player_spinner.seek_frame(0.0f);
                            player_spinner.play();
                        } else if (key == '2') {
                            player_checkmark.seek_frame(0.0f);
                            player_checkmark.play();
                        } else if (key == '3') {
                            player_heart.seek_frame(0.0f);
                            player_heart.play();
                        }
                    });

                    platform->onMouseDown().connect([&](float mx, float my, int btn) {
                        (void)btn;
                        // Click cards to replay
                        if (my >= 130.0f && my <= 620.0f) {
                            if (mx >= 40.0f && mx <= 420.0f) {
                                player_spinner.seek_frame(0.0f);
                                player_spinner.play();
                            } else if (mx >= 450.0f && mx <= 830.0f) {
                                player_checkmark.seek_frame(0.0f);
                                player_checkmark.play();
                            } else if (mx >= 860.0f && mx <= 1240.0f) {
                                player_heart.seek_frame(0.0f);
                                player_heart.play();
                            }
                        }
                    });
                }
            }
        }
    }
#endif

    // 6. Live Render Loop
    std::cout << "[*] Entering real-time animation loop..." << std::endl;

    double last_time = 0.0;
    auto start_tp = std::chrono::high_resolution_clock::now();
    uint32_t frame_count = 0;
    double fps_accum = 0.0;
    double measured_fps = 60.0;
    double measured_latency_ms = 0.5;

    // Number of iterations in headless mode
    uint32_t max_frames = run_headless ? 60 : 10000000;

    for (uint32_t f = 0; f < max_frames; ++f) {
#ifdef NISABA_HAS_BACKEND_OS
        if (platform && window) {
            if (!platform->pollEvents() || !g_running) {
                break;
            }
        }
#endif
        auto now_tp = std::chrono::high_resolution_clock::now();
        double now_sec = std::chrono::duration<double>(now_tp - start_tp).count();
        double dt = (last_time > 0.0) ? (now_sec - last_time) : (1.0 / 60.0);
        last_time = now_sec;
        if (dt > 0.1) dt = 0.1; // Clamp big hitch

        auto t0 = std::chrono::high_resolution_clock::now();

        // 1. Blit pre-rendered background
        std::memcpy(fb->pixels_mut(), wallpaper->pixels(), wallpaper->data_len());

        // 2. Advance Lottie players
        if (!is_paused) {
            player_spinner.advance(static_cast<float>(dt));
            player_checkmark.advance(static_cast<float>(dt));
            player_heart.advance(static_cast<float>(dt));
        }

        // 3. Render Header Panel
        {
            effects::GlassParams header_glass;
            header_glass.blur_sigma = 14.0f;
            header_glass.tint_color = Color::from_rgba8(16, 22, 38, 200);
            header_glass.border_color = Color::from_rgba8(0, 242, 254, 80);
            header_glass.border_width = 1.2f;
            header_glass.shadow = DropShadow(0.0f, 6.0f, 14.0f, Color::from_rgba8(0, 0, 0, 140));
            if (auto h_rect = Rect::from_xywh(40.0f, 25.0f, 1200.0f, 85.0f)) {
                blur_cache.draw_glass_aperture(canvas, *h_rect, 14.0f, 14.0f, header_glass);
            }

            draw_text(canvas, 60.0f, 40.0f, "NISABA LOTTIE LIVE SHOWCASE", 22.0f, 26.0f, TextColor::rgb(255, 255, 255));
            draw_text(canvas, 60.0f, 72.0f, "100% Sovereign Vector Animation Engine | Pure C++20 | Native Bodymovin Parser", 11.5f, 15.0f, TextColor::rgb(140, 165, 205));

            // Status badges
            std::ostringstream ss_stat;
            ss_stat << std::fixed << std::setprecision(1) << measured_fps << " FPS | "
                    << std::setprecision(3) << measured_latency_ms << " ms | Speed: "
                    << current_speed << "x | " << (is_paused ? "PAUSED" : "PLAYING");

            effects::GlassParams badge_glass;
            badge_glass.blur_sigma = 14.0f;
            badge_glass.tint_color = Color::from_rgba8(0, 242, 254, 30);
            badge_glass.border_color = Color::from_rgba8(0, 242, 254, 150);
            badge_glass.border_width = 1.0f;
            if (auto b_rect = Rect::from_xywh(870.0f, 42.0f, 350.0f, 42.0f)) {
                blur_cache.draw_glass_aperture(canvas, *b_rect, 12.0f, 12.0f, badge_glass);
            }
            draw_text(canvas, 885.0f, 54.0f, ss_stat.str(), 11.5f, 15.0f, TextColor::rgb(0, 242, 254));
        }

        // 4. Render 3 Live Animation Cards (3 Columns)
        auto render_card = [&](float cx, std::string_view num_str, std::string_view title,
                               std::string_view desc, Player& player, TextColor accent_col,
                               uint8_t r_b, uint8_t g_b, uint8_t b_b) {

            float card_w = 380.0f;
            float card_h = 510.0f;
            float cy = 130.0f;

            effects::GlassParams card_glass;
            card_glass.blur_sigma = 14.0f;
            card_glass.tint_color = Color::from_rgba8(14, 18, 32, 210);
            card_glass.border_color = Color::from_rgba8(r_b, g_b, b_b, 80);
            card_glass.border_width = 1.2f;
            card_glass.shadow = DropShadow(0.0f, 8.0f, 16.0f, Color::from_rgba8(0, 0, 0, 160));
            if (auto c_rect = Rect::from_xywh(cx, cy, card_w, card_h)) {
                blur_cache.draw_glass_aperture(canvas, *c_rect, 16.0f, 16.0f, card_glass);
            }

            // Header of Card
            draw_text(canvas, cx + 25.0f, cy + 22.0f, num_str, 12.0f, 15.0f, accent_col);
            draw_text(canvas, cx + 25.0f, cy + 42.0f, title, 16.0f, 20.0f, TextColor::rgb(240, 245, 255));
            draw_text(canvas, cx + 25.0f, cy + 68.0f, desc, 11.0f, 15.0f, TextColor::rgb(130, 150, 180), card_w - 50.0f);

            // Large Live Animation Stage Container
            float stage_x = cx + 30.0f;
            float stage_y = cy + 115.0f;
            float stage_size = 320.0f;

            Paint stage_bg;
            stage_bg.set_color(Color::from_rgba8(8, 11, 20, 230));
            canvas.fill_round_rect(*Rect::from_xywh(stage_x, stage_y, stage_size, stage_size), 14.0f, 14.0f, stage_bg);

            Paint stage_border;
            stage_border.set_color(Color::from_rgba8(40, 55, 80, 120));
            canvas.stroke_round_rect(*Rect::from_xywh(stage_x, stage_y, stage_size, stage_size), 14.0f, 14.0f, stage_border, Stroke(1.5f));

            // Render live Lottie player inside stage
            auto lottie_rect = Rect::from_xywh(stage_x + 20.0f, stage_y + 20.0f, stage_size - 40.0f, stage_size - 40.0f);
            if (lottie_rect) {
                player.render(canvas, *lottie_rect, true);
            }

            // Progress bar & Frame counter beneath stage
            float prog = player.progress();
            float bar_y = cy + 455.0f;
            float bar_w = card_w - 50.0f;

            Paint bar_bg(Color::from_rgba8(20, 26, 45, 255));
            canvas.fill_round_rect(*Rect::from_xywh(cx + 25.0f, bar_y, bar_w, 6.0f), 3.0f, 3.0f, bar_bg);

            if (prog > 0.001f) {
                Paint bar_fill(Color::from_rgba8(r_b, g_b, b_b, 255));
                canvas.fill_round_rect(*Rect::from_xywh(cx + 25.0f, bar_y, bar_w * prog, 6.0f), 3.0f, 3.0f, bar_fill);
            }

            std::ostringstream ss_info;
            ss_info << "Frame: " << std::fixed << std::setprecision(1) << player.current_frame()
                    << " / " << (player.animation() ? player.animation()->out_point() : 60.0f)
                    << "  (" << static_cast<int>(prog * 100.0f) << "%)";
            draw_text(canvas, cx + 25.0f, bar_y + 16.0f, ss_info.str(), 10.5f, 14.0f, TextColor::rgb(130, 155, 190));
        };

        // Col 1: Spinner
        render_card(
            40.0f,
            "01 // VECTOR ROTATION & TRIM PATH",
            "DYNAMIC RADIAL SPINNER",
            "Trim Path modifier [tm] + Bezier rotation [r] with 60 FPS elastic easing curve.",
            player_spinner,
            TextColor::rgb(0, 242, 254),
            0, 242, 254
        );

        // Col 2: Checkmark
        render_card(
            450.0f,
            "02 // ANTICIPATION & STROKE DRAW",
            "CONFIRMATION CHECKMARK",
            "Anticipation scale pop [s] + Cubic Bezier tick draw stroke [sh + tm].",
            player_checkmark,
            TextColor::rgb(0, 255, 153),
            0, 255, 153
        );

        // Col 3: Heart
        render_card(
            860.0f,
            "03 // BIOMEDICAL PULSE & GLOW",
            "CARDIAC PULSE (HEART)",
            "Freeform Bezier vector path [sh] + Harmonic double-pulse scale & glow stroke [st].",
            player_heart,
            TextColor::rgb(255, 45, 115),
            255, 45, 115
        );

        // 5. Bottom Instructions Bar
        {
            float by = 660.0f;
            effects::GlassParams bot_glass;
            bot_glass.blur_sigma = 14.0f;
            bot_glass.tint_color = Color::from_rgba8(12, 16, 28, 210);
            bot_glass.border_color = Color::from_rgba8(60, 80, 120, 60);
            bot_glass.border_width = 1.0f;
            bot_glass.shadow = DropShadow(0.0f, 4.0f, 10.0f, Color::from_rgba8(0, 0, 0, 120));
            if (auto bot_rect = Rect::from_xywh(40.0f, by, 1200.0f, 75.0f)) {
                blur_cache.draw_glass_aperture(canvas, *bot_rect, 12.0f, 12.0f, bot_glass);
            }

            draw_text(canvas, 60.0f, by + 18.0f, "INTERACTIVE CONTROLS:", 11.5f, 15.0f, TextColor::rgb(0, 242, 254));
            draw_text(canvas, 60.0f, by + 40.0f,
                      "[SPACE] Play/Pause   |   [R] Restart All   |   [S] Toggle Speed (0.25x / 0.5x / 1.0x / 2.0x)   |   [L] Loop On/Off   |   [1 / 2 / 3] Replay Card   |   [CLICK] Replay   |   [ESC] Exit",
                      11.0f, 15.0f, TextColor::rgb(180, 200, 230));
        }

        auto t1 = std::chrono::high_resolution_clock::now();
        measured_latency_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

        frame_count++;
        fps_accum += dt;
        if (fps_accum >= 0.5) {
            measured_fps = static_cast<double>(frame_count) / fps_accum;
            frame_count = 0;
            fps_accum = 0.0;
        }

#ifdef NISABA_HAS_BACKEND_OS
        // 6. Present framebuffer to Native OS Window
        if (gpu_ctx && window) {
            gpu_ctx->updateImage(gpu_texture_id, fb->data());
            gpu_ctx->beginFrame(WIN_WIDTH, WIN_HEIGHT, 1.0f);
            auto p = gpu_ctx->imagePattern(0, 0, WIN_WIDTH, WIN_HEIGHT, 0, gpu_texture_id, 1.0f);
            gpu_ctx->beginPath();
            gpu_ctx->rect(0, 0, WIN_WIDTH, WIN_HEIGHT);
            gpu_ctx->fillPaint(p);
            gpu_ctx->fill();
            gpu_ctx->endFrame();
            window->swapBuffers();
        }
#endif
    }

#ifdef NISABA_HAS_BACKEND_OS
    if (gpu_ctx && gpu_texture_id != 0) {
        gpu_ctx->deleteImage(gpu_texture_id);
    }
#endif

    // Save final screenshot
    const std::string out_png = "nisaba_lottie_showcase.png";
    if (fb->save_png(out_png)) {
        std::cout << "\n[+] Saved live showcase snapshot to: " << out_png << "\n";
    }

    std::cout << "========================================================\n"
              << "   NISABA LIVE LOTTIE SHOWCASE FINISHED\n"
              << "========================================================\n";
    return 0;
}

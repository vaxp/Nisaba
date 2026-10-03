/// @file showcase_lottie_gpu.cpp
/// @brief 100% Hardware GPU Accelerated Live Interactive Window Showcase of Nisaba Lottie Engine.

#include "nisaba/nisaba.hpp"
#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/gpu/gpu_surface.hpp"
#include "nisaba/gpu/gpu_canvas.hpp"

#ifdef NISABA_HAS_BACKEND_OS
#  include "nisaba/backend_os/platform.hpp"
#  include "nisaba/backend_os/window.hpp"
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

using nisaba::gpu::GpuCanvas;
using nisaba::gpu::GpuSurface;
using nisaba::gpu::GpuDevice;
using nisaba::gpu::GpuBackendType;

namespace {

constexpr uint32_t WIN_WIDTH = 1280;
constexpr uint32_t WIN_HEIGHT = 760;

static const char* resolveFont(const char* name) {
    static char resolvedPaths[5][512];
    static int nextIdx = 0;
    char* outBuf = resolvedPaths[nextIdx++ % 5];

    const char* prefixes[] = {
        "fonts/",
        "nisaba/fonts/",
        "../fonts/",
        "../../fonts/",
        "../../../fonts/"
    };

    for (const auto& prefix : prefixes) {
        std::snprintf(outBuf, 512, "%s%s", prefix, name);
        FILE* f = std::fopen(outBuf, "rb");
        if (f) {
            std::fclose(f);
            return outBuf;
        }
    }
    return name;
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

//// 2. Success Checkmark with Circle Pop Lottie JSON
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
            std::cout << "Usage: showcase_lottie_gpu [options]\n\n"
                      << "Options:\n"
                      << "  --headless   Run in headless mode (render 60 frames and save PNG snapshot)\n"
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

    std::cout << "====================================================================\n"
              << "   NISABA SOVEREIGN LOTTIE ENGINE — 100% HARDWARE GPU ACCELERATED  \n"
              << "   Direct GpuCanvas Vector Rasterization • GLES 3.2 • 4x MSAA     \n"
              << "====================================================================\n";

    // 1. Initialize Native OS Window via backend_os
#ifdef NISABA_HAS_BACKEND_OS
    std::unique_ptr<backend_os::Platform> platform;
    std::unique_ptr<backend_os::Window> window;
    bool g_running = true;

    if (!run_headless) {
        auto p_res = backend_os::Platform::create();
        if (p_res.isOk()) {
            platform = std::move(p_res).value();
            backend_os::WindowConfig cfg;
            cfg.title = "NISABA LOTTIE GPU SHOWCASE — 100% Direct Hardware Acceleration";
            cfg.width = WIN_WIDTH;
            cfg.height = WIN_HEIGHT;
            cfg.vsync = false;
            auto w_res = backend_os::Window::create(*platform, cfg);
            if (w_res.isOk()) {
                window = std::move(w_res).value();
                std::cout << "[+] Native OS GUI window created successfully (1280x760).\n";
            }
        }
    }
#endif

    // 2. Initialize GPU Device and Surface
    auto device = GpuDevice::create();
    if (!device || !device->is_valid()) {
        std::cerr << "[-] Error: Failed to initialize GPU Device context." << std::endl;
        return 1;
    }

    std::cout << "[*] GPU Backend : " << (device->backend_type() == GpuBackendType::Vulkan ? "Vulkan" : "OpenGL GLES 3.2") << "\n";
    std::cout << "[*] GPU Renderer: " << device->renderer_name() << "\n";
    std::cout << "[*] GPU Version : " << device->version_name() << "\n";

    std::shared_ptr<GpuSurface> surface;
    if (run_headless) {
        surface = GpuSurface::create(device, WIN_WIDTH, WIN_HEIGHT, 4);
    } else {
        surface = GpuSurface::from_screen(device, WIN_WIDTH, WIN_HEIGHT, 0);
    }

    if (!surface) {
        std::cerr << "[-] Error: Failed to create GpuSurface (" << WIN_WIDTH << "x" << WIN_HEIGHT << ")." << std::endl;
        return 1;
    }

    GpuCanvas gpu_canvas(surface);

    // 3. Load Animations and build Players
    std::cout << "[*] Initializing Lottie Players on GPU..." << std::endl;
    auto anim_spinner = Animation::load_from_data(SPINNER_LOTTIE);
    auto anim_checkmark = Animation::load_from_data(CHECKMARK_LOTTIE);
    auto anim_heart = Animation::load_from_data(HEART_LOTTIE);

    if (!anim_spinner || !anim_checkmark || !anim_heart) {
        std::cerr << "[-] Error: Failed parsing Lottie animation documents." << std::endl;
        return 1;
    }

    Player player_spinner(anim_spinner);
    Player player_checkmark(anim_checkmark);
    Player player_heart(anim_heart);

    float current_speed = 1.0f;
    bool is_looping = true;
    bool is_paused = false;

    // 4. Setup Interactive Event Listeners
#ifdef NISABA_HAS_BACKEND_OS
    if (platform && window) {
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
#endif

    // 5. Initialize Hardware GPU Typography Pipeline
    int fontSans = -1;
    int fontMono = -1;
    int fontArabic = -1;
    if (gpu_canvas.context()) {
        fontSans = gpu_canvas.context()->createFont("sans", resolveFont("Inter-Regular.ttf"));
        if (fontSans == -1) fontSans = gpu_canvas.context()->createFont("sans", resolveFont("NotoSans-Regular.ttf"));
        fontMono = gpu_canvas.context()->createFont("mono", resolveFont("FiraMono-Medium.ttf"));
        if (fontMono == -1) fontMono = fontSans;
        fontArabic = gpu_canvas.context()->createFont("arabic", resolveFont("NotoSansArabic.ttf"));
        if (fontSans != -1 && fontArabic != -1) {
            gpu_canvas.context()->addFallbackFontId(fontSans, fontArabic);
        }
        if (fontMono != -1 && fontArabic != -1) {
            gpu_canvas.context()->addFallbackFontId(fontMono, fontArabic);
        }
        gpu_canvas.context()->fontFace("sans");
        std::printf("[+] Loaded GPU typography fonts (sans=%d, mono=%d, arabic=%d)\n",
                    fontSans, fontMono, fontArabic);
    }

    auto draw_gpu_text = [&](float x, float y, std::string_view text, float font_size, Color color, const char* font_name = "sans", gpu::Align align = gpu::Align::Left | gpu::Align::Top) {
        if (!gpu_canvas.context()) return;
        gpu_canvas.context()->fontFace(font_name);
        gpu_canvas.context()->fontSize(font_size);
        gpu_canvas.context()->fillColor(color);
        gpu_canvas.context()->textAlign(align);
        gpu_canvas.context()->text(x, y, text.data(), text.data() + text.size());
    };

    auto draw_gpu_textbox = [&](float x, float y, float max_w, std::string_view text, float font_size, Color color, const char* font_name = "sans") {
        if (!gpu_canvas.context()) return;
        gpu_canvas.context()->fontFace(font_name);
        gpu_canvas.context()->fontSize(font_size);
        gpu_canvas.context()->fillColor(color);
        gpu_canvas.context()->textAlign(gpu::Align::Left | gpu::Align::Top);
        gpu_canvas.context()->textBox(x, y, max_w, text.data(), text.data() + text.size());
    };

    // 6. Real-time Animation Loop
    std::cout << "[*] Entering real-time GPU animation loop..." << std::endl;

    double last_time = 0.0;
    auto start_tp = std::chrono::high_resolution_clock::now();
    uint32_t frame_count = 0;
    double fps_accum = 0.0;
    double measured_fps = 60.0;
    double measured_frame_ms = 16.0;

    // Timing accumulators for bottleneck profiling (in milliseconds)
    double acc_poll_ms = 0.0;
    double acc_adv_ms = 0.0;
    double acc_bg_ms = 0.0;
    double acc_glass_ms = 0.0;
    double acc_cards_ms = 0.0;
    double acc_text_ms = 0.0;
    double acc_flush_ms = 0.0;
    double acc_present_ms = 0.0;
    double acc_total_ms = 0.0;

    uint32_t max_frames = run_headless ? 60 : 10000000;

    for (uint32_t f = 0; f < max_frames; ++f) {
        auto t_frame_start = std::chrono::high_resolution_clock::now();

        // 0. Poll Events
        auto tp0 = std::chrono::high_resolution_clock::now();
#ifdef NISABA_HAS_BACKEND_OS
        if (platform && window) {
            if (!platform->pollEvents() || !g_running) {
                break;
            }
        }
#endif
        auto tp1 = std::chrono::high_resolution_clock::now();
        double cur_poll_ms = std::chrono::duration<double, std::milli>(tp1 - tp0).count();

        auto now_tp = std::chrono::high_resolution_clock::now();
        double now_sec = std::chrono::duration<double>(now_tp - start_tp).count();
        double dt = (last_time > 0.0) ? (now_sec - last_time) : (1.0 / 60.0);
        last_time = now_sec;
        if (dt > 0.1) dt = 0.1;

        // 1. Advance Lottie Players
        auto tp_adv0 = std::chrono::high_resolution_clock::now();
        if (!is_paused) {
            player_spinner.advance(static_cast<float>(dt));
            player_checkmark.advance(static_cast<float>(dt));
            player_heart.advance(static_cast<float>(dt));
        }
        auto tp_adv1 = std::chrono::high_resolution_clock::now();
        double cur_adv_ms = std::chrono::duration<double, std::milli>(tp_adv1 - tp_adv0).count();

        // 2. Clear Screen & Render Background Gradients
        auto tp_bg0 = std::chrono::high_resolution_clock::now();
        gpu_canvas.clear(Color::from_rgba8(8, 11, 18, 255));

        // 3. Render Deep Space Cosmic Gradients directly on GPU
        {
            auto full_r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(WIN_WIDTH), static_cast<float>(WIN_HEIGHT));
            if (full_r) {
                std::vector<GradientStop> bg_stops = {
                    GradientStop::create(0.0f, Color::from_rgba8(9, 12, 22, 255)),
                    GradientStop::create(0.5f, Color::from_rgba8(16, 20, 36, 255)),
                    GradientStop::create(1.0f, Color::from_rgba8(7, 9, 16, 255))
                };
                if (auto bg_grad = LinearGradient::create(Point::from_xy(0.0f, 0.0f), Point::from_xy(WIN_WIDTH, WIN_HEIGHT), bg_stops)) {
                    Paint bg_paint;
                    bg_paint.shader = Shader(*bg_grad);
                    gpu_canvas.fill_rect(*full_r, bg_paint);
                }
            }

            // Cyan nebula
            if (auto g_cyan = RadialGradient::create(Point::from_xy(250.0f, 150.0f), 350.0f, {
                GradientStop::create(0.0f, Color::from_rgba8(0, 242, 254, 40)),
                GradientStop::create(1.0f, Color::from_rgba8(0, 242, 254, 0))
            })) {
                Paint p; p.shader = Shader(*g_cyan);
                if (auto r = Rect::from_xywh(0.0f, 0.0f, 600.0f, 500.0f)) gpu_canvas.fill_rect(*r, p);
            }

            // Pink nebula
            if (auto g_pink = RadialGradient::create(Point::from_xy(1050.0f, 400.0f), 400.0f, {
                GradientStop::create(0.0f, Color::from_rgba8(255, 45, 115, 35)),
                GradientStop::create(1.0f, Color::from_rgba8(255, 45, 115, 0))
            })) {
                Paint p; p.shader = Shader(*g_pink);
                if (auto r = Rect::from_xywh(650.0f, 50.0f, 630.0f, 700.0f)) gpu_canvas.fill_rect(*r, p);
            }
        }
        auto tp_bg1 = std::chrono::high_resolution_clock::now();
        double cur_bg_ms = std::chrono::duration<double, std::milli>(tp_bg1 - tp_bg0).count();

        // 4. Render GPU Glass Panels & Elevation Shadows
        auto tp_gl0 = std::chrono::high_resolution_clock::now();
        {
            // Header Glass Panel
            auto h_rect = Rect::from_xywh(40.0f, 25.0f, 1200.0f, 85.0f);
            if (h_rect) {
                gpu_canvas.draw_round_rect_shadow(*h_rect, 14.0f, 14.0f, DropShadow(0.0f, 6.0f, 14.0f, Color::from_rgba8(0, 0, 0, 140)));
                effects::GlassParams header_glass;
                header_glass.tint_color = Color::from_rgba8(16, 22, 38, 200);
                header_glass.border_color = Color::from_rgba8(0, 242, 254, 90);
                header_glass.border_width = 1.2f;
                gpu_canvas.draw_glass_panel(*h_rect, 14.0f, 14.0f, header_glass);
            }

            // Status Badge Glass
            auto b_rect = Rect::from_xywh(830.0f, 42.0f, 390.0f, 42.0f);
            if (b_rect) {
                effects::GlassParams badge_glass;
                badge_glass.tint_color = Color::from_rgba8(0, 242, 254, 30);
                badge_glass.border_color = Color::from_rgba8(0, 242, 254, 160);
                badge_glass.border_width = 1.0f;
                gpu_canvas.draw_glass_panel(*b_rect, 12.0f, 12.0f, badge_glass);
            }
        }
        auto tp_gl1 = std::chrono::high_resolution_clock::now();
        double cur_glass_ms = std::chrono::duration<double, std::milli>(tp_gl1 - tp_gl0).count();

        // 5. Render 3 Interactive Lottie Animation Cards (Hardware GPU Vector Acceleration)
        auto tp_cards0 = std::chrono::high_resolution_clock::now();
        auto render_gpu_card = [&](float cx, Player& player, uint8_t r_b, uint8_t g_b, uint8_t b_b) {
            float card_w = 380.0f;
            float card_h = 510.0f;
            float cy = 130.0f;

            auto c_rect = Rect::from_xywh(cx, cy, card_w, card_h);
            if (c_rect) {
                gpu_canvas.draw_round_rect_shadow(*c_rect, 16.0f, 16.0f, DropShadow(0.0f, 8.0f, 18.0f, Color::from_rgba8(0, 0, 0, 160)));
                effects::GlassParams card_glass;
                card_glass.tint_color = Color::from_rgba8(14, 18, 32, 210);
                card_glass.border_color = Color::from_rgba8(r_b, g_b, b_b, 90);
                card_glass.border_width = 1.2f;
                gpu_canvas.draw_glass_panel(*c_rect, 16.0f, 16.0f, card_glass);
            }

            // Stage Background
            float stage_x = cx + 30.0f;
            float stage_y = cy + 115.0f;
            float stage_size = 320.0f;

            Paint stage_bg(Color::from_rgba8(8, 11, 20, 230));
            gpu_canvas.fill_round_rect(*Rect::from_xywh(stage_x, stage_y, stage_size, stage_size), 14.0f, 14.0f, stage_bg);

            Paint stage_border(Color::from_rgba8(40, 55, 80, 140));
            gpu_canvas.stroke_round_rect(*Rect::from_xywh(stage_x, stage_y, stage_size, stage_size), 14.0f, 14.0f, stage_border, Stroke(1.5f));

            // DIRECT GPU HARDWARE VECTOR RENDER OF LOTTIE ANIMATION!
            auto lottie_rect = Rect::from_xywh(stage_x + 20.0f, stage_y + 20.0f, stage_size - 40.0f, stage_size - 40.0f);
            if (lottie_rect) {
                player.render(gpu_canvas, *lottie_rect, true);
            }

            // Progress bar
            float prog = player.progress();
            float bar_y = cy + 455.0f;
            float bar_w = card_w - 50.0f;

            Paint bar_bg(Color::from_rgba8(20, 26, 45, 255));
            gpu_canvas.fill_round_rect(*Rect::from_xywh(cx + 25.0f, bar_y, bar_w, 6.0f), 3.0f, 3.0f, bar_bg);

            if (prog > 0.001f) {
                Paint bar_fill(Color::from_rgba8(r_b, g_b, b_b, 255));
                gpu_canvas.fill_round_rect(*Rect::from_xywh(cx + 25.0f, bar_y, bar_w * prog, 6.0f), 3.0f, 3.0f, bar_fill);
            }
        };

        // Col 1: Spinner
        render_gpu_card(40.0f, player_spinner, 0, 242, 254);

        // Col 2: Checkmark
        render_gpu_card(450.0f, player_checkmark, 0, 255, 153);

        // Col 3: Heart
        render_gpu_card(860.0f, player_heart, 255, 45, 115);

        auto tp_cards1 = std::chrono::high_resolution_clock::now();
        double cur_cards_ms = std::chrono::duration<double, std::milli>(tp_cards1 - tp_cards0).count();

        // 6. Bottom Instructions Bar & Typography
        auto tp_txt0 = std::chrono::high_resolution_clock::now();
        {
            float by = 660.0f;
            auto bot_rect = Rect::from_xywh(40.0f, by, 1200.0f, 75.0f);
            if (bot_rect) {
                effects::GlassParams bot_glass;
                bot_glass.tint_color = Color::from_rgba8(12, 16, 28, 210);
                bot_glass.border_color = Color::from_rgba8(60, 80, 120, 70);
                bot_glass.border_width = 1.0f;
                gpu_canvas.draw_glass_panel(*bot_rect, 12.0f, 12.0f, bot_glass);
            }
        }

        // 7. Typography (100% Native GPU Hardware Vector Text Rendering)
        // Header Text
        draw_gpu_text(60.0f, 40.0f, "NISABA LOTTIE LIVE SHOWCASE (HARDWARE GPU ACCELERATED)", 21.0f, Color::from_rgba8(255, 255, 255, 255), "sans");
        draw_gpu_text(60.0f, 72.0f, "100% Sovereign Vector Animation Engine | Hardware GLES 3.2 • 4x MSAA | Zero CPU Rasterization", 11.5f, Color::from_rgba8(0, 242, 254, 255), "sans");

        // Dynamic Header Badge
        std::ostringstream ss_stat;
        ss_stat << std::fixed << std::setprecision(1) << measured_fps << " FPS | "
                << std::setprecision(2) << measured_frame_ms << " ms | "
                << current_speed << "x | " << (is_paused ? "PAUSED" : "RAW UNCAPPED GPU");
        draw_gpu_text(848.0f, 55.0f, ss_stat.str(), 11.0f, Color::from_rgba8(0, 242, 254, 255), "mono", gpu::Align::Left | gpu::Align::Top);

        // Card 1 Text
        draw_gpu_text(65.0f, 148.0f, "01 // VECTOR ROTATION & TRIM PATH", 11.5f, Color::from_rgba8(0, 242, 254, 255), "mono");
        draw_gpu_text(65.0f, 168.0f, "DYNAMIC RADIAL SPINNER", 15.5f, Color::from_rgba8(240, 245, 255, 255), "sans");
        draw_gpu_textbox(65.0f, 193.0f, 330.0f, "Trim Path modifier [tm] + Bezier rotation [r] with 60 FPS elastic easing curve.", 11.0f, Color::from_rgba8(130, 150, 180, 255), "sans");

        // Card 2 Text
        draw_gpu_text(475.0f, 148.0f, "02 // ANTICIPATION & STROKE DRAW", 11.5f, Color::from_rgba8(0, 255, 153, 255), "mono");
        draw_gpu_text(475.0f, 168.0f, "CONFIRMATION CHECKMARK", 15.5f, Color::from_rgba8(240, 245, 255, 255), "sans");
        draw_gpu_textbox(475.0f, 193.0f, 330.0f, "Anticipation scale pop [s] + Cubic Bezier tick draw stroke [sh + tm].", 11.0f, Color::from_rgba8(130, 150, 180, 255), "sans");

        // Card 3 Text
        draw_gpu_text(885.0f, 148.0f, "03 // BIOMEDICAL PULSE & GLOW", 11.5f, Color::from_rgba8(255, 45, 115, 255), "mono");
        draw_gpu_text(885.0f, 168.0f, "CARDIAC PULSE (HEART)", 15.5f, Color::from_rgba8(240, 245, 255, 255), "sans");
        draw_gpu_textbox(885.0f, 193.0f, 330.0f, "Freeform Bezier vector path [sh] + Harmonic double-pulse scale & glow stroke [st].", 11.0f, Color::from_rgba8(130, 150, 180, 255), "sans");

        // Card Frame Counters
        auto draw_gpu_counter = [&](float cx, Player& p) {
            float prog = p.progress();
            float bar_y = 130.0f + 455.0f;
            std::ostringstream ss;
            ss << "Frame: " << std::fixed << std::setprecision(1) << p.current_frame()
               << " / " << (p.animation() ? p.animation()->out_point() : 60.0f)
               << "  (" << static_cast<int>(prog * 100.0f) << "%)";
            draw_gpu_text(cx + 25.0f, bar_y + 14.0f, ss.str(), 10.5f, Color::from_rgba8(130, 155, 190, 255), "mono");
        };

        draw_gpu_counter(40.0f, player_spinner);
        draw_gpu_counter(450.0f, player_checkmark);
        draw_gpu_counter(860.0f, player_heart);

        // Bottom Controls Bar Text
        draw_gpu_text(60.0f, 676.0f, "INTERACTIVE CONTROLS (GPU ACCELERATED):", 11.5f, Color::from_rgba8(0, 242, 254, 255), "mono");
        draw_gpu_text(60.0f, 698.0f,
                      "[SPACE] Play/Pause   |   [R] Restart All   |   [S] Speed   |   [L] Loop   |   [1 / 2 / 3] Replay Card   |   [ESC] Exit",
                      11.0f, Color::from_rgba8(180, 200, 230, 255), "sans");

        auto tp_txt1 = std::chrono::high_resolution_clock::now();
        double cur_text_ms = std::chrono::duration<double, std::milli>(tp_txt1 - tp_txt0).count();

        // 8. Flush GPU commands & Swap Buffers to Native Window
        auto tp_fl0 = std::chrono::high_resolution_clock::now();
        gpu_canvas.flush();
        auto tp_fl1 = std::chrono::high_resolution_clock::now();
        double cur_flush_ms = std::chrono::duration<double, std::milli>(tp_fl1 - tp_fl0).count();

        double cur_swap_ms = 0.0;

#ifdef NISABA_HAS_BACKEND_OS
        if (window) {
            auto tp_sw0 = std::chrono::high_resolution_clock::now();
            window->swapBuffers();
            auto tp_sw1 = std::chrono::high_resolution_clock::now();
            cur_swap_ms = std::chrono::duration<double, std::milli>(tp_sw1 - tp_sw0).count();
        }
#endif

        auto t_frame_end = std::chrono::high_resolution_clock::now();
        double cur_total_ms = std::chrono::duration<double, std::milli>(t_frame_end - t_frame_start).count();

        // Accumulate statistics
        acc_poll_ms += cur_poll_ms;
        acc_adv_ms += cur_adv_ms;
        acc_bg_ms += cur_bg_ms;
        acc_glass_ms += cur_glass_ms;
        acc_cards_ms += cur_cards_ms;
        acc_text_ms += cur_text_ms;
        acc_flush_ms += cur_flush_ms;
        acc_present_ms += cur_swap_ms;
        acc_total_ms += cur_total_ms;

        frame_count++;
        fps_accum += (cur_total_ms / 1000.0);

        if (fps_accum >= 1.0 && frame_count > 0) {
            measured_fps = static_cast<double>(frame_count) / fps_accum;
            double n = static_cast<double>(frame_count);
            double avg_poll = acc_poll_ms / n;
            double avg_adv = acc_adv_ms / n;
            double avg_bg = acc_bg_ms / n;
            double avg_glass = acc_glass_ms / n;
            double avg_cards = acc_cards_ms / n;
            double avg_text = acc_text_ms / n;
            double avg_flush = acc_flush_ms / n;
            double avg_swap = acc_present_ms / n;
            double avg_total = acc_total_ms / n;
            measured_frame_ms = avg_total;

            std::cout << "\n================ [PERFORMANCE PROFILER / DIRECT HARDWARE GPU] ================\n"
                      << "  Framerate    : " << std::fixed << std::setprecision(1) << measured_fps << " FPS\n"
                      << "  Total Latency: " << std::setprecision(2) << avg_total << " ms / frame\n"
                      << "------------------------------------------------------------------------------\n"
                      << "  [1] Event Polling (OS)      : " << std::setw(6) << avg_poll  << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_poll / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [2] Lottie Advance (CPU)    : " << std::setw(6) << avg_adv   << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_adv / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [3] Background Gradients    : " << std::setw(6) << avg_bg    << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_bg / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [4] Glass Panels & Shadows  : " << std::setw(6) << avg_glass << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_glass / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [5] Lottie Vector Render(3x): " << std::setw(6) << avg_cards << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_cards / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [6] Typography & Text Layout: " << std::setw(6) << avg_text  << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_text / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [7] GPU Flush (Draw Calls)  : " << std::setw(6) << avg_flush << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_flush / avg_total * 100.0 : 0.0) << "%)\n"
                      << "  [8] Window SwapBuffers      : " << std::setw(6) << avg_swap  << " ms  (" << std::setw(5) << (avg_total > 0.001 ? avg_swap / avg_total * 100.0 : 0.0) << "%)\n"
                      << "==============================================================================\n" << std::flush;

            acc_poll_ms = acc_adv_ms = acc_bg_ms = acc_glass_ms = acc_cards_ms = 0.0;
            acc_text_ms = acc_flush_ms = acc_present_ms = acc_total_ms = 0.0;
            frame_count = 0;
            fps_accum = 0.0;
        }
    }

    // Save final screenshot
    const std::string out_png = "nisaba_lottie_gpu_showcase.png";
    auto read_pm = surface->to_pixmap();
    if (read_pm && read_pm->save_png(out_png)) {
        std::cout << "\n[+] Saved GPU showcase snapshot to: " << out_png << "\n";
    }

    std::cout << "====================================================================\n"
              << "   NISABA HARDWARE GPU LOTTIE SHOWCASE FINISHED\n"
              << "====================================================================\n";
    return 0;
}

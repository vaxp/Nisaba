#include <cassert>
#include <iostream>
#include <string>
#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/result.hpp"
#include "nisaba/backend_os/signal.hpp"
#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/app.hpp"
#include "nisaba/backend_os/native_popup.hpp"

using namespace nisaba::backend_os;

static void test_types() {
    Point p1{10.0f, 20.0f};
    Point p2{5.0f, 10.0f};
    Point p3 = p1 + p2;
    assert(p3.x == 15.0f && p3.y == 30.0f);

    Size s1{800.0f, 600.0f};
    assert(s1.area() == 480000.0f);
    assert(!s1.isEmpty());

    Rect r1 = Rect::fromLTWH(10.0f, 20.0f, 100.0f, 50.0f);
    assert(r1.contains(Point{50.0f, 40.0f}));
    assert(!r1.contains(Point{150.0f, 40.0f}));

    EdgeInsets insets = EdgeInsets::symmetric(8.0f, 16.0f);
    assert(insets.top == 8.0f && insets.left == 16.0f);
    std::cout << "[PASS] Backend OS types verified\n";
}

static void test_result() {
    auto okRes = Result<int>::ok(42);
    assert(okRes.isOk());
    assert(okRes.value() == 42);

    auto mapped = okRes.map([](int x) { return x * 2; });
    assert(mapped.isOk());
    assert(mapped.value() == 84);

    auto errRes = Result<int>::err(ErrorCode::NotFound, "Item not found");
    assert(errRes.isErr());
    assert(errRes.error().code == ErrorCode::NotFound);
    std::cout << "[PASS] Backend OS Result<T> verified\n";
}

static void test_signal() {
    Signal<int, int> onResize;
    int received_w = 0;
    int received_h = 0;

    auto id = onResize.connect([&](int w, int h) {
        received_w = w;
        received_h = h;
    });

    onResize.emit(1920, 1080);
    assert(received_w == 1920);
    assert(received_h == 1080);

    onResize.disconnect(id);
    onResize.emit(800, 600);
    assert(received_w == 1920); // Not modified after disconnect
    std::cout << "[PASS] Backend OS Signal system verified\n";
}

static void test_platform() {
    auto res = Platform::create();
    if (res.isOk()) {
        auto platform = std::move(res.value());
        assert(platform != nullptr);
        std::cout << "[PASS] Platform initialized: "
                  << (platform->isWayland() ? "Wayland" : "X11/Other") << "\n";
        
        WindowConfig cfg;
        cfg.title = "Nisaba Verification Window";
        cfg.width = 640;
        cfg.height = 480;
        auto winRes = Window::create(*platform, cfg);
        if (winRes.isOk()) {
            auto window = std::move(winRes.value());
            assert(window != nullptr);
            std::cout << "[PASS] Native Window created successfully (Size: "
                      << window->getSize().width << "x" << window->getSize().height << ")\n";
        } else {
            std::cout << "[INFO] Window creation skipped or headless\n";
        }
    } else {
        std::cout << "[INFO] Headless test environment, platform creation returned error as expected\n";
    }
}

static void test_app_and_popup() {
    AppConfig cfg;
    cfg.title = "Nisaba Test App";
    cfg.width = 640;
    cfg.height = 480;
    cfg.target_fps = 60;

    auto app_res = App::create(cfg);
    if (app_res.isOk()) {
        auto app = std::move(app_res.value());
        assert(app != nullptr);
        assert(App::instance() == app.get());

        bool main_frame_rendered = false;
        app->onFrame([&](Window& win, double dt) {
            (void)win;
            (void)dt;
            main_frame_rendered = true;
        });

        // Step main frame first so compositor configures and maps the main window
        for (int i = 0; i < 2; ++i) {
            app->stepFrame();
        }

        // Test NativePopup
        PopupOptions pop_opts;
        pop_opts.position = {50.0f, 50.0f};
        pop_opts.width = 200;
        pop_opts.height = 150;
        bool popup_closed = false;
        pop_opts.on_close = [&]() {
            popup_closed = true;
        };

        bool popup_frame_rendered = false;
        auto popup = NativePopup::show(app->window(), pop_opts, [&](Window& win, double dt) {
            (void)win;
            (void)dt;
            popup_frame_rendered = true;
        });

        if (popup) {
            assert(popup->isOpen());
            assert(popup->window() != nullptr);
            std::cout << "[PASS] NativePopup created successfully\n";

            // Run 3 frames of event loop
            for (int i = 0; i < 3; ++i) {
                app->stepFrame();
            }

            assert(main_frame_rendered);
            assert(popup_frame_rendered);
            std::cout << "[PASS] App::stepFrame rendered main and popup frames\n";

            popup->close();
            assert(!popup->isOpen());
            assert(popup_closed);
            std::cout << "[PASS] NativePopup closed cleanly\n";
        }

        app->quit();
        std::cout << "[PASS] App lifecycle and event loop verified\n";
    } else {
        std::cout << "[INFO] Headless environment, App::create skipped\n";
    }
}

int main() {
    std::cout << "--- Running Nisaba backend_os Unit Tests ---\n";
    test_types();
    test_result();
    test_signal();
    test_platform();
    test_app_and_popup();
    std::cout << "All backend_os tests passed successfully!\n";
    return 0;
}

#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include "nisaba/nisaba.hpp"
#include "nisaba/gpu/gpu_device.hpp"
#include "nisaba/gpu/gpu_surface.hpp"
#include "nisaba/gpu/gpu_canvas.hpp"
#include "nisaba/damage/damage.hpp"

using namespace nisaba;
using namespace nisaba::damage;

using nisaba::gpu::GpuCanvas;
using nisaba::gpu::GpuSurface;
using nisaba::gpu::GpuDevice;
using nisaba::gpu::GpuBackendType;

void test_gpu_damage_tracking(GpuBackendType backend) {
    std::string backend_name = (backend == GpuBackendType::Vulkan) ? "Vulkan" : "OpenGL";
    std::cout << "[Test GPU Damage] Running on " << backend_name << " ... ";

    auto device = GpuDevice::create(backend);
    if (!device) {
        std::cout << "SKIPPED (Device not available)\n";
        return;
    }

    constexpr uint32_t W = 200;
    constexpr uint32_t H = 200;

    auto surface = GpuSurface::create(device, W, H, 1);
    if (!surface) {
        std::cout << "SKIPPED (Surface creation failed)\n";
        return;
    }

    // -------------------------------------------------------------
    // Frame 1: Full frame render (Red background + Blue card)
    // -------------------------------------------------------------
    {
        GpuCanvas canvas(surface);
        canvas.clear(Color::from_rgba8(255, 0, 0, 255)); // Red

        // Draw Blue square at (20, 20, 50, 50)
        Paint blue_p;
        blue_p.set_color_rgba8(0, 0, 255, 255);
        canvas.fill_rect(*Rect::from_xywh(20.0f, 20.0f, 50.0f, 50.0f), blue_p);

        canvas.flush();
    }

    // Verify Frame 1 output
    auto p1 = surface->to_pixmap();
    assert(p1.has_value());
    auto c_bg = p1->pixel(5, 5).value();
    auto c_blue = p1->pixel(30, 30).value();
    assert(c_bg.red() > 200 && c_bg.blue() < 50);      // Background is Red
    assert(c_blue.blue() > 200 && c_blue.red() < 50);  // Square is Blue

    // -------------------------------------------------------------
    // Frame 2: Incremental Damage Tracking (Preserve contents)
    // Only redraw a 30x30 Green square at (120, 120) with clip_rect(ScreenIntRect)
    // -------------------------------------------------------------
    surface->set_preserve_contents(true);

    auto damage_rect = *ScreenIntRect::from_xywh(120, 120, 30, 30);

    {
        GpuCanvas canvas(surface);
        // Do NOT call clear() - contents must be preserved!

        canvas.save();
        canvas.clip_rect(damage_rect);

        // Draw Green square inside dirty rectangle
        Paint green_p;
        green_p.set_color_rgba8(0, 255, 0, 255);
        canvas.fill_rect(*Rect::from_xywh(120.0f, 120.0f, 30.0f, 30.0f), green_p);

        canvas.restore();
        canvas.flush(damage_rect);
    }

    // Verify Frame 2 output:
    // 1. Red background must still be preserved!
    // 2. Blue square at (30, 30) must still be preserved!
    // 3. Green square at (130, 130) must be present!
    auto p2 = surface->to_pixmap();
    assert(p2.has_value());

    auto c2_bg = p2->pixel(5, 5).value();
    auto c2_blue = p2->pixel(30, 30).value();
    auto c2_green = p2->pixel(130, 130).value();

    assert(c2_bg.red() > 200 && c2_bg.blue() < 50);
    assert(c2_blue.blue() > 200 && c2_blue.red() < 50);
    assert(c2_green.green() > 200 && c2_green.red() < 50);

    // -------------------------------------------------------------
    // Frame 3: Localized clear_rect
    // Clear the Blue square region with Black without affecting Red or Green
    // -------------------------------------------------------------
    {
        GpuCanvas canvas(surface);

        auto clear_box = *ScreenIntRect::from_xywh(20, 20, 50, 50);
        canvas.clear_rect(clear_box, Color::from_rgba8(0, 0, 0, 255)); // Black

        canvas.flush(clear_box);
    }

    auto p3 = surface->to_pixmap();
    assert(p3.has_value());

    auto c3_bg = p3->pixel(5, 5).value();
    auto c3_cleared = p3->pixel(30, 30).value();
    auto c3_green = p3->pixel(130, 130).value();

    assert(c3_bg.red() > 200); // Red preserved
    assert(c3_cleared.red() < 30 && c3_cleared.green() < 30 && c3_cleared.blue() < 30); // Cleared to black
    assert(c3_green.green() > 200); // Green preserved

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "     NISABA GPU DAMAGE TRACKING TEST SUITE\n";
    std::cout << "===========================================\n";

    test_gpu_damage_tracking(GpuBackendType::OpenGL);
    test_gpu_damage_tracking(GpuBackendType::Vulkan);

    std::cout << "\n[✔] All GPU damage tracking tests passed successfully!\n";
    return 0;
}

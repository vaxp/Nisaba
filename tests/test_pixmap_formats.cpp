#include <iostream>
#include <cassert>
#include <vector>
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"

using namespace nisaba;

void test_format_allocation() {
    // 1. RGBA8888 (Default)
    auto pm_rgba = Pixmap::allocate(100, 100, PixelFormat::RGBA8888);
    assert(pm_rgba.has_value());
    assert(pm_rgba->width() == 100);
    assert(pm_rgba->height() == 100);
    assert(pm_rgba->format() == PixelFormat::RGBA8888);
    assert(pm_rgba->bytes_per_pixel() == 4);
    assert(pm_rgba->data_len() == 100 * 100 * 4);

    // 2. BGRA8888 (Native Windows DIB / Wayland)
    auto pm_bgra = Pixmap::allocate(100, 100, PixelFormat::BGRA8888);
    assert(pm_bgra.has_value());
    assert(pm_bgra->format() == PixelFormat::BGRA8888);
    assert(pm_bgra->bytes_per_pixel() == 4);
    assert(pm_bgra->data_len() == 100 * 100 * 4);

    // 3. RGB565 (Embedded MCU / 50% RAM savings)
    auto pm_565 = Pixmap::allocate(100, 100, PixelFormat::RGB565);
    assert(pm_565.has_value());
    assert(pm_565->format() == PixelFormat::RGB565);
    assert(pm_565->bytes_per_pixel() == 2);
    assert(pm_565->data_len() == 100 * 100 * 2); // Half the size of RGBA8888!

    // 4. Alpha8 (Single channel mask / 75% RAM savings)
    auto pm_a8 = Pixmap::allocate(100, 100, PixelFormat::Alpha8);
    assert(pm_a8.has_value());
    assert(pm_a8->format() == PixelFormat::Alpha8);
    assert(pm_a8->bytes_per_pixel() == 1);
    assert(pm_a8->data_len() == 100 * 100 * 1); // 1/4 size of RGBA8888!

    std::cout << "[PASS] PixelFormat allocation and sizes verified\n";
}

void test_solid_fill_and_encoding() {
    // Fill RGBA8888
    auto pm_rgba = Pixmap::allocate(10, 10, PixelFormat::RGBA8888);
    assert(pm_rgba.has_value());
    pm_rgba->fill(Color::from_rgba8(255, 128, 64, 255));
    const auto* raw_rgba = pm_rgba->data();
    assert(raw_rgba[0] == 255);
    assert(raw_rgba[1] == 128);
    assert(raw_rgba[2] == 64);
    assert(raw_rgba[3] == 255);
    assert(pm_rgba->pixel(0, 0)->red() == 255);
    assert(pm_rgba->pixel(0, 0)->green() == 128);
    assert(pm_rgba->pixel(0, 0)->blue() == 64);

    // Fill BGRA8888: Memory must be [B, G, R, A]
    auto pm_bgra = Pixmap::allocate(10, 10, PixelFormat::BGRA8888);
    assert(pm_bgra.has_value());
    pm_bgra->fill(Color::from_rgba8(255, 128, 64, 255));
    const auto* raw_bgra = pm_bgra->data();
    assert(raw_bgra[0] == 64);  // Blue
    assert(raw_bgra[1] == 128); // Green
    assert(raw_bgra[2] == 255); // Red
    assert(raw_bgra[3] == 255); // Alpha
    // Reading pixel through pixel(x, y) returns logical PremultipliedColorU8
    assert(pm_bgra->pixel(0, 0)->red() == 255);
    assert(pm_bgra->pixel(0, 0)->green() == 128);
    assert(pm_bgra->pixel(0, 0)->blue() == 64);

    // Fill RGB565: Pure Red (255, 0, 0) -> 0xF800
    auto pm_565 = Pixmap::allocate(10, 10, PixelFormat::RGB565);
    assert(pm_565.has_value());
    pm_565->fill(Color::from_rgba8(255, 0, 0, 255));
    const auto* raw_565 = reinterpret_cast<const uint16_t*>(pm_565->data());
    assert(raw_565[0] == 0xF800);
    assert(pm_565->pixel(0, 0)->red() == 255);
    assert(pm_565->pixel(0, 0)->green() == 0);
    assert(pm_565->pixel(0, 0)->blue() == 0);

    // Fill Alpha8: Alpha = 180
    auto pm_a8 = Pixmap::allocate(10, 10, PixelFormat::Alpha8);
    assert(pm_a8.has_value());
    pm_a8->fill(Color::from_rgba8(0, 0, 0, 180));
    assert(pm_a8->data()[0] == 180);
    assert(pm_a8->pixel(0, 0)->alpha() == 180);

    std::cout << "[PASS] PixelFormat fills, raw memory layouts, and decoders verified\n";
}

void test_canvas_drawing_rgb565() {
    auto pm = Pixmap::allocate(64, 64, PixelFormat::RGB565);
    assert(pm.has_value());
    Canvas canvas(*pm);

    // Clear with dark gray
    canvas.clear(Color::from_rgba8(30, 30, 30, 255));

    // Draw solid green rectangle
    Paint p_green(Color::from_rgba8(0, 255, 0, 255));
    auto rect_opt = Rect::from_xywh(10.0f, 10.0f, 20.0f, 20.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p_green);

    // Verify inside rectangle is green
    auto px_inside = pm->pixel(15, 15);
    assert(px_inside.has_value());
    assert(px_inside->green() >= 250);
    assert(px_inside->red() == 0);
    assert(px_inside->blue() == 0);

    // Verify outside rectangle is dark gray
    auto px_outside = pm->pixel(5, 5);
    assert(px_outside.has_value());
    assert(px_outside->red() < 40);

    // Draw circle
    Paint p_blue(Color::from_rgba8(0, 0, 255, 255));
    canvas.fill_circle(45.0f, 45.0f, 10.0f, p_blue);

    auto px_circle = pm->pixel(45, 45);
    assert(px_circle.has_value());
    assert(px_circle->blue() >= 250);

    std::cout << "[PASS] Canvas vector rendering on 16-bit RGB565 verified\n";
}

void test_canvas_drawing_bgra8888() {
    auto pm = Pixmap::allocate(40, 40, PixelFormat::BGRA8888);
    assert(pm.has_value());
    Canvas canvas(*pm);

    // Clear to white
    canvas.clear(Color::WHITE);

    // Draw red rect
    Paint p_red(Color::from_rgba8(255, 0, 0, 255));
    auto rect_opt = Rect::from_xywh(5.0f, 5.0f, 10.0f, 10.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p_red);

    // Direct memory check for BGRA layout: byte 0=B(0), byte 1=G(0), byte 2=R(255), byte 3=A(255)
    const uint8_t* p = pm->data() + (8 * 40 + 8) * 4;
    assert(p[0] == 0);   // Blue
    assert(p[1] == 0);   // Green
    assert(p[2] == 255); // Red
    assert(p[3] == 255); // Alpha

    // Decoded pixel check
    auto px = pm->pixel(8, 8);
    assert(px.has_value());
    assert(px->red() == 255);
    assert(px->green() == 0);
    assert(px->blue() == 0);

    std::cout << "[PASS] Canvas vector rendering on native BGRA8888 verified\n";
}

void test_canvas_drawing_alpha8() {
    auto pm = Pixmap::allocate(30, 30, PixelFormat::Alpha8);
    assert(pm.has_value());
    Canvas canvas(*pm);

    canvas.clear(Color::TRANSPARENT);
    assert(pm->data()[0] == 0);

    Paint p(Color::from_rgba8(0, 0, 0, 200));
    auto rect_opt = Rect::from_xywh(5.0f, 5.0f, 15.0f, 15.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p);

    // Pixel inside must have alpha ~ 200
    assert(pm->data()[10 * 30 + 10] >= 195);
    assert(pm->data()[2 * 30 + 2] == 0);

    std::cout << "[PASS] Canvas mask/alpha rendering on 8-bit Alpha8 verified\n";
}

void test_format_conversions() {
    // RGB565 -> RGBA8888
    auto pm_565 = Pixmap::allocate(20, 20, PixelFormat::RGB565);
    assert(pm_565.has_value());
    pm_565->fill(Color::from_rgba8(255, 0, 128, 255));

    Pixmap pm_rgba = pm_565->to_rgba8888();
    assert(pm_rgba.format() == PixelFormat::RGBA8888);
    assert(pm_rgba.bytes_per_pixel() == 4);
    assert(pm_rgba.data_len() == 20 * 20 * 4);
    assert(pm_rgba.pixel(5, 5)->red() == 255);
    assert(pm_rgba.pixel(5, 5)->green() == 0);
    assert(pm_rgba.pixel(5, 5)->alpha() == 255);

    // BGRA8888 -> RGBA8888
    auto pm_bgra = Pixmap::allocate(20, 20, PixelFormat::BGRA8888);
    assert(pm_bgra.has_value());
    pm_bgra->fill(Color::from_rgba8(100, 150, 200, 255));
    Pixmap converted_bgra = pm_bgra->to_rgba8888();
    assert(converted_bgra.format() == PixelFormat::RGBA8888);
    assert(converted_bgra.pixel(0, 0)->red() == 100);
    assert(converted_bgra.pixel(0, 0)->green() == 150);
    assert(converted_bgra.pixel(0, 0)->blue() == 200);

    std::cout << "[PASS] Cross-format conversions to RGBA8888 verified\n";
}

void test_zero_copy_framebuffer() {
    // Simulating external MCU hardware display framebuffer
    constexpr uint32_t W = 48;
    constexpr uint32_t H = 32;
    std::vector<uint16_t> mcu_framebuffer(W * H, 0x0000);

    // Wrap zero-copy with PixmapMut
    auto mut_view = PixmapMut::from_raw_parts(
        reinterpret_cast<uint8_t*>(mcu_framebuffer.data()),
        W, H,
        W * sizeof(uint16_t),
        PixelFormat::RGB565
    );
    assert(mut_view.has_value());
    assert(mut_view->format() == PixelFormat::RGB565);

    // Render directly into external hardware buffer
    Canvas canvas(*mut_view);
    Paint p(Color::from_rgba8(255, 0, 0, 255));
    auto r = Rect::from_xywh(10, 10, 10, 10);
    assert(r.has_value());
    canvas.fill_rect(*r, p);

    // Validate directly in the external mcu_framebuffer vector!
    uint16_t center_pixel = mcu_framebuffer[15 * W + 15];
    assert(center_pixel == 0xF800); // 16-bit 565 pure Red
    assert(mcu_framebuffer[0] == 0x0000); // Corner unmodified

    std::cout << "[PASS] MCU/Hardware zero-copy direct framebuffer rendering verified\n";
}

int main() {
    std::cout << "Running Nisaba Diverse Pixel Formats Test Suite...\n";
    test_format_allocation();
    test_solid_fill_and_encoding();
    test_canvas_drawing_rgb565();
    test_canvas_drawing_bgra8888();
    test_canvas_drawing_alpha8();
    test_format_conversions();
    test_zero_copy_framebuffer();
    std::cout << "All Nisaba Pixel Format Tests Passed Successfully!\n";
    return 0;
}

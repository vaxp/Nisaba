#include <iostream>
#include <vector>
#include <cassert>
#include <cstring>
#include "nisaba/nisaba.hpp"

using namespace nisaba;

void test_zero_copy_stride() {
    std::cout << "Testing Zero-Copy Framebuffer with custom stride (DRM/KMS dumb buffer simulation)...\n";

    // Simulate a 100x50 DRM framebuffer with a 512-byte hardware pitch (stride)
    // 100 pixels * 4 = 400 bytes, but row stride is 512 bytes (112 bytes padding per scanline)
    constexpr uint32_t width = 100;
    constexpr uint32_t height = 50;
    constexpr size_t stride = 512;
    std::vector<uint8_t> drm_buffer(stride * height, 0xEE); // Fill with guard byte 0xEE

    auto pixmap = PixmapMut::from_raw_parts(drm_buffer.data(), width, height, stride);
    assert(pixmap.has_value());
    assert(pixmap->width() == width);
    assert(pixmap->height() == height);
    assert(pixmap->stride_bytes() == stride);

    // Draw on this zero-copy surface
    Canvas canvas(*pixmap);
    Paint paint;
    paint.set_color_rgba8(255, 0, 0, 255); // Opaque Red
    canvas.fill_rect(*Rect::from_xywh(10.0f, 10.0f, 30.0f, 20.0f), paint);

    // Verify inside the drawn rect
    PremultipliedColorU8* row15 = pixmap->row(15);
    assert(row15[15].red() == 255);
    assert(row15[15].green() == 0);
    assert(row15[15].blue() == 0);
    assert(row15[15].alpha() == 255);

    // Verify outside the drawn rect on active line is untouched (still 0xEE)
    assert(row15[5].red() == 0xEE && row15[5].alpha() == 0xEE);

    // Verify hardware padding bytes beyond 400 bytes are UNTOUCHED (still 0xEE)
    uint8_t* raw_row15 = drm_buffer.data() + 15 * stride;
    for (size_t p = 400; p < 512; ++p) {
        assert(raw_row15[p] == 0xEE);
    }

    // Test swap_rb (RGBA <-> BGRA for DRM_FORMAT_XRGB8888)
    pixmap->swap_rb();
    assert(row15[15].blue() == 255);
    assert(row15[15].red() == 0);

    std::cout << "[PASS] Zero-copy framebuffer with custom row stride and format swap passed!\n";
}

void test_simd_accuracy() {
    std::cout << "Testing SIMD vs Scalar bit-exact accuracy...\n";

    constexpr size_t count = 37; // Non-multiple of 4 to test tail handling
    std::vector<PremultipliedColorU8> dst_scalar(count);
    std::vector<PremultipliedColorU8> dst_simd(count);

    for (size_t i = 0; i < count; ++i) {
        auto c = PremultipliedColorU8::from_rgba_unchecked(
            static_cast<uint8_t>(i * 6),
            static_cast<uint8_t>(255 - i * 5),
            static_cast<uint8_t>(i * 3 + 10),
            static_cast<uint8_t>(100 + i * 4)
        );
        dst_scalar[i] = c;
        dst_simd[i] = c;
    }

    auto src_color = Color::from_rgba8(200, 100, 50, 160).premultiply().to_color_u8();

    simd::blend_solid_source_over_scalar(dst_scalar.data(), src_color, count);
    simd::blend_solid_source_over(dst_simd.data(), src_color, count);

    for (size_t i = 0; i < count; ++i) {
        // SSE2 integer fixed point and scalar integer fixed point must match exactly or within 1 ULP
        int diff_r = std::abs(static_cast<int>(dst_scalar[i].red()) - static_cast<int>(dst_simd[i].red()));
        int diff_g = std::abs(static_cast<int>(dst_scalar[i].green()) - static_cast<int>(dst_simd[i].green()));
        int diff_b = std::abs(static_cast<int>(dst_scalar[i].blue()) - static_cast<int>(dst_simd[i].blue()));
        int diff_a = std::abs(static_cast<int>(dst_scalar[i].alpha()) - static_cast<int>(dst_simd[i].alpha()));

        assert(diff_r <= 1);
        assert(diff_g <= 1);
        assert(diff_b <= 1);
        assert(diff_a <= 1);
    }

    std::cout << "[PASS] SIMD kernels match scalar reference accurately!\n";
}

void test_clip_path() {
    std::cout << "Testing Arbitrary Vector Clip Path and State Stack...\n";

    auto pixmap = Pixmap::allocate(100, 100);
    assert(pixmap.has_value());
    Canvas canvas(*pixmap);
    canvas.clear(Color::BLACK);

    // Save initial state
    canvas.save();

    // Clip to a circle centered at (50, 50) with radius 30
    auto circle_path = PathBuilder::from_circle(50.0f, 50.0f, 30.0f);
    assert(circle_path.has_value());
    canvas.clip_path(*circle_path);

    // Draw a huge green rectangle covering entire canvas
    Paint green_paint;
    green_paint.set_color_rgba8(0, 255, 0, 255);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 100.0f, 100.0f), green_paint);

    // Center (50, 50) must be GREEN
    auto center_px = pixmap->as_ref().pixel(50, 50);
    assert(center_px.has_value());
    assert(center_px->green() == 255);
    assert(center_px->red() == 0);

    // Corner (10, 10) is outside the circle and must remain BLACK
    auto corner_px = pixmap->as_ref().pixel(10, 10);
    assert(corner_px.has_value());
    assert(corner_px->green() == 0);
    assert(corner_px->red() == 0);

    // Restore state - clip must be popped
    canvas.restore();

    // Now drawing a blue rectangle at (0, 0, 20, 20) should succeed without clipping
    Paint blue_paint;
    blue_paint.set_color_rgba8(0, 0, 255, 255);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 20.0f, 20.0f), blue_paint);

    auto restored_px = pixmap->as_ref().pixel(10, 10);
    assert(restored_px.has_value());
    assert(restored_px->blue() == 255);

    std::cout << "[PASS] Arbitrary vector clip path and save/restore passed!\n";
}

void test_text_and_glyphs() {
    std::cout << "Testing Text, Glyphs, and Built-in Monospace Font...\n";

    auto pixmap = Pixmap::allocate(200, 60);
    assert(pixmap.has_value());
    Canvas canvas(*pixmap);
    canvas.clear(Color::BLACK);

    Paint text_paint;
    text_paint.set_color_rgba8(255, 255, 255, 255);

    // Draw debug text
    canvas.draw_text_debug("Nisaba 2D", 10.0f, 20.0f, text_paint);

    // Test that pixels were actually rendered in that region
    bool found_white = false;
    for (uint32_t y = 20; y < 40; ++y) {
        for (uint32_t x = 10; x < 100; ++x) {
            auto px = pixmap->as_ref().pixel(x, y);
            if (px && px->red() > 200) {
                found_white = true;
                break;
            }
        }
        if (found_white) break;
    }
    assert(found_white);

    // Test custom glyph run
    std::vector<uint8_t> glyph_buf(10 * 10, 255); // 10x10 solid mask
    auto glyph_mask = SubMaskRef::from_bytes(glyph_buf.data(), glyph_buf.size(), 10, 10);
    assert(glyph_mask.has_value());

    Paint yellow_paint;
    yellow_paint.set_color_rgba8(255, 255, 0, 255);
    canvas.draw_glyph(Point::from_xy(120.0f, 20.0f), *glyph_mask, yellow_paint);

    auto glyph_px = pixmap->as_ref().pixel(125, 25);
    assert(glyph_px.has_value());
    assert(glyph_px->red() == 255);
    assert(glyph_px->green() == 255);
    assert(glyph_px->blue() == 0);

    std::cout << "[PASS] Text and glyph rendering passed!\n";
}

int main() {
    std::cout << "=======================================\n";
    std::cout << " Running Nisaba Advanced Features Tests\n";
    std::cout << "=======================================\n";

    test_zero_copy_stride();
    test_simd_accuracy();
    test_clip_path();
    test_text_and_glyphs();

    std::cout << "\nAll Nisaba advanced features passed 100% successfully!\n";
    return 0;
}

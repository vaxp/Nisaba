#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "nisaba/color/color.hpp"
#include "nisaba/color/color_space_lut.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/pipeline/simd.hpp"

using namespace nisaba;

void test_lut_accuracy() {
    std::cout << "[TEST] ColorSpaceLut roundtrip accuracy and transfer function... ";

    // 1. Boundary values
    assert(ColorSpaceLut::srgb_to_linear_u12[0] == 0);
    assert(ColorSpaceLut::srgb_to_linear_u12[255] == 4095);
    assert(ColorSpaceLut::linear_u12_to_srgb[0] == 0);
    assert(ColorSpaceLut::linear_u12_to_srgb[4095] == 255);

    // 2. Round-trip across all 256 sRGB levels
    for (uint32_t i = 0; i < 256; ++i) {
        uint16_t lin = ColorSpaceLut::to_linear_u12(static_cast<uint8_t>(i));
        uint8_t srgb = ColorSpaceLut::to_srgb(lin);
        assert(srgb == i && "U12 roundtrip must have 0 error for all 256 sRGB levels!");
    }

    // 3. Float transfer functions
    assert(std::abs(ColorSpaceLut::to_linear_f32(0.0f) - 0.0f) < 1e-5f);
    assert(std::abs(ColorSpaceLut::to_linear_f32(1.0f) - 1.0f) < 1e-5f);
    assert(std::abs(ColorSpaceLut::to_srgb_f32(0.0f) - 0.0f) < 1e-5f);
    assert(std::abs(ColorSpaceLut::to_srgb_f32(1.0f) - 1.0f) < 1e-5f);

    // 4. Physical gamma verification:
    // In sRGB, encoded 128 corresponds to only ~21.6% linear light:
    float lin_128 = ColorSpaceLut::to_linear_f32(128.0f / 255.0f);
    assert(lin_128 > 0.20f && lin_128 < 0.23f);

    // In linear space, 50% physical light corresponds to sRGB ~188:
    float srgb_50 = ColorSpaceLut::to_srgb_f32(0.5f) * 255.0f;
    assert(std::round(srgb_50) >= 187.0f && std::round(srgb_50) <= 189.0f);

    // Color struct conversions
    Color red_srgb = Color::from_rgba8(255, 0, 0, 255);
    Color red_lin = ColorSpaceLut::srgb_to_linear(red_srgb);
    assert(std::abs(red_lin.red() - 1.0f) < 1e-4f);
    assert(std::abs(red_lin.green() - 0.0f) < 1e-4f);

    Color back_srgb = ColorSpaceLut::linear_to_srgb(red_lin);
    assert(back_srgb.to_color_u8().red() == 255);

    std::cout << "PASSED\n";
}

void test_solid_source_over_linear_span() {
    std::cout << "[TEST] Fast linear span blending vs legacy sRGB blending... ";

    // Destination: opaque pure green
    PremultipliedColorU8 dst_linear[16];
    PremultipliedColorU8 dst_legacy[16];
    for (int i = 0; i < 16; ++i) {
        dst_linear[i] = PremultipliedColorU8::from_rgba_unchecked(0, 255, 0, 255);
        dst_legacy[i] = PremultipliedColorU8::from_rgba_unchecked(0, 255, 0, 255);
    }

    // Source: pure red with 50% coverage (128 / 255)
    PremultipliedColorU8 src = PremultipliedColorU8::from_rgba_unchecked(255, 0, 0, 255);
    uint8_t coverage = 128;

    // Linear sRGB blend
    simd::blend_solid_source_over_linear_span(dst_linear, src, coverage, 16);

    // Legacy sRGB blend
    simd::blend_solid_source_over_coverage(dst_legacy, src, coverage, 16);

    // In legacy blending: 50% of 255 is ~128. Output is (128, 128, 0, 255) -> dark dirty fringe
    assert(dst_legacy[0].red() <= 130);
    assert(dst_legacy[0].green() <= 130);

    // In linear blending: 50% physical red + 50% physical green -> (188, 188, 0, 255) -> bright clean yellow
    assert(dst_linear[0].red() >= 185 && dst_linear[0].red() <= 190);
    assert(dst_linear[0].green() >= 185 && dst_linear[0].green() <= 190);
    assert(dst_linear[0].blue() == 0);
    assert(dst_linear[0].alpha() == 255);

    std::cout << "PASSED (Linear: " << (int)dst_linear[0].red() << " vs Legacy: " << (int)dst_legacy[0].red() << ")\n";
}

void test_red_green_antialiased_halo_elimination() {
    std::cout << "[TEST] Anti-aliased circle edge dark halo elimination... ";

    // 1. Render in legacy non-linear sRGB mode
    auto pm_legacy = Pixmap::allocate(100, 100);
    assert(pm_legacy.has_value());
    Canvas canvas_legacy(*pm_legacy);
    canvas_legacy.clear(Color::from_rgba8(0, 255, 0, 255)); // pure green background

    Paint paint_legacy(Color::from_rgba8(255, 0, 0, 255)); // pure red foreground
    paint_legacy.anti_alias = true;
    paint_legacy.set_linear_blending(false);
    canvas_legacy.fill_circle(50.0f, 50.0f, 30.0f, paint_legacy);

    // 2. Render in linear sRGB gamma-corrected mode
    auto pm_linear = Pixmap::allocate(100, 100);
    assert(pm_linear.has_value());
    Canvas canvas_linear(*pm_linear);
    canvas_linear.clear(Color::from_rgba8(0, 255, 0, 255)); // pure green background

    Paint paint_linear(Color::from_rgba8(255, 0, 0, 255)); // pure red foreground
    paint_linear.anti_alias = true;
    paint_linear.set_linear_blending(true);
    canvas_linear.fill_circle(50.0f, 50.0f, 30.0f, paint_linear);

    // 3. Inspect the anti-aliased transition pixels across the edge
    // Center is solid red in both
    auto center_leg = pm_legacy->pixel(50, 50);
    auto center_lin = pm_linear->pixel(50, 50);
    assert(center_leg && center_leg->red() == 255 && center_leg->green() == 0);
    assert(center_lin && center_lin->red() == 255 && center_lin->green() == 0);

    // Outside is solid green in both
    auto out_leg = pm_legacy->pixel(5, 5);
    auto out_lin = pm_linear->pixel(5, 5);
    assert(out_leg && out_leg->red() == 0 && out_leg->green() == 255);
    assert(out_lin && out_lin->red() == 0 && out_lin->green() == 255);

    // Find an anti-aliased edge pixel where both red and green are blended (transition zone)
    bool found_edge = false;
    for (uint32_t y = 0; y < 100 && !found_edge; ++y) {
        for (uint32_t x = 0; x < 100; ++x) {
            auto p_leg = pm_legacy->pixel(x, y);
            auto p_lin = pm_linear->pixel(x, y);
            if (p_leg && p_lin && p_leg->red() >= 20 && p_leg->red() <= 235 && p_leg->green() >= 20) {
                found_edge = true;
                float lum_lin = ColorSpaceLut::to_linear_f32(p_lin->red() / 255.0f) +
                                ColorSpaceLut::to_linear_f32(p_lin->green() / 255.0f);
                float lum_leg = ColorSpaceLut::to_linear_f32(p_leg->red() / 255.0f) +
                                ColorSpaceLut::to_linear_f32(p_leg->green() / 255.0f);
                assert(lum_lin > lum_leg && "Linear sRGB blend must have higher physical luminance at the edge!");
                break;
            }
        }
    }
    assert(found_edge && "Must have found an anti-aliased edge pixel on the circle perimeter!");

    std::cout << "PASSED\n";
}

void test_semi_transparent_rect_linear() {
    std::cout << "[TEST] Semi-transparent black over white in linear space... ";

    auto pm = Pixmap::allocate(40, 40);
    assert(pm.has_value());
    Canvas canvas(*pm);
    canvas.clear(Color::WHITE);

    // Draw semi-transparent black (alpha = 128 / 255 ~ 50% opacity)
    Paint p(Color::from_rgba8(0, 0, 0, 128));
    p.set_linear_blending(true);
    auto rect_opt = Rect::from_xywh(5.0f, 5.0f, 30.0f, 30.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p);

    // In linear blending, 50% light absorption of white (1.0) gives 0.5 linear intensity.
    // 0.5 linear intensity encoded to sRGB is ~188.
    auto px = pm->pixel(20, 20);
    assert(px.has_value());
    assert(px->red() >= 185 && px->red() <= 190);
    assert(px->green() >= 185 && px->green() <= 190);
    assert(px->blue() >= 185 && px->blue() <= 190);
    assert(px->alpha() == 255);

    std::cout << "PASSED (sRGB byte: " << (int)px->red() << " corresponds to 50% physical light)\n";
}

void test_canvas_global_linear_blending() {
    std::cout << "[TEST] Canvas::set_linear_blending global control... ";

    auto pm = Pixmap::allocate(40, 40);
    assert(pm.has_value());
    Canvas canvas(*pm);
    canvas.set_linear_blending(true);
    assert(canvas.is_linear_blending());
    canvas.clear(Color::WHITE);

    // Default paint (colorspace not explicitly set to linear)
    Paint p(Color::from_rgba8(0, 0, 0, 128));
    assert(!p.is_linear_blending());

    auto rect_opt = Rect::from_xywh(5.0f, 5.0f, 30.0f, 30.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p);

    // Because canvas has linear_blending enabled, pixel must be ~188
    auto px = pm->pixel(20, 20);
    assert(px.has_value());
    assert(px->red() >= 185 && px->red() <= 190);

    std::cout << "PASSED\n";
}

void test_rgb565_linear_blending() {
    std::cout << "[TEST] Linear sRGB blending on RGB565 surface... ";

    auto pm = Pixmap::allocate(40, 40, PixelFormat::RGB565);
    assert(pm.has_value());
    Canvas canvas(*pm);
    canvas.clear(Color::WHITE);

    Paint p(Color::from_rgba8(0, 0, 0, 128));
    p.set_linear_blending(true);
    auto rect_opt = Rect::from_xywh(5.0f, 5.0f, 30.0f, 30.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p);

    // Check pixel value in RGB565 (188 in 8-bit maps to ~23 in 5-bit, 47 in 6-bit)
    auto px = pm->pixel(20, 20);
    assert(px.has_value());
    assert(px->red() >= 180 && px->red() <= 195);
    assert(px->green() >= 180 && px->green() <= 195);
    assert(px->blue() >= 180 && px->blue() <= 195);

    std::cout << "PASSED\n";
}

void test_bgra8888_linear_blending() {
    std::cout << "[TEST] Linear sRGB blending on BGRA8888 surface... ";

    auto pm = Pixmap::allocate(40, 40, PixelFormat::BGRA8888);
    assert(pm.has_value());
    Canvas canvas(*pm);
    canvas.clear(Color::WHITE);

    Paint p(Color::from_rgba8(0, 0, 0, 128));
    p.set_linear_blending(true);
    auto rect_opt = Rect::from_xywh(5.0f, 5.0f, 30.0f, 30.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p);

    auto px = pm->pixel(20, 20);
    assert(px.has_value());
    assert(px->red() >= 185 && px->red() <= 190);
    assert(px->green() >= 185 && px->green() <= 190);
    assert(px->blue() >= 185 && px->blue() <= 190);

    // Verify raw memory is in B, G, R, A layout
    const uint8_t* raw = pm->data() + (20 * 40 + 20) * 4;
    assert(raw[0] >= 185 && raw[0] <= 190); // B
    assert(raw[1] >= 185 && raw[1] <= 190); // G
    assert(raw[2] >= 185 && raw[2] <= 190); // R
    assert(raw[3] == 255);                  // A

    std::cout << "PASSED\n";
}

void test_linear_blend_modes() {
    std::cout << "[TEST] Complex blend modes in linear light space... ";

    auto pm = Pixmap::allocate(20, 20);
    assert(pm.has_value());
    Canvas canvas(*pm);
    canvas.clear(Color::from_rgba8(128, 64, 32, 255));

    Paint p(Color::from_rgba8(200, 100, 50, 180));
    p.set_linear_blending(true);
    p.blend_mode = BlendMode::Screen;

    auto rect_opt = Rect::from_xywh(2.0f, 2.0f, 16.0f, 16.0f);
    assert(rect_opt.has_value());
    canvas.fill_rect(*rect_opt, p);

    auto px = pm->pixel(10, 10);
    assert(px.has_value());
    assert(px->red() > 128); // Screen produces brighter result
    assert(px->alpha() == 255);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Nisaba Linear sRGB Blending Test Suite " << std::endl;
    std::cout << "========================================" << std::endl;

    test_lut_accuracy();
    test_solid_source_over_linear_span();
    test_red_green_antialiased_halo_elimination();
    test_semi_transparent_rect_linear();
    test_canvas_global_linear_blending();
    test_rgb565_linear_blending();
    test_bgra8888_linear_blending();
    test_linear_blend_modes();

    std::cout << "========================================" << std::endl;
    std::cout << " All Linear sRGB Blending Tests PASSED! " << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}

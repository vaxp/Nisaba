#include <iostream>
#include <cassert>
#include <cmath>
#include "nisaba/color/color.hpp"
#include "nisaba/color/blend_mode.hpp"
#include "nisaba/raster/fixed_point.hpp"
#include "nisaba/math/screen_int_rect.hpp"

using namespace nisaba;

void test_color_premultiply_demultiply() {
    // Exact tests from nisaba
    auto c1 = ColorU8::from_rgba(10, 20, 30, 40).premultiply();
    assert(c1 == PremultipliedColorU8::from_rgba_unchecked(2, 3, 5, 40));

    auto c2 = ColorU8::from_rgba(10, 20, 30, 255).premultiply();
    assert(c2 == PremultipliedColorU8::from_rgba_unchecked(10, 20, 30, 255));

    auto d1 = PremultipliedColorU8::from_rgba_unchecked(2, 3, 5, 40).demultiply();
    assert(d1 == ColorU8::from_rgba(13, 19, 32, 40));

    auto d2 = PremultipliedColorU8::from_rgba_unchecked(10, 20, 30, 255).demultiply();
    assert(d2 == ColorU8::from_rgba(10, 20, 30, 255));

    auto d3 = PremultipliedColorU8::from_rgba_unchecked(153, 99, 54, 180).demultiply();
    assert(d3 == ColorU8::from_rgba(217, 140, 77, 180));

    // Float Color
    Color fc = Color::from_rgba8(255, 128, 0, 128);
    auto pfc = fc.premultiply();
    assert(std::abs(pfc.red() - (1.0f * (128.0f / 255.0f))) < 0.01f);

    auto dfc = pfc.demultiply();
    assert(std::abs(dfc.red() - 1.0f) < 0.01f);
    assert(std::abs(dfc.alpha() - (128.0f / 255.0f)) < 0.01f);

    std::cout << "[PASS] Color premultiply and demultiply tests passed\n";
}

void test_colorspace() {
    auto n05 = *NormalizedF32::create(0.5f);
    auto expanded_g2 = ColorSpaceHelper::expand_channel(ColorSpace::Gamma2, n05);
    assert(std::abs(expanded_g2.get() - 0.25f) < 0.001f);

    auto compressed_g2 = ColorSpaceHelper::compress_channel(ColorSpace::Gamma2, expanded_g2);
    assert(std::abs(compressed_g2.get() - 0.5f) < 0.001f);

    std::cout << "[PASS] ColorSpace conversion tests passed\n";
}

void test_blend_mode() {
    assert(should_pre_scale_coverage(BlendMode::SourceOver) == true);
    assert(should_pre_scale_coverage(BlendMode::Plus) == true);
    assert(should_pre_scale_coverage(BlendMode::SourceIn) == false);
    assert(should_pre_scale_coverage(BlendMode::Multiply) == false);

    std::cout << "[PASS] BlendMode tests passed\n";
}

void test_fixed_point() {
    // fdot6
    assert(fdot6::from_i32(1) == 64);
    assert(fdot6::from_f32(1.5f) == 96);
    assert(fdot6::floor(96) == 1);
    assert(fdot6::ceil(96) == 2);
    assert(fdot6::round(96) == 2);
    assert(fdot6::to_fdot16(64) == 65536);
    assert(fdot6::small_scale(100, 32) == 50);

    // fdot8
    assert(fdot8::from_fdot16(65536) == 256);

    // fdot16
    assert(fdot16::from_f32(1.0f) == 65536);
    assert(fdot16::floor_to_i32(65536) == 1);
    assert(fdot16::round_to_i32(65536 + 32768) == 2);
    assert(fdot16::mul(65536, 65536 * 2) == 65536 * 2);
    assert(fdot16::div(64, 32) == 65536 * 2);

    std::cout << "[PASS] Fixed-point tests passed\n";
}

void test_screen_int_rect() {
    assert(!ScreenIntRect::from_xywh(0, 0, 0, 0).has_value());
    assert(!ScreenIntRect::from_xywh(0, 0, 1, 0).has_value());
    assert(!ScreenIntRect::from_xywh(0, 0, 0, 1).has_value());

    auto r = ScreenIntRect::from_xywh(1, 2, 3, 4);
    assert(r.has_value());
    assert(r->x() == 1);
    assert(r->y() == 2);
    assert(r->width() == 3);
    assert(r->height() == 4);
    assert(r->right() == 4);
    assert(r->bottom() == 6);

    auto r_sub = ScreenIntRect::from_xywh(2, 3, 1, 1);
    assert(r->contains(*r_sub));

    std::cout << "[PASS] ScreenIntRect tests passed\n";
}

int main() {
    std::cout << "Running Nisaba Color, Fixed-Point & ScreenIntRect tests...\n";
    test_color_premultiply_demultiply();
    test_colorspace();
    test_blend_mode();
    test_fixed_point();
    test_screen_int_rect();
    std::cout << "All Color & Fixed-Point tests passed successfully!\n";
    return 0;
}

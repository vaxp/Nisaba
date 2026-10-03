#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include "nisaba/shaders/shader.hpp"
#include "nisaba/shaders/conical_gradient.hpp"
#include "nisaba/shaders/compose_shader.hpp"
#include "nisaba/effects/color_matrix.hpp"
#include "nisaba/effects/color_filter.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/paint.hpp"

using namespace nisaba;
using namespace nisaba::effects;

void test_two_point_conical_gradient() {
    std::cout << "[TEST] TwoPointConicalGradient analytical geometry... ";

    std::vector<GradientStop> stops = {
        GradientStop::create(0.0f, Color::RED),
        GradientStop::create(1.0f, Color::BLUE)
    };

    // 1. Concentric circles: C0 = (50, 50), r0 = 10; C1 = (50, 50), r1 = 50
    auto concentric = TwoPointConicalGradient::create(
        Point::from_xy(50.0f, 50.0f), 10.0f,
        Point::from_xy(50.0f, 50.0f), 50.0f,
        stops
    );
    assert(concentric.has_value());

    // Point on start circle (dist = 10) -> t = 0 -> Red
    Color c_c0 = concentric->sample(50.0f, 60.0f);
    assert(c_c0.red() > 0.95f && c_c0.blue() < 0.05f);

    // Point on end circle (dist = 50) -> t = 1 -> Blue
    Color c_c1 = concentric->sample(50.0f, 100.0f);
    assert(c_c1.blue() > 0.95f && c_c1.red() < 0.05f);

    // Point at midpoint (dist = 30) -> t = 0.5 -> ~50% Red, ~50% Blue
    Color c_mid = concentric->sample(50.0f, 80.0f);
    assert(std::abs(c_mid.red() - 0.5f) < 0.1f);
    assert(std::abs(c_mid.blue() - 0.5f) < 0.1f);

    // 2. Eccentric circles: C0 = (20, 50), r0 = 10; C1 = (80, 50), r1 = 30
    auto eccentric = TwoPointConicalGradient::create(
        Point::from_xy(20.0f, 50.0f), 10.0f,
        Point::from_xy(80.0f, 50.0f), 30.0f,
        stops
    );
    assert(eccentric.has_value());

    // Point on start circle at (20, 40) -> t = 0 -> Red
    Color c_ecc_start = eccentric->sample(20.0f, 40.0f);
    assert(c_ecc_start.red() > 0.9f && c_ecc_start.blue() < 0.1f);

    // Point on end circle at (80, 20) -> t = 1 -> Blue
    Color c_ecc_end = eccentric->sample(80.0f, 20.0f);
    assert(c_ecc_end.blue() > 0.9f && c_ecc_end.red() < 0.1f);

    // 3. Shader wrapping & span shading
    Shader sh(*eccentric);
    assert(sh.type() == Shader::Type::TwoPointConicalGradient);

    std::vector<PremultipliedColorU8> span(10);
    sh.shade_span(20.0f, 40.0f, 10, span.data());
    assert(span[0].red() > 200);

    std::cout << "PASSED\n";
}

void test_compose_shader() {
    std::cout << "[TEST] ComposeShader (BlendShader)... ";

    // Shader A: Solid Red (opaque)
    Shader sh_a = Shader::from_color(Color::RED);

    // Shader B: Solid Blue with 50% opacity
    Shader sh_b = Shader::from_color(Color::from_rgba_unchecked(0.0f, 0.0f, 1.0f, 0.5f));

    // Compose A and B with SourceOver (B over A)
    // Blue (0.5) over Red (1.0):
    // out_a = 0.5 + 1.0 * 0.5 = 1.0
    // out_r = 1.0 * (1 - 0.5) = 0.5
    // out_b = 0.5
    Shader comp = Shader::create_compose(sh_a, sh_b, BlendMode::SourceOver);
    assert(comp.type() == Shader::Type::ComposeShader);

    PremultipliedColor pm = comp.sample_premul(10.0f, 10.0f);
    assert(std::abs(pm.alpha() - 1.0f) < 0.02f);
    assert(std::abs(pm.red() - 0.5f) < 0.05f);
    assert(std::abs(pm.blue() - 0.5f) < 0.05f);

    // Test with Multiply mode
    // Red (1, 0, 0) multiply Green (0, 1, 0) = Black (0, 0, 0)
    Shader sh_green = Shader::from_color(Color::GREEN);
    Shader comp_mult = Shader::create_compose(sh_a, sh_green, BlendMode::Multiply);
    Color mult_col = comp_mult.sample(0.0f, 0.0f);
    assert(mult_col.red() < 0.05f);
    assert(mult_col.green() < 0.05f);
    assert(mult_col.blue() < 0.05f);

    // Span shading test
    std::vector<PremultipliedColorU8> span(16);
    comp.shade_span(0.0f, 0.0f, 16, span.data());
    for (int i = 0; i < 16; ++i) {
        assert(span[i].alpha() == 255);
        assert(std::abs(static_cast<int>(span[i].red()) - 128) <= 3);
        assert(std::abs(static_cast<int>(span[i].blue()) - 128) <= 3);
    }

    std::cout << "PASSED\n";
}

void test_color_matrix_operations() {
    std::cout << "[TEST] 4x5 ColorMatrix operations... ";

    // 1. Identity
    ColorMatrix id = ColorMatrix::identity();
    Color orig = Color::from_rgba_unchecked(0.2f, 0.5f, 0.8f, 0.9f);
    Color id_res = id.transform(orig);
    assert(std::abs(id_res.red() - orig.red()) < 1e-4f);
    assert(std::abs(id_res.green() - orig.green()) < 1e-4f);
    assert(std::abs(id_res.blue() - orig.blue()) < 1e-4f);
    assert(std::abs(id_res.alpha() - orig.alpha()) < 1e-4f);

    // 2. Inversion
    ColorMatrix inv = ColorMatrix::invert();
    Color inv_res = inv.transform(orig);
    assert(std::abs(inv_res.red() - (1.0f - orig.red())) < 1e-4f);
    assert(std::abs(inv_res.green() - (1.0f - orig.green())) < 1e-4f);
    assert(std::abs(inv_res.blue() - (1.0f - orig.blue())) < 1e-4f);
    assert(std::abs(inv_res.alpha() - orig.alpha()) < 1e-4f);

    // 3. Saturation: s = 0 (Grayscale BT.709)
    ColorMatrix gray_mat = ColorMatrix::saturation(0.0f);
    Color pure_red = Color::RED;
    Color gray_res = gray_mat.transform(pure_red);
    // BT.709 luminance of pure red is ~0.2126
    assert(std::abs(gray_res.red() - 0.2126f) < 0.01f);
    assert(std::abs(gray_res.green() - 0.2126f) < 0.01f);
    assert(std::abs(gray_res.blue() - 0.2126f) < 0.01f);

    // 4. Hue rotation: 0 deg and 360 deg = identity
    ColorMatrix hue0 = ColorMatrix::hue_rotate(0.0f);
    ColorMatrix hue360 = ColorMatrix::hue_rotate(360.0f);
    Color h0_res = hue0.transform(orig);
    Color h360_res = hue360.transform(orig);
    assert(std::abs(h0_res.red() - orig.red()) < 1e-3f);
    assert(std::abs(h360_res.red() - orig.red()) < 1e-3f);

    // 5. Matrix multiplication
    ColorMatrix scale_half = ColorMatrix::scale(0.5f, 0.5f, 0.5f, 1.0f);
    ColorMatrix mult_mat = scale_half * scale_half; // scale by 0.25
    Color scaled_c = mult_mat.transform(Color::WHITE);
    assert(std::abs(scaled_c.red() - 0.25f) < 1e-4f);

    // 6. Pixmap in-place application
    auto pm = Pixmap::create(10, 10);
    assert(pm.has_value());
    pm->fill(Color::RED);

    inv.apply(*pm);
    auto p0 = pm->pixel(0, 0).value();
    // Inverted red is cyan (0, 255, 255)
    assert(p0.red() == 0);
    assert(p0.green() == 255);
    assert(p0.blue() == 255);
    assert(p0.alpha() == 255);

    std::cout << "PASSED\n";
}

void test_paint_color_filter_integration() {
    std::cout << "[TEST] Paint with ColorFilter rendering integration... ";

    auto pm = Pixmap::create(40, 40);
    assert(pm.has_value());
    pm->fill(Color::WHITE);

    Canvas canvas(*pm);

    // Draw a rect with pure Red, but with an Invert color filter -> should render Cyan!
    Paint paint(Color::RED);
    paint.set_color_filter(ColorMatrix::invert());
    assert(paint.has_color_filter());

    auto r = Rect::from_xywh(5.0f, 5.0f, 30.0f, 30.0f);
    assert(r.has_value());
    canvas.fill_rect(*r, paint);

    auto center_px = pm->pixel(20, 20).value();
    assert(center_px.red() == 0);
    assert(center_px.green() == 255);
    assert(center_px.blue() == 255);
    assert(center_px.alpha() == 255);

    // Draw another rect with TwoPointConicalGradient and Grayscale color filter
    std::vector<GradientStop> stops = {
        GradientStop::create(0.0f, Color::RED),
        GradientStop::create(1.0f, Color::BLUE)
    };
    Shader conical = Shader::create_two_point_conical(
        Point::from_xy(20.0f, 20.0f), 0.0f,
        Point::from_xy(20.0f, 20.0f), 20.0f,
        stops
    );
    Paint grad_paint(conical);
    grad_paint.set_color_filter(ColorMatrix::saturation(0.0f)); // Grayscale filter!

    auto r2 = Rect::from_xywh(10.0f, 10.0f, 20.0f, 20.0f);
    canvas.fill_rect(*r2, grad_paint);

    auto grad_px = pm->pixel(20, 20).value();
    // Since grayscale filter was active, R, G, B should all be equal!
    assert(std::abs(static_cast<int>(grad_px.red()) - static_cast<int>(grad_px.green())) <= 2);
    assert(std::abs(static_cast<int>(grad_px.green()) - static_cast<int>(grad_px.blue())) <= 2);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << " Nisaba Advanced Shaders & Filters Tests\n";
    std::cout << "========================================\n";

    test_two_point_conical_gradient();
    test_compose_shader();
    test_color_matrix_operations();
    test_paint_color_filter_integration();

    std::cout << "========================================\n";
    std::cout << " All Advanced Shader & Filter Tests PASSED!\n";
    std::cout << "========================================\n";
    return 0;
}

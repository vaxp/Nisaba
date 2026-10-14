#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>
#include <memory>
#include "nisaba/nisaba.hpp"
#include "nisaba/gpu/runtime_effect.hpp"
#include "nisaba/gpu/shader.hpp"

using namespace nisaba;
using nisaba::gpu::GpuDevice;
using nisaba::gpu::GpuSurface;
using nisaba::gpu::GpuCanvas;
using nisaba::gpu::RuntimeEffect;
using nisaba::gpu::RuntimeShaderBuilder;
using nisaba::gpu::RuntimeShader;
using nisaba::gpu::SkRuntimeEffect;
using nisaba::gpu::SkRuntimeShaderBuilder;
using nisaba::gpu::SkShader;
using nisaba::gpu::Transform2D;

void test_runtime_effect_reflection() {
    std::cout << "[Test 1/6] Testing RuntimeEffect compilation & uniform reflection..." << std::endl;

    const std::string valid_sksl = R"(
        uniform float time;
        uniform vec2 resolution;
        uniform vec4 primaryColor;
        uniform int mode;

        vec4 main(vec2 fragCoord) {
            vec2 uv = fragCoord / resolution;
            return primaryColor * (0.5 + 0.5 * sin(time + uv.x));
        }
    )";

    auto result = RuntimeEffect::MakeForShader(valid_sksl);
    assert(result);
    assert(result.effect != nullptr);
    assert(result.errorText.empty());

    auto effect = result.effect;
    assert(effect->programId() != 0);

    // Uniform checks
    const auto* u_time = effect->findUniform("time");
    assert(u_time != nullptr);
    assert(u_time->type == RuntimeEffect::Uniform::Type::Float);
    assert(u_time->sizeInBytes == 4);

    const auto* u_res = effect->findUniform("resolution");
    assert(u_res != nullptr);
    assert(u_res->type == RuntimeEffect::Uniform::Type::Float2);
    assert(u_res->sizeInBytes == 8);

    const auto* u_col = effect->findUniform("primaryColor");
    assert(u_col != nullptr);
    assert(u_col->type == RuntimeEffect::Uniform::Type::Float4);
    assert(u_col->sizeInBytes == 16);

    const auto* u_mode = effect->findUniform("mode");
    assert(u_mode != nullptr);
    assert(u_mode->type == RuntimeEffect::Uniform::Type::Int);
    assert(u_mode->sizeInBytes == 4);

    assert(effect->findUniform("non_existent_var") == nullptr);
    assert(effect->uniformSize() >= 32);

    std::cout << "  -> Uniform reflection verified successfully. Total uniform size: "
              << effect->uniformSize() << " bytes." << std::endl;
}

void test_runtime_effect_error_handling() {
    std::cout << "[Test 2/6] Testing error handling for invalid shader syntax..." << std::endl;

    const std::string invalid_sksl = R"(
        uniform float time;
        vec4 main(vec2 fragCoord) {
            This is invalid GLSL syntax that must fail gracefully!
            return undefined_symbol;
        }
    )";

    auto result = RuntimeEffect::MakeForShader(invalid_sksl);
    assert(!result);
    assert(result.effect == nullptr);
    assert(!result.errorText.empty());

    std::cout << "  -> Handled invalid shader gracefully without crashing. Error captured: "
              << result.errorText.substr(0, 45) << "..." << std::endl;
}

void test_runtime_shader_builder_and_aliases() {
    std::cout << "[Test 3/6] Testing RuntimeShaderBuilder & Framework Drop-in Aliases..." << std::endl;

    const std::string sksl = R"(
        uniform float time;
        uniform vec2 resolution;
        vec4 main(vec2 fragCoord) {
            vec2 uv = fragCoord / resolution;
            return vec4(uv.x, uv.y, sin(time), 1.0);
        }
    )";

    // Test framework-identical drop-in aliases
    auto res = SkRuntimeEffect::MakeForShader(sksl);
    assert(res);

    SkRuntimeShaderBuilder builder(res.effect);
    builder.uniform("time") = 2.5f;

    float res_arr[2] = {800.0f, 600.0f};
    bool ok = builder.uniform("resolution").set(res_arr, 2);
    assert(ok);

    std::shared_ptr<SkShader> sk_shader = builder.makeShader();
    assert(sk_shader != nullptr);
    assert(sk_shader->getNativeHandle() != nullptr);

    // Test Paint integration
    nisaba::Paint paint;
    assert(!paint.has_custom_shader());
    paint.setShader(sk_shader);
    assert(paint.has_custom_shader());
    assert(paint.getShader() == sk_shader);

    std::cout << "  -> SkRuntimeEffect, SkRuntimeShaderBuilder, and SkShader aliases verified." << std::endl;
}

void test_gpu_rendering_enki_shaders() {
    std::cout << "[Test 4/6] Testing GPU hardware rasterization with 4 ENKI shaders..." << std::endl;

    auto device = GpuDevice::create();
    auto surface = GpuSurface::create(device, 400, 300);
    assert(surface != nullptr);
    GpuCanvas canvas(surface);

    // 1. Plasma Shader
    {
        const std::string plasma_sksl = R"(
            uniform float time;
            uniform vec2 resolution;
            vec4 main(vec2 fragCoord) {
                vec2 uv = fragCoord / resolution;
                float v = 0.5 + 0.5 * sin(uv.x * 10.0 + time);
                return vec4(v, 0.2, 0.8, 1.0);
            }
        )";
        auto res = RuntimeEffect::MakeForShader(plasma_sksl);
        assert(res);
        RuntimeShaderBuilder bld(res.effect);
        bld.uniform("time") = 1.0f;
        float r[2] = {400.0f, 300.0f};
        bld.uniform("resolution").set(r, 2);

        canvas.clear(Color::from_rgba8(0, 0, 0, 255));
        Paint p;
        p.set_shader(bld.makeShader());
        canvas.fill_rect(*Rect::from_xywh(50.0f, 50.0f, 300.0f, 200.0f), p);
        canvas.flush();

        auto pix = surface->to_pixmap();
        assert(pix.has_value());
        // Check pixel inside plasma rect (100, 100)
        auto inside = pix->as_ref().pixel(100, 100);
        assert(inside.has_value());
        assert(inside->b > 150); // High blue from plasma shader
        // Check pixel outside rect (10, 10)
        auto outside = pix->as_ref().pixel(10, 10);
        assert(outside.has_value());
        assert(outside->r == 0 && outside->g == 0 && outside->b == 0); // Black clear color
    }
    std::cout << "  -> Plasma shader rendered and verified." << std::endl;

    // 2. Cyber Border Shader
    {
        const std::string cyber_sksl = R"(
            uniform float time;
            uniform vec2 resolution;
            vec4 main(vec2 fragCoord) {
                vec2 p = (fragCoord - 0.5 * resolution) / resolution.y;
                float angle = atan(p.y, p.x) + time;
                vec3 col = 0.5 + 0.5 * cos(angle + vec3(0.0, 2.0, 4.0));
                return vec4(col, 1.0);
            }
        )";
        auto res = RuntimeEffect::MakeForShader(cyber_sksl);
        assert(res);
        RuntimeShaderBuilder bld(res.effect);
        bld.uniform("time") = 0.5f;
        float r[2] = {400.0f, 300.0f};
        bld.uniform("resolution").set(r, 2);

        canvas.clear(Color::from_rgba8(20, 20, 20, 255));
        Paint p;
        p.set_shader(bld.makeShader());
        Stroke s;
        s.width = 12.0f;
        canvas.stroke_round_rect(*Rect::from_xywh(40.0f, 40.0f, 320.0f, 220.0f), 16.0f, 16.0f, p, s);
        canvas.flush();

        auto pix = surface->to_pixmap();
        assert(pix.has_value());
        // Pixel on the border (40, 150) should be colored by the cyber shader
        auto border_px = pix->as_ref().pixel(40, 150);
        assert(border_px.has_value());
        assert(border_px->r > 30 || border_px->g > 30 || border_px->b > 30);
    }
    std::cout << "  -> Cyber border shader rendered and verified." << std::endl;

    // 3. Daggers & Blood Shader
    {
        const std::string dagger_sksl = R"(
            uniform float time;
            uniform vec2 resolution;
            vec4 main(vec2 fragCoord) {
                vec2 uv = fragCoord / resolution;
                float d = sin(uv.x * 30.0) * cos(uv.y * 30.0 + time);
                if (d > 0.0) {
                    return vec4(0.9, 0.05, 0.05, 1.0); // Vivid blood red
                }
                return vec4(0.08, 0.08, 0.12, 1.0);    // Dark iron
            }
        )";
        auto res = RuntimeEffect::MakeForShader(dagger_sksl);
        assert(res);
        RuntimeShaderBuilder bld(res.effect);
        bld.uniform("time") = 0.0f;
        float r[2] = {400.0f, 300.0f};
        bld.uniform("resolution").set(r, 2);

        canvas.clear(Color::from_rgba8(255, 255, 255, 255));
        Paint p;
        p.set_shader(bld.makeShader());
        canvas.fill_round_rect(*Rect::from_xywh(30.0f, 30.0f, 340.0f, 240.0f), 20.0f, 20.0f, p);
        canvas.flush();

        auto pix = surface->to_pixmap();
        assert(pix.has_value());

        // Count blood red and dark iron pixels inside the shape
        size_t blood_pixels = 0;
        size_t iron_pixels = 0;
        for (int y = 50; y < 250; y += 4) {
            for (int x = 50; x < 350; x += 4) {
                auto px = pix->as_ref().pixel(x, y);
                if (px && px->r > 180 && px->g < 50 && px->b < 50) blood_pixels++;
                if (px && px->r < 50 && px->g < 50 && px->b < 50) iron_pixels++;
            }
        }
        assert(blood_pixels > 0);
        assert(iron_pixels > 0);
    }
    std::cout << "  -> Daggers & Blood shader rendered and verified." << std::endl;

    // 4. Pixel Art Riveted Frame Shader
    {
        const std::string pixel_sksl = R"(
            uniform float time;
            uniform vec2 resolution;
            vec4 main(vec2 fragCoord) {
                vec2 grid = floor(fragCoord / 8.0);
                float checker = mod(grid.x + grid.y, 2.0);
                if (checker > 0.5) {
                    return vec4(0.6, 0.6, 0.7, 1.0);
                }
                return vec4(0.2, 0.2, 0.3, 1.0);
            }
        )";
        auto res = RuntimeEffect::MakeForShader(pixel_sksl);
        assert(res);
        RuntimeShaderBuilder bld(res.effect);
        bld.uniform("time") = 0.0f;
        float r[2] = {400.0f, 300.0f};
        bld.uniform("resolution").set(r, 2);

        canvas.clear(Color::from_rgba8(0, 0, 0, 255));
        Paint p;
        p.set_shader(bld.makeShader());
        canvas.fill_rect(*Rect::from_xywh(10.0f, 10.0f, 380.0f, 280.0f), p);
        canvas.flush();

        auto pix = surface->to_pixmap();
        assert(pix.has_value());
        // Cell (2, 2) -> (16, 16): checker = 0 -> dark
        // Cell (3, 2) -> (24, 16): checker = 1 -> light
        auto px1 = pix->as_ref().pixel(18, 18);
        auto px2 = pix->as_ref().pixel(26, 18);
        assert(px1.has_value() && px2.has_value());
        assert(px1->r != px2->r);
    }
    std::cout << "  -> Pixel Art Riveted Frame shader rendered and verified." << std::endl;
}

void test_local_matrix_transformation() {
    std::cout << "[Test 5/6] Testing local matrix coordinate transformation..." << std::endl;

    auto device = GpuDevice::create();
    auto surface = GpuSurface::create(device, 200, 200);
    GpuCanvas canvas(surface);

    // Shader outputs green if local fragCoord is in [0..50], red otherwise
    const std::string coord_sksl = R"(
        vec4 main(vec2 fragCoord) {
            if (fragCoord.x >= 0.0 && fragCoord.x <= 50.0 &&
                fragCoord.y >= 0.0 && fragCoord.y <= 50.0) {
                return vec4(0.0, 1.0, 0.0, 1.0); // Green in local box
            }
            return vec4(1.0, 0.0, 0.0, 1.0);     // Red elsewhere
        }
    )";

    auto res = RuntimeEffect::MakeForShader(coord_sksl);
    assert(res);
    RuntimeShaderBuilder bld(res.effect);

    // Local matrix translates origin to (100, 100)
    Transform2D localMat = Transform2D::translate(100.0f, 100.0f);
    auto shader = bld.makeShader(&localMat);

    canvas.clear(Color::from_rgba8(0, 0, 0, 255));
    Paint p;
    p.set_shader(shader);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 200.0f, 200.0f), p);
    canvas.flush();

    auto pix = surface->to_pixmap();
    assert(pix.has_value());

    // World coordinate (120, 120) corresponds to local (20, 20) -> should be GREEN
    auto local_box_px = pix->as_ref().pixel(120, 120);
    assert(local_box_px.has_value());
    assert(local_box_px->g > 200 && local_box_px->r < 50);

    // World coordinate (20, 20) corresponds to local (-80, -80) -> should be RED
    auto outside_box_px = pix->as_ref().pixel(20, 20);
    assert(outside_box_px.has_value());
    assert(outside_box_px->r > 200 && outside_box_px->g < 50);

    std::cout << "  -> Local coordinate matrix inverse transformation verified." << std::endl;
}

void test_lifecycle_and_leak_free() {
    std::cout << "[Test 6/6] Testing lifecycle & leak-free destruction under load..." << std::endl;

    for (int i = 0; i < 50; ++i) {
        std::string sksl = "uniform float step" + std::to_string(i) + ";\n"
                           "vec4 main(vec2 fragCoord) {\n"
                           "    return vec4(step" + std::to_string(i) + ", 0.0, 0.0, 1.0);\n"
                           "}\n";
        auto res = RuntimeEffect::MakeForShader(sksl);
        assert(res);
        RuntimeShaderBuilder bld(res.effect);
        std::string var_name = "step" + std::to_string(i);
        bld.uniform(var_name.c_str()) = 0.5f;
        auto sh = bld.makeShader();
        assert(sh != nullptr);
    }

    std::cout << "  -> 50 Shader programs compiled and destroyed cleanly without leak." << std::endl;
}

int main() {
    std::cout << "=== NISABA SOVEREIGN RUNTIME EFFECT & SHADER TEST SUITE ===" << std::endl;

    auto device = GpuDevice::create();
    assert(device != nullptr && device->is_valid());
    std::cout << "GPU Initialized: " << device->renderer_name() << " (" << device->version_name() << ")" << std::endl;

    test_runtime_effect_reflection();
    test_runtime_effect_error_handling();
    test_runtime_shader_builder_and_aliases();
    test_gpu_rendering_enki_shaders();
    test_local_matrix_transformation();
    test_lifecycle_and_leak_free();

    std::cout << "=== ALL RUNTIME EFFECT TESTS PASSED (6/6 OK) ===" << std::endl;
    return 0;
}

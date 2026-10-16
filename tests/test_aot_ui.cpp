// Nisaba — Sovereign Full-Stack 2D Graphics Engine
// Copyright (C) 2024-2026 Hussein Jaafar
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <string>
#include <filesystem>

#include "nisaba/nisaba.hpp"
#include "nisaba/vspr/vspr.hpp"

using namespace nisaba;
using namespace nisaba::vspr::aot;

void test_slot_registration_and_punchout() {
    std::cout << "[Test 1] Dynamic Slot Registration & Punch-Out ... ";

    AotSceneCompiler compiler(400, 300);
    compiler.set_punch_out_slots(true);

    // Register 4 diverse slot shapes
    auto rect_slot = compiler.register_slot_rect("rect_display", *Rect::from_xywh(20, 20, 100, 40));
    auto rrect_slot = compiler.register_slot_rrect("rrect_card", *Rect::from_xywh(140, 20, 100, 40), 8.0f, 8.0f);
    auto circle_slot = compiler.register_slot_circle("circle_avatar", Point::from_xy(280, 40), 20.0f);

    PathBuilder pb;
    pb.move_to(Point::from_xy(50, 150));
    pb.line_to(Point::from_xy(150, 150));
    pb.line_to(Point::from_xy(100, 250));
    pb.close();
    auto triangle_path = pb.finish();
    assert(triangle_path.has_value());
    auto path_slot = compiler.register_slot_path("custom_triangle", *Rect::from_xywh(50, 150, 100, 100), *triangle_path);

    assert(rect_slot == 1);
    assert(rrect_slot == 2);
    assert(circle_slot == 3);
    assert(path_slot == 4);

    // Draw solid red static background
    compiler.record_static_pass([](Canvas& c) {
        c.fill_rect(*Rect::from_xywh(0, 0, 400, 300), Paint(Color::from_rgba8(255, 0, 0, 255)));
    });

    auto surface = compiler.compile();
    assert(surface != nullptr);
    assert(surface->slots().size() == 4);

    // Pixels outside slots must be red
    auto p_outside = surface->base_plate().pixel(5, 5);
    assert(p_outside.has_value());
    assert(p_outside->r == 255 && p_outside->a == 255);

    // Pixels inside punched-out slot must be transparent
    auto p_punched = surface->base_plate().pixel(30, 30);
    assert(p_punched.has_value());
    assert(p_punched->a == 0); // Punched out to transparent!

    std::cout << "PASSED\n";
}

void test_component_state_atlas() {
    std::cout << "[Test 2] Component State Sub-Atlas (Hover & Pressed) ... ";

    AotSceneCompiler compiler(400, 300);

    // Background is dark blue
    compiler.record_static_pass([](Canvas& c) {
        c.fill_rect(*Rect::from_xywh(0, 0, 400, 300), Paint(Color::from_rgba8(10, 20, 40, 255)));
        // Button in Normal State: Gray
        c.fill_round_rect(*Rect::from_xywh(50, 50, 80, 40), 6.0f, 6.0f, Paint(Color::from_rgba8(100, 100, 100, 255)));
    });

    // Register Button Hover State: Cyan
    constexpr uint32_t BTN_ID = 101;
    compiler.register_component_state(BTN_ID, ComponentState::Hover, *Rect::from_xywh(50, 50, 80, 40),
        [](Canvas& c, const Rect& local_bounds) {
            c.fill_round_rect(local_bounds, 6.0f, 6.0f, Paint(Color::from_rgba8(0, 220, 255, 255)));
        }
    );

    // Register Button Pressed State: Amber Orange
    compiler.register_component_state(BTN_ID, ComponentState::Pressed, *Rect::from_xywh(50, 50, 80, 40),
        [](Canvas& c, const Rect& local_bounds) {
            c.fill_round_rect(local_bounds, 6.0f, 6.0f, Paint(Color::from_rgba8(255, 140, 0, 255)));
        }
    );

    auto surface = compiler.compile();
    assert(surface != nullptr);
    assert(surface->state_atlas().slices().size() == 2);

    auto target_opt = Pixmap::create(400, 300);
    assert(target_opt.has_value());
    PixmapMut target = target_opt->as_mut();

    // 1. Render in Normal state: button should be gray (100, 100, 100)
    surface->render(target);
    auto p_norm = target.pixel(70, 70);
    assert(p_norm.has_value());
    assert(p_norm->r == 100 && p_norm->g == 100 && p_norm->b == 100);

    // 2. Set Hover state: button should become cyan (0, 220, 255)
    surface->set_component_state(BTN_ID, ComponentState::Hover);
    surface->render(target);
    auto p_hov = target.pixel(70, 70);
    assert(p_hov.has_value());
    assert(p_hov->r == 0 && p_hov->g == 220 && p_hov->b == 255);

    // 3. Set Pressed state: button should become orange (255, 140, 0)
    surface->set_component_state(BTN_ID, ComponentState::Pressed);
    surface->render(target);
    auto p_press = target.pixel(70, 70);
    assert(p_press.has_value());
    assert(p_press->r == 255 && p_press->g == 140 && p_press->b == 0);

    // 4. Reset to Normal: button reverts back to gray
    surface->set_component_state(BTN_ID, ComponentState::Normal);
    surface->render(target);
    auto p_revert = target.pixel(70, 70);
    assert(p_revert.has_value());
    assert(p_revert->r == 100 && p_revert->g == 100 && p_revert->b == 100);

    std::cout << "PASSED\n";
}

void test_dynamic_slot_update_clipping() {
    std::cout << "[Test 3] Dynamic Slot Live Update & Scissor Clipping ... ";

    AotSceneCompiler compiler(400, 300);
    compiler.register_slot_rect("display", *Rect::from_xywh(50, 50, 100, 50));

    compiler.record_static_pass([](Canvas& c) {
        c.fill_rect(*Rect::from_xywh(0, 0, 400, 300), Paint(Color::from_rgba8(20, 20, 20, 255)));
    });

    auto surface = compiler.compile();
    assert(surface != nullptr);

    auto target_opt = Pixmap::create(400, 300);
    assert(target_opt.has_value());
    PixmapMut target = target_opt->as_mut();

    surface->render(target);

    // Draw into dynamic slot, intentionally trying to draw a huge rect (0,0, 400,300) in green
    Canvas live_canvas(target);
    surface->update_slot_cpu("display", live_canvas, [](Canvas& c, const Rect& bounds) {
        // Attempting to draw oversized rectangle
        c.fill_rect(*Rect::from_xywh(0, 0, 400, 300), Paint(Color::from_rgba8(0, 255, 0, 255)));
    });

    // Pixels inside slot (e.g. 75, 75) must be green
    auto p_in = target.pixel(75, 75);
    assert(p_in.has_value());
    assert(p_in->r == 0 && p_in->g == 255 && p_in->b == 0);

    // Pixels OUTSIDE slot (e.g. 20, 20 or 200, 200) MUST NOT be green (clipping protected them!)
    auto p_out = target.pixel(20, 20);
    assert(p_out.has_value());
    assert(p_out->r == 20 && p_out->g == 20 && p_out->b == 20);

    std::cout << "PASSED\n";
}

void test_vspr_binary_serialization() {
    std::cout << "[Test 4] Native .vspr Binary Serialization & Instant Deserialization ... ";

    AotSceneCompiler compiler(500, 400);
    compiler.register_slot_rect("header", *Rect::from_xywh(10, 10, 200, 30));
    compiler.register_slot_circle("sensor", Point::from_xy(350, 100), 25.0f);

    compiler.register_component_state(42, ComponentState::Hover, *Rect::from_xywh(80, 80, 60, 30),
        [](Canvas& c, const Rect& bounds) {
            c.fill_rect(bounds, Paint(Color::from_rgba8(255, 255, 0, 255)));
        }
    );

    compiler.record_static_pass([](Canvas& c) {
        c.fill_rect(*Rect::from_xywh(0, 0, 500, 400), Paint(Color::from_rgba8(33, 44, 55, 255)));
    });

    auto surface1 = compiler.compile();
    assert(surface1 != nullptr);

    // Save to binary file
    const std::string test_file = "test_compiled_scene.vspr";
    bool saved = surface1->save_to_file(test_file);
    assert(saved);

    // Load from binary file
    auto surface2 = CompiledUiSurface::load_from_file(test_file);
    assert(surface2 != nullptr);
    assert(surface2->width() == 500);
    assert(surface2->height() == 400);
    assert(surface2->slots().size() == 2);
    assert(surface2->slots()[0].tag == "header");
    assert(surface2->slots()[1].tag == "sensor");
    assert(surface2->slots()[1].shape_type == SlotShape::Circle);
    assert(surface2->slots()[1].rx == 25.0f);

    assert(surface2->state_atlas().slices().size() == 1);
    assert(surface2->state_atlas().slices()[0].comp_id == 42);
    assert(surface2->state_atlas().slices()[0].state == ComponentState::Hover);

    // Pixel verification between saved and loaded surface
    auto p1 = surface1->base_plate().pixel(100, 100);
    auto p2 = surface2->base_plate().pixel(100, 100);
    assert(p1.has_value() && p2.has_value());
    assert(p1->r == p2->r && p1->g == p2->g && p1->b == p2->b && p1->a == p2->a);

    std::filesystem::remove(test_file);
    std::cout << "PASSED\n";
}

void test_runtime_render_performance() {
    std::cout << "[Test 5] Runtime Blit Latency Benchmarking ... ";

    AotSceneCompiler compiler(800, 600);
    compiler.register_slot_rect("main_viewport", *Rect::from_xywh(50, 50, 700, 500));

    for (uint32_t i = 1; i <= 10; ++i) {
        compiler.register_component_state(i, ComponentState::Hover, *Rect::from_xywh(i * 60.0f, 20.0f, 50.0f, 25.0f),
            [](Canvas& c, const Rect& b) {
                c.fill_rect(b, Paint(Color::from_rgba8(0, 200, 255, 255)));
            }
        );
    }

    compiler.record_static_pass([](Canvas& c) {
        c.fill_rect(*Rect::from_xywh(0, 0, 800, 600), Paint(Color::from_rgba8(15, 20, 30, 255)));
    });

    auto surface = compiler.compile();
    assert(surface != nullptr);

    // Activate 3 hover states
    surface->set_component_state(1, ComponentState::Hover);
    surface->set_component_state(4, ComponentState::Hover);
    surface->set_component_state(7, ComponentState::Hover);

    auto target_opt = Pixmap::create(800, 600);
    assert(target_opt.has_value());
    PixmapMut target = target_opt->as_mut();

    // Warm-up
    surface->render(target);

    // Execute 100 consecutive frame composites
    double total_us = 0.0;
    constexpr int ITER = 100;
    for (int i = 0; i < ITER; ++i) {
        surface->render(target);
        total_us += surface->telemetry().total_frame_us;
    }
    double avg_us = total_us / ITER;

    std::cout << "PASSED (Mean Frame Latency: " << avg_us << " us, Coverage: "
              << surface->telemetry().static_pixel_coverage_pct << "% static)\n";
}

int main() {
    std::cout << "=================================================================\n";
    std::cout << "  NISABA VSPR: Ahead-Of-Time (AOT) UI Compilation Unit Tests     \n";
    std::cout << "=================================================================\n";

    test_slot_registration_and_punchout();
    test_component_state_atlas();
    test_dynamic_slot_update_clipping();
    test_vspr_binary_serialization();
    test_runtime_render_performance();

    std::cout << ">> ALL VSPR AOT UI COMPILATION TESTS PASSED!\n";
    return 0;
}

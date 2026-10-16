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

#include "nisaba/nisaba.hpp"
#include "nisaba/vspr/vspr.hpp"

using namespace nisaba;
using namespace nisaba::vspr;

void test_archetype_creation() {
    std::cout << "[Test 1] Archetype Creation & Caching ... ";
    auto& lib = ArchetypeLibrary::instance();
    lib.clear();

    assert(lib.cached_archetypes_count() == 0);

    // Create 80x50 button prototype with 8px radius
    auto arch1 = lib.get_or_create_rounded_rect(80, 50, 8.0f, 8.0f);
    assert(arch1 != nullptr);
    assert(arch1->width() == 80);
    assert(arch1->height() == 50);
    assert(arch1->is_valid());
    assert(lib.cached_archetypes_count() == 1);

    // Requesting identical dimensions & radius must return identical cached instance
    auto arch2 = lib.get_or_create_rounded_rect(80, 50, 8.0f, 8.0f);
    assert(arch2 == arch1);
    assert(lib.cached_archetypes_count() == 1);
    assert(lib.total_created_count() == 1);

    // Different size creates a separate archetype
    auto arch3 = lib.get_or_create_rounded_rect(100, 60, 8.0f, 8.0f);
    assert(arch3 != nullptr);
    assert(arch3 != arch1);
    assert(lib.cached_archetypes_count() == 2);

    std::cout << "PASSED\n";
}

void test_calculator_grid_instancing() {
    std::cout << "[Test 2] Calculator 20-Button Grid Instancing (Homogeneous Re-use) ... ";
    auto pixmap_opt = Pixmap::create(600, 800);
    assert(pixmap_opt.has_value());
    PixmapMut target = pixmap_opt->as_mut();

    auto& lib = ArchetypeLibrary::instance();
    lib.clear();

    PrimitiveInstancer instancer;

    // Build a 4x5 calculator keyboard (20 buttons, 80x60 each, radius 10px)
    constexpr int ROWS = 5;
    constexpr int COLS = 4;
    constexpr float BTN_W = 80.0f;
    constexpr float BTN_H = 60.0f;
    constexpr float GAP = 15.0f;
    constexpr float START_X = 50.0f;
    constexpr float START_Y = 100.0f;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            float x = START_X + c * (BTN_W + GAP);
            float y = START_Y + r * (BTN_H + GAP);
            Color col = (c == 3) ? Color::from_rgba8(255, 149, 0, 255) // Orange operator
                                 : Color::from_rgba8(51, 51, 51, 255);  // Dark gray button
            instancer.add_button(*Rect::from_xywh(x, y, BTN_W, BTN_H), col, 10.0f);
        }
    }

    assert(instancer.instance_count() == 20);

    // Render all 20 buttons
    instancer.render(target);

    const auto& tel = instancer.telemetry();
    assert(tel.total_instances == 20);
    assert(tel.archetypes_newly_built == 1); // Exact prototype built ONCE!
    assert(tel.archetypes_reused == 19);    // 19 buttons re-used the same archetype!
    assert(tel.bezier_curves_saved == 80);  // 80 curves avoided!

    // Verify center pixel of button (0,0) has expected color
    auto p0 = target.pixel(static_cast<uint32_t>(START_X + BTN_W * 0.5f),
                           static_cast<uint32_t>(START_Y + BTN_H * 0.5f));
    assert(p0.has_value());
    assert(p0->r == 51 && p0->g == 51 && p0->b == 51 && p0->a == 255);

    // Verify center pixel of button (0,3) is orange
    float x_op = START_X + 3 * (BTN_W + GAP);
    auto p_op = target.pixel(static_cast<uint32_t>(x_op + BTN_W * 0.5f),
                             static_cast<uint32_t>(START_Y + BTN_H * 0.5f));
    assert(p_op.has_value());
    assert(p_op->r == 255 && p_op->g == 149 && p_op->b == 0 && p_op->a == 255);

    std::cout << "PASSED (Rendered 20 buttons in " << tel.render_time_us << " us)\n";
}

void test_diverse_primitives() {
    std::cout << "[Test 3] Diverse Primitive Types (Circle, Pill, Rect) ... ";
    auto pixmap_opt = Pixmap::create(400, 400);
    assert(pixmap_opt.has_value());
    PixmapMut target = pixmap_opt->as_mut();

    PrimitiveInstancer instancer;

    // Add sharp rect
    instancer.add_rect(*Rect::from_xywh(10, 10, 50, 50), Color::from_rgba8(255, 0, 0, 255));

    // Add circle
    instancer.add_circle(Point::from_xy(100, 100), 20.0f, Color::from_rgba8(0, 255, 0, 255));

    // Add pill badge
    instancer.add_pill(*Rect::from_xywh(150, 50, 100, 30), Color::from_rgba8(0, 0, 255, 255));

    assert(instancer.instance_count() == 3);

    instancer.render(target);

    // Center of rect should be red
    auto cr = target.pixel(35, 35);
    assert(cr.has_value());
    assert(cr->r == 255 && cr->g == 0 && cr->b == 0);

    // Center of circle should be green
    auto cg = target.pixel(100, 100);
    assert(cg.has_value());
    assert(cg->r == 0 && cg->g == 255 && cg->b == 0);

    // Center of pill should be blue
    auto cb = target.pixel(200, 65);
    assert(cb.has_value());
    assert(cb->r == 0 && cb->g == 0 && cb->b == 255);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "==================================================\n";
    std::cout << "  NISABA VSPR: Primitive & Shape Instancer Tests  \n";
    std::cout << "==================================================\n";

    test_archetype_creation();
    test_calculator_grid_instancing();
    test_diverse_primitives();

    std::cout << ">> ALL VSPR INSTANCER TESTS PASSED!\n";
    return 0;
}

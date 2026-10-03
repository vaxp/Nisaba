#include "nisaba/nisaba.hpp"
#include <iostream>
#include <vector>
#include <cmath>

using namespace nisaba;

int main() {
    std::cout << "=================================================" << std::endl;
    std::cout << "   Nisaba 2D Software Rendering Engine Showcase   " << std::endl;
    std::cout << "   Embedded-First, Zero-Dependency CPU Rasterizer " << std::endl;
    std::cout << "=================================================" << std::endl;

    const uint32_t width = 800;
    const uint32_t height = 600;

    auto pixmap = Pixmap::create(width, height);
    if (!pixmap) {
        std::cerr << "Failed to allocate pixmap!" << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // 1. Background: Dark sleek gradient
    {
        std::vector<GradientStop> bg_stops = {
            GradientStop::create(0.0f, Color::from_rgba8(15, 17, 26, 255)),
            GradientStop::create(0.5f, Color::from_rgba8(25, 30, 48, 255)),
            GradientStop::create(1.0f, Color::from_rgba8(10, 12, 18, 255))
        };
        auto bg_grad = LinearGradient::create(
            Point::from_xy(0.0f, 0.0f),
            Point::from_xy(0.0f, static_cast<float>(height)),
            bg_stops
        );
        if (bg_grad) {
            Paint bg_paint;
            bg_paint.shader = Shader(*bg_grad);
            canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)), bg_paint);
        }
    }

    // 2. Glowing Orb in the center: Radial Gradient
    {
        std::vector<GradientStop> orb_stops = {
            GradientStop::create(0.0f, Color::from_rgba8(255, 230, 150, 255)), // Bright warm core
            GradientStop::create(0.2f, Color::from_rgba8(255, 100, 50, 220)),  // Orange glow
            GradientStop::create(0.6f, Color::from_rgba8(180, 20, 100, 150)),  // Magenta transition
            GradientStop::create(1.0f, Color::from_rgba8(20, 10, 40, 0))       // Fading to transparent
        };
        auto orb_grad = RadialGradient::create(
            Point::from_xy(400.0f, 300.0f),
            180.0f,
            orb_stops
        );
        if (orb_grad) {
            Paint orb_paint;
            orb_paint.shader = Shader(*orb_grad);
            orb_paint.blend_mode = BlendMode::SourceOver;
            canvas.fill_circle(400.0f, 300.0f, 180.0f, orb_paint);
        }
    }

    // 3. Elegant Rotated Orbital Rings with Dashed Strokes
    {
        canvas.save();
        canvas.translate(400.0f, 300.0f);

        auto dash = StrokeDash::create({15.0f, 10.0f, 5.0f, 10.0f}, 0.0f);

        Stroke ring_stroke(2.5f);
        ring_stroke.line_cap = LineCap::Round;
        ring_stroke.line_join = LineJoin::Round;
        ring_stroke.dash = dash;

        Paint ring_paint;
        ring_paint.blend_mode = BlendMode::Plus; // Additive glow

        for (int i = 0; i < 6; ++i) {
            canvas.rotate(30.0f);
            ring_paint.set_color_rgba8(100 + i * 25, 180 + i * 10, 255, 160);

            PathBuilder pb;
            pb.push_oval(*Rect::from_xywh(-220.0f, -80.0f, 440.0f, 160.0f));
            auto p = pb.finish();
            if (p) {
                canvas.stroke_path(*p, ring_paint, ring_stroke);
            }
        }
        canvas.restore();
    }

    // 4. Smooth Bezier Ribbon Waves
    {
        PathBuilder pb;
        pb.move_to(0.0f, 480.0f);
        pb.cubic_to(Point::from_xy(200.0f, 380.0f), Point::from_xy(350.0f, 580.0f), Point::from_xy(500.0f, 480.0f));
        pb.cubic_to(Point::from_xy(650.0f, 380.0f), Point::from_xy(750.0f, 520.0f), Point::from_xy(800.0f, 460.0f));
        pb.line_to(800.0f, 600.0f);
        pb.line_to(0.0f, 600.0f);
        pb.close();

        auto wave_path = pb.finish();
        if (wave_path) {
            std::vector<GradientStop> wave_stops = {
                GradientStop::create(0.0f, Color::from_rgba8(0, 220, 255, 180)),
                GradientStop::create(0.5f, Color::from_rgba8(130, 50, 255, 160)),
                GradientStop::create(1.0f, Color::from_rgba8(255, 50, 150, 180))
            };
            auto wave_grad = LinearGradient::create(
                Point::from_xy(0.0f, 400.0f),
                Point::from_xy(800.0f, 600.0f),
                wave_stops
            );
            if (wave_grad) {
                Paint wave_paint;
                wave_paint.shader = Shader(*wave_grad);
                wave_paint.blend_mode = BlendMode::SourceOver;
                canvas.fill_path(*wave_path, wave_paint);
            }
        }
    }

    // 5. Angular Sweep Gradient Showcase (Color Wheel Badge)
    {
        std::vector<GradientStop> sweep_stops = {
            GradientStop::create(0.00f, Color::from_rgba8(255, 0, 0, 230)),
            GradientStop::create(0.17f, Color::from_rgba8(255, 255, 0, 230)),
            GradientStop::create(0.33f, Color::from_rgba8(0, 255, 0, 230)),
            GradientStop::create(0.50f, Color::from_rgba8(0, 255, 255, 230)),
            GradientStop::create(0.67f, Color::from_rgba8(0, 0, 255, 230)),
            GradientStop::create(0.83f, Color::from_rgba8(255, 0, 255, 230)),
            GradientStop::create(1.00f, Color::from_rgba8(255, 0, 0, 230))
        };
        auto sweep = SweepGradient::create(
            Point::from_xy(100.0f, 100.0f),
            0.0f, 360.0f,
            sweep_stops
        );
        if (sweep) {
            Paint sp;
            sp.shader = Shader(*sweep);
            canvas.fill_circle(100.0f, 100.0f, 50.0f, sp);

            Stroke b_stroke(4.0f);
            Paint bp;
            bp.set_color_rgba8(255, 255, 255, 220);
            canvas.stroke_circle(100.0f, 100.0f, 50.0f, bp, b_stroke);
        }
    }

    // 6. Masked Pattern Card (Demonstrating Bilinear Image Sampling + Clip Mask)
    {
        // Generate a mini procedural texture (60x60)
        auto tex = Pixmap::create(60, 60);
        if (tex) {
            for (uint32_t ty = 0; ty < 60; ++ty) {
                for (uint32_t tx = 0; tx < 60; ++tx) {
                    uint8_t c1 = static_cast<uint8_t>((tx * 255) / 60);
                    uint8_t c2 = static_cast<uint8_t>((ty * 255) / 60);
                    tex->set_pixel(tx, ty, PremultipliedColorU8::from_rgba_unchecked(c1, c2, 200, 255));
                }
            }

            // Draw transformed with bilinear pattern
            canvas.save();
            canvas.translate(650.0f, 120.0f);
            canvas.rotate(15.0f);

            PixmapPaint pp;
            pp.quality = FilterQuality::Bilinear;
            pp.opacity = 0.9f;
            canvas.draw_pixmap(-50, -50, tex->as_ref(), pp);

            Stroke border(3.0f);
            border.line_join = LineJoin::Round;
            Paint bp;
            bp.set_color_rgba8(255, 255, 255, 240);
            canvas.stroke_rect(*Rect::from_xywh(-50.0f, -50.0f, 60.0f, 60.0f), bp, border);

            canvas.restore();
        }
    }

    // 7. Typography & Status HUD (Zero-Dependency Built-in Font)
    {
        Paint text_paint;
        text_paint.set_color_rgba8(255, 255, 255, 230);
        canvas.draw_text_debug("NISABA 2D ENGINE (C++20) - SOVEREIGN ARCHITECTURE", 20.0f, 25.0f, text_paint);

        Paint subtext_paint;
        subtext_paint.set_color_rgba8(0, 220, 255, 200);
        canvas.draw_text_debug("VAXP Organization | ZERO-DEPENDENCY | HARDWARE STRIDE | SIMD", 20.0f, 45.0f, subtext_paint);

        Paint hud_paint;
        hud_paint.set_color_rgba8(255, 200, 50, 220);
        canvas.draw_text_debug("CPU RASTERIZER - VECTOR CLIP PATHS - ZERO-COPY FRAMEBUFFER", 20.0f, 570.0f, hud_paint);
    }

    // Save outputs
    const char* bmp_path = "nisaba_showcase.bmp";
    const char* ppm_path = "nisaba_showcase.ppm";

    bool bmp_ok = pixmap->save_bmp(bmp_path);
    bool ppm_ok = pixmap->save_ppm(ppm_path);

    if (bmp_ok && ppm_ok) {
        std::cout << "[SUCCESS] Saved showcase images to:" << std::endl;
        std::cout << "  - " << bmp_path << std::endl;
        std::cout << "  - " << ppm_path << std::endl;
    } else {
        std::cerr << "[ERROR] Failed to save showcase output images." << std::endl;
        return 1;
    }

    std::cout << "Nisaba engine showcase finished flawlessly!" << std::endl;
    return 0;
}

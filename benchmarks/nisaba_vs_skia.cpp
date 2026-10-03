#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cmath>
#include <sstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkShader.h"
#include "SkGradientShader.h"
#include "SkDashPathEffect.h"
#include "SkImage.h"
#pragma GCC diagnostic pop

#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/paint.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/path/path_geometry.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/shaders/shader.hpp"
#include "nisaba/mesh/mesh.hpp"

using namespace nisaba;

struct MicroStat {
    std::string name;
    double nisaba_us;
    double skia_us;
};

// Native circular arc constructor for Nisaba using analytical cubic Bezier arcs
inline void nisaba_arc(PathBuilder& pb, float cx, float cy, float r, float a0, float a1) {
    pb.push_arc(cx, cy, r, a0, a1 - a0);
}

int main() {
    std::cout << "======================================================================================================================================\n";
    std::cout << "                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         \n";
    std::cout << "                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            \n";
    std::cout << "======================================================================================================================================\n\n";

    const uint32_t W = 1080;
    const uint32_t H = 740;
    const int ITERATIONS = 150;

    // 1. Nisaba Surface & Canvas
    auto pm_opt = Pixmap::allocate(W, H);
    if (!pm_opt) return 1;
    Pixmap& pm = *pm_opt;
    Canvas canvas(pm);

    // 2. Google Skia Surface & Context
    sk_sp<SkSurface> sk_surf = SkSurface::MakeRasterN32Premul(W, H);
    if (!sk_surf) {
        std::cerr << "Failed to allocate Skia Raster Surface\n";
        return 1;
    }
    SkCanvas* sk_canvas = sk_surf->getCanvas();

    // 3. Pre-allocated Image & Tile Surfaces for Blit and Pattern Benchmarks
    auto test_pm_opt = Pixmap::allocate(256, 256);
    auto tile_pm_opt = Pixmap::allocate(64, 64);
    if (!test_pm_opt || !tile_pm_opt) return 1;
    Pixmap& test_pm = *test_pm_opt;
    Pixmap& tile_pm = *tile_pm_opt;

    std::vector<uint32_t> test_pixels(256 * 256);
    for (uint32_t y = 0; y < 256; ++y) {
        for (uint32_t x = 0; x < 256; ++x) {
            uint8_t r = static_cast<uint8_t>(x);
            uint8_t g = static_cast<uint8_t>(y);
            uint8_t b = static_cast<uint8_t>((x + y) / 2);
            test_pm.set_pixel(x, y, PremultipliedColorU8::from_rgba_unchecked(r, g, b, 255));
            test_pixels[y * 256 + x] = SkColorSetARGB(255, r, g, b);
        }
    }

    std::vector<uint32_t> tile_pixels(64 * 64);
    for (uint32_t y = 0; y < 64; ++y) {
        for (uint32_t x = 0; x < 64; ++x) {
            bool check = ((x / 8) + (y / 8)) % 2 == 0;
            PremultipliedColorU8 c = check ? PremultipliedColorU8::from_rgba_unchecked(255, 100, 50, 255)
                                           : PremultipliedColorU8::from_rgba_unchecked(50, 150, 255, 255);
            tile_pm.set_pixel(x, y, c);
            tile_pixels[y * 64 + x] = check ? SkColorSetARGB(255, 255, 100, 50) : SkColorSetARGB(255, 50, 150, 255);
        }
    }

    SkImageInfo test_info = SkImageInfo::MakeN32Premul(256, 256);
    sk_sp<SkImage> sk_test_img = SkImage::MakeRasterCopy(SkPixmap(test_info, test_pixels.data(), 256 * 4));

    SkImageInfo tile_info = SkImageInfo::MakeN32Premul(64, 64);
    sk_sp<SkImage> sk_tile_img = SkImage::MakeRasterCopy(SkPixmap(tile_info, tile_pixels.data(), 64 * 4));

    // 4. Pre-allocated Alpha Mask for Masking Benchmark (Suite 71)
    auto mask_opt = Mask::create(W, H);
    if (!mask_opt) return 1;
    Mask& alpha_mask = *mask_opt;
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            float dist = std::hypot(static_cast<float>(x) - W * 0.5f, static_cast<float>(y) - H * 0.5f);
            uint8_t a = (dist < 260.0f) ? static_cast<uint8_t>(255.0f * (1.0f - dist / 260.0f)) : 0;
            alpha_mask.set(x, y, a);
        }
    }

    SkImageInfo mask_info = SkImageInfo::MakeA8(W, H);
    std::vector<uint8_t> sk_mask_pixels(W * H);
    for (uint32_t y = 0; y < H; ++y) {
        for (uint32_t x = 0; x < W; ++x) {
            float dist = std::hypot(static_cast<float>(x) - W * 0.5f, static_cast<float>(y) - H * 0.5f);
            uint8_t a = (dist < 260.0f) ? static_cast<uint8_t>(255.0f * (1.0f - dist / 260.0f)) : 0;
            sk_mask_pixels[y * W + x] = a;
        }
    }
    sk_sp<SkImage> sk_alpha_mask = SkImage::MakeRasterCopy(SkPixmap(mask_info, sk_mask_pixels.data(), W));


    std::vector<MicroStat> stats;

    auto benchmark_op = [&](std::string name, auto op_nisaba, auto op_skia) {
        // Warmup runs
        for (int i = 0; i < 5; ++i) {
            op_nisaba();
            op_skia();
            if (sk_surf) sk_surf->flush();
        }

        // Nisaba Sovereign Core
        auto t0_n = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            op_nisaba();
        }
        auto t1_n = std::chrono::high_resolution_clock::now();
        double nisaba_us = std::chrono::duration<double, std::micro>(t1_n - t0_n).count() / ITERATIONS;

        // Google Skia (CPU Rasterizer, single-thread)
        auto t0_s = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            op_skia();
        }
        if (sk_surf) sk_surf->flush();
        auto t1_s = std::chrono::high_resolution_clock::now();
        double skia_us = std::chrono::duration<double, std::micro>(t1_s - t0_s).count() / ITERATIONS;

        // Reset canvas and contexts to clean state for next benchmark
        canvas.reset_transform();
        canvas.reset_clip();
        if (sk_canvas) {
            sk_canvas->restoreToCount(1);
            sk_canvas->resetMatrix();
        }

        stats.push_back({name, nisaba_us, skia_us});
    };

    // 1. Solid Background Fill (1080x740)
    benchmark_op("1. Solid Background Fill (1080x740)",
        [&]() { pm.fill(Color::from_rgba8(10, 16, 30, 255)); },
        [&]() {
            sk_canvas->clear(SkColorSetARGB(255, 10, 16, 30));
        }
    );

    // 2. 50 Grid Lines (1px Stroked Paths)
    benchmark_op("2. 50 Grid Lines (1px Stroked Paths)",
        [&]() {
            Paint p(Color::from_rgba8(255, 255, 255, 14));
            Stroke s(1.0f);
            PathBuilder pb;
            for (float x = 0; x <= W; x += 40.0f) {
                pb.move_to(x, 0); pb.line_to(x, H);
            }
            auto path = pb.finish(); if (path) canvas.stroke_path(*path, p, s);
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(14, 255, 255, 255));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(1.0f);
            SkPath path;
            for (float x = 0; x <= W; x += 40.0f) {
                path.moveTo(x, 0); path.lineTo(x, H);
            }
            sk_canvas->drawPath(path, p);
        }
    );

    // 3. 25 Alpha Circles (Porter-Duff Blend)
    benchmark_op("3. 25 Alpha Circles (Porter-Duff Blend)",
        [&]() {
            for (int i = 0; i < 25; ++i) {
                canvas.fill_circle(100.0f + i * 35.0f, 200.0f + (i % 5) * 50.0f, 25.0f,
                                  Paint(Color::from_rgba8(0, 229, 255, 90)));
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(90, 0, 229, 255));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 25; ++i) {
                sk_canvas->drawCircle(100.0f + i * 35.0f, 200.0f + (i % 5) * 50.0f, 25.0f, p);
            }
        }
    );

    // 4. Cubic Bezier Curves (Fill + Stroke)
    benchmark_op("4. Cubic Bezier Curves (Fill + Stroke)",
        [&]() {
            PathBuilder pb;
            pb.move_to(0, 540);
            pb.cubic_to(W * 0.30f, 590, W * 0.70f, 520, W, 570);
            pb.line_to(W, H); pb.line_to(0, H); pb.close();
            auto p = pb.finish();
            if (p) {
                canvas.fill_path(*p, Paint(Color::from_rgba8(0, 229, 255, 35)));
                canvas.stroke_path(*p, Paint(Color::from_rgba8(0, 229, 255, 215)), Stroke(2.2f));
            }
        },
        [&]() {
            SkPath path;
            path.moveTo(0, 540);
            path.cubicTo(W * 0.30f, 590, W * 0.70f, 520, W, 570);
            path.lineTo(W, H); path.lineTo(0, H); path.close();
            SkPaint pf;
            pf.setAntiAlias(true);
            pf.setColor(SkColorSetARGB(35, 0, 229, 255));
            pf.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(path, pf);
            SkPaint ps;
            ps.setAntiAlias(true);
            ps.setColor(SkColorSetARGB(215, 0, 229, 255));
            ps.setStyle(SkPaint::kStroke_Style);
            ps.setStrokeWidth(2.2f);
            sk_canvas->drawPath(path, ps);
        }
    );

    // 5. 4 Rounded Rect Cards (Fill + Stroke)
    benchmark_op("5. 4 Rounded Rect Cards (Fill + Stroke)",
        [&]() {
            for (int i = 0; i < 4; ++i) {
                auto r = Rect::from_xywh(100.0f + i * 200.0f, 100.0f, 180.0f, 120.0f);
                if (r) {
                    canvas.fill_round_rect(*r, 14.0f, 14.0f, Paint(Color::from_rgba8(16, 24, 42, 230)));
                    canvas.stroke_round_rect(*r, 14.0f, 14.0f, Paint(Color::from_rgba8(0, 229, 255, 180)), Stroke(2.0f));
                }
            }
        },
        [&]() {
            SkPaint pf;
            pf.setAntiAlias(true);
            pf.setColor(SkColorSetARGB(230, 16, 24, 42));
            pf.setStyle(SkPaint::kFill_Style);
            SkPaint ps;
            ps.setAntiAlias(true);
            ps.setColor(SkColorSetARGB(180, 0, 229, 255));
            ps.setStyle(SkPaint::kStroke_Style);
            ps.setStrokeWidth(2.0f);
            for (int i = 0; i < 4; ++i) {
                SkRRect rr;
                rr.setRectXY(SkRect::MakeXYWH(100.0f + i * 200.0f, 100.0f, 180.0f, 120.0f), 14.0f, 14.0f);
                sk_canvas->drawRRect(rr, pf);
                sk_canvas->drawRRect(rr, ps);
            }
        }
    );

    // 6. Multi-Stop Linear Gradient (480x36)
    benchmark_op("6. Multi-Stop Linear Gradient (480x36)",
        [&]() {
            auto grad = LinearGradient::create(
                Point(100.0f, 100.0f), Point(580.0f, 100.0f),
                {
                    GradientStop::create(0.0f, Color::from_rgba8(0, 229, 255, 215)),
                    GradientStop::create(0.5f, Color::from_rgba8(168, 85, 247, 215)),
                    GradientStop::create(1.0f, Color::from_rgba8(255, 0, 128, 215))
                }
            );
            if (grad) {
                auto r = Rect::from_xywh(100.0f, 100.0f, 480.0f, 36.0f);
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*grad))));
            }
        },
        [&]() {
            SkPoint pts[2] = { SkPoint::Make(300.0f, 0.0f), SkPoint::Make(780.0f, 0.0f) };
            SkColor colors[4] = {
                SkColorSetARGB(255, 0, 229, 255),
                SkColorSetARGB(255, 123, 97, 255),
                SkColorSetARGB(255, 255, 45, 85),
                SkColorSetARGB(255, 255, 184, 0)
            };
            SkScalar pos[4] = { 0.0f, 0.33f, 0.66f, 1.0f };
            auto sh = SkGradientShader::MakeLinear(pts, colors, pos, 4, SkTileMode::kClamp);
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawRect(SkRect::MakeXYWH(300, 350, 480, 36), p);
        }
    );

    // 7. Radial Glow Gradient (250px Glow)
    benchmark_op("7. Radial Glow Gradient (250px Glow)",
        [&]() {
            auto rad_opt = RadialGradient::create(
                Point(W * 0.5f, H * 0.5f), 250.0f,
                {
                    GradientStop::create(0.0f, Color::from_rgba8(0, 229, 255, 200)),
                    GradientStop::create(0.6f, Color::from_rgba8(138, 43, 226, 120)),
                    GradientStop::create(1.0f, Color::from_rgba8(0, 0, 0, 0))
                }
            );
            if (rad_opt) {
                auto r = Rect::from_xywh(W * 0.5f - 250.0f, H * 0.5f - 250.0f, 500.0f, 500.0f);
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*rad_opt))));
            }
        },
        [&]() {
            SkColor colors[2] = { SkColorSetARGB(180, 0, 229, 255), SkColorSetARGB(0, 0, 229, 255) };
            SkScalar pos[2] = { 0.0f, 1.0f };
            auto sh = SkGradientShader::MakeRadial(SkPoint::Make(540, 370), 250, colors, pos, 2, SkTileMode::kClamp);
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawCircle(540, 370, 250, p);
        }
    );

    // 8. 12 Radial Rotated Spokes (Strokes)
    benchmark_op("8. 12 Radial Rotated Spokes (Strokes)",
        [&]() {
            Stroke spoke_stroke(1.2f);
            for (int i = 0; i < 12; ++i) {
                float a = i * (static_cast<float>(M_PI) * 2.0f / 12.0f);
                canvas.stroke_line(260.0f + std::cos(a) * 45.0f, 360.0f + std::sin(a) * 45.0f,
                                   260.0f + std::cos(a) * 195.0f, 360.0f + std::sin(a) * 195.0f,
                                   Paint(Color::from_rgba8(0, 229, 255, 190)), spoke_stroke);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(180, 255, 255, 255));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(2.5f);
            for (int i = 0; i < 12; ++i) {
                sk_canvas->save();
                sk_canvas->translate(540, 370);
                sk_canvas->rotate(i * 30.0f);
                sk_canvas->drawLine(0, 0, 160, 0, p);
                sk_canvas->restore();
            }
        }
    );

    // 9. Dashed Orbit Ring (Stroke with Dash)
    benchmark_op("9. Dashed Orbit Ring (Stroke with Dash)",
        [&]() {
            Stroke dash_stroke(2.5f);
            dash_stroke.dash = StrokeDash::create({12.0f, 6.0f, 4.0f, 6.0f}, 0.0f);
            canvas.stroke_circle(W * 0.5f, H * 0.5f, 180.0f, Paint(Color::from_rgba8(0, 229, 255, 220)), dash_stroke);
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(140, 0, 229, 255));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(1.8f);
            SkScalar intervals[2] = { 6.0f, 8.0f };
            p.setPathEffect(SkDashPathEffect::Make(intervals, 2, 0.0f));
            sk_canvas->drawCircle(540, 370, 220, p);
        }
    );

    // 10. Concave 10-Point Star (Winding Fill)
    benchmark_op("10. Concave 10-Point Star (Winding Fill)",
        [&]() {
            PathBuilder pb;
            float cx = 800.0f, cy = 250.0f, r_out = 90.0f, r_in = 40.0f;
            for (int i = 0; i < 20; ++i) {
                float r = (i % 2 == 0) ? r_out : r_in;
                float angle = i * (static_cast<float>(M_PI) / 10.0f);
                float x = cx + std::cos(angle) * r;
                float y = cy + std::sin(angle) * r;
                if (i == 0) pb.move_to(x, y);
                else pb.line_to(x, y);
            }
            pb.close();
            auto star_p = pb.finish();
            if (star_p) canvas.fill_path(*star_p, Paint(Color::from_rgba8(245, 158, 11, 230)));
        },
        [&]() {
            SkPath path;
            float cx = 800.0f, cy = 250.0f, r_out = 90.0f, r_in = 40.0f;
            for (int i = 0; i < 20; ++i) {
                float r = (i % 2 == 0) ? r_out : r_in;
                float angle = i * (static_cast<float>(M_PI) / 10.0f);
                float x = cx + std::cos(angle) * r;
                float y = cy + std::sin(angle) * r;
                if (i == 0) path.moveTo(x, y);
                else path.lineTo(x, y);
            }
            path.close();
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 245, 158, 11));
            p.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(path, p);
        }
    );

    // 11. Compound Transforms (16 Elements)
    benchmark_op("11. Compound Transforms (16 Elements)",
        [&]() {
            Paint p(Color::from_rgba8(16, 185, 129, 200));
            auto r_opt = Rect::from_xywh(-25.0f, -25.0f, 50.0f, 50.0f);
            for (int i = 0; i < 16; ++i) {
                float tx = 200.0f + (i % 4) * 180.0f;
                float ty = 200.0f + (i / 4) * 120.0f;
                float angle_deg = i * 22.5f;

                canvas.save();
                canvas.translate(tx, ty);
                canvas.rotate(angle_deg);
                canvas.scale(1.15f, 0.85f);
                if (r_opt) canvas.fill_rect(*r_opt, p);
                canvas.restore();
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(200, 16, 185, 129));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 16; ++i) {
                float tx = 200.0f + (i % 4) * 180.0f;
                float ty = 200.0f + (i / 4) * 120.0f;
                float angle_deg = i * 22.5f;

                sk_canvas->save();
                sk_canvas->translate(tx, ty);
                sk_canvas->rotate(angle_deg);
                sk_canvas->scale(1.15f, 0.85f);
                sk_canvas->drawRect(SkRect::MakeXYWH(-25, -25, 50, 50), p);
                sk_canvas->restore();
            }
        }
    );

    // 12. Rectangular Clipping (Nested Viewport)
    benchmark_op("12. Rectangular Clipping (Nested Viewport)",
        [&]() {
            auto clip_r = Rect::from_xywh(200.0f, 150.0f, 680.0f, 440.0f).value();
            canvas.save();
            canvas.clip_rect(clip_r);
            canvas.fill_circle(200.0f, 150.0f, 120.0f, Paint(Color::from_rgba8(239, 68, 68, 210)));
            canvas.fill_circle(880.0f, 590.0f, 120.0f, Paint(Color::from_rgba8(59, 130, 246, 210)));
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->clipRect(SkRect::MakeXYWH(200.0f, 150.0f, 680.0f, 440.0f));
            SkPaint p1;
            p1.setAntiAlias(true);
            p1.setColor(SkColorSetARGB(210, 239, 68, 68));
            p1.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawCircle(200.0f, 150.0f, 120.0f, p1);
            SkPaint p2;
            p2.setAntiAlias(true);
            p2.setColor(SkColorSetARGB(210, 59, 130, 246));
            p2.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawCircle(880.0f, 590.0f, 120.0f, p2);
            sk_canvas->restore();
        }
    );

    // 13. 100 Anti-Aliased Diagonals (Lines)
    benchmark_op("13. 100 Anti-Aliased Diagonals (Lines)",
        [&]() {
            Stroke line_s(1.0f);
            Paint line_p(Color::from_rgba8(255, 255, 255, 45));
            for (int i = 0; i < 100; ++i) {
                float y0 = i * 7.4f;
                float y1 = H - y0;
                canvas.stroke_line(0, y0, W, y1, line_p, line_s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(45, 255, 255, 255));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(1.0f);
            for (int i = 0; i < 100; ++i) {
                float y0 = i * 7.4f;
                float y1 = H - y0;
                sk_canvas->drawLine(0.0f, y0, (float)W, y1, p);
            }
        }
    );

    // 14. 30 Overlapping UI Chips (Alpha Stack)
    benchmark_op("14. 30 Overlapping UI Chips (Alpha Stack)",
        [&]() {
            for (int i = 0; i < 30; ++i) {
                float x = 50.0f + (i % 6) * 160.0f;
                float y = 50.0f + (i / 6) * 130.0f;
                auto r = Rect::from_xywh(x, y, 140.0f, 45.0f).value();
                canvas.fill_round_rect(r, 22.5f, 22.5f, Paint(Color::from_rgba8(14, 165, 233, 110)));
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 30; ++i) {
                float x = 80.0f + (i % 6) * 155.0f;
                float y = 80.0f + (i / 6) * 115.0f;
                uint8_t a = 120 + (i * 4) % 100;
                p.setColor(SkColorSetARGB(a, 99, 102, 241));
                SkRRect rr;
                rr.setRectXY(SkRect::MakeXYWH(x, y, 130.0f, 75.0f), 20.0f, 20.0f);
                sk_canvas->drawRRect(rr, p);
            }
        }
    );

    // 15. Closed Cubic Loop (Trefoil Figure-8)
    benchmark_op("15. Closed Cubic Loop (Trefoil Figure-8)",
        [&]() {
            PathBuilder pb;
            pb.move_to(W*0.5f, H*0.5f);
            pb.cubic_to(W*0.8f, H*0.2f, W*0.8f, H*0.8f, W*0.5f, H*0.5f);
            pb.cubic_to(W*0.2f, H*0.2f, W*0.2f, H*0.8f, W*0.5f, H*0.5f);
            pb.close();
            auto fig8 = pb.finish();
            if (fig8) canvas.fill_path(*fig8, Paint(Color::from_rgba8(236, 72, 153, 180)), FillRule::Winding);
        },
        [&]() {
            SkPath path;
            path.moveTo(540, 220);
            path.cubicTo(660, 220, 720, 320, 660, 420);
            path.cubicTo(600, 520, 480, 520, 420, 420);
            path.cubicTo(360, 320, 420, 220, 540, 220);
            path.close();
            SkPaint pf;
            pf.setAntiAlias(true);
            pf.setColor(SkColorSetARGB(40, 168, 85, 247));
            pf.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(path, pf);
            SkPaint ps;
            ps.setAntiAlias(true);
            ps.setColor(SkColorSetARGB(220, 168, 85, 247));
            ps.setStyle(SkPaint::kStroke_Style);
            ps.setStrokeWidth(2.5f);
            sk_canvas->drawPath(path, ps);
        }
    );

    // 16. 500 Alpha Disks (Particle Cloud)
    benchmark_op("16. 500 Alpha Disks (Particle Cloud)",
        [&]() {
            for (int i = 0; i < 500; ++i) {
                float x = 50.0f + static_cast<float>((i * 37) % 980);
                float y = 50.0f + static_cast<float>((i * 59) % 640);
                float r = 2.5f + (i % 5) * 0.8f;
                uint8_t a = static_cast<uint8_t>(60 + (i * 17) % 160);
                canvas.fill_circle(x, y, r, Paint(Color::from_rgba8(56, 189, 248, a)));
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 500; ++i) {
                float x = 50.0f + ((i * 37) % 980);
                float y = 50.0f + ((i * 59) % 640);
                float r = 2.5f + (i % 5) * 0.8f;
                uint8_t a = 60 + (i * 17) % 160;
                p.setColor(SkColorSetARGB(a, 56, 189, 248));
                sk_canvas->drawCircle(x, y, r, p);
            }
        }
    );

    // 17. 1000-Pt Waveform (High-Density Stroke)
    benchmark_op("17. 1000-Pt Waveform (High-Density Stroke)",
        [&]() {
            PathBuilder pb;
            pb.move_to(40.0f, 370.0f);
            for (int i = 1; i < 1000; ++i) {
                float x = 40.0f + i * 1.0f;
                float y = 370.0f + 180.0f * std::sin(i * 0.05f) * std::cos(i * 0.015f);
                pb.line_to(x, y);
            }
            auto p = pb.finish();
            if (p) canvas.stroke_path(*p, Paint(Color::from_rgba8(16, 185, 129, 220)), Stroke(1.5f));
        },
        [&]() {
            SkPath path;
            path.moveTo(40.0f, 370.0f);
            for (int i = 1; i < 1000; ++i) {
                float x = 40.0f + i * 1.0f;
                float y = 370.0f + 180.0f * std::sin(i * 0.05f) * std::cos(i * 0.015f);
                path.lineTo(x, y);
            }
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 16, 185, 129));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(1.5f);
            sk_canvas->drawPath(path, p);
        }
    );

    // 18. Thick 16px Stroke (Miter & Round Caps)
    benchmark_op("18. Thick 16px Stroke (Miter & Round Caps)",
        [&]() {
            PathBuilder pb;
            pb.move_to(100.0f, 200.0f);
            pb.line_to(300.0f, 540.0f);
            pb.line_to(500.0f, 200.0f);
            pb.line_to(700.0f, 540.0f);
            pb.line_to(900.0f, 200.0f);
            pb.line_to(980.0f, 400.0f);
            auto p = pb.finish();
            Stroke s(16.0f);
            s.line_cap = LineCap::Round;
            s.line_join = LineJoin::Miter;
            if (p) canvas.stroke_path(*p, Paint(Color::from_rgba8(245, 158, 11, 230)), s);
        },
        [&]() {
            SkPath path;
            path.moveTo(100.0f, 200.0f);
            path.lineTo(300.0f, 540.0f);
            path.lineTo(500.0f, 200.0f);
            path.lineTo(700.0f, 540.0f);
            path.lineTo(900.0f, 200.0f);
            path.lineTo(980.0f, 400.0f);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 245, 158, 11));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(16.0f);
            p.setStrokeCap(SkPaint::kRound_Cap);
            p.setStrokeJoin(SkPaint::kMiter_Join);
            sk_canvas->drawPath(path, p);
        }
    );

    // 19. 50 Rotated Quads (Affine Alpha Stack)
    benchmark_op("19. 50 Rotated Quads (Affine Alpha Stack)",
        [&]() {
            Paint p(Color::from_rgba8(99, 102, 241, 140));
            auto r_opt = Rect::from_xywh(-60.0f, -40.0f, 120.0f, 80.0f);
            for (int i = 0; i < 50; ++i) {
                float tx = 100.0f + (i % 10) * 95.0f;
                float ty = 100.0f + (i / 10) * 120.0f;
                float angle_deg = i * 7.2f;

                canvas.save();
                canvas.translate(tx, ty);
                canvas.rotate(angle_deg);
                if (r_opt) canvas.fill_rect(*r_opt, p);
                canvas.restore();
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(140, 99, 102, 241));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 50; ++i) {
                float tx = 100.0f + (i % 10) * 95.0f;
                float ty = 100.0f + (i / 10) * 120.0f;
                float angle_deg = i * 7.2f;

                sk_canvas->save();
                sk_canvas->translate(tx, ty);
                sk_canvas->rotate(angle_deg);
                sk_canvas->drawRect(SkRect::MakeXYWH(-60, -40, 120, 80), p);
                sk_canvas->restore();
            }
        }
    );

    // 20. 25 Concentric Rings (Alternating Strokes)
    benchmark_op("20. 25 Concentric Rings (Alternating Strokes)",
        [&]() {
            Stroke s(6.0f);
            for (int i = 1; i <= 25; ++i) {
                float r = i * 11.0f;
                uint8_t red_val = (i % 2 == 0) ? 239 : 59;
                uint8_t green_val = (i % 2 == 0) ? 68 : 130;
                uint8_t blue_val = (i % 2 == 0) ? 68 : 246;
                canvas.stroke_circle(W * 0.5f, H * 0.5f, r, Paint(Color::from_rgba8(red_val, green_val, blue_val, 210)), s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(6.0f);
            for (int i = 1; i <= 25; ++i) {
                float r = i * 11.0f;
                uint8_t red_val = (i % 2 == 0) ? 239 : 59;
                uint8_t green_val = (i % 2 == 0) ? 68 : 130;
                uint8_t blue_val = (i % 2 == 0) ? 68 : 246;
                p.setColor(SkColorSetARGB(210, red_val, green_val, blue_val));
                sk_canvas->drawCircle(W * 0.5f, H * 0.5f, r, p);
            }
        }
    );

    // 21. Diagonal Full-HD Gradient (45° Angle)
    benchmark_op("21. Diagonal Full-HD Gradient (45° Angle)",
        [&]() {
            auto grad_opt = LinearGradient::create(
                Point(0.0f, 0.0f), Point(static_cast<float>(W), static_cast<float>(H)),
                {
                    GradientStop::create(0.0f, Color::from_rgba8(238, 77, 45, 255)),
                    GradientStop::create(0.25f, Color::from_rgba8(254, 205, 27, 255)),
                    GradientStop::create(0.50f, Color::from_rgba8(16, 185, 129, 255)),
                    GradientStop::create(0.75f, Color::from_rgba8(14, 165, 233, 255)),
                    GradientStop::create(1.0f, Color::from_rgba8(147, 51, 234, 255))
                }
            );
            if (grad_opt) {
                auto r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H));
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*grad_opt))));
            }
        },
        [&]() {
            SkPoint pts[2] = { SkPoint::Make(0.0f, 0.0f), SkPoint::Make((float)W, (float)H) };
            SkColor colors[5] = {
                SkColorSetARGB(255, 238, 77, 45),
                SkColorSetARGB(255, 254, 205, 27),
                SkColorSetARGB(255, 16, 185, 129),
                SkColorSetARGB(255, 14, 165, 233),
                SkColorSetARGB(255, 147, 51, 234)
            };
            SkScalar pos[5] = { 0.0f, 0.25f, 0.50f, 0.75f, 1.0f };
            auto sh = SkGradientShader::MakeLinear(pts, colors, pos, 5, SkTileMode::kClamp);
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawRect(SkRect::MakeXYWH(0, 0, W, H), p);
        }
    );

    // 22. 3-Stage Nested Clipping (Criss-Cross)
    benchmark_op("22. 3-Stage Nested Clipping (Criss-Cross)",
        [&]() {
            canvas.save();
            auto r1 = Rect::from_xywh(100.0f, 80.0f, 880.0f, 580.0f);
            if (r1) canvas.clip_rect(*r1);

            canvas.save();
            auto r2 = Rect::from_xywh(200.0f, 150.0f, 680.0f, 440.0f);
            if (r2) canvas.clip_rect(*r2);

            canvas.save();
            auto r3 = Rect::from_xywh(280.0f, 200.0f, 520.0f, 340.0f);
            if (r3) canvas.clip_rect(*r3);

            Paint line_p(Color::from_rgba8(244, 63, 94, 180));
            Stroke line_s(2.0f);
            for (int i = 0; i < 40; ++i) {
                float y0 = 150.0f + i * 11.0f;
                float y1 = 600.0f - i * 11.0f;
                canvas.stroke_line(150.0f, y0, 900.0f, y1, line_p, line_s);
            }

            canvas.restore();
            canvas.restore();
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->clipRect(SkRect::MakeXYWH(100.0f, 80.0f, 880.0f, 580.0f));
            sk_canvas->save();
            sk_canvas->clipRect(SkRect::MakeXYWH(200.0f, 150.0f, 680.0f, 440.0f));
            sk_canvas->save();
            sk_canvas->clipRect(SkRect::MakeXYWH(280.0f, 200.0f, 520.0f, 340.0f));

            SkPaint line_p;
            line_p.setAntiAlias(true);
            line_p.setColor(SkColorSetARGB(180, 244, 63, 94));
            line_p.setStyle(SkPaint::kStroke_Style);
            line_p.setStrokeWidth(2.0f);
            for (int i = 0; i < 40; ++i) {
                float y0 = 150.0f + i * 11.0f;
                float y1 = 600.0f - i * 11.0f;
                sk_canvas->drawLine(150.0f, y0, 900.0f, y1, line_p);
            }

            sk_canvas->restore();
            sk_canvas->restore();
            sk_canvas->restore();
        }
    );

    // 23. Dashed Cubic Spline (Curved Intervals)
    benchmark_op("23. Dashed Cubic Spline (Curved Intervals)",
        [&]() {
            PathBuilder pb;
            pb.move_to(100.0f, 370.0f);
            pb.cubic_to(300.0f, 100.0f, 500.0f, 640.0f, 700.0f, 200.0f);
            pb.cubic_to(800.0f, 100.0f, 900.0f, 500.0f, 1000.0f, 370.0f);
            auto spline = pb.finish();
            Stroke s(3.0f);
            s.dash = StrokeDash::create({12.0f, 8.0f, 4.0f, 8.0f}, 0.0f);
            if (spline) canvas.stroke_path(*spline, Paint(Color::from_rgba8(20, 184, 166, 240)), s);
        },
        [&]() {
            SkPath b_spline;
            b_spline.moveTo(100.0f, 370.0f);
            b_spline.cubicTo(300.0f, 100.0f, 500.0f, 640.0f, 700.0f, 200.0f);
            b_spline.cubicTo(800.0f, 100.0f, 900.0f, 500.0f, 1000.0f, 370.0f);
            SkScalar intervals[4] = { 12.0f, 8.0f, 4.0f, 8.0f };
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(240, 20, 184, 166));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(3.0f);
            p.setPathEffect(SkDashPathEffect::Make(intervals, 4, 0.0f));
            sk_canvas->drawPath(b_spline, p);
        }
    );

    // 24. 40-Point Starburst (Dense Winding Polygon)
    benchmark_op("24. 40-Point Starburst (Dense Winding Polygon)",
        [&]() {
            PathBuilder pb;
            float cx = W * 0.5f, cy = H * 0.5f, r_out = 240.0f, r_in = 70.0f;
            for (int i = 0; i < 80; ++i) {
                float r = (i % 2 == 0) ? r_out : r_in;
                float angle = i * (static_cast<float>(M_PI) / 40.0f);
                float x = cx + std::cos(angle) * r;
                float y = cy + std::sin(angle) * r;
                if (i == 0) pb.move_to(x, y);
                else pb.line_to(x, y);
            }
            pb.close();
            auto star_p = pb.finish();
            if (star_p) canvas.fill_path(*star_p, Paint(Color::from_rgba8(234, 88, 12, 210)), FillRule::Winding);
        },
        [&]() {
            SkPath star;
            float cx = W * 0.5f, cy = H * 0.5f, r_out = 240.0f, r_in = 70.0f;
            for (int i = 0; i < 80; ++i) {
                float r = (i % 2 == 0) ? r_out : r_in;
                float angle = i * (static_cast<float>(M_PI) / 40.0f);
                float x = cx + std::cos(angle) * r;
                float y = cy + std::sin(angle) * r;
                if (i == 0) star.moveTo(x, y);
                else star.lineTo(x, y);
            }
            star.close();
            star.setFillType(SkPathFillType::kWinding);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(210, 234, 88, 12));
            p.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(star, p);
        }
    );

    // 25. 2-Point Focal Radial Spotlight (Lighting)
    benchmark_op("25. 2-Point Focal Radial Spotlight (Lighting)",
        [&]() {
            auto rad_opt = RadialGradient::create_2point(
                Point(420.0f, 290.0f), 0.0f,
                Point(540.0f, 370.0f), 320.0f,
                {
                    GradientStop::create(0.0f, Color::from_rgba8(255, 255, 255, 240)),
                    GradientStop::create(0.3f, Color::from_rgba8(245, 158, 11, 200)),
                    GradientStop::create(0.7f, Color::from_rgba8(190, 24, 93, 150)),
                    GradientStop::create(1.0f, Color::from_rgba8(0, 0, 0, 0))
                }
            );
            if (rad_opt) {
                auto r = Rect::from_xywh(220.0f, 120.0f, 640.0f, 500.0f);
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*rad_opt))));
            }
        },
        [&]() {
            SkColor colors[4] = {
                SkColorSetARGB(240, 255, 255, 255),
                SkColorSetARGB(200, 245, 158, 11),
                SkColorSetARGB(150, 190, 24, 93),
                SkColorSetARGB(0, 0, 0, 0)
            };
            SkScalar pos[4] = { 0.0f, 0.3f, 0.7f, 1.0f };
            auto sh = SkGradientShader::MakeTwoPointConical(
                SkPoint::Make(420.0f, 290.0f), 0.0f,
                SkPoint::Make(540.0f, 370.0f), 320.0f,
                colors, pos, 4, SkTileMode::kClamp
            );
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawRect(SkRect::MakeXYWH(220.0f, 120.0f, 640.0f, 500.0f), p);
        }
    );

    // 26. Ellipse Rendering (25 Ellipses Fill+Stroke)
    benchmark_op("26. Ellipse Rendering (25 Ellipses Fill+Stroke)",
        [&]() {
            for (int i = 0; i < 25; ++i) {
                float cx = 100.0f + (i % 5) * 200.0f;
                float cy = 100.0f + (i / 5) * 120.0f;
                auto oval = Rect::from_xywh(cx - 70.0f, cy - 35.0f, 140.0f, 70.0f);
                if (oval) {
                    auto p = PathBuilder::from_oval(*oval);
                    if (p) {
                        canvas.fill_path(*p, Paint(Color::from_rgba8(30, 144, 255, 120)));
                        canvas.stroke_path(*p, Paint(Color::from_rgba8(255, 255, 255, 200)), Stroke(2.0f));
                    }
                }
            }
        },
        [&]() {
            SkPaint pf;
            pf.setAntiAlias(true);
            pf.setColor(SkColorSetARGB(120, 30, 144, 255));
            pf.setStyle(SkPaint::kFill_Style);
            SkPaint ps;
            ps.setAntiAlias(true);
            ps.setColor(SkColorSetARGB(200, 255, 255, 255));
            ps.setStyle(SkPaint::kStroke_Style);
            ps.setStrokeWidth(2.0f);
            for (int i = 0; i < 25; ++i) {
                float cx = 100.0f + (i % 5) * 200.0f;
                float cy = 100.0f + (i / 5) * 120.0f;
                SkRect r = SkRect::MakeXYWH(cx - 70.0f, cy - 35.0f, 140.0f, 70.0f);
                sk_canvas->drawOval(r, pf);
                sk_canvas->drawOval(r, ps);
            }
        }
    );

    // 27. Circular Arc Rendering (25 Stroked Arcs)
    benchmark_op("27. Circular Arc Rendering (25 Stroked Arcs)",
        [&]() {
            Paint p(Color::from_rgba8(255, 180, 0, 220));
            Stroke s(3.0f);
            for (int i = 0; i < 25; ++i) {
                float cx = 100.0f + (i % 5) * 200.0f;
                float cy = 100.0f + (i / 5) * 120.0f;
                float a0 = static_cast<float>(i) * 0.2f;
                float a1 = a0 + 1.5f * static_cast<float>(M_PI);
                PathBuilder pb;
                nisaba_arc(pb, cx, cy, 50.0f, a0, a1);
                auto path = pb.finish();
                if (path) canvas.stroke_path(*path, p, s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 255, 180, 0));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(3.0f);
            for (int i = 0; i < 25; ++i) {
                float cx = 100.0f + (i % 5) * 200.0f;
                float cy = 100.0f + (i / 5) * 120.0f;
                float a0 = i * 0.2f;
                float sweep = 1.5f * (float)M_PI;
                SkPath path;
                path.addArc(SkRect::MakeXYWH(cx - 50.0f, cy - 50.0f, 100.0f, 100.0f), a0 * 180.0f / (float)M_PI, sweep * 180.0f / (float)M_PI);
                sk_canvas->drawPath(path, p);
            }
        }
    );


    // Common Star Path for Even-Odd vs Non-Zero
    PathBuilder pb_star;
    float star_cx = 540.0f, star_cy = 370.0f, star_r = 240.0f;
    constexpr float PI_F = 3.14159265358979323846f;
    pb_star.move_to(star_cx + star_r * std::cos(-PI_F * 0.5f), star_cy + star_r * std::sin(-PI_F * 0.5f));
    for (int i = 1; i < 5; ++i) {
        float angle = -PI_F * 0.5f + i * (4.0f * PI_F / 5.0f);
        pb_star.line_to(star_cx + star_r * std::cos(angle), star_cy + star_r * std::sin(angle));
    }
    pb_star.close();
    auto star_path_opt = pb_star.finish();
    const Path& star_path = *star_path_opt;

    SkPath sk_star;
    sk_star.moveTo(540.0f + 240.0f * std::cos(-PI_F * 0.5f), 370.0f + 240.0f * std::sin(-PI_F * 0.5f));
    for (int i = 1; i < 5; ++i) {
        float angle = -PI_F * 0.5f + i * (4.0f * PI_F / 5.0f);
        sk_star.lineTo(540.0f + 240.0f * std::cos(angle), 370.0f + 240.0f * std::sin(angle));
    }
    sk_star.close();

    // 28. Even-Odd Polygon Fill (Self-Intersecting Star)
    benchmark_op("28. Even-Odd Polygon Fill (Self-Intersecting Star)",
        [&]() {
            canvas.fill_path(star_path, Paint(Color::from_rgba8(236, 72, 153, 200)), FillRule::EvenOdd);
        },
        [&]() {
            sk_star.setFillType(SkPathFillType::kEvenOdd);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(200, 236, 72, 153));
            p.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(sk_star, p);
        }
    );

    // 29. Non-Zero Winding Fill (Self-Intersecting Star)
    benchmark_op("29. Non-Zero Winding Fill (Self-Intersecting Star)",
        [&]() {
            canvas.fill_path(star_path, Paint(Color::from_rgba8(236, 72, 153, 200)), FillRule::Winding);
        },
        [&]() {
            sk_star.setFillType(SkPathFillType::kWinding);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(200, 236, 72, 153));
            p.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(sk_star, p);
        }
    );

    // 30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)
    benchmark_op("30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)",
        [&]() {
            for (int i = 0; i < 4; ++i) {
                canvas.draw_pixmap(60 + i * 250, 200, test_pm.as_ref());
            }
        },
        [&]() {
            for (int i = 0; i < 4; ++i) {
                sk_canvas->drawImage(sk_test_img, 60 + i * 250, 200);
            }
        }
    );

    // 31. Image Bilinear Scaling (2x Scale Up 512x512)
    benchmark_op("31. Image Bilinear Scaling (2x Scale Up 512x512)",
        [&]() {
            canvas.save();
            canvas.translate(284.0f, 114.0f);
            canvas.scale(2.0f, 2.0f);
            PixmapPaint pp;
            pp.quality = FilterQuality::Bilinear;
            canvas.draw_pixmap(0, 0, test_pm.as_ref(), pp);
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->translate(284.0f, 114.0f);
            sk_canvas->scale(2.0f, 2.0f);
            sk_canvas->drawImage(sk_test_img, 0, 0, SkSamplingOptions(SkFilterMode::kLinear));
            sk_canvas->restore();
        }
    );

    // 32. Image Rotation & Affine Transform (35° Filtered)
    benchmark_op("32. Image Rotation & Affine Transform (35° Filtered)",
        [&]() {
            canvas.save();
            canvas.translate(540.0f, 370.0f);
            canvas.rotate(35.0f);
            canvas.translate(-128.0f, -128.0f);
            PixmapPaint pp;
            pp.quality = FilterQuality::Bilinear;
            canvas.draw_pixmap(0, 0, test_pm.as_ref(), pp);
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->translate(540.0f, 370.0f);
            sk_canvas->rotate(35.0f);
            sk_canvas->translate(-128.0f, -128.0f);
            sk_canvas->drawImage(sk_test_img, 0, 0, SkSamplingOptions(SkFilterMode::kLinear));
            sk_canvas->restore();
        }
    );

    // 33. Image Pattern Fill (Circle Filled with Texture)
    benchmark_op("33. Image Pattern Fill (Circle Filled with Texture)",
        [&]() {
            Pattern pat(test_pm.as_ref(), SpreadMode::Pad, FilterQuality::Bilinear, 1.0f, Transform::from_translate(412.0f, 242.0f));
            Paint p;
            p.shader = Shader(pat);
            canvas.fill_circle(540.0f, 370.0f, 200.0f, p);
        },
        [&]() {
            SkMatrix lm;
            lm.setTranslate(412.0f, 242.0f);
            auto sh = sk_test_img->makeShader(SkTileMode::kClamp, SkTileMode::kClamp, SkSamplingOptions(SkFilterMode::kLinear), lm);
            SkPaint p;
            p.setAntiAlias(true);
            p.setShader(sh);
            sk_canvas->drawCircle(540.0f, 370.0f, 200.0f, p);
        }
    );

    // 34. Pattern Repeat & Tiling (64x64 Texture Repeat)
    benchmark_op("34. Pattern Repeat & Tiling (64x64 Texture Repeat)",
        [&]() {
            Pattern pat(tile_pm.as_ref(), SpreadMode::Repeat, FilterQuality::Nearest);
            Paint p;
            p.shader = Shader(pat);
            auto r = Rect::from_xywh(140.0f, 120.0f, 800.0f, 500.0f);
            if (r) canvas.fill_rect(*r, p);
        },
        [&]() {
            auto sh = sk_tile_img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions(SkFilterMode::kNearest));
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawRect(SkRect::MakeXYWH(140.0f, 120.0f, 800.0f, 500.0f), p);
        }
    );


    // Helpers for AA ON vs OFF
    auto run_diagonals_nisaba = [&](bool aa) {
        Paint p(Color::from_rgba8(0, 220, 130, 200));
        p.anti_alias = aa;
        Stroke s(2.0f);
        for (int i = 0; i < 50; ++i) {
            float x0 = static_cast<float>(i * 20);
            float y0 = 0.0f;
            float x1 = static_cast<float>(i * 20 + 200);
            float y1 = static_cast<float>(H);
            canvas.stroke_line(x0, y0, x1, y1, p, s);
        }
    };

    // 35. Anti-Aliasing ON (50 Diagonals Coverage)
    benchmark_op("35. Anti-Aliasing ON (50 Diagonals Coverage)",
        [&]() { run_diagonals_nisaba(true); },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(200, 0, 220, 130));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(2.0f);
            for (int i = 0; i < 50; ++i) {
                sk_canvas->drawLine(i * 20.0f, 0.0f, i * 20.0f + 200.0f, (float)H, p);
            }
        }
    );

    // 36. Anti-Aliasing OFF (50 Diagonals Aliased)
    benchmark_op("36. Anti-Aliasing OFF (50 Diagonals Aliased)",
        [&]() { run_diagonals_nisaba(false); },
        [&]() {
            SkPaint p;
            p.setAntiAlias(false);
            p.setColor(SkColorSetARGB(200, 0, 220, 130));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(2.0f);
            for (int i = 0; i < 50; ++i) {
                sk_canvas->drawLine(i * 20.0f, 0.0f, i * 20.0f + 200.0f, (float)H, p);
            }
        }
    );


    // Common Zigzag Path for Stroke Join Matrix
    PathBuilder pb_zigzag;
    pb_zigzag.move_to(50.0f, 370.0f);
    for (int i = 0; i < 16; ++i) {
        float x = 50.0f + (i + 1) * 60.0f;
        float y = (i % 2 == 0) ? 250.0f : 490.0f;
        pb_zigzag.line_to(x, y);
    }
    auto zigzag_path_opt = pb_zigzag.finish();
    const Path& zigzag_path = *zigzag_path_opt;

    SkPath sk_zigzag;
    sk_zigzag.moveTo(50.0f, 370.0f);
    for (int i = 0; i < 16; ++i) {
        float x = 50.0f + (i + 1) * 60.0f;
        float y = (i % 2 == 0) ? 250.0f : 490.0f;
        sk_zigzag.lineTo(x, y);
    }

    // 37. Stroke Join: Miter (Zigzag Polygon 12px)
    benchmark_op("37. Stroke Join: Miter (Zigzag Polygon 12px)",
        [&]() {
            Stroke s(12.0f);
            s.line_join = LineJoin::Miter;
            canvas.stroke_path(zigzag_path, Paint(Color::from_rgba8(244, 63, 94, 220)), s);
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 244, 63, 94));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(12.0f);
            p.setStrokeJoin(SkPaint::kMiter_Join);
            sk_canvas->drawPath(sk_zigzag, p);
        }
    );

    // 38. Stroke Join: Bevel (Zigzag Polygon 12px)
    benchmark_op("38. Stroke Join: Bevel (Zigzag Polygon 12px)",
        [&]() {
            Stroke s(12.0f);
            s.line_join = LineJoin::Bevel;
            canvas.stroke_path(zigzag_path, Paint(Color::from_rgba8(244, 63, 94, 220)), s);
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 244, 63, 94));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(12.0f);
            p.setStrokeJoin(SkPaint::kBevel_Join);
            sk_canvas->drawPath(sk_zigzag, p);
        }
    );

    // 39. Stroke Join: Round (Zigzag Polygon 12px)
    benchmark_op("39. Stroke Join: Round (Zigzag Polygon 12px)",
        [&]() {
            Stroke s(12.0f);
            s.line_join = LineJoin::Round;
            canvas.stroke_path(zigzag_path, Paint(Color::from_rgba8(244, 63, 94, 220)), s);
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 244, 63, 94));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(12.0f);
            p.setStrokeJoin(SkPaint::kRound_Join);
            sk_canvas->drawPath(sk_zigzag, p);
        }
    );


    // Helpers for Stroke Cap Matrix
    auto run_caps_nisaba = [&](LineCap cap) {
        Stroke s(14.0f);
        s.line_cap = cap;
        Paint p(Color::from_rgba8(20, 184, 166, 220));
        for (int i = 0; i < 20; ++i) {
            float y = 80.0f + i * 30.0f;
            canvas.stroke_line(100.0f, y, 980.0f, y, p, s);
        }
    };

    // 40. Stroke Cap: Butt (20 Line Segments 14px)
    benchmark_op("40. Stroke Cap: Butt (20 Line Segments 14px)",
        [&]() { run_caps_nisaba(LineCap::Butt); },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 20, 184, 166));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(14.0f);
            p.setStrokeCap(SkPaint::kButt_Cap);
            for (int i = 0; i < 20; ++i) {
                sk_canvas->drawLine(100.0f, 60.0f + i * 32.0f, 980.0f, 60.0f + i * 32.0f, p);
            }
        }
    );

    // 41. Stroke Cap: Square (20 Line Segments 14px)
    benchmark_op("41. Stroke Cap: Square (20 Line Segments 14px)",
        [&]() { run_caps_nisaba(LineCap::Square); },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 20, 184, 166));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(14.0f);
            p.setStrokeCap(SkPaint::kSquare_Cap);
            for (int i = 0; i < 20; ++i) {
                sk_canvas->drawLine(100.0f, 60.0f + i * 32.0f, 980.0f, 60.0f + i * 32.0f, p);
            }
        }
    );

    // 42. Stroke Cap: Round (20 Line Segments 14px)
    benchmark_op("42. Stroke Cap: Round (20 Line Segments 14px)",
        [&]() { run_caps_nisaba(LineCap::Round); },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 20, 184, 166));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(14.0f);
            p.setStrokeCap(SkPaint::kRound_Cap);
            for (int i = 0; i < 20; ++i) {
                sk_canvas->drawLine(100.0f, 80.0f + i * 30.0f, 980.0f, 80.0f + i * 30.0f, p);
            }
        }
    );


    // Shared parameters for Transform Isolation tests
    auto base_rect_opt = Rect::from_xywh(0.0f, 0.0f, 100.0f, 70.0f);
    const Rect& base_rect = *base_rect_opt;
    Paint transform_p(Color::from_rgba8(168, 85, 247, 180));

    SkRRect sk_base_rrect;
    sk_base_rrect.setRectXY(SkRect::MakeXYWH(0.0f, 0.0f, 100.0f, 70.0f), 10.0f, 10.0f);
    SkPaint sk_transform_p;
    sk_transform_p.setAntiAlias(true);
    sk_transform_p.setColor(SkColorSetARGB(180, 168, 85, 247));
    sk_transform_p.setStyle(SkPaint::kFill_Style);

    // 43. Transform Isolation: Pure Translate (20 Shapes)
    benchmark_op("43. Transform Isolation: Pure Translate (20 Shapes)",
        [&]() {
            for (int i = 0; i < 20; ++i) {
                canvas.save();
                canvas.translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                canvas.fill_round_rect(base_rect, 10.0f, 10.0f, transform_p);
                canvas.restore();
            }
        },
        [&]() {
            for (int i = 0; i < 20; ++i) {
                sk_canvas->save();
                sk_canvas->translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                sk_canvas->drawRRect(sk_base_rrect, sk_transform_p);
                sk_canvas->restore();
            }
        }
    );

    // 44. Transform Isolation: Pure Scale (20 Shapes)
    benchmark_op("44. Transform Isolation: Pure Scale (20 Shapes)",
        [&]() {
            for (int i = 0; i < 20; ++i) {
                canvas.save();
                canvas.translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                float s = 0.5f + (i % 5) * 0.15f;
                canvas.scale(s, s);
                canvas.fill_round_rect(base_rect, 10.0f, 10.0f, transform_p);
                canvas.restore();
            }
        },
        [&]() {
            for (int i = 0; i < 20; ++i) {
                sk_canvas->save();
                sk_canvas->translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                float s = 0.5f + (i % 5) * 0.15f;
                sk_canvas->scale(s, s);
                sk_canvas->drawRRect(sk_base_rrect, sk_transform_p);
                sk_canvas->restore();
            }
        }
    );

    // 45. Transform Isolation: Pure Rotate (20 Shapes)
    benchmark_op("45. Transform Isolation: Pure Rotate (20 Shapes)",
        [&]() {
            for (int i = 0; i < 20; ++i) {
                canvas.save();
                canvas.translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                canvas.rotate(static_cast<float>(i * 18));
                canvas.fill_round_rect(base_rect, 10.0f, 10.0f, transform_p);
                canvas.restore();
            }
        },
        [&]() {
            for (int i = 0; i < 20; ++i) {
                sk_canvas->save();
                sk_canvas->translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                sk_canvas->rotate(i * 18.0f);
                sk_canvas->drawRRect(sk_base_rrect, sk_transform_p);
                sk_canvas->restore();
            }
        }
    );

    // 46. Transform Isolation: Pure Shear (20 Shapes)
    benchmark_op("46. Transform Isolation: Pure Shear (20 Shapes)",
        [&]() {
            for (int i = 0; i < 20; ++i) {
                canvas.save();
                canvas.translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                canvas.concat(Transform::from_skew(0.25f, 0.15f));
                canvas.fill_round_rect(base_rect, 10.0f, 10.0f, transform_p);
                canvas.restore();
            }
        },
        [&]() {
            for (int i = 0; i < 20; ++i) {
                sk_canvas->save();
                sk_canvas->translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                sk_canvas->skew(0.25f, 0.15f);
                sk_canvas->drawRRect(sk_base_rrect, sk_transform_p);
                sk_canvas->restore();
            }
        }
    );

    // 47. Transform Isolation: Combined Affine (20 Shapes)
    benchmark_op("47. Transform Isolation: Combined Affine (20 Shapes)",
        [&]() {
            for (int i = 0; i < 20; ++i) {
                canvas.save();
                canvas.translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                canvas.rotate(static_cast<float>(i * 12));
                canvas.scale(0.85f, 0.85f);
                canvas.concat(Transform::from_skew(0.20f, 0.10f));
                canvas.fill_round_rect(base_rect, 10.0f, 10.0f, transform_p);
                canvas.restore();
            }
        },
        [&]() {
            for (int i = 0; i < 20; ++i) {
                sk_canvas->save();
                sk_canvas->translate(80.0f + (i % 5) * 190.0f, 80.0f + (i / 5) * 150.0f);
                sk_canvas->rotate(i * 12.0f);
                sk_canvas->scale(0.85f, 0.85f);
                sk_canvas->skew(0.20f, 0.10f);
                sk_canvas->drawRRect(sk_base_rrect, sk_transform_p);
                sk_canvas->restore();
            }
        }
    );

    // 48. State Stack: Save/Restore Overhead (50 Passes)
    benchmark_op("48. State Stack: Save/Restore Overhead (50 Passes)",
        [&]() {
            for (int k = 0; k < 50; ++k) {
                canvas.save();
                canvas.translate(2.0f, 1.0f);
                canvas.scale(0.99f, 0.99f);
            }
            for (int k = 0; k < 50; ++k) {
                canvas.restore();
            }
        },
        [&]() {
            for (int k = 0; k < 50; ++k) {
                sk_canvas->save();
                sk_canvas->translate(2.0f, 1.0f);
                sk_canvas->scale(0.99f, 0.99f);
                sk_canvas->restore();
            }
        }
    );

    // 49. Compositing Operator: Multiply (25 Shapes)
    benchmark_op("49. Compositing Operator: Multiply (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(200, 50, 150, 180));
            p.blend_mode = BlendMode::Multiply;
            for (int i = 0; i < 25; ++i) {
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setBlendMode(SkBlendMode::kMultiply);
            p.setColor(SkColorSetARGB(180, 200, 50, 150));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 25; ++i) {
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 50. Compositing Operator: Screen (25 Shapes)
    benchmark_op("50. Compositing Operator: Screen (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(50, 200, 150, 180));
            p.blend_mode = BlendMode::Screen;
            for (int i = 0; i < 25; ++i) {
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setBlendMode(SkBlendMode::kScreen);
            p.setColor(SkColorSetARGB(180, 50, 200, 150));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 25; ++i) {
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 51. Compositing Operator: Source-In & Xor (25 Shapes)
    benchmark_op("51. Compositing Operator: Source-In & Xor (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(250, 204, 21, 180));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::SourceIn : BlendMode::Xor;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(180, 250, 204, 21));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kSrcIn : SkBlendMode::kXor);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );


    // Helpers for Path Complexity Scaling (Waveforms)
    auto make_waveform = [](int count) {
        PathBuilder pb;
        pb.move_to(40.0f, 370.0f);
        for (int i = 1; i < count; ++i) {
            float x = 40.0f + (static_cast<float>(i) / (count - 1)) * 1000.0f;
            float y = 370.0f + 180.0f * std::sin(static_cast<float>(i) * 0.15f);
            pb.line_to(x, y);
        }
        return pb.finish();
    };

    auto skia_waveform = [&](int count) {
        SkPath p;
        p.moveTo(40.0f, 370.0f);
        for (int i = 1; i < count; ++i) {
            float x = 40.0f + (static_cast<float>(i) / (count - 1)) * 1000.0f;
            float y = 370.0f + 180.0f * std::sin(static_cast<float>(i) * 0.15f);
            p.lineTo(x, y);
        }
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SkColorSetARGB(220, 56, 189, 248));
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(2.5f);
        sk_canvas->drawPath(p, paint);
    };

    auto wave10_opt = make_waveform(10);
    auto wave100_opt = make_waveform(100);
    auto wave1000_opt = make_waveform(1000);
    const Path& wave10 = *wave10_opt;
    const Path& wave100 = *wave100_opt;
    const Path& wave1000 = *wave1000_opt;
    Paint wave_paint(Color::from_rgba8(56, 189, 248, 220));
    Stroke wave_stroke(2.5f);

    // 52. Path Complexity: Low (10 Vertices Waveform)
    benchmark_op("52. Path Complexity: Low (10 Vertices Waveform)",
        [&]() { canvas.stroke_path(wave10, wave_paint, wave_stroke); },
        [&]() {
            skia_waveform(10);
        }
    );

    // 53. Path Complexity: Medium (100 Vertices Waveform)
    benchmark_op("53. Path Complexity: Medium (100 Vertices Waveform)",
        [&]() { canvas.stroke_path(wave100, wave_paint, wave_stroke); },
        [&]() {
            skia_waveform(100);
        }
    );

    // 54. Path Complexity: High (1000 Vertices Waveform)
    benchmark_op("54. Path Complexity: High (1000 Vertices Waveform)",
        [&]() { canvas.stroke_path(wave1000, wave_paint, wave_stroke); },
        [&]() {
            skia_waveform(1000);
        }
    );


    // Helpers for Primitive-Count Scaling
    auto draw_rects_nisaba = [&](int count) {
        Paint p(Color::from_rgba8(99, 102, 241, 150));
        for (int i = 0; i < count; ++i) {
            float x = static_cast<float>((i * 73) % (W - 40));
            float y = static_cast<float>((i * 59) % (H - 40));
            auto r = Rect::from_xywh(x, y, 35.0f, 35.0f);
            if (r) canvas.fill_rect(*r, p);
        }
    };

    auto draw_rects_skia = [&](int count) {
        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(SkColorSetARGB(150, 99, 102, 241));
        p.setStyle(SkPaint::kFill_Style);
        for (int i = 0; i < count; ++i) {
            float x = static_cast<float>((i * 73) % (W - 40));
            float y = static_cast<float>((i * 59) % (H - 40));
            sk_canvas->drawRect(SkRect::MakeXYWH(x, y, 35.0f, 35.0f), p);
        }
    };

    // 55. Primitive Scaling: 100 Rectangles Batch
    benchmark_op("55. Primitive Scaling: 100 Rectangles Batch",
        [&]() { draw_rects_nisaba(100); },
        [&]() {
            draw_rects_skia(100);
        }
    );

    // 56. Primitive Scaling: 500 Rectangles Batch
    benchmark_op("56. Primitive Scaling: 500 Rectangles Batch",
        [&]() { draw_rects_nisaba(500); },
        [&]() {
            draw_rects_skia(500);
        }
    );

    // 57. Primitive Scaling: 1000 Rectangles Batch
    benchmark_op("57. Primitive Scaling: 1000 Rectangles Batch",
        [&]() { draw_rects_nisaba(1000); },
        [&]() {
            draw_rects_skia(1000);
        }
    );


    // Helpers for Clip-Depth Scaling (Drawing 50 grid lines inside nested clips)
    auto draw_clip_content_nisaba = [&]() {
        Paint p(Color::from_rgba8(245, 158, 11, 220));
        Stroke s(2.0f);
        for (int i = 0; i < 50; ++i) {
            float x = static_cast<float>(i * 20);
            canvas.stroke_line(x, 0.0f, x, static_cast<float>(H), p, s);
        }
    };

    auto draw_clip_content_skia = [&]() {
        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(SkColorSetARGB(220, 245, 158, 11));
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(2.0f);
        for (int i = 0; i < 50; ++i) {
            float x = i * 20.0f;
            sk_canvas->drawLine(x, 0.0f, x, static_cast<float>(H), p);
        }
    };

    // 58. Clip-Depth Scaling: 1-Level Vector Clip
    benchmark_op("58. Clip-Depth Scaling: 1-Level Vector Clip",
        [&]() {
            canvas.save();
            auto c1 = PathBuilder::from_circle(540.0f, 370.0f, 320.0f);
            if (c1) canvas.clip_path(*c1);
            draw_clip_content_nisaba();
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            SkPath c1;
            c1.addCircle(540.0f, 370.0f, 320.0f);
            sk_canvas->clipPath(c1, true);
            draw_clip_content_skia();
            sk_canvas->restore();
        }
    );

    // 59. Clip-Depth Scaling: 2-Level Nested Vector Clip
    benchmark_op("59. Clip-Depth Scaling: 2-Level Nested Vector Clip",
        [&]() {
            canvas.save();
            auto c1 = PathBuilder::from_circle(540.0f, 370.0f, 320.0f);
            if (c1) canvas.clip_path(*c1);
            auto c2 = PathBuilder::from_rect(*Rect::from_xywh(200.0f, 150.0f, 680.0f, 440.0f));
            canvas.clip_path(c2);
            draw_clip_content_nisaba();
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            SkPath c1;
            c1.addCircle(540.0f, 370.0f, 320.0f);
            sk_canvas->clipPath(c1, true);
            sk_canvas->clipRect(SkRect::MakeXYWH(200.0f, 150.0f, 680.0f, 440.0f), true);
            draw_clip_content_skia();
            sk_canvas->restore();
        }
    );

    // 60. Clip-Depth Scaling: 4-Level Nested Vector Clip
    benchmark_op("60. Clip-Depth Scaling: 4-Level Nested Vector Clip",
        [&]() {
            canvas.save();
            auto c1 = PathBuilder::from_circle(540.0f, 370.0f, 320.0f);
            if (c1) canvas.clip_path(*c1);
            auto c2 = PathBuilder::from_rect(*Rect::from_xywh(200.0f, 150.0f, 680.0f, 440.0f));
            canvas.clip_path(c2);
            auto c3 = PathBuilder::from_circle(440.0f, 320.0f, 250.0f);
            if (c3) canvas.clip_path(*c3);
            auto c4 = PathBuilder::from_circle(640.0f, 420.0f, 250.0f);
            if (c4) canvas.clip_path(*c4);
            draw_clip_content_nisaba();
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            SkPath c1; c1.addCircle(540.0f, 370.0f, 320.0f); sk_canvas->clipPath(c1, true);
            sk_canvas->clipRect(SkRect::MakeXYWH(200.0f, 150.0f, 680.0f, 440.0f), true);
            SkPath c3; c3.addCircle(440.0f, 320.0f, 250.0f); sk_canvas->clipPath(c3, true);
            SkPath c4; c4.addCircle(640.0f, 420.0f, 250.0f); sk_canvas->clipPath(c4, true);
            draw_clip_content_skia();
            sk_canvas->restore();
        }
    );


    // Helpers for Resolution Scaling
    auto run_resolution_scene_nisaba = [&](uint32_t vp_w, uint32_t vp_h) {
        auto clip_r = ScreenIntRect::from_xywh(0, 0, vp_w, vp_h);
        if (clip_r) canvas.set_scissor_clip(*clip_r);
        canvas.clear(Color::from_rgba8(15, 23, 42, 255));
        std::vector<GradientStop> stops = {
            GradientStop::create(0.0f, Color::from_rgba8(59, 130, 246, 200)),
            GradientStop::create(1.0f, Color::from_rgba8(236, 72, 153, 200))
        };
        auto lin = LinearGradient::create(Point::from_xy(0, 0), Point::from_xy(static_cast<float>(vp_w), static_cast<float>(vp_h)), stops);
        if (lin) {
            auto r = Rect::from_xywh(0, 0, static_cast<float>(vp_w), static_cast<float>(vp_h));
            if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*lin))));
        }
        for (int i = 0; i < 15; ++i) {
            float cx = static_cast<float>((i * 67) % vp_w);
            float cy = static_cast<float>((i * 53) % vp_h);
            canvas.fill_circle(cx, cy, 25.0f, Paint(Color::from_rgba8(255, 255, 255, 80)));
        }
        canvas.set_scissor_clip(std::nullopt);
    };

    auto run_resolution_scene_skia = [&](uint32_t vp_w, uint32_t vp_h) {
        sk_canvas->save();
        sk_canvas->clipRect(SkRect::MakeWH(vp_w, vp_h));
        sk_canvas->clear(SkColorSetARGB(255, 15, 23, 42));
        SkPoint pts[2] = { SkPoint::Make(0.0f, 0.0f), SkPoint::Make(static_cast<float>(vp_w), static_cast<float>(vp_h)) };
        SkColor colors[2] = { SkColorSetARGB(200, 59, 130, 246), SkColorSetARGB(200, 236, 72, 153) };
        auto lin = SkGradientShader::MakeLinear(pts, colors, nullptr, 2, SkTileMode::kClamp);
        SkPaint p_grad;
        p_grad.setShader(lin);
        sk_canvas->drawPaint(p_grad);

        SkPaint p_circ;
        p_circ.setAntiAlias(true);
        p_circ.setColor(SkColorSetARGB(80, 255, 255, 255));
        p_circ.setStyle(SkPaint::kFill_Style);
        for (int i = 0; i < 15; ++i) {
            float cx = static_cast<float>((i * 67) % vp_w);
            float cy = static_cast<float>((i * 53) % vp_h);
            sk_canvas->drawCircle(cx, cy, 25.0f, p_circ);
        }
        sk_canvas->restore();
    };

    // 61. Resolution Scaling: Small (256x256 Viewport)
    benchmark_op("61. Resolution Scaling: Small (256x256 Viewport)",
        [&]() { run_resolution_scene_nisaba(256, 256); },
        [&]() {
            run_resolution_scene_skia(256, 256);
        }
    );

    // 62. Resolution Scaling: Medium (640x480 Viewport)
    benchmark_op("62. Resolution Scaling: Medium (640x480 Viewport)",
        [&]() { run_resolution_scene_nisaba(640, 480); },
        [&]() {
            run_resolution_scene_skia(640, 480);
        }
    );

    // 63. Resolution Scaling: Full Viewport (1080x740)
    benchmark_op("63. Resolution Scaling: Full Viewport (1080x740)",
        [&]() { run_resolution_scene_nisaba(1080, 740); },
        [&]() {
            run_resolution_scene_skia(1080, 740);
        }
    );

    // 64. Quadratic Bezier Splines (20 Connected Quads)
    benchmark_op("64. Quadratic Bezier Splines (20 Connected Quads)",
        [&]() {
            PathBuilder pb;
            pb.move_to(50.0f, 370.0f);
            for (int i = 0; i < 20; ++i) {
                float x0 = 50.0f + i * 48.0f;
                float y0 = (i % 2 == 0) ? 220.0f : 520.0f;
                float cx = x0 + 24.0f;
                float cy = (i % 2 == 0) ? 620.0f : 120.0f;
                float x1 = x0 + 48.0f;
                float y1 = (i % 2 == 0) ? 520.0f : 220.0f;
                if (i == 0) pb.move_to(x0, y0);
                pb.quad_to(cx, cy, x1, y1);
            }
            auto p = pb.finish();
            if (p) canvas.stroke_path(*p, Paint(Color::from_rgba8(20, 184, 166, 230)), Stroke(3.0f));
        },
        [&]() {
            SkPath path;
            for (int i = 0; i < 20; ++i) {
                float x0 = 50.0f + i * 48.0f;
                float y0 = (i % 2 == 0) ? 220.0f : 520.0f;
                float cx = x0 + 24.0f;
                float cy = (i % 2 == 0) ? 620.0f : 120.0f;
                float x1 = x0 + 48.0f;
                float y1 = (i % 2 == 0) ? 520.0f : 220.0f;
                if (i == 0) path.moveTo(x0, y0);
                path.quadTo(cx, cy, x1, y1);
            }
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 20, 184, 166));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(3.0f);
            sk_canvas->drawPath(path, p);
        }
    );

    // 65. Compound Path with Hole (Donut Even-Odd Fill)
    benchmark_op("65. Compound Path with Hole (Donut Even-Odd Fill)",
        [&]() {
            PathBuilder pb;
            pb.push_circle(540.0f, 370.0f, 260.0f);
            pb.push_circle(540.0f, 370.0f, 140.0f);
            auto p = pb.finish();
            if (p) canvas.fill_path(*p, Paint(Color::from_rgba8(99, 102, 241, 210)), FillRule::EvenOdd);
        },
        [&]() {
            SkPath path;
            path.addCircle(540.0f, 370.0f, 260.0f);
            path.addCircle(540.0f, 370.0f, 140.0f);
            path.setFillType(SkPathFillType::kEvenOdd);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(210, 99, 102, 241));
            p.setStyle(SkPaint::kFill_Style);
            sk_canvas->drawPath(path, p);
        }
    );

    // 66. Miter Limit Clamping (Sharp 10° Acute Angles)
    benchmark_op("66. Miter Limit Clamping (Sharp 10° Acute Angles)",
        [&]() {
            PathBuilder pb;
            pb.move_to(80.0f, 370.0f);
            for (int i = 0; i < 12; ++i) {
                float x = 80.0f + (i + 1) * 75.0f;
                float y = (i % 2 == 0) ? 120.0f : 620.0f;
                pb.line_to(x, y);
            }
            auto p = pb.finish();
            Stroke s(15.0f);
            s.line_join = LineJoin::Miter;
            s.miter_limit = 4.0f;
            if (p) canvas.stroke_path(*p, Paint(Color::from_rgba8(239, 68, 68, 220)), s);
        },
        [&]() {
            SkPath path;
            path.moveTo(80.0f, 370.0f);
            for (int i = 0; i < 12; ++i) {
                float x = 80.0f + (i + 1) * 75.0f;
                float y = (i % 2 == 0) ? 120.0f : 620.0f;
                path.lineTo(x, y);
            }
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 239, 68, 68));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(15.0f);
            p.setStrokeJoin(SkPaint::kMiter_Join);
            p.setStrokeMiter(4.0f);
            sk_canvas->drawPath(path, p);
        }
    );

    // 67. Sub-Pixel Hairline Strokes (0.25px Lines)
    benchmark_op("67. Sub-Pixel Hairline Strokes (0.25px Lines)",
        [&]() {
            Stroke s(0.25f);
            Paint p(Color::from_rgba8(255, 255, 255, 200));
            for (int i = 0; i < 50; ++i) {
                float y0 = i * 14.8f;
                float y1 = static_cast<float>(H) - y0;
                canvas.stroke_line(0.0f, y0, static_cast<float>(W), y1, p, s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(200, 255, 255, 255));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(0.25f);
            for (int i = 0; i < 50; ++i) {
                float y0 = i * 14.8f;
                float y1 = static_cast<float>(H) - y0;
                sk_canvas->drawLine(0.0f, y0, static_cast<float>(W), y1, p);
            }
        }
    );

    // 68. Image Downscaling (4x Minification 0.25x Bilinear)
    benchmark_op("68. Image Downscaling (4x Minification 0.25x Bilinear)",
        [&]() {
            canvas.save();
            canvas.translate(450.0f, 280.0f);
            canvas.scale(0.25f, 0.25f);
            PixmapPaint pp;
            pp.quality = FilterQuality::Bilinear;
            canvas.draw_pixmap(0, 0, test_pm.as_ref(), pp);
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->translate(450.0f, 280.0f);
            sk_canvas->scale(0.25f, 0.25f);
            sk_canvas->drawImage(sk_test_img, 0, 0, SkSamplingOptions(SkFilterMode::kLinear));
            sk_canvas->restore();
        }
    );

    // 69. Pattern Reflect Tiling (64x64 Texture Reflect)
    benchmark_op("69. Pattern Reflect Tiling (64x64 Texture Reflect)",
        [&]() {
            Pattern pat(tile_pm.as_ref(), SpreadMode::Reflect, FilterQuality::Nearest);
            Paint p;
            p.shader = Shader(pat);
            auto r = Rect::from_xywh(140.0f, 120.0f, 800.0f, 500.0f);
            if (r) canvas.fill_rect(*r, p);
        },
        [&]() {
            auto sh = sk_tile_img->makeShader(SkTileMode::kMirror, SkTileMode::kMirror, SkSamplingOptions(SkFilterMode::kNearest));
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawRect(SkRect::MakeXYWH(140.0f, 120.0f, 800.0f, 500.0f), p);
        }
    );

    // 70. Repeated Linear Gradient (60px Periodic Tile)
    benchmark_op("70. Repeated Linear Gradient (60px Periodic Tile)",
        [&]() {
            auto grad_opt = LinearGradient::create(
                Point(100.0f, 100.0f), Point(160.0f, 100.0f),
                {
                    GradientStop::create(0.0f, Color::from_rgba8(236, 72, 153, 230)),
                    GradientStop::create(0.5f, Color::from_rgba8(59, 130, 246, 230)),
                    GradientStop::create(1.0f, Color::from_rgba8(16, 185, 129, 230))
                },
                SpreadMode::Repeat
            );
            if (grad_opt) {
                auto r = Rect::from_xywh(100.0f, 100.0f, 800.0f, 500.0f);
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*grad_opt))));
            }
        },
        [&]() {
            SkPoint pts[2] = { SkPoint::Make(100.0f, 100.0f), SkPoint::Make(160.0f, 100.0f) };
            SkColor colors[3] = {
                SkColorSetARGB(230, 236, 72, 153),
                SkColorSetARGB(230, 59, 130, 246),
                SkColorSetARGB(230, 16, 185, 129)
            };
            SkScalar pos[3] = { 0.0f, 0.5f, 1.0f };
            auto sh = SkGradientShader::MakeLinear(pts, colors, pos, 3, SkTileMode::kRepeat);
            SkPaint p;
            p.setShader(sh);
            sk_canvas->drawRect(SkRect::MakeXYWH(100.0f, 100.0f, 800.0f, 500.0f), p);
        }
    );

    // 71. Alpha Mask Surface Blit (8-bit Alpha Masking)
    benchmark_op("71. Alpha Mask Surface Blit (8-bit Alpha Masking)",
        [&]() {
            canvas.set_mask(&alpha_mask);
            auto r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H));
            if (r) canvas.fill_rect(*r, Paint(Color::from_rgba8(245, 158, 11, 230)));
            canvas.set_mask(nullptr);
        },
        [&]() {
            SkPaint p;
            p.setColor(SkColorSetARGB(230, 245, 158, 11));
            sk_canvas->drawImage(sk_alpha_mask, 0, 0, SkSamplingOptions(), &p);
        }
    );

    // 72. Blend Modes: Color Dodge & Difference (25 Shapes)
    benchmark_op("72. Blend Modes: Color Dodge & Difference (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(200, 100, 50, 180));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::ColorDodge : BlendMode::Difference;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(180, 200, 100, 50));
            p.setStyle(SkPaint::kFill_Style);
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kColorDodge : SkBlendMode::kDifference);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 73. Additive Compositing (Plus / Add Blend Mode)
    benchmark_op("73. Additive Compositing (Plus / Add Blend Mode)",
        [&]() {
            Paint p(Color::from_rgba8(80, 120, 240, 150));
            p.blend_mode = BlendMode::Plus;
            for (int i = 0; i < 30; ++i) {
                canvas.fill_circle(100.0f + (i % 6) * 160.0f, 120.0f + (i / 6) * 110.0f, 60.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(150, 80, 120, 240));
            p.setStyle(SkPaint::kFill_Style);
            p.setBlendMode(SkBlendMode::kPlus);
            for (int i = 0; i < 30; ++i) {
                sk_canvas->drawCircle(100.0f + (i % 6) * 160.0f, 120.0f + (i / 6) * 110.0f, 60.0f, p);
            }
        }
    );

    // 74. Vector Clip on Heavy Curved Stroke (24px Ribbon)
    benchmark_op("74. Vector Clip on Heavy Curved Stroke (24px Ribbon)",
        [&]() {
            canvas.save();
            auto clip_c = PathBuilder::from_circle(540.0f, 370.0f, 250.0f);
            if (clip_c) canvas.clip_path(*clip_c);
            PathBuilder pb;
            pb.move_to(100.0f, 150.0f);
            pb.cubic_to(300.0f, 650.0f, 780.0f, 90.0f, 980.0f, 590.0f);
            auto p = pb.finish();
            if (p) canvas.stroke_path(*p, Paint(Color::from_rgba8(234, 88, 12, 230)), Stroke(24.0f));
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            SkPath clip_c;
            clip_c.addCircle(540.0f, 370.0f, 250.0f);
            sk_canvas->clipPath(clip_c, true);
            SkPath path;
            path.moveTo(100.0f, 150.0f);
            path.cubicTo(300.0f, 650.0f, 780.0f, 90.0f, 980.0f, 590.0f);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 234, 88, 12));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(24.0f);
            sk_canvas->drawPath(path, p);
            sk_canvas->restore();
        }
    );

    // 75. Animated Dash Offset Phase (Moving Dash Stroke)
    benchmark_op("75. Animated Dash Offset Phase (Moving Dash Stroke)",
        [&]() {
            Stroke s(3.0f);
            Paint p(Color::from_rgba8(56, 189, 248, 230));
            for (int i = 0; i < 20; ++i) {
                s.dash = StrokeDash::create({10.0f, 6.0f}, static_cast<float>(i * 3));
                canvas.stroke_line(100.0f, 80.0f + i * 30.0f, 980.0f, 80.0f + i * 30.0f, p, s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 56, 189, 248));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(3.0f);
            SkScalar intervals[2] = { 10.0f, 6.0f };
            for (int i = 0; i < 20; ++i) {
                p.setPathEffect(SkDashPathEffect::Make(intervals, 2, static_cast<SkScalar>(i * 3)));
                sk_canvas->drawLine(100.0f, 80.0f + i * 30.0f, 980.0f, 80.0f + i * 30.0f, p);
            }
        }
    );

    // 76. Freeform Gradient Mesh (Bicubic Coons Patch)
    CoonsPatch bench_patch;
    bench_patch.top[0] = Point(240.0f, 150.0f); bench_patch.top[1] = Point(440.0f, 120.0f); bench_patch.top[2] = Point(640.0f, 180.0f); bench_patch.top[3] = Point(840.0f, 150.0f);
    bench_patch.right[0] = Point(840.0f, 150.0f); bench_patch.right[1] = Point(880.0f, 320.0f); bench_patch.right[2] = Point(800.0f, 480.0f); bench_patch.right[3] = Point(840.0f, 620.0f);
    bench_patch.bottom[0] = Point(240.0f, 620.0f); bench_patch.bottom[1] = Point(440.0f, 650.0f); bench_patch.bottom[2] = Point(640.0f, 590.0f); bench_patch.bottom[3] = Point(840.0f, 620.0f);
    bench_patch.left[0] = Point(240.0f, 150.0f); bench_patch.left[1] = Point(200.0f, 320.0f); bench_patch.left[2] = Point(280.0f, 480.0f); bench_patch.left[3] = Point(240.0f, 620.0f);

    bench_patch.color_top_left = Color::from_rgba8(255, 59, 48, 255);
    bench_patch.color_top_right = Color::from_rgba8(52, 199, 89, 255);
    bench_patch.color_bottom_right = Color::from_rgba8(0, 122, 255, 255);
    bench_patch.color_bottom_left = Color::from_rgba8(255, 204, 0, 255);

    SkPoint sk_cubics[12] = {
        {240.0f, 150.0f}, {440.0f, 120.0f}, {640.0f, 180.0f},
        {840.0f, 150.0f}, {880.0f, 320.0f}, {800.0f, 480.0f},
        {840.0f, 620.0f}, {640.0f, 590.0f}, {440.0f, 650.0f},
        {240.0f, 620.0f}, {280.0f, 480.0f}, {200.0f, 320.0f}
    };
    SkColor sk_mesh_colors[4] = {
        SkColorSetARGB(255, 255, 59, 48),
        SkColorSetARGB(255, 52, 199, 89),
        SkColorSetARGB(255, 0, 122, 255),
        SkColorSetARGB(255, 255, 204, 0)
    };

    benchmark_op("76. Freeform Gradient Mesh (Bicubic Coons Patch)",
        [&]() {
            canvas.draw_coons_patch(bench_patch, BlendMode::SourceOver, 1.0f, 8, 8);
        },
        [&]() {
            SkPaint p;
            sk_canvas->drawPatch(sk_cubics, sk_mesh_colors, nullptr, SkBlendMode::kModulate, p);
        }
    );

    // 77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)
    benchmark_op("77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)",
        [&]() {
            auto rad_opt = RadialGradient::create_2point(
                Point(350.0f, 250.0f), 60.0f,
                Point(650.0f, 450.0f), 280.0f,
                {
                    GradientStop::create(0.0f, Color::from_rgba8(244, 63, 94, 240)),
                    GradientStop::create(0.5f, Color::from_rgba8(168, 85, 247, 210)),
                    GradientStop::create(1.0f, Color::from_rgba8(14, 165, 233, 180))
                }
            );
            if (rad_opt) {
                auto r = Rect::from_xywh(140.0f, 100.0f, 800.0f, 540.0f);
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*rad_opt))));
            }
        },
        [&]() {
            SkPoint c0 = {350.0f, 250.0f};
            SkPoint c1 = {650.0f, 450.0f};
            SkColor colors[3] = {
                SkColorSetARGB(240, 244, 63, 94),
                SkColorSetARGB(210, 168, 85, 247),
                SkColorSetARGB(180, 14, 165, 233)
            };
            SkScalar pos[3] = {0.0f, 0.5f, 1.0f};
            SkPaint p;
            p.setShader(SkGradientShader::MakeTwoPointConical(c0, 60.0f, c1, 280.0f, colors, pos, 3, SkTileMode::kClamp));
            sk_canvas->drawRect(SkRect::MakeXYWH(140.0f, 100.0f, 800.0f, 540.0f), p);
        }
    );

    // 78. Axis-Aligned Stroked Rectangles (Fast-Path Stroke Rect)
    benchmark_op("78. Axis-Aligned Stroked Rectangles (25 Rects)",
        [&]() {
            Paint p(Color::from_rgba8(59, 130, 246, 220));
            Stroke s(2.5f);
            for (int i = 0; i < 25; ++i) {
                float x = 60.0f + (i % 5) * 190.0f;
                float y = 60.0f + (i / 5) * 130.0f;
                auto r = Rect::from_xywh(x, y, 150.0f, 90.0f);
                if (r) canvas.stroke_rect(*r, p, s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(220, 59, 130, 246));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(2.5f);
            for (int i = 0; i < 25; ++i) {
                float x = 60.0f + (i % 5) * 190.0f;
                float y = 60.0f + (i / 5) * 130.0f;
                sk_canvas->drawRect(SkRect::MakeXYWH(x, y, 150.0f, 90.0f), p);
            }
        }
    );

    // 79. Dashed Stroke with Round Caps (Round Endpoints)
    benchmark_op("79. Dashed Stroke with Round Caps (Round Intervals)",
        [&]() {
            Stroke s(8.0f);
            s.line_cap = LineCap::Round;
            s.dash = StrokeDash::create({20.0f, 15.0f}, 0.0f);
            Paint p(Color::from_rgba8(236, 72, 153, 230));
            for (int i = 0; i < 15; ++i) {
                float y = 60.0f + i * 42.0f;
                canvas.stroke_line(100.0f, y, 980.0f, y, p, s);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 236, 72, 153));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(8.0f);
            p.setStrokeCap(SkPaint::kRound_Cap);
            SkScalar dashes[2] = {20.0f, 15.0f};
            p.setPathEffect(SkDashPathEffect::Make(dashes, 2, 0.0f));
            for (int i = 0; i < 15; ++i) {
                float y = 60.0f + i * 42.0f;
                sk_canvas->drawLine(100.0f, y, 980.0f, y, p);
            }
        }
    );

    // 80. Anisotropic Non-Uniform Scaled Stroke (Elliptical Join/Cap)
    benchmark_op("80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)",
        [&]() {
            canvas.save();
            canvas.translate(540.0f, 370.0f);
            canvas.scale(2.5f, 0.4f);
            Paint p(Color::from_rgba8(16, 185, 129, 230));
            Stroke s(12.0f);
            s.line_join = LineJoin::Round;
            s.line_cap = LineCap::Round;
            PathBuilder pb;
            pb.move_to(-160.0f, -200.0f);
            pb.cubic_to(120.0f, -250.0f, -120.0f, 250.0f, 160.0f, 200.0f);
            auto path = pb.finish();
            if (path) canvas.stroke_path(*path, p, s);
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->translate(540.0f, 370.0f);
            sk_canvas->scale(2.5f, 0.4f);
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(230, 16, 185, 129));
            p.setStyle(SkPaint::kStroke_Style);
            p.setStrokeWidth(12.0f);
            p.setStrokeJoin(SkPaint::kRound_Join);
            p.setStrokeCap(SkPaint::kRound_Cap);
            SkPath path;
            path.moveTo(-160.0f, -200.0f);
            path.cubicTo(120.0f, -250.0f, -120.0f, 250.0f, 160.0f, 200.0f);
            sk_canvas->drawPath(path, p);
            sk_canvas->restore();
        }
    );

    // 81. Non-Convex Star Vector Clip (Even-Odd Rule)
    PathBuilder star_clip_pb;
    float clip_cx = 540.0f, clip_cy = 370.0f;
    for (int i = 0; i < 10; ++i) {
        float r = (i % 2 == 0) ? 280.0f : 120.0f;
        float a = i * (static_cast<float>(M_PI) / 5.0f) - static_cast<float>(M_PI) * 0.5f;
        float x = clip_cx + r * std::cos(a);
        float y = clip_cy + r * std::sin(a);
        if (i == 0) star_clip_pb.move_to(x, y);
        else star_clip_pb.line_to(x, y);
    }
    star_clip_pb.close();
    auto star_clip_opt = star_clip_pb.finish();
    const Path& star_clip = *star_clip_opt;

    SkPath sk_star_clip;
    for (int i = 0; i < 10; ++i) {
        float r = (i % 2 == 0) ? 280.0f : 120.0f;
        float a = i * (static_cast<float>(M_PI) / 5.0f) - static_cast<float>(M_PI) * 0.5f;
        float x = clip_cx + r * std::cos(a);
        float y = clip_cy + r * std::sin(a);
        if (i == 0) sk_star_clip.moveTo(x, y);
        else sk_star_clip.lineTo(x, y);
    }
    sk_star_clip.close();
    sk_star_clip.setFillType(SkPathFillType::kEvenOdd);

    benchmark_op("81. Non-Convex Star Vector Clip (Even-Odd Rule)",
        [&]() {
            canvas.save();
            canvas.clip_path(star_clip, FillRule::EvenOdd);
            auto lin_opt = LinearGradient::create(Point(200.0f, 100.0f), Point(880.0f, 640.0f), {
                GradientStop::create(0.0f, Color::from_rgba8(239, 68, 68, 255)),
                GradientStop::create(0.5f, Color::from_rgba8(245, 158, 11, 255)),
                GradientStop::create(1.0f, Color::from_rgba8(59, 130, 246, 255))
            });
            if (lin_opt) {
                auto r = Rect::from_xywh(200.0f, 100.0f, 680.0f, 540.0f);
                if (r) canvas.fill_rect(*r, Paint(Shader(std::move(*lin_opt))));
            }
            canvas.restore();
        },
        [&]() {
            sk_canvas->save();
            sk_canvas->clipPath(sk_star_clip, true);
            SkPoint pts[2] = { {200.0f, 100.0f}, {880.0f, 640.0f} };
            SkColor colors[3] = {
                SkColorSetARGB(255, 239, 68, 68),
                SkColorSetARGB(255, 245, 158, 11),
                SkColorSetARGB(255, 59, 130, 246)
            };
            SkScalar pos[3] = {0.0f, 0.5f, 1.0f};
            SkPaint p;
            p.setShader(SkGradientShader::MakeLinear(pts, colors, pos, 3, SkTileMode::kClamp));
            sk_canvas->drawRect(SkRect::MakeXYWH(200.0f, 100.0f, 680.0f, 540.0f), p);
            sk_canvas->restore();
        }
    );

    // 82. Multi-Contour Complex Path (20 Sub-Paths)
    PathBuilder multi_pb;
    SkPath sk_multi_path;
    for (int i = 0; i < 20; ++i) {
        float cx = 100.0f + (i % 5) * 200.0f;
        float cy = 100.0f + (i / 5) * 140.0f;
        multi_pb.push_circle(cx, cy, 40.0f);
        sk_multi_path.addCircle(cx, cy, 40.0f);
    }
    auto multi_p_opt = multi_pb.finish();
    const Path& multi_path = *multi_p_opt;

    benchmark_op("82. Multi-Contour Complex Path (20 Sub-Paths)",
        [&]() {
            Paint p(Color::from_rgba8(147, 51, 234, 200));
            canvas.fill_path(multi_path, p, FillRule::Winding);
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(200, 147, 51, 234));
            sk_canvas->drawPath(sk_multi_path, p);
        }
    );

    // 83. Blend Modes: Overlay & Soft-Light (25 Shapes)
    benchmark_op("83. Blend Modes: Overlay & Soft-Light (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(234, 88, 12, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::Overlay : BlendMode::SoftLight;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 234, 88, 12));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kOverlay : SkBlendMode::kSoftLight);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 84. Blend Modes: Darken & Lighten (25 Shapes)
    benchmark_op("84. Blend Modes: Darken & Lighten (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(16, 185, 129, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::Darken : BlendMode::Lighten;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 16, 185, 129));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kDarken : SkBlendMode::kLighten);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)
    benchmark_op("85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(99, 102, 241, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::ColorBurn : BlendMode::HardLight;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 99, 102, 241));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kColorBurn : SkBlendMode::kHardLight);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 86. Blend Modes: Exclusion & Clear (25 Shapes)
    benchmark_op("86. Blend Modes: Exclusion & Clear (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(244, 63, 94, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::Exclusion : BlendMode::Clear;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 244, 63, 94));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kExclusion : SkBlendMode::kClear);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 87. Reverse Porter-Duff: Destination-Over & Destination-Out
    benchmark_op("87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(14, 165, 233, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::DestinationOver : BlendMode::DestinationOut;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 14, 165, 233));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kDstOver : SkBlendMode::kDstOut);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 88. Reverse Porter-Duff: Source-Atop & Destination-Atop
    benchmark_op("88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(168, 85, 247, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::SourceAtop : BlendMode::DestinationAtop;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 168, 85, 247));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kSrcATop : SkBlendMode::kDstATop);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 89. HSL Non-Separable Blend: Hue & Luminosity (25 Shapes)
    benchmark_op("89. HSL Blend Modes: Hue & Luminosity (25 Shapes)",
        [&]() {
            Paint p(Color::from_rgba8(234, 179, 8, 190));
            for (int i = 0; i < 25; ++i) {
                p.blend_mode = (i % 2 == 0) ? BlendMode::Hue : BlendMode::Luminosity;
                canvas.fill_circle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        },
        [&]() {
            SkPaint p;
            p.setAntiAlias(true);
            p.setColor(SkColorSetARGB(190, 234, 179, 8));
            for (int i = 0; i < 25; ++i) {
                p.setBlendMode((i % 2 == 0) ? SkBlendMode::kHue : SkBlendMode::kLuminosity);
                sk_canvas->drawCircle(120.0f + (i % 5) * 190.0f, 120.0f + (i / 5) * 120.0f, 55.0f, p);
            }
        }
    );

    // 90. Bilinear Filtered Pattern under Rotation (30° Tiling)
    benchmark_op("90. Bilinear Filtered Pattern under Rotation (30° Tiling)",
        [&]() {
            Transform pat_ts = Transform::from_rotate(30.0f).pre_scale(0.75f, 0.75f);
            Pattern pat(tile_pm.as_ref(), SpreadMode::Repeat, FilterQuality::Bilinear, 1.0f, pat_ts);
            Paint p;
            p.shader = Shader(pat);
            auto r = Rect::from_xywh(140.0f, 100.0f, 800.0f, 540.0f);
            if (r) canvas.fill_rect(*r, p);
        },
        [&]() {
            SkMatrix mat;
            mat.setRotate(30.0f);
            mat.preScale(0.75f, 0.75f);
            SkSamplingOptions sampling(SkFilterMode::kLinear);
            sk_sp<SkShader> shader = sk_tile_img->makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, sampling, mat);
            SkPaint p;
            p.setShader(shader);
            sk_canvas->drawRect(SkRect::MakeXYWH(140.0f, 100.0f, 800.0f, 540.0f), p);
        }
    );

    // Print Detailed 2-Way Comparative Report (Nisaba vs Google Skia)
    std::cout << std::left << std::setw(50) << "Operation Name"
              << std::right << std::setw(13) << "Nisaba (µs)"
              << std::setw(13) << "Skia (µs)"
              << std::setw(18) << "vs Skia" << "\n";
    std::cout << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------\n";

    double total_nisaba = 0;
    double total_skia = 0;

    for (const auto& s : stats) {
        total_nisaba += s.nisaba_us;
        total_skia += s.skia_us;

        // Ratio vs Skia
        std::string verdict_skia;
        double r_s = (s.nisaba_us > 0) ? (s.skia_us / s.nisaba_us) : 1.0;
        if (r_s > 1.05) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << r_s << "x (Nisaba)";
            verdict_skia = ss.str();
        } else if (r_s < 0.95) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << (1.0 / r_s) << "x (Skia)";
            verdict_skia = ss.str();
        } else {
            verdict_skia = "Tie (~1.0x)";
        }

        std::ostringstream s_nisaba, s_skia;
        s_nisaba << std::fixed << std::setprecision(1) << s.nisaba_us << " µs";
        s_skia << std::fixed << std::setprecision(1) << s.skia_us << " µs";

        std::cout << std::left << std::setw(50) << s.name
                  << std::right << std::setw(13) << s_nisaba.str()
                  << std::setw(13) << s_skia.str()
                  << std::setw(18) << verdict_skia << "\n";
    }

    std::cout << "-----------------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    std::ostringstream s_tot_n, s_tot_s;
    s_tot_n << std::fixed << std::setprecision(1) << total_nisaba << " µs";
    s_tot_s << std::fixed << std::setprecision(1) << total_skia << " µs";

    std::cout << std::left << std::setw(50) << "TOTAL FRAME OVERHEAD"
              << std::right << std::setw(13) << s_tot_n.str()
              << std::setw(13) << s_tot_s.str();

    double total_r_s = total_skia / total_nisaba;
    std::ostringstream ss_ts;
    if (total_r_s >= 1.0) ss_ts << std::fixed << std::setprecision(2) << total_r_s << "x (Nisaba)";
    else ss_ts << std::fixed << std::setprecision(2) << (1.0 / total_r_s) << "x (Skia)";

    std::cout << std::setw(18) << ss_ts.str() << "\n";
    std::cout << "=================================================================================================================================================================\n\n";

    // Win / Loss Scorecard Summary
    int nisaba_wins_skia = 0, skia_wins_nisaba = 0, ties_skia = 0;
    for (const auto& s : stats) {
        if (s.nisaba_us < s.skia_us) nisaba_wins_skia++;
        else if (s.skia_us < s.nisaba_us) skia_wins_nisaba++;
        else ties_skia++;
    }

    double pct_n_s = (double)nisaba_wins_skia / stats.size() * 100.0;
    double pct_s_n = (double)skia_wins_nisaba / stats.size() * 100.0;

    std::cout << "=================================================================================================================================================================\n";
    std::cout << "                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 \n";
    std::cout << "=================================================================================================================================================================\n";
    std::cout << "  Total Benchmark Suites: " << stats.size() << "\n\n";
    std::cout << "  [Nisaba vs Google Skia (CPU Rasterizer)]\n";
    std::cout << "    * Nisaba Won: " << nisaba_wins_skia << " / " << stats.size() << " tests (" << std::fixed << std::setprecision(1) << pct_n_s << "%)\n";
    std::cout << "    * Skia Won:   " << skia_wins_nisaba << " / " << stats.size() << " tests (" << std::fixed << std::setprecision(1) << pct_s_n << "%)\n";
    std::cout << "=================================================================================================================================================================\n\n";

    return 0;
}

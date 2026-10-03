#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::svg;
using namespace nisaba::effects;
using namespace nisaba::text;

namespace {

std::string resolve_font(std::string_view filename) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename),
        std::string("../../fonts/") + std::string(filename)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return candidates[0];
}

// Generates a rich high-tech telemetry texture for 3D card projection
Pixmap generate_telemetry_texture(uint32_t w, uint32_t h, FontSystem& fs, GlyphCache& cache) {
    auto pm = Pixmap::allocate(w, h);
    if (!pm) return Pixmap();
    pm->fill(Color::from_rgba8(14, 20, 32, 240));

    Canvas c(*pm);

    // Border highlight
    Paint border_paint(Color::from_rgba8(0, 229, 255, 200));
    Stroke border_stroke(2.5f);
    c.stroke_rect(*Rect::from_xywh(2.0f, 2.0f, w - 4.0f, h - 4.0f), border_paint, border_stroke);

    // Top title bar
    Paint bar_paint(Color::from_rgba8(0, 229, 255, 30));
    c.fill_rect(*Rect::from_xywh(4.0f, 4.0f, w - 8.0f, 36.0f), bar_paint);

    // Corner targeting brackets
    Paint corner_paint(Color::from_rgba8(0, 229, 255, 255));
    Stroke corner_stroke(3.0f);
    auto draw_corner = [&](float x, float y, float dx, float dy) {
        PathBuilder pb;
        pb.move_to(x + dx * 16.0f, y);
        pb.line_to(x, y);
        pb.line_to(x, y + dy * 16.0f);
        auto p = pb.finish();
        if (p) c.stroke_path(*p, corner_paint, corner_stroke);
    };
    draw_corner(10.0f, 10.0f, 1.0f, 1.0f);
    draw_corner(w - 10.0f, 10.0f, -1.0f, 1.0f);
    draw_corner(w - 10.0f, h - 10.0f, -1.0f, -1.0f);
    draw_corner(10.0f, h - 10.0f, 1.0f, -1.0f);

    // High fidelity typography
    auto draw_txt = [&](float x, float y, std::string_view text, float sz, TextColor col) {
        Buffer buf(Metrics(sz, sz * 1.3f));
        Attrs attrs;
        attrs.set_color(col);
        buf.set_text(text, attrs);
        buf.draw(c, cache, fs, Color::WHITE, x, y);
    };

    draw_txt(20.0f, 12.0f, "3D SPATIAL TELEMETRY", 15.0f, TextColor::rgb(0, 229, 255));
    draw_txt(w - 75.0f, 12.0f, "98.4%", 15.0f, TextColor::rgb(0, 255, 136));

    // Vector concentric circles gauge
    Point center(85.0f, 125.0f);
    Paint circle_paint(Color::from_rgba8(0, 229, 255, 60));
    Stroke circle_stroke(1.5f);
    c.stroke_circle(center.x, center.y, 45.0f, circle_paint, circle_stroke);
    c.stroke_circle(center.x, center.y, 30.0f, circle_paint, circle_stroke);
    c.stroke_circle(center.x, center.y, 15.0f, circle_paint, circle_stroke);

    // Crosshairs
    PathBuilder ch;
    ch.move_to(center.x - 52.0f, center.y); ch.line_to(center.x + 52.0f, center.y);
    ch.move_to(center.x, center.y - 52.0f); ch.line_to(center.x, center.y + 52.0f);
    auto ch_p = ch.finish();
    if (ch_p) c.stroke_path(*ch_p, border_paint, Stroke(1.0f));

    // Horizontal Telemetry metrics bars
    float bar_x = 155.0f;
    float bar_w = w - bar_x - 25.0f;
    struct Metric { std::string name; float val; Color col; };
    std::vector<Metric> metrics = {
        {"HOMOGRAPHY ACCURACY", 0.94f, Color::from_rgba8(0, 229, 255, 255)},
        {"BILINEAR FILTER RATE", 0.88f, Color::from_rgba8(0, 255, 136, 255)},
        {"DEPTH FORESHORTENING", 0.76f, Color::from_rgba8(255, 170, 0, 255)},
        {"CAMERA PERSPECTIVE Z", 0.62f, Color::from_rgba8(255, 0, 110, 255)},
    };

    float my = 60.0f;
    for (const auto& m : metrics) {
        draw_txt(bar_x, my, m.name, 11.0f, TextColor::rgb(180, 200, 225));

        // Background bar
        Paint mbg(Color::from_rgba8(30, 42, 65, 255));
        c.fill_rect(*Rect::from_xywh(bar_x, my + 16.0f, bar_w, 8.0f), mbg);

        // Filled value bar
        Paint mfill(m.col);
        c.fill_rect(*Rect::from_xywh(bar_x, my + 16.0f, bar_w * m.val, 8.0f), mfill);

        my += 34.0f;
    }

    // Arabic Subtitle
    draw_txt(20.0f, h - 30.0f, "محرك الإسقاط المنظوري ثلاثي الأبعاد • 4x4", 12.5f, TextColor::rgb(0, 229, 255));

    return *pm;
}

// Generates an intricate radial calibration test pattern for quad homography
Pixmap generate_target_texture(uint32_t w, uint32_t h, FontSystem& fs, GlyphCache& cache) {
    auto pm = Pixmap::allocate(w, h);
    if (!pm) return Pixmap();
    pm->fill(Color::from_rgba8(16, 22, 36, 245));

    Canvas c(*pm);

    // Checkerboard subtle pattern in corners
    for (uint32_t y = 0; y < h; y += 20) {
        for (uint32_t x = 0; x < w; x += 20) {
            if (((x / 20) + (y / 20)) % 2 == 0) {
                Paint p(Color::from_rgba8(25, 34, 52, 180));
                c.fill_rect(*Rect::from_xywh(static_cast<float>(x), static_cast<float>(y), 20.0f, 20.0f), p);
            }
        }
    }

    // Outer border
    Paint border(Color::from_rgba8(255, 0, 110, 220));
    c.stroke_rect(*Rect::from_xywh(3.0f, 3.0f, w - 6.0f, h - 6.0f), border, Stroke(2.0f));

    // Center circular reticle
    float cx = w * 0.5f;
    float cy = h * 0.5f;

    for (float r = 20.0f; r <= 100.0f; r += 20.0f) {
        Paint p(Color::from_rgba8(255, 0, 110, static_cast<uint8_t>(255 - r * 1.8f)));
        c.stroke_circle(cx, cy, r, p, Stroke(1.5f));
    }

    // Diagonal calibration lines
    Paint diag(Color::from_rgba8(0, 229, 255, 120));
    Stroke ds(1.2f);
    PathBuilder pb;
    pb.move_to(0.0f, 0.0f); pb.line_to(w, h);
    pb.move_to(w, 0.0f); pb.line_to(0.0f, h);
    auto pbd = pb.finish();
    if (pbd) c.stroke_path(*pbd, diag, ds);

    // Center neon dot
    c.fill_circle(cx, cy, 6.0f, Paint(Color::from_rgba8(255, 255, 255, 255)));

    // Labels
    Buffer buf(Metrics(14.0f, 18.0f));
    Attrs a;
    a.set_color(TextColor::rgb(255, 255, 255));
    buf.set_text("PROJECTIVE HOMOGRAPHY", a);
    buf.draw(c, cache, fs, Color::WHITE, 25.0f, 15.0f);

    Buffer buf2(Metrics(12.0f, 16.0f));
    Attrs a2;
    a2.set_color(TextColor::rgb(0, 255, 136));
    buf2.set_text("HECKBERT 8-DOF WARP", a2);
    buf2.draw(c, cache, fs, Color::WHITE, 45.0f, h - 25.0f);

    return *pm;
}

// Generates a sovereign cyber security holographic badge texture
Pixmap generate_badge_texture(uint32_t w, uint32_t h, FontSystem& fs, GlyphCache& cache) {
    auto pm = Pixmap::allocate(w, h);
    if (!pm) return Pixmap();
    pm->fill(Color::from_rgba8(20, 16, 32, 235));

    Canvas c(*pm);

    // Violet / Purple theme
    Paint border(Color::from_rgba8(168, 85, 247, 220));
    c.stroke_rect(*Rect::from_xywh(2.0f, 2.0f, w - 4.0f, h - 4.0f), border, Stroke(2.0f));

    // Vector Shield Icon
    const std::string svg_shield = R"(
        <svg viewBox="0 0 100 100">
            <path d="M 50 12 L 85 24 C 85 60 50 88 50 88 C 50 88 15 60 15 24 Z" fill="#2d1b4e" stroke="#a855f7" stroke-width="4"/>
            <path d="M 50 24 L 75 33 C 75 58 50 78 50 78 C 50 78 25 58 25 33 Z" fill="#181126" stroke="#c084fc" stroke-width="2"/>
            <circle cx="50" cy="50" r="12" fill="#a855f7"/>
            <circle cx="50" cy="50" r="6" fill="#ffffff"/>
        </svg>
    )";

    auto doc = SvgDocument::parse(svg_shield);
    if (doc) {
        c.draw_svg(*doc, *Rect::from_xywh(w * 0.5f - 45.0f, 20.0f, 90.0f, 90.0f));
    }

    // Typography
    auto draw_txt = [&](float x, float y, std::string_view text, float sz, TextColor col) {
        Buffer buf(Metrics(sz, sz * 1.3f));
        Attrs attrs;
        attrs.set_color(col);
        buf.set_text(text, attrs);
        buf.draw(c, cache, fs, Color::WHITE, x, y);
    };

    draw_txt(35.0f, 125.0f, "SOVEREIGN CRYPTO ENGINE", 14.0f, TextColor::rgb(240, 230, 255));
    draw_txt(25.0f, 150.0f, "Zero-Copy Hardware Stride • Pure C++20", 11.5f, TextColor::rgb(192, 132, 252));
    draw_txt(55.0f, 180.0f, "SYSTEM ENCRYPTED: VALID", 12.0f, TextColor::rgb(0, 255, 136));

    return *pm;
}

} // namespace

int main() {
    std::cout << "Rendering Nisaba Sovereign 3D Perspective & Spatial Warp Showcase..." << std::endl;

    const uint32_t W = 1200;
    const uint32_t H = 800;

    auto pixmap = Pixmap::allocate(W, H);
    if (!pixmap) {
        std::cerr << "Failed to allocate main canvas pixmap" << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // 1. Deep Space Tech Gradient Background
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(8, 12, 22, 255)),
        GradientStop::create(0.45f, Color::from_rgba8(14, 18, 30, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(5, 7, 14, 255))
    };
    auto bg_grad = LinearGradient::create(
        Point::from_xy(0.0f, 0.0f),
        Point::from_xy(1200.0f, 800.0f),
        bg_stops
    );
    Paint bg_paint;
    bg_paint.shader = Shader(*bg_grad);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 1200.0f, 800.0f), bg_paint);

    // 2. 3D Spatial Perspective Runway / Grid Floor
    // Render an infinite perspective grid receding into the center horizon
    float horizon_y = 380.0f;
    float center_x = 600.0f;
    Paint floor_line_paint(Color::from_rgba8(0, 229, 255, 40));
    Stroke floor_stroke(1.0f);

    // Receding perspective grid lines
    for (float gx = -600.0f; gx <= 1800.0f; gx += 100.0f) {
        PathBuilder pb;
        pb.move_to(center_x + (gx - center_x) * 0.08f, horizon_y);
        pb.line_to(gx, 800.0f);
        auto p = pb.finish();
        if (p) canvas.stroke_path(*p, floor_line_paint, floor_stroke);
    }

    // Horizontal perspective grid rungs (exponential spacing)
    for (int i = 1; i <= 14; ++i) {
        float t = std::pow(static_cast<float>(i) / 14.0f, 2.2f);
        float gy = horizon_y + t * (800.0f - horizon_y);
        Paint rung_paint(Color::from_rgba8(0, 229, 255, static_cast<uint8_t>(t * 70.0f + 10.0f)));
        PathBuilder pb;
        pb.move_to(0.0f, gy); pb.line_to(1200.0f, gy);
        auto p = pb.finish();
        if (p) canvas.stroke_path(*p, rung_paint, floor_stroke);
    }

    // Horizon glowing beam
    Paint beam_paint(Color::from_rgba8(0, 229, 255, 90));
    canvas.fill_rect(*Rect::from_xywh(0.0f, horizon_y - 1.0f, 1200.0f, 2.0f), beam_paint);

    // 3. Fonts and Typography
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("NotoSansArabic.ttf"));
    GlyphCache glyph_cache;

    auto draw_text = [&](float x, float y, std::string_view text, float sz, float lh, TextColor col) {
        Buffer buf(Metrics(sz, lh));
        Attrs attrs;
        attrs.set_color(col);
        buf.set_text(text, attrs);
        buf.draw(canvas, glyph_cache, font_system, Color::WHITE, x, y);
    };

    // Header Title
    draw_text(70.0f, 32.0f, "NISABA 3D PERSPECTIVE & SPATIAL ENGINE", 24.0f, 30.0f, TextColor::rgb(255, 255, 255));
    draw_text(70.0f, 68.0f, "Full 4x4 Projective Transforms • Heckbert Homography Solver • Perspective-Correct Scanline Rasterizer", 13.5f, 18.0f, TextColor::rgb(0, 229, 255));
    draw_text(70.0f, 92.0f, "محرك الإسقاط ثلاثي الأبعاد والتحويل المنظوري المتطور بدون أي مكتبات خارجية", 13.0f, 18.0f, TextColor::rgb(140, 180, 220));

    // Separator line
    Paint sep(Color::from_rgba8(255, 255, 255, 30));
    canvas.fill_rect(*Rect::from_xywh(70.0f, 120.0f, 1060.0f, 1.0f), sep);

    // 4. Generate 2D Source Textures
    auto tex_telemetry = generate_telemetry_texture(320, 220, font_system, glyph_cache);
    auto tex_target = generate_target_texture(240, 240, font_system, glyph_cache);
    auto tex_badge = generate_badge_texture(280, 220, font_system, glyph_cache);

    // 5. Render Card 1: 3D Perspective Orbit (Yaw: -32°, Pitch: +14°)
    // Left card tilted away in 3D depth
    {
        float pivot_x = 240.0f;
        float pivot_y = 440.0f;
        auto t3d = Transform4x4::from_camera_orbit(pivot_x, pivot_y, 14.0f, -32.0f, 0.0f, 750.0f);
        // Position source texture centered at pivot
        t3d = t3d * Transform4x4::from_translate(pivot_x - 160.0f, pivot_y - 110.0f, 0.0f);

        // Cast perspective floor shadow
        auto shadow_rect = Rect::from_xywh(pivot_x - 140.0f, 400.0f, 280.0f, 100.0f);
        if (shadow_rect) {
            effects::draw_round_rect_shadow(canvas.pixmap(), *shadow_rect, 30.0f, 30.0f,
                DropShadow(0.0f, 0.0f, 35.0f, Color::from_rgba8(0, 0, 0, 180)));
        }

        // Draw the 3D perspective pixmap
        canvas.draw_pixmap_3d(tex_telemetry.as_ref(), t3d, 0.98f, BlendMode::SourceOver);

        // Description label under the card
        draw_text(90.0f, 620.0f, "YAW: -32° | PITCH: +14°", 13.0f, 16.0f, TextColor::rgb(0, 229, 255));
        draw_text(90.0f, 642.0f, "Focal Distance d = 750px\nLinear (U/W, V/W, 1/W) Step", 11.5f, 15.0f, TextColor::rgb(160, 180, 200));
    }

    // 6. Render Card 2: Center Homography Quad Warping (Arbitrary 4 points)
    // Demonstrating Paul Heckbert 8-DOF quad mapping with dynamic perspective trapezoid
    {
        Point q0(480.0f, 280.0f); // Top-left
        Point q1(720.0f, 280.0f); // Top-right
        Point q2(770.0f, 560.0f); // Bottom-right (flared out)
        Point q3(430.0f, 560.0f); // Bottom-left (flared out)

        // Drop shadow behind the warped quad
        auto quad_bounds = Rect::from_xywh(430.0f, 280.0f, 340.0f, 280.0f);
        if (quad_bounds) {
            effects::draw_round_rect_shadow(canvas.pixmap(), *quad_bounds, 20.0f, 20.0f,
                DropShadow(0.0f, 15.0f, 25.0f, Color::from_rgba8(0, 0, 0, 160)));
        }

        // Render arbitrary quad with perspective-correct scanline interpolation
        canvas.draw_pixmap_perspective(tex_target.as_ref(), q0, q1, q2, q3, 1.0f, BlendMode::SourceOver);

        // Corner control handles (to visually prove exact homography mapping)
        auto draw_handle = [&](Point p, std::string_view label, TextColor col) {
            canvas.fill_circle(p.x, p.y, 5.0f, Paint(Color::from_rgba8(255, 0, 110, 255)));
            canvas.stroke_circle(p.x, p.y, 8.0f, Paint(Color::from_rgba8(255, 255, 255, 220)), Stroke(1.5f));
            draw_text(p.x - 15.0f, p.y < 350.0f ? p.y - 20.0f : p.y + 10.0f, label, 11.0f, 14.0f, col);
        };
        draw_handle(q0, "P0 (0,0)", TextColor::rgb(255, 0, 110));
        draw_handle(q1, "P1 (1,0)", TextColor::rgb(255, 0, 110));
        draw_handle(q2, "P2 (1,1)", TextColor::rgb(255, 0, 110));
        draw_handle(q3, "P3 (0,1)", TextColor::rgb(255, 0, 110));

        draw_text(485.0f, 620.0f, "HECKBERT 8-DOF HOMOGRAPHY", 13.0f, 16.0f, TextColor::rgb(255, 0, 110));
        draw_text(485.0f, 642.0f, "Arbitrary 4-Point Projective Mapping\nZero Diagonal Seams / Affine Tearing", 11.5f, 15.0f, TextColor::rgb(160, 180, 200));
    }

    // 7. Render Card 3: 3D Perspective Orbit (Yaw: +36°, Pitch: +16°)
    // Right card tilted away in opposite 3D direction
    {
        float pivot_x = 960.0f;
        float pivot_y = 440.0f;
        auto t3d = Transform4x4::from_camera_orbit(pivot_x, pivot_y, 16.0f, 36.0f, 0.0f, 750.0f);
        t3d = t3d * Transform4x4::from_translate(pivot_x - 140.0f, pivot_y - 110.0f, 0.0f);

        // Elevation drop shadow
        auto shadow_rect = Rect::from_xywh(pivot_x - 130.0f, 400.0f, 260.0f, 100.0f);
        if (shadow_rect) {
            effects::draw_round_rect_shadow(canvas.pixmap(), *shadow_rect, 30.0f, 30.0f,
                DropShadow(0.0f, 0.0f, 35.0f, Color::from_rgba8(0, 0, 0, 180)));
        }

        // Draw 3D perspective pixmap
        canvas.draw_pixmap_3d(tex_badge.as_ref(), t3d, 0.98f, BlendMode::SourceOver);

        draw_text(875.0f, 620.0f, "YAW: +36° | PITCH: +16°", 13.0f, 16.0f, TextColor::rgb(168, 85, 247));
        draw_text(875.0f, 642.0f, "Bilinear Subpixel Antialiasing\nFull 29 Blend Modes Compatible", 11.5f, 15.0f, TextColor::rgb(160, 180, 200));
    }

    // 8. Bottom Status Badge
    auto status_rect = Rect::from_xywh(70.0f, 725.0f, 1060.0f, 40.0f);
    if (status_rect) {
        Paint bg_stat(Color::from_rgba8(16, 22, 38, 200));
        canvas.fill_rect(*status_rect, bg_stat);
        Paint border_stat(Color::from_rgba8(0, 229, 255, 60));
        canvas.stroke_rect(*status_rect, border_stat, Stroke(1.0f));

        draw_text(90.0f, 735.0f, "ARCHITECTURE: Pure ISO C++20 • 0 External Dependencies • Subpixel Coverage Filtering • vaxp Sovereignty", 12.0f, 16.0f, TextColor::rgb(0, 255, 136));
        draw_text(980.0f, 735.0f, "FPS: 60+ (CPU RENDER)", 12.0f, 16.0f, TextColor::rgb(0, 229, 255));
    }

    // Save outputs
    const std::string out_bmp = "nisaba_perspective_showcase.bmp";
    if (pixmap->save_bmp(out_bmp)) {
        std::cout << "Successfully saved: " << out_bmp << std::endl;
    } else {
        std::cerr << "Failed to save BMP!" << std::endl;
        return 1;
    }

    return 0;
}

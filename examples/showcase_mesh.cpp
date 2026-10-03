#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::text;
using namespace nisaba::effects;

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

// Generates a rich high-tech circuit texture to map onto the waving mesh
Pixmap generate_circuit_texture(uint32_t w, uint32_t h, FontSystem& fs, GlyphCache& cache) {
    auto pm = Pixmap::allocate(w, h);
    if (!pm) return Pixmap();
    pm->fill(Color::from_rgba8(12, 18, 30, 255));

    Canvas c(*pm);

    // Subtle grid lines
    Paint grid_paint(Color::from_rgba8(0, 229, 255, 30));
    Stroke grid_stroke(1.0f);
    for (uint32_t x = 0; x <= w; x += 25) {
        PathBuilder pb; pb.move_to(x, 0); pb.line_to(x, h);
        auto p = pb.finish(); if (p) c.stroke_path(*p, grid_paint, grid_stroke);
    }
    for (uint32_t y = 0; y <= h; y += 25) {
        PathBuilder pb; pb.move_to(0, y); pb.line_to(w, y);
        auto p = pb.finish(); if (p) c.stroke_path(*p, grid_paint, grid_stroke);
    }

    // Circuit buses
    Paint trace_paint(Color::from_rgba8(0, 229, 255, 180));
    Stroke trace_stroke(2.0f);
    PathBuilder pb;
    pb.move_to(20.0f, 40.0f); pb.line_to(120.0f, 40.0f); pb.line_to(160.0f, 80.0f); pb.line_to(260.0f, 80.0f);
    pb.move_to(20.0f, 120.0f); pb.line_to(80.0f, 120.0f); pb.line_to(130.0f, 170.0f); pb.line_to(260.0f, 170.0f);
    pb.move_to(50.0f, 210.0f); pb.line_to(180.0f, 210.0f); pb.line_to(220.0f, 170.0f);
    auto p_trace = pb.finish();
    if (p_trace) c.stroke_path(*p_trace, trace_paint, trace_stroke);

    // Gold circuit nodes
    Paint node_paint(Color::from_rgba8(255, 180, 0, 255));
    c.fill_circle(120.0f, 40.0f, 4.0f, node_paint);
    c.fill_circle(160.0f, 80.0f, 4.0f, node_paint);
    c.fill_circle(80.0f, 120.0f, 4.0f, node_paint);
    c.fill_circle(130.0f, 170.0f, 4.0f, node_paint);

    // Neon badge
    auto badge_rect = Rect::from_xywh(18.0f, 75.0f, 85.0f, 32.0f);
    if (badge_rect) {
        Paint bg_b(Color::from_rgba8(0, 229, 255, 40));
        c.fill_rect(*badge_rect, bg_b);
        c.stroke_rect(*badge_rect, Paint(Color::from_rgba8(0, 229, 255, 220)), Stroke(1.5f));
    }

    // Typography on texture
    auto draw_txt = [&](float x, float y, std::string_view text, float sz, TextColor col) {
        Buffer buf(Metrics(sz, sz * 1.3f));
        Attrs attrs;
        attrs.set_color(col);
        buf.set_text(text, attrs);
        buf.draw(c, cache, fs, Color::WHITE, x, y);
    };

    draw_txt(25.0f, 82.0f, "NISABA", 13.0f, TextColor::rgb(0, 229, 255));
    draw_txt(20.0f, 15.0f, "DEFORMATION SURFACE", 13.0f, TextColor::rgb(255, 255, 255));
    draw_txt(20.0f, h - 30.0f, "Bilinear Mesh Warping • 2D GPU Ready", 10.5f, TextColor::rgb(160, 190, 220));

    return *pm;
}

} // namespace

int main() {
    std::cout << "Rendering Nisaba 2D Vertex Mesh & Gradient Mesh Showcase..." << std::endl;

    const uint32_t W = 1200;
    const uint32_t H = 800;

    auto pixmap = Pixmap::allocate(W, H);
    if (!pixmap) {
        std::cerr << "Failed to allocate main pixmap" << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // 1. Tech Ambient Dark Background
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(10, 14, 24, 255)),
        GradientStop::create(0.5f, Color::from_rgba8(15, 20, 34, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(6, 9, 16, 255))
    };
    auto bg_grad = LinearGradient::create(Point::from_xy(0.0f, 0.0f), Point::from_xy(1200.0f, 800.0f), bg_stops);
    Paint bg_paint;
    bg_paint.shader = Shader(*bg_grad);
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, 1200.0f, 800.0f), bg_paint);

    // Background fine grid
    Paint bg_grid(Color::from_rgba8(255, 255, 255, 8));
    Stroke bg_stroke(1.0f);
    for (float x = 0; x <= 1200; x += 40) {
        PathBuilder pb; pb.move_to(x, 0); pb.line_to(x, 800);
        auto p = pb.finish(); if (p) canvas.stroke_path(*p, bg_grid, bg_stroke);
    }
    for (float y = 0; y <= 800; y += 40) {
        PathBuilder pb; pb.move_to(0, y); pb.line_to(1200, y);
        auto p = pb.finish(); if (p) canvas.stroke_path(*p, bg_grid, bg_stroke);
    }

    // 2. Sovereign Typography Setup
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("NotoSansArabic.ttf"));
    GlyphCache cache;

    auto draw_text = [&](float x, float y, std::string_view text, float sz, float lh, TextColor col) {
        Buffer buf(Metrics(sz, lh));
        Attrs attrs;
        attrs.set_color(col);
        buf.set_text(text, attrs);
        buf.draw(canvas, cache, font_system, Color::WHITE, x, y);
    };

    // Header Title
    draw_text(70.0f, 32.0f, "NISABA 2D VERTEX MESH & GRADIENT MESHES", 24.0f, 30.0f, TextColor::rgb(255, 255, 255));
    draw_text(70.0f, 68.0f, "Bicubic Coons Patches • Barycentric Gouraud Shading • Freeform 2D Mesh Deformation & UV Mapping", 13.5f, 18.0f, TextColor::rgb(0, 229, 255));
    draw_text(70.0f, 92.0f, "محرك شبكات التشكيل الرأسي والتدرجات الحرة والتشويه الشبكي السيادي", 13.0f, 18.0f, TextColor::rgb(140, 180, 220));

    // Separator
    Paint sep(Color::from_rgba8(255, 255, 255, 30));
    canvas.fill_rect(*Rect::from_xywh(70.0f, 120.0f, 1060.0f, 1.0f), sep);

    // =========================================================================
    // FEATURE 1 (Left): Freeform 2D Mesh Deformation (Waving Cyber Banner)
    // =========================================================================
    {
        uint32_t tex_w = 320;
        uint32_t tex_h = 240;
        auto tex = generate_circuit_texture(tex_w, tex_h, font_system, cache);

        const uint32_t cols = 16;
        const uint32_t rows = 12;

        std::vector<Point> deformed_pts;
        std::vector<Point> uvs;
        deformed_pts.reserve(cols * rows);
        uvs.reserve(cols * rows);

        float origin_x = 75.0f;
        float origin_y = 200.0f;
        float mesh_w = 320.0f;
        float mesh_h = 240.0f;

        // Wave parameters
        for (uint32_t r = 0; r < rows; ++r) {
            float v = static_cast<float>(r) / static_cast<float>(rows - 1);
            for (uint32_t c = 0; c < cols; ++c) {
                float u = static_cast<float>(c) / static_cast<float>(cols - 1);

                // Complex organic wave displacement
                float wave_y = std::sin(u * 5.5f) * 22.0f + std::cos(v * 4.0f) * 12.0f;
                float wave_x = std::sin(v * 3.5f) * 14.0f;

                float px = origin_x + u * mesh_w + wave_x;
                float py = origin_y + v * mesh_h + wave_y;

                deformed_pts.push_back(Point(px, py));
                uvs.push_back(Point(u * tex_w, v * tex_h));
            }
        }

        auto deformed_mesh = Vertices::create_grid(cols, rows, deformed_pts, nullptr, &uvs);

        // Elevation shadow underneath waving mesh
        auto mesh_bounds = deformed_mesh.bounds();
        effects::draw_round_rect_shadow(canvas.pixmap(), mesh_bounds, 25.0f, 25.0f,
            DropShadow(0.0f, 18.0f, 28.0f, Color::from_rgba8(0, 0, 0, 160)));

        // Render the textured deformed mesh with bilinear filtering!
        canvas.draw_textured_vertices(deformed_mesh, tex.as_ref(), BlendMode::SourceOver, 1.0f);

        // Draw overlay wireframe on the lower quads to expose the underlying mesh topology
        Paint wire_paint(Color::from_rgba8(0, 229, 255, 90));
        Stroke wire_stroke(1.0f);
        size_t tri_count = deformed_mesh.triangle_count();
        for (size_t t = tri_count / 2; t < tri_count; ++t) {
            auto [i0, i1, i2] = deformed_mesh.get_triangle_indices(t);
            Point p0 = deformed_mesh.positions[i0];
            Point p1 = deformed_mesh.positions[i1];
            Point p2 = deformed_mesh.positions[i2];

            PathBuilder pb;
            pb.move_to(p0.x, p0.y);
            pb.line_to(p1.x, p1.y);
            pb.line_to(p2.x, p2.y);
            pb.close();
            auto p = pb.finish();
            if (p) canvas.stroke_path(*p, wire_paint, wire_stroke);
        }

        draw_text(75.0f, 540.0f, "FREEFORM 2D MESH DEFORMATION", 14.0f, 18.0f, TextColor::rgb(0, 229, 255));
        draw_text(75.0f, 565.0f, "16x12 Tessellated Grid • Barycentric UV\nOrganic Sinusoidal Warping with Bilinear Filter", 11.5f, 15.0f, TextColor::rgb(160, 180, 210));
    }

    // =========================================================================
    // FEATURE 2 (Center): Photorealistic Coons Patch Gradient Mesh (Glowing Orb)
    // =========================================================================
    {
        // 4 Interconnected Coons Patches forming a lush multi-chromatic sphere
        GradientMesh orb_mesh;

        float cx = 600.0f;
        float cy = 340.0f;
        float r = 140.0f;

        // Quad 1: Top-Left quadrant patch
        CoonsPatch p_tl;
        p_tl.top[0]    = Point(cx - r, cy);     p_tl.top[1]    = Point(cx - r, cy - r * 0.55f); p_tl.top[2]    = Point(cx - r * 0.55f, cy - r); p_tl.top[3]    = Point(cx, cy - r);
        p_tl.right[0]  = Point(cx, cy - r);     p_tl.right[1]  = Point(cx, cy - r * 0.6f);      p_tl.right[2]  = Point(cx, cy - r * 0.2f);      p_tl.right[3]  = Point(cx, cy);
        p_tl.bottom[0] = Point(cx - r, cy);     p_tl.bottom[1] = Point(cx - r * 0.6f, cy);      p_tl.bottom[2] = Point(cx - r * 0.2f, cy);      p_tl.bottom[3] = Point(cx, cy);
        p_tl.left[0]   = Point(cx - r, cy);     p_tl.left[1]   = Point(cx - r, cy - r * 0.55f); p_tl.left[2]   = Point(cx - r * 0.55f, cy - r); p_tl.left[3]   = Point(cx, cy - r);

        p_tl.color_top_left = Color::from_rgba8(0, 229, 255, 255);      // Cyan
        p_tl.color_top_right = Color::from_rgba8(255, 255, 255, 255);    // White specular highlight
        p_tl.color_bottom_left = Color::from_rgba8(168, 85, 247, 255);   // Purple
        p_tl.color_bottom_right = Color::from_rgba8(0, 100, 255, 255);   // Deep Blue
        orb_mesh.add_patch(p_tl);

        // Quad 2: Top-Right quadrant patch
        CoonsPatch p_tr;
        p_tr.top[0]    = Point(cx, cy - r);     p_tr.top[1]    = Point(cx + r * 0.55f, cy - r); p_tr.top[2]    = Point(cx + r, cy - r * 0.55f); p_tr.top[3]    = Point(cx + r, cy);
        p_tr.right[0]  = Point(cx + r, cy);     p_tr.right[1]  = Point(cx + r * 0.8f, cy);      p_tr.right[2]  = Point(cx + r * 0.3f, cy);      p_tr.right[3]  = Point(cx, cy);
        p_tr.bottom[0] = Point(cx, cy - r);     p_tr.bottom[1] = Point(cx, cy - r * 0.6f);      p_tr.bottom[2] = Point(cx, cy - r * 0.2f);      p_tr.bottom[3] = Point(cx, cy);
        p_tr.left[0]   = Point(cx, cy - r);     p_tr.left[1]   = Point(cx + r * 0.55f, cy - r); p_tr.left[2]   = Point(cx + r, cy - r * 0.55f); p_tr.left[3]   = Point(cx + r, cy);

        p_tr.color_top_left = Color::from_rgba8(255, 255, 255, 255);    // Specular
        p_tr.color_top_right = Color::from_rgba8(255, 0, 128, 255);     // Neon Magenta
        p_tr.color_bottom_left = Color::from_rgba8(0, 100, 255, 255);
        p_tr.color_bottom_right = Color::from_rgba8(255, 170, 0, 255);   // Amber Gold
        orb_mesh.add_patch(p_tr);

        // Quad 3: Bottom-Left quadrant patch
        CoonsPatch p_bl;
        p_bl.top[0]    = Point(cx - r, cy);     p_bl.top[1]    = Point(cx - r * 0.6f, cy);      p_bl.top[2]    = Point(cx - r * 0.2f, cy);      p_bl.top[3]    = Point(cx, cy);
        p_bl.right[0]  = Point(cx, cy);         p_bl.right[1]  = Point(cx, cy + r * 0.4f);      p_bl.right[2]  = Point(cx, cy + r * 0.8f);      p_bl.right[3]  = Point(cx, cy + r);
        p_bl.bottom[0] = Point(cx - r, cy);     p_bl.bottom[1] = Point(cx - r, cy + r * 0.55f); p_bl.bottom[2] = Point(cx - r * 0.55f, cy + r); p_bl.bottom[3] = Point(cx, cy + r);
        p_bl.left[0]   = Point(cx - r, cy);     p_bl.left[1]   = Point(cx - r, cy + r * 0.55f); p_bl.left[2]   = Point(cx - r * 0.55f, cy + r); p_bl.left[3]   = Point(cx, cy + r);

        p_bl.color_top_left = Color::from_rgba8(168, 85, 247, 255);
        p_bl.color_top_right = Color::from_rgba8(0, 100, 255, 255);
        p_bl.color_bottom_left = Color::from_rgba8(20, 10, 40, 255);     // Deep ambient dark
        p_bl.color_bottom_right = Color::from_rgba8(0, 255, 136, 255);   // Emerald
        orb_mesh.add_patch(p_bl);

        // Quad 4: Bottom-Right quadrant patch
        CoonsPatch p_br;
        p_br.top[0]    = Point(cx, cy);         p_br.top[1]    = Point(cx + r * 0.3f, cy);      p_br.top[2]    = Point(cx + r * 0.8f, cy);      p_br.top[3]    = Point(cx + r, cy);
        p_br.right[0]  = Point(cx + r, cy);     p_br.right[1]  = Point(cx + r, cy + r * 0.55f); p_br.right[2]  = Point(cx + r * 0.55f, cy + r); p_br.right[3]  = Point(cx, cy + r);
        p_br.bottom[0] = Point(cx, cy + r);     p_br.bottom[1] = Point(cx, cy + r * 0.8f);      p_br.bottom[2] = Point(cx, cy + r * 0.4f);      p_br.bottom[3] = Point(cx, cy);
        p_br.left[0]   = Point(cx, cy + r);     p_br.left[1]   = Point(cx + r * 0.55f, cy + r); p_br.left[2]   = Point(cx + r, cy + r * 0.55f); p_br.left[3]   = Point(cx + r, cy);

        p_br.color_top_left = Color::from_rgba8(0, 100, 255, 255);
        p_br.color_top_right = Color::from_rgba8(255, 170, 0, 255);
        p_br.color_bottom_left = Color::from_rgba8(0, 255, 136, 255);
        p_br.color_bottom_right = Color::from_rgba8(20, 30, 60, 255);
        orb_mesh.add_patch(p_br);

        // Deep drop shadow behind orb
        auto orb_rect = Rect::from_xywh(cx - r - 10.0f, cy - r - 10.0f, (r + 10.0f) * 2.0f, (r + 10.0f) * 2.0f);
        if (orb_rect) {
            effects::draw_round_rect_shadow(canvas.pixmap(), *orb_rect, r + 10.0f, r + 10.0f,
                DropShadow(0.0f, 15.0f, 40.0f, Color::from_rgba8(0, 0, 0, 190)));
        }

        // Draw the photorealistic Coons Patch Gradient Mesh
        canvas.draw_gradient_mesh(orb_mesh, BlendMode::SourceOver, 1.0f, 14);

        // Subtly outline the patch control curves
        Paint curve_outline(Color::from_rgba8(255, 255, 255, 60));
        Stroke cos(1.0f);
        canvas.stroke_circle(cx, cy, r, curve_outline, cos);

        draw_text(480.0f, 540.0f, "COONS PATCH GRADIENT MESH", 14.0f, 18.0f, TextColor::rgb(255, 0, 128));
        draw_text(480.0f, 565.0f, "4 Bicubic Interpolated Patches • C1 Smooth\nSeamless Photorealistic Non-Linear Color Flow", 11.5f, 15.0f, TextColor::rgb(160, 180, 210));
    }

    // =========================================================================
    // FEATURE 3 (Right): Low-Poly Cyber Prism (Gouraud Triangle Shading)
    // =========================================================================
    {
        float px = 970.0f;
        float py = 330.0f;

        // Construct a stylized multi-faceted low-poly gemstone / crystal
        std::vector<Point> poly_pts = {
            Point(px, py - 130.0f),       // 0: Apex Top
            Point(px - 100.0f, py - 30.0f),// 1: Left Upper
            Point(px - 60.0f, py + 80.0f), // 2: Left Lower
            Point(px + 60.0f, py + 80.0f), // 3: Right Lower
            Point(px + 100.0f, py - 30.0f),// 4: Right Upper
            Point(px, py + 140.0f),       // 5: Bottom Tip
            Point(px - 20.0f, py - 10.0f), // 6: Center Left
            Point(px + 20.0f, py - 10.0f), // 7: Center Right
        };

        std::vector<Color> poly_colors = {
            Color::from_rgba8(255, 255, 255, 255), // 0: Pure white specular
            Color::from_rgba8(0, 229, 255, 255),   // 1: Cyan
            Color::from_rgba8(168, 85, 247, 255),  // 2: Purple
            Color::from_rgba8(255, 0, 110, 255),   // 3: Neon Pink
            Color::from_rgba8(0, 255, 136, 255),   // 4: Emerald
            Color::from_rgba8(30, 20, 70, 255),    // 5: Dark base
            Color::from_rgba8(100, 200, 255, 255), // 6: Light Cyan
            Color::from_rgba8(255, 150, 220, 255), // 7: Light Magenta
        };

        std::vector<uint32_t> poly_indices = {
            // Facets radiating from apex
            0, 1, 6,
            0, 6, 7,
            0, 7, 4,
            // Center facets
            1, 2, 6,
            6, 2, 5,
            6, 5, 7,
            7, 5, 3,
            7, 3, 4,
        };

        auto poly_mesh = Vertices::create_triangles(poly_pts, poly_colors, {}, poly_indices);

        // Drop shadow
        auto poly_bounds = poly_mesh.bounds();
        effects::draw_round_rect_shadow(canvas.pixmap(), poly_bounds, 20.0f, 20.0f,
            DropShadow(0.0f, 15.0f, 30.0f, Color::from_rgba8(0, 0, 0, 170)));

        // Draw Gouraud shaded mesh
        canvas.draw_vertices(poly_mesh, BlendMode::SourceOver, 1.0f);

        // Stylized wireframe edges
        Paint poly_wire(Color::from_rgba8(255, 255, 255, 130));
        Stroke pws(1.2f);
        size_t tri_count = poly_mesh.triangle_count();
        for (size_t t = 0; t < tri_count; ++t) {
            auto [i0, i1, i2] = poly_mesh.get_triangle_indices(t);
            Point p0 = poly_mesh.positions[i0];
            Point p1 = poly_mesh.positions[i1];
            Point p2 = poly_mesh.positions[i2];

            PathBuilder pb;
            pb.move_to(p0.x, p0.y);
            pb.line_to(p1.x, p1.y);
            pb.line_to(p2.x, p2.y);
            pb.close();
            auto p = pb.finish();
            if (p) canvas.stroke_path(*p, poly_wire, pws);
        }

        draw_text(860.0f, 540.0f, "GOURAUD TRIANGLE SHADING", 14.0f, 18.0f, TextColor::rgb(168, 85, 247));
        draw_text(860.0f, 565.0f, "Low-Poly Indexed Faceting • Smooth Vertex Blending\nO(1) Scanline Differential Barycentric Stepping", 11.5f, 15.0f, TextColor::rgb(160, 180, 210));
    }

    // 4. Bottom Engineering Status Bar
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
    const std::string out_bmp = "nisaba_mesh_showcase.bmp";
    if (pixmap->save_bmp(out_bmp)) {
        std::cout << "Successfully saved: " << out_bmp << std::endl;
    } else {
        std::cerr << "Failed to save BMP!" << std::endl;
        return 1;
    }

    return 0;
}

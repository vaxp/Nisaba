#include <cassert>
#include <iostream>
#include <cmath>
#include <fstream>
#include "nisaba/nisaba.hpp"
#include "nisaba/text/ttf_font.hpp"
#include "nisaba/text/buffer.hpp"
#include "nisaba/text/font_system.hpp"

using namespace nisaba;
using nisaba::gpu::GpuVertex;
using nisaba::gpu::GpuUniforms;
using nisaba::gpu::GpuBlendState;
using nisaba::gpu::GpuTessellator;
using nisaba::gpu::GpuDevice;
using nisaba::gpu::GpuBuffer;
using nisaba::gpu::GpuSurface;
using nisaba::gpu::GpuCanvas;

namespace {
const Color RED = Color::from_rgba8(255, 0, 0, 255);
const Color GREEN = Color::from_rgba8(0, 255, 0, 255);
const Color BLUE = Color::from_rgba8(0, 0, 255, 255);
const Color YELLOW = Color::from_rgba8(255, 255, 0, 255);
const Color CYAN = Color::from_rgba8(0, 255, 255, 255);
const Color MAGENTA = Color::from_rgba8(255, 0, 255, 255);

std::string resolve_test_font(std::string_view filename) {
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
} // namespace

void test_gpu_types() {
    std::cout << "Testing GPU types..." << std::endl;

    GpuVertex v1(Point(10.0f, 20.0f), RED, 1.0f);
    assert(v1.pos.x == 10.0f);
    assert(v1.pos.y == 20.0f);
    assert(v1.color == RED);
    assert(v1.coverage == 1.0f);

    GpuVertex v2(30.0f, 40.0f, 0.5f, 0.5f, BLUE, 0.0f);
    assert(v2.pos.x == 30.0f);
    assert(v2.pos.y == 40.0f);
    assert(v2.uv.x == 0.5f);
    assert(v2.uv.y == 0.5f);
    assert(v2.color == BLUE);
    assert(v2.coverage == 0.0f);

    GpuUniforms uniforms;
    assert(uniforms.viewport_width == 1.0f);
    assert(uniforms.opacity == 1.0f);

    GpuBlendState blend(BlendMode::SourceOver);
    assert(blend.blend_mode == BlendMode::SourceOver);
    assert(blend == GpuBlendState(BlendMode::SourceOver));
    assert(blend != GpuBlendState(BlendMode::Clear));

    std::cout << "  -> GPU types OK" << std::endl;
}

void test_gpu_tessellation_rect() {
    std::cout << "Testing GPU tessellation for rectangles..." << std::endl;

    auto r = Rect::from_xywh(10.0f, 20.0f, 100.0f, 50.0f);
    assert(r.has_value());

    // Non-AA rect
    std::vector<GpuVertex> verts_no_aa;
    std::vector<uint32_t> indices_no_aa;
    GpuTessellator::tessellate_rect(verts_no_aa, indices_no_aa, *r, GREEN, Transform(), false);

    assert(verts_no_aa.size() == 4);
    assert(indices_no_aa.size() == 6); // 2 triangles
    for (const auto& v : verts_no_aa) {
        assert(v.coverage == 1.0f);
        assert(v.color == GREEN);
    }

    // AA rect with fringe
    std::vector<GpuVertex> verts_aa;
    std::vector<uint32_t> indices_aa;
    GpuTessellator::tessellate_rect(verts_aa, indices_aa, *r, GREEN, Transform(), true);

    assert(verts_aa.size() == 8);        // 4 inner + 4 outer
    assert(indices_aa.size() == 6 + 24); // 2 triangles inner + 8 triangles fringe = 30 indices

    // First 4 vertices are inner (coverage 1.0)
    for (size_t i = 0; i < 4; ++i) {
        assert(verts_aa[i].coverage == 1.0f);
    }
    // Last 4 vertices are outer (coverage 0.0)
    for (size_t i = 4; i < 8; ++i) {
        assert(verts_aa[i].coverage == 0.0f);
    }

    std::cout << "  -> GPU rect tessellation OK" << std::endl;
}

void test_gpu_tessellation_circle() {
    std::cout << "Testing GPU tessellation for circles..." << std::endl;

    std::vector<GpuVertex> verts;
    std::vector<uint32_t> indices;
    GpuTessellator::tessellate_circle(verts, indices, 100.0f, 100.0f, 40.0f, BLUE, Transform(), true);

    assert(!verts.empty());
    assert(!indices.empty());
    assert(verts[0].pos.x == 100.0f && verts[0].pos.y == 100.0f); // Center
    assert(verts[0].coverage == 1.0f);

    // Verify outer vertices have coverage 0.0
    bool has_zero_coverage = false;
    for (const auto& v : verts) {
        if (v.coverage == 0.0f) {
            has_zero_coverage = true;
            break;
        }
    }
    assert(has_zero_coverage);

    std::cout << "  -> GPU circle tessellation OK" << std::endl;
}

void test_gpu_tessellation_round_rect() {
    std::cout << "Testing GPU tessellation for rounded rectangles..." << std::endl;

    auto r = Rect::from_xywh(0.0f, 0.0f, 200.0f, 100.0f);
    assert(r.has_value());

    std::vector<GpuVertex> verts;
    std::vector<uint32_t> indices;
    GpuTessellator::tessellate_round_rect(verts, indices, *r, 15.0f, 15.0f, RED, Transform(), true);

    assert(!verts.empty());
    assert(!indices.empty());
    assert(indices.size() % 3 == 0); // Triangles

    std::cout << "  -> GPU rounded rect tessellation OK" << std::endl;
}

void test_gpu_tessellation_line() {
    std::cout << "Testing GPU tessellation for stroked lines..." << std::endl;

    Stroke stroke;
    stroke.width = 10.0f;

    std::vector<GpuVertex> verts;
    std::vector<uint32_t> indices;
    GpuTessellator::tessellate_line(verts, indices, Point(10.0f, 10.0f), Point(100.0f, 10.0f), stroke, Color::WHITE, Transform(), true);

    assert(verts.size() == 8);        // 4 inner + 4 outer fringe
    assert(indices.size() == 6 + 12); // 2 inner triangles + 4 fringe triangles = 18

    std::cout << "  -> GPU line tessellation OK" << std::endl;
}

void test_gpu_tessellation_path() {
    std::cout << "Testing GPU tessellation for arbitrary paths..." << std::endl;

    PathBuilder pb;
    pb.move_to(0.0f, 0.0f);
    pb.line_to(100.0f, 0.0f);
    pb.quad_to(150.0f, 50.0f, 100.0f, 100.0f);
    pb.cubic_to(80.0f, 120.0f, 20.0f, 120.0f, 0.0f, 100.0f);
    pb.close();
    auto path = pb.finish();
    assert(path.has_value());

    std::vector<GpuVertex> verts;
    std::vector<uint32_t> indices;
    GpuTessellator::tessellate_path(verts, indices, *path, FillRule::Winding, YELLOW, Transform(), true);

    assert(!verts.empty());
    assert(!indices.empty());
    assert(indices.size() % 3 == 0);

    std::cout << "  -> GPU path tessellation OK" << std::endl;
}

void test_gpu_tessellation_mesh() {
    std::cout << "Testing GPU tessellation for 2D meshes..." << std::endl;

    std::vector<Point> pos = { Point(0.0f, 0.0f), Point(100.0f, 0.0f), Point(50.0f, 100.0f) };
    std::vector<Color> cols = { RED, GREEN, BLUE };
    std::vector<Point> uvs = { Point(0.0f, 0.0f), Point(1.0f, 0.0f), Point(0.5f, 1.0f) };
    std::vector<uint32_t> idx = { 0, 1, 2 };

    auto mesh = Vertices::create_triangles(pos, cols, uvs, idx);
    assert(mesh.is_valid());

    std::vector<GpuVertex> verts;
    std::vector<uint32_t> indices;
    GpuTessellator::tessellate_mesh(verts, indices, mesh, Transform());

    assert(verts.size() == 3);
    assert(indices.size() == 3);
    assert(verts[0].color == RED);
    assert(verts[1].color == GREEN);
    assert(verts[2].color == BLUE);

    std::cout << "  -> GPU mesh tessellation OK" << std::endl;
}

void test_gpu_device_and_buffer() {
    std::cout << "Testing GPU device and streaming buffer..." << std::endl;

    auto device = GpuDevice::create();
    assert(device != nullptr);
    assert(device->is_valid());

    std::cout << "  Device Renderer: " << device->renderer_name() << std::endl;
    std::cout << "  Device Version:  " << device->version_name() << std::endl;

    auto buffer = GpuBuffer::create(device);
    assert(buffer != nullptr);

    std::vector<GpuVertex> v = {
        GpuVertex(0.0f, 0.0f, RED),
        GpuVertex(10.0f, 0.0f, GREEN),
        GpuVertex(5.0f, 10.0f, BLUE)
    };
    std::vector<uint32_t> i = { 0, 1, 2 };

    buffer->upload_data(v, i);
    assert(buffer->vertex_capacity() >= 3);
    assert(buffer->index_capacity() >= 3);

    std::cout << "  -> GPU device and buffer OK" << std::endl;
}

void test_gpu_surface_and_canvas() {
    std::cout << "Testing GPU surface and high-level GpuCanvas API..." << std::endl;

    auto device = GpuDevice::create();
    auto surface = GpuSurface::create(device, 400, 300);
    assert(surface != nullptr);
    assert(surface->width() == 400);
    assert(surface->height() == 300);

    GpuCanvas canvas(surface);
    assert(canvas.width() == 400);
    assert(canvas.height() == 300);

    // 1. Clear
    canvas.clear(Color::from_rgba8(30, 30, 40, 255));

    // 2. Transform stack
    canvas.save();
    canvas.translate(50.0f, 50.0f);
    canvas.scale(1.5f, 1.5f);
    canvas.rotate(45.0f);
    assert(!canvas.transform().is_identity());
    canvas.restore();
    assert(canvas.transform().is_identity());

    // 3. Clipping
    auto clip_r = Rect::from_xywh(10.0f, 10.0f, 380.0f, 280.0f);
    assert(clip_r.has_value());
    canvas.clip_rect(*clip_r);
    assert(canvas.scissor_clip().has_value());
    canvas.reset_clip();
    assert(canvas.scissor_clip()->width() == 400);

    // 4. Drawing shapes
    Paint solid_paint(Color::from_rgba8(255, 100, 50, 255));
    auto rect = Rect::from_xywh(20.0f, 20.0f, 100.0f, 60.0f);
    assert(rect.has_value());
    canvas.fill_rect(*rect, solid_paint);

    Stroke stroke;
    stroke.width = 4.0f;
    canvas.stroke_rect(*rect, solid_paint, stroke);

    canvas.fill_round_rect(*rect, 10.0f, 10.0f, solid_paint);
    canvas.stroke_round_rect(*rect, 10.0f, 10.0f, solid_paint, stroke);

    canvas.fill_circle(200.0f, 150.0f, 40.0f, solid_paint);
    canvas.stroke_circle(200.0f, 150.0f, 40.0f, solid_paint, stroke);

    canvas.stroke_line(10.0f, 10.0f, 390.0f, 290.0f, solid_paint, stroke);

    // 5. Linear Gradient
    std::vector<GradientStop> stops = {
        GradientStop::create(0.0f, RED),
        GradientStop::create(1.0f, BLUE)
    };
    auto lg = LinearGradient::create(Point(0.0f, 0.0f), Point(400.0f, 300.0f), stops);
    assert(lg.has_value());
    Paint grad_paint;
    grad_paint.shader = Shader(*lg);
    canvas.fill_rect(*Rect::from_xywh(50.0f, 50.0f, 200.0f, 150.0f), grad_paint);

    // 6. Draw 2D Mesh
    std::vector<Point> mesh_pts = { Point(250.0f, 50.0f), Point(350.0f, 50.0f), Point(300.0f, 150.0f) };
    std::vector<Color> mesh_cols = { YELLOW, CYAN, MAGENTA };
    auto mesh = Vertices::create_triangles(mesh_pts, mesh_cols);
    canvas.draw_vertices(mesh);

    // 7. Flush and Readback
    canvas.flush();
    auto pixmap = surface->to_pixmap();
    assert(pixmap.has_value());
    assert(pixmap->width() == 400);
    assert(pixmap->height() == 300);

    std::cout << "  -> GPU surface and canvas OK" << std::endl;
}

void test_gpu_cpu_coexistence() {
    std::cout << "Testing seamless CPU and GPU pipeline coexistence..." << std::endl;

    // 1. CPU Canvas rendering
    auto cpu_pixmap = Pixmap::allocate(200, 200);
    assert(cpu_pixmap.has_value());
    Canvas cpu_canvas(*cpu_pixmap);
    cpu_canvas.clear(Color::WHITE);
    Paint cpu_paint(RED);
    cpu_canvas.fill_rect(*Rect::from_xywh(20.0f, 20.0f, 60.0f, 60.0f), cpu_paint);

    // 2. GPU Canvas rendering
    auto device = GpuDevice::create();
    auto gpu_surface = GpuSurface::create(device, 200, 200);
    assert(gpu_surface != nullptr);
    GpuCanvas gpu_canvas(gpu_surface);
    gpu_canvas.clear(Color::BLACK);
    Paint gpu_paint(BLUE);
    gpu_canvas.fill_rect(*Rect::from_xywh(100.0f, 100.0f, 60.0f, 60.0f), gpu_paint);
    gpu_canvas.flush();

    // 3. Upload CPU Pixmap to GPU Texture and draw on GPU Canvas
    gpu_canvas.draw_pixmap(0, 0, cpu_pixmap->as_ref());
    gpu_canvas.flush();

    // 4. Readback GPU rendering into CPU Pixmap
    auto readback = gpu_surface->to_pixmap();
    assert(readback.has_value());
    assert(readback->width() == 200);
    assert(readback->height() == 200);

    std::cout << "  -> Seamless CPU and GPU coexistence OK" << std::endl;
}

void test_gpu_text_rendering() {
    std::cout << "Testing GPU hardware text rendering..." << std::endl;

    std::string font_path = resolve_test_font("NotoSans-Regular.ttf");
    auto font = text::TtfFont::from_file(font_path);
    assert(font != nullptr);

    auto device = GpuDevice::create();
    auto surface = GpuSurface::create(device, 400, 200);
    assert(surface != nullptr);

    GpuCanvas canvas(surface);
    canvas.clear(Color::WHITE);

    // 1. Draw text directly using text::Font & Paint
    canvas.draw_text("Nisaba GPU Hardware Text", 20.0f, 60.0f, *font, Paint(RED), 28.0f);

    // 2. Register font by name on canvas context and draw with font name
    int font_id = canvas.create_font("test_sans", font_path.c_str());
    assert(font_id > 0);
    canvas.draw_text(20.0f, 120.0f, "Rendered With Font Face", Paint(BLUE), 24.0f, "test_sans");

    canvas.flush();

    // 3. Readback and verify actual text glyph pixels were drawn
    auto readback = surface->to_pixmap();
    assert(readback.has_value());
    assert(readback->width() == 400);
    assert(readback->height() == 200);

    size_t non_white_pixels = 0;
    size_t red_tinted = 0;
    size_t blue_tinted = 0;

    for (uint32_t y = 0; y < readback->height(); ++y) {
        for (uint32_t x = 0; x < readback->width(); ++x) {
            auto opt = readback->as_ref().pixel(x, y);
            if (opt && (opt->r < 240 || opt->g < 240 || opt->b < 240)) {
                non_white_pixels++;
                if (opt->r > 128 && opt->g < 100 && opt->b < 100) {
                    red_tinted++;
                }
                if (opt->b > 128 && opt->g < 100 && opt->r < 100) {
                    blue_tinted++;
                }
            }
        }
    }

    assert(non_white_pixels > 100);
    assert(red_tinted > 30);
    assert(blue_tinted > 30);

    // Verify atlas texture was flushed
    assert(canvas.context() != nullptr);
    assert(canvas.context()->atlas().isDirty() == false);

    std::cout << "  -> GPU hardware text rendering OK (" << non_white_pixels << " text pixels rendered)" << std::endl;
}

void test_gpu_text_buffer_rendering() {
    std::cout << "Testing GPU hardware text buffer & paragraph rendering..." << std::endl;

    std::string font_path = resolve_test_font("NotoSans-Regular.ttf");
    text::FontSystem font_system;
    auto fid = font_system.load_font_file(font_path);
    assert(fid.has_value());
    font_system.set_default_font(*fid);

    text::Buffer buffer(text::Metrics(20.0f, 26.0f));
    buffer.set_size(300.0f, std::nullopt);
    buffer.set_text("Nisaba Sovereign Graphics Engine hardware text layout paragraph test for ENKI.");

    auto device = GpuDevice::create();
    auto surface = GpuSurface::create(device, 350, 200);
    assert(surface != nullptr);

    GpuCanvas canvas(surface);
    canvas.clear(Color::BLACK);

    // Draw full text buffer with default green color
    canvas.draw_text_buffer(buffer, font_system, Point(20.0f, 30.0f), GREEN);
    canvas.flush();

    auto readback = surface->to_pixmap();
    assert(readback.has_value());

    size_t green_pixels = 0;
    for (uint32_t y = 0; y < readback->height(); ++y) {
        for (uint32_t x = 0; x < readback->width(); ++x) {
            auto opt = readback->as_ref().pixel(x, y);
            if (opt && opt->g > 75 && opt->r < 50 && opt->b < 50) {
                green_pixels++;
            }
        }
    }

    assert(green_pixels > 200);

    // Test text buffer with styled colored spans
    text::Attrs red_attrs;
    red_attrs.set_color(text::TextColor::rgb(255, 0, 0));
    text::Attrs blue_attrs;
    blue_attrs.set_color(text::TextColor::rgb(0, 0, 255));

    text::Buffer styled_buffer(text::Metrics(24.0f, 30.0f));
    styled_buffer.set_text("RedWord BlueWord");
    if (!styled_buffer.lines().empty()) {
        styled_buffer.lines_mut()[0].attrs_list_mut().add_span(0, 7, red_attrs);
        styled_buffer.lines_mut()[0].attrs_list_mut().add_span(8, 16, blue_attrs);
    }

    canvas.clear(Color::BLACK);
    canvas.draw_text_buffer(styled_buffer, font_system, Point(20.0f, 40.0f), Color::WHITE);
    canvas.flush();

    auto styled_readback = surface->to_pixmap();
    assert(styled_readback.has_value());

    size_t red_glyph_pixels = 0;
    size_t blue_glyph_pixels = 0;
    for (uint32_t y = 0; y < styled_readback->height(); ++y) {
        for (uint32_t x = 0; x < styled_readback->width(); ++x) {
            auto opt = styled_readback->as_ref().pixel(x, y);
            if (opt && opt->r > 100 && opt->g < 60 && opt->b < 60) red_glyph_pixels++;
            if (opt && opt->b > 100 && opt->g < 60 && opt->r < 60) blue_glyph_pixels++;
        }
    }

    assert(red_glyph_pixels > 30);
    assert(blue_glyph_pixels > 30);

    std::cout << "  -> GPU hardware text buffer rendering OK (Red: " << red_glyph_pixels << ", Blue: " << blue_glyph_pixels << ")" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Nisaba GPU Rendering Pipeline Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_gpu_types();
    test_gpu_tessellation_rect();
    test_gpu_tessellation_circle();
    test_gpu_tessellation_round_rect();
    test_gpu_tessellation_line();
    test_gpu_tessellation_path();
    test_gpu_tessellation_mesh();
    test_gpu_device_and_buffer();
    test_gpu_surface_and_canvas();
    test_gpu_cpu_coexistence();
    test_gpu_text_rendering();
    test_gpu_text_buffer_rendering();

    std::cout << "========================================" << std::endl;
    std::cout << "All Nisaba GPU pipeline tests PASSED (12/12)!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

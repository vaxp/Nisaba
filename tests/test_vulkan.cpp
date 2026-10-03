#include <cassert>
#include <iostream>
#include <cmath>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "nisaba/gpu/vulkan_device.hpp"
#include "nisaba/gpu/vulkan_surface.hpp"
#include "nisaba/gpu/vulkan_renderer.hpp"

using namespace nisaba;
using nisaba::gpu::VulkanDevice;
using nisaba::gpu::VulkanSurface;
using nisaba::gpu::GpuCanvas;
using nisaba::gpu::GpuBackendType;

namespace {
const Color RED = Color::from_rgba8(255, 0, 0, 255);
const Color GREEN = Color::from_rgba8(0, 255, 0, 255);
const Color BLUE = Color::from_rgba8(0, 0, 255, 255);
const Color YELLOW = Color::from_rgba8(255, 255, 0, 255);
const Color CYAN = Color::from_rgba8(0, 255, 255, 255);
const Color MAGENTA = Color::from_rgba8(255, 0, 255, 255);
const Color BG_COLOR = Color::from_rgba8(25, 30, 45, 255);
} // namespace

void test_vulkan_device_init() {
    std::cout << "Testing Vulkan device initialization..." << std::endl;

    auto device = VulkanDevice::create();
    assert(device != nullptr);
    assert(device->is_valid());
    assert(device->backend_type() == GpuBackendType::Vulkan);

    std::cout << "  -> Vulkan Device Name:    " << device->renderer_name() << std::endl;
    std::cout << "  -> Vulkan API Version:    " << device->version_name() << std::endl;
    std::cout << "  -> Graphics Queue Family: " << device->graphics_queue_family() << std::endl;

    assert(device->instance() != VK_NULL_HANDLE);
    assert(device->device() != VK_NULL_HANDLE);
    assert(device->graphics_queue() != VK_NULL_HANDLE);

    std::cout << "  -> Vulkan device initialization OK" << std::endl;
}

void test_vulkan_surface_and_canvas() {
    std::cout << "Testing Vulkan surface allocation and GpuCanvas API..." << std::endl;

    auto device = VulkanDevice::create();
    assert(device != nullptr && device->is_valid());

    auto surface = VulkanSurface::create(device, 400, 300, 4); // 4x MSAA
    assert(surface != nullptr);
    assert(surface->width() == 400);
    assert(surface->height() == 300);
    assert(surface->samples() >= 4);
    assert(surface->backend_type() == GpuBackendType::Vulkan);
    assert(surface->render_pass() != VK_NULL_HANDLE);
    assert(surface->framebuffer() != VK_NULL_HANDLE);

    GpuCanvas canvas(surface);
    assert(canvas.width() == 400);
    assert(canvas.height() == 300);

    // 1. Clear with solid background
    canvas.clear(BG_COLOR);

    // 2. Transform stack
    canvas.save();
    canvas.translate(40.0f, 30.0f);
    canvas.scale(1.2f, 1.2f);
    canvas.rotate(15.0f);
    assert(!canvas.transform().is_identity());
    canvas.restore();
    assert(canvas.transform().is_identity());

    // 3. Scissor clipping
    auto clip_r = Rect::from_xywh(10.0f, 10.0f, 380.0f, 280.0f);
    assert(clip_r.has_value());
    canvas.clip_rect(*clip_r);
    assert(canvas.scissor_clip().has_value());
    canvas.reset_clip();

    // 4. Draw basic primitives
    Paint solid_red(RED);
    auto rect1 = Rect::from_xywh(20.0f, 20.0f, 80.0f, 60.0f);
    assert(rect1.has_value());
    canvas.fill_rect(*rect1, solid_red);

    Stroke stroke;
    stroke.width = 3.0f;
    Paint solid_green(GREEN);
    canvas.stroke_rect(*rect1, solid_green, stroke);

    // 5. Rounded rects and circles
    Paint solid_blue(BLUE);
    auto rrect = Rect::from_xywh(120.0f, 20.0f, 100.0f, 60.0f);
    assert(rrect.has_value());
    canvas.fill_round_rect(*rrect, 12.0f, 12.0f, solid_blue);
    canvas.stroke_round_rect(*rrect, 12.0f, 12.0f, solid_red, stroke);

    canvas.fill_circle(280.0f, 60.0f, 30.0f, solid_green);
    canvas.stroke_circle(280.0f, 60.0f, 30.0f, solid_blue, stroke);

    canvas.stroke_line(10.0f, 110.0f, 390.0f, 110.0f, solid_red, stroke);

    // 6. Linear Gradient with analytical SDF
    std::vector<GradientStop> stops = {
        GradientStop::create(0.0f, YELLOW),
        GradientStop::create(1.0f, MAGENTA)
    };
    auto lg = LinearGradient::create(Point(50.0f, 130.0f), Point(250.0f, 200.0f), stops);
    assert(lg.has_value());
    Paint grad_paint;
    grad_paint.shader = Shader(*lg);
    auto grad_rect = Rect::from_xywh(50.0f, 130.0f, 200.0f, 70.0f);
    assert(grad_rect.has_value());
    canvas.fill_rect(*grad_rect, grad_paint);

    // 7. Complex Bézier Path
    PathBuilder pb;
    pb.move_to(270.0f, 140.0f);
    pb.line_to(370.0f, 140.0f);
    pb.quad_to(390.0f, 175.0f, 370.0f, 210.0f);
    pb.cubic_to(340.0f, 220.0f, 300.0f, 220.0f, 270.0f, 180.0f);
    pb.close();
    auto path = pb.finish();
    assert(path.has_value());
    Paint path_paint(CYAN);
    canvas.fill_path(*path, path_paint);

    // 8. 2D Mesh triangles
    std::vector<Point> mesh_pts = { Point(100.0f, 220.0f), Point(200.0f, 220.0f), Point(150.0f, 280.0f) };
    std::vector<Color> mesh_cols = { RED, GREEN, BLUE };
    auto mesh = Vertices::create_triangles(mesh_pts, mesh_cols);
    canvas.draw_vertices(mesh);

    // 9. Flush and Readback
    canvas.flush();

    auto pixmap = surface->to_pixmap();
    assert(pixmap.has_value());
    assert(pixmap->width() == 400);
    assert(pixmap->height() == 300);

    // Verify center of rect1 is red
    auto p_red = pixmap->pixel(60, 50);
    assert(p_red.has_value());
    assert(p_red->alpha() == 255);
    assert(p_red->red() > 200); // Red dominant

    // Verify center of circle is green
    auto p_circle = pixmap->pixel(280, 60);
    assert(p_circle.has_value());
    assert(p_circle->alpha() == 255);
    assert(p_circle->green() > 200); // Green dominant

    std::cout << "  -> Vulkan surface, canvas, primitives, and pixel readback OK" << std::endl;
}

void test_vulkan_cpu_coexistence() {
    std::cout << "Testing CPU Pixmap and Vulkan pipeline coexistence..." << std::endl;

    // 1. Draw a pattern on CPU Canvas
    auto cpu_pixmap = Pixmap::allocate(100, 100);
    assert(cpu_pixmap.has_value());
    Canvas cpu_canvas(*cpu_pixmap);
    cpu_canvas.clear(YELLOW);
    Paint cpu_paint(BLUE);
    cpu_canvas.fill_rect(*Rect::from_xywh(20.0f, 20.0f, 60.0f, 60.0f), cpu_paint);

    // 2. Render on Vulkan surface
    auto device = VulkanDevice::create();
    auto surface = VulkanSurface::create(device, 200, 200);
    GpuCanvas vk_canvas(surface);
    vk_canvas.clear(Color::BLACK);

    // 3. Draw CPU pixmap onto Vulkan Canvas
    vk_canvas.draw_pixmap(50, 50, cpu_pixmap->as_ref());
    vk_canvas.flush();

    // 4. Readback and verify
    auto readback = surface->to_pixmap();
    assert(readback.has_value());
    assert(readback->width() == 200);
    assert(readback->height() == 200);

    // Background outside the drawn image should be black
    auto bg_pix = readback->pixel(10, 10);
    assert(bg_pix.has_value());
    assert(bg_pix->red() == 0 && bg_pix->green() == 0 && bg_pix->blue() == 0);

    // Yellow corner of the drawn CPU image (at x=55, y=55)
    auto yellow_pix = readback->pixel(55, 55);
    assert(yellow_pix.has_value());
    assert(yellow_pix->red() > 200 && yellow_pix->green() > 200);

    // Blue center of the drawn CPU image (at x=90, y=90)
    auto blue_pix = readback->pixel(90, 90);
    assert(blue_pix.has_value());
    assert(blue_pix->blue() > 200);

    std::cout << "  -> CPU and Vulkan coexistence OK" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Running Nisaba Vulkan GPU Backend Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_vulkan_device_init();
    test_vulkan_surface_and_canvas();
    test_vulkan_cpu_coexistence();

    std::cout << "========================================" << std::endl;
    std::cout << "All Nisaba Vulkan GPU tests PASSED (3/3)!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

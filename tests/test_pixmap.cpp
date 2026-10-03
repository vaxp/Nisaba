#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;

void test_pixmap_alloc_and_fill() {
    auto pm = Pixmap::allocate(100, 50);
    assert(pm.has_value());
    assert(pm->width() == 100);
    assert(pm->height() == 50);
    assert(pm->data_len() == 100 * 50 * 4);

    // Initial state must be transparent black (0, 0, 0, 0)
    for (size_t i = 0; i < pm->data_len(); ++i) {
        assert(pm->data()[i] == 0);
    }

    // Fill with solid blue (0, 0, 255, 255)
    pm->fill(Color::from_rgba8(0, 0, 255, 255));
    auto p0 = pm->pixel(0, 0);
    assert(p0.has_value());
    assert(p0->red() == 0);
    assert(p0->green() == 0);
    assert(p0->blue() == 255);
    assert(p0->alpha() == 255);

    // Check bounds
    assert(!pm->pixel(100, 50).has_value());
    assert(!pm->pixel(100, 0).has_value());
    assert(!pm->pixel(0, 50).has_value());

    std::cout << "[PASS] Pixmap allocation, bounds and fill tests passed\n";
}

void test_drm_zero_copy_buffer() {
    // Simulate a pre-allocated DRM/KMS dumb buffer or native window surface
    constexpr uint32_t W = 64;
    constexpr uint32_t H = 64;
    std::vector<uint8_t> dumb_buffer(W * H * 4, 0);

    // Wrap zero-copy
    auto mut_view = PixmapMut::from_bytes(dumb_buffer.data(), dumb_buffer.size(), W, H);
    assert(mut_view.has_value());
    assert(mut_view->width() == W);
    assert(mut_view->height() == H);

    // Render directly into dumb buffer
    mut_view->fill(Color::from_rgba8(255, 128, 0, 255));

    // Verify raw memory buffer was modified directly with zero copies
    assert(dumb_buffer[0] == 255);
    assert(dumb_buffer[1] == 128);
    assert(dumb_buffer[2] == 0);
    assert(dumb_buffer[3] == 255);

    std::cout << "[PASS] Zero-copy native buffer wrapping (DRM/KMS) passed\n";
}

void test_subpixmap_and_clone() {
    auto pm = Pixmap::allocate(20, 20);
    assert(pm.has_value());
    pm->fill(Color::from_rgba8(10, 20, 30, 255));

    auto sub = pm->as_mut().subpixmap(*IntRect::from_xywh(5, 5, 10, 10));
    assert(sub.has_value());
    assert(sub->size.width() == 10);
    assert(sub->size.height() == 10);
    assert(sub->real_width == 20);

    // Clone rect
    auto cloned = pm->clone_rect(*IntRect::from_xywh(2, 2, 4, 4));
    assert(cloned.has_value());
    assert(cloned->width() == 4);
    assert(cloned->height() == 4);
    assert(cloned->pixel(0, 0)->red() == 10);

    std::cout << "[PASS] SubPixmapMut and clone_rect passed\n";
}

void test_image_export() {
    auto pm = Pixmap::allocate(16, 16);
    assert(pm.has_value());
    pm->fill(Color::from_rgba8(255, 0, 0, 255)); // red

    bool ppm_ok = pm->save_ppm("test_output.ppm");
    assert(ppm_ok);

    bool bmp_ok = pm->save_bmp("test_output.bmp");
    assert(bmp_ok);

    // Verify BMP file exists and starts with 'BM'
    std::ifstream bmp_in("test_output.bmp", std::ios::binary);
    assert(bmp_in.is_open());
    char magic[2];
    bmp_in.read(magic, 2);
    assert(magic[0] == 'B' && magic[1] == 'M');

    std::cout << "[PASS] Zero-dependency PPM and BMP export passed\n";
}

int main() {
    std::cout << "Running Nisaba Pixmap tests...\n";
    test_pixmap_alloc_and_fill();
    test_drm_zero_copy_buffer();
    test_subpixmap_and_clone();
    test_image_export();
    std::cout << "All Pixmap tests passed successfully!\n";
    return 0;
}

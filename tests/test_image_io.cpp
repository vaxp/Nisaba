#include <cassert>
#include <iostream>
#include <vector>
#include "nisaba/image/image_io.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;
using namespace nisaba::image;

void test_format_detection_and_auto_io() {
    std::cout << "[TEST] Format detection and unified Image I/O... ";

    uint32_t w = 32;
    uint32_t h = 32;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    pixmap.fill(*Color::from_rgba(40, 180, 99, 255));

    // Save as PNG via auto extension
    std::string png_path = "/tmp/test_unified_auto.png";
    bool ok_png = pixmap.save_image(png_path);
    assert(ok_png);

    // Save as JPEG via auto extension
    std::string jpg_path = "/tmp/test_unified_auto.jpg";
    bool ok_jpg = pixmap.save_image(jpg_path);
    assert(ok_jpg);

    // Auto-load PNG without specifying format
    auto loaded_png = Pixmap::load_file(png_path);
    assert(loaded_png.has_value());
    assert(loaded_png->width() == w);
    assert(loaded_png->height() == h);

    // Auto-load JPEG without specifying format
    auto loaded_jpg = Pixmap::load_file(jpg_path);
    assert(loaded_jpg.has_value());
    assert(loaded_jpg->width() == w);
    assert(loaded_jpg->height() == h);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Running Nisaba Unified Image I/O Tests ===\n";
    test_format_detection_and_auto_io();
    std::cout << "=== All Image I/O Tests Passed! ===\n";
    return 0;
}

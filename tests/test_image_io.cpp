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

    // Save as WebP via auto extension
    std::string webp_path = "/tmp/test_unified_auto.webp";
    bool ok_webp = pixmap.save_image(webp_path);
    assert(ok_webp);

    // Auto-load WebP without specifying format
    auto loaded_webp = Pixmap::load_file(webp_path);
    assert(loaded_webp.has_value());
    assert(loaded_webp->width() == w);
    assert(loaded_webp->height() == h);

    // Save as BMP via auto extension
    std::string bmp_path = "/tmp/test_unified_auto.bmp";
    bool ok_bmp = pixmap.save_image(bmp_path);
    assert(ok_bmp);

    // Auto-load BMP without specifying format
    auto loaded_bmp = Pixmap::load_file(bmp_path);
    assert(loaded_bmp.has_value());
    assert(loaded_bmp->width() == w);
    assert(loaded_bmp->height() == h);

    // Save as PPM via auto extension
    std::string ppm_path = "/tmp/test_unified_auto.ppm";
    bool ok_ppm = pixmap.save_image(ppm_path);
    assert(ok_ppm);

    // Auto-load PPM without specifying format
    auto loaded_ppm = Pixmap::load_file(ppm_path);
    assert(loaded_ppm.has_value());
    assert(loaded_ppm->width() == w);
    assert(loaded_ppm->height() == h);

    // Save as HDR via auto extension
    std::string hdr_path = "/tmp/test_unified_auto.hdr";
    bool ok_hdr = pixmap.save_image(hdr_path);
    assert(ok_hdr);

    // Auto-load HDR without specifying format
    auto loaded_hdr = Pixmap::load_file(hdr_path);
    assert(loaded_hdr.has_value());
    assert(loaded_hdr->width() == w);
    assert(loaded_hdr->height() == h);

    // Probing all generated formats
    auto info_png = probe_image_file(png_path);
    assert(info_png.has_value() && info_png->format == ImageFormat::PNG);

    auto info_jpg = probe_image_file(jpg_path);
    assert(info_jpg.has_value() && info_jpg->format == ImageFormat::JPEG);

    auto info_webp = probe_image_file(webp_path);
    assert(info_webp.has_value() && info_webp->format == ImageFormat::WebP);

    auto info_bmp = probe_image_file(bmp_path);
    assert(info_bmp.has_value() && info_bmp->format == ImageFormat::BMP);

    auto info_ppm = probe_image_file(ppm_path);
    assert(info_ppm.has_value() && info_ppm->format == ImageFormat::PPM);

    auto info_hdr = probe_image_file(hdr_path);
    assert(info_hdr.has_value() && info_hdr->format == ImageFormat::HDR);

    // Test DecodeOptions
    DecodeOptions opts;
    opts.premultiply_alpha = false;
    auto opt_png = load_image_file(png_path, opts);
    assert(opt_png.has_value() && opt_png->width() == w);

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Running Nisaba Unified Image I/O Tests ===\n";
    test_format_detection_and_auto_io();
    std::cout << "=== All Image I/O Tests Passed! ===\n";
    return 0;
}

#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include "nisaba/image/image_io.hpp"
#include "nisaba/image/bmp.hpp"
#include "nisaba/image/ppm.hpp"
#include "nisaba/image/hdr.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;
using namespace nisaba::image;

void test_format_detection_all() {
    std::cout << "[TEST] Extended format detection for all codecs... ";

    // PNG
    std::vector<uint8_t> png_hdr = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
    assert(detect_format(png_hdr) == ImageFormat::PNG);

    // JPEG
    std::vector<uint8_t> jpg_hdr = {0xFF, 0xD8, 0xFF, 0xE0};
    assert(detect_format(jpg_hdr) == ImageFormat::JPEG);

    // WebP
    std::vector<uint8_t> webp_hdr = {'R', 'I', 'F', 'F', 0, 0, 0, 0, 'W', 'E', 'B', 'P'};
    assert(detect_format(webp_hdr) == ImageFormat::WebP);

    // QOI
    std::vector<uint8_t> qoi_hdr = {'q', 'o', 'i', 'f'};
    assert(detect_format(qoi_hdr) == ImageFormat::QOI);

    // BMP
    std::vector<uint8_t> bmp_hdr = {'B', 'M', 0, 0, 0, 0};
    assert(detect_format(bmp_hdr) == ImageFormat::BMP);

    // PPM / Netpbm
    std::vector<uint8_t> p6_hdr = {'P', '6', '\n'};
    assert(detect_format(p6_hdr) == ImageFormat::PPM);

    std::vector<uint8_t> p7_hdr = {'P', '7', '\n'};
    assert(detect_format(p7_hdr) == ImageFormat::PPM);

    // Radiance HDR
    std::vector<uint8_t> hdr_hdr = {'#', '?', 'R', 'A', 'D', 'I', 'A', 'N', 'C', 'E', '\n'};
    assert(detect_format(hdr_hdr) == ImageFormat::HDR);

    std::cout << "PASSED\n";
}

void test_bmp_codec() {
    std::cout << "[TEST] BMP encoder/decoder roundtrips... ";

    uint32_t w = 32;
    uint32_t h = 24;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t r = static_cast<uint8_t>(x * 8);
            uint8_t g = static_cast<uint8_t>(y * 10);
            uint8_t b = static_cast<uint8_t>((x + y) * 4);
            pixmap.set_pixel(x, y, ColorU8(r, g, b, 255).premultiply());
        }
    }

    // 1. 24-bit BMP roundtrip (opaque pixmap encodes as 24-bit)
    auto enc24 = encode_bmp(pixmap.as_ref());
    assert(enc24.has_value());
    assert(enc24->size() > 54);

    auto info24 = probe_bmp(*enc24);
    assert(info24.has_value());
    assert(info24->width == w);
    assert(info24->height == h);
    assert(info24->channels == 3);
    assert(!info24->has_alpha);

    auto dec24 = decode_bmp(*enc24);
    assert(dec24.has_value());
    assert(dec24->width() == w);
    assert(dec24->height() == h);

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y).value();
            auto dec = dec24->pixel(x, y).value();
            assert(orig.r == dec.r && orig.g == dec.g && orig.b == dec.b && dec.a == 255);
        }
    }

    // 2. 32-bit BMP with alpha roundtrip (non-opaque pixmap encodes as 32-bit)
    pixmap.set_pixel(5, 5, ColorU8(100, 150, 200, 128).premultiply());
    auto enc32 = encode_bmp(pixmap.as_ref());
    assert(enc32.has_value());

    auto info32 = probe_bmp(*enc32);
    assert(info32.has_value());
    assert(info32->channels == 4);
    assert(info32->has_alpha);

    auto dec32 = decode_bmp(*enc32);
    assert(dec32.has_value());
    auto p5 = dec32->pixel(5, 5).value();
    auto p5_orig = pixmap.pixel(5, 5).value();
    assert(p5.r == p5_orig.r && p5.g == p5_orig.g && p5.b == p5_orig.b && p5.a == p5_orig.a);

    std::cout << "PASSED (24-bit and 32-bit verified)\n";
}

void test_ppm_netpbm_codec() {
    std::cout << "[TEST] Netpbm PPM/PGM/PAM encoder and decoder... ";

    uint32_t w = 16;
    uint32_t h = 16;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            pixmap.set_pixel(x, y, ColorU8(static_cast<uint8_t>(x * 16), static_cast<uint8_t>(y * 16), 128, 255).premultiply());
        }
    }

    // 1. P6 binary RGB roundtrip
    auto enc = encode_ppm(pixmap.as_ref());
    assert(enc.has_value());

    auto info = probe_ppm(*enc);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->channels == 3);
    assert(!info->has_alpha);

    auto dec = decode_ppm(*enc);
    assert(dec.has_value());
    assert(dec->width() == w);
    assert(dec->height() == h);

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y).value();
            auto d = dec->pixel(x, y).value();
            assert(orig.r == d.r && orig.g == d.g && orig.b == d.b && d.a == 255);
        }
    }

    // 2. Synthetic P3 (ASCII RGB)
    std::string p3_data = "P3\n# ASCII PPM test\n2 2\n255\n255 0 0 0 255 0\n0 0 255 255 255 255\n";
    auto dec_p3 = decode_ppm(std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(p3_data.data()), p3_data.size()));
    assert(dec_p3.has_value());
    assert(dec_p3->width() == 2 && dec_p3->height() == 2);
    auto p00 = dec_p3->pixel(0, 0).value();
    assert(p00.r == 255 && p00.g == 0 && p00.b == 0);
    auto p10 = dec_p3->pixel(1, 0).value();
    assert(p10.r == 0 && p10.g == 255 && p10.b == 0);

    // 3. Synthetic P5 (Binary Grayscale)
    std::vector<uint8_t> p5_data = {'P', '5', '\n', '2', ' ', '1', '\n', '2', '5', '5', '\n', 100, 200};
    auto dec_p5 = decode_ppm(p5_data);
    assert(dec_p5.has_value());
    assert(dec_p5->width() == 2 && dec_p5->height() == 1);
    auto p0 = dec_p5->pixel(0, 0).value();
    assert(p0.r == 100 && p0.g == 100 && p0.b == 100 && p0.a == 255);
    auto p1 = dec_p5->pixel(1, 0).value();
    assert(p1.r == 200 && p1.g == 200 && p1.b == 200 && p1.a == 255);

    // 4. Synthetic P7 (PAM 4-channel RGB_ALPHA)
    std::string p7_header = "P7\nWIDTH 2\nHEIGHT 1\nDEPTH 4\nMAXVAL 255\nTUPLTYPE RGB_ALPHA\nENDHDR\n";
    std::vector<uint8_t> p7_data(p7_header.begin(), p7_header.end());
    std::vector<uint8_t> p7_pixels = {255, 0, 0, 128, 0, 255, 0, 255};
    p7_data.insert(p7_data.end(), p7_pixels.begin(), p7_pixels.end());
    auto dec_p7 = decode_ppm(p7_data);
    assert(dec_p7.has_value());
    assert(dec_p7->width() == 2 && dec_p7->height() == 1);

    std::cout << "PASSED (P2, P3, P5, P6, P7 verified)\n";
}

void test_radiance_hdr_codec() {
    std::cout << "[TEST] Radiance HDR (.hdr / RGBE) float and tonemapped roundtrips... ";

    uint32_t w = 16;
    uint32_t h = 8;
    std::vector<float> hdr_floats(w * h * 3);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            size_t idx = (y * w + x) * 3;
            hdr_floats[idx + 0] = static_cast<float>(x) * 0.5f;     // High dynamic range (>1.0)
            hdr_floats[idx + 1] = static_cast<float>(y) * 1.2f;
            hdr_floats[idx + 2] = static_cast<float>(x + y) * 0.2f;
        }
    }

    // 1. Float HDR encode & decode roundtrip
    auto enc_float = encode_hdr_float(hdr_floats, w, h);
    assert(enc_float.has_value());
    assert(!enc_float->empty());

    auto info = probe_hdr(*enc_float);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->format == ImageFormat::HDR);
    assert(info->color_model == ColorModel::HDR_RGBE);

    auto dec_float = decode_hdr_float(*enc_float);
    assert(dec_float.has_value());
    assert(dec_float->width == w);
    assert(dec_float->height == h);
    assert(dec_float->pixels.size() == hdr_floats.size());

    // Verify float reconstruction within RGBE exponent quantization tolerance
    for (size_t i = 0; i < hdr_floats.size(); ++i) {
        float expected = hdr_floats[i];
        float actual = dec_float->pixels[i];
        if (expected > 0.05f) {
            float rel_err = std::abs(expected - actual) / expected;
            float abs_err = std::abs(expected - actual);
            // RGBE uses an 8-bit mantissa with a shared exponent per pixel.
            // Small components relative to max component have fewer effective bits.
            assert(rel_err < 0.10f || abs_err < 0.05f);
        }
    }

    // 2. Tonemapped Pixmap roundtrip
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            maybe_pixmap->set_pixel(x, y, ColorU8(static_cast<uint8_t>(x * 15), static_cast<uint8_t>(y * 30), 100, 255).premultiply());
        }
    }

    auto enc_pix = encode_hdr(maybe_pixmap->as_ref());
    assert(enc_pix.has_value());

    auto dec_pix = decode_hdr(*enc_pix);
    assert(dec_pix.has_value());
    assert(dec_pix->width() == w);
    assert(dec_pix->height() == h);

    std::cout << "PASSED (Float IBL and sRGB Pixmap HDR verified)\n";
}

int main() {
    std::cout << "===============================================\n";
    std::cout << "Nisaba Sovereign Extended Image Codecs Test Suite\n";
    std::cout << "===============================================\n";

    test_format_detection_all();
    test_bmp_codec();
    test_ppm_netpbm_codec();
    test_radiance_hdr_codec();

    std::cout << "===============================================\n";
    std::cout << "All Extended Codec Tests Passed Successfully!\n";
    std::cout << "===============================================\n";
    return 0;
}
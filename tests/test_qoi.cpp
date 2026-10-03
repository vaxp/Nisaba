#include <cassert>
#include <iostream>
#include <vector>
#include <cstdio>
#include "nisaba/image/qoi.hpp"
#include "nisaba/image/image_io.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;
using namespace nisaba::image;

void test_qoi_roundtrip() {
    std::cout << "[TEST] QOI encoding and decoding roundtrip... ";

    uint32_t w = 80;
    uint32_t h = 80;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    // Fill with rich pattern: solid blocks (runs), small delta gradients (diff/luma), index reuse, alpha
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t r = static_cast<uint8_t>(x * 3);
            uint8_t g = static_cast<uint8_t>(y * 3);
            uint8_t b = static_cast<uint8_t>((x + y) * 2);
            uint8_t a = (y < 40) ? 255 : static_cast<uint8_t>(100 + x);
            ColorU8 c(r, g, b, a);
            pixmap.set_pixel(x, y, c.premultiply());
        }
    }

    // 1. Encode to memory
    auto encoded = encode_qoi(pixmap.as_ref());
    assert(encoded.has_value());
    assert(!encoded->empty());
    assert(encoded->size() > 14 + 8);

    // 2. Probe header
    auto info = probe_qoi(*encoded);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->channels == 4);
    assert(info->has_alpha == true);
    assert(info->format == ImageFormat::QOI);

    // 3. Decode back
    auto decoded = decode_qoi(*encoded);
    assert(decoded.has_value());
    assert(decoded->width() == w);
    assert(decoded->height() == h);

    // 4. Verify pixel values match exactly
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig_px = pixmap.pixel(x, y).value();
            auto dec_px = decoded->pixel(x, y).value();
            assert(orig_px == dec_px);
        }
    }

    // 5. Test File Save and Load
    const std::string tmp_file = "test_temp_roundtrip.qoi";
    bool save_ok = pixmap.save_qoi(tmp_file);
    assert(save_ok);

    auto loaded = Pixmap::load_qoi(tmp_file);
    assert(loaded.has_value());
    assert(loaded->width() == w);
    assert(loaded->height() == h);

    // Test generic magic-byte loader
    auto generic_loaded = Pixmap::load_file(tmp_file);
    assert(generic_loaded.has_value());
    assert(generic_loaded->width() == w);
    assert(generic_loaded->height() == h);

    std::remove(tmp_file.c_str());
    std::cout << "PASSED\n";
}

void test_qoi_rle_runs() {
    std::cout << "[TEST] QOI RLE long runs (> 62 pixels)... ";

    uint32_t w = 200;
    uint32_t h = 2; // 400 pixels of identical solid red
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    ColorU8 red(255, 0, 0, 255);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            pixmap.set_pixel(x, y, red.premultiply());
        }
    }

    auto encoded = encode_qoi(pixmap.as_ref());
    assert(encoded.has_value());
    // Since 400 identical pixels compress into runs of at most 62, output should be tiny!
    // 14 bytes header + 1 RGB op + ~7 RUN ops + 8 padding = ~30 bytes
    assert(encoded->size() < 50);

    auto decoded = decode_qoi(*encoded);
    assert(decoded.has_value());
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            assert(decoded->pixel(x, y).value() == red.premultiply());
        }
    }

    std::cout << "PASSED\n";
}

void test_qoi_error_resilience() {
    std::cout << "[TEST] QOI malformed data resilience... ";

    // Empty or truncated stream
    uint8_t short_data[10] = {'q', 'o', 'i', 'f', 0, 0, 0, 10, 0, 0};
    auto res_probe = probe_qoi(std::span<const uint8_t>(short_data, 10));
    assert(!res_probe.has_value());

    auto res_decode = decode_qoi(std::span<const uint8_t>(short_data, 10));
    assert(!res_decode.has_value());

    // Invalid signature
    uint8_t bad_magic[24] = {'b', 'a', 'd', '!', 0, 0, 0, 10, 0, 0, 0, 10, 4, 0};
    auto res_bad = probe_qoi(std::span<const uint8_t>(bad_magic, 24));
    assert(!res_bad.has_value());

    std::cout << "PASSED\n";
}

int main() {
    std::cout << "=== Running Sovereign QOI Codec Test Suite ===\n";
    test_qoi_roundtrip();
    test_qoi_rle_runs();
    test_qoi_error_resilience();
    std::cout << "=== All QOI Tests Passed Successfully! ===\n";
    return 0;
}

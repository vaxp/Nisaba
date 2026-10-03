#include <cassert>
#include <iostream>
#include <vector>
#include "nisaba/image/png.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;
using namespace nisaba::image;

void test_png_roundtrip() {
    std::cout << "[TEST] PNG encoding and decoding roundtrip... ";

    uint32_t w = 64;
    uint32_t h = 64;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    // Fill with known pattern: solid colors, transparency, and gradients
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t r = static_cast<uint8_t>(x * 4);
            uint8_t g = static_cast<uint8_t>(y * 4);
            uint8_t b = static_cast<uint8_t>((x + y) * 2);
            uint8_t a = (x < 32) ? 255 : 128; // Half opaque, half translucent
            ColorU8 c(r, g, b, a);
            pixmap.set_pixel(x, y, c.premultiply());
        }
    }

    // 1. Encode to memory
    auto encoded = encode_png(pixmap.as_ref(), 6);
    assert(encoded.has_value());
    assert(!encoded->empty());

    // 2. Probe header
    auto info = probe_png(*encoded);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->channels == 4);
    assert(info->has_alpha == true);
    assert(info->format == ImageFormat::PNG);

    // 3. Decode back
    auto decoded = decode_png(*encoded);
    assert(decoded.has_value());
    assert(decoded->width() == w);
    assert(decoded->height() == h);

    // 4. Verify pixel values match
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig_px = pixmap.pixel(x, y).value();
            auto dec_px = decoded->pixel(x, y).value();
            assert(orig_px == dec_px);
        }
    }

    // 5. Test File Save and Load
    std::string test_path = "/tmp/test_nisaba_sovereign.png";
    bool saved = pixmap.save_png(test_path);
    assert(saved);

    auto loaded = Pixmap::load_png(test_path);
    assert(loaded.has_value());
    assert(loaded->width() == w);
    assert(loaded->height() == h);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            assert(pixmap.pixel(x, y).value() == loaded->pixel(x, y).value());
        }
    }

    std::cout << "PASSED (Encoded Size: " << encoded->size() << " bytes)\n";
}

void test_load_real_vaxp_logo() {
    std::cout << "[TEST] Loading real-world organization logo vaxp.png... ";
    auto logo = Pixmap::load_file("assets/vaxp.png");
    if (!logo) logo = Pixmap::load_file("../assets/vaxp.png");
    if (!logo) logo = Pixmap::load_file("vaxp.png");
    if (!logo) logo = Pixmap::load_file("../vaxp.png");
    assert(logo.has_value());
    assert(logo->width() == 1024);
    assert(logo->height() == 1024);
    std::cout << "PASSED! Successfully decoded 1024x1024 palette+transparency PNG!\n";
}

int main() {
    std::cout << "=== Running Nisaba Sovereign PNG Tests ===\n";
    test_png_roundtrip();
    test_load_real_vaxp_logo();
    std::cout << "=== All PNG Tests Passed! ===\n";
    return 0;
}

#include <cassert>
#include <iostream>
#include <vector>
#include <cmath>
#include "nisaba/image/jpeg.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;
using namespace nisaba::image;

void test_jpeg_roundtrip() {
    std::cout << "[TEST] JPEG encoding and decoding roundtrip... ";

    uint32_t w = 64;
    uint32_t h = 64;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    // Fill with smooth gradient
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t r = static_cast<uint8_t>(x * 255 / w);
            uint8_t g = static_cast<uint8_t>(y * 255 / h);
            uint8_t b = static_cast<uint8_t>((x + y) * 255 / (w + h));
            ColorU8 c(r, g, b, 255);
            pixmap.set_pixel(x, y, c.premultiply());
        }
    }

    // 1. Encode to memory
    auto encoded = encode_jpeg(pixmap.as_ref(), 90);
    assert(encoded.has_value());
    assert(!encoded->empty());

    // 2. Probe header
    auto info = probe_jpeg(*encoded);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->channels == 3);
    assert(info->format == ImageFormat::JPEG);

    // 3. Decode back
    auto decoded = decode_jpeg(*encoded);
    assert(decoded.has_value());
    assert(decoded->width() == w);
    assert(decoded->height() == h);

    // 4. Measure Peak Signal-to-Noise Ratio (PSNR)
    double mse = 0.0;
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y)->demultiply();
            auto dec = decoded->pixel(x, y)->demultiply();
            double dr = static_cast<double>(orig.red()) - dec.red();
            double dg = static_cast<double>(orig.green()) - dec.green();
            double db = static_cast<double>(orig.blue()) - dec.blue();
            mse += (dr * dr + dg * dg + db * db) / 3.0;
        }
    }
    mse /= (w * h);
    double psnr = 10.0 * std::log10((255.0 * 255.0) / (mse > 1e-6 ? mse : 1e-6));
    std::cout << "[DEBUG] MSE: " << mse << ", PSNR: " << psnr << "\n";
    auto o0 = pixmap.pixel(0, 0)->demultiply();
    auto d0 = decoded->pixel(0, 0)->demultiply();
    std::cout << "[DEBUG] (0,0) Orig RGB: " << (int)o0.red() << "," << (int)o0.green() << "," << (int)o0.blue()
              << " | Dec RGB: " << (int)d0.red() << "," << (int)d0.green() << "," << (int)d0.blue() << "\n";
    auto o1 = pixmap.pixel(32, 32)->demultiply();
    auto d1 = decoded->pixel(32, 32)->demultiply();
    std::cout << "[DEBUG] (32,32) Orig RGB: " << (int)o1.red() << "," << (int)o1.green() << "," << (int)o1.blue()
              << " | Dec RGB: " << (int)d1.red() << "," << (int)d1.green() << "," << (int)d1.blue() << "\n";
    assert(psnr > 30.0); // High fidelity reconstruction (>30 dB)

    // 5. Test File Save and Load
    std::string test_path = "/tmp/test_nisaba_sovereign.jpg";
    bool saved = pixmap.save_jpeg(test_path, 90);
    assert(saved);

    auto loaded = Pixmap::load_jpeg(test_path);
    assert(loaded.has_value());
    assert(loaded->width() == w);
    assert(loaded->height() == h);

    std::cout << "PASSED (PSNR: " << psnr << " dB, Encoded Size: " << encoded->size() << " bytes)\n";
}

int main() {
    std::cout << "=== Running Nisaba Sovereign JPEG Tests ===\n";
    test_jpeg_roundtrip();
    std::cout << "=== All JPEG Tests Passed! ===\n";
    return 0;
}

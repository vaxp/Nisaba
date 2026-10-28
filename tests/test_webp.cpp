#include <cassert>
#include <iostream>
#include <vector>
#include <cstdio>
#include "nisaba/image/webp.hpp"
#include "nisaba/image/image_io.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;
using namespace nisaba::image;

void test_probe_webp() {
    std::cout << "[TEST] WebP header probing... ";

    // 1. Valid VP8L chunk
    uint32_t w = 120;
    uint32_t h = 80;
    uint32_t w_val = w - 1;
    uint32_t h_val = h - 1;

    std::vector<uint8_t> valid_webp = {
        'R', 'I', 'F', 'F',
        22, 0, 0, 0,       // File size - 8
        'W', 'E', 'B', 'P',
        'V', 'P', '8', 'L',
        6, 0, 0, 0,        // Chunk size
        0x2F,              // VP8L signature
        static_cast<uint8_t>(w_val & 0xff),
        static_cast<uint8_t>(((w_val >> 8) & 0x3f) | ((h_val & 0x03) << 6)),
        static_cast<uint8_t>((h_val >> 2) & 0xff),
        static_cast<uint8_t>(((h_val >> 10) & 0x0f) | 0x10), // alpha flag = 1
        0x00               // empty stream byte
    };

    auto info = probe_webp(valid_webp);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->format == ImageFormat::WebP);
    assert(info->has_alpha == true);
    assert(info->channels == 4);

    // 2. Corrupted / invalid header
    std::vector<uint8_t> bad_data = {'F', 'A', 'K', 'E', 0, 0, 0, 0};
    auto bad_info = probe_webp(bad_data);
    assert(!bad_info.has_value());

    std::cout << "PASSED\n";
}

void test_webp_roundtrip_solid() {
    std::cout << "[TEST] WebP Lossless solid color roundtrip... ";

    uint32_t w = 64;
    uint32_t h = 48;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    ColorU8 solid_blue(20, 100, 240, 255);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            pixmap.set_pixel(x, y, solid_blue.premultiply());
        }
    }

    // 1. Encode
    auto enc = encode_webp(pixmap.as_ref());
    assert(enc.has_value());
    assert(!enc->empty());
    assert(enc->size() > 20);

    // 2. Probe
    auto info = probe_webp(*enc);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);
    assert(info->format == ImageFormat::WebP);

    // 3. Decode
    auto dec = decode_webp(*enc);
    assert(dec.has_value());
    assert(dec->width() == w);
    assert(dec->height() == h);

    // 4. Verify pixel identity
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y).value();
            auto decoded = dec->pixel(x, y).value();
            assert(orig.r == decoded.r);
            assert(orig.g == decoded.g);
            assert(orig.b == decoded.b);
            assert(orig.a == decoded.a);
        }
    }

    std::cout << "PASSED\n";
}

void test_webp_roundtrip_gradient_with_alpha() {
    std::cout << "[TEST] WebP Lossless gradient with alpha roundtrip... ";

    uint32_t w = 32;
    uint32_t h = 32;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t r = static_cast<uint8_t>(x * 7);
            uint8_t g = static_cast<uint8_t>(y * 7);
            uint8_t b = static_cast<uint8_t>((x + y) * 3);
            uint8_t a = static_cast<uint8_t>(128 + ((x * 3) % 127));
            ColorU8 c(r, g, b, a);
            pixmap.set_pixel(x, y, c.premultiply());
        }
    }

    // 1. Encode
    auto enc = encode_webp(pixmap.as_ref());
    assert(enc.has_value());

    // 2. Decode
    auto dec = decode_webp(*enc);
    assert(dec.has_value());
    assert(dec->width() == w);
    assert(dec->height() == h);

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y).value();
            auto decoded = dec->pixel(x, y).value();
            if (orig != decoded) {
                std::cerr << "Mismatch at (" << x << ", " << y << "): orig=("
                          << (int)orig.r << "," << (int)orig.g << "," << (int)orig.b << "," << (int)orig.a
                          << ") decoded=("
                          << (int)decoded.r << "," << (int)decoded.g << "," << (int)decoded.b << "," << (int)decoded.a
                          << ")\n";
            }
            assert(orig.r == decoded.r);
            assert(orig.g == decoded.g);
            assert(orig.b == decoded.b);
            assert(orig.a == decoded.a);
        }
    }

    std::cout << "PASSED\n";
}

void test_webp_file_io() {
    std::cout << "[TEST] WebP file save and load... ";

    uint32_t w = 40;
    uint32_t h = 40;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    ColorU8 col(180, 45, 90, 255);
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            pixmap.set_pixel(x, y, col.premultiply());
        }
    }

    const std::string tmp_file = "test_temp_roundtrip.webp";

    // 1. Save via Pixmap::save_webp
    bool save_ok = pixmap.save_webp(tmp_file);
    assert(save_ok);

    // 2. Load via Pixmap::load_webp
    auto loaded = Pixmap::load_webp(tmp_file);
    assert(loaded.has_value());
    assert(loaded->width() == w);
    assert(loaded->height() == h);

    // 3. Load via generic auto-detect Pixmap::load_file
    auto auto_loaded = Pixmap::load_file(tmp_file);
    assert(auto_loaded.has_value());
    assert(auto_loaded->width() == w);
    assert(auto_loaded->height() == h);

    // 4. Verify pixel values
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y).value();
            auto dec = auto_loaded->pixel(x, y).value();
            assert(orig.r == dec.r);
            assert(orig.g == dec.g);
            assert(orig.b == dec.b);
            assert(orig.a == dec.a);
        }
    }

    // 5. Probe file
    auto probe_res = probe_image_file(tmp_file);
    assert(probe_res.has_value());
    assert(probe_res->format == ImageFormat::WebP);
    assert(probe_res->width == w);
    assert(probe_res->height == h);

    std::remove(tmp_file.c_str());
    std::cout << "PASSED\n";
}

void test_webp_arbitrary_dimensions() {
    std::cout << "[TEST] WebP arbitrary non-square dimensions (1x1, 73x19, 13x137)... ";

    std::vector<std::pair<uint32_t, uint32_t>> sizes = {
        {1, 1},
        {73, 19},
        {13, 137}
    };

    for (const auto& [w, h] : sizes) {
        auto maybe_pixmap = Pixmap::allocate(w, h);
        assert(maybe_pixmap.has_value());
        Pixmap& pixmap = *maybe_pixmap;

        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                uint8_t r = static_cast<uint8_t>((x * 17) % 256);
                uint8_t g = static_cast<uint8_t>((y * 31) % 256);
                uint8_t b = static_cast<uint8_t>((x + y * 7) % 256);
                uint8_t a = static_cast<uint8_t>(200 + ((x + y) % 55));
                pixmap.set_pixel(x, y, ColorU8(r, g, b, a).premultiply());
            }
        }

        auto enc = encode_webp(pixmap.as_ref());
        assert(enc.has_value());

        auto dec = decode_webp(*enc);
        assert(dec.has_value());
        assert(dec->width() == w);
        assert(dec->height() == h);

        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                auto orig = pixmap.pixel(x, y).value();
                auto decoded = dec->pixel(x, y).value();
                assert(orig.r == decoded.r);
                assert(orig.g == decoded.g);
                assert(orig.b == decoded.b);
                assert(orig.a == decoded.a);
            }
        }
    }

    std::cout << "PASSED\n";
}

void test_webp_vp8x_extended_probe() {
    std::cout << "[TEST] WebP VP8X extended container probe... ";

    uint32_t canvas_w = 800;
    uint32_t canvas_h = 600;
    uint32_t w24 = canvas_w - 1;
    uint32_t h24 = canvas_h - 1;

    std::vector<uint8_t> vp8x_webp = {
        'R', 'I', 'F', 'F',
        30, 0, 0, 0,
        'W', 'E', 'B', 'P',
        'V', 'P', '8', 'X',
        10, 0, 0, 0,
        0x10, 0, 0, 0, // Flags: has_alpha = 1
        static_cast<uint8_t>(w24 & 0xff),
        static_cast<uint8_t>((w24 >> 8) & 0xff),
        static_cast<uint8_t>((w24 >> 16) & 0xff),
        static_cast<uint8_t>(h24 & 0xff),
        static_cast<uint8_t>((h24 >> 8) & 0xff),
        static_cast<uint8_t>((h24 >> 16) & 0xff),
        // Empty VP8L chunk following VP8X
        'V', 'P', '8', 'L',
        2, 0, 0, 0,
        0x2F, 0x00
    };

    auto info = probe_webp(vp8x_webp);
    assert(info.has_value());
    assert(info->width == canvas_w);
    assert(info->height == canvas_h);
    assert(info->has_alpha == true);
    assert(info->channels == 4);
    assert(info->format == ImageFormat::WebP);

    std::cout << "PASSED\n";
}

void test_webp_real_world_lossless() {
    std::cout << "[TEST] Real-world Lossless WebP decoding (sample_lossless.webp)... ";
    auto img = Pixmap::load_file("assets/sample_lossless.webp");
    if (!img) {
        std::cout << "SKIPPED (asset not found)\n";
        return;
    }
    assert(img->width() == 400);
    assert(img->height() == 301);
    assert(img->pixels() != nullptr);
    std::cout << "PASSED (" << img->width() << "x" << img->height() << ")\n";
}

void test_webp_animation_decoding() {
    std::cout << "[TEST] Animated WebP decoding (assets/anim_sample.webp)... ";
    std::string path = "assets/anim_sample.webp";
    auto anim_res = decode_webp_animation_file(path);
    if (!anim_res.has_value()) {
        path = "../assets/anim_sample.webp";
        anim_res = decode_webp_animation_file(path);
    }
    if (!anim_res.has_value()) {
        std::cout << "SKIPPED (asset not found)\n";
        return;
    }
    const auto& anim = *anim_res;

    assert(anim.canvas_width == 60);
    assert(anim.canvas_height == 40);
    assert(anim.frame_count() == 2);
    assert(anim.loop_count == 3);
    assert(anim.total_duration_ms() == 300);

    // Frame 0 checks (Red)
    const auto& f0 = anim.frames[0];
    assert(f0.duration_ms == 150);
    assert(f0.pixmap.width() == 60);
    assert(f0.pixmap.height() == 40);
    auto p0 = f0.pixmap.pixel(10, 10).value();
    assert(p0.r == 255 && p0.g == 0 && p0.b == 0 && p0.a == 255);

    // Frame 1 checks (Green)
    const auto& f1 = anim.frames[1];
    assert(f1.duration_ms == 150);
    assert(f1.pixmap.width() == 60);
    assert(f1.pixmap.height() == 40);
    auto p1 = f1.pixmap.pixel(10, 10).value();
    assert(p1.r == 0 && p1.g == 255 && p1.b == 0 && p1.a == 255);

    // Still image decode of animated file should return frame 0
    auto still = decode_webp_file(path);
    assert(still.has_value());
    assert(still->width() == 60 && still->height() == 40);
    auto ps = still->pixel(10, 10).value();
    assert(ps.r == 255 && ps.g == 0 && ps.b == 0 && ps.a == 255);

    std::cout << "PASSED (2 frames, 60x40, 300ms total)\n";
}

void test_webp_real_world_lossy() {
    std::cout << "[TEST] Real-world Lossy WebP decoding (sample.webp)... ";
    std::string path = "assets/sample.webp";
    auto info = probe_image_file(path);
    if (!info) {
        path = "../assets/sample.webp";
        info = probe_image_file(path);
    }
    if (!info) {
        std::cout << "SKIPPED (asset not found)\n";
        return;
    }
    assert(info->width == 550);
    assert(info->height == 368);
    assert(info->format == ImageFormat::WebP);

    auto img = decode_webp_file(path);
    assert(img.has_value());
    assert(img->width() == 550);
    assert(img->height() == 368);

    // Verify top-left sky blue pixel from sample.webp
    auto p0 = img->pixel(0, 0).value();
    assert(p0.r == 118);
    assert(p0.g == 181);
    assert(p0.b == 241);

    std::cout << "PASSED (" << img->width() << "x" << img->height() << ", VP8 decoded successfully)\n";
}

void test_webp_lossy_roundtrip() {
    std::cout << "[TEST] Lossy WebP encoding and decoding roundtrip... ";

    uint32_t w = 64;
    uint32_t h = 64;
    auto maybe_pixmap = Pixmap::allocate(w, h);
    assert(maybe_pixmap.has_value());
    Pixmap& pixmap = *maybe_pixmap;

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint8_t r = static_cast<uint8_t>(x * 4);
            uint8_t g = static_cast<uint8_t>(y * 4);
            uint8_t b = static_cast<uint8_t>((x + y) * 2);
            pixmap.set_pixel(x, y, ColorU8(r, g, b, 255).premultiply());
        }
    }

    // 1. Encode with lossy VP8
    auto enc = encode_webp(pixmap.as_ref(), false, 80);
    assert(enc.has_value());
    assert(!enc->empty());

    // 2. Probe
    auto info = probe_webp(*enc);
    assert(info.has_value());
    assert(info->width == w);
    assert(info->height == h);

    // 3. Decode
    auto dec = decode_webp(*enc);
    assert(dec.has_value());
    assert(dec->width() == w);
    assert(dec->height() == h);

    // 4. Measure PSNR
    double mse = 0.0;
    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            auto orig = pixmap.pixel(x, y)->demultiply();
            auto d = dec->pixel(x, y)->demultiply();
            double dr = static_cast<double>(orig.red()) - d.red();
            double dg = static_cast<double>(orig.green()) - d.green();
            double db = static_cast<double>(orig.blue()) - d.blue();
            mse += (dr * dr + dg * dg + db * db) / 3.0;
        }
    }
    mse /= (w * h);
    double psnr = 10.0 * std::log10((255.0 * 255.0) / (mse > 1e-6 ? mse : 1e-6));
    assert(psnr > 28.0);

    std::cout << "PASSED (PSNR: " << psnr << " dB)\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "Nisaba Sovereign WebP Codec Test Suite\n";
    std::cout << "========================================\n";

    test_probe_webp();
    test_webp_roundtrip_solid();
    test_webp_roundtrip_gradient_with_alpha();
    test_webp_file_io();
    test_webp_arbitrary_dimensions();
    test_webp_vp8x_extended_probe();
    test_webp_real_world_lossless();
    test_webp_real_world_lossy();
    test_webp_lossy_roundtrip();
    test_webp_animation_decoding();

    std::cout << "========================================\n";
    std::cout << "All WebP Unit Tests PASSED Successfully!\n";
    std::cout << "========================================\n";
    return 0;
}



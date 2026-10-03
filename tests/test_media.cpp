#include <cassert>
#include <iostream>
#include <memory>
#include <cmath>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::media;

void test_video_frame_to_pixmap_and_canvas() {
    uint32_t w = 120;
    uint32_t h = 80;

    // 1. Create a RGBA video frame with a dynamic color gradient
    auto frame = VideoFrame::allocate(w, h, PixelFormat::RGBA32);
    frame.set_pts(0);
    uint8_t* p = frame.plane_mut(0);
    size_t stride = frame.stride(0);

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            p[y * stride + x * 4 + 0] = static_cast<uint8_t>((x * 255) / w);
            p[y * stride + x * 4 + 1] = static_cast<uint8_t>((y * 255) / h);
            p[y * stride + x * 4 + 2] = 180;
            p[y * stride + x * 4 + 3] = 255;
        }
    }

    // 2. Convert VideoFrame directly into a Nisaba Pixmap
    auto pixmap = to_pixmap(frame);
    assert(pixmap.has_value());
    assert(pixmap->width() == w);
    assert(pixmap->height() == h);

    // Verify pixel values in Pixmap
    auto ref = pixmap->as_ref();
    auto px0 = ref.pixel(0, 0);
    assert(px0.has_value());
    assert(px0->red() == 0 && px0->green() == 0 && px0->blue() == 180 && px0->alpha() == 255);

    // 3. Render video frame directly onto a Nisaba Canvas
    auto screen = Pixmap::allocate(400, 300);
    assert(screen.has_value());

    Canvas canvas(*screen);
    canvas.clear(Color::from_rgba8(20, 24, 33, 255));

    // Draw video frame at (50, 50)
    draw_video_frame(canvas, 50, 50, frame);

    // Verify canvas has video frame rendered at (50, 50)
    auto screen_ref = screen->as_ref();
    auto screen_px = screen_ref.pixel(50, 50);
    assert(screen_px.has_value());
    assert(screen_px->red() == 0 && screen_px->green() == 0 && screen_px->blue() == 180);

    // 4. Test YUV420P conversion directly to Pixmap
    auto yuv_frame = ColorConverter::convert(frame, PixelFormat::YUV420P);
    assert(yuv_frame.is_valid());

    auto yuv_pixmap = to_pixmap(yuv_frame);
    assert(yuv_pixmap.has_value());
    assert(yuv_pixmap->width() == w);
    assert(yuv_pixmap->height() == h);

    // Blit YUV frame to canvas at (200, 50)
    draw_video_frame(canvas, 200, 50, yuv_frame);

    std::cout << "[PASS] VideoFrame -> Nisaba Pixmap & Canvas direct rendering test passed.\n";
}

void test_motion_video_recording_and_playback() {
    uint32_t w = 64;
    uint32_t h = 64;

    // Simulate encoding a sequence of 10 video frames using Motion-QOI
    QoiVideoEncoder encoder;
    encoder.init(CodecParameters::create_video(CodecId::QOI_VIDEO, w, h, PixelFormat::RGBA32, Rational(30, 1)));

    std::vector<Packet> packet_stream;

    for (int f = 0; f < 10; ++f) {
        auto vf = VideoFrame::allocate(w, h, PixelFormat::RGBA32);
        vf.set_pts(f);
        uint8_t* p = vf.plane_mut(0);
        size_t stride = vf.stride(0);
        for (uint32_t y = 0; y < h; ++y) {
            for (uint32_t x = 0; x < w; ++x) {
                p[y * stride + x * 4 + 0] = static_cast<uint8_t>(f * 20);
                p[y * stride + x * 4 + 1] = static_cast<uint8_t>(x * 4);
                p[y * stride + x * 4 + 2] = static_cast<uint8_t>(y * 4);
                p[y * stride + x * 4 + 3] = 255;
            }
        }
        encoder.send_frame(vf);
        Packet pkt;
        encoder.receive_packet(pkt);
        assert(!pkt.empty());
        packet_stream.push_back(std::move(pkt));
    }

    assert(packet_stream.size() == 10);

    // Decode all frames and verify playback sequence
    QoiVideoDecoder decoder;
    decoder.init(CodecParameters::create_video(CodecId::QOI_VIDEO, w, h, PixelFormat::RGBA32));

    for (size_t f = 0; f < packet_stream.size(); ++f) {
        decoder.send_packet(packet_stream[f]);
        VideoFrame decoded;
        auto res = decoder.receive_frame(decoded);
        assert(res.is_ok());
        assert(decoded.pts() == static_cast<int64_t>(f));
        const uint8_t* p = decoded.plane(0);
        assert(p[0] == static_cast<uint8_t>(f * 20));
    }

    std::cout << "[PASS] Video sequence record & playback pipeline test passed.\n";
}

int main() {
    std::cout << "--- Running Nisaba Engine Media Integration Tests ---\n";
    test_video_frame_to_pixmap_and_canvas();
    test_motion_video_recording_and_playback();
    std::cout << "All Nisaba Media Integration tests PASSED successfully!\n";
    return 0;
}

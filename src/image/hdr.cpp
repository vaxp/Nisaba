#include "nisaba/image/hdr.hpp"
#include <fstream>
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace nisaba::image {

namespace {

struct HdrHeader {
    uint32_t width{0};
    uint32_t height{0};
    float exposure{1.0f};
    size_t data_offset{0};
};

bool read_hdr_line(std::span<const uint8_t> data, size_t& pos, std::string& line) {
    line.clear();
    if (pos >= data.size()) return false;
    while (pos < data.size()) {
        uint8_t c = data[pos++];
        if (c == '\n') break;
        if (c != '\r') line.push_back(static_cast<char>(c));
    }
    return true;
}

ImageResult<HdrHeader> parse_hdr_header(std::span<const uint8_t> data) {
    if (data.size() < 10) {
        return ImageResult<HdrHeader>::fail(ImageError::InvalidSignature);
    }

    if (std::memcmp(data.data(), "#?RADIANCE", 10) != 0 &&
        std::memcmp(data.data(), "#?RGBE", 6) != 0) {
        return ImageResult<HdrHeader>::fail(ImageError::InvalidSignature);
    }

    HdrHeader h{};
    size_t pos = 0;
    std::string line;

    // Read header lines until empty line
    while (read_hdr_line(data, pos, line)) {
        if (line.empty()) break;
        if (line.rfind("EXPOSURE=", 0) == 0) {
            float exp_val = std::strtof(line.c_str() + 9, nullptr);
            if (exp_val > 0.0f) h.exposure *= exp_val;
        }
    }

    // Read resolution line: e.g. "-Y 512 +X 1024"
    if (!read_hdr_line(data, pos, line) || line.empty()) {
        return ImageResult<HdrHeader>::fail(ImageError::CorruptedData);
    }

    char y_sign = 0, y_axis = 0, x_sign = 0, x_axis = 0;
    unsigned int y_val = 0, x_val = 0;
    if (std::sscanf(line.c_str(), "%c%c %u %c%c %u", &y_sign, &y_axis, &y_val, &x_sign, &x_axis, &x_val) == 6) {
        if ((y_axis == 'Y' || y_axis == 'y') && (x_axis == 'X' || x_axis == 'x')) {
            h.height = y_val;
            h.width = x_val;
        } else if ((y_axis == 'X' || y_axis == 'x') && (x_axis == 'Y' || x_axis == 'y')) {
            h.width = y_val;
            h.height = x_val;
        }
    }

    if (h.width == 0 || h.height == 0 || h.width > 32768 || h.height > 32768) {
        return ImageResult<HdrHeader>::fail(ImageError::InvalidDimensions);
    }

    h.data_offset = pos;
    return ImageResult<HdrHeader>::success(h);
}

inline void rgbe_to_float(uint8_t r, uint8_t g, uint8_t b, uint8_t e, float out_rgb[3]) noexcept {
    if (e == 0) {
        out_rgb[0] = out_rgb[1] = out_rgb[2] = 0.0f;
    } else {
        float scale = std::ldexp(1.0f, static_cast<int>(e) - (128 + 8));
        out_rgb[0] = (static_cast<float>(r) + 0.5f) * scale;
        out_rgb[1] = (static_cast<float>(g) + 0.5f) * scale;
        out_rgb[2] = (static_cast<float>(b) + 0.5f) * scale;
    }
}

inline void float_to_rgbe(float r, float g, float b, uint8_t out_rgbe[4]) noexcept {
    r = std::max(0.0f, r);
    g = std::max(0.0f, g);
    b = std::max(0.0f, b);
    float v = std::max({r, g, b});
    if (v < 1e-32f) {
        out_rgbe[0] = out_rgbe[1] = out_rgbe[2] = out_rgbe[3] = 0;
    } else {
        int e = 0;
        float m = std::frexp(v, &e) * 256.0f / v;
        out_rgbe[0] = static_cast<uint8_t>(std::clamp(static_cast<int>(r * m), 0, 255));
        out_rgbe[1] = static_cast<uint8_t>(std::clamp(static_cast<int>(g * m), 0, 255));
        out_rgbe[2] = static_cast<uint8_t>(std::clamp(static_cast<int>(b * m), 0, 255));
        out_rgbe[3] = static_cast<uint8_t>(std::clamp(e + 128, 0, 255));
    }
}

inline uint8_t tonemap_srgb_u8(float v) noexcept {
    v = std::max(0.0f, v);
    // ACES filmic / Reinhard hybrid curve + 1/2.2 sRGB gamma
    float mapped = v / (1.0f + v);
    float srgb = std::pow(mapped, 1.0f / 2.2f);
    return static_cast<uint8_t>(std::clamp(static_cast<int>(srgb * 255.0f + 0.5f), 0, 255));
}

} // namespace

ImageResult<ImageInfo> probe_hdr(std::span<const uint8_t> hdr_data) {
    auto res = parse_hdr_header(hdr_data);
    if (!res) return ImageResult<ImageInfo>::fail(res.error);
    const auto& h = *res;

    ImageInfo info{};
    info.width = h.width;
    info.height = h.height;
    info.channels = 3;
    info.bit_depth = 32;
    info.has_alpha = false;
    info.format = ImageFormat::HDR;
    info.color_model = ColorModel::HDR_RGBE;
    return ImageResult<ImageInfo>::success(info);
}

ImageResult<HdrImage> decode_hdr_float(std::span<const uint8_t> hdr_data) {
    auto res = parse_hdr_header(hdr_data);
    if (!res) return ImageResult<HdrImage>::fail(res.error);
    const auto& h = *res;

    HdrImage img;
    img.width = h.width;
    img.height = h.height;
    img.exposure = h.exposure;
    img.pixels.resize(static_cast<size_t>(h.width) * h.height * 3, 0.0f);

    size_t pos = h.data_offset;
    std::vector<uint8_t> scanline(static_cast<size_t>(h.width) * 4);

    for (uint32_t y = 0; y < h.height; ++y) {
        if (pos + 4 > hdr_data.size()) {
            return ImageResult<HdrImage>::fail(ImageError::PrematureEndOfStream);
        }

        uint8_t b0 = hdr_data[pos];
        uint8_t b1 = hdr_data[pos + 1];
        uint8_t b2 = hdr_data[pos + 2];
        uint8_t b3 = hdr_data[pos + 3];

        bool is_new_rle = (h.width >= 8 && h.width <= 0x7FFF &&
                           b0 == 2 && b1 == 2 && (b2 & 0x80) == 0 &&
                           ((static_cast<uint32_t>(b2) << 8) | b3) == h.width);

        if (is_new_rle) {
            pos += 4;
            for (int ch = 0; ch < 4; ++ch) {
                uint32_t x = 0;
                while (x < h.width) {
                    if (pos >= hdr_data.size()) {
                        return ImageResult<HdrImage>::fail(ImageError::PrematureEndOfStream);
                    }
                    uint8_t code = hdr_data[pos++];
                    if (code > 128) {
                        uint32_t count = code - 128;
                        if (x + count > h.width || pos >= hdr_data.size()) {
                            return ImageResult<HdrImage>::fail(ImageError::CorruptedData);
                        }
                        uint8_t val = hdr_data[pos++];
                        for (uint32_t k = 0; k < count; ++k) {
                            scanline[(x + k) * 4 + ch] = val;
                        }
                        x += count;
                    } else {
                        uint32_t count = code;
                        if (count == 0 || x + count > h.width || pos + count > hdr_data.size()) {
                            return ImageResult<HdrImage>::fail(ImageError::CorruptedData);
                        }
                        for (uint32_t k = 0; k < count; ++k) {
                            scanline[(x + k) * 4 + ch] = hdr_data[pos++];
                        }
                        x += count;
                    }
                }
            }
        } else {
            // Uncompressed RGBE scanline
            if (pos + static_cast<size_t>(h.width) * 4 > hdr_data.size()) {
                return ImageResult<HdrImage>::fail(ImageError::PrematureEndOfStream);
            }
            std::memcpy(scanline.data(), hdr_data.data() + pos, static_cast<size_t>(h.width) * 4);
            pos += static_cast<size_t>(h.width) * 4;
        }

        float* dst_row = img.pixels.data() + static_cast<size_t>(y) * h.width * 3;
        for (uint32_t x = 0; x < h.width; ++x) {
            const uint8_t* rgbe = scanline.data() + x * 4;
            rgbe_to_float(rgbe[0], rgbe[1], rgbe[2], rgbe[3], dst_row + x * 3);
        }
    }

    return ImageResult<HdrImage>::success(std::move(img));
}

ImageResult<Pixmap> decode_hdr(std::span<const uint8_t> hdr_data, DecodeOptions /*options*/) {
    auto flt_res = decode_hdr_float(hdr_data);
    if (!flt_res) return ImageResult<Pixmap>::fail(flt_res.error);
    const auto& hdr = *flt_res;

    auto maybe_pixmap = Pixmap::allocate(hdr.width, hdr.height);
    if (!maybe_pixmap) return ImageResult<Pixmap>::fail(ImageError::OutOfMemory);
    Pixmap pixmap = std::move(*maybe_pixmap);
    PremultipliedColorU8* dst = pixmap.pixels_mut();

    size_t total = static_cast<size_t>(hdr.width) * hdr.height;
    for (size_t i = 0; i < total; ++i) {
        uint8_t r = tonemap_srgb_u8(hdr.pixels[i * 3 + 0]);
        uint8_t g = tonemap_srgb_u8(hdr.pixels[i * 3 + 1]);
        uint8_t b = tonemap_srgb_u8(hdr.pixels[i * 3 + 2]);
        dst[i] = PremultipliedColorU8(r, g, b, 255);
    }

    return ImageResult<Pixmap>::success(std::move(pixmap));
}

ImageResult<std::vector<uint8_t>> encode_hdr_float(
    std::span<const float> rgb_float,
    uint32_t width,
    uint32_t height
) {
    if (width == 0 || height == 0 || rgb_float.size() < static_cast<size_t>(width) * height * 3) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    std::string header = "#?RADIANCE\nFORMAT=32-bit_rle_rgbe\n\n-Y " +
                         std::to_string(height) + " +X " + std::to_string(width) + "\n";

    std::vector<uint8_t> out;
    out.reserve(header.size() + static_cast<size_t>(width) * height * 4);
    out.insert(out.end(), header.begin(), header.end());

    std::vector<uint8_t> rgbe_row(static_cast<size_t>(width) * 4);

    for (uint32_t y = 0; y < height; ++y) {
        const float* src_row = rgb_float.data() + static_cast<size_t>(y) * width * 3;
        for (uint32_t x = 0; x < width; ++x) {
            float_to_rgbe(src_row[x * 3 + 0], src_row[x * 3 + 1], src_row[x * 3 + 2], rgbe_row.data() + x * 4);
        }

        if (width < 8 || width > 0x7FFF) {
            out.insert(out.end(), rgbe_row.begin(), rgbe_row.end());
            continue;
        }

        // Write adaptive RLE scanline header
        out.push_back(2);
        out.push_back(2);
        out.push_back(static_cast<uint8_t>((width >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(width & 0xFF));

        for (int ch = 0; ch < 4; ++ch) {
            uint32_t x = 0;
            while (x < width) {
                // Find run of identical values
                uint32_t run = 1;
                uint8_t val = rgbe_row[x * 4 + ch];
                while (x + run < width && run < 127 && rgbe_row[(x + run) * 4 + ch] == val) {
                    ++run;
                }
                if (run >= 4) {
                    out.push_back(static_cast<uint8_t>(128 + run));
                    out.push_back(val);
                    x += run;
                } else {
                    uint32_t lit = 1;
                    while (x + lit < width && lit < 128) {
                        uint32_t next_run = 1;
                        uint8_t nv = rgbe_row[(x + lit) * 4 + ch];
                        while (x + lit + next_run < width && next_run < 4 &&
                               rgbe_row[(x + lit + next_run) * 4 + ch] == nv) {
                            ++next_run;
                        }
                        if (next_run >= 4) break;
                        ++lit;
                    }
                    out.push_back(static_cast<uint8_t>(lit));
                    for (uint32_t k = 0; k < lit; ++k) {
                        out.push_back(rgbe_row[(x + k) * 4 + ch]);
                    }
                    x += lit;
                }
            }
        }
    }

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

ImageResult<std::vector<uint8_t>> encode_hdr(PixmapRef pixmap) {
    uint32_t w = pixmap.width();
    uint32_t h = pixmap.height();
    if (w == 0 || h == 0) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    std::vector<float> rgb_float(static_cast<size_t>(w) * h * 3);
    for (uint32_t y = 0; y < h; ++y) {
        const auto* row = pixmap.row(y);
        for (uint32_t x = 0; x < w; ++x) {
            ColorU8 c = row[x].demultiply();
            size_t idx = (static_cast<size_t>(y) * w + x) * 3;
            // Inverse sRGB + inverse Reinhard so decode_hdr round-trips closely
            auto to_linear = [](uint8_t u) -> float {
                float s = static_cast<float>(u) / 255.0f;
                float lin = std::pow(s, 2.2f);
                return lin / std::max(1.0f - lin, 1e-3f);
            };
            rgb_float[idx + 0] = to_linear(c.r);
            rgb_float[idx + 1] = to_linear(c.g);
            rgb_float[idx + 2] = to_linear(c.b);
        }
    }

    return encode_hdr_float(rgb_float, w, h);
}

ImageResult<Pixmap> decode_hdr_file(const std::string& file_path, DecodeOptions options) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return ImageResult<Pixmap>::fail(ImageError::IOError);
    std::streamsize size = file.tellg();
    if (size <= 0) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }
    return decode_hdr(buffer, options);
}

ImageResult<HdrImage> decode_hdr_float_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return ImageResult<HdrImage>::fail(ImageError::IOError);
    std::streamsize size = file.tellg();
    if (size <= 0) return ImageResult<HdrImage>::fail(ImageError::CorruptedData);
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<HdrImage>::fail(ImageError::IOError);
    }
    return decode_hdr_float(buffer);
}

bool save_hdr_file(PixmapRef pixmap, const std::string& file_path) {
    auto encoded = encode_hdr(pixmap);
    if (!encoded) return false;
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(encoded->data()), encoded->size());
    return file.good();
}

} // namespace nisaba::image

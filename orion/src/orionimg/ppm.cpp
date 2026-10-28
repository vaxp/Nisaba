#include "orion/orionimg/ppm.hpp"
#include <fstream>
#include <cstring>
#include <cctype>
#include <algorithm>

namespace orionimg {

namespace {

struct PpmHeader {
    uint8_t magic_num{0}; // 2, 3, 5, 6, 7
    uint32_t width{0};
    uint32_t height{0};
    uint32_t maxval{255};
    uint32_t channels{3};
    size_t data_offset{0};
};

bool is_ppm_ws(uint8_t c) noexcept {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

bool read_ppm_token(std::span<const uint8_t> data, size_t& pos, std::string& token) {
    token.clear();
    while (pos < data.size()) {
        if (data[pos] == '#') {
            while (pos < data.size() && data[pos] != '\n' && data[pos] != '\r') ++pos;
        } else if (is_ppm_ws(data[pos])) {
            ++pos;
        } else {
            break;
        }
    }
    if (pos >= data.size()) return false;
    while (pos < data.size() && !is_ppm_ws(data[pos]) && data[pos] != '#') {
        token.push_back(static_cast<char>(data[pos++]));
    }
    return !token.empty();
}

bool parse_u32_token(const std::string& s, uint32_t& out) {
    if (s.empty()) return false;
    uint64_t v = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return false;
        v = v * 10 + static_cast<uint64_t>(c - '0');
        if (v > 0xFFFFFFFFULL) return false;
    }
    out = static_cast<uint32_t>(v);
    return true;
}

ImageResult<PpmHeader> parse_ppm_header(std::span<const uint8_t> data) {
    if (data.size() < 7 || data[0] != 'P') {
        return ImageResult<PpmHeader>::fail(ImageError::InvalidSignature);
    }
    uint8_t m = data[1];
    if (m != '2' && m != '3' && m != '5' && m != '6' && m != '7') {
        return ImageResult<PpmHeader>::fail(ImageError::InvalidSignature);
    }
    if (!is_ppm_ws(data[2]) && data[2] != '#') {
        return ImageResult<PpmHeader>::fail(ImageError::InvalidSignature);
    }

    PpmHeader h{};
    h.magic_num = static_cast<uint8_t>(m - '0');
    size_t pos = 2;
    std::string tok;

    if (h.magic_num == 7) {
        // PAM header
        while (read_ppm_token(data, pos, tok)) {
            if (tok == "ENDHDR") {
                if (pos < data.size() && data[pos] == '\r') ++pos;
                if (pos < data.size() && data[pos] == '\n') ++pos;
                else if (pos < data.size() && is_ppm_ws(data[pos])) ++pos;
                break;
            } else if (tok == "WIDTH") {
                if (!read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.width)) {
                    return ImageResult<PpmHeader>::fail(ImageError::CorruptedData);
                }
            } else if (tok == "HEIGHT") {
                if (!read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.height)) {
                    return ImageResult<PpmHeader>::fail(ImageError::CorruptedData);
                }
            } else if (tok == "DEPTH") {
                if (!read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.channels)) {
                    return ImageResult<PpmHeader>::fail(ImageError::CorruptedData);
                }
            } else if (tok == "MAXVAL") {
                if (!read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.maxval)) {
                    return ImageResult<PpmHeader>::fail(ImageError::CorruptedData);
                }
            } else if (tok == "TUPLTYPE") {
                read_ppm_token(data, pos, tok);
            }
        }
    } else {
        if (!read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.width) ||
            !read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.height) ||
            !read_ppm_token(data, pos, tok) || !parse_u32_token(tok, h.maxval)) {
            return ImageResult<PpmHeader>::fail(ImageError::CorruptedData);
        }
        // Exactly one whitespace character (or CRLF) follows maxval before binary data
        if (pos < data.size() && data[pos] == '\r' && pos + 1 < data.size() && data[pos + 1] == '\n') {
            pos += 2;
        } else if (pos < data.size() && is_ppm_ws(data[pos])) {
            ++pos;
        }
        h.channels = (h.magic_num == 2 || h.magic_num == 5) ? 1 : 3;
    }

    if (h.width == 0 || h.height == 0 || h.width > 32768 || h.height > 32768 ||
        h.maxval == 0 || h.maxval > 65535 || h.channels == 0 || h.channels > 4) {
        return ImageResult<PpmHeader>::fail(ImageError::InvalidDimensions);
    }

    h.data_offset = pos;
    return ImageResult<PpmHeader>::success(h);
}

inline uint8_t scale_ppm_sample(uint32_t v, uint32_t maxval) noexcept {
    if (maxval == 255) return static_cast<uint8_t>(std::min<uint32_t>(v, 255));
    v = std::min(v, maxval);
    return static_cast<uint8_t>((v * 255U + (maxval >> 1)) / maxval);
}

} // namespace

ImageResult<ImageInfo> probe_ppm(std::span<const uint8_t> ppm_data) {
    auto res = parse_ppm_header(ppm_data);
    if (!res) return ImageResult<ImageInfo>::fail(res.error);
    const auto& h = *res;

    ImageInfo info{};
    info.width = h.width;
    info.height = h.height;
    info.channels = static_cast<uint8_t>(h.channels);
    info.bit_depth = (h.maxval > 255) ? 16 : 8;
    info.has_alpha = (h.channels == 2 || h.channels == 4);
    info.format = ImageFormat::PPM;
    if (h.channels == 1) info.color_model = ColorModel::Grayscale;
    else if (h.channels == 2) info.color_model = ColorModel::GrayscaleAlpha;
    else if (h.channels == 3) info.color_model = ColorModel::RGB;
    else info.color_model = ColorModel::RGBA;
    return ImageResult<ImageInfo>::success(info);
}

ImageResult<Image> decode_ppm(std::span<const uint8_t> ppm_data, DecodeOptions options) {
    auto res = parse_ppm_header(ppm_data);
    if (!res) return ImageResult<Image>::fail(res.error);
    const auto& h = *res;

    auto maybe_pixmap = Image::allocate(h.width, h.height);
    if (!maybe_pixmap) return ImageResult<Image>::fail(ImageError::OutOfMemory);
    Image pixmap = std::move(*maybe_pixmap);
    PremultipliedColorU8* dst = pixmap.pixels_mut();

    size_t total_pixels = static_cast<size_t>(h.width) * h.height;
    size_t pos = h.data_offset;

    auto store_px = [&](size_t idx, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        if (options.premultiply_alpha) {
            dst[idx] = ColorU8(r, g, b, a).premultiply();
        } else {
            dst[idx] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
        }
    };

    if (h.magic_num == 2 || h.magic_num == 3) {
        // ASCII P2 / P3
        std::string tok;
        for (size_t i = 0; i < total_pixels; ++i) {
            uint8_t ch[4] = {0, 0, 0, 255};
            for (uint32_t c = 0; c < h.channels; ++c) {
                uint32_t v = 0;
                if (!read_ppm_token(ppm_data, pos, tok) || !parse_u32_token(tok, v)) {
                    return ImageResult<Image>::fail(ImageError::PrematureEndOfStream);
                }
                ch[c] = scale_ppm_sample(v, h.maxval);
            }
            if (h.channels == 1) {
                store_px(i, ch[0], ch[0], ch[0], 255);
            } else {
                store_px(i, ch[0], ch[1], ch[2], 255);
            }
        }
    } else {
        // Binary P5 / P6 / P7
        size_t bytes_per_sample = (h.maxval > 255) ? 2 : 1;
        size_t needed = total_pixels * h.channels * bytes_per_sample;
        if (pos + needed > ppm_data.size()) {
            return ImageResult<Image>::fail(ImageError::PrematureEndOfStream);
        }

        const uint8_t* p = ppm_data.data() + pos;
        if (bytes_per_sample == 1 && h.maxval == 255 && h.channels == 3) {
            for (size_t i = 0; i < total_pixels; ++i) {
                store_px(i, p[0], p[1], p[2], 255);
                p += 3;
            }
        } else {
            for (size_t i = 0; i < total_pixels; ++i) {
                uint8_t ch[4] = {0, 0, 0, 255};
                for (uint32_t c = 0; c < h.channels; ++c) {
                    uint32_t v = (bytes_per_sample == 2)
                        ? ((static_cast<uint32_t>(p[0]) << 8) | p[1])
                        : p[0];
                    p += bytes_per_sample;
                    ch[c] = scale_ppm_sample(v, h.maxval);
                }
                if (h.channels == 1) {
                    store_px(i, ch[0], ch[0], ch[0], 255);
                } else if (h.channels == 2) {
                    store_px(i, ch[0], ch[0], ch[0], ch[1]);
                } else if (h.channels == 3) {
                    store_px(i, ch[0], ch[1], ch[2], 255);
                } else {
                    store_px(i, ch[0], ch[1], ch[2], ch[3]);
                }
            }
        }
    }

    return ImageResult<Image>::success(std::move(pixmap));
}

ImageResult<std::vector<uint8_t>> encode_ppm(ImageRef pixmap) {
    uint32_t w = pixmap.width();
    uint32_t h = pixmap.height();
    if (w == 0 || h == 0) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    std::string header = "P6\n" + std::to_string(w) + " " + std::to_string(h) + "\n255\n";
    size_t pixel_bytes = static_cast<size_t>(w) * h * 3;
    std::vector<uint8_t> out(header.size() + pixel_bytes);
    std::memcpy(out.data(), header.data(), header.size());

    uint8_t* dst = out.data() + header.size();
    for (uint32_t y = 0; y < h; ++y) {
        const auto* row = pixmap.row(y);
        for (uint32_t x = 0; x < w; ++x) {
            ColorU8 c = row[x].demultiply();
            dst[0] = c.r;
            dst[1] = c.g;
            dst[2] = c.b;
            dst += 3;
        }
    }

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

ImageResult<Image> decode_ppm_file(const std::string& file_path, DecodeOptions options) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return ImageResult<Image>::fail(ImageError::IOError);
    std::streamsize size = file.tellg();
    if (size <= 0) return ImageResult<Image>::fail(ImageError::CorruptedData);
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Image>::fail(ImageError::IOError);
    }
    return decode_ppm(buffer, options);
}

bool save_ppm_file(ImageRef pixmap, const std::string& file_path) {
    auto encoded = encode_ppm(pixmap);
    if (!encoded) return false;
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(encoded->data()), encoded->size());
    return file.good();
}

} // namespace orionimg

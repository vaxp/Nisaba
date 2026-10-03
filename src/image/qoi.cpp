#include "nisaba/image/qoi.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <array>

namespace nisaba::image {

namespace {

constexpr uint8_t QOI_MAGIC[4] = {'q', 'o', 'i', 'f'};
constexpr uint8_t QOI_PADDING[8] = {0, 0, 0, 0, 0, 0, 0, 1};

constexpr uint8_t QOI_OP_INDEX = 0x00; // 00xxxxxx
constexpr uint8_t QOI_OP_DIFF  = 0x40; // 01xxxxxx
constexpr uint8_t QOI_OP_LUMA  = 0x80; // 10xxxxxx
constexpr uint8_t QOI_OP_RUN   = 0xC0; // 11xxxxxx
constexpr uint8_t QOI_OP_RGB   = 0xFE; // 11111110
constexpr uint8_t QOI_OP_RGBA  = 0xFF; // 11111111

constexpr uint8_t QOI_MASK_2   = 0xC0;

inline uint32_t read_u32_be(const uint8_t* p) noexcept {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
            static_cast<uint32_t>(p[3]);
}

inline void write_u32_be(uint8_t* p, uint32_t v) noexcept {
    p[0] = static_cast<uint8_t>((v >> 24) & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 8)  & 0xFF);
    p[3] = static_cast<uint8_t>(v & 0xFF);
}

inline size_t qoi_color_hash(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept {
    return (static_cast<size_t>(r) * 3 +
            static_cast<size_t>(g) * 5 +
            static_cast<size_t>(b) * 7 +
            static_cast<size_t>(a) * 11) % 64;
}

} // namespace

ImageResult<ImageInfo> probe_qoi(std::span<const uint8_t> qoi_data) {
    if (qoi_data.size() < 14 + sizeof(QOI_PADDING)) {
        return ImageResult<ImageInfo>::fail(ImageError::PrematureEndOfStream);
    }

    if (std::memcmp(qoi_data.data(), QOI_MAGIC, 4) != 0) {
        return ImageResult<ImageInfo>::fail(ImageError::InvalidSignature);
    }

    uint32_t w = read_u32_be(qoi_data.data() + 4);
    uint32_t h = read_u32_be(qoi_data.data() + 8);
    uint8_t channels = qoi_data[12];
    uint8_t colorspace = qoi_data[13];

    if (w == 0 || h == 0 || w > 32768 || h > 32768) {
        return ImageResult<ImageInfo>::fail(ImageError::InvalidDimensions);
    }
    if (channels != 3 && channels != 4) {
        return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
    }
    if (colorspace > 1) {
        return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
    }

    ImageInfo info{};
    info.width = w;
    info.height = h;
    info.channels = channels;
    info.bit_depth = 8;
    info.has_alpha = (channels == 4);
    info.format = ImageFormat::QOI;
    info.color_model = (channels == 4) ? ColorModel::RGBA : ColorModel::RGB;

    return ImageResult<ImageInfo>::success(info);
}

ImageResult<Pixmap> decode_qoi(std::span<const uint8_t> qoi_data) {
    auto info_res = probe_qoi(qoi_data);
    if (!info_res) {
        return ImageResult<Pixmap>::fail(info_res.error);
    }

    const auto& info = *info_res;
    uint32_t w = info.width;
    uint32_t h = info.height;
    size_t total_pixels = static_cast<size_t>(w) * h;

    auto opt_pixmap = Pixmap::allocate(w, h);
    if (!opt_pixmap) {
        return ImageResult<Pixmap>::fail(ImageError::OutOfMemory);
    }
    auto& pixmap = *opt_pixmap;

    std::array<ColorU8, 64> index{};
    ColorU8 px(0, 0, 0, 255);

    PremultipliedColorU8* dst = pixmap.pixels_mut();
    size_t px_idx = 0;
    size_t p = 14;
    size_t end = qoi_data.size() - sizeof(QOI_PADDING);

    auto store_pixel = [&](ColorU8 c) noexcept {
        if (c.alpha() == 255) {
            dst[px_idx++] = PremultipliedColorU8(c.red(), c.green(), c.blue(), 255);
        } else if (c.alpha() == 0) {
            dst[px_idx++] = PremultipliedColorU8(0, 0, 0, 0);
        } else {
            dst[px_idx++] = c.premultiply();
        }
    };

    while (px_idx < total_pixels && p < end) {
        uint8_t b1 = qoi_data[p++];

        if (b1 == QOI_OP_RGB) {
            if (p + 3 > end) {
                return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
            }
            px.r = qoi_data[p++];
            px.g = qoi_data[p++];
            px.b = qoi_data[p++];
        } else if (b1 == QOI_OP_RGBA) {
            if (p + 4 > end) {
                return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
            }
            px.r = qoi_data[p++];
            px.g = qoi_data[p++];
            px.b = qoi_data[p++];
            px.a = qoi_data[p++];
        } else if ((b1 & QOI_MASK_2) == QOI_OP_INDEX) {
            px = index[b1 & 0x3F];
        } else if ((b1 & QOI_MASK_2) == QOI_OP_DIFF) {
            px.r = static_cast<uint8_t>(px.r + ((b1 >> 4) & 0x03) - 2);
            px.g = static_cast<uint8_t>(px.g + ((b1 >> 2) & 0x03) - 2);
            px.b = static_cast<uint8_t>(px.b + (b1 & 0x03) - 2);
        } else if ((b1 & QOI_MASK_2) == QOI_OP_LUMA) {
            if (p >= end) {
                return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
            }
            uint8_t b2 = qoi_data[p++];
            int dg = (b1 & 0x3F) - 32;
            int dr_dg = ((b2 >> 4) & 0x0F) - 8;
            int db_dg = (b2 & 0x0F) - 8;

            px.r = static_cast<uint8_t>(px.r + dg + dr_dg);
            px.g = static_cast<uint8_t>(px.g + dg);
            px.b = static_cast<uint8_t>(px.b + dg + db_dg);
        } else if ((b1 & QOI_MASK_2) == QOI_OP_RUN) {
            int run = (b1 & 0x3F) + 1;
            if (px_idx + run > total_pixels) {
                return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            }
            for (int r = 0; r < run; ++r) {
                store_pixel(px);
            }
            continue;
        }

        index[qoi_color_hash(px.r, px.g, px.b, px.a)] = px;
        store_pixel(px);
    }

    if (px_idx < total_pixels) {
        return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
    }

    // Verify 8-byte padding
    if (qoi_data.size() < p + sizeof(QOI_PADDING) ||
        std::memcmp(qoi_data.data() + p, QOI_PADDING, sizeof(QOI_PADDING)) != 0) {
        // Warning or error on malformed padding
        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    }

    return ImageResult<Pixmap>::success(std::move(pixmap));
}

ImageResult<std::vector<uint8_t>> encode_qoi(
    PixmapRef pixmap,
    bool linear_colorspace
) {
    uint32_t w = pixmap.width();
    uint32_t h = pixmap.height();
    if (w == 0 || h == 0 || w > 32768 || h > 32768) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    size_t total_pixels = static_cast<size_t>(w) * h;
    // Maximum possible QOI size: 14 header + total_pixels * 5 + 8 padding
    std::vector<uint8_t> out;
    out.reserve(14 + total_pixels * 4 + sizeof(QOI_PADDING));

    // 1. Header
    out.insert(out.end(), QOI_MAGIC, QOI_MAGIC + 4);
    uint8_t dim_buf[4];
    write_u32_be(dim_buf, w);
    out.insert(out.end(), dim_buf, dim_buf + 4);
    write_u32_be(dim_buf, h);
    out.insert(out.end(), dim_buf, dim_buf + 4);
    out.push_back(4); // channels: 4 (RGBA)
    out.push_back(linear_colorspace ? 1 : 0); // colorspace

    // 2. Encoding Stream
    std::array<ColorU8, 64> index{};
    ColorU8 prev_px(0, 0, 0, 255);
    int run = 0;

    for (uint32_t y = 0; y < h; ++y) {
        const auto* row = pixmap.row(y);
        for (uint32_t x = 0; x < w; ++x) {
            ColorU8 px = row[x].demultiply();

            if (px == prev_px) {
                ++run;
                if (run == 62) {
                    out.push_back(static_cast<uint8_t>(QOI_OP_RUN | (run - 1)));
                    run = 0;
                }
            } else {
                if (run > 0) {
                    out.push_back(static_cast<uint8_t>(QOI_OP_RUN | (run - 1)));
                    run = 0;
                }

                size_t index_pos = qoi_color_hash(px.r, px.g, px.b, px.a);
                if (index[index_pos] == px) {
                    out.push_back(static_cast<uint8_t>(QOI_OP_INDEX | index_pos));
                } else {
                    index[index_pos] = px;

                    if (px.a == prev_px.a) {
                        int8_t vr = static_cast<int8_t>(px.r - prev_px.r);
                        int8_t vg = static_cast<int8_t>(px.g - prev_px.g);
                        int8_t vb = static_cast<int8_t>(px.b - prev_px.b);

                        int8_t vg_r = static_cast<int8_t>(vr - vg);
                        int8_t vg_b = static_cast<int8_t>(vb - vg);

                        if (vr >= -2 && vr <= 1 &&
                            vg >= -2 && vg <= 1 &&
                            vb >= -2 && vb <= 1) {
                            out.push_back(static_cast<uint8_t>(
                                QOI_OP_DIFF | ((vr + 2) << 4) | ((vg + 2) << 2) | (vb + 2)
                            ));
                        } else if (vg >= -32 && vg <= 31 &&
                                   vg_r >= -8 && vg_r <= 7 &&
                                   vg_b >= -8 && vg_b <= 7) {
                            out.push_back(static_cast<uint8_t>(QOI_OP_LUMA | (vg + 32)));
                            out.push_back(static_cast<uint8_t>(((vg_r + 8) << 4) | (vg_b + 8)));
                        } else {
                            out.push_back(QOI_OP_RGB);
                            out.push_back(px.r);
                            out.push_back(px.g);
                            out.push_back(px.b);
                        }
                    } else {
                        out.push_back(QOI_OP_RGBA);
                        out.push_back(px.r);
                        out.push_back(px.g);
                        out.push_back(px.b);
                        out.push_back(px.a);
                    }
                }
                prev_px = px;
            }
        }
    }

    if (run > 0) {
        out.push_back(static_cast<uint8_t>(QOI_OP_RUN | (run - 1)));
    }

    // 3. End Padding
    out.insert(out.end(), QOI_PADDING, QOI_PADDING + sizeof(QOI_PADDING));

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

ImageResult<Pixmap> decode_qoi_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }

    return decode_qoi(buffer);
}

bool save_qoi_file(
    PixmapRef pixmap,
    const std::string& file_path,
    bool linear_colorspace
) {
    auto encoded = encode_qoi(pixmap, linear_colorspace);
    if (!encoded) return false;

    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(encoded->data()), static_cast<std::streamsize>(encoded->size()));
    return file.good();
}

} // namespace nisaba::image

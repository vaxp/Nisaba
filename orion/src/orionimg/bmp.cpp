#include "orion/orionimg/bmp.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>
#include <bit>

namespace orionimg {

namespace {

inline uint16_t read_u16_le(const uint8_t* p) noexcept {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

inline uint32_t read_u32_le(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}

inline int32_t read_s32_le(const uint8_t* p) noexcept {
    return static_cast<int32_t>(read_u32_le(p));
}

inline void write_u16_le(uint8_t* p, uint16_t v) noexcept {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
}

inline void write_u32_le(uint8_t* p, uint32_t v) noexcept {
    p[0] = static_cast<uint8_t>(v & 0xFF);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
}

constexpr uint32_t BI_RGB       = 0;
constexpr uint32_t BI_RLE8      = 1;
constexpr uint32_t BI_RLE4      = 2;
constexpr uint32_t BI_BITFIELDS = 3;
constexpr uint32_t BI_ALPHABITFIELDS = 6;

inline uint8_t extract_masked_channel(uint32_t pixel, uint32_t mask, uint8_t default_val = 0) noexcept {
    if (mask == 0) return default_val;
    int shift = std::countr_zero(mask);
    uint32_t shifted_mask = mask >> shift;
    uint32_t val = (pixel & mask) >> shift;
    return static_cast<uint8_t>((val * 255U + (shifted_mask >> 1)) / shifted_mask);
}

struct BmpHeaderInfo {
    uint32_t data_offset{0};
    uint32_t dib_size{0};
    uint32_t width{0};
    uint32_t height{0};
    bool top_down{false};
    uint16_t bpp{0};
    uint32_t compression{0};
    uint32_t clr_used{0};
    uint32_t r_mask{0};
    uint32_t g_mask{0};
    uint32_t b_mask{0};
    uint32_t a_mask{0};
};

ImageResult<BmpHeaderInfo> parse_bmp_header(std::span<const uint8_t> data) {
    if (data.size() < 26) {
        return ImageResult<BmpHeaderInfo>::fail(ImageError::PrematureEndOfStream);
    }
    if (data[0] != 'B' || data[1] != 'M') {
        return ImageResult<BmpHeaderInfo>::fail(ImageError::InvalidSignature);
    }

    BmpHeaderInfo h{};
    h.data_offset = read_u32_le(data.data() + 10);
    h.dib_size = read_u32_le(data.data() + 14);

    if (14 + h.dib_size > data.size()) {
        return ImageResult<BmpHeaderInfo>::fail(ImageError::PrematureEndOfStream);
    }

    if (h.dib_size == 12) {
        // BITMAPCOREHEADER
        h.width = read_u16_le(data.data() + 18);
        h.height = read_u16_le(data.data() + 20);
        uint16_t planes = read_u16_le(data.data() + 22);
        h.bpp = read_u16_le(data.data() + 24);
        if (planes != 1) return ImageResult<BmpHeaderInfo>::fail(ImageError::CorruptedData);
    } else if (h.dib_size >= 40) {
        int32_t w = read_s32_le(data.data() + 18);
        int32_t raw_h = read_s32_le(data.data() + 22);
        if (w <= 0 || raw_h == 0) {
            return ImageResult<BmpHeaderInfo>::fail(ImageError::InvalidDimensions);
        }
        h.width = static_cast<uint32_t>(w);
        h.top_down = (raw_h < 0);
        h.height = static_cast<uint32_t>(raw_h < 0 ? -raw_h : raw_h);
        uint16_t planes = read_u16_le(data.data() + 26);
        h.bpp = read_u16_le(data.data() + 28);
        h.compression = read_u32_le(data.data() + 30);
        h.clr_used = read_u32_le(data.data() + 46);
        if (planes != 1) return ImageResult<BmpHeaderInfo>::fail(ImageError::CorruptedData);

        if (h.compression == BI_BITFIELDS || h.compression == BI_ALPHABITFIELDS) {
            size_t mask_offset = (h.dib_size >= 52) ? 54 : (14 + h.dib_size);
            if (mask_offset + 12 > data.size()) {
                return ImageResult<BmpHeaderInfo>::fail(ImageError::CorruptedData);
            }
            h.r_mask = read_u32_le(data.data() + mask_offset + 0);
            h.g_mask = read_u32_le(data.data() + mask_offset + 4);
            h.b_mask = read_u32_le(data.data() + mask_offset + 8);
            if (h.dib_size >= 56 || h.compression == BI_ALPHABITFIELDS) {
                if (mask_offset + 16 <= data.size()) {
                    h.a_mask = read_u32_le(data.data() + mask_offset + 12);
                }
            }
        } else if (h.bpp == 16) {
            h.r_mask = 0x7C00;
            h.g_mask = 0x03E0;
            h.b_mask = 0x001F;
        } else if (h.bpp == 32) {
            h.r_mask = 0x00FF0000;
            h.g_mask = 0x0000FF00;
            h.b_mask = 0x000000FF;
            h.a_mask = 0xFF000000;
        }
    } else {
        return ImageResult<BmpHeaderInfo>::fail(ImageError::UnsupportedFormat);
    }

    if (h.width == 0 || h.height == 0 || h.width > 32768 || h.height > 32768) {
        return ImageResult<BmpHeaderInfo>::fail(ImageError::InvalidDimensions);
    }

    return ImageResult<BmpHeaderInfo>::success(h);
}

} // namespace

ImageResult<ImageInfo> probe_bmp(std::span<const uint8_t> bmp_data) {
    auto res = parse_bmp_header(bmp_data);
    if (!res) return ImageResult<ImageInfo>::fail(res.error);
    const auto& h = *res;

    ImageInfo info{};
    info.width = h.width;
    info.height = h.height;
    info.bit_depth = (h.bpp <= 8) ? static_cast<uint8_t>(h.bpp) : 8;
    info.has_alpha = (h.bpp == 32);
    info.channels = info.has_alpha ? 4 : 3;
    info.format = ImageFormat::BMP;
    info.color_model = (h.bpp <= 8) ? ColorModel::Indexed : (info.has_alpha ? ColorModel::RGBA : ColorModel::RGB);
    return ImageResult<ImageInfo>::success(info);
}

ImageResult<Image> decode_bmp(std::span<const uint8_t> bmp_data, DecodeOptions options) {
    auto res = parse_bmp_header(bmp_data);
    if (!res) return ImageResult<Image>::fail(res.error);
    const auto& h = *res;

    if (h.data_offset > bmp_data.size()) {
        return ImageResult<Image>::fail(ImageError::PrematureEndOfStream);
    }

    auto maybe_pixmap = Image::allocate(h.width, h.height);
    if (!maybe_pixmap) return ImageResult<Image>::fail(ImageError::OutOfMemory);
    Image pixmap = std::move(*maybe_pixmap);
    PremultipliedColorU8* dst = pixmap.pixels_mut();

    auto write_px = [&](uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
        if (x >= h.width || y >= h.height) return;
        uint32_t dst_y = h.top_down ? y : (h.height - 1 - y);
        if (options.premultiply_alpha) {
            dst[dst_y * h.width + x] = ColorU8(r, g, b, a).premultiply();
        } else {
            dst[dst_y * h.width + x] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
        }
    };

    // Load palette for bpp <= 8
    std::array<ColorU8, 256> palette{};
    if (h.bpp <= 8) {
        uint32_t num_colors = h.clr_used ? h.clr_used : (1u << h.bpp);
        num_colors = std::min<uint32_t>(num_colors, 256);
        size_t pal_offset = 14 + h.dib_size;
        size_t entry_size = (h.dib_size == 12) ? 3 : 4;
        for (uint32_t i = 0; i < num_colors && pal_offset + entry_size <= bmp_data.size(); ++i) {
            uint8_t b = bmp_data[pal_offset + 0];
            uint8_t g = bmp_data[pal_offset + 1];
            uint8_t r = bmp_data[pal_offset + 2];
            palette[i] = ColorU8(r, g, b, 255);
            pal_offset += entry_size;
        }
    }

    const uint8_t* src = bmp_data.data() + h.data_offset;
    size_t src_avail = bmp_data.size() - h.data_offset;

    if (h.compression == BI_RLE8 && h.bpp == 8) {
        uint32_t x = 0, y = 0;
        size_t pos = 0;
        while (pos + 2 <= src_avail && y < h.height) {
            uint8_t count = src[pos++];
            uint8_t val = src[pos++];
            if (count > 0) {
                ColorU8 c = palette[val];
                for (uint8_t i = 0; i < count; ++i) {
                    write_px(x++, y, c.r, c.g, c.b, 255);
                }
            } else {
                if (val == 0) { // End of line
                    x = 0;
                    ++y;
                } else if (val == 1) { // End of bitmap
                    break;
                } else if (val == 2) { // Delta
                    if (pos + 2 > src_avail) break;
                    x += src[pos++];
                    y += src[pos++];
                } else { // Absolute mode
                    for (uint8_t i = 0; i < val && pos < src_avail; ++i) {
                        ColorU8 c = palette[src[pos++]];
                        write_px(x++, y, c.r, c.g, c.b, 255);
                    }
                    if (val & 1) ++pos; // Pad to word boundary
                }
            }
        }
        return ImageResult<Image>::success(std::move(pixmap));
    }

    if (h.compression == BI_RLE4 && h.bpp == 4) {
        uint32_t x = 0, y = 0;
        size_t pos = 0;
        while (pos + 2 <= src_avail && y < h.height) {
            uint8_t count = src[pos++];
            uint8_t val = src[pos++];
            if (count > 0) {
                ColorU8 c0 = palette[(val >> 4) & 0x0F];
                ColorU8 c1 = palette[val & 0x0F];
                for (uint8_t i = 0; i < count; ++i) {
                    const ColorU8& c = (i & 1) ? c1 : c0;
                    write_px(x++, y, c.r, c.g, c.b, 255);
                }
            } else {
                if (val == 0) {
                    x = 0;
                    ++y;
                } else if (val == 1) {
                    break;
                } else if (val == 2) {
                    if (pos + 2 > src_avail) break;
                    x += src[pos++];
                    y += src[pos++];
                } else {
                    uint8_t nbytes = (val + 1) / 2;
                    for (uint8_t i = 0; i < val; ++i) {
                        size_t byte_idx = pos + (i >> 1);
                        if (byte_idx >= src_avail) break;
                        uint8_t b = src[byte_idx];
                        uint8_t idx = (i & 1) ? (b & 0x0F) : ((b >> 4) & 0x0F);
                        ColorU8 c = palette[idx];
                        write_px(x++, y, c.r, c.g, c.b, 255);
                    }
                    pos += nbytes + (nbytes & 1);
                }
            }
        }
        return ImageResult<Image>::success(std::move(pixmap));
    }

    if (h.compression != BI_RGB && h.compression != BI_BITFIELDS && h.compression != BI_ALPHABITFIELDS) {
        return ImageResult<Image>::fail(ImageError::UnsupportedFeature);
    }

    size_t row_stride = ((static_cast<size_t>(h.width) * h.bpp + 31) / 32) * 4;
    if (row_stride * h.height > src_avail) {
        return ImageResult<Image>::fail(ImageError::PrematureEndOfStream);
    }

    // For 32-bit BI_RGB BMPs, check if any pixel has non-zero alpha; if all alpha == 0, treat as opaque 255
    bool force_opaque_32 = false;
    if (h.bpp == 32 && (h.compression == BI_RGB || h.a_mask == 0)) {
        bool any_alpha = false;
        for (uint32_t y = 0; y < h.height && !any_alpha; ++y) {
            const uint8_t* row = src + y * row_stride;
            for (uint32_t x = 0; x < h.width; ++x) {
                if (row[x * 4 + 3] != 0) {
                    any_alpha = true;
                    break;
                }
            }
        }
        if (!any_alpha) force_opaque_32 = true;
    }

    // Fast path: 24-bit uncompressed RGB
    if (h.bpp == 24) {
        for (uint32_t y = 0; y < h.height; ++y) {
            uint32_t dst_y = h.top_down ? y : (h.height - 1 - y);
            PremultipliedColorU8* dst_row = dst + dst_y * h.width;
            const uint8_t* row = src + y * row_stride;
            uint32_t x = 0;
            for (; x + 4 <= h.width; x += 4) {
                dst_row[x + 0] = PremultipliedColorU8::from_rgba_unchecked(row[(x + 0) * 3 + 2], row[(x + 0) * 3 + 1], row[(x + 0) * 3 + 0], 255);
                dst_row[x + 1] = PremultipliedColorU8::from_rgba_unchecked(row[(x + 1) * 3 + 2], row[(x + 1) * 3 + 1], row[(x + 1) * 3 + 0], 255);
                dst_row[x + 2] = PremultipliedColorU8::from_rgba_unchecked(row[(x + 2) * 3 + 2], row[(x + 2) * 3 + 1], row[(x + 2) * 3 + 0], 255);
                dst_row[x + 3] = PremultipliedColorU8::from_rgba_unchecked(row[(x + 3) * 3 + 2], row[(x + 3) * 3 + 1], row[(x + 3) * 3 + 0], 255);
            }
            for (; x < h.width; ++x) {
                dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(row[x * 3 + 2], row[x * 3 + 1], row[x * 3 + 0], 255);
            }
        }
        return ImageResult<Image>::success(std::move(pixmap));
    }

    // Fast path: Standard 32-bit BGRA
    if (h.bpp == 32 && h.r_mask == 0x00FF0000 && h.g_mask == 0x0000FF00 && h.b_mask == 0x000000FF) {
        for (uint32_t y = 0; y < h.height; ++y) {
            uint32_t dst_y = h.top_down ? y : (h.height - 1 - y);
            PremultipliedColorU8* dst_row = dst + dst_y * h.width;
            const uint8_t* row = src + y * row_stride;
            for (uint32_t x = 0; x < h.width; ++x) {
                uint8_t b = row[x * 4 + 0];
                uint8_t g = row[x * 4 + 1];
                uint8_t r = row[x * 4 + 2];
                uint8_t a = force_opaque_32 ? 255 : row[x * 4 + 3];
                if (options.premultiply_alpha && a != 255) {
                    dst_row[x] = ColorU8(r, g, b, a).premultiply();
                } else {
                    dst_row[x] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
                }
            }
        }
        return ImageResult<Image>::success(std::move(pixmap));
    }

    for (uint32_t y = 0; y < h.height; ++y) {
        const uint8_t* row = src + y * row_stride;
        for (uint32_t x = 0; x < h.width; ++x) {
            if (h.bpp == 24) {
                uint8_t b = row[x * 3 + 0];
                uint8_t g = row[x * 3 + 1];
                uint8_t r = row[x * 3 + 2];
                write_px(x, y, r, g, b, 255);
            } else if (h.bpp == 32) {
                uint32_t px = read_u32_le(row + x * 4);
                uint8_t r = extract_masked_channel(px, h.r_mask);
                uint8_t g = extract_masked_channel(px, h.g_mask);
                uint8_t b = extract_masked_channel(px, h.b_mask);
                uint8_t a = force_opaque_32 ? 255 : extract_masked_channel(px, h.a_mask, 255);
                write_px(x, y, r, g, b, a);
            } else if (h.bpp == 16) {
                uint16_t px = read_u16_le(row + x * 2);
                uint8_t r = extract_masked_channel(px, h.r_mask);
                uint8_t g = extract_masked_channel(px, h.g_mask);
                uint8_t b = extract_masked_channel(px, h.b_mask);
                uint8_t a = extract_masked_channel(px, h.a_mask, 255);
                write_px(x, y, r, g, b, a);
            } else if (h.bpp == 8) {
                ColorU8 c = palette[row[x]];
                write_px(x, y, c.r, c.g, c.b, 255);
            } else if (h.bpp == 4) {
                uint8_t b = row[x >> 1];
                uint8_t idx = (x & 1) ? (b & 0x0F) : ((b >> 4) & 0x0F);
                ColorU8 c = palette[idx];
                write_px(x, y, c.r, c.g, c.b, 255);
            } else if (h.bpp == 1) {
                uint8_t b = row[x >> 3];
                uint8_t idx = (b >> (7 - (x & 7))) & 1;
                ColorU8 c = palette[idx];
                write_px(x, y, c.r, c.g, c.b, 255);
            } else {
                return ImageResult<Image>::fail(ImageError::UnsupportedFormat);
            }
        }
    }

    return ImageResult<Image>::success(std::move(pixmap));
}

ImageResult<std::vector<uint8_t>> encode_bmp(ImageRef pixmap) {
    uint32_t w = pixmap.width();
    uint32_t h = pixmap.height();
    if (w == 0 || h == 0) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    bool has_alpha = false;
    for (uint32_t y = 0; y < h && !has_alpha; ++y) {
        const auto* row = pixmap.row(y);
        for (uint32_t x = 0; x < w; ++x) {
            if (row[x].a != 255) {
                has_alpha = true;
                break;
            }
        }
    }

    uint16_t bpp = has_alpha ? 32 : 24;
    size_t bytes_per_px = bpp / 8;
    size_t row_stride = ((static_cast<size_t>(w) * bpp + 31) / 32) * 4;
    size_t data_size = row_stride * h;
    size_t file_size = 54 + data_size;

    std::vector<uint8_t> out(file_size, 0);

    // BITMAPFILEHEADER (14 bytes)
    out[0] = 'B';
    out[1] = 'M';
    write_u32_le(out.data() + 2, static_cast<uint32_t>(file_size));
    write_u32_le(out.data() + 10, 54);

    // BITMAPINFOHEADER (40 bytes)
    write_u32_le(out.data() + 14, 40);
    write_u32_le(out.data() + 18, w);
    write_u32_le(out.data() + 22, h); // Bottom-up positive height
    write_u16_le(out.data() + 26, 1); // Planes
    write_u16_le(out.data() + 28, bpp);
    write_u32_le(out.data() + 30, BI_RGB);
    write_u32_le(out.data() + 34, static_cast<uint32_t>(data_size));
    write_u32_le(out.data() + 38, 2835); // 72 DPI
    write_u32_le(out.data() + 42, 2835);

    uint8_t* dst_base = out.data() + 54;
    for (uint32_t y = 0; y < h; ++y) {
        const auto* src_row = pixmap.row(h - 1 - y);
        uint8_t* dst_row = dst_base + y * row_stride;
        for (uint32_t x = 0; x < w; ++x) {
            ColorU8 c = src_row[x].demultiply();
            dst_row[x * bytes_per_px + 0] = c.b;
            dst_row[x * bytes_per_px + 1] = c.g;
            dst_row[x * bytes_per_px + 2] = c.r;
            if (has_alpha) {
                dst_row[x * bytes_per_px + 3] = c.a;
            }
        }
    }

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

ImageResult<Image> decode_bmp_file(const std::string& file_path, DecodeOptions options) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return ImageResult<Image>::fail(ImageError::IOError);
    std::streamsize size = file.tellg();
    if (size <= 0) return ImageResult<Image>::fail(ImageError::CorruptedData);
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Image>::fail(ImageError::IOError);
    }
    return decode_bmp(buffer, options);
}

bool save_bmp_file(ImageRef pixmap, const std::string& file_path) {
    auto encoded = encode_bmp(pixmap);
    if (!encoded) return false;
    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(encoded->data()), encoded->size());
    return file.good();
}

} // namespace orionimg

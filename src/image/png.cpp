#include "nisaba/image/png.hpp"
#include "nisaba/image/deflate.hpp"
#include <fstream>
#include <cstring>
#include <cmath>
#include <algorithm>

#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h>
#endif

namespace nisaba::image {

namespace {

constexpr uint8_t PNG_SIGNATURE[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

inline uint32_t read_u32_be(const uint8_t* p) noexcept {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
           (static_cast<uint32_t>(p[3]));
}

inline void write_u32_be(uint8_t* p, uint32_t val) noexcept {
    p[0] = static_cast<uint8_t>((val >> 24) & 0xFF);
    p[1] = static_cast<uint8_t>((val >> 16) & 0xFF);
    p[2] = static_cast<uint8_t>((val >> 8) & 0xFF);
    p[3] = static_cast<uint8_t>(val & 0xFF);
}

inline uint8_t paeth_predictor(int a, int b, int c) noexcept {
    int da = b - c;
    int db = a - c;
    int pa = std::abs(da);
    int pb = std::abs(db);
    int pc = std::abs(da + db);
    if (pa <= pb && pa <= pc) return static_cast<uint8_t>(a);
    if (pb <= pc) return static_cast<uint8_t>(b);
    return static_cast<uint8_t>(c);
}

#if defined(__x86_64__) || defined(_M_X64)
inline __m128i paeth_sse2_4ch(__m128i a, __m128i b, __m128i c) noexcept {
    __m128i zero = _mm_setzero_si128();
    __m128i da = _mm_sub_epi16(b, c);
    __m128i db = _mm_sub_epi16(a, c);
    __m128i dc = _mm_add_epi16(da, db);

    __m128i pa = _mm_max_epi16(_mm_sub_epi16(zero, da), da);
    __m128i pb = _mm_max_epi16(_mm_sub_epi16(zero, db), db);
    __m128i pc = _mm_max_epi16(_mm_sub_epi16(zero, dc), dc);

    __m128i pa_le_pb = _mm_cmpeq_epi16(_mm_min_epi16(pa, pb), pa);
    __m128i pa_le_pc = _mm_cmpeq_epi16(_mm_min_epi16(pa, pc), pa);
    __m128i mask_a = _mm_and_si128(pa_le_pb, pa_le_pc);

    __m128i pb_le_pc = _mm_cmpeq_epi16(_mm_min_epi16(pb, pc), pb);
    __m128i mask_b = _mm_andnot_si128(mask_a, pb_le_pc);

    __m128i res = _mm_or_si128(_mm_and_si128(mask_a, a),
                  _mm_or_si128(_mm_and_si128(mask_b, b),
                  _mm_andnot_si128(_mm_or_si128(mask_a, mask_b), c)));
    return res;
}
#endif

struct PngHeader {
    uint32_t width{0};
    uint32_t height{0};
    uint8_t bit_depth{0};
    uint8_t color_type{0};
    uint8_t compression{0};
    uint8_t filter{0};
    uint8_t interlace{0};
};

// Adam7 pass parameters: {x0, y0, dx, dy}
constexpr int ADAM7_X0[7] = {0, 4, 0, 2, 0, 1, 0};
constexpr int ADAM7_Y0[7] = {0, 0, 4, 0, 2, 0, 1};
constexpr int ADAM7_DX[7] = {8, 8, 4, 4, 2, 2, 1};
constexpr int ADAM7_DY[7] = {8, 8, 8, 4, 4, 2, 2};

void unfilter_scanline(
    uint8_t filter_type,
    uint8_t* curr,
    const uint8_t* prior,
    size_t line_bytes,
    int bpp
) {
    switch (filter_type) {
        case 0: // None
            break;
        case 1: { // Sub
            if (bpp == 4) {
                for (size_t x = 4; x < line_bytes; x += 4) {
                    curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + curr[x - 4 + 0]);
                    curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + curr[x - 4 + 1]);
                    curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + curr[x - 4 + 2]);
                    curr[x + 3] = static_cast<uint8_t>(curr[x + 3] + curr[x - 4 + 3]);
                }
            } else if (bpp == 3) {
                for (size_t x = 3; x < line_bytes; x += 3) {
                    curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + curr[x - 3 + 0]);
                    curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + curr[x - 3 + 1]);
                    curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + curr[x - 3 + 2]);
                }
            } else {
                for (size_t x = static_cast<size_t>(bpp); x < line_bytes; ++x) {
                    curr[x] = static_cast<uint8_t>(curr[x] + curr[x - bpp]);
                }
            }
            break;
        }
        case 2: { // Up
            if (prior) {
                size_t x = 0;
#if defined(__x86_64__) || defined(_M_X64)
                for (; x + 16 <= line_bytes; x += 16) {
                    __m128i c = _mm_loadu_si128(reinterpret_cast<const __m128i*>(curr + x));
                    __m128i p = _mm_loadu_si128(reinterpret_cast<const __m128i*>(prior + x));
                    _mm_storeu_si128(reinterpret_cast<__m128i*>(curr + x), _mm_add_epi8(c, p));
                }
#endif
                for (; x < line_bytes; ++x) {
                    curr[x] = static_cast<uint8_t>(curr[x] + prior[x]);
                }
            }
            break;
        }
        case 3: { // Average
            if (prior) {
                if (bpp == 4) {
                    for (size_t x = 0; x < 4 && x < line_bytes; ++x) {
                        curr[x] = static_cast<uint8_t>(curr[x] + (prior[x] >> 1));
                    }
                    for (size_t x = 4; x < line_bytes; x += 4) {
                        curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + ((curr[x - 4 + 0] + prior[x + 0]) >> 1));
                        curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + ((curr[x - 4 + 1] + prior[x + 1]) >> 1));
                        curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + ((curr[x - 4 + 2] + prior[x + 2]) >> 1));
                        curr[x + 3] = static_cast<uint8_t>(curr[x + 3] + ((curr[x - 4 + 3] + prior[x + 3]) >> 1));
                    }
                } else if (bpp == 3) {
                    for (size_t x = 0; x < 3 && x < line_bytes; ++x) {
                        curr[x] = static_cast<uint8_t>(curr[x] + (prior[x] >> 1));
                    }
                    for (size_t x = 3; x < line_bytes; x += 3) {
                        curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + ((curr[x - 3 + 0] + prior[x + 0]) >> 1));
                        curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + ((curr[x - 3 + 1] + prior[x + 1]) >> 1));
                        curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + ((curr[x - 3 + 2] + prior[x + 2]) >> 1));
                    }
                } else {
                    for (size_t x = 0; x < static_cast<size_t>(bpp) && x < line_bytes; ++x) {
                        curr[x] = static_cast<uint8_t>(curr[x] + (prior[x] >> 1));
                    }
                    for (size_t x = static_cast<size_t>(bpp); x < line_bytes; ++x) {
                        int a = curr[x - bpp];
                        int b = prior[x];
                        curr[x] = static_cast<uint8_t>(curr[x] + ((a + b) >> 1));
                    }
                }
            } else {
                if (bpp == 4) {
                    for (size_t x = 4; x < line_bytes; x += 4) {
                        curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + (curr[x - 4 + 0] >> 1));
                        curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + (curr[x - 4 + 1] >> 1));
                        curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + (curr[x - 4 + 2] >> 1));
                        curr[x + 3] = static_cast<uint8_t>(curr[x + 3] + (curr[x - 4 + 3] >> 1));
                    }
                } else {
                    for (size_t x = static_cast<size_t>(bpp); x < line_bytes; ++x) {
                        int a = curr[x - bpp];
                        curr[x] = static_cast<uint8_t>(curr[x] + (a >> 1));
                    }
                }
            }
            break;
        }
        case 4: { // Paeth
            if (!prior) {
                // When prior is null: paeth(a, 0, 0) == a, identical to Sub
                if (bpp == 4) {
                    for (size_t x = 4; x < line_bytes; x += 4) {
                        curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + curr[x - 4 + 0]);
                        curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + curr[x - 4 + 1]);
                        curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + curr[x - 4 + 2]);
                        curr[x + 3] = static_cast<uint8_t>(curr[x + 3] + curr[x - 4 + 3]);
                    }
                } else {
                    for (size_t x = static_cast<size_t>(bpp); x < line_bytes; ++x) {
                        curr[x] = static_cast<uint8_t>(curr[x] + curr[x - bpp]);
                    }
                }
                break;
            }
            size_t first = std::min(static_cast<size_t>(bpp), line_bytes);
            for (size_t x = 0; x < first; ++x) {
                curr[x] = static_cast<uint8_t>(curr[x] + prior[x]);
            }
            if (bpp == 4) {
#if defined(__x86_64__) || defined(_M_X64)
                __m128i zero = _mm_setzero_si128();
                for (size_t x = 4; x < line_bytes; x += 4) {
                    uint32_t a_val, b_val, c_val, raw_val;
                    std::memcpy(&a_val, curr + x - 4, 4);
                    std::memcpy(&b_val, prior + x, 4);
                    std::memcpy(&c_val, prior + x - 4, 4);
                    std::memcpy(&raw_val, curr + x, 4);

                    __m128i a = _mm_unpacklo_epi8(_mm_cvtsi32_si128(static_cast<int>(a_val)), zero);
                    __m128i b = _mm_unpacklo_epi8(_mm_cvtsi32_si128(static_cast<int>(b_val)), zero);
                    __m128i c = _mm_unpacklo_epi8(_mm_cvtsi32_si128(static_cast<int>(c_val)), zero);

                    __m128i pred = paeth_sse2_4ch(a, b, c);
                    __m128i pred_8bit = _mm_packus_epi16(pred, zero);
                    __m128i raw_8bit = _mm_cvtsi32_si128(static_cast<int>(raw_val));
                    __m128i out_8bit = _mm_add_epi8(raw_8bit, pred_8bit);
                    uint32_t out_val = static_cast<uint32_t>(_mm_cvtsi128_si32(out_8bit));
                    std::memcpy(curr + x, &out_val, 4);
                }
#else
                for (size_t x = 4; x < line_bytes; x += 4) {
                    curr[x + 0] = static_cast<uint8_t>(curr[x + 0] + paeth_predictor(curr[x - 4 + 0], prior[x + 0], prior[x - 4 + 0]));
                    curr[x + 1] = static_cast<uint8_t>(curr[x + 1] + paeth_predictor(curr[x - 4 + 1], prior[x + 1], prior[x - 4 + 1]));
                    curr[x + 2] = static_cast<uint8_t>(curr[x + 2] + paeth_predictor(curr[x - 4 + 2], prior[x + 2], prior[x - 4 + 2]));
                    curr[x + 3] = static_cast<uint8_t>(curr[x + 3] + paeth_predictor(curr[x - 4 + 3], prior[x + 3], prior[x - 4 + 3]));
                }
#endif
            } else {
                for (size_t x = first; x < line_bytes; ++x) {
                    int a = curr[x - bpp];
                    int b = prior[x];
                    int c = prior[x - bpp];
                    curr[x] = static_cast<uint8_t>(curr[x] + paeth_predictor(a, b, c));
                }
            }
            break;
        }
        default:
            break;
    }
}

} // namespace

ImageResult<ImageInfo> probe_png(std::span<const uint8_t> png_data) {
    if (png_data.size() < 33) {
        return ImageResult<ImageInfo>::fail(ImageError::PrematureEndOfStream);
    }
    if (std::memcmp(png_data.data(), PNG_SIGNATURE, 8) != 0) {
        return ImageResult<ImageInfo>::fail(ImageError::InvalidSignature);
    }

    uint32_t chunk_len = read_u32_be(png_data.data() + 8);
    if (std::memcmp(png_data.data() + 12, "IHDR", 4) != 0 || chunk_len < 13) {
        return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
    }

    const uint8_t* ihdr = png_data.data() + 16;
    ImageInfo info{};
    info.width = read_u32_be(ihdr);
    info.height = read_u32_be(ihdr + 4);
    info.bit_depth = ihdr[8];
    uint8_t color_type = ihdr[9];
    info.format = ImageFormat::PNG;

    switch (color_type) {
        case 0: // Grayscale
            info.channels = 1;
            info.has_alpha = false;
            info.color_model = ColorModel::Grayscale;
            break;
        case 2: // RGB
            info.channels = 3;
            info.has_alpha = false;
            info.color_model = ColorModel::RGB;
            break;
        case 3: // Indexed
            info.channels = 1;
            info.has_alpha = false; // May have tRNS
            info.color_model = ColorModel::Indexed;
            break;
        case 4: // Grayscale + Alpha
            info.channels = 2;
            info.has_alpha = true;
            info.color_model = ColorModel::GrayscaleAlpha;
            break;
        case 6: // RGBA
            info.channels = 4;
            info.has_alpha = true;
            info.color_model = ColorModel::RGBA;
            break;
        default:
            return ImageResult<ImageInfo>::fail(ImageError::UnsupportedFormat);
    }

    return ImageResult<ImageInfo>::success(info);
}

ImageResult<Pixmap> decode_png(std::span<const uint8_t> png_data) {
    if (png_data.size() < 8 || std::memcmp(png_data.data(), PNG_SIGNATURE, 8) != 0) {
        return ImageResult<Pixmap>::fail(ImageError::InvalidSignature);
    }

    size_t offset = 8;
    PngHeader ihdr{};
    bool has_ihdr = false;
    std::vector<uint8_t> idat_data;
    std::vector<uint8_t> palette;
    std::vector<uint8_t> trns;

    while (offset + 8 <= png_data.size()) {
        uint32_t chunk_len = read_u32_be(png_data.data() + offset);
        const uint8_t* type = png_data.data() + offset + 4;
        offset += 8;

        if (offset + chunk_len + 4 > png_data.size()) {
            return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
        }

        const uint8_t* data = png_data.data() + offset;

        if (std::memcmp(type, "IHDR", 4) == 0) {
            if (chunk_len < 13) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            ihdr.width = read_u32_be(data);
            ihdr.height = read_u32_be(data + 4);
            ihdr.bit_depth = data[8];
            ihdr.color_type = data[9];
            ihdr.compression = data[10];
            ihdr.filter = data[11];
            ihdr.interlace = data[12];
            has_ihdr = true;
        } else if (std::memcmp(type, "PLTE", 4) == 0) {
            palette.assign(data, data + chunk_len);
        } else if (std::memcmp(type, "tRNS", 4) == 0) {
            trns.assign(data, data + chunk_len);
        } else if (std::memcmp(type, "IDAT", 4) == 0) {
            idat_data.insert(idat_data.end(), data, data + chunk_len);
        } else if (std::memcmp(type, "IEND", 4) == 0) {
            break;
        }

        offset += chunk_len + 4; // Skip data + CRC
    }

    if (!has_ihdr || ihdr.width == 0 || ihdr.height == 0) {
        return ImageResult<Pixmap>::fail(ImageError::InvalidDimensions);
    }
    if (ihdr.compression != 0 || ihdr.filter != 0) {
        return ImageResult<Pixmap>::fail(ImageError::UnsupportedFeature);
    }

    int channels = 0;
    switch (ihdr.color_type) {
        case 0: channels = 1; break; // Gray
        case 2: channels = 3; break; // RGB
        case 3: channels = 1; break; // Palette
        case 4: channels = 2; break; // Gray+Alpha
        case 6: channels = 4; break; // RGBA
        default: return ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
    }

    int bits_per_pixel = channels * ihdr.bit_depth;
    int bpp = (bits_per_pixel + 7) / 8;
    if (bpp < 1) bpp = 1;

    size_t row_bytes = (static_cast<size_t>(ihdr.width) * bits_per_pixel + 7) / 8;
    size_t expected_size = (ihdr.interlace == 0)
        ? static_cast<size_t>(ihdr.height) * (row_bytes + 1)
        : 0;

    // Decompress IDAT stream with expected size hint and skip redundant checksum
    auto raw_data = zlib_decompress(idat_data, expected_size, false);
    if (!raw_data) {
        return ImageResult<Pixmap>::fail(raw_data.error);
    }

    auto maybe_pixmap = Pixmap::allocate(ihdr.width, ihdr.height);
    if (!maybe_pixmap) {
        return ImageResult<Pixmap>::fail(ImageError::OutOfMemory);
    }
    Pixmap pixmap = std::move(*maybe_pixmap);

    auto extract_pixel = [&](const uint8_t* line, size_t x) -> ColorU8 {
        if (ihdr.color_type == 6) { // RGBA
            if (ihdr.bit_depth == 8) {
                const uint8_t* px = line + x * 4;
                return ColorU8(px[0], px[1], px[2], px[3]);
            } else if (ihdr.bit_depth == 16) {
                const uint8_t* px = line + x * 8;
                return ColorU8(px[0], px[2], px[4], px[6]);
            }
        } else if (ihdr.color_type == 2) { // RGB
            if (ihdr.bit_depth == 8) {
                const uint8_t* px = line + x * 3;
                uint8_t a = 255;
                if (trns.size() >= 6 && px[0] == trns[1] && px[1] == trns[3] && px[2] == trns[5]) {
                    a = 0;
                }
                return ColorU8(px[0], px[1], px[2], a);
            } else if (ihdr.bit_depth == 16) {
                const uint8_t* px = line + x * 6;
                return ColorU8(px[0], px[2], px[4], 255);
            }
        } else if (ihdr.color_type == 0) { // Grayscale
            uint8_t y = 0;
            if (ihdr.bit_depth == 8) {
                y = line[x];
            } else if (ihdr.bit_depth == 16) {
                y = line[x * 2];
            } else { // 1, 2, 4 bits
                int shift = (8 - ihdr.bit_depth) - (x % (8 / ihdr.bit_depth)) * ihdr.bit_depth;
                uint8_t mask = (1 << ihdr.bit_depth) - 1;
                uint8_t raw = (line[x / (8 / ihdr.bit_depth)] >> shift) & mask;
                y = static_cast<uint8_t>(raw * (255 / mask));
            }
            uint8_t a = 255;
            if (trns.size() >= 2 && y == trns[1]) {
                a = 0;
            }
            return ColorU8(y, y, y, a);
        } else if (ihdr.color_type == 4) { // Grayscale + Alpha
            if (ihdr.bit_depth == 8) {
                const uint8_t* px = line + x * 2;
                return ColorU8(px[0], px[0], px[0], px[1]);
            } else if (ihdr.bit_depth == 16) {
                const uint8_t* px = line + x * 4;
                return ColorU8(px[0], px[0], px[0], px[2]);
            }
        } else if (ihdr.color_type == 3) { // Palette
            uint8_t idx = 0;
            if (ihdr.bit_depth == 8) {
                idx = line[x];
            } else {
                int shift = (8 - ihdr.bit_depth) - (x % (8 / ihdr.bit_depth)) * ihdr.bit_depth;
                uint8_t mask = (1 << ihdr.bit_depth) - 1;
                idx = (line[x / (8 / ihdr.bit_depth)] >> shift) & mask;
            }
            if (static_cast<size_t>(idx) * 3 + 2 < palette.size()) {
                uint8_t r = palette[idx * 3 + 0];
                uint8_t g = palette[idx * 3 + 1];
                uint8_t b = palette[idx * 3 + 2];
                uint8_t a = (idx < trns.size()) ? trns[idx] : 255;
                return ColorU8(r, g, b, a);
            }
        }
        return ColorU8(0, 0, 0, 0);
    };

    // Pre-calculate palette lookup table for indexed images
    std::array<PremultipliedColorU8, 256> palette_lut{};
    if (ihdr.color_type == 3) {
        size_t num_colors = palette.size() / 3;
        for (size_t i = 0; i < 256; ++i) {
            if (i < num_colors) {
                uint8_t r = palette[i * 3 + 0];
                uint8_t g = palette[i * 3 + 1];
                uint8_t b = palette[i * 3 + 2];
                uint8_t a = (i < trns.size()) ? trns[i] : 255;
                if (a == 255) {
                    palette_lut[i] = PremultipliedColorU8(r, g, b, 255);
                } else if (a == 0) {
                    palette_lut[i] = PremultipliedColorU8(0, 0, 0, 0);
                } else {
                    palette_lut[i] = ColorU8(r, g, b, a).premultiply();
                }
            } else {
                palette_lut[i] = PremultipliedColorU8(0, 0, 0, 0);
            }
        }
    }

    if (ihdr.interlace == 0) {
        // Non-interlaced: decode and unfilter directly in-place
        size_t scanline_stride = row_bytes + 1;
        if (raw_data->size() < static_cast<size_t>(ihdr.height) * scanline_stride) {
            return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
        }

        uint8_t* raw_buf = raw_data->data();

        for (uint32_t y = 0; y < ihdr.height; ++y) {
            size_t src_idx = y * scanline_stride;
            uint8_t filter_type = raw_buf[src_idx];
            uint8_t* curr = raw_buf + src_idx + 1;
            const uint8_t* prior = (y > 0) ? (raw_buf + (y - 1) * scanline_stride + 1) : nullptr;

            unfilter_scanline(filter_type, curr, prior, row_bytes, bpp);

            if (ihdr.color_type == 6 && ihdr.bit_depth == 8) {
                const uint8_t* px = curr;
                PremultipliedColorU8* dst = pixmap.pixels_mut() + y * ihdr.width;
                uint32_t x = 0;
#if defined(__x86_64__) || defined(_M_X64)
                __m128i all_255 = _mm_set1_epi32(static_cast<int>(0xFF000000));
                for (; x + 4 <= ihdr.width; x += 4) {
                    __m128i p = _mm_loadu_si128(reinterpret_cast<const __m128i*>(px));
                    __m128i alphas = _mm_and_si128(p, all_255);
                    __m128i cmp = _mm_cmpeq_epi32(alphas, all_255);
                    int mask = _mm_movemask_epi8(cmp);
                    if (mask == 0xFFFF) {
                        _mm_storeu_si128(reinterpret_cast<__m128i*>(dst + x), p);
                    } else {
                        for (int k = 0; k < 4; ++k) {
                            uint8_t a = px[k * 4 + 3];
                            if (a == 255) {
                                dst[x + k] = PremultipliedColorU8(px[k*4+0], px[k*4+1], px[k*4+2], 255);
                            } else if (a == 0) {
                                dst[x + k] = PremultipliedColorU8(0, 0, 0, 0);
                            } else {
                                dst[x + k] = ColorU8(px[k*4+0], px[k*4+1], px[k*4+2], a).premultiply();
                            }
                        }
                    }
                    px += 16;
                }
#endif
                for (; x < ihdr.width; ++x) {
                    uint8_t a = px[3];
                    if (a == 255) {
                        dst[x] = PremultipliedColorU8(px[0], px[1], px[2], 255);
                    } else if (a == 0) {
                        dst[x] = PremultipliedColorU8(0, 0, 0, 0);
                    } else {
                        dst[x] = ColorU8(px[0], px[1], px[2], a).premultiply();
                    }
                    px += 4;
                }
            } else if (ihdr.color_type == 2 && ihdr.bit_depth == 8 && trns.empty()) {
                const uint8_t* px = curr;
                PremultipliedColorU8* dst = pixmap.pixels_mut() + y * ihdr.width;
                uint32_t x = 0;
                for (; x + 4 <= ihdr.width; x += 4) {
                    dst[x + 0] = PremultipliedColorU8(px[0], px[1], px[2], 255);
                    dst[x + 1] = PremultipliedColorU8(px[3], px[4], px[5], 255);
                    dst[x + 2] = PremultipliedColorU8(px[6], px[7], px[8], 255);
                    dst[x + 3] = PremultipliedColorU8(px[9], px[10], px[11], 255);
                    px += 12;
                }
                for (; x < ihdr.width; ++x) {
                    dst[x] = PremultipliedColorU8(px[0], px[1], px[2], 255);
                    px += 3;
                }
            } else if (ihdr.color_type == 3 && ihdr.bit_depth == 8) {
                const uint8_t* px = curr;
                PremultipliedColorU8* dst = pixmap.pixels_mut() + y * ihdr.width;
                uint32_t x = 0;
                for (; x + 4 <= ihdr.width; x += 4) {
                    dst[x + 0] = palette_lut[px[x + 0]];
                    dst[x + 1] = palette_lut[px[x + 1]];
                    dst[x + 2] = palette_lut[px[x + 2]];
                    dst[x + 3] = palette_lut[px[x + 3]];
                }
                for (; x < ihdr.width; ++x) {
                    dst[x] = palette_lut[px[x]];
                }
            } else {
                for (uint32_t x = 0; x < ihdr.width; ++x) {
                    ColorU8 c = extract_pixel(curr, x);
                    pixmap.set_pixel(x, y, c.premultiply());
                }
            }
        }
    } else if (ihdr.interlace == 1) {
        // Adam7 Interlaced
        size_t src_idx = 0;

        for (int pass = 0; pass < 7; ++pass) {
            int x0 = ADAM7_X0[pass];
            int y0 = ADAM7_Y0[pass];
            int dx = ADAM7_DX[pass];
            int dy = ADAM7_DY[pass];

            uint32_t pass_w = (ihdr.width > static_cast<uint32_t>(x0)) ?
                              (ihdr.width - x0 + dx - 1) / dx : 0;
            uint32_t pass_h = (ihdr.height > static_cast<uint32_t>(y0)) ?
                              (ihdr.height - y0 + dy - 1) / dy : 0;

            if (pass_w == 0 || pass_h == 0) continue;

            size_t pass_row_bytes = (static_cast<size_t>(pass_w) * bits_per_pixel + 7) / 8;
            std::vector<uint8_t> curr_row(pass_row_bytes);
            std::vector<uint8_t> prior_row(pass_row_bytes, 0);

            for (uint32_t py = 0; py < pass_h; ++py) {
                if (src_idx >= raw_data->size()) {
                    return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
                }
                uint8_t filter_type = (*raw_data)[src_idx++];
                if (src_idx + pass_row_bytes > raw_data->size()) {
                    return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
                }

                std::memcpy(curr_row.data(), raw_data->data() + src_idx, pass_row_bytes);
                src_idx += pass_row_bytes;

                unfilter_scanline(filter_type, curr_row.data(), (py > 0) ? prior_row.data() : nullptr, pass_row_bytes, bpp);

                uint32_t dst_y = y0 + py * dy;
                for (uint32_t px = 0; px < pass_w; ++px) {
                    uint32_t dst_x = x0 + px * dx;
                    ColorU8 c = extract_pixel(curr_row.data(), px);
                    pixmap.set_pixel(dst_x, dst_y, c.premultiply());
                }

                std::swap(curr_row, prior_row);
            }
        }
    } else {
        return ImageResult<Pixmap>::fail(ImageError::UnsupportedFeature);
    }

    return ImageResult<Pixmap>::success(std::move(pixmap));
}

// ============================================================================
// PNG Encoder Implementation
// ============================================================================

namespace {

void write_chunk(std::vector<uint8_t>& out, const char* type, std::span<const uint8_t> data) {
    uint8_t len_bytes[4];
    write_u32_be(len_bytes, static_cast<uint32_t>(data.size()));
    out.insert(out.end(), len_bytes, len_bytes + 4);

    size_t type_start = out.size();
    out.insert(out.end(), type, type + 4);
    out.insert(out.end(), data.begin(), data.end());

    uint32_t chunk_crc = crc32(std::span<const uint8_t>(out.data() + type_start, 4 + data.size()));
    uint8_t crc_bytes[4];
    write_u32_be(crc_bytes, chunk_crc);
    out.insert(out.end(), crc_bytes, crc_bytes + 4);
}

} // namespace

ImageResult<std::vector<uint8_t>> encode_png(
    PixmapRef pixmap,
    int compression_level
) {
    uint32_t w = pixmap.width();
    uint32_t h = pixmap.height();
    if (w == 0 || h == 0) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    std::vector<uint8_t> png_bytes;
    png_bytes.reserve(w * h * 4 + 1024);

    // 1. Signature
    png_bytes.insert(png_bytes.end(), PNG_SIGNATURE, PNG_SIGNATURE + 8);

    // 2. IHDR Chunk
    uint8_t ihdr[13];
    write_u32_be(ihdr, w);
    write_u32_be(ihdr + 4, h);
    ihdr[8] = 8;  // bit depth: 8
    ihdr[9] = 6;  // color type: RGBA
    ihdr[10] = 0; // compression: deflate
    ihdr[11] = 0; // filter: adaptive
    ihdr[12] = 0; // interlace: none
    write_chunk(png_bytes, "IHDR", std::span<const uint8_t>(ihdr, 13));

    // 3. Prepare Raw Scanlines with Adaptive Filtering
    size_t row_bytes = static_cast<size_t>(w) * 4;
    std::vector<uint8_t> uncompressed_stream;
    uncompressed_stream.reserve((row_bytes + 1) * h);

    std::vector<uint8_t> raw_row(row_bytes);
    std::vector<uint8_t> prior_row(row_bytes, 0);

    std::vector<uint8_t> sub_filtered(row_bytes);
    std::vector<uint8_t> up_filtered(row_bytes);
    std::vector<uint8_t> paeth_filtered(row_bytes);

    for (uint32_t y = 0; y < h; ++y) {
        const auto* px = pixmap.row(y);
        for (uint32_t x = 0; x < w; ++x) {
            ColorU8 c = px[x].demultiply();
            raw_row[x * 4 + 0] = c.red();
            raw_row[x * 4 + 1] = c.green();
            raw_row[x * 4 + 2] = c.blue();
            raw_row[x * 4 + 3] = c.alpha();
        }

        if (compression_level <= 3) {
            // Fast mode: use Sub filter directly
            uncompressed_stream.push_back(1);
            for (size_t x = 0; x < row_bytes; ++x) {
                uint8_t val = raw_row[x];
                uint8_t a = (x >= 4) ? raw_row[x - 4] : 0;
                uncompressed_stream.push_back(static_cast<uint8_t>(val - a));
            }
            continue;
        }

        // Evaluate Filter Types (None, Sub, Up, Paeth) using Sum of Absolute Differences (SAD)
        uint64_t sad_none = 0;
        uint64_t sad_sub = 0;
        uint64_t sad_up = 0;
        uint64_t sad_paeth = 0;

        for (size_t x = 0; x < row_bytes; ++x) {
            uint8_t val = raw_row[x];
            uint8_t a = (x >= 4) ? raw_row[x - 4] : 0;
            uint8_t b = (y > 0) ? prior_row[x] : 0;
            uint8_t c = (y > 0 && x >= 4) ? prior_row[x - 4] : 0;

            // None (0)
            sad_none += val;

            // Sub (1)
            uint8_t f_sub = static_cast<uint8_t>(val - a);
            sub_filtered[x] = f_sub;
            sad_sub += (f_sub > 128 ? 256 - f_sub : f_sub);

            // Up (2)
            uint8_t f_up = static_cast<uint8_t>(val - b);
            up_filtered[x] = f_up;
            sad_up += (f_up > 128 ? 256 - f_up : f_up);

            // Paeth (4)
            uint8_t f_paeth = static_cast<uint8_t>(val - paeth_predictor(a, b, c));
            paeth_filtered[x] = f_paeth;
            sad_paeth += (f_paeth > 128 ? 256 - f_paeth : f_paeth);
        }

        // Choose optimal filter
        uint8_t best_filter = 0;
        const uint8_t* best_data = raw_row.data();
        uint64_t min_sad = sad_none;

        if (sad_sub < min_sad) {
            min_sad = sad_sub;
            best_filter = 1;
            best_data = sub_filtered.data();
        }
        if (sad_up < min_sad) {
            min_sad = sad_up;
            best_filter = 2;
            best_data = up_filtered.data();
        }
        if (sad_paeth < min_sad) {
            best_filter = 4;
            best_data = paeth_filtered.data();
        }

        uncompressed_stream.push_back(best_filter);
        uncompressed_stream.insert(uncompressed_stream.end(), best_data, best_data + row_bytes);

        std::memcpy(prior_row.data(), raw_row.data(), row_bytes);
    }

    // 4. Compress Stream with Deflate/Zlib
    auto compressed = zlib_compress(uncompressed_stream, compression_level);
    if (!compressed) {
        return ImageResult<std::vector<uint8_t>>::fail(compressed.error);
    }

    // 5. Write IDAT Chunk
    write_chunk(png_bytes, "IDAT", *compressed);

    // 6. Write IEND Chunk
    write_chunk(png_bytes, "IEND", {});

    return ImageResult<std::vector<uint8_t>>::success(std::move(png_bytes));
}

ImageResult<Pixmap> decode_png_file(const std::string& file_path) {
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

    return decode_png(buffer);
}

bool save_png_file(
    PixmapRef pixmap,
    const std::string& file_path,
    int compression_level
) {
    auto encoded = encode_png(pixmap, compression_level);
    if (!encoded) return false;

    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(encoded->data()), encoded->size());
    return file.good();
}

} // namespace nisaba::image

#include "orion/orionimg/webp.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <array>
#include <vector>
#include <memory>
#include <cmath>

namespace orionimg {

namespace {

// ============================================================================
// 1. RIFF & WebP Chunk Headers
// ============================================================================

constexpr uint32_t FOURCC(char a, char b, char c, char d) noexcept {
    return static_cast<uint32_t>(static_cast<uint8_t>(a)) |
          (static_cast<uint32_t>(static_cast<uint8_t>(b)) << 8) |
          (static_cast<uint32_t>(static_cast<uint8_t>(c)) << 16) |
          (static_cast<uint32_t>(static_cast<uint8_t>(d)) << 24);
}

constexpr uint32_t CHUNK_RIFF = FOURCC('R', 'I', 'F', 'F');
constexpr uint32_t CHUNK_WEBP = FOURCC('W', 'E', 'B', 'P');
constexpr uint32_t CHUNK_VP8L = FOURCC('V', 'P', '8', 'L');
constexpr uint32_t CHUNK_VP8  = FOURCC('V', 'P', '8', ' ');
constexpr uint32_t CHUNK_VP8X = FOURCC('V', 'P', '8', 'X');
constexpr uint32_t CHUNK_ALPH = FOURCC('A', 'L', 'P', 'H');
constexpr uint32_t CHUNK_ANIM = FOURCC('A', 'N', 'I', 'M');
constexpr uint32_t CHUNK_ANMF = FOURCC('A', 'N', 'M', 'F');
constexpr uint32_t CHUNK_ICCP = FOURCC('I', 'C', 'C', 'P');
constexpr uint32_t CHUNK_EXIF = FOURCC('E', 'X', 'I', 'F');
constexpr uint32_t CHUNK_XMP  = FOURCC('X', 'M', 'P', ' ');

inline uint32_t read_u32_le(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
          (static_cast<uint32_t>(p[1]) << 8) |
          (static_cast<uint32_t>(p[2]) << 16) |
          (static_cast<uint32_t>(p[3]) << 24);
}

inline uint32_t read_u24_le(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
          (static_cast<uint32_t>(p[1]) << 8) |
          (static_cast<uint32_t>(p[2]) << 16);
}

inline uint16_t read_u16_le(const uint8_t* p) noexcept {
    return static_cast<uint16_t>(p[0]) |
          (static_cast<uint16_t>(p[1]) << 8);
}

inline void write_u32_le(uint8_t* p, uint32_t v) noexcept {
    p[0] = static_cast<uint8_t>(v & 0xff);
    p[1] = static_cast<uint8_t>((v >> 8) & 0xff);
    p[2] = static_cast<uint8_t>((v >> 16) & 0xff);
    p[3] = static_cast<uint8_t>((v >> 24) & 0xff);
}

static uint16_t bit_reverse_16(uint16_t v, int len) noexcept {
    uint16_t r = 0;
    for (int i = 0; i < len; ++i) {
        r = static_cast<uint16_t>((r << 1) | ((v >> i) & 1));
    }
    return r;
}

struct WebPChunk {
    uint32_t fourcc{0};
    std::span<const uint8_t> payload;
};

struct WebPContainer {
    uint32_t canvas_width{0};
    uint32_t canvas_height{0};
    bool has_icc{false};
    bool has_alpha{false};
    bool has_exif{false};
    bool has_xmp{false};
    bool has_animation{false};
    uint8_t exif_orientation{1};

    std::optional<WebPChunk> vp8l_chunk;
    std::optional<WebPChunk> vp8_chunk;
    std::optional<WebPChunk> alph_chunk;
    std::optional<WebPChunk> anim_chunk;
    ColorU8 background_color{0, 0, 0, 0};
    uint32_t loop_count{0};
    std::vector<WebPChunk> anmf_chunks;
};

ImageResult<WebPContainer> parse_webp_container(std::span<const uint8_t> data) {
    if (data.size() < 12) {
        return ImageResult<WebPContainer>::fail(ImageError::InvalidSignature);
    }

    if (read_u32_le(data.data()) != CHUNK_RIFF ||
        read_u32_le(data.data() + 8) != CHUNK_WEBP) {
        return ImageResult<WebPContainer>::fail(ImageError::InvalidSignature);
    }

    uint32_t riff_size = read_u32_le(data.data() + 4);
    size_t max_len = std::min(data.size(), static_cast<size_t>(riff_size) + 8);

    WebPContainer container;
    size_t offset = 12;

    while (offset + 8 <= max_len) {
        uint32_t chunk_fourcc = read_u32_le(data.data() + offset);
        uint32_t chunk_len = read_u32_le(data.data() + offset + 4);
        offset += 8;

        size_t avail = (offset <= max_len) ? (max_len - offset) : 0;
        size_t payload_len = std::min(static_cast<size_t>(chunk_len), avail);
        std::span<const uint8_t> payload(data.data() + offset, payload_len);

        WebPChunk chunk{chunk_fourcc, payload};

        if (chunk_fourcc == CHUNK_VP8X) {
            if (payload.size() < 10) {
                return ImageResult<WebPContainer>::fail(ImageError::CorruptedData);
            }
            uint8_t flags = payload[0];
            container.has_icc       = (flags & 0x20) != 0;
            container.has_alpha     = (flags & 0x10) != 0;
            container.has_exif      = (flags & 0x08) != 0;
            container.has_xmp       = (flags & 0x04) != 0;
            container.has_animation = (flags & 0x02) != 0;

            container.canvas_width  = 1 + read_u24_le(payload.data() + 4);
            container.canvas_height = 1 + read_u24_le(payload.data() + 7);
        } else if (chunk_fourcc == CHUNK_VP8L) {
            if (!container.vp8l_chunk.has_value()) {
                container.vp8l_chunk = chunk;
            }
        } else if (chunk_fourcc == CHUNK_VP8) {
            if (!container.vp8_chunk.has_value()) {
                container.vp8_chunk = chunk;
            }
        } else if (chunk_fourcc == CHUNK_ALPH) {
            if (!container.alph_chunk.has_value()) {
                container.alph_chunk = chunk;
            }
        } else if (chunk_fourcc == CHUNK_ANIM) {
            container.anim_chunk = chunk;
            if (payload.size() >= 6) {
                uint8_t b = payload[0];
                uint8_t g = payload[1];
                uint8_t r = payload[2];
                uint8_t a = payload[3];
                container.background_color = ColorU8{r, g, b, a};
                container.loop_count = read_u16_le(payload.data() + 4);
            }
        } else if (chunk_fourcc == CHUNK_ANMF) {
            container.anmf_chunks.push_back(chunk);
        } else if (chunk_fourcc == CHUNK_EXIF) {
            if (payload.size() >= 6 &&
                payload[0] == 'E' && payload[1] == 'x' &&
                payload[2] == 'i' && payload[3] == 'f' &&
                payload[4] == 0   && payload[5] == 0) {
                container.exif_orientation = parse_exif_orientation(payload.subspan(6));
            } else {
                container.exif_orientation = parse_exif_orientation(payload);
            }
        }

        size_t padded_len = static_cast<size_t>(chunk_len) + (chunk_len & 1);
        if (offset + padded_len < offset) break;
        offset += padded_len;
    }

    return ImageResult<WebPContainer>::success(std::move(container));
}

Image apply_webp_exif_orientation(const Image& src, uint8_t orientation) {
    if (orientation <= 1 || orientation > 8) return src;
    uint32_t w = src.width();
    uint32_t h = src.height();
    bool swap_dims = (orientation >= 5 && orientation <= 8);
    uint32_t dst_w = swap_dims ? h : w;
    uint32_t dst_h = swap_dims ? w : h;

    auto maybe_dst = Image::allocate(dst_w, dst_h, src.format());
    if (!maybe_dst.has_value()) return src;
    Image dst = std::move(*maybe_dst);

    const PremultipliedColorU8* sp = src.pixels();
    PremultipliedColorU8* dp = dst.pixels_mut();

    for (uint32_t y = 0; y < h; ++y) {
        for (uint32_t x = 0; x < w; ++x) {
            uint32_t dx = x, dy = y;
            switch (orientation) {
                case 2: dx = w - 1 - x; dy = y; break;
                case 3: dx = w - 1 - x; dy = h - 1 - y; break;
                case 4: dx = x; dy = h - 1 - y; break;
                case 5: dx = y; dy = x; break;
                case 6: dx = h - 1 - y; dy = x; break;
                case 7: dx = h - 1 - y; dy = w - 1 - x; break;
                case 8: dx = y; dy = w - 1 - x; break;
                default: break;
            }
            dp[dy * dst_w + dx] = sp[y * w + x];
        }
    }
    return dst;
}

// ============================================================================
// 2. VP8L (Lossless WebP) Bitstream & Canonical Huffman Engine (RFC 9649)
// ============================================================================

class Vp8lBitReader {
public:
    explicit Vp8lBitReader(std::span<const uint8_t> data) noexcept
        : data_(data.data()), size_(data.size()) {}

    uint32_t read_bits(int n) noexcept {
        if (n == 0) return 0;
        fill_buffer();
        uint32_t val = static_cast<uint32_t>(bit_buf_ & ((1ULL << n) - 1));
        bit_buf_ >>= n;
        bit_count_ -= n;
        return val;
    }

    uint32_t peek_bits(int n) noexcept {
        fill_buffer();
        return static_cast<uint32_t>(bit_buf_ & ((1ULL << n) - 1));
    }

    void consume_bits(int n) noexcept {
        bit_buf_ >>= n;
        bit_count_ -= n;
    }

private:
    void fill_buffer() noexcept {
        while (bit_count_ <= 56 && pos_ < size_) {
            bit_buf_ |= static_cast<uint64_t>(data_[pos_++]) << bit_count_;
            bit_count_ += 8;
        }
    }

    const uint8_t* data_{nullptr};
    size_t size_{0};
    size_t pos_{0};
    uint64_t bit_buf_{0};
    int bit_count_{0};
};

struct Vp8lHuffmanTree {
    static constexpr int MAX_BITS = 15;
    static constexpr int FAST_BITS = 9;

    struct FastEntry {
        uint16_t symbol{0};
        uint8_t bits{0};
    };

    std::array<FastEntry, 1 << FAST_BITS> fast_table{};
    std::vector<uint16_t> slow_symbols;
    std::vector<uint8_t> slow_lengths;
    std::vector<uint16_t> slow_codes_rev;
    bool is_single_symbol{false};
    uint16_t single_symbol{0};

    bool build_single(uint16_t sym) {
        is_single_symbol = true;
        single_symbol = sym;
        return true;
    }

    bool build_two(uint16_t sym0, uint16_t sym1) {
        is_single_symbol = false;
        for (uint32_t i = 0; i < (1u << FAST_BITS); ++i) {
            fast_table[i].symbol = (i & 1) ? sym1 : sym0;
            fast_table[i].bits = 1;
        }
        return true;
    }

    bool build_from_lengths(std::span<const uint8_t> code_lengths) {
        is_single_symbol = false;
        std::array<int, MAX_BITS + 1> bl_count{};
        int valid_symbols = 0;
        uint16_t last_valid_sym = 0;

        for (size_t i = 0; i < code_lengths.size(); ++i) {
            uint8_t len = code_lengths[i];
            if (len > MAX_BITS) return false;
            if (len > 0) {
                bl_count[len]++;
                valid_symbols++;
                last_valid_sym = static_cast<uint16_t>(i);
            }
        }

        if (valid_symbols == 0) return false;
        if (valid_symbols == 1) {
            return build_single(last_valid_sym);
        }

        std::array<uint16_t, MAX_BITS + 1> next_code{};
        uint16_t code = 0;
        for (int bits = 1; bits <= MAX_BITS; ++bits) {
            code = static_cast<uint16_t>((code + bl_count[bits - 1]) << 1);
            next_code[bits] = code;
        }

        for (auto& e : fast_table) {
            e = FastEntry{0, 0};
        }
        slow_symbols.clear();
        slow_lengths.clear();
        slow_codes_rev.clear();

        for (size_t i = 0; i < code_lengths.size(); ++i) {
            uint8_t len = code_lengths[i];
            if (len == 0) continue;

            uint16_t c = next_code[len]++;
            uint16_t rev = bit_reverse_16(c, len);

            if (len <= FAST_BITS) {
                uint32_t step = 1u << len;
                for (uint32_t idx = rev; idx < (1u << FAST_BITS); idx += step) {
                    fast_table[idx] = FastEntry{static_cast<uint16_t>(i), len};
                }
            } else {
                slow_symbols.push_back(static_cast<uint16_t>(i));
                slow_lengths.push_back(len);
                slow_codes_rev.push_back(rev);
            }
        }

        return true;
    }

    uint16_t decode(Vp8lBitReader& br) const noexcept {
        if (is_single_symbol) {
            return single_symbol;
        }

        uint32_t peek = br.peek_bits(MAX_BITS);
        const auto& fast = fast_table[peek & ((1u << FAST_BITS) - 1)];
        if (fast.bits > 0) {
            br.consume_bits(fast.bits);
            return fast.symbol;
        }

        for (size_t i = 0; i < slow_symbols.size(); ++i) {
            uint8_t len = slow_lengths[i];
            if ((peek & ((1u << len) - 1)) == slow_codes_rev[i]) {
                br.consume_bits(len);
                return slow_symbols[i];
            }
        }

        br.consume_bits(1);
        return 0;
    }
};

constexpr std::array<uint8_t, 19> CODE_LENGTH_ORDER = {
    17, 18, 0, 1, 2, 3, 4, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
};

bool read_huffman_code(Vp8lBitReader& br, int alphabet_size, Vp8lHuffmanTree& tree) {
    bool is_simple = br.read_bits(1) != 0;
    if (is_simple) {
        int num_symbols = static_cast<int>(br.read_bits(1)) + 1;
        int is_first_8bits = static_cast<int>(br.read_bits(1));
        uint16_t sym0 = static_cast<uint16_t>(br.read_bits(is_first_8bits ? 8 : 1));
        if (num_symbols == 1) {
            return tree.build_single(sym0);
        } else {
            uint16_t sym1 = static_cast<uint16_t>(br.read_bits(8));
            return tree.build_two(sym0, sym1);
        }
    }

    int num_code_lengths = static_cast<int>(br.read_bits(4)) + 4;
    if (num_code_lengths > 19) return false;

    std::array<uint8_t, 19> code_length_code_lengths{};
    for (int i = 0; i < num_code_lengths; ++i) {
        code_length_code_lengths[CODE_LENGTH_ORDER[i]] = static_cast<uint8_t>(br.read_bits(3));
    }

    Vp8lHuffmanTree code_len_tree;
    if (!code_len_tree.build_from_lengths(code_length_code_lengths)) {
        return false;
    }

    int max_symbol = alphabet_size;
    if (br.read_bits(1)) {
        int length_nbits = 2 + 2 * static_cast<int>(br.read_bits(3));
        max_symbol = 2 + static_cast<int>(br.read_bits(length_nbits));
        if (max_symbol > alphabet_size) {
            max_symbol = alphabet_size;
        }
    }

    std::vector<uint8_t> code_lengths(alphabet_size, 0);
    uint8_t prev_code_len = 8;
    int symbol = 0;

    while (symbol < alphabet_size) {
        if (max_symbol-- == 0) break;
        uint16_t code_len = code_len_tree.decode(br);
        if (code_len < 16) {
            code_lengths[symbol++] = static_cast<uint8_t>(code_len);
            if (code_len != 0) {
                prev_code_len = static_cast<uint8_t>(code_len);
            }
        } else {
            int repeat = 0;
            uint8_t fill_val = 0;
            if (code_len == 16) {
                repeat = 3 + static_cast<int>(br.read_bits(2));
                fill_val = prev_code_len;
            } else if (code_len == 17) {
                repeat = 3 + static_cast<int>(br.read_bits(3));
            } else {
                repeat = 11 + static_cast<int>(br.read_bits(7));
            }
            while (repeat-- > 0 && symbol < alphabet_size) {
                code_lengths[symbol++] = fill_val;
            }
        }
    }

    return tree.build_from_lengths(code_lengths);
}

struct HuffmanGroup {
    std::array<Vp8lHuffmanTree, 5> trees;
    uint16_t decode_symbol(Vp8lBitReader& br, int tree_idx) const noexcept {
        return trees[tree_idx].decode(br);
    }
};

enum Vp8lTransformType : uint8_t {
    PREDICTOR_TRANSFORM      = 0,
    COLOR_TRANSFORM          = 1,
    SUBTRACT_GREEN           = 2,
    COLOR_INDEXING_TRANSFORM = 3
};

struct Vp8lTransform {
    Vp8lTransformType type;
    int size_bits{0};
    uint32_t width{0};
    uint32_t height{0};
    uint32_t orig_width{0};
    int num_colors{0};
    std::vector<uint32_t> data;
};

inline uint8_t clamp_u8(int v) noexcept {
    return static_cast<uint8_t>(std::clamp(v, 0, 255));
}

inline uint32_t add_pixels(uint32_t a, uint32_t b) noexcept {
    uint8_t a_out = static_cast<uint8_t>((a >> 24) + (b >> 24));
    uint8_t r_out = static_cast<uint8_t>((a >> 16) + (b >> 16));
    uint8_t g_out = static_cast<uint8_t>((a >> 8)  + (b >> 8));
    uint8_t b_out = static_cast<uint8_t>(a + b);
    return (static_cast<uint32_t>(a_out) << 24) |
           (static_cast<uint32_t>(r_out) << 16) |
           (static_cast<uint32_t>(g_out) << 8)  |
           static_cast<uint32_t>(b_out);
}

inline uint32_t avg2(uint32_t a, uint32_t b) noexcept {
    return (((a ^ b) & 0xfefefefeU) >> 1) + (a & b);
}

inline uint32_t select_pred(uint32_t l, uint32_t t, uint32_t tl) noexcept {
    auto sub_abs = [](uint32_t x, uint32_t y, int shift) {
        return std::abs(static_cast<int>((x >> shift) & 0xff) - static_cast<int>((y >> shift) & 0xff));
    };
    int pa = sub_abs(t, tl, 24) + sub_abs(t, tl, 16) + sub_abs(t, tl, 8) + sub_abs(t, tl, 0);
    int pb = sub_abs(l, tl, 24) + sub_abs(l, tl, 16) + sub_abs(l, tl, 8) + sub_abs(l, tl, 0);
    return (pa < pb) ? l : t;
}

inline uint32_t clamp_add_sub_full(uint32_t a, uint32_t b, uint32_t c) noexcept {
    auto op = [](uint32_t x, uint32_t y, uint32_t z, int s) -> uint32_t {
        int v = static_cast<int>((x >> s) & 0xff) + static_cast<int>((y >> s) & 0xff) - static_cast<int>((z >> s) & 0xff);
        return static_cast<uint32_t>(clamp_u8(v)) << s;
    };
    return op(a, b, c, 24) | op(a, b, c, 16) | op(a, b, c, 8) | op(a, b, c, 0);
}

inline uint32_t clamp_add_sub_half(uint32_t a, uint32_t b) noexcept {
    auto op = [](uint32_t x, uint32_t y, int s) -> uint32_t {
        int vx = static_cast<int>((x >> s) & 0xff);
        int vy = static_cast<int>((y >> s) & 0xff);
        return static_cast<uint32_t>(clamp_u8(vx + (vx - vy) / 2)) << s;
    };
    return op(a, b, 24) | op(a, b, 16) | op(a, b, 8) | op(a, b, 0);
}

uint32_t predict_pixel(int mode, uint32_t l, uint32_t t, uint32_t tl, uint32_t tr) noexcept {
    switch (mode) {
        case 0:  return 0xff000000U;
        case 1:  return l;
        case 2:  return t;
        case 3:  return tr;
        case 4:  return tl;
        case 5:  return avg2(avg2(l, tr), t);
        case 6:  return avg2(l, tl);
        case 7:  return avg2(l, t);
        case 8:  return avg2(t, tl);
        case 9:  return avg2(t, tr);
        case 10: return avg2(avg2(l, tl), avg2(t, tr));
        case 11: return select_pred(l, t, tl);
        case 12: return clamp_add_sub_full(l, t, tl);
        case 13: return clamp_add_sub_half(avg2(l, t), tl);
        default: return 0xff000000U;
    }
}

inline int decode_prefix_value(Vp8lBitReader& br, int prefix_code) noexcept {
    if (prefix_code < 4) {
        return prefix_code + 1;
    }
    int extra_bits = (prefix_code - 2) >> 1;
    int offset = ((2 + (prefix_code & 1)) << extra_bits) + 1;
    return offset + static_cast<int>(br.read_bits(extra_bits));
}

constexpr std::array<std::pair<int8_t, int8_t>, 120> kPlaneCodeToDistance = {{
    { 0, 1}, { 1, 0}, { 1, 1}, {-1, 1}, { 0, 2}, { 2, 0}, { 1, 2}, {-1, 2},
    { 2, 1}, {-2, 1}, { 2, 2}, {-2, 2}, { 0, 3}, { 3, 0}, { 1, 3}, {-1, 3},
    { 3, 1}, {-3, 1}, { 2, 3}, {-2, 3}, { 3, 2}, {-3, 2}, { 0, 4}, { 4, 0},
    { 1, 4}, {-1, 4}, { 4, 1}, {-4, 1}, { 3, 3}, {-3, 3}, { 2, 4}, {-2, 4},
    { 4, 2}, {-4, 2}, { 0, 5}, { 3, 4}, {-3, 4}, { 4, 3}, {-4, 3}, { 5, 0},
    { 1, 5}, {-1, 5}, { 5, 1}, {-5, 1}, { 2, 5}, {-2, 5}, { 5, 2}, {-5, 2},
    { 4, 4}, {-4, 4}, { 3, 5}, {-3, 5}, { 5, 3}, {-5, 3}, { 0, 6}, { 6, 0},
    { 1, 6}, {-1, 6}, { 6, 1}, {-6, 1}, { 2, 6}, {-2, 6}, { 6, 2}, {-6, 2},
    { 4, 5}, {-4, 5}, { 5, 4}, {-5, 4}, { 3, 6}, {-3, 6}, { 6, 3}, {-6, 3},
    { 0, 7}, { 7, 0}, { 1, 7}, {-1, 7}, { 5, 5}, {-5, 5}, { 7, 1}, {-7, 1},
    { 4, 6}, {-4, 6}, { 6, 4}, {-6, 4}, { 2, 7}, {-2, 7}, { 7, 2}, {-7, 2},
    { 3, 7}, {-3, 7}, { 7, 3}, {-7, 3}, { 5, 6}, {-5, 6}, { 6, 5}, {-6, 5},
    { 8, 0}, { 4, 7}, {-4, 7}, { 7, 4}, {-7, 4}, { 8, 1}, { 8, 2}, { 6, 6},
    {-6, 6}, { 8, 3}, { 5, 7}, {-5, 7}, { 7, 5}, {-7, 5}, { 8, 4}, { 6, 7},
    {-6, 7}, { 7, 6}, {-7, 6}, { 8, 5}, { 7, 7}, {-7, 7}, { 8, 6}, { 8, 7}
}};

inline int plane_code_to_distance(int xsize, int plane_code) noexcept {
    if (plane_code > 120) {
        return plane_code - 120;
    }
    auto [dx, dy] = kPlaneCodeToDistance[plane_code - 1];
    int dist = static_cast<int>(dx) + static_cast<int>(dy) * xsize;
    return (dist < 1) ? 1 : dist;
}

inline int color_transform_delta(int8_t t, int8_t c) noexcept {
    return (static_cast<int>(t) * static_cast<int>(c)) >> 5;
}

bool decode_vp8l_stream(
    Vp8lBitReader& br,
    uint32_t width,
    uint32_t height,
    bool is_main_image,
    std::vector<uint32_t>& out_pixels
) {
    uint32_t current_width = width;
    std::vector<Vp8lTransform> transforms;

    if (is_main_image) {
        while (br.read_bits(1)) {
            Vp8lTransform t;
            t.type = static_cast<Vp8lTransformType>(br.read_bits(2));
            t.orig_width = current_width;

            if (t.type == PREDICTOR_TRANSFORM || t.type == COLOR_TRANSFORM) {
                t.size_bits = static_cast<int>(br.read_bits(3)) + 2;
                t.width = (current_width + (1u << t.size_bits) - 1) >> t.size_bits;
                t.height = (height + (1u << t.size_bits) - 1) >> t.size_bits;
                if (!decode_vp8l_stream(br, t.width, t.height, false, t.data)) {
                    return false;
                }
            } else if (t.type == SUBTRACT_GREEN) {
                // No extra bitstream data
            } else if (t.type == COLOR_INDEXING_TRANSFORM) {
                t.num_colors = static_cast<int>(br.read_bits(8)) + 1;
                t.width = static_cast<uint32_t>(t.num_colors);
                t.height = 1;
                if (!decode_vp8l_stream(br, t.width, 1, false, t.data)) {
                    return false;
                }
                for (size_t i = 1; i < t.data.size(); ++i) {
                    t.data[i] = add_pixels(t.data[i], t.data[i - 1]);
                }

                int width_bits = 0;
                if (t.num_colors <= 2) width_bits = 3;
                else if (t.num_colors <= 4) width_bits = 2;
                else if (t.num_colors <= 16) width_bits = 1;

                t.size_bits = width_bits;
                current_width = (current_width + (1u << width_bits) - 1) >> width_bits;
            }
            transforms.push_back(std::move(t));
        }
    }

    int color_cache_bits = 0;
    std::vector<uint32_t> color_cache;
    if (br.read_bits(1)) {
        color_cache_bits = static_cast<int>(br.read_bits(4));
        if (color_cache_bits < 1 || color_cache_bits > 11) {
            return false;
        }
        color_cache.assign(1u << color_cache_bits, 0);
    }

    int num_huff_groups = 1;
    int huffman_bits = 0;
    uint32_t huffman_xsize = 0;
    std::vector<uint32_t> entropy_image;

    if (is_main_image && br.read_bits(1)) {
        huffman_bits = static_cast<int>(br.read_bits(3)) + 2;
        huffman_xsize = (current_width + (1u << huffman_bits) - 1) >> huffman_bits;
        uint32_t huffman_ysize = (height + (1u << huffman_bits) - 1) >> huffman_bits;
        if (!decode_vp8l_stream(br, huffman_xsize, huffman_ysize, false, entropy_image)) {
            return false;
        }
        for (uint32_t px : entropy_image) {
            int group = static_cast<int>((px >> 8) & 0xffff);
            if (group + 1 > num_huff_groups) {
                num_huff_groups = group + 1;
            }
        }
    }

    int green_alphabet_size = 256 + 24 + (color_cache_bits > 0 ? (1 << color_cache_bits) : 0);
    std::vector<HuffmanGroup> huff_groups(num_huff_groups);
    for (int g = 0; g < num_huff_groups; ++g) {
        if (!read_huffman_code(br, green_alphabet_size, huff_groups[g].trees[0]) ||
            !read_huffman_code(br, 256, huff_groups[g].trees[1]) ||
            !read_huffman_code(br, 256, huff_groups[g].trees[2]) ||
            !read_huffman_code(br, 256, huff_groups[g].trees[3]) ||
            !read_huffman_code(br, 40,  huff_groups[g].trees[4])) {
            return false;
        }
    }

    size_t total_pixels = static_cast<size_t>(current_width) * height;
    out_pixels.assign(total_pixels, 0);
    size_t pixel_idx = 0;

    while (pixel_idx < total_pixels) {
        int group_idx = 0;
        if (!entropy_image.empty()) {
            uint32_t x = static_cast<uint32_t>(pixel_idx % current_width);
            uint32_t y = static_cast<uint32_t>(pixel_idx / current_width);
            uint32_t meta_idx = (y >> huffman_bits) * huffman_xsize + (x >> huffman_bits);
            if (meta_idx < entropy_image.size()) {
                group_idx = static_cast<int>((entropy_image[meta_idx] >> 8) & 0xffff);
                if (group_idx >= num_huff_groups) group_idx = 0;
            }
        }
        const auto& hg = huff_groups[group_idx];

        uint16_t green_sym = hg.decode_symbol(br, 0);
        if (green_sym < 256) {
            uint16_t red   = hg.decode_symbol(br, 1);
            uint16_t blue  = hg.decode_symbol(br, 2);
            uint16_t alpha = hg.decode_symbol(br, 3);

            uint32_t argb = (static_cast<uint32_t>(alpha) << 24) |
                            (static_cast<uint32_t>(red)   << 16) |
                            (static_cast<uint32_t>(green_sym) << 8) |
                             static_cast<uint32_t>(blue);
            out_pixels[pixel_idx++] = argb;

            if (color_cache_bits > 0) {
                uint32_t key = (0x1e35a7bdU * argb) >> (32 - color_cache_bits);
                color_cache[key] = argb;
            }
        } else if (green_sym < 256 + 24) {
            int len_code = static_cast<int>(green_sym) - 256;
            int length = decode_prefix_value(br, len_code);
            int dist_code = static_cast<int>(hg.decode_symbol(br, 4));
            int dist_symbol = decode_prefix_value(br, dist_code);
            int distance = plane_code_to_distance(static_cast<int>(current_width), dist_symbol);

            while (length-- > 0 && pixel_idx < total_pixels) {
                size_t src_idx = (pixel_idx >= static_cast<size_t>(distance)) ? (pixel_idx - static_cast<size_t>(distance)) : 0;
                uint32_t argb = out_pixels[src_idx];
                out_pixels[pixel_idx++] = argb;
                if (color_cache_bits > 0) {
                    uint32_t key = (0x1e35a7bdU * argb) >> (32 - color_cache_bits);
                    color_cache[key] = argb;
                }
            }
        } else {
            int cache_idx = static_cast<int>(green_sym) - (256 + 24);
            if (color_cache_bits > 0 && cache_idx >= 0 && cache_idx < (1 << color_cache_bits)) {
                uint32_t argb = color_cache[cache_idx];
                out_pixels[pixel_idx++] = argb;
                uint32_t key = (0x1e35a7bdU * argb) >> (32 - color_cache_bits);
                color_cache[key] = argb;
            } else {
                out_pixels[pixel_idx++] = 0xff000000U;
            }
        }
    }

    for (auto it = transforms.rbegin(); it != transforms.rend(); ++it) {
        if (it->type == SUBTRACT_GREEN) {
            for (size_t i = 0; i < out_pixels.size(); ++i) {
                uint32_t argb = out_pixels[i];
                uint8_t g = static_cast<uint8_t>((argb >> 8) & 0xff);
                uint8_t r = static_cast<uint8_t>(((argb >> 16) + g) & 0xff);
                uint8_t b = static_cast<uint8_t>((argb + g) & 0xff);
                out_pixels[i] = (argb & 0xff00ff00U) | (static_cast<uint32_t>(r) << 16) | static_cast<uint32_t>(b);
            }
        } else if (it->type == COLOR_TRANSFORM) {
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < current_width; ++x) {
                    uint32_t c_elem = it->data[(y >> it->size_bits) * it->width + (x >> it->size_bits)];
                    int8_t green_to_red  = static_cast<int8_t>(c_elem & 0xff);
                    int8_t green_to_blue = static_cast<int8_t>((c_elem >> 8) & 0xff);
                    int8_t red_to_blue   = static_cast<int8_t>((c_elem >> 16) & 0xff);

                    uint32_t argb = out_pixels[y * current_width + x];
                    uint8_t g = static_cast<uint8_t>((argb >> 8) & 0xff);
                    uint8_t r = static_cast<uint8_t>((argb >> 16) & 0xff);
                    uint8_t b = static_cast<uint8_t>(argb & 0xff);

                    r = static_cast<uint8_t>((r + color_transform_delta(green_to_red, static_cast<int8_t>(g))) & 0xff);
                    b = static_cast<uint8_t>((b + color_transform_delta(green_to_blue, static_cast<int8_t>(g))) & 0xff);
                    b = static_cast<uint8_t>((b + color_transform_delta(red_to_blue, static_cast<int8_t>(r))) & 0xff);

                    out_pixels[y * current_width + x] = (argb & 0xff00ff00U) |
                                                        (static_cast<uint32_t>(r) << 16) |
                                                         static_cast<uint32_t>(b);
                }
            }
        } else if (it->type == PREDICTOR_TRANSFORM) {
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < current_width; ++x) {
                    size_t idx = static_cast<size_t>(y) * current_width + x;
                    uint32_t pred = 0;
                    if (x == 0 && y == 0) {
                        pred = 0xff000000U;
                    } else if (y == 0) {
                        pred = out_pixels[idx - 1];
                    } else if (x == 0) {
                        pred = out_pixels[idx - current_width];
                    } else {
                        uint32_t t_x = x >> it->size_bits;
                        uint32_t t_y = y >> it->size_bits;
                        int mode = static_cast<int>((it->data[t_y * it->width + t_x] >> 8) & 0x0f);
                        uint32_t l  = out_pixels[idx - 1];
                        uint32_t t  = out_pixels[idx - current_width];
                        uint32_t tl = out_pixels[idx - current_width - 1];
                        uint32_t tr = out_pixels[idx - current_width + 1];
                        pred = predict_pixel(mode, l, t, tl, tr);
                    }
                    out_pixels[idx] = add_pixels(out_pixels[idx], pred);
                }
            }
        } else if (it->type == COLOR_INDEXING_TRANSFORM) {
            uint32_t full_width = it->orig_width;
            int width_bits = it->size_bits;
            std::vector<uint32_t> unpacked(static_cast<size_t>(full_width) * height, 0xff000000U);

            if (width_bits == 0) {
                for (size_t i = 0; i < out_pixels.size(); ++i) {
                    uint8_t pal_idx = static_cast<uint8_t>((out_pixels[i] >> 8) & 0xff);
                    unpacked[i] = (pal_idx < it->data.size()) ? it->data[pal_idx] : 0U;
                }
            } else {
                int bits_per_pixel = 8 >> width_bits;
                int pixels_per_byte = 1 << width_bits;
                uint8_t bit_mask = static_cast<uint8_t>((1 << bits_per_pixel) - 1);

                for (uint32_t y = 0; y < height; ++y) {
                    for (uint32_t x = 0; x < current_width; ++x) {
                        uint8_t packed = static_cast<uint8_t>((out_pixels[y * current_width + x] >> 8) & 0xff);
                        for (int sub = 0; sub < pixels_per_byte; ++sub) {
                            uint32_t dst_x = (x << width_bits) + static_cast<uint32_t>(sub);
                            if (dst_x >= full_width) break;
                            uint8_t pal_idx = (packed >> (sub * bits_per_pixel)) & bit_mask;
                            unpacked[y * full_width + dst_x] = (pal_idx < it->data.size()) ? it->data[pal_idx] : 0U;
                        }
                    }
                }
            }
            out_pixels = std::move(unpacked);
            current_width = full_width;
        }
    }

    return true;
}

ImageResult<Image> decode_vp8l_internal(std::span<const uint8_t> data, DecodeOptions options = {}) {
    if (data.size() < 5 || data[0] != 0x2F) {
        return ImageResult<Image>::fail(ImageError::InvalidSignature);
    }

    uint32_t width  = 1 + (static_cast<uint32_t>(data[1]) | ((static_cast<uint32_t>(data[2]) & 0x3F) << 8));
    uint32_t height = 1 + ((static_cast<uint32_t>(data[2]) >> 6) |
                           (static_cast<uint32_t>(data[3]) << 2) |
                          ((static_cast<uint32_t>(data[4]) & 0x0F) << 10));
    bool has_alpha  = (data[4] & 0x10) != 0;
    uint8_t version = (data[4] >> 5) & 0x07;

    if (version != 0) {
        return ImageResult<Image>::fail(ImageError::UnsupportedFormat);
    }

    auto maybe_pixmap = Image::allocate(width, height);
    if (!maybe_pixmap.has_value()) {
        return ImageResult<Image>::fail(ImageError::OutOfMemory);
    }
    Image pixmap = std::move(*maybe_pixmap);

    Vp8lBitReader br(data.subspan(5));
    std::vector<uint32_t> pixels;
    if (!decode_vp8l_stream(br, width, height, true, pixels)) {
        return ImageResult<Image>::fail(ImageError::CorruptedData);
    }

    PremultipliedColorU8* dst = pixmap.pixels_mut();
    for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
        uint32_t argb = pixels[i];
        uint8_t a = has_alpha ? static_cast<uint8_t>((argb >> 24) & 0xff) : 255;
        uint8_t r = static_cast<uint8_t>((argb >> 16) & 0xff);
        uint8_t g = static_cast<uint8_t>((argb >> 8)  & 0xff);
        uint8_t b = static_cast<uint8_t>(argb & 0xff);
        if (options.premultiply_alpha) {
            dst[i] = ColorU8{r, g, b, a}.premultiply();
        } else {
            dst[i] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
        }
    }

    return ImageResult<Image>::success(std::move(pixmap));
}

// ============================================================================
// 3. Lossy WebP (VP8 Keyframe RFC 6386 + ALPH Chunk) Sovereign Decoder
// ============================================================================

class Vp8BoolDecoder {
public:
    Vp8BoolDecoder() = default;
    explicit Vp8BoolDecoder(std::span<const uint8_t> data) noexcept {
        init(data);
    }

    void init(std::span<const uint8_t> data) noexcept {
        data_ = data.data();
        size_ = data.size();
        pos_ = 0;
        range_ = 255;
        value_ = 0;
        bit_count_ = 0;
        uint32_t b0 = (pos_ < size_) ? data_[pos_++] : 0;
        uint32_t b1 = (pos_ < size_) ? data_[pos_++] : 0;
        value_ = (b0 << 8) | b1;
    }

    int get_bit(uint8_t prob) noexcept {
        uint32_t split = 1 + (((range_ - 1) * prob) >> 8);
        uint32_t bigsplit = split << 8;
        int bit = 0;

        if (value_ >= bigsplit) {
            range_ -= split;
            value_ -= bigsplit;
            bit = 1;
        } else {
            range_ = split;
        }

        while (range_ < 128) {
            value_ <<= 1;
            range_ <<= 1;
            if (++bit_count_ == 8) {
                bit_count_ = 0;
                load_byte();
            }
        }
        return bit;
    }

    uint32_t get_uint(int num_bits) noexcept {
        uint32_t v = 0;
        for (int i = num_bits - 1; i >= 0; --i) {
            v |= static_cast<uint32_t>(get_bit(128)) << i;
        }
        return v;
    }

    int32_t get_sint(int num_bits) noexcept {
        int32_t v = static_cast<int32_t>(get_uint(num_bits));
        return get_bit(128) ? -v : v;
    }

    int get_tree(const int8_t* tree, const uint8_t* probs, int start_node = 0) noexcept {
        int i = start_node;
        while ((i = tree[i + get_bit(probs[i >> 1])]) > 0) {}
        return -i;
    }

private:
    void load_byte() noexcept {
        uint8_t b = (pos_ < size_) ? data_[pos_++] : 0;
        value_ |= static_cast<uint32_t>(b);
    }

    const uint8_t* data_{nullptr};
    size_t size_{0};
    size_t pos_{0};
    uint32_t range_{255};
    uint32_t value_{0};
    int bit_count_{0};
};

// VP8 Quantization Lookups (RFC 6386 Section 14.1)
constexpr int16_t kVp8DcQLookup[128] = {
    4, 5, 6, 7, 8, 9, 10, 10, 11, 12, 13, 14, 15, 16, 17, 17,
    18, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 27, 28,
    29, 30, 31, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42, 43,
    44, 45, 46, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58,
    59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
    75, 76, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
    91, 93, 95, 96, 98, 100, 101, 102, 104, 106, 108, 110, 112, 114, 116, 118,
    122, 124, 126, 128, 130, 132, 134, 136, 138, 140, 143, 145, 148, 151, 154, 157
};

constexpr int16_t kVp8AcQLookup[128] = {
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
    36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76,
    78, 80, 82, 84, 86, 88, 90, 92, 94, 96, 98, 100, 102, 104, 106, 108,
    110, 112, 114, 116, 119, 122, 125, 128, 131, 134, 137, 140, 143, 146, 149, 152,
    155, 158, 161, 164, 167, 170, 173, 177, 181, 185, 189, 193, 197, 201, 205, 209,
    213, 217, 221, 225, 229, 234, 239, 245, 249, 254, 259, 264, 269, 274, 279, 284
};

// Intra prediction mode enums
enum Vp8MbMode : int {
    DC_PRED = 0,
    V_PRED  = 1,
    H_PRED  = 2,
    TM_PRED = 3,
    B_PRED  = 4
};

enum Vp8BMode : int {
    B_DC_PRED = 0,
    B_TM_PRED = 1,
    B_VE_PRED = 2,
    B_HE_PRED = 3,
    B_LD_PRED = 4,
    B_RD_PRED = 5,
    B_VR_PRED = 6,
    B_VL_PRED = 7,
    B_HD_PRED = 8,
    B_HU_PRED = 9
};

constexpr int8_t kVp8SegmentTree[6] = { 2, 4, -0, -1, -2, -3 };
constexpr int8_t kVp8KfYModeTree[8] = { -B_PRED, 2, 4, 6, -DC_PRED, -V_PRED, -H_PRED, -TM_PRED };
constexpr uint8_t kVp8KfYModeProbs[4] = { 145, 156, 163, 128 };
constexpr int8_t kVp8KfUvModeTree[6] = { -DC_PRED, 2, -V_PRED, 4, -H_PRED, -TM_PRED };
constexpr uint8_t kVp8KfUvModeProbs[3] = { 142, 114, 183 };
constexpr int8_t kVp8BModeTree[18] = {
    -B_DC_PRED, 2, -B_TM_PRED, 4, -B_VE_PRED, 6, 8, 12,
    -B_HE_PRED, 10, -B_RD_PRED, -B_VR_PRED, -B_LD_PRED, 14,
    -B_VL_PRED, 16, -B_HD_PRED, -B_HU_PRED
};

constexpr uint8_t kVp8KfBModeProbs[10][10][9] = {
    {
    { 231, 120, 48, 89, 115, 113, 120, 152, 112 },
    { 152, 179, 64, 126, 170, 118, 46, 70, 95 },
    { 175, 69, 143, 80, 85, 82, 72, 155, 103 },
    { 56, 58, 10, 171, 218, 189, 17, 13, 152 },
    { 144, 71, 10, 38, 171, 213, 144, 34, 26 },
    { 114, 26, 17, 163, 44, 195, 21, 10, 173 },
    { 121, 24, 80, 195, 26, 62, 44, 64, 85 },
    { 170, 46, 55, 19, 136, 160, 33, 206, 71 },
    { 63, 20, 8, 114, 114, 208, 12, 9, 226 },
    { 81, 40, 11, 96, 182, 84, 29, 16, 36 }
},
    {
    { 134, 183, 89, 137, 98, 101, 106, 165, 148 },
    { 72, 187, 100, 130, 157, 111, 32, 75, 80 },
    { 66, 102, 167, 99, 74, 62, 40, 234, 128 },
    { 41, 53, 9, 178, 241, 141, 26, 8, 107 },
    { 104, 79, 12, 27, 217, 255, 87, 17, 7 },
    { 74, 43, 26, 146, 73, 166, 49, 23, 157 },
    { 65, 38, 105, 160, 51, 52, 31, 115, 128 },
    { 87, 68, 71, 44, 114, 51, 15, 186, 23 },
    { 47, 41, 14, 110, 182, 183, 21, 17, 194 },
    { 66, 45, 25, 102, 197, 189, 23, 18, 22 }
},
    {
    { 88, 88, 147, 150, 42, 46, 45, 196, 205 },
    { 43, 97, 183, 117, 85, 38, 35, 179, 61 },
    { 39, 53, 200, 87, 26, 21, 43, 232, 171 },
    { 56, 34, 51, 104, 114, 102, 29, 93, 77 },
    { 107, 54, 32, 26, 51, 1, 81, 43, 31 },
    { 39, 28, 85, 171, 58, 165, 90, 98, 64 },
    { 34, 22, 116, 206, 23, 34, 43, 166, 73 },
    { 68, 25, 106, 22, 64, 171, 36, 225, 114 },
    { 34, 19, 21, 102, 132, 188, 16, 76, 124 },
    { 62, 18, 78, 95, 85, 57, 50, 48, 51 }
},
    {
    { 193, 101, 35, 159, 215, 111, 89, 46, 111 },
    { 60, 148, 31, 172, 219, 228, 21, 18, 111 },
    { 112, 113, 77, 85, 179, 255, 38, 120, 114 },
    { 40, 42, 1, 196, 245, 209, 10, 25, 109 },
    { 100, 80, 8, 43, 154, 1, 51, 26, 71 },
    { 88, 43, 29, 140, 166, 213, 37, 43, 154 },
    { 61, 63, 30, 155, 67, 45, 68, 1, 209 },
    { 142, 78, 78, 16, 255, 128, 34, 197, 171 },
    { 41, 40, 5, 102, 211, 183, 4, 1, 221 },
    { 51, 50, 17, 168, 209, 192, 23, 25, 82 }
},
    {
    { 125, 98, 42, 88, 104, 85, 117, 175, 82 },
    { 95, 84, 53, 89, 128, 100, 113, 101, 45 },
    { 75, 79, 123, 47, 51, 128, 81, 171, 1 },
    { 57, 17, 5, 71, 102, 57, 53, 41, 49 },
    { 115, 21, 2, 10, 102, 255, 166, 23, 6 },
    { 38, 33, 13, 121, 57, 73, 26, 1, 85 },
    { 41, 10, 67, 138, 77, 110, 90, 47, 114 },
    { 101, 29, 16, 10, 85, 128, 101, 196, 26 },
    { 57, 18, 10, 102, 102, 213, 34, 20, 43 },
    { 117, 20, 15, 36, 163, 128, 68, 1, 26 }
},
    {
    { 138, 31, 36, 171, 27, 166, 38, 44, 229 },
    { 67, 87, 58, 169, 82, 115, 26, 59, 179 },
    { 63, 59, 90, 180, 59, 166, 93, 73, 154 },
    { 40, 40, 21, 116, 143, 209, 34, 39, 175 },
    { 57, 46, 22, 24, 128, 1, 54, 17, 37 },
    { 47, 15, 16, 183, 34, 223, 49, 45, 183 },
    { 46, 17, 33, 183, 6, 98, 15, 32, 183 },
    { 65, 32, 73, 115, 28, 128, 23, 128, 205 },
    { 40, 3, 9, 115, 51, 192, 18, 6, 223 },
    { 87, 37, 9, 115, 59, 77, 64, 21, 47 }
},
    {
    { 104, 55, 44, 218, 9, 54, 53, 130, 226 },
    { 64, 90, 70, 205, 40, 41, 23, 26, 57 },
    { 54, 57, 112, 184, 5, 41, 38, 166, 213 },
    { 30, 34, 26, 133, 152, 116, 10, 32, 134 },
    { 75, 32, 12, 51, 192, 255, 160, 43, 51 },
    { 39, 19, 53, 221, 26, 114, 32, 73, 255 },
    { 31, 9, 65, 234, 2, 15, 1, 118, 73 },
    { 88, 31, 35, 67, 102, 85, 55, 186, 85 },
    { 56, 21, 23, 111, 59, 205, 45, 37, 192 },
    { 55, 38, 70, 124, 73, 102, 1, 34, 98 }
},
    {
    { 102, 61, 71, 37, 34, 53, 31, 243, 192 },
    { 69, 60, 71, 38, 73, 119, 28, 222, 37 },
    { 68, 45, 128, 34, 1, 47, 11, 245, 171 },
    { 62, 17, 19, 70, 146, 85, 55, 62, 70 },
    { 75, 15, 9, 9, 64, 255, 184, 119, 16 },
    { 37, 43, 37, 154, 100, 163, 85, 160, 1 },
    { 63, 9, 92, 136, 28, 64, 32, 201, 85 },
    { 86, 6, 28, 5, 64, 255, 25, 248, 1 },
    { 56, 8, 17, 132, 137, 255, 55, 116, 128 },
    { 58, 15, 20, 82, 135, 57, 26, 121, 40 }
},
    {
    { 164, 50, 31, 137, 154, 133, 25, 35, 218 },
    { 51, 103, 44, 131, 131, 123, 31, 6, 158 },
    { 86, 40, 64, 135, 148, 224, 45, 183, 128 },
    { 22, 26, 17, 131, 240, 154, 14, 1, 209 },
    { 83, 12, 13, 54, 192, 255, 68, 47, 28 },
    { 45, 16, 21, 91, 64, 222, 7, 1, 197 },
    { 56, 21, 39, 155, 60, 138, 23, 102, 213 },
    { 85, 26, 85, 85, 128, 128, 32, 146, 171 },
    { 18, 11, 7, 63, 144, 171, 4, 4, 246 },
    { 35, 27, 10, 146, 174, 171, 12, 26, 128 }
},
    {
    { 190, 80, 35, 99, 180, 80, 126, 54, 45 },
    { 85, 126, 47, 87, 176, 51, 41, 20, 32 },
    { 101, 75, 128, 139, 118, 146, 116, 128, 85 },
    { 56, 41, 15, 176, 236, 85, 37, 9, 62 },
    { 146, 36, 19, 30, 171, 255, 97, 27, 20 },
    { 71, 30, 17, 119, 118, 255, 17, 18, 138 },
    { 101, 38, 60, 138, 55, 70, 43, 26, 142 },
    { 138, 45, 61, 62, 219, 1, 81, 188, 64 },
    { 32, 41, 20, 117, 151, 142, 20, 21, 163 },
    { 112, 19, 12, 61, 195, 128, 48, 4, 24 }
}
};

// Coefficient Token Tree (RFC 6386 Section 13.2)
enum Vp8CoeffToken : int {
    DCT_EOB = 0,
    DCT_0   = 1,
    DCT_1   = 2,
    DCT_2   = 3,
    DCT_3   = 4,
    DCT_4   = 5,
    DCT_CAT1 = 6,
    DCT_CAT2 = 7,
    DCT_CAT3 = 8,
    DCT_CAT4 = 9,
    DCT_CAT5 = 10,
    DCT_CAT6 = 11
};

constexpr int8_t kVp8CoeffTree[22] = {
    -DCT_EOB, 2,
    -DCT_0, 4,
    -DCT_1, 6,
    8, 12,
    -DCT_2, 10,
    -DCT_3, -DCT_4,
    14, 16,
    -DCT_CAT1, -DCT_CAT2,
    18, 20,
    -DCT_CAT3, -DCT_CAT4,
    -DCT_CAT5, -DCT_CAT6
};

constexpr uint8_t kVp8Cat1Probs[] = { 159 };
constexpr uint8_t kVp8Cat2Probs[] = { 165, 145 };
constexpr uint8_t kVp8Cat3Probs[] = { 173, 148, 140 };
constexpr uint8_t kVp8Cat4Probs[] = { 176, 155, 140, 135 };
constexpr uint8_t kVp8Cat5Probs[] = { 180, 157, 141, 134, 130 };
constexpr uint8_t kVp8Cat6Probs[] = { 254, 254, 243, 230, 196, 177, 153, 140, 133, 130, 129 };

constexpr uint8_t kVp8CoeffBands[16] = {
    0, 1, 2, 3, 6, 4, 5, 6, 6, 6, 6, 6, 6, 6, 6, 7
};

constexpr uint8_t kVp8Zigzag[16] = {
    0,  1,  4,  8,
    5,  2,  3,  6,
    9, 12, 13, 10,
    7, 11, 14, 15
};

constexpr uint8_t kVp8DefaultCoeffProbs[4][8][3][11] = {
    {
    {
    { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }
},
    {
    { 253, 136, 254, 255, 228, 219, 128, 128, 128, 128, 128 },
    { 189, 129, 242, 255, 227, 213, 255, 219, 128, 128, 128 },
    { 106, 126, 227, 252, 214, 209, 255, 255, 128, 128, 128 }
},
    {
    { 1, 98, 248, 255, 236, 226, 255, 255, 128, 128, 128 },
    { 181, 133, 238, 254, 221, 234, 255, 154, 128, 128, 128 },
    { 78, 134, 202, 247, 198, 180, 255, 219, 128, 128, 128 }
},
    {
    { 1, 185, 249, 255, 243, 255, 128, 128, 128, 128, 128 },
    { 184, 150, 247, 255, 236, 224, 128, 128, 128, 128, 128 },
    { 77, 110, 216, 255, 236, 230, 128, 128, 128, 128, 128 }
},
    {
    { 1, 101, 251, 255, 241, 255, 128, 128, 128, 128, 128 },
    { 170, 139, 241, 252, 236, 209, 255, 255, 128, 128, 128 },
    { 37, 116, 196, 243, 228, 255, 255, 255, 128, 128, 128 }
},
    {
    { 1, 204, 254, 255, 245, 255, 128, 128, 128, 128, 128 },
    { 207, 160, 250, 255, 238, 128, 128, 128, 128, 128, 128 },
    { 102, 103, 231, 255, 211, 171, 128, 128, 128, 128, 128 }
},
    {
    { 1, 152, 252, 255, 240, 255, 128, 128, 128, 128, 128 },
    { 177, 135, 243, 255, 234, 225, 128, 128, 128, 128, 128 },
    { 80, 129, 211, 255, 194, 224, 128, 128, 128, 128, 128 }
},
    {
    { 1, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 246, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 255, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }
}
},
    {
    {
    { 198, 35, 237, 223, 193, 187, 162, 160, 145, 155, 62 },
    { 131, 45, 198, 221, 172, 176, 220, 157, 252, 221, 1 },
    { 68, 47, 146, 208, 149, 167, 221, 162, 255, 223, 128 }
},
    {
    { 1, 149, 241, 255, 221, 224, 255, 255, 128, 128, 128 },
    { 184, 141, 234, 253, 222, 220, 255, 199, 128, 128, 128 },
    { 81, 99, 181, 242, 176, 190, 249, 202, 255, 255, 128 }
},
    {
    { 1, 129, 232, 253, 214, 197, 242, 196, 255, 255, 128 },
    { 99, 121, 210, 250, 201, 198, 255, 202, 128, 128, 128 },
    { 23, 91, 163, 242, 170, 187, 247, 210, 255, 255, 128 }
},
    {
    { 1, 200, 246, 255, 234, 255, 128, 128, 128, 128, 128 },
    { 109, 178, 241, 255, 231, 245, 255, 255, 128, 128, 128 },
    { 44, 130, 201, 253, 205, 192, 255, 255, 128, 128, 128 }
},
    {
    { 1, 132, 239, 251, 219, 209, 255, 165, 128, 128, 128 },
    { 94, 136, 225, 251, 218, 190, 255, 255, 128, 128, 128 },
    { 22, 100, 174, 245, 186, 161, 255, 199, 128, 128, 128 }
},
    {
    { 1, 182, 249, 255, 232, 235, 128, 128, 128, 128, 128 },
    { 124, 143, 241, 255, 227, 234, 128, 128, 128, 128, 128 },
    { 35, 77, 181, 251, 193, 211, 255, 205, 128, 128, 128 }
},
    {
    { 1, 157, 247, 255, 236, 231, 255, 255, 128, 128, 128 },
    { 121, 141, 235, 255, 225, 227, 255, 255, 128, 128, 128 },
    { 45, 99, 188, 251, 195, 217, 255, 224, 128, 128, 128 }
},
    {
    { 1, 1, 251, 255, 213, 255, 128, 128, 128, 128, 128 },
    { 203, 1, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
    { 137, 1, 177, 255, 224, 255, 128, 128, 128, 128, 128 }
}
},
    {
    {
    { 253, 9, 248, 251, 207, 208, 255, 192, 128, 128, 128 },
    { 175, 13, 224, 243, 193, 185, 249, 198, 255, 255, 128 },
    { 73, 17, 171, 221, 161, 179, 236, 167, 255, 234, 128 }
},
    {
    { 1, 95, 247, 253, 212, 183, 255, 255, 128, 128, 128 },
    { 239, 90, 244, 250, 211, 209, 255, 255, 128, 128, 128 },
    { 155, 77, 195, 248, 188, 195, 255, 255, 128, 128, 128 }
},
    {
    { 1, 24, 239, 251, 218, 219, 255, 205, 128, 128, 128 },
    { 201, 51, 219, 255, 196, 186, 128, 128, 128, 128, 128 },
    { 69, 46, 190, 239, 201, 218, 255, 228, 128, 128, 128 }
},
    {
    { 1, 191, 251, 255, 255, 128, 128, 128, 128, 128, 128 },
    { 223, 165, 249, 255, 213, 255, 128, 128, 128, 128, 128 },
    { 141, 124, 248, 255, 255, 128, 128, 128, 128, 128, 128 }
},
    {
    { 1, 16, 248, 255, 255, 128, 128, 128, 128, 128, 128 },
    { 190, 36, 230, 255, 236, 255, 128, 128, 128, 128, 128 },
    { 149, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
},
    {
    { 1, 226, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 247, 192, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 240, 128, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
},
    {
    { 1, 134, 252, 255, 255, 128, 128, 128, 128, 128, 128 },
    { 213, 62, 250, 255, 255, 128, 128, 128, 128, 128, 128 },
    { 55, 93, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
},
    {
    { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 }
}
},
    {
    {
    { 202, 24, 213, 235, 186, 191, 220, 160, 240, 175, 255 },
    { 126, 38, 182, 232, 169, 184, 228, 174, 255, 187, 128 },
    { 61, 46, 138, 219, 151, 178, 240, 170, 255, 216, 128 }
},
    {
    { 1, 112, 230, 250, 199, 191, 247, 159, 255, 255, 128 },
    { 166, 109, 228, 252, 211, 215, 255, 174, 128, 128, 128 },
    { 39, 77, 162, 232, 172, 180, 245, 178, 255, 255, 128 }
},
    {
    { 1, 52, 220, 246, 198, 199, 249, 220, 255, 255, 128 },
    { 124, 74, 191, 243, 183, 193, 250, 221, 255, 255, 128 },
    { 24, 71, 130, 219, 154, 170, 243, 182, 255, 255, 128 }
},
    {
    { 1, 182, 225, 249, 219, 240, 255, 224, 128, 128, 128 },
    { 149, 150, 226, 252, 216, 205, 255, 171, 128, 128, 128 },
    { 28, 108, 170, 242, 183, 194, 254, 223, 255, 255, 128 }
},
    {
    { 1, 81, 230, 252, 204, 203, 255, 192, 128, 128, 128 },
    { 123, 102, 209, 247, 188, 196, 255, 233, 128, 128, 128 },
    { 20, 95, 153, 243, 164, 173, 255, 203, 128, 128, 128 }
},
    {
    { 1, 222, 248, 255, 216, 213, 128, 128, 128, 128, 128 },
    { 168, 175, 246, 252, 235, 205, 255, 255, 128, 128, 128 },
    { 47, 116, 215, 255, 211, 212, 255, 255, 128, 128, 128 }
},
    {
    { 1, 121, 236, 253, 212, 214, 255, 255, 128, 128, 128 },
    { 141, 84, 213, 252, 201, 202, 255, 219, 128, 128, 128 },
    { 42, 80, 160, 240, 162, 185, 255, 205, 128, 128, 128 }
},
    {
    { 1, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 244, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 },
    { 238, 1, 255, 128, 128, 128, 128, 128, 128, 128, 128 }
}
}
};

constexpr uint8_t kVp8CoeffUpdateProbs[4][8][3][11] = {
    {
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 176, 246, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 223, 241, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 249, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 244, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 234, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 246, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 239, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 251, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 251, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 254, 253, 255, 254, 255, 255, 255, 255, 255, 255 },
    { 250, 255, 254, 255, 254, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
}
},
    {
    {
    { 217, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 225, 252, 241, 253, 255, 255, 254, 255, 255, 255, 255 },
    { 234, 250, 241, 250, 253, 255, 253, 254, 255, 255, 255 }
},
    {
    { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 223, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 238, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 248, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 249, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 253, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 247, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 252, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 253, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
}
},
    {
    {
    { 186, 251, 250, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 234, 251, 244, 254, 255, 255, 255, 255, 255, 255, 255 },
    { 251, 251, 243, 253, 254, 255, 254, 255, 255, 255, 255 }
},
    {
    { 255, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 236, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 251, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
}
},
    {
    {
    { 248, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 250, 254, 252, 254, 255, 255, 255, 255, 255, 255, 255 },
    { 248, 254, 249, 253, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 246, 253, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 252, 254, 251, 254, 254, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 254, 252, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 248, 254, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 253, 255, 254, 254, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 245, 251, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 253, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 251, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 252, 253, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 252, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 249, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 254, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 253, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 250, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
},
    {
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 },
    { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
}
}
};

struct Vp8QuantFactors {
    int16_t y1_dc{0}, y1_ac{0};
    int16_t y2_dc{0}, y2_ac{0};
    int16_t uv_dc{0}, uv_ac{0};
};

// Inverse Walsh-Hadamard Transform (4x4) for Y2 block
void vp8_iwht4x4(const int16_t in[16], int16_t out[16]) noexcept {
    int32_t tmp[16];
    for (int i = 0; i < 4; ++i) {
        int32_t a1 = in[0 + i] + in[12 + i];
        int32_t b1 = in[4 + i] + in[8 + i];
        int32_t c1 = in[4 + i] - in[8 + i];
        int32_t d1 = in[0 + i] - in[12 + i];
        tmp[0 + i]  = a1 + b1;
        tmp[4 + i]  = c1 + d1;
        tmp[8 + i]  = a1 - b1;
        tmp[12 + i] = d1 - c1;
    }
    for (int i = 0; i < 4; ++i) {
        const int32_t* t = tmp + i * 4;
        int32_t dc = t[0] + 3;
        int32_t a1 = dc + t[3];
        int32_t b1 = t[1] + t[2];
        int32_t c1 = t[1] - t[2];
        int32_t d1 = dc - t[3];
        out[i * 4 + 0] = static_cast<int16_t>((a1 + b1) >> 3);
        out[i * 4 + 1] = static_cast<int16_t>((c1 + d1) >> 3);
        out[i * 4 + 2] = static_cast<int16_t>((a1 - b1) >> 3);
        out[i * 4 + 3] = static_cast<int16_t>((d1 - c1) >> 3);
    }
}

// Inverse DCT (4x4) + Add to destination plane
void vp8_idct4x4_add(const int16_t in[16], uint8_t* dst, int stride) noexcept {
    static constexpr int32_t kCospi8Sqrt2Minus1 = 20091;
    static constexpr int32_t kSinpi8Sqrt2 = 35468;

    int32_t tmp[16];
    for (int i = 0; i < 4; ++i) {
        int32_t a1 = in[0 + i] + in[8 + i];
        int32_t b1 = in[0 + i] - in[8 + i];
        int32_t t1 = (in[4 + i] * kSinpi8Sqrt2) >> 16;
        int32_t t2 = in[12 + i] + ((in[12 + i] * kCospi8Sqrt2Minus1) >> 16);
        int32_t c1 = t1 - t2;
        t1 = in[4 + i] + ((in[4 + i] * kCospi8Sqrt2Minus1) >> 16);
        t2 = (in[12 + i] * kSinpi8Sqrt2) >> 16;
        int32_t d1 = t1 + t2;

        tmp[i * 4 + 0] = a1 + d1;
        tmp[i * 4 + 3] = a1 - d1;
        tmp[i * 4 + 1] = b1 + c1;
        tmp[i * 4 + 2] = b1 - c1;
    }

    for (int i = 0; i < 4; ++i) {
        int32_t a1 = tmp[0 + i] + tmp[8 + i];
        int32_t b1 = tmp[0 + i] - tmp[8 + i];
        int32_t t1 = (tmp[4 + i] * kSinpi8Sqrt2) >> 16;
        int32_t t2 = tmp[12 + i] + ((tmp[12 + i] * kCospi8Sqrt2Minus1) >> 16);
        int32_t c1 = t1 - t2;
        t1 = tmp[4 + i] + ((tmp[4 + i] * kCospi8Sqrt2Minus1) >> 16);
        t2 = (tmp[12 + i] * kSinpi8Sqrt2) >> 16;
        int32_t d1 = t1 + t2;

        uint8_t* row = dst + i * stride;
        row[0] = clamp_u8(row[0] + ((a1 + d1 + 4) >> 3));
        row[3] = clamp_u8(row[3] + ((a1 - d1 + 4) >> 3));
        row[1] = clamp_u8(row[1] + ((b1 + c1 + 4) >> 3));
        row[2] = clamp_u8(row[2] + ((b1 - c1 + 4) >> 3));
    }
}

int decode_vp8_coeffs(
    Vp8BoolDecoder& bd,
    const uint8_t probs[8][3][11],
    int ctx,
    int16_t dc_q,
    int16_t ac_q,
    int first_coeff,
    int16_t out_block[16]
) noexcept {
    int eob_pos = first_coeff;
    int start_node = 0;

    for (int i = first_coeff; i < 16; ++i) {
        const uint8_t* p = probs[kVp8CoeffBands[i]][ctx];
        int token = bd.get_tree(kVp8CoeffTree, p, start_node);
        if (token == DCT_EOB) {
            break;
        }
        eob_pos = i + 1;
        if (token == DCT_0) {
            ctx = 0;
            start_node = 2; // Skip EOB check on next coefficient
            continue;
        }

        start_node = 0;
        int32_t v = 0;
        if (token <= DCT_4) {
            v = token - 1;
        } else {
            const uint8_t* cat_probs = nullptr;
            int cat_len = 0;
            int base = 0;
            switch (token) {
                case DCT_CAT1: cat_probs = kVp8Cat1Probs; cat_len = 1;  base = 5;  break;
                case DCT_CAT2: cat_probs = kVp8Cat2Probs; cat_len = 2;  base = 7;  break;
                case DCT_CAT3: cat_probs = kVp8Cat3Probs; cat_len = 3;  base = 11; break;
                case DCT_CAT4: cat_probs = kVp8Cat4Probs; cat_len = 4;  base = 19; break;
                case DCT_CAT5: cat_probs = kVp8Cat5Probs; cat_len = 5;  base = 35; break;
                case DCT_CAT6: cat_probs = kVp8Cat6Probs; cat_len = 11; base = 67; break;
                default: break;
            }
            int extra = 0;
            for (int b = 0; b < cat_len; ++b) {
                extra = (extra << 1) | bd.get_bit(cat_probs[b]);
            }
            v = base + extra;
        }

        ctx = (v == 1) ? 1 : 2;
        if (bd.get_bit(128)) {
            v = -v;
        }

        int16_t q = (i == 0) ? dc_q : ac_q;
        out_block[kVp8Zigzag[i]] = static_cast<int16_t>(v * q);
    }

    return (eob_pos > first_coeff) ? 1 : 0;
}

void vp8_predict_16x16(
    int mode,
    uint8_t* dst,
    int stride,
    bool has_top,
    bool has_left
) noexcept {
    if (mode == V_PRED) {
        const uint8_t* top = dst - stride;
        for (int y = 0; y < 16; ++y) {
            std::memcpy(dst + y * stride, top, 16);
        }
    } else if (mode == H_PRED) {
        for (int y = 0; y < 16; ++y) {
            uint8_t left = dst[y * stride - 1];
            std::memset(dst + y * stride, left, 16);
        }
    } else if (mode == TM_PRED) {
        const uint8_t* top = dst - stride;
        uint8_t tl = top[-1];
        for (int y = 0; y < 16; ++y) {
            uint8_t left = dst[y * stride - 1];
            int diff = static_cast<int>(left) - static_cast<int>(tl);
            for (int x = 0; x < 16; ++x) {
                dst[y * stride + x] = clamp_u8(static_cast<int>(top[x]) + diff);
            }
        }
    } else {
        // DC_PRED
        int sum = 0;
        int shift = 0;
        if (has_top) {
            for (int x = 0; x < 16; ++x) sum += dst[-stride + x];
            shift += 4;
        }
        if (has_left) {
            for (int y = 0; y < 16; ++y) sum += dst[y * stride - 1];
            shift += 4;
        }
        uint8_t dc = 128;
        if (shift == 8) {
            dc = static_cast<uint8_t>((sum + 16) >> 5);
        } else if (shift == 4) {
            dc = static_cast<uint8_t>((sum + 8) >> 4);
        }
        for (int y = 0; y < 16; ++y) {
            std::memset(dst + y * stride, dc, 16);
        }
    }
}

void vp8_predict_8x8(
    int mode,
    uint8_t* dst,
    int stride,
    bool has_top,
    bool has_left
) noexcept {
    if (mode == V_PRED) {
        const uint8_t* top = dst - stride;
        for (int y = 0; y < 8; ++y) {
            std::memcpy(dst + y * stride, top, 8);
        }
    } else if (mode == H_PRED) {
        for (int y = 0; y < 8; ++y) {
            uint8_t left = dst[y * stride - 1];
            std::memset(dst + y * stride, left, 8);
        }
    } else if (mode == TM_PRED) {
        const uint8_t* top = dst - stride;
        uint8_t tl = top[-1];
        for (int y = 0; y < 8; ++y) {
            uint8_t left = dst[y * stride - 1];
            int diff = static_cast<int>(left) - static_cast<int>(tl);
            for (int x = 0; x < 8; ++x) {
                dst[y * stride + x] = clamp_u8(static_cast<int>(top[x]) + diff);
            }
        }
    } else {
        int sum = 0;
        int shift = 0;
        if (has_top) {
            for (int x = 0; x < 8; ++x) sum += dst[-stride + x];
            shift += 3;
        }
        if (has_left) {
            for (int y = 0; y < 8; ++y) sum += dst[y * stride - 1];
            shift += 3;
        }
        uint8_t dc = 128;
        if (shift == 6) {
            dc = static_cast<uint8_t>((sum + 8) >> 4);
        } else if (shift == 3) {
            dc = static_cast<uint8_t>((sum + 4) >> 3);
        }
        for (int y = 0; y < 8; ++y) {
            std::memset(dst + y * stride, dc, 8);
        }
    }
}

inline uint8_t avg3_u8(uint8_t a, uint8_t b, uint8_t c) noexcept {
    return static_cast<uint8_t>((static_cast<int>(a) + 2 * static_cast<int>(b) + static_cast<int>(c) + 2) >> 2);
}

inline uint8_t avg2_u8(uint8_t a, uint8_t b) noexcept {
    return static_cast<uint8_t>((static_cast<int>(a) + static_cast<int>(b) + 1) >> 1);
}

void vp8_predict_4x4(
    int mode,
    uint8_t* dst,
    int stride,
    const uint8_t* top_right_override
) noexcept {
    uint8_t A[8];
    for (int i = 0; i < 4; ++i) A[i] = dst[-stride + i];
    if (top_right_override != nullptr) {
        for (int i = 0; i < 4; ++i) A[4 + i] = top_right_override[i];
    } else {
        for (int i = 0; i < 4; ++i) A[4 + i] = dst[-stride + 4 + i];
    }

    uint8_t L[4] = {
        dst[0 * stride - 1],
        dst[1 * stride - 1],
        dst[2 * stride - 1],
        dst[3 * stride - 1]
    };
    uint8_t P = dst[-stride - 1];

    switch (mode) {
        case B_DC_PRED: {
            int sum = A[0] + A[1] + A[2] + A[3] + L[0] + L[1] + L[2] + L[3];
            uint8_t dc = static_cast<uint8_t>((sum + 4) >> 3);
            for (int y = 0; y < 4; ++y) std::memset(dst + y * stride, dc, 4);
            break;
        }
        case B_TM_PRED: {
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    dst[y * stride + x] = clamp_u8(static_cast<int>(L[y]) + static_cast<int>(A[x]) - static_cast<int>(P));
                }
            }
            break;
        }
        case B_VE_PRED: {
            uint8_t v[4] = {
                avg3_u8(P, A[0], A[1]),
                avg3_u8(A[0], A[1], A[2]),
                avg3_u8(A[1], A[2], A[3]),
                avg3_u8(A[2], A[3], A[4])
            };
            for (int y = 0; y < 4; ++y) std::memcpy(dst + y * stride, v, 4);
            break;
        }
        case B_HE_PRED: {
            uint8_t h[4] = {
                avg3_u8(P, L[0], L[1]),
                avg3_u8(L[0], L[1], L[2]),
                avg3_u8(L[1], L[2], L[3]),
                avg3_u8(L[2], L[3], L[3])
            };
            for (int y = 0; y < 4; ++y) std::memset(dst + y * stride, h[y], 4);
            break;
        }
        case B_LD_PRED: {
            dst[0 * stride + 0] = avg3_u8(A[0], A[1], A[2]);
            dst[0 * stride + 1] = dst[1 * stride + 0] = avg3_u8(A[1], A[2], A[3]);
            dst[0 * stride + 2] = dst[1 * stride + 1] = dst[2 * stride + 0] = avg3_u8(A[2], A[3], A[4]);
            dst[0 * stride + 3] = dst[1 * stride + 2] = dst[2 * stride + 1] = dst[3 * stride + 0] = avg3_u8(A[3], A[4], A[5]);
            dst[1 * stride + 3] = dst[2 * stride + 2] = dst[3 * stride + 1] = avg3_u8(A[4], A[5], A[6]);
            dst[2 * stride + 3] = dst[3 * stride + 2] = avg3_u8(A[5], A[6], A[7]);
            dst[3 * stride + 3] = avg3_u8(A[6], A[7], A[7]);
            break;
        }
        case B_RD_PRED: {
            uint8_t E[9] = { L[3], L[2], L[1], L[0], P, A[0], A[1], A[2], A[3] };
            for (int y = 0; y < 4; ++y) {
                for (int x = 0; x < 4; ++x) {
                    int idx = 3 - y + x;
                    dst[y * stride + x] = avg3_u8(E[idx], E[idx + 1], E[idx + 2]);
                }
            }
            break;
        }
        case B_VR_PRED: {
            dst[0 * stride + 0] = dst[2 * stride + 1] = avg2_u8(P, A[0]);
            dst[0 * stride + 1] = dst[2 * stride + 2] = avg2_u8(A[0], A[1]);
            dst[0 * stride + 2] = dst[2 * stride + 3] = avg2_u8(A[1], A[2]);
            dst[0 * stride + 3] = avg2_u8(A[2], A[3]);

            dst[1 * stride + 0] = dst[3 * stride + 1] = avg3_u8(L[0], P, A[0]);
            dst[1 * stride + 1] = dst[3 * stride + 2] = avg3_u8(P, A[0], A[1]);
            dst[1 * stride + 2] = dst[3 * stride + 3] = avg3_u8(A[0], A[1], A[2]);
            dst[1 * stride + 3] = avg3_u8(A[1], A[2], A[3]);

            dst[2 * stride + 0] = avg3_u8(L[1], L[0], P);
            dst[3 * stride + 0] = avg3_u8(L[2], L[1], L[0]);
            break;
        }
        case B_VL_PRED: {
            dst[0 * stride + 0] = avg2_u8(A[0], A[1]);
            dst[0 * stride + 1] = dst[2 * stride + 0] = avg2_u8(A[1], A[2]);
            dst[0 * stride + 2] = dst[2 * stride + 1] = avg2_u8(A[2], A[3]);
            dst[0 * stride + 3] = dst[2 * stride + 2] = avg2_u8(A[3], A[4]);
            dst[2 * stride + 3] = avg2_u8(A[4], A[5]);

            dst[1 * stride + 0] = avg3_u8(A[0], A[1], A[2]);
            dst[1 * stride + 1] = dst[3 * stride + 0] = avg3_u8(A[1], A[2], A[3]);
            dst[1 * stride + 2] = dst[3 * stride + 1] = avg3_u8(A[2], A[3], A[4]);
            dst[1 * stride + 3] = dst[3 * stride + 2] = avg3_u8(A[3], A[4], A[5]);
            dst[3 * stride + 3] = avg3_u8(A[4], A[5], A[6]);
            break;
        }
        case B_HD_PRED: {
            dst[0 * stride + 0] = dst[1 * stride + 2] = avg2_u8(P, L[0]);
            dst[0 * stride + 1] = dst[1 * stride + 3] = avg3_u8(L[0], P, A[0]);
            dst[0 * stride + 2] = avg3_u8(P, A[0], A[1]);
            dst[0 * stride + 3] = avg3_u8(A[0], A[1], A[2]);

            dst[1 * stride + 0] = dst[2 * stride + 2] = avg2_u8(L[0], L[1]);
            dst[1 * stride + 1] = dst[2 * stride + 3] = avg3_u8(P, L[0], L[1]);

            dst[2 * stride + 0] = dst[3 * stride + 2] = avg2_u8(L[1], L[2]);
            dst[2 * stride + 1] = dst[3 * stride + 3] = avg3_u8(L[0], L[1], L[2]);

            dst[3 * stride + 0] = avg2_u8(L[2], L[3]);
            dst[3 * stride + 1] = avg3_u8(L[1], L[2], L[3]);
            break;
        }
        case B_HU_PRED: {
            dst[0 * stride + 0] = avg2_u8(L[0], L[1]);
            dst[0 * stride + 1] = avg3_u8(L[0], L[1], L[2]);
            dst[0 * stride + 2] = dst[1 * stride + 0] = avg2_u8(L[1], L[2]);
            dst[0 * stride + 3] = dst[1 * stride + 1] = avg3_u8(L[1], L[2], L[3]);
            dst[1 * stride + 2] = dst[2 * stride + 0] = avg2_u8(L[2], L[3]);
            dst[1 * stride + 3] = dst[2 * stride + 1] = avg3_u8(L[2], L[3], L[3]);
            dst[2 * stride + 2] = dst[2 * stride + 3] =
            dst[3 * stride + 0] = dst[3 * stride + 1] =
            dst[3 * stride + 2] = dst[3 * stride + 3] = L[3];
            break;
        }
        default: break;
    }
}

// VP8 In-Loop Deblocking Filter Helpers (RFC 6386 Section 15)
inline int8_t clamp_s8(int v) noexcept {
    return static_cast<int8_t>(std::clamp(v, -128, 127));
}

inline void vp8_filter_common(
    uint8_t* p1, uint8_t* p0, uint8_t* q0, uint8_t* q1,
    bool use_outer_taps, int hev
) noexcept {
    int32_t vp1 = static_cast<int32_t>(*p1) - 128;
    int32_t vp0 = static_cast<int32_t>(*p0) - 128;
    int32_t vq0 = static_cast<int32_t>(*q0) - 128;
    int32_t vq1 = static_cast<int32_t>(*q1) - 128;

    int32_t a = 3 * (vq0 - vp0);
    if (use_outer_taps || hev) {
        a += clamp_s8(vp1 - vq1);
    }
    a = clamp_s8(a);

    int32_t f1 = ((a + 4 > 127) ? 127 : a + 4) >> 3;
    int32_t f2 = ((a + 3 > 127) ? 127 : a + 3) >> 3;

    *q0 = static_cast<uint8_t>(clamp_s8(vq0 - f1) + 128);
    *p0 = static_cast<uint8_t>(clamp_s8(vp0 + f2) + 128);

    if (!use_outer_taps && !hev) {
        int32_t f3 = (f1 + 1) >> 1;
        *q1 = static_cast<uint8_t>(clamp_s8(vq1 - f3) + 128);
        *p1 = static_cast<uint8_t>(clamp_s8(vp1 + f3) + 128);
    }
}

inline void vp8_mb_filter_edge(
    uint8_t* p2, uint8_t* p1, uint8_t* p0, uint8_t* q0, uint8_t* q1, uint8_t* q2,
    int edge_limit, int interior_limit, int hev_thresh
) noexcept {
    if (std::abs(static_cast<int>(*p0) - static_cast<int>(*q0)) * 2 +
        std::abs(static_cast<int>(*p1) - static_cast<int>(*q1)) / 2 > edge_limit) {
        return;
    }
    if (std::abs(static_cast<int>(*p2) - static_cast<int>(*p1)) > interior_limit ||
        std::abs(static_cast<int>(*p1) - static_cast<int>(*p0)) > interior_limit ||
        std::abs(static_cast<int>(*q1) - static_cast<int>(*q0)) > interior_limit ||
        std::abs(static_cast<int>(*q2) - static_cast<int>(*q1)) > interior_limit) {
        return;
    }

    bool hev = std::abs(static_cast<int>(*p1) - static_cast<int>(*p0)) > hev_thresh ||
               std::abs(static_cast<int>(*q1) - static_cast<int>(*q0)) > hev_thresh;
    if (hev) {
        vp8_filter_common(p1, p0, q0, q1, true, 1);
    } else {
        int32_t vp2 = static_cast<int32_t>(*p2) - 128;
        int32_t vp1 = static_cast<int32_t>(*p1) - 128;
        int32_t vp0 = static_cast<int32_t>(*p0) - 128;
        int32_t vq0 = static_cast<int32_t>(*q0) - 128;
        int32_t vq1 = static_cast<int32_t>(*q1) - 128;
        int32_t vq2 = static_cast<int32_t>(*q2) - 128;

        int32_t w = clamp_s8(clamp_s8(vp1 - vq1) + 3 * (vq0 - vp0));
        int32_t a = (27 * w + 63) >> 7;
        *q0 = static_cast<uint8_t>(clamp_s8(vq0 - a) + 128);
        *p0 = static_cast<uint8_t>(clamp_s8(vp0 + a) + 128);

        a = (18 * w + 63) >> 7;
        *q1 = static_cast<uint8_t>(clamp_s8(vq1 - a) + 128);
        *p1 = static_cast<uint8_t>(clamp_s8(vp1 + a) + 128);

        a = (9 * w + 63) >> 7;
        *q2 = static_cast<uint8_t>(clamp_s8(vq2 - a) + 128);
        *p2 = static_cast<uint8_t>(clamp_s8(vp2 + a) + 128);
    }
}

inline void vp8_subblock_filter_edge(
    uint8_t* p1, uint8_t* p0, uint8_t* q0, uint8_t* q1,
    int edge_limit, int interior_limit, int hev_thresh
) noexcept {
    if (std::abs(static_cast<int>(*p0) - static_cast<int>(*q0)) * 2 +
        std::abs(static_cast<int>(*p1) - static_cast<int>(*q1)) / 2 > edge_limit) {
        return;
    }
    if (std::abs(static_cast<int>(*p1) - static_cast<int>(*p0)) > interior_limit ||
        std::abs(static_cast<int>(*q1) - static_cast<int>(*q0)) > interior_limit) {
        return;
    }
    int hev = (std::abs(static_cast<int>(*p1) - static_cast<int>(*p0)) > hev_thresh ||
               std::abs(static_cast<int>(*q1) - static_cast<int>(*q0)) > hev_thresh) ? 1 : 0;
    vp8_filter_common(p1, p0, q0, q1, false, hev);
}

// Decode ALPH chunk (RFC 9649 Section 2.7.2)
bool decode_webp_alph(
    std::span<const uint8_t> alph_data,
    uint32_t width,
    uint32_t height,
    std::vector<uint8_t>& out_alpha
) {
    if (alph_data.size() < 1) return false;
    uint8_t header = alph_data[0];
    uint8_t comp_method = header & 0x03;
    uint8_t filter_method = (header >> 2) & 0x03;

    size_t total = static_cast<size_t>(width) * height;
    out_alpha.assign(total, 255);

    std::span<const uint8_t> bitstream = alph_data.subspan(1);
    if (comp_method == 0) {
        if (bitstream.size() < total) return false;
        std::memcpy(out_alpha.data(), bitstream.data(), total);
    } else if (comp_method == 1) {
        Vp8lBitReader br(bitstream);
        std::vector<uint32_t> argb_pixels;
        if (!decode_vp8l_stream(br, width, height, true, argb_pixels) || argb_pixels.size() < total) {
            return false;
        }
        for (size_t i = 0; i < total; ++i) {
            out_alpha[i] = static_cast<uint8_t>((argb_pixels[i] >> 8) & 0xff); // Green channel holds alpha
        }
    } else {
        return false;
    }

    // Apply spatial unfiltering
    if (filter_method == 1) {
        // Horizontal filter
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t pred = (x == 0) ? (y == 0 ? 0 : out_alpha[(y - 1) * width]) : out_alpha[y * width + x - 1];
                out_alpha[y * width + x] = static_cast<uint8_t>(out_alpha[y * width + x] + pred);
            }
        }
    } else if (filter_method == 2) {
        // Vertical filter
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t pred = (y == 0) ? (x == 0 ? 0 : out_alpha[x - 1]) : out_alpha[(y - 1) * width + x];
                out_alpha[y * width + x] = static_cast<uint8_t>(out_alpha[y * width + x] + pred);
            }
        }
    } else if (filter_method == 3) {
        // Gradient filter
        for (uint32_t y = 0; y < height; ++y) {
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t pred = 0;
                if (x == 0 && y == 0) pred = 0;
                else if (y == 0) pred = out_alpha[x - 1];
                else if (x == 0) pred = out_alpha[(y - 1) * width];
                else {
                    int a = out_alpha[y * width + x - 1];
                    int b = out_alpha[(y - 1) * width + x];
                    int c = out_alpha[(y - 1) * width + x - 1];
                    pred = clamp_u8(a + b - c);
                }
                out_alpha[y * width + x] = static_cast<uint8_t>(out_alpha[y * width + x] + pred);
            }
        }
    }

    return true;
}

ImageResult<Image> decode_vp8_internal(
    std::span<const uint8_t> data,
    std::span<const uint8_t> alpha_data,
    DecodeOptions options = {}
) {
    if (data.size() < 10) {
        return ImageResult<Image>::fail(ImageError::InvalidSignature);
    }

    uint32_t frame_tag = read_u24_le(data.data());
    bool is_keyframe = (frame_tag & 1) == 0;
    uint32_t part0_len = frame_tag >> 5;

    if (!is_keyframe || data[3] != 0x9D || data[4] != 0x01 || data[5] != 0x2A) {
        return ImageResult<Image>::fail(ImageError::InvalidSignature);
    }

    uint32_t width  = read_u16_le(data.data() + 6) & 0x3FFF;
    uint32_t height = read_u16_le(data.data() + 8) & 0x3FFF;
    if (width == 0 || height == 0) {
        return ImageResult<Image>::fail(ImageError::InvalidDimensions);
    }

    if (10 + part0_len > data.size()) {
        return ImageResult<Image>::fail(ImageError::CorruptedData);
    }

    Vp8BoolDecoder bd0(data.subspan(10, part0_len));
    // Color space and clamping type (2 bits)
    (void)bd0.get_bit(128);
    (void)bd0.get_bit(128);

    // 1. Segmentation Header
    bool update_segmentation = bd0.get_bit(128) != 0;
    bool segment_abs_delta = false;
    int8_t segment_quant[4] = {0, 0, 0, 0};
    int8_t segment_lf[4] = {0, 0, 0, 0};
    uint8_t segment_probs[3] = {255, 255, 255};
    bool update_mb_seg_map = false;

    if (update_segmentation) {
        update_mb_seg_map = bd0.get_bit(128) != 0;
        bool update_seg_data = bd0.get_bit(128) != 0;
        if (update_seg_data) {
            segment_abs_delta = bd0.get_bit(128) != 0;
            for (int i = 0; i < 4; ++i) {
                if (bd0.get_bit(128)) segment_quant[i] = static_cast<int8_t>(bd0.get_sint(7));
            }
            for (int i = 0; i < 4; ++i) {
                if (bd0.get_bit(128)) segment_lf[i] = static_cast<int8_t>(bd0.get_sint(6));
            }
        }
        if (update_mb_seg_map) {
            for (int i = 0; i < 3; ++i) {
                if (bd0.get_bit(128)) segment_probs[i] = static_cast<uint8_t>(bd0.get_uint(8));
            }
        }
    }

    // 2. Loop Filter Header
    int filter_type = bd0.get_bit(128);
    int filter_level = static_cast<int>(bd0.get_uint(6));
    int sharpness_level = static_cast<int>(bd0.get_uint(3));
    bool lf_adj_enable = bd0.get_bit(128) != 0;
    int8_t ref_lf_deltas[4] = {0, 0, 0, 0};
    int8_t mode_lf_deltas[4] = {0, 0, 0, 0};
    if (lf_adj_enable && bd0.get_bit(128)) {
        for (int i = 0; i < 4; ++i) {
            if (bd0.get_bit(128)) ref_lf_deltas[i] = static_cast<int8_t>(bd0.get_sint(6));
        }
        for (int i = 0; i < 4; ++i) {
            if (bd0.get_bit(128)) mode_lf_deltas[i] = static_cast<int8_t>(bd0.get_sint(6));
        }
    }

    // 3. Token Partitions
    int log2_parts = static_cast<int>(bd0.get_uint(2));
    int num_parts = 1 << log2_parts;
    size_t part_table_offset = 10 + part0_len;
    size_t part_table_bytes = static_cast<size_t>(num_parts - 1) * 3;
    if (part_table_offset + part_table_bytes > data.size()) {
        return ImageResult<Image>::fail(ImageError::CorruptedData);
    }

    std::vector<Vp8BoolDecoder> token_bds(num_parts);
    size_t cur_part_offset = part_table_offset + part_table_bytes;
    for (int p = 0; p < num_parts; ++p) {
        size_t p_len = 0;
        if (p + 1 < num_parts) {
            p_len = read_u24_le(data.data() + part_table_offset + p * 3);
        } else {
            p_len = (cur_part_offset <= data.size()) ? (data.size() - cur_part_offset) : 0;
        }
        size_t avail = (cur_part_offset <= data.size()) ? (data.size() - cur_part_offset) : 0;
        p_len = std::min(p_len, avail);
        token_bds[p].init(data.subspan(cur_part_offset, p_len));
        cur_part_offset += p_len;
    }

    // 4. Quantization Indices
    int y_ac_qi = static_cast<int>(bd0.get_uint(7));
    auto read_delta_q = [&]() -> int {
        return bd0.get_bit(128) ? static_cast<int>(bd0.get_sint(4)) : 0;
    };
    int y_dc_delta  = read_delta_q();
    int y2_dc_delta = read_delta_q();
    int y2_ac_delta = read_delta_q();
    int uv_dc_delta = read_delta_q();
    int uv_ac_delta = read_delta_q();

    Vp8QuantFactors quant_factors[4];
    for (int s = 0; s < 4; ++s) {
        int base_q = y_ac_qi;
        if (update_segmentation) {
            base_q = segment_abs_delta ? segment_quant[s] : (base_q + segment_quant[s]);
        }
        base_q = std::clamp(base_q, 0, 127);

        auto q_idx = [&](int delta) { return std::clamp(base_q + delta, 0, 127); };
        quant_factors[s].y1_dc = kVp8DcQLookup[q_idx(y_dc_delta)];
        quant_factors[s].y1_ac = kVp8AcQLookup[q_idx(0)];
        quant_factors[s].y2_dc = static_cast<int16_t>(kVp8DcQLookup[q_idx(y2_dc_delta)] * 2);
        quant_factors[s].y2_ac = static_cast<int16_t>(std::max<int>(8, (kVp8AcQLookup[q_idx(y2_ac_delta)] * 155) / 100));
        quant_factors[s].uv_dc = std::min<int16_t>(132, kVp8DcQLookup[q_idx(uv_dc_delta)]);
        quant_factors[s].uv_ac = kVp8AcQLookup[q_idx(uv_ac_delta)];
    }

    // Refresh entropy probs bit (1 bit on keyframes)
    (void)bd0.get_bit(128);

    // 5. Coefficient Probability Updates
    uint8_t coeff_probs[4][8][3][11];
    std::memcpy(coeff_probs, kVp8DefaultCoeffProbs, sizeof(coeff_probs));
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 3; ++k) {
                for (int l = 0; l < 11; ++l) {
                    if (bd0.get_bit(kVp8CoeffUpdateProbs[i][j][k][l])) {
                        coeff_probs[i][j][k][l] = static_cast<uint8_t>(bd0.get_uint(8));
                    }
                }
            }
        }
    }

    // 6. Skip Coefficient Flag
    bool mb_no_skip_coeff = bd0.get_bit(128) != 0;
    uint8_t prob_skip_false = mb_no_skip_coeff ? static_cast<uint8_t>(bd0.get_uint(8)) : 0;

    // 7. Allocate padded YUV420 planes (with 1-row/1-col border for intra prediction)
    uint32_t mb_cols = (width + 15) >> 4;
    uint32_t mb_rows = (height + 15) >> 4;

    int y_stride  = static_cast<int>(mb_cols * 16 + 16);
    int uv_stride = static_cast<int>(mb_cols * 8 + 16);
    std::vector<uint8_t> y_plane(static_cast<size_t>(mb_rows * 16 + 16) * y_stride, 128);
    std::vector<uint8_t> u_plane(static_cast<size_t>(mb_rows * 8 + 16) * uv_stride, 128);
    std::vector<uint8_t> v_plane(static_cast<size_t>(mb_rows * 8 + 16) * uv_stride, 128);

    uint8_t* y_origin = y_plane.data() + 4 * y_stride + 8;
    uint8_t* u_origin = u_plane.data() + 4 * uv_stride + 8;
    uint8_t* v_origin = v_plane.data() + 4 * uv_stride + 8;

    // Initialize top border to 127 and top-left corner to 129 per RFC 6386
    std::memset(y_origin - y_stride - 1, 127, mb_cols * 16 + 5);
    std::memset(u_origin - uv_stride - 1, 127, mb_cols * 8 + 5);
    std::memset(v_origin - uv_stride - 1, 127, mb_cols * 8 + 5);

    struct MbContext {
        uint8_t nz[9]{}; // 0..3: Y, 4..5: U, 6..7: V, 8: Y2
        uint8_t b_modes[4]{B_DC_PRED, B_DC_PRED, B_DC_PRED, B_DC_PRED};
    };
    std::vector<MbContext> top_ctx(mb_cols);

    struct MbFilterInfo {
        uint8_t segment{0};
        uint8_t y_mode{DC_PRED};
        bool skip{false};
        bool has_subblock_Filter{false};
    };
    std::vector<MbFilterInfo> mb_infos(static_cast<size_t>(mb_rows) * mb_cols);

    // 8. Macroblock Decode Loop
    for (uint32_t mb_y = 0; mb_y < mb_rows; ++mb_y) {
        // Initialize left macroblock border to 129 (and top-left pixel to 129 for row 0)
        for (int r = -1; r < 16; ++r) {
            if (r == -1) continue;
            y_origin[(static_cast<int>(mb_y) * 16 + r) * y_stride - 1] = 129;
        }
        for (int r = -1; r < 8; ++r) {
            if (r == -1) continue;
            u_origin[(static_cast<int>(mb_y) * 8 + r) * uv_stride - 1] = 129;
            v_origin[(static_cast<int>(mb_y) * 8 + r) * uv_stride - 1] = 129;
        }

        MbContext left_ctx{};
        Vp8BoolDecoder& tbd = token_bds[mb_y & (num_parts - 1)];

        for (uint32_t mb_x = 0; mb_x < mb_cols; ++mb_x) {
            uint8_t segment = 0;
            if (update_segmentation && update_mb_seg_map) {
                segment = static_cast<uint8_t>(bd0.get_tree(kVp8SegmentTree, segment_probs));
            }
            const auto& qf = quant_factors[segment];

            bool skip = mb_no_skip_coeff ? (bd0.get_bit(prob_skip_false) != 0) : false;

            int y_mode = bd0.get_tree(kVp8KfYModeTree, kVp8KfYModeProbs);
            uint8_t sub_modes[16];
            if (y_mode == B_PRED) {
                for (int by = 0; by < 4; ++by) {
                    for (int bx = 0; bx < 4; ++bx) {
                        int above_mode = (by == 0) ? top_ctx[mb_x].b_modes[bx] : sub_modes[(by - 1) * 4 + bx];
                        int left_mode  = (bx == 0) ? left_ctx.b_modes[by]      : sub_modes[by * 4 + (bx - 1)];
                        int b_mode = bd0.get_tree(kVp8BModeTree, kVp8KfBModeProbs[above_mode][left_mode]);
                        sub_modes[by * 4 + bx] = static_cast<uint8_t>(b_mode);
                    }
                }
                for (int i = 0; i < 4; ++i) {
                    top_ctx[mb_x].b_modes[i] = sub_modes[12 + i];
                    left_ctx.b_modes[i]      = sub_modes[i * 4 + 3];
                }
            } else {
                uint8_t mapped = B_DC_PRED;
                if (y_mode == V_PRED) mapped = B_VE_PRED;
                else if (y_mode == H_PRED) mapped = B_HE_PRED;
                else if (y_mode == TM_PRED) mapped = B_TM_PRED;
                for (int i = 0; i < 16; ++i) sub_modes[i] = mapped;
                for (int i = 0; i < 4; ++i) {
                    top_ctx[mb_x].b_modes[i] = mapped;
                    left_ctx.b_modes[i]      = mapped;
                }
            }

            int uv_mode = bd0.get_tree(kVp8KfUvModeTree, kVp8KfUvModeProbs);

            // Decode Residual Tokens
            int16_t y2_block[16]{};
            int16_t y2_dc_out[16]{};
            int16_t y_blocks[16][16]{};
            int16_t u_blocks[4][16]{};
            int16_t v_blocks[4][16]{};
            bool any_nz = false;

            if (!skip) {
                if (y_mode != B_PRED) {
                    int ctx = top_ctx[mb_x].nz[8] + left_ctx.nz[8];
                    int nz = decode_vp8_coeffs(tbd, coeff_probs[1], ctx, qf.y2_dc, qf.y2_ac, 0, y2_block);
                    top_ctx[mb_x].nz[8] = left_ctx.nz[8] = static_cast<uint8_t>(nz);
                    vp8_iwht4x4(y2_block, y2_dc_out);
                    if (nz) any_nz = true;
                }

                int first_coeff = (y_mode == B_PRED) ? 0 : 1;
                int plane_type  = (y_mode == B_PRED) ? 3 : 0;

                for (int by = 0; by < 4; ++by) {
                    for (int bx = 0; bx < 4; ++bx) {
                        int b_idx = by * 4 + bx;
                        if (y_mode != B_PRED) {
                            y_blocks[b_idx][0] = y2_dc_out[b_idx];
                        }
                        int ctx = top_ctx[mb_x].nz[bx] + left_ctx.nz[by];
                        int nz = decode_vp8_coeffs(tbd, coeff_probs[plane_type], ctx, qf.y1_dc, qf.y1_ac, first_coeff, y_blocks[b_idx]);
                        top_ctx[mb_x].nz[bx] = left_ctx.nz[by] = static_cast<uint8_t>(nz);
                        if (nz) any_nz = true;
                    }
                }

                for (int by = 0; by < 2; ++by) {
                    for (int bx = 0; bx < 2; ++bx) {
                        int b_idx = by * 2 + bx;
                        int ctx = top_ctx[mb_x].nz[4 + bx] + left_ctx.nz[4 + by];
                        int nz = decode_vp8_coeffs(tbd, coeff_probs[2], ctx, qf.uv_dc, qf.uv_ac, 0, u_blocks[b_idx]);
                        top_ctx[mb_x].nz[4 + bx] = left_ctx.nz[4 + by] = static_cast<uint8_t>(nz);
                        if (nz) any_nz = true;
                    }
                }

                for (int by = 0; by < 2; ++by) {
                    for (int bx = 0; bx < 2; ++bx) {
                        int b_idx = by * 2 + bx;
                        int ctx = top_ctx[mb_x].nz[6 + bx] + left_ctx.nz[6 + by];
                        int nz = decode_vp8_coeffs(tbd, coeff_probs[2], ctx, qf.uv_dc, qf.uv_ac, 0, v_blocks[b_idx]);
                        top_ctx[mb_x].nz[6 + bx] = left_ctx.nz[6 + by] = static_cast<uint8_t>(nz);
                        if (nz) any_nz = true;
                    }
                }
            } else {
                for (int i = 0; i < 8; ++i) {
                    top_ctx[mb_x].nz[i] = 0;
                    left_ctx.nz[i] = 0;
                }
                if (y_mode != B_PRED) {
                    top_ctx[mb_x].nz[8] = 0;
                    left_ctx.nz[8] = 0;
                }
            }

            // Reconstruct Macroblock Pixels
            uint8_t* mb_y_dst = y_origin + (mb_y * 16) * y_stride + (mb_x * 16);
            uint8_t* mb_u_dst = u_origin + (mb_y * 8) * uv_stride + (mb_x * 8);
            uint8_t* mb_v_dst = v_origin + (mb_y * 8) * uv_stride + (mb_x * 8);

            if (y_mode != B_PRED) {
                vp8_predict_16x16(y_mode, mb_y_dst, y_stride, mb_y > 0, mb_x > 0);
                for (int by = 0; by < 4; ++by) {
                    for (int bx = 0; bx < 4; ++bx) {
                        int b_idx = by * 4 + bx;
                        vp8_idct4x4_add(y_blocks[b_idx], mb_y_dst + (by * 4) * y_stride + (bx * 4), y_stride);
                    }
                }
            } else {
                // Save macroblock top-right 4 pixels for subblocks at bx == 3 and by > 0
                uint8_t mb_top_right[4];
                if (mb_y == 0) {
                    std::memset(mb_top_right, 127, 4);
                } else if (mb_x + 1 < mb_cols) {
                    std::memcpy(mb_top_right, mb_y_dst - y_stride + 16, 4);
                } else {
                    std::memset(mb_top_right, mb_y_dst[-y_stride + 15], 4);
                }

                for (int by = 0; by < 4; ++by) {
                    for (int bx = 0; bx < 4; ++bx) {
                        int b_idx = by * 4 + bx;
                        uint8_t* sb_dst = mb_y_dst + (by * 4) * y_stride + (bx * 4);
                        const uint8_t* tr_override = (bx == 3) ? mb_top_right : nullptr;
                        vp8_predict_4x4(sub_modes[b_idx], sb_dst, y_stride, tr_override);
                        vp8_idct4x4_add(y_blocks[b_idx], sb_dst, y_stride);
                    }
                }
            }

            vp8_predict_8x8(uv_mode, mb_u_dst, uv_stride, mb_y > 0, mb_x > 0);
            vp8_predict_8x8(uv_mode, mb_v_dst, uv_stride, mb_y > 0, mb_x > 0);

            for (int by = 0; by < 2; ++by) {
                for (int bx = 0; bx < 2; ++bx) {
                    int b_idx = by * 2 + bx;
                    vp8_idct4x4_add(u_blocks[b_idx], mb_u_dst + (by * 4) * uv_stride + (bx * 4), uv_stride);
                    vp8_idct4x4_add(v_blocks[b_idx], mb_v_dst + (by * 4) * uv_stride + (bx * 4), uv_stride);
                }
            }

            auto& info = mb_infos[mb_y * mb_cols + mb_x];
            info.segment = segment;
            info.y_mode = static_cast<uint8_t>(y_mode);
            info.skip = skip;
            info.has_subblock_Filter = (y_mode == B_PRED) || (!skip && any_nz);
        }
    }

    // 9. In-Loop Deblocking Filter (RFC 6386 Section 15)
    if (filter_level > 0) {
        for (uint32_t mb_y = 0; mb_y < mb_rows; ++mb_y) {
            for (uint32_t mb_x = 0; mb_x < mb_cols; ++mb_x) {
                const auto& info = mb_infos[mb_y * mb_cols + mb_x];
                int lvl = filter_level;
                if (update_segmentation) {
                    lvl = segment_abs_delta ? segment_lf[info.segment] : (lvl + segment_lf[info.segment]);
                }
                if (lf_adj_enable) {
                    lvl += ref_lf_deltas[0]; // Intra frame reference = 0
                    if (info.y_mode == B_PRED) {
                        lvl += mode_lf_deltas[0];
                    }
                }
                lvl = std::clamp(lvl, 0, 63);
                if (lvl == 0) continue;

                int interior_limit = lvl;
                if (sharpness_level > 0) {
                    interior_limit >>= (sharpness_level > 4 ? 2 : 1);
                    if (interior_limit > 9 - sharpness_level) {
                        interior_limit = 9 - sharpness_level;
                    }
                }
                if (interior_limit < 1) interior_limit = 1;

                int hev_thresh = 0;
                if (lvl >= 40) hev_thresh = 2;
                else if (lvl >= 15) hev_thresh = 1;

                int mbedge_limit = (lvl + 2) * 2 + interior_limit;
                int sub_limit    = lvl * 2 + interior_limit;

                uint8_t* mb_y_dst = y_origin + (mb_y * 16) * y_stride + (mb_x * 16);
                uint8_t* mb_u_dst = u_origin + (mb_y * 8) * uv_stride + (mb_x * 8);
                uint8_t* mb_v_dst = v_origin + (mb_y * 8) * uv_stride + (mb_x * 8);

                // Filter Left Macroblock Edge
                if (mb_x > 0) {
                    for (int r = 0; r < 16; ++r) {
                        uint8_t* p = mb_y_dst + r * y_stride;
                        vp8_mb_filter_edge(p - 3, p - 2, p - 1, p, p + 1, p + 2, mbedge_limit, interior_limit, hev_thresh);
                    }
                    if (filter_type == 0) {
                        for (int r = 0; r < 8; ++r) {
                            uint8_t* pu = mb_u_dst + r * uv_stride;
                            uint8_t* pv = mb_v_dst + r * uv_stride;
                            vp8_mb_filter_edge(pu - 3, pu - 2, pu - 1, pu, pu + 1, pu + 2, mbedge_limit, interior_limit, hev_thresh);
                            vp8_mb_filter_edge(pv - 3, pv - 2, pv - 1, pv, pv + 1, pv + 2, mbedge_limit, interior_limit, hev_thresh);
                        }
                    }
                }

                // Filter Vertical Internal Subblock Edges
                if (info.has_subblock_Filter) {
                    for (int c = 4; c < 16; c += 4) {
                        for (int r = 0; r < 16; ++r) {
                            uint8_t* p = mb_y_dst + r * y_stride + c;
                            vp8_subblock_filter_edge(p - 2, p - 1, p, p + 1, sub_limit, interior_limit, hev_thresh);
                        }
                    }
                    if (filter_type == 0) {
                        for (int r = 0; r < 8; ++r) {
                            uint8_t* pu = mb_u_dst + r * uv_stride + 4;
                            uint8_t* pv = mb_v_dst + r * uv_stride + 4;
                            vp8_subblock_filter_edge(pu - 2, pu - 1, pu, pu + 1, sub_limit, interior_limit, hev_thresh);
                            vp8_subblock_filter_edge(pv - 2, pv - 1, pv, pv + 1, sub_limit, interior_limit, hev_thresh);
                        }
                    }
                }

                // Filter Top Macroblock Edge
                if (mb_y > 0) {
                    for (int c = 0; c < 16; ++c) {
                        uint8_t* p = mb_y_dst + c;
                        vp8_mb_filter_edge(p - 3 * y_stride, p - 2 * y_stride, p - y_stride, p, p + y_stride, p + 2 * y_stride, mbedge_limit, interior_limit, hev_thresh);
                    }
                    if (filter_type == 0) {
                        for (int c = 0; c < 8; ++c) {
                            uint8_t* pu = mb_u_dst + c;
                            uint8_t* pv = mb_v_dst + c;
                            vp8_mb_filter_edge(pu - 3 * uv_stride, pu - 2 * uv_stride, pu - uv_stride, pu, pu + uv_stride, pu + 2 * uv_stride, mbedge_limit, interior_limit, hev_thresh);
                            vp8_mb_filter_edge(pv - 3 * uv_stride, pv - 2 * uv_stride, pv - uv_stride, pv, pv + uv_stride, pv + 2 * uv_stride, mbedge_limit, interior_limit, hev_thresh);
                        }
                    }
                }

                // Filter Horizontal Internal Subblock Edges
                if (info.has_subblock_Filter) {
                    for (int r = 4; r < 16; r += 4) {
                        for (int c = 0; c < 16; ++c) {
                            uint8_t* p = mb_y_dst + r * y_stride + c;
                            vp8_subblock_filter_edge(p - 2 * y_stride, p - y_stride, p, p + y_stride, sub_limit, interior_limit, hev_thresh);
                        }
                    }
                    if (filter_type == 0) {
                        for (int c = 0; c < 8; ++c) {
                            uint8_t* pu = mb_u_dst + 4 * uv_stride + c;
                            uint8_t* pv = mb_v_dst + 4 * uv_stride + c;
                            vp8_subblock_filter_edge(pu - 2 * uv_stride, pu - uv_stride, pu, pu + uv_stride, sub_limit, interior_limit, hev_thresh);
                            vp8_subblock_filter_edge(pv - 2 * uv_stride, pv - uv_stride, pv, pv + uv_stride, sub_limit, interior_limit, hev_thresh);
                        }
                    }
                }
            }
        }
    }

    // 10. Optional ALPH Chunk Decoding
    std::vector<uint8_t> alpha_plane;
    bool has_alpha = false;
    if (!alpha_data.empty()) {
        has_alpha = decode_webp_alph(alpha_data, width, height, alpha_plane);
    }

    // 11. YUV420 -> RGBA8 Conversion with Bilinear Chroma Upsampling
    auto maybe_pixmap = Image::allocate(width, height);
    if (!maybe_pixmap.has_value()) {
        return ImageResult<Image>::fail(ImageError::OutOfMemory);
    }
    Image pixmap = std::move(*maybe_pixmap);
    PremultipliedColorU8* dst = pixmap.pixels_mut();

    int uv_w = static_cast<int>((width + 1) >> 1);
    int uv_h = static_cast<int>((height + 1) >> 1);

    for (uint32_t y = 0; y < height; ++y) {
        int cy0 = static_cast<int>(y) >> 1;
        int cy1 = (y & 1) ? std::min(cy0 + 1, uv_h - 1) : std::max(cy0 - 1, 0);

        for (uint32_t x = 0; x < width; ++x) {
            int cx0 = static_cast<int>(x) >> 1;
            int cx1 = (x & 1) ? std::min(cx0 + 1, uv_w - 1) : std::max(cx0 - 1, 0);

            int y_val = y_origin[y * y_stride + x];
            int u_val = (9 * u_origin[cy0 * uv_stride + cx0] +
                         3 * u_origin[cy0 * uv_stride + cx1] +
                         3 * u_origin[cy1 * uv_stride + cx0] +
                         1 * u_origin[cy1 * uv_stride + cx1] + 8) >> 4;
            int v_val = (9 * v_origin[cy0 * uv_stride + cx0] +
                         3 * v_origin[cy0 * uv_stride + cx1] +
                         3 * v_origin[cy1 * uv_stride + cx0] +
                         1 * v_origin[cy1 * uv_stride + cx1] + 8) >> 4;

            // ITU-R BT.601 YCbCr to RGB
            int c = y_val - 16;
            int d = u_val - 128;
            int e = v_val - 128;
            uint8_t r = clamp_u8((298 * c + 409 * e + 128) >> 8);
            uint8_t g = clamp_u8((298 * c - 100 * d - 208 * e + 128) >> 8);
            uint8_t b = clamp_u8((298 * c + 516 * d + 128) >> 8);
            uint8_t a = has_alpha ? alpha_plane[y * width + x] : 255;

            if (options.premultiply_alpha) {
                dst[y * width + x] = ColorU8{r, g, b, a}.premultiply();
            } else {
                dst[y * width + x] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
            }
        }
    }

    return ImageResult<Image>::success(std::move(pixmap));
}

} // namespace

// ============================================================================
// 4. Sovereign VP8L (Lossless) & VP8 (Lossy) Encoders
// ============================================================================

namespace {

class Vp8lBitWriter {
public:
    void write_bits(uint32_t val, int n) {
        for (int i = 0; i < n; ++i) {
            bit_buf_ |= ((static_cast<uint64_t>(val) >> i) & 1ULL) << bit_count_;
            if (++bit_count_ == 8) {
                bytes_.push_back(static_cast<uint8_t>(bit_buf_));
                bit_buf_ = 0;
                bit_count_ = 0;
            }
        }
    }

    void flush() {
        if (bit_count_ > 0) {
            bytes_.push_back(static_cast<uint8_t>(bit_buf_));
            bit_buf_ = 0;
            bit_count_ = 0;
        }
    }

    std::vector<uint8_t>& bytes() { return bytes_; }

private:
    std::vector<uint8_t> bytes_;
    uint64_t bit_buf_{0};
    int bit_count_{0};
};

struct Vp8lEncHuffTable {
    std::vector<uint8_t> lengths;
    std::vector<uint16_t> codes_rev;
    int num_active{0};
    int sym0{-1};
    int sym1{-1};
};

Vp8lEncHuffTable build_vp8l_huff_table(std::span<const uint32_t> freqs) {
    int alphabet_size = static_cast<int>(freqs.size());
    Vp8lEncHuffTable table;
    table.lengths.assign(alphabet_size, 0);
    table.codes_rev.assign(alphabet_size, 0);

    struct Node {
        uint64_t freq;
        int sym;
        int left;
        int right;
    };

    std::vector<Node> nodes;
    for (int i = 0; i < alphabet_size; ++i) {
        if (freqs[i] > 0) {
            if (table.num_active == 0) table.sym0 = i;
            else if (table.num_active == 1) table.sym1 = i;
            table.num_active++;
            nodes.push_back({freqs[i], i, -1, -1});
        }
    }

    if (table.num_active == 0) {
        table.num_active = 1;
        table.sym0 = 0;
        table.lengths[0] = 0;
        return table;
    }

    if (table.num_active == 1 && table.sym0 <= 255) {
        table.lengths[table.sym0] = 0;
        return table;
    }

    if (table.num_active == 2 && table.sym0 <= 255 && table.sym1 <= 255) {
        table.lengths[table.sym0] = 1;
        table.lengths[table.sym1] = 1;
        table.codes_rev[table.sym0] = 0;
        table.codes_rev[table.sym1] = 1;
        return table;
    }

    if (table.num_active == 1) {
        // Single symbol > 255: encode via complex tree with 1 active symbol
        table.lengths[table.sym0] = 1;
        table.codes_rev[table.sym0] = 0;
        return table;
    }

    std::vector<int> active_indices(nodes.size());
    for (size_t i = 0; i < nodes.size(); ++i) active_indices[i] = static_cast<int>(i);

    while (active_indices.size() > 1) {
        std::sort(active_indices.begin(), active_indices.end(), [&](int a, int b) {
            if (nodes[a].freq != nodes[b].freq) return nodes[a].freq > nodes[b].freq;
            return a > b;
        });
        int n1 = active_indices.back(); active_indices.pop_back();
        int n2 = active_indices.back(); active_indices.pop_back();
        int parent = static_cast<int>(nodes.size());
        nodes.push_back({nodes[n1].freq + nodes[n2].freq, -1, n1, n2});
        active_indices.push_back(parent);
    }

    auto assign_depth = [&](auto& self, int idx, int depth) -> void {
        if (nodes[idx].sym >= 0) {
            table.lengths[nodes[idx].sym] = static_cast<uint8_t>(std::clamp(depth, 1, 15));
            return;
        }
        self(self, nodes[idx].left, depth + 1);
        self(self, nodes[idx].right, depth + 1);
    };
    assign_depth(assign_depth, active_indices[0], 0);

    // Enforce Kraft equality: sum(2^(15 - len)) == 32768
    int32_t kraft = 0;
    for (int i = 0; i < alphabet_size; ++i) {
        if (table.lengths[i] > 0) {
            kraft += (1 << (15 - table.lengths[i]));
        }
    }

    while (kraft > (1 << 15)) {
        int best_sym = -1;
        for (int i = alphabet_size - 1; i >= 0; --i) {
            if (table.lengths[i] > 0 && table.lengths[i] < 15) {
                if (best_sym < 0 || table.lengths[i] > table.lengths[best_sym]) {
                    best_sym = i;
                }
            }
        }
        if (best_sym < 0) break;
        kraft -= (1 << (15 - table.lengths[best_sym]));
        table.lengths[best_sym]++;
        kraft += (1 << (15 - table.lengths[best_sym]));
    }

    while (kraft < (1 << 15)) {
        int32_t diff = (1 << 15) - kraft;
        int best_sym = -1;
        for (int i = 0; i < alphabet_size; ++i) {
            if (table.lengths[i] > 1) {
                int32_t gain = (1 << (15 - table.lengths[i]));
                if (gain <= diff) {
                    if (best_sym < 0 || table.lengths[i] > table.lengths[best_sym]) {
                        best_sym = i;
                    }
                }
            }
        }
        if (best_sym < 0) break;
        kraft += (1 << (15 - table.lengths[best_sym]));
        table.lengths[best_sym]--;
    }

    // Compute canonical codes
    std::array<int, 16> bl_count{};
    for (uint8_t len : table.lengths) {
        if (len > 0) bl_count[len]++;
    }
    std::array<uint16_t, 16> next_code{};
    uint16_t code = 0;
    for (int bits = 1; bits <= 15; ++bits) {
        code = static_cast<uint16_t>((code + bl_count[bits - 1]) << 1);
        next_code[bits] = code;
    }
    for (int i = 0; i < alphabet_size; ++i) {
        uint8_t len = table.lengths[i];
        if (len > 0) {
            uint16_t c = next_code[len]++;
            table.codes_rev[i] = bit_reverse_16(c, len);
        }
    }

    return table;
}

void write_vp8l_huff_table(Vp8lBitWriter& bw, const Vp8lEncHuffTable& table) {
    if (table.num_active == 1 && table.sym0 <= 255) {
        bw.write_bits(1, 1); // is_simple = 1
        bw.write_bits(0, 1); // 1 symbol
        bool is_8bits = table.sym0 > 1;
        bw.write_bits(is_8bits ? 1 : 0, 1);
        bw.write_bits(static_cast<uint32_t>(table.sym0), is_8bits ? 8 : 1);
        return;
    }
    if (table.num_active == 2 && table.sym0 <= 255 && table.sym1 <= 255) {
        bw.write_bits(1, 1); // is_simple = 1
        bw.write_bits(1, 1); // 2 symbols
        bool is_8bits = table.sym0 > 1;
        bw.write_bits(is_8bits ? 1 : 0, 1);
        bw.write_bits(static_cast<uint32_t>(table.sym0), is_8bits ? 8 : 1);
        bw.write_bits(static_cast<uint32_t>(table.sym1), 8);
        return;
    }

    // Complex canonical Huffman tree: encode code lengths 0..15 with fixed 4-bit code-length tree
    bw.write_bits(0, 1); // is_simple = 0
    bw.write_bits(19 - 4, 4); // num_code_lengths = 19
    for (int i = 0; i < 19; ++i) {
        uint8_t cl = (CODE_LENGTH_ORDER[i] < 16) ? 4 : 0;
        bw.write_bits(cl, 3);
    }
    bw.write_bits(0, 1); // use_max_symbol = 0 (reads full alphabet_size)
    for (uint8_t len : table.lengths) {
        bw.write_bits(bit_reverse_16(len, 4), 4);
    }
}

inline void encode_vp8l_prefix(int val, int& prefix_code, int& extra_bits, uint32_t& extra_val) noexcept {
    val -= 1;
    if (val < 4) {
        prefix_code = val;
        extra_bits = 0;
        extra_val = 0;
        return;
    }
    int eb = 0;
    int tmp = val;
    while (tmp >= 4) {
        tmp >>= 1;
        ++eb;
    }
    prefix_code = 2 + (eb << 1) + (tmp & 1);
    extra_bits = eb;
    extra_val = static_cast<uint32_t>(val - ((2 + (tmp & 1)) << eb));
}

std::vector<uint8_t> encode_vp8l_payload(ImageRef pixmap) {
    uint32_t width = pixmap.width();
    uint32_t height = pixmap.height();
    const PremultipliedColorU8* px_data = pixmap.pixels();
    size_t total_pixels = static_cast<size_t>(width) * height;

    bool has_alpha = false;
    std::vector<uint32_t> argb_pixels(total_pixels);
    for (size_t i = 0; i < total_pixels; ++i) {
        ColorU8 c = px_data[i].demultiply();
        if (c.a != 255) has_alpha = true;
        // Apply SubtractGreen transform inline: R' = (R - G) & 0xff, B' = (B - G) & 0xff
        uint8_t r_sub = static_cast<uint8_t>((c.r - c.g) & 0xff);
        uint8_t b_sub = static_cast<uint8_t>((c.b - c.g) & 0xff);
        argb_pixels[i] = (static_cast<uint32_t>(c.a) << 24) |
                         (static_cast<uint32_t>(r_sub) << 16) |
                         (static_cast<uint32_t>(c.g) << 8) |
                          static_cast<uint32_t>(b_sub);
    }

    constexpr int kCacheBits = 6;
    constexpr int kCacheSize = 1 << kCacheBits;
    std::array<uint32_t, kCacheSize> color_cache{};
    std::array<bool, kCacheSize> cache_valid{};

    struct Token {
        uint16_t green_sym{0};
        uint8_t r{0}, b{0}, a{0};
        uint8_t len_extra_bits{0};
        uint32_t len_extra_val{0};
        bool is_len{false};
    };
    std::vector<Token> tokens;
    tokens.reserve(total_pixels);

    std::vector<uint32_t> freq_g(256 + 24 + kCacheSize, 0);
    std::vector<uint32_t> freq_r(256, 0);
    std::vector<uint32_t> freq_b(256, 0);
    std::vector<uint32_t> freq_a(256, 0);
    std::vector<uint32_t> freq_d(40, 0);

    size_t idx = 0;
    while (idx < total_pixels) {
        uint32_t argb = argb_pixels[idx];

        // Check for RLE backward match (distance = 1 -> plane code = 2 -> dist_symbol = 2 -> prefix_code = 1)
        if (idx > 0 && argb_pixels[idx - 1] == argb) {
            int run_len = 0;
            while (idx + run_len < total_pixels &&
                   run_len < 4096 &&
                   argb_pixels[idx + run_len] == argb) {
                uint32_t key = (0x1e35a7bdU * argb) >> (32 - kCacheBits);
                color_cache[key] = argb;
                cache_valid[key] = true;
                ++run_len;
            }
            int prefix_code = 0, extra_bits = 0;
            uint32_t extra_val = 0;
            encode_vp8l_prefix(run_len, prefix_code, extra_bits, extra_val);

            Token tk{};
            tk.green_sym = static_cast<uint16_t>(256 + prefix_code);
            tk.len_extra_bits = static_cast<uint8_t>(extra_bits);
            tk.len_extra_val = extra_val;
            tk.is_len = true;
            tokens.push_back(tk);

            freq_g[tk.green_sym]++;
            freq_d[1]++; // plane_code = 2 (dx=1, dy=0) -> prefix_code = 1
            idx += static_cast<size_t>(run_len);
            continue;
        }

        uint32_t key = (0x1e35a7bdU * argb) >> (32 - kCacheBits);
        if (cache_valid[key] && color_cache[key] == argb) {
            Token tk{};
            tk.green_sym = static_cast<uint16_t>(256 + 24 + key);
            tokens.push_back(tk);
            freq_g[tk.green_sym]++;
            idx++;
            continue;
        }

        color_cache[key] = argb;
        cache_valid[key] = true;

        Token tk{};
        tk.a = static_cast<uint8_t>((argb >> 24) & 0xff);
        tk.r = static_cast<uint8_t>((argb >> 16) & 0xff);
        tk.green_sym = static_cast<uint16_t>((argb >> 8) & 0xff);
        tk.b = static_cast<uint8_t>(argb & 0xff);
        tokens.push_back(tk);

        freq_g[tk.green_sym]++;
        freq_r[tk.r]++;
        freq_b[tk.b]++;
        freq_a[tk.a]++;
        idx++;
    }

    auto huff_g = build_vp8l_huff_table(freq_g);
    auto huff_r = build_vp8l_huff_table(freq_r);
    auto huff_b = build_vp8l_huff_table(freq_b);
    auto huff_a = build_vp8l_huff_table(freq_a);
    auto huff_d = build_vp8l_huff_table(freq_d);

    std::vector<uint8_t> vp8l_stream;
    vp8l_stream.push_back(0x2F);
    uint32_t w_val = width - 1;
    uint32_t h_val = height - 1;
    vp8l_stream.push_back(static_cast<uint8_t>(w_val & 0xff));
    vp8l_stream.push_back(static_cast<uint8_t>(((w_val >> 8) & 0x3f) | ((h_val & 0x03) << 6)));
    vp8l_stream.push_back(static_cast<uint8_t>((h_val >> 2) & 0xff));
    vp8l_stream.push_back(static_cast<uint8_t>(((h_val >> 10) & 0x0f) | (has_alpha ? 0x10 : 0x00)));

    Vp8lBitWriter bw;
    // Transform present = 1 -> SUBTRACT_GREEN (2), then no more transforms (0)
    bw.write_bits(1, 1);
    bw.write_bits(SUBTRACT_GREEN, 2);
    bw.write_bits(0, 1);

    // Color cache present = 1 -> kCacheBits
    bw.write_bits(1, 1);
    bw.write_bits(kCacheBits, 4);

    // Meta Huffman = 0
    bw.write_bits(0, 1);

    write_vp8l_huff_table(bw, huff_g);
    write_vp8l_huff_table(bw, huff_r);
    write_vp8l_huff_table(bw, huff_b);
    write_vp8l_huff_table(bw, huff_a);
    write_vp8l_huff_table(bw, huff_d);

    auto write_sym = [&](const Vp8lEncHuffTable& ht, int sym) {
        uint8_t len = ht.lengths[sym];
        if (len > 0) {
            bw.write_bits(ht.codes_rev[sym], len);
        }
    };

    for (const auto& tk : tokens) {
        write_sym(huff_g, tk.green_sym);
        if (tk.green_sym < 256) {
            write_sym(huff_r, tk.r);
            write_sym(huff_b, tk.b);
            write_sym(huff_a, tk.a);
        } else if (tk.is_len) {
            if (tk.len_extra_bits > 0) {
                bw.write_bits(tk.len_extra_val, tk.len_extra_bits);
            }
            write_sym(huff_d, 1); // dist_code = 1 -> plane_code = 2 -> distance = 1
        }
    }

    bw.flush();
    vp8l_stream.insert(vp8l_stream.end(), bw.bytes().begin(), bw.bytes().end());
    return vp8l_stream;
}

// ============================================================================
// Sovereign VP8 Lossy Keyframe Encoder (RFC 6386)
// ============================================================================

class Vp8BoolEncoder {
public:
    void put_bit(int bit, uint8_t prob) {
        uint32_t split = 1 + (((range_ - 1) * prob) >> 8);
        if (bit) {
            bottom_ += split;
            range_ -= split;
        } else {
            range_ = split;
        }
        while (range_ < 128) {
            range_ <<= 1;
            if (bottom_ & (1u << 31)) {
                add_one_to_output();
            }
            bottom_ <<= 1;
            if (--bit_count_ == 0) {
                bytes_.push_back(static_cast<uint8_t>(bottom_ >> 24));
                bottom_ &= (1u << 24) - 1;
                bit_count_ = 8;
            }
        }
    }

    void put_uint(uint32_t v, int bits) {
        for (int i = bits - 1; i >= 0; --i) {
            put_bit((v >> i) & 1, 128);
        }
    }

    void flush() {
        int c = bit_count_;
        uint32_t v = bottom_;
        if (v & (1u << (32 - c))) {
            add_one_to_output();
        }
        v <<= (c & 7);
        c >>= 3;
        while (--c >= 0) {
            v <<= 8;
        }
        for (int i = 0; i < 4; ++i) {
            bytes_.push_back(static_cast<uint8_t>(v >> 24));
            v <<= 8;
        }
    }

    const std::vector<uint8_t>& bytes() const noexcept { return bytes_; }

private:
    void add_one_to_output() {
        for (auto it = bytes_.rbegin(); it != bytes_.rend(); ++it) {
            if (*it < 255) {
                (*it)++;
                break;
            }
            *it = 0;
        }
    }

    std::vector<uint8_t> bytes_;
    uint32_t range_{255};
    uint32_t bottom_{0};
    int bit_count_{24};
};

void vp8_fdct4x4(const int16_t in[16], int16_t out[16]) noexcept {
    int32_t tmp[16];
    for (int i = 0; i < 4; ++i) {
        int32_t a1 = (in[i * 4 + 0] + in[i * 4 + 3]) * 8;
        int32_t b1 = (in[i * 4 + 1] + in[i * 4 + 2]) * 8;
        int32_t c1 = (in[i * 4 + 1] - in[i * 4 + 2]) * 8;
        int32_t d1 = (in[i * 4 + 0] - in[i * 4 + 3]) * 8;

        tmp[i * 4 + 0] = a1 + b1;
        tmp[i * 4 + 2] = a1 - b1;
        tmp[i * 4 + 1] = (c1 * 2217 + d1 * 5352 + 14500) >> 12;
        tmp[i * 4 + 3] = (d1 * 2217 - c1 * 5352 + 7500) >> 12;
    }
    for (int i = 0; i < 4; ++i) {
        int32_t a1 = tmp[0 * 4 + i] + tmp[3 * 4 + i];
        int32_t b1 = tmp[1 * 4 + i] + tmp[2 * 4 + i];
        int32_t c1 = tmp[1 * 4 + i] - tmp[2 * 4 + i];
        int32_t d1 = tmp[0 * 4 + i] - tmp[3 * 4 + i];

        out[0 * 4 + i] = static_cast<int16_t>((a1 + b1 + 7) >> 4);
        out[2 * 4 + i] = static_cast<int16_t>((a1 - b1 + 7) >> 4);
        out[1 * 4 + i] = static_cast<int16_t>(((c1 * 2217 + d1 * 5352 + 12000) >> 16) + (d1 != 0 ? 1 : 0));
        out[3 * 4 + i] = static_cast<int16_t>((d1 * 2217 - c1 * 5352 + 51000) >> 16);
    }
}

void vp8_fwht4x4(const int16_t in[16], int16_t out[16]) noexcept {
    int32_t tmp[16];
    for (int i = 0; i < 4; ++i) {
        int32_t a1 = in[0 + i] + in[12 + i];
        int32_t b1 = in[4 + i] + in[8 + i];
        int32_t c1 = in[4 + i] - in[8 + i];
        int32_t d1 = in[0 + i] - in[12 + i];
        tmp[0 + i]  = a1 + b1;
        tmp[4 + i]  = c1 + d1;
        tmp[8 + i]  = a1 - b1;
        tmp[12 + i] = d1 - c1;
    }
    for (int i = 0; i < 4; ++i) {
        const int32_t* t = tmp + i * 4;
        int32_t a1 = t[0] + t[3];
        int32_t b1 = t[1] + t[2];
        int32_t c1 = t[1] - t[2];
        int32_t d1 = t[0] - t[3];
        int32_t a2 = a1 + b1;
        int32_t b2 = c1 + d1;
        int32_t c2 = a1 - b1;
        int32_t d2 = d1 - c1;
        out[i * 4 + 0] = static_cast<int16_t>((a2 + (a2 >= 0 ? 1 : 0)) >> 1);
        out[i * 4 + 1] = static_cast<int16_t>((b2 + (b2 >= 0 ? 1 : 0)) >> 1);
        out[i * 4 + 2] = static_cast<int16_t>((c2 + (c2 >= 0 ? 1 : 0)) >> 1);
        out[i * 4 + 3] = static_cast<int16_t>((d2 + (d2 >= 0 ? 1 : 0)) >> 1);
    }
}

int encode_vp8_coeffs(
    Vp8BoolEncoder& be,
    const uint8_t probs[8][3][11],
    int ctx,
    int16_t dc_q,
    int16_t ac_q,
    int first_coeff,
    const int16_t in_block[16],
    int16_t dequant_block[16]
) noexcept {
    int16_t qcoeffs[16]{};
    int last_nz = -1;
    for (int i = first_coeff; i < 16; ++i) {
        int pos = kVp8Zigzag[i];
        int16_t q = (i == 0) ? dc_q : ac_q;
        int16_t c = static_cast<int16_t>(in_block[pos] / q);
        qcoeffs[i] = c;
        dequant_block[pos] = static_cast<int16_t>(c * q);
        if (c != 0) last_nz = i;
    }

    bool skip_eob = false;
    for (int i = first_coeff; i < 16; ++i) {
        const uint8_t* p = probs[kVp8CoeffBands[i]][ctx];
        if (i > last_nz) {
            if (!skip_eob) {
                be.put_bit(0, p[0]); // DCT_EOB
            }
            break;
        }
        int v = std::abs(static_cast<int>(qcoeffs[i]));
        if (v == 0) {
            if (!skip_eob) be.put_bit(1, p[0]);
            be.put_bit(0, p[1]); // DCT_0
            ctx = 0;
            skip_eob = true;
            continue;
        }

        if (!skip_eob) be.put_bit(1, p[0]);
        be.put_bit(1, p[1]); // Not DCT_0
        skip_eob = false;

        if (v == 1) {
            be.put_bit(0, p[2]); // DCT_1
        } else {
            be.put_bit(1, p[2]);
            if (v <= 4) {
                be.put_bit(0, p[3]);
                if (v == 2) {
                    be.put_bit(0, p[4]); // DCT_2
                } else {
                    be.put_bit(1, p[4]);
                    be.put_bit(v == 4 ? 1 : 0, p[5]); // DCT_3 or DCT_4
                }
            } else {
                be.put_bit(1, p[3]);
                auto write_cat = [&](const uint8_t* cp, int len, int extra) {
                    for (int b = len - 1; b >= 0; --b) {
                        be.put_bit((extra >> b) & 1, cp[len - 1 - b]);
                    }
                };
                if (v <= 10) {
                    be.put_bit(0, p[6]);
                    if (v <= 6) {
                        be.put_bit(0, p[7]); // CAT1 (5..6)
                        write_cat(kVp8Cat1Probs, 1, v - 5);
                    } else {
                        be.put_bit(1, p[7]); // CAT2 (7..10)
                        write_cat(kVp8Cat2Probs, 2, v - 7);
                    }
                } else {
                    be.put_bit(1, p[6]);
                    if (v <= 34) {
                        be.put_bit(0, p[8]);
                        if (v <= 18) {
                            be.put_bit(0, p[9]); // CAT3 (11..18)
                            write_cat(kVp8Cat3Probs, 3, v - 11);
                        } else {
                            be.put_bit(1, p[9]); // CAT4 (19..34)
                            write_cat(kVp8Cat4Probs, 4, v - 19);
                        }
                    } else {
                        be.put_bit(1, p[8]);
                        if (v <= 66) {
                            be.put_bit(0, p[10]); // CAT5 (35..66)
                            write_cat(kVp8Cat5Probs, 5, v - 35);
                        } else {
                            be.put_bit(1, p[10]); // CAT6 (67..2048)
                            write_cat(kVp8Cat6Probs, 11, std::min(v, 2048) - 67);
                        }
                    }
                }
            }
        }

        be.put_bit(qcoeffs[i] < 0 ? 1 : 0, 128);
        ctx = (v == 1) ? 1 : 2;
    }

    return (last_nz >= first_coeff) ? 1 : 0;
}

std::vector<uint8_t> encode_vp8_payload(ImageRef pixmap, int quality) {
    uint32_t width = pixmap.width();
    uint32_t height = pixmap.height();
    const PremultipliedColorU8* px_data = pixmap.pixels();

    int q_clamped = std::clamp(quality, 1, 100);
    int y_ac_qi = std::clamp((100 - q_clamped) * 127 / 100, 0, 127);

    int16_t y1_dc = kVp8DcQLookup[y_ac_qi];
    int16_t y1_ac = kVp8AcQLookup[y_ac_qi];
    int16_t y2_dc = static_cast<int16_t>(kVp8DcQLookup[y_ac_qi] * 2);
    int16_t y2_ac = static_cast<int16_t>(std::max<int>(8, (kVp8AcQLookup[y_ac_qi] * 155) / 100));
    int16_t uv_dc = std::min<int16_t>(132, kVp8DcQLookup[y_ac_qi]);
    int16_t uv_ac = kVp8AcQLookup[y_ac_qi];

    uint32_t mb_cols = (width + 15) >> 4;
    uint32_t mb_rows = (height + 15) >> 4;

    // Convert RGBA8 to padded YUV420 source planes
    uint32_t pad_w = mb_cols * 16;
    uint32_t pad_h = mb_rows * 16;
    std::vector<uint8_t> src_y(static_cast<size_t>(pad_w) * pad_h, 0);
    std::vector<uint8_t> src_u(static_cast<size_t>(pad_w / 2) * (pad_h / 2), 128);
    std::vector<uint8_t> src_v(static_cast<size_t>(pad_w / 2) * (pad_h / 2), 128);

    for (uint32_t y = 0; y < pad_h; ++y) {
        uint32_t sy = std::min(y, height - 1);
        for (uint32_t x = 0; x < pad_w; ++x) {
            uint32_t sx = std::min(x, width - 1);
            ColorU8 c = px_data[sy * width + sx].demultiply();
            int y_val = ((66 * c.r + 129 * c.g + 25 * c.b + 128) >> 8) + 16;
            src_y[y * pad_w + x] = clamp_u8(y_val);
        }
    }

    for (uint32_t cy = 0; cy < pad_h / 2; ++cy) {
        for (uint32_t cx = 0; cx < pad_w / 2; ++cx) {
            int r_sum = 0, g_sum = 0, b_sum = 0;
            for (uint32_t dy = 0; dy < 2; ++dy) {
                for (uint32_t dx = 0; dx < 2; ++dx) {
                    uint32_t sy = std::min(cy * 2 + dy, height - 1);
                    uint32_t sx = std::min(cx * 2 + dx, width - 1);
                    ColorU8 c = px_data[sy * width + sx].demultiply();
                    r_sum += c.r; g_sum += c.g; b_sum += c.b;
                }
            }
            int r = (r_sum + 2) >> 2;
            int g = (g_sum + 2) >> 2;
            int b = (b_sum + 2) >> 2;
            int u_val = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            int v_val = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            src_u[cy * (pad_w / 2) + cx] = clamp_u8(u_val);
            src_v[cy * (pad_w / 2) + cx] = clamp_u8(v_val);
        }
    }

    Vp8BoolEncoder bd0;
    Vp8BoolEncoder bd1;

    // Header in Partition 0
    bd0.put_bit(0, 128); // color_space = 0
    bd0.put_bit(0, 128); // clamping = 0
    bd0.put_bit(0, 128); // update_segmentation = 0
    bd0.put_bit(0, 128); // filter_type = 0
    bd0.put_uint(0, 6);  // filter_level = 0
    bd0.put_uint(0, 3);  // sharpness = 0
    bd0.put_bit(0, 128); // lf_adj_enable = 0
    bd0.put_uint(0, 2);  // log2_parts = 0 (1 partition)
    bd0.put_uint(static_cast<uint32_t>(y_ac_qi), 7);
    for (int i = 0; i < 5; ++i) bd0.put_bit(0, 128); // no delta q
    bd0.put_bit(0, 128); // refresh_entropy_probs = 0

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 3; ++k) {
                for (int l = 0; l < 11; ++l) {
                    bd0.put_bit(0, kVp8CoeffUpdateProbs[i][j][k][l]);
                }
            }
        }
    }
    bd0.put_bit(0, 128); // mb_no_skip_coeff = 0

    // Reconstructed planes for in-loop intra prediction
    int y_stride  = static_cast<int>(mb_cols * 16 + 16);
    int uv_stride = static_cast<int>(mb_cols * 8 + 16);
    std::vector<uint8_t> rec_y(static_cast<size_t>(mb_rows * 16 + 16) * y_stride, 128);
    std::vector<uint8_t> rec_u(static_cast<size_t>(mb_rows * 8 + 16) * uv_stride, 128);
    std::vector<uint8_t> rec_v(static_cast<size_t>(mb_rows * 8 + 16) * uv_stride, 128);

    uint8_t* y_origin = rec_y.data() + 4 * y_stride + 8;
    uint8_t* u_origin = rec_u.data() + 4 * uv_stride + 8;
    uint8_t* v_origin = rec_v.data() + 4 * uv_stride + 8;

    std::memset(y_origin - y_stride - 1, 127, mb_cols * 16 + 5);
    std::memset(u_origin - uv_stride - 1, 127, mb_cols * 8 + 5);
    std::memset(v_origin - uv_stride - 1, 127, mb_cols * 8 + 5);

    struct EncMbCtx {
        uint8_t nz[9]{};
    };
    std::vector<EncMbCtx> top_ctx(mb_cols);

    for (uint32_t mb_y = 0; mb_y < mb_rows; ++mb_y) {
        for (int r = -1; r < 16; ++r) {
            if (r == -1) continue;
            y_origin[(static_cast<int>(mb_y) * 16 + r) * y_stride - 1] = 129;
        }
        for (int r = -1; r < 8; ++r) {
            if (r == -1) continue;
            u_origin[(static_cast<int>(mb_y) * 8 + r) * uv_stride - 1] = 129;
            v_origin[(static_cast<int>(mb_y) * 8 + r) * uv_stride - 1] = 129;
        }

        EncMbCtx left_ctx{};

        for (uint32_t mb_x = 0; mb_x < mb_cols; ++mb_x) {
            // Write y_mode = DC_PRED (1, 0, 0) and uv_mode = DC_PRED (0)
            bd0.put_bit(1, kVp8KfYModeProbs[0]);
            bd0.put_bit(0, kVp8KfYModeProbs[1]);
            bd0.put_bit(0, kVp8KfYModeProbs[2]);
            bd0.put_bit(0, kVp8KfUvModeProbs[0]);

            uint8_t* mb_y_dst = y_origin + (mb_y * 16) * y_stride + (mb_x * 16);
            uint8_t* mb_u_dst = u_origin + (mb_y * 8) * uv_stride + (mb_x * 8);
            uint8_t* mb_v_dst = v_origin + (mb_y * 8) * uv_stride + (mb_x * 8);

            vp8_predict_16x16(DC_PRED, mb_y_dst, y_stride, mb_y > 0, mb_x > 0);
            vp8_predict_8x8(DC_PRED, mb_u_dst, uv_stride, mb_y > 0, mb_x > 0);
            vp8_predict_8x8(DC_PRED, mb_v_dst, uv_stride, mb_y > 0, mb_x > 0);

            int16_t y_dct[16][16]{};
            int16_t y2_in[16]{};
            for (int by = 0; by < 4; ++by) {
                for (int bx = 0; bx < 4; ++bx) {
                    int b_idx = by * 4 + bx;
                    int16_t res[16];
                    for (int r = 0; r < 4; ++r) {
                        for (int c = 0; c < 4; ++c) {
                            uint32_t py = mb_y * 16 + by * 4 + r;
                            uint32_t px = mb_x * 16 + bx * 4 + c;
                            res[r * 4 + c] = static_cast<int16_t>(src_y[py * pad_w + px]) -
                                             static_cast<int16_t>(mb_y_dst[(by * 4 + r) * y_stride + (bx * 4 + c)]);
                        }
                    }
                    vp8_fdct4x4(res, y_dct[b_idx]);
                    y2_in[b_idx] = y_dct[b_idx][0];
                }
            }

            int16_t y2_wht[16]{};
            int16_t y2_deq[16]{};
            int16_t y2_dc_rec[16]{};
            vp8_fwht4x4(y2_in, y2_wht);

            int ctx_y2 = top_ctx[mb_x].nz[8] + left_ctx.nz[8];
            int nz_y2 = encode_vp8_coeffs(bd1, kVp8DefaultCoeffProbs[1], ctx_y2, y2_dc, y2_ac, 0, y2_wht, y2_deq);
            top_ctx[mb_x].nz[8] = left_ctx.nz[8] = static_cast<uint8_t>(nz_y2);
            vp8_iwht4x4(y2_deq, y2_dc_rec);

            for (int by = 0; by < 4; ++by) {
                for (int bx = 0; bx < 4; ++bx) {
                    int b_idx = by * 4 + bx;
                    int16_t y1_deq[16]{};
                    y1_deq[0] = y2_dc_rec[b_idx];
                    int ctx = top_ctx[mb_x].nz[bx] + left_ctx.nz[by];
                    int nz = encode_vp8_coeffs(bd1, kVp8DefaultCoeffProbs[0], ctx, y1_dc, y1_ac, 1, y_dct[b_idx], y1_deq);
                    top_ctx[mb_x].nz[bx] = left_ctx.nz[by] = static_cast<uint8_t>(nz);
                    vp8_idct4x4_add(y1_deq, mb_y_dst + (by * 4) * y_stride + (bx * 4), y_stride);
                }
            }

            for (int by = 0; by < 2; ++by) {
                for (int bx = 0; bx < 2; ++bx) {
                    int16_t res[16], dct[16], deq[16]{};
                    for (int r = 0; r < 4; ++r) {
                        for (int c = 0; c < 4; ++c) {
                            uint32_t py = mb_y * 8 + by * 4 + r;
                            uint32_t px = mb_x * 8 + bx * 4 + c;
                            res[r * 4 + c] = static_cast<int16_t>(src_u[py * (pad_w / 2) + px]) -
                                             static_cast<int16_t>(mb_u_dst[(by * 4 + r) * uv_stride + (bx * 4 + c)]);
                        }
                    }
                    vp8_fdct4x4(res, dct);
                    int ctx = top_ctx[mb_x].nz[4 + bx] + left_ctx.nz[4 + by];
                    int nz = encode_vp8_coeffs(bd1, kVp8DefaultCoeffProbs[2], ctx, uv_dc, uv_ac, 0, dct, deq);
                    top_ctx[mb_x].nz[4 + bx] = left_ctx.nz[4 + by] = static_cast<uint8_t>(nz);
                    vp8_idct4x4_add(deq, mb_u_dst + (by * 4) * uv_stride + (bx * 4), uv_stride);
                }
            }

            for (int by = 0; by < 2; ++by) {
                for (int bx = 0; bx < 2; ++bx) {
                    int16_t res[16], dct[16], deq[16]{};
                    for (int r = 0; r < 4; ++r) {
                        for (int c = 0; c < 4; ++c) {
                            uint32_t py = mb_y * 8 + by * 4 + r;
                            uint32_t px = mb_x * 8 + bx * 4 + c;
                            res[r * 4 + c] = static_cast<int16_t>(src_v[py * (pad_w / 2) + px]) -
                                             static_cast<int16_t>(mb_v_dst[(by * 4 + r) * uv_stride + (bx * 4 + c)]);
                        }
                    }
                    vp8_fdct4x4(res, dct);
                    int ctx = top_ctx[mb_x].nz[6 + bx] + left_ctx.nz[6 + by];
                    int nz = encode_vp8_coeffs(bd1, kVp8DefaultCoeffProbs[2], ctx, uv_dc, uv_ac, 0, dct, deq);
                    top_ctx[mb_x].nz[6 + bx] = left_ctx.nz[6 + by] = static_cast<uint8_t>(nz);
                    vp8_idct4x4_add(deq, mb_v_dst + (by * 4) * uv_stride + (bx * 4), uv_stride);
                }
            }
        }
    }

    bd0.flush();
    bd1.flush();

    uint32_t part0_len = static_cast<uint32_t>(bd0.bytes().size());
    uint32_t frame_tag = (part0_len << 5) | (1u << 4) | 0u; // keyframe, version 0, show_frame 1

    std::vector<uint8_t> vp8_stream;
    vp8_stream.reserve(10 + bd0.bytes().size() + bd1.bytes().size());
    vp8_stream.push_back(static_cast<uint8_t>(frame_tag & 0xff));
    vp8_stream.push_back(static_cast<uint8_t>((frame_tag >> 8) & 0xff));
    vp8_stream.push_back(static_cast<uint8_t>((frame_tag >> 16) & 0xff));
    vp8_stream.push_back(0x9D);
    vp8_stream.push_back(0x01);
    vp8_stream.push_back(0x2A);
    vp8_stream.push_back(static_cast<uint8_t>(width & 0xff));
    vp8_stream.push_back(static_cast<uint8_t>((width >> 8) & 0x3f));
    vp8_stream.push_back(static_cast<uint8_t>(height & 0xff));
    vp8_stream.push_back(static_cast<uint8_t>((height >> 8) & 0x3f));
    vp8_stream.insert(vp8_stream.end(), bd0.bytes().begin(), bd0.bytes().end());
    vp8_stream.insert(vp8_stream.end(), bd1.bytes().begin(), bd1.bytes().end());
    return vp8_stream;
}

} // namespace

// ============================================================================
// Public WebP API Implementation
// ============================================================================

bool is_animated_webp(std::span<const uint8_t> webp_data) {
    auto cont_res = parse_webp_container(webp_data);
    if (!cont_res.has_value()) return false;
    return cont_res->has_animation && !cont_res->anmf_chunks.empty();
}

ImageResult<WebPAnimation> decode_webp_animation(std::span<const uint8_t> webp_data) {
    auto cont_res = parse_webp_container(webp_data);
    if (!cont_res.has_value()) {
        return ImageResult<WebPAnimation>::fail(cont_res.error);
    }

    const auto& container = *cont_res;
    if (container.anmf_chunks.empty()) {
        return ImageResult<WebPAnimation>::fail(ImageError::UnsupportedFormat);
    }

    uint32_t canvas_w = container.canvas_width;
    uint32_t canvas_h = container.canvas_height;
    if (canvas_w == 0 || canvas_h == 0) {
        return ImageResult<WebPAnimation>::fail(ImageError::InvalidDimensions);
    }

    WebPAnimation anim;
    anim.canvas_width = canvas_w;
    anim.canvas_height = canvas_h;
    anim.loop_count = container.loop_count;
    anim.background_color = container.background_color;

    auto maybe_canvas = Image::allocate(canvas_w, canvas_h);
    if (!maybe_canvas.has_value()) {
        return ImageResult<WebPAnimation>::fail(ImageError::OutOfMemory);
    }
    Image canvas = std::move(*maybe_canvas);

    PremultipliedColorU8 bg_color = container.background_color.premultiply();
    PremultipliedColorU8* canvas_pixels = canvas.pixels_mut();
    for (size_t i = 0; i < static_cast<size_t>(canvas_w) * canvas_h; ++i) {
        canvas_pixels[i] = bg_color;
    }

    uint32_t prev_x = 0, prev_y = 0, prev_w = 0, prev_h = 0;
    bool prev_dispose_bg = false;

    for (const auto& anmf_chunk : container.anmf_chunks) {
        auto payload = anmf_chunk.payload;
        if (payload.size() < 16) continue;

        uint32_t frame_x = read_u24_le(payload.data()) * 2;
        uint32_t frame_y = read_u24_le(payload.data() + 3) * 2;
        uint32_t frame_w = 1 + read_u24_le(payload.data() + 6);
        uint32_t frame_h = 1 + read_u24_le(payload.data() + 9);
        uint32_t duration = read_u24_le(payload.data() + 12);
        uint8_t flags = payload[15];

        bool no_blend   = (flags & 0x02) != 0;
        bool dispose_bg = (flags & 0x01) != 0;

        size_t sub_offset = 16;
        std::optional<WebPChunk> frame_vp8l;
        std::optional<WebPChunk> frame_vp8;
        std::optional<WebPChunk> frame_alph;

        while (sub_offset + 8 <= payload.size()) {
            uint32_t sub_fourcc = read_u32_le(payload.data() + sub_offset);
            uint32_t sub_len = read_u32_le(payload.data() + sub_offset + 4);
            sub_offset += 8;

            size_t sub_avail = (sub_offset <= payload.size()) ? (payload.size() - sub_offset) : 0;
            size_t sub_payload_len = std::min(static_cast<size_t>(sub_len), sub_avail);
            std::span<const uint8_t> sub_payload(payload.data() + sub_offset, sub_payload_len);
            WebPChunk sub_chunk{sub_fourcc, sub_payload};

            if (sub_fourcc == CHUNK_VP8L) {
                frame_vp8l = sub_chunk;
            } else if (sub_fourcc == CHUNK_VP8) {
                frame_vp8 = sub_chunk;
            } else if (sub_fourcc == CHUNK_ALPH) {
                frame_alph = sub_chunk;
            }

            sub_offset += sub_len + (sub_len & 1);
        }

        ImageResult<Image> sub_res = ImageResult<Image>::fail(ImageError::UnsupportedFormat);
        if (frame_vp8l.has_value()) {
            sub_res = decode_vp8l_internal(frame_vp8l->payload);
        } else if (frame_vp8.has_value()) {
            std::span<const uint8_t> alph_data;
            if (frame_alph.has_value()) {
                alph_data = frame_alph->payload;
            }
            sub_res = decode_vp8_internal(frame_vp8->payload, alph_data);
        }

        if (!sub_res.has_value()) continue;
        const Image& sub_pixmap = *sub_res;

        if (prev_dispose_bg) {
            for (uint32_t py = 0; py < prev_h && (prev_y + py) < canvas_h; ++py) {
                for (uint32_t px = 0; px < prev_w && (prev_x + px) < canvas_w; ++px) {
                    canvas_pixels[(prev_y + py) * canvas_w + (prev_x + px)] = bg_color;
                }
            }
        }

        const PremultipliedColorU8* sub_pixels = sub_pixmap.pixels();
        for (uint32_t py = 0; py < frame_h && py < sub_pixmap.height() && (frame_y + py) < canvas_h; ++py) {
            for (uint32_t px = 0; px < frame_w && px < sub_pixmap.width() && (frame_x + px) < canvas_w; ++px) {
                size_t dst_idx = (frame_y + py) * canvas_w + (frame_x + px);
                PremultipliedColorU8 src = sub_pixels[py * sub_pixmap.width() + px];

                if (no_blend || src.a == 255) {
                    canvas_pixels[dst_idx] = src;
                } else if (src.a != 0) {
                    PremultipliedColorU8 dst = canvas_pixels[dst_idx];
                    uint16_t inv_a = 255 - src.a;
                    uint8_t r = static_cast<uint8_t>(src.r + ((static_cast<uint16_t>(dst.r) * inv_a + 127) / 255));
                    uint8_t g = static_cast<uint8_t>(src.g + ((static_cast<uint16_t>(dst.g) * inv_a + 127) / 255));
                    uint8_t b = static_cast<uint8_t>(src.b + ((static_cast<uint16_t>(dst.b) * inv_a + 127) / 255));
                    uint8_t a = static_cast<uint8_t>(src.a + ((static_cast<uint16_t>(dst.a) * inv_a + 127) / 255));
                    canvas_pixels[dst_idx] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, a);
                }
            }
        }

        WebPAnimationFrame anim_frame;
        anim_frame.image = canvas;
        anim_frame.duration_ms = duration;
        anim_frame.x = frame_x;
        anim_frame.y = frame_y;
        anim_frame.width = frame_w;
        anim_frame.height = frame_h;
        anim_frame.blend_method = no_blend ? WebPBlendMethod::Overwrite : WebPBlendMethod::AlphaBlend;
        anim_frame.dispose_method = dispose_bg ? WebPDisposeMethod::Background : WebPDisposeMethod::None;
        anim.frames.push_back(std::move(anim_frame));

        prev_x = frame_x;
        prev_y = frame_y;
        prev_w = frame_w;
        prev_h = frame_h;
        prev_dispose_bg = dispose_bg;
    }

    if (anim.frames.empty()) {
        return ImageResult<WebPAnimation>::fail(ImageError::UnsupportedFormat);
    }

    return ImageResult<WebPAnimation>::success(std::move(anim));
}

ImageResult<Image> decode_webp(std::span<const uint8_t> webp_data, DecodeOptions options) {
    auto cont_res = parse_webp_container(webp_data);
    if (!cont_res.has_value()) {
        return ImageResult<Image>::fail(cont_res.error);
    }

    const auto& container = *cont_res;
    ImageResult<Image> res = ImageResult<Image>::fail(ImageError::UnsupportedFormat);

    if (container.has_animation && !container.anmf_chunks.empty()) {
        auto anim_res = decode_webp_animation(webp_data);
        if (anim_res.has_value() && !anim_res->frames.empty()) {
            res = ImageResult<Image>::success(std::move(anim_res->frames[0].image));
        }
    } else if (container.vp8l_chunk.has_value()) {
        res = decode_vp8l_internal(container.vp8l_chunk->payload, options);
    } else if (container.vp8_chunk.has_value()) {
        std::span<const uint8_t> alpha_payload;
        if (container.alph_chunk.has_value()) {
            alpha_payload = container.alph_chunk->payload;
        }
        res = decode_vp8_internal(container.vp8_chunk->payload, alpha_payload, options);
    }

    if (res.has_value() && options.apply_exif_orientation && container.exif_orientation > 1) {
        *res.value = apply_webp_exif_orientation(*res.value, container.exif_orientation);
    }

    return res;
}

ImageResult<ImageInfo> probe_webp(std::span<const uint8_t> webp_data) {
    auto cont_res = parse_webp_container(webp_data);
    if (!cont_res.has_value()) {
        return ImageResult<ImageInfo>::fail(cont_res.error);
    }

    const auto& container = *cont_res;
    ImageInfo info;
    info.format = ImageFormat::WebP;
    info.bit_depth = 8;
    info.color_model = ColorModel::RGBA;
    info.exif_orientation = container.exif_orientation;

    if (container.canvas_width > 0 && container.canvas_height > 0) {
        info.width = container.canvas_width;
        info.height = container.canvas_height;
        info.has_alpha = container.has_alpha;
        info.channels = info.has_alpha ? 4 : 3;
        return ImageResult<ImageInfo>::success(info);
    }

    if (container.vp8l_chunk.has_value()) {
        auto pl = container.vp8l_chunk->payload;
        if (pl.size() >= 5 && pl[0] == 0x2F) {
            info.width = 1 + (pl[1] | ((pl[2] & 0x3F) << 8));
            info.height = 1 + ((pl[2] >> 6) | (pl[3] << 2) | ((pl[4] & 0x0F) << 10));
            info.has_alpha = (pl[4] & 0x10) != 0;
            info.channels = info.has_alpha ? 4 : 3;
            return ImageResult<ImageInfo>::success(info);
        }
    }

    if (container.vp8_chunk.has_value()) {
        auto pl = container.vp8_chunk->payload;
        if (pl.size() >= 10 && pl[3] == 0x9D && pl[4] == 0x01 && pl[5] == 0x2A) {
            info.width = (pl[6] | (pl[7] << 8)) & 0x3FFF;
            info.height = (pl[8] | (pl[9] << 8)) & 0x3FFF;
            info.has_alpha = container.alph_chunk.has_value();
            info.channels = info.has_alpha ? 4 : 3;
            return ImageResult<ImageInfo>::success(info);
        }
    }

    return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
}

ImageResult<std::vector<uint8_t>> encode_webp(
    ImageRef pixmap,
    bool lossless,
    int quality
) {
    if (pixmap.is_empty()) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    Image temp_converted;
    if (pixmap.format() != PixelFormat::RGBA8888) {
        temp_converted = pixmap.to_rgba8888();
        pixmap = temp_converted.as_ref();
    }

    uint32_t chunk_fourcc = lossless ? CHUNK_VP8L : CHUNK_VP8;
    std::vector<uint8_t> chunk_stream = lossless
        ? encode_vp8l_payload(pixmap)
        : encode_vp8_payload(pixmap, quality);

    uint32_t chunk_size = static_cast<uint32_t>(chunk_stream.size());
    uint32_t padded_size = chunk_size + (chunk_size & 1);
    uint32_t riff_payload_size = 4 + 8 + padded_size;

    std::vector<uint8_t> file_out;
    file_out.reserve(8 + riff_payload_size);

    uint8_t u32_buf[4];
    write_u32_le(u32_buf, CHUNK_RIFF);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);
    write_u32_le(u32_buf, riff_payload_size);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);
    write_u32_le(u32_buf, CHUNK_WEBP);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);

    write_u32_le(u32_buf, chunk_fourcc);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);
    write_u32_le(u32_buf, chunk_size);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);
    file_out.insert(file_out.end(), chunk_stream.begin(), chunk_stream.end());

    if (chunk_size & 1) {
        file_out.push_back(0);
    }

    return ImageResult<std::vector<uint8_t>>::success(std::move(file_out));
}

ImageResult<Image> decode_webp_file(const std::string& file_path, DecodeOptions options) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return ImageResult<Image>::fail(ImageError::IOError);
    }
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return ImageResult<Image>::fail(ImageError::CorruptedData);
    }
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Image>::fail(ImageError::IOError);
    }
    return decode_webp(buffer, options);
}

ImageResult<WebPAnimation> decode_webp_animation_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return ImageResult<WebPAnimation>::fail(ImageError::IOError);
    }
    std::streamsize size = file.tellg();
    if (size <= 0) {
        return ImageResult<WebPAnimation>::fail(ImageError::CorruptedData);
    }
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<WebPAnimation>::fail(ImageError::IOError);
    }
    return decode_webp_animation(buffer);
}

bool save_webp_file(
    ImageRef pixmap,
    const std::string& file_path,
    bool lossless,
    int quality
) {
    auto enc_res = encode_webp(pixmap, lossless, quality);
    if (!enc_res.has_value()) return false;

    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(enc_res->data()), enc_res->size());
    return file.good();
}

} // namespace orionimg

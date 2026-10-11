#include "nisaba/image/webp.hpp"
#include <cstring>
#include <fstream>
#include <algorithm>
#include <array>
#include <vector>
#include <memory>
#include <cmath>

namespace nisaba::image {

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

static uint8_t bit_reverse_8(uint8_t v) noexcept {
    v = static_cast<uint8_t>(((v & 0xAA) >> 1) | ((v & 0x55) << 1));
    v = static_cast<uint8_t>(((v & 0xCC) >> 2) | ((v & 0x33) << 2));
    return static_cast<uint8_t>((v >> 4) | (v << 4));
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

    std::optional<WebPChunk> vp8l_chunk;
    std::optional<WebPChunk> vp8_chunk;
    std::optional<WebPChunk> alph_chunk;
    std::optional<WebPChunk> anim_chunk;
    Color background_color{Color::TRANSPARENT};
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

    uint32_t riff_len = read_u32_le(data.data() + 4);
    (void)riff_len;

    WebPContainer container;
    size_t offset = 12;

    while (offset + 8 <= data.size()) {
        uint32_t fourcc = read_u32_le(data.data() + offset);
        uint32_t chunk_len = read_u32_le(data.data() + offset + 4);
        offset += 8;

        size_t available = (offset <= data.size()) ? (data.size() - offset) : 0;
        size_t payload_len = std::min(static_cast<size_t>(chunk_len), available);
        std::span<const uint8_t> payload(data.data() + offset, payload_len);
        WebPChunk chunk{fourcc, payload};

        if (fourcc == CHUNK_VP8X) {
            if (chunk_len < 10) {
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
        } else if (fourcc == CHUNK_VP8L) {
            container.vp8l_chunk = chunk;
        } else if (fourcc == CHUNK_VP8) {
            container.vp8_chunk = chunk;
        } else if (fourcc == CHUNK_ALPH) {
            container.alph_chunk = chunk;
            container.has_alpha = true;
        } else if (fourcc == CHUNK_ANIM) {
            container.anim_chunk = chunk;
            if (chunk_len >= 6) {
                uint8_t b = payload[0];
                uint8_t g = payload[1];
                uint8_t r = payload[2];
                uint8_t a = payload[3];
                container.background_color = Color::from_rgba8(r, g, b, a);
                container.loop_count = read_u16_le(payload.data() + 4);
            }
        } else if (fourcc == CHUNK_ANMF) {
            container.anmf_chunks.push_back(chunk);
        }

        // Chunks are 2-byte aligned
        offset += chunk_len;
        if (chunk_len & 1) {
            offset += 1;
        }
    }

    if (!container.vp8l_chunk && !container.vp8_chunk && container.anmf_chunks.empty()) {
        return ImageResult<WebPContainer>::fail(ImageError::UnsupportedFormat);
    }

    return ImageResult<WebPContainer>::success(container);
}

// ============================================================================
// 2. VP8L (Lossless WebP) Decoder
// ============================================================================

class Vp8lBitReader {
public:
    explicit Vp8lBitReader(std::span<const uint8_t> data) noexcept
        : data_(data.data()), size_(data.size()), pos_(0), bit_buf_(0), bits_count_(0) {}

    bool has_error() const noexcept { return error_; }

    void refill() noexcept {
        while (bits_count_ <= 56 && pos_ < size_) {
            bit_buf_ |= static_cast<uint64_t>(data_[pos_++]) << bits_count_;
            bits_count_ += 8;
        }
    }

    uint32_t peek_bits(int n) noexcept {
        if (n == 0) return 0;
        if (bits_count_ < n) {
            refill();
            if (bits_count_ < n) {
                error_ = true;
                return 0;
            }
        }
        return static_cast<uint32_t>(bit_buf_ & ((1ULL << n) - 1));
    }

    void consume_bits(int n) noexcept {
        bit_buf_ >>= n;
        bits_count_ -= n;
    }

    uint32_t read_bits(int n) noexcept {
        uint32_t val = peek_bits(n);
        consume_bits(n);
        return val;
    }

    uint32_t read_bit() noexcept {
        return read_bits(1);
    }

    size_t bytes_consumed() const noexcept {
        return pos_ - (bits_count_ / 8);
    }

private:
    const uint8_t* data_;
    size_t size_;
    size_t pos_;
    uint64_t bit_buf_;
    int bits_count_;
    bool error_{false};
};

// Fast and safe Huffman decoder with direct 8-bit lookup table
struct HuffmanTable {
    struct FastEntry {
        uint8_t len{0};
        uint16_t symbol{0};
    };
    std::array<FastEntry, 256> fast_lut{};
    std::vector<std::pair<uint32_t, uint16_t>> full_table;
    int single_symbol{-1};

    bool build(std::span<const uint16_t> lengths) {
        full_table.clear();
        fast_lut.fill(FastEntry{0, 0});
        single_symbol = -1;

        int count = 0;
        int last_sym = -1;
        for (size_t i = 0; i < lengths.size(); ++i) {
            if (lengths[i] > 0) {
                count++;
                last_sym = static_cast<int>(i);
            }
        }
        if (count == 0) return true;
        if (count == 1) {
            single_symbol = last_sym;
            return true;
        }

        std::array<uint32_t, 16> code_count{};
        for (auto l : lengths) {
            if (l > 0 && l <= 15) code_count[l]++;
        }
        code_count[0] = 0;

        std::array<uint32_t, 16> next_code{};
        uint32_t code = 0;
        for (int bits = 1; bits <= 15; bits++) {
            code = (code + code_count[bits - 1]) << 1;
            next_code[bits] = code;
        }

        full_table.resize(lengths.size());
        for (size_t i = 0; i < lengths.size(); ++i) {
            uint16_t len = lengths[i];
            if (len == 0) {
                full_table[i] = {0, 0};
                continue;
            }
            uint32_t c = next_code[len]++;
            uint32_t rev = 0;
            for (int b = 0; b < len; ++b) {
                if (c & (1U << b)) rev |= (1U << (len - 1 - b));
            }
            full_table[i] = {rev, len};

            // Populate fast 8-bit LUT if len <= 8
            if (len <= 8) {
                int step = 1 << len;
                for (int m = static_cast<int>(rev); m < 256; m += step) {
                    fast_lut[m] = FastEntry{static_cast<uint8_t>(len), static_cast<uint16_t>(i)};
                }
            }
        }
        return true;
    }

    uint32_t decode(Vp8lBitReader& br) const noexcept {
        if (single_symbol >= 0) return static_cast<uint32_t>(single_symbol);

        // Try fast 8-bit LUT
        uint32_t peek8 = br.peek_bits(8);
        const auto& fe = fast_lut[peek8 & 0xff];
        if (fe.len > 0) {
            br.consume_bits(fe.len);
            return fe.symbol;
        }

        // Fallback for codes > 8 bits
        uint32_t code = 0;
        for (int len = 1; len <= 15; ++len) {
            code |= (br.read_bit() << (len - 1));
            if (len > 8) {
                for (size_t i = 0; i < full_table.size(); ++i) {
                    if (full_table[i].second == len && full_table[i].first == code) {
                        return static_cast<uint32_t>(i);
                    }
                }
            }
        }
        return UINT32_MAX;
    }
};

struct HuffmanGroup {
    std::vector<uint16_t> green_lengths;
    std::vector<uint16_t> red_lengths;
    std::vector<uint16_t> blue_lengths;
    std::vector<uint16_t> alpha_lengths;
    std::vector<uint16_t> dist_lengths;

    std::array<HuffmanTable, 5> tables;

    uint32_t decode_symbol(Vp8lBitReader& br, int tree_idx) const noexcept {
        return tables[tree_idx].decode(br);
    }
};

bool read_huffman_code_lengths(Vp8lBitReader& br, int num_symbols, std::vector<uint16_t>& lengths) {
    lengths.assign(num_symbols, 0);

    bool is_simple = br.read_bit() != 0;
    if (is_simple) {
        int num_simple = br.read_bit() + 1;
        int first_symbol_len_bits = br.read_bit() ? 8 : 1;
        uint32_t sym0 = br.read_bits(first_symbol_len_bits);
        if (sym0 < lengths.size()) {
            lengths[sym0] = 1;
        }
        if (num_simple == 2) {
            uint32_t sym1 = br.read_bits(8);
            if (sym1 < lengths.size()) {
                lengths[sym1] = 1;
            }
        }
        return true;
    }

    // Normal complex Huffman tree
    constexpr int CODE_LENGTH_ORDER[19] = {
        17, 18, 0, 1, 2, 3, 4, 5, 16, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15
    };
    int num_code_lengths = 4 + br.read_bits(4);
    if (num_code_lengths > 19) num_code_lengths = 19;

    std::array<uint16_t, 19> code_length_code_lengths{};
    for (int i = 0; i < num_code_lengths; ++i) {
        code_length_code_lengths[CODE_LENGTH_ORDER[i]] = static_cast<uint16_t>(br.read_bits(3));
    }

    HuffmanTable code_length_table;
    code_length_table.build(code_length_code_lengths);

    int max_symbol = num_symbols;
    if (br.read_bit()) {
        int length_nbits = 2 + 2 * br.read_bits(3);
        max_symbol = 2 + br.read_bits(length_nbits);
        if (max_symbol > num_symbols) max_symbol = num_symbols;
    }

    int sym = 0;
    uint16_t prev_len = 8;
    while (sym < num_symbols) {
        if (sym >= max_symbol) {
            lengths[sym++] = 0;
            continue;
        }
        uint32_t code = code_length_table.decode(br);
        if (code < 16) {
            lengths[sym++] = static_cast<uint16_t>(code);
            if (code != 0) prev_len = static_cast<uint16_t>(code);
        } else if (code == 16) {
            int repeat = 3 + br.read_bits(2);
            while (repeat-- > 0 && sym < num_symbols) {
                lengths[sym++] = prev_len;
            }
        } else if (code == 17) {
            int repeat = 3 + br.read_bits(3);
            while (repeat-- > 0 && sym < num_symbols) {
                lengths[sym++] = 0;
            }
        } else if (code == 18) {
            int repeat = 11 + br.read_bits(7);
            while (repeat-- > 0 && sym < num_symbols) {
                lengths[sym++] = 0;
            }
        } else {
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// Reversible Spatial Transformations
// ----------------------------------------------------------------------------

enum TransformType {
    PREDICTOR_TRANSFORM = 0,
    COLOR_TRANSFORM = 1,
    SUBTRACT_GREEN = 2,
    COLOR_INDEXING_TRANSFORM = 3
};

struct Vp8lTransform {
    TransformType type;
    int size_bits{0};
    uint32_t width{0};
    uint32_t height{0};
    std::vector<uint32_t> data;
};

inline uint32_t average2(uint32_t a, uint32_t b) noexcept {
    return ((a ^ b) >> 1) + (a & b);
}

inline uint8_t clamp_u8(int v) noexcept {
    return static_cast<uint8_t>(std::clamp(v, 0, 255));
}

inline uint32_t select_paeth(uint32_t l, uint32_t t, uint32_t tl) noexcept {
    int la = (l >> 24) & 0xff, lr = (l >> 16) & 0xff, lg = (l >> 8) & 0xff, lb = l & 0xff;
    int ta = (t >> 24) & 0xff, tr = (t >> 16) & 0xff, tg = (t >> 8) & 0xff, tb = t & 0xff;
    int tla = (tl >> 24) & 0xff, tlr = (tl >> 16) & 0xff, tlg = (tl >> 8) & 0xff, tlb = tl & 0xff;

    auto paeth = [](int pa, int pb, int pc) -> uint8_t {
        int p = pa + pb - pc;
        int da = std::abs(p - pa);
        int db = std::abs(p - pb);
        int dc = std::abs(p - pc);
        if (da <= db && da <= dc) return static_cast<uint8_t>(pa);
        if (db <= dc) return static_cast<uint8_t>(pb);
        return static_cast<uint8_t>(pc);
    };

    return (paeth(la, ta, tla) << 24) |
           (paeth(lr, tr, tlr) << 16) |
           (paeth(lg, tg, tlg) << 8)  |
            paeth(lb, tb, tlb);
}

inline uint32_t clamp_add_subtract(uint32_t a, uint32_t b, uint32_t c) noexcept {
    auto op = [](uint8_t av, uint8_t bv, uint8_t cv) -> uint8_t {
        return clamp_u8(static_cast<int>(av) + static_cast<int>(bv) - static_cast<int>(cv));
    };
    return (op((a >> 24) & 0xff, (b >> 24) & 0xff, (c >> 24) & 0xff) << 24) |
           (op((a >> 16) & 0xff, (b >> 16) & 0xff, (c >> 16) & 0xff) << 16) |
           (op((a >> 8) & 0xff,  (b >> 8) & 0xff,  (c >> 8) & 0xff) << 8)  |
            op(a & 0xff, b & 0xff, c & 0xff);
}

inline uint32_t predict_pixel(int mode, uint32_t l, uint32_t t, uint32_t tl, uint32_t tr) noexcept {
    switch (mode) {
        case 0:  return 0xff000000;
        case 1:  return l;
        case 2:  return t;
        case 3:  return tr;
        case 4:  return tl;
        case 5:  return average2(average2(l, tr), t);
        case 6:  return average2(l, tl);
        case 7:  return average2(l, t);
        case 8:  return average2(tl, t);
        case 9:  return average2(t, tr);
        case 10: return average2(average2(l, tl), average2(t, tr));
        case 11: return select_paeth(l, t, tl);
        case 12: return clamp_add_subtract(l, t, tl);
        case 13: return clamp_add_subtract(l, average2(t, tl), tl);
        default: return 0xff000000;
    }
}

// Full VP8L Decoder Execution
ImageResult<Pixmap> decode_vp8l_internal(std::span<const uint8_t> data) {
    if (data.size() < 5 || data[0] != 0x2F) {
        return ImageResult<Pixmap>::fail(ImageError::InvalidSignature);
    }

    uint32_t width  = 1 + (data[1] | ((data[2] & 0x3F) << 8));
    uint32_t height = 1 + ((data[2] >> 6) | (data[3] << 2) | ((data[4] & 0x0F) << 10));
    bool has_alpha  = (data[4] & 0x10) != 0;
    uint32_t version = (data[4] >> 5) & 0x07;

    if (version != 0 || width == 0 || height == 0) {
        return ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
    }

    Vp8lBitReader br(data.subspan(5));

    // 1. Read Transforms
    std::vector<Vp8lTransform> transforms;
    while (br.read_bit()) {
        Vp8lTransform t;
        int t_type = br.read_bits(2);
        t.type = static_cast<TransformType>(t_type);
        if (t.type == PREDICTOR_TRANSFORM || t.type == COLOR_TRANSFORM) {
            t.size_bits = br.read_bits(3) + 2;
            int block_sz = 1 << t.size_bits;
            t.width = (width + block_sz - 1) >> t.size_bits;
            t.height = (height + block_sz - 1) >> t.size_bits;
            t.data.resize(t.width * t.height);
            for (size_t i = 0; i < t.data.size(); ++i) {
                t.data[i] = (br.read_bits(8) << 8); // Mode in green channel
            }
        } else if (t.type == COLOR_INDEXING_TRANSFORM) {
            int num_colors = br.read_bits(8) + 1;
            t.data.resize(num_colors);
            for (int i = 0; i < num_colors; ++i) {
                t.data[i] = br.read_bits(32);
            }
            if (num_colors <= 2) t.size_bits = 3;
            else if (num_colors <= 4) t.size_bits = 2;
            else if (num_colors <= 16) t.size_bits = 1;
            else t.size_bits = 0;
        }
        transforms.push_back(std::move(t));
    }

    // 2. Read Color Cache & Huffman Code Groups
    int color_cache_bits = 0;
    if (br.read_bit()) {
        color_cache_bits = br.read_bits(4);
    }

    int num_entropy_groups = 1;
    if (br.read_bit()) {
        int meta_bits = br.read_bits(3) + 2;
        int meta_w = (width + (1 << meta_bits) - 1) >> meta_bits;
        int meta_h = (height + (1 << meta_bits) - 1) >> meta_bits;
        for (int i = 0; i < meta_w * meta_h; ++i) {
            int g = br.read_bits(16);
            if (g >= num_entropy_groups) num_entropy_groups = g + 1;
        }
    }

    std::vector<HuffmanGroup> huffman_groups(num_entropy_groups);
    for (int g = 0; g < num_entropy_groups; ++g) {
        auto& hg = huffman_groups[g];
        int num_green = 256 + 24 + (color_cache_bits > 0 ? (1 << color_cache_bits) : 0);
        read_huffman_code_lengths(br, num_green, hg.green_lengths);
        read_huffman_code_lengths(br, 256, hg.red_lengths);
        read_huffman_code_lengths(br, 256, hg.blue_lengths);
        read_huffman_code_lengths(br, 256, hg.alpha_lengths);
        read_huffman_code_lengths(br, 40, hg.dist_lengths);

        hg.tables[0].build(hg.green_lengths);
        hg.tables[1].build(hg.red_lengths);
        hg.tables[2].build(hg.blue_lengths);
        hg.tables[3].build(hg.alpha_lengths);
        hg.tables[4].build(hg.dist_lengths);
    }

    // 3. Decode Image Data Stream
    auto maybe_pixmap = Pixmap::allocate(width, height);
    if (!maybe_pixmap.has_value()) {
        return ImageResult<Pixmap>::fail(ImageError::OutOfMemory);
    }
    Pixmap& pixmap = *maybe_pixmap;

    std::vector<uint32_t> pixels(width * height, 0xff000000);
    std::vector<uint32_t> color_cache(color_cache_bits > 0 ? (1 << color_cache_bits) : 0, 0);

    size_t pixel_idx = 0;
    size_t total_pixels = width * height;

    constexpr int LENGTH_PREFIX_EXTRA_BITS[24] = {
        0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 0, 0
    };
    constexpr int LENGTH_PREFIX_OFFSETS[24] = {
        1, 2, 3, 4, 5, 6, 7, 9, 11, 15, 19, 27, 35, 51, 67, 99, 131, 195, 259, 387, 515, 771, 1027, 1539
    };
    constexpr int DISTANCE_PREFIX_EXTRA_BITS[40] = {
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15, 16, 16, 17, 17, 18, 18
    };
    constexpr int DISTANCE_PREFIX_OFFSETS[40] = {
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 32769, 49153, 65537, 98305, 131073, 196609, 262145, 393217, 524289, 786433
    };

    while (pixel_idx < total_pixels && !br.has_error()) {
        const auto& hg = huffman_groups[0];
        uint32_t green_sym = hg.decode_symbol(br, 0);

        if (green_sym < 256) {
            uint32_t red   = hg.decode_symbol(br, 1);
            uint32_t blue  = hg.decode_symbol(br, 2);
            uint32_t alpha = hg.decode_symbol(br, 3);
            uint32_t argb  = (alpha << 24) | (red << 16) | (green_sym << 8) | blue;
            pixels[pixel_idx++] = argb;

            if (color_cache_bits > 0) {
                uint32_t key = (0x1e35a7bdU * argb) >> (32 - color_cache_bits);
                color_cache[key] = argb;
            }
        } else if (green_sym < 256 + 24) {
            int len_code = green_sym - 256;
            int length = LENGTH_PREFIX_OFFSETS[len_code] + br.read_bits(LENGTH_PREFIX_EXTRA_BITS[len_code]);
            int dist_code = hg.decode_symbol(br, 4);
            int dist_prefix = dist_code < 40 ? dist_code : 39;
            int distance = DISTANCE_PREFIX_OFFSETS[dist_prefix] + br.read_bits(DISTANCE_PREFIX_EXTRA_BITS[dist_prefix]);

            while (length-- > 0 && pixel_idx < total_pixels) {
                size_t src_idx = (pixel_idx >= static_cast<size_t>(distance)) ? (pixel_idx - distance) : 0;
                uint32_t argb = pixels[src_idx];
                pixels[pixel_idx++] = argb;
                if (color_cache_bits > 0) {
                    uint32_t key = (0x1e35a7bdU * argb) >> (32 - color_cache_bits);
                    color_cache[key] = argb;
                }
            }
        } else if (color_cache_bits > 0 && green_sym < 256 + 24 + (1U << color_cache_bits)) {
            int cache_idx = green_sym - (256 + 24);
            pixels[pixel_idx++] = color_cache[cache_idx];
        }
    }

    // 4. Invert Transforms (in reverse order)
    for (auto it = transforms.rbegin(); it != transforms.rend(); ++it) {
        if (it->type == SUBTRACT_GREEN) {
            for (size_t i = 0; i < pixels.size(); ++i) {
                uint8_t a = (pixels[i] >> 24) & 0xff;
                uint8_t r = (pixels[i] >> 16) & 0xff;
                uint8_t g = (pixels[i] >> 8)  & 0xff;
                uint8_t b = pixels[i] & 0xff;
                r = (r + g) & 0xff;
                b = (b + g) & 0xff;
                pixels[i] = (a << 24) | (r << 16) | (g << 8) | b;
            }
        } else if (it->type == PREDICTOR_TRANSFORM) {
            for (uint32_t y = 0; y < height; ++y) {
                for (uint32_t x = 0; x < width; ++x) {
                    size_t idx = y * width + x;
                    uint32_t t_x = x >> it->size_bits;
                    uint32_t t_y = y >> it->size_bits;
                    int mode = (it->data[t_y * it->width + t_x] >> 8) & 0x0f;

                    uint32_t l  = (x > 0) ? pixels[idx - 1] : 0xff000000;
                    uint32_t t  = (y > 0) ? pixels[idx - width] : 0xff000000;
                    uint32_t tl = (x > 0 && y > 0) ? pixels[idx - width - 1] : 0xff000000;
                    uint32_t tr = (x + 1 < width && y > 0) ? pixels[idx - width + 1] : t;

                    uint32_t pred = predict_pixel(mode, l, t, tl, tr);

                    uint8_t a = ((pixels[idx] >> 24) + (pred >> 24)) & 0xff;
                    uint8_t r = ((pixels[idx] >> 16) + (pred >> 16)) & 0xff;
                    uint8_t g = ((pixels[idx] >> 8)  + (pred >> 8))  & 0xff;
                    uint8_t b = (pixels[idx] + pred) & 0xff;
                    pixels[idx] = (a << 24) | (r << 16) | (g << 8) | b;
                }
            }
        } else if (it->type == COLOR_INDEXING_TRANSFORM) {
            for (size_t i = 0; i < pixels.size(); ++i) {
                uint8_t idx = (pixels[i] >> 8) & 0xff;
                if (idx < it->data.size()) {
                    pixels[i] = it->data[idx];
                }
            }
        }
    }

    // 5. Transfer to Pixmap RGBA8
    PremultipliedColorU8* dst = pixmap.pixels_mut();
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            uint32_t argb = pixels[y * width + x];
            uint8_t a = (argb >> 24) & 0xff;
            uint8_t r = (argb >> 16) & 0xff;
            uint8_t g = (argb >> 8)  & 0xff;
            uint8_t b = argb & 0xff;
            uint8_t final_a = has_alpha ? a : 255;
            dst[y * width + x] = PremultipliedColorU8::from_rgba_unchecked(r, g, b, final_a);
        }
    }

    return ImageResult<Pixmap>::success(std::move(pixmap));
}

// ============================================================================
// 3. Lossy WebP (VP8 Keyframe) Decoder
// ============================================================================

ImageResult<Pixmap> decode_vp8_internal(std::span<const uint8_t> /*data*/, std::span<const uint8_t> /*alpha_data*/) {
    // Sovereign Modern C++20 VP8 Intra-Decoder is pending native ground-up implementation
    return ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
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

    auto maybe_canvas = Pixmap::allocate(canvas_w, canvas_h);
    if (!maybe_canvas.has_value()) {
        return ImageResult<WebPAnimation>::fail(ImageError::OutOfMemory);
    }
    Pixmap canvas = std::move(*maybe_canvas);

    PremultipliedColorU8 bg_color = container.background_color.to_color_u8().premultiply();
    PremultipliedColorU8* canvas_pixels = canvas.pixels_mut();
    for (size_t i = 0; i < canvas_w * canvas_h; ++i) {
        canvas_pixels[i] = bg_color;
    }

    uint32_t prev_x = 0;
    uint32_t prev_y = 0;
    uint32_t prev_w = 0;
    uint32_t prev_h = 0;
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

        // Parse sub-chunks inside ANMF payload
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

        ImageResult<Pixmap> sub_res = ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
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
        const Pixmap& sub_pixmap = *sub_res;

        // 1. Handle previous frame's disposal
        if (prev_dispose_bg) {
            for (uint32_t py = 0; py < prev_h && (prev_y + py) < canvas_h; ++py) {
                for (uint32_t px = 0; px < prev_w && (prev_x + px) < canvas_w; ++px) {
                    canvas_pixels[(prev_y + py) * canvas_w + (prev_x + px)] = bg_color;
                }
            }
        }

        // 2. Composite sub-pixmap onto canvas
        const PremultipliedColorU8* sub_pixels = sub_pixmap.pixels();
        for (uint32_t py = 0; py < frame_h && py < sub_pixmap.height() && (frame_y + py) < canvas_h; ++py) {
            for (uint32_t px = 0; px < frame_w && px < sub_pixmap.width() && (frame_x + px) < canvas_w; ++px) {
                size_t dst_idx = (frame_y + py) * canvas_w + (frame_x + px);
                PremultipliedColorU8 src = sub_pixels[py * sub_pixmap.width() + px];

                if (no_blend || src.a == 255) {
                    canvas_pixels[dst_idx] = src;
                } else if (src.a == 0) {
                    // Transparent: leave dst unchanged
                } else {
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

        // 3. Snapshot composited canvas for this frame
        WebPAnimationFrame anim_frame;
        anim_frame.pixmap = canvas;
        anim_frame.duration_ms = duration;
        anim_frame.x = frame_x;
        anim_frame.y = frame_y;
        anim_frame.width = frame_w;
        anim_frame.height = frame_h;
        anim_frame.blend_method = no_blend ? WebPBlendMethod::Overwrite : WebPBlendMethod::AlphaBlend;
        anim_frame.dispose_method = dispose_bg ? WebPDisposeMethod::Background : WebPDisposeMethod::None;
        anim.frames.push_back(std::move(anim_frame));

        // 4. Update previous frame disposal state
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

ImageResult<Pixmap> decode_webp(std::span<const uint8_t> webp_data) {
    auto cont_res = parse_webp_container(webp_data);
    if (!cont_res.has_value()) {
        return ImageResult<Pixmap>::fail(cont_res.error);
    }

    const auto& container = *cont_res;

    // 0. Animated WebP: decode and return the first composited frame
    if (container.has_animation && !container.anmf_chunks.empty()) {
        auto anim_res = decode_webp_animation(webp_data);
        if (anim_res.has_value() && !anim_res->frames.empty()) {
            return ImageResult<Pixmap>::success(std::move(anim_res->frames[0].pixmap));
        }
    }

    // 1. Lossless WebP (VP8L)
    if (container.vp8l_chunk.has_value()) {
        return decode_vp8l_internal(container.vp8l_chunk->payload);
    }

    // 2. Lossy WebP (VP8)
    if (container.vp8_chunk.has_value()) {
        std::span<const uint8_t> alpha_payload;
        if (container.alph_chunk.has_value()) {
            alpha_payload = container.alph_chunk->payload;
        }
        return decode_vp8_internal(container.vp8_chunk->payload, alpha_payload);
    }

    return ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
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

    if (container.has_animation && !container.anmf_chunks.empty() && container.canvas_width > 0 && container.canvas_height > 0) {
        info.width = container.canvas_width;
        info.height = container.canvas_height;
        info.has_alpha = container.has_alpha;
        info.channels = info.has_alpha ? 4 : 3;
        return ImageResult<ImageInfo>::success(info);
    }

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
    PixmapRef pixmap,
    bool lossless,
    int quality
) {
    (void)lossless;
    (void)quality;
    if (pixmap.is_empty()) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    Pixmap temp_converted;
    if (pixmap.format() != PixelFormat::RGBA8888) {
        temp_converted = pixmap.to_rgba8888();
        pixmap = temp_converted.as_ref();
    }

    uint32_t width = pixmap.width();
    uint32_t height = pixmap.height();
    const PremultipliedColorU8* px_data = pixmap.pixels();

    // Check if alpha channel is used and whether channels are uniform
    bool has_alpha = false;
    int const_g = -1;
    int const_r = -1;
    int const_b = -1;
    int const_a = -1;

    if (width > 0 && height > 0) {
        const_g = px_data[0].g;
        const_r = px_data[0].r;
        const_b = px_data[0].b;
        const_a = px_data[0].a;

        for (uint32_t i = 0; i < width * height; ++i) {
            auto px = px_data[i];
            if (px.a != 255) has_alpha = true;
            if (const_g != px.g) const_g = -1;
            if (const_r != px.r) const_r = -1;
            if (const_b != px.b) const_b = -1;
            if (const_a != px.a) const_a = -1;
        }
    }

    if (!has_alpha) {
        const_a = 255;
    }

    // Sovereign Lossless WebP (VP8L) Canonical Bitstream Builder
    std::vector<uint8_t> vp8l_stream;
    vp8l_stream.push_back(0x2F); // VP8L Signature

    // 14-bit width - 1, 14-bit height - 1, alpha flag, version = 0
    uint32_t w_val = width - 1;
    uint32_t h_val = height - 1;

    vp8l_stream.push_back(static_cast<uint8_t>(w_val & 0xff));
    vp8l_stream.push_back(static_cast<uint8_t>(((w_val >> 8) & 0x3f) | ((h_val & 0x03) << 6)));
    vp8l_stream.push_back(static_cast<uint8_t>((h_val >> 2) & 0xff));
    vp8l_stream.push_back(static_cast<uint8_t>(((h_val >> 10) & 0x0f) | (has_alpha ? 0x10 : 0x00)));

    // Bitstream payload builder (LSB first)
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

    Vp8lBitWriter bw;

    // Transform bit = 0 (No spatial transforms)
    bw.write_bits(0, 1);

    // Color cache bit = 0
    bw.write_bits(0, 1);

    // Meta Huffman bit = 0
    bw.write_bits(0, 1);

    // Helper to write canonical Huffman tree (Green: 280, Red: 256, Blue: 256, Alpha: 256, Distance: 40)
    auto write_huffman_tree = [&](int num_symbols, int single_val) {
        (void)num_symbols;
        if (single_val >= 0) {
            // Simple 1-symbol tree
            bw.write_bits(1, 1); // is_simple = 1
            bw.write_bits(0, 1); // 1 symbol
            if (single_val <= 1) {
                bw.write_bits(0, 1); // 1-bit length indicator
                bw.write_bits(static_cast<uint32_t>(single_val), 1);
            } else {
                bw.write_bits(1, 1); // 8-bit length indicator
                bw.write_bits(static_cast<uint32_t>(single_val), 8);
            }
        } else {
            // Complex canonical tree: all 256 symbols have length 8
            bw.write_bits(0, 1); // is_simple = 0
            bw.write_bits(12 - 4, 4); // num_code_lengths = 12

            // CODE_LENGTH_ORDER: 17, 18, 0, 1, 2, 3, 4, 5, 16, 6, 7, 8
            // Index 11 is symbol 8. Indices 0..10 have length 0.
            for (int i = 0; i < 11; ++i) {
                bw.write_bits(0, 3);
            }
            bw.write_bits(1, 3); // Symbol 8 has length 1

            // max_symbol = 256
            bw.write_bits(1, 1); // has max_symbol
            bw.write_bits(3, 3); // length_nbits = 2 + 2*3 = 8
            bw.write_bits(254, 8); // max_symbol = 2 + 254 = 256
            // Note: code_length_table has only 1 symbol (8), so decode consumes 0 bits!
        }
    };

    // Write 5 Huffman Trees
    write_huffman_tree(280, const_g); // Tree 0: Green
    write_huffman_tree(256, const_r); // Tree 1: Red
    write_huffman_tree(256, const_b); // Tree 2: Blue
    write_huffman_tree(256, const_a); // Tree 3: Alpha
    write_huffman_tree(40,  0);       // Tree 4: Distance (simple 1-symbol tree for 0)

    // Write pixel symbols
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            auto px = px_data[y * width + x];
            if (const_g < 0) {
                bw.write_bits(bit_reverse_8(px.g), 8);
            }
            if (const_r < 0) {
                bw.write_bits(bit_reverse_8(px.r), 8);
            }
            if (const_b < 0) {
                bw.write_bits(bit_reverse_8(px.b), 8);
            }
            if (const_a < 0) {
                bw.write_bits(bit_reverse_8(px.a), 8);
            }
        }
    }
    bw.flush();

    vp8l_stream.insert(vp8l_stream.end(), bw.bytes().begin(), bw.bytes().end());

    // Assemble RIFF / WEBP Container
    uint32_t vp8l_size = static_cast<uint32_t>(vp8l_stream.size());
    uint32_t padded_vp8l_size = vp8l_size + (vp8l_size & 1);
    uint32_t riff_payload_size = 4 + 8 + padded_vp8l_size; // "WEBP" + "VP8L" + len + payload

    std::vector<uint8_t> file_out;
    file_out.reserve(8 + riff_payload_size);

    // RIFF Header
    file_out.push_back('R'); file_out.push_back('I'); file_out.push_back('F'); file_out.push_back('F');
    uint8_t u32_buf[4];
    write_u32_le(u32_buf, riff_payload_size);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);

    // WEBP FourCC
    file_out.push_back('W'); file_out.push_back('E'); file_out.push_back('B'); file_out.push_back('P');

    // VP8L Chunk
    file_out.push_back('V'); file_out.push_back('P'); file_out.push_back('8'); file_out.push_back('L');
    write_u32_le(u32_buf, vp8l_size);
    file_out.insert(file_out.end(), u32_buf, u32_buf + 4);
    file_out.insert(file_out.end(), vp8l_stream.begin(), vp8l_stream.end());

    if (vp8l_size & 1) {
        file_out.push_back(0); // Pad byte
    }

    return ImageResult<std::vector<uint8_t>>::success(std::move(file_out));
}

ImageResult<Pixmap> decode_webp_file(const std::string& file_path) {
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
    return decode_webp(buffer);
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
    PixmapRef pixmap,
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

} // namespace nisaba::image

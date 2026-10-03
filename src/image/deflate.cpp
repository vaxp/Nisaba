#include "nisaba/image/deflate.hpp"
#include <array>
#include <cstring>
#include <algorithm>
#include <limits>

namespace nisaba::image {

// ============================================================================
// 1. CRC-32 (ISO 3309)
// ============================================================================

namespace {

constexpr std::array<uint32_t, 256> make_crc32_table() {
    std::array<uint32_t, 256> table{};
    for (uint32_t i = 0; i < 256; ++i) {
        uint32_t c = i;
        for (int k = 0; k < 8; ++k) {
            c = (c & 1) ? (0xEDB88320U ^ (c >> 1)) : (c >> 1);
        }
        table[i] = c;
    }
    return table;
}

inline constexpr auto CRC32_TABLE = make_crc32_table();

} // namespace

uint32_t crc32(std::span<const uint8_t> data, uint32_t prev_crc) noexcept {
    uint32_t c = prev_crc ^ 0xFFFFFFFFU;
    for (uint8_t byte : data) {
        c = CRC32_TABLE[(c ^ byte) & 0xFF] ^ (c >> 8);
    }
    return c ^ 0xFFFFFFFFU;
}

// ============================================================================
// 2. Adler-32 (RFC 1950)
// ============================================================================

uint32_t adler32(std::span<const uint8_t> data, uint32_t prev_adler) noexcept {
    constexpr uint32_t BASE = 65521U;
    constexpr size_t NMAX = 5552;

    uint32_t s1 = prev_adler & 0xFFFFU;
    uint32_t s2 = (prev_adler >> 16) & 0xFFFFU;

    const uint8_t* ptr = data.data();
    size_t len = data.size();

    while (len > 0) {
        size_t k = std::min(len, NMAX);
        len -= k;

        // Unroll loop for speed
        while (k >= 16) {
            for (int i = 0; i < 16; ++i) {
                s1 += *ptr++;
                s2 += s1;
            }
            k -= 16;
        }
        while (k > 0) {
            s1 += *ptr++;
            s2 += s1;
            --k;
        }
        s1 %= BASE;
        s2 %= BASE;
    }

    return (s2 << 16) | s1;
}

// ============================================================================
// 3. Fast BitReader
// ============================================================================

namespace {

class BitReader {
public:
    explicit BitReader(std::span<const uint8_t> data) noexcept
        : ptr_(data.data()), end_(data.data() + data.size()) {
        refill();
    }

    bool has_error() const noexcept { return error_; }
    void set_error() noexcept { error_ = true; }

    int bit_count() const noexcept { return bit_count_; }

    inline void refill() noexcept {
        // Fast path: load 4 bytes if bit_count_ <= 32 and enough data remains
        if (ptr_ + 4 <= end_ && bit_count_ <= 32) {
            uint32_t chunk;
            std::memcpy(&chunk, ptr_, sizeof(chunk));
            bit_buf_ |= (static_cast<uint64_t>(chunk) << bit_count_);
            ptr_ += 4;
            bit_count_ += 32;
        }
        while (bit_count_ <= 56 && ptr_ < end_) {
            bit_buf_ |= (static_cast<uint64_t>(*ptr_++) << bit_count_);
            bit_count_ += 8;
        }
    }

    inline uint32_t peek_bits_fast(int n) const noexcept {
        return static_cast<uint32_t>(bit_buf_ & ((1ULL << n) - 1));
    }

    inline uint32_t peek_bits(int n) noexcept {
        if (bit_count_ < n) {
            refill();
        }
        return peek_bits_fast(n);
    }

    inline void consume_bits(int n) noexcept {
        bit_buf_ >>= n;
        bit_count_ -= n;
    }

    inline uint32_t read_bits(int n) noexcept {
        if (bit_count_ < n) {
            refill();
            if (bit_count_ < n) {
                error_ = true;
                return 0;
            }
        }
        uint32_t val = peek_bits_fast(n);
        consume_bits(n);
        return val;
    }

    void align_to_byte() noexcept {
        int drop = bit_count_ % 8;
        bit_buf_ >>= drop;
        bit_count_ -= drop;
    }

    const uint8_t* current_byte_ptr() noexcept {
        align_to_byte();
        int bytes_buffered = bit_count_ / 8;
        ptr_ -= bytes_buffered;
        bit_buf_ = 0;
        bit_count_ = 0;
        return ptr_;
    }

    size_t remaining_bytes() const noexcept {
        return (ptr_ < end_) ? static_cast<size_t>(end_ - ptr_) : 0;
    }

    void advance_bytes(size_t n) noexcept {
        ptr_ += n;
    }

private:
    const uint8_t* ptr_{nullptr};
    const uint8_t* end_{nullptr};
    uint64_t bit_buf_{0};
    int bit_count_{0};
    bool error_{false};
};

// Reverse n bits of value x
inline constexpr uint32_t reverse_bits(uint32_t x, int n) noexcept {
    uint32_t res = 0;
    for (int i = 0; i < n; ++i) {
        res = (res << 1) | ((x >> i) & 1);
    }
    return res;
}

// ============================================================================
// 4. Fast Huffman Decoder Table (Canonical O(1) Lookup)
// ============================================================================

constexpr int FAST_BITS = 10;
constexpr size_t FAST_SIZE = 1 << FAST_BITS;

struct FastHuffman {
    uint16_t fast[FAST_SIZE]{};
    uint16_t firstcode[16]{};
    uint16_t firstsymbol[16]{};
    int maxcode[17]{};
    uint16_t value[288]{};
    uint8_t size[288]{};

    bool build(std::span<const uint8_t> lengths) {
        int num = static_cast<int>(lengths.size());
        if (num > 288) return false;

        std::memset(fast, 0, sizeof(fast));
        std::memset(firstcode, 0, sizeof(firstcode));
        std::memset(firstsymbol, 0, sizeof(firstsymbol));
        std::memset(maxcode, 0, sizeof(maxcode));
        std::memset(value, 0, sizeof(value));
        std::memset(size, 0, sizeof(size));

        int sizes[17]{};
        for (int i = 0; i < num; ++i) {
            if (lengths[i] > 15) return false;
            sizes[lengths[i]]++;
        }
        sizes[0] = 0;
        for (int i = 1; i < 16; ++i) {
            if (sizes[i] > (1 << i)) return false;
        }

        int code = 0;
        int k = 0;
        int next_code[16]{};
        for (int i = 1; i < 16; ++i) {
            next_code[i] = code;
            firstcode[i] = static_cast<uint16_t>(code);
            firstsymbol[i] = static_cast<uint16_t>(k);
            code = (code + sizes[i]);
            if (sizes[i] && (code - 1 >= (1 << i))) return false;
            maxcode[i] = code << (16 - i); // preshift for inner loop
            code <<= 1;
            k += sizes[i];
        }
        maxcode[16] = 0x10000; // sentinel

        for (int i = 0; i < num; ++i) {
            int s = lengths[i];
            if (s) {
                int c = next_code[s] - firstcode[s] + firstsymbol[s];
                value[c] = static_cast<uint16_t>(i);
                size[c] = static_cast<uint8_t>(s);
                if (s <= FAST_BITS) {
                    int j = reverse_bits(next_code[s], s);
                    while (j < static_cast<int>(FAST_SIZE)) {
                        fast[j] = static_cast<uint16_t>((s << 9) | i);
                        j += (1 << s);
                    }
                }
                next_code[s]++;
            }
        }
        return true;
    }

    int decode(BitReader& reader) const noexcept {
        if (reader.bit_count() < 16) {
            reader.refill();
        }
        uint32_t bits = reader.peek_bits_fast(FAST_BITS);
        uint16_t entry = fast[bits];
        if (entry) {
            int s = entry >> 9;
            reader.consume_bits(s);
            return entry & 511;
        }

        // Slow path for codes > FAST_BITS
        uint32_t k = reverse_bits(reader.peek_bits_fast(16), 16);
        int s = FAST_BITS + 1;
        while (k >= static_cast<uint32_t>(maxcode[s])) {
            ++s;
        }
        if (s == 16) {
            reader.set_error();
            return -1;
        }
        int b = (k >> (16 - s)) - firstcode[s] + firstsymbol[s];
        reader.consume_bits(s);
        return value[b];
    }
};

// ============================================================================
// 5. Fixed Huffman Tables Initialization
// ============================================================================

struct FixedTables {
    FastHuffman lit_len;
    FastHuffman dist;

    FixedTables() {
        std::array<uint8_t, 288> lit_lengths{};
        for (int i = 0; i <= 143; ++i) lit_lengths[i] = 8;
        for (int i = 144; i <= 255; ++i) lit_lengths[i] = 9;
        for (int i = 256; i <= 279; ++i) lit_lengths[i] = 7;
        for (int i = 280; i <= 287; ++i) lit_lengths[i] = 8;
        lit_len.build(lit_lengths);

        std::array<uint8_t, 32> dist_lengths{};
        dist_lengths.fill(5);
        dist.build(dist_lengths);
    }
};

const FixedTables& get_fixed_tables() {
    static const FixedTables tables;
    return tables;
}

// RFC 1951 Base Lengths and Extra Bits
constexpr std::array<uint16_t, 29> LENGTH_BASE = {
    3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};
constexpr std::array<uint8_t, 29> LENGTH_EXTRA = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

// RFC 1951 Base Distances and Extra Bits
constexpr std::array<uint16_t, 30> DIST_BASE = {
    1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
    8193, 12289, 16385, 24577
};
constexpr std::array<uint8_t, 30> DIST_EXTRA = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

// RFC 1951 Code Length Order
constexpr std::array<uint8_t, 19> CODE_LEN_ORDER = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

} // namespace

// ============================================================================
// 6. Deflate Decompress (RFC 1951)
// ============================================================================

ImageResult<std::vector<uint8_t>> deflate_decompress(
    std::span<const uint8_t> deflate_data,
    size_t expected_size
) {
    BitReader reader(deflate_data);
    std::vector<uint8_t> out;
    size_t capacity = (expected_size > 0) ? (expected_size + 4096) : 65536;
    out.resize(capacity);
    uint8_t* out_start = out.data();
    uint8_t* zout = out_start;
    uint8_t* zout_end = out_start + capacity;

    auto ensure_space = [&](size_t needed) {
        size_t current_len = static_cast<size_t>(zout - out_start);
        if (current_len + needed > capacity) {
            size_t new_cap = std::max(capacity * 2, current_len + needed + 4096);
            out.resize(new_cap);
            out_start = out.data();
            zout = out_start + current_len;
            capacity = new_cap;
            zout_end = out_start + capacity;
        }
    };

    bool bfinal = false;
    while (!bfinal) {
        bfinal = (reader.read_bits(1) != 0);
        uint32_t btype = reader.read_bits(2);

        if (reader.has_error()) {
            return ImageResult<std::vector<uint8_t>>::fail(ImageError::PrematureEndOfStream);
        }

        if (btype == 0) {
            // Uncompressed block
            reader.align_to_byte();
            const uint8_t* p = reader.current_byte_ptr();
            if (reader.remaining_bytes() < 4) {
                return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
            }
            uint16_t len = static_cast<uint16_t>(p[0] | (p[1] << 8));
            uint16_t nlen = static_cast<uint16_t>(p[2] | (p[3] << 8));
            reader.advance_bytes(4);

            if (static_cast<uint16_t>(len ^ 0xFFFFU) != nlen) {
                return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
            }
            if (reader.remaining_bytes() < len) {
                return ImageResult<std::vector<uint8_t>>::fail(ImageError::PrematureEndOfStream);
            }

            p = reader.current_byte_ptr();
            ensure_space(len + 300);
            std::memcpy(zout, p, len);
            zout += len;
            reader.advance_bytes(len);
        } else if (btype == 1 || btype == 2) {
            // Huffman block
            const FastHuffman* lit_len_tree = nullptr;
            const FastHuffman* dist_tree = nullptr;
            FastHuffman dyn_lit_len;
            FastHuffman dyn_dist;

            if (btype == 1) {
                // Fixed Huffman
                const auto& fixed = get_fixed_tables();
                lit_len_tree = &fixed.lit_len;
                dist_tree = &fixed.dist;
            } else {
                // Dynamic Huffman
                uint32_t hlit = reader.read_bits(5) + 257;
                uint32_t hdist = reader.read_bits(5) + 1;
                uint32_t hclen = reader.read_bits(4) + 4;

                if (hlit > 286 || hdist > 30) {
                    return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                }

                std::array<uint8_t, 19> code_lengths{};
                for (uint32_t i = 0; i < hclen; ++i) {
                    code_lengths[CODE_LEN_ORDER[i]] = static_cast<uint8_t>(reader.read_bits(3));
                }

                FastHuffman clen_tree;
                if (!clen_tree.build(code_lengths)) {
                    return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                }

                std::vector<uint8_t> all_lengths(hlit + hdist, 0);
                size_t idx = 0;
                while (idx < hlit + hdist) {
                    int sym = clen_tree.decode(reader);
                    if (sym < 0) {
                        return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                    }
                    if (sym < 16) {
                        all_lengths[idx++] = static_cast<uint8_t>(sym);
                    } else if (sym == 16) {
                        if (idx == 0) return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                        uint8_t prev = all_lengths[idx - 1];
                        uint32_t repeat = 3 + reader.read_bits(2);
                        if (idx + repeat > hlit + hdist) return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                        for (uint32_t r = 0; r < repeat; ++r) all_lengths[idx++] = prev;
                    } else if (sym == 17) {
                        uint32_t repeat = 3 + reader.read_bits(3);
                        if (idx + repeat > hlit + hdist) return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                        idx += repeat;
                    } else if (sym == 18) {
                        uint32_t repeat = 11 + reader.read_bits(7);
                        if (idx + repeat > hlit + hdist) return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                        idx += repeat;
                    }
                }

                if (!dyn_lit_len.build(std::span<const uint8_t>(all_lengths.data(), hlit))) {
                    return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                }
                if (!dyn_dist.build(std::span<const uint8_t>(all_lengths.data() + hlit, hdist))) {
                    return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                }

                lit_len_tree = &dyn_lit_len;
                dist_tree = &dyn_dist;
            }

            // LZ77 Decode Loop
            while (true) {
                if (zout + 300 > zout_end) {
                    ensure_space(300);
                }

                int sym = lit_len_tree->decode(reader);
                if (sym < 0 || reader.has_error()) {
                    return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                }
                if (sym < 256) {
                    *zout++ = static_cast<uint8_t>(sym);
                } else if (sym == 256) {
                    // End of block
                    break;
                } else if (sym <= 285) {
                    uint32_t len_idx = sym - 257;
                    uint32_t length = LENGTH_BASE[len_idx];
                    uint8_t extra_len_bits = LENGTH_EXTRA[len_idx];
                    if (extra_len_bits > 0) {
                        length += reader.read_bits(extra_len_bits);
                    }

                    int dist_sym = dist_tree->decode(reader);
                    if (dist_sym < 0 || dist_sym >= 30) {
                        return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                    }

                    uint32_t distance = DIST_BASE[dist_sym];
                    uint8_t extra_dist_bits = DIST_EXTRA[dist_sym];
                    if (extra_dist_bits > 0) {
                        distance += reader.read_bits(extra_dist_bits);
                    }

                    size_t current_len = static_cast<size_t>(zout - out_start);
                    if (distance == 0 || distance > current_len) {
                        return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                    }

                    const uint8_t* p = zout - distance;
                    if (distance == 1) {
                        std::memset(zout, *p, length);
                        zout += length;
                    } else if (distance >= 8 && length <= 8) {
                        uint64_t chunk;
                        std::memcpy(&chunk, p, 8);
                        std::memcpy(zout, &chunk, 8);
                        zout += length;
                    } else if (distance >= length) {
                        std::memcpy(zout, p, length);
                        zout += length;
                    } else {
                        do {
                            *zout++ = *p++;
                        } while (--length);
                    }
                } else {
                    return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
                }
            }
        } else {
            // Reserved / error
            return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
        }
    }

    out.resize(static_cast<size_t>(zout - out_start));
    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

// ============================================================================
// 7. Zlib Decompress (RFC 1950)
// ============================================================================

ImageResult<std::vector<uint8_t>> zlib_decompress(
    std::span<const uint8_t> compressed_data,
    size_t expected_size,
    bool verify_checksum
) {
    if (compressed_data.size() < 6) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::PrematureEndOfStream);
    }

    uint8_t cmf = compressed_data[0];
    uint8_t flg = compressed_data[1];

    if ((cmf * 256 + flg) % 31 != 0) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidSignature);
    }

    uint8_t cm = cmf & 0x0F;
    if (cm != 8) { // Only DEFLATE supported
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::UnsupportedFeature);
    }

    size_t offset = 2;
    if (flg & 0x20) { // FDICT
        offset += 4;
        if (compressed_data.size() < offset + 4) {
            return ImageResult<std::vector<uint8_t>>::fail(ImageError::PrematureEndOfStream);
        }
    }

    size_t deflate_len = compressed_data.size() - offset - 4;
    auto deflate_span = compressed_data.subspan(offset, deflate_len);

    auto result = deflate_decompress(deflate_span, expected_size);
    if (!result) {
        return result;
    }

    // Verify Adler-32 only if requested
    if (verify_checksum) {
        size_t adler_offset = compressed_data.size() - 4;
        uint32_t stream_adler = (static_cast<uint32_t>(compressed_data[adler_offset + 0]) << 24) |
                                (static_cast<uint32_t>(compressed_data[adler_offset + 1]) << 16) |
                                (static_cast<uint32_t>(compressed_data[adler_offset + 2]) << 8)  |
                                (static_cast<uint32_t>(compressed_data[adler_offset + 3]));

        uint32_t computed_adler = adler32(*result);
        if (stream_adler != computed_adler) {
            return ImageResult<std::vector<uint8_t>>::fail(ImageError::CorruptedData);
        }
    }

    return result;
}

// ============================================================================
// 8. Deflate / Zlib Fast Sovereign Compressor
// ============================================================================

namespace {

class BitWriter {
public:
    void write_bits(uint32_t val, int n) {
        bit_buf_ |= (static_cast<uint64_t>(val & ((1ULL << n) - 1)) << bit_count_);
        bit_count_ += n;
        while (bit_count_ >= 8) {
            data_.push_back(static_cast<uint8_t>(bit_buf_ & 0xFF));
            bit_buf_ >>= 8;
            bit_count_ -= 8;
        }
    }

    void flush() {
        if (bit_count_ > 0) {
            data_.push_back(static_cast<uint8_t>(bit_buf_ & 0xFF));
            bit_buf_ = 0;
            bit_count_ = 0;
        }
    }

    std::vector<uint8_t> take() && {
        flush();
        return std::move(data_);
    }

private:
    std::vector<uint8_t> data_;
    uint64_t bit_buf_{0};
    int bit_count_{0};
};

// Fixed Huffman code values and lengths for literals/lengths
struct FixedCode {
    uint16_t code;
    uint8_t len;
};

inline FixedCode get_fixed_lit_code(int sym) noexcept {
    if (sym <= 143) {
        return {static_cast<uint16_t>(0x30 + sym), 8};
    } else if (sym <= 255) {
        return {static_cast<uint16_t>(0x190 + (sym - 144)), 9};
    } else if (sym <= 279) {
        return {static_cast<uint16_t>(sym - 256), 7};
    } else {
        return {static_cast<uint16_t>(0xC0 + (sym - 280)), 8};
    }
}

inline void write_fixed_symbol(BitWriter& writer, int sym) {
    auto fc = get_fixed_lit_code(sym);
    // Fixed code is transmitted MSB first, so we reverse it for LSB-first bit writer
    writer.write_bits(reverse_bits(fc.code, fc.len), fc.len);
}

inline void write_fixed_distance(BitWriter& writer, int dist_code) {
    // 5 bits for distance in fixed Huffman
    writer.write_bits(reverse_bits(dist_code, 5), 5);
}

} // namespace

ImageResult<std::vector<uint8_t>> deflate_compress(
    std::span<const uint8_t> uncompressed_data,
    int level
) {
    BitWriter writer;

    if (level == 0 || uncompressed_data.empty()) {
        // Uncompressed blocks
        size_t offset = 0;
        size_t total = uncompressed_data.size();
        do {
            uint16_t chunk = static_cast<uint16_t>(std::min(total - offset, size_t(65535)));
            bool bfinal = (offset + chunk >= total);
            writer.write_bits(bfinal ? 1 : 0, 1);
            writer.write_bits(0, 2); // BTYPE 00
            writer.flush();

            // Direct write len and ~len
            writer.write_bits(chunk & 0xFF, 8);
            writer.write_bits((chunk >> 8) & 0xFF, 8);
            uint16_t nlen = ~chunk;
            writer.write_bits(nlen & 0xFF, 8);
            writer.write_bits((nlen >> 8) & 0xFF, 8);
            for (size_t i = 0; i < chunk; ++i) {
                writer.write_bits(uncompressed_data[offset + i], 8);
            }
            offset += chunk;
        } while (offset < total);

        return ImageResult<std::vector<uint8_t>>::success(std::move(writer).take());
    }

    // Fast LZ77 with Fixed Huffman encoding
    writer.write_bits(1, 1); // BFINAL = 1
    writer.write_bits(1, 2); // BTYPE 01 (Fixed Huffman)

    constexpr size_t HASH_BITS = 15;
    constexpr size_t HASH_SIZE = 1 << HASH_BITS;
    constexpr size_t HASH_MASK = HASH_SIZE - 1;
    constexpr size_t WINDOW_SIZE = 32768;
    constexpr size_t WINDOW_MASK = WINDOW_SIZE - 1;

    std::vector<int32_t> head(HASH_SIZE, -1);
    std::array<int32_t, WINDOW_SIZE> prev;
    prev.fill(-1);

    auto hash3 = [](const uint8_t* p) -> uint32_t {
        return ((static_cast<uint32_t>(p[0]) << 10) ^
                (static_cast<uint32_t>(p[1]) << 5)  ^
                (static_cast<uint32_t>(p[2]))) & HASH_MASK;
    };

    const uint8_t* src = uncompressed_data.data();
    size_t src_len = uncompressed_data.size();
    size_t i = 0;

    int max_chain = (level >= 6) ? 32 : (level >= 4 ? 16 : 4);

    while (i < src_len) {
        uint32_t best_len = 0;
        uint32_t best_dist = 0;

        if (i + 3 <= src_len) {
            uint32_t h = hash3(src + i);
            int32_t match_pos = head[h];
            head[h] = static_cast<int32_t>(i);
            prev[i & WINDOW_MASK] = match_pos;

            int chain = 0;
            while (match_pos >= 0 && chain++ < max_chain) {
                size_t dist = i - match_pos;
                if (dist > WINDOW_SIZE) break;

                // Check match length
                size_t max_m = std::min(size_t(258), src_len - i);
                size_t m = 0;
                while (m < max_m && src[i + m] == src[match_pos + m]) {
                    ++m;
                }

                if (m > best_len && m >= 3) {
                    best_len = static_cast<uint32_t>(m);
                    best_dist = static_cast<uint32_t>(dist);
                    if (best_len == 258) break;
                }
                match_pos = prev[match_pos & WINDOW_MASK];
            }
        }

        if (best_len >= 3) {
            // Find length code
            uint32_t len_idx = 0;
            while (len_idx < 28 && LENGTH_BASE[len_idx + 1] <= best_len) {
                ++len_idx;
            }
            write_fixed_symbol(writer, 257 + len_idx);
            uint8_t ex_len = LENGTH_EXTRA[len_idx];
            if (ex_len > 0) {
                writer.write_bits(best_len - LENGTH_BASE[len_idx], ex_len);
            }

            // Find distance code
            uint32_t dist_idx = 0;
            while (dist_idx < 29 && DIST_BASE[dist_idx + 1] <= best_dist) {
                ++dist_idx;
            }
            write_fixed_distance(writer, dist_idx);
            uint8_t ex_dist = DIST_EXTRA[dist_idx];
            if (ex_dist > 0) {
                writer.write_bits(best_dist - DIST_BASE[dist_idx], ex_dist);
            }

            if (level >= 4) {
                // Insert intermediate hashes for high compression
                for (size_t k = 1; k < best_len && (i + k + 2) < src_len; ++k) {
                    uint32_t hk = hash3(src + i + k);
                    prev[(i + k) & WINDOW_MASK] = head[hk];
                    head[hk] = static_cast<int32_t>(i + k);
                }
            }
            i += best_len;
        } else {
            // Emit literal
            write_fixed_symbol(writer, src[i]);
            ++i;
        }
    }

    // End of block code (256)
    write_fixed_symbol(writer, 256);
    writer.flush();

    return ImageResult<std::vector<uint8_t>>::success(std::move(writer).take());
}

ImageResult<std::vector<uint8_t>> zlib_compress(
    std::span<const uint8_t> uncompressed_data,
    int level
) {
    auto def = deflate_compress(uncompressed_data, level);
    if (!def) return def;

    std::vector<uint8_t> out;
    out.reserve(def->size() + 6);

    // CMF: CM=8 (Deflate), CINFO=7 (32K window) -> 0x78
    uint8_t cmf = 0x78;
    // FLG: level flags (level 6 -> 2), check bits
    uint8_t flg = 0x9C; // 0x78 * 256 + 0x9C = 30876 % 31 == 0
    if (level <= 1) {
        flg = 0x01; // check bits for fast
        while ((cmf * 256 + flg) % 31 != 0) ++flg;
    }

    out.push_back(cmf);
    out.push_back(flg);

    out.insert(out.end(), def->begin(), def->end());

    uint32_t adler = adler32(uncompressed_data);
    out.push_back(static_cast<uint8_t>((adler >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((adler >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((adler >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(adler & 0xFF));

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

} // namespace nisaba::image

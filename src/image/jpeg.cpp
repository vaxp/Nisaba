#include "nisaba/image/jpeg.hpp"
#include <fstream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>
#include <bit>

#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h>
#endif

namespace nisaba::image {

namespace {

// ============================================================================
// JPEG Constants and Tables
// ============================================================================

constexpr uint8_t ZIGZAG[64] = {
     0,  1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

// Standard Luminance Quantization Table
constexpr uint8_t STD_LUMA_QT[64] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

// Standard Chrominance Quantization Table
constexpr uint8_t STD_CHROMA_QT[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99
};

// Standard DC Luminance Huffman Bits and Values
constexpr uint8_t STD_DC_LUMA_BITS[16] = {0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0};
constexpr uint8_t STD_DC_LUMA_VALS[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

// Standard DC Chrominance Huffman Bits and Values
constexpr uint8_t STD_DC_CHROMA_BITS[16] = {0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};
constexpr uint8_t STD_DC_CHROMA_VALS[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

// Standard AC Luminance Huffman Bits and Values
constexpr uint8_t STD_AC_LUMA_BITS[16] = {0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125};
constexpr uint8_t STD_AC_LUMA_VALS[162] = {
    0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
    0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
    0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
    0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
    0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
    0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
    0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
    0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

// Standard AC Chrominance Huffman Bits and Values
constexpr uint8_t STD_AC_CHROMA_BITS[16] = {0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 119};
constexpr uint8_t STD_AC_CHROMA_VALS[162] = {
    0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
    0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
    0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
    0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
    0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
    0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
    0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
    0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
    0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
    0xf9, 0xfa
};

// ============================================================================
// JPEG Huffman Decoder Structures
// ============================================================================

struct JpegHuffTable {
    static constexpr int FAST_BITS = 9;
    static constexpr size_t FAST_SIZE = 1 << FAST_BITS;

    // Direct lookup for codes <= FAST_BITS: (symbol << 4) | code_length
    std::array<uint16_t, FAST_SIZE> fast{};
    
    // Canonical Huffman prefix bounds for codes > FAST_BITS
    std::array<int32_t, 17> maxcode{};
    std::array<int32_t, 17> valoffset{};
    std::vector<uint8_t> vals;

    // Fast AC table for combined (run, size, diff) lookup in 1 step
    std::array<int16_t, FAST_SIZE> fast_ac{};

    bool build(std::span<const uint8_t, 16> bits, std::span<const uint8_t> values, bool is_ac = false) {
        fast.fill(0);
        fast_ac.fill(0);
        maxcode.fill(-1);
        valoffset.fill(0);
        vals.assign(values.begin(), values.end());

        uint16_t code = 0;
        size_t val_idx = 0;

        for (int l = 1; l <= 16; ++l) {
            uint8_t count = bits[l - 1];
            if (count > 0) {
                valoffset[l] = static_cast<int32_t>(val_idx) - code;
                for (int i = 0; i < count; ++i) {
                    if (val_idx >= values.size()) return false;
                    uint8_t sym = values[val_idx++];

                    if (l <= FAST_BITS) {
                        uint32_t step = 1U << (FAST_BITS - l);
                        uint32_t base = static_cast<uint32_t>(code) << (FAST_BITS - l);
                        for (uint32_t idx = base; idx < base + step; ++idx) {
                            fast[idx] = static_cast<uint16_t>((sym << 4) | l);
                        }
                    }
                    ++code;
                }
                maxcode[l] = code - 1;
            }
            code <<= 1;
        }

        if (is_ac) {
            build_fast_ac();
        }
        return true;
    }

    void build_fast_ac() {
        for (int i = 0; i < static_cast<int>(FAST_SIZE); ++i) {
            uint16_t entry = fast[i];
            uint8_t len = entry & 0x0F;
            if (len > 0 && len <= FAST_BITS) {
                uint8_t sym = entry >> 4;
                int run = (sym >> 4) & 15;
                int magbits = sym & 15;
                if (magbits && len + magbits <= FAST_BITS) {
                    int k = ((i << len) & (FAST_SIZE - 1)) >> (FAST_BITS - magbits);
                    int m = 1 << (magbits - 1);
                    if (k < m) k += (-1 << magbits) + 1;
                    if (k >= -128 && k <= 127) {
                        fast_ac[i] = static_cast<int16_t>((k << 8) | (run << 4) | (len + magbits));
                    }
                }
            }
        }
    }
};

// ============================================================================
// JPEG MSB-first Bitstream Reader with 0xFF byte stuffing handling
// ============================================================================

class JpegBitReader {
public:
    explicit JpegBitReader(std::span<const uint8_t> data) noexcept
        : ptr_(data.data()), end_(data.data() + data.size()) {
        refill();
    }

    bool has_error() const noexcept { return error_; }
    void set_error() noexcept { error_ = true; }

    inline void refill() noexcept {
        while (bit_count_ <= 48 && ptr_ < end_) {
            uint8_t b = *ptr_++;
            if (b == 0xFF) {
                if (ptr_ < end_) {
                    uint8_t marker = *ptr_++;
                    if (marker == 0x00) {
                        b = 0xFF; // Stuffed byte
                    } else if (marker >= 0xD0 && marker <= 0xD7) {
                        rst_marker_ = marker;
                        b = 0xFF;
                    } else if (marker == 0xD9) {
                        ptr_ = end_;
                        b = 0xFF;
                    }
                }
            }
            bit_buf_ = (bit_buf_ << 8) | b;
            bit_count_ += 8;
        }
    }

    inline uint32_t peek_bits(int n) noexcept {
        if (bit_count_ < n) {
            refill();
        }
        if (bit_count_ < n) {
            return static_cast<uint32_t>((bit_buf_ << (n - bit_count_)) & ((1ULL << n) - 1));
        }
        return static_cast<uint32_t>((bit_buf_ >> (bit_count_ - n)) & ((1ULL << n) - 1));
    }

    inline void consume_bits(int n) noexcept {
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
        uint32_t val = static_cast<uint32_t>((bit_buf_ >> (bit_count_ - n)) & ((1ULL << n) - 1));
        bit_count_ -= n;
        return val;
    }

    int decode_symbol(const JpegHuffTable& table) noexcept {
        uint32_t peek = peek_bits(JpegHuffTable::FAST_BITS);
        uint16_t entry = table.fast[peek];
        uint8_t len = entry & 0x0F;

        if (len > 0 && len <= JpegHuffTable::FAST_BITS) {
            consume_bits(len);
            return entry >> 4;
        }

        // Fast ISO 10918-1 bounded lookup for codes > FAST_BITS
        for (int l = JpegHuffTable::FAST_BITS + 1; l <= 16; ++l) {
            if (table.maxcode[l] >= 0) {
                uint32_t code = peek_bits(l);
                if (static_cast<int32_t>(code) <= table.maxcode[l]) {
                    consume_bits(l);
                    size_t v_idx = static_cast<size_t>(table.valoffset[l] + static_cast<int32_t>(code));
                    if (v_idx < table.vals.size()) {
                        return table.vals[v_idx];
                    }
                    break;
                }
            }
        }

        error_ = true;
        return -1;
    }

    inline int read_diff(int s) noexcept {
        if (s == 0) return 0;
        uint32_t val = read_bits(s);
        if ((val & (1U << (s - 1))) == 0) {
            return static_cast<int>(val) - ((1 << s) - 1);
        }
        return static_cast<int>(val);
    }

    void align_to_byte() noexcept {
        bit_count_ = 0;
        bit_buf_ = 0;
    }

    uint8_t get_and_clear_rst() noexcept {
        uint8_t r = rst_marker_;
        rst_marker_ = 0;
        return r;
    }

private:
    const uint8_t* ptr_{nullptr};
    const uint8_t* end_{nullptr};
    uint64_t bit_buf_{0};
    int bit_count_{0};
    uint8_t rst_marker_{0};
    bool error_{false};
};

// ============================================================================
// Fast AAN / Chen Fixed-Point IDCT (13-bit precision)
// ============================================================================

const auto& get_dct_table() noexcept {
    static const auto table = []() {
        std::array<std::array<float, 8>, 8> t{};
        constexpr double PI = 3.14159265358979323846;
        for (int u = 0; u < 8; ++u) {
            double cu = (u == 0) ? (1.0 / 1.4142135623730950488) : 1.0;
            for (int x = 0; x < 8; ++x) {
                t[u][x] = static_cast<float>(0.5 * cu * std::cos((2 * x + 1) * u * PI / 16.0));
            }
        }
        return t;
    }();
    return table;
}

#define NISABA_F2F(x)  static_cast<int>(((x) * 4096.0f + 0.5f))

#define NISABA_IDCT_1D(s0, s1, s2, s3, s4, s5, s6, s7) \
    int p2 = s2; \
    int p3 = s6; \
    int p1 = (p2 + p3) * NISABA_F2F(0.5411961f); \
    int t2 = p1 + p3 * NISABA_F2F(-1.847759065f); \
    int t3 = p1 + p2 * NISABA_F2F(0.765366865f); \
    p2 = s0; \
    p3 = s4; \
    int t0 = (p2 + p3) << 12; \
    int t1 = (p2 - p3) << 12; \
    int x0 = t0 + t3; \
    int x3 = t0 - t3; \
    int x1 = t1 + t2; \
    int x2 = t1 - t2; \
    t0 = s7; \
    t1 = s5; \
    t2 = s3; \
    t3 = s1; \
    p3 = t0 + t2; \
    int p4 = t1 + t3; \
    p1 = t0 + t3; \
    p2 = t1 + t2; \
    int p5 = (p3 + p4) * NISABA_F2F(1.175875602f); \
    t0 = t0 * NISABA_F2F(0.298631336f); \
    t1 = t1 * NISABA_F2F(2.053119869f); \
    t2 = t2 * NISABA_F2F(3.072711026f); \
    t3 = t3 * NISABA_F2F(1.501321110f); \
    p1 = p5 + p1 * NISABA_F2F(-0.899976223f); \
    p2 = p5 + p2 * NISABA_F2F(-2.562915447f); \
    p3 = p3 * NISABA_F2F(-1.961570560f); \
    p4 = p4 * NISABA_F2F(-0.390180644f); \
    t3 += p1 + p4; \
    t2 += p2 + p3; \
    t1 += p2 + p4; \
    t0 += p1 + p3;

inline uint8_t clamp_u8(int x) noexcept {
    if (static_cast<unsigned>(x) > 255) {
        if (x < 0) return 0;
        return 255;
    }
    return static_cast<uint8_t>(x);
}

#if defined(__x86_64__) || defined(_M_X64)
static void idct_8x8_simd(const int16_t data[64], uint8_t out[64]) noexcept {
    __m128i row0, row1, row2, row3, row4, row5, row6, row7;
    __m128i tmp;

    #define dct_const(x,y)  _mm_setr_epi16((x),(y),(x),(y),(x),(y),(x),(y))

    #define dct_rot(out0,out1, x,y,c0,c1) \
       __m128i c0##lo = _mm_unpacklo_epi16((x),(y)); \
       __m128i c0##hi = _mm_unpackhi_epi16((x),(y)); \
       __m128i out0##_l = _mm_madd_epi16(c0##lo, c0); \
       __m128i out0##_h = _mm_madd_epi16(c0##hi, c0); \
       __m128i out1##_l = _mm_madd_epi16(c0##lo, c1); \
       __m128i out1##_h = _mm_madd_epi16(c0##hi, c1)

    #define dct_widen(out, in) \
       __m128i out##_l = _mm_srai_epi32(_mm_unpacklo_epi16(_mm_setzero_si128(), (in)), 4); \
       __m128i out##_h = _mm_srai_epi32(_mm_unpackhi_epi16(_mm_setzero_si128(), (in)), 4)

    #define dct_wadd(out, a, b) \
       __m128i out##_l = _mm_add_epi32(a##_l, b##_l); \
       __m128i out##_h = _mm_add_epi32(a##_h, b##_h)

    #define dct_wsub(out, a, b) \
       __m128i out##_l = _mm_sub_epi32(a##_l, b##_l); \
       __m128i out##_h = _mm_sub_epi32(a##_h, b##_h)

    #define dct_bfly32o(out0, out1, a,b,bias,s) \
       { \
          __m128i abiased_l = _mm_add_epi32(a##_l, bias); \
          __m128i abiased_h = _mm_add_epi32(a##_h, bias); \
          dct_wadd(sum, abiased, b); \
          dct_wsub(dif, abiased, b); \
          out0 = _mm_packs_epi32(_mm_srai_epi32(sum_l, s), _mm_srai_epi32(sum_h, s)); \
          out1 = _mm_packs_epi32(_mm_srai_epi32(dif_l, s), _mm_srai_epi32(dif_h, s)); \
       }

    #define dct_interleave8(a, b) \
       tmp = a; \
       a = _mm_unpacklo_epi8(a, b); \
       b = _mm_unpackhi_epi8(tmp, b)

    #define dct_interleave16(a, b) \
       tmp = a; \
       a = _mm_unpacklo_epi16(a, b); \
       b = _mm_unpackhi_epi16(tmp, b)

    #define dct_pass(bias,shift) \
       { \
          dct_rot(t2e,t3e, row2,row6, rot0_0,rot0_1); \
          __m128i sum04 = _mm_add_epi16(row0, row4); \
          __m128i dif04 = _mm_sub_epi16(row0, row4); \
          dct_widen(t0e, sum04); \
          dct_widen(t1e, dif04); \
          dct_wadd(x0, t0e, t3e); \
          dct_wsub(x3, t0e, t3e); \
          dct_wadd(x1, t1e, t2e); \
          dct_wsub(x2, t1e, t2e); \
          dct_rot(y0o,y2o, row7,row3, rot2_0,rot2_1); \
          dct_rot(y1o,y3o, row5,row1, rot3_0,rot3_1); \
          __m128i sum17 = _mm_add_epi16(row1, row7); \
          __m128i sum35 = _mm_add_epi16(row3, row5); \
          dct_rot(y4o,y5o, sum17,sum35, rot1_0,rot1_1); \
          dct_wadd(x4, y0o, y4o); \
          dct_wadd(x5, y1o, y5o); \
          dct_wadd(x6, y2o, y5o); \
          dct_wadd(x7, y3o, y4o); \
          dct_bfly32o(row0,row7, x0,x7,bias,shift); \
          dct_bfly32o(row1,row6, x1,x6,bias,shift); \
          dct_bfly32o(row2,row5, x2,x5,bias,shift); \
          dct_bfly32o(row3,row4, x3,x4,bias,shift); \
       }

    const __m128i rot0_0 = dct_const(NISABA_F2F(0.5411961f), NISABA_F2F(0.5411961f) + NISABA_F2F(-1.847759065f));
    const __m128i rot0_1 = dct_const(NISABA_F2F(0.5411961f) + NISABA_F2F( 0.765366865f), NISABA_F2F(0.5411961f));
    const __m128i rot1_0 = dct_const(NISABA_F2F(1.175875602f) + NISABA_F2F(-0.899976223f), NISABA_F2F(1.175875602f));
    const __m128i rot1_1 = dct_const(NISABA_F2F(1.175875602f), NISABA_F2F(1.175875602f) + NISABA_F2F(-2.562915447f));
    const __m128i rot2_0 = dct_const(NISABA_F2F(-1.961570560f) + NISABA_F2F( 0.298631336f), NISABA_F2F(-1.961570560f));
    const __m128i rot2_1 = dct_const(NISABA_F2F(-1.961570560f), NISABA_F2F(-1.961570560f) + NISABA_F2F( 3.072711026f));
    const __m128i rot3_0 = dct_const(NISABA_F2F(-0.390180644f) + NISABA_F2F( 2.053119869f), NISABA_F2F(-0.390180644f));
    const __m128i rot3_1 = dct_const(NISABA_F2F(-0.390180644f), NISABA_F2F(-0.390180644f) + NISABA_F2F( 1.501321110f));

    const __m128i bias_0 = _mm_set1_epi32(512);
    const __m128i bias_1 = _mm_set1_epi32(65536 + (128 << 17));

    row0 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 0*8));
    row1 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 1*8));
    row2 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 2*8));
    row3 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 3*8));
    row4 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 4*8));
    row5 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 5*8));
    row6 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 6*8));
    row7 = _mm_load_si128(reinterpret_cast<const __m128i*>(data + 7*8));

    dct_pass(bias_0, 10);

    // Transpose 16-bit
    dct_interleave16(row0, row4);
    dct_interleave16(row1, row5);
    dct_interleave16(row2, row6);
    dct_interleave16(row3, row7);

    dct_interleave16(row0, row2);
    dct_interleave16(row1, row3);
    dct_interleave16(row4, row6);
    dct_interleave16(row5, row7);

    dct_interleave16(row0, row1);
    dct_interleave16(row2, row3);
    dct_interleave16(row4, row5);
    dct_interleave16(row6, row7);

    dct_pass(bias_1, 17);

    // Pack to 8-bit unsigned
    __m128i p0 = _mm_packus_epi16(row0, row1);
    __m128i p1 = _mm_packus_epi16(row2, row3);
    __m128i p2 = _mm_packus_epi16(row4, row5);
    __m128i p3 = _mm_packus_epi16(row6, row7);

    // Transpose 8-bit
    dct_interleave8(p0, p2);
    dct_interleave8(p1, p3);

    dct_interleave8(p0, p1);
    dct_interleave8(p2, p3);

    dct_interleave8(p0, p2);
    dct_interleave8(p1, p3);

    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 0*8), p0);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 1*8), _mm_shuffle_epi32(p0, 0x4e));
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 2*8), p2);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 3*8), _mm_shuffle_epi32(p2, 0x4e));
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 4*8), p1);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 5*8), _mm_shuffle_epi32(p1, 0x4e));
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 6*8), p3);
    _mm_storel_epi64(reinterpret_cast<__m128i*>(out + 7*8), _mm_shuffle_epi32(p3, 0x4e));

    #undef dct_const
    #undef dct_rot
    #undef dct_widen
    #undef dct_wadd
    #undef dct_wsub
    #undef dct_bfly32o
    #undef dct_interleave8
    #undef dct_interleave16
    #undef dct_pass
}
#endif

[[maybe_unused]] void idct_8x8(const int16_t in[64], uint8_t out[64]) noexcept {
    int val[64];
    // Column pass
    for (int i = 0; i < 8; ++i) {
        if (in[i + 8] == 0 && in[i + 16] == 0 && in[i + 24] == 0 && in[i + 32] == 0 &&
            in[i + 40] == 0 && in[i + 48] == 0 && in[i + 56] == 0) {
            int dcterm = in[i] << 2;
            val[i + 0]  = dcterm;
            val[i + 8]  = dcterm;
            val[i + 16] = dcterm;
            val[i + 24] = dcterm;
            val[i + 32] = dcterm;
            val[i + 40] = dcterm;
            val[i + 48] = dcterm;
            val[i + 56] = dcterm;
        } else {
            NISABA_IDCT_1D(in[i + 0], in[i + 8], in[i + 16], in[i + 24],
                           in[i + 32], in[i + 40], in[i + 48], in[i + 56]);
            val[i + 0]  = (x0 + t3 + 512) >> 10;
            val[i + 56] = (x0 - t3 + 512) >> 10;
            val[i + 8]  = (x1 + t2 + 512) >> 10;
            val[i + 48] = (x1 - t2 + 512) >> 10;
            val[i + 16] = (x2 + t1 + 512) >> 10;
            val[i + 40] = (x2 - t1 + 512) >> 10;
            val[i + 24] = (x3 + t0 + 512) >> 10;
            val[i + 32] = (x3 - t0 + 512) >> 10;
        }
    }

    // Row pass
    constexpr int bias = 65536 + (128 << 17);
    for (int i = 0; i < 8; ++i) {
        const int* v = val + i * 8;
        uint8_t* o = out + i * 8;
        NISABA_IDCT_1D(v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        o[0] = clamp_u8((x0 + t3 + bias) >> 17);
        o[7] = clamp_u8((x0 - t3 + bias) >> 17);
        o[1] = clamp_u8((x1 + t2 + bias) >> 17);
        o[6] = clamp_u8((x1 - t2 + bias) >> 17);
        o[2] = clamp_u8((x2 + t1 + bias) >> 17);
        o[5] = clamp_u8((x2 - t1 + bias) >> 17);
        o[3] = clamp_u8((x3 + t0 + bias) >> 17);
        o[4] = clamp_u8((x3 - t0 + bias) >> 17);
    }
}

// BT.601 YCbCr -> RGB conversion directly to PremultipliedColorU8
inline PremultipliedColorU8 ycbcr_to_rgb(uint8_t y, uint8_t cb, uint8_t cr) noexcept {
    int cb_shift = cb - 128;
    int cr_shift = cr - 128;
    int r = y + ((1436 * cr_shift + 512) >> 10);
    int g = y - ((352 * cb_shift + 731 * cr_shift + 512) >> 10);
    int b = y + ((1815 * cb_shift + 512) >> 10);
    return PremultipliedColorU8(clamp_u8(r), clamp_u8(g), clamp_u8(b), 255);
}

} // namespace

// ============================================================================
// JPEG Decoder Implementation
// ============================================================================

ImageResult<ImageInfo> probe_jpeg(std::span<const uint8_t> jpeg_data) {
    if (jpeg_data.size() < 4 || jpeg_data[0] != 0xFF || jpeg_data[1] != 0xD8) {
        return ImageResult<ImageInfo>::fail(ImageError::InvalidSignature);
    }

    size_t offset = 2;
    while (offset + 4 <= jpeg_data.size()) {
        if (jpeg_data[offset] != 0xFF) {
            ++offset;
            continue;
        }
        uint8_t marker = jpeg_data[offset + 1];
        offset += 2;

        if (marker == 0xD8 || marker == 0xD9 || marker == 0x00 || (marker >= 0xD0 && marker <= 0xD7)) {
            continue;
        }

        if (offset + 2 > jpeg_data.size()) break;
        uint16_t len = (static_cast<uint16_t>(jpeg_data[offset]) << 8) | jpeg_data[offset + 1];

        if (marker == 0xC0 || marker == 0xC2) { // SOF0 (Baseline) or SOF2 (Progressive)
            if (offset + len > jpeg_data.size() || len < 8) {
                return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
            }
            ImageInfo info{};
            info.bit_depth = jpeg_data[offset + 2];
            info.height = (static_cast<uint32_t>(jpeg_data[offset + 3]) << 8) | jpeg_data[offset + 4];
            info.width = (static_cast<uint32_t>(jpeg_data[offset + 5]) << 8) | jpeg_data[offset + 6];
            info.channels = jpeg_data[offset + 7];
            info.has_alpha = false;
            info.format = ImageFormat::JPEG;
            info.color_model = (info.channels == 1) ? ColorModel::Grayscale : ColorModel::RGB;
            return ImageResult<ImageInfo>::success(info);
        }

        offset += len;
    }

    return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
}

ImageResult<Pixmap> decode_jpeg(std::span<const uint8_t> jpeg_data) {
    if (jpeg_data.size() < 4 || jpeg_data[0] != 0xFF || jpeg_data[1] != 0xD8) {
        return ImageResult<Pixmap>::fail(ImageError::InvalidSignature);
    }

    struct Component {
        uint8_t id{0};
        uint8_t h_factor{1};
        uint8_t v_factor{1};
        uint8_t qt_id{0};
        uint8_t dc_table{0};
        uint8_t ac_table{0};
        int32_t dc_pred{0};
    };

    uint32_t width = 0;
    uint32_t height = 0;
    std::vector<Component> components;
    std::array<std::array<uint8_t, 64>, 4> q_tables{};
    std::array<bool, 4> has_qt{false, false, false, false};

    std::array<JpegHuffTable, 4> dc_huff{};
    std::array<JpegHuffTable, 4> ac_huff{};
    uint16_t restart_interval = 0;

    size_t offset = 2;
    size_t sos_offset = 0;

    while (offset + 4 <= jpeg_data.size()) {
        if (jpeg_data[offset] != 0xFF) {
            ++offset;
            continue;
        }
        uint8_t marker = jpeg_data[offset + 1];
        offset += 2;

        if (marker == 0xD8 || marker == 0x00 || (marker >= 0xD0 && marker <= 0xD7)) {
            continue;
        }
        if (marker == 0xD9) { // EOI
            break;
        }

        uint16_t len = (static_cast<uint16_t>(jpeg_data[offset]) << 8) | jpeg_data[offset + 1];
        if (offset + len > jpeg_data.size() || len < 2) {
            return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
        }

        const uint8_t* payload = jpeg_data.data() + offset + 2;
        size_t payload_len = len - 2;

        if (marker == 0xC2) { // Progressive JPEG (SOF2)
            return ImageResult<Pixmap>::fail(ImageError::UnsupportedFeature);
        } else if (marker == 0xC0) { // SOF0 (Baseline Sequential)
            if (payload_len < 6) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            height = (static_cast<uint32_t>(payload[1]) << 8) | payload[2];
            width = (static_cast<uint32_t>(payload[3]) << 8) | payload[4];
            if (width == 0 || height == 0 || width > 32768 || height > 32768) {
                return ImageResult<Pixmap>::fail(ImageError::InvalidDimensions);
            }
            uint8_t num_comps = payload[5];
            if (num_comps == 0 || num_comps > 4 || payload_len < 6 + static_cast<size_t>(num_comps) * 3) {
                return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            }

            components.resize(num_comps);
            for (uint8_t c = 0; c < num_comps; ++c) {
                components[c].id = payload[6 + c * 3];
                components[c].h_factor = (payload[6 + c * 3 + 1] >> 4) & 0x0F;
                components[c].v_factor = payload[6 + c * 3 + 1] & 0x0F;
                components[c].qt_id = payload[6 + c * 3 + 2] & 0x0F;
                if (components[c].h_factor == 0 || components[c].h_factor > 4 ||
                    components[c].v_factor == 0 || components[c].v_factor > 4 ||
                    components[c].qt_id >= 4) {
                    return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                }
            }
        } else if (marker == 0xDB) { // DQT
            size_t p_off = 0;
            while (p_off < payload_len) {
                uint8_t info = payload[p_off++];
                uint8_t qt_id = info & 0x0F;
                uint8_t prec = (info >> 4) & 0x0F;
                if (qt_id >= 4) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);

                if (prec == 0) { // 8-bit
                    if (p_off + 64 > payload_len) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                    for (int i = 0; i < 64; ++i) {
                        q_tables[qt_id][ZIGZAG[i]] = payload[p_off + i];
                    }
                    has_qt[qt_id] = true;
                    p_off += 64;
                } else {
                    return ImageResult<Pixmap>::fail(ImageError::UnsupportedFeature);
                }
            }
        } else if (marker == 0xC4) { // DHT
            size_t p_off = 0;
            while (p_off < payload_len) {
                uint8_t info = payload[p_off++];
                uint8_t table_class = (info >> 4) & 0x0F; // 0 = DC, 1 = AC
                uint8_t table_id = info & 0x0F;
                if (table_id >= 4) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);

                if (p_off + 16 > payload_len) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                std::array<uint8_t, 16> bits{};
                size_t total_vals = 0;
                for (int i = 0; i < 16; ++i) {
                    bits[i] = payload[p_off + i];
                    total_vals += bits[i];
                }
                p_off += 16;

                if (p_off + total_vals > payload_len) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                std::span<const uint8_t> vals(payload + p_off, total_vals);
                p_off += total_vals;

                if (table_class == 0) {
                    dc_huff[table_id].build(bits, vals, false);
                } else {
                    ac_huff[table_id].build(bits, vals, true);
                }
            }
        } else if (marker == 0xDD) { // DRI
            if (payload_len >= 2) {
                restart_interval = (static_cast<uint16_t>(payload[0]) << 8) | payload[1];
            }
        } else if (marker == 0xDA) { // SOS
            if (payload_len < 3) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            uint8_t num_comps = payload[0];
            for (uint8_t i = 0; i < num_comps; ++i) {
                uint8_t id = payload[1 + i * 2];
                uint8_t tables = payload[1 + i * 2 + 1];
                for (auto& comp : components) {
                    if (comp.id == id) {
                        comp.dc_table = (tables >> 4) & 0x0F;
                        comp.ac_table = tables & 0x0F;
                    }
                }
            }
            sos_offset = offset + len;
            break;
        }

        offset += len;
    }

    if (width == 0 || height == 0 || components.empty() || sos_offset == 0) {
        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    }

    auto maybe_pixmap = Pixmap::allocate(width, height);
    if (!maybe_pixmap) return ImageResult<Pixmap>::fail(ImageError::OutOfMemory);
    Pixmap pixmap = std::move(*maybe_pixmap);

    // Compute maximum sampling factors
    uint8_t max_h = 1;
    uint8_t max_v = 1;
    for (const auto& comp : components) {
        max_h = std::max(max_h, comp.h_factor);
        max_v = std::max(max_v, comp.v_factor);
    }

    uint32_t mcu_w = max_h * 8;
    uint32_t mcu_h = max_v * 8;
    uint32_t mcu_cols = (width + mcu_w - 1) / mcu_w;
    uint32_t mcu_rows = (height + mcu_h - 1) / mcu_h;

    JpegBitReader reader(jpeg_data.subspan(sos_offset));

    auto decode_block = [&](Component& comp, uint8_t block_out[64]) -> bool {
        alignas(16) int16_t coeffs[64]{};
        const auto& q_tab = q_tables[comp.qt_id];

        // DC
        int s = reader.decode_symbol(dc_huff[comp.dc_table]);
        if (s < 0) return false;
        int diff = reader.read_diff(s);
        comp.dc_pred += diff;
        coeffs[0] = static_cast<int16_t>(comp.dc_pred * q_tab[0]);

        // AC
        const auto& ac_table = ac_huff[comp.ac_table];
        int k = 1;
        while (k < 64) {
            uint32_t peek = reader.peek_bits(9);
            int16_t fast_val = ac_table.fast_ac[peek];
            if (fast_val) {
                k += (fast_val >> 4) & 15;
                if (k >= 64) break;
                coeffs[ZIGZAG[k]] = static_cast<int16_t>((fast_val >> 8) * q_tab[ZIGZAG[k]]);
                reader.consume_bits(fast_val & 15);
                ++k;
            } else {
                int sym = reader.decode_symbol(ac_table);
                if (sym < 0) return false;
                if (sym == 0x00) break; // EOB

                uint8_t run = (sym >> 4) & 0x0F;
                uint8_t cat = sym & 0x0F;

                k += run;
                if (k >= 64) break;

                if (cat > 0) {
                    int ac_diff = reader.read_diff(cat);
                    coeffs[ZIGZAG[k]] = static_cast<int16_t>(ac_diff * q_tab[ZIGZAG[k]]);
                }
                ++k;
            }
        }

        if (k == 1) {
            int val = clamp_u8(((coeffs[0] + 4) >> 3) + 128);
            std::memset(block_out, val, 64);
            return true;
        }

#if defined(__x86_64__) || defined(_M_X64)
        idct_8x8_simd(coeffs, block_out);
#else
        idct_8x8(coeffs, block_out);
#endif
        return true;
    };

    uint32_t mcu_count = 0;

    PremultipliedColorU8* pix_base = pixmap.pixels_mut();

    if (components.size() == 1) {
        // Grayscale
        auto& comp = components[0];
        alignas(16) uint8_t block[64];

        for (uint32_t my = 0; my < mcu_rows; ++my) {
            for (uint32_t mx = 0; mx < mcu_cols; ++mx) {
                if (restart_interval > 0 && mcu_count > 0 && (mcu_count % restart_interval == 0)) {
                    reader.align_to_byte();
                    comp.dc_pred = 0;
                }
                if (!decode_block(comp, block)) {
                    return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                }

                uint32_t bx = mx * 8;
                uint32_t by = my * 8;
                for (int y = 0; y < 8; ++y) {
                    uint32_t py = by + y;
                    if (py >= height) continue;
                    PremultipliedColorU8* dst_row = pix_base + py * width;
                    for (int x = 0; x < 8; ++x) {
                        uint32_t px = bx + x;
                        if (px >= width) continue;
                        uint8_t val = block[y * 8 + x];
                        dst_row[px] = PremultipliedColorU8(val, val, val, 255);
                    }
                }
                ++mcu_count;
            }
        }
    } else if (components.size() == 3) {
        // YCbCr (4:4:4, 4:2:2, or 4:2:0)
        alignas(16) uint8_t y_blocks[4][64];
        alignas(16) uint8_t cb_block[64];
        alignas(16) uint8_t cr_block[64];

        int num_y_blocks = max_h * max_v;
        if (num_y_blocks > 4) {
            return ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
        }

        bool is_420 = (max_h == 2 && max_v == 2);
        bool is_444 = (max_h == 1 && max_v == 1);

        for (uint32_t my = 0; my < mcu_rows; ++my) {
            for (uint32_t mx = 0; mx < mcu_cols; ++mx) {
                if (restart_interval > 0 && mcu_count > 0 && (mcu_count % restart_interval == 0)) {
                    reader.align_to_byte();
                    for (auto& comp : components) comp.dc_pred = 0;
                }

                // Decode Y blocks
                for (int i = 0; i < num_y_blocks; ++i) {
                    if (!decode_block(components[0], y_blocks[i])) {
                        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                    }
                }
                // Decode Cb and Cr
                if (!decode_block(components[1], cb_block) ||
                    !decode_block(components[2], cr_block)) {
                    return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                }

                uint32_t mcu_px = mx * mcu_w;
                uint32_t mcu_py = my * mcu_h;

                if (is_420) {
                    // Fast path for standard 4:2:0 JPEG
                    for (uint32_t dy = 0; dy < 16; ++dy) {
                        uint32_t py = mcu_py + dy;
                        if (py >= height) break;

                        PremultipliedColorU8* dst_row = pix_base + py * width;
                        uint32_t by = dy >> 3;
                        uint32_t y_sub_y = dy & 7;
                        uint32_t cb_y = dy >> 1;
                        const uint8_t* cb_row = cb_block + cb_y * 8;
                        const uint8_t* cr_row = cr_block + cb_y * 8;

                        for (uint32_t dx = 0; dx < 16; dx += 2) {
                            uint32_t px = mcu_px + dx;
                            if (px >= width) break;

                            uint32_t cb_x = dx >> 1;
                            int cb_shift = cb_row[cb_x] - 128;
                            int cr_shift = cr_row[cb_x] - 128;
                            int r_add = (1436 * cr_shift + 512) >> 10;
                            int g_sub = (352 * cb_shift + 731 * cr_shift + 512) >> 10;
                            int b_add = (1815 * cb_shift + 512) >> 10;

                            // Pixel 0
                            uint32_t bx0 = dx >> 3;
                            uint32_t y_block_idx0 = (by << 1) | bx0;
                            uint8_t y_val0 = y_blocks[y_block_idx0][y_sub_y * 8 + (dx & 7)];
                            dst_row[px] = PremultipliedColorU8(clamp_u8(y_val0 + r_add),
                                                               clamp_u8(y_val0 - g_sub),
                                                               clamp_u8(y_val0 + b_add), 255);

                            // Pixel 1
                            if (px + 1 < width) {
                                uint32_t dx1 = dx + 1;
                                uint32_t bx1 = dx1 >> 3;
                                uint32_t y_block_idx1 = (by << 1) | bx1;
                                uint8_t y_val1 = y_blocks[y_block_idx1][y_sub_y * 8 + (dx1 & 7)];
                                dst_row[px + 1] = PremultipliedColorU8(clamp_u8(y_val1 + r_add),
                                                                       clamp_u8(y_val1 - g_sub),
                                                                       clamp_u8(y_val1 + b_add), 255);
                            }
                        }
                    }
                } else if (is_444) {
                    // Fast path for 4:4:4 JPEG
                    for (uint32_t dy = 0; dy < 8; ++dy) {
                        uint32_t py = mcu_py + dy;
                        if (py >= height) break;

                        PremultipliedColorU8* dst_row = pix_base + py * width;
                        const uint8_t* y_row = y_blocks[0] + dy * 8;
                        const uint8_t* cb_row = cb_block + dy * 8;
                        const uint8_t* cr_row = cr_block + dy * 8;

                        for (uint32_t dx = 0; dx < 8; ++dx) {
                            uint32_t px = mcu_px + dx;
                            if (px >= width) break;

                            dst_row[px] = ycbcr_to_rgb(y_row[dx], cb_row[dx], cr_row[dx]);
                        }
                    }
                } else {
                    // Generic fallback for 4:2:2 or other subsamplings
                    for (uint32_t dy = 0; dy < mcu_h; ++dy) {
                        uint32_t py = mcu_py + dy;
                        if (py >= height) continue;

                        PremultipliedColorU8* dst_row = pix_base + py * width;
                        for (uint32_t dx = 0; dx < mcu_w; ++dx) {
                            uint32_t px = mcu_px + dx;
                            if (px >= width) continue;

                            uint32_t by = dy / 8;
                            uint32_t bx = dx / 8;
                            uint32_t y_block_idx = by * max_h + bx;
                            uint8_t y_val = y_blocks[y_block_idx][(dy % 8) * 8 + (dx % 8)];

                            uint32_t cb_x = (dx * 8) / mcu_w;
                            uint32_t cb_y = (dy * 8) / mcu_h;
                            uint8_t cb_val = cb_block[cb_y * 8 + cb_x];
                            uint8_t cr_val = cr_block[cb_y * 8 + cb_x];

                            dst_row[px] = ycbcr_to_rgb(y_val, cb_val, cr_val);
                        }
                    }
                }
                ++mcu_count;
            }
        }
    }

    return ImageResult<Pixmap>::success(std::move(pixmap));
}

// ============================================================================
// Baseline JPEG Sovereign Encoder
// ============================================================================

namespace {

class JpegBitWriter {
public:
    explicit JpegBitWriter(size_t reserve_bytes = 0) {
        if (reserve_bytes > 0) data_.reserve(reserve_bytes);
    }

    void write_bits(uint32_t val, int n) {
        bit_buf_ |= (static_cast<uint64_t>(val & ((1ULL << n) - 1)) << (64 - bit_count_ - n));
        bit_count_ += n;
        while (bit_count_ >= 8) {
            uint8_t byte = static_cast<uint8_t>((bit_buf_ >> 56) & 0xFF);
            data_.push_back(byte);
            if (byte == 0xFF) {
                data_.push_back(0x00); // Byte stuffing
            }
            bit_buf_ <<= 8;
            bit_count_ -= 8;
        }
    }

    void flush() {
        if (bit_count_ > 0) {
            uint8_t byte = static_cast<uint8_t>((bit_buf_ >> 56) & 0xFF);
            data_.push_back(byte);
            if (byte == 0xFF) {
                data_.push_back(0x00);
            }
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

// Fast Separable 2D Forward DCT: D = T * in * T^T
void fdct_8x8(const int32_t in[64], float out[64]) noexcept {
    const auto& T = get_dct_table();
    float temp[64];

    // Pass 1: Row Pass: temp = in * T^T
    for (int r = 0; r < 8; ++r) {
        const int32_t* in_row = in + r * 8;
        float* t_row = temp + r * 8;
        for (int u = 0; u < 8; ++u) {
            const float* tu = T[u].data();
            t_row[u] = in_row[0] * tu[0] + in_row[1] * tu[1] + in_row[2] * tu[2] + in_row[3] * tu[3] +
                       in_row[4] * tu[4] + in_row[5] * tu[5] + in_row[6] * tu[6] + in_row[7] * tu[7];
        }
    }

    // Pass 2: Column Pass: out = T * temp
    for (int v = 0; v < 8; ++v) {
        float* out_row = out + v * 8;
        for (int u = 0; u < 8; ++u) {
            out_row[u] = T[v][0] * temp[0 * 8 + u] + T[v][1] * temp[1 * 8 + u] +
                         T[v][2] * temp[2 * 8 + u] + T[v][3] * temp[3 * 8 + u] +
                         T[v][4] * temp[4 * 8 + u] + T[v][5] * temp[5 * 8 + u] +
                         T[v][6] * temp[6 * 8 + u] + T[v][7] * temp[7 * 8 + u];
        }
    }
}

struct FastEncHuffman {
    std::array<uint16_t, 256> codes{};
    std::array<uint8_t, 256> lens{};

    void build(std::span<const uint8_t, 16> bits, std::span<const uint8_t> values) {
        codes.fill(0);
        lens.fill(0);
        uint16_t code = 0;
        size_t v_idx = 0;
        for (int l = 1; l <= 16; ++l) {
            for (int i = 0; i < bits[l - 1]; ++i) {
                uint8_t sym = values[v_idx++];
                codes[sym] = code++;
                lens[sym] = static_cast<uint8_t>(l);
            }
            code <<= 1;
        }
    }
};

inline int get_category(int val) noexcept {
    if (val == 0) return 0;
    return std::bit_width(static_cast<uint32_t>(std::abs(val)));
}

inline void write_diff(JpegBitWriter& writer, int diff) {
    if (diff == 0) return;
    int abs_d = std::abs(diff);
    int s = std::bit_width(static_cast<uint32_t>(abs_d));
    uint32_t val = (diff > 0) ? static_cast<uint32_t>(diff) : static_cast<uint32_t>((1U << s) - 1 + diff);
    writer.write_bits(val, s);
}

} // namespace

ImageResult<std::vector<uint8_t>> encode_jpeg(
    PixmapRef pixmap,
    int quality
) {
    uint32_t w = pixmap.width();
    uint32_t h = pixmap.height();
    if (w == 0 || h == 0) {
        return ImageResult<std::vector<uint8_t>>::fail(ImageError::InvalidDimensions);
    }

    quality = std::clamp(quality, 1, 100);
    int scale = (quality < 50) ? (5000 / quality) : (200 - quality * 2);

    std::array<uint8_t, 64> luma_qt{};
    std::array<uint8_t, 64> chroma_qt{};
    for (int i = 0; i < 64; ++i) {
        luma_qt[i] = static_cast<uint8_t>(std::clamp((STD_LUMA_QT[i] * scale + 50) / 100, 1, 255));
        chroma_qt[i] = static_cast<uint8_t>(std::clamp((STD_CHROMA_QT[i] * scale + 50) / 100, 1, 255));
    }

    FastEncHuffman dc_luma_huff;
    FastEncHuffman ac_luma_huff;
    FastEncHuffman dc_chroma_huff;
    FastEncHuffman ac_chroma_huff;

    dc_luma_huff.build(STD_DC_LUMA_BITS, STD_DC_LUMA_VALS);
    ac_luma_huff.build(STD_AC_LUMA_BITS, STD_AC_LUMA_VALS);
    dc_chroma_huff.build(STD_DC_CHROMA_BITS, STD_DC_CHROMA_VALS);
    ac_chroma_huff.build(STD_AC_CHROMA_BITS, STD_AC_CHROMA_VALS);

    std::vector<uint8_t> out;
    out.reserve(w * h + 2048);

    auto write_marker = [&](uint16_t m) {
        out.push_back(static_cast<uint8_t>((m >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(m & 0xFF));
    };

    // 1. SOI
    write_marker(0xFFD8);

    // 2. APP0 (JFIF)
    write_marker(0xFFE0);
    write_marker(16); // len
    const char jfif_id[] = "JFIF";
    out.insert(out.end(), jfif_id, jfif_id + 5);
    out.push_back(1); // v1.1
    out.push_back(1);
    out.push_back(0); // units: none
    out.push_back(0); // x-density = 1
    out.push_back(1);
    out.push_back(0); // y-density = 1
    out.push_back(1);
    out.push_back(0); // thumbnail
    out.push_back(0);

    // 3. DQT
    write_marker(0xFFDB);
    write_marker(132); // 2 + 65 + 65
    out.push_back(0x00); // Luma table 0
    for (int i = 0; i < 64; ++i) out.push_back(luma_qt[ZIGZAG[i]]);
    out.push_back(0x01); // Chroma table 1
    for (int i = 0; i < 64; ++i) out.push_back(chroma_qt[ZIGZAG[i]]);

    // 4. SOF0 (Baseline 8x8 4:4:4)
    write_marker(0xFFC0);
    write_marker(17);
    out.push_back(8); // 8-bit precision
    out.push_back(static_cast<uint8_t>((h >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(h & 0xFF));
    out.push_back(static_cast<uint8_t>((w >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(w & 0xFF));
    out.push_back(3); // 3 components
    // Y (ID 1, 1x1, QT 0)
    out.push_back(1); out.push_back(0x11); out.push_back(0);
    // Cb (ID 2, 1x1, QT 1)
    out.push_back(2); out.push_back(0x11); out.push_back(1);
    // Cr (ID 3, 1x1, QT 1)
    out.push_back(3); out.push_back(0x11); out.push_back(1);

    // 5. DHT
    auto write_dht = [&](uint8_t tc_th, std::span<const uint8_t, 16> bits, std::span<const uint8_t> vals) {
        write_marker(0xFFC4);
        write_marker(static_cast<uint16_t>(3 + 16 + vals.size()));
        out.push_back(tc_th);
        out.insert(out.end(), bits.begin(), bits.end());
        out.insert(out.end(), vals.begin(), vals.end());
    };
    write_dht(0x00, STD_DC_LUMA_BITS, STD_DC_LUMA_VALS);
    write_dht(0x10, STD_AC_LUMA_BITS, STD_AC_LUMA_VALS);
    write_dht(0x01, STD_DC_CHROMA_BITS, STD_DC_CHROMA_VALS);
    write_dht(0x11, STD_AC_CHROMA_BITS, STD_AC_CHROMA_VALS);

    // 6. SOS
    write_marker(0xFFDA);
    write_marker(12);
    out.push_back(3);
    out.push_back(1); out.push_back(0x00);
    out.push_back(2); out.push_back(0x11);
    out.push_back(3); out.push_back(0x11);
    out.push_back(0); out.push_back(63); out.push_back(0);

    // 7. Entropy Encode Scan
    std::array<float, 64> inv_luma_qt{};
    std::array<float, 64> inv_chroma_qt{};
    for (int i = 0; i < 64; ++i) {
        inv_luma_qt[i] = 1.0f / luma_qt[i];
        inv_chroma_qt[i] = 1.0f / chroma_qt[i];
    }

    JpegBitWriter writer(w * h / 2);
    int last_dc_y = 0;
    int last_dc_cb = 0;
    int last_dc_cr = 0;

    auto encode_block = [&](const int16_t block[64],
                            const FastEncHuffman& dc_huff, const FastEncHuffman& ac_huff,
                            int& last_dc) {
        // DC
        int dc_diff = block[0] - last_dc;
        last_dc = block[0];
        int dc_cat = get_category(dc_diff);
        writer.write_bits(dc_huff.codes[dc_cat], dc_huff.lens[dc_cat]);
        write_diff(writer, dc_diff);

        // AC
        int r = 0;
        for (int k = 1; k < 64; ++k) {
            int ac = block[ZIGZAG[k]];
            if (ac == 0) {
                ++r;
            } else {
                while (r > 15) {
                    writer.write_bits(ac_huff.codes[0xF0], ac_huff.lens[0xF0]); // ZRL
                    r -= 16;
                }
                int ac_cat = get_category(ac);
                uint8_t sym = static_cast<uint8_t>((r << 4) | ac_cat);
                writer.write_bits(ac_huff.codes[sym], ac_huff.lens[sym]);
                write_diff(writer, ac);
                r = 0;
            }
        }
        if (r > 0) {
            writer.write_bits(ac_huff.codes[0x00], ac_huff.lens[0x00]); // EOB
        }
    };

    uint32_t mcu_cols = (w + 7) / 8;
    uint32_t mcu_rows = (h + 7) / 8;

    int32_t y_raw[64];
    int32_t cb_raw[64];
    int32_t cr_raw[64];

    float y_dct[64];
    float cb_dct[64];
    float cr_dct[64];

    int16_t y_q[64];
    int16_t cb_q[64];
    int16_t cr_q[64];

    for (uint32_t my = 0; my < mcu_rows; ++my) {
        for (uint32_t mx = 0; mx < mcu_cols; ++mx) {
            uint32_t bx = mx * 8;
            uint32_t by = my * 8;

            for (int py = 0; py < 8; ++py) {
                uint32_t cy = std::min(by + py, h - 1);
                const auto* row = pixmap.row(cy);
                for (int px = 0; px < 8; ++px) {
                    uint32_t cx = std::min(bx + px, w - 1);
                    ColorU8 c = row[cx].demultiply();

                    // Fixed-point ITU-R BT.601 integer conversion
                    int r = c.red();
                    int g = c.green();
                    int b = c.blue();
                    int y = (19595 * r + 38470 * g + 7471 * b + 32768) >> 16;
                    int cb = (-11059 * r - 21709 * g + 32768 * b + 8421376) >> 16;
                    int cr = (32768 * r - 27439 * g - 5329 * b + 8421376) >> 16;

                    y_raw[py * 8 + px] = y - 128;
                    cb_raw[py * 8 + px] = cb - 128;
                    cr_raw[py * 8 + px] = cr - 128;
                }
            }

            fdct_8x8(y_raw, y_dct);
            fdct_8x8(cb_raw, cb_dct);
            fdct_8x8(cr_raw, cr_dct);

            for (int i = 0; i < 64; ++i) {
                y_q[i] = static_cast<int16_t>(std::round(y_dct[i] * inv_luma_qt[i]));
                cb_q[i] = static_cast<int16_t>(std::round(cb_dct[i] * inv_chroma_qt[i]));
                cr_q[i] = static_cast<int16_t>(std::round(cr_dct[i] * inv_chroma_qt[i]));
            }

            encode_block(y_q, dc_luma_huff, ac_luma_huff, last_dc_y);
            encode_block(cb_q, dc_chroma_huff, ac_chroma_huff, last_dc_cb);
            encode_block(cr_q, dc_chroma_huff, ac_chroma_huff, last_dc_cr);
        }
    }

    auto bitstream = std::move(writer).take();
    out.insert(out.end(), bitstream.begin(), bitstream.end());

    // 8. EOI
    write_marker(0xFFD9);

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

ImageResult<Pixmap> decode_jpeg_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return ImageResult<Pixmap>::fail(ImageError::IOError);

    std::streamsize size = file.tellg();
    if (size <= 0) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }

    return decode_jpeg(buffer);
}

bool save_jpeg_file(
    PixmapRef pixmap,
    const std::string& file_path,
    int quality
) {
    auto encoded = encode_jpeg(pixmap, quality);
    if (!encoded) return false;

    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(encoded->data()), encoded->size());
    return file.good();
}

} // namespace nisaba::image

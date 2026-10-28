#include "nisaba/image/jpeg.hpp"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>
#include <bit>

#if defined(__x86_64__) || defined(_M_X64)
#include <emmintrin.h>
#endif
#if defined(__ARM_NEON) || defined(__aarch64__)
#include <arm_neon.h>
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

        uint32_t code = 0;
        size_t val_idx = 0;

        for (int l = 1; l <= 16; ++l) {
            uint8_t count = bits[l - 1];
            if (count > 0) {
                valoffset[l] = static_cast<int32_t>(val_idx) - static_cast<int32_t>(code);
                for (int i = 0; i < count; ++i) {
                    if (val_idx >= values.size()) return false;
                    uint8_t sym = values[val_idx++];

                    if (l <= FAST_BITS) {
                        uint32_t step = 1U << (FAST_BITS - l);
                        uint32_t base = code << (FAST_BITS - l);
                        for (uint32_t idx = base; idx < base + step && idx < FAST_SIZE; ++idx) {
                            fast[idx] = static_cast<uint16_t>((sym << 4) | l);
                        }
                    }
                    ++code;
                }
                maxcode[l] = static_cast<int32_t>(code - 1);
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
        : base_(data.data()), ptr_(data.data()), end_(data.data() + data.size()) {
        refill();
    }

    bool has_error() const noexcept { return error_; }
    void set_error() noexcept { error_ = true; }

    inline void refill() noexcept {
        while (bit_count_ <= 48 && ptr_ < end_) {
            if (*ptr_ == 0xFF) {
                if (ptr_ + 1 < end_) {
                    uint8_t marker = ptr_[1];
                    if (marker == 0x00) {
                        ptr_ += 2;
                        bit_buf_ = (bit_buf_ << 8) | 0xFF;
                        bit_count_ += 8;
                        continue;
                    }
                    if (marker == 0xFF) {
                        ++ptr_;
                        continue;
                    }
                    // Any other marker (RST0-7, DHT, SOS, EOI, etc.): stop refilling
                    break;
                } else {
                    break;
                }
            }
            uint8_t b = *ptr_++;
            bit_buf_ = (bit_buf_ << 8) | b;
            bit_count_ += 8;
        }
    }

    void restart() noexcept {
        bit_count_ = 0;
        bit_buf_ = 0;
        while (ptr_ < end_) {
            if (*ptr_ == 0xFF) {
                if (ptr_ + 1 < end_ && ptr_[1] >= 0xD0 && ptr_[1] <= 0xD7) {
                    ptr_ += 2;
                    break;
                }
                if (ptr_ + 1 < end_ && ptr_[1] != 0x00 && ptr_[1] != 0xFF) {
                    break;
                }
            }
            ++ptr_;
        }
        refill();
    }

    size_t find_next_marker_offset() noexcept {
        // Advance past remaining entropy bytes until next non-RST marker
        while (ptr_ + 1 < end_) {
            if (ptr_[0] == 0xFF) {
                uint8_t m = ptr_[1];
                if (m == 0x00) {
                    ptr_ += 2;
                    continue;
                }
                if (m == 0xFF) {
                    ++ptr_;
                    continue;
                }
                if (m >= 0xD0 && m <= 0xD7) {
                    ptr_ += 2;
                    continue;
                }
                return static_cast<size_t>(ptr_ - base_);
            }
            ++ptr_;
        }
        return static_cast<size_t>(end_ - base_);
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
        if (n == 0) return 0;
        if (bit_count_ < n) {
            refill();
            if (bit_count_ < n) {
                uint32_t val = static_cast<uint32_t>((bit_buf_ << (n - bit_count_)) & ((1ULL << n) - 1));
                bit_count_ = 0;
                return val;
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

private:
    const uint8_t* base_{nullptr};
    const uint8_t* ptr_{nullptr};
    const uint8_t* end_{nullptr};
    uint64_t bit_buf_{0};
    int bit_count_{0};
    bool error_{false};
};

// ============================================================================
// Fast AAN / Chen Fixed-Point IDCT (13-bit precision)
// ============================================================================

#define NISABA_F2F(x) static_cast<int>(((x) * 4096.0f + 0.5f))

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
        return (x < 0) ? 0 : 255;
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

    row0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 0*8));
    row1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 1*8));
    row2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 2*8));
    row3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 3*8));
    row4 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 4*8));
    row5 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 5*8));
    row6 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 6*8));
    row7 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 7*8));

    dct_pass(bias_0, 10);

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

    __m128i p0 = _mm_packus_epi16(row0, row1);
    __m128i p1 = _mm_packus_epi16(row2, row3);
    __m128i p2 = _mm_packus_epi16(row4, row5);
    __m128i p3 = _mm_packus_epi16(row6, row7);

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

inline void run_idct_block(const int16_t coeffs[64], uint8_t out[64]) noexcept {
    // Fast DC-only check
    bool ac_zero = true;
    for (int k = 1; k < 64; ++k) {
        if (coeffs[k] != 0) {
            ac_zero = false;
            break;
        }
    }
    if (ac_zero) {
        uint8_t val = clamp_u8(((coeffs[0] + 4) >> 3) + 128);
        std::memset(out, val, 64);
        return;
    }
#if defined(__x86_64__) || defined(_M_X64)
    idct_8x8_simd(coeffs, out);
#else
    idct_8x8(coeffs, out);
#endif
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

// Apply EXIF Orientation (1..8) to a Pixmap
Pixmap apply_exif_orientation_pixmap(Pixmap&& src, uint8_t orientation) {
    if (orientation <= 1 || orientation > 8) return std::move(src);
    uint32_t w = src.width();
    uint32_t h = src.height();
    bool swap_wh = (orientation >= 5 && orientation <= 8);
    uint32_t dst_w = swap_wh ? h : w;
    uint32_t dst_h = swap_wh ? w : h;

    auto opt_dst = Pixmap::allocate(dst_w, dst_h);
    if (!opt_dst) return std::move(src);
    Pixmap dst = std::move(*opt_dst);

    const PremultipliedColorU8* s_px = src.pixels();
    PremultipliedColorU8* d_px = dst.pixels_mut();

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
            d_px[dy * dst_w + dx] = s_px[y * w + x];
        }
    }
    return dst;
}

} // namespace

// ============================================================================
// JPEG Decoder Implementation (Baseline SOF0, Extended SOF1, Progressive SOF2)
// ============================================================================

ImageResult<ImageInfo> probe_jpeg(std::span<const uint8_t> jpeg_data) {
    if (jpeg_data.size() < 4 || jpeg_data[0] != 0xFF || jpeg_data[1] != 0xD8) {
        return ImageResult<ImageInfo>::fail(ImageError::InvalidSignature);
    }

    uint8_t exif_orientation = 1;
    size_t offset = 2;
    while (offset + 4 <= jpeg_data.size()) {
        if (jpeg_data[offset] != 0xFF) {
            ++offset;
            continue;
        }
        uint8_t marker = jpeg_data[offset + 1];
        offset += 2;

        if (marker == 0xFF) {
            --offset;
            continue;
        }
        if (marker == 0xD8 || marker == 0xD9 || marker == 0x00 || (marker >= 0xD0 && marker <= 0xD7)) {
            continue;
        }

        if (offset + 2 > jpeg_data.size()) break;
        uint16_t len = (static_cast<uint16_t>(jpeg_data[offset]) << 8) | jpeg_data[offset + 1];
        if (len < 2 || offset + len > jpeg_data.size()) break;

        if (marker == 0xE1 && len > 8) { // APP1 (EXIF)
            exif_orientation = parse_exif_orientation(jpeg_data.subspan(offset + 2, len - 2));
        } else if (marker == 0xC0 || marker == 0xC1 || marker == 0xC2) {
            if (len < 8) {
                return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
            }
            ImageInfo info{};
            info.bit_depth = jpeg_data[offset + 2];
            info.height = (static_cast<uint32_t>(jpeg_data[offset + 3]) << 8) | jpeg_data[offset + 4];
            info.width = (static_cast<uint32_t>(jpeg_data[offset + 5]) << 8) | jpeg_data[offset + 6];
            info.channels = jpeg_data[offset + 7];
            info.has_alpha = false;
            info.is_progressive = (marker == 0xC2);
            info.exif_orientation = exif_orientation;
            info.format = ImageFormat::JPEG;
            if (info.channels == 1) info.color_model = ColorModel::Grayscale;
            else if (info.channels == 4) info.color_model = ColorModel::CMYK;
            else info.color_model = ColorModel::RGB;
            return ImageResult<ImageInfo>::success(info);
        }

        offset += len;
    }

    return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
}

ImageResult<Pixmap> decode_jpeg(std::span<const uint8_t> jpeg_data, DecodeOptions options) {
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
        uint32_t blocks_w{0};
        uint32_t blocks_h{0};
        uint32_t stride_blocks{0};
        std::vector<int16_t> coeffs;     // Used for Progressive / Multi-scan
        std::vector<uint8_t> pixel_plane; // Decoded component plane
    };

    uint32_t width = 0;
    uint32_t height = 0;
    bool is_progressive = false;
    bool sof_found = false;
    uint8_t exif_orientation = 1;
    int adobe_transform = -1; // -1 = not present, 0 = RGB/CMYK, 1 = YCbCr, 2 = YCCK

    std::vector<Component> components;
    std::array<std::array<uint16_t, 64>, 4> q_tables{};
    std::array<JpegHuffTable, 4> dc_huff{};
    std::array<JpegHuffTable, 4> ac_huff{};
    uint16_t restart_interval = 0;

    uint8_t max_h = 1;
    uint8_t max_v = 1;
    uint32_t mcu_w = 8;
    uint32_t mcu_h = 8;
    uint32_t mcu_cols = 0;
    uint32_t mcu_rows = 0;

    // Initialize default standard Huffman tables as fallback for streams omitting DHT
    dc_huff[0].build(STD_DC_LUMA_BITS, STD_DC_LUMA_VALS, false);
    ac_huff[0].build(STD_AC_LUMA_BITS, STD_AC_LUMA_VALS, true);
    dc_huff[1].build(STD_DC_CHROMA_BITS, STD_DC_CHROMA_VALS, false);
    ac_huff[1].build(STD_AC_CHROMA_BITS, STD_AC_CHROMA_VALS, true);

    size_t offset = 2;
    size_t scans_decoded = 0;

    while (offset + 2 <= jpeg_data.size()) {
        if (jpeg_data[offset] != 0xFF) {
            ++offset;
            continue;
        }
        uint8_t marker = jpeg_data[offset + 1];
        offset += 2;

        if (marker == 0xFF) {
            --offset;
            continue;
        }
        if (marker == 0xD8 || marker == 0x00 || (marker >= 0xD0 && marker <= 0xD7)) {
            continue;
        }
        if (marker == 0xD9) { // EOI
            break;
        }

        if (offset + 2 > jpeg_data.size()) {
            return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
        }
        uint16_t len = (static_cast<uint16_t>(jpeg_data[offset]) << 8) | jpeg_data[offset + 1];
        if (len < 2 || offset + len > jpeg_data.size()) {
            return ImageResult<Pixmap>::fail(ImageError::PrematureEndOfStream);
        }

        const uint8_t* payload = jpeg_data.data() + offset + 2;
        size_t payload_len = len - 2;

        if (marker == 0xE1 && payload_len >= 6) { // APP1 (EXIF)
            exif_orientation = parse_exif_orientation(std::span<const uint8_t>(payload, payload_len));
        } else if (marker == 0xEE && payload_len >= 12) { // APP14 (Adobe)
            if (std::memcmp(payload, "Adobe", 5) == 0) {
                adobe_transform = payload[11];
            }
        } else if (marker == 0xC0 || marker == 0xC1 || marker == 0xC2) { // SOF0, SOF1, SOF2
            if (payload_len < 6) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            is_progressive = (marker == 0xC2);
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
            max_h = 1;
            max_v = 1;
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
                max_h = std::max(max_h, components[c].h_factor);
                max_v = std::max(max_v, components[c].v_factor);
            }

            mcu_w = max_h * 8;
            mcu_h = max_v * 8;
            mcu_cols = (width + mcu_w - 1) / mcu_w;
            mcu_rows = (height + mcu_h - 1) / mcu_h;

            for (auto& comp : components) {
                comp.stride_blocks = mcu_cols * comp.h_factor;
                uint32_t total_blocks_h = mcu_rows * comp.v_factor;
                comp.blocks_w = (static_cast<uint64_t>(width) * comp.h_factor + max_h - 1) / max_h;
                comp.blocks_w = (comp.blocks_w + 7) / 8;
                comp.blocks_h = (static_cast<uint64_t>(height) * comp.v_factor + max_v - 1) / max_v;
                comp.blocks_h = (comp.blocks_h + 7) / 8;
                comp.coeffs.assign(static_cast<size_t>(comp.stride_blocks) * total_blocks_h * 64, 0);
            }
            sof_found = true;
        } else if (marker == 0xDB) { // DQT
            size_t p_off = 0;
            while (p_off < payload_len) {
                uint8_t info = payload[p_off++];
                uint8_t qt_id = info & 0x0F;
                uint8_t prec = (info >> 4) & 0x0F;
                if (qt_id >= 4) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);

                if (prec == 0) { // 8-bit DQT
                    if (p_off + 64 > payload_len) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                    for (int i = 0; i < 64; ++i) {
                        q_tables[qt_id][ZIGZAG[i]] = payload[p_off + i];
                    }
                    p_off += 64;
                } else if (prec == 1) { // 16-bit DQT
                    if (p_off + 128 > payload_len) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                    for (int i = 0; i < 64; ++i) {
                        q_tables[qt_id][ZIGZAG[i]] = static_cast<uint16_t>((payload[p_off + i * 2] << 8) | payload[p_off + i * 2 + 1]);
                    }
                    p_off += 128;
                } else {
                    return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
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
                    if (!dc_huff[table_id].build(bits, vals, false)) {
                        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                    }
                } else {
                    if (!ac_huff[table_id].build(bits, vals, true)) {
                        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                    }
                }
            }
        } else if (marker == 0xDD) { // DRI
            if (payload_len >= 2) {
                restart_interval = (static_cast<uint16_t>(payload[0]) << 8) | payload[1];
            }
        } else if (marker == 0xDA) { // SOS (Start of Scan)
            if (!sof_found) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            if (payload_len < 1) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            uint8_t num_scan_comps = payload[0];
            if (num_scan_comps == 0 || num_scan_comps > 4 || payload_len < 1 + static_cast<size_t>(num_scan_comps) * 2 + 3) {
                return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            }

            std::vector<size_t> scan_comp_indices;
            scan_comp_indices.reserve(num_scan_comps);
            for (uint8_t i = 0; i < num_scan_comps; ++i) {
                uint8_t id = payload[1 + i * 2];
                uint8_t tables = payload[1 + i * 2 + 1];
                bool found = false;
                for (size_t c = 0; c < components.size(); ++c) {
                    if (components[c].id == id) {
                        components[c].dc_table = (tables >> 4) & 0x0F;
                        components[c].ac_table = tables & 0x0F;
                        if (components[c].dc_table >= 4 || components[c].ac_table >= 4) {
                            return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                        }
                        scan_comp_indices.push_back(c);
                        found = true;
                        break;
                    }
                }
                if (!found) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            }

            const uint8_t* spec_ptr = payload + 1 + num_scan_comps * 2;
            uint8_t ss = spec_ptr[0];
            uint8_t se = spec_ptr[1];
            uint8_t ah = (spec_ptr[2] >> 4) & 0x0F;
            uint8_t al = spec_ptr[2] & 0x0F;
            if (ss > 63 || se > 63 || ss > se) {
                return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
            }

            size_t sos_data_offset = offset + len;
            JpegBitReader reader(jpeg_data.subspan(sos_data_offset));

            for (auto& comp : components) comp.dc_pred = 0;
            uint32_t eob_run = 0;

            auto decode_block_seq = [&](Component& comp, int16_t* block) -> bool {
                int s = reader.decode_symbol(dc_huff[comp.dc_table]);
                if (s < 0) return false;
                int diff = reader.read_diff(s);
                comp.dc_pred += diff;
                block[0] = static_cast<int16_t>(comp.dc_pred);

                const auto& ac_table = ac_huff[comp.ac_table];
                int k = 1;
                while (k < 64) {
                    uint32_t peek = reader.peek_bits(9);
                    int16_t fast_val = ac_table.fast_ac[peek];
                    if (fast_val) {
                        k += (fast_val >> 4) & 15;
                        if (k >= 64) break;
                        block[ZIGZAG[k]] = static_cast<int16_t>(fast_val >> 8);
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
                            block[ZIGZAG[k]] = static_cast<int16_t>(ac_diff);
                        }
                        ++k;
                    }
                }
                return true;
            };

            auto decode_block_prog_dc_first = [&](Component& comp, int16_t* block) -> bool {
                int s = reader.decode_symbol(dc_huff[comp.dc_table]);
                if (s < 0) return false;
                int diff = reader.read_diff(s);
                comp.dc_pred += diff;
                block[0] = static_cast<int16_t>(comp.dc_pred << al);
                return true;
            };

            auto decode_block_prog_dc_refine = [&](int16_t* block) -> bool {
                uint32_t bit = reader.read_bits(1);
                block[0] = static_cast<int16_t>(block[0] | (bit << al));
                return true;
            };

            auto decode_block_prog_ac_first = [&](Component& comp, int16_t* block) -> bool {
                if (eob_run > 0) {
                    --eob_run;
                    return true;
                }
                const auto& ac_table = ac_huff[comp.ac_table];
                int k = ss;
                while (k <= se) {
                    int sym = reader.decode_symbol(ac_table);
                    if (sym < 0) return false;
                    int r = (sym >> 4) & 0x0F;
                    int s = sym & 0x0F;
                    if (s == 0) {
                        if (r == 15) {
                            k += 16;
                        } else {
                            eob_run = (1U << r);
                            if (r > 0) {
                                eob_run += reader.read_bits(r);
                            }
                            --eob_run;
                            break;
                        }
                    } else {
                        k += r;
                        if (k > se) break;
                        int diff = reader.read_diff(s);
                        block[ZIGZAG[k]] = static_cast<int16_t>(diff << al);
                        ++k;
                    }
                }
                return true;
            };

            auto decode_block_prog_ac_refine = [&](Component& comp, int16_t* block) -> bool {
                const int16_t p1 = static_cast<int16_t>(1 << al);
                const int16_t m1 = static_cast<int16_t>((-1) << al);
                const auto& ac_table = ac_huff[comp.ac_table];
                int k = ss;

                if (eob_run == 0) {
                    while (k <= se) {
                        int sym = reader.decode_symbol(ac_table);
                        if (sym < 0) return false;
                        int r = (sym >> 4) & 0x0F;
                        int s = sym & 0x0F;
                        int16_t new_val = 0;

                        if (s == 0) {
                            if (r < 15) {
                                eob_run = (1U << r);
                                if (r > 0) {
                                    eob_run += reader.read_bits(r);
                                }
                                break;
                            }
                        } else if (s == 1) {
                            new_val = reader.read_bits(1) ? p1 : m1;
                        } else {
                            return false;
                        }

                        // Advance over r zero coefficients while refining non-zero coefficients
                        do {
                            int16_t* coef = &block[ZIGZAG[k]];
                            if (*coef != 0) {
                                if (reader.read_bits(1)) {
                                    if ((*coef & p1) == 0) {
                                        *coef = static_cast<int16_t>(*coef + (*coef > 0 ? p1 : m1));
                                    }
                                }
                            } else {
                                if (--r < 0) break;
                            }
                            ++k;
                        } while (k <= se);

                        if (new_val != 0 && k <= se) {
                            block[ZIGZAG[k]] = new_val;
                        }
                        ++k;
                    }
                }

                if (eob_run > 0) {
                    for (; k <= se; ++k) {
                        int16_t* coef = &block[ZIGZAG[k]];
                        if (*coef != 0) {
                            if (reader.read_bits(1)) {
                                if ((*coef & p1) == 0) {
                                    *coef = static_cast<int16_t>(*coef + (*coef > 0 ? p1 : m1));
                                }
                            }
                        }
                    }
                    --eob_run;
                }
                return true;
            };

            auto process_one_block = [&](Component& comp, int16_t* block) -> bool {
                if (!is_progressive) {
                    return decode_block_seq(comp, block);
                }
                if (ss == 0 && se == 0) {
                    if (ah == 0) return decode_block_prog_dc_first(comp, block);
                    return decode_block_prog_dc_refine(block);
                } else {
                    if (ah == 0) return decode_block_prog_ac_first(comp, block);
                    return decode_block_prog_ac_refine(comp, block);
                }
            };

            uint32_t mcu_counter = 0;
            if (num_scan_comps == 1) {
                // Non-interleaved scan (1 component only)
                Component& comp = components[scan_comp_indices[0]];
                for (uint32_t by = 0; by < comp.blocks_h; ++by) {
                    for (uint32_t bx = 0; bx < comp.blocks_w; ++bx) {
                        if (restart_interval > 0 && mcu_counter > 0 && (mcu_counter % restart_interval == 0)) {
                            reader.restart();
                            comp.dc_pred = 0;
                            eob_run = 0;
                        }
                        int16_t* block = comp.coeffs.data() + (static_cast<size_t>(by) * comp.stride_blocks + bx) * 64;
                        if (!process_one_block(comp, block)) {
                            return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                        }
                        ++mcu_counter;
                    }
                }
            } else {
                // Interleaved MCU scan
                for (uint32_t my = 0; my < mcu_rows; ++my) {
                    for (uint32_t mx = 0; mx < mcu_cols; ++mx) {
                        if (restart_interval > 0 && mcu_counter > 0 && (mcu_counter % restart_interval == 0)) {
                            reader.restart();
                            for (auto& comp : components) comp.dc_pred = 0;
                            eob_run = 0;
                        }
                        for (size_t c_idx : scan_comp_indices) {
                            Component& comp = components[c_idx];
                            for (uint8_t vy = 0; vy < comp.v_factor; ++vy) {
                                for (uint8_t hx = 0; hx < comp.h_factor; ++hx) {
                                    uint32_t bx = mx * comp.h_factor + hx;
                                    uint32_t by = my * comp.v_factor + vy;
                                    int16_t* block = comp.coeffs.data() + (static_cast<size_t>(by) * comp.stride_blocks + bx) * 64;
                                    if (!process_one_block(comp, block)) {
                                        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
                                    }
                                }
                            }
                        }
                        ++mcu_counter;
                    }
                }
            }

            ++scans_decoded;
            offset = sos_data_offset + reader.find_next_marker_offset();
            continue;
        }

        offset += len;
    }

    if (!sof_found || scans_decoded == 0 || width == 0 || height == 0 || components.empty()) {
        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    }

    auto maybe_pixmap = Pixmap::allocate(width, height);
    if (!maybe_pixmap) return ImageResult<Pixmap>::fail(ImageError::OutOfMemory);
    Pixmap pixmap = std::move(*maybe_pixmap);
    PremultipliedColorU8* pix_base = pixmap.pixels_mut();

    // Perform Dequantization + IDCT for each component into its plane
    for (auto& comp : components) {
        uint32_t plane_w = comp.stride_blocks * 8;
        uint32_t plane_h = mcu_rows * comp.v_factor * 8;
        comp.pixel_plane.resize(static_cast<size_t>(plane_w) * plane_h);

        const auto& qt = q_tables[comp.qt_id];
        alignas(16) int16_t dq_block[64];
        alignas(16) uint8_t idct_out[64];

        uint32_t total_by = mcu_rows * comp.v_factor;
        for (uint32_t by = 0; by < total_by; ++by) {
            for (uint32_t bx = 0; bx < comp.stride_blocks; ++bx) {
                const int16_t* src_b = comp.coeffs.data() + (static_cast<size_t>(by) * comp.stride_blocks + bx) * 64;
                for (int i = 0; i < 64; ++i) {
                    dq_block[i] = static_cast<int16_t>(src_b[i] * qt[i]);
                }
                run_idct_block(dq_block, idct_out);

                uint8_t* dst_base = comp.pixel_plane.data() + (static_cast<size_t>(by) * 8) * plane_w + bx * 8;
                for (int r = 0; r < 8; ++r) {
                    std::memcpy(dst_base + r * plane_w, idct_out + r * 8, 8);
                }
            }
        }
        // Free coefficient buffer early
        comp.coeffs.clear();
        comp.coeffs.shrink_to_fit();
    }

    // Helper to sample component plane with bilinear chroma upsampling
    auto sample_plane_bilinear = [&](const Component& comp, uint32_t x, uint32_t y) noexcept -> uint8_t {
        uint32_t plane_w = comp.stride_blocks * 8;
        if (comp.h_factor == max_h && comp.v_factor == max_v) {
            return comp.pixel_plane[static_cast<size_t>(y) * plane_w + x];
        }
        uint32_t comp_w = (width * comp.h_factor + max_h - 1) / max_h;
        uint32_t comp_h = (height * comp.v_factor + max_v - 1) / max_v;
        if (comp_w == 0) comp_w = 1;
        if (comp_h == 0) comp_h = 1;

        // Triangle/bilinear filter ([3/4, 1/4]) for 2x subsampling
        if (max_h == 2 * comp.h_factor && (max_v == 2 * comp.v_factor || max_v == comp.v_factor)) {
            int cx0 = static_cast<int>(x >> 1);
            int cx1 = cx0 + ((x & 1) ? 1 : -1);
            cx0 = std::clamp(cx0, 0, static_cast<int>(comp_w) - 1);
            cx1 = std::clamp(cx1, 0, static_cast<int>(comp_w) - 1);

            if (max_v == 2 * comp.v_factor) {
                int cy0 = static_cast<int>(y >> 1);
                int cy1 = cy0 + ((y & 1) ? 1 : -1);
                cy0 = std::clamp(cy0, 0, static_cast<int>(comp_h) - 1);
                cy1 = std::clamp(cy1, 0, static_cast<int>(comp_h) - 1);

                const uint8_t* row0 = comp.pixel_plane.data() + static_cast<size_t>(cy0) * plane_w;
                const uint8_t* row1 = comp.pixel_plane.data() + static_cast<size_t>(cy1) * plane_w;
                int p00 = row0[cx0];
                int p10 = row0[cx1];
                int p01 = row1[cx0];
                int p11 = row1[cx1];
                return static_cast<uint8_t>((9 * p00 + 3 * p10 + 3 * p01 + p11 + 8) >> 4);
            } else {
                const uint8_t* row0 = comp.pixel_plane.data() + static_cast<size_t>(y) * plane_w;
                return static_cast<uint8_t>((3 * row0[cx0] + row0[cx1] + 2) >> 2);
            }
        }

        uint32_t sx = (x * comp.h_factor) / max_h;
        uint32_t sy = (y * comp.v_factor) / max_v;
        return comp.pixel_plane[static_cast<size_t>(sy) * plane_w + sx];
    };

    if (components.size() == 1) {
        // Grayscale
        uint32_t plane_w = components[0].stride_blocks * 8;
        const uint8_t* y_plane = components[0].pixel_plane.data();
        for (uint32_t y = 0; y < height; ++y) {
            const uint8_t* src_row = y_plane + static_cast<size_t>(y) * plane_w;
            PremultipliedColorU8* dst_row = pix_base + static_cast<size_t>(y) * width;
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t val = src_row[x];
                dst_row[x] = PremultipliedColorU8(val, val, val, 255);
            }
        }
    } else if (components.size() == 3) {
        bool is_rgb = (adobe_transform == 0) ||
                      (components[0].id == 'R' && components[1].id == 'G' && components[2].id == 'B');

        if (!is_rgb && components[0].h_factor == 1 && components[0].v_factor == 1 &&
            components[1].h_factor == 1 && components[1].v_factor == 1 &&
            components[2].h_factor == 1 && components[2].v_factor == 1) {
            // Fast path for 4:4:4 YCbCr
            uint32_t plane_w = components[0].stride_blocks * 8;
            for (uint32_t y = 0; y < height; ++y) {
                const uint8_t* y_row = components[0].pixel_plane.data() + static_cast<size_t>(y) * plane_w;
                const uint8_t* cb_row = components[1].pixel_plane.data() + static_cast<size_t>(y) * plane_w;
                const uint8_t* cr_row = components[2].pixel_plane.data() + static_cast<size_t>(y) * plane_w;
                PremultipliedColorU8* dst_row = pix_base + static_cast<size_t>(y) * width;
                for (uint32_t x = 0; x < width; ++x) {
                    dst_row[x] = ycbcr_to_rgb(y_row[x], cb_row[x], cr_row[x]);
                }
            }
        } else {
            uint32_t y_plane_w = components[0].stride_blocks * 8;
            for (uint32_t y = 0; y < height; ++y) {
                PremultipliedColorU8* dst_row = pix_base + static_cast<size_t>(y) * width;
                const uint8_t* y_row = (components[0].h_factor == max_h && components[0].v_factor == max_v)
                    ? (components[0].pixel_plane.data() + static_cast<size_t>(y) * y_plane_w)
                    : nullptr;
                for (uint32_t x = 0; x < width; ++x) {
                    uint8_t c0 = y_row ? y_row[x] : sample_plane_bilinear(components[0], x, y);
                    uint8_t c1 = sample_plane_bilinear(components[1], x, y);
                    uint8_t c2 = sample_plane_bilinear(components[2], x, y);
                    if (is_rgb) {
                        dst_row[x] = PremultipliedColorU8(c0, c1, c2, 255);
                    } else {
                        dst_row[x] = ycbcr_to_rgb(c0, c1, c2);
                    }
                }
            }
        }
    } else if (components.size() == 4) {
        // CMYK or YCCK
        bool is_ycck = (adobe_transform == 2);
        for (uint32_t y = 0; y < height; ++y) {
            PremultipliedColorU8* dst_row = pix_base + static_cast<size_t>(y) * width;
            for (uint32_t x = 0; x < width; ++x) {
                uint8_t c0 = sample_plane_bilinear(components[0], x, y);
                uint8_t c1 = sample_plane_bilinear(components[1], x, y);
                uint8_t c2 = sample_plane_bilinear(components[2], x, y);
                uint8_t k  = sample_plane_bilinear(components[3], x, y);
                uint8_t r, g, b;
                if (is_ycck) {
                    auto rgb = ycbcr_to_rgb(c0, c1, c2);
                    r = static_cast<uint8_t>(( (255 - rgb.red())   * k + 127) / 255);
                    g = static_cast<uint8_t>(( (255 - rgb.green()) * k + 127) / 255);
                    b = static_cast<uint8_t>(( (255 - rgb.blue())  * k + 127) / 255);
                } else {
                    r = static_cast<uint8_t>((c0 * k + 127) / 255);
                    g = static_cast<uint8_t>((c1 * k + 127) / 255);
                    b = static_cast<uint8_t>((c2 * k + 127) / 255);
                }
                dst_row[x] = PremultipliedColorU8(r, g, b, 255);
            }
        }
    }

    if (options.apply_exif_orientation && exif_orientation > 1) {
        pixmap = apply_exif_orientation_pixmap(std::move(pixmap), exif_orientation);
    }

    return ImageResult<Pixmap>::success(std::move(pixmap));
}

// ============================================================================
// Baseline & Progressive JPEG Sovereign Encoder
// ============================================================================

namespace {

class JpegBitWriter {
public:
    explicit JpegBitWriter(size_t reserve_bytes = 0) {
        if (reserve_bytes > 0) data_.reserve(reserve_bytes);
    }

    void write_bits(uint32_t val, int n) {
        if (n <= 0) return;
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
            uint8_t pad_mask = static_cast<uint8_t>((1U << (8 - bit_count_)) - 1);
            uint8_t byte = static_cast<uint8_t>(((bit_buf_ >> 56) & 0xFF) | pad_mask);
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

void fdct_8x8(const int32_t in[64], float out[64]) noexcept {
    const auto& T = get_dct_table();
    float temp[64];

    for (int r = 0; r < 8; ++r) {
        const int32_t* in_row = in + r * 8;
        float* t_row = temp + r * 8;
        for (int u = 0; u < 8; ++u) {
            const float* tu = T[u].data();
            t_row[u] = in_row[0] * tu[0] + in_row[1] * tu[1] + in_row[2] * tu[2] + in_row[3] * tu[3] +
                       in_row[4] * tu[4] + in_row[5] * tu[5] + in_row[6] * tu[6] + in_row[7] * tu[7];
        }
    }

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
    int quality,
    bool progressive
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
    write_marker(16);
    const char jfif_id[] = "JFIF";
    out.insert(out.end(), jfif_id, jfif_id + 5);
    out.push_back(1);
    out.push_back(1);
    out.push_back(0);
    out.push_back(0);
    out.push_back(1);
    out.push_back(0);
    out.push_back(1);
    out.push_back(0);
    out.push_back(0);

    // 3. DQT
    write_marker(0xFFDB);
    write_marker(132);
    out.push_back(0x00);
    for (int i = 0; i < 64; ++i) out.push_back(luma_qt[ZIGZAG[i]]);
    out.push_back(0x01);
    for (int i = 0; i < 64; ++i) out.push_back(chroma_qt[ZIGZAG[i]]);

    // 4. SOF0 (Baseline) or SOF2 (Progressive)
    write_marker(progressive ? 0xFFC2 : 0xFFC0);
    write_marker(17);
    out.push_back(8);
    out.push_back(static_cast<uint8_t>((h >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(h & 0xFF));
    out.push_back(static_cast<uint8_t>((w >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(w & 0xFF));
    out.push_back(3);
    out.push_back(1); out.push_back(0x11); out.push_back(0);
    out.push_back(2); out.push_back(0x11); out.push_back(1);
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

    std::array<float, 64> inv_luma_qt{};
    std::array<float, 64> inv_chroma_qt{};
    for (int i = 0; i < 64; ++i) {
        inv_luma_qt[i] = 1.0f / luma_qt[i];
        inv_chroma_qt[i] = 1.0f / chroma_qt[i];
    }

    uint32_t mcu_cols = (w + 7) / 8;
    uint32_t mcu_rows = (h + 7) / 8;
    size_t total_blocks = static_cast<size_t>(mcu_cols) * mcu_rows;

    if (!progressive) {
        // Baseline Single-Scan SOS
        write_marker(0xFFDA);
        write_marker(12);
        out.push_back(3);
        out.push_back(1); out.push_back(0x00);
        out.push_back(2); out.push_back(0x11);
        out.push_back(3); out.push_back(0x11);
        out.push_back(0); out.push_back(63); out.push_back(0);

        JpegBitWriter writer(w * h / 2);
        int last_dc_y = 0, last_dc_cb = 0, last_dc_cr = 0;

        auto encode_block = [&](const int16_t block[64],
                                const FastEncHuffman& dc_huff, const FastEncHuffman& ac_huff,
                                int& last_dc) {
            int dc_diff = block[0] - last_dc;
            last_dc = block[0];
            int dc_cat = get_category(dc_diff);
            writer.write_bits(dc_huff.codes[dc_cat], dc_huff.lens[dc_cat]);
            write_diff(writer, dc_diff);

            int r = 0;
            for (int k = 1; k < 64; ++k) {
                int ac = block[ZIGZAG[k]];
                if (ac == 0) {
                    ++r;
                } else {
                    while (r > 15) {
                        writer.write_bits(ac_huff.codes[0xF0], ac_huff.lens[0xF0]);
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
                writer.write_bits(ac_huff.codes[0x00], ac_huff.lens[0x00]);
            }
        };

        int32_t y_raw[64], cb_raw[64], cr_raw[64];
        float y_dct[64], cb_dct[64], cr_dct[64];
        int16_t y_q[64], cb_q[64], cr_q[64];

        for (uint32_t my = 0; my < mcu_rows; ++my) {
            for (uint32_t mx = 0; mx < mcu_cols; ++mx) {
                uint32_t bx = mx * 8, by = my * 8;
                for (int py = 0; py < 8; ++py) {
                    uint32_t cy = std::min(by + py, h - 1);
                    const auto* row = pixmap.row(cy);
                    for (int px = 0; px < 8; ++px) {
                        uint32_t cx = std::min(bx + px, w - 1);
                        ColorU8 c = row[cx].demultiply();
                        int r = c.red(), g = c.green(), b = c.blue();
                        int y  = ( 19595 * r + 38470 * g +  7471 * b + 32768) >> 16;
                        int cb = (-11059 * r - 21709 * g + 32768 * b + 8421376) >> 16;
                        int cr = ( 32768 * r - 27439 * g -  5329 * b + 8421376) >> 16;
                        y_raw[py * 8 + px]  = y - 128;
                        cb_raw[py * 8 + px] = cb - 128;
                        cr_raw[py * 8 + px] = cr - 128;
                    }
                }
                fdct_8x8(y_raw, y_dct);
                fdct_8x8(cb_raw, cb_dct);
                fdct_8x8(cr_raw, cr_dct);
                for (int i = 0; i < 64; ++i) {
                    y_q[i]  = static_cast<int16_t>(std::round(y_dct[i]  * inv_luma_qt[i]));
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
    } else {
        // Multi-Scan Progressive JPEG (SOF2)
        // Store all quantized blocks for Y, Cb, Cr
        std::vector<std::array<int16_t, 64>> all_y(total_blocks);
        std::vector<std::array<int16_t, 64>> all_cb(total_blocks);
        std::vector<std::array<int16_t, 64>> all_cr(total_blocks);

        int32_t y_raw[64], cb_raw[64], cr_raw[64];
        float y_dct[64], cb_dct[64], cr_dct[64];

        for (uint32_t my = 0; my < mcu_rows; ++my) {
            for (uint32_t mx = 0; mx < mcu_cols; ++mx) {
                size_t b_idx = static_cast<size_t>(my) * mcu_cols + mx;
                uint32_t bx = mx * 8, by = my * 8;
                for (int py = 0; py < 8; ++py) {
                    uint32_t cy = std::min(by + py, h - 1);
                    const auto* row = pixmap.row(cy);
                    for (int px = 0; px < 8; ++px) {
                        uint32_t cx = std::min(bx + px, w - 1);
                        ColorU8 c = row[cx].demultiply();
                        int r = c.red(), g = c.green(), b = c.blue();
                        int y  = ( 19595 * r + 38470 * g +  7471 * b + 32768) >> 16;
                        int cb = (-11059 * r - 21709 * g + 32768 * b + 8421376) >> 16;
                        int cr = ( 32768 * r - 27439 * g -  5329 * b + 8421376) >> 16;
                        y_raw[py * 8 + px]  = y - 128;
                        cb_raw[py * 8 + px] = cb - 128;
                        cr_raw[py * 8 + px] = cr - 128;
                    }
                }
                fdct_8x8(y_raw, y_dct);
                fdct_8x8(cb_raw, cb_dct);
                fdct_8x8(cr_raw, cr_dct);
                for (int i = 0; i < 64; ++i) {
                    all_y[b_idx][i]  = static_cast<int16_t>(std::round(y_dct[i]  * inv_luma_qt[i]));
                    all_cb[b_idx][i] = static_cast<int16_t>(std::round(cb_dct[i] * inv_chroma_qt[i]));
                    all_cr[b_idx][i] = static_cast<int16_t>(std::round(cr_dct[i] * inv_chroma_qt[i]));
                }
            }
        }

        // Scan 1: Interleaved DC First (Ss=0, Se=0, Ah=0, Al=1)
        {
            write_marker(0xFFDA);
            write_marker(12);
            out.push_back(3);
            out.push_back(1); out.push_back(0x00);
            out.push_back(2); out.push_back(0x11);
            out.push_back(3); out.push_back(0x11);
            out.push_back(0); out.push_back(0); out.push_back(0x01); // Al=1

            JpegBitWriter writer(total_blocks * 2);
            int last_dc_y = 0, last_dc_cb = 0, last_dc_cr = 0;
            auto enc_dc = [&](int16_t dc_val, const FastEncHuffman& huff, int& last_dc) {
                int shifted = dc_val >> 1;
                int diff = shifted - last_dc;
                last_dc = shifted;
                int cat = get_category(diff);
                writer.write_bits(huff.codes[cat], huff.lens[cat]);
                write_diff(writer, diff);
            };
            for (size_t i = 0; i < total_blocks; ++i) {
                enc_dc(all_y[i][0], dc_luma_huff, last_dc_y);
                enc_dc(all_cb[i][0], dc_chroma_huff, last_dc_cb);
                enc_dc(all_cr[i][0], dc_chroma_huff, last_dc_cr);
            }
            auto bs = std::move(writer).take();
            out.insert(out.end(), bs.begin(), bs.end());
        }

        // Helper for AC Band Scan (Ss..Se, Ah=0, Al=0)
        auto emit_ac_scan = [&](uint8_t comp_id, const std::vector<std::array<int16_t, 64>>& blocks,
                                uint8_t huff_sel, const FastEncHuffman& ac_huff, uint8_t ss, uint8_t se) {
            write_marker(0xFFDA);
            write_marker(8);
            out.push_back(1);
            out.push_back(comp_id);
            out.push_back(huff_sel);
            out.push_back(ss);
            out.push_back(se);
            out.push_back(0x00);

            JpegBitWriter writer(total_blocks * 4);
            for (size_t i = 0; i < total_blocks; ++i) {
                int r = 0;
                for (int k = ss; k <= se; ++k) {
                    int ac = blocks[i][ZIGZAG[k]];
                    if (ac == 0) {
                        ++r;
                    } else {
                        while (r > 15) {
                            writer.write_bits(ac_huff.codes[0xF0], ac_huff.lens[0xF0]);
                            r -= 16;
                        }
                        int cat = get_category(ac);
                        uint8_t sym = static_cast<uint8_t>((r << 4) | cat);
                        writer.write_bits(ac_huff.codes[sym], ac_huff.lens[sym]);
                        write_diff(writer, ac);
                        r = 0;
                    }
                }
                if (r > 0) {
                    writer.write_bits(ac_huff.codes[0x00], ac_huff.lens[0x00]); // EOB0
                }
            }
            auto bs = std::move(writer).take();
            out.insert(out.end(), bs.begin(), bs.end());
        };

        // Scan 2: Y AC Low frequency (1..5)
        emit_ac_scan(1, all_y, 0x00, ac_luma_huff, 1, 5);
        // Scan 3: Cb AC (1..63)
        emit_ac_scan(2, all_cb, 0x11, ac_chroma_huff, 1, 63);
        // Scan 4: Cr AC (1..63)
        emit_ac_scan(3, all_cr, 0x11, ac_chroma_huff, 1, 63);
        // Scan 5: Y AC High frequency (6..63)
        emit_ac_scan(1, all_y, 0x00, ac_luma_huff, 6, 63);

        // Scan 6: Interleaved DC Refine (Ss=0, Se=0, Ah=1, Al=0)
        {
            write_marker(0xFFDA);
            write_marker(12);
            out.push_back(3);
            out.push_back(1); out.push_back(0x00);
            out.push_back(2); out.push_back(0x11);
            out.push_back(3); out.push_back(0x11);
            out.push_back(0); out.push_back(0); out.push_back(0x10); // Ah=1, Al=0

            JpegBitWriter writer(total_blocks);
            for (size_t i = 0; i < total_blocks; ++i) {
                writer.write_bits(all_y[i][0] & 1, 1);
                writer.write_bits(all_cb[i][0] & 1, 1);
                writer.write_bits(all_cr[i][0] & 1, 1);
            }
            auto bs = std::move(writer).take();
            out.insert(out.end(), bs.begin(), bs.end());
        }
    }

    // EOI
    write_marker(0xFFD9);

    return ImageResult<std::vector<uint8_t>>::success(std::move(out));
}

ImageResult<Pixmap> decode_jpeg_file(const std::string& file_path, DecodeOptions options) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return ImageResult<Pixmap>::fail(ImageError::IOError);

    std::streamsize size = file.tellg();
    if (size <= 0) return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }

    return decode_jpeg(buffer, options);
}

bool save_jpeg_file(
    PixmapRef pixmap,
    const std::string& file_path,
    int quality,
    bool progressive
) {
    auto encoded = encode_jpeg(pixmap, quality, progressive);
    if (!encoded) return false;

    std::ofstream file(file_path, std::ios::binary);
    if (!file.is_open()) return false;

    file.write(reinterpret_cast<const char*>(encoded->data()), encoded->size());
    return file.good();
}

} // namespace nisaba::image

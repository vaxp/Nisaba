#include "nisaba/pdf/pdf_crypto.hpp"
#include <cstring>
#include <algorithm>

namespace nisaba::pdf {

namespace {

// =============================================================================
// 1. Sovereign MD5 Implementation (RFC 1321)
// =============================================================================

inline uint32_t f_md5(uint32_t x, uint32_t y, uint32_t z) { return (x & y) | (~x & z); }
inline uint32_t g_md5(uint32_t x, uint32_t y, uint32_t z) { return (x & z) | (y & ~z); }
inline uint32_t h_md5(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
inline uint32_t i_md5(uint32_t x, uint32_t y, uint32_t z) { return y ^ (x | ~z); }

inline uint32_t rotl32(uint32_t x, int n) {
    return (x << n) | (x >> (32 - n));
}

#define MD5_STEP(f, a, b, c, d, x, s, ac) \
    (a) += f((b), (c), (d)) + (x) + static_cast<uint32_t>(ac); \
    (a) = rotl32((a), (s)); \
    (a) += (b);

} // namespace

std::array<uint8_t, 16> md5_hash(std::span<const uint8_t> data) {
    uint32_t a = 0x67452301;
    uint32_t b = 0xefcdab89;
    uint32_t c = 0x98badcfe;
    uint32_t d = 0x10325476;

    uint64_t bit_len = static_cast<uint64_t>(data.size()) * 8;

    std::vector<uint8_t> msg(data.begin(), data.end());
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }
    for (int i = 0; i < 8; ++i) {
        msg.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF));
    }

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t m[16];
        for (int i = 0; i < 16; ++i) {
            size_t idx = chunk + i * 4;
            m[i] = static_cast<uint32_t>(msg[idx]) |
                   (static_cast<uint32_t>(msg[idx + 1]) << 8) |
                   (static_cast<uint32_t>(msg[idx + 2]) << 16) |
                   (static_cast<uint32_t>(msg[idx + 3]) << 24);
        }

        uint32_t aa = a, bb = b, cc = c, dd = d;

        MD5_STEP(f_md5, a, b, c, d, m[0], 7, 0xd76aa478);
        MD5_STEP(f_md5, d, a, b, c, m[1], 12, 0xe8c7b756);
        MD5_STEP(f_md5, c, d, a, b, m[2], 17, 0x242070db);
        MD5_STEP(f_md5, b, c, d, a, m[3], 22, 0xc1bdceee);
        MD5_STEP(f_md5, a, b, c, d, m[4], 7, 0xf57c0faf);
        MD5_STEP(f_md5, d, a, b, c, m[5], 12, 0x4787c62a);
        MD5_STEP(f_md5, c, d, a, b, m[6], 17, 0xa8304613);
        MD5_STEP(f_md5, b, c, d, a, m[7], 22, 0xfd469501);
        MD5_STEP(f_md5, a, b, c, d, m[8], 7, 0x698098d8);
        MD5_STEP(f_md5, d, a, b, c, m[9], 12, 0x8b44f7af);
        MD5_STEP(f_md5, c, d, a, b, m[10], 17, 0xffff5bb1);
        MD5_STEP(f_md5, b, c, d, a, m[11], 22, 0x895cd7be);
        MD5_STEP(f_md5, a, b, c, d, m[12], 7, 0x6b901122);
        MD5_STEP(f_md5, d, a, b, c, m[13], 12, 0xfd987193);
        MD5_STEP(f_md5, c, d, a, b, m[14], 17, 0xa679438e);
        MD5_STEP(f_md5, b, c, d, a, m[15], 22, 0x49b40821);

        MD5_STEP(g_md5, a, b, c, d, m[1], 5, 0xf61e2562);
        MD5_STEP(g_md5, d, a, b, c, m[6], 9, 0xc040b340);
        MD5_STEP(g_md5, c, d, a, b, m[11], 14, 0x265e5a51);
        MD5_STEP(g_md5, b, c, d, a, m[0], 20, 0xe9b6c7aa);
        MD5_STEP(g_md5, a, b, c, d, m[5], 5, 0xd62f105d);
        MD5_STEP(g_md5, d, a, b, c, m[10], 9, 0x02441453);
        MD5_STEP(g_md5, c, d, a, b, m[15], 14, 0xd8a1e681);
        MD5_STEP(g_md5, b, c, d, a, m[4], 20, 0xe7d3fbc8);
        MD5_STEP(g_md5, a, b, c, d, m[9], 5, 0x21e1cde6);
        MD5_STEP(g_md5, d, a, b, c, m[14], 9, 0xc33707d6);
        MD5_STEP(g_md5, c, d, a, b, m[3], 14, 0xf4d50d87);
        MD5_STEP(g_md5, b, c, d, a, m[8], 20, 0x455a14ed);
        MD5_STEP(g_md5, a, b, c, d, m[13], 5, 0xa9e3e905);
        MD5_STEP(g_md5, d, a, b, c, m[2], 9, 0xfcefa3f8);
        MD5_STEP(g_md5, c, d, a, b, m[7], 14, 0x676f02d9);
        MD5_STEP(g_md5, b, c, d, a, m[12], 20, 0x8d2a4c8a);

        MD5_STEP(h_md5, a, b, c, d, m[5], 4, 0xfffa3942);
        MD5_STEP(h_md5, d, a, b, c, m[8], 11, 0x8771f681);
        MD5_STEP(h_md5, c, d, a, b, m[11], 16, 0x6d9d6122);
        MD5_STEP(h_md5, b, c, d, a, m[14], 23, 0xfde5380c);
        MD5_STEP(h_md5, a, b, c, d, m[1], 4, 0xa4beea44);
        MD5_STEP(h_md5, d, a, b, c, m[4], 11, 0x4bdecfa9);
        MD5_STEP(h_md5, c, d, a, b, m[7], 16, 0xf6bb4b60);
        MD5_STEP(h_md5, b, c, d, a, m[10], 23, 0xbebfbc70);
        MD5_STEP(h_md5, a, b, c, d, m[13], 4, 0x289b7ec6);
        MD5_STEP(h_md5, d, a, b, c, m[0], 11, 0xeaa127fa);
        MD5_STEP(h_md5, c, d, a, b, m[3], 16, 0xd4ef3085);
        MD5_STEP(h_md5, b, c, d, a, m[6], 23, 0x04881d05);
        MD5_STEP(h_md5, a, b, c, d, m[9], 4, 0xd9d4d039);
        MD5_STEP(h_md5, d, a, b, c, m[12], 11, 0xe6db99e5);
        MD5_STEP(h_md5, c, d, a, b, m[15], 16, 0x1fa27cf8);
        MD5_STEP(h_md5, b, c, d, a, m[2], 23, 0xc4ac5665);

        MD5_STEP(i_md5, a, b, c, d, m[0], 6, 0xf4292244);
        MD5_STEP(i_md5, d, a, b, c, m[7], 10, 0x432aff97);
        MD5_STEP(i_md5, c, d, a, b, m[14], 15, 0xab9423a7);
        MD5_STEP(i_md5, b, c, d, a, m[5], 21, 0xfc93a039);
        MD5_STEP(i_md5, a, b, c, d, m[12], 6, 0x655b59c3);
        MD5_STEP(i_md5, d, a, b, c, m[3], 10, 0x8f0ccc92);
        MD5_STEP(i_md5, c, d, a, b, m[10], 15, 0xffeff47d);
        MD5_STEP(i_md5, b, c, d, a, m[1], 21, 0x85845dd1);
        MD5_STEP(i_md5, a, b, c, d, m[8], 6, 0x6fa87e4f);
        MD5_STEP(i_md5, d, a, b, c, m[15], 10, 0xfe2ce6e0);
        MD5_STEP(i_md5, c, d, a, b, m[6], 15, 0xa3014314);
        MD5_STEP(i_md5, b, c, d, a, m[13], 21, 0x4e0811a1);
        MD5_STEP(i_md5, a, b, c, d, m[4], 6, 0xf7537e82);
        MD5_STEP(i_md5, d, a, b, c, m[11], 10, 0xbd3af235);
        MD5_STEP(i_md5, c, d, a, b, m[2], 15, 0x2ad7d2bb);
        MD5_STEP(i_md5, b, c, d, a, m[9], 21, 0xeb86d391);

        a += aa;
        b += bb;
        c += cc;
        d += dd;
    }

    std::array<uint8_t, 16> out;
    for (int i = 0; i < 4; ++i) {
        out[i] = (a >> (i * 8)) & 0xFF;
        out[i + 4] = (b >> (i * 8)) & 0xFF;
        out[i + 8] = (c >> (i * 8)) & 0xFF;
        out[i + 12] = (d >> (i * 8)) & 0xFF;
    }
    return out;
}

// =============================================================================
// 2. Sovereign SHA-256 Implementation (FIPS 180-4)
// =============================================================================

namespace {

inline uint32_t rotr32(uint32_t x, int n) {
    return (x >> n) | (x << (32 - n));
}

static const uint32_t kSha256K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

} // namespace

std::array<uint8_t, 32> sha256_hash(std::span<const uint8_t> data) {
    uint32_t h[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    uint64_t bit_len = static_cast<uint64_t>(data.size()) * 8;

    std::vector<uint8_t> msg(data.begin(), data.end());
    msg.push_back(0x80);
    while ((msg.size() % 64) != 56) {
        msg.push_back(0x00);
    }
    for (int i = 7; i >= 0; --i) {
        msg.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF));
    }

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i) {
            size_t idx = chunk + i * 4;
            w[i] = (static_cast<uint32_t>(msg[idx]) << 24) |
                   (static_cast<uint32_t>(msg[idx + 1]) << 16) |
                   (static_cast<uint32_t>(msg[idx + 2]) << 8) |
                   static_cast<uint32_t>(msg[idx + 3]);
        }
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], h_val = h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t s1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t temp1 = h_val + s1 + ch + kSha256K[i] + w[i];
            uint32_t s0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = s0 + maj;

            h_val = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += h_val;
    }

    std::array<uint8_t, 32> out;
    for (int i = 0; i < 8; ++i) {
        out[i * 4] = (h[i] >> 24) & 0xFF;
        out[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        out[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        out[i * 4 + 3] = h[i] & 0xFF;
    }
    return out;
}

// =============================================================================
// 3. Sovereign RC4 Cipher (Rivest Cipher 4)
// =============================================================================

std::vector<uint8_t> rc4_crypt(
    std::span<const uint8_t> key,
    std::span<const uint8_t> input
) {
    if (key.empty()) return std::vector<uint8_t>(input.begin(), input.end());

    uint8_t s[256];
    for (int i = 0; i < 256; ++i) {
        s[i] = static_cast<uint8_t>(i);
    }

    uint8_t j = 0;
    for (int i = 0; i < 256; ++i) {
        j = j + s[i] + key[i % key.size()];
        std::swap(s[i], s[j]);
    }

    std::vector<uint8_t> out(input.size());
    uint8_t i = 0;
    j = 0;
    for (size_t n = 0; n < input.size(); ++n) {
        i++;
        j += s[i];
        std::swap(s[i], s[j]);
        uint8_t k = s[static_cast<uint8_t>(s[i] + s[j])];
        out[n] = input[n] ^ k;
    }

    return out;
}

// =============================================================================
// 4. Sovereign AES Cipher (128-bit & 256-bit CBC mode)
// =============================================================================

namespace {

static const uint8_t kSBox[256] = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static const uint8_t kInvSBox[256] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static const uint8_t kRcon[11] = {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
};

inline uint8_t gmul(uint8_t a, uint8_t b) {
    uint8_t p = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) p ^= a;
        bool hi_bit = (a & 0x80) != 0;
        a <<= 1;
        if (hi_bit) a ^= 0x1b;
        b >>= 1;
    }
    return p;
}

void aes_key_expand(std::span<const uint8_t> key, std::vector<uint32_t>& round_keys, int& nr) {
    size_t nk = key.size() / 4;
    nr = (nk == 4) ? 10 : ((nk == 6) ? 12 : 14);

    round_keys.resize(4 * (nr + 1));
    for (size_t i = 0; i < nk; ++i) {
        round_keys[i] = (static_cast<uint32_t>(key[4 * i]) << 24) |
                        (static_cast<uint32_t>(key[4 * i + 1]) << 16) |
                        (static_cast<uint32_t>(key[4 * i + 2]) << 8) |
                        static_cast<uint32_t>(key[4 * i + 3]);
    }

    for (size_t i = nk; i < static_cast<size_t>(4 * (nr + 1)); ++i) {
        uint32_t temp = round_keys[i - 1];
        if (i % nk == 0) {
            temp = rotl32(temp, 8);
            temp = (static_cast<uint32_t>(kSBox[(temp >> 24) & 0xFF]) << 24) |
                   (static_cast<uint32_t>(kSBox[(temp >> 16) & 0xFF]) << 16) |
                   (static_cast<uint32_t>(kSBox[(temp >> 8) & 0xFF]) << 8) |
                   static_cast<uint32_t>(kSBox[temp & 0xFF]);
            temp ^= (static_cast<uint32_t>(kRcon[i / nk]) << 24);
        } else if (nk > 6 && (i % nk == 4)) {
            temp = (static_cast<uint32_t>(kSBox[(temp >> 24) & 0xFF]) << 24) |
                   (static_cast<uint32_t>(kSBox[(temp >> 16) & 0xFF]) << 16) |
                   (static_cast<uint32_t>(kSBox[(temp >> 8) & 0xFF]) << 8) |
                   static_cast<uint32_t>(kSBox[temp & 0xFF]);
        }
        round_keys[i] = round_keys[i - nk] ^ temp;
    }
}

void aes_decrypt_block(const uint32_t* rk, int nr, const uint8_t* in, uint8_t* out) {
    uint8_t state[4][4];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = in[r + 4 * c];
        }
    }

    // Initial round key addition
    for (int c = 0; c < 4; ++c) {
        uint32_t k = rk[nr * 4 + c];
        state[0][c] ^= (k >> 24) & 0xFF;
        state[1][c] ^= (k >> 16) & 0xFF;
        state[2][c] ^= (k >> 8) & 0xFF;
        state[3][c] ^= k & 0xFF;
    }

    for (int round = nr - 1; round >= 1; --round) {
        // InvShiftRows
        uint8_t temp = state[1][3];
        state[1][3] = state[1][2]; state[1][2] = state[1][1]; state[1][1] = state[1][0]; state[1][0] = temp;
        std::swap(state[2][0], state[2][2]); std::swap(state[2][1], state[2][3]);
        temp = state[3][0];
        state[3][0] = state[3][1]; state[3][1] = state[3][2]; state[3][2] = state[3][3]; state[3][3] = temp;

        // InvSubBytes
        for (int r = 0; r < 4; ++r) {
            for (int c = 0; c < 4; ++c) {
                state[r][c] = kInvSBox[state[r][c]];
            }
        }

        // AddRoundKey
        for (int c = 0; c < 4; ++c) {
            uint32_t k = rk[round * 4 + c];
            state[0][c] ^= (k >> 24) & 0xFF;
            state[1][c] ^= (k >> 16) & 0xFF;
            state[2][c] ^= (k >> 8) & 0xFF;
            state[3][c] ^= k & 0xFF;
        }

        // InvMixColumns
        for (int c = 0; c < 4; ++c) {
            uint8_t s0 = state[0][c], s1 = state[1][c], s2 = state[2][c], s3 = state[3][c];
            state[0][c] = gmul(0x0e, s0) ^ gmul(0x0b, s1) ^ gmul(0x0d, s2) ^ gmul(0x09, s3);
            state[1][c] = gmul(0x09, s0) ^ gmul(0x0e, s1) ^ gmul(0x0b, s2) ^ gmul(0x0d, s3);
            state[2][c] = gmul(0x0d, s0) ^ gmul(0x09, s1) ^ gmul(0x0e, s2) ^ gmul(0x0b, s3);
            state[3][c] = gmul(0x0b, s0) ^ gmul(0x0d, s1) ^ gmul(0x09, s2) ^ gmul(0x0e, s3);
        }
    }

    // Final round (no InvMixColumns)
    uint8_t temp = state[1][3];
    state[1][3] = state[1][2]; state[1][2] = state[1][1]; state[1][1] = state[1][0]; state[1][0] = temp;
    std::swap(state[2][0], state[2][2]); std::swap(state[2][1], state[2][3]);
    temp = state[3][0];
    state[3][0] = state[3][1]; state[3][1] = state[3][2]; state[3][2] = state[3][3]; state[3][3] = temp;

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = kInvSBox[state[r][c]];
        }
    }

    for (int c = 0; c < 4; ++c) {
        uint32_t k = rk[c];
        state[0][c] ^= (k >> 24) & 0xFF;
        state[1][c] ^= (k >> 16) & 0xFF;
        state[2][c] ^= (k >> 8) & 0xFF;
        state[3][c] ^= k & 0xFF;
    }

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            out[r + 4 * c] = state[r][c];
        }
    }
}

} // namespace

std::vector<uint8_t> aes_decrypt_cbc(
    std::span<const uint8_t> key,
    std::span<const uint8_t> input,
    std::span<const uint8_t, 16> iv
) {
    if (input.empty() || (input.size() % 16) != 0) {
        return std::vector<uint8_t>(input.begin(), input.end());
    }

    std::vector<uint32_t> round_keys;
    int nr = 10;
    aes_key_expand(key, round_keys, nr);

    std::vector<uint8_t> out(input.size());
    uint8_t prev_block[16];
    std::memcpy(prev_block, iv.data(), 16);

    for (size_t i = 0; i < input.size(); i += 16) {
        uint8_t dec[16];
        aes_decrypt_block(round_keys.data(), nr, input.data() + i, dec);
        for (int j = 0; j < 16; ++j) {
            out[i + j] = dec[j] ^ prev_block[j];
        }
        std::memcpy(prev_block, input.data() + i, 16);
    }

    // Strip PKCS#7 padding if valid
    if (!out.empty()) {
        uint8_t pad_val = out.back();
        if (pad_val > 0 && pad_val <= 16 && pad_val <= out.size()) {
            bool valid_pad = true;
            for (size_t p = out.size() - pad_val; p < out.size(); ++p) {
                if (out[p] != pad_val) {
                    valid_pad = false;
                    break;
                }
            }
            if (valid_pad) {
                out.resize(out.size() - pad_val);
            }
        }
    }

    return out;
}

// =============================================================================
// 5. Standard Security Handler Implementation
// =============================================================================

static const uint8_t kPdfPasswordPadding[32] = {
    0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41,
    0x64, 0x00, 0x4E, 0x56, 0xFF, 0xFA, 0x01, 0x08,
    0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68, 0x3E, 0x80,
    0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A
};

PdfSecurityHandler::PdfSecurityHandler(const PdfDict& encrypt_dict, const PdfArray& id_array) {
    if (encrypt_dict.empty()) return;

    is_encrypted_ = true;

    auto it_v = encrypt_dict.find("V");
    if (it_v != encrypt_dict.end()) version_ = static_cast<int>(it_v->second.as_int(1));
    auto it_r = encrypt_dict.find("R");
    if (it_r != encrypt_dict.end()) revision_ = static_cast<int>(it_r->second.as_int(2));
    auto it_l = encrypt_dict.find("Length");
    if (it_l != encrypt_dict.end()) key_len_bytes_ = static_cast<size_t>(it_l->second.as_int(40)) / 8;
    auto it_p = encrypt_dict.find("P");
    if (it_p != encrypt_dict.end()) permissions_ = it_p->second.as_int(-1);
    auto it_em = encrypt_dict.find("EncryptMetadata");
    if (it_em != encrypt_dict.end()) encrypt_metadata_ = it_em->second.as_bool(true);

    auto it_ov = encrypt_dict.find("O");
    if (it_ov != encrypt_dict.end() && it_ov->second.is_string()) {
        o_string_ = it_ov->second.as_string();
    }
    auto it_uv = encrypt_dict.find("U");
    if (it_uv != encrypt_dict.end() && it_uv->second.is_string()) {
        u_string_ = it_uv->second.as_string();
    }

    if (!id_array.empty() && id_array[0].is_string()) {
        id_first_ = id_array[0].as_string();
    }

    if (version_ == 4 || version_ == 5) {
        is_aes_ = true;
        if (version_ == 5 || revision_ >= 5) {
            is_aes256_ = true;
            key_len_bytes_ = 32;
        } else {
            key_len_bytes_ = 16;
        }
    }

    // Automatically attempt authenticating with default empty password
    authenticate("");
}

bool PdfSecurityHandler::authenticate(const std::string& password) {
    if (!is_encrypted_) return true;

    auto derive_key = [&](std::span<const uint8_t> pw_bytes) -> std::vector<uint8_t> {
        std::vector<uint8_t> pw_data;
        pw_data.reserve(32);
        size_t copy_len = std::min(pw_bytes.size(), size_t(32));
        for (size_t i = 0; i < copy_len; ++i) pw_data.push_back(pw_bytes[i]);
        for (size_t i = copy_len; i < 32; ++i) pw_data.push_back(kPdfPasswordPadding[i]);

        std::vector<uint8_t> hash_input = pw_data;
        hash_input.insert(hash_input.end(), o_string_.begin(), o_string_.end());

        uint32_t p_val = static_cast<uint32_t>(permissions_);
        hash_input.push_back(static_cast<uint8_t>(p_val & 0xFF));
        hash_input.push_back(static_cast<uint8_t>((p_val >> 8) & 0xFF));
        hash_input.push_back(static_cast<uint8_t>((p_val >> 16) & 0xFF));
        hash_input.push_back(static_cast<uint8_t>((p_val >> 24) & 0xFF));

        hash_input.insert(hash_input.end(), id_first_.begin(), id_first_.end());

        if (revision_ >= 4 && !encrypt_metadata_) {
            hash_input.push_back(0xFF);
            hash_input.push_back(0xFF);
            hash_input.push_back(0xFF);
            hash_input.push_back(0xFF);
        }

        auto digest = md5_hash(hash_input);

        if (revision_ >= 3) {
            for (int iter = 0; iter < 50; ++iter) {
                std::vector<uint8_t> iter_buf(digest.data(), digest.data() + key_len_bytes_);
                digest = md5_hash(iter_buf);
            }
        }

        return std::vector<uint8_t>(digest.data(), digest.data() + key_len_bytes_);
    };

    auto check_u_matches = [&](std::span<const uint8_t> key) -> bool {
        if (revision_ == 2) {
            std::vector<uint8_t> u_calc = rc4_crypt(key, std::span<const uint8_t>(kPdfPasswordPadding, 32));
            return (u_string_.size() >= 32 && std::memcmp(u_calc.data(), u_string_.data(), 32) == 0);
        } else if (revision_ >= 3 && revision_ <= 4) {
            std::vector<uint8_t> u_input(kPdfPasswordPadding, kPdfPasswordPadding + 32);
            u_input.insert(u_input.end(), id_first_.begin(), id_first_.end());
            auto u_hash = md5_hash(u_input);

            std::vector<uint8_t> cur(u_hash.data(), u_hash.data() + 16);
            for (int iter = 0; iter < 20; ++iter) {
                std::vector<uint8_t> iter_key(key.begin(), key.end());
                for (auto& b : iter_key) b ^= static_cast<uint8_t>(iter);
                cur = rc4_crypt(iter_key, cur);
            }
            return (u_string_.size() >= 16 && std::memcmp(cur.data(), u_string_.data(), 16) == 0);
        }
        return false;
    };

    // 1. Try candidate password as user password
    std::span<const uint8_t> pw_span(reinterpret_cast<const uint8_t*>(password.data()), password.size());
    std::vector<uint8_t> user_key = derive_key(pw_span);

    if (revision_ >= 5) {
        if (u_string_.size() >= 48) {
            std::vector<uint8_t> test_pw(password.begin(), password.end());
            test_pw.insert(test_pw.end(), u_string_.data() + 32, u_string_.data() + 40); // User salt
            test_pw.insert(test_pw.end(), u_string_.data(), u_string_.data() + 32);
            auto h = sha256_hash(test_pw);
            if (std::memcmp(h.data(), u_string_.data(), 32) == 0) {
                encryption_key_.assign(test_pw.begin(), test_pw.begin() + 32);
                is_authenticated_ = true;
                return true;
            }
        }
    } else if (check_u_matches(user_key)) {
        encryption_key_ = std::move(user_key);
        is_authenticated_ = true;
        return true;
    }

    // 2. Try candidate password as owner password (ISO 32000-1 Algorithm 7)
    if (!o_string_.empty() && revision_ <= 4) {
        std::vector<uint8_t> opw_data;
        opw_data.reserve(32);
        size_t copy_len = std::min(password.size(), size_t(32));
        for (size_t i = 0; i < copy_len; ++i) opw_data.push_back(static_cast<uint8_t>(password[i]));
        for (size_t i = copy_len; i < 32; ++i) opw_data.push_back(kPdfPasswordPadding[i]);

        auto odigest = md5_hash(opw_data);
        if (revision_ >= 3) {
            for (int iter = 0; iter < 50; ++iter) {
                std::vector<uint8_t> iter_buf(odigest.data(), odigest.data() + key_len_bytes_);
                odigest = md5_hash(iter_buf);
            }
        }
        std::vector<uint8_t> o_key(odigest.data(), odigest.data() + key_len_bytes_);

        std::vector<uint8_t> cand_u(o_string_.begin(), o_string_.end());
        if (revision_ == 2) {
            cand_u = rc4_crypt(o_key, cand_u);
        } else if (revision_ >= 3 && revision_ <= 4) {
            for (int iter = 19; iter >= 0; --iter) {
                std::vector<uint8_t> iter_key = o_key;
                for (auto& b : iter_key) b ^= static_cast<uint8_t>(iter);
                cand_u = rc4_crypt(iter_key, cand_u);
            }
        }

        if (cand_u.size() >= 32) {
            cand_u.resize(32);
            std::vector<uint8_t> cand_file_key = derive_key(cand_u);
            if (check_u_matches(cand_file_key)) {
                encryption_key_ = std::move(cand_file_key);
                is_authenticated_ = true;
                return true;
            }
        }
    }

    return false;
}

std::vector<uint8_t> PdfSecurityHandler::compute_object_key(
    uint32_t obj_id,
    uint32_t gen,
    bool is_aes
) const {
    if (is_aes256_) {
        return encryption_key_;
    }

    std::vector<uint8_t> k = encryption_key_;
    k.push_back(static_cast<uint8_t>(obj_id & 0xFF));
    k.push_back(static_cast<uint8_t>((obj_id >> 8) & 0xFF));
    k.push_back(static_cast<uint8_t>((obj_id >> 16) & 0xFF));
    k.push_back(static_cast<uint8_t>(gen & 0xFF));
    k.push_back(static_cast<uint8_t>((gen >> 8) & 0xFF));

    if (is_aes) {
        k.push_back('s');
        k.push_back('A');
        k.push_back('l');
        k.push_back('T');
    }

    auto digest = md5_hash(k);
    size_t out_len = std::min(encryption_key_.size() + 5, size_t(16));
    return std::vector<uint8_t>(digest.data(), digest.data() + out_len);
}

std::vector<uint8_t> PdfSecurityHandler::decrypt_stream(
    uint32_t obj_id,
    uint32_t gen,
    std::span<const uint8_t> data
) const {
    if (!is_encrypted_ || !is_authenticated_ || data.empty()) {
        return std::vector<uint8_t>(data.begin(), data.end());
    }

    if (is_aes_) {
        if (data.size() < 16) return std::vector<uint8_t>(data.begin(), data.end());
        std::span<const uint8_t, 16> iv(data.data(), 16);
        std::span<const uint8_t> cipher_data(data.data() + 16, data.size() - 16);
        auto key = compute_object_key(obj_id, gen, true);
        return aes_decrypt_cbc(key, cipher_data, iv);
    } else {
        auto key = compute_object_key(obj_id, gen, false);
        return rc4_crypt(key, data);
    }
}

std::string PdfSecurityHandler::decrypt_string(
    uint32_t obj_id,
    uint32_t gen,
    std::string_view str
) const {
    if (!is_encrypted_ || !is_authenticated_ || str.empty()) {
        return std::string(str);
    }

    std::span<const uint8_t> bytes(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    std::vector<uint8_t> dec = decrypt_stream(obj_id, gen, bytes);
    return std::string(reinterpret_cast<const char*>(dec.data()), dec.size());
}

} // namespace nisaba::pdf

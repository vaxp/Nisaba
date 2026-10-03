#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include "nisaba/image/image_types.hpp"

namespace nisaba::image {

/// Calculate ISO 3309 CRC-32 checksum (used in PNG chunks).
uint32_t crc32(std::span<const uint8_t> data, uint32_t prev_crc = 0) noexcept;

/// Calculate RFC 1950 Adler-32 checksum (used in zlib stream wrapper).
uint32_t adler32(std::span<const uint8_t> data, uint32_t prev_adler = 1) noexcept;

/// Decompresses RFC 1950 zlib-wrapped data stream (header + deflate + adler32).
/// Used in PNG IDAT chunks.
ImageResult<std::vector<uint8_t>> zlib_decompress(
    std::span<const uint8_t> compressed_data,
    size_t expected_size = 0,
    bool verify_checksum = false
);

/// Decompresses raw RFC 1951 DEFLATE stream.
ImageResult<std::vector<uint8_t>> deflate_decompress(
    std::span<const uint8_t> deflate_data,
    size_t expected_size = 0
);

/// Compresses data into RFC 1950 zlib-wrapped data stream (header + deflate + adler32).
/// level: 0 = store (uncompressed), 1 = fast LZ77, 6 = high compression LZ77.
ImageResult<std::vector<uint8_t>> zlib_compress(
    std::span<const uint8_t> uncompressed_data,
    int level = 6
);

/// Compresses data into raw RFC 1951 DEFLATE stream.
ImageResult<std::vector<uint8_t>> deflate_compress(
    std::span<const uint8_t> uncompressed_data,
    int level = 6
);

} // namespace nisaba::image

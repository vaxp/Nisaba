#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Decode WebP image from memory buffer into a Nisaba Pixmap.
/// Supports Lossless WebP (VP8L), Lossy WebP (VP8), and Alpha containers (VP8X / ALPH).
ImageResult<Pixmap> decode_webp(std::span<const uint8_t> webp_data);

/// Query WebP header information without decoding all pixel data.
ImageResult<ImageInfo> probe_webp(std::span<const uint8_t> webp_data);

/// Encode a PixmapRef into WebP format in memory (Lossless VP8L).
ImageResult<std::vector<uint8_t>> encode_webp(
    PixmapRef pixmap,
    bool lossless = true,
    int quality = 90
);

/// Helper to decode WebP from a file path.
ImageResult<Pixmap> decode_webp_file(const std::string& file_path);

/// Helper to encode and save a PixmapRef into a WebP file.
bool save_webp_file(
    PixmapRef pixmap,
    const std::string& file_path,
    bool lossless = true,
    int quality = 90
);

} // namespace nisaba::image

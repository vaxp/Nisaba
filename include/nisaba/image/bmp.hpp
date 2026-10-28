#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Decode a BMP image from memory buffer into a Nisaba Pixmap.
ImageResult<Pixmap> decode_bmp(
    std::span<const uint8_t> bmp_data,
    DecodeOptions options = {}
);

/// Query BMP header information without decoding pixel data.
ImageResult<ImageInfo> probe_bmp(std::span<const uint8_t> bmp_data);

/// Encode a PixmapRef into BMP format in memory (24-bit BGR when opaque, 32-bit BGRA with alpha).
ImageResult<std::vector<uint8_t>> encode_bmp(PixmapRef pixmap);

/// Helper to decode BMP from a file path.
ImageResult<Pixmap> decode_bmp_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save a PixmapRef into a BMP file.
bool save_bmp_file(
    PixmapRef pixmap,
    const std::string& file_path
);

} // namespace nisaba::image

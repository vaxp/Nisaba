#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Decode JPEG image from memory buffer into a Nisaba Pixmap.
/// Supports Baseline Sequential JFIF (SOF0) with 4:4:4, 4:2:2, 4:2:0, and Grayscale.
ImageResult<Pixmap> decode_jpeg(std::span<const uint8_t> jpeg_data);

/// Query JPEG header information without decoding all pixel data.
ImageResult<ImageInfo> probe_jpeg(std::span<const uint8_t> jpeg_data);

/// Encode a PixmapRef into JPEG format in memory.
/// quality: 1 (lowest) to 100 (highest), default 90.
ImageResult<std::vector<uint8_t>> encode_jpeg(
    PixmapRef pixmap,
    int quality = 90
);

/// Helper to decode JPEG from a file path.
ImageResult<Pixmap> decode_jpeg_file(const std::string& file_path);

/// Helper to encode and save a PixmapRef into a JPEG file.
bool save_jpeg_file(
    PixmapRef pixmap,
    const std::string& file_path,
    int quality = 90
);

} // namespace nisaba::image

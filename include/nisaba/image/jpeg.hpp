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
/// Supports Baseline Sequential (SOF0), Extended Sequential (SOF1), and Progressive (SOF2)
/// with 4:4:4, 4:2:2, 4:2:0, Grayscale, RGB, and CMYK/YCCK.
ImageResult<Pixmap> decode_jpeg(
    std::span<const uint8_t> jpeg_data,
    DecodeOptions options = {}
);

/// Query JPEG header information without decoding all pixel data.
ImageResult<ImageInfo> probe_jpeg(std::span<const uint8_t> jpeg_data);

/// Encode a PixmapRef into JPEG format in memory.
/// quality: 1 (lowest) to 100 (highest), default 90.
/// progressive: if true, encodes a multi-scan Progressive JPEG (SOF2).
ImageResult<std::vector<uint8_t>> encode_jpeg(
    PixmapRef pixmap,
    int quality = 90,
    bool progressive = false
);

/// Helper to decode JPEG from a file path.
ImageResult<Pixmap> decode_jpeg_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save a PixmapRef into a JPEG file.
bool save_jpeg_file(
    PixmapRef pixmap,
    const std::string& file_path,
    int quality = 90,
    bool progressive = false
);

} // namespace nisaba::image

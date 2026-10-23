#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/image.hpp"

namespace orionimg {

/// Decode JPEG image from memory buffer into an Image.
/// Supports Baseline Sequential JFIF (SOF0) with 4:4:4, 4:2:2, 4:2:0, and Grayscale.
ImageResult<Image> decode_jpeg(std::span<const uint8_t> jpeg_data);

/// Query JPEG header information without decoding all pixel data.
ImageResult<ImageInfo> probe_jpeg(std::span<const uint8_t> jpeg_data);

/// Encode an ImageRef into JPEG format in memory.
/// quality: 1 (lowest) to 100 (highest), default 90.
ImageResult<std::vector<uint8_t>> encode_jpeg(
    ImageRef image,
    int quality = 90
);

/// Helper to decode JPEG from a file path.
ImageResult<Image> decode_jpeg_file(const std::string& file_path);

/// Helper to encode and save an ImageRef into a JPEG file.
bool save_jpeg_file(
    ImageRef image,
    const std::string& file_path,
    int quality = 90
);

} // namespace orionimg

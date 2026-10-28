#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/image.hpp"

namespace orionimg {

/// Decode a BMP image from memory buffer into an Image.
ImageResult<Image> decode_bmp(
    std::span<const uint8_t> bmp_data,
    DecodeOptions options = {}
);

/// Query BMP header information without decoding pixel data.
ImageResult<ImageInfo> probe_bmp(std::span<const uint8_t> bmp_data);

/// Encode an ImageRef into BMP format in memory (24-bit BGR when opaque, 32-bit BGRA with alpha).
ImageResult<std::vector<uint8_t>> encode_bmp(ImageRef image);

/// Helper to decode BMP from a file path.
ImageResult<Image> decode_bmp_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save an ImageRef into a BMP file.
bool save_bmp_file(
    ImageRef image,
    const std::string& file_path
);

} // namespace orionimg

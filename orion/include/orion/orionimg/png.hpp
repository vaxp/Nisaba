#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/image.hpp"

namespace orionimg {

/// Decode PNG image from memory buffer into an Image.
ImageResult<Image> decode_png(
    std::span<const uint8_t> png_data,
    DecodeOptions options = {}
);

/// Query PNG header information without decoding all pixel data.
ImageResult<ImageInfo> probe_png(std::span<const uint8_t> png_data);

/// Encode an ImageRef into PNG format in memory.
/// compression_level: 0 (store/none), 1 (fast), 6 (default high compression).
ImageResult<std::vector<uint8_t>> encode_png(
    ImageRef image,
    int compression_level = 6
);

/// Helper to decode PNG from a file path.
ImageResult<Image> decode_png_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save an ImageRef into a PNG file.
bool save_png_file(
    ImageRef image,
    const std::string& file_path,
    int compression_level = 6
);

} // namespace orionimg

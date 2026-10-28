#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/image.hpp"

namespace orionimg {

/// Decode QOI (Quite OK Image) image from memory buffer into an Image.
ImageResult<Image> decode_qoi(
    std::span<const uint8_t> qoi_data,
    DecodeOptions options = {}
);

/// Query QOI header information without decoding all pixel data.
ImageResult<ImageInfo> probe_qoi(std::span<const uint8_t> qoi_data);

/// Encode an ImageRef into QOI format in memory.
/// linear_colorspace: false = sRGB with linear alpha, true = all channels linear.
ImageResult<std::vector<uint8_t>> encode_qoi(
    ImageRef image,
    bool linear_colorspace = false
);

/// Helper to decode QOI from a file path.
ImageResult<Image> decode_qoi_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save an ImageRef into a QOI file.
bool save_qoi_file(
    ImageRef image,
    const std::string& file_path,
    bool linear_colorspace = false
);

} // namespace orionimg

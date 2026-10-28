#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/image.hpp"

namespace orionimg {

/// Decode a Netpbm (PPM/PGM/PAM: P2, P3, P5, P6, P7) image from memory into an Image.
ImageResult<Image> decode_ppm(
    std::span<const uint8_t> ppm_data,
    DecodeOptions options = {}
);

/// Query PPM/PGM/PAM header information without decoding pixel data.
ImageResult<ImageInfo> probe_ppm(std::span<const uint8_t> ppm_data);

/// Encode an ImageRef into binary P6 PPM format in memory.
ImageResult<std::vector<uint8_t>> encode_ppm(ImageRef image);

/// Helper to decode PPM/PGM/PAM from a file path.
ImageResult<Image> decode_ppm_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save an ImageRef into a PPM file.
bool save_ppm_file(
    ImageRef image,
    const std::string& file_path
);

} // namespace orionimg

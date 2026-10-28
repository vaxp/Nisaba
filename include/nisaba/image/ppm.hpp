#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Decode a Netpbm (PPM/PGM/PAM: P2, P3, P5, P6, P7) image from memory into a Pixmap.
ImageResult<Pixmap> decode_ppm(
    std::span<const uint8_t> ppm_data,
    DecodeOptions options = {}
);

/// Query PPM/PGM/PAM header information without decoding pixel data.
ImageResult<ImageInfo> probe_ppm(std::span<const uint8_t> ppm_data);

/// Encode a PixmapRef into binary P6 PPM format in memory.
ImageResult<std::vector<uint8_t>> encode_ppm(PixmapRef pixmap);

/// Helper to decode PPM/PGM/PAM from a file path.
ImageResult<Pixmap> decode_ppm_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to encode and save a PixmapRef into a PPM file.
bool save_ppm_file(
    PixmapRef pixmap,
    const std::string& file_path
);

} // namespace nisaba::image

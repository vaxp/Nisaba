#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Decode QOI (Quite OK Image) image from memory buffer into a Nisaba Pixmap.
ImageResult<Pixmap> decode_qoi(std::span<const uint8_t> qoi_data);

/// Query QOI header information without decoding all pixel data.
ImageResult<ImageInfo> probe_qoi(std::span<const uint8_t> qoi_data);

/// Encode a PixmapRef into QOI format in memory.
/// linear_colorspace: false = sRGB with linear alpha, true = all channels linear.
ImageResult<std::vector<uint8_t>> encode_qoi(
    PixmapRef pixmap,
    bool linear_colorspace = false
);

/// Helper to decode QOI from a file path.
ImageResult<Pixmap> decode_qoi_file(const std::string& file_path);

/// Helper to encode and save a PixmapRef into a QOI file.
bool save_qoi_file(
    PixmapRef pixmap,
    const std::string& file_path,
    bool linear_colorspace = false
);

} // namespace nisaba::image

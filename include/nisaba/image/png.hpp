#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Decode PNG image from memory buffer into a Nisaba Pixmap.
ImageResult<Pixmap> decode_png(std::span<const uint8_t> png_data);

/// Query PNG header information without decoding all pixel data.
ImageResult<ImageInfo> probe_png(std::span<const uint8_t> png_data);

/// Encode a PixmapRef into PNG format in memory.
/// compression_level: 0 (store/none), 1 (fast), 6 (default high compression).
ImageResult<std::vector<uint8_t>> encode_png(
    PixmapRef pixmap,
    int compression_level = 6
);

/// Helper to decode PNG from a file path.
ImageResult<Pixmap> decode_png_file(const std::string& file_path);

/// Helper to encode and save a PixmapRef into a PNG file.
bool save_png_file(
    PixmapRef pixmap,
    const std::string& file_path,
    int compression_level = 6
);

} // namespace nisaba::image

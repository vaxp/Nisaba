#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include "nisaba/image/image_types.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Linear floating-point RGB buffer decoded from a Radiance HDR (.hdr / RGBE) environment map.
struct HdrImage {
    uint32_t width{0};
    uint32_t height{0};
    float exposure{1.0f};
    std::vector<float> pixels; // Interleaved R, G, B linear float values (width * height * 3)
};

/// Decode a Radiance HDR (.hdr / RGBE) image into floating-point linear RGB (for 3D PBR / IBL).
ImageResult<HdrImage> decode_hdr_float(std::span<const uint8_t> hdr_data);

/// Decode a Radiance HDR (.hdr / RGBE) image into an 8-bit tone-mapped sRGB Pixmap.
ImageResult<Pixmap> decode_hdr(
    std::span<const uint8_t> hdr_data,
    DecodeOptions options = {}
);

/// Query Radiance HDR header information without decoding scanlines.
ImageResult<ImageInfo> probe_hdr(std::span<const uint8_t> hdr_data);

/// Encode linear floating-point RGB pixels (width * height * 3) into Radiance HDR format (with adaptive RLE).
ImageResult<std::vector<uint8_t>> encode_hdr_float(
    std::span<const float> rgb_float,
    uint32_t width,
    uint32_t height
);

/// Encode an 8-bit PixmapRef into Radiance HDR format in memory.
ImageResult<std::vector<uint8_t>> encode_hdr(PixmapRef pixmap);

/// Helper to decode Radiance HDR from a file path into a tone-mapped Pixmap.
ImageResult<Pixmap> decode_hdr_file(
    const std::string& file_path,
    DecodeOptions options = {}
);

/// Helper to decode Radiance HDR from a file path into floating-point linear RGB.
ImageResult<HdrImage> decode_hdr_float_file(const std::string& file_path);

/// Helper to encode and save a PixmapRef into a Radiance HDR file.
bool save_hdr_file(
    PixmapRef pixmap,
    const std::string& file_path
);

} // namespace nisaba::image

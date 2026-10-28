#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <optional>
#include <span>
#include <vector>
#include <cstring>

namespace orionimg {

enum class ImageFormat {
    Unknown,
    PNG,
    JPEG,
    QOI,
    BMP,
    PPM,
    WebP,
    HDR
};

enum class ColorModel {
    Grayscale,
    GrayscaleAlpha,
    RGB,
    RGBA,
    Indexed,
    CMYK,
    HDR_RGBE
};

struct DecodeOptions {
    bool premultiply_alpha{true};
    bool apply_exif_orientation{true};
};

struct ImageInfo {
    uint32_t width{0};
    uint32_t height{0};
    uint8_t channels{0};
    uint8_t bit_depth{8};
    bool has_alpha{false};
    bool is_progressive{false};
    uint8_t exif_orientation{1};
    ImageFormat format{ImageFormat::Unknown};
    ColorModel color_model{ColorModel::RGBA};
};

enum class ImageError {
    Ok,
    InvalidSignature,
    UnsupportedFormat,
    CorruptedData,
    PrematureEndOfStream,
    InvalidDimensions,
    DecompressionFailed,
    CompressionFailed,
    UnsupportedFeature,
    IOError,
    OutOfMemory
};

inline const char* image_error_to_string(ImageError err) noexcept {
    switch (err) {
        case ImageError::Ok: return "Ok";
        case ImageError::InvalidSignature: return "Invalid signature/header";
        case ImageError::UnsupportedFormat: return "Unsupported image format";
        case ImageError::CorruptedData: return "Corrupted image stream";
        case ImageError::PrematureEndOfStream: return "Premature end of stream";
        case ImageError::InvalidDimensions: return "Invalid image dimensions";
        case ImageError::DecompressionFailed: return "Decompression failed";
        case ImageError::CompressionFailed: return "Compression failed";
        case ImageError::UnsupportedFeature: return "Unsupported feature";
        case ImageError::IOError: return "I/O error";
        case ImageError::OutOfMemory: return "Out of memory";
        default: return "Unknown error";
    }
}

template<typename T>
struct ImageResult {
    std::optional<T> value;
    ImageError error{ImageError::Ok};

    bool has_value() const noexcept { return value.has_value(); }
    explicit operator bool() const noexcept { return has_value(); }

    T& operator*() & { return *value; }
    const T& operator*() const& { return *value; }
    T&& operator*() && { return *std::move(value); }

    T* operator->() noexcept { return &*value; }
    const T* operator->() const noexcept { return &*value; }

    static ImageResult<T> success(T val) {
        return ImageResult{std::move(val), ImageError::Ok};
    }

    static ImageResult<T> fail(ImageError err) {
        return ImageResult{std::nullopt, err};
    }
};

/// Parse EXIF Orientation tag (0x0112) from TIFF header or APP1 "Exif\0\0" payload.
/// Returns 1..8 (default 1 = normal).
inline uint8_t parse_exif_orientation(std::span<const uint8_t> data) noexcept {
    if (data.size() >= 6 && std::memcmp(data.data(), "Exif\0\0", 6) == 0) {
        data = data.subspan(6);
    }
    if (data.size() < 8) return 1;

    bool le = false;
    if (data[0] == 'I' && data[1] == 'I') {
        le = true;
    } else if (data[0] == 'M' && data[1] == 'M') {
        le = false;
    } else {
        return 1;
    }

    auto r16 = [&](size_t off) -> uint16_t {
        if (le) return static_cast<uint16_t>(data[off] | (data[off + 1] << 8));
        return static_cast<uint16_t>((data[off] << 8) | data[off + 1]);
    };
    auto r32 = [&](size_t off) -> uint32_t {
        if (le) {
            return static_cast<uint32_t>(data[off]) |
                   (static_cast<uint32_t>(data[off + 1]) << 8) |
                   (static_cast<uint32_t>(data[off + 2]) << 16) |
                   (static_cast<uint32_t>(data[off + 3]) << 24);
        }
        return (static_cast<uint32_t>(data[off]) << 24) |
               (static_cast<uint32_t>(data[off + 1]) << 16) |
               (static_cast<uint32_t>(data[off + 2]) << 8) |
               static_cast<uint32_t>(data[off + 3]);
    };

    if (r16(2) != 0x002A) return 1;
    uint32_t ifd_off = r32(4);
    if (ifd_off + 2 > data.size()) return 1;

    uint16_t num_entries = r16(ifd_off);
    size_t pos = ifd_off + 2;
    for (uint16_t i = 0; i < num_entries; ++i) {
        if (pos + 12 > data.size()) break;
        uint16_t tag = r16(pos);
        uint16_t type = r16(pos + 2);
        uint32_t count = r32(pos + 4);
        if (tag == 0x0112 && type == 3 && count >= 1) {
            uint16_t val = r16(pos + 8);
            if (val >= 1 && val <= 8) return static_cast<uint8_t>(val);
        }
        pos += 12;
    }
    return 1;
}

} // namespace orionimg

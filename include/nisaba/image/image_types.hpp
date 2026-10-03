#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <optional>
#include <span>
#include <vector>
#include "nisaba/math/size.hpp"
#include "nisaba/color/color.hpp"

namespace nisaba::image {

enum class ImageFormat {
    Unknown,
    PNG,
    JPEG,
    QOI,
    BMP,
    PPM
};

enum class ColorModel {
    Grayscale,
    GrayscaleAlpha,
    RGB,
    RGBA,
    Indexed
};

struct ImageInfo {
    uint32_t width{0};
    uint32_t height{0};
    uint8_t channels{0};
    uint8_t bit_depth{8};
    bool has_alpha{false};
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

} // namespace nisaba::image

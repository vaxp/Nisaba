#include "nisaba/image/image_io.hpp"
#include <fstream>
#include <cstring>
#include <algorithm>

namespace nisaba::image {

ImageFormat detect_format(std::span<const uint8_t> data) noexcept {
    if (data.size() >= 8) {
        constexpr uint8_t PNG_MAGIC[8] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};
        if (std::memcmp(data.data(), PNG_MAGIC, 8) == 0) {
            return ImageFormat::PNG;
        }
    }
    if (data.size() >= 4) {
        constexpr uint8_t QOI_MAGIC[4] = {'q', 'o', 'i', 'f'};
        if (std::memcmp(data.data(), QOI_MAGIC, 4) == 0) {
            return ImageFormat::QOI;
        }
    }
    if (data.size() >= 2) {
        if (data[0] == 0xFF && data[1] == 0xD8) {
            return ImageFormat::JPEG;
        }
        if (data[0] == 'B' && data[1] == 'M') {
            return ImageFormat::BMP;
        }
        if (data[0] == 'P' && (data[1] == '6' || data[1] == '3')) {
            return ImageFormat::PPM;
        }
    }
    return ImageFormat::Unknown;
}

ImageResult<Pixmap> load_image_from_memory(std::span<const uint8_t> data) {
    ImageFormat fmt = detect_format(data);
    switch (fmt) {
        case ImageFormat::PNG:
            return decode_png(data);
        case ImageFormat::JPEG:
            return decode_jpeg(data);
        case ImageFormat::QOI:
            return decode_qoi(data);
        default:
            return ImageResult<Pixmap>::fail(ImageError::UnsupportedFormat);
    }
}

ImageResult<Pixmap> load_image_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        return ImageResult<Pixmap>::fail(ImageError::CorruptedData);
    }
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return ImageResult<Pixmap>::fail(ImageError::IOError);
    }

    return load_image_from_memory(buffer);
}

ImageResult<ImageInfo> probe_image_from_memory(std::span<const uint8_t> data) {
    ImageFormat fmt = detect_format(data);
    switch (fmt) {
        case ImageFormat::PNG:
            return probe_png(data);
        case ImageFormat::JPEG:
            return probe_jpeg(data);
        case ImageFormat::QOI:
            return probe_qoi(data);
        default:
            return ImageResult<ImageInfo>::fail(ImageError::UnsupportedFormat);
    }
}

ImageResult<ImageInfo> probe_image_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return ImageResult<ImageInfo>::fail(ImageError::IOError);
    }

    std::array<uint8_t, 4096> header_buf{};
    file.read(reinterpret_cast<char*>(header_buf.data()), header_buf.size());
    std::streamsize count = file.gcount();
    if (count <= 0) {
        return ImageResult<ImageInfo>::fail(ImageError::CorruptedData);
    }

    return probe_image_from_memory(std::span<const uint8_t>(header_buf.data(), static_cast<size_t>(count)));
}

bool save_image_file(
    PixmapRef pixmap,
    const std::string& path,
    int quality_or_compression
) {
    // Determine format by file extension
    auto dot_pos = path.rfind('.');
    if (dot_pos == std::string::npos) {
        return save_png_file(pixmap, path, quality_or_compression);
    }

    std::string ext = path.substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return std::tolower(c);
    });

    if (ext == "png") {
        return save_png_file(pixmap, path, quality_or_compression);
    } else if (ext == "jpg" || ext == "jpeg") {
        return save_jpeg_file(pixmap, path, quality_or_compression);
    } else if (ext == "qoi") {
        return save_qoi_file(pixmap, path);
    } else if (ext == "bmp") {
        return pixmap.save_bmp(path);
    } else if (ext == "ppm") {
        return pixmap.save_ppm(path);
    }

    return save_png_file(pixmap, path, quality_or_compression);
}

} // namespace nisaba::image

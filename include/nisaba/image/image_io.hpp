#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "nisaba/image/image_types.hpp"
#include "nisaba/image/png.hpp"
#include "nisaba/image/jpeg.hpp"
#include "nisaba/image/qoi.hpp"
#include "nisaba/canvas/pixmap.hpp"

namespace nisaba::image {

/// Detect image format by inspecting magic bytes.
ImageFormat detect_format(std::span<const uint8_t> data) noexcept;

/// Automatically identify format and decode image from memory.
ImageResult<Pixmap> load_image_from_memory(std::span<const uint8_t> data);

/// Automatically identify format and decode image from file path.
ImageResult<Pixmap> load_image_file(const std::string& path);

/// Query image header/dimensions without full decode.
ImageResult<ImageInfo> probe_image_from_memory(std::span<const uint8_t> data);

/// Query image header/dimensions from file.
ImageResult<ImageInfo> probe_image_file(const std::string& path);

/// Automatically save image by inspecting file extension (.png, .jpg, .jpeg, .bmp).
bool save_image_file(
    PixmapRef pixmap,
    const std::string& path,
    int quality_or_compression = 90
);

} // namespace nisaba::image

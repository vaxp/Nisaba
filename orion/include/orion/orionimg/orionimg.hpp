#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "orion/orionimg/image.hpp"
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/deflate.hpp"
#include "orion/orionimg/png.hpp"
#include "orion/orionimg/jpeg.hpp"
#include "orion/orionimg/qoi.hpp"
#include "orion/orionimg/webp.hpp"

namespace orionimg {

/// Detect image format by inspecting magic bytes.
ImageFormat detect_format(std::span<const uint8_t> data) noexcept;

/// Automatically identify format and decode image from memory.
ImageResult<Image> load_image_from_memory(std::span<const uint8_t> data);

/// Automatically identify format and decode image from file path.
ImageResult<Image> load_image_file(const std::string& path);

/// Query image header/dimensions without full decode.
ImageResult<ImageInfo> probe_image_from_memory(std::span<const uint8_t> data);

/// Query image header/dimensions from file.
ImageResult<ImageInfo> probe_image_file(const std::string& path);

/// Automatically save image by inspecting file extension (.png, .jpg, .jpeg, .bmp, .ppm, .qoi, .webp).
bool save_image_file(
    ImageRef image,
    const std::string& path,
    int quality_or_compression = 90
);

} // namespace orionimg

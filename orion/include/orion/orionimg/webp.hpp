#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <string>
#include <optional>
#include "orion/orionimg/image_types.hpp"
#include "orion/orionimg/image.hpp"

namespace orionimg {

/// Blending method for animated WebP frames
enum class WebPBlendMethod : uint8_t {
    AlphaBlend = 0, ///< Blend frame with previous canvas using alpha
    Overwrite  = 1  ///< Overwrite rectangle without blending
};

/// Disposal method for animated WebP frames
enum class WebPDisposeMethod : uint8_t {
    None       = 0, ///< Do not dispose, leave canvas for next frame
    Background = 1  ///< Dispose frame rectangle to background color
};

/// A single animated frame with its composited canvas and timing
struct WebPAnimationFrame {
    Image image;                ///< Fully composited canvas for this frame
    uint32_t duration_ms{100};  ///< Frame duration in milliseconds
    uint32_t x{0};              ///< Frame sub-rectangle X offset
    uint32_t y{0};              ///< Frame sub-rectangle Y offset
    uint32_t width{0};          ///< Frame sub-rectangle width
    uint32_t height{0};         ///< Frame sub-rectangle height
    WebPBlendMethod blend_method{WebPBlendMethod::AlphaBlend};
    WebPDisposeMethod dispose_method{WebPDisposeMethod::None};
};

/// Multi-frame WebP animation container
struct WebPAnimation {
    uint32_t canvas_width{0};
    uint32_t canvas_height{0};
    uint32_t loop_count{0};     ///< 0 = infinite loop
    ColorU8 background_color{0, 0, 0, 0};
    std::vector<WebPAnimationFrame> frames;

    [[nodiscard]] size_t frame_count() const noexcept { return frames.size(); }
    [[nodiscard]] bool empty() const noexcept { return frames.empty(); }
    [[nodiscard]] uint32_t total_duration_ms() const noexcept {
        uint32_t total = 0;
        for (const auto& f : frames) total += f.duration_ms;
        return total;
    }
};

/// Decode WebP image from memory buffer into an Image.
/// Supports Lossless WebP (VP8L), Lossy WebP (VP8), Animated WebP (first frame), and Alpha containers (VP8X / ALPH).
ImageResult<Image> decode_webp(std::span<const uint8_t> webp_data);

/// Decode multi-frame WebP animation from memory buffer
ImageResult<WebPAnimation> decode_webp_animation(std::span<const uint8_t> webp_data);

/// Decode multi-frame WebP animation from file path
ImageResult<WebPAnimation> decode_webp_animation_file(const std::string& file_path);

/// Check if a WebP data stream is animated
bool is_animated_webp(std::span<const uint8_t> webp_data);

/// Query WebP header information without decoding all pixel data.
ImageResult<ImageInfo> probe_webp(std::span<const uint8_t> webp_data);

/// Encode an ImageRef into WebP format in memory (Lossless VP8L).
ImageResult<std::vector<uint8_t>> encode_webp(
    ImageRef image,
    bool lossless = true,
    int quality = 90
);

/// Helper to decode WebP from a file path.
ImageResult<Image> decode_webp_file(const std::string& file_path);

/// Helper to encode and save an ImageRef into a WebP file.
bool save_webp_file(
    ImageRef image,
    const std::string& file_path,
    bool lossless = true,
    int quality = 90
);

} // namespace orionimg

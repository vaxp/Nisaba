#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <optional>
#include "nisaba/types.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/transform.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/color/blend_mode.hpp"
#include "nisaba/shaders/shader.hpp"

namespace nisaba::gpu {

/// 2D GPU Vertex with subpixel anti-aliasing coverage.
/// Attributes layout:
///   Location 0: vec2 a_position (x, y)
///   Location 1: vec2 a_texcoord (u, v)
///   Location 2: vec4 a_color    (r, g, b, a) [0.0 - 1.0]
///   Location 3: float a_coverage (0.0 - 1.0) for boundary AA fringe
struct GpuVertex {
    nisaba::Point pos{0.0f, 0.0f};
    nisaba::Point uv{0.0f, 0.0f};
    nisaba::Color color{nisaba::Color::TRANSPARENT};
    float coverage{1.0f};

    constexpr GpuVertex() noexcept = default;
    constexpr GpuVertex(nisaba::Point p, nisaba::Color c, float cov = 1.0f) noexcept
        : pos(p), uv(0.0f, 0.0f), color(c), coverage(cov) {}
    constexpr GpuVertex(nisaba::Point p, nisaba::Point u, nisaba::Color c, float cov = 1.0f) noexcept
        : pos(p), uv(u), color(c), coverage(cov) {}
    constexpr GpuVertex(float x, float y, nisaba::Color c, float cov = 1.0f) noexcept
        : pos(x, y), uv(0.0f, 0.0f), color(c), coverage(cov) {}
    constexpr GpuVertex(float x, float y, float u, float v, nisaba::Color c, float cov = 1.0f) noexcept
        : pos(x, y), uv(u, v), color(c), coverage(cov) {}
};

/// Hardware GPU Backend driver architecture.
enum class GpuBackendType : uint8_t {
    Auto,
    OpenGL,
    Vulkan
};

/// Drawing primitive topologies.
enum class GpuDrawMode : uint8_t {
    Triangles,
    TriangleStrip,
    TriangleFan,
    Lines,
    LineStrip
};

/// GPU Shader types for the rendering pipeline.
enum class GpuShaderKind : uint8_t {
    SolidColor,
    LinearGradient,
    RadialGradient,
    SweepGradient,
    Texture,
    VertexMesh,
    Shadow,
    GaussianBlur,
    Custom
};

/// Texture filtering modes.
enum class GpuTextureFilter : uint8_t {
    Nearest,
    Linear
};

/// Texture wrapping / spread modes.
enum class GpuTextureWrap : uint8_t {
    Clamp,
    Repeat,
    MirroredRepeat
};

/// Texture pixel format on GPU.
enum class GpuTextureFormat : uint8_t {
    RGBA8,
    BGRA8,
    Alpha8
};

/// Maximum gradient stops supported analytically in GPU fragment shaders.
inline constexpr size_t GPU_MAX_GRADIENT_STOPS = 16;

/// Uniform buffer data for GPU pipeline states.
struct GpuUniforms {
    float viewport_width{1.0f};
    float viewport_height{1.0f};
    nisaba::Transform transform{};
    nisaba::Color solid_color{nisaba::Color::BLACK};

    // Gradient parameters
    nisaba::Point grad_start{0.0f, 0.0f};
    nisaba::Point grad_end{0.0f, 0.0f};
    nisaba::Point grad_center{0.0f, 0.0f};
    nisaba::Point grad_focal{0.0f, 0.0f};
    float grad_radius{1.0f};
    nisaba::SpreadMode spread_mode{nisaba::SpreadMode::Pad};

    uint32_t num_stops{0};
    std::array<float, GPU_MAX_GRADIENT_STOPS> stop_positions{};
    std::array<nisaba::Color, GPU_MAX_GRADIENT_STOPS> stop_colors{};

    // Texture parameters
    float opacity{1.0f};
    bool has_texture{false};
    bool use_vertex_color{false};

    // Analytical SDF Shadow / Glow parameters
    nisaba::Rect shadow_rect{};
    nisaba::Point shadow_radii{0.0f, 0.0f};
    float shadow_sigma{0.0f};
    nisaba::Color shadow_color{nisaba::Color::TRANSPARENT};

    // Two-pass separable Gaussian blur parameters
    nisaba::Point blur_dir{1.0f, 0.0f};
    float blur_radius{0.0f};

    constexpr GpuUniforms() noexcept = default;
};

/// High-level GPU blend state descriptor.
struct GpuBlendState {
    nisaba::BlendMode blend_mode{nisaba::BlendMode::SourceOver};

    constexpr GpuBlendState() noexcept = default;
    constexpr explicit GpuBlendState(nisaba::BlendMode bm) noexcept : blend_mode(bm) {}

    constexpr bool operator==(const GpuBlendState& o) const noexcept {
        return blend_mode == o.blend_mode;
    }
    constexpr bool operator!=(const GpuBlendState& o) const noexcept {
        return blend_mode != o.blend_mode;
    }
};

} // namespace nisaba::gpu

#pragma once

#include "nisaba/physics/math/vec3.hpp"
#include <cstdint>
#include <vector>
#include <cstddef>

namespace nisaba::render3d {

using Vec3 = nisaba::physics::Vec3;

enum class VertexAttribute : uint8_t {
    Position = 0,
    Normal = 1,
    Tangent = 2,
    TexCoord0 = 3,
    TexCoord1 = 4,
    Color = 5,
    BoneIndices = 6,
    BoneWeights = 7
};

enum class VertexFormat : uint8_t {
    Float = 0,
    Float2 = 1,
    Float3 = 2,
    Float4 = 3,
    UByte4Norm = 4,
    UShort2 = 5,
    UShort4 = 6
};

[[nodiscard]] constexpr size_t vertex_format_size(VertexFormat format) noexcept {
    switch (format) {
        case VertexFormat::Float:      return 4;
        case VertexFormat::Float2:     return 8;
        case VertexFormat::Float3:     return 12;
        case VertexFormat::Float4:     return 16;
        case VertexFormat::UByte4Norm: return 4;
        case VertexFormat::UShort2:    return 4;
        case VertexFormat::UShort4:    return 8;
    }
    return 0;
}

struct VertexAttributeDescriptor {
    VertexAttribute attribute;
    VertexFormat format;
    uint32_t offset;
};

/// @brief Configurable vertex layout specification describing buffer strides and attribute bindings.
class VertexLayout {
public:
    std::vector<VertexAttributeDescriptor> attributes;
    uint32_t stride{0};

    VertexLayout() = default;

    /// @brief Appends an attribute and automatically computes its offset and increments layout stride.
    void add_attribute(VertexAttribute attr, VertexFormat fmt) noexcept {
        const uint32_t offset = stride;
        stride += static_cast<uint32_t>(vertex_format_size(fmt));
        attributes.push_back({attr, fmt, offset});
    }

    /// @brief Pre-configured standard PBR vertex layout (Position, Normal, Tangent4, UV0, UV1) = 64 bytes.
    [[nodiscard]] static VertexLayout standard_pbr() noexcept {
        VertexLayout layout;
        layout.attributes.push_back({VertexAttribute::Position,  VertexFormat::Float3, 0});
        layout.attributes.push_back({VertexAttribute::Normal,    VertexFormat::Float3, 16});
        layout.attributes.push_back({VertexAttribute::Tangent,   VertexFormat::Float4, 32});
        layout.attributes.push_back({VertexAttribute::TexCoord0, VertexFormat::Float2, 48});
        layout.attributes.push_back({VertexAttribute::TexCoord1, VertexFormat::Float2, 56});
        layout.stride = 64;
        return layout;
    }
};

/// @brief Standard interleaved PBR 3D Vertex structure.
/// Exactly 64 bytes (one full CPU cache line), 16-byte aligned for cache-line optimization and SIMD computation.
struct alignas(16) Vertex3D {
    Vec3 position{0.0f, 0.0f, 0.0f};           // 16 bytes (Vec3 is 16-byte aligned)
    Vec3 normal{0.0f, 1.0f, 0.0f};             // 16 bytes
    float tangent[4]{1.0f, 0.0f, 0.0f, 1.0f};  // 16 bytes: Tangent XYZ + handedness W (+1.0 or -1.0)
    float uv[2]{0.0f, 0.0f};                   // 8 bytes: Primary UV coordinate (U, V)
    float uv1[2]{0.0f, 0.0f};                  // 8 bytes: Secondary UV / lightmap coordinate

    constexpr Vertex3D() noexcept = default;

    constexpr Vertex3D(
        const Vec3& in_pos,
        const Vec3& in_norm,
        float u, float v,
        float tx = 1.0f, float ty = 0.0f, float tz = 0.0f, float tw = 1.0f,
        float u1 = 0.0f, float v1 = 0.0f
    ) noexcept
        : position(in_pos),
          normal(in_norm),
          tangent{tx, ty, tz, tw},
          uv{u, v},
          uv1{u1, v1} {}
};

static_assert(sizeof(Vertex3D) == 64, "Vertex3D must be exactly 64 bytes (cache-line aligned)");
static_assert(alignof(Vertex3D) == 16, "Vertex3D must be 16-byte aligned");

} // namespace nisaba::render3d

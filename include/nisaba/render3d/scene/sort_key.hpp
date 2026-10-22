#pragma once

#include <cstdint>
#include <algorithm>

namespace nisaba::render3d {

enum class RenderPassID : uint8_t {
    Shadow       = 0,
    EarlyZ       = 1,
    Opaque       = 2,
    Transparent  = 3,
    PostProcess  = 4
};

/// @brief Utility class for encoding and decoding 64-bit draw call sorting keys.
/// Formatted as: [ Pass ID (4b) | Material ID (16b) | Depth Key (24b) | Primitive ID (20b) ]
class DrawSortKey64 {
public:
    static constexpr uint64_t PASS_MASK      = 0xF000000000000000ULL;
    static constexpr uint64_t MATERIAL_MASK  = 0x0FFFF00000000000ULL;
    static constexpr uint64_t DEPTH_MASK     = 0x00000FFFFFF00000ULL;
    static constexpr uint64_t PRIMITIVE_MASK = 0x00000000000FFFFFULL;

    static constexpr uint32_t MAX_DEPTH_INT  = 0x00FFFFFFU; // 24-bit max (16,777,215)

    [[nodiscard]] static constexpr uint64_t encode(
        RenderPassID pass,
        uint16_t material_id,
        float linear_depth,
        bool is_transparent,
        uint32_t primitive_id,
        float max_depth = 1000.0f
    ) noexcept {
        const float d_clamped = std::clamp(linear_depth / (max_depth > 1e-4f ? max_depth : 1.0f), 0.0f, 1.0f);
        uint32_t depth_bits = static_cast<uint32_t>(d_clamped * static_cast<float>(MAX_DEPTH_INT));

        // For transparent passes, reverse depth key for back-to-front rendering
        if (is_transparent || pass == RenderPassID::Transparent) {
            depth_bits = MAX_DEPTH_INT - depth_bits;
        }

        const uint64_t pass_part = static_cast<uint64_t>(static_cast<uint8_t>(pass) & 0x0F) << 60;
        const uint64_t mat_part  = static_cast<uint64_t>(material_id) << 44;
        const uint64_t depth_part= static_cast<uint64_t>(depth_bits & MAX_DEPTH_INT) << 20;
        const uint64_t prim_part = static_cast<uint64_t>(primitive_id & 0x000FFFFF);

        return pass_part | mat_part | depth_part | prim_part;
    }

    [[nodiscard]] static constexpr RenderPassID decode_pass(uint64_t key) noexcept {
        return static_cast<RenderPassID>((key >> 60) & 0x0F);
    }

    [[nodiscard]] static constexpr uint16_t decode_material_id(uint64_t key) noexcept {
        return static_cast<uint16_t>((key >> 44) & 0xFFFF);
    }

    [[nodiscard]] static constexpr uint32_t decode_depth_bits(uint64_t key) noexcept {
        return static_cast<uint32_t>((key >> 20) & MAX_DEPTH_INT);
    }

    [[nodiscard]] static constexpr uint32_t decode_primitive_id(uint64_t key) noexcept {
        return static_cast<uint32_t>(key & 0x000FFFFF);
    }
};

} // namespace nisaba::render3d

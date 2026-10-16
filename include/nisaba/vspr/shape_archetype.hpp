// Nisaba — Sovereign Full-Stack 2D Graphics Engine
// Copyright (C) 2024-2026 Hussein Jaafar
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <algorithm>
#include <cmath>

#include "nisaba/math/rect.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/math/border_radius.hpp"
#include "nisaba/math/rrect.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/mask.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/pipeline/simd.hpp"

namespace nisaba::vspr {

/// @brief Shape classification for canonical primitive archetypes.
enum class ShapeType : uint8_t {
    Rect,
    RoundedRect,
    Circle,
    Pill,
    Triangle
};

/// @brief 64-bit compact hash key for archetype lookup.
struct ArchetypeKey {
    ShapeType type{ShapeType::RoundedRect};
    uint16_t width{0};
    uint16_t height{0};
    uint16_t param0{0}; // e.g. radius or corner radius (fixed-point * 10)
    uint16_t param1{0}; // e.g. secondary radius or parameter
    uint16_t param2{0}; // e.g. border thickness

    constexpr bool operator==(const ArchetypeKey& other) const noexcept {
        return type == other.type &&
               width == other.width &&
               height == other.height &&
               param0 == other.param0 &&
               param1 == other.param1 &&
               param2 == other.param2;
    }
};

struct ArchetypeKeyHash {
    size_t operator()(const ArchetypeKey& k) const noexcept {
        // FNV-1a 64-bit hash
        uint64_t hash = 14695981039346656037ULL;
        auto step = [&hash](uint64_t val) {
            hash ^= val;
            hash *= 1099511628211ULL;
        };
        step(static_cast<uint64_t>(k.type));
        step((static_cast<uint64_t>(k.width) << 16) | k.height);
        step((static_cast<uint64_t>(k.param0) << 32) | (static_cast<uint64_t>(k.param1) << 16) | k.param2);
        return static_cast<size_t>(hash);
    }
};

/// @brief Canonical Pre-Rasterized Shape Archetype.
/// Stores a high-precision 256-subpixel antialiased 8-bit coverage mask built ONCE,
/// allowing any number of identical/reflected instances to be blitted at pure SIMD streaming memory speed.
class ShapeArchetype {
public:
    ShapeArchetype() = default;

    ShapeArchetype(uint32_t width, uint32_t height, std::vector<uint8_t> mask)
        : width_(width), height_(height), mask_(std::move(mask)) {}

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }
    [[nodiscard]] bool is_valid() const noexcept { return width_ > 0 && height_ > 0 && !mask_.empty(); }
    [[nodiscard]] const uint8_t* mask_data() const noexcept { return mask_.data(); }

    /// @brief Blits this archetype into a target pixmap with solid color modulation via SIMD.
    void blit(
        PixmapMut& dst,
        int32_t dst_x,
        int32_t dst_y,
        PremultipliedColorU8 premult_color,
        const std::optional<ScreenIntRect>& clip = std::nullopt
    ) const noexcept {
        if (!is_valid()) return;

        int32_t x0 = std::max(0, dst_x);
        int32_t y0 = std::max(0, dst_y);
        int32_t x1 = std::min(static_cast<int32_t>(dst.width()), dst_x + static_cast<int32_t>(width_));
        int32_t y1 = std::min(static_cast<int32_t>(dst.height()), dst_y + static_cast<int32_t>(height_));

        if (clip.has_value()) {
            x0 = std::max(x0, static_cast<int32_t>(clip->x()));
            y0 = std::max(y0, static_cast<int32_t>(clip->y()));
            x1 = std::min(x1, static_cast<int32_t>(clip->right()));
            y1 = std::min(y1, static_cast<int32_t>(clip->bottom()));
        }

        if (x0 >= x1 || y0 >= y1) return;

        const size_t span_len = static_cast<size_t>(x1 - x0);
        const int32_t src_x_offset = x0 - dst_x;

        for (int32_t cy = y0; cy < y1; ++cy) {
            const int32_t src_y = cy - dst_y;
            PremultipliedColorU8* dst_row = dst.row(static_cast<size_t>(cy)) + x0;
            const uint8_t* mask_row = mask_.data() + (src_y * width_) + src_x_offset;

            simd::blend_solid_mask_span(dst_row, premult_color, mask_row, span_len);
        }
    }

private:
    uint32_t width_{0};
    uint32_t height_{0};
    std::vector<uint8_t> mask_;
};

/// @brief Pre-calculated Corner Quadrant Archetype for 9-Slice Instanced Scaling.
/// A single quarter-arc mask of radius R allows instant generation of ANY rounded rect of arbitrary W x H
/// without computing any Bézier curve equations at runtime.
class CornerQuadrantArchetype {
public:
    CornerQuadrantArchetype() = default;

    explicit CornerQuadrantArchetype(uint32_t radius)
        : radius_(radius) {
        if (radius == 0) return;
        mask_.assign(static_cast<size_t>(radius) * radius, 0);

        // Pre-render a top-left quarter circle arc with exact analytical subpixel antialiasing
        float r_f = static_cast<float>(radius);
        float cx = r_f;
        float cy = r_f;

        for (uint32_t y = 0; y < radius; ++y) {
            float py = static_cast<float>(y) + 0.5f;
            for (uint32_t x = 0; x < radius; ++x) {
                float px = static_cast<float>(x) + 0.5f;
                float dx = px - cx;
                float dy = py - cy;
                float dist = std::sqrt(dx * dx + dy * dy);
                float delta = r_f - dist;
                // Signed distance to 256-level coverage
                float cov = std::clamp(delta + 0.5f, 0.0f, 1.0f);
                mask_[y * radius + x] = static_cast<uint8_t>(cov * 255.0f + 0.5f);
            }
        }
    }

    [[nodiscard]] uint32_t radius() const noexcept { return radius_; }
    [[nodiscard]] bool is_valid() const noexcept { return radius_ > 0 && !mask_.empty(); }
    [[nodiscard]] const uint8_t* mask() const noexcept { return mask_.data(); }

private:
    uint32_t radius_{0};
    std::vector<uint8_t> mask_;
};

/// @brief Repository and Cache for Pre-Baked Canonical Shape Archetypes.
/// Eliminates redundant curve subdivision, edge building, and sorting across repetitive UI layouts.
class ArchetypeLibrary {
public:
    static ArchetypeLibrary& instance() {
        static ArchetypeLibrary s_instance;
        return s_instance;
    }

    /// @brief Gets or builds a canonical rounded rectangle archetype.
    std::shared_ptr<const ShapeArchetype> get_or_create_rounded_rect(
        uint32_t width, uint32_t height, float rx, float ry
    ) {
        if (width == 0 || height == 0) return nullptr;

        rx = std::clamp(rx, 0.0f, static_cast<float>(width) * 0.5f);
        ry = std::clamp(ry, 0.0f, static_cast<float>(height) * 0.5f);

        ArchetypeKey key{
            .type = ShapeType::RoundedRect,
            .width = static_cast<uint16_t>(width),
            .height = static_cast<uint16_t>(height),
            .param0 = static_cast<uint16_t>(rx * 10.0f + 0.5f),
            .param1 = static_cast<uint16_t>(ry * 10.0f + 0.5f),
            .param2 = 0
        };

        auto it = archetypes_.find(key);
        if (it != archetypes_.end()) {
            return it->second;
        }

        // Build canonical prototype
        auto r = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
        if (!r) return nullptr;

        auto path = PathBuilder::from_rounded_rect(*r, rx, ry);
        if (!path) return nullptr;

        auto mask_opt = Mask::allocate(width, height);
        if (!mask_opt) return nullptr;

        mask_opt->fill_path(*path, FillRule::Winding, true, Transform());

        auto arch = std::make_shared<ShapeArchetype>(width, height, mask_opt->take_data());
        archetypes_[key] = arch;
        ++archetypes_created_count_;
        return arch;
    }

    /// @brief Gets or builds a canonical circle archetype of specified radius.
    std::shared_ptr<const ShapeArchetype> get_or_create_circle(float radius) {
        if (radius <= 0.0f) return nullptr;
        uint32_t dim = static_cast<uint32_t>(std::ceil(radius * 2.0f));

        ArchetypeKey key{
            .type = ShapeType::Circle,
            .width = static_cast<uint16_t>(dim),
            .height = static_cast<uint16_t>(dim),
            .param0 = static_cast<uint16_t>(radius * 10.0f + 0.5f),
            .param1 = 0,
            .param2 = 0
        };

        auto it = archetypes_.find(key);
        if (it != archetypes_.end()) {
            return it->second;
        }

        auto path = PathBuilder::from_circle(radius, radius, radius);
        if (!path) return nullptr;

        auto mask_opt = Mask::allocate(dim, dim);
        if (!mask_opt) return nullptr;

        mask_opt->fill_path(*path, FillRule::Winding, true, Transform());

        auto arch = std::make_shared<ShapeArchetype>(dim, dim, mask_opt->take_data());
        archetypes_[key] = arch;
        ++archetypes_created_count_;
        return arch;
    }

    /// @brief Gets or builds a canonical pill / capsule archetype.
    std::shared_ptr<const ShapeArchetype> get_or_create_pill(uint32_t width, uint32_t height) {
        float r = static_cast<float>(std::min(width, height)) * 0.5f;
        return get_or_create_rounded_rect(width, height, r, r);
    }

    /// @brief Gets or builds a canonical corner quadrant for 9-slice variable-size buttons.
    std::shared_ptr<const CornerQuadrantArchetype> get_or_create_corner(uint32_t radius) {
        auto it = corners_.find(radius);
        if (it != corners_.end()) {
            return it->second;
        }
        auto corner = std::make_shared<CornerQuadrantArchetype>(radius);
        corners_[radius] = corner;
        return corner;
    }

    /// @brief Number of distinct archetypes compiled and cached.
    [[nodiscard]] size_t cached_archetypes_count() const noexcept {
        return archetypes_.size();
    }

    /// @brief Cumulative counter of archetypes created since initialization.
    [[nodiscard]] size_t total_created_count() const noexcept {
        return archetypes_created_count_;
    }

    /// @brief Clears the archetype cache.
    void clear() {
        archetypes_.clear();
        corners_.clear();
    }

private:
    ArchetypeLibrary() = default;

    std::unordered_map<ArchetypeKey, std::shared_ptr<ShapeArchetype>, ArchetypeKeyHash> archetypes_;
    std::unordered_map<uint32_t, std::shared_ptr<CornerQuadrantArchetype>> corners_;
    size_t archetypes_created_count_{0};
};

} // namespace nisaba::vspr

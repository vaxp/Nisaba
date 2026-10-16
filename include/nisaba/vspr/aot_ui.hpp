// Nisaba — Sovereign Full-Stack 2D Graphics Engine
// Copyright (C) 2024-2026 Hussein Jaafar
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once

/// @file aot_ui.hpp
/// @brief Nisaba Sovereign VSPR — Ahead-Of-Time (AOT) UI Scene Compilation & Dynamic Slot Punching.
///
/// Bakes 90% static UI scenes (backgrounds, drop shadows, glassmorphism, static text, icons)
/// into an immutable hardware-aligned texture plate once. Real-time updates occur exclusively
/// inside tagged Dynamic Slots (10% live data) and via a compact Component State Sub-Atlas
/// for interactive elements (Hover, Pressed), achieving sub-0.08 ms full-frame render latency.

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <optional>
#include <chrono>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cmath>
#include <algorithm>

#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/path/path.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/pipeline/simd.hpp"

#if defined(NISABA_HAS_GPU) || defined(NISABA_GLEW) || defined(NISABA_HAS_VULKAN)
#  include "nisaba/gpu/gpu_canvas.hpp"
#  include "nisaba/gpu/gpu_texture.hpp"
#  include "nisaba/gpu/context.hpp"
#endif

namespace nisaba::vspr::aot {

// =============================================================================
// 1. Dynamic Slot Types & Geometries
// =============================================================================

/// @brief Geometric shape type for punch-through dynamic slots.
enum class SlotShape : uint8_t {
    Rect = 0,         ///< Axis-aligned rectangular slot (fastest, hardware scissor clip).
    RoundedRect = 1,  ///< Rounded rectangle with corner radii (rx, ry).
    Circle = 2,       ///< Circular cutout slot.
    CustomPath = 3    ///< Arbitrary vector path cutout (stencil clip).
};

/// @brief Definition of a dynamic, live punch-through slot on the baked plate.
struct DynamicSlot {
    uint32_t id{0};
    std::string tag;
    SlotShape shape_type{SlotShape::Rect};
    Rect bounds{};
    float rx{0.0f};
    float ry{0.0f};
    std::optional<Path> custom_mask_path{std::nullopt};
    bool is_dirty{true};
    uint32_t z_order{0};

    [[nodiscard]] constexpr float width() const noexcept { return bounds.width(); }
    [[nodiscard]] constexpr float height() const noexcept { return bounds.height(); }

    [[nodiscard]] std::optional<ScreenIntRect> screen_rect() const noexcept {
        int32_t x0 = std::max(0, static_cast<int32_t>(std::round(bounds.left())));
        int32_t y0 = std::max(0, static_cast<int32_t>(std::round(bounds.top())));
        uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(bounds.width())));
        uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(bounds.height())));
        return ScreenIntRect::from_xywh(static_cast<uint32_t>(x0), static_cast<uint32_t>(y0), w, h);
    }
};

// =============================================================================
// 2. Component Interactive States & Sub-Atlas
// =============================================================================

/// @brief Discrete interactive states for UI components.
enum class ComponentState : uint8_t {
    Normal = 0,    ///< Default static state (already baked into base plate).
    Hover = 1,     ///< Cursor hover state.
    Pressed = 2,   ///< Active / clicked state.
    Disabled = 3,  ///< Inactive / disabled appearance.
    Focused = 4    ///< Keyboard / input focused state.
};

/// @brief Location of a pre-baked component state slice in the sub-atlas.
struct StateSlice {
    uint32_t comp_id{0};
    ComponentState state{ComponentState::Normal};
    Rect local_bounds{};
    uint32_t atlas_x{0};
    uint32_t atlas_y{0};
    uint32_t atlas_w{0};
    uint32_t atlas_h{0};
};

/// @brief Sub-Surface Atlas storing pre-rendered interactive states for UI components.
class ComponentStateAtlas {
public:
    ComponentStateAtlas() = default;

    /// @brief Registers an unbaked state slice dimension.
    void add_slice_meta(const StateSlice& slice) {
        slices_.push_back(slice);
    }

    /// @brief Sets the backing atlas pixmap and updates GPU texture if available.
    void set_atlas_pixmap(Pixmap pixmap) {
        atlas_pixmap_ = std::move(pixmap);
        gpu_image_id_ = -1;
    }

    [[nodiscard]] const Pixmap& pixmap() const noexcept { return atlas_pixmap_; }
    [[nodiscard]] Pixmap& pixmap_mut() noexcept { return atlas_pixmap_; }

    [[nodiscard]] const std::vector<StateSlice>& slices() const noexcept { return slices_; }

    [[nodiscard]] const StateSlice* find_slice(uint32_t comp_id, ComponentState state) const noexcept {
        for (const auto& s : slices_) {
            if (s.comp_id == comp_id && s.state == state) {
                return &s;
            }
        }
        return nullptr;
    }

    /// @brief Direct CPU SIMD Blit of a state slice onto target Pixmap.
    bool blit_state_cpu(PixmapMut& target, uint32_t comp_id, ComponentState state) const {
        const auto* slice = find_slice(comp_id, state);
        if (!slice || atlas_pixmap_.width() == 0 || atlas_pixmap_.height() == 0) {
            return false;
        }

        const int32_t dst_x = static_cast<int32_t>(std::round(slice->local_bounds.left()));
        const int32_t dst_y = static_cast<int32_t>(std::round(slice->local_bounds.top()));
        const uint32_t w = slice->atlas_w;
        const uint32_t h = slice->atlas_h;

        const auto* src_pixels = atlas_pixmap_.pixels();
        const size_t src_stride = atlas_pixmap_.width();

        for (uint32_t row = 0; row < h; ++row) {
            int32_t dy = dst_y + static_cast<int32_t>(row);
            if (dy < 0 || dy >= static_cast<int32_t>(target.height())) continue;

            int32_t sx = static_cast<int32_t>(slice->atlas_x);
            int32_t sy = static_cast<int32_t>(slice->atlas_y + row);

            const PremultipliedColorU8* src_row = src_pixels + sy * src_stride + sx;
            PremultipliedColorU8* dst_row = target.row(static_cast<size_t>(dy)) + dst_x;

            int32_t span_w = static_cast<int32_t>(w);
            if (dst_x + span_w > static_cast<int32_t>(target.width())) {
                span_w = static_cast<int32_t>(target.width()) - dst_x;
            }
            if (span_w <= 0) continue;

            simd::blend_source_over_span(dst_row, src_row, static_cast<size_t>(span_w));
        }
        return true;
    }

#if defined(NISABA_HAS_GPU) || defined(NISABA_GLEW) || defined(NISABA_HAS_VULKAN)
    /// @brief Direct GPU Blit of a state slice using hardware texture mapping.
    bool blit_state_gpu(gpu::GpuCanvas& canvas, uint32_t comp_id, ComponentState state) {
        auto* ctx = canvas.context();
        if (!ctx || atlas_pixmap_.width() == 0) return false;

        const auto* slice = find_slice(comp_id, state);
        if (!slice) return false;

        if (gpu_image_id_ <= 0) {
            gpu_image_id_ = ctx->createImageRGBA(
                static_cast<int>(atlas_pixmap_.width()),
                static_cast<int>(atlas_pixmap_.height()),
                0,
                atlas_pixmap_.data()
            );
        }

        if (gpu_image_id_ <= 0) return false;

        float dst_x = slice->local_bounds.left();
        float dst_y = slice->local_bounds.top();
        float w = static_cast<float>(slice->atlas_w);
        float h = static_cast<float>(slice->atlas_h);

        float ox = dst_x - static_cast<float>(slice->atlas_x);
        float oy = dst_y - static_cast<float>(slice->atlas_y);
        float aw = static_cast<float>(atlas_pixmap_.width());
        float ah = static_cast<float>(atlas_pixmap_.height());

        auto pattern = ctx->imagePattern(ox, oy, aw, ah, 0.0f, gpu_image_id_, 1.0f);
        ctx->beginPath();
        ctx->rect(dst_x, dst_y, w, h);
        ctx->fillPaint(pattern);
        ctx->fill();
        return true;
    }
#endif

private:
    std::vector<StateSlice> slices_;
    Pixmap atlas_pixmap_;
    int gpu_image_id_{-1};
};

// =============================================================================
// 3. Telemetry & Profiling Information
// =============================================================================

struct AotTelemetry {
    double total_frame_us{0.0};
    double base_plate_us{0.0};
    double state_atlas_us{0.0};
    size_t active_states_rendered{0};
    size_t bezier_curves_eliminated{0};
    double static_pixel_coverage_pct{0.0};
    double dynamic_pixel_coverage_pct{0.0};
};

// =============================================================================
// 4. CompiledUiSurface (Runtime Presentation & Compositor)
// =============================================================================

/// @brief Pre-compiled, immutable UI surface combining baked plate, slots, and states.
class CompiledUiSurface {
public:
    CompiledUiSurface(uint32_t width, uint32_t height)
        : width_(width), height_(height) {}

    [[nodiscard]] uint32_t width() const noexcept { return width_; }
    [[nodiscard]] uint32_t height() const noexcept { return height_; }

    [[nodiscard]] const Pixmap& base_plate() const noexcept { return base_plate_; }
    [[nodiscard]] Pixmap& base_plate_mut() noexcept { return base_plate_; }

    void set_base_plate(Pixmap plate) {
        base_plate_ = std::move(plate);
        gpu_base_image_id_ = -1;
    }

    [[nodiscard]] const ComponentStateAtlas& state_atlas() const noexcept { return state_atlas_; }
    [[nodiscard]] ComponentStateAtlas& state_atlas_mut() noexcept { return state_atlas_; }

    [[nodiscard]] const std::vector<DynamicSlot>& slots() const noexcept { return slots_; }
    [[nodiscard]] std::vector<DynamicSlot>& slots_mut() noexcept { return slots_; }

    [[nodiscard]] const AotTelemetry& telemetry() const noexcept { return telemetry_; }

    // -------------------------------------------------------------------------
    // Component State Management (Developer programmatic control)
    // -------------------------------------------------------------------------
    void set_component_state(uint32_t comp_id, ComponentState state) noexcept {
        if (state == ComponentState::Normal) {
            active_states_.erase(comp_id);
        } else {
            active_states_[comp_id] = state;
        }
    }

    [[nodiscard]] ComponentState get_component_state(uint32_t comp_id) const noexcept {
        auto it = active_states_.find(comp_id);
        if (it != active_states_.end()) {
            return it->second;
        }
        return ComponentState::Normal;
    }

    void reset_all_states() noexcept {
        active_states_.clear();
    }

    // -------------------------------------------------------------------------
    // Dynamic Slot Lookup & Clipping
    // -------------------------------------------------------------------------
    [[nodiscard]] DynamicSlot* find_slot(std::string_view tag) noexcept {
        for (auto& s : slots_) {
            if (s.tag == tag) return &s;
        }
        return nullptr;
    }

    [[nodiscard]] const DynamicSlot* find_slot(std::string_view tag) const noexcept {
        for (const auto& s : slots_) {
            if (s.tag == tag) return &s;
        }
        return nullptr;
    }

    void mark_slot_dirty(std::string_view tag, bool dirty = true) noexcept {
        auto* s = find_slot(tag);
        if (s) s->is_dirty = dirty;
    }

    // -------------------------------------------------------------------------
    // Direct CPU Frame Execution Path (Sub-0.08 ms)
    // -------------------------------------------------------------------------
    void render(PixmapMut& target) {
        auto t0 = std::chrono::high_resolution_clock::now();
        telemetry_ = {};

        if (base_plate_.width() == 0 || base_plate_.height() == 0) return;

        // 1. Base Plate Blit (Full Screen Quad / SIMD memcpy)
        auto t_base_0 = std::chrono::high_resolution_clock::now();
        const size_t copy_lines = std::min(static_cast<size_t>(target.height()), static_cast<size_t>(base_plate_.height()));
        const size_t copy_bytes = std::min(target.stride_bytes(), base_plate_.stride_bytes());
        const auto* src_data = base_plate_.data();
        const size_t src_stride = base_plate_.stride_bytes();

        for (size_t y = 0; y < copy_lines; ++y) {
            std::memcpy(target.row_raw(y), src_data + y * src_stride, copy_bytes);
        }
        auto t_base_1 = std::chrono::high_resolution_clock::now();
        telemetry_.base_plate_us = std::chrono::duration<double, std::micro>(t_base_1 - t_base_0).count();

        // 2. Active Component States Sub-Atlas Blit
        auto t_state_0 = std::chrono::high_resolution_clock::now();
        size_t states_rendered = 0;
        for (const auto& [comp_id, state] : active_states_) {
            if (state != ComponentState::Normal) {
                if (state_atlas_.blit_state_cpu(target, comp_id, state)) {
                    states_rendered++;
                }
            }
        }
        auto t_state_1 = std::chrono::high_resolution_clock::now();
        telemetry_.state_atlas_us = std::chrono::duration<double, std::micro>(t_state_1 - t_state_0).count();
        telemetry_.active_states_rendered = states_rendered;

        auto t1 = std::chrono::high_resolution_clock::now();
        telemetry_.total_frame_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        compute_coverage_metrics();
    }

    /// @brief Updates a dynamic slot on CPU with custom render callback.
    template <typename RenderFn>
    void update_slot_cpu(std::string_view tag, Canvas& canvas, RenderFn&& render_fn) {
        auto* slot = find_slot(tag);
        if (!slot) return;

        canvas.save();
        if (slot->shape_type == SlotShape::Rect) {
            canvas.clip_rect(slot->bounds);
        } else if (slot->shape_type == SlotShape::RoundedRect) {
            canvas.clip_rrect(RRect::from_rect_and_radius(slot->bounds, slot->rx, slot->ry));
        } else if (slot->shape_type == SlotShape::Circle) {
            float cx = slot->bounds.left() + slot->rx;
            float cy = slot->bounds.top() + slot->rx;
            auto path_opt = PathBuilder::from_circle(cx, cy, slot->rx);
            if (path_opt) canvas.clip_path(*path_opt);
        } else if (slot->shape_type == SlotShape::CustomPath && slot->custom_mask_path) {
            canvas.clip_path(*slot->custom_mask_path);
        }

        render_fn(canvas, slot->bounds);
        canvas.restore();
        slot->is_dirty = false;
    }

#if defined(NISABA_HAS_GPU) || defined(NISABA_GLEW) || defined(NISABA_HAS_VULKAN)
    // -------------------------------------------------------------------------
    // Direct GPU Frame Execution Path (Single Draw Call Base Blit)
    // -------------------------------------------------------------------------
    void render_gpu(gpu::GpuCanvas& canvas) {
        auto t0 = std::chrono::high_resolution_clock::now();
        telemetry_ = {};

        auto* ctx = canvas.context();
        if (!ctx || base_plate_.width() == 0) return;

        // 1. Upload base plate texture to GPU if not cached
        if (gpu_base_image_id_ <= 0) {
            gpu_base_image_id_ = ctx->createImageRGBA(
                static_cast<int>(base_plate_.width()),
                static_cast<int>(base_plate_.height()),
                0,
                base_plate_.data()
            );
        }

        // 2. Base Plate Quad Blit (Single Draw Call)
        auto t_base_0 = std::chrono::high_resolution_clock::now();
        if (gpu_base_image_id_ > 0) {
            float w = static_cast<float>(base_plate_.width());
            float h = static_cast<float>(base_plate_.height());
            auto pattern = ctx->imagePattern(0.0f, 0.0f, w, h, 0.0f, gpu_base_image_id_, 1.0f);
            ctx->beginPath();
            ctx->rect(0.0f, 0.0f, w, h);
            ctx->fillPaint(pattern);
            ctx->fill();
        }
        auto t_base_1 = std::chrono::high_resolution_clock::now();
        telemetry_.base_plate_us = std::chrono::duration<double, std::micro>(t_base_1 - t_base_0).count();

        // 3. Active Interactive States Blit
        auto t_state_0 = std::chrono::high_resolution_clock::now();
        size_t states_rendered = 0;
        for (const auto& [comp_id, state] : active_states_) {
            if (state != ComponentState::Normal) {
                if (state_atlas_.blit_state_gpu(canvas, comp_id, state)) {
                    states_rendered++;
                }
            }
        }
        auto t_state_1 = std::chrono::high_resolution_clock::now();
        telemetry_.state_atlas_us = std::chrono::duration<double, std::micro>(t_state_1 - t_state_0).count();
        telemetry_.active_states_rendered = states_rendered;

        auto t1 = std::chrono::high_resolution_clock::now();
        telemetry_.total_frame_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
        compute_coverage_metrics();
    }

    /// @brief Updates a dynamic slot on GPU with hardware clipping.
    template <typename RenderFn>
    void update_slot_gpu(std::string_view tag, gpu::GpuCanvas& canvas, RenderFn&& render_fn) {
        auto* slot = find_slot(tag);
        if (!slot) return;

        canvas.save();
        if (slot->shape_type == SlotShape::Rect) {
            canvas.clip_rect(slot->bounds);
        } else if (slot->shape_type == SlotShape::RoundedRect) {
            canvas.clip_rrect(RRect::from_rect_and_radius(slot->bounds, slot->rx, slot->ry));
        } else if (slot->shape_type == SlotShape::Circle) {
            float cx = slot->bounds.left() + slot->rx;
            float cy = slot->bounds.top() + slot->rx;
            auto path_opt = PathBuilder::from_circle(cx, cy, slot->rx);
            if (path_opt) canvas.clip_path(*path_opt);
        } else if (slot->shape_type == SlotShape::CustomPath && slot->custom_mask_path) {
            canvas.clip_path(*slot->custom_mask_path);
        }

        render_fn(canvas, slot->bounds);
        canvas.restore();
        slot->is_dirty = false;
    }
#endif

    // =========================================================================
    // 5. Native .vspr Binary Serialization & Instant Deserialization
    // =========================================================================

    /// Magic identifier for .vspr files ('V','S','P','R' in little-endian = 0x52505356)
    static constexpr uint32_t VSPR_MAGIC = 0x52505356;
    static constexpr uint32_t VSPR_VERSION = 1;

    /// @brief Saves the compiled surface to a high-speed binary .vspr file.
    bool save_to_file(const std::string& path) const {
        std::ofstream os(path, std::ios::binary);
        if (!os.is_open()) return false;
        return serialize(os);
    }

    /// @brief Loads a pre-compiled surface directly from a .vspr file (Zero-Compute Startup).
    static std::shared_ptr<CompiledUiSurface> load_from_file(const std::string& path) {
        std::ifstream is(path, std::ios::binary);
        if (!is.is_open()) return nullptr;
        return deserialize(is);
    }

    bool serialize(std::ostream& os) const {
        // 1. Header
        os.write(reinterpret_cast<const char*>(&VSPR_MAGIC), sizeof(VSPR_MAGIC));
        os.write(reinterpret_cast<const char*>(&VSPR_VERSION), sizeof(VSPR_VERSION));
        os.write(reinterpret_cast<const char*>(&width_), sizeof(width_));
        os.write(reinterpret_cast<const char*>(&height_), sizeof(height_));

        // 2. Dynamic Slots
        uint32_t num_slots = static_cast<uint32_t>(slots_.size());
        os.write(reinterpret_cast<const char*>(&num_slots), sizeof(num_slots));
        for (const auto& s : slots_) {
            os.write(reinterpret_cast<const char*>(&s.id), sizeof(s.id));
            uint32_t tag_len = static_cast<uint32_t>(s.tag.size());
            os.write(reinterpret_cast<const char*>(&tag_len), sizeof(tag_len));
            if (tag_len > 0) os.write(s.tag.data(), tag_len);

            uint8_t shape = static_cast<uint8_t>(s.shape_type);
            os.write(reinterpret_cast<const char*>(&shape), sizeof(shape));

            float b[4] = {s.bounds.left(), s.bounds.top(), s.bounds.right(), s.bounds.bottom()};
            os.write(reinterpret_cast<const char*>(b), sizeof(b));
            os.write(reinterpret_cast<const char*>(&s.rx), sizeof(s.rx));
            os.write(reinterpret_cast<const char*>(&s.ry), sizeof(s.ry));
            os.write(reinterpret_cast<const char*>(&s.z_order), sizeof(s.z_order));
        }

        // 3. State Slices
        const auto& slices = state_atlas_.slices();
        uint32_t num_slices = static_cast<uint32_t>(slices.size());
        os.write(reinterpret_cast<const char*>(&num_slices), sizeof(num_slices));
        for (const auto& sl : slices) {
            os.write(reinterpret_cast<const char*>(&sl.comp_id), sizeof(sl.comp_id));
            uint8_t st = static_cast<uint8_t>(sl.state);
            os.write(reinterpret_cast<const char*>(&st), sizeof(st));

            float b[4] = {sl.local_bounds.left(), sl.local_bounds.top(), sl.local_bounds.right(), sl.local_bounds.bottom()};
            os.write(reinterpret_cast<const char*>(b), sizeof(b));
            os.write(reinterpret_cast<const char*>(&sl.atlas_x), sizeof(sl.atlas_x));
            os.write(reinterpret_cast<const char*>(&sl.atlas_y), sizeof(sl.atlas_y));
            os.write(reinterpret_cast<const char*>(&sl.atlas_w), sizeof(sl.atlas_w));
            os.write(reinterpret_cast<const char*>(&sl.atlas_h), sizeof(sl.atlas_h));
        }

        // 4. Atlas Pixmap
        const auto& atlas_p = state_atlas_.pixmap();
        uint32_t aw = atlas_p.width();
        uint32_t ah = atlas_p.height();
        os.write(reinterpret_cast<const char*>(&aw), sizeof(aw));
        os.write(reinterpret_cast<const char*>(&ah), sizeof(ah));
        if (aw > 0 && ah > 0) {
            os.write(reinterpret_cast<const char*>(atlas_p.data()), atlas_p.data_len());
        }

        // 5. Base Plate Pixmap
        uint32_t pw = base_plate_.width();
        uint32_t ph = base_plate_.height();
        os.write(reinterpret_cast<const char*>(&pw), sizeof(pw));
        os.write(reinterpret_cast<const char*>(&ph), sizeof(ph));
        if (pw > 0 && ph > 0) {
            os.write(reinterpret_cast<const char*>(base_plate_.data()), base_plate_.data_len());
        }

        return os.good();
    }

    static std::shared_ptr<CompiledUiSurface> deserialize(std::istream& is) {
        uint32_t magic = 0, version = 0, w = 0, h = 0;
        is.read(reinterpret_cast<char*>(&magic), sizeof(magic));
        is.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (magic != VSPR_MAGIC || version != VSPR_VERSION) return nullptr;

        is.read(reinterpret_cast<char*>(&w), sizeof(w));
        is.read(reinterpret_cast<char*>(&h), sizeof(h));
        if (w == 0 || h == 0) return nullptr;

        auto surface = std::make_shared<CompiledUiSurface>(w, h);

        // 2. Dynamic Slots
        uint32_t num_slots = 0;
        is.read(reinterpret_cast<char*>(&num_slots), sizeof(num_slots));
        surface->slots_.reserve(num_slots);
        for (uint32_t i = 0; i < num_slots; ++i) {
            DynamicSlot s;
            is.read(reinterpret_cast<char*>(&s.id), sizeof(s.id));
            uint32_t tag_len = 0;
            is.read(reinterpret_cast<char*>(&tag_len), sizeof(tag_len));
            if (tag_len > 0) {
                s.tag.resize(tag_len);
                is.read(s.tag.data(), tag_len);
            }
            uint8_t shape = 0;
            is.read(reinterpret_cast<char*>(&shape), sizeof(shape));
            s.shape_type = static_cast<SlotShape>(shape);

            float b[4] = {0.0f};
            is.read(reinterpret_cast<char*>(b), sizeof(b));
            auto rect_opt = Rect::from_ltrb(b[0], b[1], b[2], b[3]);
            if (rect_opt) s.bounds = *rect_opt;

            is.read(reinterpret_cast<char*>(&s.rx), sizeof(s.rx));
            is.read(reinterpret_cast<char*>(&s.ry), sizeof(s.ry));
            is.read(reinterpret_cast<char*>(&s.z_order), sizeof(s.z_order));
            surface->slots_.push_back(std::move(s));
        }

        // 3. State Slices
        uint32_t num_slices = 0;
        is.read(reinterpret_cast<char*>(&num_slices), sizeof(num_slices));
        for (uint32_t i = 0; i < num_slices; ++i) {
            StateSlice sl;
            is.read(reinterpret_cast<char*>(&sl.comp_id), sizeof(sl.comp_id));
            uint8_t st = 0;
            is.read(reinterpret_cast<char*>(&st), sizeof(st));
            sl.state = static_cast<ComponentState>(st);

            float b[4] = {0.0f};
            is.read(reinterpret_cast<char*>(b), sizeof(b));
            auto rect_opt = Rect::from_ltrb(b[0], b[1], b[2], b[3]);
            if (rect_opt) sl.local_bounds = *rect_opt;

            is.read(reinterpret_cast<char*>(&sl.atlas_x), sizeof(sl.atlas_x));
            is.read(reinterpret_cast<char*>(&sl.atlas_y), sizeof(sl.atlas_y));
            is.read(reinterpret_cast<char*>(&sl.atlas_w), sizeof(sl.atlas_w));
            is.read(reinterpret_cast<char*>(&sl.atlas_h), sizeof(sl.atlas_h));
            surface->state_atlas_.add_slice_meta(sl);
        }

        // 4. Atlas Pixmap
        uint32_t aw = 0, ah = 0;
        is.read(reinterpret_cast<char*>(&aw), sizeof(aw));
        is.read(reinterpret_cast<char*>(&ah), sizeof(ah));
        if (aw > 0 && ah > 0) {
            auto ap = Pixmap::create(aw, ah);
            if (ap) {
                is.read(reinterpret_cast<char*>(ap->data_mut()), ap->data_len());
                surface->state_atlas_.set_atlas_pixmap(std::move(*ap));
            }
        }

        // 5. Base Plate Pixmap
        uint32_t pw = 0, ph = 0;
        is.read(reinterpret_cast<char*>(&pw), sizeof(pw));
        is.read(reinterpret_cast<char*>(&ph), sizeof(ph));
        if (pw > 0 && ph > 0) {
            auto bp = Pixmap::create(pw, ph);
            if (bp) {
                is.read(reinterpret_cast<char*>(bp->data_mut()), bp->data_len());
                surface->base_plate_ = std::move(*bp);
            }
        }

        if (!is.good()) return nullptr;
        surface->compute_coverage_metrics();
        return surface;
    }

    void compute_coverage_metrics() {
        double total_pixels = static_cast<double>(width_) * static_cast<double>(height_);
        if (total_pixels <= 0.0) return;

        double slot_pixels = 0.0;
        for (const auto& s : slots_) {
            slot_pixels += (s.width() * s.height());
        }
        telemetry_.dynamic_pixel_coverage_pct = std::min(100.0, (slot_pixels / total_pixels) * 100.0);
        telemetry_.static_pixel_coverage_pct = std::max(0.0, 100.0 - telemetry_.dynamic_pixel_coverage_pct);
    }

private:

    uint32_t width_{0};
    uint32_t height_{0};
    Pixmap base_plate_{};
    ComponentStateAtlas state_atlas_{};
    std::vector<DynamicSlot> slots_{};
    std::unordered_map<uint32_t, ComponentState> active_states_{};
    int gpu_base_image_id_{-1};
    AotTelemetry telemetry_{};
};

// =============================================================================
// 6. AotSceneCompiler (Offline / Initialization Baking Engine)
// =============================================================================

/// @brief Builder and Compiler for baking scenes, punch-through slots, and interactive states.
class AotSceneCompiler {
public:
    explicit AotSceneCompiler(uint32_t width, uint32_t height)
        : width_(width), height_(height) {}

    // -------------------------------------------------------------------------
    // Slot Registrations
    // -------------------------------------------------------------------------
    uint32_t register_slot_rect(std::string_view tag, const Rect& bounds, uint32_t z_order = 0) {
        DynamicSlot slot;
        slot.id = next_slot_id_++;
        slot.tag = std::string(tag);
        slot.shape_type = SlotShape::Rect;
        slot.bounds = bounds;
        slot.z_order = z_order;
        slots_.push_back(std::move(slot));
        return slot.id;
    }

    uint32_t register_slot_rrect(std::string_view tag, const Rect& bounds, float rx, float ry, uint32_t z_order = 0) {
        DynamicSlot slot;
        slot.id = next_slot_id_++;
        slot.tag = std::string(tag);
        slot.shape_type = SlotShape::RoundedRect;
        slot.bounds = bounds;
        slot.rx = rx;
        slot.ry = ry;
        slot.z_order = z_order;
        slots_.push_back(std::move(slot));
        return slot.id;
    }

    uint32_t register_slot_circle(std::string_view tag, Point center, float radius, uint32_t z_order = 0) {
        DynamicSlot slot;
        slot.id = next_slot_id_++;
        slot.tag = std::string(tag);
        slot.shape_type = SlotShape::Circle;
        auto r_opt = Rect::from_xywh(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f);
        if (r_opt) slot.bounds = *r_opt;
        slot.rx = radius;
        slot.ry = radius;
        slot.z_order = z_order;
        slots_.push_back(std::move(slot));
        return slot.id;
    }

    uint32_t register_slot_path(std::string_view tag, const Rect& bounds, const Path& mask_path, uint32_t z_order = 0) {
        DynamicSlot slot;
        slot.id = next_slot_id_++;
        slot.tag = std::string(tag);
        slot.shape_type = SlotShape::CustomPath;
        slot.bounds = bounds;
        slot.custom_mask_path = mask_path;
        slot.z_order = z_order;
        slots_.push_back(std::move(slot));
        return slot.id;
    }

    // -------------------------------------------------------------------------
    // Component State Registration
    // -------------------------------------------------------------------------
    void register_component_state(
        uint32_t comp_id,
        ComponentState state,
        const Rect& bounds,
        std::function<void(Canvas&, const Rect& local_bounds)> render_fn
    ) {
        UnbakedState u;
        u.comp_id = comp_id;
        u.state = state;
        u.bounds = bounds;
        u.render_fn = std::move(render_fn);
        unbaked_states_.push_back(std::move(u));
    }

    // -------------------------------------------------------------------------
    // Static Scene Recording & Baking Options
    // -------------------------------------------------------------------------
    void record_static_pass(std::function<void(Canvas&)> render_fn) {
        static_render_fn_ = std::move(render_fn);
    }

    void set_punch_out_slots(bool punch) noexcept {
        punch_out_slots_ = punch;
    }

    // -------------------------------------------------------------------------
    // Compilation Execution
    // -------------------------------------------------------------------------
    [[nodiscard]] std::shared_ptr<CompiledUiSurface> compile() {
        auto surface = std::make_shared<CompiledUiSurface>(width_, height_);

        // 1. Bake Static Base Plate
        auto plate_opt = Pixmap::create(width_, height_);
        if (!plate_opt) return nullptr;
        plate_opt->fill(Color::TRANSPARENT);

        Canvas canvas(*plate_opt);
        if (static_render_fn_) {
            static_render_fn_(canvas);
        }

        // Punch out slots if requested (clears slot area to transparent)
        if (punch_out_slots_) {
            for (const auto& s : slots_) {
                int32_t x0 = std::max(0, static_cast<int32_t>(std::round(s.bounds.left())));
                int32_t y0 = std::max(0, static_cast<int32_t>(std::round(s.bounds.top())));
                uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(s.bounds.width())));
                uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(s.bounds.height())));

                auto mut_plate = plate_opt->as_mut();
                for (uint32_t r = 0; r < h; ++r) {
                    uint32_t cy = static_cast<uint32_t>(y0) + r;
                    if (cy >= plate_opt->height()) break;
                    PremultipliedColorU8* row = mut_plate.row(cy) + x0;
                    size_t clear_len = std::min(static_cast<size_t>(w), static_cast<size_t>(plate_opt->width() - x0));
                    for (size_t k = 0; k < clear_len; ++k) {
                        row[k] = PremultipliedColorU8::TRANSPARENT;
                    }
                }
            }
        }
        surface->set_base_plate(std::move(*plate_opt));

        // 2. Transfer Dynamic Slots
        surface->slots_mut() = slots_;

        // 3. Bake Component State Atlas (Shelf Packing)
        if (!unbaked_states_.empty()) {
            bake_state_atlas(*surface);
        }
        surface->compute_coverage_metrics();
        return surface;
    }

#if defined(NISABA_HAS_GPU) || defined(NISABA_GLEW) || defined(NISABA_HAS_VULKAN)
    /// @brief Direct Hardware GPU Compilation & Baking Path (Pre-renders scene at 4x MSAA).
    [[nodiscard]] std::shared_ptr<CompiledUiSurface> compile_gpu(
        std::shared_ptr<gpu::GpuDevice> device,
        std::function<void(gpu::GpuCanvas&)> gpu_render_fn,
        std::function<void(gpu::Context*)> font_init_fn = nullptr
    ) {
        if (!device) return nullptr;
        auto surface = std::make_shared<CompiledUiSurface>(width_, height_);

        auto gpu_surf = gpu::GpuSurface::create(device, width_, height_, 4);
        if (!gpu_surf) return nullptr;

        device->make_current();
        gpu_surf->bind();
        gpu::GpuCanvas canvas(gpu_surf);
        canvas.clear(Color::from_rgba8(10, 13, 22, 255));

        if (font_init_fn && canvas.context()) {
            font_init_fn(canvas.context());
        }

        if (gpu_render_fn) {
            gpu_render_fn(canvas);
        }
        canvas.flush();

        gpu_surf->resolve();
        auto pix = gpu_surf->to_pixmap();
        if (pix) {
            surface->set_base_plate(std::move(*pix));
        }

        surface->slots_mut() = slots_;
        if (!unbaked_states_.empty()) {
            bake_state_atlas(*surface);
        }
        surface->compute_coverage_metrics();
        return surface;
    }
#endif

private:
    struct UnbakedState {
        uint32_t comp_id{0};
        ComponentState state{ComponentState::Normal};
        Rect bounds{};
        std::function<void(Canvas&, const Rect& local_bounds)> render_fn;
    };

    void bake_state_atlas(CompiledUiSurface& surface) {
        // Simple Shelf Bin-Packing for state slices
        uint32_t total_w = 0;
        uint32_t max_h = 0;
        for (const auto& u : unbaked_states_) {
            uint32_t w = static_cast<uint32_t>(std::ceil(u.bounds.width())) + 4; // 2px gutter
            uint32_t h = static_cast<uint32_t>(std::ceil(u.bounds.height())) + 4;
            total_w += w;
            if (h > max_h) max_h = h;
        }

        // Clamp width into a balanced texture (e.g. max 2048 wide)
        uint32_t atlas_w = std::min(total_w, 2048u);
        if (atlas_w == 0) atlas_w = 64;
        if (max_h == 0) max_h = 64;

        // Estimate required height for multi-shelf
        uint32_t cur_x = 0;
        uint32_t cur_y = 0;
        uint32_t shelf_h = 0;

        struct PlannedSlice {
            UnbakedState u;
            uint32_t ax{0};
            uint32_t ay{0};
            uint32_t aw{0};
            uint32_t ah{0};
        };
        std::vector<PlannedSlice> plan;
        plan.reserve(unbaked_states_.size());

        for (const auto& u : unbaked_states_) {
            uint32_t w = static_cast<uint32_t>(std::ceil(u.bounds.width()));
            uint32_t h = static_cast<uint32_t>(std::ceil(u.bounds.height()));
            uint32_t pad_w = w + 4;
            uint32_t pad_h = h + 4;

            if (cur_x + pad_w > atlas_w) {
                cur_x = 0;
                cur_y += shelf_h;
                shelf_h = 0;
            }

            PlannedSlice ps;
            ps.u = u;
            ps.ax = cur_x + 2;
            ps.ay = cur_y + 2;
            ps.aw = w;
            ps.ah = h;
            plan.push_back(ps);

            cur_x += pad_w;
            if (pad_h > shelf_h) shelf_h = pad_h;
        }
        uint32_t atlas_h = cur_y + shelf_h;
        if (atlas_h == 0) atlas_h = 64;

        auto atlas_opt = Pixmap::create(atlas_w, atlas_h);
        if (!atlas_opt) return;
        atlas_opt->fill(Color::TRANSPARENT);

        Canvas atlas_canvas(*atlas_opt);

        // Render each state slice into its allocated shelf position
        for (const auto& ps : plan) {
            auto slice_rect_opt = Rect::from_xywh(
                static_cast<float>(ps.ax), static_cast<float>(ps.ay),
                static_cast<float>(ps.aw), static_cast<float>(ps.ah)
            );
            if (!slice_rect_opt) continue;

            atlas_canvas.save();
            atlas_canvas.clip_rect(*slice_rect_opt);
            atlas_canvas.translate(static_cast<float>(ps.ax), static_cast<float>(ps.ay));

            auto local_zero_opt = Rect::from_xywh(0.0f, 0.0f, static_cast<float>(ps.aw), static_cast<float>(ps.ah));
            if (local_zero_opt && ps.u.render_fn) {
                ps.u.render_fn(atlas_canvas, *local_zero_opt);
            }
            atlas_canvas.restore();

            StateSlice slice;
            slice.comp_id = ps.u.comp_id;
            slice.state = ps.u.state;
            slice.local_bounds = ps.u.bounds;
            slice.atlas_x = ps.ax;
            slice.atlas_y = ps.ay;
            slice.atlas_w = ps.aw;
            slice.atlas_h = ps.ah;
            surface.state_atlas_mut().add_slice_meta(slice);
        }

        surface.state_atlas_mut().set_atlas_pixmap(std::move(*atlas_opt));
    }

    uint32_t width_{0};
    uint32_t height_{0};
    uint32_t next_slot_id_{1};
    bool punch_out_slots_{false};
    std::vector<DynamicSlot> slots_{};
    std::vector<UnbakedState> unbaked_states_{};
    std::function<void(Canvas&)> static_render_fn_{};
};

} // namespace nisaba::vspr::aot

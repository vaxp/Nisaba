// Nisaba — Sovereign Full-Stack 2D Graphics Engine
// Copyright (C) 2024-2026 Hussein Jaafar
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#pragma once

#include <vector>
#include <memory>
#include <chrono>
#include <optional>
#include <algorithm>

#include "nisaba/math/rect.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/screen_int_rect.hpp"
#include "nisaba/math/border_radius.hpp"
#include "nisaba/math/rrect.hpp"
#include "nisaba/color/color.hpp"
#include "nisaba/canvas/pixmap.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/vspr/shape_archetype.hpp"

#if defined(NISABA_HAS_GPU) || defined(NISABA_GLEW) || defined(NISABA_HAS_VULKAN)
#include "nisaba/gpu/gpu_canvas.hpp"
#endif

namespace nisaba::vspr {

/// @brief Represents a single visual instance of a primitive shape with customized styling.
struct ShapeInstance {
    ShapeType type{ShapeType::RoundedRect};
    Rect bounds{};
    Color fill_color{Color::BLACK};
    BorderRadius radii{};
    float border_width{0.0f};
    Color border_color{Color::TRANSPARENT};
    float opacity{1.0f};

    constexpr ShapeInstance() noexcept = default;

    static ShapeInstance button(
        const Rect& b,
        const Color& fill,
        float radius = 8.0f,
        float border_w = 0.0f,
        const Color& border_col = Color::TRANSPARENT
    ) noexcept {
        ShapeInstance inst;
        inst.type = ShapeType::RoundedRect;
        inst.bounds = b;
        inst.fill_color = fill;
        inst.radii = BorderRadius::all(radius);
        inst.border_width = border_w;
        inst.border_color = border_col;
        return inst;
    }

    static ShapeInstance circle(
        Point center,
        float radius,
        const Color& fill,
        float border_w = 0.0f,
        const Color& border_col = Color::TRANSPARENT
    ) noexcept {
        ShapeInstance inst;
        inst.type = ShapeType::Circle;
        inst.bounds = Rect::from_xywh(center.x - radius, center.y - radius, radius * 2.0f, radius * 2.0f).value_or(Rect{});
        inst.fill_color = fill;
        inst.radii = BorderRadius::all(radius);
        inst.border_width = border_w;
        inst.border_color = border_col;
        return inst;
    }

    static ShapeInstance pill(
        const Rect& b,
        const Color& fill,
        float border_w = 0.0f,
        const Color& border_col = Color::TRANSPARENT
    ) noexcept {
        ShapeInstance inst;
        inst.type = ShapeType::Pill;
        inst.bounds = b;
        inst.fill_color = fill;
        float r = std::min(b.width(), b.height()) * 0.5f;
        inst.radii = BorderRadius::all(r);
        inst.border_width = border_w;
        inst.border_color = border_col;
        return inst;
    }

    static ShapeInstance rect(
        const Rect& b,
        const Color& fill,
        float border_w = 0.0f,
        const Color& border_col = Color::TRANSPARENT
    ) noexcept {
        ShapeInstance inst;
        inst.type = ShapeType::Rect;
        inst.bounds = b;
        inst.fill_color = fill;
        inst.border_width = border_w;
        inst.border_color = border_col;
        return inst;
    }
};

/// @brief Real-time performance metrics for the Primitive Instancer subsystem.
struct InstancerTelemetry {
    size_t total_instances{0};
    size_t archetypes_reused{0};
    size_t archetypes_newly_built{0};
    size_t bezier_curves_saved{0};
    double render_time_us{0.0};
};

/// @brief VSPR Primitive & Shape Instancing Engine.
/// Implements the Canonical Prototype & Geometric Instancing paradigm:
/// Constructs standard shapes (buttons, cards, badges, circles, pills) ONCE,
/// and blits/instances them across repetitive UI scenes with per-instance color,
/// position, and styling without re-evaluating curve equations or rasterizer edge trees.
class PrimitiveInstancer {
public:
    PrimitiveInstancer() = default;

    /// @brief Clears queued instances while keeping pre-allocated archetypes hot in L1/L2 cache.
    void clear() noexcept {
        instances_.clear();
    }

    /// @brief Reserves capacity for instance streaming.
    void reserve(size_t capacity) {
        instances_.reserve(capacity);
    }

    /// @brief Adds a generic shape instance.
    void add_instance(const ShapeInstance& instance) {
        instances_.push_back(instance);
    }

    /// @brief Adds an interactive UI button with uniform or specified radius.
    void add_button(
        const Rect& bounds,
        const Color& fill,
        float radius = 8.0f,
        float border_width = 0.0f,
        const Color& border_color = Color::TRANSPARENT
    ) {
        instances_.push_back(ShapeInstance::button(bounds, fill, radius, border_width, border_color));
    }

    /// @brief Adds a rounded rectangle card or panel.
    void add_rounded_rect(
        const Rect& bounds,
        const Color& fill,
        const BorderRadius& radii,
        float border_width = 0.0f,
        const Color& border_color = Color::TRANSPARENT
    ) {
        ShapeInstance inst;
        inst.type = ShapeType::RoundedRect;
        inst.bounds = bounds;
        inst.fill_color = fill;
        inst.radii = radii;
        inst.border_width = border_width;
        inst.border_color = border_color;
        instances_.push_back(inst);
    }

    /// @brief Adds a circular badge, indicator dot, or avatar.
    void add_circle(Point center, float radius, const Color& fill, float border_width = 0.0f, const Color& border_color = Color::TRANSPARENT) {
        instances_.push_back(ShapeInstance::circle(center, radius, fill, border_width, border_color));
    }

    /// @brief Adds a pill / capsule badge or tag.
    void add_pill(const Rect& bounds, const Color& fill, float border_width = 0.0f, const Color& border_color = Color::TRANSPARENT) {
        instances_.push_back(ShapeInstance::pill(bounds, fill, border_width, border_color));
    }

    /// @brief Adds a sharp rectangle.
    void add_rect(const Rect& bounds, const Color& fill, float border_width = 0.0f, const Color& border_color = Color::TRANSPARENT) {
        instances_.push_back(ShapeInstance::rect(bounds, fill, border_width, border_color));
    }

    /// @brief Number of instances queued for the current batch.
    [[nodiscard]] size_t instance_count() const noexcept {
        return instances_.size();
    }

    /// @brief Direct CPU Execution Path: renders all queued instances directly onto target Pixmap.
    void render(PixmapMut& target, const std::optional<ScreenIntRect>& clip = std::nullopt) {
        auto start_time = std::chrono::high_resolution_clock::now();
        telemetry_ = {};
        telemetry_.total_instances = instances_.size();

        if (instances_.empty()) return;

        auto& library = ArchetypeLibrary::instance();
        size_t initial_archetype_count = library.cached_archetypes_count();

        for (const auto& inst : instances_) {
            if (inst.bounds.is_empty()) continue;

            const int32_t x = static_cast<int32_t>(std::round(inst.bounds.left()));
            const int32_t y = static_cast<int32_t>(std::round(inst.bounds.top()));
            const uint32_t w = static_cast<uint32_t>(std::max(1.0f, std::round(inst.bounds.width())));
            const uint32_t h = static_cast<uint32_t>(std::max(1.0f, std::round(inst.bounds.height())));

            PremultipliedColorU8 premult = inst.fill_color.to_color_u8().premultiply();
            if (inst.opacity < 0.999f) {
                premult.a = static_cast<uint8_t>(premult.a * inst.opacity + 0.5f);
                premult.r = static_cast<uint8_t>(premult.r * inst.opacity + 0.5f);
                premult.g = static_cast<uint8_t>(premult.g * inst.opacity + 0.5f);
                premult.b = static_cast<uint8_t>(premult.b * inst.opacity + 0.5f);
            }

            switch (inst.type) {
                case ShapeType::Rect: {
                    // Fast pure SIMD fill for sharp rects
                    int32_t x0 = std::max(0, x);
                    int32_t y0 = std::max(0, y);
                    int32_t x1 = std::min(static_cast<int32_t>(target.width()), x + static_cast<int32_t>(w));
                    int32_t y1 = std::min(static_cast<int32_t>(target.height()), y + static_cast<int32_t>(h));
                    if (clip.has_value()) {
                        x0 = std::max(x0, static_cast<int32_t>(clip->x()));
                        y0 = std::max(y0, static_cast<int32_t>(clip->y()));
                        x1 = std::min(x1, static_cast<int32_t>(clip->right()));
                        y1 = std::min(y1, static_cast<int32_t>(clip->bottom()));
                    }
                    if (x0 < x1 && y0 < y1) {
                        size_t span_len = static_cast<size_t>(x1 - x0);
                        for (int32_t cy = y0; cy < y1; ++cy) {
                            PremultipliedColorU8* dst_row = target.row(static_cast<size_t>(cy)) + x0;
                            if (premult.a == 255) {
                                simd::fill_solid_span(dst_row, premult, span_len);
                            } else {
                                simd::blend_solid_source_over(dst_row, premult, span_len);
                            }
                        }
                    }
                    break;
                }

                case ShapeType::RoundedRect: {
                    float r = inst.radii.top_left;
                    auto archetype = library.get_or_create_rounded_rect(w, h, r, r);
                    if (archetype) {
                        archetype->blit(target, x, y, premult, clip);
                        telemetry_.bezier_curves_saved += 4; // 4 cubic Bezier curves avoided!
                    }
                    break;
                }

                case ShapeType::Circle: {
                    float r = static_cast<float>(std::min(w, h)) * 0.5f;
                    auto archetype = library.get_or_create_circle(r);
                    if (archetype) {
                        archetype->blit(target, x, y, premult, clip);
                        telemetry_.bezier_curves_saved += 4;
                    }
                    break;
                }

                case ShapeType::Pill: {
                    auto archetype = library.get_or_create_pill(w, h);
                    if (archetype) {
                        archetype->blit(target, x, y, premult, clip);
                        telemetry_.bezier_curves_saved += 4;
                    }
                    break;
                }

                default:
                    break;
            }
        }

        size_t final_archetype_count = library.cached_archetypes_count();
        telemetry_.archetypes_newly_built = final_archetype_count - initial_archetype_count;
        telemetry_.archetypes_reused = instances_.size() - telemetry_.archetypes_newly_built;

        auto end_time = std::chrono::high_resolution_clock::now();
        telemetry_.render_time_us = std::chrono::duration<double, std::micro>(end_time - start_time).count();
    }

    /// @brief Canvas Execution Path: renders all queued instances onto a Canvas surface.
    void render(Canvas& canvas) {
        render(canvas.pixmap());
    }

#if defined(NISABA_HAS_GPU) || defined(NISABA_GLEW) || defined(NISABA_HAS_VULKAN)
    /// @brief GPU Execution Path: streams queued instances to hardware GPU canvas with archetype tracking.
    void render_gpu(gpu::GpuCanvas& gpu_canvas) {
        auto start_time = std::chrono::high_resolution_clock::now();
        telemetry_ = {};
        telemetry_.total_instances = instances_.size();

        if (instances_.empty()) return;

        auto& library = ArchetypeLibrary::instance();
        size_t initial_count = library.cached_archetypes_count();

        for (const auto& inst : instances_) {
            if (inst.type == ShapeType::RoundedRect) {
                library.get_or_create_rounded_rect(
                    static_cast<uint32_t>(inst.bounds.width()),
                    static_cast<uint32_t>(inst.bounds.height()),
                    inst.radii.top_left, inst.radii.top_left
                );
            } else if (inst.type == ShapeType::Circle) {
                float r = std::min(inst.bounds.width(), inst.bounds.height()) * 0.5f;
                library.get_or_create_circle(r);
            } else if (inst.type == ShapeType::Pill) {
                library.get_or_create_pill(
                    static_cast<uint32_t>(inst.bounds.width()),
                    static_cast<uint32_t>(inst.bounds.height())
                );
            }
        }
        size_t final_count = library.cached_archetypes_count();
        telemetry_.archetypes_newly_built = final_count - initial_count;
        telemetry_.archetypes_reused = instances_.size() - telemetry_.archetypes_newly_built;

        for (const auto& inst : instances_) {
            if (inst.bounds.is_empty()) continue;

            Paint paint(inst.fill_color);
            if (inst.opacity < 0.999f) {
                Color c = inst.fill_color;
                c.apply_opacity(inst.opacity);
                paint.set_color(c);
            }

            switch (inst.type) {
                case ShapeType::Rect:
                    gpu_canvas.fill_rect(inst.bounds, paint);
                    break;
                case ShapeType::RoundedRect:
                case ShapeType::Pill:
                    gpu_canvas.fill_rrect(inst.bounds, inst.radii, paint);
                    telemetry_.bezier_curves_saved += 4;
                    break;
                case ShapeType::Circle: {
                    float r = std::min(inst.bounds.width(), inst.bounds.height()) * 0.5f;
                    gpu_canvas.fill_circle(inst.bounds.left() + r, inst.bounds.top() + r, r, paint);
                    telemetry_.bezier_curves_saved += 4;
                    break;
                }
                default:
                    break;
            }

            if (inst.border_width > 0.0f && inst.border_color.alpha() > 0.001f) {
                Paint border_paint(inst.border_color);
                Stroke stroke(inst.border_width);
                switch (inst.type) {
                    case ShapeType::Rect:
                        gpu_canvas.stroke_rect(inst.bounds, border_paint, stroke);
                        break;
                    case ShapeType::RoundedRect:
                    case ShapeType::Pill:
                        gpu_canvas.stroke_rrect(inst.bounds, inst.radii, border_paint, stroke);
                        break;
                    case ShapeType::Circle: {
                        float r = std::min(inst.bounds.width(), inst.bounds.height()) * 0.5f;
                        gpu_canvas.stroke_circle(inst.bounds.left() + r, inst.bounds.top() + r, r, border_paint, stroke);
                        break;
                    }
                    default:
                        break;
                }
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        telemetry_.render_time_us = std::chrono::duration<double, std::micro>(end_time - start_time).count();
    }
#endif

    /// @brief Accesses execution telemetry and profiling statistics.
    [[nodiscard]] const InstancerTelemetry& telemetry() const noexcept {
        return telemetry_;
    }

private:
    std::vector<ShapeInstance> instances_;
    InstancerTelemetry telemetry_{};
};

} // namespace nisaba::vspr

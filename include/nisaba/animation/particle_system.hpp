#pragma once

/// @file particle_system.hpp
/// @brief High-performance 2D Particle Simulation Engine for Nisaba.
/// Zero runtime allocations, pure physics simulation, and direct Nisaba Canvas rendering.

#include "nisaba/color/color.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/painter.hpp"
#include "nisaba/path/path_builder.hpp"
#include "nisaba/math/point.hpp"
#include "nisaba/math/rect.hpp"
#include "nisaba/math/size.hpp"
#include "nisaba/math/transform.hpp"

#include <vector>
#include <random>
#include <memory>
#include <chrono>

namespace nisaba::animation {

enum class ParticleShape {
    Circle,
    Square,
    ConfettiRibbon,
    Star,
};

enum class ParticlePreset {
    ConfettiBurst,
    NeonSparks,
    AmbientDust,
    SnowFall,
};

struct Particle {
    Point position{0.0f, 0.0f};
    Point velocity{0.0f, 0.0f};
    Color color = Color::WHITE;
    float size  = 4.0f;
    float rotation = 0.0f;
    float rotation_speed = 0.0f;
    float life  = 1.0f;
    float max_life = 1.0f;
    bool  alive = false;
};

struct ParticleConfig {
    size_t             max_particles    = 150;
    float              emission_rate    = 60.0f;  ///< Particles per second (for continuous emitters)
    Point              gravity          = {0.0f, 400.0f}; ///< Gravity acceleration (pixels/s^2)
    float              drag             = 0.02f;  ///< Air resistance drag factor
    float              min_life         = 0.8f;
    float              max_life         = 1.8f;
    float              min_speed        = 120.0f;
    float              max_speed        = 320.0f;
    float              min_angle_rad    = 0.0f;
    float              max_angle_rad    = 6.2831853f; // 2 * PI
    float              min_size         = 4.0f;
    float              max_size         = 10.0f;
    std::vector<Color> color_palette;
    ParticleShape      shape            = ParticleShape::ConfettiRibbon;
    bool               burst_mode       = false;  ///< If true, emits all particles in one explosive burst
};

struct ParticlePresets {
    static ParticleConfig confetti();
    static ParticleConfig neon_sparks();
    static ParticleConfig ambient_dust();
    static ParticleConfig snow_fall();
    static ParticleConfig from_preset(ParticlePreset preset);

    static ParticleConfig neonSparks() { return neon_sparks(); }
    static ParticleConfig ambientDust() { return ambient_dust(); }
    static ParticleConfig snowFall() { return snow_fall(); }
    static ParticleConfig fromPreset(ParticlePreset preset) { return from_preset(preset); }
};

class ParticleSystem {
public:
    explicit ParticleSystem(const ParticleConfig& config = ParticlePresets::confetti());

    void set_config(const ParticleConfig& config);
    void setConfig(const ParticleConfig& config) { set_config(config); }
    [[nodiscard]] const ParticleConfig& config() const noexcept { return config_; }

    /// Trigger an explosive burst from origin point
    void burst(Point origin);

    /// Set continuous emitter state
    void set_emitting(bool emitting) noexcept { is_emitting_ = emitting; }
    void setEmitting(bool emitting) noexcept { set_emitting(emitting); }
    [[nodiscard]] bool is_emitting() const noexcept { return is_emitting_; }
    [[nodiscard]] bool isEmitting() const noexcept { return is_emitting(); }

    /// Set continuous emission spawn origin
    void set_emitter_position(Point origin) noexcept { emitter_origin_ = origin; }
    void setEmitterPosition(Point origin) noexcept { set_emitter_position(origin); }

    /// Advance particle physics by delta time (in seconds)
    void update(float dt_sec, Size boundary = Size::from_wh(1600.0f, 1000.0f).value());

    /// Render all active particles natively onto Nisaba Canvas
    void render(Canvas& canvas, const Rect& bounds) const;

    /// Clear all active particles
    void clear() noexcept;

    [[nodiscard]] size_t active_count() const noexcept;
    [[nodiscard]] size_t activeCount() const noexcept { return active_count(); }
    [[nodiscard]] bool has_active_particles() const noexcept;
    [[nodiscard]] bool hasActiveParticles() const noexcept { return has_active_particles(); }

private:
    void spawn_particle(Point origin);
    float random_float(float min_val, float max_val);
    Color random_color();

    ParticleConfig config_;
    std::vector<Particle> pool_;
    Point emitter_origin_{100.0f, 100.0f};
    bool is_emitting_ = false;
    float emission_accumulator_ = 0.0f;

    mutable std::mt19937 rng_{42};
};

} // namespace nisaba::animation

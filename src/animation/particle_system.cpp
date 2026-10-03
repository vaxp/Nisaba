/// @file particle_system.cpp
/// @brief 2D Particle Physics Simulation implementation for Nisaba.

#include "nisaba/animation/particle_system.hpp"
#include <cmath>
#include <algorithm>

namespace nisaba::animation {

// ════════════════════════════════════════════════════════════════
// ParticlePresets
// ════════════════════════════════════════════════════════════════

ParticleConfig ParticlePresets::confetti() {
    ParticleConfig cfg;
    cfg.max_particles = 120;
    cfg.emission_rate = 0.0f;
    cfg.burst_mode = true;
    cfg.gravity = {0.0f, 350.0f};
    cfg.drag = 0.015f;
    cfg.min_life = 1.2f;
    cfg.max_life = 2.4f;
    cfg.min_speed = 220.0f;
    cfg.max_speed = 520.0f;
    // Upward cone
    cfg.min_angle_rad = -3.14159f * 0.85f;
    cfg.max_angle_rad = -3.14159f * 0.15f;
    cfg.min_size = 4.0f;
    cfg.max_size = 9.0f;
    cfg.shape = ParticleShape::ConfettiRibbon;
    cfg.color_palette = {
        Color::from_rgba8(255, 51, 102, 255),
        Color::from_rgba8(255, 204, 0, 255),
        Color::from_rgba8(0, 255, 204, 255),
        Color::from_rgba8(59, 130, 246, 255),
        Color::from_rgba8(168, 85, 247, 255),
        Color::from_rgba8(255, 255, 255, 255)
    };
    return cfg;
}

ParticleConfig ParticlePresets::neon_sparks() {
    ParticleConfig cfg;
    cfg.max_particles = 80;
    cfg.emission_rate = 0.0f;
    cfg.burst_mode = true;
    cfg.gravity = {0.0f, 200.0f};
    cfg.drag = 0.04f;
    cfg.min_life = 0.4f;
    cfg.max_life = 1.0f;
    cfg.min_speed = 150.0f;
    cfg.max_speed = 400.0f;
    cfg.min_angle_rad = 0.0f;
    cfg.max_angle_rad = 6.2831853f; // Full radial
    cfg.min_size = 2.0f;
    cfg.max_size = 5.0f;
    cfg.shape = ParticleShape::Circle;
    cfg.color_palette = {
        Color::from_rgba8(0, 255, 255, 255),
        Color::from_rgba8(112, 0, 255, 255),
        Color::from_rgba8(255, 0, 85, 255),
        Color::from_rgba8(255, 255, 255, 255)
    };
    return cfg;
}

ParticleConfig ParticlePresets::ambient_dust() {
    ParticleConfig cfg;
    cfg.max_particles = 60;
    cfg.emission_rate = 15.0f;
    cfg.burst_mode = false;
    cfg.gravity = {0.0f, -15.0f}; // Subtle upward float
    cfg.drag = 0.01f;
    cfg.min_life = 2.0f;
    cfg.max_life = 4.0f;
    cfg.min_speed = 10.0f;
    cfg.max_speed = 35.0f;
    cfg.min_angle_rad = 0.0f;
    cfg.max_angle_rad = 6.2831853f;
    cfg.min_size = 1.5f;
    cfg.max_size = 3.5f;
    cfg.shape = ParticleShape::Circle;
    cfg.color_palette = {
        Color::from_rgba8(255, 255, 255, 128),
        Color::from_rgba8(0, 255, 255, 96),
        Color::from_rgba8(255, 215, 0, 80)
    };
    return cfg;
}

ParticleConfig ParticlePresets::snow_fall() {
    ParticleConfig cfg;
    cfg.max_particles = 100;
    cfg.emission_rate = 30.0f;
    cfg.burst_mode = false;
    cfg.gravity = {10.0f, 60.0f};
    cfg.drag = 0.005f;
    cfg.min_life = 3.0f;
    cfg.max_life = 5.0f;
    cfg.min_speed = 20.0f;
    cfg.max_speed = 50.0f;
    cfg.min_angle_rad = 0.5f;
    cfg.max_angle_rad = 1.2f;
    cfg.min_size = 2.0f;
    cfg.max_size = 5.0f;
    cfg.shape = ParticleShape::Circle;
    cfg.color_palette = {
        Color::from_rgba8(255, 255, 255, 240),
        Color::from_rgba8(200, 220, 255, 200),
        Color::from_rgba8(255, 255, 255, 160)
    };
    return cfg;
}

ParticleConfig ParticlePresets::from_preset(ParticlePreset preset) {
    switch (preset) {
        case ParticlePreset::ConfettiBurst: return confetti();
        case ParticlePreset::NeonSparks:    return neon_sparks();
        case ParticlePreset::AmbientDust:   return ambient_dust();
        case ParticlePreset::SnowFall:      return snow_fall();
    }
    return confetti();
}

// ════════════════════════════════════════════════════════════════
// ParticleSystem Implementation
// ════════════════════════════════════════════════════════════════

ParticleSystem::ParticleSystem(const ParticleConfig& config) {
    set_config(config);
}

void ParticleSystem::set_config(const ParticleConfig& config) {
    config_ = config;
    pool_.clear();
    pool_.resize(config_.max_particles);
    is_emitting_ = !config_.burst_mode && (config_.emission_rate > 0.0f);
}

float ParticleSystem::random_float(float min_val, float max_val) {
    std::uniform_real_distribution<float> dist(min_val, max_val);
    return dist(rng_);
}

Color ParticleSystem::random_color() {
    if (config_.color_palette.empty()) return Color::WHITE;
    std::uniform_int_distribution<size_t> dist(0, config_.color_palette.size() - 1);
    return config_.color_palette[dist(rng_)];
}

void ParticleSystem::spawn_particle(Point origin) {
    for (auto& p : pool_) {
        if (!p.alive) {
            p.alive = true;
            p.position = origin;

            float speed = random_float(config_.min_speed, config_.max_speed);
            float angle = random_float(config_.min_angle_rad, config_.max_angle_rad);
            p.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};

            p.max_life = random_float(config_.min_life, config_.max_life);
            p.life = p.max_life;
            p.size = random_float(config_.min_size, config_.max_size);
            p.rotation = random_float(0.0f, 6.2831853f);
            p.rotation_speed = random_float(-5.0f, 5.0f);
            p.color = random_color();
            return;
        }
    }
}

void ParticleSystem::burst(Point origin) {
    for (size_t i = 0; i < config_.max_particles; ++i) {
        spawn_particle(origin);
    }
}

void ParticleSystem::clear() noexcept {
    for (auto& p : pool_) {
        p.alive = false;
    }
}

size_t ParticleSystem::active_count() const noexcept {
    size_t count = 0;
    for (const auto& p : pool_) {
        if (p.alive) ++count;
    }
    return count;
}

bool ParticleSystem::has_active_particles() const noexcept {
    for (const auto& p : pool_) {
        if (p.alive) return true;
    }
    return false;
}

void ParticleSystem::update(float dt_sec, Size /*boundary*/) {
    if (is_emitting_) {
        emission_accumulator_ += config_.emission_rate * dt_sec;
        while (emission_accumulator_ >= 1.0f) {
            spawn_particle(emitter_origin_);
            emission_accumulator_ -= 1.0f;
        }
    }

    for (auto& p : pool_) {
        if (!p.alive) continue;

        p.life -= dt_sec;
        if (p.life <= 0.0f) {
            p.alive = false;
            continue;
        }

        // Apply forces
        p.velocity.x += config_.gravity.x * dt_sec;
        p.velocity.y += config_.gravity.y * dt_sec;

        p.velocity.x *= (1.0f - config_.drag);
        p.velocity.y *= (1.0f - config_.drag);

        p.position.x += p.velocity.x * dt_sec;
        p.position.y += p.velocity.y * dt_sec;

        p.rotation += p.rotation_speed * dt_sec;
    }
}

void ParticleSystem::render(Canvas& canvas, const Rect& /*bounds*/) const {
    for (const auto& p : pool_) {
        if (!p.alive) continue;

        float norm_life = std::clamp(p.life / p.max_life, 0.0f, 1.0f);
        float a = std::clamp(p.color.alpha() * norm_life, 0.0f, 1.0f);
        Color active_color = Color::from_rgba_unchecked(
            p.color.red(),
            p.color.green(),
            p.color.blue(),
            a
        );

        switch (config_.shape) {
            case ParticleShape::Circle: {
                auto path = PathBuilder::from_circle(p.position.x, p.position.y, p.size);
                if (path) canvas.fill_path(*path, Paint(active_color));
                break;
            }
            case ParticleShape::Square: {
                auto r = Rect::from_xywh(p.position.x - p.size, p.position.y - p.size, p.size * 2.0f, p.size * 2.0f);
                if (r) {
                    Path path = PathBuilder::from_rect(*r);
                    canvas.fill_path(path, Paint(active_color));
                }
                break;
            }
            case ParticleShape::ConfettiRibbon: {
                float flip = std::cos(p.rotation);
                float rh = std::max(0.8f, p.size * std::abs(flip));
                auto r = Rect::from_xywh(-p.size, -rh * 0.5f, p.size * 2.0f, rh);
                if (r) {
                    Path path = PathBuilder::from_rect(*r);
                    Transform ts = Transform::from_translate(p.position.x, p.position.y)
                        .pre_concat(Transform::from_rotate(p.rotation * 57.29578f));
                    auto transformed = path.transform(ts);
                    if (transformed) {
                        canvas.fill_path(*transformed, Paint(active_color));
                    }
                }
                break;
            }
            case ParticleShape::Star: {
                auto path = PathBuilder::from_circle(p.position.x, p.position.y, p.size);
                if (path) canvas.fill_path(*path, Paint(active_color));
                break;
            }
        }
    }
}

} // namespace nisaba::animation

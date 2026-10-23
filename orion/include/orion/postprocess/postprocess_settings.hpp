#pragma once

#include "orion/math/vec3.hpp"
#include <cstdint>

namespace orion {


/// @brief Configuration parameters for Ground Truth Ambient Occlusion (GTAO).
struct GtaoSettings {
    bool enabled{true};
    float radius{1.5f};              // World space sample search radius
    float falloff_end{2.5f};         // Maximum occlusion distance
    uint32_t num_slices{3};          // Screen-space radial direction slices (2 to 4)
    uint32_t num_steps{3};           // Radial steps per slice (2 to 4)
    float intensity{1.0f};           // AO power multiplier
    float bilateral_sharpness{16.0f};// Depth edge-preservation sharpness
};

/// @brief Configuration parameters for Cinematic 13-tap / 9-tap Pyramid Bloom.
struct BloomSettings {
    bool enabled{true};
    float threshold{1.0f};           // HDR luminance threshold for bloom extraction
    float soft_knee{0.5f};           // Knee curve factor for smooth highlight roll-off
    float intensity{0.35f};          // Final bloom additive multiplier
    float scatter{0.75f};            // Spatial dispersion across pyramid mips
    uint32_t max_mips{5};            // Number of pyramid downsample passes
};

/// @brief Configuration parameters for Volumetric Atmospheric Height Fog.
struct FogSettings {
    bool enabled{false};
    float density{0.015f};           // Global exponential air density
    float height_falloff{0.15f};     // Vertical decay factor
    float height{0.0f};              // Reference ground elevation
    float start_distance{5.0f};      // Linear distance before fog begins
    Vec3 in_scattering_color{0.7f, 0.8f, 0.95f}; // Sun atmospheric scatter tint
    float sun_intensity{2.0f};       // Sun in-scattering brightness
};

/// @brief Configuration parameters for Photographic Color Grading and Tonemapping.
struct ColorGradingSettings {
    bool enabled{true};
    float exposure{1.0f};            // Photographic exposure scale
    float contrast{1.05f};           // S-curve contrast boost
    float saturation{1.05f};         // Vibrancy / saturation multiplier
    Vec3 white_balance{1.0f, 1.0f, 1.0f}; // Kelvin color temperature tint
    float gamma{2.2f};               // Display gamma exponent
};

/// @brief Master settings container aggregating all post-processing stages.
struct PostProcessPipelineSettings {
    bool enabled{false};
    GtaoSettings gtao{};
    BloomSettings bloom{};
    FogSettings fog{};
    ColorGradingSettings grading{};
};

} // namespace orion

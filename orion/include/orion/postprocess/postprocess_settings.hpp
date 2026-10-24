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

/// @brief Tone mapping curve operator selection  
enum class ToneMappingOperator : uint8_t {
    ACES = 0,       // Academy Color Encoding System filmic fit
    AgX = 1,        // Troy Sobotka AgX Log + Sigmoid (hue-preserving highlight rolloff)
    AgXPunchy = 2,  // AgX with Punchy contrast & chroma look
    PbrNeutral = 3, // Khronos PBR Neutral tone mapper (zero hue shift for e-commerce/CAD)
    Reinhard = 4,   // Classic Reinhard operator
    None = 5        // Linear clamped output
};

/// @brief Configuration parameters for Hierarchical Screen-Space Reflections (SSR).
struct SsrSettings {
    bool enabled{false};
    float max_distance{15.0f};       // Maximum ray-marching distance in view space
    float stride{0.25f};             // Step stride along reflection ray
    float thickness{0.35f};          // Depth intersection thickness tolerance
    float intensity{0.65f};          // Reflection blend factor
    uint32_t max_steps{24};          // Ray-march step count
};

/// @brief Configuration parameters for Physical Circle-of-Confusion Depth of Field (DoF).
struct DofSettings {
    bool enabled{false};
    float focus_distance{5.0f};      // Distance in meters to the sharp focal plane
    float focus_range{3.0f};         // Depth range around focal distance that stays in focus
    float bokeh_radius{4.0f};        // Maximum bokeh disc blur radius in pixels
};

/// @brief Configuration parameters for Temporal & Fast Approximate Anti-Aliasing (TAA / FXAA).
struct AntiAliasingSettings {
    bool enable_fxaa{false};
    bool enable_taa{false};
    float taa_blend_factor{0.1f};    // History blend weight (lower = more stable, higher = less ghosting)
    float fxaa_subpix{0.75f};        // Sub-pixel aliasing removal factor
    float fxaa_edge_threshold{0.125f}; // Minimum local contrast required to apply FXAA
};

/// @brief Configuration parameters for Photographic Color Grading and Tonemapping.
struct ColorGradingSettings {
    bool enabled{true};
    ToneMappingOperator tone_mapping{ToneMappingOperator::ACES};
    float exposure{1.0f};            // Photographic exposure scale
    float contrast{1.05f};           // S-curve contrast boost
    float saturation{1.05f};         // Vibrancy / saturation multiplier
    Vec3 white_balance{1.0f, 1.0f, 1.0f}; // Kelvin color temperature tint
    float gamma{2.2f};               // Display gamma exponent
    float vignette_intensity{0.0f};  // Natural cos^4 lens vignette intensity [0..1]
    float vignette_smoothness{0.5f}; // Lens vignette falloff smoothness
    float chromatic_aberration{0.0f};// Radial R/G/B lens dispersion strength in UV units
    bool enable_dithering{true};     // Triangular PDF screen-space dithering to eliminate 8-bit banding
};

/// @brief Master settings container aggregating all post-processing stages.
struct PostProcessPipelineSettings {
    bool enabled{false};
    GtaoSettings gtao{};
    BloomSettings bloom{};
    FogSettings fog{};
    SsrSettings ssr{};
    DofSettings dof{};
    AntiAliasingSettings anti_aliasing{};
    ColorGradingSettings grading{};
};

} // namespace orion

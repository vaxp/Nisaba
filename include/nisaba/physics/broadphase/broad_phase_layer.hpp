#pragma once

#include <cstdint>

namespace nisaba::physics {

/// @brief Fine-grained object collision classification (0..65535).
using ObjectLayer = uint16_t;

/// @brief Coarse spatial partitioning tree layer.
using BroadPhaseLayer = uint8_t;

namespace Layers {
    inline constexpr ObjectLayer STATIC  = 0;
    inline constexpr ObjectLayer MOVING  = 1;
    inline constexpr ObjectLayer SENSOR  = 2;
    inline constexpr ObjectLayer CUSTOM0 = 3;
    inline constexpr ObjectLayer CUSTOM1 = 4;
}

namespace BroadPhaseLayers {
    inline constexpr BroadPhaseLayer NON_MOVING = 0;
    inline constexpr BroadPhaseLayer MOVING     = 1;
    inline constexpr BroadPhaseLayer SENSOR     = 2;
    inline constexpr uint32_t NUM_LAYERS        = 3;
}

/// @brief Filter interface deciding if an object layer should be tested in a given broadphase layer.
class BroadPhaseLayerInterface {
public:
    virtual ~BroadPhaseLayerInterface() = default;

    [[nodiscard]] virtual BroadPhaseLayer get_broad_phase_layer(ObjectLayer layer) const noexcept {
        if (layer == Layers::STATIC) return BroadPhaseLayers::NON_MOVING;
        if (layer == Layers::SENSOR) return BroadPhaseLayers::SENSOR;
        return BroadPhaseLayers::MOVING;
    }
};

/// @brief Filter interface deciding if two object layers can potentially collide.
class ObjectLayerPairFilter {
public:
    virtual ~ObjectLayerPairFilter() = default;

    [[nodiscard]] virtual bool should_collide(ObjectLayer layer_a, ObjectLayer layer_b) const noexcept {
        // Static never collides with Static
        if (layer_a == Layers::STATIC && layer_b == Layers::STATIC) {
            return false;
        }
        return true;
    }
};

} // namespace nisaba::physics

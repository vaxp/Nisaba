#pragma once

#include "orion/math/vec3.hpp"
#include "orion/camera/camera.hpp"

namespace orion {


/// @brief Interactive 3D Orbit Camera Controller providing smooth spherical rotation,
/// zoom, and screen-space panning around an arbitrary target point.
class alignas(16) OrbitCameraController {
public:
    float min_distance{0.05f};
    float max_distance{5000.0f};
    float min_pitch{-89.0f * (3.14159265358979323846f / 180.0f)};
    float max_pitch{89.0f * (3.14159265358979323846f / 180.0f)};

    Vec3 target{0.0f, 0.0f, 0.0f};
    float distance{5.0f};
    float yaw{0.0f};    // Azimuth angle in radians (around world Y)
    float pitch{0.0f};  // Elevation angle in radians (-pi/2 < pitch < pi/2)

    float orbit_sensitivity{1.0f};
    float zoom_sensitivity{1.0f};
    float pan_sensitivity{1.0f};

    OrbitCameraController() noexcept = default;
    OrbitCameraController(
        const Vec3& in_target,
        float in_distance,
        float in_yaw = 0.0f,
        float in_pitch = 0.0f
    ) noexcept;

    /// @brief Increments spherical orbit angles (yaw and pitch) in radians. Clamps pitch safely.
    void orbit(float delta_yaw, float delta_pitch) noexcept;

    /// @brief Adjusts orbit distance (zoom in/out). Clamps to [min_distance, max_distance].
    void zoom(float delta_distance) noexcept;

    /// @brief Pans the target point along the camera's local right and up axes.
    void pan(float delta_x, float delta_y, const Camera3D& camera) noexcept;

    /// @brief Sets target pivot point in world space.
    void set_target(const Vec3& in_target) noexcept { target = in_target; }

    /// @brief Sets orbit distance with bounds clamping.
    void set_distance(float in_dist) noexcept;

    /// @brief Sets yaw and pitch angles with bounds clamping on pitch.
    void set_angles(float in_yaw, float in_pitch) noexcept;

    /// @brief Calculates current eye position in world space.
    [[nodiscard]] Vec3 compute_eye_position() const noexcept;

    /// @brief Updates camera position and look-at target.
    void update(Camera3D& camera) const noexcept;
};

} // namespace orion

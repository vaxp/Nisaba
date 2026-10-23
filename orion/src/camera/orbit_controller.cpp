#include "orion/camera/orbit_controller.hpp"
#include <algorithm>
#include <cmath>

namespace orion {

OrbitCameraController::OrbitCameraController(
    const Vec3& in_target,
    float in_distance,
    float in_yaw,
    float in_pitch
) noexcept
    : target(in_target),
      yaw(in_yaw) {
    distance = std::clamp(in_distance, min_distance, max_distance);
    pitch = std::clamp(in_pitch, min_pitch, max_pitch);
}

void OrbitCameraController::orbit(float delta_yaw, float delta_pitch) noexcept {
    yaw += delta_yaw * orbit_sensitivity;
    pitch += delta_pitch * orbit_sensitivity;
    pitch = std::clamp(pitch, min_pitch, max_pitch);

    // Keep yaw normalized within [-2pi, 2pi]
    constexpr float two_pi = 2.0f * 3.14159265358979323846f;
    while (yaw > two_pi) yaw -= two_pi;
    while (yaw < -two_pi) yaw += two_pi;
}

void OrbitCameraController::zoom(float delta_distance) noexcept {
    distance = std::clamp(distance + delta_distance * zoom_sensitivity, min_distance, max_distance);
}

void OrbitCameraController::pan(float delta_x, float delta_y, const Camera3D& camera) noexcept {
    const Vec3 right = camera.get_right();
    const Vec3 up = camera.get_camera_up();
    target += right * (delta_x * pan_sensitivity) + up * (delta_y * pan_sensitivity);
}

void OrbitCameraController::set_distance(float in_dist) noexcept {
    distance = std::clamp(in_dist, min_distance, max_distance);
}

void OrbitCameraController::set_angles(float in_yaw, float in_pitch) noexcept {
    yaw = in_yaw;
    pitch = std::clamp(in_pitch, min_pitch, max_pitch);
}

Vec3 OrbitCameraController::compute_eye_position() const noexcept {
    const float cos_pitch = std::cos(pitch);
    const float sin_pitch = std::sin(pitch);
    const float sin_yaw = std::sin(yaw);
    const float cos_yaw = std::cos(yaw);

    const float x = target.x + distance * cos_pitch * sin_yaw;
    const float y = target.y + distance * sin_pitch;
    const float z = target.z + distance * cos_pitch * cos_yaw;

    return Vec3(x, y, z);
}

void OrbitCameraController::update(Camera3D& camera) const noexcept {
    const Vec3 eye = compute_eye_position();
    camera.set_look_at(eye, target, Vec3(0.0f, 1.0f, 0.0f));
}

} // namespace orion

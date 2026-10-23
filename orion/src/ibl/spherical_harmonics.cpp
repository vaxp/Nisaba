#include "orion/ibl/spherical_harmonics.hpp"
#include <algorithm>

namespace orion {

Vec3 SphericalHarmonics9::evaluate(const Vec3& normal) const noexcept {
    const float x = normal.x;
    const float y = normal.y;
    const float z = normal.z;

    // Constants from Ramamoorthi & Hanrahan (2001)
    constexpr float c1 = 0.429043f;
    constexpr float c2 = 0.511664f;
    constexpr float c3 = 0.743125f;
    constexpr float c4 = 0.886227f;
    constexpr float c5 = 0.247708f;

    // 0: L00
    // 1: L1-1 (y), 2: L10 (z), 3: L11 (x)
    // 4: L2-2 (xy), 5: L2-1 (yz), 6: L20 (3z^2-1), 7: L21 (xz), 8: L22 (x^2-y^2)
    const Vec3 l00 = coefficients[0];
    const Vec3 l1_1 = coefficients[1];
    const Vec3 l10 = coefficients[2];
    const Vec3 l11 = coefficients[3];
    const Vec3 l2_2 = coefficients[4];
    const Vec3 l2_1 = coefficients[5];
    const Vec3 l20 = coefficients[6];
    const Vec3 l21 = coefficients[7];
    const Vec3 l22 = coefficients[8];

    Vec3 irradiance =
        l22 * (c1 * (x * x - y * y)) +
        l20 * (c3 * z * z - c5) +
        l00 * c4 +
        (l2_2 * (x * y) + l21 * (x * z) + l2_1 * (y * z)) * (2.0f * c1) +
        (l11 * x + l1_1 * y + l10 * z) * (2.0f * c2);

    return Vec3(
        std::max(0.0f, irradiance.x),
        std::max(0.0f, irradiance.y),
        std::max(0.0f, irradiance.z)
    );
}

void SphericalHarmonics9::scale(float factor) noexcept {
    for (auto& c : coefficients) {
        c = c * factor;
    }
}

void SphericalHarmonics9::add(const SphericalHarmonics9& other) noexcept {
    for (size_t i = 0; i < 9; ++i) {
        coefficients[i] = coefficients[i] + other.coefficients[i];
    }
}

SphericalHarmonics9 SphericalHarmonics9::create_daylight(const Vec3& sun_dir) noexcept {
    SphericalHarmonics9 sh;
    const Vec3 s = sun_dir.normalized();

    // Base atmospheric Rayleigh blue sky irradiance
    sh.coefficients[0] = Vec3(0.55f, 0.70f, 0.95f);  // L00 ambient
    sh.coefficients[1] = Vec3(0.20f, 0.30f, 0.45f);  // L1-1 upward sky gradient
    sh.coefficients[2] = Vec3(0.05f, 0.08f, 0.12f);  // L10
    sh.coefficients[3] = Vec3(0.05f, 0.08f, 0.12f);  // L11
    sh.coefficients[4] = Vec3(0.02f, 0.03f, 0.05f);  // L2-2
    sh.coefficients[5] = Vec3(0.02f, 0.03f, 0.05f);  // L2-1
    sh.coefficients[6] = Vec3(0.08f, 0.12f, 0.18f);  // L20
    sh.coefficients[7] = Vec3(0.02f, 0.03f, 0.05f);  // L21
    sh.coefficients[8] = Vec3(0.03f, 0.04f, 0.06f);  // L22

    // Directional sunlight contribution projected to first order
    const Vec3 sun_color(1.2f, 1.15f, 1.0f);
    sh.coefficients[1] = sh.coefficients[1] + sun_color * (s.y * 0.4f);
    sh.coefficients[2] = sh.coefficients[2] + sun_color * (s.z * 0.4f);
    sh.coefficients[3] = sh.coefficients[3] + sun_color * (s.x * 0.4f);

    return sh;
}

SphericalHarmonics9 SphericalHarmonics9::create_sunset() noexcept {
    SphericalHarmonics9 sh;
    sh.coefficients[0] = Vec3(0.65f, 0.35f, 0.25f);
    sh.coefficients[1] = Vec3(0.35f, 0.18f, 0.28f);
    sh.coefficients[2] = Vec3(0.15f, 0.08f, 0.05f);
    sh.coefficients[3] = Vec3(0.40f, 0.20f, 0.08f);
    sh.coefficients[4] = Vec3(0.12f, 0.06f, 0.04f);
    sh.coefficients[5] = Vec3(0.08f, 0.04f, 0.03f);
    sh.coefficients[6] = Vec3(0.20f, 0.10f, 0.05f);
    sh.coefficients[7] = Vec3(0.10f, 0.05f, 0.02f);
    sh.coefficients[8] = Vec3(0.15f, 0.07f, 0.03f);
    return sh;
}

SphericalHarmonics9 SphericalHarmonics9::create_studio() noexcept {
    SphericalHarmonics9 sh;
    sh.coefficients[0] = Vec3(0.75f, 0.75f, 0.78f);
    sh.coefficients[1] = Vec3(0.15f, 0.15f, 0.18f);
    sh.coefficients[2] = Vec3(0.08f, 0.08f, 0.10f);
    sh.coefficients[3] = Vec3(0.20f, 0.18f, 0.16f);
    sh.coefficients[4] = Vec3(0.04f, 0.04f, 0.04f);
    sh.coefficients[5] = Vec3(0.03f, 0.03f, 0.03f);
    sh.coefficients[6] = Vec3(0.06f, 0.06f, 0.06f);
    sh.coefficients[7] = Vec3(0.04f, 0.04f, 0.04f);
    sh.coefficients[8] = Vec3(0.05f, 0.05f, 0.05f);
    return sh;
}

} // namespace orion

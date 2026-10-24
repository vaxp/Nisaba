#include "orion/ibl/spherical_harmonics.hpp"
#include "orion/math/simd_math.hpp"
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
    constexpr float two_c1 = 2.0f * c1;
    constexpr float two_c2 = 2.0f * c2;

    const float w0 = c4;
    const float w1 = two_c2 * y;
    const float w2 = two_c2 * z;
    const float w3 = two_c2 * x;
    const float w4 = two_c1 * (x * y);
    const float w5 = two_c1 * (y * z);
    const float w6 = c3 * z * z - c5;
    const float w7 = two_c1 * (x * z);
    const float w8 = c1 * (x * x - y * y);

#if defined(NISABA_PHYSICS_SIMD_SSE)
    __m128 acc = _mm_mul_ps(_mm_load_ps(&coefficients[0].x), _mm_set1_ps(w0));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[1].x), _mm_set1_ps(w1)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[2].x), _mm_set1_ps(w2)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[3].x), _mm_set1_ps(w3)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[4].x), _mm_set1_ps(w4)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[5].x), _mm_set1_ps(w5)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[6].x), _mm_set1_ps(w6)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[7].x), _mm_set1_ps(w7)));
    acc = _mm_add_ps(acc, _mm_mul_ps(_mm_load_ps(&coefficients[8].x), _mm_set1_ps(w8)));
    acc = _mm_max_ps(acc, _mm_setzero_ps());
    alignas(16) float out[4];
    _mm_store_ps(out, acc);
    return Vec3(out[0], out[1], out[2]);
#else
    Vec3 irradiance =
        coefficients[0] * w0 +
        coefficients[1] * w1 +
        coefficients[2] * w2 +
        coefficients[3] * w3 +
        coefficients[4] * w4 +
        coefficients[5] * w5 +
        coefficients[6] * w6 +
        coefficients[7] * w7 +
        coefficients[8] * w8;

    return Vec3(
        std::max(0.0f, irradiance.x),
        std::max(0.0f, irradiance.y),
        std::max(0.0f, irradiance.z)
    );
#endif
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

#pragma once

#include "nisaba/compositor/texture.hpp"

namespace nisaba::compositor {

/**
 * @brief Ultra-lightweight 1x1 GPU texture representing solid colors.
 */
class SolidColorTexture : public Texture {
public:
    static std::shared_ptr<SolidColorTexture> create(
        float r,
        float g,
        float b,
        float a = 1.0f);

    SolidColorTexture(GLuint id, float r, float g, float b, float a);
    ~SolidColorTexture() override = default;

    [[nodiscard]] float red() const noexcept { return m_r; }
    [[nodiscard]] float green() const noexcept { return m_g; }
    [[nodiscard]] float blue() const noexcept { return m_b; }
    [[nodiscard]] float alpha() const noexcept { return m_a; }

private:
    float m_r{0.0f};
    float m_g{0.0f};
    float m_b{0.0f};
    float m_a{1.0f};
};

} // namespace nisaba::compositor

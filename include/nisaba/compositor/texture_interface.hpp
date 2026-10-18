#pragma once

#include <cstdint>

namespace nisaba::compositor {

/**
 * @brief Identifies the underlying graphics API backend for rendering objects.
 */
enum class RenderBackendType {
    OpenGL,
    Vulkan,
    SoftwarePixman,
    Custom
};

/**
 * @brief Sovereign abstract texture interface decoupled from any specific graphics API.
 */
class ITexture {
public:
    virtual ~ITexture() = default;

    [[nodiscard]] virtual uint32_t width() const noexcept = 0;
    [[nodiscard]] virtual uint32_t height() const noexcept = 0;
    [[nodiscard]] virtual uint32_t format() const noexcept = 0;
    [[nodiscard]] virtual bool hasAlpha() const noexcept = 0;
    [[nodiscard]] virtual RenderBackendType backendType() const noexcept = 0;

    /**
     * @brief Returns a backend-native handle (e.g. GLuint as uintptr_t, or VkImage pointer).
     */
    [[nodiscard]] virtual void* nativeHandle() const noexcept = 0;
};

} // namespace nisaba::compositor

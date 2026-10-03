#pragma once
/// @file native_popup.hpp
/// @brief Native Compositor-level Popup surface manager.

#include "nisaba/backend_os/types.hpp"
#include "nisaba/backend_os/window.hpp"
#include "nisaba/backend_os/app.hpp"

#include <memory>
#include <functional>

namespace nisaba::backend_os {

/// @brief Options for spawning a native compositor popup surface.
struct PopupOptions {
    Point position{0.0f, 0.0f};  ///< Coordinates relative to parent or screen.
    int width = 200;
    int height = 150;
    bool auto_dismiss = true;     ///< Dismiss when clicking outside the popup.
    std::function<void()> on_close;
};

/// @brief Represents an active native popup surface.
class NativePopup : public std::enable_shared_from_this<NativePopup> {
public:
    using RenderCallback = std::function<void(Window& window, double dt)>;

    /// Show a native popup anchored to parent window.
    static std::shared_ptr<NativePopup> show(
        Window& parent,
        const PopupOptions& options,
        RenderCallback render_cb
    );

    ~NativePopup();

    /// Close and destroy the native popup.
    void close();

    /// Check if popup is currently open.
    [[nodiscard]] bool isOpen() const { return host_ != nullptr; }

    /// Access the underlying native Window.
    [[nodiscard]] Window* window() const;

private:
    NativePopup(const PopupOptions& options);

    PopupOptions options_;
    HostedSurface* host_ = nullptr;
};

} // namespace nisaba::backend_os

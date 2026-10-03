/// @file toplevel.hpp
/// @brief Foreign toplevel window management abstractions for Wayland & X11 (EWMH).

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace nisaba::backend_os {

/// @brief Bitfield flags representing the display and focus states of a foreign window.
enum class WindowState : uint32_t {
    Normal     = 0,
    Maximized  = 1 << 0,
    Minimized  = 1 << 1,
    Activated  = 1 << 2, // Focused window
    Fullscreen = 1 << 3,
};

inline WindowState operator|(WindowState a, WindowState b) {
    return static_cast<WindowState>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline WindowState operator&(WindowState a, WindowState b) {
    return static_cast<WindowState>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline WindowState operator~(WindowState a) {
    return static_cast<WindowState>(~static_cast<uint32_t>(a));
}

inline WindowState& operator|=(WindowState& a, WindowState b) {
    a = a | b;
    return a;
}

inline WindowState& operator&=(WindowState& a, WindowState b) {
    a = a & b;
    return a;
}

inline bool hasWindowState(WindowState mask, WindowState flag) {
    return (static_cast<uint32_t>(mask) & static_cast<uint32_t>(flag)) == static_cast<uint32_t>(flag);
}

/// @brief Abstract interface representing an external open application window.
/// Managed via Wayland wlr-foreign-toplevel or X11 EWMH hints.
class ToplevelWindow : public std::enable_shared_from_this<ToplevelWindow> {
public:
    virtual ~ToplevelWindow() = default;

    /// @brief Unique numeric identifier (Wayland pointer handle or X11 Window ID).
    [[nodiscard]] virtual uint64_t id() const = 0;

    /// @brief Current window title (e.g. document name or web page title).
    [[nodiscard]] virtual std::string title() const = 0;

    /// @brief Application identifier or class name (e.g. "org.mozilla.firefox" or "kitty").
    [[nodiscard]] virtual std::string appId() const = 0;

    /// @brief Current window state bitmask.
    [[nodiscard]] virtual WindowState state() const = 0;

    /// @brief True if this window currently has keyboard and input focus.
    [[nodiscard]] virtual bool isActivated() const {
        return hasWindowState(state(), WindowState::Activated);
    }

    /// @brief True if the window is currently maximized.
    [[nodiscard]] virtual bool isMaximized() const {
        return hasWindowState(state(), WindowState::Maximized);
    }

    /// @brief True if the window is minimized / hidden to taskbar.
    [[nodiscard]] virtual bool isMinimized() const {
        return hasWindowState(state(), WindowState::Minimized);
    }

    /// @brief True if the window occupies the full monitor.
    [[nodiscard]] virtual bool isFullscreen() const {
        return hasWindowState(state(), WindowState::Fullscreen);
    }

    // ── Window Actions ──────────────────────────────────────────

    /// @brief Bring this window to the foreground and give it focus.
    virtual void activate() = 0;

    /// @brief Request to minimize or restore this window.
    virtual void setMinimized(bool min) = 0;

    /// @brief Request to maximize or unmaximize this window.
    virtual void setMaximized(bool max) = 0;

    /// @brief Request to make this window fullscreen or return to normal.
    virtual void setFullscreen(bool full) = 0;

    /// @brief Request the application to close this window.
    virtual void close() = 0;
};

/// @brief In-memory mock/local implementation of ToplevelWindow for unit tests and dummy backends.
class MemoryToplevelWindow : public ToplevelWindow {
public:
    MemoryToplevelWindow(uint64_t id, std::string title, std::string app_id, WindowState state = WindowState::Normal)
        : m_id(id), m_title(std::move(title)), m_app_id(std::move(app_id)), m_state(state) {}

    [[nodiscard]] uint64_t id() const override { return m_id; }
    [[nodiscard]] std::string title() const override { return m_title; }
    [[nodiscard]] std::string appId() const override { return m_app_id; }
    [[nodiscard]] WindowState state() const override { return m_state; }

    void setTitle(std::string title) { m_title = std::move(title); }
    void setAppId(std::string app_id) { m_app_id = std::move(app_id); }
    void setState(WindowState state) { m_state = state; }

    void activate() override {
        m_state |= WindowState::Activated;
        m_state = static_cast<WindowState>(static_cast<uint32_t>(m_state) & ~static_cast<uint32_t>(WindowState::Minimized));
    }

    void setMinimized(bool min) override {
        if (min) {
            m_state |= WindowState::Minimized;
            m_state = static_cast<WindowState>(static_cast<uint32_t>(m_state) & ~static_cast<uint32_t>(WindowState::Activated));
        } else {
            m_state = static_cast<WindowState>(static_cast<uint32_t>(m_state) & ~static_cast<uint32_t>(WindowState::Minimized));
        }
    }

    void setMaximized(bool max) override {
        if (max) {
            m_state |= WindowState::Maximized;
        } else {
            m_state = static_cast<WindowState>(static_cast<uint32_t>(m_state) & ~static_cast<uint32_t>(WindowState::Maximized));
        }
    }

    void setFullscreen(bool full) override {
        if (full) {
            m_state |= WindowState::Fullscreen;
        } else {
            m_state = static_cast<WindowState>(static_cast<uint32_t>(m_state) & ~static_cast<uint32_t>(WindowState::Fullscreen));
        }
    }

    void close() override {
        m_closed = true;
    }

    [[nodiscard]] bool isClosed() const { return m_closed; }

private:
    uint64_t m_id;
    std::string m_title;
    std::string m_app_id;
    WindowState m_state;
    bool m_closed = false;
};

} // namespace nisaba::backend_os

#pragma once
/// @file win32_platform.hpp
/// @brief Native Windows (Win32) Platform backend.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/output.hpp"
#include <unordered_set>
#include <memory>
#include <vector>
#include <string>

namespace nisaba::backend_os {
class Window;
}

namespace nisaba::backend_os::win32 {

class Win32PlatformBackend {
public:
    explicit Win32PlatformBackend(Platform* owner);
    ~Win32PlatformBackend();

    bool init();
    void shutdown();
    bool pollEvents();

    void registerWindow(Window* w);
    void unregisterWindow(Window* w);
    [[nodiscard]] const std::unordered_set<Window*>& windows() const { return windows_; }

    // ── Clipboard Subsystem ──────────────────────────────────────
    void setClipboardData(const ClipboardData& data, ClipboardType type = ClipboardType::Clipboard);
    void setClipboardText(std::string_view text, ClipboardType type = ClipboardType::Clipboard);
    [[nodiscard]] std::string getClipboardText(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] ClipboardData getClipboardData(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<uint8_t> getClipboardDataForMime(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<std::string> getClipboardFormats(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] bool hasClipboardFormat(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;

    // ── Drag and Drop Subsystem ──────────────────────────────────
    bool startDrag(const DragData& data, DragAction actions);

    // ── Cursor ───────────────────────────────────────────────────
    void setCursor(SystemCursor cursor);

    // ── Output / Monitor Subsystem ───────────────────────────────
    [[nodiscard]] std::vector<std::shared_ptr<Output>> getOutputs() const;
    [[nodiscard]] std::shared_ptr<Output> getOutputByName(std::string_view name) const;
    [[nodiscard]] std::shared_ptr<Output> getPrimaryOutput() const;
    void updateOutputs();

    // ── Foreign Toplevel Subsystem ────────────────────────────────
    [[nodiscard]] std::vector<std::shared_ptr<ToplevelWindow>> getToplevels() const;
    [[nodiscard]] std::shared_ptr<ToplevelWindow> getActiveToplevel() const;

    [[nodiscard]] Platform* getOwner() const { return owner_; }

private:
    Platform* owner_ = nullptr;
    std::unordered_set<Window*> windows_;

    ClipboardData local_clipboard_;
    SystemCursor current_cursor_ = SystemCursor::Arrow;

    class Win32Output;
    std::vector<std::shared_ptr<Output>> outputs_;
};

} // namespace nisaba::backend_os::win32

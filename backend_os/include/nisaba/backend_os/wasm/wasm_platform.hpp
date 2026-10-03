#pragma once
/// @file wasm_platform.hpp
/// @brief WebAssembly (Wasm / HTML5 / Emscripten) Platform backend for Nisaba.

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/output.hpp"
#include <unordered_set>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace nisaba::backend_os {
class Window;
}

namespace nisaba::backend_os::wasm {

class WasmOutput;

class WasmPlatformBackend {
public:
    explicit WasmPlatformBackend(Platform* owner);
    ~WasmPlatformBackend();

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

    // ── Output / Viewport Subsystem ──────────────────────────────
    [[nodiscard]] std::vector<std::shared_ptr<Output>> getOutputs() const;
    [[nodiscard]] std::shared_ptr<Output> getOutputByName(std::string_view name) const;
    [[nodiscard]] std::shared_ptr<Output> getPrimaryOutput() const;
    void updateOutputs();
    void handleResize();

    // ── Foreign Toplevel Subsystem (Browser tabs / popups) ───────
    [[nodiscard]] std::vector<std::shared_ptr<ToplevelWindow>> getToplevels() const;
    [[nodiscard]] std::shared_ptr<ToplevelWindow> getActiveToplevel() const;

    [[nodiscard]] Platform* getOwner() const { return owner_; }

private:
    Platform* owner_ = nullptr;
    std::unordered_set<Window*> windows_;

    ClipboardData local_clipboard_;
    SystemCursor current_cursor_ = SystemCursor::Arrow;

    std::vector<std::shared_ptr<Output>> outputs_;
};

} // namespace nisaba::backend_os::wasm

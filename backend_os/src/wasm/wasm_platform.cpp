/// @file wasm_platform.cpp
/// @brief WebAssembly (Wasm / HTML5 / Emscripten) Platform backend implementation.

#include "nisaba/backend_os/wasm/wasm_platform.hpp"
#include "nisaba/backend_os/wasm/wasm_window.hpp"
#include "nisaba/backend_os/window.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

namespace nisaba::backend_os::wasm {

// ── Wasm Output (Browser Viewport representation) ───────────────────

class WasmOutput : public Output {
public:
    uint32_t id_ = 1;
    std::string name_ = "Web Viewport";
    std::string make_ = "Browser";
    std::string model_ = "HTML5 Canvas";
    std::string description_ = "WebAssembly Display Surface";
    Rect geometry_{0.0f, 0.0f, 1920.0f, 1080.0f};
    Rect logical_geometry_{0.0f, 0.0f, 1920.0f, 1080.0f};
    int32_t physical_w_mm_ = 500;
    int32_t physical_h_mm_ = 300;
    int32_t scale_factor_ = 1;
    double fractional_scale_ = 1.0;
    std::vector<OutputMode> modes_;
    OutputMode current_mode_{1920, 1080, 60000, true};
    bool is_primary_ = true;

    [[nodiscard]] uint32_t id() const noexcept override { return id_; }
    [[nodiscard]] const std::string& name() const noexcept override { return name_; }
    [[nodiscard]] const std::string& make() const noexcept override { return make_; }
    [[nodiscard]] const std::string& model() const noexcept override { return model_; }
    [[nodiscard]] const std::string& description() const noexcept override { return description_; }
    [[nodiscard]] Rect geometry() const noexcept override { return geometry_; }
    [[nodiscard]] Rect logicalGeometry() const noexcept override { return logical_geometry_; }
    [[nodiscard]] int32_t physicalWidthMm() const noexcept override { return physical_w_mm_; }
    [[nodiscard]] int32_t physicalHeightMm() const noexcept override { return physical_h_mm_; }
    [[nodiscard]] int32_t scaleFactor() const noexcept override { return scale_factor_; }
    [[nodiscard]] double fractionalScale() const noexcept override { return fractional_scale_; }
    [[nodiscard]] OutputTransform transform() const noexcept override { return OutputTransform::Normal; }
    [[nodiscard]] OutputSubpixel subpixel() const noexcept override { return OutputSubpixel::HorizontalRgb; }
    [[nodiscard]] const std::vector<OutputMode>& modes() const noexcept override { return modes_; }
    [[nodiscard]] const OutputMode& currentMode() const noexcept override { return current_mode_; }
    [[nodiscard]] bool isPrimary() const noexcept override { return is_primary_; }
    [[nodiscard]] void* nativeHandle() const noexcept override { return nullptr; }
};

// ── Static DOM Event Dispatchers ────────────────────────────────────

#if defined(__EMSCRIPTEN__)

static EM_BOOL onMouseMove(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    float x = static_cast<float>(e->targetX);
    float y = static_cast<float>(e->targetY);

    backend->getOwner()->onMouseMove().emit(x, y);
    return EM_TRUE;
}

static EM_BOOL onMouseDown(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    float x = static_cast<float>(e->targetX);
    float y = static_cast<float>(e->targetY);
    int button = e->button + 1; // 0->1 (Left), 1->2 (Middle), 2->3 (Right)

    backend->getOwner()->onMouseDown().emit(x, y, button);
    return EM_TRUE;
}

static EM_BOOL onMouseUp(int eventType, const EmscriptenMouseEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    float x = static_cast<float>(e->targetX);
    float y = static_cast<float>(e->targetY);
    int button = e->button + 1;

    backend->getOwner()->onMouseUp().emit(x, y, button);
    return EM_TRUE;
}

static EM_BOOL onWheel(int eventType, const EmscriptenWheelEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    float dx = static_cast<float>(e->deltaX);
    float dy = static_cast<float>(e->deltaY);

    // Normalize wheel delta (DOM deltaMode: 0 = pixels, 1 = lines, 2 = pages)
    if (e->deltaMode == 1) {
        dx *= 20.0f;
        dy *= 20.0f;
    }

    backend->getOwner()->onScroll().emit(dx, dy);
    return EM_TRUE;
}

static EM_BOOL onKeyDown(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    int modifiers = 0;
    if (e->shiftKey) modifiers |= static_cast<int>(KeyMod::Shift);
    if (e->ctrlKey)  modifiers |= static_cast<int>(KeyMod::Ctrl);
    if (e->altKey)   modifiers |= static_cast<int>(KeyMod::Alt);
    if (e->metaKey)  modifiers |= static_cast<int>(KeyMod::Super);

    int keycode = static_cast<int>(e->which ? e->which : e->keyCode);
    backend->getOwner()->onKeyDown().emit(keycode, modifiers);

    // Text input emission if printable
    if (e->key[0] != '\0' && e->key[1] == '\0' && !e->ctrlKey && !e->metaKey) {
        backend->getOwner()->onTextInput().emit(std::string_view(e->key, 1));
    }

    // Prevent default scrolling on arrow keys, space bar, backspace
    if (keycode == 32 || (keycode >= 37 && keycode <= 40) || keycode == 8) {
        return EM_TRUE;
    }
    return EM_FALSE;
}

static EM_BOOL onKeyUp(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    int modifiers = 0;
    if (e->shiftKey) modifiers |= static_cast<int>(KeyMod::Shift);
    if (e->ctrlKey)  modifiers |= static_cast<int>(KeyMod::Ctrl);
    if (e->altKey)   modifiers |= static_cast<int>(KeyMod::Alt);
    if (e->metaKey)  modifiers |= static_cast<int>(KeyMod::Super);

    int keycode = static_cast<int>(e->which ? e->which : e->keyCode);
    backend->getOwner()->onKeyUp().emit(keycode, modifiers);
    return EM_TRUE;
}

static EM_BOOL onTouchStart(int eventType, const EmscriptenTouchEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner() || e->numTouches <= 0) return EM_FALSE;

    const auto& t = e->touches[0];
    backend->getOwner()->onMouseDown().emit(static_cast<float>(t.targetX), static_cast<float>(t.targetY), 1);
    return EM_TRUE;
}

static EM_BOOL onTouchMove(int eventType, const EmscriptenTouchEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner() || e->numTouches <= 0) return EM_FALSE;

    const auto& t = e->touches[0];
    backend->getOwner()->onMouseMove().emit(static_cast<float>(t.targetX), static_cast<float>(t.targetY));
    return EM_TRUE;
}

static EM_BOOL onTouchEnd(int eventType, const EmscriptenTouchEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend || !backend->getOwner()) return EM_FALSE;

    // Report last known position or 0
    float x = 0.0f, y = 0.0f;
    if (e->numTouches > 0) {
        x = static_cast<float>(e->touches[0].targetX);
        y = static_cast<float>(e->touches[0].targetY);
    }
    backend->getOwner()->onMouseUp().emit(x, y, 1);
    return EM_TRUE;
}

static EM_BOOL onWindowResize(int eventType, const EmscriptenUiEvent* e, void* userData) {
    auto* backend = static_cast<WasmPlatformBackend*>(userData);
    if (!backend) return EM_FALSE;
    backend->handleResize();
    return EM_TRUE;
}

#endif // __EMSCRIPTEN__

// ── WasmPlatformBackend Implementation ──────────────────────────────

WasmPlatformBackend::WasmPlatformBackend(Platform* owner)
    : owner_(owner) {}

WasmPlatformBackend::~WasmPlatformBackend() {
    shutdown();
}

bool WasmPlatformBackend::init() {
    updateOutputs();

#if defined(__EMSCRIPTEN__)
    const char* target = "#canvas";

    // Bind Mouse & Pointer Events
    emscripten_set_mousemove_callback(target, this, EM_TRUE, onMouseMove);
    emscripten_set_mousedown_callback(target, this, EM_TRUE, onMouseDown);
    emscripten_set_mouseup_callback(target, this, EM_TRUE, onMouseUp);
    emscripten_set_wheel_callback(target, this, EM_TRUE, onWheel);

    // Bind Keyboard Events on Document
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, onKeyDown);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, this, EM_TRUE, onKeyUp);

    // Bind Touch Events on Canvas
    emscripten_set_touchstart_callback(target, this, EM_TRUE, onTouchStart);
    emscripten_set_touchmove_callback(target, this, EM_TRUE, onTouchMove);
    emscripten_set_touchend_callback(target, this, EM_TRUE, onTouchEnd);
    emscripten_set_touchcancel_callback(target, this, EM_TRUE, onTouchEnd);

    // Bind Window Resize
    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, EM_TRUE, onWindowResize);
#endif

    return true;
}

void WasmPlatformBackend::shutdown() {
    windows_.clear();
}

bool WasmPlatformBackend::pollEvents() {
    // In WebAssembly / Emscripten, browser DOM events are dispatched asynchronously
    // directly via event callbacks before each frame.
    return true;
}

void WasmPlatformBackend::registerWindow(Window* w) {
    if (w) windows_.insert(w);
}

void WasmPlatformBackend::unregisterWindow(Window* w) {
    if (w) windows_.erase(w);
}

// ── Clipboard Subsystem ─────────────────────────────────────────────

void WasmPlatformBackend::setClipboardData(const ClipboardData& data, ClipboardType type) {
    local_clipboard_ = data;
    setClipboardText(data.getText(), type);
}

void WasmPlatformBackend::setClipboardText(std::string_view text, ClipboardType) {
    local_clipboard_.setText(text);
#if defined(__EMSCRIPTEN__)
    std::string text_str(text);
    EM_ASM({
        var str = UTF8ToString($0);
        if (navigator.clipboard && navigator.clipboard.writeText) {
            navigator.clipboard.writeText(str).catch(function(err) {});
        }
    }, text_str.c_str());
#endif
}

std::string WasmPlatformBackend::getClipboardText(ClipboardType) const {
    return local_clipboard_.getText();
}

ClipboardData WasmPlatformBackend::getClipboardData(ClipboardType) const {
    return local_clipboard_;
}

std::vector<uint8_t> WasmPlatformBackend::getClipboardDataForMime(std::string_view mime_type, ClipboardType) const {
    if (mime_type == "text/plain") {
        auto t = local_clipboard_.getText();
        return std::vector<uint8_t>(t.begin(), t.end());
    }
    return {};
}

std::vector<std::string> WasmPlatformBackend::getClipboardFormats(ClipboardType) const {
    return {"text/plain"};
}

bool WasmPlatformBackend::hasClipboardFormat(std::string_view mime_type, ClipboardType) const {
    return mime_type == "text/plain";
}

bool WasmPlatformBackend::startDrag(const DragData&, DragAction) {
    return false;
}

// ── Cursor ──────────────────────────────────────────────────────────

void WasmPlatformBackend::setCursor(SystemCursor cursor) {
    current_cursor_ = cursor;
#if defined(__EMSCRIPTEN__)
    const char* css_cursor = "default";
    switch (cursor) {
        case SystemCursor::Default:           css_cursor = "default"; break;
        case SystemCursor::Arrow:             css_cursor = "default"; break;
        case SystemCursor::Pointer:           css_cursor = "pointer"; break;
        case SystemCursor::Text:              css_cursor = "text"; break;
        case SystemCursor::Crosshair:         css_cursor = "crosshair"; break;
        case SystemCursor::Move:              css_cursor = "move"; break;
        case SystemCursor::NotAllowed:        css_cursor = "not-allowed"; break;
        case SystemCursor::ResizeHorizontal:  css_cursor = "ew-resize"; break;
        case SystemCursor::ResizeVertical:    css_cursor = "ns-resize"; break;
        case SystemCursor::ResizeTopLeft:     css_cursor = "nwse-resize"; break;
        case SystemCursor::ResizeTopRight:    css_cursor = "nesw-resize"; break;
        case SystemCursor::ResizeBottomLeft:  css_cursor = "nesw-resize"; break;
        case SystemCursor::ResizeBottomRight: css_cursor = "nwse-resize"; break;
        case SystemCursor::Wait:              css_cursor = "wait"; break;
    }

    EM_ASM({
        var c = UTF8ToString($0);
        var canvas = document.querySelector('#canvas') || document.querySelector('canvas');
        if (canvas) canvas.style.cursor = c;
    }, css_cursor);
#endif
}

// ── Output / Monitor Subsystem ──────────────────────────────────────

void WasmPlatformBackend::updateOutputs() {
    outputs_.clear();
    auto out = std::make_shared<WasmOutput>();

#if defined(__EMSCRIPTEN__)
    double w = 0.0, h = 0.0;
    emscripten_get_element_css_size("#canvas", &w, &h);
    if (w <= 10.0 || h <= 10.0) {
        w = EM_ASM_DOUBLE({ return window.innerWidth; });
        h = EM_ASM_DOUBLE({ return window.innerHeight; });
        if (w <= 0.0 || h <= 0.0) {
            w = 1280.0;
            h = 800.0;
        }
    }
    double dpr = emscripten_get_device_pixel_ratio();
    if (dpr <= 0.0) dpr = 1.0;

    out->geometry_ = Rect{0.0f, 0.0f, static_cast<float>(w * dpr), static_cast<float>(h * dpr)};
    out->logical_geometry_ = Rect{0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)};
    out->scale_factor_ = static_cast<int32_t>(dpr);
    out->fractional_scale_ = dpr;
    out->current_mode_ = OutputMode{static_cast<int32_t>(w * dpr), static_cast<int32_t>(h * dpr), 60000, true};
#endif

    outputs_.push_back(out);
}

void WasmPlatformBackend::handleResize() {
    updateOutputs();

#if defined(__EMSCRIPTEN__)
    double css_w = 0.0, css_h = 0.0;
    emscripten_get_element_css_size("#canvas", &css_w, &css_h);
    if (css_w <= 10.0 || css_h <= 10.0) {
        css_w = EM_ASM_DOUBLE({ return window.innerWidth; });
        css_h = EM_ASM_DOUBLE({ return window.innerHeight; });
    }
    double dpr = emscripten_get_device_pixel_ratio();
    if (dpr <= 0.0) dpr = 1.0;

    for (Window* win : windows_) {
        if (!win) continue;
        auto* wasm_win = static_cast<WasmWindow*>(win->getBackendWindow());
        if (wasm_win) {
            wasm_win->handleCanvasResize(css_w, css_h, dpr);
        }
    }
#endif
}

std::vector<std::shared_ptr<Output>> WasmPlatformBackend::getOutputs() const {
    return outputs_;
}

std::shared_ptr<Output> WasmPlatformBackend::getOutputByName(std::string_view name) const {
    for (const auto& out : outputs_) {
        if (out && out->name() == name) return out;
    }
    return getPrimaryOutput();
}

std::shared_ptr<Output> WasmPlatformBackend::getPrimaryOutput() const {
    if (!outputs_.empty()) return outputs_.front();
    return nullptr;
}

std::vector<std::shared_ptr<ToplevelWindow>> WasmPlatformBackend::getToplevels() const {
    return {};
}

std::shared_ptr<ToplevelWindow> WasmPlatformBackend::getActiveToplevel() const {
    return nullptr;
}

} // namespace nisaba::backend_os::wasm

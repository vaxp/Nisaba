/// @file win32_platform.cpp
/// @brief Native Windows (Win32) Platform backend implementation.

#include "nisaba/backend_os/windows/win32_platform.hpp"
#include "nisaba/backend_os/windows/win32_window.hpp"
#include "nisaba/backend_os/window.hpp"

#include <shellscalingapi.h>
#include <iostream>
#include <vector>
#include <string>

namespace nisaba::backend_os::win32 {



// ── Win32 Output (Monitor representation) ──────────────────────────

class Win32Output : public Output {
public:
    uint32_t id_ = 0;
    std::string name_;
    std::string make_ = "Generic";
    std::string model_ = "Monitor";
    std::string description_ = "Generic Display";
    Rect geometry_;
    Rect logical_geometry_;
    int32_t physical_w_mm_ = 500;
    int32_t physical_h_mm_ = 300;
    int32_t scale_factor_ = 1;
    double fractional_scale_ = 1.0;
    std::vector<OutputMode> modes_;
    OutputMode current_mode_;
    bool is_primary_ = false;
    HMONITOR hmonitor_ = nullptr;

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
    [[nodiscard]] void* nativeHandle() const noexcept override { return (void*)hmonitor_; }
};

// ── Win32PlatformBackend Implementation ────────────────────────────

Win32PlatformBackend::Win32PlatformBackend(Platform* owner)
    : owner_(owner) {}

Win32PlatformBackend::~Win32PlatformBackend() {
    shutdown();
}

static void ensureIcuLoaded() {
    static bool s_loaded = false;
    if (s_loaded) return;

    typedef void (*FnUdataSetCommonData)(const void*, int*);
    typedef void (*FnUdataSetFileAccess)(int, int*);

    HMODULE hIcu = GetModuleHandleW(L"icuuc.dll");
    if (!hIcu) hIcu = GetModuleHandleW(nullptr);
    if (!hIcu) return;

    auto pfnSetCommonData = reinterpret_cast<FnUdataSetCommonData>(GetProcAddress(hIcu, "udata_setCommonData"));
    auto pfnSetFileAccess = reinterpret_cast<FnUdataSetFileAccess>(GetProcAddress(hIcu, "udata_setFileAccess"));
    if (!pfnSetCommonData) return;

    std::vector<std::wstring> candidates;

    // 1. Next to current executable and parent directory trees
    wchar_t exe_path[MAX_PATH] = {0};
    if (GetModuleFileNameW(nullptr, exe_path, MAX_PATH)) {
        wchar_t* last_slash = wcsrchr(exe_path, L'\\');
        if (last_slash) {
            *last_slash = L'\0';
            candidates.push_back(std::wstring(exe_path) + L"\\icudtl.dat");
            candidates.push_back(std::wstring(exe_path) + L"\\..\\icudtl.dat");
            candidates.push_back(std::wstring(exe_path) + L"\\..\\..\\icudtl.dat");
        }
    }
    // 2. Relative to working directory
    candidates.push_back(L"icudtl.dat");

    for (const auto& path : candidates) {
        HANDLE file = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (file != INVALID_HANDLE_VALUE) {
            HANDLE mapping = CreateFileMappingW(file, nullptr, PAGE_READONLY, 0, 0, nullptr);
            CloseHandle(file);
            if (mapping) {
                void* addr = MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, 0);
                if (addr) {
                    int status = 0;
                    pfnSetCommonData(addr, &status);
                    if (status == 0) {
                        if (pfnSetFileAccess) {
                            int access_status = 0;
                            pfnSetFileAccess(1 /* UDATA_ONLY_PACKAGES */, &access_status);
                        }
                        s_loaded = true;
                        std::cout << "[Nisaba Platform] Preloaded ICU data successfully\n";
                        return;
                    }
                }
            }
        }
    }
}

bool Win32PlatformBackend::init() {
    ensureIcuLoaded();

    // 1. Enable Per-Monitor DPI Awareness V2 if available
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        typedef BOOL (WINAPI *SetProcessDpiAwarenessContextProc)(DPI_AWARENESS_CONTEXT);
        auto setDpiContext = (SetProcessDpiAwarenessContextProc)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        if (setDpiContext) {
            setDpiContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        } else {
            SetProcessDPIAware();
        }
    }

    std::cout << "[Nisaba Platform] Windows Win32 backend initialized\n";
    updateOutputs();
    return true;
}

void Win32PlatformBackend::shutdown() {
    windows_.clear();
}

bool Win32PlatformBackend::pollEvents() {
    MSG msg;
    while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            if (owner_) owner_->onQuit().emit();
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return true;
}

void Win32PlatformBackend::registerWindow(Window* w) {
    if (w) windows_.insert(w);
}

void Win32PlatformBackend::unregisterWindow(Window* w) {
    if (w) windows_.erase(w);
}

// ── Clipboard ──────────────────────────────────────────────────────

void Win32PlatformBackend::setClipboardText(std::string_view text, ClipboardType type) {
    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();

    std::wstring wide = utf8ToWide(text);
    size_t bytes = (wide.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (hMem) {
        void* ptr = GlobalLock(hMem);
        if (ptr) {
            memcpy(ptr, wide.c_str(), bytes);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
    }
    CloseClipboard();

    local_clipboard_.setText(text);
    if (owner_) owner_->onClipboardChanged().emit(type);
}

std::string Win32PlatformBackend::getClipboardText(ClipboardType /*type*/) const {
    if (!OpenClipboard(nullptr)) return local_clipboard_.getText();

    std::string result;
    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        auto* wstr = static_cast<const wchar_t*>(GlobalLock(hData));
        if (wstr) {
            result = wideToUtf8(wstr);
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();
    return !result.empty() ? result : local_clipboard_.getText();
}

void Win32PlatformBackend::setClipboardData(const ClipboardData& data, ClipboardType type) {
    setClipboardText(data.getText(), type);
    local_clipboard_ = data;
}

ClipboardData Win32PlatformBackend::getClipboardData(ClipboardType type) const {
    ClipboardData cd;
    cd.setText(getClipboardText(type));
    return cd;
}

std::vector<uint8_t> Win32PlatformBackend::getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const {
    if (mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain) {
        auto str = getClipboardText(type);
        return {str.begin(), str.end()};
    }
    return {};
}

std::vector<std::string> Win32PlatformBackend::getClipboardFormats(ClipboardType /*type*/) const {
    return { std::string(mime::TextPlainUtf8), std::string(mime::TextPlain) };
}

bool Win32PlatformBackend::hasClipboardFormat(std::string_view mime_type, ClipboardType /*type*/) const {
    return mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain;
}

// ── Drag and Drop ──────────────────────────────────────────────────

bool Win32PlatformBackend::startDrag(const DragData& /*data*/, DragAction /*actions*/) {
    return false;
}

// ── Cursor ─────────────────────────────────────────────────────────

void Win32PlatformBackend::setCursor(SystemCursor cursor) {
    current_cursor_ = cursor;
    LPCSTR id = IDC_ARROW;
    switch (cursor) {
        case SystemCursor::Pointer:          id = IDC_HAND;      break;
        case SystemCursor::Text:             id = IDC_IBEAM;     break;
        case SystemCursor::Crosshair:        id = IDC_CROSS;     break;
        case SystemCursor::Move:             id = IDC_SIZEALL;   break;
        case SystemCursor::NotAllowed:       id = IDC_NO;        break;
        case SystemCursor::ResizeHorizontal: id = IDC_SIZEWE;    break;
        case SystemCursor::ResizeVertical:   id = IDC_SIZENS;    break;
        case SystemCursor::ResizeTopLeft:
        case SystemCursor::ResizeBottomRight:id = IDC_SIZENWSE;  break;
        case SystemCursor::ResizeTopRight:
        case SystemCursor::ResizeBottomLeft: id = IDC_SIZENESW;  break;
        case SystemCursor::Wait:             id = IDC_WAIT;      break;
        default:                             id = IDC_ARROW;     break;
    }
    SetCursor(LoadCursorA(nullptr, id));
}

// ── Output / Monitors ──────────────────────────────────────────────

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC, LPRECT, LPARAM dwData) {
    auto* outputs = reinterpret_cast<std::vector<std::shared_ptr<Output>>*>(dwData);

    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(MONITORINFOEXW);
    if (GetMonitorInfoW(hMon, &mi)) {
        auto out = std::make_shared<Win32Output>();
        out->id_ = static_cast<uint32_t>(outputs->size() + 1);
        out->hmonitor_ = hMon;
        out->name_ = wideToUtf8(mi.szDevice);
        out->description_ = out->name_;
        out->is_primary_ = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;

        int w = mi.rcMonitor.right - mi.rcMonitor.left;
        int h = mi.rcMonitor.bottom - mi.rcMonitor.top;
        out->geometry_ = Rect{ (float)mi.rcMonitor.left, (float)mi.rcMonitor.top, (float)w, (float)h };
        out->logical_geometry_ = out->geometry_;

        // Determine DPI
        UINT dpiX = 96, dpiY = 96;
        HMODULE shcore = LoadLibraryW(L"shcore.dll");
        if (shcore) {
            typedef HRESULT (WINAPI *GetDpiForMonitorProc)(HMONITOR, int, UINT*, UINT*);
            auto getDpi = (GetDpiForMonitorProc)GetProcAddress(shcore, "GetDpiForMonitor");
            if (getDpi) {
                getDpi(hMon, 0 /*MDT_EFFECTIVE_DPI*/, &dpiX, &dpiY);
            }
            FreeLibrary(shcore);
        }
        out->fractional_scale_ = dpiX / 96.0;
        out->scale_factor_ = (int)std::round(out->fractional_scale_);

        OutputMode mode;
        mode.width = w;
        mode.height = h;
        mode.refresh_rate_mHz = 60000;
        mode.is_current = true;
        mode.is_preferred = true;
        out->modes_.push_back(mode);
        out->current_mode_ = mode;

        outputs->push_back(out);
    }
    return TRUE;
}

void Win32PlatformBackend::updateOutputs() {
    outputs_.clear();
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&outputs_));
}

std::vector<std::shared_ptr<Output>> Win32PlatformBackend::getOutputs() const {
    return outputs_;
}

std::shared_ptr<Output> Win32PlatformBackend::getOutputByName(std::string_view name) const {
    for (const auto& out : outputs_) {
        if (out && out->name() == name) return out;
    }
    return nullptr;
}

std::shared_ptr<Output> Win32PlatformBackend::getPrimaryOutput() const {
    for (const auto& out : outputs_) {
        if (out && out->isPrimary()) return out;
    }
    return outputs_.empty() ? nullptr : outputs_.front();
}

// ── Foreign Toplevels ──────────────────────────────────────────────

std::vector<std::shared_ptr<ToplevelWindow>> Win32PlatformBackend::getToplevels() const {
    return {};
}

std::shared_ptr<ToplevelWindow> Win32PlatformBackend::getActiveToplevel() const {
    return nullptr;
}

} // namespace nisaba::backend_os::win32

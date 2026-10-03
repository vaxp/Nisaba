#pragma once
/// @file wayland_platform.hpp
/// @brief Native Wayland Client Platform backend with wlr-layer-shell, xdg-shell, and EGL.

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/layer_surface.hpp"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <xkbcommon/xkbcommon.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

// Wayland protocol headers are generated as C — must be wrapped in extern "C"
// to prevent the parameter named 'namespace' from conflicting with C++ keyword.
extern "C" {
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "wlr-foreign-toplevel-management-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"
}

#include "nisaba/backend_os/output.hpp"
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <string>

namespace nisaba::backend_os::wayland {

class WaylandOutput;

class WaylandPlatformBackend {
public:
    WaylandPlatformBackend(Platform* owner);
    ~WaylandPlatformBackend();

    bool init();
    void shutdown();
    bool pollEvents();

    [[nodiscard]] wl_display* getDisplay() const { return display_; }
    [[nodiscard]] wl_compositor* getCompositor() const { return compositor_; }
    [[nodiscard]] zwlr_layer_shell_v1* getLayerShell() const { return layer_shell_; }
    [[nodiscard]] xdg_wm_base* getXdgWmBase() const { return xdg_wm_base_; }
    [[nodiscard]] zxdg_output_manager_v1* getXdgOutputManager() const { return xdg_output_manager_; }
    [[nodiscard]] zxdg_decoration_manager_v1* getDecorationManager() const { return decoration_manager_; }
    [[nodiscard]] wl_seat* getSeat() const { return seat_; }
    [[nodiscard]] EGLDisplay getEGLDisplay() const { return egl_display_; }
    [[nodiscard]] EGLConfig getEGLConfig() const { return egl_config_; }
    [[nodiscard]] EGLContext getEGLContext() const { return egl_context_; }
    [[nodiscard]] Platform* getOwner() const { return owner_; }

    // Output / Monitor Management
    [[nodiscard]] std::vector<std::shared_ptr<Output>> getOutputs() const;
    [[nodiscard]] std::shared_ptr<Output> getOutputByName(std::string_view name) const;
    [[nodiscard]] std::shared_ptr<Output> getPrimaryOutput() const;
    [[nodiscard]] std::shared_ptr<WaylandOutput> findOutputByPtr(const WaylandOutput* ptr) const;

    void registerSurface(LayerSurface* surface);
    void unregisterSurface(LayerSurface* surface);

    // Cursor management
    void setCursor(SystemCursor cursor);
    void updateWaylandCursor();

    // Clipboard Subsystem
    void setClipboardData(const ClipboardData& data, ClipboardType type);
    [[nodiscard]] ClipboardData getClipboardData(ClipboardType type) const;
    [[nodiscard]] std::vector<uint8_t> getClipboardDataForMime(std::string_view mime_type, ClipboardType type) const;
    [[nodiscard]] std::vector<std::string> getClipboardFormats(ClipboardType type) const;
    [[nodiscard]] bool hasClipboardFormat(std::string_view mime_type, ClipboardType type) const;

    // Drag and Drop Subsystem
    bool startDrag(const DragData& data, DragAction actions);

    // Foreign Toplevel Subsystem
    [[nodiscard]] std::vector<std::shared_ptr<ToplevelWindow>> getToplevels() const;
    [[nodiscard]] std::shared_ptr<ToplevelWindow> getActiveToplevel() const;

    // Internal registry & seat listeners
    void handleGlobal(uint32_t name, const char* interface, uint32_t version);
    void handleGlobalRemove(uint32_t name);
    void handleSeatCapabilities(wl_seat* seat, uint32_t caps);

    // Pointer events
    void handlePointerEnter(uint32_t serial, wl_surface* surface, wl_fixed_t sx, wl_fixed_t sy);
    void handlePointerLeave(wl_surface* surface);
    void handlePointerMotion(uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
    void handlePointerButton(uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    void handlePointerAxis(uint32_t time, uint32_t axis, wl_fixed_t value);

    // Keyboard events
    void handleKeymap(uint32_t format, int fd, uint32_t size);
    void handleKeyboardEnter(uint32_t serial, wl_surface* surface);
    void handleKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void handleModifiers(uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

    // Data device callbacks
    void handleDataOffer(wl_data_offer* offer);
    void handleDataDeviceEnter(uint32_t serial, wl_surface* surface, wl_fixed_t x, wl_fixed_t y, wl_data_offer* offer);
    void handleDataDeviceLeave();
    void handleDataDeviceMotion(uint32_t time, wl_fixed_t x, wl_fixed_t y);
    void handleDataDeviceDrop();
    void handleDataDeviceSelection(wl_data_offer* offer);

    // Foreign toplevel callbacks
    void handleToplevelHandle(zwlr_foreign_toplevel_handle_v1* handle);
    void handleToplevelTitle(zwlr_foreign_toplevel_handle_v1* handle, const char* title);
    void handleToplevelAppId(zwlr_foreign_toplevel_handle_v1* handle, const char* app_id);
    void handleToplevelState(zwlr_foreign_toplevel_handle_v1* handle, wl_array* state);
    void handleToplevelDone(zwlr_foreign_toplevel_handle_v1* handle);
    void handleToplevelClosed(zwlr_foreign_toplevel_handle_v1* handle);

    void setKeyRepeatInfo(int32_t rate, int32_t delay);

    uint32_t getLastPointerSerial() const { return last_pointer_serial_; }
    uint32_t getLastKeyboardSerial() const { return last_keyboard_serial_; }

    // Wayland Data Transfer Structs
    struct WaylandOfferContext {
        wl_data_offer* offer = nullptr;
        std::vector<std::string> mime_types;
        uint32_t source_actions = 0;
        uint32_t dnd_action = 0;
    };

    struct ActiveDataSource {
        wl_data_source* source = nullptr;
        ClipboardData   data;
    };

private:
    Platform* owner_ = nullptr;

    wl_display*                         display_             = nullptr;
    wl_registry*                        registry_            = nullptr;
    wl_compositor*                      compositor_          = nullptr;
    wl_subcompositor*                   subcompositor_       = nullptr;
    wl_shm*                             shm_                 = nullptr;
    wl_seat*                            seat_                = nullptr;
    wl_pointer*                         pointer_             = nullptr;
    wl_keyboard*                        keyboard_            = nullptr;
    wl_touch*                           touch_               = nullptr;
    zwlr_layer_shell_v1*                layer_shell_         = nullptr;
    xdg_wm_base*                        xdg_wm_base_         = nullptr;
    wl_data_device_manager*             data_device_manager_ = nullptr;
    wl_data_device*                     data_device_         = nullptr;
    zwlr_foreign_toplevel_manager_v1*   toplevel_manager_    = nullptr;
    zxdg_output_manager_v1*             xdg_output_manager_  = nullptr;
    zxdg_decoration_manager_v1*         decoration_manager_  = nullptr;

    // Wayland cursor support
    wl_cursor_theme*     cursor_theme_   = nullptr;
    wl_surface*          cursor_surface_ = nullptr;
    uint32_t             last_pointer_serial_ = 0;
    uint32_t             last_keyboard_serial_ = 0;
    SystemCursor         current_cursor_ = SystemCursor::Arrow;

    std::vector<std::shared_ptr<WaylandOutput>> outputs_;
    std::unordered_map<wl_output*, std::shared_ptr<WaylandOutput>> output_map_;
    std::unordered_set<LayerSurface*> surfaces_;

    // XKB Keymap
    xkb_context* xkb_context_ = nullptr;
    xkb_keymap*  xkb_keymap_  = nullptr;
    xkb_state*   xkb_state_   = nullptr;
    int          active_modifiers_ = 0;

    // Key Repeat State
    int32_t  repeat_rate_ = 0;
    int32_t  repeat_delay_ = 0;
    uint32_t repeating_key_ = 0;
    uint32_t repeating_keycode_ = 0;
    double   repeat_next_fire_time_ = 0.0;

    // EGL
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLConfig  egl_config_  = nullptr;
    EGLContext egl_context_ = EGL_NO_CONTEXT;
    int        egl_major_   = 0;
    int        egl_minor_   = 0;

    // Active surface under pointer
    wl_surface* pointer_focus_surface_ = nullptr;
    float last_px_ = 0.0f;
    float last_py_ = 0.0f;

    // Active Clipboard and DnD state
    std::unordered_map<wl_data_offer*, std::shared_ptr<WaylandOfferContext>> active_offers_;
    std::shared_ptr<WaylandOfferContext> current_selection_offer_;
    std::shared_ptr<WaylandOfferContext> current_dnd_offer_;
    uint32_t last_dnd_serial_ = 0;

    // Outgoing Data Source
    std::unique_ptr<ActiveDataSource> outgoing_selection_source_;
    std::unique_ptr<ActiveDataSource> outgoing_dnd_source_;

    // Local fallback clipboard buffer
    ClipboardData local_clipboard_;
    ClipboardData local_primary_;

    // Foreign Toplevel State
    class WaylandToplevel;
    std::vector<std::shared_ptr<WaylandToplevel>> toplevels_;
    std::unordered_map<zwlr_foreign_toplevel_handle_v1*, std::shared_ptr<WaylandToplevel>> toplevel_map_;
    std::shared_ptr<WaylandToplevel> active_toplevel_;
};

} // namespace nisaba::backend_os::wayland

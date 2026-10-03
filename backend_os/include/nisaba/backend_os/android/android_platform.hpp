#pragma once
/// @file android_platform.hpp
/// @brief Android (NDK NativeActivity) platform backend.
/// Internal backend used by Platform when compiled for __ANDROID__.
/// Mirrors the interface of Win32PlatformBackend and X11PlatformBackend.

#ifdef __ANDROID__

#include "nisaba/backend_os/platform.hpp"
#include "nisaba/backend_os/permissions.hpp"
#include "nisaba/backend_os/output.hpp"

#include <android/native_activity.h>
#include <android/input.h>
#include <android/looper.h>
#include <android/configuration.h>
#include <android/log.h>

#include <EGL/egl.h>
#include <GLES3/gl3.h>

#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <functional>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>

#define Nisaba_ANDROID_LOG_TAG "nisaba"
#define Nisaba_ALOG(...)  __android_log_print(ANDROID_LOG_DEBUG,  Nisaba_ANDROID_LOG_TAG, __VA_ARGS__)
#define Nisaba_ALOGE(...) __android_log_print(ANDROID_LOG_ERROR,  Nisaba_ANDROID_LOG_TAG, __VA_ARGS__)
#define Nisaba_ALOGW(...) __android_log_print(ANDROID_LOG_WARN,   Nisaba_ANDROID_LOG_TAG, __VA_ARGS__)

namespace nisaba::backend_os {
class Window;
} // namespace nisaba::backend_os

namespace nisaba::backend_os::android {

// ── Forward declarations ──────────────────────────────────────
class AndroidSurface;
class AndroidPlatformBackend;

/// Shared glue state stored in ANativeActivity::instance.
struct NisabaAndroidGlue {
    ANativeActivity*                      activity       = nullptr;
    AndroidPlatformBackend*               backend        = nullptr;
    ANativeWindow*                        native_window  = nullptr;
    AInputQueue*                          input_queue    = nullptr;
    std::thread                           app_thread;
    std::mutex                            mutex;
    std::condition_variable               cv;
    bool                                  window_created = false;
    bool                                  destroyed      = false;
};

// ────────────────────────────────────────────────────────────────
// AndroidPlatformBackend
// ────────────────────────────────────────────────────────────────

/// Android platform backend — owns EGL, input queue, and lifecycle hooks.
/// Created and owned by Platform::Impl when compiled for Android.
class AndroidPlatformBackend {
public:
    explicit AndroidPlatformBackend(Platform* owner, ANativeActivity* activity);
    ~AndroidPlatformBackend();

    // Non-copyable, non-movable
    AndroidPlatformBackend(const AndroidPlatformBackend&) = delete;
    AndroidPlatformBackend& operator=(const AndroidPlatformBackend&) = delete;

    bool init();
    void shutdown();

    /// Poll ALooper for pending events and dispatch them.
    /// @return false if the application has been asked to quit.
    bool pollEvents();

    // ── EGL accessors (used by AndroidSurface) ───────────────────
    [[nodiscard]] EGLDisplay getEGLDisplay() const { return egl_display_; }
    [[nodiscard]] EGLConfig  getEGLConfig()  const { return egl_config_; }
    [[nodiscard]] EGLContext getEGLContext() const { return egl_context_; }

    // ── Activity / Looper ────────────────────────────────────────
    [[nodiscard]] ANativeActivity* getActivity() const { return activity_; }
    [[nodiscard]] ALooper*         getLooper()   const { return looper_; }
    [[nodiscard]] AConfiguration*  getConfig()   const { return config_; }
    [[nodiscard]] ANativeWindow*   getNativeWindow() const { return current_native_window_; }

    // ── Clipboard (via JNI) ──────────────────────────────────────
    void setClipboardData(const ClipboardData& data, ClipboardType type = ClipboardType::Clipboard);
    void setClipboardText(std::string_view text, ClipboardType type = ClipboardType::Clipboard);
    [[nodiscard]] std::string getClipboardText(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] ClipboardData getClipboardData(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<uint8_t> getClipboardDataForMime(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] std::vector<std::string> getClipboardFormats(ClipboardType type = ClipboardType::Clipboard) const;
    [[nodiscard]] bool hasClipboardFormat(std::string_view mime_type, ClipboardType type = ClipboardType::Clipboard) const;

    // ── Drag & Drop — no-op on Android ──────────────────────────
    bool startDrag(const DragData& /*data*/, DragAction /*actions*/) { return false; }

    // ── Cursor — no-op on Android ────────────────────────────────
    void setCursor(SystemCursor /*cursor*/) {}

    // ── Output / Monitor ─────────────────────────────────────────
    [[nodiscard]] std::vector<std::shared_ptr<Output>> getOutputs() const;
    [[nodiscard]] std::shared_ptr<Output> getOutputByName(std::string_view name) const;
    [[nodiscard]] std::shared_ptr<Output> getPrimaryOutput() const;
    void updateOutputFromConfig();

    // ── Foreign Toplevel — not applicable on Android ─────────────
    [[nodiscard]] std::vector<std::shared_ptr<ToplevelWindow>> getToplevels() const { return {}; }
    [[nodiscard]] std::shared_ptr<ToplevelWindow> getActiveToplevel() const { return nullptr; }

    // ── Lifecycle callbacks (called by android_app_glue) ─────────

    /// Called when the ANativeWindow is ready for rendering.
    /// AndroidSurface registers itself to receive this notification.
    void onNativeWindowCreated(ANativeWindow* window);

    /// Called when the ANativeWindow is about to be destroyed.
    void onNativeWindowDestroyed(ANativeWindow* window);

    /// Called when focus changes.
    void onWindowFocusChanged(bool has_focus);

    /// Called when the app should pause (e.g. home button pressed).
    void onPause();

    /// Called when the app resumes.
    void onResume();

    /// Called when the process should terminate.
    void onDestroy();

    /// Attach an input queue for event processing.
    void onInputQueueCreated(AInputQueue* queue);

    /// Detach the input queue.
    void onInputQueueDestroyed(AInputQueue* queue);

    // ── Surface registration ─────────────────────────────────────
    void registerSurface(AndroidSurface* surface);
    void unregisterSurface(AndroidSurface* surface);

    [[nodiscard]] Platform* getOwner() const { return owner_; }

    // ── Display metrics ──────────────────────────────────────────
    [[nodiscard]] float getDpiScale() const { return dpi_scale_; }

    // ── Safe Area ────────────────────────────────────────────────
    /// Returns the system safe area insets in logical dp units.
    /// On Android this queries status_bar_height and navigation_bar_height
    /// from system resources via JNI, with sensible fallbacks.
    [[nodiscard]] EdgeInsets getSafeAreaInsets() const;

    // ── Runtime Permissions ──────────────────────────────────────
    /// Check the status of an Android permission string.
    [[nodiscard]] PermissionStatus checkPermission(std::string_view permission);

    /// Check if rationale should be shown before requesting the permission.
    [[nodiscard]] bool shouldShowRationale(std::string_view permission);

    /// Request a set of Android permissions asynchronously.
    void requestPermissions(const std::vector<std::string>& permissions,
                            std::function<void(const std::unordered_map<std::string, PermissionStatus>&)> callback);

    /// Launch system application settings page for this package.
    bool openAppSettings();

    /// Get Android SDK version (Build.VERSION.SDK_INT).
    [[nodiscard]] int getAndroidSdkVersion() const;

    /// Called by onWindowFocusChanged(true) / onResume() to resolve any pending permission requests.
    void resolvePendingPermissions();

    /// Called by JNIEXPORT onPermissionsResult if custom Java activity is used.
    void onNativePermissionsResult(int request_code,
                                   const std::vector<std::string>& permissions,
                                   const std::vector<int>& grant_results);

private:
    Platform*        owner_    = nullptr;
    ANativeActivity* activity_ = nullptr;
    ALooper*         looper_   = nullptr;
    AConfiguration*  config_   = nullptr;
    ANativeWindow*   current_native_window_ = nullptr;  ///< engine-thread-only after init

    // EGL shared context (engine-thread-only)
    EGLDisplay egl_display_ = EGL_NO_DISPLAY;
    EGLConfig  egl_config_  = nullptr;
    EGLContext egl_context_ = EGL_NO_CONTEXT;

    // Display info
    float dpi_scale_     = 1.0f;
    int   screen_width_  = 0;
    int   screen_height_ = 0;

    // Clipboard fallback buffer
    mutable ClipboardData clipboard_buffer_;

    // ── Registered surfaces (protected by surfaces_mutex_) ────────
    std::unordered_set<AndroidSurface*> surfaces_;

    // ── Input queue (protected by input_queue_mutex_) ─────────────
    AInputQueue* input_queue_ = nullptr;

    // ── Thread-safe runtime state ──────────────────────────────────
    //    Written by UI thread, read by engine thread.
    std::atomic<bool> quit_requested_{ false };
    std::atomic<bool> paused_        { false };
    /// true iff an EGL surface is currently valid and renderable.
    std::atomic<bool> has_window_    { false };

    // ── Deferred window events ─────────────────────────────────────
    // NativeActivity lifecycle callbacks arrive on the UI thread, but all
    // EGL surface operations MUST happen on the engine (GL) thread.
    // The UI thread queues events here; the engine processes them in
    // processPendingWindowEvents() at the top of each pollEvents() call.
    std::atomic<bool> needs_processing_{ false };
    std::mutex        window_event_mutex_;
    bool              window_destroy_pending_ = false;
    ANativeWindow*    window_create_pending_  = nullptr;

    // ── Synchronisation primitives ─────────────────────────────────
    mutable std::mutex      surfaces_mutex_;           ///< guards surfaces_ set
    std::mutex              input_queue_mutex_;        ///< guards input_queue_
    std::mutex              state_mutex_;              ///< paired with state_cv_
    std::condition_variable state_cv_;                 ///< wakes engine thread on new events
    std::mutex              window_destroy_ack_mutex_; ///< guards window_destroy_ack_cv_
    std::condition_variable window_destroy_ack_cv_;    ///< signals UI thread when engine freed surface

    // ── Output representing the device screen ─────────────────────
    class AndroidOutput;
    std::shared_ptr<AndroidOutput> primary_output_;

    // ── Safe area insets cache ─────────────────────────────────────
    mutable std::mutex        insets_mutex_;
    mutable EdgeInsets        cached_insets_;
    mutable std::atomic<bool> insets_dirty_{ true };

    // ── EGL initialisation (engine thread) ───────────────────────
    bool initEGL();
    void destroyEGL();

    // ── Deferred window lifecycle (engine thread) ─────────────────
    /// Process any window-created / window-destroyed events queued by the UI thread.
    /// Must be called from the engine thread so EGL ops run on the correct thread.
    void processPendingWindowEvents();

    // ── Input processing (engine thread) ─────────────────────────
    void processInputQueue();
    void handleInputEvent(AInputEvent* event);
    void handleTouchEvent(AInputEvent* event);
    void handleKeyEvent(AInputEvent* event);

    // ── JNI helpers ───────────────────────────────────────────────
    std::string jniGetClipboardText() const;
    void        jniSetClipboardText(std::string_view text) const;

    // ── Permissions JNI helpers & state ───────────────────────────
    struct PendingPermissionRequest {
        int request_code = 0;
        std::vector<std::string> permissions;
        std::function<void(const std::unordered_map<std::string, PermissionStatus>&)> callback;
        std::vector<bool> pre_rationales;
    };

    mutable std::mutex                    permissions_mutex_;
    std::vector<PendingPermissionRequest> pending_permission_requests_;
    std::atomic<int>                      next_permission_request_code_{ 1000 };
    mutable int                           cached_sdk_version_ = -1;

    int  jniCheckPermission(std::string_view permission) const;
    bool jniShouldShowRationale(std::string_view permission) const;
    void jniRequestPermissions(const std::vector<std::string>& permissions, int request_code);
    bool jniOpenAppSettings();
    int  jniGetSdkVersion() const;

    // ── ALooper pipe for wakeup ───────────────────────────────────
    int  pipe_read_fd_  = -1;
    int  pipe_write_fd_ = -1;
    void createWakeupPipe();
    void destroyWakeupPipe();
    /// Write a byte to the wakeup pipe so ALooper_pollOnce / state_cv_ wakes up.
    void wakeupLooper();
    static int looperCallback(int fd, int events, void* data);
};

} // namespace nisaba::backend_os::android

#endif // __ANDROID__

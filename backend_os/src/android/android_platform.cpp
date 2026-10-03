/// @file android_platform.cpp
/// @brief Android (NDK NativeActivity) platform backend implementation.
/// All Android-specific code is isolated here behind __ANDROID__ guards.

#ifdef __ANDROID__

#include "nisaba/backend_os/android/android_platform.hpp"
#include "nisaba/backend_os/android/android_surface.hpp"
#include "nisaba/backend_os/clipboard.hpp"

#include <android/configuration.h>
#include <android/native_window.h>
#include <jni.h>

#include <unistd.h>  // pipe(), read(), write()
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cmath>
#include <sstream>
#include <chrono>

namespace nisaba::backend_os::android {

static AndroidPlatformBackend* s_active_backend = nullptr;

// ════════════════════════════════════════════════════════════════
// AndroidOutput — represents the device screen as a single Output
// ════════════════════════════════════════════════════════════════

class AndroidPlatformBackend::AndroidOutput : public Output {
public:
    // Deliberately simple — Android devices have one screen
    uint32_t     id_          = 0;
    std::string  name_        = "android-screen";
    std::string  make_        = "Android";
    std::string  model_       = "Device";
    std::string  description_ = "Android Display";
    Rect         geometry_;
    Rect         logical_geometry_;
    int32_t      physical_width_mm_  = 0;
    int32_t      physical_height_mm_ = 0;
    int32_t      scale_factor_       = 1;
    double       fractional_scale_   = 1.0;
    OutputTransform transform_       = OutputTransform::Normal;
    OutputSubpixel  subpixel_        = OutputSubpixel::Unknown;
    std::vector<OutputMode> modes_;
    OutputMode   current_mode_;
    bool         is_primary_        = true;

    [[nodiscard]] uint32_t     id()              const noexcept override { return id_; }
    [[nodiscard]] const std::string& name()      const noexcept override { return name_; }
    [[nodiscard]] const std::string& make()      const noexcept override { return make_; }
    [[nodiscard]] const std::string& model()     const noexcept override { return model_; }
    [[nodiscard]] const std::string& description() const noexcept override { return description_; }
    [[nodiscard]] Rect         geometry()        const noexcept override { return geometry_; }
    [[nodiscard]] Rect         logicalGeometry() const noexcept override { return logical_geometry_; }
    [[nodiscard]] int32_t      physicalWidthMm() const noexcept override { return physical_width_mm_; }
    [[nodiscard]] int32_t      physicalHeightMm()const noexcept override { return physical_height_mm_; }
    [[nodiscard]] int32_t      scaleFactor()     const noexcept override { return scale_factor_; }
    [[nodiscard]] double       fractionalScale() const noexcept override { return fractional_scale_; }
    [[nodiscard]] OutputTransform transform()    const noexcept override { return transform_; }
    [[nodiscard]] OutputSubpixel  subpixel()     const noexcept override { return subpixel_; }
    [[nodiscard]] const std::vector<OutputMode>& modes() const noexcept override { return modes_; }
    [[nodiscard]] const OutputMode& currentMode()const noexcept override { return current_mode_; }
    [[nodiscard]] bool         isPrimary()       const noexcept override { return is_primary_; }
    [[nodiscard]] void*        nativeHandle()    const noexcept override { return nullptr; }
};

// ════════════════════════════════════════════════════════════════
// Constructor / Destructor
// ════════════════════════════════════════════════════════════════

AndroidPlatformBackend::AndroidPlatformBackend(Platform* owner, ANativeActivity* activity)
    : owner_(owner), activity_(activity)
{
}

AndroidPlatformBackend::~AndroidPlatformBackend() {
    shutdown();
}

// ════════════════════════════════════════════════════════════════
// init / shutdown
// ════════════════════════════════════════════════════════════════

bool AndroidPlatformBackend::init() {
    Nisaba_ALOG("AndroidPlatformBackend::init");

    // Read device configuration
    config_ = AConfiguration_new();
    if (config_) {
        AConfiguration_fromAssetManager(config_, activity_->assetManager);
    }

    // Acquire ALooper for the main thread
    looper_ = ALooper_forThread();
    if (!looper_) {
        looper_ = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }
    if (!looper_) {
        Nisaba_ALOGE("Failed to obtain ALooper");
        return false;
    }

    // Create wakeup pipe (allows pollEvents to be interrupted)
    createWakeupPipe();

    // Initialise EGL shared context (surfaces created later per-window)
    if (!initEGL()) {
        Nisaba_ALOGE("EGL initialisation failed");
        return false;
    }

    // Build initial output information from the configuration
    updateOutputFromConfig();

    // Connect backend to glue and adopt any pre-existing native window / input queue.
    // init() runs on the engine thread, so we set state directly — no queueing needed.
    auto* glue = reinterpret_cast<NisabaAndroidGlue*>(activity_->instance);
    if (glue) {
        glue->backend = this;
        if (glue->native_window) {
            Nisaba_ALOG("Adopting native window from glue: %p", glue->native_window);
            current_native_window_ = glue->native_window;
            has_window_.store(true, std::memory_order_release);
        }
        if (glue->input_queue) {
            Nisaba_ALOG("Adopting input queue from glue: %p", glue->input_queue);
            std::lock_guard<std::mutex> lk(input_queue_mutex_);
            input_queue_ = glue->input_queue;
        }
    }

    s_active_backend = this;
    Nisaba_ALOG("AndroidPlatformBackend initialised — EGL %d.%d", 0, 0);
    return true;
}

void AndroidPlatformBackend::shutdown() {
    Nisaba_ALOG("AndroidPlatformBackend::shutdown");
    if (s_active_backend == this) {
        s_active_backend = nullptr;
    }
    surfaces_.clear();
    destroyEGL();
    destroyWakeupPipe();
    if (config_) {
        AConfiguration_delete(config_);
        config_ = nullptr;
    }
}

// ════════════════════════════════════════════════════════════════
// EGL — shared context lifetime
// ════════════════════════════════════════════════════════════════

bool AndroidPlatformBackend::initEGL() {
    egl_display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl_display_ == EGL_NO_DISPLAY) {
        Nisaba_ALOGE("eglGetDisplay failed");
        return false;
    }

    EGLint major = 0, minor = 0;
    if (!eglInitialize(egl_display_, &major, &minor)) {
        Nisaba_ALOGE("eglInitialize failed");
        return false;
    }
    Nisaba_ALOG("EGL version %d.%d", major, minor);

    // Config: RGBA8, depth 24, stencil 8, OpenGL ES 3
    const EGLint config_attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_ALPHA_SIZE,      8,
        EGL_DEPTH_SIZE,      24,
        EGL_STENCIL_SIZE,    8,
        EGL_NONE
    };

    EGLint num_configs = 0;
    if (!eglChooseConfig(egl_display_, config_attribs, &egl_config_, 1, &num_configs)
        || num_configs == 0)
    {
        // Fallback: try without depth/stencil
        const EGLint fallback_attribs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
            EGL_RED_SIZE,   8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE,  8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };
        if (!eglChooseConfig(egl_display_, fallback_attribs, &egl_config_, 1, &num_configs)
            || num_configs == 0)
        {
            Nisaba_ALOGE("eglChooseConfig failed");
            return false;
        }
        Nisaba_ALOGW("Using EGL config without depth/stencil");
    }

    // Create a shared context — surfaces are created per-window
    const EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    egl_context_ = eglCreateContext(egl_display_, egl_config_, EGL_NO_CONTEXT, ctx_attribs);
    if (egl_context_ == EGL_NO_CONTEXT) {
        Nisaba_ALOGE("eglCreateContext failed (error 0x%x)", eglGetError());
        return false;
    }

    Nisaba_ALOG("EGL context created successfully");
    return true;
}

void AndroidPlatformBackend::destroyEGL() {
    if (egl_display_ == EGL_NO_DISPLAY) return;

    eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    if (egl_context_ != EGL_NO_CONTEXT) {
        eglDestroyContext(egl_display_, egl_context_);
        egl_context_ = EGL_NO_CONTEXT;
    }

    eglTerminate(egl_display_);
    egl_display_ = EGL_NO_DISPLAY;
    egl_config_  = nullptr;
}

// ════════════════════════════════════════════════════════════════
// Wakeup pipe
// ════════════════════════════════════════════════════════════════

static int s_wakeup_ident = 1;  // ALooper ident for the wakeup pipe

void AndroidPlatformBackend::createWakeupPipe() {
    int fds[2];
    if (pipe2(fds, O_NONBLOCK | O_CLOEXEC) != 0) {
        if (pipe(fds) != 0) {
            Nisaba_ALOGE("pipe() failed: %s", strerror(errno));
            return;
        }
        int flags0 = fcntl(fds[0], F_GETFL, 0);
        fcntl(fds[0], F_SETFL, flags0 | O_NONBLOCK);
        int flags1 = fcntl(fds[1], F_GETFL, 0);
        fcntl(fds[1], F_SETFL, flags1 | O_NONBLOCK);
    }
    pipe_read_fd_  = fds[0];
    pipe_write_fd_ = fds[1];

    // Register the read end with ALooper
    ALooper_addFd(looper_, pipe_read_fd_, s_wakeup_ident,
                  ALOOPER_EVENT_INPUT, nullptr, nullptr);
}

void AndroidPlatformBackend::destroyWakeupPipe() {
    if (pipe_read_fd_ >= 0) {
        ALooper_removeFd(looper_, pipe_read_fd_);
        close(pipe_read_fd_);
        pipe_read_fd_ = -1;
    }
    if (pipe_write_fd_ >= 0) {
        close(pipe_write_fd_);
        pipe_write_fd_ = -1;
    }
}

void AndroidPlatformBackend::wakeupLooper() {
    if (pipe_write_fd_ >= 0) {
        char c = 1;
        (void)write(pipe_write_fd_, &c, 1);
    }
    if (looper_) {
        ALooper_wake(looper_);
    }
}

// ════════════════════════════════════════════════════════════════
// Deferred window events — processPendingWindowEvents
// All EGL surface creation / destruction runs on the engine thread.
// The UI thread only stores pointers and notifies; the engine does the work.
// ════════════════════════════════════════════════════════════════

void AndroidPlatformBackend::processPendingWindowEvents() {
    bool          do_destroy = false;
    ANativeWindow* do_create = nullptr;

    {
        std::lock_guard<std::mutex> lk(window_event_mutex_);
        do_destroy = window_destroy_pending_;
        do_create  = window_create_pending_;
        window_destroy_pending_ = false;
        window_create_pending_  = nullptr;
    }
    needs_processing_.store(false, std::memory_order_release);

    // ── Destroy EGL surface (keep EGL context alive for fast recreation) ────────
    if (do_destroy) {
        Nisaba_ALOG("processPendingWindowEvents: destroying EGL surfaces");
        current_native_window_ = nullptr;
        has_window_.store(false, std::memory_order_release);

        std::vector<AndroidSurface*> slist;
        {
            std::lock_guard<std::mutex> lk(surfaces_mutex_);
            slist.assign(surfaces_.begin(), surfaces_.end());
        }
        for (auto* s : slist) s->onNativeWindowDestroyed();

        // Acknowledge to the UI thread that the native window has been fully released
        {
            std::lock_guard<std::mutex> lk(window_destroy_ack_mutex_);
            window_destroy_ack_cv_.notify_all();
        }
    }

    // ── Create EGL surface for the new native window ──────────────────────────
    if (do_create) {
        Nisaba_ALOG("processPendingWindowEvents: creating EGL surfaces for %p", do_create);
        current_native_window_ = do_create;

        std::vector<AndroidSurface*> slist;
        {
            std::lock_guard<std::mutex> lk(surfaces_mutex_);
            slist.assign(surfaces_.begin(), surfaces_.end());
        }
        for (auto* s : slist) s->onNativeWindowCreated(do_create);

        has_window_.store(true, std::memory_order_release);
        // Geometry may have changed (rotation) — invalidate safe-area cache
        insets_dirty_.store(true, std::memory_order_relaxed);
    }
}

// ════════════════════════════════════════════════════════════════
// pollEvents — main event loop tick
// ════════════════════════════════════════════════════════════════

bool AndroidPlatformBackend::pollEvents() {
    // ── Step 1: Process any queued window lifecycle events ──────────────────────
    // EGL surface creation / destruction must happen on the engine (GL) thread.
    processPendingWindowEvents();

    // ── Step 2: Block engine thread when there is no renderable surface ──────────
    // This eliminates busy-spinning and guarantees that makeCurrent /
    // swapBuffers are never called while the EGL surface is absent,
    // fixing the black-screen-on-resume and random crash after backgrounding.
    while (!has_window_.load(std::memory_order_acquire)) {
        if (quit_requested_.load(std::memory_order_relaxed)) {
            owner_->onQuit().emit();
            return false;
        }
        Nisaba_ALOG("pollEvents: no surface — engine thread suspending");
        {
            std::unique_lock<std::mutex> lock(state_mutex_);
            state_cv_.wait(lock, [this] {
                return needs_processing_.load(std::memory_order_relaxed)
                    || quit_requested_.load(std::memory_order_relaxed);
            });
        }
        // Re-process whatever woke us up (surface create / quit)
        processPendingWindowEvents();
    }

    if (quit_requested_.load(std::memory_order_relaxed)) {
        owner_->onQuit().emit();
        return false;
    }

    // ── Step 3: Drain the ALooper (non-blocking) ─────────────────────────────
    int   ident   = 0;
    int   events  = 0;
    void* data_ptr = nullptr;
    while ((ident = ALooper_pollOnce(0, nullptr, &events, &data_ptr)) >= 0) {
        if (ident == s_wakeup_ident) {
            char buf[64];
            while (read(pipe_read_fd_, buf, sizeof(buf)) > 0) {}
        }
    }

    // ── Step 4: Process pending input events ──────────────────────────────
    processInputQueue();

    return !quit_requested_.load(std::memory_order_relaxed);
}

// ════════════════════════════════════════════════════════════════
// Input Processing
// ════════════════════════════════════════════════════════════════

void AndroidPlatformBackend::processInputQueue() {
    // Snapshot the queue pointer under mutex — the UI thread may replace it
    // via onInputQueueCreated / onInputQueueDestroyed at any time.
    AInputQueue* queue;
    {
        std::lock_guard<std::mutex> lk(input_queue_mutex_);
        queue = input_queue_;
    }
    if (!queue) return;

    AInputEvent* event = nullptr;
    while (AInputQueue_hasEvents(queue) > 0) {
        if (AInputQueue_getEvent(queue, &event) < 0) break;
        if (AInputQueue_preDispatchEvent(queue, event)) {
            continue;  // IME consumed the event; already finished
        }
        handleInputEvent(event);
        AInputQueue_finishEvent(queue, event, 1 /*handled*/);
    }
}

void AndroidPlatformBackend::handleInputEvent(AInputEvent* event) {
    int32_t type = AInputEvent_getType(event);

    if (type == AINPUT_EVENT_TYPE_MOTION) {
        handleTouchEvent(event);
    } else if (type == AINPUT_EVENT_TYPE_KEY) {
        handleKeyEvent(event);
    }
}

void AndroidPlatformBackend::handleTouchEvent(AInputEvent* event) {
    // Map primary pointer (index 0) to mouse-like signals.
    // This keeps full backward compatibility with the existing signal system.
    int32_t action      = AMotionEvent_getAction(event);
    int32_t action_code = action & AMOTION_EVENT_ACTION_MASK;

    // Primary pointer coordinates
    float x = AMotionEvent_getX(event, 0);
    float y = AMotionEvent_getY(event, 0);

    // Dispatch to Platform global signals — no window handle on Android
    switch (action_code) {
        case AMOTION_EVENT_ACTION_DOWN:
        case AMOTION_EVENT_ACTION_POINTER_DOWN: {
            if ((action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT) == 0) {
                // Primary pointer down → left mouse button down
                owner_->onMouseDown().emit(x, y, 1 /*left button*/);
            }
            break;
        }
        case AMOTION_EVENT_ACTION_UP:
        case AMOTION_EVENT_ACTION_POINTER_UP: {
            if ((action >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT) == 0) {
                owner_->onMouseUp().emit(x, y, 1 /*left button*/);
            }
            break;
        }
        case AMOTION_EVENT_ACTION_MOVE: {
            owner_->onMouseMove().emit(x, y);
            break;
        }
        case AMOTION_EVENT_ACTION_CANCEL: {
            // Treat as mouse-up to clean up widget state
            owner_->onMouseUp().emit(x, y, 1);
            break;
        }
        default:
            break;
    }
}

void AndroidPlatformBackend::handleKeyEvent(AInputEvent* event) {
    int32_t action   = AKeyEvent_getAction(event);
    int32_t keycode  = AKeyEvent_getKeyCode(event);
    int32_t meta     = AKeyEvent_getMetaState(event);

    // Map Android meta flags to nisaba KeyMod bitmask
    int modifiers = 0;
    if (meta & AMETA_SHIFT_ON)   modifiers |= static_cast<int>(KeyMod::Shift);
    if (meta & AMETA_CTRL_ON)    modifiers |= static_cast<int>(KeyMod::Ctrl);
    if (meta & AMETA_ALT_ON)     modifiers |= static_cast<int>(KeyMod::Alt);
    if (meta & AMETA_META_ON)    modifiers |= static_cast<int>(KeyMod::Super);

    if (action == AKEY_EVENT_ACTION_DOWN) {
        // Back button → quit signal; wake the engine in case it is suspended
        if (keycode == AKEYCODE_BACK) {
            quit_requested_.store(true, std::memory_order_release);
            needs_processing_.store(true, std::memory_order_release);
            wakeupLooper();
            {
                std::lock_guard<std::mutex> lk(state_mutex_);
                state_cv_.notify_all();
            }
            return;
        }
        owner_->onKeyDown().emit(keycode, modifiers);
    } else if (action == AKEY_EVENT_ACTION_UP) {
        owner_->onKeyUp().emit(keycode, modifiers);
    }
}

// ════════════════════════════════════════════════════════════════
// Lifecycle callbacks (invoked on the UI / main thread by NativeActivity)
// Surface creation / destruction is DEFERRED to the engine thread via
// processPendingWindowEvents() to satisfy OpenGL ES threading requirements.
// ════════════════════════════════════════════════════════════════

void AndroidPlatformBackend::onNativeWindowCreated(ANativeWindow* window) {
    Nisaba_ALOG("onNativeWindowCreated (UI thread): %p", window);
    {
        std::lock_guard<std::mutex> lk(window_event_mutex_);
        window_create_pending_  = window;
    }
    needs_processing_.store(true, std::memory_order_release);
    // Wake the engine thread: first the ALooper (so pollOnce returns quickly),
    // then the condition variable (in case the engine is blocked on state_cv_).
    wakeupLooper();
    {
        std::lock_guard<std::mutex> lk(state_mutex_);
        state_cv_.notify_all();
    }
}

void AndroidPlatformBackend::onNativeWindowDestroyed(ANativeWindow* /*window*/) {
    Nisaba_ALOG("onNativeWindowDestroyed (UI thread) — requesting surface destruction");
    {
        std::lock_guard<std::mutex> lk(window_event_mutex_);
        window_destroy_pending_ = true;
    }
    needs_processing_.store(true, std::memory_order_release);
    wakeupLooper();
    {
        std::lock_guard<std::mutex> lk(state_mutex_);
        state_cv_.notify_all();
    }

    // Synchronize: wait until engine thread has destroyed EGL surfaces
    // before allowing Android OS to free ANativeWindow.
    {
        std::unique_lock<std::mutex> lk(window_destroy_ack_mutex_);
        window_destroy_ack_cv_.wait_for(lk, std::chrono::milliseconds(500), [this]() {
            return !has_window_.load(std::memory_order_acquire);
        });
    }
    Nisaba_ALOG("onNativeWindowDestroyed (UI thread) — engine thread acknowledged release");
}

void AndroidPlatformBackend::onWindowFocusChanged(bool has_focus) {
    // Focus callbacks carry no EGL ops, so forward immediately.
    // Copy surface list under lock to avoid iterator invalidation.
    std::vector<AndroidSurface*> slist;
    {
        std::lock_guard<std::mutex> lk(surfaces_mutex_);
        slist.assign(surfaces_.begin(), surfaces_.end());
    }
    for (auto* s : slist) s->onWindowFocusChanged(has_focus);

    if (has_focus) {
        resolvePendingPermissions();
    }
}

void AndroidPlatformBackend::onPause() {
    Nisaba_ALOG("onPause");
    paused_.store(true, std::memory_order_release);
    // The engine thread will suspend naturally on the next pollEvents()
    // call once the surface is destroyed (onNativeWindowDestroyed follows).
}

void AndroidPlatformBackend::onResume() {
    Nisaba_ALOG("onResume");
    paused_.store(false, std::memory_order_release);
    insets_dirty_.store(true, std::memory_order_relaxed);
    wakeupLooper();
    {
        std::lock_guard<std::mutex> lk(state_mutex_);
        state_cv_.notify_all();
    }
    resolvePendingPermissions();
}

void AndroidPlatformBackend::onDestroy() {
    Nisaba_ALOG("onDestroy");
    quit_requested_.store(true, std::memory_order_release);
    needs_processing_.store(true, std::memory_order_release);
    // Wake the engine thread regardless of which wait it is in
    wakeupLooper();
    {
        std::lock_guard<std::mutex> lk(state_mutex_);
        state_cv_.notify_all();
    }
}

void AndroidPlatformBackend::onInputQueueCreated(AInputQueue* queue) {
    // Do NOT attach the queue to our worker ALooper:
    // onInputQueueCreated is called on the UI thread; the dispatcher needs to
    // receive acknowledgements there. Attaching to the worker looper breaks
    // this, causing an ANR after ~5 s.
    std::lock_guard<std::mutex> lk(input_queue_mutex_);
    input_queue_ = queue;
}

void AndroidPlatformBackend::onInputQueueDestroyed(AInputQueue* /*queue*/) {
    std::lock_guard<std::mutex> lk(input_queue_mutex_);
    input_queue_ = nullptr;
}

// ════════════════════════════════════════════════════════════════
// Surface registration
// ════════════════════════════════════════════════════════════════

void AndroidPlatformBackend::registerSurface(AndroidSurface* surface) {
    if (!surface) return;
    {
        std::lock_guard<std::mutex> lk(surfaces_mutex_);
        surfaces_.insert(surface);
    }
    // If a native window is already available (init() already ran and adopted
    // the boot window), notify the surface immediately — this runs on the
    // engine thread so EGL operations are safe.
    ANativeWindow* win = current_native_window_;
    if (win) {
        surface->onNativeWindowCreated(win);
        has_window_.store(true, std::memory_order_release);
    }
}

void AndroidPlatformBackend::unregisterSurface(AndroidSurface* surface) {
    std::lock_guard<std::mutex> lk(surfaces_mutex_);
    surfaces_.erase(surface);
}

// ════════════════════════════════════════════════════════════════
// Output / Monitor
// ════════════════════════════════════════════════════════════════

void AndroidPlatformBackend::updateOutputFromConfig() {
    if (!primary_output_) {
        primary_output_ = std::make_shared<AndroidOutput>();
    }

    // Density-independent pixels
    int32_t density = AConfiguration_getDensity(config_);
    if (density <= 0) density = ACONFIGURATION_DENSITY_MEDIUM; // 160 dpi fallback

    dpi_scale_ = static_cast<float>(density) / 160.0f;  // 160 dpi = 1.0x baseline

    primary_output_->fractional_scale_ = static_cast<double>(dpi_scale_);
    primary_output_->scale_factor_     = static_cast<int32_t>(std::round(dpi_scale_));

    // Invalidate safe-area cache: display geometry / density changed
    insets_dirty_.store(true, std::memory_order_relaxed);

    Nisaba_ALOG("Display density: %d dpi, scale: %.2f", density, dpi_scale_);
}

std::vector<std::shared_ptr<Output>> AndroidPlatformBackend::getOutputs() const {
    if (primary_output_) return { primary_output_ };
    return {};
}

std::shared_ptr<Output> AndroidPlatformBackend::getOutputByName(std::string_view name) const {
    if (primary_output_ && primary_output_->name() == name) return primary_output_;
    return nullptr;
}

std::shared_ptr<Output> AndroidPlatformBackend::getPrimaryOutput() const {
    return primary_output_;
}

// ════════════════════════════════════════════════════════════════
// Clipboard — JNI bridge
// ════════════════════════════════════════════════════════════════

std::string AndroidPlatformBackend::jniGetClipboardText() const {
    if (!activity_) return {};

    JNIEnv* env = nullptr;
    JavaVM* jvm = activity_->vm;
    bool    attached = false;

    int rc = jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (rc == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return {};
        attached = true;
    } else if (rc != JNI_OK || !env) {
        return {};
    }

    std::string result;

    // Get ClipboardManager via Context
    jclass ctx_class = env->GetObjectClass(activity_->clazz);
    jmethodID getSystemService = env->GetMethodID(ctx_class,
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");

    jstring svc_name = env->NewStringUTF("clipboard");
    jobject clip_mgr = env->CallObjectMethod(activity_->clazz, getSystemService, svc_name);
    env->DeleteLocalRef(svc_name);

    if (clip_mgr) {
        jclass cm_class = env->GetObjectClass(clip_mgr);
        jmethodID getText = env->GetMethodID(cm_class,
            "getText", "()Ljava/lang/CharSequence;");

        jobject charseq = env->CallObjectMethod(clip_mgr, getText);
        if (charseq) {
            jclass cs_class = env->GetObjectClass(charseq);
            jmethodID toStr = env->GetMethodID(cs_class, "toString",
                "()Ljava/lang/String;");
            jstring jstr = (jstring)env->CallObjectMethod(charseq, toStr);
            if (jstr) {
                const char* chars = env->GetStringUTFChars(jstr, nullptr);
                if (chars) {
                    result = chars;
                    env->ReleaseStringUTFChars(jstr, chars);
                }
                env->DeleteLocalRef(jstr);
            }
            env->DeleteLocalRef(charseq);
        }
        env->DeleteLocalRef(clip_mgr);
    }
    env->DeleteLocalRef(ctx_class);

    if (attached) jvm->DetachCurrentThread();
    return result;
}

void AndroidPlatformBackend::jniSetClipboardText(std::string_view text) const {
    if (!activity_) return;

    JNIEnv* env = nullptr;
    JavaVM* jvm = activity_->vm;
    bool    attached = false;

    int rc = jvm->GetEnv((void**)&env, JNI_VERSION_1_6);
    if (rc == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
        attached = true;
    } else if (rc != JNI_OK || !env) {
        return;
    }

    jclass ctx_class = env->GetObjectClass(activity_->clazz);
    jmethodID getSystemService = env->GetMethodID(ctx_class,
        "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");

    jstring svc_name = env->NewStringUTF("clipboard");
    jobject clip_mgr = env->CallObjectMethod(activity_->clazz, getSystemService, svc_name);
    env->DeleteLocalRef(svc_name);

    if (clip_mgr) {
        jclass cm_class = env->GetObjectClass(clip_mgr);
        jmethodID setText = env->GetMethodID(cm_class,
            "setText", "(Ljava/lang/CharSequence;)V");

        std::string s(text);
        jstring jstr = env->NewStringUTF(s.c_str());
        env->CallVoidMethod(clip_mgr, setText, jstr);
        env->DeleteLocalRef(jstr);
        env->DeleteLocalRef(clip_mgr);
    }
    env->DeleteLocalRef(ctx_class);

    if (attached) jvm->DetachCurrentThread();
}

void AndroidPlatformBackend::setClipboardText(std::string_view text, ClipboardType /*type*/) {
    ClipboardData cd;
    cd.setText(text);
    clipboard_buffer_ = cd;
    jniSetClipboardText(text);
}

void AndroidPlatformBackend::setClipboardData(const ClipboardData& data, ClipboardType /*type*/) {
    clipboard_buffer_ = data;
    jniSetClipboardText(data.getText());
}

std::string AndroidPlatformBackend::getClipboardText(ClipboardType /*type*/) const {
    std::string jni_text = jniGetClipboardText();
    return jni_text.empty() ? clipboard_buffer_.getText() : jni_text;
}

ClipboardData AndroidPlatformBackend::getClipboardData(ClipboardType type) const {
    ClipboardData cd;
    cd.setText(getClipboardText(type));
    return cd;
}

std::vector<uint8_t> AndroidPlatformBackend::getClipboardDataForMime(
    std::string_view mime_type, ClipboardType type) const
{
    if (mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain) {
        std::string text = getClipboardText(type);
        return { text.begin(), text.end() };
    }
    return {};
}

std::vector<std::string> AndroidPlatformBackend::getClipboardFormats(ClipboardType /*type*/) const {
    return { std::string(mime::TextPlainUtf8), std::string(mime::TextPlain) };
}

bool AndroidPlatformBackend::hasClipboardFormat(std::string_view mime_type, ClipboardType /*type*/) const {
    return mime_type == mime::TextPlainUtf8 || mime_type == mime::TextPlain;
}

// ════════════════════════════════════════════════════════════════
// Safe Area Insets
// Strategy 1 (preferred): Window.getDecorView().getRootWindowInsets()
//   getStableInsetTop/Bottom() — accounts for display cutouts + status/nav bars.
// Strategy 2 (fallback):  Resources.getSystem() dimension lookup.
// Strategy 3 (last resort): hardcoded dp constants.
// ════════════════════════════════════════════════════════════════

EdgeInsets AndroidPlatformBackend::getSafeAreaInsets() const {
    // ── Fast path: return cached value when the display config hasn't changed ───────
    // This avoids expensive JNI calls (AttachCurrentThread + 3+ method lookups)
    // on every frame that queries safe-area insets.
    if (!insets_dirty_.load(std::memory_order_acquire)) {
        std::lock_guard<std::mutex> lock(insets_mutex_);
        return cached_insets_;
    }

    constexpr float kFallbackTop    = 36.0f;  // status bar + notch ~36dp
    constexpr float kFallbackBottom = 24.0f;  // gesture nav bar

    // Helper: store result in cache and return it
    auto storeAndReturn = [this](EdgeInsets insets) -> EdgeInsets {
        std::lock_guard<std::mutex> lock(insets_mutex_);
        cached_insets_ = insets;
        insets_dirty_.store(false, std::memory_order_release);
        return insets;
    };

    if (!activity_) {
        return storeAndReturn(EdgeInsets::only(kFallbackTop, 0.0f, kFallbackBottom, 0.0f));
    }

    JavaVM* jvm = activity_->vm;
    if (!jvm) {
        return storeAndReturn(EdgeInsets::only(kFallbackTop, 0.0f, kFallbackBottom, 0.0f));
    }

    JNIEnv* env = nullptr;
    bool attached = false;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            return storeAndReturn(EdgeInsets::only(kFallbackTop, 0.0f, kFallbackBottom, 0.0f));
        }
        attached = true;
    }

    float top_dp    = kFallbackTop;
    float bottom_dp = kFallbackBottom;
    bool  got_insets = false;
    const float dpi = (dpi_scale_ > 0.1f) ? dpi_scale_ : 1.0f;

    // ── Strategy 1: WindowInsets from DecorView ────────────────────────────────
    // Uses activity.getWindow().getDecorView().getRootWindowInsets()
    // .getStableInsetTop() / .getStableInsetBottom()
    // These stable insets include the status bar, display cutout (notch/camera),
    // and navigation bar in pixels — the most reliable source on API 20+.
    do {
        jclass act_class = env->GetObjectClass(activity_->clazz);
        if (!act_class || env->ExceptionCheck()) { env->ExceptionClear(); break; }

        jmethodID getWindow = env->GetMethodID(act_class, "getWindow",
                                               "()Landroid/view/Window;");
        env->DeleteLocalRef(act_class);
        if (!getWindow || env->ExceptionCheck()) { env->ExceptionClear(); break; }

        jobject window = env->CallObjectMethod(activity_->clazz, getWindow);
        if (!window || env->ExceptionCheck()) { env->ExceptionClear(); break; }

        jclass win_class = env->GetObjectClass(window);
        jmethodID getDecorView = env->GetMethodID(win_class, "getDecorView",
                                                   "()Landroid/view/View;");
        env->DeleteLocalRef(win_class);
        if (!getDecorView || env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(window);
            break;
        }

        jobject decor = env->CallObjectMethod(window, getDecorView);
        env->DeleteLocalRef(window);
        if (!decor || env->ExceptionCheck()) { env->ExceptionClear(); break; }

        jclass view_class = env->GetObjectClass(decor);
        jmethodID getRootWI = env->GetMethodID(view_class, "getRootWindowInsets",
                                                "()Landroid/view/WindowInsets;");
        env->DeleteLocalRef(view_class);
        if (!getRootWI || env->ExceptionCheck()) {
            env->ExceptionClear();
            env->DeleteLocalRef(decor);
            break;
        }

        jobject wi = env->CallObjectMethod(decor, getRootWI);
        env->DeleteLocalRef(decor);
        if (!wi || env->ExceptionCheck()) { env->ExceptionClear(); break; }

        jclass wi_class      = env->GetObjectClass(wi);
        jmethodID stabTop    = env->GetMethodID(wi_class, "getStableInsetTop",    "()I");
        jmethodID stabBottom = env->GetMethodID(wi_class, "getStableInsetBottom", "()I");
        env->DeleteLocalRef(wi_class);

        if (stabTop && stabBottom && !env->ExceptionCheck()) {
            jint top_px    = env->CallIntMethod(wi, stabTop);
            jint bottom_px = env->CallIntMethod(wi, stabBottom);
            if (!env->ExceptionCheck()) {
                if (top_px    > 0) top_dp    = static_cast<float>(top_px)    / dpi;
                if (bottom_px > 0) bottom_dp = static_cast<float>(bottom_px) / dpi;
                got_insets = true;
            } else {
                env->ExceptionClear();
            }
        } else {
            env->ExceptionClear();
        }
        env->DeleteLocalRef(wi);
    } while (false);

    // ── Strategy 2: Resources.getSystem() dimension lookup ─────────────────────
    if (!got_insets) {
        auto queryDimenPx = [&](const char* name) -> int {
            jclass res_class = env->FindClass("android/content/res/Resources");
            if (!res_class) return -1;
            jmethodID getRes = env->GetStaticMethodID(res_class,
                "getSystem", "()Landroid/content/res/Resources;");
            if (!getRes) { env->DeleteLocalRef(res_class); return -1; }
            jobject resources = env->CallStaticObjectMethod(res_class, getRes);
            env->DeleteLocalRef(res_class);
            if (!resources) return -1;

            jclass robj_class = env->GetObjectClass(resources);
            jmethodID getIdent = env->GetMethodID(robj_class,
                "getIdentifier",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)I");
            jmethodID getDimen = env->GetMethodID(robj_class,
                "getDimensionPixelSize", "(I)I");
            env->DeleteLocalRef(robj_class);
            if (!getIdent || !getDimen) { env->DeleteLocalRef(resources); return -1; }

            jstring jName    = env->NewStringUTF(name);
            jstring jDimen   = env->NewStringUTF("dimen");
            jstring jAndroid = env->NewStringUTF("android");
            jint resId = env->CallIntMethod(resources, getIdent, jName, jDimen, jAndroid);
            env->DeleteLocalRef(jName);
            env->DeleteLocalRef(jDimen);
            env->DeleteLocalRef(jAndroid);

            int px = -1;
            if (resId > 0) {
                px = (int)env->CallIntMethod(resources, getDimen, resId);
                if (env->ExceptionCheck()) { env->ExceptionClear(); px = -1; }
            }
            env->DeleteLocalRef(resources);
            return px;
        };

        int status_px = queryDimenPx("status_bar_height");
        if (status_px > 0) top_dp = static_cast<float>(status_px) / dpi;

        int nav_px = queryDimenPx("navigation_bar_height");
        if (nav_px > 0) bottom_dp = static_cast<float>(nav_px) / dpi;
    }

    if (attached) jvm->DetachCurrentThread();

    Nisaba_ALOG("SafeArea insets: top=%.1fdp bottom=%.1fdp (via %s)",
              top_dp, bottom_dp, got_insets ? "WindowInsets" : "Resources");
    return storeAndReturn(EdgeInsets::only(top_dp, 0.0f, bottom_dp, 0.0f));
}

// ════════════════════════════════════════════════════════════════
// Runtime Permissions Subsystem (JNI)
// ════════════════════════════════════════════════════════════════

int AndroidPlatformBackend::jniGetSdkVersion() const {
    if (cached_sdk_version_ > 0) return cached_sdk_version_;
    if (!activity_ || !activity_->vm) return 26;

    JavaVM* jvm = activity_->vm;
    JNIEnv* env = nullptr;
    bool attached = false;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return 26;
        attached = true;
    }

    int sdk = 26;
    jclass version_class = env->FindClass("android/os/Build$VERSION");
    if (version_class) {
        jfieldID sdk_int_fid = env->GetStaticFieldID(version_class, "SDK_INT", "I");
        if (sdk_int_fid) {
            sdk = env->GetStaticIntField(version_class, sdk_int_fid);
        }
        env->DeleteLocalRef(version_class);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (attached) jvm->DetachCurrentThread();

    cached_sdk_version_ = sdk;
    return sdk;
}

int AndroidPlatformBackend::jniCheckPermission(std::string_view permission) const {
    if (!activity_ || !activity_->vm || !activity_->clazz) return -1;

    JavaVM* jvm = activity_->vm;
    JNIEnv* env = nullptr;
    bool attached = false;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return -1;
        attached = true;
    }

    int result = -1;
    jclass act_class = env->GetObjectClass(activity_->clazz);
    if (act_class) {
        jmethodID check_perm = env->GetMethodID(act_class, "checkSelfPermission", "(Ljava/lang/String;)I");
        if (check_perm) {
            std::string perm_str(permission);
            jstring jperm = env->NewStringUTF(perm_str.c_str());
            result = env->CallIntMethod(activity_->clazz, check_perm, jperm);
            env->DeleteLocalRef(jperm);
        }
        env->DeleteLocalRef(act_class);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (attached) jvm->DetachCurrentThread();

    return result;
}

bool AndroidPlatformBackend::jniShouldShowRationale(std::string_view permission) const {
    if (!activity_ || !activity_->vm || !activity_->clazz) return false;

    JavaVM* jvm = activity_->vm;
    JNIEnv* env = nullptr;
    bool attached = false;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return false;
        attached = true;
    }

    jboolean should_show = JNI_FALSE;
    jclass act_class = env->GetObjectClass(activity_->clazz);
    if (act_class) {
        jmethodID rationale_method = env->GetMethodID(act_class, "shouldShowRequestPermissionRationale", "(Ljava/lang/String;)Z");
        if (rationale_method) {
            std::string perm_str(permission);
            jstring jperm = env->NewStringUTF(perm_str.c_str());
            should_show = env->CallBooleanMethod(activity_->clazz, rationale_method, jperm);
            env->DeleteLocalRef(jperm);
        }
        env->DeleteLocalRef(act_class);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (attached) jvm->DetachCurrentThread();

    return should_show == JNI_TRUE;
}

void AndroidPlatformBackend::jniRequestPermissions(const std::vector<std::string>& permissions, int request_code) {
    if (!activity_ || !activity_->vm || !activity_->clazz || permissions.empty()) return;

    JavaVM* jvm = activity_->vm;
    JNIEnv* env = nullptr;
    bool attached = false;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return;
        attached = true;
    }

    jclass act_class = env->GetObjectClass(activity_->clazz);
    if (act_class) {
        jmethodID req_method = env->GetMethodID(act_class, "requestPermissions", "([Ljava/lang/String;I)V");
        if (req_method) {
            jclass str_class = env->FindClass("java/lang/String");
            if (str_class) {
                jobjectArray perm_array = env->NewObjectArray(permissions.size(), str_class, nullptr);
                for (size_t i = 0; i < permissions.size(); ++i) {
                    jstring s = env->NewStringUTF(permissions[i].c_str());
                    env->SetObjectArrayElement(perm_array, i, s);
                    env->DeleteLocalRef(s);
                }
                env->CallVoidMethod(activity_->clazz, req_method, perm_array, (jint)request_code);
                env->DeleteLocalRef(perm_array);
                env->DeleteLocalRef(str_class);
            }
        }
        env->DeleteLocalRef(act_class);
    }
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (attached) jvm->DetachCurrentThread();
}

bool AndroidPlatformBackend::jniOpenAppSettings() {
    if (!activity_ || !activity_->vm || !activity_->clazz) return false;

    JavaVM* jvm = activity_->vm;
    JNIEnv* env = nullptr;
    bool attached = false;
    if (jvm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_EDETACHED) {
        if (jvm->AttachCurrentThread(&env, nullptr) != JNI_OK) return false;
        attached = true;
    }

    bool success = false;
    do {
        jclass act_class = env->GetObjectClass(activity_->clazz);
        if (!act_class) break;

        jmethodID get_pkg = env->GetMethodID(act_class, "getPackageName", "()Ljava/lang/String;");
        if (!get_pkg) { env->DeleteLocalRef(act_class); break; }

        jstring pkg_name = (jstring)env->CallObjectMethod(activity_->clazz, get_pkg);
        if (!pkg_name) { env->DeleteLocalRef(act_class); break; }

        jclass intent_class = env->FindClass("android/content/Intent");
        if (!intent_class) {
            env->DeleteLocalRef(pkg_name);
            env->DeleteLocalRef(act_class);
            break;
        }

        jmethodID intent_init = env->GetMethodID(intent_class, "<init>", "(Ljava/lang/String;)V");
        if (!intent_init) {
            env->DeleteLocalRef(intent_class);
            env->DeleteLocalRef(pkg_name);
            env->DeleteLocalRef(act_class);
            break;
        }

        jstring action = env->NewStringUTF("android.settings.APPLICATION_DETAILS_SETTINGS");
        jobject intent = env->NewObject(intent_class, intent_init, action);
        env->DeleteLocalRef(action);
        if (!intent) {
            env->DeleteLocalRef(intent_class);
            env->DeleteLocalRef(pkg_name);
            env->DeleteLocalRef(act_class);
            break;
        }

        jclass uri_class = env->FindClass("android/net/Uri");
        if (uri_class) {
            jmethodID from_parts = env->GetStaticMethodID(uri_class, "fromParts",
                "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Landroid/net/Uri;");
            if (from_parts) {
                jstring scheme = env->NewStringUTF("package");
                jobject uri = env->CallStaticObjectMethod(uri_class, from_parts, scheme, pkg_name, nullptr);
                env->DeleteLocalRef(scheme);

                if (uri) {
                    jmethodID set_data = env->GetMethodID(intent_class, "setData",
                        "(Landroid/net/Uri;)Landroid/content/Intent;");
                    if (set_data) {
                        env->CallObjectMethod(intent, set_data, uri);
                    }
                    env->DeleteLocalRef(uri);
                }
            }
            env->DeleteLocalRef(uri_class);
        }

        jmethodID start_act = env->GetMethodID(act_class, "startActivity", "(Landroid/content/Intent;)V");
        if (start_act) {
            env->CallVoidMethod(activity_->clazz, start_act, intent);
            if (!env->ExceptionCheck()) {
                success = true;
            }
        }

        env->DeleteLocalRef(intent);
        env->DeleteLocalRef(intent_class);
        env->DeleteLocalRef(pkg_name);
        env->DeleteLocalRef(act_class);
    } while (false);

    if (env->ExceptionCheck()) env->ExceptionClear();
    if (attached) jvm->DetachCurrentThread();

    return success;
}

PermissionStatus AndroidPlatformBackend::checkPermission(std::string_view permission) {
    int res = jniCheckPermission(permission);
    if (res == 0) {
        return PermissionStatus::Granted;
    }
    return PermissionStatus::Denied;
}

bool AndroidPlatformBackend::shouldShowRationale(std::string_view permission) {
    return jniShouldShowRationale(permission);
}

int AndroidPlatformBackend::getAndroidSdkVersion() const {
    return jniGetSdkVersion();
}

bool AndroidPlatformBackend::openAppSettings() {
    return jniOpenAppSettings();
}

void AndroidPlatformBackend::requestPermissions(
    const std::vector<std::string>& permissions,
    std::function<void(const std::unordered_map<std::string, PermissionStatus>&)> callback)
{
    if (permissions.empty()) {
        if (callback) callback({});
        return;
    }

    bool all_granted = true;
    std::unordered_map<std::string, PermissionStatus> results;
    std::vector<std::string> to_request;
    std::vector<bool> pre_rationales;

    for (const auto& perm : permissions) {
        if (jniCheckPermission(perm) == 0) {
            results[perm] = PermissionStatus::Granted;
        } else {
            all_granted = false;
            to_request.push_back(perm);
            pre_rationales.push_back(jniShouldShowRationale(perm));
        }
    }

    if (all_granted) {
        if (callback) callback(results);
        return;
    }

    int code = next_permission_request_code_.fetch_add(1);
    PendingPermissionRequest req;
    req.request_code = code;
    req.permissions = permissions;
    req.callback = std::move(callback);
    req.pre_rationales = std::move(pre_rationales);

    {
        std::lock_guard<std::mutex> lk(permissions_mutex_);
        pending_permission_requests_.push_back(std::move(req));
    }

    Nisaba_ALOG("Requesting %zu Android permissions (request_code=%d)", to_request.size(), code);
    jniRequestPermissions(to_request, code);
}

void AndroidPlatformBackend::resolvePendingPermissions() {
    std::vector<PendingPermissionRequest> reqs;
    {
        std::lock_guard<std::mutex> lk(permissions_mutex_);
        if (pending_permission_requests_.empty()) return;
        reqs = std::move(pending_permission_requests_);
        pending_permission_requests_.clear();
    }

    Nisaba_ALOG("Resolving %zu pending permission request(s)", reqs.size());

    for (auto& req : reqs) {
        std::unordered_map<std::string, PermissionStatus> result;
        for (size_t i = 0; i < req.permissions.size(); ++i) {
            const auto& perm = req.permissions[i];
            if (jniCheckPermission(perm) == 0) {
                result[perm] = PermissionStatus::Granted;
            } else {
                bool rationale = jniShouldShowRationale(perm);
                bool had_rationale_before = (i < req.pre_rationales.size()) ? req.pre_rationales[i] : false;
                if (!rationale && had_rationale_before) {
                    result[perm] = PermissionStatus::PermanentlyDenied;
                } else {
                    result[perm] = PermissionStatus::Denied;
                }
            }
        }
        if (req.callback) {
            req.callback(result);
        }
    }
}

void AndroidPlatformBackend::onNativePermissionsResult(
    int request_code,
    const std::vector<std::string>& permissions,
    const std::vector<int>& grant_results)
{
    PendingPermissionRequest matched_req;
    bool found = false;

    {
        std::lock_guard<std::mutex> lk(permissions_mutex_);
        for (auto it = pending_permission_requests_.begin(); it != pending_permission_requests_.end(); ++it) {
            if (it->request_code == request_code) {
                matched_req = std::move(*it);
                pending_permission_requests_.erase(it);
                found = true;
                break;
            }
        }
    }

    if (!found) return;

    std::unordered_map<std::string, PermissionStatus> result;
    for (size_t i = 0; i < permissions.size(); ++i) {
        const auto& perm = permissions[i];
        int code = (i < grant_results.size()) ? grant_results[i] : -1;
        if (code == 0) {
            result[perm] = PermissionStatus::Granted;
        } else {
            bool rationale = jniShouldShowRationale(perm);
            result[perm] = rationale ? PermissionStatus::Denied : PermissionStatus::PermanentlyDenied;
        }
    }

    if (matched_req.callback) {
        matched_req.callback(result);
    }
}

} // namespace nisaba::backend_os::android

extern "C" JNIEXPORT void JNICALL
Java_nisaba_NisabaNativeActivity_onPermissionsResult(
    JNIEnv* env, jobject /*thiz*/, jint request_code,
    jobjectArray jpermissions, jintArray jgrant_results)
{
    if (!nisaba::backend_os::android::s_active_backend || !env || !jpermissions || !jgrant_results) return;

    jsize num_perms = env->GetArrayLength(jpermissions);
    jsize num_results = env->GetArrayLength(jgrant_results);
    jint* results_ptr = env->GetIntArrayElements(jgrant_results, nullptr);

    std::vector<std::string> perms;
    std::vector<int> grants;
    perms.reserve(num_perms);
    grants.reserve(num_results);

    for (jsize i = 0; i < num_perms; ++i) {
        auto jstr = (jstring)env->GetObjectArrayElement(jpermissions, i);
        if (jstr) {
            const char* utf = env->GetStringUTFChars(jstr, nullptr);
            if (utf) {
                perms.emplace_back(utf);
                env->ReleaseStringUTFChars(jstr, utf);
            }
            env->DeleteLocalRef(jstr);
        }
    }

    if (results_ptr) {
        for (jsize i = 0; i < num_results; ++i) {
            grants.push_back(results_ptr[i]);
        }
        env->ReleaseIntArrayElements(jgrant_results, results_ptr, JNI_ABORT);
    }

    nisaba::backend_os::android::s_active_backend->onNativePermissionsResult(request_code, perms, grants);
}

#endif // __ANDROID__

#include "nisaba/compositor/renderer.hpp"
#include "nisaba/compositor/gbm_device.hpp"
#include "nisaba/compositor/log.hpp"

namespace nisaba::compositor {

struct Renderer::Impl {
    DrmFormatSet formats;
    bool inFrame{false};

    Impl() {
        formats.add(DRM_FORMAT_XRGB8888, DRM_FORMAT_MOD_LINEAR);
        formats.add(DRM_FORMAT_ARGB8888, DRM_FORMAT_MOD_LINEAR);
        formats.add(DRM_FORMAT_XBGR8888, DRM_FORMAT_MOD_LINEAR);
        formats.add(DRM_FORMAT_ABGR8888, DRM_FORMAT_MOD_LINEAR);
    }
};

std::shared_ptr<Renderer> Renderer::create(std::shared_ptr<EglContext> egl) {
    if (!egl) {
        egl = EglContext::createAuto();
    }
    if (!egl) {
        logError("Failed to initialize hardware EGL context for Renderer");
        return nullptr;
    }

    if (!egl->makeCurrent()) {
        logError("Failed to make EGL context current for Renderer initialization");
        return nullptr;
    }

    auto shaders = ShaderManager::create(true);
    if (!shaders) {
        logError("Failed to compile ShaderManager for Renderer");
        return nullptr;
    }

    auto blur = DualKawaseBlur::create(egl);
    if (!blur) {
        logWarn("DualKawaseBlur could not be initialized for Renderer (will proceed without blur)");
    }

    return std::make_shared<Renderer>(std::move(egl), std::move(shaders), std::move(blur));
}

Renderer::Renderer(
    std::shared_ptr<EglContext> egl,
    std::shared_ptr<ShaderManager> shaders,
    std::shared_ptr<DualKawaseBlur> blur)
    : m_impl(std::make_unique<Impl>()),
      m_egl(std::move(egl)),
      m_shaders(std::move(shaders)),
      m_blur(std::move(blur)) {}

Renderer::~Renderer() = default;

std::string_view Renderer::name() const noexcept {
    return "Nisaba-Sovereign-GPU-Renderer";
}

const DrmFormatSet& Renderer::supportedFormats() const noexcept {
    return m_impl->formats;
}

std::shared_ptr<IBuffer> Renderer::importBuffer(const DmaBufAttributes& attrs) {
    if (!m_egl) return nullptr;

    auto img = EglImage::createFromDmaBuf(m_egl, attrs);
    if (!img) {
        logError("Renderer failed to import DMA-BUF buffer");
        return nullptr;
    }

    return nullptr;
}

bool Renderer::beginFrame(IBuffer& /*target*/, const RectI& /*damage*/) {
    m_impl->inFrame = true;
    return true;
}

bool Renderer::endFrame() {
    if (!m_impl->inFrame) return false;
    m_impl->inFrame = false;
    return true;
}

std::unique_ptr<RenderPass> Renderer::beginPass(
    std::shared_ptr<RenderTarget> target,
    wl_output_transform outputTransform) {
    if (!m_egl || !m_shaders || !target) return nullptr;
    if (eglGetCurrentContext() != m_egl->context()) {
        m_egl->makeCurrent();
    }
    return RenderPass::begin(std::move(target), m_shaders, outputTransform);
}

std::unique_ptr<IRenderPass> Renderer::beginPass(
    std::shared_ptr<IRenderTarget> target,
    wl_output_transform outputTransform) {
    auto glTarget = std::dynamic_pointer_cast<RenderTarget>(target);
    if (!glTarget) return nullptr;
    return beginPass(std::move(glTarget), outputTransform);
}

// ==========================================
// Allocator Implementation
// ==========================================

std::shared_ptr<Allocator> Allocator::create(std::shared_ptr<GbmAllocator> gbmAlloc) {
    if (!gbmAlloc) {
        auto dev = GbmDevice::createAuto();
        if (!dev) {
            logError("Allocator failed to open DRM device for GBM");
            return nullptr;
        }
        gbmAlloc = GbmAllocator::create(dev);
        if (!gbmAlloc) {
            logError("Allocator failed to create GbmAllocator");
            return nullptr;
        }
    }
    return std::make_shared<Allocator>(std::move(gbmAlloc));
}

Allocator::Allocator(std::shared_ptr<GbmAllocator> gbmAlloc)
    : m_gbmAlloc(std::move(gbmAlloc)) {}

Allocator::~Allocator() = default;

std::string_view Allocator::name() const noexcept {
    return "Nisaba-Sovereign-GBM-Allocator";
}

std::shared_ptr<IBuffer> Allocator::allocate(
    uint32_t width,
    uint32_t height,
    uint32_t format,
    uint64_t modifier) {
    if (!m_gbmAlloc) return nullptr;
    return m_gbmAlloc->allocateBuffer(width, height, format, modifier);
}

} // namespace nisaba::compositor

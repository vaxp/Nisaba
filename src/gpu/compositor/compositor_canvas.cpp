#include "nisaba/compositor/compositor_canvas.hpp"
#include "nisaba/compositor/log.hpp"

namespace nisaba::compositor {

std::unique_ptr<CompositorCanvas> CompositorCanvas::create(
    std::shared_ptr<EglContext> egl,
    std::shared_ptr<Renderer> renderer) {
    if (!renderer) {
        renderer = Renderer::create(egl);
    }
    if (!renderer) {
        logError("CompositorCanvas::create: Failed to create Renderer");
        return nullptr;
    }

    if (!egl) {
        egl = renderer->egl();
    }

    return std::make_unique<CompositorCanvas>(std::move(egl), std::move(renderer));
}

CompositorCanvas::CompositorCanvas(
    std::shared_ptr<EglContext> egl,
    std::shared_ptr<Renderer> renderer)
    : m_egl(std::move(egl)), m_renderer(std::move(renderer)) {}

CompositorCanvas::~CompositorCanvas() = default;

bool CompositorCanvas::beginFrame(
    std::shared_ptr<RenderTarget> target,
    wl_output_transform transform) {
    if (!m_renderer || !target) return false;

    m_currentDamage = nullptr;
    m_currentPass = m_renderer->beginPass(std::move(target), transform);
    return m_currentPass != nullptr;
}

void CompositorCanvas::setDamage(const Region* damage) {
    m_currentDamage = damage;
}

void CompositorCanvas::clear(const Color& color) {
    if (m_currentPass) {
        m_currentPass->clear(color);
    }
}

void CompositorCanvas::drawWindowSurface(
    const Texture& texture,
    const RectI& destBox,
    const RectF& srcBox,
    float alpha,
    wl_output_transform transform) {
    if (m_currentPass) {
        m_currentPass->renderTexture(texture, destBox, srcBox, alpha, transform, m_currentDamage);
    }
}

void CompositorCanvas::drawRect(const RectI& box, const Color& color) {
    if (m_currentPass) {
        m_currentPass->renderRect(box, color, m_currentDamage);
    }
}

void CompositorCanvas::drawFrostedGlass(
    const Texture& source,
    const RectI& destBox,
    const RectI& srcBox,
    const DualKawaseConfig& config,
    const Texture* maskTexture) {
    if (m_currentPass && m_renderer && m_renderer->blur()) {
        m_currentPass->renderBlur(*m_renderer->blur(), source, destBox, srcBox, config, maskTexture);
    }
}

void CompositorCanvas::drawScene(const SceneTree& tree) {
    if (m_currentPass) {
        tree.render(*m_currentPass, PointI{0, 0}, m_currentDamage);
    }
}

bool CompositorCanvas::endFrame() {
    if (!m_currentPass) return false;

    bool ok = m_currentPass->submit();
    m_currentPass.reset();
    m_currentDamage = nullptr;
    return ok;
}

} // namespace nisaba::compositor

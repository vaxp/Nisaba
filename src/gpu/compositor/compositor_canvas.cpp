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
    m_currentTarget = target;
    m_currentPass = m_renderer->beginPass(target, transform);
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

gpu::GpuCanvas* CompositorCanvas::vg() {
    if (!m_currentTarget) return nullptr;

    if (!m_gpuDevice) {
        m_gpuDevice = gpu::GpuDevice::create();
    }
    if (!m_gpuDevice) return nullptr;

    uint32_t w = static_cast<uint32_t>(m_currentTarget->width());
    uint32_t h = static_cast<uint32_t>(m_currentTarget->height());
    uint32_t fbo = m_currentTarget->fbo();

    if (!m_gpuSurface || m_gpuSurface->width() != w || m_gpuSurface->height() != h || m_gpuSurface->fbo_id() != fbo) {
        m_gpuCanvas.reset();
        m_gpuSurface = gpu::GpuSurface::from_render_target(m_gpuDevice, w, h, fbo);
        if (m_gpuSurface) {
            m_gpuCanvas = std::make_unique<gpu::GpuCanvas>(m_gpuSurface);
            if (m_gpuCanvas && m_gpuCanvas->context()) {
                const char* sansPaths[] = {
                    "fonts/Inter-Regular.ttf", "../fonts/Inter-Regular.ttf",
                    "fonts/NotoSans-Regular.ttf", "../fonts/NotoSans-Regular.ttf"
                };
                for (const auto* p : sansPaths) {
                    if (m_gpuCanvas->context()->createFont("sans", p) != -1) break;
                }
                const char* monoPaths[] = {
                    "fonts/FiraMono-Medium.ttf", "../fonts/FiraMono-Medium.ttf"
                };
                for (const auto* p : monoPaths) {
                    if (m_gpuCanvas->context()->createFont("mono", p) != -1) break;
                }
            }
        }
    }

    if (m_gpuCanvas) {
        if (m_currentDamage) {
            auto ext = m_currentDamage->extents();
            auto clipRect = nisaba::Rect::from_xywh(
                static_cast<float>(ext.x),
                static_cast<float>(ext.y),
                static_cast<float>(ext.width),
                static_cast<float>(ext.height)
            );
            if (clipRect) {
                m_gpuCanvas->clip_rect(*clipRect);
            }
        } else {
            m_gpuCanvas->reset_clip();
        }
    }

    return m_gpuCanvas.get();
}

void CompositorCanvas::drawWindowDecoration(
    const RectI& clientBox,
    std::string_view title) {
    drawWindowDecoration(clientBox, WindowDecoStyle{}, title);
}

void CompositorCanvas::drawWindowDecoration(
    const RectI& clientBox,
    const WindowDecoStyle& style,
    std::string_view title) {
    RectI fullBox{
        clientBox.x,
        clientBox.y - static_cast<int>(style.titleBarHeight),
        clientBox.width,
        clientBox.height + static_cast<int>(style.titleBarHeight)
    };

    if (m_currentDamage && !m_currentDamage->intersects(fullBox)) {
        return; // Early discard via O(1) damage culling
    }

    GlStateGuard guard;
    auto* canvas = vg();
    if (!canvas) return;

    auto fullRect = nisaba::Rect::from_xywh(
        static_cast<float>(fullBox.x),
        static_cast<float>(fullBox.y),
        static_cast<float>(fullBox.width),
        static_cast<float>(fullBox.height)
    );
    if (!fullRect) return;

    // 1. Analytical Gaussian Drop Shadow
    canvas->draw_round_rect_shadow(*fullRect, style.cornerRadius, style.cornerRadius, style.shadow);

    // 2. Window Body fill
    nisaba::Paint bodyPaint(nisaba::Color::from_rgba_unchecked(
        style.bodyColor.r, style.bodyColor.g, style.bodyColor.b, style.bodyColor.a
    ));
    canvas->fill_round_rect(*fullRect, style.cornerRadius, style.cornerRadius, bodyPaint);

    // 3. Titlebar background
    auto titleRect = nisaba::Rect::from_xywh(
        static_cast<float>(fullBox.x),
        static_cast<float>(fullBox.y),
        static_cast<float>(fullBox.width),
        style.titleBarHeight
    );
    if (titleRect) {
        canvas->save();
        canvas->clip_rect(*titleRect);
        nisaba::Paint titlePaint(nisaba::Color::from_rgba_unchecked(
            style.titleBarColor.r, style.titleBarColor.g, style.titleBarColor.b, style.titleBarColor.a
        ));
        canvas->fill_round_rect(*nisaba::Rect::from_xywh(
            static_cast<float>(fullBox.x),
            static_cast<float>(fullBox.y),
            static_cast<float>(fullBox.width),
            style.titleBarHeight + style.cornerRadius
        ), style.cornerRadius, style.cornerRadius, titlePaint);
        canvas->restore();
    }

    // 4. Subtle Specular Border
    if (style.borderWidth > 0.0f) {
        nisaba::Stroke st;
        st.width = style.borderWidth;
        nisaba::Paint borderPaint(nisaba::Color::from_rgba_unchecked(
            style.borderColor.r, style.borderColor.g, style.borderColor.b, style.borderColor.a
        ));
        canvas->stroke_round_rect(*fullRect, style.cornerRadius, style.cornerRadius, borderPaint, st);
    }

    // 5. Title text
    if (!title.empty()) {
        float textX = static_cast<float>(fullBox.x) + (style.showButtons ? 72.0f : 16.0f);
        float textY = static_cast<float>(fullBox.y) + style.titleBarHeight * 0.70f;
        nisaba::Paint textPaint(nisaba::Color::from_rgba_unchecked(
            style.titleTextColor.r, style.titleTextColor.g, style.titleTextColor.b, style.titleTextColor.a
        ));
        canvas->draw_text(textX, textY, title, textPaint, style.titleFontSize, "sans");
    }

    // 6. Window control buttons (macOS / Modern Linux style)
    if (style.showButtons) {
        float btnY = static_cast<float>(fullBox.y) + style.titleBarHeight * 0.5f;
        float btnRadius = 5.5f;
        float startBtnX = static_cast<float>(fullBox.x) + 16.0f;

        // Close button
        canvas->fill_circle(startBtnX, btnY, btnRadius,
            nisaba::Paint(nisaba::Color::from_rgba_unchecked(
                style.closeButtonColor.r, style.closeButtonColor.g, style.closeButtonColor.b, style.closeButtonColor.a)));

        // Minimize button
        canvas->fill_circle(startBtnX + 16.0f, btnY, btnRadius,
            nisaba::Paint(nisaba::Color::from_rgba_unchecked(
                style.minButtonColor.r, style.minButtonColor.g, style.minButtonColor.b, style.minButtonColor.a)));

        // Maximize button
        canvas->fill_circle(startBtnX + 32.0f, btnY, btnRadius,
            nisaba::Paint(nisaba::Color::from_rgba_unchecked(
                style.maxButtonColor.r, style.maxButtonColor.g, style.maxButtonColor.b, style.maxButtonColor.a)));
    }
}

void CompositorCanvas::drawDesktopPanel(
    const RectI& panelBounds,
    const effects::GlassParams& glassParams,
    const std::vector<const svg::SvgDocument*>& launcherIcons) {
    if (m_currentDamage && !m_currentDamage->intersects(panelBounds)) return;

    GlStateGuard guard;
    auto* canvas = vg();
    if (!canvas) return;

    auto rect = nisaba::Rect::from_xywh(
        static_cast<float>(panelBounds.x),
        static_cast<float>(panelBounds.y),
        static_cast<float>(panelBounds.width),
        static_cast<float>(panelBounds.height)
    );
    if (!rect) return;

    canvas->draw_glass_panel(*rect, 16.0f, 16.0f, glassParams);

    if (!launcherIcons.empty()) {
        float iconSize = std::min(static_cast<float>(panelBounds.height) * 0.65f, 44.0f);
        float spacing = 16.0f;
        float totalW = static_cast<float>(launcherIcons.size()) * iconSize + static_cast<float>(launcherIcons.size() - 1) * spacing;
        float startX = static_cast<float>(panelBounds.x) + (static_cast<float>(panelBounds.width) - totalW) * 0.5f;
        float startY = static_cast<float>(panelBounds.y) + (static_cast<float>(panelBounds.height) - iconSize) * 0.5f;

        for (size_t i = 0; i < launcherIcons.size(); ++i) {
            if (launcherIcons[i]) {
                auto iconRect = nisaba::Rect::from_xywh(startX + static_cast<float>(i) * (iconSize + spacing), startY, iconSize, iconSize);
                if (iconRect) {
                    canvas->draw_svg(*launcherIcons[i], *iconRect);
                }
            }
        }
    }
}

void CompositorCanvas::applyWindowShader(
    const Texture& windowTexture,
    const RectI& destBox,
    std::shared_ptr<gpu::RuntimeEffect> effect,
    gpu::RuntimeShaderBuilder uniforms) {
    if (destBox.isEmpty() || !windowTexture.id() || !effect) return;
    if (m_currentDamage && !m_currentDamage->intersects(destBox)) return;

    GlStateGuard guard;
    auto* canvas = vg();
    if (!canvas) return;

    auto rect = nisaba::Rect::from_xywh(
        static_cast<float>(destBox.x),
        static_cast<float>(destBox.y),
        static_cast<float>(destBox.width),
        static_cast<float>(destBox.height)
    );
    if (!rect) return;

    auto customShader = uniforms.makeShader(static_cast<const nisaba::Transform*>(nullptr));
    if (!customShader) return;

    nisaba::Paint p;
    p.custom_shader = customShader;
    canvas->fill_rect(*rect, p);
}

void CompositorCanvas::drawText(
    std::string_view text,
    float x,
    float y,
    const Color& color,
    float font_size,
    const char* font_name) {
    if (text.empty()) return;

    GlStateGuard guard;
    auto* canvas = vg();
    if (!canvas) return;

    nisaba::Paint p(nisaba::Color::from_rgba_unchecked(color.r, color.g, color.b, color.a));
    canvas->draw_text(x, y, text, p, font_size, font_name ? font_name : "sans");
}

void CompositorCanvas::drawSvg(
    const svg::SvgDocument& doc,
    const RectI& destBounds) {
    if (m_currentDamage && !m_currentDamage->intersects(destBounds)) return;

    GlStateGuard guard;
    auto* canvas = vg();
    if (!canvas) return;

    auto rect = nisaba::Rect::from_xywh(
        static_cast<float>(destBounds.x),
        static_cast<float>(destBounds.y),
        static_cast<float>(destBounds.width),
        static_cast<float>(destBounds.height)
    );
    if (rect) {
        canvas->draw_svg(doc, *rect);
    }
}

bool CompositorCanvas::endFrame() {
    if (m_gpuCanvas) {
        GlStateGuard guard;
        m_gpuCanvas->flush();
    }

    if (!m_currentPass) return false;

    bool ok = m_currentPass->submit();
    m_currentPass.reset();
    m_currentDamage = nullptr;
    m_currentTarget = nullptr;
    return ok;
}

} // namespace nisaba::compositor

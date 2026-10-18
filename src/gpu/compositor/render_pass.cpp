#include "nisaba/compositor/render_pass.hpp"
#include "nisaba/compositor/log.hpp"

#include <vector>

namespace nisaba::compositor {

std::unique_ptr<RenderPass> RenderPass::begin(
    std::shared_ptr<RenderTarget> target,
    std::shared_ptr<ShaderManager> shaders,
    wl_output_transform outputTransform) {
    if (!target || !shaders) {
        logError("RenderPass::begin failed: target or shaders is null");
        return nullptr;
    }

    return std::make_unique<RenderPass>(target, shaders, outputTransform);
}

RenderPass::RenderPass(
    std::shared_ptr<RenderTarget> target,
    std::shared_ptr<ShaderManager> shaders,
    wl_output_transform outputTransform)
    : m_target(std::move(target)),
      m_shaders(std::move(shaders)) {
    m_target->bind();
    m_projection = Matrix3::project(m_target->width(), m_target->height(), outputTransform);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_SCISSOR_TEST);
}

RenderPass::~RenderPass() {
    if (!m_submitted) {
        submit();
    }
}

void RenderPass::clear(const Color& color) const noexcept {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void RenderPass::renderRect(
    const RectI& box,
    const Color& color,
    const Region* clip) const {
    if (box.isEmpty()) return;

    if (color.a >= 0.999f) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    const auto& shader = m_shaders->solidQuad();
    shader.use();

    Matrix3 mvp = Matrix3::projectBox(box, WL_OUTPUT_TRANSFORM_NORMAL, m_projection);
    shader.setProjection(mvp);
    shader.setColor(color.premultiplied());

    renderQuads(box, clip, shader.posAttrib());
}

void RenderPass::renderTexture(
    const ITexture& texture,
    const RectI& destBox,
    const RectF& srcBox,
    float alpha,
    wl_output_transform transform,
    const Region* clip) const {
    if (texture.backendType() == RenderBackendType::OpenGL) {
        renderTexture(static_cast<const Texture&>(texture), destBox, srcBox, alpha, transform, clip);
    }
}

void RenderPass::renderTexture(
    const Texture& texture,
    const RectI& destBox,
    const RectF& srcBox,
    float alpha,
    wl_output_transform transform,
    const Region* clip) const {
    if (destBox.isEmpty() || !texture.id()) return;

    const TextureQuadProgram* shader = nullptr;
    if (texture.target() == GL_TEXTURE_EXTERNAL_OES) {
        if (!texture.hasAlpha()) {
            shader = m_shaders->textureExternalRgbx();
        } else {
            shader = m_shaders->textureExternal();
        }
        if (!shader) {
            logError("External OES shader not available for external texture");
            return;
        }
    } else {
        // Standard 2D texture (RGBA or RGBX)
        if (!texture.hasAlpha()) {
            shader = &m_shaders->textureRgbx();
        } else {
            shader = &m_shaders->textureRgba();
        }
    }

    if (!texture.hasAlpha() && alpha >= 0.999f) {
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    shader->use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(texture.target(), texture.id());
    glTexParameteri(texture.target(), GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(texture.target(), GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    shader->setTextureUnit(0);
    shader->setAlpha(alpha);

    Matrix3 mvp = Matrix3::projectBox(destBox, transform, m_projection);
    shader->setProjection(mvp);

    Matrix3 texMvp = Matrix3::projectTexture(srcBox, transform);
    shader->setTextureProjection(texMvp);

    renderQuads(destBox, clip, shader->posAttrib());

    glBindTexture(texture.target(), 0);
}

void RenderPass::renderBlur(
    DualKawaseBlur& blurEngine,
    const Texture& source,
    const RectI& destBox,
    const RectI& srcBox,
    const DualKawaseConfig& config,
    const Texture* maskTexture) {
    if (destBox.isEmpty() || !source.id() || !m_target) return;
    blurEngine.renderBlurSubRegion(source, srcBox, *m_target, destBox, config, maskTexture);
}

void RenderPass::renderQuads(
    const RectI& box,
    const Region* clip,
    GLint posAttrib) const {
    if (posAttrib < 0) return;

    if (!clip) {
        static constexpr float kUnitQuad[12] = {
            0.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
        };
        glEnableVertexAttribArray(static_cast<GLuint>(posAttrib));
        glVertexAttribPointer(static_cast<GLuint>(posAttrib), 2, GL_FLOAT, GL_FALSE, 0, kUnitQuad);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(static_cast<GLuint>(posAttrib));
        return;
    }

    Region boxRegion(box);
    boxRegion.intersect(*clip);
    auto rects = boxRegion.rects();
    if (rects.empty()) return;

    std::vector<float> verts;
    verts.reserve(rects.size() * 12);
    const float invW = 1.0f / static_cast<float>(box.width);
    const float invH = 1.0f / static_cast<float>(box.height);

    for (const auto& r : rects) {
        float x1 = static_cast<float>(r.x - box.x) * invW;
        float y1 = static_cast<float>(r.y - box.y) * invH;
        float x2 = static_cast<float>(r.x + r.width - box.x) * invW;
        float y2 = static_cast<float>(r.y + r.height - box.y) * invH;

        // Triangle 1
        verts.push_back(x1); verts.push_back(y1);
        verts.push_back(x2); verts.push_back(y1);
        verts.push_back(x1); verts.push_back(y2);

        // Triangle 2
        verts.push_back(x2); verts.push_back(y1);
        verts.push_back(x2); verts.push_back(y2);
        verts.push_back(x1); verts.push_back(y2);
    }

    glEnableVertexAttribArray(static_cast<GLuint>(posAttrib));
    glVertexAttribPointer(static_cast<GLuint>(posAttrib), 2, GL_FLOAT, GL_FALSE, 0, verts.data());
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(rects.size() * 6));
    glDisableVertexAttribArray(static_cast<GLuint>(posAttrib));
}

bool RenderPass::submit() {
    if (m_submitted) return true;

    glFlush();
    RenderTarget::unbind();
    m_submitted = true;
    return true;
}

} // namespace nisaba::compositor

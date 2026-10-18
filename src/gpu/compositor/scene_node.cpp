#include "nisaba/compositor/scene_node.hpp"
#include "nisaba/compositor/scene_tree.hpp"

namespace nisaba::compositor {

PointI SceneNode::absolutePosition() const noexcept {
    PointI pos{m_x, m_y};
    const SceneTree* p = m_parent;
    while (p) {
        pos.x += p->x();
        pos.y += p->y();
        p = p->parent();
    }
    return pos;
}

// ==========================================
// SceneRect Implementation
// ==========================================

std::shared_ptr<SceneRect> SceneRect::create(int width, int height, const Color& color) {
    return std::make_shared<SceneRect>(width, height, color);
}

SceneRect::SceneRect(int width, int height, const Color& color)
    : SceneNode(SceneNodeType::Rect),
      m_width(width),
      m_height(height),
      m_color(color) {}

void SceneRect::render(
    IRenderPass& pass,
    const PointI& parentOffset,
    const Region* damage) const {
    if (!m_enabled || m_width <= 0 || m_height <= 0) return;

    RectI box{parentOffset.x + m_x, parentOffset.y + m_y, m_width, m_height};
    pass.renderRect(box, m_color, damage);
}

SceneNode* SceneRect::nodeAt(const PointI& rootPoint) {
    if (!m_enabled || m_width <= 0 || m_height <= 0) return nullptr;

    auto absPos = absolutePosition();
    RectI box{absPos.x, absPos.y, m_width, m_height};
    if (box.contains(rootPoint)) {
        return this;
    }
    return nullptr;
}

// ==========================================
// SceneBuffer Implementation
// ==========================================

std::shared_ptr<SceneBuffer> SceneBuffer::create(
    std::shared_ptr<Texture> texture,
    int width,
    int height) {
    if (texture) {
        if (width <= 0) width = texture->width();
        if (height <= 0) height = texture->height();
    }
    return std::make_shared<SceneBuffer>(std::move(texture), width, height);
}

SceneBuffer::SceneBuffer(std::shared_ptr<Texture> texture, int width, int height)
    : SceneNode(SceneNodeType::Buffer),
      m_texture(std::move(texture)),
      m_width(width),
      m_height(height) {}

void SceneBuffer::setTexture(std::shared_ptr<Texture> texture, int width, int height) {
    m_texture = std::move(texture);
    if (m_texture) {
        if (width <= 0) m_width = m_texture->width();
        else m_width = width;

        if (height <= 0) m_height = m_texture->height();
        else m_height = height;
    } else {
        m_width = width;
        m_height = height;
    }
}

void SceneBuffer::render(
    IRenderPass& pass,
    const PointI& parentOffset,
    const Region* damage) const {
    if (!m_enabled || !m_texture || m_width <= 0 || m_height <= 0) return;

    RectI box{parentOffset.x + m_x, parentOffset.y + m_y, m_width, m_height};
    pass.renderTexture(*m_texture, box, m_srcBox, m_alpha, m_transform, damage);
}

SceneNode* SceneBuffer::nodeAt(const PointI& rootPoint) {
    if (!m_enabled || m_width <= 0 || m_height <= 0) return nullptr;

    auto absPos = absolutePosition();
    RectI box{absPos.x, absPos.y, m_width, m_height};
    if (box.contains(rootPoint)) {
        return this;
    }
    return nullptr;
}

} // namespace nisaba::compositor

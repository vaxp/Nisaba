#pragma once

#include "nisaba/compositor/color.hpp"
#include "nisaba/compositor/geometry.hpp"
#include "nisaba/compositor/render_pass.hpp"
#include "nisaba/compositor/texture.hpp"

#include <memory>
#include <string>

namespace nisaba::compositor {

class SceneTree;

enum class SceneNodeType {
    Tree,
    Rect,
    Buffer
};

/**
 * @brief Base class for 2D desktop scene graph elements.
 */
class SceneNode : public std::enable_shared_from_this<SceneNode> {
public:
    explicit SceneNode(SceneNodeType type) noexcept
        : m_type(type) {}

    virtual ~SceneNode() = default;

    SceneNode(const SceneNode&) = delete;
    SceneNode& operator=(const SceneNode&) = delete;

    [[nodiscard]] SceneNodeType type() const noexcept { return m_type; }
    [[nodiscard]] bool isEnabled() const noexcept { return m_enabled; }
    void setEnabled(bool enabled) noexcept { m_enabled = enabled; }

    [[nodiscard]] int x() const noexcept { return m_x; }
    [[nodiscard]] int y() const noexcept { return m_y; }
    void setPosition(int x, int y) noexcept {
        m_x = x;
        m_y = y;
    }

    [[nodiscard]] SceneTree* parent() const noexcept { return m_parent; }
    void setParent(SceneTree* parent) noexcept { m_parent = parent; }

    /**
     * @brief Computes absolute root coordinate of this node by walking up parent tree.
     */
    [[nodiscard]] PointI absolutePosition() const noexcept;

    /**
     * @brief Recursively renders the node into an active RenderPass.
     */
    virtual void render(
        IRenderPass& pass,
        const PointI& parentOffset,
        const Region* damage) const = 0;

    /**
     * @brief Hit-tests point in root coordinates, returning topmost matching node.
     */
    virtual SceneNode* nodeAt(const PointI& rootPoint) = 0;

protected:
    SceneNodeType m_type;
    int m_x{0};
    int m_y{0};
    bool m_enabled{true};
    SceneTree* m_parent{nullptr};
};

/**
 * @brief Solid color rectangle scene node for borders, backgrounds, and accents.
 */
class SceneRect : public SceneNode {
public:
    static std::shared_ptr<SceneRect> create(int width, int height, const Color& color);

    SceneRect(int width, int height, const Color& color);
    ~SceneRect() override = default;

    void setSize(int width, int height) noexcept {
        m_width = width;
        m_height = height;
    }

    void setColor(const Color& color) noexcept { m_color = color; }
    [[nodiscard]] const Color& color() const noexcept { return m_color; }
    [[nodiscard]] int width() const noexcept { return m_width; }
    [[nodiscard]] int height() const noexcept { return m_height; }

    void render(
        IRenderPass& pass,
        const PointI& parentOffset,
        const Region* damage) const override;

    SceneNode* nodeAt(const PointI& rootPoint) override;

private:
    int m_width{0};
    int m_height{0};
    Color m_color{Color::black()};
};

/**
 * @brief Texture buffer scene node for Wayland surfaces, icons, and wallpapers.
 */
class SceneBuffer : public SceneNode {
public:
    static std::shared_ptr<SceneBuffer> create(
        std::shared_ptr<Texture> texture,
        int width = 0,
        int height = 0);

    SceneBuffer(std::shared_ptr<Texture> texture, int width, int height);
    ~SceneBuffer() override = default;

    void setTexture(std::shared_ptr<Texture> texture, int width = 0, int height = 0);
    [[nodiscard]] const std::shared_ptr<Texture>& texture() const noexcept { return m_texture; }

    void setSourceBox(const RectF& src) noexcept { m_srcBox = src; }
    [[nodiscard]] const RectF& sourceBox() const noexcept { return m_srcBox; }

    void setAlpha(float alpha) noexcept { m_alpha = alpha; }
    [[nodiscard]] float alpha() const noexcept { return m_alpha; }

    void setTransform(wl_output_transform t) noexcept { m_transform = t; }
    [[nodiscard]] wl_output_transform transform() const noexcept { return m_transform; }

    [[nodiscard]] int width() const noexcept { return m_width; }
    [[nodiscard]] int height() const noexcept { return m_height; }

    void setUserData(void* data) noexcept { m_userData = data; }
    [[nodiscard]] void* userData() const noexcept { return m_userData; }

    void render(
        IRenderPass& pass,
        const PointI& parentOffset,
        const Region* damage) const override;

    SceneNode* nodeAt(const PointI& rootPoint) override;

private:
    std::shared_ptr<Texture> m_texture;
    int m_width{0};
    int m_height{0};
    RectF m_srcBox{0.0, 0.0, 1.0, 1.0};
    float m_alpha{1.0f};
    wl_output_transform m_transform{WL_OUTPUT_TRANSFORM_NORMAL};
    void* m_userData{nullptr};
};

} // namespace nisaba::compositor

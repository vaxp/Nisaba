#include <iostream>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

// Custom drawer for DevTools-style box model visualization (Margin, Border, Padding, Content)
void draw_devtools_box_model(nisaba::Canvas& canvas, const nisaba::layout::Node* node, float absX, float absY) {
    if (!node) return;

    const float l = absX + node->getLayout().position(PhysicalEdge::Left);
    const float t = absY + node->getLayout().position(PhysicalEdge::Top);
    const float w = node->getLayout().dimension(Dimension::Width);
    const float h = node->getLayout().dimension(Dimension::Height);

    const float ml = node->getLayout().margin(PhysicalEdge::Left);
    const float mt = node->getLayout().margin(PhysicalEdge::Top);
    const float mr = node->getLayout().margin(PhysicalEdge::Right);
    const float mb = node->getLayout().margin(PhysicalEdge::Bottom);

    const float bl = node->getLayout().border(PhysicalEdge::Left);
    const float bt = node->getLayout().border(PhysicalEdge::Top);
    const float br = node->getLayout().border(PhysicalEdge::Right);
    const float bb = node->getLayout().border(PhysicalEdge::Bottom);

    const float pl = node->getLayout().padding(PhysicalEdge::Left);
    const float pt = node->getLayout().padding(PhysicalEdge::Top);
    const float pr = node->getLayout().padding(PhysicalEdge::Right);
    const float pb = node->getLayout().padding(PhysicalEdge::Bottom);

    // 1. Margin area (Outer rect: amber/orange tint)
    Paint margin_paint;
    margin_paint.set_color_rgba8(246, 178, 107, 140);
    canvas.fill_rect(*nisaba::Rect::from_xywh(l - ml, t - mt, w + ml + mr, h + mt + mb), margin_paint);

    // 2. Border area (Yellow/gold)
    Paint border_paint;
    border_paint.set_color_rgba8(255, 229, 153, 200);
    canvas.fill_rect(*nisaba::Rect::from_xywh(l, t, w, h), border_paint);

    // 3. Padding area (Light green)
    Paint padding_paint;
    padding_paint.set_color_rgba8(182, 215, 168, 200);
    canvas.fill_rect(*nisaba::Rect::from_xywh(l + bl, t + bt, w - bl - br, h - bt - bb), padding_paint);

    // 4. Content area (Light blue/teal)
    Paint content_paint;
    content_paint.set_color_rgba8(142, 198, 255, 240);
    const float cw = w - bl - br - pl - pr;
    const float ch = h - bt - bb - pt - pb;
    canvas.fill_rect(*nisaba::Rect::from_xywh(l + bl + pl, t + bt + pt, cw, ch), content_paint);

    // Outline stroke
    Paint outline_paint;
    outline_paint.set_color_rgba8(40, 50, 70, 255);
    Stroke stroke(1.5f);
    canvas.stroke_rect(*nisaba::Rect::from_xywh(l - ml, t - mt, w + ml + mr, h + mt + mb), outline_paint, stroke);
    canvas.stroke_rect(*nisaba::Rect::from_xywh(l, t, w, h), outline_paint, stroke);

    // DevTools Text Annotations using Sovereign Vector Text Engine
    draw_label(canvas, "margin: " + std::to_string(static_cast<int>(mt)), l - ml + 4.0f, t - mt + 2.0f, 10.0f, Color::from_rgba8(70, 40, 10, 240));
    draw_label(canvas, "border: " + std::to_string(static_cast<int>(bt)), l + 4.0f, t + 2.0f, 10.0f, Color::from_rgba8(80, 60, 10, 240));
    draw_label(canvas, "padding: " + std::to_string(static_cast<int>(pt)), l + bl + 4.0f, t + bt + 2.0f, 10.0f, Color::from_rgba8(30, 70, 20, 240));
    std::string content_str = "content: " + std::to_string(static_cast<int>(cw)) + "x" + std::to_string(static_cast<int>(ch));
    draw_label(canvas, content_str, l + bl + pl + 8.0f, t + bt + pt + (ch / 2.0f) - 6.0f, 12.0f, Color::from_rgba8(15, 35, 80, 255));
}

int main() {
    std::cout << "=== Nisaba Layout Example: Box Model (Margin, Border, Padding, Auto-Margins & BoxSizing) ===" << std::endl;

    const uint32_t canvasWidth = 1200;
    const uint32_t canvasHeight = 920;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    draw_label(canvas, "Flexbox: Box Model, BoxSizing & Margin-Auto Centering", 50.0f, 22.0f, 20.0f, Color::from_rgba8(220, 235, 255, 255));

    // Section 1: BoxSizing::BorderBox vs BoxSizing::ContentBox
    {
        std::cout << "\n--- Section 1: BoxSizing Comparison (BorderBox vs ContentBox) ---" << std::endl;

        const float cW = 520.0f;
        const float cH = 320.0f;
        const float yOffset = 70.0f;

        // BorderBox Case
        {
            draw_label(canvas, "BoxSizing::BorderBox (Total box is clamped to specified 300x180)", 50.0f, yOffset - 20.0f, 13.0f, Color::from_rgba8(170, 195, 235, 255));

            Node root;
            root.style().setFlexDirection(FlexDirection::Column);
            root.style().setDimension(Dimension::Width, Style::SizeLength::points(cW));
            root.style().setDimension(Dimension::Height, Style::SizeLength::points(cH));
            root.style().setPadding(nisaba::layout::Edge::All, Style::Length::points(20.0f));

            Node child;
            child.style().setBoxSizing(BoxSizing::BorderBox);
            child.style().setDimension(Dimension::Width, Style::SizeLength::points(300.0f));
            child.style().setDimension(Dimension::Height, Style::SizeLength::points(180.0f));
            child.style().setMargin(nisaba::layout::Edge::All, Style::Length::points(20.0f));
            child.style().setBorder(nisaba::layout::Edge::All, Style::Length::points(15.0f));
            child.style().setPadding(nisaba::layout::Edge::All, Style::Length::points(30.0f));

            root.insertChild(&child, 0);
            solveFlexLayout(&root, cW, cH, Direction::LTR);

            print_layout_metrics(&root, "Container (BorderBox child)");
            render_layout_tree(canvas, &root, 50.0f, yOffset, 0, 0, false);
            draw_devtools_box_model(canvas, root.getChild(0), 50.0f, yOffset);
        }

        // ContentBox Case
        {
            draw_label(canvas, "BoxSizing::ContentBox (Content is 300x180 -> Total box expands to 390x270)", 630.0f, yOffset - 20.0f, 13.0f, Color::from_rgba8(170, 195, 235, 255));

            Node root;
            root.style().setFlexDirection(FlexDirection::Column);
            root.style().setDimension(Dimension::Width, Style::SizeLength::points(cW));
            root.style().setDimension(Dimension::Height, Style::SizeLength::points(cH));
            root.style().setPadding(nisaba::layout::Edge::All, Style::Length::points(20.0f));

            Node child;
            child.style().setBoxSizing(BoxSizing::ContentBox);
            child.style().setDimension(Dimension::Width, Style::SizeLength::points(300.0f));
            child.style().setDimension(Dimension::Height, Style::SizeLength::points(180.0f));
            child.style().setMargin(nisaba::layout::Edge::All, Style::Length::points(20.0f));
            child.style().setBorder(nisaba::layout::Edge::All, Style::Length::points(15.0f));
            child.style().setPadding(nisaba::layout::Edge::All, Style::Length::points(30.0f));

            root.insertChild(&child, 0);
            solveFlexLayout(&root, cW, cH, Direction::LTR);

            print_layout_metrics(&root, "Container (ContentBox child)");
            render_layout_tree(canvas, &root, 630.0f, yOffset, 0, 0, false);
            draw_devtools_box_model(canvas, root.getChild(0), 630.0f, yOffset);
        }
    }

    // Section 2: Margin Auto (Auto-centering and Auto-pushing)
    {
        std::cout << "\n--- Section 2: Margin Auto (Push right & Full Centering) ---" << std::endl;

        const float cW = 520.0f;
        const float cH = 400.0f;
        const float yOffset = 460.0f;

        // Case A: Margin-left auto (Push item to right edge of flex container)
        {
            draw_label(canvas, "Margin-Left: Auto (Pushes Item 2 to the far right edge)", 50.0f, yOffset - 20.0f, 13.0f, Color::from_rgba8(170, 195, 235, 255));

            Node root;
            root.style().setFlexDirection(FlexDirection::Row);
            root.style().setDimension(Dimension::Width, Style::SizeLength::points(cW));
            root.style().setDimension(Dimension::Height, Style::SizeLength::points(cH));
            root.style().setPadding(nisaba::layout::Edge::All, Style::Length::points(20.0f));

            std::vector<Node> children(2);
            // First item: regular
            children[0].style().setDimension(Dimension::Width, Style::SizeLength::points(120.0f));
            children[0].style().setDimension(Dimension::Height, Style::SizeLength::points(80.0f));

            // Second item: margin-left auto (pushed to far right)
            children[1].style().setDimension(Dimension::Width, Style::SizeLength::points(120.0f));
            children[1].style().setDimension(Dimension::Height, Style::SizeLength::points(80.0f));
            children[1].style().setMargin(nisaba::layout::Edge::Left, Style::Length::ofAuto());

            root.insertChild(&children[0], 0);
            root.insertChild(&children[1], 1);

            solveFlexLayout(&root, cW, cH, Direction::LTR);
            print_layout_metrics(&root, "Margin-Left Auto Push");
            render_layout_tree(canvas, &root, 50.0f, yOffset);
        }

        // Case B: Margin All Auto (Full horizontal & vertical centering without justify/align)
        {
            draw_label(canvas, "Margin: Auto (Perfect horizontal & vertical centering without justify/align)", 630.0f, yOffset - 20.0f, 13.0f, Color::from_rgba8(170, 195, 235, 255));

            Node root;
            root.style().setFlexDirection(FlexDirection::Row);
            root.style().setDimension(Dimension::Width, Style::SizeLength::points(cW));
            root.style().setDimension(Dimension::Height, Style::SizeLength::points(cH));
            root.style().setPadding(nisaba::layout::Edge::All, Style::Length::points(20.0f));

            Node child;
            child.style().setDimension(Dimension::Width, Style::SizeLength::points(200.0f));
            child.style().setDimension(Dimension::Height, Style::SizeLength::points(140.0f));
            child.style().setMargin(nisaba::layout::Edge::All, Style::Length::ofAuto());

            root.insertChild(&child, 0);

            solveFlexLayout(&root, cW, cH, Direction::LTR);
            print_layout_metrics(&root, "Margin All Auto Centering");
            render_layout_tree(canvas, &root, 630.0f, yOffset);
        }
    }

    const std::string outPath = "layout_box_model.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

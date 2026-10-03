#include <iostream>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: JustifyContent (Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly) ===" << std::endl;

    const uint32_t canvasWidth = 1200;
    const uint32_t canvasHeight = 920;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    struct JustifyCase {
        const char* title;
        Justify justify;
        float x;
        float y;
        float w;
        float h;
    };

    std::vector<JustifyCase> cases = {
        {"Justify::FlexStart", Justify::FlexStart, 50.0f, 65.0f, 520.0f, 230.0f},
        {"Justify::Center", Justify::Center, 630.0f, 65.0f, 520.0f, 230.0f},
        {"Justify::FlexEnd", Justify::FlexEnd, 50.0f, 355.0f, 520.0f, 230.0f},
        {"Justify::SpaceBetween", Justify::SpaceBetween, 630.0f, 355.0f, 520.0f, 230.0f},
        {"Justify::SpaceAround", Justify::SpaceAround, 50.0f, 645.0f, 520.0f, 230.0f},
        {"Justify::SpaceEvenly", Justify::SpaceEvenly, 630.0f, 645.0f, 520.0f, 230.0f},
    };

    draw_label(canvas, "Flexbox: JustifyContent (Alignment along Main Axis)", 50.0f, 22.0f, 20.0f, Color::from_rgba8(220, 235, 255, 255));

    for (const auto& c : cases) {
        std::cout << "\nCase: " << c.title << std::endl;
        draw_label(canvas, c.title, c.x, c.y - 20.0f, 15.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setJustifyContent(c.justify);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(c.w));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(c.h));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));

        std::vector<Node> children(3);
        // Varying widths to see space distribution clearly
        const float widths[3] = {120.0f, 160.0f, 100.0f};
        for (size_t i = 0; i < children.size(); ++i) {
            children[i].style().setDimension(Dimension::Width, Style::SizeLength::points(widths[i]));
            children[i].style().setDimension(Dimension::Height, Style::SizeLength::points(55.0f));
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, c.w, c.h, Direction::LTR);
        print_layout_metrics(&root, c.title);

        render_layout_tree(canvas, &root, c.x, c.y);
    }

    const std::string outPath = "layout_justify_content.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

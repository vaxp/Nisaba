#include <iostream>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: AlignItems & AlignSelf ===" << std::endl;

    const uint32_t canvasWidth = 1100;
    const uint32_t canvasHeight = 930;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    struct AlignCase {
        const char* title;
        Align align;
        bool customAlignSelf;
        float y;
    };

    std::vector<AlignCase> cases = {
        {"AlignItems::FlexStart", Align::FlexStart, false, 65.0f},
        {"AlignItems::Center", Align::Center, false, 235.0f},
        {"AlignItems::FlexEnd", Align::FlexEnd, false, 405.0f},
        {"AlignItems::Stretch (No explicit child height)", Align::Stretch, false, 575.0f},
        {"AlignSelf Overrides (Container Start, Children self-align)", Align::FlexStart, true, 745.0f},
    };

    const float containerWidth = 980.0f;
    const float containerHeight = 125.0f;
    const float containerX = 60.0f;

    draw_label(canvas, "Flexbox: AlignItems & AlignSelf (Cross-Axis Alignment)", 60.0f, 22.0f, 20.0f, Color::from_rgba8(220, 235, 255, 255));

    for (const auto& c : cases) {
        std::cout << "\nCase: " << c.title << std::endl;
        draw_label(canvas, c.title, containerX, c.y - 18.0f, 14.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setAlignItems(c.align);
        root.style().setJustifyContent(Justify::SpaceEvenly);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(containerWidth));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(containerHeight));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(10.0f));

        std::vector<Node> children(4);
        const float heights[4] = {40.0f, 65.0f, 85.0f, 50.0f};

        for (size_t i = 0; i < children.size(); ++i) {
            children[i].style().setDimension(Dimension::Width, Style::SizeLength::points(150.0f));

            if (c.align != Align::Stretch || c.customAlignSelf) {
                children[i].style().setDimension(Dimension::Height, Style::SizeLength::points(heights[i]));
            }

            if (c.customAlignSelf) {
                if (i == 0) children[i].style().setAlignSelf(Align::FlexStart);
                if (i == 1) children[i].style().setAlignSelf(Align::Center);
                if (i == 2) children[i].style().setAlignSelf(Align::FlexEnd);
                if (i == 3) {
                    // Test stretch on child 3
                    children[i].style().setDimension(Dimension::Height, Style::SizeLength::ofAuto());
                    children[i].style().setAlignSelf(Align::Stretch);
                }
            }

            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, containerWidth, containerHeight, Direction::LTR);
        print_layout_metrics(&root, c.title);

        render_layout_tree(canvas, &root, containerX, c.y);
    }

    const std::string outPath = "layout_align_items.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

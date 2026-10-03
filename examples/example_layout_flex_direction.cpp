#include <iostream>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: FlexDirection (Row, Column, RowReverse, ColumnReverse) ===" << std::endl;

    const uint32_t canvasWidth = 1200;
    const uint32_t canvasHeight = 840;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    struct DirectionCase {
        const char* title;
        FlexDirection direction;
        float x;
        float y;
        float w;
        float h;
    };

    std::vector<DirectionCase> cases = {
        {"FlexDirection::Row", FlexDirection::Row, 60.0f, 80.0f, 500.0f, 300.0f},
        {"FlexDirection::Column", FlexDirection::Column, 640.0f, 80.0f, 500.0f, 300.0f},
        {"FlexDirection::RowReverse", FlexDirection::RowReverse, 60.0f, 460.0f, 500.0f, 300.0f},
        {"FlexDirection::ColumnReverse", FlexDirection::ColumnReverse, 640.0f, 460.0f, 500.0f, 300.0f},
    };

    draw_label(canvas, "Flexbox: FlexDirection (Row, Column, RowReverse, ColumnReverse)", 60.0f, 25.0f, 20.0f, Color::from_rgba8(220, 235, 255, 255));

    for (const auto& c : cases) {
        std::cout << "\nCase: " << c.title << std::endl;
        draw_label(canvas, c.title, c.x, c.y - 22.0f, 15.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(c.direction);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(c.w));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(c.h));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));
        root.style().setGap(Gutter::All, Style::Length::points(12.0f));

        std::vector<Node> children(4);
        for (size_t i = 0; i < children.size(); ++i) {
            children[i].style().setDimension(Dimension::Width, Style::SizeLength::points(80.0f));
            children[i].style().setDimension(Dimension::Height, Style::SizeLength::points(50.0f));
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, c.w, c.h, Direction::LTR);
        print_layout_metrics(&root, c.title);

        render_layout_tree(canvas, &root, c.x, c.y);
    }

    const std::string outPath = "layout_flex_direction.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

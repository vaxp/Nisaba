#include <iostream>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: FlexWrap, Gap, and AlignContent ===" << std::endl;

    const uint32_t canvasWidth = 1200;
    const uint32_t canvasHeight = 930;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    struct WrapCase {
        const char* title;
        Wrap wrap;
        Align alignContent;
        float rowGap;
        float colGap;
        float x;
        float y;
        float w;
        float h;
    };

    std::vector<WrapCase> cases = {
        {"Wrap::Wrap (AlignContent::FlexStart, RowGap: 10, ColGap: 15)",
         Wrap::Wrap, Align::FlexStart, 10.0f, 15.0f, 50.0f, 65.0f, 520.0f, 370.0f},

        {"Wrap::Wrap (AlignContent::Center, RowGap: 10, ColGap: 15)",
         Wrap::Wrap, Align::Center, 10.0f, 15.0f, 630.0f, 65.0f, 520.0f, 370.0f},

        {"Wrap::Wrap (AlignContent::SpaceBetween, RowGap: 10, ColGap: 15)",
         Wrap::Wrap, Align::SpaceBetween, 10.0f, 15.0f, 50.0f, 495.0f, 520.0f, 370.0f},

        {"Wrap::WrapReverse (AlignContent::FlexStart, RowGap: 10, ColGap: 15)",
         Wrap::WrapReverse, Align::FlexStart, 10.0f, 15.0f, 630.0f, 495.0f, 520.0f, 370.0f},
    };

    draw_label(canvas, "Flexbox: FlexWrap, Row & Column Gaps, and AlignContent", 50.0f, 22.0f, 20.0f, Color::from_rgba8(220, 235, 255, 255));

    for (const auto& c : cases) {
        std::cout << "\n--- " << c.title << " ---" << std::endl;
        draw_label(canvas, c.title, c.x, c.y - 20.0f, 13.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setFlexWrap(c.wrap);
        root.style().setAlignContent(c.alignContent);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(c.w));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(c.h));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));
        root.style().setGap(Gutter::Row, Style::Length::points(c.rowGap));
        root.style().setGap(Gutter::Column, Style::Length::points(c.colGap));

        // Create 8 child items that will wrap across multiple lines
        std::vector<Node> children(8);
        const float itemW[8] = {110.0f, 140.0f, 90.0f, 130.0f, 120.0f, 100.0f, 150.0f, 80.0f};
        const float itemH[8] = {50.0f, 65.0f, 45.0f, 60.0f, 55.0f, 70.0f, 50.0f, 60.0f};

        for (size_t i = 0; i < children.size(); ++i) {
            children[i].style().setDimension(Dimension::Width, Style::SizeLength::points(itemW[i]));
            children[i].style().setDimension(Dimension::Height, Style::SizeLength::points(itemH[i]));
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, c.w, c.h, Direction::LTR);
        print_layout_metrics(&root, c.title);

        render_layout_tree(canvas, &root, c.x, c.y);
    }

    const std::string outPath = "layout_flex_wrap_gap.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

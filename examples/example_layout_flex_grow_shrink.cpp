#include <iostream>
#include <vector>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: FlexGrow, FlexShrink, FlexBasis & Min/Max Constraints ===" << std::endl;

    const uint32_t canvasWidth = 1200;
    const uint32_t canvasHeight = 920;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    draw_label(canvas, "Flexbox: FlexGrow, FlexShrink & Min/Max Constraints", 60.0f, 22.0f, 20.0f, Color::from_rgba8(220, 235, 255, 255));

    const float boxW = 1080.0f;
    const float boxH = 150.0f;
    const float startX = 60.0f;

    // Case 1: flexGrow ratios (1 : 2 : 1)
    {
        const char* title = "Case 1: Proportional flexGrow (Child 0: grow 1, Child 1: grow 2, Child 2: grow 1)";
        std::cout << "\n--- " << title << " ---" << std::endl;
        draw_label(canvas, title, startX, 48.0f, 14.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(boxW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(boxH));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));
        root.style().setGap(Gutter::All, Style::Length::points(15.0f));

        std::vector<Node> children(3);
        children[0].style().setFlexGrow(FloatOptional{1.0f});
        children[0].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[1].style().setFlexGrow(FloatOptional{2.0f});
        children[1].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[2].style().setFlexGrow(FloatOptional{1.0f});
        children[2].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        for (size_t i = 0; i < children.size(); ++i) {
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, boxW, boxH, Direction::LTR);
        print_layout_metrics(&root, "flexGrow (1:2:1)");
        render_layout_tree(canvas, &root, startX, 68.0f);
    }

    // Case 2: One flexGrow item among fixed-width items (Spacer pattern)
    {
        const char* title = "Case 2: Flexible Center Spacer (Fixed 120px, flexGrow 1.0, Fixed 120px)";
        std::cout << "\n--- " << title << " ---" << std::endl;
        draw_label(canvas, title, startX, 258.0f, 14.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(boxW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(boxH));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));
        root.style().setGap(Gutter::All, Style::Length::points(15.0f));

        std::vector<Node> children(3);
        children[0].style().setDimension(Dimension::Width, Style::SizeLength::points(120.0f));
        children[0].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[1].style().setFlexGrow(FloatOptional{1.0f});
        children[1].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[2].style().setDimension(Dimension::Width, Style::SizeLength::points(120.0f));
        children[2].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        for (size_t i = 0; i < children.size(); ++i) {
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, boxW, boxH, Direction::LTR);
        print_layout_metrics(&root, "Flexible Center Item");
        render_layout_tree(canvas, &root, startX, 278.0f);
    }

    // Case 3: flexShrink in overflowing container (Child 0 rigid shrink=0, Child 1 shrink=1, Child 2 shrink=2)
    {
        const char* title = "Case 3: flexShrink Overflow Absorption (Child 0: rigid 0, Child 1: shrink 1, Child 2: shrink 2)";
        std::cout << "\n--- " << title << " ---" << std::endl;
        draw_label(canvas, title, startX, 468.0f, 14.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(boxW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(boxH));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));
        root.style().setGap(Gutter::All, Style::Length::points(15.0f));

        std::vector<Node> children(3);
        // Each child requests 500px (total 1500px + gaps > 1080px)
        children[0].style().setDimension(Dimension::Width, Style::SizeLength::points(500.0f));
        children[0].style().setFlexShrink(FloatOptional{0.0f}); // Will not shrink at all!
        children[0].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[1].style().setDimension(Dimension::Width, Style::SizeLength::points(500.0f));
        children[1].style().setFlexShrink(FloatOptional{1.0f});
        children[1].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[2].style().setDimension(Dimension::Width, Style::SizeLength::points(500.0f));
        children[2].style().setFlexShrink(FloatOptional{2.0f});
        children[2].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        for (size_t i = 0; i < children.size(); ++i) {
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, boxW, boxH, Direction::LTR);
        print_layout_metrics(&root, "flexShrink (0 vs 1 vs 2)");
        render_layout_tree(canvas, &root, startX, 488.0f);
    }

    // Case 4: minWidth and maxWidth Clamping
    {
        const char* title = "Case 4: Dimension Clamping (Child 0: maxWidth 200px, Child 1: minWidth 450px, Child 2: unconstrained)";
        std::cout << "\n--- " << title << " ---" << std::endl;
        draw_label(canvas, title, startX, 678.0f, 14.0f, Color::from_rgba8(170, 195, 235, 255));

        Node root;
        root.style().setFlexDirection(FlexDirection::Row);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(boxW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(boxH));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(15.0f));
        root.style().setGap(Gutter::All, Style::Length::points(15.0f));

        std::vector<Node> children(3);
        children[0].style().setFlexGrow(FloatOptional{1.0f});
        children[0].style().setMaxDimension(Dimension::Width, Style::SizeLength::points(200.0f));
        children[0].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[1].style().setFlexGrow(FloatOptional{1.0f});
        children[1].style().setMinDimension(Dimension::Width, Style::SizeLength::points(450.0f));
        children[1].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        children[2].style().setFlexGrow(FloatOptional{1.0f});
        children[2].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));

        for (size_t i = 0; i < children.size(); ++i) {
            root.insertChild(&children[i], i);
        }

        solveFlexLayout(&root, boxW, boxH, Direction::LTR);
        print_layout_metrics(&root, "minWidth & maxWidth Clamping");
        render_layout_tree(canvas, &root, startX, 698.0f);
    }

    const std::string outPath = "layout_flex_grow_shrink.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

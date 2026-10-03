#include <iostream>
#include <vector>
#include <memory>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: Stack & Absolute Positioning ===" << std::endl;

    const uint32_t canvasWidth = 1200;
    const uint32_t canvasHeight = 920;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) {
        std::cerr << "Failed to allocate pixmap surface" << std::endl;
        return 1;
    }
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    draw_label(canvas, "Nisaba Sovereign Engine: Stack & Absolute Positioning", 60.0f, 25.0f, 22.0f, Color::from_rgba8(220, 235, 255, 255));
    draw_label(canvas, "Z-Index Layering, Floating Badges, Modals, Full-Bleed Overlays & Inset Constraints", 60.0f, 52.0f, 13.0f, Color::from_rgba8(140, 170, 215, 200));

    const float cardW = 510.0f;
    const float cardH = 360.0f;

    // =========================================================================
    // CASE 1: Flutter-Style Stack (Overlapping Layered Depth Cards)
    // =========================================================================
    {
        const float x = 60.0f;
        const float y = 105.0f;
        draw_label(canvas, "1. Flutter Stack (Depth Layering & Coordinate Sharing)", x, y - 20.0f, 14.0f, Color::from_rgba8(170, 205, 245, 255));

        Node root;
        root.style().setPositionType(PositionType::Relative);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(cardW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(cardH));

        // Layer 0: Background Layer
        Node card0;
        card0.style().setPositionType(PositionType::Absolute);
        card0.style().setPosition(LayoutEdge::Top, Style::Length::points(25.0f));
        card0.style().setPosition(LayoutEdge::Left, Style::Length::points(30.0f));
        card0.style().setDimension(Dimension::Width, Style::SizeLength::points(360.0f));
        card0.style().setDimension(Dimension::Height, Style::SizeLength::points(210.0f));

        // Layer 1: Mid Depth Layer
        Node card1;
        card1.style().setPositionType(PositionType::Absolute);
        card1.style().setPosition(LayoutEdge::Top, Style::Length::points(65.0f));
        card1.style().setPosition(LayoutEdge::Left, Style::Length::points(75.0f));
        card1.style().setDimension(Dimension::Width, Style::SizeLength::points(360.0f));
        card1.style().setDimension(Dimension::Height, Style::SizeLength::points(210.0f));

        // Layer 2: Front Active Card
        Node card2;
        card2.style().setPositionType(PositionType::Absolute);
        card2.style().setPosition(LayoutEdge::Top, Style::Length::points(105.0f));
        card2.style().setPosition(LayoutEdge::Left, Style::Length::points(120.0f));
        card2.style().setDimension(Dimension::Width, Style::SizeLength::points(360.0f));
        card2.style().setDimension(Dimension::Height, Style::SizeLength::points(210.0f));

        // Layer 3: Floating Status Badge pinned to front card top-right
        Node badge;
        badge.style().setPositionType(PositionType::Absolute);
        badge.style().setPosition(LayoutEdge::Top, Style::Length::points(85.0f));
        badge.style().setPosition(LayoutEdge::Right, Style::Length::points(15.0f));
        badge.style().setDimension(Dimension::Width, Style::SizeLength::points(110.0f));
        badge.style().setDimension(Dimension::Height, Style::SizeLength::points(36.0f));

        root.insertChild(&card0, 0);
        root.insertChild(&card1, 1);
        root.insertChild(&card2, 2);
        root.insertChild(&badge, 3);

        solveFlexLayout(&root, cardW, cardH, Direction::LTR);
        print_layout_metrics(&root, "Case 1: Flutter Stack Layering");
        render_layout_tree(canvas, &root, x, y);
    }

    // =========================================================================
    // CASE 2: Product Card with Normal Flow + Floating Overlays & FAB
    // =========================================================================
    {
        const float x = 630.0f;
        const float y = 105.0f;
        draw_label(canvas, "2. Flow Content + Absolute Overlays & Badges", x, y - 20.0f, 14.0f, Color::from_rgba8(170, 205, 245, 255));

        Node root;
        root.style().setPositionType(PositionType::Relative);
        root.style().setFlexDirection(FlexDirection::Column);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(cardW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(cardH));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(16.0f));
        root.style().setGap(Gutter::Row, Style::Length::points(12.0f));

        // Normal flow: Media banner placeholder
        Node mediaHero;
        mediaHero.style().setDimension(Dimension::Width, Style::SizeLength::percent(100.0f));
        mediaHero.style().setDimension(Dimension::Height, Style::SizeLength::points(170.0f));

        // Normal flow: Product Details Block
        Node details;
        details.style().setDimension(Dimension::Width, Style::SizeLength::percent(100.0f));
        details.style().setDimension(Dimension::Height, Style::SizeLength::points(60.0f));

        // Normal flow: Meta info
        Node meta;
        meta.style().setDimension(Dimension::Width, Style::SizeLength::percent(60.0f));
        meta.style().setDimension(Dimension::Height, Style::SizeLength::points(40.0f));

        // Absolute Overlay 1: Discount Pill (top-left)
        Node promoTag;
        promoTag.style().setPositionType(PositionType::Absolute);
        promoTag.style().setPosition(LayoutEdge::Top, Style::Length::points(26.0f));
        promoTag.style().setPosition(LayoutEdge::Left, Style::Length::points(26.0f));
        promoTag.style().setDimension(Dimension::Width, Style::SizeLength::points(100.0f));
        promoTag.style().setDimension(Dimension::Height, Style::SizeLength::points(30.0f));

        // Absolute Overlay 2: Floating Wishlist Button (top-right)
        Node favBtn;
        favBtn.style().setPositionType(PositionType::Absolute);
        favBtn.style().setPosition(LayoutEdge::Top, Style::Length::points(26.0f));
        favBtn.style().setPosition(LayoutEdge::Right, Style::Length::points(26.0f));
        favBtn.style().setDimension(Dimension::Width, Style::SizeLength::points(36.0f));
        favBtn.style().setDimension(Dimension::Height, Style::SizeLength::points(36.0f));

        // Absolute Overlay 3: Floating Action Button (FAB "+ BUY NOW" bottom-right)
        Node fab;
        fab.style().setPositionType(PositionType::Absolute);
        fab.style().setPosition(LayoutEdge::Bottom, Style::Length::points(20.0f));
        fab.style().setPosition(LayoutEdge::Right, Style::Length::points(20.0f));
        fab.style().setDimension(Dimension::Width, Style::SizeLength::points(140.0f));
        fab.style().setDimension(Dimension::Height, Style::SizeLength::points(44.0f));

        root.insertChild(&mediaHero, 0);
        root.insertChild(&details, 1);
        root.insertChild(&meta, 2);
        root.insertChild(&promoTag, 3);
        root.insertChild(&favBtn, 4);
        root.insertChild(&fab, 5);

        solveFlexLayout(&root, cardW, cardH, Direction::LTR);
        print_layout_metrics(&root, "Case 2: E-Commerce Overlays");
        render_layout_tree(canvas, &root, x, y);
    }

    // =========================================================================
    // CASE 3: Modal Dialog & Scrim Overlay (Full-Bleed Stretch Insets)
    // =========================================================================
    {
        const float x = 60.0f;
        const float y = 520.0f;
        draw_label(canvas, "3. Modal Dialog with Scrim Backdrop (Full Inset Pinning)", x, y - 20.0f, 14.0f, Color::from_rgba8(170, 205, 245, 255));

        Node root;
        root.style().setPositionType(PositionType::Relative);
        root.style().setFlexDirection(FlexDirection::Column);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(cardW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(cardH));
        root.style().setPadding(LayoutEdge::All, Style::Length::points(20.0f));
        root.style().setGap(Gutter::Row, Style::Length::points(14.0f));

        // Background app flow (simulating content behind the modal)
        std::vector<Node> bgRows(4);
        for (size_t i = 0; i < bgRows.size(); ++i) {
            bgRows[i].style().setDimension(Dimension::Width, Style::SizeLength::percent(100.0f));
            bgRows[i].style().setDimension(Dimension::Height, Style::SizeLength::points(55.0f));
            root.insertChild(&bgRows[i], i);
        }

        // Absolute Scrim Overlay (Stretching completely over container: top=0, left=0, right=0, bottom=0)
        Node scrim;
        scrim.style().setPositionType(PositionType::Absolute);
        scrim.style().setPosition(LayoutEdge::Top, Style::Length::points(0.0f));
        scrim.style().setPosition(LayoutEdge::Left, Style::Length::points(0.0f));
        scrim.style().setPosition(LayoutEdge::Right, Style::Length::points(0.0f));
        scrim.style().setPosition(LayoutEdge::Bottom, Style::Length::points(0.0f));

        // Absolute Modal Window (Centered via horizontal & vertical insets)
        Node modal;
        modal.style().setPositionType(PositionType::Absolute);
        modal.style().setPosition(LayoutEdge::Top, Style::Length::points(50.0f));
        modal.style().setPosition(LayoutEdge::Bottom, Style::Length::points(50.0f));
        modal.style().setPosition(LayoutEdge::Left, Style::Length::points(45.0f));
        modal.style().setPosition(LayoutEdge::Right, Style::Length::points(45.0f));
        modal.style().setPadding(LayoutEdge::All, Style::Length::points(18.0f));
        modal.style().setFlexDirection(FlexDirection::Column);
        modal.style().setJustifyContent(Justify::SpaceBetween);

        // Modal contents in normal flow inside the modal
        Node modalHeader;
        modalHeader.style().setDimension(Dimension::Width, Style::SizeLength::percent(75.0f));
        modalHeader.style().setDimension(Dimension::Height, Style::SizeLength::points(32.0f));

        Node modalBody;
        modalBody.style().setDimension(Dimension::Width, Style::SizeLength::percent(100.0f));
        modalBody.style().setDimension(Dimension::Height, Style::SizeLength::points(90.0f));

        Node modalActions;
        modalActions.style().setFlexDirection(FlexDirection::Row);
        modalActions.style().setJustifyContent(Justify::FlexEnd);
        modalActions.style().setGap(Gutter::Column, Style::Length::points(12.0f));
        modalActions.style().setDimension(Dimension::Height, Style::SizeLength::points(38.0f));

        Node btnCancel;
        btnCancel.style().setDimension(Dimension::Width, Style::SizeLength::points(90.0f));
        btnCancel.style().setDimension(Dimension::Height, Style::SizeLength::points(38.0f));

        Node btnOk;
        btnOk.style().setDimension(Dimension::Width, Style::SizeLength::points(110.0f));
        btnOk.style().setDimension(Dimension::Height, Style::SizeLength::points(38.0f));

        modalActions.insertChild(&btnCancel, 0);
        modalActions.insertChild(&btnOk, 1);

        // Absolute Close Button inside modal
        Node closeBtn;
        closeBtn.style().setPositionType(PositionType::Absolute);
        closeBtn.style().setPosition(LayoutEdge::Top, Style::Length::points(10.0f));
        closeBtn.style().setPosition(LayoutEdge::Right, Style::Length::points(10.0f));
        closeBtn.style().setDimension(Dimension::Width, Style::SizeLength::points(28.0f));
        closeBtn.style().setDimension(Dimension::Height, Style::SizeLength::points(28.0f));

        modal.insertChild(&modalHeader, 0);
        modal.insertChild(&modalBody, 1);
        modal.insertChild(&modalActions, 2);
        modal.insertChild(&closeBtn, 3);

        root.insertChild(&scrim, 4);
        root.insertChild(&modal, 5);

        solveFlexLayout(&root, cardW, cardH, Direction::LTR);
        print_layout_metrics(&root, "Case 3: Modal Dialog & Scrim");
        render_layout_tree(canvas, &root, x, y);
    }

    // =========================================================================
    // CASE 4: Four Corners Pinning & Floating Bottom Dock (HUD / Dashboard)
    // =========================================================================
    {
        const float x = 630.0f;
        const float y = 520.0f;
        draw_label(canvas, "4. Four-Corner Anchors & Floating Bottom Dock", x, y - 20.0f, 14.0f, Color::from_rgba8(170, 205, 245, 255));

        Node root;
        root.style().setPositionType(PositionType::Relative);
        root.style().setDimension(Dimension::Width, Style::SizeLength::points(cardW));
        root.style().setDimension(Dimension::Height, Style::SizeLength::points(cardH));

        // 1. Top-Left Pin (Player Status / Info)
        Node pinTL;
        pinTL.style().setPositionType(PositionType::Absolute);
        pinTL.style().setPosition(LayoutEdge::Top, Style::Length::points(16.0f));
        pinTL.style().setPosition(LayoutEdge::Left, Style::Length::points(16.0f));
        pinTL.style().setDimension(Dimension::Width, Style::SizeLength::points(130.0f));
        pinTL.style().setDimension(Dimension::Height, Style::SizeLength::points(46.0f));

        // 2. Top-Right Pin (Settings / Live Telemetry)
        Node pinTR;
        pinTR.style().setPositionType(PositionType::Absolute);
        pinTR.style().setPosition(LayoutEdge::Top, Style::Length::points(16.0f));
        pinTR.style().setPosition(LayoutEdge::Right, Style::Length::points(16.0f));
        pinTR.style().setDimension(Dimension::Width, Style::SizeLength::points(110.0f));
        pinTR.style().setDimension(Dimension::Height, Style::SizeLength::points(46.0f));

        // 3. Center Target Indicator (Crosshairs / Center View)
        Node centerBox;
        centerBox.style().setPositionType(PositionType::Absolute);
        centerBox.style().setPosition(LayoutEdge::Top, Style::Length::points(110.0f));
        centerBox.style().setPosition(LayoutEdge::Left, Style::Length::points(165.0f));
        centerBox.style().setDimension(Dimension::Width, Style::SizeLength::points(180.0f));
        centerBox.style().setDimension(Dimension::Height, Style::SizeLength::points(110.0f));

        // 4. Bottom-Left Pin (Mini map)
        Node pinBL;
        pinBL.style().setPositionType(PositionType::Absolute);
        pinBL.style().setPosition(LayoutEdge::Bottom, Style::Length::points(16.0f));
        pinBL.style().setPosition(LayoutEdge::Left, Style::Length::points(16.0f));
        pinBL.style().setDimension(Dimension::Width, Style::SizeLength::points(95.0f));
        pinBL.style().setDimension(Dimension::Height, Style::SizeLength::points(50.0f));

        // 5. Bottom-Right Pin (Inventory)
        Node pinBR;
        pinBR.style().setPositionType(PositionType::Absolute);
        pinBR.style().setPosition(LayoutEdge::Bottom, Style::Length::points(16.0f));
        pinBR.style().setPosition(LayoutEdge::Right, Style::Length::points(16.0f));
        pinBR.style().setDimension(Dimension::Width, Style::SizeLength::points(95.0f));
        pinBR.style().setDimension(Dimension::Height, Style::SizeLength::points(50.0f));

        // 6. Floating Bottom Action Dock (Anchored horizontally between left & right)
        Node bottomDock;
        bottomDock.style().setPositionType(PositionType::Absolute);
        bottomDock.style().setPosition(LayoutEdge::Bottom, Style::Length::points(16.0f));
        bottomDock.style().setPosition(LayoutEdge::Left, Style::Length::points(130.0f));
        bottomDock.style().setPosition(LayoutEdge::Right, Style::Length::points(130.0f));
        bottomDock.style().setDimension(Dimension::Height, Style::SizeLength::points(50.0f));

        root.insertChild(&pinTL, 0);
        root.insertChild(&pinTR, 1);
        root.insertChild(&centerBox, 2);
        root.insertChild(&pinBL, 3);
        root.insertChild(&pinBR, 4);
        root.insertChild(&bottomDock, 5);

        solveFlexLayout(&root, cardW, cardH, Direction::LTR);
        print_layout_metrics(&root, "Case 4: Four-Corner & Dock");
        render_layout_tree(canvas, &root, x, y);
    }

    const std::string outPath = "layout_absolute_stack.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    } else {
        std::cerr << "\n[ERROR] Failed to save output image: " << outPath << std::endl;
        return 1;
    }

    return 0;
}

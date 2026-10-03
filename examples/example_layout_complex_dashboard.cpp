#include <iostream>
#include <vector>
#include <memory>
#include "nisaba/nisaba.hpp"
#include "layout_render_util.hpp"

using namespace nisaba;
using namespace nisaba::layout;
using namespace layout_example;

int main() {
    std::cout << "=== Nisaba Layout Example: Complex Modern Dashboard UI ===" << std::endl;

    const uint32_t canvasWidth = 1280;
    const uint32_t canvasHeight = 850;

    auto pixmap = Pixmap::create(canvasWidth, canvasHeight);
    if (!pixmap) return 1;
    Canvas canvas(*pixmap);
    draw_background(canvas, canvasWidth, canvasHeight);

    // Root App Container (Full Window)
    Node appRoot;
    appRoot.style().setFlexDirection(FlexDirection::Column);
    appRoot.style().setDimension(Dimension::Width, Style::SizeLength::points(1240.0f));
    appRoot.style().setDimension(Dimension::Height, Style::SizeLength::points(810.0f));
    appRoot.style().setPadding(LayoutEdge::All, Style::Length::points(16.0f));
    appRoot.style().setGap(Gutter::All, Style::Length::points(16.0f));

    // -------------------------------------------------------------
    // 1. Navigation Header
    // -------------------------------------------------------------
    Node navHeader;
    navHeader.style().setFlexDirection(FlexDirection::Row);
    navHeader.style().setDimension(Dimension::Height, Style::SizeLength::points(56.0f));
    navHeader.style().setAlignItems(Align::Center);
    navHeader.style().setJustifyContent(Justify::SpaceBetween);
    navHeader.style().setPadding(LayoutEdge::Horizontal, Style::Length::points(16.0f));
    navHeader.style().setPadding(LayoutEdge::Vertical, Style::Length::points(10.0f));

    // Brand Logo
    Node brandLogo;
    brandLogo.style().setDimension(Dimension::Width, Style::SizeLength::points(140.0f));
    brandLogo.style().setDimension(Dimension::Height, Style::SizeLength::points(36.0f));

    // Nav Links Container
    Node navLinks;
    navLinks.style().setFlexDirection(FlexDirection::Row);
    navLinks.style().setGap(Gutter::Column, Style::Length::points(12.0f));
    std::vector<Node> links(4);
    for (size_t i = 0; i < links.size(); ++i) {
        links[i].style().setDimension(Dimension::Width, Style::SizeLength::points(85.0f));
        links[i].style().setDimension(Dimension::Height, Style::SizeLength::points(32.0f));
        navLinks.insertChild(&links[i], i);
    }

    // User Profile Avatar
    Node userAvatar;
    userAvatar.style().setDimension(Dimension::Width, Style::SizeLength::points(40.0f));
    userAvatar.style().setDimension(Dimension::Height, Style::SizeLength::points(40.0f));

    navHeader.insertChild(&brandLogo, 0);
    navHeader.insertChild(&navLinks, 1);
    navHeader.insertChild(&userAvatar, 2);
    appRoot.insertChild(&navHeader, 0);

    // -------------------------------------------------------------
    // 2. Main Body (Sidebar + Content Workspace)
    // -------------------------------------------------------------
    Node mainBody;
    mainBody.style().setFlexDirection(FlexDirection::Row);
    mainBody.style().setFlexGrow(FloatOptional{1.0f});
    mainBody.style().setGap(Gutter::Column, Style::Length::points(16.0f));

    // Sidebar
    Node sidebar;
    sidebar.style().setFlexDirection(FlexDirection::Column);
    sidebar.style().setDimension(Dimension::Width, Style::SizeLength::points(220.0f));
    sidebar.style().setPadding(LayoutEdge::All, Style::Length::points(14.0f));
    sidebar.style().setGap(Gutter::Row, Style::Length::points(10.0f));

    std::vector<Node> sidebarItems(6);
    for (size_t i = 0; i < sidebarItems.size(); ++i) {
        sidebarItems[i].style().setDimension(Dimension::Height, Style::SizeLength::points(42.0f));
        sidebarItems[i].style().setDimension(Dimension::Width, Style::SizeLength::percent(100.0f));
        sidebar.insertChild(&sidebarItems[i], i);
    }
    mainBody.insertChild(&sidebar, 0);

    // Dashboard Content Workspace
    Node contentWorkspace;
    contentWorkspace.style().setFlexDirection(FlexDirection::Column);
    contentWorkspace.style().setFlexGrow(FloatOptional{1.0f});
    contentWorkspace.style().setGap(Gutter::Row, Style::Length::points(16.0f));

    // 2.1 KPI Stats Row (4 summary cards)
    Node statsRow;
    statsRow.style().setFlexDirection(FlexDirection::Row);
    statsRow.style().setDimension(Dimension::Height, Style::SizeLength::points(95.0f));
    statsRow.style().setGap(Gutter::Column, Style::Length::points(14.0f));

    std::vector<Node> statCards(4);
    for (size_t i = 0; i < statCards.size(); ++i) {
        statCards[i].style().setFlexGrow(FloatOptional{1.0f});
        statCards[i].style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));
        statCards[i].style().setPadding(LayoutEdge::All, Style::Length::points(10.0f));
        statsRow.insertChild(&statCards[i], i);
    }
    contentWorkspace.insertChild(&statsRow, 0);

    // 2.2 Middle Grid: Analytics & Activity
    Node middleGrid;
    middleGrid.style().setFlexDirection(FlexDirection::Row);
    middleGrid.style().setFlexGrow(FloatOptional{1.0f});
    middleGrid.style().setGap(Gutter::Column, Style::Length::points(14.0f));

    // Large Chart Card (flexGrow: 2)
    Node chartCard;
    chartCard.style().setFlexGrow(FloatOptional{2.0f});
    chartCard.style().setPadding(LayoutEdge::All, Style::Length::points(14.0f));
    chartCard.style().setFlexDirection(FlexDirection::Column);
    chartCard.style().setJustifyContent(Justify::SpaceBetween);

    Node chartHeader;
    chartHeader.style().setDimension(Dimension::Height, Style::SizeLength::points(28.0f));
    chartCard.insertChild(&chartHeader, 0);

    // Simulated Bar Chart columns
    Node chartBarsContainer;
    chartBarsContainer.style().setFlexDirection(FlexDirection::Row);
    chartBarsContainer.style().setFlexGrow(FloatOptional{1.0f});
    chartBarsContainer.style().setAlignItems(Align::FlexEnd);
    chartBarsContainer.style().setGap(Gutter::Column, Style::Length::points(12.0f));
    chartBarsContainer.style().setPadding(LayoutEdge::Top, Style::Length::points(12.0f));

    const float barHeights[7] = {80.0f, 130.0f, 190.0f, 110.0f, 220.0f, 175.0f, 240.0f};
    std::vector<Node> bars(7);
    for (size_t i = 0; i < bars.size(); ++i) {
        bars[i].style().setFlexGrow(FloatOptional{1.0f});
        bars[i].style().setDimension(Dimension::Height, Style::SizeLength::points(barHeights[i]));
        chartBarsContainer.insertChild(&bars[i], i);
    }
    chartCard.insertChild(&chartBarsContainer, 1);
    middleGrid.insertChild(&chartCard, 0);

    // Activity Feed Card (flexGrow: 1)
    Node activityCard;
    activityCard.style().setFlexGrow(FloatOptional{1.0f});
    activityCard.style().setPadding(LayoutEdge::All, Style::Length::points(14.0f));
    activityCard.style().setFlexDirection(FlexDirection::Column);
    activityCard.style().setGap(Gutter::Row, Style::Length::points(10.0f));

    std::vector<Node> activityRows(5);
    for (size_t i = 0; i < activityRows.size(); ++i) {
        activityRows[i].style().setDimension(Dimension::Height, Style::SizeLength::points(40.0f));
        activityCard.insertChild(&activityRows[i], i);
    }
    middleGrid.insertChild(&activityCard, 1);
    contentWorkspace.insertChild(&middleGrid, 1);

    // 2.3 Bottom Cards Row: Featuring Aspect Ratio & Absolute Badge
    Node bottomRow;
    bottomRow.style().setFlexDirection(FlexDirection::Row);
    bottomRow.style().setDimension(Dimension::Height, Style::SizeLength::points(140.0f));
    bottomRow.style().setGap(Gutter::Column, Style::Length::points(14.0f));

    // Card 1: Standard card
    Node bCard1;
    bCard1.style().setFlexGrow(FloatOptional{1.0f});
    bottomRow.insertChild(&bCard1, 0);

    // Card 2: Aspect Ratio card (16:9)
    Node bCard2;
    bCard2.style().setDimension(Dimension::Height, Style::SizeLength::percent(100.0f));
    bCard2.style().setAspectRatio(FloatOptional{16.0f / 9.0f});
    bottomRow.insertChild(&bCard2, 1);

    // Card 3: Card with Floating Absolute Notification Badge
    Node bCard3;
    bCard3.style().setFlexGrow(FloatOptional{1.0f});
    bCard3.style().setPositionType(PositionType::Relative);

    Node badge;
    badge.style().setPositionType(PositionType::Absolute);
    badge.style().setPosition(LayoutEdge::Top, Style::Length::points(-8.0f));
    badge.style().setPosition(LayoutEdge::Right, Style::Length::points(-8.0f));
    badge.style().setDimension(Dimension::Width, Style::SizeLength::points(26.0f));
    badge.style().setDimension(Dimension::Height, Style::SizeLength::points(26.0f));

    bCard3.insertChild(&badge, 0);
    bottomRow.insertChild(&bCard3, 2);

    contentWorkspace.insertChild(&bottomRow, 2);
    mainBody.insertChild(&contentWorkspace, 1);
    appRoot.insertChild(&mainBody, 1);

    // Compute Layout
    solveFlexLayout(&appRoot, 1240.0f, 810.0f, Direction::LTR);
    print_layout_metrics(&appRoot, "App Dashboard Layout");

    // Render tree to canvas
    const float rootX = 20.0f;
    const float rootY = 20.0f;
    render_layout_tree(canvas, &appRoot, rootX, rootY, 0, 0, false);

    // -------------------------------------------------------------
    // Draw Semantic Typography using Sovereign Vector Text Engine
    // -------------------------------------------------------------

    // Header Typography
    const float headerX = rootX + 16.0f;
    const float headerY = rootY + 16.0f;
    draw_label(canvas, "NISABA CLOUD", headerX + 22.0f, headerY + 16.0f, 16.0f, Color::WHITE);

    const std::vector<std::string> navTitles = {"Overview", "Analytics", "Clusters", "Settings"};
    for (size_t i = 0; i < navTitles.size(); ++i) {
        float lx = headerX + 461.0f + (i * 97.0f) + 12.0f;
        draw_label(canvas, navTitles[i], lx, headerY + 18.0f, 13.0f, Color::from_rgba8(200, 215, 245, 240));
    }
    draw_label(canvas, "JD", headerX + 1162.0f, headerY + 18.0f, 13.0f, Color::WHITE);

    // Sidebar Typography
    const float sidebarX = rootX + 16.0f;
    const float sidebarY = rootY + 88.0f;
    const std::vector<std::string> sidebarTitles = {
        "Dashboard", "Microservices", "Telemetry", "Team Members", "API Gateways", "Preferences"
    };
    for (size_t i = 0; i < sidebarTitles.size(); ++i) {
        float sy = sidebarY + 14.0f + (i * 52.0f) + 12.0f;
        draw_label(canvas, sidebarTitles[i], sidebarX + 24.0f, sy, 13.0f, Color::WHITE);
    }

    // KPI Stat Cards Typography
    const float contentX = sidebarX + 236.0f;
    const float kpiY = sidebarY;
    struct KpiInfo { const char* label; const char* val; };
    const KpiInfo kpis[4] = {
        {"Total Revenue", "$124,500"},
        {"Active Users", "14,820"},
        {"Uptime SLA", "99.98%"},
        {"Cloud Deploys", "3,210"}
    };
    for (size_t i = 0; i < 4; ++i) {
        float cx = contentX + (i * 247.0f) + 14.0f;
        draw_label(canvas, kpis[i].label, cx, kpiY + 14.0f, 12.0f, Color::from_rgba8(230, 240, 255, 200));
        draw_label(canvas, kpis[i].val, cx, kpiY + 42.0f, 22.0f, Color::WHITE);
    }

    // Chart Card Typography
    const float chartX = contentX;
    const float chartY = kpiY + 111.0f;
    draw_label(canvas, "Network Throughput & QPS Performance", chartX + 16.0f, chartY + 16.0f, 14.0f, Color::from_rgba8(20, 35, 60, 255));

    const std::vector<std::string> days = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    for (size_t i = 0; i < days.size(); ++i) {
        float bx = chartX + 14.0f + (i * 91.0f) + 24.0f;
        draw_label(canvas, days[i], bx, chartY + 400.0f, 11.0f, Color::WHITE);
    }

    // Activity Card Typography
    const float actX = chartX + 667.0f;
    const float actY = chartY;
    const std::vector<std::string> activities = {
        "PR #42 merged into main",
        "Release v2.1.0 published",
        "Cluster autoscaled (16 nodes)",
        "Database backup finished",
        "Security scan: 0 alerts"
    };
    for (size_t i = 0; i < activities.size(); ++i) {
        float ay = actY + 14.0f + (i * 50.0f) + 10.0f;
        draw_label(canvas, activities[i], actX + 16.0f, ay, 12.0f, Color::WHITE);
    }

    // Bottom Row Typography
    const float botY = chartY + 455.0f;
    draw_label(canvas, "System Status: Nominal (All nodes active)", contentX + 16.0f, botY + 24.0f, 13.0f, Color::WHITE);
    draw_label(canvas, "16:9 Aspect Ratio Container", contentX + 362.0f + 16.0f, botY + 24.0f, 13.0f, Color::WHITE);
    draw_label(canvas, "Real-time Notification Alerts", contentX + 624.0f + 16.0f, botY + 24.0f, 13.0f, Color::WHITE);

    // Absolute Badge Label
    draw_label(canvas, "9+", contentX + 972.0f - 18.0f, botY - 3.0f, 11.0f, Color::WHITE);

    const std::string outPath = "layout_complex_dashboard.png";
    if (pixmap->save_png(outPath)) {
        std::cout << "\n[SUCCESS] Rendered visual layout to: " << outPath << std::endl;
    }

    return 0;
}

#include <iostream>
#include <fstream>
#include <vector>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::text;

namespace {

std::string resolve_font(std::string_view filename) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename),
        std::string("../../fonts/") + std::string(filename),
        std::string("fonts/") + std::string(filename),
        std::string("../fonts/") + std::string(filename)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return candidates[0];
}

} // namespace

int main() {
    std::cout << "Creating Nisaba Sovereign Vector & Multilingual Text Engine Showcase..." << std::endl;

    const uint32_t W = 1000;
    const uint32_t H = 720;

    auto pixmap = Pixmap::create(W, H);
    if (!pixmap) {
        std::cerr << "Failed to allocate pixmap." << std::endl;
        return 1;
    }

    Canvas canvas(*pixmap);

    // 1. Sleek dark background with vertical gradient
    std::vector<GradientStop> bg_stops = {
        GradientStop::create(0.0f, Color::from_rgba8(14, 18, 28, 255)),
        GradientStop::create(1.0f, Color::from_rgba8(8, 10, 16, 255))
    };
    auto bg_grad = LinearGradient::create(
        Point::from_xy(0.0f, 0.0f),
        Point::from_xy(0.0f, static_cast<float>(H)),
        bg_stops
    );

    Paint bg_paint;
    if (bg_grad) {
        bg_paint.shader = Shader(*bg_grad);
    } else {
        bg_paint.set_color_rgba8(14, 18, 28, 255);
    }
    canvas.fill_rect(*Rect::from_xywh(0.0f, 0.0f, static_cast<float>(W), static_cast<float>(H)), bg_paint);

    // 2. Initialize Sovereign Font System & Cache with Full Multilingual Suite
    FontSystem font_system;
    font_system.load_font_file(resolve_font("Inter-Regular.ttf"));
    font_system.load_font_file(resolve_font("NotoSansArabic.ttf"));
    font_system.load_font_file(resolve_font("DroidSansFallbackFull.ttf")); // Full CJK + Cyrillic
    font_system.load_font_file(resolve_font("FiraMono-Medium.ttf"));

    GlyphCache cache;
    Stroke border_stroke(1.0f);

    // 3. Header Card
    Paint card_paint;
    card_paint.set_color_rgba8(22, 28, 42, 220);
    canvas.fill_rect(*Rect::from_xywh(40.0f, 30.0f, 920.0f, 130.0f), card_paint);

    Paint card_border;
    card_border.set_color_rgba8(40, 52, 78, 255);
    Stroke header_stroke(1.5f);
    canvas.stroke_rect(*Rect::from_xywh(40.0f, 30.0f, 920.0f, 130.0f), card_border, header_stroke);

    // Title Buffer
    Buffer title_buf(Metrics(26.0f, 34.0f));
    Attrs title_attrs;
    title_attrs.set_color(TextColor::rgb(0, 220, 255)); // Cyan
    title_buf.set_text("NISABA 2D GRAPHICS & MULTILINGUAL TEXT ENGINE", title_attrs);
    title_buf.draw(canvas, cache, font_system, Color::WHITE, 70.0f, 48.0f);

    // Subtitle Buffer
    Buffer sub_buf(Metrics(14.0f, 20.0f));
    Attrs sub_attrs;
    sub_attrs.set_color(TextColor::rgb(160, 175, 205));
    sub_buf.set_text("100% Sovereign C++20 • Zero External Dependencies • Native Arabic, CJK, Russian & Robotics/Embedded Support", sub_attrs);
    sub_buf.draw(canvas, cache, font_system, Color::WHITE, 70.0f, 95.0f);

    // 4. Arabic Typography Card (Right column / RTL focus)
    Paint arabic_card;
    arabic_card.set_color_rgba8(20, 26, 38, 220);
    canvas.fill_rect(*Rect::from_xywh(40.0f, 180.0f, 445.0f, 320.0f), arabic_card);
    canvas.stroke_rect(*Rect::from_xywh(40.0f, 180.0f, 445.0f, 320.0f), card_border, border_stroke);

    Buffer arabic_title(Metrics(19.0f, 26.0f));
    Attrs ar_t_attrs;
    ar_t_attrs.set_color(TextColor::rgb(255, 180, 50)); // Amber
    arabic_title.set_text("الرندرة العربية السيادية والأنظمة المدمجة", ar_t_attrs);
    arabic_title.draw(canvas, cache, font_system, Color::WHITE, 60.0f, 195.0f);

    Buffer arabic_body(Metrics(15.0f, 26.0f));
    arabic_body.set_size(405.0f, std::nullopt);
    arabic_body.set_wrap(Wrap::Word);
    arabic_body.set_align(Align::Right);

    Attrs ar_body_attrs;
    ar_body_attrs.set_color(TextColor::rgb(230, 235, 245));
    arabic_body.set_text(
        "مرحباً بكم في محرك نيسابا السيادي من منظمة vaxp.\n"
        "تم تصميم النواة لتعمل بكفاءة فائقة على روبوتات التحكم والأنظمة المدمجة وشاشات العرض المباشر DRM/KMS.\n"
        "يدعم المحرك الاتجاهية الثنائية BiDi والتشكيل السياقي للحروف ومركبات اللام-ألف دون أي مكتبة خارجية.",
        ar_body_attrs
    );
    arabic_body.draw(canvas, cache, font_system, Color::WHITE, 60.0f, 235.0f);

    // 5. Global Multilingual Card (Russian, Japanese, Chinese, English)
    Paint wrap_card;
    wrap_card.set_color_rgba8(20, 26, 38, 220);
    canvas.fill_rect(*Rect::from_xywh(515.0f, 180.0f, 445.0f, 320.0f), wrap_card);
    canvas.stroke_rect(*Rect::from_xywh(515.0f, 180.0f, 445.0f, 320.0f), card_border, border_stroke);

    Buffer eng_title(Metrics(19.0f, 26.0f));
    Attrs en_t_attrs;
    en_t_attrs.set_color(TextColor::rgb(100, 230, 150)); // Emerald
    eng_title.set_text("Global Embedded: CJK & Cyrillic", en_t_attrs);
    eng_title.draw(canvas, cache, font_system, Color::WHITE, 535.0f, 195.0f);

    Buffer multi_body(Metrics(14.0f, 23.0f));
    multi_body.set_size(405.0f, std::nullopt);
    multi_body.set_wrap(Wrap::Word);

    Attrs multi_attrs;
    multi_attrs.set_color(TextColor::rgb(220, 230, 242));
    multi_body.set_text(
        "• Русский: Робототехника и бортовые системы реального времени.\n"
        "• 日本語: ロボット工学と組み込みシステム制御、自動改行対応。\n"
        "• 中文: 智能工业机器人操作系统与高精度亚像素抗锯齿。\n"
        "• English: Autonomous Navigation & Sensor HUD.",
        multi_attrs
    );
    multi_body.draw(canvas, cache, font_system, Color::WHITE, 535.0f, 235.0f);

    // Highlight / Status Caret Badge
    Paint sel_paint;
    sel_paint.set_color_rgba8(0, 120, 215, 130);
    canvas.fill_rect(*Rect::from_xywh(535.0f, 445.0f, 260.0f, 26.0f), sel_paint);

    Buffer sel_text(Metrics(14.0f, 24.0f));
    Attrs sel_attrs;
    sel_attrs.set_color(TextColor::rgb(255, 255, 255));
    sel_text.set_text("Active Multilingual Telemetry |", sel_attrs);
    sel_text.draw(canvas, cache, font_system, Color::WHITE, 545.0f, 447.0f);

    // 6. Monospace Code Card at Bottom
    Paint code_card;
    code_card.set_color_rgba8(16, 20, 30, 240);
    canvas.fill_rect(*Rect::from_xywh(40.0f, 520.0f, 920.0f, 170.0f), code_card);
    canvas.stroke_rect(*Rect::from_xywh(40.0f, 520.0f, 920.0f, 170.0f), card_border, border_stroke);

    Buffer code_buf(Metrics(13.5f, 21.0f));
    Attrs code_attrs;
    code_attrs.set_family(Family::monospace()).set_color(TextColor::rgb(240, 160, 200));
    code_buf.set_text(
        "// Sovereign C++20 Multilingual Robotics Integration:\n"
        "nisaba::text::FontSystem font_system;\n"
        "font_system.load_font_file(\"fonts/Inter-Regular.ttf\");\n"
        "font_system.load_font_file(\"fonts/NotoSansArabic.ttf\");\n"
        "font_system.load_font_file(\"fonts/DroidSansFallbackFull.ttf\"); // CJK & Cyrillic\n"
        "nisaba::text::Buffer buffer(Metrics(16.0f, 22.0f));\n"
        "buffer.set_text(\"Русский, 日本語, 中文, العربية & English in a single buffer!\");\n"
        "buffer.draw(canvas, cache, font_system, Color::WHITE, 100.0f, 100.0f);",
        code_attrs
    );
    code_buf.draw(canvas, cache, font_system, Color::WHITE, 60.0f, 530.0f);

    // Save image
    std::string out_filename = "nisaba_text_showcase.bmp";
    pixmap->save_bmp(out_filename);
    pixmap->save_png("nisaba_text_showcase.png");
    std::cout << "Successfully generated and saved: " << out_filename << " and nisaba_text_showcase.png" << std::endl;

    return 0;
}

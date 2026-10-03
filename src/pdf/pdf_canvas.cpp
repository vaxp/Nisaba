#include "nisaba/pdf/pdf_canvas.hpp"
#include "nisaba/path/path_builder.hpp"
#include <cstdio>
#include <cmath>

namespace nisaba::pdf {

PdfCanvas::PdfCanvas(float page_width, float page_height)
    : width_(page_width), height_(page_height) {
    // Establish Nisaba top-down coordinate space:
    // CTM = [1 0 0 -1 0 page_height]
    char buf[64];
    snprintf(buf, sizeof(buf), "1 0 0 -1 0 %.2f cm\n", height_);
    stream_ += buf;
}

void PdfCanvas::save() {
    stream_ += "q\n";
}

void PdfCanvas::restore() {
    stream_ += "Q\n";
}

void PdfCanvas::translate(float dx, float dy) {
    char buf[64];
    snprintf(buf, sizeof(buf), "1 0 0 1 %.2f %.2f cm\n", dx, dy);
    stream_ += buf;
}

void PdfCanvas::scale(float sx, float sy) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%.4f 0 0 %.4f 0 0 cm\n", sx, sy);
    stream_ += buf;
}

void PdfCanvas::concat(const Transform& ts) {
    char buf[96];
    snprintf(buf, sizeof(buf), "%.4f %.4f %.4f %.4f %.2f %.2f cm\n",
             ts.sx, ts.ky, ts.kx, ts.sy, ts.tx, ts.ty);
    stream_ += buf;
}

void PdfCanvas::apply_fill_paint(const Paint& paint) {
    Color color = paint.shader.is_solid_color() ? paint.shader.solid_color() : paint.shader.sample(0.0f, 0.0f);
    float a = color.alpha();
    if (a < 0.999f) {
        std::string gs = get_or_add_extgstate(a);
        stream_ += "/" + gs + " gs\n";
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%.4f %.4f %.4f rg\n",
             color.red(), color.green(), color.blue());
    stream_ += buf;
    current_fill_color_ = color;
}

void PdfCanvas::apply_stroke_paint(const Paint& paint, const Stroke& stroke) {
    Color color = paint.shader.is_solid_color() ? paint.shader.solid_color() : paint.shader.sample(0.0f, 0.0f);
    float a = color.alpha();
    if (a < 0.999f) {
        std::string gs = get_or_add_extgstate(a);
        stream_ += "/" + gs + " gs\n";
    }
    char buf[64];
    snprintf(buf, sizeof(buf), "%.4f %.4f %.4f RG\n",
             color.red(), color.green(), color.blue());
    stream_ += buf;

    // Stroke width
    snprintf(buf, sizeof(buf), "%.2f w\n", stroke.width);
    stream_ += buf;

    // Line cap: Butt = 0, Round = 1, Square = 2
    int cap = 0;
    if (stroke.line_cap == LineCap::Round) cap = 1;
    else if (stroke.line_cap == LineCap::Square) cap = 2;
    snprintf(buf, sizeof(buf), "%d J\n", cap);
    stream_ += buf;

    // Line join: Miter = 0, Round = 1, Bevel = 2
    int join = 0;
    if (stroke.line_join == LineJoin::Round) join = 1;
    else if (stroke.line_join == LineJoin::Bevel) join = 2;
    snprintf(buf, sizeof(buf), "%d j\n", join);
    stream_ += buf;

    // Miter limit
    snprintf(buf, sizeof(buf), "%.2f M\n", stroke.miter_limit);
    stream_ += buf;

    // Dash pattern
    if (stroke.dash.has_value() && !stroke.dash->array().empty()) {
        std::string dash = "[";
        for (size_t i = 0; i < stroke.dash->array().size(); ++i) {
            if (i > 0) dash += ' ';
            char dbuf[32];
            snprintf(dbuf, sizeof(dbuf), "%.2f", stroke.dash->array()[i]);
            dash += dbuf;
        }
        char dbuf[32];
        snprintf(dbuf, sizeof(dbuf), "] %.2f d\n", stroke.dash->offset());
        dash += dbuf;
        stream_ += dash;
    } else {
        stream_ += "[] 0 d\n";
    }
}

void PdfCanvas::emit_path_commands(const Path& path) {
    char buf[128];
    Point last_pt(0, 0);

    auto iter = path.segments();
    while (auto seg_opt = iter.next()) {
        const auto& seg = *seg_opt;
        switch (seg.type) {
            case PathSegment::Type::MoveTo:
                snprintf(buf, sizeof(buf), "%.2f %.2f m\n", seg.p0.x, seg.p0.y);
                stream_ += buf;
                last_pt = seg.p0;
                break;
            case PathSegment::Type::LineTo:
                snprintf(buf, sizeof(buf), "%.2f %.2f l\n", seg.p0.x, seg.p0.y);
                stream_ += buf;
                last_pt = seg.p0;
                break;
            case PathSegment::Type::QuadTo: {
                constexpr float k = 2.0f / 3.0f;
                Point cp1(last_pt.x + (seg.p0.x - last_pt.x) * k, last_pt.y + (seg.p0.y - last_pt.y) * k);
                Point cp2(seg.p1.x + (seg.p0.x - seg.p1.x) * k, seg.p1.y + (seg.p0.y - seg.p1.y) * k);
                snprintf(buf, sizeof(buf), "%.2f %.2f %.2f %.2f %.2f %.2f c\n",
                         cp1.x, cp1.y, cp2.x, cp2.y, seg.p1.x, seg.p1.y);
                stream_ += buf;
                last_pt = seg.p1;
                break;
            }
            case PathSegment::Type::CubicTo:
                snprintf(buf, sizeof(buf), "%.2f %.2f %.2f %.2f %.2f %.2f c\n",
                         seg.p0.x, seg.p0.y, seg.p1.x, seg.p1.y, seg.p2.x, seg.p2.y);
                stream_ += buf;
                last_pt = seg.p2;
                break;
            case PathSegment::Type::Close:
                stream_ += "h\n";
                break;
        }
    }
}

void PdfCanvas::fill_rect(const Rect& rect, const Paint& paint) {
    apply_fill_paint(paint);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %.2f %.2f %.2f re f\n",
             rect.left(), rect.top(), rect.width(), rect.height());
    stream_ += buf;
}

void PdfCanvas::fill_path(const Path& path, const Paint& paint, FillRule fill_rule) {
    if (path.is_empty()) return;
    apply_fill_paint(paint);
    emit_path_commands(path);
    if (fill_rule == FillRule::EvenOdd) {
        stream_ += "f*\n";
    } else {
        stream_ += "f\n";
    }
}

void PdfCanvas::stroke_path(const Path& path, const Paint& paint, const Stroke& stroke) {
    if (path.is_empty()) return;
    apply_stroke_paint(paint, stroke);
    emit_path_commands(path);
    stream_ += "S\n";
}

void PdfCanvas::stroke_rect(const Rect& rect, const Paint& paint, const Stroke& stroke) {
    apply_stroke_paint(paint, stroke);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %.2f %.2f %.2f re S\n",
             rect.left(), rect.top(), rect.width(), rect.height());
    stream_ += buf;
}

void PdfCanvas::draw_line(Point p1, Point p2, const Paint& paint, const Stroke& stroke) {
    apply_stroke_paint(paint, stroke);
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f %.2f m %.2f %.2f l S\n",
             p1.x, p1.y, p2.x, p2.y);
    stream_ += buf;
}

void PdfCanvas::draw_circle(Point center, float radius, const Paint& paint) {
    // Approximate circle with 4 cubic Béziers
    const float k = 0.5522847498f * radius;
    PathBuilder pb;
    pb.move_to(center.x, center.y - radius);
    pb.cubic_to(center.x + k, center.y - radius, center.x + radius, center.y - k, center.x + radius, center.y);
    pb.cubic_to(center.x + radius, center.y + k, center.x + k, center.y + radius, center.x, center.y + radius);
    pb.cubic_to(center.x - k, center.y + radius, center.x - radius, center.y + k, center.x - radius, center.y);
    pb.cubic_to(center.x - radius, center.y - k, center.x - k, center.y - radius, center.x, center.y - radius);
    pb.close();
    auto p = pb.finish();
    if (p) fill_path(*p, paint);
}

void PdfCanvas::stroke_circle(Point center, float radius, const Paint& paint, const Stroke& stroke) {
    const float k = 0.5522847498f * radius;
    PathBuilder pb;
    pb.move_to(center.x, center.y - radius);
    pb.cubic_to(center.x + k, center.y - radius, center.x + radius, center.y - k, center.x + radius, center.y);
    pb.cubic_to(center.x + radius, center.y + k, center.x + k, center.y + radius, center.x, center.y + radius);
    pb.cubic_to(center.x - k, center.y + radius, center.x - radius, center.y + k, center.x - radius, center.y);
    pb.cubic_to(center.x - radius, center.y - k, center.x - k, center.y - radius, center.x, center.y - radius);
    pb.close();
    auto p = pb.finish();
    if (p) stroke_path(*p, paint, stroke);
}

std::string PdfCanvas::get_or_add_font(std::string_view font_name) {
    for (const auto& f : fonts_) {
        if (f.base_font == font_name) {
            return f.resource_name;
        }
    }
    std::string res_name = "F" + std::to_string(fonts_.size() + 1);
    fonts_.push_back(PdfFontResource{
        .resource_name = res_name,
        .base_font = std::string(font_name)
    });
    return res_name;
}

std::string PdfCanvas::get_or_add_extgstate(float alpha) {
    int key = static_cast<int>(std::round(alpha * 1000.0f));
    auto it = extgstates_.find(key);
    if (it != extgstates_.end()) {
        return it->second;
    }
    std::string name = "GS" + std::to_string(extgstates_.size() + 1);
    extgstates_[key] = name;
    return name;
}

void PdfCanvas::draw_text(
    std::string_view text,
    float x,
    float y,
    float font_size,
    Color color,
    std::string_view font_name
) {
    if (text.empty()) return;
    std::string font_res = get_or_add_font(font_name);
    apply_fill_paint(Paint(color));

    std::string escaped = escape_pdf_string(text);
    // In our flipped CTM ([1 0 0 -1 0 H]), we use Tm [1 0 0 -1 x baseline]
    // to invert Y locally for the glyphs so they render upright!
    char buf[256];
    float baseline = y + font_size * 0.8f;
    snprintf(buf, sizeof(buf), "BT\n/%s %.2f Tf\n1 0 0 -1 %.2f %.2f Tm\n(%s) Tj\nET\n",
             font_res.c_str(), font_size, x, baseline, escaped.c_str());
    stream_ += buf;
}

void PdfCanvas::draw_pixmap(const Pixmap& pixmap, const Rect& dest_rect) {
    if (pixmap.width() == 0 || pixmap.height() == 0) return;

    std::string res_name = "Im" + std::to_string(images_.size() + 1);
    images_.push_back(PdfImageResource{
        .name = res_name,
        .width = pixmap.width(),
        .height = pixmap.height(),
        .data = std::vector<uint8_t>(pixmap.data(), pixmap.data() + pixmap.data_len())
    });

    // In PDF, an image XObject fills the unit square [0,0]x[1,1] with origin at bottom-left.
    // In our top-down CTM, we map [0,1] to [dest.top, dest.bottom]:
    // Matrix [dest.w, 0, 0, -dest.h, dest.x, dest.bottom]
    char buf[128];
    snprintf(buf, sizeof(buf), "q\n%.2f 0 0 -%.2f %.2f %.2f cm\n/%s Do\nQ\n",
             dest_rect.width(), dest_rect.height(),
             dest_rect.left(), dest_rect.bottom(),
             res_name.c_str());
    stream_ += buf;
}

} // namespace nisaba::pdf

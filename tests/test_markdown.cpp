#include <cassert>
#include <iostream>
#include <string>
#include "nisaba/nisaba.hpp"

using namespace nisaba;
using namespace nisaba::markdown;

void test_markdown_parser_blocks() {
    std::cout << "[Test] MarkdownParser: Block elements..." << std::endl;

    std::string md = 
        "# Heading 1\n"
        "## Heading 2\n"
        "### Heading 3\n"
        "#### Heading 4\n"
        "##### Heading 5\n"
        "###### Heading 6\n"
        "\n"
        "---\n"
        "\n"
        "This is a standard paragraph of text.\n"
        "It spans across two lines.\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->type == BlockType::Document);
    assert(doc->children.size() == 8); // 6 headings + 1 divider + 1 paragraph

    // Verify headings
    for (int i = 0; i < 6; ++i) {
        assert(doc->children[i]->type == BlockType::Heading);
        assert(doc->children[i]->heading_level == (i + 1));
        assert(!doc->children[i]->inlines.empty());
    }

    // Verify thematic break
    assert(doc->children[6]->type == BlockType::ThematicBreak);

    // Verify paragraph
    assert(doc->children[7]->type == BlockType::Paragraph);
    assert(!doc->children[7]->inlines.empty());

    std::cout << "  Block elements verified." << std::endl;
}

void test_markdown_inlines() {
    std::cout << "[Test] MarkdownParser: Inlines & formatting..." << std::endl;

    std::string text = "Hello **bold** and *italic* and ***bold italic*** and `code` and ~~strike~~ and [link](https://example.com) and ![img](pic.png)";
    auto spans = MarkdownParser::parse_inlines(text);

    assert(!spans.empty());
    bool found_bold = false;
    bool found_italic = false;
    bool found_bold_italic = false;
    bool found_code = false;
    bool found_strike = false;
    bool found_link = false;
    bool found_image = false;

    for (const auto& s : spans) {
        if (s.type == InlineType::Bold && s.text == "bold") found_bold = true;
        if (s.type == InlineType::Italic && s.text == "italic") found_italic = true;
        if (s.type == InlineType::BoldItalic && s.text == "bold italic") found_bold_italic = true;
        if (s.type == InlineType::CodeSpan && s.text == "code") found_code = true;
        if (s.type == InlineType::Strikethrough && s.text == "strike") found_strike = true;
        if (s.type == InlineType::Link && s.text == "link" && s.target == "https://example.com") found_link = true;
        if (s.type == InlineType::Image && s.text == "img" && s.target == "pic.png") found_image = true;
    }

    assert(found_bold);
    assert(found_italic);
    assert(found_bold_italic);
    assert(found_code);
    assert(found_strike);
    assert(found_link);
    assert(found_image);

    std::cout << "  Inline formatting verified." << std::endl;
}

void test_markdown_code_blocks() {
    std::cout << "[Test] MarkdownParser: Fenced code blocks..." << std::endl;

    std::string md = 
        "```cpp\n"
        "#include <iostream>\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n"
        "```\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->children.size() == 1);
    const auto& cb = *doc->children[0];
    assert(cb.type == BlockType::CodeBlock);
    assert(cb.language == "cpp");
    assert(cb.code_lines.size() == 4);
    assert(cb.code_lines[0] == "#include <iostream>");
    assert(cb.code_lines[3] == "}");

    std::cout << "  Fenced code block verified." << std::endl;
}

void test_markdown_lists() {
    std::cout << "[Test] MarkdownParser: Lists and GFM task lists..." << std::endl;

    std::string md = 
        "- Item 1\n"
        "- Item 2\n"
        "- [ ] Unchecked task\n"
        "- [x] Checked task\n"
        "\n"
        "1. First\n"
        "2. Second\n"
        "3. Third\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->children.size() == 2); // 1 unordered list + 1 ordered list

    // Unordered list
    const auto& ul = *doc->children[0];
    assert(ul.type == BlockType::List);
    assert(!ul.is_ordered_list);
    assert(ul.children.size() == 4);
    assert(!ul.children[0]->is_task_item);
    assert(ul.children[2]->is_task_item && !ul.children[2]->is_task_checked);
    assert(ul.children[3]->is_task_item && ul.children[3]->is_task_checked);

    // Ordered list
    const auto& ol = *doc->children[1];
    assert(ol.type == BlockType::List);
    assert(ol.is_ordered_list);
    assert(ol.children.size() == 3);
    assert(ol.list_start_number == 1);

    std::cout << "  Lists and task lists verified." << std::endl;
}

void test_markdown_tables() {
    std::cout << "[Test] MarkdownParser: GFM Tables..." << std::endl;

    std::string md = 
        "| Name | Role | Score |\n"
        "| :--- | :--: | ----: |\n"
        "| Alice | Admin | 95 |\n"
        "| Bob | User | 82 |\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->children.size() == 1);

    const auto& tbl = *doc->children[0];
    assert(tbl.type == BlockType::Table);
    assert(tbl.table_alignments.size() == 3);
    assert(tbl.table_alignments[0] == TableAlign::Left);
    assert(tbl.table_alignments[1] == TableAlign::Center);
    assert(tbl.table_alignments[2] == TableAlign::Right);

    assert(tbl.table_rows.size() == 3); // 1 header + 2 data rows
    assert(tbl.table_rows[0].is_header);
    assert(!tbl.table_rows[1].is_header);
    assert(tbl.table_rows[1].cells.size() == 3);

    std::cout << "  GFM Tables verified." << std::endl;
}

void test_markdown_blockquotes() {
    std::cout << "[Test] MarkdownParser: BlockQuotes..." << std::endl;

    std::string md = 
        "> This is a blockquote.\n"
        "> Continued on second line.\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->children.size() == 1);

    const auto& bq = *doc->children[0];
    assert(bq.type == BlockType::BlockQuote);
    assert(!bq.children.empty());

    std::cout << "  BlockQuotes verified." << std::endl;
}

void test_markdown_document_and_render() {
    std::cout << "[Test] MarkdownDocument and MarkdownRenderer..." << std::endl;

    std::string md = 
        "# Welcome to Nisaba Markdown\n"
        "\n"
        "Nisaba provides **ultra-fast**, sovereign 2D document rendering.\n"
        "\n"
        "```cpp\n"
        "// Embedded C++20 code block\n"
        "auto pixmap = Pixmap::create(800, 600);\n"
        "```\n"
        "\n"
        "- [x] High performance software rasterizer\n"
        "- [x] Zero external dependencies\n"
        "- [ ] Native PDF export support\n"
        "\n"
        "| Feature | Speed | Status |\n"
        "| :--- | :---: | ---: |\n"
        "| Vector Core | 7.5x Cairo | Active |\n"
        "| Markdown | O(1) SIMD | Active |\n";

    auto doc = MarkdownDocument::from_string(md);
    assert(doc.is_valid());

    auto style = MarkdownStyle::dark_theme();
    text::FontSystem font_system;
    text::GlyphCache glyph_cache;

    float height = doc.layout(700.0f, style, font_system, glyph_cache);
    assert(height > 100.0f);
    assert(doc.total_height() == height);

    // Allocate pixmap and render document
    auto pixmap = Pixmap::create(800, static_cast<uint32_t>(height + 40.0f));
    assert(pixmap.has_value());
    Canvas canvas(*pixmap);
    canvas.clear(Color::from_rgba8(10, 14, 24, 255));

    MarkdownRenderer::render(canvas, doc, 50.0f, 20.0f, 700.0f, style, font_system, glyph_cache);

    std::cout << "  Document layout and render verified. Computed height: " << height << "px." << std::endl;
}

void test_markdown_html_tags() {
    std::cout << "[Test] Markdown: HTML Tags (Inline & Block)..." << std::endl;

    std::string inline_html = "Press <kbd>Ctrl</kbd> + <kbd>C</kbd> or use H<sub>2</sub>O and E=mc<sup>2</sup> with <mark>highlight</mark> and <u>underline</u> and <br> newline.";
    auto spans = MarkdownParser::parse_inlines(inline_html);

    bool found_kbd = false;
    bool found_sub = false;
    bool found_sup = false;
    bool found_mark = false;
    bool found_u = false;
    bool found_br = false;

    for (const auto& s : spans) {
        if (s.type == InlineType::Kbd && s.text == "Ctrl") found_kbd = true;
        if (s.type == InlineType::Subscript && s.text == "2") found_sub = true;
        if (s.type == InlineType::Superscript && s.text == "2") found_sup = true;
        if (s.type == InlineType::Highlight && s.text == "highlight") found_mark = true;
        if (s.type == InlineType::Underline && s.text == "underline") found_u = true;
        if (s.type == InlineType::LineBreak) found_br = true;
    }

    assert(found_kbd);
    assert(found_sub);
    assert(found_sup);
    assert(found_mark);
    assert(found_u);
    assert(found_br);

    // Block HTML: <details> and <summary>
    std::string block_html = 
        "<details open>\n"
        "<summary>Advanced Configuration</summary>\n"
        "Here is the collapsed or expanded content.\n"
        "</details>\n";

    auto doc = MarkdownParser::parse(block_html);
    assert(doc != nullptr);
    assert(doc->children.size() == 1);
    const auto& details = *doc->children[0];
    assert(details.type == BlockType::Details);
    assert(details.is_open);
    assert(details.summary_text == "Advanced Configuration");
    assert(!details.children.empty());

    std::cout << "  HTML tags (Inline <kbd>, <sub>, <sup>, <mark>, <br> & Block <details>) verified." << std::endl;
}

void test_markdown_github_alerts() {
    std::cout << "[Test] Markdown: GitHub Callout Alerts..." << std::endl;

    std::string md = 
        "> [!NOTE]\n"
        "> Useful information that users should know.\n"
        "\n"
        "> [!TIP]\n"
        "> Helpful advice for doing things better.\n"
        "\n"
        "> [!IMPORTANT]\n"
        "> Key information users need to know.\n"
        "\n"
        "> [!WARNING]\n"
        "> Urgent info that needs immediate attention.\n"
        "\n"
        "> [!CAUTION]\n"
        "> Advises about risks or negative outcomes.\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->children.size() == 5);

    assert(doc->children[0]->type == BlockType::Alert && doc->children[0]->alert_type == AlertType::Note);
    assert(doc->children[1]->type == BlockType::Alert && doc->children[1]->alert_type == AlertType::Tip);
    assert(doc->children[2]->type == BlockType::Alert && doc->children[2]->alert_type == AlertType::Important);
    assert(doc->children[3]->type == BlockType::Alert && doc->children[3]->alert_type == AlertType::Warning);
    assert(doc->children[4]->type == BlockType::Alert && doc->children[4]->alert_type == AlertType::Caution);

    std::cout << "  All 5 GitHub Callout Alerts verified ([!NOTE], [!TIP], [!IMPORTANT], [!WARNING], [!CAUTION])." << std::endl;
}

void test_markdown_syntax_highlighter() {
    std::cout << "[Test] Markdown: Syntax Highlighting Lexer..." << std::endl;

    bool in_comment = false;
    auto cpp_tokens = SyntaxHighlighter::tokenize_line("constexpr uint32_t count = 42; // Sovereign", "cpp", in_comment);
    assert(!cpp_tokens.empty());

    bool found_keyword = false;
    bool found_type = false;
    bool found_number = false;
    bool found_comment = false;

    for (const auto& tok : cpp_tokens) {
        if (tok.type == TokenType::Keyword && tok.text == "constexpr") found_keyword = true;
        if (tok.type == TokenType::Type && tok.text == "uint32_t") found_type = true;
        if (tok.type == TokenType::NumberLiteral && tok.text == "42") found_number = true;
        if (tok.type == TokenType::Comment) found_comment = true;
    }

    assert(found_keyword);
    assert(found_type);
    assert(found_number);
    assert(found_comment);

    // Python test
    in_comment = false;
    auto py_tokens = SyntaxHighlighter::tokenize_line("def render_frame(time_sec): return True", "python", in_comment);
    bool found_py_def = false;
    bool found_py_return = false;
    for (const auto& tok : py_tokens) {
        if (tok.type == TokenType::Keyword && tok.text == "def") found_py_def = true;
        if (tok.type == TokenType::Keyword && tok.text == "return") found_py_return = true;
    }
    assert(found_py_def && found_py_return);

    // Rust test
    in_comment = false;
    auto rust_tokens = SyntaxHighlighter::tokenize_line("pub fn create_buffer(mut len: usize) -> Result", "rust", in_comment);
    bool found_rust_fn = false;
    bool found_rust_pub = false;
    for (const auto& tok : rust_tokens) {
        if (tok.type == TokenType::Keyword && tok.text == "pub") found_rust_pub = true;
        if (tok.type == TokenType::Keyword && tok.text == "fn") found_rust_fn = true;
    }
    assert(found_rust_pub && found_rust_fn);

    // CMake test
    in_comment = false;
    auto cmake_tokens = SyntaxHighlighter::tokenize_line("project(Nisaba VERSION 2.0)", "cmake", in_comment);
    bool found_cmake_proj = false;
    for (const auto& tok : cmake_tokens) {
        if (tok.type == TokenType::Keyword && tok.text == "project") found_cmake_proj = true;
    }
    assert(found_cmake_proj);

    // HTML test
    in_comment = false;
    auto html_tokens = SyntaxHighlighter::tokenize_line("<div class=\"container\"><!-- test --></div>", "html", in_comment);
    bool found_html_tag = false;
    bool found_html_attr = false;
    bool found_html_cmt = false;
    for (const auto& tok : html_tokens) {
        if (tok.type == TokenType::Keyword && tok.text == "div") found_html_tag = true;
        if (tok.type == TokenType::Type && tok.text == "class") found_html_attr = true;
        if (tok.type == TokenType::Comment) found_html_cmt = true;
    }
    assert(found_html_tag && found_html_attr && found_html_cmt);

    // Theme color verification
    auto dark_st = MarkdownStyle::dark_theme();
    auto light_st = MarkdownStyle::light_theme();
    assert(SyntaxHighlighter::token_color(TokenType::Keyword, dark_st) == dark_st.syn_keyword);
    assert(SyntaxHighlighter::token_color(TokenType::Keyword, light_st) == light_st.syn_keyword);
    assert(SyntaxHighlighter::token_color(TokenType::Comment, dark_st) == dark_st.syn_comment);

    std::cout << "  Syntax Highlighting Lexer verified for C++, Python, Rust, CMake, HTML and themes." << std::endl;
}

void test_markdown_link_reference_definitions() {
    std::cout << "[Test] Markdown: Link Reference Definitions..." << std::endl;

    std::string md = 
        "Read the [documentation][docs] or visit [GitHub] directly.\n"
        "\n"
        "[docs]: https://nisaba.dev/docs \"Nisaba Docs\"\n"
        "[github]: https://github.com/vaxp/nisaba\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    // Link reference lines should be consumed into ref_map and not emitted as blocks
    assert(doc->children.size() == 1);
    assert(doc->children[0]->type == BlockType::Paragraph);

    const auto& inlines = doc->children[0]->inlines;
    bool found_docs_link = false;
    bool found_gh_link = false;

    for (const auto& s : inlines) {
        if (s.type == InlineType::Link && s.text == "documentation" && s.target == "https://nisaba.dev/docs") {
            found_docs_link = true;
        }
        if (s.type == InlineType::Link && s.text == "GitHub" && s.target == "https://github.com/vaxp/nisaba") {
            found_gh_link = true;
        }
    }

    assert(found_docs_link);
    assert(found_gh_link);

    std::cout << "  Link Reference Definitions verified." << std::endl;
}

void test_markdown_emojis_and_autolinks() {
    std::cout << "[Test] Markdown: Emojis and Autolinks..." << std::endl;

    std::string text = "Launch :rocket: and celebrate :tada: with :sparkles: Visit https://github.com/vaxp/nisaba for code!";
    auto spans = MarkdownParser::parse_inlines(text);

    bool found_rocket = false;
    bool found_tada = false;
    bool found_autolink = false;

    for (const auto& s : spans) {
        if (s.text.find("🚀") != std::string::npos) found_rocket = true;
        if (s.text.find("🎉") != std::string::npos) found_tada = true;
        if (s.type == InlineType::Link && s.target == "https://github.com/vaxp/nisaba") found_autolink = true;
    }

    assert(found_rocket);
    assert(found_tada);
    assert(found_autolink);

    std::cout << "  Emoji replacement and autolinks verified." << std::endl;
}

void test_markdown_footnotes_and_slugs() {
    std::cout << "[Test] Markdown: Footnotes and Anchor Slugs..." << std::endl;

    // Slugs
    assert(MarkdownParser::slugify("Nisaba 2D Engine: Fast & Sovereign!") == "nisaba-2d-engine-fast-sovereign");
    assert(MarkdownParser::slugify("Architecture & Highlights") == "architecture-highlights");

    // Footnotes
    std::string md = 
        "This is statement with footnote[^1].\n"
        "\n"
        "[^1]: The official Nisaba documentation citation.\n";

    auto doc = MarkdownParser::parse(md);
    assert(doc != nullptr);
    assert(doc->children.size() == 2);

    // Verify FootnoteRef in paragraph
    const auto& para = *doc->children[0];
    bool found_fn_ref = false;
    for (const auto& s : para.inlines) {
        if (s.type == InlineType::FootnoteRef && s.footnote_index == 1) found_fn_ref = true;
    }
    assert(found_fn_ref);

    // Verify FootnoteDef block
    const auto& fn_def = *doc->children[1];
    assert(fn_def.type == BlockType::FootnoteDef);
    assert(fn_def.footnote_index == 1);
    assert(!fn_def.inlines.empty());

    std::cout << "  Footnotes and Anchor Slugs verified." << std::endl;
}

void test_markdown_interactive_hit_test() {
    std::cout << "[Test] Markdown: Interactive Hit-Testing and TOC..." << std::endl;

    std::string md = 
        "# Architecture Overview\n"
        "\n"
        "Check [Official Website](https://vaxp.org) for details.\n"
        "\n"
        "<details open>\n"
        "<summary>Show Engine Telemetry</summary>\n"
        "FPS: 144\n"
        "</details>\n"
        "\n"
        "- [ ] Review pull request\n"
        "- [x] Merge changes\n";

    auto doc = MarkdownDocument::from_string(md);
    assert(doc.is_valid());

    auto style = MarkdownStyle::dark_theme();
    text::FontSystem font_system;
    text::GlyphCache glyph_cache;
    doc.layout(800.0f, style, font_system, glyph_cache);

    // 1. Table of contents extraction
    auto toc = doc.table_of_contents();
    assert(toc.size() == 1);
    assert(toc[0].first == "Architecture Overview");
    assert(toc[0].second == "architecture-overview");

    // 2. Details toggle interaction
    const auto& details_node = doc.root()->children[2];
    assert(details_node->type == BlockType::Details);
    assert(details_node->is_open);

    bool toggled = doc.toggle_details(details_node.get());
    assert(toggled);
    assert(!details_node->is_open);

    // 3. Task list interaction
    const auto& list_node = doc.root()->children[3];
    assert(list_node->type == BlockType::List);
    auto& item0 = list_node->children[0];
    assert(item0->is_task_item && !item0->is_task_checked);

    bool item_toggled = doc.toggle_task_item(item0.get());
    assert(item_toggled);
    assert(item0->is_task_checked);

    // 4. Test handle_click and get_cursor_at
    std::string clicked_url;
    const auto& link_para = doc.root()->children[1];
    doc.handle_click(10.0f, link_para->layout_y + 5.0f, [&](const std::string& url) {
        clicked_url = url;
    });
    assert(clicked_url == "https://vaxp.org");

    // Click on details summary to toggle it back open
    bool details_reopened = doc.handle_click(10.0f, details_node->layout_y + 5.0f);
    assert(details_reopened);
    assert(details_node->is_open);

    // Cursor shape verification
    auto link_cursor = doc.get_cursor_at(10.0f, link_para->layout_y + 5.0f);
    assert(link_cursor == MarkdownDocument::CursorType::Pointer);

    std::cout << "  Interactive hit testing, TOC, and state toggles verified." << std::endl;
}

void test_markdown_advanced_layout_and_recursion() {
    std::cout << "[Test] Markdown: Recursive Inlines, Loose Lists & Advanced Tables..." << std::endl;

    // 1. Recursive inlines
    std::string inline_md = "Check out [the **sovereign** *Nisaba* graphics](https://nisaba.dev)";
    auto spans = MarkdownParser::parse_inlines(inline_md);
    assert(spans.size() == 2); // "Check out " + Link
    assert(spans[1].type == InlineType::Link);
    assert(!spans[1].children.empty());
    bool found_nested_bold = false;
    for (const auto& ch : spans[1].children) {
        if (ch.type == InlineType::Bold && ch.text == "sovereign") found_nested_bold = true;
    }
    assert(found_nested_bold);

    // 2. Loose lists
    std::string loose_md = 
        "- Item Alpha\n"
        "\n"
        "- Item Beta\n";
    auto doc_loose = MarkdownParser::parse(loose_md);
    assert(doc_loose != nullptr && doc_loose->children.size() == 1);
    const auto& list_node = doc_loose->children[0];
    assert(list_node->type == BlockType::List);
    assert(list_node->is_loose_list);
    assert(list_node->children.size() == 2);

    // 3. Multi-line table cell layout
    std::string table_md = 
        "| Feature | Description |\n"
        "| :--- | :--- |\n"
        "| Renderer | This is a very extensive multi-line description intended to wrap across multiple horizontal lines in the table layout |\n";

    auto doc_table = MarkdownDocument::from_string(table_md);
    assert(doc_table.is_valid());
    auto style = MarkdownStyle::dark_theme();
    text::FontSystem font_system;
    text::GlyphCache glyph_cache;

    doc_table.layout(350.0f, style, font_system, glyph_cache);
    const auto& tbl_block = doc_table.root()->children[0];
    assert(tbl_block->type == BlockType::Table);
    assert(tbl_block->table_rows.size() == 2);

    float default_min_h = style.base_font_size * style.line_height_multiplier + 2.0f * style.table_cell_padding_v;
    // Row 1 (data row) should have wrapped into multiple lines and thus be taller than a single line
    assert(tbl_block->table_rows[1].layout_height > default_min_h);

    std::cout << "  Recursive inlines, loose lists & advanced table wrapping verified." << std::endl;
}

void test_markdown_vector_pdf_exporter() {
    std::cout << "[Test] Markdown: True Vector PDF Export Pipeline..." << std::endl;

    std::string md = 
        "# Nisaba Sovereign Graphics Engine\n"
        "## Vector PDF Document Generation\n"
        "\n"
        "This document is dynamically generated by Nisaba's native PDF vector pipeline.\n"
        "\n"
        "> [!NOTE]\n"
        "> PDF export uses zero third-party dependencies, standard Type 1 fonts, and vector primitives.\n"
        "\n"
        "> [!TIP]\n"
        "> Tables and callout alerts format automatically across page breaks.\n"
        "\n"
        "```cpp\n"
        "// Embedded C++ snippet\n"
        "auto doc = MarkdownDocument::from_string(md);\n"
        "MarkdownPdfExporter::export_to_file(doc, \"nisaba_test_export.pdf\", style, fonts, cache);\n"
        "```\n"
        "\n"
        "- [x] High performance software rasterizer\n"
        "- [x] GitHub callout alerts\n"
        "- [ ] Multi-threaded GPU pipeline\n"
        "\n"
        "| Feature | Speed | Status |\n"
        "| :--- | :---: | ---: |\n"
        "| Vector Core | 7.5x Cairo | Verified |\n"
        "| PDF Exporter | ISO 32000-1 | Verified |\n";

    auto doc = MarkdownDocument::from_string(md);
    assert(doc.is_valid());

    auto style = MarkdownStyle::light_theme();
    text::FontSystem font_system;
    text::GlyphCache glyph_cache;

    MarkdownPdfExportOptions opts;
    opts.show_header = true;
    opts.show_footer = true;
    opts.show_page_numbers = true;
    opts.header_left = "Nisaba Sovereign Engine";
    opts.doc_title = "Nisaba Vector PDF Test";

    auto pdf_bytes = MarkdownPdfExporter::export_to_bytes(doc, style, font_system, glyph_cache, opts);
    assert(!pdf_bytes.empty());
    assert(pdf_bytes.size() > 500);

    // Verify PDF header magic
    std::string_view pdf_view(reinterpret_cast<const char*>(pdf_bytes.data()), pdf_bytes.size());
    assert(pdf_view.starts_with("%PDF-"));

    // Verify PDF EOF marker
    assert(pdf_view.find("%%EOF") != std::string_view::npos);

    // Verify write to file
    bool saved = MarkdownPdfExporter::export_to_file(doc, "nisaba_test_export.pdf", style, font_system, glyph_cache, opts);
    assert(saved);

    std::cout << "  Vector PDF Export Pipeline verified. Generated " << pdf_bytes.size() << " bytes." << std::endl;
}

int main() {
    std::cout << "=== NISABA SOVEREIGN MARKDOWN ENGINE TESTS ===" << std::endl;

    test_markdown_parser_blocks();
    test_markdown_inlines();
    test_markdown_code_blocks();
    test_markdown_lists();
    test_markdown_tables();
    test_markdown_blockquotes();
    test_markdown_document_and_render();

    // New GitHub-Grade Markdown test suites
    test_markdown_html_tags();
    test_markdown_github_alerts();
    test_markdown_syntax_highlighter();
    test_markdown_link_reference_definitions();
    test_markdown_emojis_and_autolinks();
    test_markdown_footnotes_and_slugs();
    test_markdown_interactive_hit_test();
    test_markdown_advanced_layout_and_recursion();
    test_markdown_vector_pdf_exporter();

    std::cout << "\n>>> ALL 16 MARKDOWN TEST SUITES PASSED WITH HIGHEST DISTINCTION! <<<\n" << std::endl;
    return 0;
}

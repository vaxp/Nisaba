#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <fstream>
#include <chrono>

#include "nisaba/nisaba.hpp"

#ifdef NISABA_HAS_BACKEND_OS
#  include "nisaba/backend_os/platform.hpp"
#  include "nisaba/backend_os/window.hpp"
#  include "nisaba/gpu/context.hpp"
#  include "nisaba/gpu/gl3_renderer.hpp"
#  ifdef NISABA_GLEW
#    include <GL/glew.h>
#  else
#    include <GL/gl.h>
#  endif
#endif

using namespace nisaba;
using namespace nisaba::markdown;

static std::string resolve_font_path(std::string_view font_name) {
    std::vector<std::string> candidates = {
        std::string("fonts/") + std::string(font_name),
        std::string("../fonts/") + std::string(font_name),
        std::string("../../fonts/") + std::string(font_name),
        std::string("nisaba/fonts/") + std::string(font_name)
    };
    for (const auto& path : candidates) {
        std::ifstream f(path, std::ios::binary);
        if (f.good()) return path;
    }
    return "";
}

int main(int argc, char** argv) {
    bool run_headless = false;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--headless" || arg == "-h") {
            run_headless = true;
        }
    }

    std::cout << "====================================================================================\n";
    std::cout << "   NISABA 2D GRAPHICS ENGINE — SOVEREIGN MARKDOWN & GFM RENDERING SHOWCASE\n";
    std::cout << "   Full-Featured Document Layout, Typography, Tables, Code Blocks, and Task Lists\n";
    std::cout << "====================================================================================\n";

    // 1. Initialize Font System
    text::FontSystem font_system;
    text::GlyphCache glyph_cache;

    auto inter_path = resolve_font_path("Inter-Regular.ttf");
    auto fira_path = resolve_font_path("FiraMono-Medium.ttf");

    if (!inter_path.empty()) {
        font_system.load_font_file(inter_path);
        std::cout << "[+] Loaded Sans Font: " << inter_path << "\n";
    }
    if (!fira_path.empty()) {
        font_system.load_font_file(fira_path);
        std::cout << "[+] Loaded Monospace Font: " << fira_path << "\n";
    }

    // 2. Comprehensive GitHub-Grade Markdown Document
    std::string markdown_source = 
        "# Nisaba — Full-Stack 2D Graphics Engine :rocket:\n"
        "\n"
        "**Nisaba** is an embedded-first, zero-dependency, ultra-lightweight 2D graphics rendering and layout engine written from scratch in **Modern C++ (C++20)** :sparkles: Visit https://github.com/vaxp/nisaba for updates.\n"
        "\n"
        "> [!NOTE]\n"
        "> Nisaba delivers radical technological sovereignty with zero external dependencies and 100% pure ISO C++20 compliance.\n"
        "\n"
        "> [!TIP]\n"
        "> Press <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>P</kbd> to open Command Palette, or use formula E = mc<sup>2</sup> and chemistry H<sub>2</sub>O with <mark>Hardware Acceleration</mark>.\n"
        "\n"
        "> [!IMPORTANT]\n"
        "> Multi-backend rendering supports native **Vulkan 1.0+** and **OpenGL ES 3.2** with 4x hardware MSAA and anti-banding dithering.\n"
        "\n"
        "> [!WARNING]\n"
        "> Embedded DRM/KMS dumb buffer targets require zero-copy pitch alignment with `PixmapMut::from_raw_parts`.\n"
        "\n"
        "> [!CAUTION]\n"
        "> Do not link third-party windowing libraries (GLFW/SDL). Use Nisaba's native sovereign `backend_os` subsystem.\n"
        "\n"
        "<details open>\n"
        "<summary>Sovereign Hardware GPU & Codec Architecture (Click to Toggle)</summary>\n"
        "\n"
        "The GPU pipeline leverages direct SPIR-V bytecode bytecode shaders with hardware 4x MSAA resolve. Sovereign codecs encode HD QOI in under 7.9 ms :fire:\n"
        "\n"
        "</details>\n"
        "\n"
        "### Key Capabilities & Status [^1]\n"
        "\n"
        "- [x] High-precision subpixel anti-aliasing (256 coverage levels)\n"
        "- [x] Hardware accelerated GPU rendering (Vulkan 1.0+ & OpenGL ES 3.2)\n"
        "- [x] Sovereign image codecs: PNG, JPEG, QOI, Deflate (RFC 1950/1951)\n"
        "- [x] Sovereign typography with TrueType parsing, BiDi (UAX #9) & Arabic shaping\n"
        "- [x] GitHub-Grade Markdown Engine with HTML tags and Callout Alerts :octocat:\n"
        "- [ ] Sovereign ISO 32000 PDF document engine\n"
        "\n"
        "### Performance Benchmark vs GNU Cairo 1.18\n"
        "\n"
        "| Operation Category | Nisaba CPU (µs) | Cairo 1.18 (µs) | Speedup Ratio |\n"
        "| :--- | :---: | :---: | ---: |\n"
        "| Diagonal Full-HD Gradient | 308.7 µs | 6653.0 µs | **21.55x** (Nisaba) |\n"
        "| 2-Point Focal Spotlight | 327.1 µs | 5231.8 µs | **15.99x** (Nisaba) |\n"
        "| Radial Glow Gradient | 288.6 µs | 4000.8 µs | **13.86x** (Nisaba) |\n"
        "| 25 Concentric Rings | 461.4 µs | 6002.2 µs | **13.01x** (Nisaba) |\n"
        "| Dashed Orbit Ring | 31.7 µs | 364.1 µs | **11.48x** (Nisaba) |\n"
        "| 25 Alpha Circles Blend | 68.4 µs | 703.0 µs | **10.27x** (Nisaba) |\n"
        "| **TOTAL 25-SUITE FRAME** | **5972.5 µs** | **44968.7 µs** | **7.53x Faster** |\n"
        "\n"
        "---\n"
        "\n"
        "### Modern C++20 Syntax Highlighting\n"
        "\n"
        "```cpp\n"
        "#include <nisaba/nisaba.hpp>\n"
        "using namespace nisaba;\n"
        "\n"
        "int main() {\n"
        "    // Allocate a high-definition 2D canvas\n"
        "    auto doc = markdown::MarkdownDocument::from_string(content);\n"
        "    auto pixmap = Pixmap::create(1080, 800);\n"
        "    Canvas canvas(*pixmap);\n"
        "    markdown::MarkdownRenderer::render(canvas, doc, 40.0f, 20.0f, 1000.0f, style, fonts, cache);\n"
        "    pixmap->save_image(\"output.png\");\n"
        "    return 0;\n"
        "}\n"
        "```\n"
        "\n"
        "[^1]: Verified via 13/13 automated test suites in Nisaba test runner.\n";

    // 3. Parse Markdown
    auto t0 = std::chrono::high_resolution_clock::now();
    auto doc = MarkdownDocument::from_string(markdown_source);
    auto t1 = std::chrono::high_resolution_clock::now();
    double parse_us = std::chrono::duration<double, std::micro>(t1 - t0).count();
    std::cout << "[+] Parsed markdown document into AST in " << std::fixed << std::setprecision(2) << parse_us << " µs.\n";

    // 4. Layout Document
    constexpr float DOC_WIDTH = 960.0f;
    constexpr float PAGE_PAD_X = 50.0f;
    constexpr float PAGE_PAD_Y = 40.0f;
    constexpr uint32_t TOTAL_WIDTH = 1060;

    auto style = MarkdownStyle::dark_theme();
    float doc_height = doc.layout(DOC_WIDTH, style, font_system, glyph_cache);
    uint32_t total_height = static_cast<uint32_t>(std::ceil(doc_height + 2.0f * PAGE_PAD_Y));

    std::cout << "[+] Layout computed: " << DOC_WIDTH << "x" << doc_height << " px (Total Canvas: "
              << TOTAL_WIDTH << "x" << total_height << " px).\n";

    // 5. Render to CPU Canvas Pixmap
    auto pixmap = Pixmap::create(TOTAL_WIDTH, total_height);
    if (!pixmap) {
        std::cerr << "[-] Failed to allocate pixmap surface!\n";
        return 1;
    }
    Canvas canvas(*pixmap);

    // Subtle dark gradient background
    canvas.clear(Color::from_rgba8(10, 14, 22, 255));

    auto t2 = std::chrono::high_resolution_clock::now();
    MarkdownRenderer::render(canvas, doc, PAGE_PAD_X, PAGE_PAD_Y, DOC_WIDTH, style, font_system, glyph_cache);
    auto t3 = std::chrono::high_resolution_clock::now();
    double render_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
    std::cout << "[+] Rendered entire document onto Canvas in " << std::fixed << std::setprecision(2) << render_ms << " ms.\n";

    // 6. Save Showcase Image (High-Resolution PNG)
    std::string out_png = "showcase/nisaba_markdown_showcase.png";
    if (pixmap->save_image(out_png)) {
        std::cout << "[+] Saved showcase image: " << out_png << "\n";
    }

    // 7. Export True Multi-Page Vector PDF Document (Phase 6)
    std::string out_pdf = "showcase/nisaba_markdown_showcase.pdf";
    MarkdownPdfExportOptions pdf_opts;
    pdf_opts.doc_title = "Nisaba Sovereign Markdown Showcase";
    pdf_opts.header_left = "Nisaba Sovereign Graphics Engine";
    pdf_opts.footer_left = "Generated with Sovereign Nisaba GFM Engine";
    pdf_opts.show_header = true;
    pdf_opts.show_footer = true;
    pdf_opts.show_page_numbers = true;
    if (MarkdownPdfExporter::export_to_file(doc, out_pdf, style, font_system, glyph_cache, pdf_opts)) {
        std::cout << "[+] Exported ISO 32000-1 vector PDF document: " << out_pdf << "\n";
    }

    // 8. Interactive Hit-Testing & Table of Contents Telemetry (Phase 5)
    auto toc = doc.table_of_contents();
    std::cout << "[+] Extracted Table of Contents (" << toc.size() << " sections):\n";
    for (const auto& [title, slug] : toc) {
        std::cout << "    - " << title << " [#" << slug << "]\n";
    }

#ifdef NISABA_HAS_BACKEND_OS
    if (!run_headless) {
        std::cout << "[*] Launching native interactive window viewer...\n";
        auto plat_res = backend_os::Platform::create();
        if (plat_res.isOk()) {
            auto platform = std::move(plat_res.value());
            backend_os::WindowConfig cfg;
            cfg.title = "Nisaba Sovereign Markdown Document Showcase";
            cfg.width = 1060;
            cfg.height = 800;
            cfg.vsync = true;
            auto win_res = backend_os::Window::create(*platform, cfg);
            if (win_res.isOk()) {
                auto window = std::move(win_res.value());
                window->makeCurrent();

#ifdef NISABA_GLEW
                glewInit();
#endif
                auto gpu_ctx = nisaba::gpu::createContextGL3(nisaba::gpu::CreateFlags::Antialias);
                if (gpu_ctx) {
                    int texture_id = gpu_ctx->createImageRGBA(TOTAL_WIDTH, total_height, 0, pixmap->data());
                    bool running = true;
                    float scroll_offset = 0.0f;
                    float max_scroll = std::max(0.0f, static_cast<float>(total_height) - 800.0f);

                    window->onClose().connect([&]() { running = false; });
                    platform->onKeyDown().connect([&](int key, int mods) {
                        (void)mods;
                        if (key == 27 || key == 'q' || key == 'Q') running = false;
                        else if (key == 264 || key == 'j') scroll_offset = std::min(max_scroll, scroll_offset + 40.0f); // Down
                        else if (key == 265 || key == 'k') scroll_offset = std::max(0.0f, scroll_offset - 40.0f);       // Up
                    });

                    platform->onScroll().connect([&](float dx, float dy) {
                        (void)dx;
                        scroll_offset = std::clamp(scroll_offset - dy * 30.0f, 0.0f, max_scroll);
                    });

                    // Interactive mouse clicks (toggle details, checkboxes, links)
                    platform->onMouseDown().connect([&](float mx, float my, int btn) {
                        if (btn == 1) { // Left click
                            float doc_x = mx - PAGE_PAD_X;
                            float doc_y = my + scroll_offset - PAGE_PAD_Y;
                            bool state_changed = doc.handle_click(doc_x, doc_y, [&](const std::string& url) {
                                std::cout << "[*] User clicked link: " << url << "\n";
                            });
                            if (state_changed) {
                                std::cout << "[*] Interactive element toggled! Re-rendering...\n";
                                doc_height = doc.layout(DOC_WIDTH, style, font_system, glyph_cache);
                                canvas.clear(Color::from_rgba8(10, 14, 22, 255));
                                MarkdownRenderer::render(canvas, doc, PAGE_PAD_X, PAGE_PAD_Y, DOC_WIDTH, style, font_system, glyph_cache);
                                gpu_ctx->updateImage(texture_id, pixmap->data());
                            }
                        }
                    });

                    int display_frames = 0;
                    while (running && platform->pollEvents() && display_frames < 60) {
                        gpu_ctx->beginFrame(1060, 800, 1.0f);
                        auto pattern = gpu_ctx->imagePattern(0, -scroll_offset, TOTAL_WIDTH, total_height, 0, texture_id, 1.0f);
                        gpu_ctx->beginPath();
                        gpu_ctx->rect(0, 0, 1060, 800);
                        gpu_ctx->fillPaint(pattern);
                        gpu_ctx->fill();
                        gpu_ctx->endFrame();
                        window->swapBuffers();
                        display_frames++;
                    }

                    gpu_ctx->deleteImage(texture_id);
                }
            }
        }
    }
#endif

    std::cout << "\n====================================================================================\n";
    std::cout << "   NISABA MARKDOWN ENGINE SHOWCASE COMPLETED SUCCESSFULLY!\n";
    std::cout << "====================================================================================\n";
    return 0;
}

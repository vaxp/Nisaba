#include "nisaba/pdf/pdf_reader.hpp"
#include "nisaba/text/font_system.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <cstdio>
#include <memory>
#include <array>
#include <sstream>

using namespace nisaba;
using namespace nisaba::pdf;

struct BenchmarkRow {
    size_t page;
    double nisaba_cold_ms;
    double nisaba_warm_ms;
    double mupdf_cold_ms;
    double mupdf_warm_ms;
};

// Helper to run mutool draw and parse timings per page
std::vector<double> run_mutool_timings(const std::string& pdf_path) {
    std::vector<double> timings;
    std::string cmd = "mutool draw -s t -F pam -o /dev/null " + pdf_path + " 2>&1";
    std::array<char, 256> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) return timings;

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        std::string line(buffer.data());
        // Look for: page assets/sr.pdf X Yms
        auto pos_page = line.find("page ");
        auto pos_ms = line.find("ms");
        if (pos_page != std::string::npos && pos_ms != std::string::npos && line.find("total") == std::string::npos) {
            std::istringstream iss(line);
            std::string tok_page, tok_file, tok_num, tok_time;
            if (iss >> tok_page >> tok_file >> tok_num >> tok_time) {
                // tok_time is like "143ms"
                double ms = std::strtod(tok_time.c_str(), nullptr);
                if (ms > 0.0) {
                    timings.push_back(ms);
                }
            }
        }
    }
    return timings;
}

int main(int argc, char** argv) {
    std::string pdf_path = (argc > 1) ? argv[1] : "assets/sr.pdf";

    std::cout << "\n========================================================================================\n";
    std::cout << "          NISABA SOVEREIGN PDF ENGINE VS REFERENCE ENGINE (BENCHMARK)\n";
    std::cout << "========================================================================================\n";
    std::cout << "[*] Benchmark Target: " << pdf_path << "\n";
    std::cout << "[*] Running Reference Engine (Cold Pass)...\n";
    auto ref_cold = run_mutool_timings(pdf_path);

    std::cout << "[*] Running Reference Engine (Warm Pass)...\n";
    auto ref_warm = run_mutool_timings(pdf_path);

    std::cout << "[*] Initializing Nisaba Sovereign Engine...\n";
    text::FontSystem font_system;
    font_system.load_font_file("assets/fonts/Inter-Regular.ttf");
    font_system.load_font_file("assets/fonts/NotoSansArabic-Regular.ttf");

    PdfReader reader;
    if (!reader.open_from_file(pdf_path)) {
        std::cerr << "[-] Error: Failed to open PDF: " << pdf_path << "\n";
        return 1;
    }

    size_t count = reader.page_count();
    std::vector<BenchmarkRow> rows(count);

    // Nisaba Cold Pass
    std::cout << "[*] Running Nisaba Engine (Cold Pass)...\n";
    for (size_t p = 0; p < count; ++p) {
        auto box = reader.page_box(p);
        uint32_t w = static_cast<uint32_t>(box.width());
        uint32_t h = static_cast<uint32_t>(box.height());
        auto pm = Pixmap::allocate(w, h);
        if (!pm) continue;
        Canvas canvas(*pm);
        canvas.clear(Color::WHITE);

        auto t0 = std::chrono::high_resolution_clock::now();
        reader.render_page(p, canvas, 1.0f, &font_system);
        auto t1 = std::chrono::high_resolution_clock::now();

        rows[p].page = p + 1;
        rows[p].nisaba_cold_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        rows[p].mupdf_cold_ms = (p < ref_cold.size()) ? ref_cold[p] : 0.0;
    }

    // Nisaba Warm Pass
    std::cout << "[*] Running Nisaba Engine (Warm Pass)...\n";
    for (size_t p = 0; p < count; ++p) {
        auto box = reader.page_box(p);
        uint32_t w = static_cast<uint32_t>(box.width());
        uint32_t h = static_cast<uint32_t>(box.height());
        auto pm = Pixmap::allocate(w, h);
        if (!pm) continue;
        Canvas canvas(*pm);
        canvas.clear(Color::WHITE);

        auto t0 = std::chrono::high_resolution_clock::now();
        reader.render_page(p, canvas, 1.0f, &font_system);
        auto t1 = std::chrono::high_resolution_clock::now();

        rows[p].nisaba_warm_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        rows[p].mupdf_warm_ms = (p < ref_warm.size()) ? ref_warm[p] : 0.0;
    }

    // Print Comparative Benchmark Table
    std::cout << "\n┌──────┬───────────────────────────────┬───────────────────────────────┬────────────────┐\n";
    std::cout << "│ Page │  Nisaba Sovereign Engine (ms) │  Reference Engine (MuPDF ms)  │ Speedup Factor │\n";
    std::cout << "│      ├───────────────┬───────────────┼───────────────┬───────────────┤ (Warm Pass)    │\n";
    std::cout << "│      │ Cold Render   │ Warm Render   │ Cold Render   │ Warm Render   │                │\n";
    std::cout << "├──────┼───────────────┼───────────────┼───────────────┼───────────────┼────────────────┤\n";

    double sum_nisaba_cold = 0.0, sum_nisaba_warm = 0.0;
    double sum_ref_cold = 0.0, sum_ref_warm = 0.0;

    for (const auto& r : rows) {
        sum_nisaba_cold += r.nisaba_cold_ms;
        sum_nisaba_warm += r.nisaba_warm_ms;
        sum_ref_cold += r.mupdf_cold_ms;
        sum_ref_warm += r.mupdf_warm_ms;

        double speedup = (r.nisaba_warm_ms > 0.0) ? (r.mupdf_warm_ms / r.nisaba_warm_ms) : 1.0;

        std::cout << "│ " << std::setw(4) << r.page << " │ "
                  << std::setw(11) << std::fixed << std::setprecision(2) << r.nisaba_cold_ms << " ms │ "
                  << std::setw(11) << std::fixed << std::setprecision(2) << r.nisaba_warm_ms << " ms │ "
                  << std::setw(11) << std::fixed << std::setprecision(2) << r.mupdf_cold_ms << " ms │ "
                  << std::setw(11) << std::fixed << std::setprecision(2) << r.mupdf_warm_ms << " ms │ "
                  << std::setw(12) << std::fixed << std::setprecision(1) << speedup << "x │\n";
    }

    std::cout << "├──────┼───────────────┼───────────────┼───────────────┼───────────────┼────────────────┤\n";
    double avg_nisaba_cold = sum_nisaba_cold / count;
    double avg_nisaba_warm = sum_nisaba_warm / count;
    double avg_ref_cold = sum_ref_cold / count;
    double avg_ref_warm = sum_ref_warm / count;
    double avg_speedup = (avg_nisaba_warm > 0.0) ? (avg_ref_warm / avg_nisaba_warm) : 1.0;

    std::cout << "│  AVG │ "
              << std::setw(11) << std::fixed << std::setprecision(2) << avg_nisaba_cold << " ms │ "
              << std::setw(11) << std::fixed << std::setprecision(2) << avg_nisaba_warm << " ms │ "
              << std::setw(11) << std::fixed << std::setprecision(2) << avg_ref_cold << " ms │ "
              << std::setw(11) << std::fixed << std::setprecision(2) << avg_ref_warm << " ms │ "
              << std::setw(12) << std::fixed << std::setprecision(1) << avg_speedup << "x │\n";
    std::cout << "└──────┴───────────────┴───────────────┴───────────────┴───────────────┴────────────────┘\n\n";

    return 0;
}

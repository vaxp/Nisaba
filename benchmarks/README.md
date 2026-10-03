# Nisaba Comparative Benchmarks & External Profilers

> [!NOTE]
> **Notice to Developers and Reviewers:**
> All files located in this directory are standalone comparative benchmarks, micro-profilers, and third-party reference headers (`stb_image.h`, `stb_truetype.h`, `fontstash.h`).
> **These files are used strictly for offline performance benchmarking and parity validation against external libraries.**
> **They are completely decoupled from the core project and have NO relationship to Nisaba's native runtime architecture (`src/` and `include/`).**

> [!IMPORTANT]
> **Benchmarking Philosophy & Engineering Purpose:**
> The purpose of these benchmarks is **not** to boast or claim that our engine is superior to other solutions. Rather, they serve as an objective diagnostic tool to uncover architectural bottlenecks, profiling regressions, and cache inefficiencies when evaluated against libraries that are widely recognized as industry standards (such as Cairo, Google , Blend2D, and STB). Comparing against established baselines allows us to continuously optimize and ensure consistent, predictable real-time performance.

---

## Purpose & Scope

The benchmarks here rigorously measure Nisaba's sovereign C++20 implementations against industry baselines:

1. **Image Decoders (`benchmark_image_decoders.cpp`)**:
   - Benchmarks Nisaba's native sovereign PNG, JPEG, and QOI decoders against Sean Barrett's `stb_image.h`.
   - Validates memory efficiency, decode latency, and throughput across diverse resolutions and bit depths.

2. **Text Engines (`benchmark_text_engines.cpp`)**:
   - Compares Nisaba's native sovereign C++20 text engine (`FontSystem`, `TtfFont`, `GlyphCache`, `Buffer`, `Bidi`) against the external `stb_truetype.h` & `fontstash.h` pipeline.
   - Measures parsing, glyph caching, kerning calculations, vector outline extraction, and layout.

3. **2D Graphics Micro-Profiler (`micro_profiler.cpp`)**:
   - Comprehensive multi-suite rendering micro-benchmarks comparing Nisaba against Cairo and Blend2D.
   - Measures raw geometry rasterization, antialiased Béziers, gradients, clipping, and blits to identify CPU rasterization bottlenecks.

4. **GPU Micro-Profiler (`gpu_micro_profiler.cpp`)**:
   - 20-suite hardware GPU micro-profiling suite comparing Nisaba GPU against Google  (Ganesh GL) to isolate draw call, state change, and shader pipeline overhead.

---

## Building & Running

To build benchmarks:
```bash
meson setup build -Dbuild_benchmarks=true
ninja -C build
```

To run text engine benchmark:
```bash
./build/benchmarks/benchmark_text_engines
```

To run image decoder benchmark:
```bash
./build/benchmarks/benchmark_image_decoders
```

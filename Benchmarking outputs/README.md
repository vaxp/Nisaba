# Nisaba Sovereign Graphics Engine — Benchmarking Suite & Execution Reports

> [!IMPORTANT]
> **Critical Methodological Notice on Test Conditions & Optimization Layers (VSPR Layer):**
> 
> We explicitly and unequivocally state that **all comparative benchmarks and performance evaluations documented within this directory were executed entirely without enabling the engine's performance optimization layer (VSPR layer)**.
> 
> **All performance-boosting features and workload-reduction techniques were completely disabled**, including adaptive surface caching, dirty-region bounding box culling, and intelligent path simplification.
> 
> **Purpose & Architectural Significance:**
> The primary objective is to measure and benchmark the **pure, raw baseline execution performance** of the Nisaba engine against industry-standard engines (GNU Cairo, Google Skia CPU, Google Skia Ganesh GL, stb_image, and stb_truetype). This conclusively demonstrates the inherent algorithmic and computational efficiency of Nisaba's core rasterizer and pipeline under heavy, direct computational loads without relying on high-level workload reduction or optimization caching layers.

---

## Index of Benchmark Reports (10 Consecutive Runs)

Each benchmark suite was executed **10 consecutive times in an automated and independent manner** using the unified test orchestrator [`scripts/run_benchmarks_10x.py`](../scripts/run_benchmarks_10x.py). Exact arithmetic means were calculated across all quantitative metrics, and complete, unmodified standard output logs for each individual run are provided at the end of each report:

| Benchmark Report | Competitor Libraries | Workload Scope | Overall Nisaba Performance (10-Run Mean) |
| :--- | :--- | :--- | :--- |
| **[`benchmark_image_decoders.md`](./benchmark_image_decoders.md)** | `stb_image.h` | Full-fidelity PNG, JPEG, and QOI decoding with bit-for-bit PSNR validation | **1.10x to 1.35x faster** decoding, with sovereign QOI decoding throughput exceeding 427 MP/s and 100% bit-for-bit exact match (Infinity dB PSNR) |
| **[`benchmark_text_engines.md`](./benchmark_text_engines.md)** | `stb_truetype` + `fontstash` | End-to-end typography pipeline: table parsing, CMAP mapping, subpixel glyph rasterization, and multi-line BiDi layout | **4.74x faster** in glyph rasterization, **2.22x faster** in paragraph layout, and **1.45x faster** in vector outline extraction |
| **[`nisaba_vs_cairo.md`](./nisaba_vs_cairo.md)** | `GNU Cairo 1.18` (CPU) | 90 complex vector suites (curved paths, radial/linear gradients, Porter-Duff blending, alpha masking, affine transforms) | **7.57x faster** overall frame overhead (30.98 ms vs. 234.56 ms)<br>Nisaba won **89 of 90 suites (98.9%)** |
| **[`nisaba_vs_skia.md`](./nisaba_vs_skia.md)** | `Google Skia CPU` | The identical 90 complex geometric suites under identical visual parameters | **2.40x faster** overall frame overhead (31.34 ms vs. 75.12 ms)<br>Nisaba won **70 of 90 suites (77.8%)** |
| **[`gpu_micro_profiler.md`](./gpu_micro_profiler.md)** | `Google Skia (Ganesh GL)` | 100 hardware-accelerated GPU micro-benchmarks via OpenGL FBOs, batching, and command submission | **4.69x faster** in CPU command submission overhead<br>**1.09x faster** in end-to-end frame throughput |

---

## Hardware & Test Environment Specifications

- **Operating System:** Linux x86_64 (Ubuntu).
- **CPU:** Intel Core i7-8565U @ 1.80GHz (8 vCPUs).
- **Viewport Dimensions:** 1080x740 (for CPU rasterization suites) and 1080x720 (for GPU OpenGL profiler suites).
- **Measurement Methodology:** 10 complete consecutive iterations per benchmark to eliminate OS scheduler noise and extract stable arithmetic means.
- **Engine Operating Mode:** Unassisted Raw Baseline (**VSPR Layer Disabled**).

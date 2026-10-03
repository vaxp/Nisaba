# Benchmark Report: Nisaba Image Decoders vs stb_image.h

**Execution Date:** 2026-10-03 14:48:45  
**Runs Executed:** 10 consecutive runs  
**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  

---

## Executive Summary & 10-Run Averages

The table below displays the **arithmetic mean of 10 consecutive benchmark executions** for each test workload, measuring decoding time (ms), throughput (Megapixels/sec), speedup factor, and visual PSNR fidelity.

| Test Image | Format | Resolution | File Size | Nisaba Mean Time | STB Mean Time | Nisaba Throughput | STB Throughput | Average Speedup | Fidelity (PSNR) |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| `vaxp.png` | **PNG** | 1024x1024 (1.05 MP) | 158.71 KB | **1.92 ms** | 2.17 ms | **548.9 MP/s** | 485.3 MP/s | **1.13x FASTER** | Infinity dB (Bit-for-Bit Perfect 100% Exact Match) |
| `showcase_vaxp.png` | **PNG** | 1400x900 (1.26 MP) | 270.19 KB | **8.43 ms** | 11.39 ms | **149.7 MP/s** | 110.8 MP/s | **1.35x FASTER** | Infinity dB (Bit-for-Bit Perfect 100% Exact Match) |
| `showcase_vaxp.jpg` | **JPEG** | 1400x900 (1.26 MP) | 144.14 KB | **8.89 ms** | 11.57 ms | **142.3 MP/s** | 109.3 MP/s | **1.30x FASTER** | 74.44 dB (Visually Indistinguishable) |
| `showcase_vaxp.qoi` | **QOI** | 1400x900 (1.26 MP) | 323.24 KB | **2.95 ms** | N/A (Unsupported) | **427.8 MP/s** | N/A | N/A (Sovereign Only) |  |
| `nisaba_gpu_showcase.png` | **PNG** | 1400x900 (1.26 MP) | 451.89 KB | **5.28 ms** | 5.83 ms | **238.7 MP/s** | 216.0 MP/s | **1.10x FASTER** | Infinity dB (Bit-for-Bit Perfect 100% Exact Match) |

---

## Individual Run Logs (Runs 1 to 10)

Below are the complete, unmodified standard output logs for each individual execution.

### Run 1
<details>
<summary>Click to expand Run 1 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.08 ms      1.69 ms      4.76 ms     505.2 MP/s
stb_image (External)      2.29 ms      1.90 ms      3.58 ms     457.9 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.10x FASTER than stb_image (0.21 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.56 ms      7.08 ms      9.49 ms     147.2 MP/s
stb_image (External)     11.45 ms     10.93 ms     12.64 ms     110.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.34x FASTER than stb_image (2.89 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.63 ms      7.97 ms     10.34 ms     146.1 MP/s
stb_image (External)     10.92 ms     10.40 ms     13.09 ms     115.4 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.27x FASTER than stb_image (2.29 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.77 ms      2.51 ms      4.09 ms     454.8 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.10 ms      4.66 ms      5.64 ms     246.9 MP/s
stb_image (External)      5.73 ms      5.26 ms      7.01 ms     220.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.12x FASTER than stb_image (0.62 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 2
<details>
<summary>Click to expand Run 2 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.80 ms      1.63 ms      2.81 ms     581.0 MP/s
stb_image (External)      2.11 ms      1.85 ms      3.85 ms     497.9 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.17x FASTER than stb_image (0.30 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.27 ms      6.74 ms      8.93 ms     152.4 MP/s
stb_image (External)     11.57 ms     10.77 ms     16.94 ms     108.9 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.40x FASTER than stb_image (3.31 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.65 ms      7.86 ms      9.78 ms     145.6 MP/s
stb_image (External)     10.68 ms     10.25 ms     12.69 ms     118.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.23x FASTER than stb_image (2.03 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.76 ms      2.51 ms      4.37 ms     456.8 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.11 ms      4.67 ms      5.84 ms     246.5 MP/s
stb_image (External)      5.79 ms      5.17 ms      7.38 ms     217.4 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.13x FASTER than stb_image (0.68 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 3
<details>
<summary>Click to expand Run 3 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.85 ms      1.63 ms      3.00 ms     565.9 MP/s
stb_image (External)      2.18 ms      1.85 ms      3.80 ms     480.7 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.18x FASTER than stb_image (0.33 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.44 ms      7.03 ms      9.50 ms     149.2 MP/s
stb_image (External)     11.40 ms     10.96 ms     12.62 ms     110.5 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.35x FASTER than stb_image (2.96 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native            10.06 ms      8.05 ms     16.40 ms     125.2 MP/s
stb_image (External)     11.83 ms     10.16 ms     19.44 ms     106.5 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.18x FASTER than stb_image (1.77 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             3.01 ms      2.55 ms      4.69 ms     418.6 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.01 ms      4.63 ms      6.88 ms     251.3 MP/s
stb_image (External)      6.00 ms      5.30 ms      6.79 ms     209.8 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.20x FASTER than stb_image (0.99 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 4
<details>
<summary>Click to expand Run 4 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.70 ms      1.63 ms      2.92 ms     615.4 MP/s
stb_image (External)      1.89 ms      1.84 ms      3.19 ms     555.3 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.11x FASTER than stb_image (0.18 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             7.60 ms      6.50 ms      8.42 ms     165.7 MP/s
stb_image (External)     10.34 ms     10.11 ms     12.02 ms     121.8 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.36x FASTER than stb_image (2.74 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             7.81 ms      7.72 ms      8.77 ms     161.3 MP/s
stb_image (External)     13.69 ms      9.98 ms     23.27 ms      92.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.75x FASTER than stb_image (5.88 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             3.27 ms      3.14 ms      4.75 ms     385.1 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.58 ms      5.41 ms      7.06 ms     225.9 MP/s
stb_image (External)      5.96 ms      5.83 ms      6.81 ms     211.3 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.07x FASTER than stb_image (0.39 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 5
<details>
<summary>Click to expand Run 5 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.97 ms      1.93 ms      3.30 ms     533.0 MP/s
stb_image (External)      2.28 ms      2.13 ms      3.65 ms     459.6 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.16x FASTER than stb_image (0.31 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.60 ms      7.48 ms      9.45 ms     146.5 MP/s
stb_image (External)     11.43 ms     11.36 ms     12.61 ms     110.2 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.33x FASTER than stb_image (2.83 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.98 ms      8.70 ms     10.88 ms     140.3 MP/s
stb_image (External)     11.57 ms     10.96 ms     13.15 ms     108.9 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.29x FASTER than stb_image (2.58 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             3.00 ms      2.89 ms      4.63 ms     420.0 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.35 ms      5.27 ms      5.95 ms     235.7 MP/s
stb_image (External)      5.77 ms      5.69 ms      6.19 ms     218.4 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.08x FASTER than stb_image (0.42 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 6
<details>
<summary>Click to expand Run 6 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.93 ms      1.87 ms      3.12 ms     543.0 MP/s
stb_image (External)      2.18 ms      2.12 ms      3.74 ms     480.9 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.13x FASTER than stb_image (0.25 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.61 ms      7.29 ms      9.32 ms     146.4 MP/s
stb_image (External)     11.45 ms     11.35 ms     13.28 ms     110.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.33x FASTER than stb_image (2.84 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.92 ms      8.69 ms      9.96 ms     141.3 MP/s
stb_image (External)     11.44 ms     11.20 ms     13.40 ms     110.2 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.28x FASTER than stb_image (2.52 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.93 ms      2.89 ms      4.14 ms     429.5 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.31 ms      5.17 ms      6.47 ms     237.2 MP/s
stb_image (External)      5.76 ms      5.69 ms      6.17 ms     218.8 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.08x FASTER than stb_image (0.45 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 7
<details>
<summary>Click to expand Run 7 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.93 ms      1.88 ms      3.11 ms     542.4 MP/s
stb_image (External)      2.17 ms      2.13 ms      3.65 ms     482.9 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.12x FASTER than stb_image (0.24 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.55 ms      7.29 ms      9.01 ms     147.3 MP/s
stb_image (External)     11.67 ms     11.38 ms     11.99 ms     108.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.36x FASTER than stb_image (3.12 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.93 ms      8.70 ms     10.00 ms     141.2 MP/s
stb_image (External)     11.32 ms     10.97 ms     13.09 ms     111.3 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.27x FASTER than stb_image (2.39 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.95 ms      2.89 ms      4.14 ms     427.6 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.34 ms      5.28 ms      5.52 ms     235.8 MP/s
stb_image (External)      5.77 ms      5.69 ms      6.18 ms     218.5 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.08x FASTER than stb_image (0.42 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 8
<details>
<summary>Click to expand Run 8 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.97 ms      1.93 ms      3.18 ms     533.0 MP/s
stb_image (External)      2.17 ms      2.13 ms      3.64 ms     483.8 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.10x FASTER than stb_image (0.20 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.60 ms      7.29 ms      8.98 ms     146.5 MP/s
stb_image (External)     11.54 ms     11.36 ms     11.93 ms     109.2 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.34x FASTER than stb_image (2.94 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.92 ms      8.69 ms      9.92 ms     141.3 MP/s
stb_image (External)     11.29 ms     11.03 ms     13.08 ms     111.6 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.27x FASTER than stb_image (2.38 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.94 ms      2.89 ms      4.31 ms     429.2 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.34 ms      5.26 ms      5.56 ms     235.8 MP/s
stb_image (External)      5.86 ms      5.69 ms      6.33 ms     215.1 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.10x FASTER than stb_image (0.52 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 9
<details>
<summary>Click to expand Run 9 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.96 ms      1.93 ms      3.20 ms     534.3 MP/s
stb_image (External)      2.19 ms      2.13 ms      3.64 ms     479.6 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.11x FASTER than stb_image (0.22 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.55 ms      7.29 ms      9.90 ms     147.4 MP/s
stb_image (External)     11.58 ms     11.36 ms     15.51 ms     108.8 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.35x FASTER than stb_image (3.03 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.99 ms      8.76 ms     10.27 ms     140.1 MP/s
stb_image (External)     11.55 ms     11.46 ms     13.64 ms     109.1 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.28x FASTER than stb_image (2.56 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.97 ms      2.89 ms      4.14 ms     424.9 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.33 ms      5.28 ms      5.48 ms     236.3 MP/s
stb_image (External)      5.78 ms      5.69 ms      6.27 ms     218.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.08x FASTER than stb_image (0.45 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

### Run 10
<details>
<summary>Click to expand Run 10 full log</summary>

```text
======================================================================
  Nisaba Image Engine vs stb_image: Performance & Fidelity Benchmark 
======================================================================
Running 50 iterations per image test...

----------------------------------------------------------------------
Image: vaxp.png [PNG]
Resolution: 1024x1024 (1.05 MP) | File Size: 158.71 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             1.96 ms      1.88 ms      3.08 ms     536.3 MP/s
stb_image (External)      2.21 ms      2.13 ms      3.64 ms     474.2 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.13x FASTER than stb_image (0.26 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1048576 / 1048576)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 270.19 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.50 ms      7.33 ms      9.00 ms     148.2 MP/s
stb_image (External)     11.45 ms     11.32 ms     13.46 ms     110.1 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.35x FASTER than stb_image (2.94 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

----------------------------------------------------------------------
Image: showcase_vaxp.jpg [JPEG]
Resolution: 1400x900 (1.26 MP) | File Size: 144.14 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             8.97 ms      8.69 ms     11.02 ms     140.5 MP/s
stb_image (External)     11.45 ms     11.25 ms     13.47 ms     110.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.28x FASTER than stb_image (2.48 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/showcase_vaxp_stb.png
  - Identical Pixels vs STB    : 99.06% (1248212 / 1260000)
  - Max Color Channel Delta    : 1 / 255
  - Mean Absolute Error (MAE)  : 0.0023
  - PSNR (Quality Metric)      : 74.44 dB (Visually Indistinguishable)

----------------------------------------------------------------------
Image: showcase_vaxp.qoi [QOI]
Resolution: 1400x900 (1.26 MP) | File Size: 323.24 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             2.92 ms      2.88 ms      4.13 ms     431.7 MP/s
stb_image (External)                Format Not Supported by stb_image
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/showcase_vaxp_nisaba.png
  - Visual Fidelity            : Validated against sovereign QOI spec (Decoded cleanly)

----------------------------------------------------------------------
Image: nisaba_gpu_showcase.png [PNG]
Resolution: 1400x900 (1.26 MP) | File Size: 451.89 KB | Iterations: 50
----------------------------------------------------------------------
Decoder                 Avg Time    Min Time    Max Time    Throughput
----------------------------------------------------------------------
Nisaba Native             5.34 ms      5.18 ms      6.43 ms     236.0 MP/s
stb_image (External)      5.91 ms      5.82 ms      7.49 ms     213.0 MP/s
----------------------------------------------------------------------
>>> Speed: Nisaba is 1.11x FASTER than stb_image (0.58 ms faster per image)
----------------------------------------------------------------------
[Image Fidelity & Extraction Verification]
  - Saved Nisaba Decoded Image : build/decoded_images/nisaba_gpu_showcase_nisaba.png
  - Saved STB Decoded Image    : build/decoded_images/nisaba_gpu_showcase_stb.png
  - Identical Pixels vs STB    : 100.00% (1260000 / 1260000)
  - Max Color Channel Delta    : 0 / 255
  - Mean Absolute Error (MAE)  : 0.0000
  - PSNR (Quality Metric)      : Infinity dB (Bit-for-Bit Perfect 100% Exact Match)

======================================================================
  Benchmark Completed! Decoded images exported to build/decoded_images
======================================================================
```
</details>

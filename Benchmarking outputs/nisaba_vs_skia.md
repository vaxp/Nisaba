# Benchmark Report: Nisaba vs Skia (90 Rigorous Geometric Suites)

**Execution Date:** 2026-10-03 14:58:24  
**Runs Executed:** 10 consecutive runs  
**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  

---

## Executive Summary & Aggregate Averages

Across **10 consecutive executions** testing 90 vector geometry, shader, blending, and compositing benchmarks:
- **Nisaba Average Total Frame Time:** `31338.3 µs` (31.34 ms)
- **Skia Average Total Frame Time:** `75118.2 µs` (75.12 ms)
- **Overall Speedup:** **2.40x FASTER (Nisaba)**
- **Win / Loss Scorecard (10-Run Mean):**
  * **Nisaba Won:** **70 / 90 suites (77.8%)**
  * **Skia Won:** 16 / 90 suites (17.8%)
  * **Ties:** 4 suites

---

## 10-Run Mean Results by Benchmark Suite

| # | Benchmark Suite Description | Nisaba Mean (µs) | Skia Mean (µs) | Speedup / Winner |
| :---: | :--- | :---: | :---: | :---: |
| 1 | Solid Background Fill (1080x740) | **88.7 µs** | 121.2 µs | **1.37x (Nisaba)** |
| 2 | 50 Grid Lines (1px Stroked Paths) | **78.2 µs** | 108.8 µs | **1.39x (Nisaba)** |
| 3 | 25 Alpha Circles (Porter-Duff Blend) | **65.5 µs** | 163.9 µs | **2.50x (Nisaba)** |
| 4 | Cubic Bezier Curves (Fill + Stroke) | **202.4 µs** | 182.3 µs | 1.11x (Skia) |
| 5 | 4 Rounded Rect Cards (Fill + Stroke) | **64.6 µs** | 163.6 µs | **2.53x (Nisaba)** |
| 6 | Multi-Stop Linear Gradient (480x36) | **9.8 µs** | 8.5 µs | 1.15x (Skia) |
| 7 | Radial Glow Gradient (250px Glow) | **312.1 µs** | 927.6 µs | **2.97x (Nisaba)** |
| 8 | 12 Radial Rotated Spokes (Strokes) | **33.8 µs** | 75.5 µs | **2.23x (Nisaba)** |
| 9 | Dashed Orbit Ring (Stroke with Dash) | **32.7 µs** | 198.4 µs | **6.07x (Nisaba)** |
| 10 | Concave 10-Point Star (Winding Fill) | **45.7 µs** | 34.5 µs | 1.32x (Skia) |
| 11 | Compound Transforms (16 Elements) | **79.4 µs** | 82.9 µs | Tie (~1.0x) |
| 12 | Rectangular Clipping (Nested Viewport) | **13.8 µs** | 27.7 µs | **2.01x (Nisaba)** |
| 13 | 100 Anti-Aliased Diagonals (Lines) | **1080.7 µs** | 1373.7 µs | **1.27x (Nisaba)** |
| 14 | 30 Overlapping UI Chips (Alpha Stack) | **156.9 µs** | 286.7 µs | **1.83x (Nisaba)** |
| 15 | Closed Cubic Loop (Trefoil Figure-8) | **77.2 µs** | 112.5 µs | **1.46x (Nisaba)** |
| 16 | 500 Alpha Disks (Particle Cloud) | **269.0 µs** | 842.2 µs | **3.13x (Nisaba)** |
| 17 | 1000-Pt Waveform (High-Density Stroke) | **239.1 µs** | 974.9 µs | **4.08x (Nisaba)** |
| 18 | Thick 16px Stroke (Miter & Round Caps) | **273.9 µs** | 104.6 µs | 2.62x (Skia) |
| 19 | 50 Rotated Quads (Affine Alpha Stack) | **548.6 µs** | 655.6 µs | **1.20x (Nisaba)** |
| 20 | 25 Concentric Rings (Alternating Strokes) | **474.7 µs** | 1837.8 µs | **3.87x (Nisaba)** |
| 21 | Diagonal Full-HD Gradient (45° Angle) | **187.6 µs** | 359.9 µs | **1.92x (Nisaba)** |
| 22 | 3-Stage Nested Clipping (Criss-Cross) | **582.9 µs** | 959.1 µs | **1.65x (Nisaba)** |
| 23 | Dashed Cubic Spline (Curved Intervals) | **189.2 µs** | 176.7 µs | 1.07x (Skia) |
| 24 | 40-Point Starburst (Dense Winding Polygon) | **676.9 µs** | 685.8 µs | Tie (~1.0x) |
| 25 | 2-Point Focal Radial Spotlight (Lighting) | **353.5 µs** | 2773.2 µs | **7.84x (Nisaba)** |
| 26 | Ellipse Rendering (25 Ellipses Fill+Stroke) | **912.8 µs** | 985.5 µs | **1.08x (Nisaba)** |
| 27 | Circular Arc Rendering (25 Stroked Arcs) | **593.5 µs** | 740.9 µs | **1.25x (Nisaba)** |
| 28 | Even-Odd Polygon Fill (Self-Intersecting Star) | **156.8 µs** | 72.9 µs | 2.15x (Skia) |
| 29 | Non-Zero Winding Fill (Self-Intersecting Star) | **138.5 µs** | 67.8 µs | 2.04x (Skia) |
| 30 | Direct Pixmap Blit (4x 256x256 1:1 Surfaces) | **56.5 µs** | 139.8 µs | **2.47x (Nisaba)** |
| 31 | Image Bilinear Scaling (2x Scale Up 512x512) | **390.8 µs** | 1047.4 µs | **2.68x (Nisaba)** |
| 32 | Image Rotation & Affine Transform (35° Filtered) | **470.3 µs** | 720.5 µs | **1.53x (Nisaba)** |
| 33 | Image Pattern Fill (Circle Filled with Texture) | **31.1 µs** | 217.9 µs | **7.01x (Nisaba)** |
| 34 | Pattern Repeat & Tiling (64x64 Texture Repeat) | **80.1 µs** | 228.2 µs | **2.85x (Nisaba)** |
| 35 | Anti-Aliasing ON (50 Diagonals Coverage) | **1020.5 µs** | 1349.7 µs | **1.32x (Nisaba)** |
| 36 | Anti-Aliasing OFF (50 Diagonals Aliased) | **1013.1 µs** | 407.3 µs | 2.49x (Skia) |
| 37 | Stroke Join: Miter (Zigzag Polygon 12px) | **205.7 µs** | 344.8 µs | **1.68x (Nisaba)** |
| 38 | Stroke Join: Bevel (Zigzag Polygon 12px) | **205.0 µs** | 205.9 µs | Tie (~1.0x) |
| 39 | Stroke Join: Round (Zigzag Polygon 12px) | **208.1 µs** | 229.3 µs | **1.10x (Nisaba)** |
| 40 | Stroke Cap: Butt (20 Line Segments 14px) | **44.7 µs** | 121.3 µs | **2.72x (Nisaba)** |
| 41 | Stroke Cap: Square (20 Line Segments 14px) | **50.2 µs** | 125.2 µs | **2.49x (Nisaba)** |
| 42 | Stroke Cap: Round (20 Line Segments 14px) | **44.5 µs** | 170.5 µs | **3.83x (Nisaba)** |
| 43 | Transform Isolation: Pure Translate (20 Shapes) | **78.5 µs** | 132.1 µs | **1.68x (Nisaba)** |
| 44 | Transform Isolation: Pure Scale (20 Shapes) | **81.2 µs** | 119.0 µs | **1.47x (Nisaba)** |
| 45 | Transform Isolation: Pure Rotate (20 Shapes) | **361.9 µs** | 319.4 µs | 1.13x (Skia) |
| 46 | Transform Isolation: Pure Shear (20 Shapes) | **261.4 µs** | 232.6 µs | 1.12x (Skia) |
| 47 | Transform Isolation: Combined Affine (20 Shapes) | **346.4 µs** | 322.2 µs | 1.08x (Skia) |
| 48 | State Stack: Save/Restore Overhead (50 Passes) | **0.7 µs** | 4.3 µs | **6.39x (Nisaba)** |
| 49 | Compositing Operator: Multiply (25 Shapes) | **483.9 µs** | 945.8 µs | **1.95x (Nisaba)** |
| 50 | Compositing Operator: Screen (25 Shapes) | **299.3 µs** | 917.3 µs | **3.06x (Nisaba)** |
| 51 | Compositing Operator: Source-In & Xor (25 Shapes) | **336.6 µs** | 893.4 µs | **2.65x (Nisaba)** |
| 52 | Path Complexity: Low (10 Vertices Waveform) | **26.7 µs** | 43.3 µs | **1.62x (Nisaba)** |
| 53 | Path Complexity: Medium (100 Vertices Waveform) | **75.9 µs** | 289.9 µs | **3.82x (Nisaba)** |
| 54 | Path Complexity: High (1000 Vertices Waveform) | **599.9 µs** | 2204.3 µs | **3.67x (Nisaba)** |
| 55 | Primitive Scaling: 100 Rectangles Batch | **85.1 µs** | 123.8 µs | **1.45x (Nisaba)** |
| 56 | Primitive Scaling: 500 Rectangles Batch | **428.4 µs** | 620.4 µs | **1.45x (Nisaba)** |
| 57 | Primitive Scaling: 1000 Rectangles Batch | **851.8 µs** | 1238.3 µs | **1.45x (Nisaba)** |
| 58 | Clip-Depth Scaling: 1-Level Vector Clip | **271.6 µs** | 3047.1 µs | **11.22x (Nisaba)** |
| 59 | Clip-Depth Scaling: 2-Level Nested Vector Clip | **202.3 µs** | 1799.9 µs | **8.90x (Nisaba)** |
| 60 | Clip-Depth Scaling: 4-Level Nested Vector Clip | **186.7 µs** | 1393.9 µs | **7.47x (Nisaba)** |
| 61 | Resolution Scaling: Small (256x256 Viewport) | **177.3 µs** | 238.4 µs | **1.34x (Nisaba)** |
| 62 | Resolution Scaling: Medium (640x480 Viewport) | **346.0 µs** | 745.3 µs | **2.15x (Nisaba)** |
| 63 | Resolution Scaling: Full Viewport (1080x740) | **675.2 µs** | 1759.1 µs | **2.61x (Nisaba)** |
| 64 | Quadratic Bezier Splines (20 Connected Quads) | **681.6 µs** | 687.7 µs | Tie (~1.0x) |
| 65 | Compound Path with Hole (Donut Even-Odd Fill) | **211.7 µs** | 142.7 µs | 1.48x (Skia) |
| 66 | Miter Limit Clamping (Sharp 10° Acute Angles) | **359.6 µs** | 295.9 µs | 1.22x (Skia) |
| 67 | Sub-Pixel Hairline Strokes (0.25px Lines) | **966.9 µs** | 703.6 µs | 1.37x (Skia) |
| 68 | Image Downscaling (4x Minification 0.25x Bilinear) | **8.7 µs** | 17.8 µs | **2.06x (Nisaba)** |
| 69 | Pattern Reflect Tiling (64x64 Texture Reflect) | **835.8 µs** | 628.0 µs | 1.33x (Skia) |
| 70 | Repeated Linear Gradient (60px Periodic Tile) | **158.9 µs** | 862.0 µs | **5.42x (Nisaba)** |
| 71 | Alpha Mask Surface Blit (8-bit Alpha Masking) | **369.0 µs** | 2909.1 µs | **7.88x (Nisaba)** |
| 72 | Blend Modes: Color Dodge & Difference (25 Shapes) | **395.5 µs** | 1376.7 µs | **3.48x (Nisaba)** |
| 73 | Additive Compositing (Plus / Add Blend Mode) | **253.1 µs** | 1146.6 µs | **4.53x (Nisaba)** |
| 74 | Vector Clip on Heavy Curved Stroke (24px Ribbon) | **139.6 µs** | 231.6 µs | **1.66x (Nisaba)** |
| 75 | Animated Dash Offset Phase (Moving Dash Stroke) | **24.1 µs** | 384.3 µs | **15.91x (Nisaba)** |
| 76 | Freeform Gradient Mesh (Bicubic Coons Patch) | **2557.2 µs** | 3407.8 µs | **1.33x (Nisaba)** |
| 77 | True 2-Point Conical Gradient (r0 > 0, r1 > 0) | **711.6 µs** | 4160.2 µs | **5.85x (Nisaba)** |
| 78 | Axis-Aligned Stroked Rectangles (25 Rects) | **53.6 µs** | 65.8 µs | **1.23x (Nisaba)** |
| 79 | Dashed Stroke with Round Caps (Round Intervals) | **17.9 µs** | 679.2 µs | **37.95x (Nisaba)** |
| 80 | Anisotropic Scaled Stroke (scale 2.5x, 0.4y) | **42.2 µs** | 59.4 µs | **1.41x (Nisaba)** |
| 81 | Non-Convex Star Vector Clip (Even-Odd Rule) | **336.0 µs** | 379.9 µs | **1.13x (Nisaba)** |
| 82 | Multi-Contour Complex Path (20 Sub-Paths) | **262.8 µs** | 198.6 µs | 1.32x (Skia) |
| 83 | Blend Modes: Overlay & Soft-Light (25 Shapes) | **558.9 µs** | 1756.6 µs | **3.14x (Nisaba)** |
| 84 | Blend Modes: Darken & Lighten (25 Shapes) | **384.2 µs** | 998.9 µs | **2.60x (Nisaba)** |
| 85 | Blend Modes: Color-Burn & Hard-Light (25 Shapes) | **404.6 µs** | 1504.0 µs | **3.72x (Nisaba)** |
| 86 | Blend Modes: Exclusion & Clear (25 Shapes) | **281.5 µs** | 724.6 µs | **2.57x (Nisaba)** |
| 87 | Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes) | **310.7 µs** | 861.4 µs | **2.77x (Nisaba)** |
| 88 | Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes) | **317.3 µs** | 895.7 µs | **2.82x (Nisaba)** |
| 89 | HSL Blend Modes: Hue & Luminosity (25 Shapes) | **554.9 µs** | 3163.7 µs | **5.70x (Nisaba)** |
| 90 | Bilinear Filtered Pattern under Rotation (30° Tiling) | **2204.4 µs** | 9074.2 µs | **4.12x (Nisaba)** |
| **--** | **TOTAL FRAME OVERHEAD** | **31338.3 µs** | **75118.2 µs** | **2.40x (Nisaba)** |

---

## Individual Run Logs (Runs 1 to 10)

### Run 1
<details>
<summary>Click to expand Run 1 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    90.9 µs    122.6 µs    1.35x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   80.7 µs    113.2 µs    1.40x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.6 µs    167.5 µs    2.48x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                222.4 µs    189.7 µs      1.17x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                67.3 µs    169.0 µs    2.51x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs      8.8 µs      1.15x (Skia)
7. Radial Glow Gradient (250px Glow)                  322.5 µs    960.4 µs    2.98x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     77.9 µs    2.23x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                34.1 µs    205.0 µs    6.01x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               47.0 µs     35.8 µs      1.31x (Skia)
11. Compound Transforms (16 Elements)                  82.2 µs     86.2 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             14.3 µs     28.5 µs    1.99x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1118.4 µs   1425.9 µs    1.27x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             164.5 µs    299.3 µs    1.82x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               80.8 µs    117.0 µs    1.45x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  278.4 µs    910.9 µs    3.27x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            254.1 µs   1057.6 µs    4.16x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            304.1 µs    112.2 µs      2.71x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             579.9 µs    689.1 µs    1.19x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         510.6 µs   1933.7 µs    3.79x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            196.6 µs    379.5 µs    1.93x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             606.0 µs   1000.3 µs    1.65x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            200.3 µs    184.8 µs      1.08x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        707.1 µs    717.0 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         374.1 µs   2857.7 µs    7.64x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       937.4 µs   1042.2 µs    1.11x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          609.5 µs    757.5 µs    1.24x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    160.6 µs     74.6 µs      2.15x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    141.1 µs     69.5 µs      2.03x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.3 µs    143.2 µs    2.41x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      398.6 µs   1065.0 µs    2.67x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    486.9 µs    746.4 µs    1.53x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.5 µs    224.0 µs    6.90x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     83.6 µs    234.7 µs    2.81x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1080.4 µs   1424.3 µs    1.32x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1041.1 µs    417.8 µs      2.49x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          210.8 µs    354.2 µs    1.68x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          210.8 µs    215.5 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          217.5 µs    240.0 µs    1.10x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           48.6 µs    126.9 µs    2.61x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.7 µs    130.2 µs    2.47x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.9 µs    178.7 µs    3.81x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.7 µs    137.6 µs    1.66x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        86.1 µs    124.3 µs    1.44x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      381.6 µs    337.5 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       285.1 µs    242.8 µs      1.17x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    364.5 µs    339.4 µs      1.07x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.8 µs    6.87x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        506.5 µs    973.4 µs    1.92x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          310.7 µs    947.6 µs    3.05x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    347.1 µs    947.4 µs    2.73x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        28.6 µs     45.8 µs    1.60x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     80.7 µs    308.3 µs    3.82x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    641.5 µs   2324.0 µs    3.62x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            90.1 µs    129.9 µs    1.44x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           453.1 µs    649.4 µs    1.43x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          904.1 µs   1306.9 µs    1.45x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           289.1 µs   3245.5 µs   11.23x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    213.6 µs   1896.0 µs    8.88x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    203.3 µs   1481.4 µs    7.29x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      189.2 µs    252.7 µs    1.34x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     366.3 µs    783.7 µs    2.14x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      707.7 µs   1831.7 µs    2.59x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     713.5 µs    721.4 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     221.3 µs    149.8 µs      1.48x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    376.4 µs    308.6 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1009.0 µs    736.4 µs      1.37x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.1 µs     18.6 µs    2.04x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    876.7 µs    654.2 µs      1.34x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     164.7 µs    898.7 µs    5.46x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     392.4 µs   3088.0 µs    7.87x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    411.3 µs   1420.8 µs    3.45x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      268.1 µs   1188.3 µs    4.43x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    144.9 µs    237.7 µs    1.64x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.0 µs    397.6 µs   15.92x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2636.9 µs   3590.1 µs    1.36x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    745.5 µs   4330.0 µs    5.81x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.6 µs     68.2 µs    1.23x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     18.6 µs    697.3 µs   37.48x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       43.3 µs     61.0 µs    1.41x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       349.9 µs    392.4 µs    1.12x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         282.3 µs    222.6 µs      1.27x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     586.8 µs   1839.1 µs    3.13x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         403.6 µs   1042.2 µs    2.58x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    425.7 µs   1586.3 µs    3.73x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        296.6 µs    772.6 µs    2.60x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    323.4 µs    912.6 µs    2.82x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    340.8 µs    941.2 µs    2.76x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     572.4 µs   3281.4 µs    5.73x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2299.3 µs   9493.6 µs    4.13x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32793.3 µs  78655.1 µs    2.40x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 74 / 90 tests (82.2%)
    * Skia Won:   16 / 90 tests (17.8%)
=================================================================================================================================================================
```
</details>

### Run 2
<details>
<summary>Click to expand Run 2 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    94.8 µs    126.2 µs    1.33x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   82.0 µs    115.2 µs    1.41x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.6 µs    171.9 µs    2.54x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                207.8 µs    189.1 µs      1.10x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                67.2 µs    170.9 µs    2.54x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.2 µs      8.8 µs      1.16x (Skia)
7. Radial Glow Gradient (250px Glow)                  322.8 µs    955.0 µs    2.96x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     78.4 µs    2.24x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.8 µs    204.8 µs    6.05x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.8 µs     35.4 µs      1.32x (Skia)
11. Compound Transforms (16 Elements)                  82.0 µs     86.0 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             14.3 µs     28.7 µs    2.01x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1119.9 µs   1421.5 µs    1.27x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             163.0 µs    301.5 µs    1.85x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               80.4 µs    117.0 µs    1.46x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  276.4 µs    878.8 µs    3.18x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            245.8 µs    999.3 µs    4.07x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            280.7 µs    107.7 µs      2.61x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             564.4 µs    671.0 µs    1.19x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         488.2 µs   1879.1 µs    3.85x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            202.3 µs    377.4 µs    1.87x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             606.2 µs   1000.4 µs    1.65x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            197.6 µs    184.8 µs      1.07x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        696.3 µs    694.3 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         363.0 µs   2868.9 µs    7.90x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       943.7 µs   1028.8 µs    1.09x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          611.4 µs    760.1 µs    1.24x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    161.3 µs     74.8 µs      2.16x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    142.2 µs     69.6 µs      2.04x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.2 µs    143.3 µs    2.42x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      398.4 µs   1068.7 µs    2.68x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    489.2 µs    745.2 µs    1.52x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.4 µs    224.4 µs    6.92x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     83.0 µs    234.5 µs    2.83x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1040.6 µs   1385.6 µs    1.33x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1041.8 µs    442.2 µs      2.36x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          210.6 µs    353.4 µs    1.68x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          210.5 µs    210.2 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          220.6 µs    240.4 µs    1.09x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           47.1 µs    130.0 µs    2.76x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         53.0 µs    131.5 µs    2.48x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          47.0 µs    179.8 µs    3.83x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     83.2 µs    140.3 µs    1.69x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        85.6 µs    125.9 µs    1.47x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      379.6 µs    335.0 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       272.6 µs    244.6 µs      1.11x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    364.6 µs    341.5 µs      1.07x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.4 µs    6.25x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        510.5 µs    978.7 µs    1.92x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          316.4 µs    953.4 µs    3.01x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    348.6 µs    949.0 µs    2.72x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        28.2 µs     45.6 µs    1.62x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     80.9 µs    309.1 µs    3.82x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    627.1 µs   2294.3 µs    3.66x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            90.0 µs    130.0 µs    1.44x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           457.8 µs    660.6 µs    1.44x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          904.1 µs   1310.9 µs    1.45x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           286.9 µs   3174.9 µs   11.07x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    216.1 µs   1906.2 µs    8.82x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    196.7 µs   1450.6 µs    7.37x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      183.8 µs    247.3 µs    1.35x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     358.2 µs    786.1 µs    2.19x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      706.9 µs   1851.1 µs    2.62x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     712.8 µs    720.6 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     220.9 µs    148.3 µs      1.49x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    374.6 µs    306.7 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1007.0 µs    738.0 µs      1.36x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.1 µs     18.7 µs    2.07x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    876.7 µs    658.4 µs      1.33x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     164.7 µs    890.0 µs    5.40x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     383.4 µs   3011.9 µs    7.86x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    411.1 µs   1438.1 µs    3.50x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      268.7 µs   1224.6 µs    4.56x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    149.4 µs    250.3 µs    1.68x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.9 µs    410.7 µs   15.88x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2659.3 µs   3519.3 µs    1.32x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    740.4 µs   4341.1 µs    5.86x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.6 µs     68.3 µs    1.23x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     18.5 µs    713.0 µs   38.44x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       44.7 µs     62.6 µs    1.40x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       349.3 µs    396.6 µs    1.14x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         271.3 µs    205.8 µs      1.32x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     586.8 µs   1818.3 µs    3.10x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         403.2 µs   1054.5 µs    2.62x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    431.2 µs   1585.7 µs    3.68x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        295.9 µs    758.5 µs    2.56x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    324.1 µs    898.5 µs    2.77x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    331.1 µs    933.8 µs    2.82x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     592.9 µs   3296.8 µs    5.56x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2298.0 µs   9463.3 µs    4.12x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32645.8 µs  78266.3 µs    2.40x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 72 / 90 tests (80.0%)
    * Skia Won:   18 / 90 tests (20.0%)
=================================================================================================================================================================
```
</details>

### Run 3
<details>
<summary>Click to expand Run 3 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    91.2 µs    125.9 µs    1.38x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   80.6 µs    111.4 µs    1.38x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                68.6 µs    170.7 µs    2.49x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                209.2 µs    189.0 µs      1.11x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                67.1 µs    169.5 µs    2.53x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs      8.9 µs      1.14x (Skia)
7. Radial Glow Gradient (250px Glow)                  322.6 µs    954.8 µs    2.96x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     77.5 µs    2.22x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.8 µs    203.7 µs    6.02x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               47.1 µs     35.6 µs      1.32x (Skia)
11. Compound Transforms (16 Elements)                  81.9 µs     86.1 µs    1.05x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             14.1 µs     28.4 µs    2.01x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1123.9 µs   1411.2 µs    1.26x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             159.6 µs    295.7 µs    1.85x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               80.4 µs    116.9 µs    1.45x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  280.4 µs    857.6 µs    3.06x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            245.5 µs    998.9 µs    4.07x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            279.5 µs    107.2 µs      2.61x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             562.8 µs    685.6 µs    1.22x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         487.5 µs   1919.0 µs    3.94x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            196.0 µs    375.2 µs    1.91x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             590.0 µs    967.8 µs    1.64x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            191.0 µs    178.8 µs      1.07x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        683.2 µs    696.7 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         363.7 µs   2883.3 µs    7.93x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       939.6 µs   1021.2 µs    1.09x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          611.4 µs    767.2 µs    1.25x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    161.1 µs     74.6 µs      2.16x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    141.9 µs     69.5 µs      2.04x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       57.1 µs    142.4 µs    2.49x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      407.9 µs   1093.4 µs    2.68x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    479.5 µs    743.3 µs    1.55x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     31.7 µs    224.3 µs    7.08x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.3 µs    234.1 µs    2.85x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1035.5 µs   1381.7 µs    1.33x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1039.5 µs    412.4 µs      2.52x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          211.5 µs    354.0 µs    1.67x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          210.5 µs    209.3 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          208.3 µs    231.6 µs    1.11x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           44.1 µs    121.4 µs    2.75x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         51.9 µs    129.9 µs    2.50x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.4 µs    177.3 µs    3.82x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.4 µs    138.1 µs    1.68x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        85.3 µs    124.4 µs    1.46x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      378.6 µs    333.6 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       272.2 µs    243.1 µs      1.12x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    364.1 µs    330.0 µs      1.10x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.3 µs    6.38x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        514.2 µs   1006.6 µs    1.96x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          327.5 µs    989.3 µs    3.02x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    356.1 µs    927.3 µs    2.60x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.7 µs     44.1 µs    1.59x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     77.8 µs    298.7 µs    3.84x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    620.0 µs   2295.4 µs    3.70x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            88.3 µs    128.5 µs    1.45x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           445.8 µs    650.2 µs    1.46x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          886.4 µs   1288.3 µs    1.45x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           282.0 µs   3177.1 µs   11.27x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    213.1 µs   1877.6 µs    8.81x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    193.5 µs   1428.8 µs    7.38x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      182.5 µs    246.5 µs    1.35x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     358.9 µs    784.8 µs    2.19x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      709.0 µs   1871.4 µs    2.64x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     721.9 µs    718.5 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     220.8 µs    148.8 µs      1.48x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    371.3 µs    306.0 µs      1.21x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1006.8 µs    740.8 µs      1.36x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.0 µs     18.6 µs    2.06x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    878.2 µs    664.2 µs      1.32x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     169.2 µs    913.4 µs    5.40x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     389.7 µs   3013.1 µs    7.73x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    413.8 µs   1411.3 µs    3.41x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      257.7 µs   1175.9 µs    4.56x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    143.7 µs    237.6 µs    1.65x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     24.9 µs    397.6 µs   15.97x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2652.2 µs   3657.7 µs    1.38x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    737.2 µs   4324.5 µs    5.87x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.1 µs     67.8 µs    1.23x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     18.5 µs    698.7 µs   37.79x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       43.8 µs     62.3 µs    1.42x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       348.7 µs    393.0 µs    1.13x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         270.6 µs    202.0 µs      1.34x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     580.3 µs   1853.3 µs    3.19x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         406.0 µs   1063.0 µs    2.62x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    424.1 µs   1579.9 µs    3.73x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        295.9 µs    757.4 µs    2.56x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    324.2 µs    894.7 µs    2.76x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    337.2 µs    945.2 µs    2.80x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     594.6 µs   3285.8 µs    5.53x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2308.2 µs   9440.3 µs    4.09x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32534.4 µs  78202.1 µs    2.40x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 72 / 90 tests (80.0%)
    * Skia Won:   18 / 90 tests (20.0%)
=================================================================================================================================================================
```
</details>

### Run 4
<details>
<summary>Click to expand Run 4 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    92.2 µs    125.5 µs    1.36x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   81.8 µs    112.7 µs    1.38x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.8 µs    171.4 µs    2.53x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                205.6 µs    183.5 µs      1.12x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                65.2 µs    166.2 µs    2.55x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs      8.7 µs      1.16x (Skia)
7. Radial Glow Gradient (250px Glow)                  322.5 µs    980.2 µs    3.04x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     77.8 µs    2.22x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.9 µs    205.4 µs    6.06x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               47.2 µs     35.4 µs      1.33x (Skia)
11. Compound Transforms (16 Elements)                  82.3 µs     85.2 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             14.3 µs     28.7 µs    2.00x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1135.8 µs   1438.1 µs    1.27x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             164.9 µs    306.4 µs    1.86x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               81.3 µs    117.0 µs    1.44x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  279.6 µs    873.3 µs    3.12x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            246.3 µs   1004.0 µs    4.08x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            282.1 µs    108.2 µs      2.61x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             568.6 µs    676.3 µs    1.19x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         489.5 µs   1883.1 µs    3.85x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.3 µs    364.9 µs    1.92x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             601.6 µs   1010.3 µs    1.68x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            199.7 µs    189.7 µs      1.05x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        707.3 µs    714.5 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         366.0 µs   2854.1 µs    7.80x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       937.7 µs   1004.9 µs    1.07x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          612.1 µs    770.5 µs    1.26x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    161.5 µs     75.6 µs      2.14x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    143.2 µs     69.7 µs      2.05x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.0 µs    143.2 µs    2.43x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      398.9 µs   1087.3 µs    2.73x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    482.5 µs    743.4 µs    1.54x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.4 µs    225.4 µs    6.96x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     83.2 µs    234.3 µs    2.82x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1096.4 µs   1388.6 µs    1.27x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1046.0 µs    421.8 µs      2.48x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          208.3 µs    343.1 µs    1.65x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          199.2 µs    197.9 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          198.5 µs    215.2 µs    1.08x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           41.7 µs    112.6 µs    2.70x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         46.8 µs    116.5 µs    2.49x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          41.6 µs    160.3 µs    3.85x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     73.5 µs    122.5 µs    1.67x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        74.1 µs    108.8 µs    1.47x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      328.6 µs    291.0 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       235.1 µs    213.2 µs      1.10x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    305.9 µs    286.9 µs      1.07x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.6 µs      3.8 µs    6.44x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        432.6 µs    854.7 µs    1.98x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          266.1 µs    809.7 µs    3.04x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    303.2 µs    781.1 µs    2.58x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        23.4 µs     37.4 µs    1.60x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     66.0 µs    252.3 µs    3.82x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    525.5 µs   1963.7 µs    3.74x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            76.5 µs    109.8 µs    1.44x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           389.3 µs    569.2 µs    1.46x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          775.5 µs   1100.5 µs    1.42x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           240.5 µs   2659.7 µs   11.06x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    176.9 µs   1594.7 µs    9.01x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    166.5 µs   1224.0 µs    7.35x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      156.3 µs    210.7 µs    1.35x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     302.4 µs    649.8 µs    2.15x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      588.6 µs   1586.3 µs    2.70x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     610.8 µs    610.4 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     186.2 µs    128.1 µs      1.45x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    316.9 µs    260.3 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         852.7 µs    620.7 µs      1.37x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      7.6 µs     15.8 µs    2.07x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    729.6 µs    544.6 µs      1.34x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     139.8 µs    754.7 µs    5.40x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     323.3 µs   2574.8 µs    7.96x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    348.9 µs   1226.7 µs    3.52x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      226.2 µs   1007.8 µs    4.45x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    122.2 µs    201.5 µs    1.65x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     21.1 µs    336.0 µs   15.89x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2280.5 µs   2975.0 µs    1.30x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    622.9 µs   3643.4 µs    5.85x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         47.5 µs     57.9 µs    1.22x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     15.9 µs    607.5 µs   38.25x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       36.9 µs     52.1 µs    1.41x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       294.8 µs    333.6 µs    1.13x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         230.6 µs    175.0 µs      1.32x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     498.1 µs   1585.4 µs    3.18x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         342.0 µs    905.3 µs    2.65x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    361.2 µs   1359.2 µs    3.76x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        255.6 µs    643.8 µs    2.52x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    273.8 µs    759.7 µs    2.77x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    279.4 µs    786.9 µs    2.82x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     492.7 µs   2787.3 µs    5.66x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   1957.4 µs   8067.3 µs    4.12x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                29541.8 µs  69481.5 µs    2.35x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 72 / 90 tests (80.0%)
    * Skia Won:   18 / 90 tests (20.0%)
=================================================================================================================================================================
```
</details>

### Run 5
<details>
<summary>Click to expand Run 5 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    77.6 µs    106.4 µs    1.37x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   68.1 µs     94.5 µs    1.39x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                57.2 µs    141.3 µs    2.47x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                174.9 µs    162.6 µs      1.08x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                57.0 µs    144.6 µs    2.54x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  8.5 µs      7.4 µs      1.16x (Skia)
7. Radial Glow Gradient (250px Glow)                  275.4 µs    830.3 µs    3.02x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  30.0 µs     68.4 µs    2.28x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                29.0 µs    177.5 µs    6.12x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               40.6 µs     30.9 µs      1.31x (Skia)
11. Compound Transforms (16 Elements)                  71.1 µs     74.4 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             12.3 µs     24.8 µs    2.01x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)                967.5 µs   1202.5 µs    1.24x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             139.3 µs    254.4 µs    1.83x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               68.2 µs     98.5 µs    1.44x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  233.9 µs    749.9 µs    3.21x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            213.6 µs    867.3 µs    4.06x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            238.6 µs     91.9 µs      2.60x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             490.0 µs    584.9 µs    1.19x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         422.9 µs   1599.1 µs    3.78x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            160.5 µs    312.5 µs    1.95x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             506.1 µs    823.7 µs    1.63x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            166.2 µs    154.5 µs      1.08x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        595.2 µs    599.5 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         308.8 µs   2410.9 µs    7.81x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       792.0 µs    849.8 µs    1.07x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          525.3 µs    668.2 µs    1.27x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    139.7 µs     64.8 µs      2.16x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    126.3 µs     61.2 µs      2.06x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       49.4 µs    123.6 µs    2.50x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      346.2 µs    940.1 µs    2.72x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    408.2 µs    628.5 µs    1.54x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     26.8 µs    189.5 µs    7.06x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     69.6 µs    197.9 µs    2.84x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)          867.2 µs   1166.8 µs    1.35x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)          882.6 µs    356.4 µs      2.48x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          184.1 µs    305.9 µs    1.66x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          182.7 µs    181.8 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          184.4 µs    203.4 µs    1.10x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           39.1 µs    107.5 µs    2.75x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         43.9 µs    111.5 µs    2.54x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          39.1 µs    151.5 µs    3.88x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     68.1 µs    119.2 µs    1.75x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        71.5 µs    107.2 µs    1.50x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      320.2 µs    283.1 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       228.5 µs    205.0 µs      1.11x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    307.6 µs    286.9 µs      1.07x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.6 µs      3.8 µs    6.48x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        427.6 µs    828.0 µs    1.94x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          264.5 µs    835.1 µs    3.16x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    301.2 µs    797.0 µs    2.65x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        23.8 µs     38.5 µs    1.62x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     68.9 µs    261.0 µs    3.79x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    537.0 µs   1956.9 µs    3.64x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            74.3 µs    108.5 µs    1.46x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           372.4 µs    543.0 µs    1.46x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          745.5 µs   1105.0 µs    1.48x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           246.7 µs   2730.1 µs   11.07x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    181.3 µs   1614.3 µs    8.90x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    164.0 µs   1249.9 µs    7.62x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      160.8 µs    214.0 µs    1.33x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     309.6 µs    671.7 µs    2.17x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      604.5 µs   1547.2 µs    2.56x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     603.7 µs    614.4 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     190.2 µs    128.7 µs      1.48x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    324.4 µs    266.6 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         877.7 µs    640.5 µs      1.37x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      7.9 µs     16.2 µs    2.06x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    748.5 µs    561.2 µs      1.33x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     143.1 µs    775.4 µs    5.42x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     330.7 µs   2638.6 µs    7.98x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    358.3 µs   1270.4 µs    3.55x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      230.2 µs   1033.7 µs    4.49x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    128.0 µs    212.4 µs    1.66x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     22.4 µs    352.2 µs   15.69x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2313.6 µs   3065.1 µs    1.32x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    643.5 µs   3842.9 µs    5.97x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         50.8 µs     62.1 µs    1.22x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     16.8 µs    648.2 µs   38.55x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       39.9 µs     56.0 µs    1.40x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       326.5 µs    360.6 µs    1.10x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         247.3 µs    185.8 µs      1.33x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     523.3 µs   1668.5 µs    3.19x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         363.8 µs    935.5 µs    2.57x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    390.5 µs   1457.6 µs    3.73x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        270.7 µs    696.6 µs    2.57x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    295.8 µs    819.2 µs    2.77x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    302.8 µs    848.2 µs    2.80x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     524.3 µs   3010.2 µs    5.74x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2111.3 µs   8952.7 µs    4.24x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                28184.1 µs  68545.8 µs    2.43x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 73 / 90 tests (81.1%)
    * Skia Won:   17 / 90 tests (18.9%)
=================================================================================================================================================================
```
</details>

### Run 6
<details>
<summary>Click to expand Run 6 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    90.6 µs    121.4 µs    1.34x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   77.6 µs    108.6 µs    1.40x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                65.3 µs    168.5 µs    2.58x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                203.0 µs    182.9 µs      1.11x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                64.9 µs    164.1 µs    2.53x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  9.8 µs      8.5 µs      1.15x (Skia)
7. Radial Glow Gradient (250px Glow)                  314.9 µs    920.7 µs    2.92x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  33.4 µs     75.2 µs    2.25x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                32.5 µs    198.1 µs    6.09x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               45.4 µs     34.7 µs      1.31x (Skia)
11. Compound Transforms (16 Elements)                  79.3 µs     82.6 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             13.8 µs     27.6 µs    2.00x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1050.3 µs   1338.5 µs    1.27x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             155.2 µs    280.4 µs    1.81x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               75.8 µs    111.5 µs    1.47x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  268.9 µs    834.3 µs    3.10x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            239.4 µs    970.8 µs    4.06x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            272.4 µs    104.4 µs      2.61x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             545.0 µs    641.7 µs    1.18x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         461.8 µs   1818.9 µs    3.94x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.3 µs    371.7 µs    1.95x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             606.0 µs    976.3 µs    1.61x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            191.7 µs    178.1 µs      1.08x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        699.1 µs    716.7 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         362.7 µs   2855.3 µs    7.87x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       959.9 µs   1022.1 µs    1.06x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          610.6 µs    757.3 µs    1.24x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    164.3 µs     76.8 µs      2.14x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    145.7 µs     71.5 µs      2.04x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.2 µs    146.9 µs    2.48x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      410.9 µs   1082.2 µs    2.63x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    485.6 µs    744.3 µs    1.53x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     31.7 µs    225.8 µs    7.11x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.0 µs    233.7 µs    2.85x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1061.2 µs   1404.1 µs    1.32x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1036.4 µs    412.6 µs      2.51x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          209.9 µs    354.0 µs    1.69x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          212.8 µs    215.2 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          216.5 µs    241.2 µs    1.11x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           46.5 µs    126.9 µs    2.73x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.5 µs    130.2 µs    2.48x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.4 µs    177.5 µs    3.82x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.0 µs    137.1 µs    1.67x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        84.9 µs    124.4 µs    1.47x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      379.7 µs    334.9 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       271.9 µs    241.3 µs      1.13x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    363.9 µs    338.3 µs      1.08x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.6 µs    6.59x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        508.9 µs   1006.1 µs    1.98x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          308.4 µs    951.9 µs    3.09x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    360.4 µs    958.6 µs    2.66x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        28.8 µs     48.4 µs    1.68x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     80.6 µs    308.3 µs    3.83x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    640.0 µs   2349.4 µs    3.67x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            88.4 µs    128.9 µs    1.46x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           443.5 µs    643.9 µs    1.45x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          890.2 µs   1292.7 µs    1.45x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           282.4 µs   3192.9 µs   11.31x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    211.5 µs   1897.4 µs    8.97x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    193.1 µs   1494.4 µs    7.74x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      188.4 µs    252.6 µs    1.34x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     363.9 µs    783.2 µs    2.15x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      707.8 µs   1812.5 µs    2.56x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     709.7 µs    724.4 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     219.6 µs    148.2 µs      1.48x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    383.1 µs    317.8 µs      1.21x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1022.2 µs    731.4 µs      1.40x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.0 µs     18.7 µs    2.07x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    876.7 µs    661.4 µs      1.33x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     164.2 µs    913.4 µs    5.56x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     392.8 µs   3038.9 µs    7.74x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    420.8 µs   1449.6 µs    3.45x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      264.6 µs   1197.4 µs    4.53x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    148.8 µs    249.3 µs    1.68x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.7 µs    409.7 µs   15.95x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2659.7 µs   3623.2 µs    1.36x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    767.5 µs   4343.2 µs    5.66x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.5 µs     69.1 µs    1.25x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     18.4 µs    702.6 µs   38.17x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       44.3 µs     62.4 µs    1.41x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       349.7 µs    396.2 µs    1.13x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         273.8 µs    209.2 µs      1.31x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     584.2 µs   1819.3 µs    3.11x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         403.1 µs   1044.8 µs    2.59x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    431.8 µs   1579.1 µs    3.66x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        296.9 µs    760.9 µs    2.56x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    332.7 µs    923.5 µs    2.78x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    333.5 µs    925.8 µs    2.78x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     571.5 µs   3315.1 µs    5.80x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2309.2 µs   9513.8 µs    4.12x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32529.6 µs  78164.1 µs    2.40x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 74 / 90 tests (82.2%)
    * Skia Won:   16 / 90 tests (17.8%)
=================================================================================================================================================================
```
</details>

### Run 7
<details>
<summary>Click to expand Run 7 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    90.9 µs    127.3 µs    1.40x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   81.9 µs    113.8 µs    1.39x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                69.4 µs    171.7 µs    2.47x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                212.0 µs    191.4 µs      1.11x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                67.3 µs    169.8 µs    2.52x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.2 µs      8.7 µs      1.17x (Skia)
7. Radial Glow Gradient (250px Glow)                  322.6 µs    955.0 µs    2.96x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     77.7 µs    2.22x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.8 µs    205.7 µs    6.08x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.9 µs     35.9 µs      1.31x (Skia)
11. Compound Transforms (16 Elements)                  82.3 µs     85.9 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             14.4 µs     28.7 µs    2.00x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1119.6 µs   1453.4 µs    1.30x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             164.6 µs    298.5 µs    1.81x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               81.0 µs    116.8 µs    1.44x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  281.7 µs    864.3 µs    3.07x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            246.2 µs   1003.8 µs    4.08x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            281.1 µs    108.3 µs      2.60x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             563.8 µs    668.7 µs    1.19x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         488.3 µs   1927.2 µs    3.95x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            194.8 µs    375.9 µs    1.93x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             605.3 µs    970.7 µs    1.60x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            192.2 µs    179.6 µs      1.07x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        683.9 µs    693.2 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         362.8 µs   2893.6 µs    7.98x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       939.3 µs   1012.3 µs    1.08x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          609.6 µs    760.4 µs    1.25x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    160.9 µs     75.1 µs      2.14x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    141.1 µs     69.5 µs      2.03x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.0 µs    143.1 µs    2.43x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      401.2 µs   1083.8 µs    2.70x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    482.1 µs    744.4 µs    1.54x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.5 µs    225.2 µs    6.92x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     83.3 µs    235.0 µs    2.82x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1043.2 µs   1382.0 µs    1.32x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1066.8 µs    429.1 µs      2.49x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          217.6 µs    366.9 µs    1.69x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          217.2 µs    222.3 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          219.0 µs    240.9 µs    1.10x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           47.0 µs    127.0 µs    2.70x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.8 µs    130.8 µs    2.48x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.9 µs    178.9 µs    3.81x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.8 µs    138.8 µs    1.68x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        85.6 µs    124.6 µs    1.45x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      379.4 µs    334.5 µs      1.13x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       272.4 µs    241.5 µs      1.13x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    352.9 µs    328.5 µs      1.07x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.4 µs    6.33x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        494.7 µs    983.4 µs    1.99x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          310.2 µs    953.8 µs    3.07x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    357.7 µs    945.8 µs    2.64x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        28.2 µs     45.7 µs    1.62x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     79.3 µs    299.5 µs    3.77x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    622.0 µs   2297.5 µs    3.69x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            90.2 µs    133.4 µs    1.48x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           456.7 µs    664.1 µs    1.45x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          909.5 µs   1323.8 µs    1.46x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           284.4 µs   3187.1 µs   11.21x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    213.7 µs   1891.2 µs    8.85x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    196.8 µs   1450.5 µs    7.37x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      183.2 µs    247.2 µs    1.35x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     355.9 µs    763.5 µs    2.15x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      715.5 µs   1859.7 µs    2.60x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     726.3 µs    720.1 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     221.1 µs    148.6 µs      1.49x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    374.7 µs    307.0 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1007.5 µs    738.2 µs      1.36x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.1 µs     18.8 µs    2.06x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    877.4 µs    676.6 µs      1.30x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     169.8 µs    917.2 µs    5.40x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     392.2 µs   3013.7 µs    7.68x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    410.9 µs   1434.4 µs    3.49x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      266.7 µs   1201.8 µs    4.51x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    144.8 µs    241.4 µs    1.67x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.0 µs    397.5 µs   15.91x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2635.9 µs   3504.3 µs    1.33x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    734.9 µs   4357.6 µs    5.93x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         57.6 µs     70.2 µs    1.22x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     19.1 µs    719.5 µs   37.62x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       44.7 µs     62.4 µs    1.39x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       349.0 µs    396.0 µs    1.13x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         271.9 µs    204.2 µs      1.33x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     586.7 µs   1840.0 µs    3.14x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         410.6 µs   1054.3 µs    2.57x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    423.3 µs   1578.9 µs    3.73x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        295.3 µs    757.7 µs    2.57x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    323.1 µs    896.2 µs    2.77x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    330.2 µs    951.1 µs    2.88x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     591.9 µs   3627.3 µs    6.13x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2371.4 µs   9557.5 µs    4.03x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32694.6 µs  78763.3 µs    2.41x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 73 / 90 tests (81.1%)
    * Skia Won:   17 / 90 tests (18.9%)
=================================================================================================================================================================
```
</details>

### Run 8
<details>
<summary>Click to expand Run 8 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    91.3 µs    125.1 µs    1.37x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   80.6 µs    112.2 µs    1.39x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.0 µs    167.2 µs    2.49x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                204.2 µs    183.5 µs      1.11x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                66.2 µs    168.7 µs    2.55x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs      8.7 µs      1.15x (Skia)
7. Radial Glow Gradient (250px Glow)                  322.6 µs    955.5 µs    2.96x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     78.0 µs    2.23x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.8 µs    206.2 µs    6.10x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               47.0 µs     35.5 µs      1.33x (Skia)
11. Compound Transforms (16 Elements)                  81.4 µs     85.4 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             14.3 µs     28.8 µs    2.01x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1117.1 µs   1412.8 µs    1.26x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             158.7 µs    287.5 µs    1.81x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               78.4 µs    116.5 µs    1.49x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  276.6 µs    863.4 µs    3.12x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            245.7 µs   1003.5 µs    4.08x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            284.6 µs    108.3 µs      2.63x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             568.3 µs    706.7 µs    1.24x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         498.9 µs   1923.4 µs    3.86x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.3 µs    363.3 µs    1.91x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             588.8 µs    968.5 µs    1.64x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            190.8 µs    177.9 µs      1.07x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        697.0 µs    719.0 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         363.2 µs   2857.5 µs    7.87x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       928.8 µs   1010.8 µs    1.09x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          615.3 µs    763.0 µs    1.24x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    160.7 µs     74.8 µs      2.15x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    140.9 µs     69.5 µs      2.03x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       58.1 µs    149.2 µs    2.57x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      411.0 µs   1083.0 µs    2.64x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    478.8 µs    743.4 µs    1.55x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     31.8 µs    224.1 µs    7.06x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.1 µs    233.7 µs    2.85x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1037.5 µs   1386.6 µs    1.34x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1053.3 µs    427.4 µs      2.46x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          216.8 µs    363.9 µs    1.68x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          216.8 µs    215.4 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          217.2 µs    239.3 µs    1.10x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           46.4 µs    126.4 µs    2.72x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.3 µs    130.0 µs    2.49x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          45.0 µs    172.5 µs    3.83x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     79.5 µs    133.0 µs    1.67x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        81.6 µs    119.4 µs    1.46x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      366.1 µs    329.7 µs      1.11x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       273.6 µs    247.4 µs      1.11x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    367.6 µs    339.6 µs      1.08x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.5 µs    6.34x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        509.7 µs   1007.2 µs    1.98x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          318.6 µs    970.8 µs    3.05x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    345.9 µs    917.1 µs    2.65x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.5 µs     44.0 µs    1.60x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     78.2 µs    299.0 µs    3.82x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    619.7 µs   2293.6 µs    3.70x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            88.4 µs    128.7 µs    1.46x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           443.1 µs    639.4 µs    1.44x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          875.0 µs   1277.3 µs    1.46x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           284.1 µs   3260.3 µs   11.47x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    214.4 µs   1873.3 µs    8.74x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    194.2 µs   1429.0 µs    7.36x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      181.9 µs    247.6 µs    1.36x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     369.3 µs    783.9 µs    2.12x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      705.3 µs   1828.2 µs    2.59x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     700.6 µs    718.2 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     220.0 µs    148.0 µs      1.49x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    373.8 µs    306.4 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1016.3 µs    735.9 µs      1.38x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      8.9 µs     18.6 µs    2.09x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    877.5 µs    660.8 µs      1.33x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     169.3 µs    917.5 µs    5.42x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     381.3 µs   3021.2 µs    7.92x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    411.0 µs   1485.3 µs    3.61x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      264.9 µs   1232.5 µs    4.65x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    144.0 µs    237.5 µs    1.65x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     24.9 µs    396.1 µs   15.88x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2699.6 µs   3561.9 µs    1.32x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    741.3 µs   4325.2 µs    5.83x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.0 µs     67.6 µs    1.23x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     18.5 µs    698.4 µs   37.78x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       43.4 µs     60.9 µs    1.40x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       346.4 µs    392.8 µs    1.13x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         269.3 µs    201.5 µs      1.34x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     580.4 µs   1881.7 µs    3.24x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         407.8 µs   1051.7 µs    2.58x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    422.3 µs   1576.9 µs    3.73x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        294.4 µs    755.7 µs    2.57x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    325.3 µs    900.1 µs    2.77x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    329.1 µs    944.8 µs    2.87x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     580.9 µs   3272.2 µs    5.63x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2357.0 µs   9637.3 µs    4.09x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32592.2 µs  78455.9 µs    2.41x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 73 / 90 tests (81.1%)
    * Skia Won:   17 / 90 tests (18.9%)
=================================================================================================================================================================
```
</details>

### Run 9
<details>
<summary>Click to expand Run 9 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    89.8 µs    125.1 µs    1.39x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   80.2 µs    111.8 µs    1.39x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.3 µs    166.5 µs    2.47x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                209.9 µs    190.8 µs      1.10x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                66.7 µs    170.4 µs    2.56x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs      8.9 µs      1.13x (Skia)
7. Radial Glow Gradient (250px Glow)                  323.4 µs    955.9 µs    2.96x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.0 µs     77.8 µs    2.22x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.8 µs    204.5 µs    6.04x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               49.0 µs     35.8 µs      1.37x (Skia)
11. Compound Transforms (16 Elements)                  81.8 µs     85.7 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             14.2 µs     28.6 µs    2.01x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1112.6 µs   1418.8 µs    1.28x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             160.3 µs    289.7 µs    1.81x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               77.6 µs    115.4 µs    1.49x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  278.1 µs    860.7 µs    3.10x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            246.5 µs    999.6 µs    4.05x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            279.6 µs    107.1 µs      2.61x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             568.3 µs    669.0 µs    1.18x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         488.5 µs   1867.0 µs    3.82x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.3 µs    363.2 µs    1.91x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             608.6 µs   1029.1 µs    1.69x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            196.8 µs    183.6 µs      1.07x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        706.9 µs    717.0 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         363.5 µs   2840.7 µs    7.82x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       941.5 µs   1003.4 µs    1.07x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          612.2 µs    761.0 µs    1.24x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    161.2 µs     74.8 µs      2.16x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    142.0 µs     69.2 µs      2.05x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       56.9 µs    142.5 µs    2.50x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      398.3 µs   1064.3 µs    2.67x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    495.6 µs    731.7 µs    1.48x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     31.7 µs    225.6 µs    7.11x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.4 µs    240.2 µs    2.91x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1041.9 µs   1380.2 µs    1.32x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1039.5 µs    409.3 µs      2.54x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          210.2 µs    354.0 µs    1.68x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          210.2 µs    210.1 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          217.0 µs    239.6 µs    1.10x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           46.5 µs    127.5 µs    2.74x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.3 µs    131.5 µs    2.52x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.5 µs    178.7 µs    3.85x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.1 µs    137.6 µs    1.68x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        85.2 µs    125.8 µs    1.48x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      386.1 µs    335.4 µs      1.15x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       272.6 µs    241.0 µs      1.13x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    364.8 µs    340.0 µs      1.07x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      4.5 µs    6.47x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        510.3 µs    990.4 µs    1.94x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          309.4 µs    949.3 µs    3.07x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    346.2 µs    924.3 µs    2.67x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.9 µs     45.8 µs    1.64x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     80.6 µs    309.4 µs    3.84x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    641.0 µs   2306.6 µs    3.60x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            88.2 µs    128.6 µs    1.46x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           443.8 µs    642.1 µs    1.45x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          885.8 µs   1293.9 µs    1.46x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           282.7 µs   3155.1 µs   11.16x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    206.2 µs   1829.4 µs    8.87x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    193.1 µs   1467.0 µs    7.60x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      188.6 µs    251.6 µs    1.33x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     365.3 µs    785.5 µs    2.15x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      711.8 µs   1831.2 µs    2.57x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     703.4 µs    719.3 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     226.6 µs    149.8 µs      1.51x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    375.3 µs    311.3 µs      1.21x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1006.0 µs    736.9 µs      1.37x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.0 µs     18.7 µs    2.07x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    875.4 µs    644.0 µs      1.36x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     165.1 µs    885.9 µs    5.37x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     380.7 µs   3052.3 µs    8.02x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    411.2 µs   1406.6 µs    3.42x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      260.5 µs   1181.6 µs    4.54x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    145.3 µs    237.8 µs    1.64x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     24.9 µs    398.2 µs   15.99x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2678.0 µs   3522.1 µs    1.32x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    740.8 µs   4343.9 µs    5.86x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.7 µs     68.0 µs    1.22x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     18.6 µs    696.8 µs   37.48x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       43.1 µs     61.0 µs    1.42x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       345.9 µs    392.4 µs    1.13x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         276.3 µs    204.2 µs      1.35x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     553.4 µs   1653.7 µs    2.99x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         351.6 µs    899.2 µs    2.56x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    358.9 µs   1334.4 µs    3.72x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        250.9 µs    660.1 µs    2.63x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    287.9 µs    787.1 µs    2.73x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    287.0 µs    813.6 µs    2.84x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     499.9 µs   2761.9 µs    5.53x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   1941.9 µs   8036.9 µs    4.14x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                31870.5 µs  75038.4 µs    2.35x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 73 / 90 tests (81.1%)
    * Skia Won:   17 / 90 tests (18.9%)
=================================================================================================================================================================
```
</details>

### Run 10
<details>
<summary>Click to expand Run 10 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GOOGLE SKIA — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)   Skia (µs)           vs Skia
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    77.9 µs    106.4 µs    1.37x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   68.3 µs     94.6 µs    1.39x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                57.7 µs    142.7 µs    2.47x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                175.1 µs    160.2 µs      1.09x (Skia)
5. 4 Rounded Rect Cards (Fill + Stroke)                56.8 µs    143.0 µs    2.52x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  8.6 µs      7.4 µs      1.15x (Skia)
7. Radial Glow Gradient (250px Glow)                  272.1 µs    808.3 µs    2.97x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  29.6 µs     65.8 µs    2.22x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                28.6 µs    173.3 µs    6.05x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               39.6 µs     30.2 µs      1.31x (Skia)
11. Compound Transforms (16 Elements)                  69.3 µs     71.6 µs       Tie (~1.0x)
12. Rectangular Clipping (Nested Viewport)             11.9 µs     24.1 µs    2.02x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)                941.8 µs   1214.5 µs    1.29x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             139.0 µs    253.7 µs    1.83x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               68.2 µs     98.9 µs    1.45x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  235.9 µs    729.2 µs    3.09x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            207.9 µs    843.9 µs    4.06x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            236.0 µs     90.8 µs      2.60x (Skia)
19. 50 Rotated Quads (Affine Alpha Stack)             475.0 µs    563.5 µs    1.19x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         411.2 µs   1627.4 µs    3.96x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            165.0 µs    315.6 µs    1.91x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             510.3 µs    843.8 µs    1.65x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            165.8 µs    154.8 µs      1.07x (Skia)
24. 40-Point Starburst (Dense Winding Polygon)        592.9 µs    590.2 µs       Tie (~1.0x)
25. 2-Point Focal Radial Spotlight (Lighting)         307.4 µs   2409.7 µs    7.84x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       807.6 µs    859.7 µs    1.06x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          517.1 µs    643.3 µs    1.24x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    136.2 µs     63.2 µs      2.15x (Skia)
29. Non-Zero Winding Fill (Self-Intersecting Star)    120.5 µs     58.5 µs      2.06x (Skia)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       48.2 µs    120.3 µs    2.50x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      336.9 µs    906.3 µs    2.69x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    414.5 µs    634.2 µs    1.53x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     27.1 µs    190.4 µs    7.02x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     69.4 µs    204.2 µs    2.94x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)          901.4 µs   1197.0 µs    1.33x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)          884.4 µs    344.4 µs      2.57x (Skia)
37. Stroke Join: Miter (Zigzag Polygon 12px)          177.4 µs    298.8 µs    1.68x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          179.2 µs    181.0 µs       Tie (~1.0x)
39. Stroke Join: Round (Zigzag Polygon 12px)          182.4 µs    201.6 µs    1.11x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           39.6 µs    106.6 µs    2.69x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         43.9 µs    109.5 µs    2.49x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          39.0 µs    149.8 µs    3.84x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     69.0 µs    116.9 µs    1.70x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        72.3 µs    105.2 µs    1.45x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      319.2 µs    279.8 µs      1.14x (Skia)
46. Transform Isolation: Pure Shear (20 Shapes)       229.6 µs    205.6 µs      1.12x (Skia)
47. Transform Isolation: Combined Affine (20 Shapes)    308.3 µs    290.8 µs      1.06x (Skia)
48. State Stack: Save/Restore Overhead (50 Passes)      0.6 µs      3.7 µs    6.26x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        424.2 µs    829.3 µs    1.96x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          261.5 µs    811.8 µs    3.10x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    299.9 µs    786.4 µs    2.62x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        23.2 µs     37.3 µs    1.61x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     66.0 µs    253.0 µs    3.84x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    524.7 µs   1962.1 µs    3.74x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            76.5 µs    111.3 µs    1.46x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           378.3 µs    541.6 µs    1.43x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          741.8 µs   1084.2 µs    1.46x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           237.3 µs   2688.7 µs   11.33x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    176.7 µs   1619.0 µs    9.16x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    166.0 µs   1263.2 µs    7.61x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      158.5 µs    214.2 µs    1.35x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     310.4 µs    661.2 µs    2.13x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      594.9 µs   1571.3 µs    2.64x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     613.0 µs    609.5 µs       Tie (~1.0x)
65. Compound Path with Hole (Donut Even-Odd Fill)     190.6 µs    128.8 µs      1.48x (Skia)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    325.4 µs    267.8 µs      1.22x (Skia)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         863.4 µs    616.8 µs      1.40x (Skia)
68. Image Downscaling (4x Minification 0.25x Bilinear)      7.8 µs     15.7 µs    2.02x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    741.4 µs    554.8 µs      1.34x (Skia)
70. Repeated Linear Gradient (60px Periodic Tile)     139.3 µs    753.6 µs    5.41x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     323.6 µs   2638.9 µs    8.15x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    357.6 µs   1223.8 µs    3.42x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      223.5 µs   1022.6 µs    4.58x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    125.3 µs    210.2 µs    1.68x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     21.7 µs    347.3 µs   16.00x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2356.2 µs   3058.8 µs    1.30x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    641.7 µs   3750.2 µs    5.84x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         47.9 µs     59.1 µs    1.23x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     16.1 µs    610.5 µs   37.87x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       37.6 µs     53.2 µs    1.41x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       299.5 µs    345.2 µs    1.15x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         234.4 µs    175.8 µs      1.33x (Skia)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     508.9 µs   1606.8 µs    3.16x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         349.9 µs    938.6 µs    2.68x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    376.6 µs   1402.4 µs    3.72x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        262.6 µs    682.8 µs    2.60x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    296.6 µs    822.5 µs    2.77x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    302.0 µs    866.4 µs    2.87x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     527.8 µs   2999.1 µs    5.68x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2090.6 µs   8579.2 µs    4.10x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                27996.9 µs  67609.4 µs    2.41x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs Google Skia (CPU Rasterizer)]
    * Nisaba Won: 72 / 90 tests (80.0%)
    * Skia Won:   18 / 90 tests (20.0%)
=================================================================================================================================================================
```
</details>

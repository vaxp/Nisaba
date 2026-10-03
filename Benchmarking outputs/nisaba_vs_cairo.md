# Benchmark Report: Nisaba vs Cairo (90 Rigorous Geometric Suites)

**Execution Date:** 2026-10-03 14:55:39  
**Runs Executed:** 10 consecutive runs  
**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  

---

## Executive Summary & Aggregate Averages

Across **10 consecutive executions** testing 90 vector geometry, shader, blending, and compositing benchmarks:
- **Nisaba Average Total Frame Time:** `30977.9 µs` (30.98 ms)
- **Cairo Average Total Frame Time:** `234563.3 µs` (234.56 ms)
- **Overall Speedup:** **7.57x FASTER (Nisaba)**
- **Win / Loss Scorecard (10-Run Mean):**
  * **Nisaba Won:** **89 / 90 suites (98.9%)**
  * **Cairo Won:** 0 / 90 suites (0.0%)
  * **Ties:** 1 suites

---

## 10-Run Mean Results by Benchmark Suite

| # | Benchmark Suite Description | Nisaba Mean (µs) | Cairo Mean (µs) | Speedup / Winner |
| :---: | :--- | :---: | :---: | :---: |
| 1 | Solid Background Fill (1080x740) | **90.8 µs** | 94.8 µs | Tie (~1.0x) |
| 2 | 50 Grid Lines (1px Stroked Paths) | **79.0 µs** | 246.2 µs | **3.12x (Nisaba)** |
| 3 | 25 Alpha Circles (Porter-Duff Blend) | **67.2 µs** | 757.5 µs | **11.28x (Nisaba)** |
| 4 | Cubic Bezier Curves (Fill + Stroke) | **200.9 µs** | 260.6 µs | **1.30x (Nisaba)** |
| 5 | 4 Rounded Rect Cards (Fill + Stroke) | **62.8 µs** | 231.4 µs | **3.68x (Nisaba)** |
| 6 | Multi-Stop Linear Gradient (480x36) | **9.8 µs** | 22.1 µs | **2.27x (Nisaba)** |
| 7 | Radial Glow Gradient (250px Glow) | **314.2 µs** | 4527.3 µs | **14.41x (Nisaba)** |
| 8 | 12 Radial Rotated Spokes (Strokes) | **31.9 µs** | 172.9 µs | **5.42x (Nisaba)** |
| 9 | Dashed Orbit Ring (Stroke with Dash) | **31.1 µs** | 393.4 µs | **12.64x (Nisaba)** |
| 10 | Concave 10-Point Star (Winding Fill) | **43.9 µs** | 113.7 µs | **2.59x (Nisaba)** |
| 11 | Compound Transforms (16 Elements) | **72.3 µs** | 335.2 µs | **4.64x (Nisaba)** |
| 12 | Rectangular Clipping (Nested Viewport) | **13.2 µs** | 97.8 µs | **7.42x (Nisaba)** |
| 13 | 100 Anti-Aliased Diagonals (Lines) | **1052.8 µs** | 9452.3 µs | **8.98x (Nisaba)** |
| 14 | 30 Overlapping UI Chips (Alpha Stack) | **154.4 µs** | 889.7 µs | **5.76x (Nisaba)** |
| 15 | Closed Cubic Loop (Trefoil Figure-8) | **76.5 µs** | 178.9 µs | **2.34x (Nisaba)** |
| 16 | 500 Alpha Disks (Particle Cloud) | **272.2 µs** | 3346.6 µs | **12.30x (Nisaba)** |
| 17 | 1000-Pt Waveform (High-Density Stroke) | **231.8 µs** | 2277.0 µs | **9.82x (Nisaba)** |
| 18 | Thick 16px Stroke (Miter & Round Caps) | **268.4 µs** | 590.7 µs | **2.20x (Nisaba)** |
| 19 | 50 Rotated Quads (Affine Alpha Stack) | **528.3 µs** | 2917.8 µs | **5.52x (Nisaba)** |
| 20 | 25 Concentric Rings (Alternating Strokes) | **489.9 µs** | 6971.3 µs | **14.23x (Nisaba)** |
| 21 | Diagonal Full-HD Gradient (45° Angle) | **185.4 µs** | 7710.5 µs | **41.58x (Nisaba)** |
| 22 | 3-Stage Nested Clipping (Criss-Cross) | **555.0 µs** | 1936.3 µs | **3.49x (Nisaba)** |
| 23 | Dashed Cubic Spline (Curved Intervals) | **182.0 µs** | 593.5 µs | **3.26x (Nisaba)** |
| 24 | 40-Point Starburst (Dense Winding Polygon) | **683.4 µs** | 1294.6 µs | **1.89x (Nisaba)** |
| 25 | 2-Point Focal Radial Spotlight (Lighting) | **361.8 µs** | 5992.1 µs | **16.56x (Nisaba)** |
| 26 | Ellipse Rendering (25 Ellipses Fill+Stroke) | **932.9 µs** | 3780.9 µs | **4.05x (Nisaba)** |
| 27 | Circular Arc Rendering (25 Stroked Arcs) | **609.1 µs** | 2466.5 µs | **4.05x (Nisaba)** |
| 28 | Even-Odd Polygon Fill (Self-Intersecting Star) | **152.1 µs** | 281.8 µs | **1.85x (Nisaba)** |
| 29 | Non-Zero Winding Fill (Self-Intersecting Star) | **140.4 µs** | 230.3 µs | **1.64x (Nisaba)** |
| 30 | Direct Pixmap Blit (4x 256x256 1:1 Surfaces) | **58.8 µs** | 110.1 µs | **1.87x (Nisaba)** |
| 31 | Image Bilinear Scaling (2x Scale Up 512x512) | **398.5 µs** | 506.4 µs | **1.27x (Nisaba)** |
| 32 | Image Rotation & Affine Transform (35° Filtered) | **497.7 µs** | 1017.3 µs | **2.04x (Nisaba)** |
| 33 | Image Pattern Fill (Circle Filled with Texture) | **32.9 µs** | 45.3 µs | **1.38x (Nisaba)** |
| 34 | Pattern Repeat & Tiling (64x64 Texture Repeat) | **81.3 µs** | 206.4 µs | **2.54x (Nisaba)** |
| 35 | Anti-Aliasing ON (50 Diagonals Coverage) | **1027.1 µs** | 3675.1 µs | **3.58x (Nisaba)** |
| 36 | Anti-Aliasing OFF (50 Diagonals Aliased) | **1029.0 µs** | 2953.2 µs | **2.87x (Nisaba)** |
| 37 | Stroke Join: Miter (Zigzag Polygon 12px) | **197.6 µs** | 1062.0 µs | **5.37x (Nisaba)** |
| 38 | Stroke Join: Bevel (Zigzag Polygon 12px) | **198.8 µs** | 989.3 µs | **4.98x (Nisaba)** |
| 39 | Stroke Join: Round (Zigzag Polygon 12px) | **197.9 µs** | 1037.1 µs | **5.24x (Nisaba)** |
| 40 | Stroke Cap: Butt (20 Line Segments 14px) | **45.5 µs** | 102.4 µs | **2.25x (Nisaba)** |
| 41 | Stroke Cap: Square (20 Line Segments 14px) | **51.2 µs** | 105.0 µs | **2.05x (Nisaba)** |
| 42 | Stroke Cap: Round (20 Line Segments 14px) | **45.6 µs** | 292.2 µs | **6.41x (Nisaba)** |
| 43 | Transform Isolation: Pure Translate (20 Shapes) | **79.3 µs** | 344.7 µs | **4.35x (Nisaba)** |
| 44 | Transform Isolation: Pure Scale (20 Shapes) | **81.9 µs** | 315.8 µs | **3.86x (Nisaba)** |
| 45 | Transform Isolation: Pure Rotate (20 Shapes) | **374.4 µs** | 1147.0 µs | **3.06x (Nisaba)** |
| 46 | Transform Isolation: Pure Shear (20 Shapes) | **267.9 µs** | 863.6 µs | **3.22x (Nisaba)** |
| 47 | Transform Isolation: Combined Affine (20 Shapes) | **360.3 µs** | 1115.9 µs | **3.10x (Nisaba)** |
| 48 | State Stack: Save/Restore Overhead (50 Passes) | **0.7 µs** | 6.4 µs | **8.83x (Nisaba)** |
| 49 | Compositing Operator: Multiply (25 Shapes) | **490.0 µs** | 5130.6 µs | **10.47x (Nisaba)** |
| 50 | Compositing Operator: Screen (25 Shapes) | **301.6 µs** | 5730.2 µs | **19.00x (Nisaba)** |
| 51 | Compositing Operator: Source-In & Xor (25 Shapes) | **334.2 µs** | 11119.4 µs | **33.27x (Nisaba)** |
| 52 | Path Complexity: Low (10 Vertices Waveform) | **26.0 µs** | 94.3 µs | **3.63x (Nisaba)** |
| 53 | Path Complexity: Medium (100 Vertices Waveform) | **75.2 µs** | 679.8 µs | **9.04x (Nisaba)** |
| 54 | Path Complexity: High (1000 Vertices Waveform) | **586.2 µs** | 6293.1 µs | **10.73x (Nisaba)** |
| 55 | Primitive Scaling: 100 Rectangles Batch | **85.8 µs** | 144.7 µs | **1.69x (Nisaba)** |
| 56 | Primitive Scaling: 500 Rectangles Batch | **428.1 µs** | 722.5 µs | **1.69x (Nisaba)** |
| 57 | Primitive Scaling: 1000 Rectangles Batch | **864.0 µs** | 1451.6 µs | **1.68x (Nisaba)** |
| 58 | Clip-Depth Scaling: 1-Level Vector Clip | **269.5 µs** | 846.0 µs | **3.14x (Nisaba)** |
| 59 | Clip-Depth Scaling: 2-Level Nested Vector Clip | **209.0 µs** | 566.2 µs | **2.71x (Nisaba)** |
| 60 | Clip-Depth Scaling: 4-Level Nested Vector Clip | **193.2 µs** | 527.8 µs | **2.73x (Nisaba)** |
| 61 | Resolution Scaling: Small (256x256 Viewport) | **102.6 µs** | 1121.2 µs | **10.93x (Nisaba)** |
| 62 | Resolution Scaling: Medium (640x480 Viewport) | **297.2 µs** | 3607.4 µs | **12.14x (Nisaba)** |
| 63 | Resolution Scaling: Full Viewport (1080x740) | **684.9 µs** | 8741.9 µs | **12.76x (Nisaba)** |
| 64 | Quadratic Bezier Splines (20 Connected Quads) | **702.8 µs** | 1092.2 µs | **1.55x (Nisaba)** |
| 65 | Compound Path with Hole (Donut Even-Odd Fill) | **203.8 µs** | 515.9 µs | **2.53x (Nisaba)** |
| 66 | Miter Limit Clamping (Sharp 10° Acute Angles) | **334.8 µs** | 1531.1 µs | **4.57x (Nisaba)** |
| 67 | Sub-Pixel Hairline Strokes (0.25px Lines) | **969.9 µs** | 4533.1 µs | **4.67x (Nisaba)** |
| 68 | Image Downscaling (4x Minification 0.25x Bilinear) | **8.6 µs** | 10.2 µs | **1.17x (Nisaba)** |
| 69 | Pattern Reflect Tiling (64x64 Texture Reflect) | **492.1 µs** | 10419.2 µs | **21.17x (Nisaba)** |
| 70 | Repeated Linear Gradient (60px Periodic Tile) | **152.2 µs** | 346.9 µs | **2.28x (Nisaba)** |
| 71 | Alpha Mask Surface Blit (8-bit Alpha Masking) | **357.5 µs** | 412.9 µs | **1.15x (Nisaba)** |
| 72 | Blend Modes: Color Dodge & Difference (25 Shapes) | **387.1 µs** | 7844.2 µs | **20.26x (Nisaba)** |
| 73 | Additive Compositing (Plus / Add Blend Mode) | **255.1 µs** | 1788.6 µs | **7.01x (Nisaba)** |
| 74 | Vector Clip on Heavy Curved Stroke (24px Ribbon) | **139.1 µs** | 161.0 µs | **1.16x (Nisaba)** |
| 75 | Animated Dash Offset Phase (Moving Dash Stroke) | **23.9 µs** | 2485.3 µs | **104.21x (Nisaba)** |
| 76 | Freeform Gradient Mesh (Bicubic Coons Patch) | **2498.8 µs** | 9389.2 µs | **3.76x (Nisaba)** |
| 77 | True 2-Point Conical Gradient (r0 > 0, r1 > 0) | **724.3 µs** | 5740.6 µs | **7.93x (Nisaba)** |
| 78 | Axis-Aligned Stroked Rectangles (25 Rects) | **52.3 µs** | 273.2 µs | **5.23x (Nisaba)** |
| 79 | Dashed Stroke with Round Caps (Round Intervals) | **18.4 µs** | 1891.9 µs | **102.93x (Nisaba)** |
| 80 | Anisotropic Scaled Stroke (scale 2.5x, 0.4y) | **45.3 µs** | 150.2 µs | **3.31x (Nisaba)** |
| 81 | Non-Convex Star Vector Clip (Even-Odd Rule) | **376.1 µs** | 1641.4 µs | **4.36x (Nisaba)** |
| 82 | Multi-Contour Complex Path (20 Sub-Paths) | **271.1 µs** | 778.9 µs | **2.87x (Nisaba)** |
| 83 | Blend Modes: Overlay & Soft-Light (25 Shapes) | **518.4 µs** | 8550.4 µs | **16.49x (Nisaba)** |
| 84 | Blend Modes: Darken & Lighten (25 Shapes) | **392.5 µs** | 5573.0 µs | **14.20x (Nisaba)** |
| 85 | Blend Modes: Color-Burn & Hard-Light (25 Shapes) | **466.4 µs** | 7852.8 µs | **16.84x (Nisaba)** |
| 86 | Blend Modes: Exclusion & Clear (25 Shapes) | **281.0 µs** | 3280.5 µs | **11.68x (Nisaba)** |
| 87 | Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes) | **280.5 µs** | 3664.8 µs | **13.07x (Nisaba)** |
| 88 | Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes) | **318.3 µs** | 13421.2 µs | **42.17x (Nisaba)** |
| 89 | HSL Blend Modes: Hue & Luminosity (25 Shapes) | **604.1 µs** | 11936.0 µs | **19.76x (Nisaba)** |
| 90 | Bilinear Filtered Pattern under Rotation (30° Tiling) | **2236.6 µs** | 8443.0 µs | **3.77x (Nisaba)** |
| **--** | **TOTAL FRAME OVERHEAD** | **30977.9 µs** | **234563.3 µs** | **7.57x (Nisaba)** |

---

## Individual Run Logs (Runs 1 to 10)

### Run 1
<details>
<summary>Click to expand Run 1 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    90.1 µs     94.2 µs       Tie (~1.0x)
2. 50 Grid Lines (1px Stroked Paths)                   79.2 µs    238.5 µs    3.01x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                64.2 µs    712.4 µs   11.10x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                195.5 µs    249.2 µs    1.27x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                58.7 µs    217.7 µs    3.71x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  9.3 µs     21.0 µs    2.27x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  303.9 µs   4278.3 µs   14.08x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  30.4 µs    163.4 µs    5.38x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                29.4 µs    372.3 µs   12.66x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               41.8 µs    107.2 µs    2.56x (Nisaba)
11. Compound Transforms (16 Elements)                  68.6 µs    316.9 µs    4.62x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             12.5 µs     92.1 µs    7.38x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1003.2 µs   9093.0 µs    9.06x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             148.3 µs    849.5 µs    5.73x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               72.3 µs    167.7 µs    2.32x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  255.5 µs   3188.7 µs   12.48x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            220.5 µs   2183.3 µs    9.90x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            264.5 µs    582.4 µs    2.20x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             512.0 µs   2944.5 µs    5.75x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         497.0 µs   6999.0 µs   14.08x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            184.6 µs   7665.3 µs   41.53x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             565.2 µs   1970.2 µs    3.49x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            187.3 µs    609.2 µs    3.25x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        698.5 µs   1318.7 µs    1.89x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         366.2 µs   6176.5 µs   16.87x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       973.2 µs   3876.7 µs    3.98x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          617.5 µs   2512.9 µs    4.07x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    155.9 µs    286.1 µs    1.84x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    143.8 µs    234.2 µs    1.63x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       60.5 µs    113.4 µs    1.87x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      412.1 µs    520.8 µs    1.26x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    506.8 µs   1052.5 µs    2.08x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.6 µs     45.4 µs    1.39x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.4 µs    208.6 µs    2.53x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1032.0 µs   3740.3 µs    3.62x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1043.6 µs   3050.7 µs    2.92x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          203.1 µs   1082.2 µs    5.33x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          203.1 µs   1011.5 µs    4.98x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          203.4 µs   1056.9 µs    5.20x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           47.4 µs    105.1 µs    2.22x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.7 µs    107.9 µs    2.05x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          47.3 µs    296.4 µs    6.26x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     81.8 µs    354.7 µs    4.34x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        84.4 µs    324.7 µs    3.85x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      388.4 µs   1198.7 µs    3.09x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       275.2 µs    895.8 µs    3.25x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    372.1 µs   1157.8 µs    3.11x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.5 µs    9.56x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        497.2 µs   5232.1 µs   10.52x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          310.0 µs   5917.3 µs   19.09x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    347.7 µs  11698.5 µs   33.64x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        26.9 µs     95.1 µs    3.54x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     77.4 µs    701.6 µs    9.06x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    611.6 µs   6666.7 µs   10.90x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            90.0 µs    152.3 µs    1.69x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           450.8 µs    756.7 µs    1.68x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          919.2 µs   1565.1 µs    1.70x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           284.6 µs    873.0 µs    3.07x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    219.8 µs    602.7 µs    2.74x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    207.8 µs    562.9 µs    2.71x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      110.5 µs   1178.3 µs   10.66x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     315.6 µs   4123.0 µs   13.06x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      930.5 µs  12209.3 µs   13.12x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     918.8 µs   1346.9 µs    1.47x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     248.1 µs    628.4 µs    2.53x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    401.5 µs   1800.5 µs    4.48x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1087.8 µs   5021.8 µs    4.62x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.3 µs     11.0 µs    1.18x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    529.7 µs  11079.0 µs   20.92x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     175.1 µs    384.1 µs    2.19x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     393.8 µs    450.7 µs    1.14x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    422.0 µs   8418.1 µs   19.95x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      291.3 µs   1947.3 µs    6.69x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    146.8 µs    173.5 µs    1.18x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.4 µs   2764.9 µs  108.92x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2848.3 µs  11267.1 µs    3.96x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    943.6 µs   6894.1 µs    7.31x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         63.3 µs    331.7 µs    5.24x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     23.7 µs   2530.8 µs  106.75x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       62.1 µs    210.9 µs    3.40x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       571.3 µs   2123.5 µs    3.72x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         326.1 µs    962.6 µs    2.95x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     690.5 µs  11063.7 µs   16.02x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         488.1 µs   6099.2 µs   12.50x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    497.1 µs   8370.1 µs   16.84x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        294.6 µs   3452.9 µs   11.72x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    299.2 µs   4264.3 µs   14.25x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    382.5 µs  15542.5 µs   40.63x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     668.1 µs  12682.4 µs   18.98x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2367.6 µs  11551.7 µs    4.88x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                33554.0 µs 257558.9 µs    7.68x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 2
<details>
<summary>Click to expand Run 2 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                   131.5 µs    141.8 µs    1.08x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                  112.5 µs    384.4 µs    3.42x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)               100.5 µs   1141.6 µs   11.36x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                273.7 µs    346.7 µs    1.27x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                86.3 µs    295.8 µs    3.43x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 12.4 µs     28.1 µs    2.27x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  382.2 µs   5186.8 µs   13.57x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  35.1 µs    189.1 µs    5.38x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                34.0 µs    417.7 µs   12.30x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.3 µs    121.0 µs    2.61x (Nisaba)
11. Compound Transforms (16 Elements)                  77.0 µs    359.7 µs    4.67x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             14.4 µs    105.5 µs    7.34x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1134.5 µs   9914.1 µs    8.74x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             162.0 µs    928.8 µs    5.73x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               78.9 µs    183.2 µs    2.32x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  279.1 µs   3488.3 µs   12.50x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            237.6 µs   2363.6 µs    9.95x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            281.7 µs    611.5 µs    2.17x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             550.7 µs   3041.3 µs    5.52x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         510.2 µs   7168.4 µs   14.05x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            191.0 µs   7934.7 µs   41.54x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             569.3 µs   1980.2 µs    3.48x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            188.0 µs    610.0 µs    3.25x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        697.6 µs   1323.6 µs    1.90x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         363.1 µs   6139.4 µs   16.91x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       939.0 µs   3841.7 µs    4.09x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          617.8 µs   2506.8 µs    4.06x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    155.5 µs    286.5 µs    1.84x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    143.1 µs    234.2 µs    1.64x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       68.2 µs    110.3 µs    1.62x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      398.4 µs    503.4 µs    1.26x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    507.8 µs   1020.4 µs    2.01x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.5 µs     45.4 µs    1.40x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.3 µs    208.4 µs    2.53x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1050.5 µs   3711.3 µs    3.53x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1044.4 µs   2961.7 µs    2.84x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          196.7 µs   1067.7 µs    5.43x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          199.0 µs    947.4 µs    4.76x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          186.5 µs    955.2 µs    5.12x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           41.3 µs     93.6 µs    2.26x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         47.1 µs     96.2 µs    2.04x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          42.1 µs    290.8 µs    6.90x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     69.8 µs    304.2 µs    4.36x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        72.1 µs    278.7 µs    3.86x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      333.1 µs    998.1 µs    3.00x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       231.2 µs    751.3 µs    3.25x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    311.4 µs    973.0 µs    3.12x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.6 µs      5.6 µs    9.51x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        420.4 µs   4453.9 µs   10.60x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          261.8 µs   5011.2 µs   19.14x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    294.3 µs   9741.2 µs   33.10x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        22.9 µs     91.7 µs    4.01x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     66.7 µs    608.5 µs    9.12x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    516.1 µs   5563.3 µs   10.78x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            74.3 µs    126.5 µs    1.70x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           371.7 µs    640.5 µs    1.72x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          756.8 µs   1258.2 µs    1.66x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           232.4 µs    756.7 µs    3.26x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    187.5 µs    507.2 µs    2.70x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    171.4 µs    471.0 µs    2.75x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)       92.3 µs   1015.5 µs   11.00x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     271.8 µs   3244.8 µs   11.94x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      601.0 µs   7608.6 µs   12.66x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     641.4 µs    989.8 µs    1.54x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     182.8 µs    463.7 µs    2.54x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    301.3 µs   1374.4 µs    4.56x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         882.9 µs   4148.8 µs    4.70x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      7.9 µs      9.4 µs    1.18x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    457.8 µs   9914.5 µs   21.66x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     143.4 µs    331.9 µs    2.31x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     345.7 µs    393.8 µs    1.14x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    376.1 µs   7433.7 µs   19.77x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      233.4 µs   1606.3 µs    6.88x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    123.8 µs    145.5 µs    1.18x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     21.5 µs   2206.1 µs  102.38x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2257.4 µs   8357.9 µs    3.70x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    635.0 µs   5052.4 µs    7.96x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         44.8 µs    242.6 µs    5.41x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     16.1 µs   1653.3 µs  102.48x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       39.2 µs    129.5 µs    3.31x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       317.0 µs   1450.1 µs    4.57x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         237.2 µs    680.8 µs    2.87x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     467.5 µs   7487.3 µs   16.01x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         338.5 µs   5023.6 µs   14.84x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    428.5 µs   7155.8 µs   16.70x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        254.9 µs   3013.2 µs   11.82x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    263.8 µs   3317.5 µs   12.57x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    285.6 µs  12476.2 µs   43.68x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     574.6 µs  11351.3 µs   19.75x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2233.4 µs   8167.3 µs    3.66x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                29772.7 µs 222273.0 µs    7.47x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 3
<details>
<summary>Click to expand Run 3 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    93.6 µs     90.2 µs       Tie (~1.0x)
2. 50 Grid Lines (1px Stroked Paths)                   75.5 µs    233.4 µs    3.09x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                63.8 µs    730.8 µs   11.45x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                192.1 µs    252.0 µs    1.31x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                60.8 µs    225.3 µs    3.70x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  9.5 µs     21.5 µs    2.26x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  304.4 µs   4544.0 µs   14.93x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  32.0 µs    175.8 µs    5.49x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                32.0 µs    404.2 µs   12.64x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               45.0 µs    116.9 µs    2.60x (Nisaba)
11. Compound Transforms (16 Elements)                  74.2 µs    344.7 µs    4.64x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             13.7 µs    102.2 µs    7.45x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1083.9 µs  10046.6 µs    9.27x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             161.2 µs    953.6 µs    5.92x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               81.6 µs    188.7 µs    2.31x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  288.6 µs   3539.3 µs   12.26x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            247.7 µs   2421.7 µs    9.78x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            280.3 µs    610.5 µs    2.18x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             552.1 µs   3035.3 µs    5.50x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         497.3 µs   7267.9 µs   14.61x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.9 µs   7949.9 µs   41.64x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             565.5 µs   1996.0 µs    3.53x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            186.2 µs    609.8 µs    3.27x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        701.9 µs   1316.5 µs    1.88x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         381.7 µs   6124.2 µs   16.04x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       944.1 µs   3834.8 µs    4.06x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          620.7 µs   2500.9 µs    4.03x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    154.9 µs    286.9 µs    1.85x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    142.9 µs    234.3 µs    1.64x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       58.0 µs    113.4 µs    1.96x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      410.8 µs    519.8 µs    1.27x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    506.1 µs   1019.1 µs    2.01x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     34.5 µs     45.8 µs    1.33x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     81.5 µs    208.4 µs    2.56x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1039.3 µs   3777.6 µs    3.63x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1065.5 µs   3062.4 µs    2.87x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          196.4 µs   1074.3 µs    5.47x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          202.0 µs   1008.0 µs    4.99x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          202.7 µs   1060.8 µs    5.23x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           46.5 µs    105.3 µs    2.26x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.3 µs    107.8 µs    2.06x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.5 µs    294.8 µs    6.35x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     80.8 µs    352.4 µs    4.36x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        83.6 µs    322.4 µs    3.86x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      386.7 µs   1154.5 µs    2.99x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       265.3 µs    892.9 µs    3.37x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    369.5 µs   1146.1 µs    3.10x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.5 µs    9.53x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        567.1 µs   5299.7 µs    9.35x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          309.5 µs   5893.3 µs   19.04x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    347.1 µs  11731.0 µs   33.80x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.6 µs     98.1 µs    3.56x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     79.8 µs    704.1 µs    8.83x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    621.4 µs   6618.8 µs   10.65x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            88.1 µs    149.5 µs    1.70x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           446.2 µs    754.8 µs    1.69x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          881.8 µs   1498.1 µs    1.70x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           280.6 µs    914.8 µs    3.26x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    224.4 µs    602.8 µs    2.69x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    204.4 µs    580.2 µs    2.84x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      110.1 µs   1197.5 µs   10.87x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     316.7 µs   3811.2 µs   12.03x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      698.2 µs   8920.4 µs   12.78x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     723.5 µs   1136.9 µs    1.57x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     214.9 µs    546.0 µs    2.54x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    346.3 µs   1578.9 µs    4.56x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1021.2 µs   4765.5 µs    4.67x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.3 µs     10.9 µs    1.17x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    532.4 µs  10958.2 µs   20.58x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     159.9 µs    363.6 µs    2.27x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     371.6 µs    436.5 µs    1.17x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    410.9 µs   8378.6 µs   20.39x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      267.2 µs   1906.4 µs    7.14x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    161.3 µs    173.0 µs    1.07x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.8 µs   2657.4 µs  103.17x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2656.9 µs   9882.0 µs    3.72x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    755.8 µs   6023.1 µs    7.97x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         53.9 µs    280.6 µs    5.20x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     19.0 µs   1938.5 µs  102.08x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       46.7 µs    153.2 µs    3.28x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       374.9 µs   1701.4 µs    4.54x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         281.3 µs    825.2 µs    2.93x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     531.1 µs   8882.1 µs   16.72x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         405.1 µs   5917.0 µs   14.61x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    493.1 µs   8381.1 µs   17.00x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        303.6 µs   3513.1 µs   11.57x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    293.4 µs   3805.6 µs   12.97x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    331.8 µs  14033.6 µs   42.29x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     643.8 µs  12532.9 µs   19.47x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2367.6 µs   8600.8 µs    3.63x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                32212.5 µs 244586.6 µs    7.59x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 89 / 90 tests (98.9%)
    * Cairo Won:  1 / 90 tests (1.1%)
=================================================================================================================================================================
```
</details>

### Run 4
<details>
<summary>Click to expand Run 4 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    88.7 µs     93.7 µs    1.06x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   79.4 µs    246.1 µs    3.10x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.5 µs    750.7 µs   11.12x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                209.5 µs    265.9 µs    1.27x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                64.5 µs    240.7 µs    3.73x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs     23.0 µs    2.26x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  324.0 µs   4779.6 µs   14.75x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  33.9 µs    183.0 µs    5.39x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                33.0 µs    418.8 µs   12.71x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.8 µs    120.2 µs    2.57x (Nisaba)
11. Compound Transforms (16 Elements)                  76.9 µs    363.8 µs    4.73x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             13.9 µs    104.3 µs    7.49x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1115.8 µs   9904.2 µs    8.88x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             162.6 µs    918.2 µs    5.65x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               78.7 µs    192.8 µs    2.45x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  280.3 µs   3490.6 µs   12.46x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            240.6 µs   2379.1 µs    9.89x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            280.9 µs    632.4 µs    2.25x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             559.4 µs   3033.9 µs    5.42x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         508.2 µs   7142.5 µs   14.05x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.2 µs   8007.1 µs   42.09x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             582.0 µs   1988.7 µs    3.42x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            187.0 µs    608.6 µs    3.25x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        697.6 µs   1315.8 µs    1.89x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         366.4 µs   6049.2 µs   16.51x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       946.0 µs   3832.6 µs    4.05x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          617.9 µs   2518.2 µs    4.08x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    155.4 µs    286.9 µs    1.85x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    143.2 µs    233.7 µs    1.63x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       58.9 µs    109.9 µs    1.87x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      399.3 µs    518.8 µs    1.30x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    500.0 µs   1044.2 µs    2.09x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.6 µs     45.8 µs    1.40x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.4 µs    208.5 µs    2.53x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1043.3 µs   3711.2 µs    3.56x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1042.9 µs   2967.4 µs    2.85x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          197.0 µs   1075.0 µs    5.46x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          203.1 µs   1018.1 µs    5.01x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          203.1 µs   1101.2 µs    5.42x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           47.0 µs    105.0 µs    2.24x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.8 µs    108.1 µs    2.05x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          47.4 µs    297.9 µs    6.28x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.2 µs    353.7 µs    4.30x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        85.1 µs    323.1 µs    3.80x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      399.6 µs   1190.5 µs    2.98x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       274.5 µs    890.1 µs    3.24x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    372.5 µs   1130.3 µs    3.03x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.5 µs    9.39x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        490.4 µs   5224.4 µs   10.65x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          305.6 µs   5769.2 µs   18.88x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    302.0 µs   9887.5 µs   32.74x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        22.8 µs     91.5 µs    4.01x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     66.7 µs    593.4 µs    8.89x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    517.3 µs   5555.4 µs   10.74x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            75.6 µs    127.7 µs    1.69x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           385.7 µs    639.8 µs    1.66x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          763.2 µs   1275.3 µs    1.67x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           240.1 µs    799.2 µs    3.33x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    190.9 µs    507.8 µs    2.66x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    174.7 µs    471.8 µs    2.70x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)       92.9 µs   1029.4 µs   11.08x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     266.2 µs   3188.2 µs   11.98x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      590.8 µs   7587.5 µs   12.84x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     613.5 µs    963.9 µs    1.57x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     179.0 µs    450.9 µs    2.52x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    293.6 µs   1340.5 µs    4.57x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         857.4 µs   4011.2 µs    4.68x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      7.7 µs      9.0 µs    1.17x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    436.6 µs   9333.1 µs   21.38x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     136.7 µs    310.3 µs    2.27x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     323.7 µs    372.1 µs    1.15x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    348.1 µs   7104.1 µs   20.41x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      233.6 µs   1621.1 µs    6.94x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    125.3 µs    145.9 µs    1.16x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     21.6 µs   2214.7 µs  102.32x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2234.8 µs   8345.6 µs    3.73x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    635.3 µs   5145.7 µs    8.10x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         46.4 µs    242.2 µs    5.22x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     16.2 µs   1651.9 µs  102.00x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       39.4 µs    130.2 µs    3.30x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       320.2 µs   1451.6 µs    4.53x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         240.4 µs    680.4 µs    2.83x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     445.6 µs   7451.0 µs   16.72x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         349.3 µs   5026.4 µs   14.39x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    415.5 µs   7029.3 µs   16.92x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        247.6 µs   2978.8 µs   12.03x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    254.2 µs   3253.6 µs   12.80x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    281.2 µs  11835.4 µs   42.08x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     527.2 µs  10586.5 µs   20.08x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2002.9 µs   7315.5 µs    3.65x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                29402.8 µs 220078.5 µs    7.48x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 5
<details>
<summary>Click to expand Run 5 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    78.6 µs     82.2 µs       Tie (~1.0x)
2. 50 Grid Lines (1px Stroked Paths)                   69.0 µs    209.6 µs    3.04x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                57.6 µs    647.3 µs   11.23x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                176.3 µs    229.2 µs    1.30x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                54.6 µs    206.7 µs    3.79x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  8.8 µs     19.9 µs    2.27x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  280.6 µs   3997.8 µs   14.25x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  28.9 µs    156.6 µs    5.42x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                28.9 µs    354.7 µs   12.29x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               39.3 µs    102.0 µs    2.59x (Nisaba)
11. Compound Transforms (16 Elements)                  65.1 µs    300.7 µs    4.62x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             11.9 µs     88.5 µs    7.40x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)                946.2 µs   8596.3 µs    9.09x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             140.3 µs    802.3 µs    5.72x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               68.9 µs    163.5 µs    2.37x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  251.8 µs   3033.0 µs   12.05x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            208.3 µs   2059.2 µs    9.89x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            243.7 µs    530.9 µs    2.18x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             478.4 µs   2683.0 µs    5.61x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         450.7 µs   6548.1 µs   14.53x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            174.1 µs   7154.3 µs   41.10x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             521.1 µs   1797.2 µs    3.45x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            171.5 µs    557.9 µs    3.25x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        654.3 µs   1243.8 µs    1.90x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         342.2 µs   5773.3 µs   16.87x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       917.2 µs   3644.0 µs    3.97x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          600.9 µs   2407.0 µs    4.01x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    145.8 µs    269.7 µs    1.85x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    134.7 µs    222.9 µs    1.65x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       57.3 µs    106.3 µs    1.86x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      387.0 µs    490.2 µs    1.27x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    477.3 µs    989.4 µs    2.07x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     31.7 µs     44.2 µs    1.39x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     80.5 µs    202.3 µs    2.51x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1015.3 µs   3730.9 µs    3.67x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1043.7 µs   2985.8 µs    2.86x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          203.0 µs   1077.5 µs    5.31x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          202.9 µs   1021.4 µs    5.03x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          202.9 µs   1068.5 µs    5.27x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           47.1 µs    105.2 µs    2.23x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         53.3 µs    108.6 µs    2.04x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          47.0 µs    298.2 µs    6.35x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.2 µs    351.8 µs    4.28x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        83.8 µs    322.4 µs    3.85x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      384.9 µs   1189.2 µs    3.09x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       275.2 µs    889.0 µs    3.23x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    371.3 µs   1156.8 µs    3.12x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.5 µs    9.50x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        496.0 µs   5233.5 µs   10.55x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          322.0 µs   5993.5 µs   18.61x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    348.3 µs  11666.3 µs   33.49x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.8 µs     98.4 µs    3.55x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     79.8 µs    724.8 µs    9.09x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    613.8 µs   6585.4 µs   10.73x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            90.0 µs    151.8 µs    1.69x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           450.4 µs    764.4 µs    1.70x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          901.3 µs   1569.8 µs    1.74x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           292.4 µs    898.7 µs    3.07x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    222.1 µs    602.3 µs    2.71x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    208.5 µs    555.9 µs    2.67x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      106.9 µs   1171.6 µs   10.96x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     314.9 µs   3796.8 µs   12.06x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      713.7 µs   8940.9 µs   12.53x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     716.9 µs   1138.9 µs    1.59x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     212.4 µs    534.5 µs    2.52x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    348.3 µs   1585.5 µs    4.55x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1011.5 µs   4811.4 µs    4.76x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.1 µs     10.5 µs    1.16x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    513.5 µs  11191.0 µs   21.79x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     162.2 µs    364.5 µs    2.25x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     371.8 µs    429.9 µs    1.16x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    410.9 µs   8370.8 µs   20.37x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      278.9 µs   1926.5 µs    6.91x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    149.4 µs    172.2 µs    1.15x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.7 µs   2635.8 µs  102.71x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2678.5 µs   9890.7 µs    3.69x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    754.0 µs   6046.5 µs    8.02x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         54.4 µs    284.4 µs    5.23x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     19.1 µs   1937.3 µs  101.44x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       46.6 µs    153.2 µs    3.28x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       379.9 µs   1699.4 µs    4.47x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         281.7 µs    805.5 µs    2.86x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     525.9 µs   8908.1 µs   16.94x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         419.4 µs   5858.8 µs   13.97x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    493.9 µs   8359.3 µs   16.93x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        303.2 µs   3446.7 µs   11.37x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    293.7 µs   3801.7 µs   12.94x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    328.4 µs  13986.4 µs   42.59x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     630.3 µs  12641.7 µs   20.06x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2362.9 µs   8540.7 µs    3.61x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                31366.9 µs 238311.6 µs    7.60x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 6
<details>
<summary>Click to expand Run 6 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    91.9 µs     96.3 µs       Tie (~1.0x)
2. 50 Grid Lines (1px Stroked Paths)                   81.6 µs    246.2 µs    3.02x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.6 µs    763.8 µs   11.31x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                208.6 µs    268.0 µs    1.28x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                64.6 µs    239.8 µs    3.71x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs     22.9 µs    2.26x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  346.8 µs   4796.6 µs   13.83x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  33.0 µs    178.3 µs    5.39x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                32.2 µs    421.1 µs   13.07x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.6 µs    120.8 µs    2.59x (Nisaba)
11. Compound Transforms (16 Elements)                  76.6 µs    355.0 µs    4.64x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             13.8 µs    102.6 µs    7.42x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1094.4 µs  10032.0 µs    9.17x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             162.2 µs    940.9 µs    5.80x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               81.3 µs    189.1 µs    2.32x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  287.9 µs   3470.6 µs   12.06x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            247.7 µs   2379.7 µs    9.61x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            281.0 µs    640.3 µs    2.28x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             560.5 µs   3041.3 µs    5.43x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         510.6 µs   7422.7 µs   14.54x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            196.1 µs   8033.8 µs   40.97x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             573.7 µs   2026.0 µs    3.53x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            187.6 µs    609.2 µs    3.25x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        698.5 µs   1329.3 µs    1.90x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         401.3 µs   6039.3 µs   15.05x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       942.0 µs   3830.5 µs    4.07x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          615.6 µs   2531.2 µs    4.11x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    155.5 µs    287.0 µs    1.85x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    143.0 µs    234.7 µs    1.64x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       61.0 µs    118.6 µs    1.94x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      411.7 µs    520.1 µs    1.26x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    519.9 µs   1030.4 µs    1.98x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     39.1 µs     49.3 µs    1.26x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.8 µs    208.5 µs    2.52x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1051.2 µs   3713.6 µs    3.53x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1043.5 µs   3022.4 µs    2.90x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          204.9 µs   1085.5 µs    5.30x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          203.2 µs   1007.0 µs    4.96x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          200.4 µs   1034.3 µs    5.16x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           45.6 µs    102.5 µs    2.25x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         51.2 µs    104.5 µs    2.04x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          45.5 µs    298.5 µs    6.56x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     82.0 µs    363.0 µs    4.43x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        83.9 µs    324.7 µs    3.87x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      385.3 µs   1191.9 µs    3.09x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       267.9 µs    894.5 µs    3.34x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    371.6 µs   1158.8 µs    3.12x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.7 µs    9.23x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        503.9 µs   5275.6 µs   10.47x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          309.6 µs   5906.7 µs   19.08x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    334.7 µs  11653.1 µs   34.82x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        26.8 µs     95.2 µs    3.55x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     79.6 µs    723.1 µs    9.09x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    610.5 µs   6586.9 µs   10.79x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            89.8 µs    151.5 µs    1.69x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           450.0 µs    763.5 µs    1.70x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          904.5 µs   1450.8 µs    1.60x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           257.7 µs    795.0 µs    3.09x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    194.1 µs    520.0 µs    2.68x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    179.7 µs    483.8 µs    2.69x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)       92.8 µs   1011.3 µs   10.90x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     268.3 µs   3223.4 µs   12.01x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      590.2 µs   7480.9 µs   12.68x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     610.6 µs    952.6 µs    1.56x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     179.2 µs    455.8 µs    2.54x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    293.4 µs   1376.1 µs    4.69x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         867.7 µs   4036.3 µs    4.65x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      7.7 µs      9.0 µs    1.17x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    443.2 µs   9303.8 µs   20.99x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     136.5 µs    310.3 µs    2.27x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     320.3 µs    373.5 µs    1.17x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    347.8 µs   7067.2 µs   20.32x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      227.3 µs   1645.3 µs    7.24x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    125.4 µs    145.7 µs    1.16x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     21.7 µs   2258.0 µs  103.98x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2236.0 µs   8377.2 µs    3.75x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    641.7 µs   5076.3 µs    7.91x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         46.5 µs    249.7 µs    5.37x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     16.1 µs   1636.7 µs  101.36x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       39.3 µs    130.3 µs    3.32x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       321.2 µs   1439.1 µs    4.48x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         239.7 µs    682.7 µs    2.85x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     446.1 µs   7529.4 µs   16.88x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         346.7 µs   5038.6 µs   14.53x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    423.9 µs   7085.0 µs   16.71x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        252.2 µs   2935.1 µs   11.64x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    247.3 µs   3223.4 µs   13.04x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    280.7 µs  11812.8 µs   42.08x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     523.9 µs  10754.7 µs   20.53x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2018.1 µs   7479.5 µs    3.71x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                29915.7 µs 224388.5 µs    7.50x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 7
<details>
<summary>Click to expand Run 7 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    78.1 µs     82.2 µs    1.05x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   69.5 µs    209.1 µs    3.01x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                57.6 µs    655.8 µs   11.39x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                174.9 µs    226.8 µs    1.30x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                56.1 µs    209.3 µs    3.73x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  8.7 µs     19.8 µs    2.27x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  280.8 µs   4136.2 µs   14.73x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  29.8 µs    165.4 µs    5.55x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                28.6 µs    363.1 µs   12.70x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               40.5 µs    105.0 µs    2.59x (Nisaba)
11. Compound Transforms (16 Elements)                  66.7 µs    316.5 µs    4.74x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             12.4 µs     91.7 µs    7.40x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1005.1 µs   8935.6 µs    8.89x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             148.5 µs    852.5 µs    5.74x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               72.5 µs    167.7 µs    2.31x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  259.0 µs   3189.9 µs   12.32x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            220.4 µs   2168.0 µs    9.84x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            257.2 µs    559.9 µs    2.18x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             500.2 µs   2811.0 µs    5.62x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         480.5 µs   6864.7 µs   14.29x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            180.7 µs   7588.4 µs   42.00x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             525.2 µs   1930.2 µs    3.68x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            181.4 µs    595.7 µs    3.28x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        697.8 µs   1314.7 µs    1.88x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         363.6 µs   6099.3 µs   16.77x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       942.2 µs   3911.8 µs    4.15x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          617.9 µs   2505.1 µs    4.05x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    154.6 µs    286.7 µs    1.85x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    143.1 µs    234.2 µs    1.64x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.6 µs    113.7 µs    1.91x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      414.8 µs    521.2 µs    1.26x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    521.5 µs   1062.1 µs    2.04x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     33.6 µs     46.9 µs    1.40x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     85.1 µs    215.4 µs    2.53x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1047.9 µs   3796.5 µs    3.62x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1025.8 µs   2965.6 µs    2.89x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          203.1 µs   1084.7 µs    5.34x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          203.0 µs   1018.8 µs    5.02x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          203.6 µs   1059.7 µs    5.21x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           47.0 µs    105.3 µs    2.24x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.8 µs    108.0 µs    2.04x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.9 µs    295.6 µs    6.30x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     81.7 µs    352.4 µs    4.31x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        84.2 µs    324.7 µs    3.86x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      373.6 µs   1169.9 µs    3.13x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       308.0 µs    894.1 µs    2.90x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    371.2 µs   1170.0 µs    3.15x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      1.1 µs      6.7 µs    5.82x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        511.5 µs   5231.6 µs   10.23x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          310.9 µs   5916.0 µs   19.03x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    352.7 µs  11728.6 µs   33.25x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.6 µs     98.1 µs    3.55x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     79.6 µs    733.3 µs    9.21x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    631.6 µs   6567.6 µs   10.40x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            90.0 µs    152.1 µs    1.69x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           446.7 µs    764.9 µs    1.71x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          930.1 µs   1528.6 µs    1.64x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           296.5 µs    900.5 µs    3.04x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    228.4 µs    612.3 µs    2.68x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    209.4 µs    562.8 µs    2.69x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      110.6 µs   1205.0 µs   10.90x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     315.5 µs   3813.5 µs   12.09x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      697.7 µs   8913.9 µs   12.78x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     726.8 µs   1137.5 µs    1.57x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     212.0 µs    534.0 µs    2.52x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    357.3 µs   1612.0 µs    4.51x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1022.7 µs   4795.1 µs    4.69x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.0 µs     10.8 µs    1.21x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    513.9 µs  10958.8 µs   21.33x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     160.3 µs    374.7 µs    2.34x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     387.5 µs    441.2 µs    1.14x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    410.2 µs   8361.8 µs   20.39x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      268.9 µs   1932.9 µs    7.19x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    149.4 µs    172.9 µs    1.16x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.8 µs   2665.2 µs  103.11x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2653.6 µs   9943.5 µs    3.75x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    732.9 µs   6054.4 µs    8.26x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         56.3 µs    286.1 µs    5.08x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     19.2 µs   1957.4 µs  102.08x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       46.4 µs    153.6 µs    3.31x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       396.9 µs   1706.9 µs    4.30x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         294.1 µs    830.3 µs    2.82x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     543.8 µs   8845.4 µs   16.27x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         408.1 µs   5889.5 µs   14.43x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    493.2 µs   8366.0 µs   16.96x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        295.7 µs   3415.6 µs   11.55x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    299.9 µs   3782.9 µs   12.62x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    332.8 µs  14072.8 µs   42.28x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     643.3 µs  12678.4 µs   19.71x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2405.2 µs   8567.3 µs    3.56x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                31922.3 µs 241183.5 µs    7.56x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 8
<details>
<summary>Click to expand Run 8 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    89.0 µs     93.0 µs       Tie (~1.0x)
2. 50 Grid Lines (1px Stroked Paths)                   77.6 µs    244.7 µs    3.15x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.3 µs    752.2 µs   11.17x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                197.9 µs    260.9 µs    1.32x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                64.1 µs    238.5 µs    3.72x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.1 µs     22.9 µs    2.27x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  322.9 µs   4779.4 µs   14.80x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  34.0 µs    183.1 µs    5.38x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                32.9 µs    416.1 µs   12.63x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.7 µs    120.9 µs    2.59x (Nisaba)
11. Compound Transforms (16 Elements)                  76.4 µs    348.9 µs    4.57x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             13.8 µs    102.0 µs    7.39x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1111.5 µs   9863.7 µs    8.87x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             162.1 µs    931.6 µs    5.75x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               81.3 µs    188.8 µs    2.32x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  291.4 µs   3580.4 µs   12.29x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            248.2 µs   2416.1 µs    9.73x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            279.6 µs    610.5 µs    2.18x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             556.7 µs   3041.6 µs    5.46x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         509.6 µs   7123.2 µs   13.98x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            190.0 µs   8032.4 µs   42.28x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             582.9 µs   2009.4 µs    3.45x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            186.8 µs    609.3 µs    3.26x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        697.9 µs   1318.0 µs    1.89x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         362.8 µs   6107.5 µs   16.83x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       944.0 µs   3861.6 µs    4.09x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          638.1 µs   2557.5 µs    4.01x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    154.7 µs    298.3 µs    1.93x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    147.0 µs    242.4 µs    1.65x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       59.0 µs    112.8 µs    1.91x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      410.9 µs    529.8 µs    1.29x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    509.5 µs   1035.5 µs    2.03x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.1 µs     45.3 µs    1.41x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     81.8 µs    208.4 µs    2.55x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1040.6 µs   3721.5 µs    3.58x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1040.8 µs   2966.7 µs    2.85x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          203.1 µs   1093.9 µs    5.39x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          203.1 µs   1036.3 µs    5.10x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          203.1 µs   1063.2 µs    5.24x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           46.5 µs    105.4 µs    2.27x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         52.2 µs    107.7 µs    2.06x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          46.4 µs    296.9 µs    6.40x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     80.9 µs    357.2 µs    4.41x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        83.5 µs    327.9 µs    3.92x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      385.2 µs   1194.2 µs    3.10x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       275.0 µs    893.3 µs    3.25x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    380.8 µs   1152.5 µs    3.03x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.5 µs    9.53x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        495.6 µs   5249.7 µs   10.59x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          308.9 µs   5912.8 µs   19.14x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    362.7 µs  11673.1 µs   32.18x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        27.7 µs     98.2 µs    3.54x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     78.4 µs    710.2 µs    9.06x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    611.9 µs   6607.2 µs   10.80x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            91.0 µs    154.1 µs    1.69x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           455.1 µs    746.2 µs    1.64x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          878.9 µs   1516.3 µs    1.73x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           289.8 µs    897.9 µs    3.10x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    218.1 µs    604.0 µs    2.77x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    197.9 µs    543.3 µs    2.75x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      106.8 µs   1199.7 µs   11.23x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     313.9 µs   3832.2 µs   12.21x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      716.5 µs   8947.3 µs   12.49x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     718.3 µs   1139.7 µs    1.59x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     210.7 µs    532.4 µs    2.53x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    349.9 µs   1626.6 µs    4.65x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1014.9 µs   4737.0 µs    4.67x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.1 µs     10.5 µs    1.16x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    513.8 µs  10956.6 µs   21.32x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     159.8 µs    372.3 µs    2.33x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     374.0 µs    442.7 µs    1.18x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    409.6 µs   8345.5 µs   20.38x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      264.7 µs   1900.0 µs    7.18x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    147.6 µs    172.7 µs    1.17x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     25.4 µs   2680.9 µs  105.44x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2638.9 µs   9856.9 µs    3.74x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    755.9 µs   6004.6 µs    7.94x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         55.6 µs    285.5 µs    5.14x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     19.0 µs   1962.5 µs  103.32x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       46.4 µs    153.1 µs    3.30x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       377.0 µs   1696.2 µs    4.50x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         284.2 µs    811.0 µs    2.85x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     527.7 µs   8886.3 µs   16.84x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         411.5 µs   5922.6 µs   14.39x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    497.7 µs   8354.0 µs   16.78x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        307.8 µs   3495.9 µs   11.36x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    295.9 µs   3922.4 µs   13.25x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    328.1 µs  13894.6 µs   42.35x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     623.0 µs  11834.9 µs   19.00x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   1988.0 µs   7304.3 µs    3.67x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                31820.8 µs 242603.5 µs    7.62x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 9
<details>
<summary>Click to expand Run 9 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    77.8 µs     81.9 µs    1.05x (Nisaba)
2. 50 Grid Lines (1px Stroked Paths)                   67.9 µs    207.4 µs    3.05x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                58.5 µs    654.8 µs   11.19x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                176.2 µs    239.5 µs    1.36x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                54.7 µs    202.4 µs    3.70x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                  8.5 µs     19.3 µs    2.27x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  273.0 µs   3984.5 µs   14.60x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  27.6 µs    151.5 µs    5.48x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                27.4 µs    347.4 µs   12.68x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               39.7 µs    102.4 µs    2.58x (Nisaba)
11. Compound Transforms (16 Elements)                  64.7 µs    296.2 µs    4.58x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             11.8 µs     87.3 µs    7.37x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)                939.0 µs   8312.8 µs    8.85x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             136.5 µs    779.1 µs    5.71x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               68.0 µs    159.3 µs    2.34x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  241.3 µs   2990.6 µs   12.39x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            207.4 µs   2023.0 µs    9.75x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            236.4 µs    518.7 µs    2.19x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             458.4 µs   2550.6 µs    5.56x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         430.7 µs   6022.6 µs   13.98x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            164.4 µs   6716.8 µs   40.86x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             500.8 µs   1704.0 µs    3.40x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            157.3 µs    515.6 µs    3.28x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        590.7 µs   1147.2 µs    1.94x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         307.0 µs   5279.5 µs   17.20x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       829.2 µs   3304.1 µs    3.98x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          525.3 µs   2115.7 µs    4.03x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    133.4 µs    243.3 µs    1.82x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    120.5 µs    198.1 µs    1.64x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       48.2 µs     92.7 µs    1.92x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      341.8 µs    436.9 µs    1.28x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    420.1 µs    894.7 µs    2.13x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     27.8 µs     39.6 µs    1.42x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     71.2 µs    180.9 µs    2.54x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)          887.6 µs   3144.7 µs    3.54x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)          896.0 µs   2547.5 µs    2.84x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          166.1 µs    898.0 µs    5.41x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          170.6 µs    847.4 µs    4.97x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          171.5 µs    889.7 µs    5.19x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           39.0 µs     88.2 µs    2.26x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         44.0 µs     90.2 µs    2.05x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          39.0 µs    248.4 µs    6.36x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     68.1 µs    294.2 µs    4.32x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        70.1 µs    272.3 µs    3.89x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      323.4 µs    998.2 µs    3.09x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       231.4 µs    746.6 µs    3.23x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    311.8 µs    973.9 µs    3.12x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.6 µs      5.6 µs    9.26x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        420.3 µs   4495.7 µs   10.70x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          260.3 µs   4991.5 µs   19.18x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    298.1 µs   9818.0 µs   32.94x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        23.3 µs     82.5 µs    3.55x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     66.6 µs    598.9 µs    8.99x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    515.9 µs   5590.7 µs   10.84x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            76.2 µs    129.6 µs    1.70x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           382.0 µs    648.2 µs    1.70x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          788.0 µs   1318.4 µs    1.67x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           241.6 µs    754.5 µs    3.12x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    187.6 µs    519.0 µs    2.77x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    175.9 µs    484.4 µs    2.75x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)       93.5 µs   1015.3 µs   10.86x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     274.2 µs   3271.7 µs   11.93x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      609.9 µs   7821.4 µs   12.83x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     640.0 µs   1011.1 µs    1.58x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     187.8 µs    479.1 µs    2.55x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    309.4 µs   1436.6 µs    4.64x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)         914.5 µs   4291.7 µs    4.69x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      8.1 µs      9.7 µs    1.19x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    466.7 µs   9936.1 µs   21.29x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     151.5 µs    343.9 µs    2.27x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     361.4 µs    414.7 µs    1.15x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    388.4 µs   7909.7 µs   20.37x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      258.8 µs   1787.1 µs    6.91x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    138.4 µs    162.9 µs    1.18x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     24.1 µs   2525.7 µs  105.00x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2552.6 µs   9642.0 µs    3.78x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    754.3 µs   5983.6 µs    7.93x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         54.4 µs    284.1 µs    5.22x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     19.3 µs   1990.3 µs  102.92x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       47.9 µs    158.0 µs    3.30x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       385.9 µs   1708.1 µs    4.43x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         287.3 µs    830.4 µs    2.89x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     543.4 µs   8839.6 µs   16.27x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         400.5 µs   5946.4 µs   14.85x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    492.4 µs   8235.6 µs   16.72x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        295.0 µs   3489.5 µs   11.83x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    294.3 µs   3803.1 µs   12.92x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    331.3 µs  13963.3 µs   42.15x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     622.6 µs  12533.4 µs   20.13x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2344.3 µs   8599.8 µs    3.67x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                28950.4 µs 222500.8 µs    7.69x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

### Run 10
<details>
<summary>Click to expand Run 10 full log</summary>

```text
======================================================================================================================================
                               NISABA vs GNU CAIRO 1.18 — CLASH OF TITANS (90 SUITES)                         
                                          (Strict 1:1 Parity — Single-Threaded CPU Engine)                                            
======================================================================================================================================

Operation Name                                     Nisaba (µs)  Cairo (µs)          vs Cairo
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Fill (1080x740)                    88.7 µs     92.8 µs       Tie (~1.0x)
2. 50 Grid Lines (1px Stroked Paths)                   77.6 µs    242.7 µs    3.13x (Nisaba)
3. 25 Alpha Circles (Porter-Duff Blend)                67.2 µs    766.1 µs   11.40x (Nisaba)
4. Cubic Bezier Curves (Fill + Stroke)                204.8 µs    267.8 µs    1.31x (Nisaba)
5. 4 Rounded Rect Cards (Fill + Stroke)                63.9 µs    237.9 µs    3.72x (Nisaba)
6. Multi-Stop Linear Gradient (480x36)                 10.0 µs     22.9 µs    2.28x (Nisaba)
7. Radial Glow Gradient (250px Glow)                  323.4 µs   4790.1 µs   14.81x (Nisaba)
8. 12 Radial Rotated Spokes (Strokes)                  34.4 µs    182.4 µs    5.31x (Nisaba)
9. Dashed Orbit Ring (Stroke with Dash)                32.8 µs    418.2 µs   12.76x (Nisaba)
10. Concave 10-Point Star (Winding Fill)               46.7 µs    120.3 µs    2.58x (Nisaba)
11. Compound Transforms (16 Elements)                  76.5 µs    349.3 µs    4.57x (Nisaba)
12. Rectangular Clipping (Nested Viewport)             13.7 µs    101.9 µs    7.44x (Nisaba)
13. 100 Anti-Aliased Diagonals (Lines)               1094.1 µs   9824.6 µs    8.98x (Nisaba)
14. 30 Overlapping UI Chips (Alpha Stack)             160.8 µs    940.1 µs    5.85x (Nisaba)
15. Closed Cubic Loop (Trefoil Figure-8)               81.0 µs    188.5 µs    2.33x (Nisaba)
16. 500 Alpha Disks (Particle Cloud)                  286.8 µs   3494.3 µs   12.18x (Nisaba)
17. 1000-Pt Waveform (High-Density Stroke)            239.5 µs   2376.2 µs    9.92x (Nisaba)
18. Thick 16px Stroke (Miter & Round Caps)            279.2 µs    610.0 µs    2.18x (Nisaba)
19. 50 Rotated Quads (Affine Alpha Stack)             555.0 µs   2995.2 µs    5.40x (Nisaba)
20. 25 Concentric Rings (Alternating Strokes)         503.9 µs   7154.2 µs   14.20x (Nisaba)
21. Diagonal Full-HD Gradient (45° Angle)            192.5 µs   8022.4 µs   41.68x (Nisaba)
22. 3-Stage Nested Clipping (Criss-Cross)             564.5 µs   1961.2 µs    3.47x (Nisaba)
23. Dashed Cubic Spline (Curved Intervals)            186.5 µs    609.4 µs    3.27x (Nisaba)
24. 40-Point Starburst (Dense Winding Polygon)        699.4 µs   1318.8 µs    1.89x (Nisaba)
25. 2-Point Focal Radial Spotlight (Lighting)         363.3 µs   6132.8 µs   16.88x (Nisaba)
26. Ellipse Rendering (25 Ellipses Fill+Stroke)       951.6 µs   3871.0 µs    4.07x (Nisaba)
27. Circular Arc Rendering (25 Stroked Arcs)          619.1 µs   2509.9 µs    4.05x (Nisaba)
28. Even-Odd Polygon Fill (Self-Intersecting Star)    154.9 µs    286.1 µs    1.85x (Nisaba)
29. Non-Zero Winding Fill (Self-Intersecting Star)    142.4 µs    234.0 µs    1.64x (Nisaba)
30. Direct Pixmap Blit (4x 256x256 1:1 Surfaces)       57.3 µs    109.8 µs    1.92x (Nisaba)
31. Image Bilinear Scaling (2x Scale Up 512x512)      398.0 µs    503.4 µs    1.27x (Nisaba)
32. Image Rotation & Affine Transform (35° Filtered)    508.2 µs   1024.6 µs    2.02x (Nisaba)
33. Image Pattern Fill (Circle Filled with Texture)     32.1 µs     45.6 µs    1.42x (Nisaba)
34. Pattern Repeat & Tiling (64x64 Texture Repeat)     82.7 µs    215.0 µs    2.60x (Nisaba)
35. Anti-Aliasing ON (50 Diagonals Coverage)         1063.7 µs   3703.6 µs    3.48x (Nisaba)
36. Anti-Aliasing OFF (50 Diagonals Aliased)         1043.4 µs   3002.1 µs    2.88x (Nisaba)
37. Stroke Join: Miter (Zigzag Polygon 12px)          202.6 µs   1080.9 µs    5.34x (Nisaba)
38. Stroke Join: Bevel (Zigzag Polygon 12px)          197.5 µs    976.8 µs    4.94x (Nisaba)
39. Stroke Join: Round (Zigzag Polygon 12px)          202.3 µs   1081.7 µs    5.35x (Nisaba)
40. Stroke Cap: Butt (20 Line Segments 14px)           48.0 µs    108.7 µs    2.26x (Nisaba)
41. Stroke Cap: Square (20 Line Segments 14px)         54.1 µs    110.9 µs    2.05x (Nisaba)
42. Stroke Cap: Round (20 Line Segments 14px)          48.0 µs    304.5 µs    6.35x (Nisaba)
43. Transform Isolation: Pure Translate (20 Shapes)     83.4 µs    363.8 µs    4.36x (Nisaba)
44. Transform Isolation: Pure Scale (20 Shapes)        88.3 µs    337.2 µs    3.82x (Nisaba)
45. Transform Isolation: Pure Rotate (20 Shapes)      383.5 µs   1184.4 µs    3.09x (Nisaba)
46. Transform Isolation: Pure Shear (20 Shapes)       274.8 µs    888.9 µs    3.24x (Nisaba)
47. Transform Isolation: Combined Affine (20 Shapes)    370.8 µs   1139.5 µs    3.07x (Nisaba)
48. State Stack: Save/Restore Overhead (50 Passes)      0.7 µs      6.5 µs    9.56x (Nisaba)
49. Compositing Operator: Multiply (25 Shapes)        497.2 µs   5609.6 µs   11.28x (Nisaba)
50. Compositing Operator: Screen (25 Shapes)          317.9 µs   5990.5 µs   18.85x (Nisaba)
51. Compositing Operator: Source-In & Xor (25 Shapes)    354.3 µs  11596.2 µs   32.73x (Nisaba)
52. Path Complexity: Low (10 Vertices Waveform)        26.7 µs     94.7 µs    3.55x (Nisaba)
53. Path Complexity: Medium (100 Vertices Waveform)     77.2 µs    699.9 µs    9.06x (Nisaba)
54. Path Complexity: High (1000 Vertices Waveform)    612.4 µs   6588.5 µs   10.76x (Nisaba)
55. Primitive Scaling: 100 Rectangles Batch            92.9 µs    152.3 µs    1.64x (Nisaba)
56. Primitive Scaling: 500 Rectangles Batch           441.9 µs    746.1 µs    1.69x (Nisaba)
57. Primitive Scaling: 1000 Rectangles Batch          916.3 µs   1535.6 µs    1.68x (Nisaba)
58. Clip-Depth Scaling: 1-Level Vector Clip           279.2 µs    869.6 µs    3.11x (Nisaba)
59. Clip-Depth Scaling: 2-Level Nested Vector Clip    216.8 µs    584.4 µs    2.69x (Nisaba)
60. Clip-Depth Scaling: 4-Level Nested Vector Clip    202.0 µs    561.8 µs    2.78x (Nisaba)
61. Resolution Scaling: Small (256x256 Viewport)      109.6 µs   1188.7 µs   10.85x (Nisaba)
62. Resolution Scaling: Medium (640x480 Viewport)     314.6 µs   3769.7 µs   11.98x (Nisaba)
63. Resolution Scaling: Full Viewport (1080x740)      700.3 µs   8988.9 µs   12.84x (Nisaba)
64. Quadratic Bezier Splines (20 Connected Quads)     717.7 µs   1104.8 µs    1.54x (Nisaba)
65. Compound Path with Hole (Donut Even-Odd Fill)     210.8 µs    533.8 µs    2.53x (Nisaba)
66. Miter Limit Clamping (Sharp 10° Acute Angles)    346.5 µs   1580.0 µs    4.56x (Nisaba)
67. Sub-Pixel Hairline Strokes (0.25px Lines)        1018.5 µs   4712.7 µs    4.63x (Nisaba)
68. Image Downscaling (4x Minification 0.25x Bilinear)      9.2 µs     10.7 µs    1.17x (Nisaba)
69. Pattern Reflect Tiling (64x64 Texture Reflect)    513.6 µs  10560.8 µs   20.56x (Nisaba)
70. Repeated Linear Gradient (60px Periodic Tile)     136.6 µs    313.1 µs    2.29x (Nisaba)
71. Alpha Mask Surface Blit (8-bit Alpha Masking)     325.0 µs    373.5 µs    1.15x (Nisaba)
72. Blend Modes: Color Dodge & Difference (25 Shapes)    347.5 µs   7052.1 µs   20.29x (Nisaba)
73. Additive Compositing (Plus / Add Blend Mode)      226.9 µs   1613.3 µs    7.11x (Nisaba)
74. Vector Clip on Heavy Curved Stroke (24px Ribbon)    123.7 µs    145.4 µs    1.18x (Nisaba)
75. Animated Dash Offset Phase (Moving Dash Stroke)     21.5 µs   2244.8 µs  104.49x (Nisaba)
76. Freeform Gradient Mesh (Bicubic Coons Patch)     2231.1 µs   8329.6 µs    3.73x (Nisaba)
77. True 2-Point Conical Gradient (r0 > 0, r1 > 0)    634.1 µs   5125.4 µs    8.08x (Nisaba)
78. Axis-Aligned Stroked Rectangles (25 Rects)         47.1 µs    245.0 µs    5.20x (Nisaba)
79. Dashed Stroke with Round Caps (Round Intervals)     16.1 µs   1660.3 µs  103.06x (Nisaba)
80. Anisotropic Scaled Stroke (scale 2.5x, 0.4y)       39.4 µs    129.7 µs    3.29x (Nisaba)
81. Non-Convex Star Vector Clip (Even-Odd Rule)       317.0 µs   1437.5 µs    4.54x (Nisaba)
82. Multi-Contour Complex Path (20 Sub-Paths)         238.8 µs    680.2 µs    2.85x (Nisaba)
83. Blend Modes: Overlay & Soft-Light (25 Shapes)     462.4 µs   7611.5 µs   16.46x (Nisaba)
84. Blend Modes: Darken & Lighten (25 Shapes)         357.5 µs   5007.8 µs   14.01x (Nisaba)
85. Blend Modes: Color-Burn & Hard-Light (25 Shapes)    428.2 µs   7191.6 µs   16.79x (Nisaba)
86. Blend Modes: Exclusion & Clear (25 Shapes)        255.1 µs   3064.1 µs   12.01x (Nisaba)
87. Reverse Porter-Duff: Dest-Over & Dest-Out (25 Shapes)    262.9 µs   3473.2 µs   13.21x (Nisaba)
88. Reverse Porter-Duff: Src-Atop & Dest-Atop (25 Shapes)    300.5 µs  12594.7 µs   41.91x (Nisaba)
89. HSL Blend Modes: Hue & Luminosity (25 Shapes)     584.1 µs  11764.3 µs   20.14x (Nisaba)
90. Bilinear Filtered Pattern under Rotation (30° Tiling)   2276.4 µs   8302.9 µs    3.65x (Nisaba)
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD                                30860.5 µs 232148.5 µs    7.52x (Nisaba)
=================================================================================================================================================================

=================================================================================================================================================================
                                                  WIN / LOSS BENCHMARK SCORECARD                                                                 
=================================================================================================================================================================
  Total Benchmark Suites: 90

  [Nisaba vs GNU Cairo 1.18]
    * Nisaba Won: 90 / 90 tests (100.0%)
    * Cairo Won:  0 / 90 tests (0.0%)
=================================================================================================================================================================
```
</details>

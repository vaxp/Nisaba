# Benchmark Report: Nisaba GPU Engine vs Google Skia (Ganesh GL)

**Execution Date:** 2026-10-16 21:47:33  
**Runs Executed:** 10 consecutive runs  
**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  

---

## Executive Summary & Aggregate Averages

Across **10 consecutive executions** of the 100-suite GPU micro-profiling benchmark:
- **Overall CPU Command Submission Overhead:** **7.16x FASTER (Nisaba)**
  * Nisaba Mean CPU Overhead: `2663.9 µs` (2.66 ms)
  * Skia Mean CPU Overhead: `19081.7 µs` (19.08 ms)
- **Overall End-to-End Frame Throughput:** **1.13x FASTER (Nisaba)**
  * Nisaba Mean Total Frame Time: `107873.9 µs` (107.87 ms)
  * Skia Mean Total Frame Time: `121610.8 µs` (121.61 ms)

---

## 10-Run Mean Results Across All 100 GPU Suites

| # | Suite Name | Nisaba CPU (µs) | Nisaba GPU (µs) | Nisaba Total (µs) | Skia CPU (µs) | Skia GPU (µs) | Skia Total (µs) | CPU Ratio | Total Ratio |
| :---: | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 1 | Solid Background Clear (1080x720) | **3.1** | 896.4 | **899.6** | 11.5 | 856.8 | 868.3 | **3.7x Nisaba** | Tie (~1.0x) |
| 2 | 100 Grid Lines (1px Hairlines) | **18.4** | 916.4 | **934.8** | 43.9 | 1008.4 | 1052.2 | **2.4x Nisaba** | **1.13x Nisaba** |
| 3 | 20 Thick Diagonal Lines (16px Round) | **27.7** | 969.3 | **996.9** | 249.4 | 1069.2 | 1318.5 | **9.0x Nisaba** | **1.32x Nisaba** |
| 4 | 100 Solid Rectangles (Varied Bounds) | **29.3** | 1016.5 | **1045.8** | 76.3 | 963.9 | 1040.2 | **2.6x Nisaba** | Tie (~1.0x) |
| 5 | 100 Alpha Rectangles (Overlapping Blend) | **24.2** | 1297.0 | **1321.2** | 73.0 | 1232.9 | 1305.9 | **3.0x Nisaba** | Tie (~1.0x) |
| 6 | 50 Simple Rounded Rects (12px Radius) | **17.9** | 1207.3 | **1225.2** | 58.4 | 980.5 | 1038.9 | **3.3x Nisaba** | 1.18x Skia |
| 7 | 50 Varying-Radius Rounded Rects | **17.9** | 1242.8 | **1260.7** | 46.0 | 993.0 | 1039.0 | **2.6x Nisaba** | 1.21x Skia |
| 8 | 50 Stroked Rounded Rects (1.5px Border) | **22.3** | 1017.8 | **1040.0** | 58.7 | 951.3 | 1010.0 | **2.6x Nisaba** | Tie (~1.0x) |
| 9 | 100 Alpha Circles / Disks (Fill) | **20.6** | 975.3 | **995.8** | 73.6 | 945.0 | 1018.5 | **3.6x Nisaba** | Tie (~1.0x) |
| 10 | 25 Concentric Rings (2px Stroke) | **40.2** | 964.9 | **1005.1** | 41.6 | 1015.8 | 1057.4 | Tie (~1.0x) | Tie (~1.0x) |
| 11 | 50 Two-Stop Linear Gradients | **18.9** | 1423.2 | **1442.1** | 190.8 | 1237.0 | 1427.8 | **10.1x Nisaba** | Tie (~1.0x) |
| 12 | 50 Diagonal Gradients (45° Angle) | **19.3** | 1448.1 | **1467.4** | 194.5 | 1176.2 | 1370.7 | **10.1x Nisaba** | 1.07x Skia |
| 13 | 25 Radial Glow Gradients (300px) | **15.2** | 1968.9 | **1984.0** | 108.0 | 1435.1 | 1543.1 | **7.1x Nisaba** | 1.29x Skia |
| 14 | 20 Box Blur / Drop Shadows (15px) | **12.3** | 1154.6 | **1166.9** | 99.3 | 1207.9 | 1307.2 | **8.1x Nisaba** | **1.12x Nisaba** |
| 15 | 10 Concave 10-Point Stars (Fill) | **46.2** | 1099.2 | **1145.5** | 390.5 | 1101.9 | 1492.3 | **8.4x Nisaba** | **1.30x Nisaba** |
| 16 | 10 Concave 10-Point Stars (3px Stroke) | **31.7** | 968.8 | **1000.5** | 877.5 | 1149.3 | 2026.8 | **27.7x Nisaba** | **2.03x Nisaba** |
| 17 | 1000-Point Waveform (Dense Stroke) | **91.5** | 918.4 | **1009.9** | 845.6 | 1230.5 | 2076.0 | **9.2x Nisaba** | **2.06x Nisaba** |
| 18 | 50 Rotated Cards (Affine Transforms) | **19.2** | 1079.7 | **1098.9** | 67.2 | 1003.3 | 1070.5 | **3.5x Nisaba** | Tie (~1.0x) |
| 19 | 25 Scissor Clipped Viewports | **14.2** | 980.6 | **994.8** | 68.2 | 563.9 | 632.1 | **4.8x Nisaba** | 1.57x Skia |
| 20 | 50 UI Text Labels (Inter-Regular) | **101.5** | 910.0 | **1011.5** | 100.1 | 917.3 | 1017.3 | Tie (~1.0x) | Tie (~1.0x) |
| 21 | 25 Quadratic Bezier Curves (Fill+Stroke) | **103.2** | 1047.8 | **1151.0** | 371.5 | 1151.1 | 1522.7 | **3.6x Nisaba** | **1.32x Nisaba** |
| 22 | 20 Cubic Bezier Splines (4px Stroke) | **40.7** | 962.7 | **1003.4** | 1255.5 | 1376.6 | 2632.2 | **30.8x Nisaba** | **2.62x Nisaba** |
| 23 | Complex Bezier Ribbon (24px Alpha) | **15.0** | 960.1 | **975.1** | 191.0 | 1124.4 | 1315.3 | **12.7x Nisaba** | **1.35x Nisaba** |
| 24 | Circular Arcs with arcTo (20 Corners) | **29.7** | 955.4 | **985.1** | 519.1 | 1227.0 | 1746.1 | **17.5x Nisaba** | **1.77x Nisaba** |
| 25 | Directional Arcs with arc() (30 Arcs) | **58.6** | 946.2 | **1004.9** | 155.1 | 958.6 | 1113.7 | **2.6x Nisaba** | **1.11x Nisaba** |
| 26 | Compound Path with Holes (15 Donuts) | **89.1** | 1313.6 | **1402.8** | 536.4 | 1166.1 | 1702.6 | **6.0x Nisaba** | **1.21x Nisaba** |
| 27 | Trefoil Figure-8 Cubic Knot (8 Loops) | **41.8** | 952.0 | **993.8** | 494.0 | 1100.5 | 1594.5 | **11.8x Nisaba** | **1.60x Nisaba** |
| 28 | Archimedean Spiral Polyline (500 Pts) | **47.4** | 938.1 | **985.5** | 824.2 | 1225.0 | 2049.2 | **17.4x Nisaba** | **2.08x Nisaba** |
| 29 | Multi-Contour Disjoint Paths (25 Sub-Paths) | **10.5** | 979.4 | **989.9** | 272.9 | 1195.8 | 1468.7 | **26.0x Nisaba** | **1.48x Nisaba** |
| 30 | Sharp Zigzag Mountain (50 Vertices) | **28.0** | 1351.1 | **1379.0** | 1190.3 | 1366.3 | 2556.6 | **42.6x Nisaba** | **1.85x Nisaba** |
| 31 | Stroke Join: Miter (12px Stroke) | **11.9** | 972.0 | **983.9** | 396.1 | 1141.4 | 1537.5 | **33.3x Nisaba** | **1.56x Nisaba** |
| 32 | Stroke Join: Bevel (12px Stroke) | **11.5** | 980.0 | **991.5** | 393.5 | 1157.1 | 1550.6 | **34.3x Nisaba** | **1.56x Nisaba** |
| 33 | Stroke Join: Round (12px Stroke) | **15.5** | 975.1 | **990.5** | 409.3 | 1175.5 | 1584.8 | **26.4x Nisaba** | **1.60x Nisaba** |
| 34 | Stroke Cap: Butt (30 Segments 12px) | **14.4** | 956.4 | **970.8** | 46.6 | 1005.6 | 1052.2 | **3.2x Nisaba** | **1.08x Nisaba** |
| 35 | Stroke Cap: Square (30 Segments 12px) | **14.9** | 908.2 | **923.1** | 46.8 | 995.9 | 1042.6 | **3.1x Nisaba** | **1.13x Nisaba** |
| 36 | Stroke Cap: Round (30 Segments 12px) | **38.6** | 971.4 | **1010.0** | 62.9 | 953.0 | 1015.8 | **1.6x Nisaba** | Tie (~1.0x) |
| 37 | Miter Limit Clamping (10° Acute Spikes) | **16.9** | 978.3 | **995.3** | 700.4 | 1248.0 | 1948.5 | **41.3x Nisaba** | **1.96x Nisaba** |
| 38 | Sub-Pixel Hairline Strokes (0.25px Lines) | **19.9** | 966.7 | **986.6** | 71.4 | 953.6 | 1025.0 | **3.6x Nisaba** | Tie (~1.0x) |
| 39 | Ultra-Heavy Geometric Ribbon (40px Wide) | **12.2** | 988.4 | **1000.5** | 159.0 | 1092.9 | 1251.9 | **13.0x Nisaba** | **1.25x Nisaba** |
| 40 | Stepped Variable Stroke Widths (1-20px) | **28.1** | 976.0 | **1004.1** | 556.9 | 1184.2 | 1741.1 | **19.8x Nisaba** | **1.73x Nisaba** |
| 41 | 50 Ellipses (rx=60, ry=25 Fill) | **16.1** | 1007.2 | **1023.3** | 42.5 | 980.5 | 1022.9 | **2.6x Nisaba** | Tie (~1.0x) |
| 42 | 50 Stroked Ellipses (2.5px Border) | **16.3** | 1219.3 | **1235.6** | 42.5 | 1054.9 | 1097.4 | **2.6x Nisaba** | 1.13x Skia |
| 43 | 50 Pill / Capsule Badges (r=h/2) | **21.5** | 1033.4 | **1054.9** | 61.4 | 936.0 | 997.3 | **2.9x Nisaba** | 1.06x Skia |
| 44 | 50 Asymmetric Teardrop Rounded Rects | **16.9** | 1126.8 | **1143.7** | 46.5 | 960.0 | 1006.6 | **2.8x Nisaba** | 1.14x Skia |
| 45 | High-Density Disks Cloud (500 Disks) | **64.0** | 948.8 | **1012.8** | 215.6 | 955.2 | 1170.8 | **3.4x Nisaba** | **1.16x Nisaba** |
| 46 | High-Density Stroked Rings (250 Rings) | **35.2** | 1149.0 | **1184.2** | 138.0 | 906.6 | 1044.6 | **3.9x Nisaba** | 1.13x Skia |
| 47 | 250 Filled Rectangles Batch | **38.4** | 993.1 | **1031.4** | 131.7 | 934.9 | 1066.6 | **3.4x Nisaba** | Tie (~1.0x) |
| 48 | 150 Stroked Rectangles (2px Border) | **38.4** | 954.4 | **992.8** | 84.0 | 934.0 | 1018.0 | **2.2x Nisaba** | Tie (~1.0x) |
| 49 | 40 Crosshair Aim Reticles | **86.6** | 935.0 | **1021.6** | 305.0 | 1032.1 | 1337.1 | **3.5x Nisaba** | **1.31x Nisaba** |
| 50 | 25 Regular Octagons (Fill + Stroke) | **82.0** | 1005.9 | **1087.9** | 313.2 | 1118.2 | 1431.4 | **3.8x Nisaba** | **1.32x Nisaba** |
| 51 | 30 Vertical Card Linear Gradients | **15.2** | 1261.3 | **1276.5** | 152.2 | 982.1 | 1134.2 | **10.0x Nisaba** | 1.13x Skia |
| 52 | 30 Horizontal Bar Progress Gradients | **15.6** | 1301.5 | **1317.1** | 148.6 | 975.5 | 1124.0 | **9.5x Nisaba** | 1.17x Skia |
| 53 | 15 Concentric Radial Spotlights | **11.6** | 1169.6 | **1181.2** | 92.0 | 960.4 | 1052.4 | **7.9x Nisaba** | 1.12x Skia |
| 54 | 25 Soft Ambient Card Shadows | **14.3** | 1261.2 | **1275.4** | 125.3 | 1064.2 | 1189.5 | **8.8x Nisaba** | 1.07x Skia |
| 55 | 15 Neon Button Glows (Cyan Intense) | **11.7** | 1077.8 | **1089.5** | 84.8 | 950.3 | 1035.2 | **7.3x Nisaba** | Tie (~1.0x) |
| 56 | 16 Multi-Angle Gradient Fan Slices | **27.4** | 992.0 | **1019.4** | 106.9 | 898.4 | 1005.3 | **3.9x Nisaba** | Tie (~1.0x) |
| 57 | Full-Screen Diagonal Horizon Gradient | **8.8** | 1101.6 | **1110.5** | 32.4 | 949.2 | 981.6 | **3.7x Nisaba** | 1.13x Skia |
| 58 | 20 Inset Well Shadows (Recessed) | **54.5** | 1289.2 | **1343.7** | 107.2 | 1042.5 | 1149.7 | **2.0x Nisaba** | 1.17x Skia |
| 59 | 10 Pulsing Circular Radar Wave Glows | **13.4** | 1492.9 | **1506.3** | 66.1 | 1324.1 | 1390.3 | **4.9x Nisaba** | 1.08x Skia |
| 60 | Waveform Polyline Stroked with Gradient | **48.7** | 961.0 | **1009.7** | 569.1 | 1239.8 | 1808.9 | **11.7x Nisaba** | **1.79x Nisaba** |
| 61 | Repeated Texture Tiling (64x64 Pattern) | **9.9** | 1058.8 | **1068.8** | 32.9 | 953.5 | 986.4 | **3.3x Nisaba** | 1.08x Skia |
| 62 | Scaled Image Quad (2x 512x512 Blit) | **9.2** | 986.3 | **995.5** | 31.1 | 911.9 | 943.0 | **3.4x Nisaba** | 1.06x Skia |
| 63 | 45° Rotated Pattern Fill in Circle | **9.4** | 1034.6 | **1044.0** | 38.6 | 946.4 | 985.1 | **4.1x Nisaba** | 1.06x Skia |
| 64 | Translucent Texture Overlay (50% Alpha) | **9.8** | 1036.3 | **1046.1** | 35.8 | 954.1 | 989.9 | **3.6x Nisaba** | 1.06x Skia |
| 65 | Multi-Avatar Grid (16 Avatars) | **12.1** | 992.8 | **1004.9** | 44.7 | 921.5 | 966.1 | **3.7x Nisaba** | Tie (~1.0x) |
| 66 | Texture Pattern on Star Path | **19.4** | 983.8 | **1003.2** | 139.0 | 940.8 | 1079.8 | **7.2x Nisaba** | **1.08x Nisaba** |
| 67 | 20px Curved Path Stroked with Texture | **14.7** | 991.6 | **1006.4** | 173.7 | 1133.8 | 1307.5 | **11.8x Nisaba** | **1.30x Nisaba** |
| 68 | Minified Texture Quad (0.25x Downscale) | **18.8** | 981.5 | **1000.4** | 71.0 | 920.5 | 991.5 | **3.8x Nisaba** | Tie (~1.0x) |
| 69 | Pure Translation Stack (50 Cards) | **23.4** | 1034.4 | **1057.8** | 63.6 | 955.7 | 1019.3 | **2.7x Nisaba** | Tie (~1.0x) |
| 70 | Pure Rotation Cluster (36 Spokes 10°) | **17.7** | 956.3 | **974.0** | 53.1 | 932.4 | 985.5 | **3.0x Nisaba** | Tie (~1.0x) |
| 71 | Pure Scale Zoom Progression (20 Rects) | **23.9** | 940.7 | **964.6** | 38.1 | 966.8 | 1004.9 | **1.6x Nisaba** | Tie (~1.0x) |
| 72 | Shear / Skew Parallelograms (30 Quads) | **17.3** | 965.4 | **982.8** | 50.3 | 946.4 | 996.6 | **2.9x Nisaba** | Tie (~1.0x) |
| 73 | Combined Affine Transforms (30 Shapes) | **17.0** | 950.5 | **967.5** | 45.6 | 944.4 | 990.0 | **2.7x Nisaba** | Tie (~1.0x) |
| 74 | Deep Hierarchical State Stack (20 Levels) | **14.7** | 977.5 | **992.2** | 41.7 | 957.7 | 999.4 | **2.8x Nisaba** | Tie (~1.0x) |
| 75 | Reset Transform Stress (50 Cycles) | **20.0** | 944.0 | **964.0** | 56.8 | 930.7 | 987.5 | **2.8x Nisaba** | Tie (~1.0x) |
| 76 | Planetary Orbit Hierarchy (10 Systems) | **15.5** | 926.1 | **941.5** | 45.0 | 934.5 | 979.4 | **2.9x Nisaba** | Tie (~1.0x) |
| 77 | Multi-Scissor Grid (16 Viewports) | **13.6** | 972.7 | **986.3** | 58.9 | 635.3 | 694.2 | **4.3x Nisaba** | 1.42x Skia |
| 78 | Hierarchical Intersecting Scissors (4 Levels) | **14.4** | 1119.5 | **1133.8** | 39.2 | 1033.1 | 1072.3 | **2.7x Nisaba** | 1.06x Skia |
| 79 | Scissored Stroked Waves (1000 Pts Clipped) | **95.4** | 933.9 | **1029.3** | 882.1 | 926.8 | 1808.9 | **9.2x Nisaba** | **1.76x Nisaba** |
| 80 | Scissored Gradient Card (Clipped) | **13.0** | 1104.3 | **1117.3** | 85.8 | 581.0 | 666.8 | **6.6x Nisaba** | 1.68x Skia |
| 81 | Circular Overflow Clip (25 Boxes) | **14.8** | 967.6 | **982.4** | 71.5 | 509.1 | 580.5 | **4.8x Nisaba** | 1.69x Skia |
| 82 | Scissor Invalidation / Reset (25 Cycles) | **14.1** | 975.5 | **989.6** | 48.3 | 950.4 | 998.7 | **3.4x Nisaba** | Tie (~1.0x) |
| 83 | Composite Op: Source-Over (30 Shapes) | **14.3** | 981.4 | **995.8** | 41.5 | 941.7 | 983.2 | **2.9x Nisaba** | Tie (~1.0x) |
| 84 | Composite Op: Lighter / Plus (30 Particles) | **12.9** | 950.9 | **963.8** | 44.8 | 943.9 | 988.6 | **3.5x Nisaba** | Tie (~1.0x) |
| 85 | Composite Op: Source-In (Alpha Masking) | **13.8** | 945.0 | **958.7** | 43.5 | 1117.9 | 1161.4 | **3.2x Nisaba** | **1.21x Nisaba** |
| 86 | Composite Op: Source-Out (Cutout) | **14.5** | 969.4 | **983.9** | 42.5 | 1092.2 | 1134.7 | **2.9x Nisaba** | **1.15x Nisaba** |
| 87 | Composite Op: Atop (Target Bounds) | **13.6** | 976.4 | **990.0** | 44.6 | 941.0 | 985.7 | **3.3x Nisaba** | Tie (~1.0x) |
| 88 | Composite Op: Dest-Over (Under-Drawing) | **13.8** | 977.8 | **991.6** | 44.0 | 951.4 | 995.4 | **3.2x Nisaba** | Tie (~1.0x) |
| 89 | Composite Op: Dest-Out (Eraser Mask) | **13.9** | 981.1 | **995.1** | 45.3 | 941.6 | 986.9 | **3.3x Nisaba** | Tie (~1.0x) |
| 90 | Composite Op: Dest-Atop (Inverse) | **14.3** | 971.0 | **985.3** | 43.5 | 1104.8 | 1148.3 | **3.0x Nisaba** | **1.17x Nisaba** |
| 91 | Composite Op: Xor (Exclusive Blend) | **13.7** | 976.9 | **990.6** | 44.5 | 934.0 | 978.5 | **3.2x Nisaba** | Tie (~1.0x) |
| 92 | Composite Op: Copy (Direct Overwrite) | **15.3** | 957.6 | **972.9** | 44.8 | 941.0 | 985.8 | **2.9x Nisaba** | Tie (~1.0x) |
| 93 | UI Dashboard Gauge (Arc Track + Needle) | **25.2** | 959.5 | **984.7** | 203.8 | 1042.7 | 1246.5 | **8.1x Nisaba** | **1.27x Nisaba** |
| 94 | Audio Spectrum Visualizer (64 Bars) | **31.3** | 1126.7 | **1158.1** | 275.0 | 1076.5 | 1351.5 | **8.8x Nisaba** | **1.17x Nisaba** |
| 95 | Circular Progress Rings (8 Meters) | **43.9** | 976.8 | **1020.7** | 385.6 | 1107.1 | 1492.7 | **8.8x Nisaba** | **1.46x Nisaba** |
| 96 | Modern Card Stack (5 Elevated Cards) | **16.3** | 1625.8 | **1642.2** | 83.2 | 1488.8 | 1572.0 | **5.1x Nisaba** | Tie (~1.0x) |
| 97 | CAD Cross-Hatch Pattern (80 Angled Lines) | **23.6** | 917.0 | **940.6** | 41.5 | 1029.2 | 1070.7 | **1.8x Nisaba** | **1.14x Nisaba** |
| 98 | Floating Action Button (FAB 10 Buttons) | **36.7** | 979.3 | **1016.0** | 125.2 | 933.6 | 1058.8 | **3.4x Nisaba** | Tie (~1.0x) |
| 99 | Anti-Aliasing Geometry Grid (50 Diamonds) | **30.7** | 953.7 | **984.4** | 116.8 | 923.7 | 1040.5 | **3.8x Nisaba** | **1.06x Nisaba** |
| 100 | Master Vector Stress: Mixed Mega-Scene | **21.4** | 1334.5 | **1355.9** | 73.3 | 1095.0 | 1168.3 | **3.4x Nisaba** | 1.16x Skia |
| **--** | **TOTAL FRAME OVERHEAD** | **2663.9** | **105210.0** | **107873.9** | **19081.7** | **102529.1** | **121610.8** | **7.16x Nisaba** | **1.13x Nisaba** |

---

## Individual Run Logs (Runs 1 to 10)

### Run 1
<details>
<summary>Click to expand Run 1 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 824.1 µs, Skia: 638.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 901.5 µs, Skia: 1082.0 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 954.7 µs, Skia: 1276.2 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1025.4 µs, Skia: 997.9 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1284.1 µs, Skia: 1229.2 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1031.5 µs, Skia: 854.3 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1245.1 µs, Skia: 1019.6 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1027.5 µs, Skia: 998.1 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 998.0 µs, Skia: 999.1 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1003.1 µs, Skia: 1031.2 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1491.9 µs, Skia: 1454.7 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1530.0 µs, Skia: 1375.2 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 1990.8 µs, Skia: 1527.2 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1276.6 µs, Skia: 1311.5 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1103.0 µs, Skia: 1454.0 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 993.7 µs, Skia: 2063.9 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 992.7 µs, Skia: 2058.2 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1027.3 µs, Skia: 1038.3 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1018.3 µs, Skia: 639.0 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1019.1 µs, Skia: 1007.8 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1162.7 µs, Skia: 1528.7 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 962.6 µs, Skia: 2618.6 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 986.4 µs, Skia: 1345.5 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 980.5 µs, Skia: 1736.2 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1002.3 µs, Skia: 1096.4 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1418.4 µs, Skia: 1711.9 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 998.2 µs, Skia: 1587.1 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 992.0 µs, Skia: 2037.9 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 981.4 µs, Skia: 1485.0 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1512.6 µs, Skia: 2573.1 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 955.3 µs, Skia: 1550.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1001.7 µs, Skia: 1548.4 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1004.8 µs, Skia: 1583.5 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 951.4 µs, Skia: 1014.7 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 908.5 µs, Skia: 1034.8 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1006.8 µs, Skia: 1013.9 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 986.7 µs, Skia: 1957.1 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 995.3 µs, Skia: 1028.0 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1128.9 µs, Skia: 1276.2 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 997.2 µs, Skia: 1742.8 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1039.0 µs, Skia: 1039.5 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1255.9 µs, Skia: 1127.5 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1047.1 µs, Skia: 998.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1178.7 µs, Skia: 996.9 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 997.4 µs, Skia: 1157.7 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1149.2 µs, Skia: 1081.5 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1058.1 µs, Skia: 1100.2 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 998.4 µs, Skia: 1045.4 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1023.9 µs, Skia: 1325.3 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1102.2 µs, Skia: 1358.2 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1207.0 µs, Skia: 1153.1 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1303.5 µs, Skia: 1114.6 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1202.5 µs, Skia: 1059.5 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1100.7 µs, Skia: 1230.7 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1070.7 µs, Skia: 1026.7 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1037.6 µs, Skia: 1026.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1085.5 µs, Skia: 1005.6 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1333.7 µs, Skia: 1154.3 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1583.4 µs, Skia: 1511.9 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1027.6 µs, Skia: 1823.8 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1061.6 µs, Skia: 999.1 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1002.8 µs, Skia: 992.2 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1038.7 µs, Skia: 1076.7 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1028.4 µs, Skia: 997.7 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 997.4 µs, Skia: 981.8 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1012.5 µs, Skia: 1090.3 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 999.5 µs, Skia: 1335.6 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1008.7 µs, Skia: 1007.8 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1027.4 µs, Skia: 1008.3 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 955.4 µs, Skia: 999.3 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 956.9 µs, Skia: 1000.2 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 982.8 µs, Skia: 993.8 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 997.5 µs, Skia: 997.2 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 998.1 µs, Skia: 998.7 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 954.8 µs, Skia: 983.9 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 925.0 µs, Skia: 988.5 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 998.3 µs, Skia: 711.3 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1090.7 µs, Skia: 1007.5 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1025.1 µs, Skia: 1811.9 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1037.9 µs, Skia: 656.0 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 992.1 µs, Skia: 586.3 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 989.1 µs, Skia: 995.9 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 998.0 µs, Skia: 997.6 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 866.8 µs, Skia: 914.9 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 975.5 µs, Skia: 1120.7 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 988.2 µs, Skia: 1137.2 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 982.6 µs, Skia: 991.9 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 998.9 µs, Skia: 996.5 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 937.9 µs, Skia: 919.0 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1003.8 µs, Skia: 1261.8 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 957.7 µs, Skia: 985.5 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1048.3 µs, Skia: 985.6 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 976.0 µs, Skia: 1209.2 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1186.7 µs, Skia: 1352.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1002.4 µs, Skia: 1511.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1614.7 µs, Skia: 1532.6 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 946.3 µs, Skia: 1061.2 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1007.7 µs, Skia: 1067.2 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 997.9 µs, Skia: 1016.3 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1283.1 µs, Skia: 1057.0 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.5 µs  820.6 µs    824.1 µs    9.9 µs  629.0 µs    638.9 µs       2.8x Nisaba          1.29x Skia
2. 100 Grid Lines (1px Hairlines)            16.5 µs  885.0 µs    901.5 µs   51.1 µs 1030.9 µs   1082.0 µs       3.1x Nisaba        1.20x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      27.4 µs  927.3 µs    954.7 µs  232.2 µs 1044.1 µs   1276.2 µs       8.5x Nisaba        1.34x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      29.6 µs  995.8 µs   1025.4 µs   71.5 µs  926.4 µs    997.9 µs       2.4x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   26.4 µs 1257.6 µs   1284.1 µs   74.1 µs 1155.1 µs   1229.2 µs       2.8x Nisaba         Tie (~1.0x)
6. 50 Simple Rounded Rects (12px Radius)     17.5 µs 1014.0 µs   1031.5 µs   60.2 µs  794.1 µs    854.3 µs       3.4x Nisaba          1.21x Skia
7. 50 Varying-Radius Rounded Rects           19.3 µs 1225.8 µs   1245.1 µs   48.0 µs  971.6 µs   1019.6 µs       2.5x Nisaba          1.22x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   22.0 µs 1005.5 µs   1027.5 µs   57.2 µs  940.9 µs    998.1 µs       2.6x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          22.7 µs  975.4 µs    998.0 µs   74.5 µs  924.6 µs    999.1 µs       3.3x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         40.1 µs  963.0 µs   1003.1 µs   41.2 µs  990.1 µs   1031.2 µs               Tie         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             18.6 µs 1473.3 µs   1491.9 µs  193.0 µs 1261.7 µs   1454.7 µs      10.4x Nisaba         Tie (~1.0x)
12. 50 Diagonal Gradients (45° Angle)       22.4 µs 1507.6 µs   1530.0 µs  199.3 µs 1175.9 µs   1375.2 µs       8.9x Nisaba          1.11x Skia
13. 25 Radial Glow Gradients (300px)         16.4 µs 1974.5 µs   1990.8 µs  112.2 µs 1415.0 µs   1527.2 µs       6.9x Nisaba          1.30x Skia
14. 20 Box Blur / Drop Shadows (15px)        12.8 µs 1263.8 µs   1276.6 µs  102.2 µs 1209.3 µs   1311.5 µs       8.0x Nisaba         Tie (~1.0x)
15. 10 Concave 10-Point Stars (Fill)         42.7 µs 1060.3 µs   1103.0 µs  371.0 µs 1083.0 µs   1454.0 µs       8.7x Nisaba        1.32x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   29.1 µs  964.6 µs    993.7 µs  881.5 µs 1182.4 µs   2063.9 µs      30.3x Nisaba        2.08x Nisaba
17. 1000-Point Waveform (Dense Stroke)       84.1 µs  908.6 µs    992.7 µs  821.9 µs 1236.3 µs   2058.2 µs       9.8x Nisaba        2.07x Nisaba
18. 50 Rotated Cards (Affine Transforms)     18.0 µs 1009.3 µs   1027.3 µs   67.7 µs  970.7 µs   1038.3 µs       3.8x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             13.3 µs 1005.0 µs   1018.3 µs   62.2 µs  576.7 µs    639.0 µs       4.7x Nisaba          1.59x Skia
20. 50 UI Text Labels (Inter-Regular)        99.3 µs  919.8 µs   1019.1 µs  101.7 µs  906.1 µs   1007.8 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  104.3 µs 1058.4 µs   1162.7 µs  361.0 µs 1167.7 µs   1528.7 µs       3.5x Nisaba        1.31x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     40.8 µs  921.7 µs    962.6 µs 1253.0 µs 1365.6 µs   2618.6 µs      30.7x Nisaba        2.72x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       15.6 µs  970.8 µs    986.4 µs  182.1 µs 1163.4 µs   1345.5 µs      11.7x Nisaba        1.36x Nisaba
24. Circular Arcs with arcTo (20 Corners)    32.7 µs  947.8 µs    980.5 µs  516.4 µs 1219.8 µs   1736.2 µs      15.8x Nisaba        1.77x Nisaba
25. Directional Arcs with arc() (30 Arcs)    58.9 µs  943.4 µs   1002.3 µs  166.7 µs  929.7 µs   1096.4 µs       2.8x Nisaba        1.09x Nisaba
26. Compound Path with Holes (15 Donuts)     91.7 µs 1326.8 µs   1418.4 µs  528.6 µs 1183.3 µs   1711.9 µs       5.8x Nisaba        1.21x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    40.0 µs  958.2 µs    998.2 µs  481.8 µs 1105.3 µs   1587.1 µs      12.1x Nisaba        1.59x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    48.2 µs  943.8 µs    992.0 µs  804.7 µs 1233.2 µs   2037.9 µs      16.7x Nisaba        2.05x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)    9.6 µs  971.7 µs    981.4 µs  284.9 µs 1200.1 µs   1485.0 µs      29.5x Nisaba        1.51x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      26.4 µs 1486.2 µs   1512.6 µs 1195.3 µs 1377.8 µs   2573.1 µs      45.2x Nisaba        1.70x Nisaba
31. Stroke Join: Miter (12px Stroke)         11.6 µs  943.7 µs    955.3 µs  391.6 µs 1158.6 µs   1550.1 µs      33.9x Nisaba        1.62x Nisaba
32. Stroke Join: Bevel (12px Stroke)         10.5 µs  991.1 µs   1001.7 µs  394.3 µs 1154.1 µs   1548.4 µs      37.5x Nisaba        1.55x Nisaba
33. Stroke Join: Round (12px Stroke)         13.3 µs  991.5 µs   1004.8 µs  397.0 µs 1186.5 µs   1583.5 µs      29.9x Nisaba        1.58x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      15.7 µs  935.7 µs    951.4 µs   44.6 µs  970.1 µs   1014.7 µs       2.8x Nisaba        1.07x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    14.2 µs  894.3 µs    908.5 µs   43.5 µs  991.3 µs   1034.8 µs       3.1x Nisaba        1.14x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     36.7 µs  970.0 µs   1006.8 µs   61.7 µs  952.2 µs   1013.9 µs       1.7x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   16.7 µs  970.0 µs    986.7 µs  693.5 µs 1263.7 µs   1957.1 µs      41.5x Nisaba        1.98x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   18.6 µs  976.7 µs    995.3 µs   70.8 µs  957.2 µs   1028.0 µs       3.8x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   11.5 µs 1117.4 µs   1128.9 µs  157.8 µs 1118.4 µs   1276.2 µs      13.8x Nisaba        1.13x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   29.5 µs  967.7 µs    997.2 µs  564.0 µs 1178.8 µs   1742.8 µs      19.1x Nisaba        1.75x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          15.3 µs 1023.7 µs   1039.0 µs   40.6 µs  998.9 µs   1039.5 µs       2.7x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       16.4 µs 1239.5 µs   1255.9 µs   44.8 µs 1082.7 µs   1127.5 µs       2.7x Nisaba          1.11x Skia
43. 50 Pill / Capsule Badges (r=h/2)         19.5 µs 1027.7 µs   1047.1 µs   56.9 µs  941.8 µs    998.6 µs       2.9x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     17.1 µs 1161.6 µs   1178.7 µs   47.7 µs  949.2 µs    996.9 µs       2.8x Nisaba          1.18x Skia
45. High-Density Disks Cloud (500 Disks)     62.0 µs  935.4 µs    997.4 µs  208.1 µs  949.6 µs   1157.7 µs       3.4x Nisaba        1.16x Nisaba
46. High-Density Stroked Rings (250 Rings)   35.7 µs 1113.5 µs   1149.2 µs  137.1 µs  944.5 µs   1081.5 µs       3.8x Nisaba          1.06x Skia
47. 250 Filled Rectangles Batch              37.2 µs 1021.0 µs   1058.1 µs  132.9 µs  967.3 µs   1100.2 µs       3.6x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      39.1 µs  959.3 µs    998.4 µs   82.9 µs  962.5 µs   1045.4 µs       2.1x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                92.0 µs  931.9 µs   1023.9 µs  292.4 µs 1032.8 µs   1325.3 µs       3.2x Nisaba        1.29x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      86.7 µs 1015.5 µs   1102.2 µs  307.6 µs 1050.6 µs   1358.2 µs       3.5x Nisaba        1.23x Nisaba
51. 30 Vertical Card Linear Gradients        15.0 µs 1192.1 µs   1207.0 µs  156.4 µs  996.7 µs   1153.1 µs      10.4x Nisaba         Tie (~1.0x)
52. 30 Horizontal Bar Progress Gradients     15.4 µs 1288.1 µs   1303.5 µs  143.6 µs  971.0 µs   1114.6 µs       9.3x Nisaba          1.17x Skia
53. 15 Concentric Radial Spotlights          12.0 µs 1190.5 µs   1202.5 µs   92.1 µs  967.4 µs   1059.5 µs       7.7x Nisaba          1.13x Skia
54. 25 Soft Ambient Card Shadows             15.5 µs 1085.2 µs   1100.7 µs  123.7 µs 1107.0 µs   1230.7 µs       8.0x Nisaba        1.12x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      12.3 µs 1058.3 µs   1070.7 µs   85.9 µs  940.8 µs   1026.7 µs       7.0x Nisaba         Tie (~1.0x)
56. 16 Multi-Angle Gradient Fan Slices       25.9 µs 1011.7 µs   1037.6 µs  106.7 µs  919.9 µs   1026.7 µs       4.1x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     7.7 µs 1077.8 µs   1085.5 µs   31.0 µs  974.6 µs   1005.6 µs       4.0x Nisaba          1.08x Skia
58. 20 Inset Well Shadows (Recessed)         62.0 µs 1271.7 µs   1333.7 µs  104.9 µs 1049.4 µs   1154.3 µs       1.7x Nisaba          1.16x Skia
59. 10 Pulsing Circular Radar Wave Glows     14.4 µs 1569.0 µs   1583.4 µs   69.2 µs 1442.7 µs   1511.9 µs       4.8x Nisaba         Tie (~1.0x)
60. Waveform Polyline Stroked with Gradient   49.7 µs  977.9 µs   1027.6 µs  581.9 µs 1242.0 µs   1823.8 µs      11.7x Nisaba        1.77x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)    9.1 µs 1052.4 µs   1061.6 µs   32.5 µs  966.6 µs    999.1 µs       3.6x Nisaba          1.06x Skia
62. Scaled Image Quad (2x 512x512 Blit)       8.2 µs  994.6 µs   1002.8 µs   29.0 µs  963.2 µs    992.2 µs       3.5x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle       9.0 µs 1029.7 µs   1038.7 µs   38.4 µs 1038.3 µs   1076.7 µs       4.3x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)    9.5 µs 1018.9 µs   1028.4 µs   34.2 µs  963.5 µs    997.7 µs       3.6x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           11.8 µs  985.6 µs    997.4 µs   44.0 µs  937.8 µs    981.8 µs       3.7x Nisaba         Tie (~1.0x)
66. Texture Pattern on Star Path             18.8 µs  993.7 µs   1012.5 µs  143.0 µs  947.3 µs   1090.3 µs       7.6x Nisaba        1.08x Nisaba
67. 20px Curved Path Stroked with Texture    14.4 µs  985.1 µs    999.5 µs  175.8 µs 1159.8 µs   1335.6 µs      12.2x Nisaba        1.34x Nisaba
68. Minified Texture Quad (0.25x Downscale)   17.2 µs  991.5 µs   1008.7 µs   71.7 µs  936.1 µs   1007.8 µs       4.2x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        23.2 µs 1004.2 µs   1027.4 µs   63.9 µs  944.4 µs   1008.3 µs       2.8x Nisaba         Tie (~1.0x)
70. Pure Rotation Cluster (36 Spokes 10°)   18.8 µs  936.6 µs    955.4 µs   55.0 µs  944.3 µs    999.3 µs       2.9x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   27.9 µs  928.9 µs    956.9 µs   39.3 µs  960.9 µs   1000.2 µs       1.4x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   18.5 µs  964.3 µs    982.8 µs   52.1 µs  941.6 µs    993.8 µs       2.8x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   17.8 µs  979.8 µs    997.5 µs   46.3 µs  950.9 µs    997.2 µs       2.6x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   15.1 µs  983.0 µs    998.1 µs   41.5 µs  957.2 µs    998.7 µs       2.7x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       19.3 µs  935.4 µs    954.8 µs   58.2 µs  925.8 µs    983.9 µs       3.0x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   16.0 µs  908.9 µs    925.0 µs   41.7 µs  946.8 µs    988.5 µs       2.6x Nisaba        1.07x Nisaba
77. Multi-Scissor Grid (16 Viewports)        14.5 µs  983.8 µs    998.3 µs   60.5 µs  650.8 µs    711.3 µs       4.2x Nisaba          1.40x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   15.7 µs 1075.0 µs   1090.7 µs   41.7 µs  965.8 µs   1007.5 µs       2.7x Nisaba          1.08x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   96.0 µs  929.1 µs   1025.1 µs  911.8 µs  900.1 µs   1811.9 µs       9.5x Nisaba        1.77x Nisaba
80. Scissored Gradient Card (Clipped)        11.2 µs 1026.7 µs   1037.9 µs   82.4 µs  573.6 µs    656.0 µs       7.4x Nisaba          1.58x Skia
81. Circular Overflow Clip (25 Boxes)        14.6 µs  977.5 µs    992.1 µs   71.4 µs  514.9 µs    586.3 µs       4.9x Nisaba          1.69x Skia
82. Scissor Invalidation / Reset (25 Cycles)   14.6 µs  974.4 µs    989.1 µs   49.4 µs  946.5 µs    995.9 µs       3.4x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    14.1 µs  983.9 µs    998.0 µs   41.3 µs  956.2 µs    997.6 µs       2.9x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   12.8 µs  854.0 µs    866.8 µs   42.8 µs  872.0 µs    914.9 µs       3.4x Nisaba        1.06x Nisaba
85. Composite Op: Source-In (Alpha Masking)   16.2 µs  959.3 µs    975.5 µs   43.1 µs 1077.6 µs   1120.7 µs       2.7x Nisaba        1.15x Nisaba
86. Composite Op: Source-Out (Cutout)        15.7 µs  972.5 µs    988.2 µs   39.4 µs 1097.8 µs   1137.2 µs       2.5x Nisaba        1.15x Nisaba
87. Composite Op: Atop (Target Bounds)       14.0 µs  968.7 µs    982.6 µs   42.5 µs  949.3 µs    991.9 µs       3.0x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   13.5 µs  985.3 µs    998.9 µs   43.0 µs  953.5 µs    996.5 µs       3.2x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     13.3 µs  924.6 µs    937.9 µs   45.1 µs  873.9 µs    919.0 µs       3.4x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        12.5 µs  991.3 µs   1003.8 µs   46.1 µs 1215.7 µs   1261.8 µs       3.7x Nisaba        1.26x Nisaba
91. Composite Op: Xor (Exclusive Blend)      16.6 µs  941.0 µs    957.7 µs   44.7 µs  940.8 µs    985.5 µs       2.7x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    15.6 µs 1032.7 µs   1048.3 µs   43.6 µs  942.0 µs    985.6 µs       2.8x Nisaba          1.06x Skia
93. UI Dashboard Gauge (Arc Track + Needle)   25.0 µs  951.0 µs    976.0 µs  201.1 µs 1008.1 µs   1209.2 µs       8.1x Nisaba        1.24x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      34.2 µs 1152.4 µs   1186.7 µs  277.3 µs 1075.5 µs   1352.9 µs       8.1x Nisaba        1.14x Nisaba
95. Circular Progress Rings (8 Meters)       43.1 µs  959.3 µs   1002.4 µs  385.4 µs 1126.0 µs   1511.4 µs       8.9x Nisaba        1.51x Nisaba
96. Modern Card Stack (5 Elevated Cards)     16.4 µs 1598.3 µs   1614.7 µs   82.6 µs 1449.9 µs   1532.6 µs       5.1x Nisaba          1.05x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   22.2 µs  924.2 µs    946.3 µs   41.3 µs 1019.9 µs   1061.2 µs       1.9x Nisaba        1.12x Nisaba
98. Floating Action Button (FAB 10 Buttons)   36.3 µs  971.4 µs   1007.7 µs  124.4 µs  942.8 µs   1067.2 µs       3.4x Nisaba        1.06x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   30.6 µs  967.3 µs    997.9 µs  113.3 µs  903.0 µs   1016.3 µs       3.7x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   20.6 µs 1262.5 µs   1283.1 µs   71.6 µs  985.4 µs   1057.0 µs       3.5x Nisaba          1.21x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2673.1 µs104654.0 µs 107327.2 µs18984.5 µs102217.4 µs 121201.8 µs      7.10x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 41 suites | Skia won in 21 suites | Ties: 38
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.10x Nisaba
===================================================================================================================================================
```
</details>

### Run 2
<details>
<summary>Click to expand Run 2 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 877.1 µs, Skia: 880.5 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 909.0 µs, Skia: 1008.9 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1005.8 µs, Skia: 1327.5 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1068.6 µs, Skia: 1104.9 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1250.0 µs, Skia: 1258.6 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1249.5 µs, Skia: 1048.7 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1271.9 µs, Skia: 1026.6 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1028.5 µs, Skia: 1017.8 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 998.2 µs, Skia: 1033.5 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1002.5 µs, Skia: 1057.6 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1479.5 µs, Skia: 1436.6 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1517.3 µs, Skia: 1362.6 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 1955.9 µs, Skia: 1557.7 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1201.0 µs, Skia: 1301.9 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1153.3 µs, Skia: 1584.8 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1005.6 µs, Skia: 2010.2 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1007.5 µs, Skia: 2059.6 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1195.5 µs, Skia: 1116.7 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1037.1 µs, Skia: 641.4 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1017.5 µs, Skia: 1007.4 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1132.2 µs, Skia: 1513.9 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1012.2 µs, Skia: 2650.1 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 997.8 µs, Skia: 1315.2 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 998.7 µs, Skia: 1727.5 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1007.1 µs, Skia: 1088.0 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1345.8 µs, Skia: 1812.1 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 991.1 µs, Skia: 1602.7 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 958.0 µs, Skia: 2038.2 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 940.4 µs, Skia: 1449.4 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1361.6 µs, Skia: 2526.0 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 965.9 µs, Skia: 1522.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 966.1 µs, Skia: 1559.1 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1004.6 µs, Skia: 1579.6 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 996.1 µs, Skia: 1088.2 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 811.9 µs, Skia: 1025.5 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1007.2 µs, Skia: 1028.2 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1021.0 µs, Skia: 2018.3 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.2 µs, Skia: 1027.7 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 996.5 µs, Skia: 1250.1 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1002.0 µs, Skia: 1735.3 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 998.0 µs, Skia: 1008.5 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1116.2 µs, Skia: 1108.7 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1067.4 µs, Skia: 979.1 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1068.7 µs, Skia: 997.9 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1017.9 µs, Skia: 1217.3 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1086.3 µs, Skia: 1030.6 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1048.8 µs, Skia: 1047.7 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 986.9 µs, Skia: 1018.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1028.4 µs, Skia: 1332.2 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1103.5 µs, Skia: 1517.2 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1436.0 µs, Skia: 1127.7 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1261.9 µs, Skia: 1125.9 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1054.5 µs, Skia: 1027.7 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1315.7 µs, Skia: 1215.3 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1128.1 µs, Skia: 1036.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1007.9 µs, Skia: 1007.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1177.9 µs, Skia: 997.9 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1307.6 µs, Skia: 1187.0 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1553.6 µs, Skia: 1492.9 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 989.3 µs, Skia: 1794.3 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1088.0 µs, Skia: 1007.8 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1042.0 µs, Skia: 903.6 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1028.4 µs, Skia: 986.7 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1019.0 µs, Skia: 991.4 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1036.6 µs, Skia: 973.8 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1028.8 µs, Skia: 1101.5 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1005.1 µs, Skia: 1315.5 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1007.4 µs, Skia: 957.3 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1108.2 µs, Skia: 1051.2 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 998.5 µs, Skia: 1018.4 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1008.1 µs, Skia: 997.8 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 998.0 µs, Skia: 1008.7 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 997.5 µs, Skia: 998.3 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 1007.1 µs, Skia: 1027.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 998.3 µs, Skia: 997.3 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 956.4 µs, Skia: 911.9 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 990.7 µs, Skia: 671.3 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1150.4 µs, Skia: 1065.8 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1021.6 µs, Skia: 1804.7 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1047.5 µs, Skia: 656.5 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 914.1 µs, Skia: 569.6 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 997.4 µs, Skia: 997.2 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1028.9 µs, Skia: 1008.4 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1008.1 µs, Skia: 993.3 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 978.4 µs, Skia: 1127.6 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 992.8 µs, Skia: 1120.7 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 997.8 µs, Skia: 936.8 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 958.8 µs, Skia: 988.1 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 999.2 µs, Skia: 998.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 978.3 µs, Skia: 1237.9 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 991.2 µs, Skia: 995.6 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 967.1 µs, Skia: 991.4 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 986.1 µs, Skia: 1240.1 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1186.9 µs, Skia: 1371.1 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1011.8 µs, Skia: 1497.0 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1598.1 µs, Skia: 1616.2 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 950.1 µs, Skia: 1076.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1017.2 µs, Skia: 1089.0 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 997.3 µs, Skia: 1058.0 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1455.0 µs, Skia: 1211.3 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          4.7 µs  872.5 µs    877.1 µs   13.1 µs  867.4 µs    880.5 µs       2.8x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            17.1 µs  892.0 µs    909.0 µs   42.5 µs  966.3 µs   1008.9 µs       2.5x Nisaba        1.11x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      26.4 µs  979.5 µs   1005.8 µs  248.1 µs 1079.4 µs   1327.5 µs       9.4x Nisaba        1.32x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      30.4 µs 1038.2 µs   1068.6 µs   76.9 µs 1027.9 µs   1104.9 µs       2.5x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   24.6 µs 1225.4 µs   1250.0 µs   70.0 µs 1188.7 µs   1258.6 µs       2.8x Nisaba         Tie (~1.0x)
6. 50 Simple Rounded Rects (12px Radius)     17.0 µs 1232.6 µs   1249.5 µs   58.1 µs  990.5 µs   1048.7 µs       3.4x Nisaba          1.19x Skia
7. 50 Varying-Radius Rounded Rects           16.8 µs 1255.0 µs   1271.9 µs   45.2 µs  981.4 µs   1026.6 µs       2.7x Nisaba          1.24x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   23.2 µs 1005.3 µs   1028.5 µs   58.2 µs  959.6 µs   1017.8 µs       2.5x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          21.1 µs  977.1 µs    998.2 µs   69.6 µs  964.0 µs   1033.5 µs       3.3x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         41.2 µs  961.3 µs   1002.5 µs   40.2 µs 1017.3 µs   1057.6 µs               Tie        1.05x Nisaba
11. 50 Two-Stop Linear Gradients             21.5 µs 1458.0 µs   1479.5 µs  194.0 µs 1242.5 µs   1436.6 µs       9.0x Nisaba         Tie (~1.0x)
12. 50 Diagonal Gradients (45° Angle)       21.0 µs 1496.2 µs   1517.3 µs  193.7 µs 1168.9 µs   1362.6 µs       9.2x Nisaba          1.11x Skia
13. 25 Radial Glow Gradients (300px)         14.7 µs 1941.2 µs   1955.9 µs  110.8 µs 1446.9 µs   1557.7 µs       7.5x Nisaba          1.26x Skia
14. 20 Box Blur / Drop Shadows (15px)        10.7 µs 1190.3 µs   1201.0 µs   97.4 µs 1204.5 µs   1301.9 µs       9.1x Nisaba        1.08x Nisaba
15. 10 Concave 10-Point Stars (Fill)         47.8 µs 1105.4 µs   1153.3 µs  404.2 µs 1180.5 µs   1584.8 µs       8.5x Nisaba        1.37x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   34.7 µs  970.8 µs   1005.6 µs  876.3 µs 1133.8 µs   2010.2 µs      25.2x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       93.6 µs  913.9 µs   1007.5 µs  831.1 µs 1228.5 µs   2059.6 µs       8.9x Nisaba        2.04x Nisaba
18. 50 Rotated Cards (Affine Transforms)     20.2 µs 1175.3 µs   1195.5 µs   69.8 µs 1046.9 µs   1116.7 µs       3.5x Nisaba          1.07x Skia
19. 25 Scissor Clipped Viewports             13.8 µs 1023.3 µs   1037.1 µs   67.9 µs  573.5 µs    641.4 µs       4.9x Nisaba          1.62x Skia
20. 50 UI Text Labels (Inter-Regular)       102.6 µs  914.9 µs   1017.5 µs  102.3 µs  905.1 µs   1007.4 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  106.2 µs 1026.0 µs   1132.2 µs  370.3 µs 1143.6 µs   1513.9 µs       3.5x Nisaba        1.34x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     40.3 µs  971.9 µs   1012.2 µs 1237.7 µs 1412.4 µs   2650.1 µs      30.7x Nisaba        2.62x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       14.7 µs  983.1 µs    997.8 µs  190.9 µs 1124.3 µs   1315.2 µs      13.0x Nisaba        1.32x Nisaba
24. Circular Arcs with arcTo (20 Corners)    30.9 µs  967.8 µs    998.7 µs  522.1 µs 1205.4 µs   1727.5 µs      16.9x Nisaba        1.73x Nisaba
25. Directional Arcs with arc() (30 Arcs)    58.9 µs  948.2 µs   1007.1 µs  150.8 µs  937.2 µs   1088.0 µs       2.6x Nisaba        1.08x Nisaba
26. Compound Path with Holes (15 Donuts)     86.6 µs 1259.2 µs   1345.8 µs  537.1 µs 1275.0 µs   1812.1 µs       6.2x Nisaba        1.35x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    39.3 µs  951.8 µs    991.1 µs  482.0 µs 1120.8 µs   1602.7 µs      12.3x Nisaba        1.62x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    47.1 µs  910.9 µs    958.0 µs  804.3 µs 1233.8 µs   2038.2 µs      17.1x Nisaba        2.13x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   10.8 µs  929.6 µs    940.4 µs  275.5 µs 1173.9 µs   1449.4 µs      25.5x Nisaba        1.54x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      27.0 µs 1334.6 µs   1361.6 µs 1196.9 µs 1329.1 µs   2526.0 µs      44.3x Nisaba        1.86x Nisaba
31. Stroke Join: Miter (12px Stroke)         13.8 µs  952.1 µs    965.9 µs  392.7 µs 1129.4 µs   1522.1 µs      28.5x Nisaba        1.58x Nisaba
32. Stroke Join: Bevel (12px Stroke)         13.3 µs  952.8 µs    966.1 µs  389.8 µs 1169.3 µs   1559.1 µs      29.3x Nisaba        1.61x Nisaba
33. Stroke Join: Round (12px Stroke)         17.3 µs  987.3 µs   1004.6 µs  399.7 µs 1180.0 µs   1579.6 µs      23.1x Nisaba        1.57x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      14.0 µs  982.0 µs    996.1 µs   46.8 µs 1041.4 µs   1088.2 µs       3.3x Nisaba        1.09x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    14.7 µs  797.1 µs    811.9 µs   48.2 µs  977.3 µs   1025.5 µs       3.3x Nisaba        1.26x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     39.2 µs  968.0 µs   1007.2 µs   64.2 µs  964.0 µs   1028.2 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   15.9 µs 1005.1 µs   1021.0 µs  707.4 µs 1310.8 µs   2018.3 µs      44.6x Nisaba        1.98x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   19.7 µs  977.5 µs    997.2 µs   71.1 µs  956.6 µs   1027.7 µs       3.6x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   12.1 µs  984.4 µs    996.5 µs  159.0 µs 1091.2 µs   1250.1 µs      13.1x Nisaba        1.25x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   30.3 µs  971.7 µs   1002.0 µs  557.5 µs 1177.8 µs   1735.3 µs      18.4x Nisaba        1.73x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          15.3 µs  982.7 µs    998.0 µs   40.7 µs  967.8 µs   1008.5 µs       2.7x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       14.2 µs 1102.0 µs   1116.2 µs   42.1 µs 1066.5 µs   1108.7 µs       3.0x Nisaba         Tie (~1.0x)
43. 50 Pill / Capsule Badges (r=h/2)         25.4 µs 1042.1 µs   1067.4 µs   62.8 µs  916.3 µs    979.1 µs       2.5x Nisaba          1.09x Skia
44. 50 Asymmetric Teardrop Rounded Rects     16.9 µs 1051.8 µs   1068.7 µs   46.6 µs  951.3 µs    997.9 µs       2.8x Nisaba          1.07x Skia
45. High-Density Disks Cloud (500 Disks)     62.7 µs  955.2 µs   1017.9 µs  224.6 µs  992.7 µs   1217.3 µs       3.6x Nisaba        1.20x Nisaba
46. High-Density Stroked Rings (250 Rings)   37.8 µs 1048.5 µs   1086.3 µs  134.7 µs  895.9 µs   1030.6 µs       3.6x Nisaba          1.05x Skia
47. 250 Filled Rectangles Batch              37.1 µs 1011.7 µs   1048.8 µs  135.5 µs  912.2 µs   1047.7 µs       3.7x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      36.9 µs  950.0 µs    986.9 µs   83.1 µs  935.2 µs   1018.3 µs       2.3x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                84.6 µs  943.8 µs   1028.4 µs  306.3 µs 1025.9 µs   1332.2 µs       3.6x Nisaba        1.30x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      81.5 µs 1022.1 µs   1103.5 µs  314.1 µs 1203.1 µs   1517.2 µs       3.9x Nisaba        1.37x Nisaba
51. 30 Vertical Card Linear Gradients        15.0 µs 1421.0 µs   1436.0 µs  147.1 µs  980.6 µs   1127.7 µs       9.8x Nisaba          1.27x Skia
52. 30 Horizontal Bar Progress Gradients     15.2 µs 1246.7 µs   1261.9 µs  150.2 µs  975.7 µs   1125.9 µs       9.9x Nisaba          1.12x Skia
53. 15 Concentric Radial Spotlights          11.7 µs 1042.8 µs   1054.5 µs   88.3 µs  939.4 µs   1027.7 µs       7.6x Nisaba         Tie (~1.0x)
54. 25 Soft Ambient Card Shadows             16.0 µs 1299.8 µs   1315.7 µs  126.6 µs 1088.8 µs   1215.3 µs       7.9x Nisaba          1.08x Skia
55. 15 Neon Button Glows (Cyan Intense)      12.2 µs 1115.9 µs   1128.1 µs   83.7 µs  952.7 µs   1036.5 µs       6.8x Nisaba          1.09x Skia
56. 16 Multi-Angle Gradient Fan Slices       29.4 µs  978.4 µs   1007.9 µs  106.2 µs  901.5 µs   1007.7 µs       3.6x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    10.2 µs 1167.7 µs   1177.9 µs   32.1 µs  965.8 µs    997.9 µs       3.1x Nisaba          1.18x Skia
58. 20 Inset Well Shadows (Recessed)         50.0 µs 1257.6 µs   1307.6 µs  109.9 µs 1077.1 µs   1187.0 µs       2.2x Nisaba          1.10x Skia
59. 10 Pulsing Circular Radar Wave Glows     13.1 µs 1540.5 µs   1553.6 µs   64.9 µs 1428.0 µs   1492.9 µs       5.0x Nisaba         Tie (~1.0x)
60. Waveform Polyline Stroked with Gradient   48.5 µs  940.9 µs    989.3 µs  563.4 µs 1230.9 µs   1794.3 µs      11.6x Nisaba        1.81x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)    9.5 µs 1078.4 µs   1088.0 µs   32.5 µs  975.3 µs   1007.8 µs       3.4x Nisaba          1.08x Skia
62. Scaled Image Quad (2x 512x512 Blit)      11.6 µs 1030.4 µs   1042.0 µs   33.2 µs  870.4 µs    903.6 µs       2.9x Nisaba          1.15x Skia
63. 45° Rotated Pattern Fill in Circle       8.7 µs 1019.7 µs   1028.4 µs   38.9 µs  947.8 µs    986.7 µs       4.5x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)    8.6 µs 1010.4 µs   1019.0 µs   37.3 µs  954.1 µs    991.4 µs       4.4x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           11.4 µs 1025.2 µs   1036.6 µs   42.3 µs  931.5 µs    973.8 µs       3.7x Nisaba          1.06x Skia
66. Texture Pattern on Star Path             20.2 µs 1008.6 µs   1028.8 µs  137.6 µs  963.8 µs   1101.5 µs       6.8x Nisaba        1.07x Nisaba
67. 20px Curved Path Stroked with Texture    14.5 µs  990.6 µs   1005.1 µs  173.1 µs 1142.4 µs   1315.5 µs      12.0x Nisaba        1.31x Nisaba
68. Minified Texture Quad (0.25x Downscale)   17.2 µs  990.3 µs   1007.4 µs   71.9 µs  885.4 µs    957.3 µs       4.2x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        26.0 µs 1082.2 µs   1108.2 µs   62.4 µs  988.7 µs   1051.2 µs       2.4x Nisaba          1.05x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   16.3 µs  982.3 µs    998.5 µs   51.6 µs  966.8 µs   1018.4 µs       3.2x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   23.1 µs  985.0 µs   1008.1 µs   35.5 µs  962.3 µs    997.8 µs       1.5x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   17.0 µs  981.0 µs    998.0 µs   48.5 µs  960.2 µs   1008.7 µs       2.9x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   16.1 µs  981.4 µs    997.5 µs   47.0 µs  951.3 µs    998.3 µs       2.9x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   13.7 µs  993.4 µs   1007.1 µs   41.5 µs  986.4 µs   1027.8 µs       3.0x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       23.1 µs  975.2 µs    998.3 µs   55.6 µs  941.7 µs    997.3 µs       2.4x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   17.2 µs  939.3 µs    956.4 µs   42.7 µs  869.2 µs    911.9 µs       2.5x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        15.1 µs  975.6 µs    990.7 µs   54.7 µs  616.5 µs    671.3 µs       3.6x Nisaba          1.48x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   14.7 µs 1135.7 µs   1150.4 µs   40.7 µs 1025.2 µs   1065.8 µs       2.8x Nisaba          1.08x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   96.1 µs  925.5 µs   1021.6 µs  863.1 µs  941.6 µs   1804.7 µs       9.0x Nisaba        1.77x Nisaba
80. Scissored Gradient Card (Clipped)        13.0 µs 1034.5 µs   1047.5 µs   85.7 µs  570.8 µs    656.5 µs       6.6x Nisaba          1.60x Skia
81. Circular Overflow Clip (25 Boxes)        14.6 µs  899.5 µs    914.1 µs   72.1 µs  497.6 µs    569.6 µs       4.9x Nisaba          1.60x Skia
82. Scissor Invalidation / Reset (25 Cycles)   15.6 µs  981.9 µs    997.4 µs   46.4 µs  950.8 µs    997.2 µs       3.0x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    14.6 µs 1014.3 µs   1028.9 µs   44.2 µs  964.2 µs   1008.4 µs       3.0x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   12.8 µs  995.2 µs   1008.1 µs   46.5 µs  946.9 µs    993.3 µs       3.6x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   15.3 µs  963.2 µs    978.4 µs   45.6 µs 1082.0 µs   1127.6 µs       3.0x Nisaba        1.15x Nisaba
86. Composite Op: Source-Out (Cutout)        15.1 µs  977.7 µs    992.8 µs   42.4 µs 1078.4 µs   1120.7 µs       2.8x Nisaba        1.13x Nisaba
87. Composite Op: Atop (Target Bounds)       11.7 µs  986.1 µs    997.8 µs   46.5 µs  890.3 µs    936.8 µs       4.0x Nisaba          1.07x Skia
88. Composite Op: Dest-Over (Under-Drawing)   16.0 µs  942.7 µs    958.8 µs   42.5 µs  945.6 µs    988.1 µs       2.6x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     14.4 µs  984.7 µs    999.2 µs   41.6 µs  956.7 µs    998.3 µs       2.9x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        14.2 µs  964.2 µs    978.3 µs   45.2 µs 1192.7 µs   1237.9 µs       3.2x Nisaba        1.27x Nisaba
91. Composite Op: Xor (Exclusive Blend)      11.7 µs  979.5 µs    991.2 µs   44.1 µs  951.5 µs    995.6 µs       3.8x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    13.7 µs  953.4 µs    967.1 µs   44.8 µs  946.6 µs    991.4 µs       3.3x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   25.5 µs  960.6 µs    986.1 µs  201.2 µs 1039.0 µs   1240.1 µs       7.9x Nisaba        1.26x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      30.9 µs 1156.0 µs   1186.9 µs  277.8 µs 1093.3 µs   1371.1 µs       9.0x Nisaba        1.16x Nisaba
95. Circular Progress Rings (8 Meters)       44.1 µs  967.7 µs   1011.8 µs  384.3 µs 1112.7 µs   1497.0 µs       8.7x Nisaba        1.48x Nisaba
96. Modern Card Stack (5 Elevated Cards)     17.4 µs 1580.8 µs   1598.1 µs   84.0 µs 1532.2 µs   1616.2 µs       4.8x Nisaba         Tie (~1.0x)
97. CAD Cross-Hatch Pattern (80 Angled Lines)   21.8 µs  928.3 µs    950.1 µs   41.6 µs 1035.1 µs   1076.7 µs       1.9x Nisaba        1.13x Nisaba
98. Floating Action Button (FAB 10 Buttons)   34.9 µs  982.3 µs   1017.2 µs  127.3 µs  961.8 µs   1089.0 µs       3.7x Nisaba        1.07x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   30.0 µs  967.2 µs    997.3 µs  117.9 µs  940.0 µs   1058.0 µs       3.9x Nisaba        1.06x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   23.4 µs 1431.5 µs   1455.0 µs   75.4 µs 1135.9 µs   1211.3 µs       3.2x Nisaba          1.20x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2679.7 µs105372.0 µs 108051.7 µs19003.3 µs103239.5 µs 122242.8 µs      7.09x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 41 suites | Skia won in 25 suites | Ties: 34
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.09x Nisaba
===================================================================================================================================================
```
</details>

### Run 3
<details>
<summary>Click to expand Run 3 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 959.5 µs, Skia: 943.2 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1015.9 µs, Skia: 1151.8 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 995.6 µs, Skia: 1337.5 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1026.9 µs, Skia: 1029.4 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1453.0 µs, Skia: 1269.7 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1252.9 µs, Skia: 1089.4 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1178.8 µs, Skia: 1043.9 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1039.3 µs, Skia: 1008.4 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 988.3 µs, Skia: 1027.1 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1031.2 µs, Skia: 1117.1 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1491.2 µs, Skia: 1364.3 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1436.0 µs, Skia: 1376.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2022.9 µs, Skia: 1582.5 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1230.0 µs, Skia: 1301.8 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1176.1 µs, Skia: 1486.2 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 990.2 µs, Skia: 2017.6 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1025.0 µs, Skia: 2095.0 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1168.7 µs, Skia: 1130.6 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 981.7 µs, Skia: 642.2 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1019.2 µs, Skia: 1038.8 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1151.9 µs, Skia: 1525.0 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1009.3 µs, Skia: 2668.3 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 982.3 µs, Skia: 1341.0 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1000.9 µs, Skia: 1828.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1006.0 µs, Skia: 1086.5 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1408.1 µs, Skia: 1667.7 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 994.8 µs, Skia: 1623.1 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 986.3 µs, Skia: 2047.4 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 993.6 µs, Skia: 1468.2 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1399.3 µs, Skia: 2522.5 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 931.7 µs, Skia: 1554.2 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 966.9 µs, Skia: 1549.5 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 944.1 µs, Skia: 1558.2 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 916.3 µs, Skia: 1009.6 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 941.6 µs, Skia: 1123.9 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1007.8 µs, Skia: 998.6 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1001.8 µs, Skia: 1957.9 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 987.1 µs, Skia: 1009.5 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 949.1 µs, Skia: 1203.7 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1004.0 µs, Skia: 1737.2 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1050.0 µs, Skia: 1040.7 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1408.0 µs, Skia: 1088.2 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1039.1 µs, Skia: 996.8 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1097.8 µs, Skia: 999.1 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1047.0 µs, Skia: 1177.9 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1252.0 µs, Skia: 1066.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1044.8 µs, Skia: 1072.5 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1081.6 µs, Skia: 995.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1008.1 µs, Skia: 1345.3 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1073.5 µs, Skia: 1440.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1413.8 µs, Skia: 1087.7 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1413.6 µs, Skia: 1134.5 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1245.2 µs, Skia: 1088.5 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1412.3 µs, Skia: 1196.0 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1019.0 µs, Skia: 1038.2 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1018.8 µs, Skia: 997.3 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1089.4 µs, Skia: 1007.0 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1307.5 µs, Skia: 1176.8 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1526.5 µs, Skia: 1429.6 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1123.1 µs, Skia: 1864.6 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1091.6 µs, Skia: 1022.4 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1046.8 µs, Skia: 998.4 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1028.4 µs, Skia: 988.0 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1047.1 µs, Skia: 986.2 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1005.9 µs, Skia: 995.9 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 943.2 µs, Skia: 1083.1 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1003.1 µs, Skia: 1301.4 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 999.0 µs, Skia: 1005.5 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1049.4 µs, Skia: 996.9 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 933.0 µs, Skia: 997.6 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 941.9 µs, Skia: 994.3 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 972.8 µs, Skia: 1013.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 994.8 µs, Skia: 1070.6 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 986.9 µs, Skia: 998.3 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 969.7 µs, Skia: 947.8 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 956.6 µs, Skia: 986.8 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 975.9 µs, Skia: 695.1 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1208.5 µs, Skia: 1167.5 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1022.4 µs, Skia: 1792.3 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1209.7 µs, Skia: 673.5 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 962.9 µs, Skia: 574.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 998.4 µs, Skia: 997.6 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1019.2 µs, Skia: 1007.1 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 999.3 µs, Skia: 999.9 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 833.0 µs, Skia: 1126.7 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 991.2 µs, Skia: 1131.1 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 959.7 µs, Skia: 1027.8 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 978.4 µs, Skia: 997.7 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 997.3 µs, Skia: 998.1 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 999.8 µs, Skia: 1143.7 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 978.7 µs, Skia: 997.2 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 936.8 µs, Skia: 960.1 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1003.1 µs, Skia: 1260.7 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1198.7 µs, Skia: 1362.1 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1008.8 µs, Skia: 1483.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1622.7 µs, Skia: 1513.0 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 938.2 µs, Skia: 1044.5 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1019.2 µs, Skia: 1068.1 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 864.9 µs, Skia: 1039.2 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1282.0 µs, Skia: 1129.7 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.7 µs  955.8 µs    959.5 µs   12.3 µs  930.9 µs    943.2 µs       3.3x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            20.8 µs  995.1 µs   1015.9 µs   45.2 µs 1106.6 µs   1151.8 µs       2.2x Nisaba        1.13x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      26.4 µs  969.2 µs    995.6 µs  246.2 µs 1091.3 µs   1337.5 µs       9.3x Nisaba        1.34x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      28.7 µs  998.1 µs   1026.9 µs   78.1 µs  951.3 µs   1029.4 µs       2.7x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   23.0 µs 1430.0 µs   1453.0 µs   71.8 µs 1197.9 µs   1269.7 µs       3.1x Nisaba          1.14x Skia
6. 50 Simple Rounded Rects (12px Radius)     19.2 µs 1233.6 µs   1252.9 µs   58.3 µs 1031.0 µs   1089.4 µs       3.0x Nisaba          1.15x Skia
7. 50 Varying-Radius Rounded Rects           20.0 µs 1158.8 µs   1178.8 µs   45.5 µs  998.4 µs   1043.9 µs       2.3x Nisaba          1.13x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   21.1 µs 1018.2 µs   1039.3 µs   55.9 µs  952.5 µs   1008.4 µs       2.7x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          20.9 µs  967.5 µs    988.3 µs   73.8 µs  953.3 µs   1027.1 µs       3.5x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         38.7 µs  992.5 µs   1031.2 µs   43.3 µs 1073.9 µs   1117.1 µs       1.1x Nisaba        1.08x Nisaba
11. 50 Two-Stop Linear Gradients             18.2 µs 1473.0 µs   1491.2 µs  187.2 µs 1177.1 µs   1364.3 µs      10.3x Nisaba          1.09x Skia
12. 50 Diagonal Gradients (45° Angle)       19.9 µs 1416.1 µs   1436.0 µs  190.9 µs 1186.1 µs   1376.9 µs       9.6x Nisaba         Tie (~1.0x)
13. 25 Radial Glow Gradients (300px)         14.2 µs 2008.7 µs   2022.9 µs  110.4 µs 1472.1 µs   1582.5 µs       7.8x Nisaba          1.28x Skia
14. 20 Box Blur / Drop Shadows (15px)        13.0 µs 1217.1 µs   1230.0 µs  101.1 µs 1200.8 µs   1301.8 µs       7.8x Nisaba        1.06x Nisaba
15. 10 Concave 10-Point Stars (Fill)         49.0 µs 1127.0 µs   1176.1 µs  381.7 µs 1104.5 µs   1486.2 µs       7.8x Nisaba        1.26x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   33.4 µs  956.8 µs    990.2 µs  841.1 µs 1176.5 µs   2017.6 µs      25.2x Nisaba        2.04x Nisaba
17. 1000-Point Waveform (Dense Stroke)       92.5 µs  932.5 µs   1025.0 µs  853.9 µs 1241.2 µs   2095.0 µs       9.2x Nisaba        2.04x Nisaba
18. 50 Rotated Cards (Affine Transforms)     20.4 µs 1148.3 µs   1168.7 µs   63.3 µs 1067.3 µs   1130.6 µs       3.1x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             15.9 µs  965.8 µs    981.7 µs   69.7 µs  572.5 µs    642.2 µs       4.4x Nisaba          1.53x Skia
20. 50 UI Text Labels (Inter-Regular)       105.2 µs  913.9 µs   1019.2 µs  100.3 µs  938.4 µs   1038.8 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  100.4 µs 1051.5 µs   1151.9 µs  374.8 µs 1150.2 µs   1525.0 µs       3.7x Nisaba        1.32x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     39.8 µs  969.6 µs   1009.3 µs 1264.1 µs 1404.2 µs   2668.3 µs      31.8x Nisaba        2.64x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       13.6 µs  968.8 µs    982.3 µs  195.6 µs 1145.4 µs   1341.0 µs      14.4x Nisaba        1.37x Nisaba
24. Circular Arcs with arcTo (20 Corners)    30.0 µs  970.9 µs   1000.9 µs  527.0 µs 1301.3 µs   1828.3 µs      17.6x Nisaba        1.83x Nisaba
25. Directional Arcs with arc() (30 Arcs)    60.8 µs  945.1 µs   1006.0 µs  151.2 µs  935.3 µs   1086.5 µs       2.5x Nisaba        1.08x Nisaba
26. Compound Path with Holes (15 Donuts)     87.6 µs 1320.5 µs   1408.1 µs  555.1 µs 1112.6 µs   1667.7 µs       6.3x Nisaba        1.18x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    45.1 µs  949.7 µs    994.8 µs  478.0 µs 1145.2 µs   1623.1 µs      10.6x Nisaba        1.63x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    48.1 µs  938.1 µs    986.3 µs  831.8 µs 1215.7 µs   2047.4 µs      17.3x Nisaba        2.08x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)    9.8 µs  983.9 µs    993.6 µs  269.4 µs 1198.8 µs   1468.2 µs      27.6x Nisaba        1.48x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      26.0 µs 1373.3 µs   1399.3 µs 1189.0 µs 1333.5 µs   2522.5 µs      45.8x Nisaba        1.80x Nisaba
31. Stroke Join: Miter (12px Stroke)         11.1 µs  920.6 µs    931.7 µs  411.1 µs 1143.2 µs   1554.2 µs      37.0x Nisaba        1.67x Nisaba
32. Stroke Join: Bevel (12px Stroke)         12.3 µs  954.6 µs    966.9 µs  385.8 µs 1163.7 µs   1549.5 µs      31.5x Nisaba        1.60x Nisaba
33. Stroke Join: Round (12px Stroke)         17.2 µs  926.9 µs    944.1 µs  415.2 µs 1143.0 µs   1558.2 µs      24.2x Nisaba        1.65x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      14.9 µs  901.5 µs    916.3 µs   44.2 µs  965.4 µs   1009.6 µs       3.0x Nisaba        1.10x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    13.7 µs  927.9 µs    941.6 µs   51.3 µs 1072.6 µs   1123.9 µs       3.7x Nisaba        1.19x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     39.3 µs  968.5 µs   1007.8 µs   62.0 µs  936.6 µs    998.6 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   18.3 µs  983.5 µs   1001.8 µs  689.9 µs 1267.9 µs   1957.9 µs      37.7x Nisaba        1.95x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   21.2 µs  965.9 µs    987.1 µs   70.6 µs  938.8 µs   1009.5 µs       3.3x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   14.1 µs  935.0 µs    949.1 µs  156.8 µs 1046.9 µs   1203.7 µs      11.1x Nisaba        1.27x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   29.0 µs  975.0 µs   1004.0 µs  562.0 µs 1175.2 µs   1737.2 µs      19.4x Nisaba        1.73x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          15.6 µs 1034.3 µs   1050.0 µs   44.1 µs  996.6 µs   1040.7 µs       2.8x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       14.2 µs 1393.8 µs   1408.0 µs   43.5 µs 1044.7 µs   1088.2 µs       3.1x Nisaba          1.29x Skia
43. 50 Pill / Capsule Badges (r=h/2)         16.9 µs 1022.2 µs   1039.1 µs   59.0 µs  937.7 µs    996.8 µs       3.5x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     15.8 µs 1081.9 µs   1097.8 µs   47.4 µs  951.7 µs    999.1 µs       3.0x Nisaba          1.10x Skia
45. High-Density Disks Cloud (500 Disks)     63.9 µs  983.1 µs   1047.0 µs  212.9 µs  965.0 µs   1177.9 µs       3.3x Nisaba        1.12x Nisaba
46. High-Density Stroked Rings (250 Rings)   35.2 µs 1216.8 µs   1252.0 µs  139.7 µs  926.4 µs   1066.1 µs       4.0x Nisaba          1.17x Skia
47. 250 Filled Rectangles Batch              38.6 µs 1006.2 µs   1044.8 µs  132.1 µs  940.4 µs   1072.5 µs       3.4x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      42.3 µs 1039.3 µs   1081.6 µs   84.8 µs  910.5 µs    995.3 µs       2.0x Nisaba          1.09x Skia
49. 40 Crosshair Aim Reticles                87.3 µs  920.8 µs   1008.1 µs  306.2 µs 1039.1 µs   1345.3 µs       3.5x Nisaba        1.33x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      82.3 µs  991.2 µs   1073.5 µs  312.4 µs 1128.0 µs   1440.4 µs       3.8x Nisaba        1.34x Nisaba
51. 30 Vertical Card Linear Gradients        14.6 µs 1399.3 µs   1413.8 µs  147.9 µs  939.8 µs   1087.7 µs      10.1x Nisaba          1.30x Skia
52. 30 Horizontal Bar Progress Gradients     15.7 µs 1397.9 µs   1413.6 µs  148.6 µs  986.0 µs   1134.5 µs       9.5x Nisaba          1.25x Skia
53. 15 Concentric Radial Spotlights          10.9 µs 1234.3 µs   1245.2 µs   90.4 µs  998.1 µs   1088.5 µs       8.3x Nisaba          1.14x Skia
54. 25 Soft Ambient Card Shadows             12.4 µs 1399.9 µs   1412.3 µs  121.4 µs 1074.6 µs   1196.0 µs       9.8x Nisaba          1.18x Skia
55. 15 Neon Button Glows (Cyan Intense)      10.8 µs 1008.2 µs   1019.0 µs   83.5 µs  954.8 µs   1038.2 µs       7.8x Nisaba         Tie (~1.0x)
56. 16 Multi-Angle Gradient Fan Slices       30.8 µs  988.0 µs   1018.8 µs  108.2 µs  889.1 µs    997.3 µs       3.5x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     9.6 µs 1079.8 µs   1089.4 µs   30.1 µs  976.9 µs   1007.0 µs       3.1x Nisaba          1.08x Skia
58. 20 Inset Well Shadows (Recessed)         58.5 µs 1249.0 µs   1307.5 µs  104.6 µs 1072.2 µs   1176.8 µs       1.8x Nisaba          1.11x Skia
59. 10 Pulsing Circular Radar Wave Glows     12.7 µs 1513.8 µs   1526.5 µs   64.2 µs 1365.4 µs   1429.6 µs       5.1x Nisaba          1.07x Skia
60. Waveform Polyline Stroked with Gradient   52.2 µs 1070.9 µs   1123.1 µs  565.2 µs 1299.4 µs   1864.6 µs      10.8x Nisaba        1.66x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)    9.2 µs 1082.3 µs   1091.6 µs   34.3 µs  988.1 µs   1022.4 µs       3.7x Nisaba          1.07x Skia
62. Scaled Image Quad (2x 512x512 Blit)       9.6 µs 1037.3 µs   1046.8 µs   30.3 µs  968.1 µs    998.4 µs       3.2x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle       9.7 µs 1018.7 µs   1028.4 µs   40.2 µs  947.8 µs    988.0 µs       4.1x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   11.1 µs 1036.0 µs   1047.1 µs   36.7 µs  949.6 µs    986.2 µs       3.3x Nisaba          1.06x Skia
65. Multi-Avatar Grid (16 Avatars)           13.2 µs  992.7 µs   1005.9 µs   49.0 µs  946.8 µs    995.9 µs       3.7x Nisaba         Tie (~1.0x)
66. Texture Pattern on Star Path             18.5 µs  924.7 µs    943.2 µs  141.1 µs  942.0 µs   1083.1 µs       7.6x Nisaba        1.15x Nisaba
67. 20px Curved Path Stroked with Texture    14.1 µs  989.1 µs   1003.1 µs  175.0 µs 1126.4 µs   1301.4 µs      12.5x Nisaba        1.30x Nisaba
68. Minified Texture Quad (0.25x Downscale)   17.7 µs  981.3 µs    999.0 µs   72.5 µs  933.0 µs   1005.5 µs       4.1x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        25.7 µs 1023.6 µs   1049.4 µs   64.5 µs  932.3 µs    996.9 µs       2.5x Nisaba          1.05x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   21.0 µs  912.0 µs    933.0 µs   54.5 µs  943.1 µs    997.6 µs       2.6x Nisaba        1.07x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   24.5 µs  917.4 µs    941.9 µs   38.5 µs  955.8 µs    994.3 µs       1.6x Nisaba        1.06x Nisaba
72. Shear / Skew Parallelograms (30 Quads)   16.6 µs  956.2 µs    972.8 µs   54.2 µs  959.5 µs   1013.6 µs       3.3x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   17.5 µs  977.3 µs    994.8 µs   48.2 µs 1022.4 µs   1070.6 µs       2.8x Nisaba        1.08x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   14.6 µs  972.3 µs    986.9 µs   44.9 µs  953.4 µs    998.3 µs       3.1x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       24.8 µs  944.9 µs    969.7 µs   54.9 µs  893.0 µs    947.8 µs       2.2x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   15.6 µs  940.9 µs    956.6 µs   47.7 µs  939.1 µs    986.8 µs       3.1x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        12.7 µs  963.2 µs    975.9 µs   58.3 µs  636.7 µs    695.1 µs       4.6x Nisaba          1.40x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   16.0 µs 1192.5 µs   1208.5 µs   42.0 µs 1125.5 µs   1167.5 µs       2.6x Nisaba         Tie (~1.0x)
79. Scissored Stroked Waves (1000 Pts Clipped)   93.0 µs  929.4 µs   1022.4 µs  862.3 µs  929.9 µs   1792.3 µs       9.3x Nisaba        1.75x Nisaba
80. Scissored Gradient Card (Clipped)        12.2 µs 1197.5 µs   1209.7 µs   84.7 µs  588.8 µs    673.5 µs       6.9x Nisaba          1.80x Skia
81. Circular Overflow Clip (25 Boxes)        14.6 µs  948.3 µs    962.9 µs   72.1 µs  501.8 µs    574.0 µs       5.0x Nisaba          1.68x Skia
82. Scissor Invalidation / Reset (25 Cycles)   12.0 µs  986.4 µs    998.4 µs   47.6 µs  950.1 µs    997.6 µs       4.0x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    15.5 µs 1003.7 µs   1019.2 µs   43.5 µs  963.5 µs   1007.1 µs       2.8x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   14.1 µs  985.2 µs    999.3 µs   44.2 µs  955.8 µs    999.9 µs       3.1x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   13.3 µs  819.7 µs    833.0 µs   42.2 µs 1084.5 µs   1126.7 µs       3.2x Nisaba        1.35x Nisaba
86. Composite Op: Source-Out (Cutout)        15.9 µs  975.3 µs    991.2 µs   42.9 µs 1088.2 µs   1131.1 µs       2.7x Nisaba        1.14x Nisaba
87. Composite Op: Atop (Target Bounds)       14.2 µs  945.5 µs    959.7 µs   50.6 µs  977.2 µs   1027.8 µs       3.6x Nisaba        1.07x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   14.1 µs  964.3 µs    978.4 µs   44.1 µs  953.7 µs    997.7 µs       3.1x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     13.4 µs  984.0 µs    997.3 µs   46.2 µs  951.9 µs    998.1 µs       3.5x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        13.5 µs  986.3 µs    999.8 µs   41.1 µs 1102.6 µs   1143.7 µs       3.0x Nisaba        1.14x Nisaba
91. Composite Op: Xor (Exclusive Blend)      12.7 µs  966.0 µs    978.7 µs   44.0 µs  953.2 µs    997.2 µs       3.5x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    14.0 µs  922.8 µs    936.8 µs   43.9 µs  916.2 µs    960.1 µs       3.1x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   22.7 µs  980.4 µs   1003.1 µs  204.3 µs 1056.4 µs   1260.7 µs       9.0x Nisaba        1.26x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      31.6 µs 1167.1 µs   1198.7 µs  275.4 µs 1086.8 µs   1362.1 µs       8.7x Nisaba        1.14x Nisaba
95. Circular Progress Rings (8 Meters)       44.1 µs  964.7 µs   1008.8 µs  383.5 µs 1100.0 µs   1483.4 µs       8.7x Nisaba        1.47x Nisaba
96. Modern Card Stack (5 Elevated Cards)     16.8 µs 1605.9 µs   1622.7 µs   85.6 µs 1427.4 µs   1513.0 µs       5.1x Nisaba          1.07x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   23.2 µs  914.9 µs    938.2 µs   40.3 µs 1004.2 µs   1044.5 µs       1.7x Nisaba        1.11x Nisaba
98. Floating Action Button (FAB 10 Buttons)   34.6 µs  984.6 µs   1019.2 µs  130.3 µs  937.8 µs   1068.1 µs       3.8x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   31.3 µs  833.6 µs    864.9 µs  115.7 µs  923.5 µs   1039.2 µs       3.7x Nisaba        1.20x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   22.6 µs 1259.5 µs   1282.0 µs   75.4 µs 1054.3 µs   1129.7 µs       3.3x Nisaba          1.13x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2687.7 µs106027.2 µs 108714.9 µs19068.0 µs103285.6 µs 122353.6 µs      7.09x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 44 suites | Skia won in 25 suites | Ties: 31
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.09x Nisaba
===================================================================================================================================================
```
</details>

### Run 4
<details>
<summary>Click to expand Run 4 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 951.7 µs, Skia: 933.6 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 916.5 µs, Skia: 1050.6 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 995.4 µs, Skia: 1310.9 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1047.0 µs, Skia: 1057.9 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1151.9 µs, Skia: 1264.3 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1266.9 µs, Skia: 1060.4 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1306.1 µs, Skia: 1070.9 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1048.0 µs, Skia: 1019.2 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 998.2 µs, Skia: 1043.2 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 997.4 µs, Skia: 1146.9 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1465.2 µs, Skia: 1386.8 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1398.8 µs, Skia: 1431.2 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2053.1 µs, Skia: 1593.0 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1085.5 µs, Skia: 1292.0 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1167.3 µs, Skia: 1484.4 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1008.3 µs, Skia: 2019.0 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1030.6 µs, Skia: 2092.6 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1027.4 µs, Skia: 1057.3 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1000.4 µs, Skia: 633.7 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 998.3 µs, Skia: 999.1 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1139.2 µs, Skia: 1525.9 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1007.9 µs, Skia: 2616.7 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 1115.4 µs, Skia: 1301.0 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1008.6 µs, Skia: 1718.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1018.1 µs, Skia: 1066.3 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1431.9 µs, Skia: 1693.2 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1011.0 µs, Skia: 1581.3 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 997.3 µs, Skia: 2076.3 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 1000.2 µs, Skia: 1474.0 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1534.2 µs, Skia: 2583.1 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 994.2 µs, Skia: 1516.6 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 998.4 µs, Skia: 1537.3 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 996.2 µs, Skia: 1673.7 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 998.6 µs, Skia: 1048.5 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 943.8 µs, Skia: 1041.2 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1010.0 µs, Skia: 995.5 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 998.2 µs, Skia: 1957.8 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 998.2 µs, Skia: 1027.3 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 994.3 µs, Skia: 1247.7 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1012.5 µs, Skia: 1724.4 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1047.0 µs, Skia: 1038.6 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1240.2 µs, Skia: 1075.1 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1049.0 µs, Skia: 1017.7 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1117.2 µs, Skia: 1007.6 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1013.6 µs, Skia: 1141.1 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1289.4 µs, Skia: 999.2 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 998.8 µs, Skia: 1018.9 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 947.1 µs, Skia: 1011.4 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1015.5 µs, Skia: 1341.6 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1083.3 µs, Skia: 1402.5 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1241.5 µs, Skia: 1133.5 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1187.5 µs, Skia: 1070.0 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1053.5 µs, Skia: 1003.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1203.4 µs, Skia: 1242.8 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1088.2 µs, Skia: 932.4 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 997.7 µs, Skia: 989.5 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 952.0 µs, Skia: 826.3 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1322.8 µs, Skia: 1110.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1167.3 µs, Skia: 1148.5 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 993.6 µs, Skia: 1767.7 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1019.3 µs, Skia: 997.8 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 856.7 µs, Skia: 786.5 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1023.2 µs, Skia: 982.3 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1007.1 µs, Skia: 998.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 973.3 µs, Skia: 839.9 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1042.6 µs, Skia: 1093.9 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 997.5 µs, Skia: 1317.4 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 957.5 µs, Skia: 995.9 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1028.4 µs, Skia: 996.1 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 974.8 µs, Skia: 980.6 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 960.4 µs, Skia: 999.0 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 967.4 µs, Skia: 994.5 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 715.7 µs, Skia: 918.3 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 979.6 µs, Skia: 998.4 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 1033.7 µs, Skia: 986.5 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 930.8 µs, Skia: 997.8 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 984.3 µs, Skia: 680.2 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1058.1 µs, Skia: 1040.4 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1050.8 µs, Skia: 1820.4 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1204.0 µs, Skia: 672.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 995.7 µs, Skia: 595.7 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 974.9 µs, Skia: 990.5 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 998.6 µs, Skia: 997.9 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 988.4 µs, Skia: 948.4 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 954.5 µs, Skia: 1131.3 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 996.5 µs, Skia: 1099.3 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 979.2 µs, Skia: 914.3 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 998.0 µs, Skia: 998.7 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 995.4 µs, Skia: 990.4 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 984.9 µs, Skia: 1119.7 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 998.6 µs, Skia: 987.0 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 971.4 µs, Skia: 981.8 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 998.6 µs, Skia: 1266.8 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1176.7 µs, Skia: 1347.6 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1017.6 µs, Skia: 1493.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1906.8 µs, Skia: 1578.4 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 947.4 µs, Skia: 1036.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 998.0 µs, Skia: 1038.8 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 998.5 µs, Skia: 1027.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1248.9 µs, Skia: 1132.0 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          2.4 µs  949.3 µs    951.7 µs   10.7 µs  922.9 µs    933.6 µs       4.4x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            17.8 µs  898.7 µs    916.5 µs   42.3 µs 1008.4 µs   1050.6 µs       2.4x Nisaba        1.15x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      29.1 µs  966.3 µs    995.4 µs  254.4 µs 1056.6 µs   1310.9 µs       8.7x Nisaba        1.32x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      29.1 µs 1018.0 µs   1047.0 µs   79.8 µs  978.1 µs   1057.9 µs       2.7x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   21.5 µs 1130.4 µs   1151.9 µs   72.5 µs 1191.8 µs   1264.3 µs       3.4x Nisaba        1.10x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     18.2 µs 1248.6 µs   1266.9 µs   60.1 µs 1000.3 µs   1060.4 µs       3.3x Nisaba          1.19x Skia
7. 50 Varying-Radius Rounded Rects           18.5 µs 1287.7 µs   1306.1 µs   43.8 µs 1027.1 µs   1070.9 µs       2.4x Nisaba          1.22x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   21.3 µs 1026.7 µs   1048.0 µs   59.8 µs  959.4 µs   1019.2 µs       2.8x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          21.1 µs  977.1 µs    998.2 µs   80.2 µs  963.0 µs   1043.2 µs       3.8x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         39.4 µs  958.0 µs    997.4 µs   43.1 µs 1103.8 µs   1146.9 µs       1.1x Nisaba        1.15x Nisaba
11. 50 Two-Stop Linear Gradients             17.4 µs 1447.8 µs   1465.2 µs  185.4 µs 1201.4 µs   1386.8 µs      10.6x Nisaba          1.06x Skia
12. 50 Diagonal Gradients (45° Angle)       16.5 µs 1382.3 µs   1398.8 µs  204.6 µs 1226.6 µs   1431.2 µs      12.4x Nisaba         Tie (~1.0x)
13. 25 Radial Glow Gradients (300px)         15.2 µs 2038.0 µs   2053.1 µs  105.5 µs 1487.5 µs   1593.0 µs       7.0x Nisaba          1.29x Skia
14. 20 Box Blur / Drop Shadows (15px)        11.9 µs 1073.6 µs   1085.5 µs   97.6 µs 1194.4 µs   1292.0 µs       8.2x Nisaba        1.19x Nisaba
15. 10 Concave 10-Point Stars (Fill)         46.4 µs 1120.9 µs   1167.3 µs  396.5 µs 1088.0 µs   1484.4 µs       8.5x Nisaba        1.27x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   33.7 µs  974.5 µs   1008.3 µs  844.5 µs 1174.5 µs   2019.0 µs      25.1x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       88.1 µs  942.6 µs   1030.6 µs  855.8 µs 1236.8 µs   2092.6 µs       9.7x Nisaba        2.03x Nisaba
18. 50 Rotated Cards (Affine Transforms)     18.3 µs 1009.1 µs   1027.4 µs   67.5 µs  989.8 µs   1057.3 µs       3.7x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             13.2 µs  987.2 µs   1000.4 µs   71.4 µs  562.3 µs    633.7 µs       5.4x Nisaba          1.58x Skia
20. 50 UI Text Labels (Inter-Regular)       102.7 µs  895.6 µs    998.3 µs   99.6 µs  899.5 µs    999.1 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  104.6 µs 1034.7 µs   1139.2 µs  365.1 µs 1160.7 µs   1525.9 µs       3.5x Nisaba        1.34x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     39.5 µs  968.4 µs   1007.9 µs 1254.1 µs 1362.5 µs   2616.7 µs      31.7x Nisaba        2.60x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       14.4 µs 1101.0 µs   1115.4 µs  191.7 µs 1109.4 µs   1301.0 µs      13.3x Nisaba        1.17x Nisaba
24. Circular Arcs with arcTo (20 Corners)    28.1 µs  980.5 µs   1008.6 µs  516.8 µs 1201.5 µs   1718.3 µs      18.4x Nisaba        1.70x Nisaba
25. Directional Arcs with arc() (30 Arcs)    58.9 µs  959.2 µs   1018.1 µs  151.6 µs  914.7 µs   1066.3 µs       2.6x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     89.2 µs 1342.8 µs   1431.9 µs  549.6 µs 1143.6 µs   1693.2 µs       6.2x Nisaba        1.18x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    41.0 µs  970.0 µs   1011.0 µs  490.1 µs 1091.3 µs   1581.3 µs      12.0x Nisaba        1.56x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    45.3 µs  952.1 µs    997.3 µs  822.1 µs 1254.2 µs   2076.3 µs      18.2x Nisaba        2.08x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   10.9 µs  989.3 µs   1000.2 µs  269.9 µs 1204.1 µs   1474.0 µs      24.8x Nisaba        1.47x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      31.9 µs 1502.3 µs   1534.2 µs 1198.8 µs 1384.3 µs   2583.1 µs      37.5x Nisaba        1.68x Nisaba
31. Stroke Join: Miter (12px Stroke)         11.3 µs  982.9 µs    994.2 µs  415.1 µs 1101.5 µs   1516.6 µs      36.9x Nisaba        1.53x Nisaba
32. Stroke Join: Bevel (12px Stroke)         11.7 µs  986.8 µs    998.4 µs  396.9 µs 1140.5 µs   1537.3 µs      34.0x Nisaba        1.54x Nisaba
33. Stroke Join: Round (12px Stroke)         12.8 µs  983.3 µs    996.2 µs  423.2 µs 1250.6 µs   1673.7 µs      33.0x Nisaba        1.68x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      13.0 µs  985.6 µs    998.6 µs   45.3 µs 1003.3 µs   1048.5 µs       3.5x Nisaba        1.05x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    15.8 µs  928.0 µs    943.8 µs   49.6 µs  991.6 µs   1041.2 µs       3.1x Nisaba        1.10x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     37.7 µs  972.3 µs   1010.0 µs   62.0 µs  933.5 µs    995.5 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   16.6 µs  981.5 µs    998.2 µs  710.8 µs 1246.9 µs   1957.8 µs      42.8x Nisaba        1.96x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   18.2 µs  980.0 µs    998.2 µs   68.1 µs  959.2 µs   1027.3 µs       3.7x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   11.1 µs  983.2 µs    994.3 µs  164.5 µs 1083.2 µs   1247.7 µs      14.8x Nisaba        1.25x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   27.3 µs  985.2 µs   1012.5 µs  537.6 µs 1186.8 µs   1724.4 µs      19.7x Nisaba        1.70x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          18.1 µs 1028.9 µs   1047.0 µs   43.8 µs  994.8 µs   1038.6 µs       2.4x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       18.4 µs 1221.8 µs   1240.2 µs   42.9 µs 1032.2 µs   1075.1 µs       2.3x Nisaba          1.15x Skia
43. 50 Pill / Capsule Badges (r=h/2)         25.2 µs 1023.8 µs   1049.0 µs   60.1 µs  957.6 µs   1017.7 µs       2.4x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     15.3 µs 1101.9 µs   1117.2 µs   48.3 µs  959.4 µs   1007.6 µs       3.2x Nisaba          1.11x Skia
45. High-Density Disks Cloud (500 Disks)     67.0 µs  946.6 µs   1013.6 µs  213.2 µs  927.9 µs   1141.1 µs       3.2x Nisaba        1.13x Nisaba
46. High-Density Stroked Rings (250 Rings)   36.2 µs 1253.2 µs   1289.4 µs  133.0 µs  866.2 µs    999.2 µs       3.7x Nisaba          1.29x Skia
47. 250 Filled Rectangles Batch              39.4 µs  959.4 µs    998.8 µs  129.2 µs  889.7 µs   1018.9 µs       3.3x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      41.5 µs  905.6 µs    947.1 µs   82.7 µs  928.7 µs   1011.4 µs       2.0x Nisaba        1.07x Nisaba
49. 40 Crosshair Aim Reticles                86.7 µs  928.9 µs   1015.5 µs  302.0 µs 1039.6 µs   1341.6 µs       3.5x Nisaba        1.32x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      85.6 µs  997.6 µs   1083.3 µs  308.7 µs 1093.8 µs   1402.5 µs       3.6x Nisaba        1.29x Nisaba
51. 30 Vertical Card Linear Gradients        14.4 µs 1227.1 µs   1241.5 µs  147.9 µs  985.6 µs   1133.5 µs      10.3x Nisaba          1.10x Skia
52. 30 Horizontal Bar Progress Gradients     14.4 µs 1173.1 µs   1187.5 µs  143.6 µs  926.4 µs   1070.0 µs      10.0x Nisaba          1.11x Skia
53. 15 Concentric Radial Spotlights          10.2 µs 1043.3 µs   1053.5 µs   91.1 µs  912.0 µs   1003.1 µs       8.9x Nisaba         Tie (~1.0x)
54. 25 Soft Ambient Card Shadows             15.2 µs 1188.2 µs   1203.4 µs  131.4 µs 1111.3 µs   1242.8 µs       8.7x Nisaba         Tie (~1.0x)
55. 15 Neon Button Glows (Cyan Intense)      12.5 µs 1075.8 µs   1088.2 µs   84.9 µs  847.5 µs    932.4 µs       6.8x Nisaba          1.17x Skia
56. 16 Multi-Angle Gradient Fan Slices       24.9 µs  972.9 µs    997.7 µs  114.2 µs  875.3 µs    989.5 µs       4.6x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     8.4 µs  943.6 µs    952.0 µs   33.6 µs  792.7 µs    826.3 µs       4.0x Nisaba          1.15x Skia
58. 20 Inset Well Shadows (Recessed)         47.8 µs 1275.0 µs   1322.8 µs  106.1 µs 1004.4 µs   1110.5 µs       2.2x Nisaba          1.19x Skia
59. 10 Pulsing Circular Radar Wave Glows     13.4 µs 1153.8 µs   1167.3 µs   65.4 µs 1083.1 µs   1148.5 µs       4.9x Nisaba         Tie (~1.0x)
60. Waveform Polyline Stroked with Gradient   44.8 µs  948.7 µs    993.6 µs  553.3 µs 1214.4 µs   1767.7 µs      12.3x Nisaba        1.78x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   11.0 µs 1008.3 µs   1019.3 µs   30.0 µs  967.8 µs    997.8 µs       2.7x Nisaba         Tie (~1.0x)
62. Scaled Image Quad (2x 512x512 Blit)       7.7 µs  849.0 µs    856.7 µs   29.1 µs  757.4 µs    786.5 µs       3.8x Nisaba          1.09x Skia
63. 45° Rotated Pattern Fill in Circle       9.2 µs 1014.0 µs   1023.2 µs   34.4 µs  947.8 µs    982.3 µs       3.7x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)    8.4 µs  998.7 µs   1007.1 µs   33.5 µs  964.4 µs    998.0 µs       4.0x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           13.1 µs  960.2 µs    973.3 µs   44.0 µs  795.9 µs    839.9 µs       3.4x Nisaba          1.16x Skia
66. Texture Pattern on Star Path             19.1 µs 1023.4 µs   1042.6 µs  140.6 µs  953.3 µs   1093.9 µs       7.4x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    14.4 µs  983.0 µs    997.5 µs  175.2 µs 1142.2 µs   1317.4 µs      12.1x Nisaba        1.32x Nisaba
68. Minified Texture Quad (0.25x Downscale)   19.7 µs  937.8 µs    957.5 µs   72.6 µs  923.3 µs    995.9 µs       3.7x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        22.3 µs 1006.1 µs   1028.4 µs   62.8 µs  933.4 µs    996.1 µs       2.8x Nisaba         Tie (~1.0x)
70. Pure Rotation Cluster (36 Spokes 10°)   17.1 µs  957.7 µs    974.8 µs   52.7 µs  927.8 µs    980.6 µs       3.1x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   24.5 µs  935.9 µs    960.4 µs   38.6 µs  960.4 µs    999.0 µs       1.6x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   15.8 µs  951.5 µs    967.4 µs   50.9 µs  943.6 µs    994.5 µs       3.2x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   17.6 µs  698.1 µs    715.7 µs   44.0 µs  874.3 µs    918.3 µs       2.5x Nisaba        1.28x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   14.7 µs  964.9 µs    979.6 µs   39.3 µs  959.1 µs    998.4 µs       2.7x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       17.9 µs 1015.8 µs   1033.7 µs   54.2 µs  932.3 µs    986.5 µs       3.0x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   16.4 µs  914.5 µs    930.8 µs   46.5 µs  951.4 µs    997.8 µs       2.8x Nisaba        1.07x Nisaba
77. Multi-Scissor Grid (16 Viewports)        14.7 µs  969.5 µs    984.3 µs   56.2 µs  624.0 µs    680.2 µs       3.8x Nisaba          1.45x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   14.4 µs 1043.7 µs   1058.1 µs   38.0 µs 1002.4 µs   1040.4 µs       2.6x Nisaba         Tie (~1.0x)
79. Scissored Stroked Waves (1000 Pts Clipped)   94.6 µs  956.2 µs   1050.8 µs  879.1 µs  941.3 µs   1820.4 µs       9.3x Nisaba        1.73x Nisaba
80. Scissored Gradient Card (Clipped)        11.1 µs 1192.9 µs   1204.0 µs   84.0 µs  588.4 µs    672.4 µs       7.5x Nisaba          1.79x Skia
81. Circular Overflow Clip (25 Boxes)        15.7 µs  980.0 µs    995.7 µs   70.2 µs  525.5 µs    595.7 µs       4.5x Nisaba          1.67x Skia
82. Scissor Invalidation / Reset (25 Cycles)   16.1 µs  958.8 µs    974.9 µs   50.4 µs  940.2 µs    990.5 µs       3.1x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    13.9 µs  984.7 µs    998.6 µs   41.7 µs  956.2 µs    997.9 µs       3.0x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   13.0 µs  975.4 µs    988.4 µs   46.5 µs  902.0 µs    948.4 µs       3.6x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   12.3 µs  942.2 µs    954.5 µs   44.9 µs 1086.4 µs   1131.3 µs       3.6x Nisaba        1.19x Nisaba
86. Composite Op: Source-Out (Cutout)        16.6 µs  979.9 µs    996.5 µs   44.3 µs 1055.0 µs   1099.3 µs       2.7x Nisaba        1.10x Nisaba
87. Composite Op: Atop (Target Bounds)       13.9 µs  965.3 µs    979.2 µs   45.3 µs  869.0 µs    914.3 µs       3.3x Nisaba          1.07x Skia
88. Composite Op: Dest-Over (Under-Drawing)   14.7 µs  983.3 µs    998.0 µs   46.0 µs  952.7 µs    998.7 µs       3.1x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     14.8 µs  980.6 µs    995.4 µs   45.3 µs  945.1 µs    990.4 µs       3.1x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        14.8 µs  970.2 µs    984.9 µs   43.1 µs 1076.6 µs   1119.7 µs       2.9x Nisaba        1.14x Nisaba
91. Composite Op: Xor (Exclusive Blend)      14.2 µs  984.4 µs    998.6 µs   44.7 µs  942.3 µs    987.0 µs       3.1x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    15.9 µs  955.5 µs    971.4 µs   46.8 µs  934.9 µs    981.8 µs       2.9x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   24.8 µs  973.8 µs    998.6 µs  201.6 µs 1065.2 µs   1266.8 µs       8.1x Nisaba        1.27x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      32.2 µs 1144.4 µs   1176.7 µs  271.5 µs 1076.0 µs   1347.6 µs       8.4x Nisaba        1.15x Nisaba
95. Circular Progress Rings (8 Meters)       44.4 µs  973.2 µs   1017.6 µs  389.5 µs 1103.9 µs   1493.4 µs       8.8x Nisaba        1.47x Nisaba
96. Modern Card Stack (5 Elevated Cards)     15.9 µs 1890.8 µs   1906.8 µs   82.3 µs 1496.1 µs   1578.4 µs       5.2x Nisaba          1.21x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   24.1 µs  923.3 µs    947.4 µs   42.0 µs  994.7 µs   1036.7 µs       1.7x Nisaba        1.09x Nisaba
98. Floating Action Button (FAB 10 Buttons)   37.3 µs  960.7 µs    998.0 µs  128.5 µs  910.2 µs   1038.8 µs       3.4x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   30.3 µs  968.3 µs    998.5 µs  114.3 µs  912.8 µs   1027.1 µs       3.8x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   23.6 µs 1225.3 µs   1248.9 µs   76.7 µs 1055.3 µs   1132.0 µs       3.2x Nisaba          1.10x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2652.3 µs104372.4 µs 107024.7 µs19074.7 µs101328.5 µs 120403.1 µs      7.19x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 41 suites | Skia won in 21 suites | Ties: 38
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.19x Nisaba
===================================================================================================================================================
```
</details>

### Run 5
<details>
<summary>Click to expand Run 5 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 886.3 µs, Skia: 829.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 866.2 µs, Skia: 1031.4 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1004.6 µs, Skia: 1317.3 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1037.9 µs, Skia: 1026.1 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1236.1 µs, Skia: 1389.9 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1134.5 µs, Skia: 1069.9 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1216.2 µs, Skia: 1045.1 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1027.3 µs, Skia: 997.4 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 997.1 µs, Skia: 998.0 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 996.8 µs, Skia: 1038.0 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1523.4 µs, Skia: 1336.6 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1393.1 µs, Skia: 1429.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 1956.6 µs, Skia: 1571.8 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1116.7 µs, Skia: 1284.3 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1167.2 µs, Skia: 1486.2 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 986.6 µs, Skia: 1997.0 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 983.6 µs, Skia: 2040.8 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1190.2 µs, Skia: 1125.1 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1032.6 µs, Skia: 636.5 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1007.5 µs, Skia: 1009.1 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1123.3 µs, Skia: 1521.8 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1011.7 µs, Skia: 2610.1 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 746.9 µs, Skia: 1317.2 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 914.4 µs, Skia: 1766.1 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 998.1 µs, Skia: 1057.2 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1417.6 µs, Skia: 1649.0 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1012.0 µs, Skia: 1567.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 995.6 µs, Skia: 2038.5 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 954.8 µs, Skia: 1465.1 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1328.5 µs, Skia: 2591.9 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 997.6 µs, Skia: 1508.3 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 967.0 µs, Skia: 1559.0 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1002.4 µs, Skia: 1656.4 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 993.0 µs, Skia: 1035.3 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 924.8 µs, Skia: 1029.5 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1007.1 µs, Skia: 998.7 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1002.4 µs, Skia: 1915.7 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 962.1 µs, Skia: 1000.1 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 920.7 µs, Skia: 1227.2 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1000.6 µs, Skia: 1731.0 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1018.4 µs, Skia: 1005.7 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1157.2 µs, Skia: 1055.9 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1038.9 µs, Skia: 997.3 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1092.7 µs, Skia: 996.9 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1003.7 µs, Skia: 1190.2 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1361.2 µs, Skia: 1033.8 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1014.3 µs, Skia: 1086.0 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 951.8 µs, Skia: 1033.7 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1031.7 µs, Skia: 1343.1 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1088.0 µs, Skia: 1419.2 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1242.4 µs, Skia: 1172.5 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1171.1 µs, Skia: 1162.9 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1282.7 µs, Skia: 1092.4 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1418.7 µs, Skia: 1217.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1145.0 µs, Skia: 1048.4 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1028.9 µs, Skia: 1013.1 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1196.5 µs, Skia: 980.5 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1344.8 µs, Skia: 1210.4 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1392.2 µs, Skia: 1287.9 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1009.4 µs, Skia: 1847.5 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1157.1 µs, Skia: 1006.7 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1004.3 µs, Skia: 942.6 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1051.1 µs, Skia: 983.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1028.5 µs, Skia: 938.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 981.1 µs, Skia: 908.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 997.8 µs, Skia: 1073.4 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1007.2 µs, Skia: 1296.3 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 998.2 µs, Skia: 996.9 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1058.3 µs, Skia: 1037.4 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 998.7 µs, Skia: 1007.2 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 999.2 µs, Skia: 1105.7 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 974.1 µs, Skia: 1000.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 999.0 µs, Skia: 939.6 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 991.9 µs, Skia: 1000.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 972.2 µs, Skia: 992.0 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 943.2 µs, Skia: 982.7 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 996.6 µs, Skia: 712.0 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1028.3 µs, Skia: 1047.9 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1025.4 µs, Skia: 1814.1 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1210.8 µs, Skia: 692.7 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 997.0 µs, Skia: 555.2 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 998.6 µs, Skia: 1019.0 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 865.8 µs, Skia: 760.1 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 687.0 µs, Skia: 979.2 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 959.4 µs, Skia: 1123.2 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 983.7 µs, Skia: 1091.5 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 994.6 µs, Skia: 993.2 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 997.2 µs, Skia: 999.4 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 999.9 µs, Skia: 998.1 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 971.3 µs, Skia: 1103.3 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 978.4 µs, Skia: 987.1 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 973.5 µs, Skia: 985.7 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 900.5 µs, Skia: 1242.3 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1118.0 µs, Skia: 1358.4 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1033.3 µs, Skia: 1503.9 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1586.3 µs, Skia: 1556.8 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 932.9 µs, Skia: 1063.8 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 996.6 µs, Skia: 1048.1 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 976.8 µs, Skia: 989.5 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1356.9 µs, Skia: 1111.9 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          2.5 µs  883.7 µs    886.3 µs   11.0 µs  818.9 µs    829.9 µs       4.4x Nisaba          1.07x Skia
2. 100 Grid Lines (1px Hairlines)            19.3 µs  846.8 µs    866.2 µs   42.0 µs  989.4 µs   1031.4 µs       2.2x Nisaba        1.19x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      24.9 µs  979.7 µs   1004.6 µs  243.9 µs 1073.4 µs   1317.3 µs       9.8x Nisaba        1.31x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      29.4 µs 1008.5 µs   1037.9 µs   79.5 µs  946.6 µs   1026.1 µs       2.7x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   25.4 µs 1210.7 µs   1236.1 µs   74.3 µs 1315.6 µs   1389.9 µs       2.9x Nisaba        1.12x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     18.0 µs 1116.5 µs   1134.5 µs   55.6 µs 1014.4 µs   1069.9 µs       3.1x Nisaba          1.06x Skia
7. 50 Varying-Radius Rounded Rects           18.4 µs 1197.8 µs   1216.2 µs   45.2 µs  999.9 µs   1045.1 µs       2.5x Nisaba          1.16x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   22.2 µs 1005.1 µs   1027.3 µs   60.9 µs  936.5 µs    997.4 µs       2.7x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          19.3 µs  977.8 µs    997.1 µs   73.7 µs  924.4 µs    998.0 µs       3.8x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         42.7 µs  954.2 µs    996.8 µs   44.8 µs  993.2 µs   1038.0 µs       1.1x Nisaba         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             22.1 µs 1501.3 µs   1523.4 µs  190.0 µs 1146.5 µs   1336.6 µs       8.6x Nisaba          1.14x Skia
12. 50 Diagonal Gradients (45° Angle)       21.2 µs 1371.8 µs   1393.1 µs  198.0 µs 1231.8 µs   1429.9 µs       9.3x Nisaba         Tie (~1.0x)
13. 25 Radial Glow Gradients (300px)         15.8 µs 1940.9 µs   1956.6 µs  106.6 µs 1465.3 µs   1571.8 µs       6.8x Nisaba          1.24x Skia
14. 20 Box Blur / Drop Shadows (15px)        13.5 µs 1103.2 µs   1116.7 µs   98.4 µs 1185.9 µs   1284.3 µs       7.3x Nisaba        1.15x Nisaba
15. 10 Concave 10-Point Stars (Fill)         45.9 µs 1121.3 µs   1167.2 µs  397.1 µs 1089.1 µs   1486.2 µs       8.6x Nisaba        1.27x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   34.4 µs  952.2 µs    986.6 µs  878.0 µs 1119.0 µs   1997.0 µs      25.5x Nisaba        2.02x Nisaba
17. 1000-Point Waveform (Dense Stroke)       90.9 µs  892.7 µs    983.6 µs  837.0 µs 1203.8 µs   2040.8 µs       9.2x Nisaba        2.07x Nisaba
18. 50 Rotated Cards (Affine Transforms)     22.6 µs 1167.6 µs   1190.2 µs   68.0 µs 1057.1 µs   1125.1 µs       3.0x Nisaba          1.06x Skia
19. 25 Scissor Clipped Viewports             15.1 µs 1017.5 µs   1032.6 µs   68.7 µs  567.8 µs    636.5 µs       4.5x Nisaba          1.62x Skia
20. 50 UI Text Labels (Inter-Regular)       102.4 µs  905.1 µs   1007.5 µs   97.9 µs  911.2 µs   1009.1 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  104.1 µs 1019.1 µs   1123.3 µs  379.0 µs 1142.7 µs   1521.8 µs       3.6x Nisaba        1.35x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     39.5 µs  972.1 µs   1011.7 µs 1237.2 µs 1372.9 µs   2610.1 µs      31.3x Nisaba        2.58x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       14.0 µs  732.9 µs    746.9 µs  185.4 µs 1131.8 µs   1317.2 µs      13.3x Nisaba        1.76x Nisaba
24. Circular Arcs with arcTo (20 Corners)    30.6 µs  883.8 µs    914.4 µs  518.2 µs 1247.9 µs   1766.1 µs      16.9x Nisaba        1.93x Nisaba
25. Directional Arcs with arc() (30 Arcs)    55.4 µs  942.6 µs    998.1 µs  152.8 µs  904.5 µs   1057.2 µs       2.8x Nisaba        1.06x Nisaba
26. Compound Path with Holes (15 Donuts)     85.7 µs 1332.0 µs   1417.6 µs  523.6 µs 1125.4 µs   1649.0 µs       6.1x Nisaba        1.16x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    40.3 µs  971.7 µs   1012.0 µs  478.7 µs 1088.7 µs   1567.4 µs      11.9x Nisaba        1.55x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    47.7 µs  947.9 µs    995.6 µs  815.9 µs 1222.6 µs   2038.5 µs      17.1x Nisaba        2.05x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   10.8 µs  944.0 µs    954.8 µs  267.4 µs 1197.7 µs   1465.1 µs      24.8x Nisaba        1.53x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      27.7 µs 1300.8 µs   1328.5 µs 1183.0 µs 1408.9 µs   2591.9 µs      42.8x Nisaba        1.95x Nisaba
31. Stroke Join: Miter (12px Stroke)         11.6 µs  985.9 µs    997.6 µs  407.1 µs 1101.1 µs   1508.3 µs      35.0x Nisaba        1.51x Nisaba
32. Stroke Join: Bevel (12px Stroke)         11.0 µs  956.0 µs    967.0 µs  390.9 µs 1168.2 µs   1559.0 µs      35.6x Nisaba        1.61x Nisaba
33. Stroke Join: Round (12px Stroke)         16.3 µs  986.1 µs   1002.4 µs  417.0 µs 1239.4 µs   1656.4 µs      25.5x Nisaba        1.65x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      12.9 µs  980.1 µs    993.0 µs   46.5 µs  988.9 µs   1035.3 µs       3.6x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    13.7 µs  911.1 µs    924.8 µs   43.1 µs  986.4 µs   1029.5 µs       3.1x Nisaba        1.11x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     37.3 µs  969.8 µs   1007.1 µs   61.9 µs  936.8 µs    998.7 µs       1.7x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   15.8 µs  986.6 µs   1002.4 µs  700.2 µs 1215.5 µs   1915.7 µs      44.4x Nisaba        1.91x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   21.0 µs  941.1 µs    962.1 µs   73.6 µs  926.5 µs   1000.1 µs       3.5x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   12.5 µs  908.2 µs    920.7 µs  159.2 µs 1068.0 µs   1227.2 µs      12.7x Nisaba        1.33x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   26.9 µs  973.7 µs   1000.6 µs  559.2 µs 1171.8 µs   1731.0 µs      20.8x Nisaba        1.73x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          16.2 µs 1002.2 µs   1018.4 µs   41.7 µs  964.0 µs   1005.7 µs       2.6x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       15.3 µs 1141.9 µs   1157.2 µs   41.6 µs 1014.3 µs   1055.9 µs       2.7x Nisaba          1.10x Skia
43. 50 Pill / Capsule Badges (r=h/2)         27.2 µs 1011.7 µs   1038.9 µs   62.7 µs  934.6 µs    997.3 µs       2.3x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     16.4 µs 1076.3 µs   1092.7 µs   46.4 µs  950.5 µs    996.9 µs       2.8x Nisaba          1.10x Skia
45. High-Density Disks Cloud (500 Disks)     64.3 µs  939.4 µs   1003.7 µs  212.4 µs  977.8 µs   1190.2 µs       3.3x Nisaba        1.19x Nisaba
46. High-Density Stroked Rings (250 Rings)   36.9 µs 1324.3 µs   1361.2 µs  143.8 µs  890.0 µs   1033.8 µs       3.9x Nisaba          1.32x Skia
47. 250 Filled Rectangles Batch              37.2 µs  977.1 µs   1014.3 µs  132.5 µs  953.5 µs   1086.0 µs       3.6x Nisaba        1.07x Nisaba
48. 150 Stroked Rectangles (2px Border)      38.3 µs  913.5 µs    951.8 µs   86.4 µs  947.4 µs   1033.7 µs       2.3x Nisaba        1.09x Nisaba
49. 40 Crosshair Aim Reticles                86.5 µs  945.2 µs   1031.7 µs  307.0 µs 1036.1 µs   1343.1 µs       3.5x Nisaba        1.30x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      79.0 µs 1009.0 µs   1088.0 µs  311.0 µs 1108.2 µs   1419.2 µs       3.9x Nisaba        1.30x Nisaba
51. 30 Vertical Card Linear Gradients        14.7 µs 1227.6 µs   1242.4 µs  156.6 µs 1015.9 µs   1172.5 µs      10.6x Nisaba          1.06x Skia
52. 30 Horizontal Bar Progress Gradients     14.9 µs 1156.2 µs   1171.1 µs  154.4 µs 1008.5 µs   1162.9 µs      10.3x Nisaba         Tie (~1.0x)
53. 15 Concentric Radial Spotlights          13.7 µs 1269.0 µs   1282.7 µs   96.1 µs  996.2 µs   1092.4 µs       7.0x Nisaba          1.17x Skia
54. 25 Soft Ambient Card Shadows             15.8 µs 1402.8 µs   1418.7 µs  135.1 µs 1082.4 µs   1217.5 µs       8.5x Nisaba          1.17x Skia
55. 15 Neon Button Glows (Cyan Intense)      11.7 µs 1133.3 µs   1145.0 µs   84.8 µs  963.6 µs   1048.4 µs       7.2x Nisaba          1.09x Skia
56. 16 Multi-Angle Gradient Fan Slices       28.5 µs 1000.4 µs   1028.9 µs  108.4 µs  904.7 µs   1013.1 µs       3.8x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     9.0 µs 1187.5 µs   1196.5 µs   33.4 µs  947.1 µs    980.5 µs       3.7x Nisaba          1.22x Skia
58. 20 Inset Well Shadows (Recessed)         50.4 µs 1294.5 µs   1344.8 µs  117.9 µs 1092.5 µs   1210.4 µs       2.3x Nisaba          1.11x Skia
59. 10 Pulsing Circular Radar Wave Glows     14.8 µs 1377.4 µs   1392.2 µs   65.7 µs 1222.2 µs   1287.9 µs       4.4x Nisaba          1.08x Skia
60. Waveform Polyline Stroked with Gradient   51.1 µs  958.3 µs   1009.4 µs  576.6 µs 1270.9 µs   1847.5 µs      11.3x Nisaba        1.83x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   10.0 µs 1147.1 µs   1157.1 µs   33.4 µs  973.3 µs   1006.7 µs       3.3x Nisaba          1.15x Skia
62. Scaled Image Quad (2x 512x512 Blit)       9.8 µs  994.5 µs   1004.3 µs   32.9 µs  909.7 µs    942.6 µs       3.4x Nisaba          1.07x Skia
63. 45° Rotated Pattern Fill in Circle       9.7 µs 1041.4 µs   1051.1 µs   37.6 µs  945.8 µs    983.4 µs       3.9x Nisaba          1.07x Skia
64. Translucent Texture Overlay (50% Alpha)   10.8 µs 1017.7 µs   1028.5 µs   37.3 µs  900.6 µs    938.0 µs       3.5x Nisaba          1.10x Skia
65. Multi-Avatar Grid (16 Avatars)           11.2 µs  969.9 µs    981.1 µs   45.7 µs  862.5 µs    908.2 µs       4.1x Nisaba          1.08x Skia
66. Texture Pattern on Star Path             18.4 µs  979.5 µs    997.8 µs  136.9 µs  936.5 µs   1073.4 µs       7.5x Nisaba        1.08x Nisaba
67. 20px Curved Path Stroked with Texture    15.2 µs  991.9 µs   1007.2 µs  171.8 µs 1124.5 µs   1296.3 µs      11.3x Nisaba        1.29x Nisaba
68. Minified Texture Quad (0.25x Downscale)   17.2 µs  981.0 µs    998.2 µs   73.5 µs  923.4 µs    996.9 µs       4.3x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        24.7 µs 1033.6 µs   1058.3 µs   65.1 µs  972.3 µs   1037.4 µs       2.6x Nisaba         Tie (~1.0x)
70. Pure Rotation Cluster (36 Spokes 10°)   16.7 µs  982.0 µs    998.7 µs   50.9 µs  956.3 µs   1007.2 µs       3.1x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   22.0 µs  977.3 µs    999.2 µs   38.1 µs 1067.6 µs   1105.7 µs       1.7x Nisaba        1.11x Nisaba
72. Shear / Skew Parallelograms (30 Quads)   17.1 µs  957.0 µs    974.1 µs   51.2 µs  949.4 µs   1000.6 µs       3.0x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   15.8 µs  983.2 µs    999.0 µs   45.7 µs  893.9 µs    939.6 µs       2.9x Nisaba          1.06x Skia
74. Deep Hierarchical State Stack (20 Levels)   16.7 µs  975.2 µs    991.9 µs   41.0 µs  959.9 µs   1000.8 µs       2.4x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       20.4 µs  951.9 µs    972.2 µs   60.2 µs  931.8 µs    992.0 µs       3.0x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   14.8 µs  928.5 µs    943.2 µs   45.0 µs  937.7 µs    982.7 µs       3.0x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        14.2 µs  982.3 µs    996.6 µs   56.0 µs  656.0 µs    712.0 µs       3.9x Nisaba          1.40x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   12.5 µs 1015.8 µs   1028.3 µs   38.3 µs 1009.6 µs   1047.9 µs       3.1x Nisaba         Tie (~1.0x)
79. Scissored Stroked Waves (1000 Pts Clipped)   97.5 µs  927.9 µs   1025.4 µs  884.2 µs  930.0 µs   1814.1 µs       9.1x Nisaba        1.77x Nisaba
80. Scissored Gradient Card (Clipped)        16.0 µs 1194.8 µs   1210.8 µs   85.0 µs  607.7 µs    692.7 µs       5.3x Nisaba          1.75x Skia
81. Circular Overflow Clip (25 Boxes)        13.2 µs  983.8 µs    997.0 µs   70.6 µs  484.6 µs    555.2 µs       5.3x Nisaba          1.80x Skia
82. Scissor Invalidation / Reset (25 Cycles)   14.4 µs  984.3 µs    998.6 µs   58.1 µs  960.9 µs   1019.0 µs       4.0x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    13.7 µs  852.0 µs    865.8 µs   33.0 µs  727.1 µs    760.1 µs       2.4x Nisaba          1.14x Skia
84. Composite Op: Lighter / Plus (30 Particles)   10.2 µs  676.8 µs    687.0 µs   43.7 µs  935.5 µs    979.2 µs       4.3x Nisaba        1.43x Nisaba
85. Composite Op: Source-In (Alpha Masking)   14.3 µs  945.1 µs    959.4 µs   44.5 µs 1078.7 µs   1123.2 µs       3.1x Nisaba        1.17x Nisaba
86. Composite Op: Source-Out (Cutout)        15.3 µs  968.4 µs    983.7 µs   43.2 µs 1048.4 µs   1091.5 µs       2.8x Nisaba        1.11x Nisaba
87. Composite Op: Atop (Target Bounds)       12.4 µs  982.2 µs    994.6 µs   41.0 µs  952.2 µs    993.2 µs       3.3x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   14.5 µs  982.7 µs    997.2 µs   46.6 µs  952.8 µs    999.4 µs       3.2x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     15.4 µs  984.5 µs    999.9 µs   48.1 µs  950.0 µs    998.1 µs       3.1x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        14.0 µs  957.3 µs    971.3 µs   44.9 µs 1058.4 µs   1103.3 µs       3.2x Nisaba        1.14x Nisaba
91. Composite Op: Xor (Exclusive Blend)      13.0 µs  965.4 µs    978.4 µs   42.7 µs  944.4 µs    987.1 µs       3.3x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    15.2 µs  958.4 µs    973.5 µs   44.3 µs  941.4 µs    985.7 µs       2.9x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   24.5 µs  876.0 µs    900.5 µs  203.9 µs 1038.4 µs   1242.3 µs       8.3x Nisaba        1.38x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      30.5 µs 1087.5 µs   1118.0 µs  282.5 µs 1075.9 µs   1358.4 µs       9.3x Nisaba        1.22x Nisaba
95. Circular Progress Rings (8 Meters)       44.9 µs  988.4 µs   1033.3 µs  386.2 µs 1117.6 µs   1503.9 µs       8.6x Nisaba        1.46x Nisaba
96. Modern Card Stack (5 Elevated Cards)     16.8 µs 1569.5 µs   1586.3 µs   85.9 µs 1470.9 µs   1556.8 µs       5.1x Nisaba         Tie (~1.0x)
97. CAD Cross-Hatch Pattern (80 Angled Lines)   25.9 µs  907.0 µs    932.9 µs   41.3 µs 1022.5 µs   1063.8 µs       1.6x Nisaba        1.14x Nisaba
98. Floating Action Button (FAB 10 Buttons)   38.8 µs  957.7 µs    996.6 µs  125.7 µs  922.4 µs   1048.1 µs       3.2x Nisaba        1.05x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   32.4 µs  944.4 µs    976.8 µs  121.6 µs  867.9 µs    989.5 µs       3.8x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   22.6 µs 1334.3 µs   1356.9 µs   71.3 µs 1040.6 µs   1111.9 µs       3.1x Nisaba          1.22x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2675.7 µs104085.9 µs 106761.6 µs19094.2 µs101954.6 µs 121048.8 µs      7.14x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 43 suites | Skia won in 28 suites | Ties: 29
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.14x Nisaba
===================================================================================================================================================
```
</details>

### Run 6
<details>
<summary>Click to expand Run 6 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 906.2 µs, Skia: 917.5 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 929.1 µs, Skia: 1047.8 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1011.9 µs, Skia: 1334.2 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1043.8 µs, Skia: 1046.6 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1455.0 µs, Skia: 1307.7 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1259.6 µs, Skia: 1043.8 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1220.6 µs, Skia: 1034.3 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1028.0 µs, Skia: 1007.0 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 987.0 µs, Skia: 1001.3 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 978.3 µs, Skia: 1047.2 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1222.9 µs, Skia: 1522.4 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1515.9 µs, Skia: 1344.6 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2023.8 µs, Skia: 1527.7 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1080.0 µs, Skia: 1289.9 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1202.4 µs, Skia: 1489.4 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1014.5 µs, Skia: 2018.3 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1008.7 µs, Skia: 2097.3 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1067.6 µs, Skia: 1008.6 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 993.5 µs, Skia: 621.5 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1029.1 µs, Skia: 1006.7 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1137.3 µs, Skia: 1523.9 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 999.2 µs, Skia: 2696.5 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 1000.7 µs, Skia: 1305.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1001.6 µs, Skia: 1750.9 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 999.2 µs, Skia: 1057.4 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1335.1 µs, Skia: 1679.0 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1000.1 µs, Skia: 1578.5 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1008.8 µs, Skia: 2060.3 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 1005.8 µs, Skia: 1475.1 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1343.4 µs, Skia: 2596.7 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 986.1 µs, Skia: 1555.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1002.9 µs, Skia: 1563.1 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 985.9 µs, Skia: 1563.8 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 997.8 µs, Skia: 1038.2 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 925.1 µs, Skia: 1042.4 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1008.8 µs, Skia: 1026.4 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 998.6 µs, Skia: 1917.9 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.1 µs, Skia: 1009.0 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 995.4 µs, Skia: 1238.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1000.8 µs, Skia: 1747.1 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1008.6 µs, Skia: 1007.6 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1132.7 µs, Skia: 1098.2 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1050.1 µs, Skia: 989.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1303.9 µs, Skia: 1006.6 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1009.3 µs, Skia: 1120.6 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1217.0 µs, Skia: 1046.3 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1003.9 µs, Skia: 1036.8 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 999.0 µs, Skia: 998.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1016.4 µs, Skia: 1354.3 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1112.7 µs, Skia: 1442.5 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1137.3 µs, Skia: 1095.8 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1312.6 µs, Skia: 1136.4 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1152.8 µs, Skia: 1047.9 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1219.8 µs, Skia: 1129.4 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1122.4 µs, Skia: 1028.7 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1030.0 µs, Skia: 998.2 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1066.5 µs, Skia: 1052.2 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1334.8 µs, Skia: 1138.2 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1528.9 µs, Skia: 1483.0 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 998.5 µs, Skia: 1796.4 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1137.7 µs, Skia: 1007.5 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1038.3 µs, Skia: 997.8 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1139.4 µs, Skia: 973.9 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1170.6 µs, Skia: 999.1 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1039.4 µs, Skia: 1007.6 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 998.9 µs, Skia: 1082.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1004.0 µs, Skia: 1305.0 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 997.2 µs, Skia: 988.7 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1024.1 µs, Skia: 1000.9 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 991.2 µs, Skia: 922.6 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 941.3 µs, Skia: 974.4 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 995.7 µs, Skia: 936.3 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 993.9 µs, Skia: 998.4 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 997.4 µs, Skia: 997.9 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 959.0 µs, Skia: 984.7 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 937.2 µs, Skia: 965.6 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 960.7 µs, Skia: 720.3 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1171.2 µs, Skia: 1093.6 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1020.7 µs, Skia: 1800.0 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1078.7 µs, Skia: 678.1 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 993.8 µs, Skia: 587.3 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 932.3 µs, Skia: 986.7 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 988.9 µs, Skia: 996.4 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1000.3 µs, Skia: 1047.2 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 950.7 µs, Skia: 1111.9 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 957.3 µs, Skia: 1137.8 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 996.7 µs, Skia: 1009.4 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 987.7 µs, Skia: 1008.7 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 994.9 µs, Skia: 991.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 986.8 µs, Skia: 1137.2 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1002.4 µs, Skia: 918.0 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 977.4 µs, Skia: 987.9 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 994.9 µs, Skia: 1259.0 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1118.3 µs, Skia: 1322.0 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1016.2 µs, Skia: 1497.1 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1571.6 µs, Skia: 1584.9 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 943.0 µs, Skia: 1180.3 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1019.9 µs, Skia: 1037.7 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 998.5 µs, Skia: 1045.4 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1428.5 µs, Skia: 1229.7 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          2.6 µs  903.5 µs    906.2 µs   11.2 µs  906.3 µs    917.5 µs       4.2x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            19.7 µs  909.4 µs    929.1 µs   41.2 µs 1006.6 µs   1047.8 µs       2.1x Nisaba        1.13x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      26.7 µs  985.2 µs   1011.9 µs  260.9 µs 1073.3 µs   1334.2 µs       9.8x Nisaba        1.32x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      29.4 µs 1014.4 µs   1043.8 µs   73.7 µs  972.9 µs   1046.6 µs       2.5x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   23.7 µs 1431.4 µs   1455.0 µs   71.6 µs 1236.1 µs   1307.7 µs       3.0x Nisaba          1.11x Skia
6. 50 Simple Rounded Rects (12px Radius)     17.4 µs 1242.2 µs   1259.6 µs   56.3 µs  987.6 µs   1043.8 µs       3.2x Nisaba          1.21x Skia
7. 50 Varying-Radius Rounded Rects           16.8 µs 1203.8 µs   1220.6 µs   46.8 µs  987.5 µs   1034.3 µs       2.8x Nisaba          1.18x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   20.7 µs 1007.4 µs   1028.0 µs   58.3 µs  948.6 µs   1007.0 µs       2.8x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          20.8 µs  966.3 µs    987.0 µs   73.5 µs  927.8 µs   1001.3 µs       3.5x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         39.6 µs  938.6 µs    978.3 µs   38.5 µs 1008.6 µs   1047.2 µs               Tie        1.07x Nisaba
11. 50 Two-Stop Linear Gradients             18.8 µs 1204.1 µs   1222.9 µs  199.3 µs 1323.1 µs   1522.4 µs      10.6x Nisaba        1.24x Nisaba
12. 50 Diagonal Gradients (45° Angle)       18.8 µs 1497.1 µs   1515.9 µs  191.6 µs 1153.0 µs   1344.6 µs      10.2x Nisaba          1.13x Skia
13. 25 Radial Glow Gradients (300px)         15.1 µs 2008.7 µs   2023.8 µs  103.5 µs 1424.2 µs   1527.7 µs       6.9x Nisaba          1.32x Skia
14. 20 Box Blur / Drop Shadows (15px)        12.6 µs 1067.4 µs   1080.0 µs  101.0 µs 1188.9 µs   1289.9 µs       8.0x Nisaba        1.19x Nisaba
15. 10 Concave 10-Point Stars (Fill)         48.9 µs 1153.5 µs   1202.4 µs  386.8 µs 1102.6 µs   1489.4 µs       7.9x Nisaba        1.24x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   32.3 µs  982.3 µs   1014.5 µs  881.1 µs 1137.2 µs   2018.3 µs      27.3x Nisaba        1.99x Nisaba
17. 1000-Point Waveform (Dense Stroke)       90.9 µs  917.8 µs   1008.7 µs  868.6 µs 1228.7 µs   2097.3 µs       9.6x Nisaba        2.08x Nisaba
18. 50 Rotated Cards (Affine Transforms)     19.1 µs 1048.5 µs   1067.6 µs   69.6 µs  939.0 µs   1008.6 µs       3.6x Nisaba          1.06x Skia
19. 25 Scissor Clipped Viewports             14.8 µs  978.7 µs    993.5 µs   68.5 µs  553.1 µs    621.5 µs       4.6x Nisaba          1.60x Skia
20. 50 UI Text Labels (Inter-Regular)       104.0 µs  925.1 µs   1029.1 µs   98.2 µs  908.5 µs   1006.7 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  102.1 µs 1035.2 µs   1137.3 µs  366.3 µs 1157.6 µs   1523.9 µs       3.6x Nisaba        1.34x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     39.4 µs  959.8 µs    999.2 µs 1276.0 µs 1420.5 µs   2696.5 µs      32.3x Nisaba        2.70x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       14.2 µs  986.5 µs   1000.7 µs  201.8 µs 1104.0 µs   1305.8 µs      14.2x Nisaba        1.30x Nisaba
24. Circular Arcs with arcTo (20 Corners)    28.9 µs  972.7 µs   1001.6 µs  499.2 µs 1251.7 µs   1750.9 µs      17.3x Nisaba        1.75x Nisaba
25. Directional Arcs with arc() (30 Arcs)    57.6 µs  941.6 µs    999.2 µs  147.0 µs  910.4 µs   1057.4 µs       2.6x Nisaba        1.06x Nisaba
26. Compound Path with Holes (15 Donuts)     89.2 µs 1246.0 µs   1335.1 µs  523.3 µs 1155.7 µs   1679.0 µs       5.9x Nisaba        1.26x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    40.6 µs  959.5 µs   1000.1 µs  504.7 µs 1073.9 µs   1578.5 µs      12.4x Nisaba        1.58x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    47.2 µs  961.6 µs   1008.8 µs  848.1 µs 1212.2 µs   2060.3 µs      18.0x Nisaba        2.04x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)    9.8 µs  996.0 µs   1005.8 µs  279.7 µs 1195.4 µs   1475.1 µs      28.6x Nisaba        1.47x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      27.5 µs 1316.0 µs   1343.4 µs 1232.4 µs 1364.2 µs   2596.7 µs      44.9x Nisaba        1.93x Nisaba
31. Stroke Join: Miter (12px Stroke)         10.3 µs  975.8 µs    986.1 µs  386.5 µs 1168.6 µs   1555.1 µs      37.5x Nisaba        1.58x Nisaba
32. Stroke Join: Bevel (12px Stroke)         12.0 µs  991.0 µs   1002.9 µs  398.1 µs 1165.0 µs   1563.1 µs      33.3x Nisaba        1.56x Nisaba
33. Stroke Join: Round (12px Stroke)         15.9 µs  970.1 µs    985.9 µs  404.0 µs 1159.8 µs   1563.8 µs      25.5x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      17.3 µs  980.6 µs    997.8 µs   46.5 µs  991.7 µs   1038.2 µs       2.7x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    14.8 µs  910.2 µs    925.1 µs   44.7 µs  997.7 µs   1042.4 µs       3.0x Nisaba        1.13x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     39.3 µs  969.5 µs   1008.8 µs   64.1 µs  962.3 µs   1026.4 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   17.6 µs  981.0 µs    998.6 µs  693.6 µs 1224.4 µs   1917.9 µs      39.5x Nisaba        1.92x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   22.1 µs  974.9 µs    997.1 µs   74.3 µs  934.7 µs   1009.0 µs       3.4x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   12.1 µs  983.2 µs    995.4 µs  150.6 µs 1087.9 µs   1238.5 µs      12.4x Nisaba        1.24x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   28.3 µs  972.5 µs   1000.8 µs  546.9 µs 1200.2 µs   1747.1 µs      19.3x Nisaba        1.75x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          16.4 µs  992.2 µs   1008.6 µs   41.3 µs  966.3 µs   1007.6 µs       2.5x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       15.7 µs 1116.9 µs   1132.7 µs   44.2 µs 1054.0 µs   1098.2 µs       2.8x Nisaba         Tie (~1.0x)
43. 50 Pill / Capsule Badges (r=h/2)         17.1 µs 1033.0 µs   1050.1 µs   65.0 µs  924.6 µs    989.6 µs       3.8x Nisaba          1.06x Skia
44. 50 Asymmetric Teardrop Rounded Rects     18.6 µs 1285.2 µs   1303.9 µs   46.8 µs  959.8 µs   1006.6 µs       2.5x Nisaba          1.30x Skia
45. High-Density Disks Cloud (500 Disks)     66.3 µs  943.1 µs   1009.3 µs  212.0 µs  908.5 µs   1120.6 µs       3.2x Nisaba        1.11x Nisaba
46. High-Density Stroked Rings (250 Rings)   36.2 µs 1180.8 µs   1217.0 µs  133.5 µs  912.8 µs   1046.3 µs       3.7x Nisaba          1.16x Skia
47. 250 Filled Rectangles Batch              38.1 µs  965.7 µs   1003.9 µs  128.4 µs  908.4 µs   1036.8 µs       3.4x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      35.7 µs  963.3 µs    999.0 µs   81.2 µs  917.1 µs    998.3 µs       2.3x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                84.8 µs  931.6 µs   1016.4 µs  309.4 µs 1044.9 µs   1354.3 µs       3.6x Nisaba        1.33x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      79.8 µs 1032.9 µs   1112.7 µs  316.0 µs 1126.5 µs   1442.5 µs       4.0x Nisaba        1.30x Nisaba
51. 30 Vertical Card Linear Gradients        15.3 µs 1122.0 µs   1137.3 µs  144.4 µs  951.4 µs   1095.8 µs       9.4x Nisaba         Tie (~1.0x)
52. 30 Horizontal Bar Progress Gradients     15.4 µs 1297.2 µs   1312.6 µs  148.0 µs  988.4 µs   1136.4 µs       9.6x Nisaba          1.15x Skia
53. 15 Concentric Radial Spotlights          11.1 µs 1141.7 µs   1152.8 µs   94.5 µs  953.5 µs   1047.9 µs       8.5x Nisaba          1.10x Skia
54. 25 Soft Ambient Card Shadows             13.9 µs 1205.9 µs   1219.8 µs  120.5 µs 1008.9 µs   1129.4 µs       8.7x Nisaba          1.08x Skia
55. 15 Neon Button Glows (Cyan Intense)      10.4 µs 1112.0 µs   1122.4 µs   84.1 µs  944.6 µs   1028.7 µs       8.1x Nisaba          1.09x Skia
56. 16 Multi-Angle Gradient Fan Slices       26.0 µs 1004.0 µs   1030.0 µs  103.0 µs  895.2 µs    998.2 µs       4.0x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     9.3 µs 1057.2 µs   1066.5 µs   34.0 µs 1018.2 µs   1052.2 µs       3.7x Nisaba         Tie (~1.0x)
58. 20 Inset Well Shadows (Recessed)         51.9 µs 1282.8 µs   1334.8 µs  108.7 µs 1029.5 µs   1138.2 µs       2.1x Nisaba          1.17x Skia
59. 10 Pulsing Circular Radar Wave Glows     14.1 µs 1514.8 µs   1528.9 µs   67.5 µs 1415.6 µs   1483.0 µs       4.8x Nisaba         Tie (~1.0x)
60. Waveform Polyline Stroked with Gradient   49.0 µs  949.5 µs    998.5 µs  568.1 µs 1228.3 µs   1796.4 µs      11.6x Nisaba        1.80x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   11.1 µs 1126.6 µs   1137.7 µs   33.2 µs  974.3 µs   1007.5 µs       3.0x Nisaba          1.13x Skia
62. Scaled Image Quad (2x 512x512 Blit)       9.3 µs 1029.0 µs   1038.3 µs   30.6 µs  967.2 µs    997.8 µs       3.3x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      10.4 µs 1129.0 µs   1139.4 µs   38.7 µs  935.2 µs    973.9 µs       3.7x Nisaba          1.17x Skia
64. Translucent Texture Overlay (50% Alpha)   11.3 µs 1159.3 µs   1170.6 µs   35.9 µs  963.2 µs    999.1 µs       3.2x Nisaba          1.17x Skia
65. Multi-Avatar Grid (16 Avatars)           14.2 µs 1025.2 µs   1039.4 µs   41.2 µs  966.4 µs   1007.6 µs       2.9x Nisaba         Tie (~1.0x)
66. Texture Pattern on Star Path             19.0 µs  979.9 µs    998.9 µs  138.9 µs  943.3 µs   1082.2 µs       7.3x Nisaba        1.08x Nisaba
67. 20px Curved Path Stroked with Texture    16.0 µs  988.0 µs   1004.0 µs  167.9 µs 1137.1 µs   1305.0 µs      10.5x Nisaba        1.30x Nisaba
68. Minified Texture Quad (0.25x Downscale)   18.8 µs  978.4 µs    997.2 µs   66.6 µs  922.1 µs    988.7 µs       3.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        23.0 µs 1001.1 µs   1024.1 µs   62.5 µs  938.4 µs   1000.9 µs       2.7x Nisaba         Tie (~1.0x)
70. Pure Rotation Cluster (36 Spokes 10°)   17.1 µs  974.1 µs    991.2 µs   55.8 µs  866.8 µs    922.6 µs       3.3x Nisaba          1.07x Skia
71. Pure Scale Zoom Progression (20 Rects)   23.5 µs  917.9 µs    941.3 µs   40.2 µs  934.1 µs    974.4 µs       1.7x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   20.1 µs  975.6 µs    995.7 µs   51.1 µs  885.2 µs    936.3 µs       2.5x Nisaba          1.06x Skia
73. Combined Affine Transforms (30 Shapes)   16.7 µs  977.1 µs    993.9 µs   45.6 µs  952.7 µs    998.4 µs       2.7x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   15.4 µs  982.0 µs    997.4 µs   42.8 µs  955.1 µs    997.9 µs       2.8x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       19.7 µs  939.3 µs    959.0 µs   59.7 µs  925.1 µs    984.7 µs       3.0x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   14.5 µs  922.7 µs    937.2 µs   46.9 µs  918.7 µs    965.6 µs       3.2x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        12.5 µs  948.2 µs    960.7 µs   63.1 µs  657.2 µs    720.3 µs       5.1x Nisaba          1.33x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   16.2 µs 1155.0 µs   1171.2 µs   41.7 µs 1051.9 µs   1093.6 µs       2.6x Nisaba          1.07x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   98.1 µs  922.6 µs   1020.7 µs  868.5 µs  931.5 µs   1800.0 µs       8.9x Nisaba        1.76x Nisaba
80. Scissored Gradient Card (Clipped)        13.7 µs 1065.0 µs   1078.7 µs   88.6 µs  589.5 µs    678.1 µs       6.4x Nisaba          1.59x Skia
81. Circular Overflow Clip (25 Boxes)        17.0 µs  976.8 µs    993.8 µs   69.3 µs  518.0 µs    587.3 µs       4.1x Nisaba          1.69x Skia
82. Scissor Invalidation / Reset (25 Cycles)   14.1 µs  918.2 µs    932.3 µs   46.7 µs  939.9 µs    986.7 µs       3.3x Nisaba        1.06x Nisaba
83. Composite Op: Source-Over (30 Shapes)    14.6 µs  974.3 µs    988.9 µs   40.6 µs  955.9 µs    996.4 µs       2.8x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   14.7 µs  985.7 µs   1000.3 µs   45.8 µs 1001.4 µs   1047.2 µs       3.1x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   13.2 µs  937.5 µs    950.7 µs   43.8 µs 1068.0 µs   1111.9 µs       3.3x Nisaba        1.17x Nisaba
86. Composite Op: Source-Out (Cutout)        15.2 µs  942.1 µs    957.3 µs   41.5 µs 1096.3 µs   1137.8 µs       2.7x Nisaba        1.19x Nisaba
87. Composite Op: Atop (Target Bounds)       15.6 µs  981.1 µs    996.7 µs   43.8 µs  965.6 µs   1009.4 µs       2.8x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   13.8 µs  973.9 µs    987.7 µs   43.6 µs  965.1 µs   1008.7 µs       3.2x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     12.8 µs  982.1 µs    994.9 µs   46.5 µs  944.8 µs    991.3 µs       3.6x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        13.0 µs  973.9 µs    986.8 µs   45.3 µs 1091.9 µs   1137.2 µs       3.5x Nisaba        1.15x Nisaba
91. Composite Op: Xor (Exclusive Blend)      13.6 µs  988.8 µs   1002.4 µs   44.5 µs  873.5 µs    918.0 µs       3.3x Nisaba          1.09x Skia
92. Composite Op: Copy (Direct Overwrite)    16.4 µs  961.0 µs    977.4 µs   43.3 µs  944.6 µs    987.9 µs       2.6x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   26.1 µs  968.8 µs    994.9 µs  199.1 µs 1059.9 µs   1259.0 µs       7.6x Nisaba        1.27x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      32.6 µs 1085.7 µs   1118.3 µs  269.1 µs 1052.9 µs   1322.0 µs       8.3x Nisaba        1.18x Nisaba
95. Circular Progress Rings (8 Meters)       42.0 µs  974.3 µs   1016.2 µs  373.3 µs 1123.9 µs   1497.1 µs       8.9x Nisaba        1.47x Nisaba
96. Modern Card Stack (5 Elevated Cards)     16.3 µs 1555.3 µs   1571.6 µs   81.1 µs 1503.8 µs   1584.9 µs       5.0x Nisaba         Tie (~1.0x)
97. CAD Cross-Hatch Pattern (80 Angled Lines)   22.2 µs  920.8 µs    943.0 µs   42.7 µs 1137.6 µs   1180.3 µs       1.9x Nisaba        1.25x Nisaba
98. Floating Action Button (FAB 10 Buttons)   39.1 µs  980.8 µs   1019.9 µs  122.8 µs  914.9 µs   1037.7 µs       3.1x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   34.2 µs  964.2 µs    998.5 µs  119.3 µs  926.1 µs   1045.4 µs       3.5x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   20.1 µs 1408.4 µs   1428.5 µs   72.4 µs 1157.3 µs   1229.7 µs       3.6x Nisaba          1.16x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2671.3 µs105250.5 µs 107921.8 µs19092.0 µs102564.2 µs 121656.2 µs      7.15x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 40 suites | Skia won in 26 suites | Ties: 34
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.15x Nisaba
===================================================================================================================================================
```
</details>

### Run 7
<details>
<summary>Click to expand Run 7 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 907.7 µs, Skia: 936.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 925.1 µs, Skia: 1047.4 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1005.5 µs, Skia: 1338.2 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1065.7 µs, Skia: 1049.6 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1471.3 µs, Skia: 1405.1 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1267.3 µs, Skia: 1046.4 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1289.8 µs, Skia: 1041.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1029.1 µs, Skia: 1038.2 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 988.5 µs, Skia: 1008.1 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1053.6 µs, Skia: 1042.9 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1370.3 µs, Skia: 1432.6 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1424.2 µs, Skia: 1325.8 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 1883.0 µs, Skia: 1530.7 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1151.7 µs, Skia: 1289.8 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1021.2 µs, Skia: 1493.1 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1001.2 µs, Skia: 2026.5 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1028.4 µs, Skia: 2082.8 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1066.7 µs, Skia: 1041.5 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1001.2 µs, Skia: 603.2 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1027.5 µs, Skia: 1037.3 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1242.8 µs, Skia: 1541.1 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 986.6 µs, Skia: 2652.7 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 998.8 µs, Skia: 1319.7 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1003.6 µs, Skia: 1752.9 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1007.5 µs, Skia: 1097.0 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1398.8 µs, Skia: 1668.6 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 997.5 µs, Skia: 1597.5 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 998.6 µs, Skia: 2047.0 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 993.0 µs, Skia: 1464.5 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1322.1 µs, Skia: 2505.9 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1065.6 µs, Skia: 1556.0 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 998.5 µs, Skia: 1557.2 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 997.6 µs, Skia: 1577.7 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 999.0 µs, Skia: 1087.5 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 967.0 µs, Skia: 1019.1 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1008.2 µs, Skia: 999.2 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 949.9 µs, Skia: 1951.5 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.8 µs, Skia: 1037.0 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1006.2 µs, Skia: 1273.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 999.6 µs, Skia: 1735.9 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 997.8 µs, Skia: 997.8 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1282.6 µs, Skia: 1087.4 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1136.1 µs, Skia: 1020.2 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1303.3 µs, Skia: 1011.4 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1004.4 µs, Skia: 1169.5 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1162.2 µs, Skia: 1035.8 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1026.7 µs, Skia: 1086.7 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 998.2 µs, Skia: 1036.2 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1014.0 µs, Skia: 1318.7 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1081.2 µs, Skia: 1423.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1286.8 µs, Skia: 1162.4 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1439.9 µs, Skia: 1137.7 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1123.8 µs, Skia: 1056.4 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1257.0 µs, Skia: 1183.3 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1122.4 µs, Skia: 1113.0 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1006.9 µs, Skia: 1026.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1113.2 µs, Skia: 986.4 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1461.3 µs, Skia: 1173.8 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1642.9 µs, Skia: 1448.1 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1013.3 µs, Skia: 1832.6 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1149.9 µs, Skia: 1009.7 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1012.6 µs, Skia: 986.9 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1017.7 µs, Skia: 995.9 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1058.9 µs, Skia: 996.1 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1005.7 µs, Skia: 975.1 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1002.8 µs, Skia: 1092.3 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 997.3 µs, Skia: 1298.6 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1066.8 µs, Skia: 987.2 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1038.0 µs, Skia: 1058.4 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 941.7 µs, Skia: 915.3 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 954.5 µs, Skia: 992.4 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 994.8 µs, Skia: 1011.1 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 979.9 µs, Skia: 987.7 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 988.4 µs, Skia: 997.7 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 936.8 µs, Skia: 997.4 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 966.0 µs, Skia: 989.6 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 981.0 µs, Skia: 700.2 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1202.5 µs, Skia: 1099.4 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1050.5 µs, Skia: 1786.9 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1047.9 µs, Skia: 635.0 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 998.2 µs, Skia: 575.9 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1000.3 µs, Skia: 1017.9 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 978.0 µs, Skia: 999.5 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 998.4 µs, Skia: 987.7 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 983.2 µs, Skia: 1125.4 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 975.1 µs, Skia: 1109.9 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 987.9 µs, Skia: 988.5 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 999.0 µs, Skia: 997.8 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 999.5 µs, Skia: 965.2 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 992.3 µs, Skia: 1102.2 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 997.8 µs, Skia: 997.7 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 977.5 µs, Skia: 1009.9 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 996.6 µs, Skia: 1235.8 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1182.0 µs, Skia: 1342.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1023.8 µs, Skia: 1497.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1559.1 µs, Skia: 1526.4 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 946.7 µs, Skia: 1058.5 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1039.8 µs, Skia: 1078.2 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 998.3 µs, Skia: 1067.6 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1375.9 µs, Skia: 1205.9 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          2.5 µs  905.3 µs    907.7 µs   10.5 µs  926.4 µs    936.9 µs       4.3x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            18.7 µs  906.4 µs    925.1 µs   43.2 µs 1004.2 µs   1047.4 µs       2.3x Nisaba        1.13x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      28.7 µs  976.8 µs   1005.5 µs  253.2 µs 1085.0 µs   1338.2 µs       8.8x Nisaba        1.33x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      29.9 µs 1035.7 µs   1065.7 µs   74.1 µs  975.5 µs   1049.6 µs       2.5x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   24.9 µs 1446.4 µs   1471.3 µs   74.7 µs 1330.4 µs   1405.1 µs       3.0x Nisaba         Tie (~1.0x)
6. 50 Simple Rounded Rects (12px Radius)     17.6 µs 1249.7 µs   1267.3 µs   57.1 µs  989.3 µs   1046.4 µs       3.3x Nisaba          1.21x Skia
7. 50 Varying-Radius Rounded Rects           17.6 µs 1272.2 µs   1289.8 µs   47.1 µs  994.4 µs   1041.5 µs       2.7x Nisaba          1.24x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   22.2 µs 1006.9 µs   1029.1 µs   58.1 µs  980.1 µs   1038.2 µs       2.6x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          20.6 µs  967.9 µs    988.5 µs   70.0 µs  938.1 µs   1008.1 µs       3.4x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         39.6 µs 1013.9 µs   1053.6 µs   41.8 µs 1001.1 µs   1042.9 µs       1.1x Nisaba         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             17.6 µs 1352.7 µs   1370.3 µs  190.0 µs 1242.5 µs   1432.6 µs      10.8x Nisaba         Tie (~1.0x)
12. 50 Diagonal Gradients (45° Angle)       17.1 µs 1407.1 µs   1424.2 µs  187.8 µs 1138.0 µs   1325.8 µs      11.0x Nisaba          1.07x Skia
13. 25 Radial Glow Gradients (300px)         14.7 µs 1868.3 µs   1883.0 µs  112.2 µs 1418.5 µs   1530.7 µs       7.6x Nisaba          1.23x Skia
14. 20 Box Blur / Drop Shadows (15px)        13.7 µs 1138.0 µs   1151.7 µs  100.2 µs 1189.6 µs   1289.8 µs       7.3x Nisaba        1.12x Nisaba
15. 10 Concave 10-Point Stars (Fill)         44.4 µs  976.8 µs   1021.2 µs  387.4 µs 1105.7 µs   1493.1 µs       8.7x Nisaba        1.46x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   27.7 µs  973.5 µs   1001.2 µs  890.4 µs 1136.1 µs   2026.5 µs      32.1x Nisaba        2.02x Nisaba
17. 1000-Point Waveform (Dense Stroke)       93.2 µs  935.1 µs   1028.4 µs  862.7 µs 1220.2 µs   2082.8 µs       9.3x Nisaba        2.03x Nisaba
18. 50 Rotated Cards (Affine Transforms)     17.6 µs 1049.1 µs   1066.7 µs   64.8 µs  976.7 µs   1041.5 µs       3.7x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             12.4 µs  988.8 µs   1001.2 µs   67.5 µs  535.8 µs    603.2 µs       5.4x Nisaba          1.66x Skia
20. 50 UI Text Labels (Inter-Regular)        96.3 µs  931.2 µs   1027.5 µs   98.1 µs  939.2 µs   1037.3 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  105.0 µs 1137.8 µs   1242.8 µs  377.1 µs 1164.0 µs   1541.1 µs       3.6x Nisaba        1.24x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     39.4 µs  947.2 µs    986.6 µs 1272.4 µs 1380.3 µs   2652.7 µs      32.3x Nisaba        2.69x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       17.5 µs  981.2 µs    998.8 µs  187.7 µs 1132.1 µs   1319.7 µs      10.7x Nisaba        1.32x Nisaba
24. Circular Arcs with arcTo (20 Corners)    31.6 µs  972.0 µs   1003.6 µs  532.9 µs 1220.0 µs   1752.9 µs      16.9x Nisaba        1.75x Nisaba
25. Directional Arcs with arc() (30 Arcs)    58.2 µs  949.2 µs   1007.5 µs  151.0 µs  946.0 µs   1097.0 µs       2.6x Nisaba        1.09x Nisaba
26. Compound Path with Holes (15 Donuts)     83.8 µs 1314.9 µs   1398.8 µs  502.4 µs 1166.2 µs   1668.6 µs       6.0x Nisaba        1.19x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    41.1 µs  956.4 µs    997.5 µs  481.9 µs 1115.6 µs   1597.5 µs      11.7x Nisaba        1.60x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    47.2 µs  951.4 µs    998.6 µs  847.1 µs 1199.9 µs   2047.0 µs      18.0x Nisaba        2.05x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   11.3 µs  981.7 µs    993.0 µs  283.0 µs 1181.5 µs   1464.5 µs      25.0x Nisaba        1.47x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      24.9 µs 1297.1 µs   1322.1 µs 1164.2 µs 1341.7 µs   2505.9 µs      46.7x Nisaba        1.90x Nisaba
31. Stroke Join: Miter (12px Stroke)         12.7 µs 1052.9 µs   1065.6 µs  395.8 µs 1160.2 µs   1556.0 µs      31.1x Nisaba        1.46x Nisaba
32. Stroke Join: Bevel (12px Stroke)         11.6 µs  986.9 µs    998.5 µs  393.2 µs 1164.0 µs   1557.2 µs      33.9x Nisaba        1.56x Nisaba
33. Stroke Join: Round (12px Stroke)         15.6 µs  982.0 µs    997.6 µs  408.4 µs 1169.4 µs   1577.7 µs      26.2x Nisaba        1.58x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      13.5 µs  985.5 µs    999.0 µs   48.8 µs 1038.7 µs   1087.5 µs       3.6x Nisaba        1.09x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    15.8 µs  951.1 µs    967.0 µs   45.6 µs  973.5 µs   1019.1 µs       2.9x Nisaba        1.05x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     36.6 µs  971.6 µs   1008.2 µs   61.3 µs  937.9 µs    999.2 µs       1.7x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   17.2 µs  932.7 µs    949.9 µs  708.9 µs 1242.5 µs   1951.5 µs      41.2x Nisaba        2.05x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   18.3 µs  979.5 µs    997.8 µs   68.4 µs  968.6 µs   1037.0 µs       3.7x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   13.2 µs  993.0 µs   1006.2 µs  160.6 µs 1112.8 µs   1273.5 µs      12.2x Nisaba        1.27x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   24.7 µs  974.9 µs    999.6 µs  555.3 µs 1180.6 µs   1735.9 µs      22.5x Nisaba        1.74x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          16.5 µs  981.3 µs    997.8 µs   42.8 µs  955.0 µs    997.8 µs       2.6x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       17.3 µs 1265.3 µs   1282.6 µs   40.6 µs 1046.9 µs   1087.4 µs       2.3x Nisaba          1.18x Skia
43. 50 Pill / Capsule Badges (r=h/2)         19.0 µs 1117.1 µs   1136.1 µs   65.0 µs  955.2 µs   1020.2 µs       3.4x Nisaba          1.11x Skia
44. 50 Asymmetric Teardrop Rounded Rects     18.9 µs 1284.3 µs   1303.3 µs   43.9 µs  967.5 µs   1011.4 µs       2.3x Nisaba          1.29x Skia
45. High-Density Disks Cloud (500 Disks)     60.4 µs  944.0 µs   1004.4 µs  219.0 µs  950.5 µs   1169.5 µs       3.6x Nisaba        1.16x Nisaba
46. High-Density Stroked Rings (250 Rings)   31.2 µs 1131.0 µs   1162.2 µs  134.5 µs  901.3 µs   1035.8 µs       4.3x Nisaba          1.12x Skia
47. 250 Filled Rectangles Batch              38.7 µs  988.0 µs   1026.7 µs  131.6 µs  955.2 µs   1086.7 µs       3.4x Nisaba        1.06x Nisaba
48. 150 Stroked Rectangles (2px Border)      40.2 µs  958.0 µs    998.2 µs   81.5 µs  954.7 µs   1036.2 µs       2.0x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                81.6 µs  932.4 µs   1014.0 µs  302.1 µs 1016.5 µs   1318.7 µs       3.7x Nisaba        1.30x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      79.1 µs 1002.1 µs   1081.2 µs  310.2 µs 1113.2 µs   1423.4 µs       3.9x Nisaba        1.32x Nisaba
51. 30 Vertical Card Linear Gradients        14.9 µs 1271.9 µs   1286.8 µs  157.2 µs 1005.3 µs   1162.4 µs      10.6x Nisaba          1.11x Skia
52. 30 Horizontal Bar Progress Gradients     15.6 µs 1424.3 µs   1439.9 µs  151.4 µs  986.3 µs   1137.7 µs       9.7x Nisaba          1.27x Skia
53. 15 Concentric Radial Spotlights          11.4 µs 1112.4 µs   1123.8 µs   94.5 µs  961.8 µs   1056.4 µs       8.3x Nisaba          1.06x Skia
54. 25 Soft Ambient Card Shadows             13.4 µs 1243.6 µs   1257.0 µs  123.9 µs 1059.4 µs   1183.3 µs       9.3x Nisaba          1.06x Skia
55. 15 Neon Button Glows (Cyan Intense)      11.5 µs 1110.9 µs   1122.4 µs   90.5 µs 1022.5 µs   1113.0 µs       7.9x Nisaba         Tie (~1.0x)
56. 16 Multi-Angle Gradient Fan Slices       28.3 µs  978.7 µs   1006.9 µs  106.3 µs  920.4 µs   1026.7 µs       3.8x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     8.7 µs 1104.5 µs   1113.2 µs   31.3 µs  955.1 µs    986.4 µs       3.6x Nisaba          1.13x Skia
58. 20 Inset Well Shadows (Recessed)         55.1 µs 1406.1 µs   1461.3 µs  108.8 µs 1065.0 µs   1173.8 µs       2.0x Nisaba          1.24x Skia
59. 10 Pulsing Circular Radar Wave Glows     13.0 µs 1629.9 µs   1642.9 µs   68.7 µs 1379.4 µs   1448.1 µs       5.3x Nisaba          1.13x Skia
60. Waveform Polyline Stroked with Gradient   46.9 µs  966.3 µs   1013.3 µs  586.9 µs 1245.7 µs   1832.6 µs      12.5x Nisaba        1.81x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)    9.1 µs 1140.8 µs   1149.9 µs   32.6 µs  977.1 µs   1009.7 µs       3.6x Nisaba          1.14x Skia
62. Scaled Image Quad (2x 512x512 Blit)       7.6 µs 1005.0 µs   1012.6 µs   32.9 µs  954.0 µs    986.9 µs       4.3x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle       7.9 µs 1009.8 µs   1017.7 µs   40.3 µs  955.6 µs    995.9 µs       5.1x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)    9.6 µs 1049.4 µs   1058.9 µs   38.3 µs  957.7 µs    996.1 µs       4.0x Nisaba          1.06x Skia
65. Multi-Avatar Grid (16 Avatars)           11.6 µs  994.2 µs   1005.7 µs   45.2 µs  929.9 µs    975.1 µs       3.9x Nisaba         Tie (~1.0x)
66. Texture Pattern on Star Path             21.1 µs  981.7 µs   1002.8 µs  143.2 µs  949.1 µs   1092.3 µs       6.8x Nisaba        1.09x Nisaba
67. 20px Curved Path Stroked with Texture    13.5 µs  983.8 µs    997.3 µs  169.0 µs 1129.6 µs   1298.6 µs      12.5x Nisaba        1.30x Nisaba
68. Minified Texture Quad (0.25x Downscale)   20.7 µs 1046.0 µs   1066.8 µs   72.2 µs  915.0 µs    987.2 µs       3.5x Nisaba          1.08x Skia
69. Pure Translation Stack (50 Cards)        23.5 µs 1014.5 µs   1038.0 µs   64.1 µs  994.3 µs   1058.4 µs       2.7x Nisaba         Tie (~1.0x)
70. Pure Rotation Cluster (36 Spokes 10°)   19.1 µs  922.6 µs    941.7 µs   54.2 µs  861.1 µs    915.3 µs       2.8x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   23.8 µs  930.7 µs    954.5 µs   34.0 µs  958.4 µs    992.4 µs       1.4x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   17.9 µs  976.9 µs    994.8 µs   48.3 µs  962.8 µs   1011.1 µs       2.7x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   17.8 µs  962.1 µs    979.9 µs   44.6 µs  943.2 µs    987.7 µs       2.5x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   13.9 µs  974.4 µs    988.4 µs   40.5 µs  957.2 µs    997.7 µs       2.9x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       19.0 µs  917.9 µs    936.8 µs   54.8 µs  942.6 µs    997.4 µs       2.9x Nisaba        1.06x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   17.1 µs  948.9 µs    966.0 µs   45.7 µs  943.9 µs    989.6 µs       2.7x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        11.5 µs  969.5 µs    981.0 µs   57.2 µs  643.1 µs    700.2 µs       5.0x Nisaba          1.40x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   12.8 µs 1189.7 µs   1202.5 µs   36.3 µs 1063.1 µs   1099.4 µs       2.8x Nisaba          1.09x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   92.1 µs  958.4 µs   1050.5 µs  904.3 µs  882.6 µs   1786.9 µs       9.8x Nisaba        1.70x Nisaba
80. Scissored Gradient Card (Clipped)        11.2 µs 1036.8 µs   1047.9 µs   83.9 µs  551.1 µs    635.0 µs       7.5x Nisaba          1.65x Skia
81. Circular Overflow Clip (25 Boxes)        14.9 µs  983.3 µs    998.2 µs   72.2 µs  503.8 µs    575.9 µs       4.8x Nisaba          1.73x Skia
82. Scissor Invalidation / Reset (25 Cycles)   12.9 µs  987.5 µs   1000.3 µs   47.6 µs  970.3 µs   1017.9 µs       3.7x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    15.7 µs  962.4 µs    978.0 µs   44.2 µs  955.3 µs    999.5 µs       2.8x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   11.7 µs  986.8 µs    998.4 µs   46.1 µs  941.7 µs    987.7 µs       3.9x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   14.9 µs  968.3 µs    983.2 µs   40.8 µs 1084.7 µs   1125.4 µs       2.7x Nisaba        1.14x Nisaba
86. Composite Op: Source-Out (Cutout)        13.5 µs  961.6 µs    975.1 µs   44.0 µs 1065.9 µs   1109.9 µs       3.3x Nisaba        1.14x Nisaba
87. Composite Op: Atop (Target Bounds)       12.5 µs  975.4 µs    987.9 µs   47.3 µs  941.2 µs    988.5 µs       3.8x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   13.2 µs  985.8 µs    999.0 µs   44.6 µs  953.2 µs    997.8 µs       3.4x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     14.0 µs  985.5 µs    999.5 µs   48.7 µs  916.5 µs    965.2 µs       3.5x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        14.3 µs  977.9 µs    992.3 µs   42.6 µs 1059.6 µs   1102.2 µs       3.0x Nisaba        1.11x Nisaba
91. Composite Op: Xor (Exclusive Blend)      14.2 µs  983.6 µs    997.8 µs   45.2 µs  952.6 µs    997.7 µs       3.2x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    19.9 µs  957.6 µs    977.5 µs   51.9 µs  958.0 µs   1009.9 µs       2.6x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   25.4 µs  971.2 µs    996.6 µs  198.4 µs 1037.4 µs   1235.8 µs       7.8x Nisaba        1.24x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      29.0 µs 1152.9 µs   1182.0 µs  275.8 µs 1067.1 µs   1342.9 µs       9.5x Nisaba        1.14x Nisaba
95. Circular Progress Rings (8 Meters)       46.3 µs  977.5 µs   1023.8 µs  377.5 µs 1119.9 µs   1497.4 µs       8.2x Nisaba        1.46x Nisaba
96. Modern Card Stack (5 Elevated Cards)     15.4 µs 1543.7 µs   1559.1 µs   82.5 µs 1443.9 µs   1526.4 µs       5.4x Nisaba         Tie (~1.0x)
97. CAD Cross-Hatch Pattern (80 Angled Lines)   24.3 µs  922.4 µs    946.7 µs   42.3 µs 1016.2 µs   1058.5 µs       1.7x Nisaba        1.12x Nisaba
98. Floating Action Button (FAB 10 Buttons)   35.5 µs 1004.3 µs   1039.8 µs  127.4 µs  950.8 µs   1078.2 µs       3.6x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   29.7 µs  968.6 µs    998.3 µs  114.8 µs  952.8 µs   1067.6 µs       3.9x Nisaba        1.07x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   21.5 µs 1354.4 µs   1375.9 µs   75.4 µs 1130.5 µs   1205.9 µs       3.5x Nisaba          1.14x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2617.7 µs106177.0 µs 108794.7 µs19138.3 µs102831.8 µs 121970.1 µs      7.31x Nisaba        1.12x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 41 suites | Skia won in 24 suites | Ties: 35
         Overall Frame Throughput (End-to-End): 1.12x Nisaba
         Overall CPU Command Submission:       7.31x Nisaba
===================================================================================================================================================
```
</details>

### Run 8
<details>
<summary>Click to expand Run 8 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 961.9 µs, Skia: 938.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 992.7 µs, Skia: 1042.8 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 986.2 µs, Skia: 1302.6 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1069.2 µs, Skia: 1047.5 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1344.0 µs, Skia: 1268.7 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1251.9 µs, Skia: 1060.3 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1267.4 µs, Skia: 1028.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1117.7 µs, Skia: 1008.9 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 989.0 µs, Skia: 1007.5 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 983.8 µs, Skia: 1007.2 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1421.4 µs, Skia: 1418.2 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1483.3 µs, Skia: 1358.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 1918.5 µs, Skia: 1490.8 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1090.9 µs, Skia: 1277.6 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1178.9 µs, Skia: 1493.6 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1010.1 µs, Skia: 1998.2 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 999.4 µs, Skia: 2072.2 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1145.7 µs, Skia: 1105.7 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 865.4 µs, Skia: 632.2 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1006.1 µs, Skia: 1018.5 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1111.2 µs, Skia: 1518.1 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1023.7 µs, Skia: 2593.0 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 949.6 µs, Skia: 1286.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 983.2 µs, Skia: 1727.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 998.2 µs, Skia: 1059.8 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1418.7 µs, Skia: 1687.1 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 999.3 µs, Skia: 1591.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 940.1 µs, Skia: 2075.2 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 995.7 µs, Skia: 1478.5 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1316.2 µs, Skia: 2523.9 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1001.0 µs, Skia: 1517.4 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1004.0 µs, Skia: 1567.6 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1004.7 µs, Skia: 1503.1 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 998.4 µs, Skia: 1098.6 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 882.7 µs, Skia: 1013.4 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1008.4 µs, Skia: 1022.5 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 996.9 µs, Skia: 1939.5 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 946.4 µs, Skia: 1007.2 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 997.3 µs, Skia: 1266.2 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1011.8 µs, Skia: 1707.9 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1037.6 µs, Skia: 1047.2 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1246.1 µs, Skia: 1110.7 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1048.7 µs, Skia: 966.2 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1074.8 µs, Skia: 997.7 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1018.2 µs, Skia: 1191.6 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1099.4 µs, Skia: 1067.9 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1033.3 µs, Skia: 1091.7 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 995.9 µs, Skia: 1028.5 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1037.0 µs, Skia: 1350.5 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1078.5 µs, Skia: 1441.1 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1295.9 µs, Skia: 1154.1 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1259.4 µs, Skia: 1085.7 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1277.6 µs, Skia: 1096.5 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1249.3 µs, Skia: 1188.8 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1041.8 µs, Skia: 1072.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 998.0 µs, Skia: 996.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1061.3 µs, Skia: 1004.2 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1346.3 µs, Skia: 1030.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1429.0 µs, Skia: 1305.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 923.6 µs, Skia: 1816.1 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 865.4 µs, Skia: 829.8 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 945.7 µs, Skia: 844.8 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1029.1 µs, Skia: 986.6 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1024.4 µs, Skia: 997.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 987.2 µs, Skia: 987.6 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 999.0 µs, Skia: 1028.1 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1060.9 µs, Skia: 1256.9 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 997.2 µs, Skia: 969.2 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1058.0 µs, Skia: 987.3 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 990.1 µs, Skia: 998.2 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 946.0 µs, Skia: 999.6 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 975.9 µs, Skia: 999.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 998.1 µs, Skia: 996.7 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 990.0 µs, Skia: 984.1 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 970.7 µs, Skia: 996.1 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 917.3 µs, Skia: 993.0 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 957.5 µs, Skia: 697.6 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1037.6 µs, Skia: 1048.1 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1004.2 µs, Skia: 1775.0 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1049.9 µs, Skia: 667.0 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 981.8 µs, Skia: 611.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 998.1 µs, Skia: 997.4 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1023.7 µs, Skia: 1008.6 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1013.5 µs, Skia: 1018.1 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1002.1 µs, Skia: 1308.7 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 978.4 µs, Skia: 1122.5 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1008.5 µs, Skia: 998.1 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 988.8 µs, Skia: 938.5 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 987.4 µs, Skia: 997.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 975.7 µs, Skia: 1128.1 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 976.7 µs, Skia: 999.7 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 955.3 µs, Skia: 1011.5 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 998.7 µs, Skia: 1236.9 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1170.6 µs, Skia: 1342.0 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1068.2 µs, Skia: 1503.0 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1511.5 µs, Skia: 1602.3 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 906.6 µs, Skia: 1057.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1024.7 µs, Skia: 1056.8 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1006.2 µs, Skia: 1057.7 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1449.9 µs, Skia: 1210.5 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          2.4 µs  959.4 µs    961.9 µs   12.4 µs  926.3 µs    938.7 µs       5.1x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            17.5 µs  975.2 µs    992.7 µs   40.6 µs 1002.2 µs   1042.8 µs       2.3x Nisaba        1.05x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      31.2 µs  955.1 µs    986.2 µs  242.8 µs 1059.8 µs   1302.6 µs       7.8x Nisaba        1.32x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      29.9 µs 1039.3 µs   1069.2 µs   78.6 µs  968.8 µs   1047.5 µs       2.6x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   25.7 µs 1318.3 µs   1344.0 µs   75.8 µs 1192.9 µs   1268.7 µs       2.9x Nisaba          1.06x Skia
6. 50 Simple Rounded Rects (12px Radius)     18.6 µs 1233.3 µs   1251.9 µs   58.0 µs 1002.3 µs   1060.3 µs       3.1x Nisaba          1.18x Skia
7. 50 Varying-Radius Rounded Rects           18.7 µs 1248.7 µs   1267.4 µs   46.4 µs  982.1 µs   1028.5 µs       2.5x Nisaba          1.23x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   27.0 µs 1090.7 µs   1117.7 µs   59.5 µs  949.3 µs   1008.9 µs       2.2x Nisaba          1.11x Skia
9. 100 Alpha Circles / Disks (Fill)          20.0 µs  969.0 µs    989.0 µs   73.9 µs  933.6 µs   1007.5 µs       3.7x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         38.4 µs  945.4 µs    983.8 µs   40.4 µs  966.8 µs   1007.2 µs       1.1x Nisaba         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             19.5 µs 1402.0 µs   1421.4 µs  188.1 µs 1230.1 µs   1418.2 µs       9.7x Nisaba         Tie (~1.0x)
12. 50 Diagonal Gradients (45° Angle)       17.2 µs 1466.1 µs   1483.3 µs  194.0 µs 1164.9 µs   1358.9 µs      11.3x Nisaba          1.09x Skia
13. 25 Radial Glow Gradients (300px)         14.5 µs 1904.0 µs   1918.5 µs  107.4 µs 1383.4 µs   1490.8 µs       7.4x Nisaba          1.29x Skia
14. 20 Box Blur / Drop Shadows (15px)        11.5 µs 1079.3 µs   1090.9 µs   97.6 µs 1180.0 µs   1277.6 µs       8.5x Nisaba        1.17x Nisaba
15. 10 Concave 10-Point Stars (Fill)         44.5 µs 1134.4 µs   1178.9 µs  400.0 µs 1093.5 µs   1493.6 µs       9.0x Nisaba        1.27x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   32.2 µs  977.9 µs   1010.1 µs  866.9 µs 1131.3 µs   1998.2 µs      26.9x Nisaba        1.98x Nisaba
17. 1000-Point Waveform (Dense Stroke)       95.5 µs  904.0 µs    999.4 µs  847.3 µs 1224.9 µs   2072.2 µs       8.9x Nisaba        2.07x Nisaba
18. 50 Rotated Cards (Affine Transforms)     20.4 µs 1125.2 µs   1145.7 µs   67.2 µs 1038.5 µs   1105.7 µs       3.3x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             14.4 µs  851.0 µs    865.4 µs   69.1 µs  563.1 µs    632.2 µs       4.8x Nisaba          1.37x Skia
20. 50 UI Text Labels (Inter-Regular)        97.2 µs  908.9 µs   1006.1 µs   98.0 µs  920.5 µs   1018.5 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  104.1 µs 1007.1 µs   1111.2 µs  373.4 µs 1144.6 µs   1518.1 µs       3.6x Nisaba        1.37x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     48.1 µs  975.6 µs   1023.7 µs 1242.7 µs 1350.3 µs   2593.0 µs      25.8x Nisaba        2.53x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       15.4 µs  934.2 µs    949.6 µs  189.5 µs 1097.3 µs   1286.8 µs      12.3x Nisaba        1.36x Nisaba
24. Circular Arcs with arcTo (20 Corners)    27.0 µs  956.2 µs    983.2 µs  517.2 µs 1210.0 µs   1727.3 µs      19.1x Nisaba        1.76x Nisaba
25. Directional Arcs with arc() (30 Arcs)    59.9 µs  938.3 µs    998.2 µs  151.4 µs  908.5 µs   1059.8 µs       2.5x Nisaba        1.06x Nisaba
26. Compound Path with Holes (15 Donuts)     90.7 µs 1328.0 µs   1418.7 µs  539.6 µs 1147.5 µs   1687.1 µs       5.9x Nisaba        1.19x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    40.1 µs  959.2 µs    999.3 µs  491.3 µs 1100.1 µs   1591.4 µs      12.3x Nisaba        1.59x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    46.9 µs  893.2 µs    940.1 µs  841.8 µs 1233.5 µs   2075.2 µs      17.9x Nisaba        2.21x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   10.3 µs  985.5 µs    995.7 µs  267.9 µs 1210.5 µs   1478.5 µs      26.1x Nisaba        1.48x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      31.2 µs 1285.0 µs   1316.2 µs 1173.2 µs 1350.7 µs   2523.9 µs      37.7x Nisaba        1.92x Nisaba
31. Stroke Join: Miter (12px Stroke)         14.1 µs  986.9 µs   1001.0 µs  388.6 µs 1128.8 µs   1517.4 µs      27.5x Nisaba        1.52x Nisaba
32. Stroke Join: Bevel (12px Stroke)         11.2 µs  992.8 µs   1004.0 µs  385.8 µs 1181.8 µs   1567.6 µs      34.5x Nisaba        1.56x Nisaba
33. Stroke Join: Round (12px Stroke)         15.0 µs  989.7 µs   1004.7 µs  422.8 µs 1080.3 µs   1503.1 µs      28.1x Nisaba        1.50x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      12.2 µs  986.3 µs    998.4 µs   48.6 µs 1050.1 µs   1098.6 µs       4.0x Nisaba        1.10x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    14.2 µs  868.5 µs    882.7 µs   49.1 µs  964.3 µs   1013.4 µs       3.5x Nisaba        1.15x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     38.4 µs  969.9 µs   1008.4 µs   62.5 µs  960.0 µs   1022.5 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   16.9 µs  980.0 µs    996.9 µs  698.3 µs 1241.2 µs   1939.5 µs      41.2x Nisaba        1.95x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   18.5 µs  927.9 µs    946.4 µs   68.8 µs  938.4 µs   1007.2 µs       3.7x Nisaba        1.06x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   12.0 µs  985.3 µs    997.3 µs  163.6 µs 1102.7 µs   1266.2 µs      13.7x Nisaba        1.27x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   27.7 µs  984.1 µs   1011.8 µs  555.1 µs 1152.8 µs   1707.9 µs      20.1x Nisaba        1.69x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          14.8 µs 1022.7 µs   1037.6 µs   43.7 µs 1003.6 µs   1047.2 µs       2.9x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       17.8 µs 1228.4 µs   1246.1 µs   42.1 µs 1068.7 µs   1110.7 µs       2.4x Nisaba          1.12x Skia
43. 50 Pill / Capsule Badges (r=h/2)         25.0 µs 1023.7 µs   1048.7 µs   61.0 µs  905.2 µs    966.2 µs       2.4x Nisaba          1.09x Skia
44. 50 Asymmetric Teardrop Rounded Rects     17.1 µs 1057.8 µs   1074.8 µs   47.0 µs  950.7 µs    997.7 µs       2.8x Nisaba          1.08x Skia
45. High-Density Disks Cloud (500 Disks)     62.9 µs  955.2 µs   1018.2 µs  218.0 µs  973.7 µs   1191.6 µs       3.5x Nisaba        1.17x Nisaba
46. High-Density Stroked Rings (250 Rings)   34.4 µs 1065.0 µs   1099.4 µs  143.2 µs  924.7 µs   1067.9 µs       4.2x Nisaba         Tie (~1.0x)
47. 250 Filled Rectangles Batch              38.5 µs  994.8 µs   1033.3 µs  134.2 µs  957.5 µs   1091.7 µs       3.5x Nisaba        1.06x Nisaba
48. 150 Stroked Rectangles (2px Border)      35.7 µs  960.2 µs    995.9 µs   88.0 µs  940.5 µs   1028.5 µs       2.5x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                88.0 µs  948.9 µs   1037.0 µs  309.8 µs 1040.7 µs   1350.5 µs       3.5x Nisaba        1.30x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      83.7 µs  994.8 µs   1078.5 µs  321.1 µs 1120.0 µs   1441.1 µs       3.8x Nisaba        1.34x Nisaba
51. 30 Vertical Card Linear Gradients        17.7 µs 1278.2 µs   1295.9 µs  154.5 µs  999.6 µs   1154.1 µs       8.7x Nisaba          1.12x Skia
52. 30 Horizontal Bar Progress Gradients     16.8 µs 1242.6 µs   1259.4 µs  147.2 µs  938.6 µs   1085.7 µs       8.8x Nisaba          1.16x Skia
53. 15 Concentric Radial Spotlights          11.3 µs 1266.2 µs   1277.6 µs   92.2 µs 1004.3 µs   1096.5 µs       8.2x Nisaba          1.17x Skia
54. 25 Soft Ambient Card Shadows             14.7 µs 1234.7 µs   1249.3 µs  125.8 µs 1062.9 µs   1188.8 µs       8.6x Nisaba         Tie (~1.0x)
55. 15 Neon Button Glows (Cyan Intense)      12.4 µs 1029.4 µs   1041.8 µs   84.4 µs  988.1 µs   1072.5 µs       6.8x Nisaba         Tie (~1.0x)
56. 16 Multi-Angle Gradient Fan Slices       26.2 µs  971.8 µs    998.0 µs  109.3 µs  887.4 µs    996.7 µs       4.2x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     8.7 µs 1052.6 µs   1061.3 µs   33.1 µs  971.1 µs   1004.2 µs       3.8x Nisaba          1.06x Skia
58. 20 Inset Well Shadows (Recessed)         51.3 µs 1295.0 µs   1346.3 µs  104.7 µs  925.8 µs   1030.5 µs       2.0x Nisaba          1.31x Skia
59. 10 Pulsing Circular Radar Wave Glows     13.4 µs 1415.6 µs   1429.0 µs   66.1 µs 1239.2 µs   1305.3 µs       4.9x Nisaba          1.09x Skia
60. Waveform Polyline Stroked with Gradient   47.7 µs  875.9 µs    923.6 µs  577.7 µs 1238.4 µs   1816.1 µs      12.1x Nisaba        1.97x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   11.0 µs  854.4 µs    865.4 µs   35.0 µs  794.8 µs    829.8 µs       3.2x Nisaba         Tie (~1.0x)
62. Scaled Image Quad (2x 512x512 Blit)       9.7 µs  936.0 µs    945.7 µs   30.9 µs  813.9 µs    844.8 µs       3.2x Nisaba          1.12x Skia
63. 45° Rotated Pattern Fill in Circle      10.3 µs 1018.7 µs   1029.1 µs   40.8 µs  945.8 µs    986.6 µs       3.9x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   10.2 µs 1014.1 µs   1024.4 µs   32.5 µs  964.5 µs    997.0 µs       3.2x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           13.0 µs  974.1 µs    987.2 µs   43.9 µs  943.7 µs    987.6 µs       3.4x Nisaba         Tie (~1.0x)
66. Texture Pattern on Star Path             21.0 µs  978.0 µs    999.0 µs  131.9 µs  896.2 µs   1028.1 µs       6.3x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    16.0 µs 1044.9 µs   1060.9 µs  178.6 µs 1078.2 µs   1256.9 µs      11.2x Nisaba        1.18x Nisaba
68. Minified Texture Quad (0.25x Downscale)   22.6 µs  974.6 µs    997.2 µs   72.3 µs  896.9 µs    969.2 µs       3.2x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        21.4 µs 1036.7 µs   1058.0 µs   63.1 µs  924.2 µs    987.3 µs       3.0x Nisaba          1.07x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   16.6 µs  973.5 µs    990.1 µs   53.7 µs  944.5 µs    998.2 µs       3.2x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   22.7 µs  923.3 µs    946.0 µs   36.5 µs  963.1 µs    999.6 µs       1.6x Nisaba        1.06x Nisaba
72. Shear / Skew Parallelograms (30 Quads)   15.9 µs  960.0 µs    975.9 µs   49.2 µs  950.4 µs    999.6 µs       3.1x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   14.2 µs  983.9 µs    998.1 µs   44.9 µs  951.8 µs    996.7 µs       3.2x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   12.0 µs  978.0 µs    990.0 µs   40.6 µs  943.5 µs    984.1 µs       3.4x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       16.2 µs  954.4 µs    970.7 µs   55.9 µs  940.2 µs    996.1 µs       3.4x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   14.2 µs  903.1 µs    917.3 µs   45.2 µs  947.8 µs    993.0 µs       3.2x Nisaba        1.08x Nisaba
77. Multi-Scissor Grid (16 Viewports)        14.1 µs  943.4 µs    957.5 µs   60.1 µs  637.5 µs    697.6 µs       4.3x Nisaba          1.37x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   13.1 µs 1024.6 µs   1037.6 µs   36.0 µs 1012.1 µs   1048.1 µs       2.8x Nisaba         Tie (~1.0x)
79. Scissored Stroked Waves (1000 Pts Clipped)   94.8 µs  909.5 µs   1004.2 µs  877.7 µs  897.3 µs   1775.0 µs       9.3x Nisaba        1.77x Nisaba
80. Scissored Gradient Card (Clipped)        12.7 µs 1037.2 µs   1049.9 µs   87.4 µs  579.6 µs    667.0 µs       6.9x Nisaba          1.57x Skia
81. Circular Overflow Clip (25 Boxes)        14.1 µs  967.7 µs    981.8 µs   74.3 µs  536.8 µs    611.0 µs       5.3x Nisaba          1.61x Skia
82. Scissor Invalidation / Reset (25 Cycles)   14.0 µs  984.1 µs    998.1 µs   43.4 µs  954.0 µs    997.4 µs       3.1x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    13.0 µs 1010.6 µs   1023.7 µs   38.9 µs  969.7 µs   1008.6 µs       3.0x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   12.7 µs 1000.8 µs   1013.5 µs   43.9 µs  974.2 µs   1018.1 µs       3.5x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   12.4 µs  989.7 µs   1002.1 µs   44.4 µs 1264.3 µs   1308.7 µs       3.6x Nisaba        1.31x Nisaba
86. Composite Op: Source-Out (Cutout)        12.8 µs  965.6 µs    978.4 µs   42.9 µs 1079.6 µs   1122.5 µs       3.4x Nisaba        1.15x Nisaba
87. Composite Op: Atop (Target Bounds)       13.5 µs  995.1 µs   1008.5 µs   39.6 µs  958.5 µs    998.1 µs       2.9x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   12.6 µs  976.2 µs    988.8 µs   45.3 µs  893.2 µs    938.5 µs       3.6x Nisaba          1.05x Skia
89. Composite Op: Dest-Out (Eraser Mask)     12.2 µs  975.2 µs    987.4 µs   43.3 µs  954.1 µs    997.3 µs       3.5x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        15.3 µs  960.4 µs    975.7 µs   40.8 µs 1087.3 µs   1128.1 µs       2.7x Nisaba        1.16x Nisaba
91. Composite Op: Xor (Exclusive Blend)      14.0 µs  962.7 µs    976.7 µs   44.7 µs  955.0 µs    999.7 µs       3.2x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    13.3 µs  942.0 µs    955.3 µs   41.4 µs  970.1 µs   1011.5 µs       3.1x Nisaba        1.06x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   21.8 µs  976.8 µs    998.7 µs  204.8 µs 1032.1 µs   1236.9 µs       9.4x Nisaba        1.24x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      29.6 µs 1140.9 µs   1170.6 µs  270.6 µs 1071.4 µs   1342.0 µs       9.1x Nisaba        1.15x Nisaba
95. Circular Progress Rings (8 Meters)       43.1 µs 1025.1 µs   1068.2 µs  383.8 µs 1119.2 µs   1503.0 µs       8.9x Nisaba        1.41x Nisaba
96. Modern Card Stack (5 Elevated Cards)     15.1 µs 1496.4 µs   1511.5 µs   82.4 µs 1519.9 µs   1602.3 µs       5.4x Nisaba        1.06x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   22.3 µs  884.2 µs    906.6 µs   39.5 µs 1018.2 µs   1057.7 µs       1.8x Nisaba        1.17x Nisaba
98. Floating Action Button (FAB 10 Buttons)   35.6 µs  989.2 µs   1024.7 µs  119.0 µs  937.8 µs   1056.8 µs       3.3x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   28.7 µs  977.4 µs   1006.2 µs  112.6 µs  945.1 µs   1057.7 µs       3.9x Nisaba        1.05x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   18.5 µs 1431.4 µs   1449.9 µs   71.8 µs 1138.8 µs   1210.5 µs       3.9x Nisaba          1.20x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2644.6 µs104408.7 µs 107053.3 µs19042.0 µs102013.7 µs 121055.7 µs      7.20x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 44 suites | Skia won in 23 suites | Ties: 33
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.20x Nisaba
===================================================================================================================================================
```
</details>

### Run 9
<details>
<summary>Click to expand Run 9 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 792.5 µs, Skia: 784.8 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 932.2 µs, Skia: 1004.4 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1001.0 µs, Skia: 1324.9 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1039.0 µs, Skia: 1008.8 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1438.6 µs, Skia: 1296.9 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1276.7 µs, Skia: 1071.0 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1356.9 µs, Skia: 1043.4 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1028.1 µs, Skia: 1006.9 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1017.0 µs, Skia: 1039.6 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 998.3 µs, Skia: 1049.0 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1467.3 µs, Skia: 1464.1 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1421.8 µs, Skia: 1350.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2094.9 µs, Skia: 1521.4 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1127.9 µs, Skia: 1314.3 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1108.5 µs, Skia: 1482.3 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1006.5 µs, Skia: 2018.2 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1016.9 µs, Skia: 2073.4 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1088.1 µs, Skia: 1077.3 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1026.0 µs, Skia: 643.7 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 992.7 µs, Skia: 1031.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1137.7 µs, Skia: 1524.6 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1023.1 µs, Skia: 2589.4 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 973.6 µs, Skia: 1298.7 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 999.8 µs, Skia: 1723.9 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1007.3 µs, Skia: 1066.2 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1412.8 µs, Skia: 1684.5 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 999.1 µs, Skia: 1553.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 999.4 µs, Skia: 2038.3 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 1036.8 µs, Skia: 1452.4 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1330.5 µs, Skia: 2577.3 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 935.5 µs, Skia: 1539.3 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1012.7 µs, Skia: 1534.7 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 962.0 µs, Skia: 1555.9 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 859.0 µs, Skia: 998.9 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 928.3 µs, Skia: 1028.0 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1008.4 µs, Skia: 1007.5 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 990.8 µs, Skia: 1908.5 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.6 µs, Skia: 1026.4 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1011.6 µs, Skia: 1276.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1012.5 µs, Skia: 1786.7 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1008.3 µs, Skia: 996.4 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1231.9 µs, Skia: 1102.4 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1016.6 µs, Skia: 1007.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1087.8 µs, Skia: 1008.0 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1013.2 µs, Skia: 1177.9 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1128.4 µs, Skia: 1037.8 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1017.4 µs, Skia: 1057.9 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 961.6 µs, Skia: 1006.2 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1028.9 µs, Skia: 1335.7 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1058.0 µs, Skia: 1430.9 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1241.5 µs, Skia: 1113.7 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1470.1 µs, Skia: 1155.4 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1181.1 µs, Skia: 1014.6 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1330.6 µs, Skia: 1158.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1040.3 µs, Skia: 1037.9 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1038.5 µs, Skia: 998.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1258.2 µs, Skia: 1007.6 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1319.3 µs, Skia: 1155.9 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1536.5 µs, Skia: 1412.5 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1008.3 µs, Skia: 1778.5 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1117.4 µs, Skia: 987.1 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 998.0 µs, Skia: 979.3 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1037.8 µs, Skia: 992.3 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1047.2 µs, Skia: 997.9 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1007.1 µs, Skia: 1064.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1008.1 µs, Skia: 1068.1 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 988.2 µs, Skia: 1304.2 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 968.2 µs, Skia: 998.7 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1059.7 µs, Skia: 1008.4 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 958.7 µs, Skia: 997.5 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 945.0 µs, Skia: 980.8 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 990.5 µs, Skia: 999.3 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 997.1 µs, Skia: 988.1 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 983.6 µs, Skia: 992.6 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 971.3 µs, Skia: 987.3 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 944.2 µs, Skia: 980.3 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 989.0 µs, Skia: 634.2 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1183.9 µs, Skia: 1107.3 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1009.2 µs, Skia: 1862.6 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1227.6 µs, Skia: 676.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 995.5 µs, Skia: 581.2 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 999.4 µs, Skia: 986.6 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1029.1 µs, Skia: 1038.9 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1028.6 µs, Skia: 1009.9 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 945.9 µs, Skia: 1121.2 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 967.9 µs, Skia: 1095.4 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 993.5 µs, Skia: 999.1 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1011.5 µs, Skia: 1032.2 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1038.6 µs, Skia: 1015.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 981.9 µs, Skia: 1120.4 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1000.9 µs, Skia: 994.3 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 953.9 µs, Skia: 950.2 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 991.4 µs, Skia: 1314.2 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1075.1 µs, Skia: 1360.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1027.1 µs, Skia: 1474.1 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1755.5 µs, Skia: 1581.6 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 935.9 µs, Skia: 1090.0 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1028.4 µs, Skia: 1067.0 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 998.2 µs, Skia: 1036.5 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1308.3 µs, Skia: 1246.0 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.6 µs  788.8 µs    792.5 µs   11.1 µs  773.8 µs    784.8 µs       3.0x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            18.3 µs  913.9 µs    932.2 µs   46.6 µs  957.8 µs   1004.4 µs       2.5x Nisaba        1.08x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      29.6 µs  971.5 µs   1001.0 µs  254.8 µs 1070.1 µs   1324.9 µs       8.6x Nisaba        1.32x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      28.0 µs 1011.0 µs   1039.0 µs   74.0 µs  934.8 µs   1008.8 µs       2.6x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   22.6 µs 1416.0 µs   1438.6 µs   71.1 µs 1225.9 µs   1296.9 µs       3.1x Nisaba          1.11x Skia
6. 50 Simple Rounded Rects (12px Radius)     16.8 µs 1260.0 µs   1276.7 µs   59.0 µs 1012.0 µs   1071.0 µs       3.5x Nisaba          1.19x Skia
7. 50 Varying-Radius Rounded Rects           16.5 µs 1340.4 µs   1356.9 µs   46.2 µs  997.2 µs   1043.4 µs       2.8x Nisaba          1.30x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   22.0 µs 1006.1 µs   1028.1 µs   60.6 µs  946.3 µs   1006.9 µs       2.8x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          19.1 µs  997.9 µs   1017.0 µs   72.6 µs  967.0 µs   1039.6 µs       3.8x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         41.7 µs  956.6 µs    998.3 µs   40.0 µs 1008.9 µs   1049.0 µs               Tie        1.05x Nisaba
11. 50 Two-Stop Linear Gradients             17.7 µs 1449.6 µs   1467.3 µs  190.1 µs 1274.0 µs   1464.1 µs      10.7x Nisaba         Tie (~1.0x)
12. 50 Diagonal Gradients (45° Angle)       20.1 µs 1401.7 µs   1421.8 µs  193.8 µs 1157.1 µs   1350.9 µs       9.6x Nisaba         Tie (~1.0x)
13. 25 Radial Glow Gradients (300px)         15.1 µs 2079.8 µs   2094.9 µs  107.0 µs 1414.4 µs   1521.4 µs       7.1x Nisaba          1.38x Skia
14. 20 Box Blur / Drop Shadows (15px)        11.2 µs 1116.7 µs   1127.9 µs  101.3 µs 1213.0 µs   1314.3 µs       9.0x Nisaba        1.17x Nisaba
15. 10 Concave 10-Point Stars (Fill)         47.0 µs 1061.5 µs   1108.5 µs  386.6 µs 1095.8 µs   1482.3 µs       8.2x Nisaba        1.34x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   32.1 µs  974.5 µs   1006.5 µs  903.3 µs 1114.9 µs   2018.2 µs      28.2x Nisaba        2.01x Nisaba
17. 1000-Point Waveform (Dense Stroke)       92.0 µs  924.8 µs   1016.9 µs  853.5 µs 1219.8 µs   2073.4 µs       9.3x Nisaba        2.04x Nisaba
18. 50 Rotated Cards (Affine Transforms)     18.2 µs 1069.8 µs   1088.1 µs   67.1 µs 1010.3 µs   1077.3 µs       3.7x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             14.3 µs 1011.7 µs   1026.0 µs   71.0 µs  572.8 µs    643.7 µs       5.0x Nisaba          1.59x Skia
20. 50 UI Text Labels (Inter-Regular)       105.5 µs  887.2 µs    992.7 µs  100.3 µs  931.3 µs   1031.6 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   98.0 µs 1039.7 µs   1137.7 µs  359.8 µs 1164.8 µs   1524.6 µs       3.7x Nisaba        1.34x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     38.8 µs  984.3 µs   1023.1 µs 1234.9 µs 1354.5 µs   2589.4 µs      31.9x Nisaba        2.53x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       15.8 µs  957.9 µs    973.6 µs  186.9 µs 1111.8 µs   1298.7 µs      11.9x Nisaba        1.33x Nisaba
24. Circular Arcs with arcTo (20 Corners)    27.9 µs  971.9 µs    999.8 µs  503.2 µs 1220.7 µs   1723.9 µs      18.0x Nisaba        1.72x Nisaba
25. Directional Arcs with arc() (30 Arcs)    56.1 µs  951.2 µs   1007.3 µs  151.9 µs  914.3 µs   1066.2 µs       2.7x Nisaba        1.06x Nisaba
26. Compound Path with Holes (15 Donuts)     90.4 µs 1322.3 µs   1412.8 µs  529.6 µs 1154.9 µs   1684.5 µs       5.9x Nisaba        1.19x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    40.6 µs  958.5 µs    999.1 µs  493.3 µs 1060.1 µs   1553.4 µs      12.2x Nisaba        1.55x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    49.2 µs  950.2 µs    999.4 µs  805.2 µs 1233.1 µs   2038.3 µs      16.4x Nisaba        2.04x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   10.1 µs 1026.7 µs   1036.8 µs  268.1 µs 1184.3 µs   1452.4 µs      26.4x Nisaba        1.40x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      27.9 µs 1302.6 µs   1330.5 µs 1179.3 µs 1398.0 µs   2577.3 µs      42.3x Nisaba        1.94x Nisaba
31. Stroke Join: Miter (12px Stroke)         11.1 µs  924.4 µs    935.5 µs  376.7 µs 1162.6 µs   1539.3 µs      34.0x Nisaba        1.65x Nisaba
32. Stroke Join: Bevel (12px Stroke)         11.0 µs 1001.7 µs   1012.7 µs  392.1 µs 1142.6 µs   1534.7 µs      35.7x Nisaba        1.52x Nisaba
33. Stroke Join: Round (12px Stroke)         16.4 µs  945.6 µs    962.0 µs  396.3 µs 1159.5 µs   1555.9 µs      24.2x Nisaba        1.62x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      16.4 µs  842.6 µs    859.0 µs   47.0 µs  951.9 µs    998.9 µs       2.9x Nisaba        1.16x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    16.5 µs  911.8 µs    928.3 µs   44.5 µs  983.5 µs   1028.0 µs       2.7x Nisaba        1.11x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     42.0 µs  966.4 µs   1008.4 µs   64.3 µs  943.2 µs   1007.5 µs       1.5x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   16.9 µs  973.9 µs    990.8 µs  682.0 µs 1226.5 µs   1908.5 µs      40.3x Nisaba        1.93x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   19.8 µs  977.8 µs    997.6 µs   71.7 µs  954.7 µs   1026.4 µs       3.6x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   11.6 µs 1000.0 µs   1011.6 µs  157.3 µs 1119.1 µs   1276.5 µs      13.5x Nisaba        1.26x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   27.4 µs  985.1 µs   1012.5 µs  562.4 µs 1224.3 µs   1786.7 µs      20.5x Nisaba        1.76x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          17.2 µs  991.1 µs   1008.3 µs   41.1 µs  955.3 µs    996.4 µs       2.4x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       13.5 µs 1218.4 µs   1231.9 µs   42.4 µs 1060.0 µs   1102.4 µs       3.1x Nisaba          1.12x Skia
43. 50 Pill / Capsule Badges (r=h/2)         15.7 µs 1000.9 µs   1016.6 µs   56.6 µs  951.0 µs   1007.6 µs       3.6x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     15.2 µs 1072.6 µs   1087.8 µs   46.9 µs  961.0 µs   1008.0 µs       3.1x Nisaba          1.08x Skia
45. High-Density Disks Cloud (500 Disks)     65.7 µs  947.6 µs   1013.2 µs  222.6 µs  955.3 µs   1177.9 µs       3.4x Nisaba        1.16x Nisaba
46. High-Density Stroked Rings (250 Rings)   32.9 µs 1095.5 µs   1128.4 µs  136.5 µs  901.3 µs   1037.8 µs       4.1x Nisaba          1.09x Skia
47. 250 Filled Rectangles Batch              38.1 µs  979.3 µs   1017.4 µs  130.8 µs  927.1 µs   1057.9 µs       3.4x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      38.4 µs  923.2 µs    961.6 µs   84.7 µs  921.6 µs   1006.2 µs       2.2x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                84.5 µs  944.4 µs   1028.9 µs  302.4 µs 1033.3 µs   1335.7 µs       3.6x Nisaba        1.30x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      80.7 µs  977.3 µs   1058.0 µs  308.3 µs 1122.6 µs   1430.9 µs       3.8x Nisaba        1.35x Nisaba
51. 30 Vertical Card Linear Gradients        14.3 µs 1227.2 µs   1241.5 µs  153.3 µs  960.4 µs   1113.7 µs      10.7x Nisaba          1.11x Skia
52. 30 Horizontal Bar Progress Gradients     15.2 µs 1454.8 µs   1470.1 µs  145.6 µs 1009.8 µs   1155.4 µs       9.6x Nisaba          1.27x Skia
53. 15 Concentric Radial Spotlights          11.9 µs 1169.2 µs   1181.1 µs   89.0 µs  925.7 µs   1014.6 µs       7.5x Nisaba          1.16x Skia
54. 25 Soft Ambient Card Shadows             13.5 µs 1317.2 µs   1330.6 µs  121.7 µs 1036.7 µs   1158.5 µs       9.1x Nisaba          1.15x Skia
55. 15 Neon Button Glows (Cyan Intense)       9.9 µs 1030.4 µs   1040.3 µs   82.5 µs  955.4 µs   1037.9 µs       8.3x Nisaba         Tie (~1.0x)
56. 16 Multi-Angle Gradient Fan Slices       25.4 µs 1013.0 µs   1038.5 µs  101.9 µs  896.9 µs    998.7 µs       4.0x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     8.4 µs 1249.8 µs   1258.2 µs   31.0 µs  976.6 µs   1007.6 µs       3.7x Nisaba          1.25x Skia
58. 20 Inset Well Shadows (Recessed)         56.3 µs 1263.0 µs   1319.3 µs  102.8 µs 1053.1 µs   1155.9 µs       1.8x Nisaba          1.14x Skia
59. 10 Pulsing Circular Radar Wave Glows     13.0 µs 1523.5 µs   1536.5 µs   63.6 µs 1348.9 µs   1412.5 µs       4.9x Nisaba          1.09x Skia
60. Waveform Polyline Stroked with Gradient   48.0 µs  960.3 µs   1008.3 µs  551.6 µs 1226.9 µs   1778.5 µs      11.5x Nisaba        1.76x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   10.7 µs 1106.7 µs   1117.4 µs   33.5 µs  953.7 µs    987.1 µs       3.1x Nisaba          1.13x Skia
62. Scaled Image Quad (2x 512x512 Blit)       9.8 µs  988.2 µs    998.0 µs   30.3 µs  949.0 µs    979.3 µs       3.1x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle       8.7 µs 1029.1 µs   1037.8 µs   40.7 µs  951.6 µs    992.3 µs       4.7x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)    8.8 µs 1038.5 µs   1047.2 µs   37.2 µs  960.6 µs    997.9 µs       4.2x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           11.3 µs  995.9 µs   1007.1 µs   44.9 µs 1019.3 µs   1064.2 µs       4.0x Nisaba        1.06x Nisaba
66. Texture Pattern on Star Path             18.8 µs  989.3 µs   1008.1 µs  137.5 µs  930.6 µs   1068.1 µs       7.3x Nisaba        1.06x Nisaba
67. 20px Curved Path Stroked with Texture    14.2 µs  974.0 µs    988.2 µs  171.2 µs 1132.9 µs   1304.2 µs      12.1x Nisaba        1.32x Nisaba
68. Minified Texture Quad (0.25x Downscale)   19.2 µs  949.0 µs    968.2 µs   67.4 µs  931.2 µs    998.7 µs       3.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        21.2 µs 1038.5 µs   1059.7 µs   62.6 µs  945.8 µs   1008.4 µs       3.0x Nisaba         Tie (~1.0x)
70. Pure Rotation Cluster (36 Spokes 10°)   18.0 µs  940.7 µs    958.7 µs   51.5 µs  946.0 µs    997.5 µs       2.9x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   23.1 µs  922.0 µs    945.0 µs   39.2 µs  941.6 µs    980.8 µs       1.7x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   14.8 µs  975.7 µs    990.5 µs   47.8 µs  951.5 µs    999.3 µs       3.2x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   17.9 µs  979.2 µs    997.1 µs   44.4 µs  943.7 µs    988.1 µs       2.5x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   16.2 µs  967.4 µs    983.6 µs   42.7 µs  949.9 µs    992.6 µs       2.6x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       19.8 µs  951.6 µs    971.3 µs   57.0 µs  930.3 µs    987.3 µs       2.9x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   15.3 µs  928.9 µs    944.2 µs   40.0 µs  940.3 µs    980.3 µs       2.6x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        13.5 µs  975.4 µs    989.0 µs   62.0 µs  572.3 µs    634.2 µs       4.6x Nisaba          1.56x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   13.3 µs 1170.5 µs   1183.9 µs   37.8 µs 1069.5 µs   1107.3 µs       2.8x Nisaba          1.07x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   96.1 µs  913.1 µs   1009.2 µs  893.5 µs  969.2 µs   1862.6 µs       9.3x Nisaba        1.85x Nisaba
80. Scissored Gradient Card (Clipped)        15.3 µs 1212.3 µs   1227.6 µs   89.1 µs  587.4 µs    676.4 µs       5.8x Nisaba          1.81x Skia
81. Circular Overflow Clip (25 Boxes)        13.9 µs  981.6 µs    995.5 µs   69.8 µs  511.3 µs    581.2 µs       5.0x Nisaba          1.71x Skia
82. Scissor Invalidation / Reset (25 Cycles)   13.6 µs  985.9 µs    999.4 µs   46.1 µs  940.5 µs    986.6 µs       3.4x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    13.9 µs 1015.2 µs   1029.1 µs   42.4 µs  996.6 µs   1038.9 µs       3.1x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   12.6 µs 1016.0 µs   1028.6 µs   43.6 µs  966.2 µs   1009.9 µs       3.5x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   13.6 µs  932.3 µs    945.9 µs   40.5 µs 1080.7 µs   1121.2 µs       3.0x Nisaba        1.19x Nisaba
86. Composite Op: Source-Out (Cutout)        11.7 µs  956.1 µs    967.9 µs   42.9 µs 1052.4 µs   1095.4 µs       3.7x Nisaba        1.13x Nisaba
87. Composite Op: Atop (Target Bounds)       12.3 µs  981.2 µs    993.5 µs   43.9 µs  955.2 µs    999.1 µs       3.6x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   11.3 µs 1000.2 µs   1011.5 µs   41.4 µs  990.8 µs   1032.2 µs       3.7x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     12.9 µs 1025.6 µs   1038.6 µs   44.5 µs  970.8 µs   1015.3 µs       3.4x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        14.1 µs  967.8 µs    981.9 µs   42.8 µs 1077.5 µs   1120.4 µs       3.0x Nisaba        1.14x Nisaba
91. Composite Op: Xor (Exclusive Blend)      11.7 µs  989.1 µs   1000.9 µs   45.2 µs  949.1 µs    994.3 µs       3.9x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    13.3 µs  940.6 µs    953.9 µs   42.2 µs  908.0 µs    950.2 µs       3.2x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   25.7 µs  965.7 µs    991.4 µs  213.3 µs 1100.9 µs   1314.2 µs       8.3x Nisaba        1.33x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      30.2 µs 1044.9 µs   1075.1 µs  274.9 µs 1086.1 µs   1360.9 µs       9.1x Nisaba        1.27x Nisaba
95. Circular Progress Rings (8 Meters)       43.8 µs  983.4 µs   1027.1 µs  402.1 µs 1072.0 µs   1474.1 µs       9.2x Nisaba        1.44x Nisaba
96. Modern Card Stack (5 Elevated Cards)     16.4 µs 1739.0 µs   1755.5 µs   84.7 µs 1496.9 µs   1581.6 µs       5.2x Nisaba          1.11x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   24.2 µs  911.7 µs    935.9 µs   41.9 µs 1048.1 µs   1090.0 µs       1.7x Nisaba        1.16x Nisaba
98. Floating Action Button (FAB 10 Buttons)   36.2 µs  992.3 µs   1028.4 µs  125.0 µs  942.0 µs   1067.0 µs       3.5x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   29.8 µs  968.3 µs    998.2 µs  119.5 µs  917.0 µs   1036.5 µs       4.0x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   20.9 µs 1287.3 µs   1308.3 µs   71.2 µs 1174.8 µs   1246.0 µs       3.4x Nisaba         Tie (~1.0x)
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2621.1 µs105723.2 µs 108344.3 µs18940.2 µs102553.7 µs 121493.9 µs      7.23x Nisaba        1.12x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 40 suites | Skia won in 21 suites | Ties: 39
         Overall Frame Throughput (End-to-End): 1.12x Nisaba
         Overall CPU Command Submission:       7.23x Nisaba
===================================================================================================================================================
```
</details>

### Run 10
<details>
<summary>Click to expand Run 10 full log</summary>

```text
=========================================================================================================
          NISABA GPU vs GOOGLE SKIA (GANESH GL) — 100-SUITE GRANULAR GPU MICRO-PROFILER                  
                       (Strict 1:1 Parity • 1080x720 Framebuffer • 0x MSAA)                             
=========================================================================================================

GPU Driver   : Mesa Intel(R) UHD Graphics 620 (WHL GT2)
OpenGL Core  : 4.6 (Core Profile) Mesa 25.2.8-0ubuntu0.24.04.2
Iterations   : 100 runs per benchmark suite
Target Size  : 1080x720 (0x MSAA)

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 928.6 µs, Skia: 878.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 960.0 µs, Skia: 1055.1 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1008.7 µs, Skia: 1316.1 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1035.0 µs, Skia: 1033.7 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1128.1 µs, Skia: 1368.6 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1261.6 µs, Skia: 1044.5 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1254.1 µs, Skia: 1035.8 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1027.0 µs, Skia: 998.4 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 997.1 µs, Skia: 1028.0 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1006.0 µs, Skia: 1037.2 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1488.1 µs, Skia: 1461.9 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1554.0 µs, Skia: 1351.4 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 1941.0 µs, Skia: 1528.2 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1308.9 µs, Skia: 1408.4 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1176.6 µs, Skia: 1469.5 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 988.4 µs, Skia: 2098.9 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1006.5 µs, Skia: 2088.5 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1012.1 µs, Skia: 1003.6 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 991.3 µs, Skia: 627.8 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 998.0 µs, Skia: 1017.1 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1171.9 µs, Skia: 1503.5 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 997.9 µs, Skia: 2626.4 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 999.5 µs, Skia: 1322.2 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 960.1 µs, Skia: 1729.6 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1005.1 µs, Skia: 1461.8 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1440.3 µs, Skia: 1772.5 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 934.5 µs, Skia: 1662.1 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 978.4 µs, Skia: 2033.1 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 997.3 µs, Skia: 1474.6 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1341.8 µs, Skia: 2565.3 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1005.6 µs, Skia: 1555.7 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 996.8 µs, Skia: 1530.0 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1003.2 µs, Skia: 1595.8 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 998.3 µs, Skia: 1102.0 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 997.7 µs, Skia: 1068.6 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1027.8 µs, Skia: 1067.9 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1006.3 µs, Skia: 1960.3 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 987.5 µs, Skia: 1077.9 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1005.5 µs, Skia: 1259.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1000.3 µs, Skia: 1762.3 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1028.3 µs, Skia: 1047.2 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1284.9 µs, Skia: 1119.8 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1055.6 µs, Skia: 1000.3 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1112.3 µs, Skia: 1043.6 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1003.4 µs, Skia: 1164.2 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1097.1 µs, Skia: 1047.4 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1068.2 µs, Skia: 1067.4 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1007.7 µs, Skia: 1006.8 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1011.6 µs, Skia: 1324.0 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1097.9 µs, Skia: 1438.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1262.8 µs, Skia: 1142.0 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1351.8 µs, Skia: 1117.0 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1238.8 µs, Skia: 1037.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1246.8 µs, Skia: 1133.1 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1117.3 µs, Skia: 1017.2 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1029.8 µs, Skia: 998.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1104.1 µs, Skia: 948.1 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1358.8 µs, Skia: 1159.4 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1703.1 µs, Skia: 1382.9 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1009.9 µs, Skia: 1767.3 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 999.5 µs, Skia: 995.8 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1008.0 µs, Skia: 997.9 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1045.9 µs, Skia: 885.2 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1029.6 µs, Skia: 997.8 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1015.1 µs, Skia: 927.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 998.1 µs, Skia: 1085.0 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1000.7 µs, Skia: 1344.2 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1003.5 µs, Skia: 1007.5 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1127.0 µs, Skia: 1048.3 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 997.7 µs, Skia: 1018.2 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 992.6 µs, Skia: 1004.6 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 975.9 µs, Skia: 1008.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1001.8 µs, Skia: 1005.5 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 998.7 µs, Skia: 998.1 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 873.0 µs, Skia: 1001.7 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 938.5 µs, Skia: 998.0 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1028.9 µs, Skia: 720.0 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1207.0 µs, Skia: 1045.7 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1063.4 µs, Skia: 1821.2 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1058.9 µs, Skia: 660.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 992.5 µs, Skia: 569.1 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1007.9 µs, Skia: 998.0 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1027.5 µs, Skia: 1017.5 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1047.9 µs, Skia: 987.9 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1004.7 µs, Skia: 1317.5 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1008.2 µs, Skia: 1301.3 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 999.1 µs, Skia: 997.6 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 997.8 µs, Skia: 996.0 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1000.5 µs, Skia: 995.5 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 978.1 µs, Skia: 1128.8 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1024.0 µs, Skia: 922.4 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 968.1 µs, Skia: 994.1 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1001.5 µs, Skia: 1199.8 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1167.6 µs, Skia: 1354.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 997.9 µs, Skia: 1466.3 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1695.2 µs, Skia: 1628.0 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 958.5 µs, Skia: 1037.6 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1008.5 µs, Skia: 1037.3 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1007.8 µs, Skia: 1067.5 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1370.6 µs, Skia: 1149.3 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.4 µs  925.2 µs    928.6 µs   12.8 µs  866.2 µs    878.9 µs       3.7x Nisaba          1.06x Skia
2. 100 Grid Lines (1px Hairlines)            18.5 µs  941.5 µs    960.0 µs   44.0 µs 1011.1 µs   1055.1 µs       2.4x Nisaba        1.10x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      26.3 µs  982.4 µs   1008.7 µs  257.3 µs 1058.8 µs   1316.1 µs       9.8x Nisaba        1.30x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      28.7 µs 1006.3 µs   1035.0 µs   77.0 µs  956.6 µs   1033.7 µs       2.7x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   24.7 µs 1103.3 µs   1128.1 µs   73.9 µs 1294.7 µs   1368.6 µs       3.0x Nisaba        1.21x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     19.0 µs 1242.6 µs   1261.6 µs   61.5 µs  983.0 µs   1044.5 µs       3.2x Nisaba          1.21x Skia
7. 50 Varying-Radius Rounded Rects           16.6 µs 1237.5 µs   1254.1 µs   45.9 µs  989.9 µs   1035.8 µs       2.8x Nisaba          1.21x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   21.3 µs 1005.7 µs   1027.0 µs   58.3 µs  940.1 µs    998.4 µs       2.7x Nisaba         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          20.3 µs  976.8 µs    997.1 µs   74.0 µs  954.0 µs   1028.0 µs       3.6x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         40.8 µs  965.2 µs   1006.0 µs   42.5 µs  994.7 µs   1037.2 µs               Tie         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             17.4 µs 1470.7 µs   1488.1 µs  190.4 µs 1271.5 µs   1461.9 µs      11.0x Nisaba         Tie (~1.0x)
12. 50 Diagonal Gradients (45° Angle)       19.1 µs 1534.9 µs   1554.0 µs  191.3 µs 1160.1 µs   1351.4 µs      10.0x Nisaba          1.15x Skia
13. 25 Radial Glow Gradients (300px)         15.9 µs 1925.1 µs   1941.0 µs  104.3 µs 1423.9 µs   1528.2 µs       6.6x Nisaba          1.27x Skia
14. 20 Box Blur / Drop Shadows (15px)        12.0 µs 1296.9 µs   1308.9 µs   96.0 µs 1312.4 µs   1408.4 µs       8.0x Nisaba        1.08x Nisaba
15. 10 Concave 10-Point Stars (Fill)         45.8 µs 1130.8 µs   1176.6 µs  393.5 µs 1076.0 µs   1469.5 µs       8.6x Nisaba        1.25x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   27.4 µs  961.0 µs    988.4 µs  911.5 µs 1187.4 µs   2098.9 µs      33.2x Nisaba        2.12x Nisaba
17. 1000-Point Waveform (Dense Stroke)       94.2 µs  912.2 µs   1006.5 µs  824.1 µs 1264.4 µs   2088.5 µs       8.7x Nisaba        2.08x Nisaba
18. 50 Rotated Cards (Affine Transforms)     17.6 µs  994.5 µs   1012.1 µs   66.7 µs  936.9 µs   1003.6 µs       3.8x Nisaba         Tie (~1.0x)
19. 25 Scissor Clipped Viewports             14.4 µs  976.8 µs    991.3 µs   66.1 µs  561.6 µs    627.8 µs       4.6x Nisaba          1.58x Skia
20. 50 UI Text Labels (Inter-Regular)        99.6 µs  898.3 µs    998.0 µs  104.3 µs  912.8 µs   1017.1 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  103.1 µs 1068.8 µs   1171.9 µs  388.2 µs 1115.3 µs   1503.5 µs       3.8x Nisaba        1.28x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     41.4 µs  956.4 µs    997.9 µs 1283.4 µs 1343.0 µs   2626.4 µs      31.0x Nisaba        2.63x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       14.8 µs  984.7 µs    999.5 µs  198.1 µs 1124.1 µs   1322.2 µs      13.4x Nisaba        1.32x Nisaba
24. Circular Arcs with arcTo (20 Corners)    29.4 µs  930.6 µs    960.1 µs  537.8 µs 1191.8 µs   1729.6 µs      18.3x Nisaba        1.80x Nisaba
25. Directional Arcs with arc() (30 Arcs)    61.5 µs  943.6 µs   1005.1 µs  176.8 µs 1285.0 µs   1461.8 µs       2.9x Nisaba        1.45x Nisaba
26. Compound Path with Holes (15 Donuts)     96.5 µs 1343.8 µs   1440.3 µs  575.2 µs 1197.2 µs   1772.5 µs       6.0x Nisaba        1.23x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    49.8 µs  884.7 µs    934.5 µs  557.8 µs 1104.3 µs   1662.1 µs      11.2x Nisaba        1.78x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    46.7 µs  931.8 µs    978.4 µs  821.3 µs 1211.8 µs   2033.1 µs      17.6x Nisaba        2.08x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   11.4 µs  985.9 µs    997.3 µs  263.3 µs 1211.3 µs   1474.6 µs      23.1x Nisaba        1.48x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      29.1 µs 1312.7 µs   1341.8 µs 1190.5 µs 1374.7 µs   2565.3 µs      40.9x Nisaba        1.91x Nisaba
31. Stroke Join: Miter (12px Stroke)         11.2 µs  994.3 µs   1005.6 µs  395.6 µs 1160.1 µs   1555.7 µs      35.3x Nisaba        1.55x Nisaba
32. Stroke Join: Bevel (12px Stroke)         10.2 µs  986.6 µs    996.8 µs  408.5 µs 1121.5 µs   1530.0 µs      39.9x Nisaba        1.53x Nisaba
33. Stroke Join: Round (12px Stroke)         15.0 µs  988.2 µs   1003.2 µs  409.1 µs 1186.7 µs   1595.8 µs      27.3x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      14.0 µs  984.3 µs    998.3 µs   47.8 µs 1054.2 µs   1102.0 µs       3.4x Nisaba        1.10x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    15.9 µs  981.8 µs    997.7 µs   48.2 µs 1020.4 µs   1068.6 µs       3.0x Nisaba        1.07x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     39.7 µs  988.1 µs   1027.8 µs   64.6 µs 1003.4 µs   1067.9 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   17.6 µs  988.7 µs   1006.3 µs  719.8 µs 1240.5 µs   1960.3 µs      41.0x Nisaba        1.95x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   21.9 µs  965.5 µs    987.5 µs   76.7 µs 1001.2 µs   1077.9 µs       3.5x Nisaba        1.09x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   11.7 µs  993.8 µs   1005.5 µs  160.6 µs 1098.9 µs   1259.5 µs      13.7x Nisaba        1.25x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   30.0 µs  970.3 µs   1000.3 µs  568.8 µs 1193.5 µs   1762.3 µs      19.0x Nisaba        1.76x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          15.8 µs 1012.6 µs   1028.3 µs   45.0 µs 1002.2 µs   1047.2 µs       2.9x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)       19.8 µs 1265.0 µs   1284.9 µs   40.7 µs 1079.1 µs   1119.8 µs       2.1x Nisaba          1.15x Skia
43. 50 Pill / Capsule Badges (r=h/2)         24.1 µs 1031.5 µs   1055.6 µs   64.6 µs  935.7 µs   1000.3 µs       2.7x Nisaba          1.06x Skia
44. 50 Asymmetric Teardrop Rounded Rects     17.3 µs 1095.0 µs   1112.3 µs   44.2 µs  999.4 µs   1043.6 µs       2.6x Nisaba          1.07x Skia
45. High-Density Disks Cloud (500 Disks)     65.1 µs  938.3 µs   1003.4 µs  213.5 µs  950.7 µs   1164.2 µs       3.3x Nisaba        1.16x Nisaba
46. High-Density Stroked Rings (250 Rings)   35.4 µs 1061.8 µs   1097.1 µs  144.0 µs  903.4 µs   1047.4 µs       4.1x Nisaba         Tie (~1.0x)
47. 250 Filled Rectangles Batch              40.7 µs 1027.5 µs   1068.2 µs  130.2 µs  937.3 µs   1067.4 µs       3.2x Nisaba         Tie (~1.0x)
48. 150 Stroked Rectangles (2px Border)      36.2 µs  971.5 µs   1007.7 µs   85.1 µs  921.7 µs   1006.8 µs       2.4x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                89.6 µs  922.0 µs   1011.6 µs  312.0 µs 1012.0 µs   1324.0 µs       3.5x Nisaba        1.31x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      81.7 µs 1016.2 µs   1097.9 µs  322.5 µs 1115.9 µs   1438.4 µs       3.9x Nisaba        1.31x Nisaba
51. 30 Vertical Card Linear Gradients        16.0 µs 1246.8 µs   1262.8 µs  156.5 µs  985.5 µs   1142.0 µs       9.8x Nisaba          1.11x Skia
52. 30 Horizontal Bar Progress Gradients     17.3 µs 1334.5 µs   1351.8 µs  153.0 µs  964.0 µs   1117.0 µs       8.8x Nisaba          1.21x Skia
53. 15 Concentric Radial Spotlights          11.8 µs 1227.0 µs   1238.8 µs   91.9 µs  945.3 µs   1037.1 µs       7.8x Nisaba          1.19x Skia
54. 25 Soft Ambient Card Shadows             12.5 µs 1234.3 µs   1246.8 µs  122.6 µs 1010.5 µs   1133.1 µs       9.8x Nisaba          1.10x Skia
55. 15 Neon Button Glows (Cyan Intense)      13.2 µs 1104.0 µs   1117.3 µs   84.0 µs  933.2 µs   1017.2 µs       6.3x Nisaba          1.10x Skia
56. 16 Multi-Angle Gradient Fan Slices       28.7 µs 1001.1 µs   1029.8 µs  104.8 µs  893.8 µs    998.7 µs       3.7x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient     8.5 µs 1095.7 µs   1104.1 µs   34.2 µs  914.0 µs    948.1 µs       4.0x Nisaba          1.16x Skia
58. 20 Inset Well Shadows (Recessed)         61.8 µs 1296.9 µs   1358.8 µs  103.9 µs 1055.5 µs   1159.4 µs       1.7x Nisaba          1.17x Skia
59. 10 Pulsing Circular Radar Wave Glows     12.3 µs 1690.7 µs   1703.1 µs   66.1 µs 1316.8 µs   1382.9 µs       5.4x Nisaba          1.23x Skia
60. Waveform Polyline Stroked with Gradient   48.8 µs  961.1 µs   1009.9 µs  565.9 µs 1201.4 µs   1767.3 µs      11.6x Nisaba        1.75x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)    8.7 µs  990.8 µs    999.5 µs   32.3 µs  963.5 µs    995.8 µs       3.7x Nisaba         Tie (~1.0x)
62. Scaled Image Quad (2x 512x512 Blit)       8.6 µs  999.4 µs   1008.0 µs   31.6 µs  966.3 µs    997.9 µs       3.7x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      10.1 µs 1035.9 µs   1045.9 µs   36.3 µs  848.8 µs    885.2 µs       3.6x Nisaba          1.18x Skia
64. Translucent Texture Overlay (50% Alpha)   10.0 µs 1019.6 µs   1029.6 µs   34.7 µs  963.1 µs    997.8 µs       3.5x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           10.5 µs 1004.6 µs   1015.1 µs   46.4 µs  880.8 µs    927.2 µs       4.4x Nisaba          1.09x Skia
66. Texture Pattern on Star Path             18.7 µs  979.4 µs    998.1 µs  139.4 µs  945.6 µs   1085.0 µs       7.5x Nisaba        1.09x Nisaba
67. 20px Curved Path Stroked with Texture    15.1 µs  985.6 µs   1000.7 µs  179.3 µs 1164.9 µs   1344.2 µs      11.9x Nisaba        1.34x Nisaba
68. Minified Texture Quad (0.25x Downscale)   18.1 µs  985.4 µs   1003.5 µs   69.2 µs  938.4 µs   1007.5 µs       3.8x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        23.5 µs 1103.5 µs   1127.0 µs   64.9 µs  983.4 µs   1048.3 µs       2.8x Nisaba          1.08x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   16.3 µs  981.4 µs    997.7 µs   50.9 µs  967.3 µs   1018.2 µs       3.1x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   24.4 µs  968.2 µs    992.6 µs   40.9 µs  963.7 µs   1004.6 µs       1.7x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   19.7 µs  956.2 µs    975.9 µs   49.3 µs  959.3 µs   1008.6 µs       2.5x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   18.7 µs  983.1 µs   1001.8 µs   45.3 µs  960.1 µs   1005.5 µs       2.4x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   14.8 µs  983.9 µs    998.7 µs   42.6 µs  955.5 µs    998.1 µs       2.9x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       19.9 µs  853.1 µs    873.0 µs   57.6 µs  944.1 µs   1001.7 µs       2.9x Nisaba        1.15x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   13.6 µs  924.9 µs    938.5 µs   48.2 µs  949.8 µs    998.0 µs       3.5x Nisaba        1.06x Nisaba
77. Multi-Scissor Grid (16 Viewports)        13.0 µs 1015.9 µs   1028.9 µs   60.8 µs  659.2 µs    720.0 µs       4.7x Nisaba          1.43x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   14.9 µs 1192.1 µs   1207.0 µs   39.9 µs 1005.8 µs   1045.7 µs       2.7x Nisaba          1.15x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   95.7 µs  967.6 µs   1063.4 µs  876.5 µs  944.8 µs   1821.2 µs       9.2x Nisaba        1.71x Nisaba
80. Scissored Gradient Card (Clipped)        14.0 µs 1044.9 µs   1058.9 µs   87.4 µs  573.0 µs    660.4 µs       6.2x Nisaba          1.60x Skia
81. Circular Overflow Clip (25 Boxes)        15.5 µs  977.1 µs    992.5 µs   72.9 µs  496.2 µs    569.1 µs       4.7x Nisaba          1.74x Skia
82. Scissor Invalidation / Reset (25 Cycles)   13.9 µs  994.0 µs   1007.9 µs   47.6 µs  950.5 µs    998.0 µs       3.4x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    14.3 µs 1013.2 µs   1027.5 µs   44.8 µs  972.6 µs   1017.5 µs       3.1x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   14.5 µs 1033.4 µs   1047.9 µs   44.8 µs  943.1 µs    987.9 µs       3.1x Nisaba          1.06x Skia
85. Composite Op: Source-In (Alpha Masking)   12.0 µs  992.7 µs   1004.7 µs   45.1 µs 1272.4 µs   1317.5 µs       3.8x Nisaba        1.31x Nisaba
86. Composite Op: Source-Out (Cutout)        13.6 µs  994.6 µs   1008.2 µs   41.5 µs 1259.8 µs   1301.3 µs       3.1x Nisaba        1.29x Nisaba
87. Composite Op: Atop (Target Bounds)       15.8 µs  983.3 µs    999.1 µs   45.8 µs  951.8 µs    997.6 µs       2.9x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   14.3 µs  983.5 µs    997.8 µs   42.6 µs  953.4 µs    996.0 µs       3.0x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     15.8 µs  984.7 µs   1000.5 µs   43.6 µs  951.9 µs    995.5 µs       2.8x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        17.4 µs  960.7 µs    978.1 µs   42.8 µs 1086.0 µs   1128.8 µs       2.5x Nisaba        1.15x Nisaba
91. Composite Op: Xor (Exclusive Blend)      15.5 µs 1008.4 µs   1024.0 µs   45.2 µs  877.2 µs    922.4 µs       2.9x Nisaba          1.11x Skia
92. Composite Op: Copy (Direct Overwrite)    16.1 µs  952.0 µs    968.1 µs   45.9 µs  948.2 µs    994.1 µs       2.8x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   30.7 µs  970.9 µs   1001.5 µs  210.4 µs  989.4 µs   1199.8 µs       6.9x Nisaba        1.20x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      32.6 µs 1135.0 µs   1167.6 µs  274.7 µs 1080.3 µs   1354.9 µs       8.4x Nisaba        1.16x Nisaba
95. Circular Progress Rings (8 Meters)       43.4 µs  954.5 µs    997.9 µs  390.6 µs 1075.8 µs   1466.3 µs       9.0x Nisaba        1.47x Nisaba
96. Modern Card Stack (5 Elevated Cards)     16.4 µs 1678.8 µs   1695.2 µs   81.2 µs 1546.9 µs   1628.0 µs       4.9x Nisaba         Tie (~1.0x)
97. CAD Cross-Hatch Pattern (80 Angled Lines)   25.8 µs  932.7 µs    958.5 µs   42.0 µs  995.6 µs   1037.6 µs       1.6x Nisaba        1.08x Nisaba
98. Floating Action Button (FAB 10 Buttons)   38.7 µs  969.9 µs   1008.5 µs  122.1 µs  915.1 µs   1037.3 µs       3.2x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   29.9 µs  977.8 µs   1007.8 µs  118.7 µs  948.8 µs   1067.5 µs       4.0x Nisaba        1.06x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   20.4 µs 1350.2 µs   1370.6 µs   71.9 µs 1077.4 µs   1149.3 µs       3.5x Nisaba          1.19x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          2715.8 µs106028.9 µs 108744.7 µs19380.1 µs103302.0 µs 122682.1 µs      7.14x Nisaba        1.13x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 43 suites | Skia won in 27 suites | Ties: 30
         Overall Frame Throughput (End-to-End): 1.13x Nisaba
         Overall CPU Command Submission:       7.14x Nisaba
===================================================================================================================================================
```
</details>

# Benchmark Report: Nisaba GPU Engine vs Google Skia (Ganesh GL)

**Execution Date:** 2026-10-03 15:02:35  
**Runs Executed:** 10 consecutive runs  
**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  

---

## Executive Summary & Aggregate Averages

Across **10 consecutive executions** of the 100-suite GPU micro-profiling benchmark:
- **Overall CPU Command Submission Overhead:** **4.69x FASTER (Nisaba)**
  * Nisaba Mean CPU Overhead: `4020.7 µs` (4.02 ms)
  * Skia Mean CPU Overhead: `18872.6 µs` (18.87 ms)
- **Overall End-to-End Frame Throughput:** **1.09x FASTER (Nisaba)**
  * Nisaba Mean Total Frame Time: `112836.6 µs` (112.84 ms)
  * Skia Mean Total Frame Time: `123364.9 µs` (123.36 ms)

---

## 10-Run Mean Results Across All 100 GPU Suites

| # | Suite Name | Nisaba CPU (µs) | Nisaba GPU (µs) | Nisaba Total (µs) | Skia CPU (µs) | Skia GPU (µs) | Skia Total (µs) | CPU Ratio | Total Ratio |
| :---: | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 1 | Solid Background Clear (1080x720) | **16.3** | 869.5 | **885.8** | 10.4 | 823.0 | 833.5 | 1.6x Skia | 1.06x Skia |
| 2 | 100 Grid Lines (1px Hairlines) | **39.4** | 965.0 | **1004.4** | 43.1 | 1031.4 | 1074.5 | **1.1x Nisaba** | **1.07x Nisaba** |
| 3 | 20 Thick Diagonal Lines (16px Round) | **29.8** | 1009.8 | **1039.5** | 238.4 | 1078.8 | 1317.2 | **8.0x Nisaba** | **1.27x Nisaba** |
| 4 | 100 Solid Rectangles (Varied Bounds) | **25.3** | 1136.6 | **1161.9** | 75.3 | 928.1 | 1003.4 | **3.0x Nisaba** | 1.16x Skia |
| 5 | 100 Alpha Rectangles (Overlapping Blend) | **26.1** | 1548.0 | **1574.1** | 72.2 | 1282.7 | 1354.8 | **2.8x Nisaba** | 1.16x Skia |
| 6 | 50 Simple Rounded Rects (12px Radius) | **20.8** | 1358.2 | **1379.0** | 61.0 | 1101.9 | 1162.9 | **2.9x Nisaba** | 1.19x Skia |
| 7 | 50 Varying-Radius Rounded Rects | **22.3** | 1347.6 | **1370.0** | 46.8 | 1024.7 | 1071.5 | **2.1x Nisaba** | 1.28x Skia |
| 8 | 50 Stroked Rounded Rects (1.5px Border) | **82.2** | 918.8 | **1001.0** | 59.1 | 949.9 | 1009.0 | 1.4x Skia | Tie (~1.0x) |
| 9 | 100 Alpha Circles / Disks (Fill) | **24.8** | 1115.2 | **1140.0** | 69.9 | 947.4 | 1017.3 | **2.8x Nisaba** | 1.12x Skia |
| 10 | 25 Concentric Rings (2px Stroke) | **87.9** | 919.8 | **1007.8** | 40.8 | 1076.6 | 1117.4 | 2.2x Skia | **1.11x Nisaba** |
| 11 | 50 Two-Stop Linear Gradients | **99.7** | 1541.7 | **1641.5** | 183.2 | 1308.5 | 1491.7 | **1.8x Nisaba** | 1.10x Skia |
| 12 | 50 Diagonal Gradients (45° Angle) | **99.1** | 1414.9 | **1514.0** | 184.5 | 1169.0 | 1353.5 | **1.9x Nisaba** | 1.12x Skia |
| 13 | 25 Radial Glow Gradients (300px) | **28.2** | 2158.8 | **2186.9** | 102.9 | 1487.8 | 1590.7 | **3.7x Nisaba** | 1.37x Skia |
| 14 | 20 Box Blur / Drop Shadows (15px) | **27.2** | 1247.8 | **1275.0** | 96.9 | 1314.3 | 1411.3 | **3.6x Nisaba** | **1.11x Nisaba** |
| 15 | 10 Concave 10-Point Stars (Fill) | **40.8** | 1073.3 | **1114.1** | 377.2 | 1110.1 | 1487.3 | **9.3x Nisaba** | **1.34x Nisaba** |
| 16 | 10 Concave 10-Point Stars (3px Stroke) | **31.2** | 967.1 | **998.3** | 893.1 | 1121.7 | 2014.8 | **28.6x Nisaba** | **2.02x Nisaba** |
| 17 | 1000-Point Waveform (Dense Stroke) | **88.7** | 922.7 | **1011.4** | 871.2 | 1232.6 | 2103.8 | **9.8x Nisaba** | **2.08x Nisaba** |
| 18 | 50 Rotated Cards (Affine Transforms) | **92.2** | 1122.7 | **1214.9** | 64.3 | 1013.7 | 1078.0 | 1.4x Skia | 1.13x Skia |
| 19 | 25 Scissor Clipped Viewports | **23.4** | 1018.5 | **1041.9** | 64.6 | 570.6 | 635.3 | **2.8x Nisaba** | 1.64x Skia |
| 20 | 50 UI Text Labels (Inter-Regular) | **106.7** | 894.9 | **1001.6** | 94.0 | 905.7 | 999.7 | 1.1x Skia | Tie (~1.0x) |
| 21 | 25 Quadratic Bezier Curves (Fill+Stroke) | **68.7** | 945.4 | **1014.1** | 361.9 | 1164.0 | 1526.0 | **5.3x Nisaba** | **1.50x Nisaba** |
| 22 | 20 Cubic Bezier Splines (4px Stroke) | **45.6** | 987.9 | **1033.5** | 1242.6 | 1382.1 | 2624.7 | **27.2x Nisaba** | **2.54x Nisaba** |
| 23 | Complex Bezier Ribbon (24px Alpha) | **20.8** | 976.6 | **997.5** | 174.4 | 1169.3 | 1343.7 | **8.4x Nisaba** | **1.35x Nisaba** |
| 24 | Circular Arcs with arcTo (20 Corners) | **33.9** | 966.2 | **1000.0** | 506.4 | 1262.2 | 1768.6 | **15.0x Nisaba** | **1.77x Nisaba** |
| 25 | Directional Arcs with arc() (30 Arcs) | **61.0** | 951.4 | **1012.4** | 147.1 | 894.4 | 1041.5 | **2.4x Nisaba** | Tie (~1.0x) |
| 26 | Compound Path with Holes (15 Donuts) | **73.0** | 1327.2 | **1400.2** | 531.7 | 1180.2 | 1711.8 | **7.3x Nisaba** | **1.22x Nisaba** |
| 27 | Trefoil Figure-8 Cubic Knot (8 Loops) | **43.3** | 958.5 | **1001.8** | 477.5 | 1133.8 | 1611.3 | **11.0x Nisaba** | **1.61x Nisaba** |
| 28 | Archimedean Spiral Polyline (500 Pts) | **51.5** | 950.6 | **1002.1** | 846.7 | 1234.4 | 2081.1 | **16.4x Nisaba** | **2.08x Nisaba** |
| 29 | Multi-Contour Disjoint Paths (25 Sub-Paths) | **16.7** | 968.1 | **984.8** | 259.2 | 1198.9 | 1458.1 | **15.5x Nisaba** | **1.48x Nisaba** |
| 30 | Sharp Zigzag Mountain (50 Vertices) | **27.3** | 1446.5 | **1473.8** | 1239.8 | 1381.8 | 2621.6 | **45.4x Nisaba** | **1.78x Nisaba** |
| 31 | Stroke Join: Miter (12px Stroke) | **18.0** | 990.3 | **1008.2** | 402.1 | 1175.6 | 1577.8 | **22.4x Nisaba** | **1.56x Nisaba** |
| 32 | Stroke Join: Bevel (12px Stroke) | **19.1** | 966.5 | **985.5** | 391.2 | 1167.5 | 1558.7 | **20.5x Nisaba** | **1.58x Nisaba** |
| 33 | Stroke Join: Round (12px Stroke) | **22.0** | 973.8 | **995.8** | 407.9 | 1186.1 | 1594.0 | **18.5x Nisaba** | **1.60x Nisaba** |
| 34 | Stroke Cap: Butt (30 Segments 12px) | **25.1** | 939.3 | **964.4** | 45.2 | 973.3 | 1018.6 | **1.8x Nisaba** | **1.06x Nisaba** |
| 35 | Stroke Cap: Square (30 Segments 12px) | **24.6** | 981.5 | **1006.1** | 46.0 | 986.2 | 1032.2 | **1.9x Nisaba** | Tie (~1.0x) |
| 36 | Stroke Cap: Round (30 Segments 12px) | **35.1** | 977.3 | **1012.4** | 62.2 | 951.7 | 1013.9 | **1.8x Nisaba** | Tie (~1.0x) |
| 37 | Miter Limit Clamping (10° Acute Spikes) | **22.4** | 976.0 | **998.4** | 696.5 | 1279.8 | 1976.3 | **31.1x Nisaba** | **1.98x Nisaba** |
| 38 | Sub-Pixel Hairline Strokes (0.25px Lines) | **33.1** | 967.7 | **1000.8** | 68.4 | 949.0 | 1017.5 | **2.1x Nisaba** | Tie (~1.0x) |
| 39 | Ultra-Heavy Geometric Ribbon (40px Wide) | **17.7** | 936.1 | **953.8** | 151.7 | 1105.0 | 1256.8 | **8.6x Nisaba** | **1.32x Nisaba** |
| 40 | Stepped Variable Stroke Widths (1-20px) | **29.8** | 968.1 | **997.9** | 547.6 | 1216.7 | 1764.3 | **18.4x Nisaba** | **1.77x Nisaba** |
| 41 | 50 Ellipses (rx=60, ry=25 Fill) | **21.1** | 1105.0 | **1126.1** | 42.0 | 966.4 | 1008.3 | **2.0x Nisaba** | 1.12x Skia |
| 42 | 50 Stroked Ellipses (2.5px Border) | **108.8** | 900.5 | **1009.4** | 41.7 | 1225.8 | 1267.4 | 2.6x Skia | **1.26x Nisaba** |
| 43 | 50 Pill / Capsule Badges (r=h/2) | **22.4** | 1186.5 | **1208.9** | 60.1 | 956.4 | 1016.5 | **2.7x Nisaba** | 1.19x Skia |
| 44 | 50 Asymmetric Teardrop Rounded Rects | **23.8** | 1238.1 | **1261.9** | 48.5 | 942.4 | 990.9 | **2.0x Nisaba** | 1.27x Skia |
| 45 | High-Density Disks Cloud (500 Disks) | **57.1** | 963.9 | **1021.0** | 210.8 | 922.4 | 1133.2 | **3.7x Nisaba** | **1.11x Nisaba** |
| 46 | High-Density Stroked Rings (250 Rings) | **38.9** | 1063.6 | **1102.5** | 132.9 | 877.1 | 1010.0 | **3.4x Nisaba** | 1.09x Skia |
| 47 | 250 Filled Rectangles Batch | **39.5** | 1236.8 | **1276.3** | 129.2 | 898.0 | 1027.2 | **3.3x Nisaba** | 1.24x Skia |
| 48 | 150 Stroked Rectangles (2px Border) | **68.8** | 951.1 | **1020.0** | 83.5 | 946.0 | 1029.5 | **1.2x Nisaba** | Tie (~1.0x) |
| 49 | 40 Crosshair Aim Reticles | **84.8** | 928.3 | **1013.1** | 295.9 | 1025.2 | 1321.1 | **3.5x Nisaba** | **1.30x Nisaba** |
| 50 | 25 Regular Octagons (Fill + Stroke) | **49.0** | 953.9 | **1002.9** | 300.1 | 1109.2 | 1409.3 | **6.1x Nisaba** | **1.41x Nisaba** |
| 51 | 30 Vertical Card Linear Gradients | **66.5** | 1228.7 | **1295.2** | 147.1 | 975.2 | 1122.3 | **2.2x Nisaba** | 1.15x Skia |
| 52 | 30 Horizontal Bar Progress Gradients | **60.3** | 1263.7 | **1324.0** | 146.9 | 1001.8 | 1148.6 | **2.4x Nisaba** | 1.15x Skia |
| 53 | 15 Concentric Radial Spotlights | **60.5** | 1238.3 | **1298.8** | 89.0 | 931.7 | 1020.6 | **1.5x Nisaba** | 1.27x Skia |
| 54 | 25 Soft Ambient Card Shadows | **30.6** | 1410.3 | **1441.0** | 122.8 | 1085.8 | 1208.5 | **4.0x Nisaba** | 1.19x Skia |
| 55 | 15 Neon Button Glows (Cyan Intense) | **26.4** | 1164.1 | **1190.5** | 82.8 | 944.7 | 1027.5 | **3.1x Nisaba** | 1.16x Skia |
| 56 | 16 Multi-Angle Gradient Fan Slices | **35.6** | 991.4 | **1027.0** | 104.1 | 911.6 | 1015.7 | **2.9x Nisaba** | Tie (~1.0x) |
| 57 | Full-Screen Diagonal Horizon Gradient | **17.1** | 1193.5 | **1210.6** | 32.6 | 961.8 | 994.4 | **1.9x Nisaba** | 1.22x Skia |
| 58 | 20 Inset Well Shadows (Recessed) | **50.7** | 1329.4 | **1380.1** | 102.2 | 1046.4 | 1148.6 | **2.0x Nisaba** | 1.20x Skia |
| 59 | 10 Pulsing Circular Radar Wave Glows | **51.5** | 2007.4 | **2058.9** | 63.5 | 1679.0 | 1742.5 | **1.2x Nisaba** | 1.18x Skia |
| 60 | Waveform Polyline Stroked with Gradient | **55.5** | 951.7 | **1007.2** | 566.1 | 1276.0 | 1842.1 | **10.2x Nisaba** | **1.83x Nisaba** |
| 61 | Repeated Texture Tiling (64x64 Pattern) | **18.9** | 1237.9 | **1256.8** | 33.1 | 967.4 | 1000.5 | **1.7x Nisaba** | 1.26x Skia |
| 62 | Scaled Image Quad (2x 512x512 Blit) | **19.4** | 1008.2 | **1027.7** | 28.5 | 944.7 | 973.2 | **1.5x Nisaba** | 1.06x Skia |
| 63 | 45° Rotated Pattern Fill in Circle | **22.4** | 991.5 | **1013.9** | 36.2 | 951.7 | 987.9 | **1.6x Nisaba** | Tie (~1.0x) |
| 64 | Translucent Texture Overlay (50% Alpha) | **21.8** | 1238.2 | **1260.0** | 35.5 | 964.2 | 999.7 | **1.6x Nisaba** | 1.26x Skia |
| 65 | Multi-Avatar Grid (16 Avatars) | **57.3** | 985.9 | **1043.2** | 43.0 | 953.4 | 996.3 | 1.3x Skia | Tie (~1.0x) |
| 66 | Texture Pattern on Star Path | **23.8** | 975.4 | **999.2** | 131.6 | 903.4 | 1035.0 | **5.5x Nisaba** | Tie (~1.0x) |
| 67 | 20px Curved Path Stroked with Texture | **22.6** | 975.4 | **998.0** | 167.3 | 1143.0 | 1310.2 | **7.4x Nisaba** | **1.31x Nisaba** |
| 68 | Minified Texture Quad (0.25x Downscale) | **45.5** | 973.8 | **1019.2** | 69.1 | 935.4 | 1004.5 | **1.5x Nisaba** | Tie (~1.0x) |
| 69 | Pure Translation Stack (50 Cards) | **24.2** | 1126.2 | **1150.5** | 63.8 | 935.2 | 999.0 | **2.6x Nisaba** | 1.15x Skia |
| 70 | Pure Rotation Cluster (36 Spokes 10°) | **33.9** | 951.1 | **985.0** | 54.2 | 941.4 | 995.5 | **1.6x Nisaba** | Tie (~1.0x) |
| 71 | Pure Scale Zoom Progression (20 Rects) | **28.5** | 968.9 | **997.4** | 36.0 | 956.8 | 992.9 | **1.3x Nisaba** | Tie (~1.0x) |
| 72 | Shear / Skew Parallelograms (30 Quads) | **30.2** | 943.2 | **973.5** | 49.0 | 965.8 | 1014.9 | **1.6x Nisaba** | Tie (~1.0x) |
| 73 | Combined Affine Transforms (30 Shapes) | **66.2** | 944.2 | **1010.4** | 44.4 | 938.3 | 982.7 | 1.5x Skia | Tie (~1.0x) |
| 74 | Deep Hierarchical State Stack (20 Levels) | **24.7** | 954.3 | **979.0** | 40.6 | 959.2 | 999.8 | **1.6x Nisaba** | Tie (~1.0x) |
| 75 | Reset Transform Stress (50 Cycles) | **23.8** | 971.0 | **994.8** | 54.3 | 934.8 | 989.1 | **2.3x Nisaba** | Tie (~1.0x) |
| 76 | Planetary Orbit Hierarchy (10 Systems) | **40.7** | 948.3 | **989.0** | 42.4 | 948.2 | 990.6 | Tie (~1.0x) | Tie (~1.0x) |
| 77 | Multi-Scissor Grid (16 Viewports) | **21.3** | 1018.6 | **1039.9** | 56.9 | 708.1 | 765.1 | **2.7x Nisaba** | 1.36x Skia |
| 78 | Hierarchical Intersecting Scissors (4 Levels) | **21.1** | 1224.8 | **1245.9** | 39.6 | 1009.8 | 1049.3 | **1.9x Nisaba** | 1.19x Skia |
| 79 | Scissored Stroked Waves (1000 Pts Clipped) | **86.9** | 923.0 | **1009.9** | 897.6 | 943.9 | 1841.6 | **10.3x Nisaba** | **1.82x Nisaba** |
| 80 | Scissored Gradient Card (Clipped) | **67.4** | 1039.1 | **1106.4** | 80.8 | 595.4 | 676.2 | **1.2x Nisaba** | 1.64x Skia |
| 81 | Circular Overflow Clip (25 Boxes) | **24.1** | 997.4 | **1021.5** | 70.5 | 505.2 | 575.7 | **2.9x Nisaba** | 1.77x Skia |
| 82 | Scissor Invalidation / Reset (25 Cycles) | **21.4** | 969.0 | **990.4** | 50.1 | 910.5 | 960.7 | **2.3x Nisaba** | Tie (~1.0x) |
| 83 | Composite Op: Source-Over (30 Shapes) | **20.1** | 1006.4 | **1026.5** | 43.2 | 943.8 | 987.0 | **2.2x Nisaba** | Tie (~1.0x) |
| 84 | Composite Op: Lighter / Plus (30 Particles) | **20.8** | 1095.5 | **1116.3** | 45.2 | 936.8 | 982.0 | **2.2x Nisaba** | 1.14x Skia |
| 85 | Composite Op: Source-In (Alpha Masking) | **20.1** | 1004.6 | **1024.7** | 44.3 | 1197.2 | 1241.6 | **2.2x Nisaba** | **1.21x Nisaba** |
| 86 | Composite Op: Source-Out (Cutout) | **20.1** | 973.3 | **993.3** | 44.0 | 1209.8 | 1253.8 | **2.2x Nisaba** | **1.26x Nisaba** |
| 87 | Composite Op: Atop (Target Bounds) | **18.8** | 956.8 | **975.6** | 43.2 | 951.4 | 994.5 | **2.3x Nisaba** | Tie (~1.0x) |
| 88 | Composite Op: Dest-Over (Under-Drawing) | **19.7** | 1044.3 | **1064.0** | 43.4 | 962.2 | 1005.6 | **2.2x Nisaba** | 1.06x Skia |
| 89 | Composite Op: Dest-Out (Eraser Mask) | **19.5** | 1006.4 | **1025.9** | 42.0 | 945.0 | 987.0 | **2.2x Nisaba** | Tie (~1.0x) |
| 90 | Composite Op: Dest-Atop (Inverse) | **18.7** | 999.2 | **1017.9** | 42.0 | 1228.9 | 1271.0 | **2.3x Nisaba** | **1.25x Nisaba** |
| 91 | Composite Op: Xor (Exclusive Blend) | **18.7** | 997.9 | **1016.6** | 42.3 | 948.9 | 991.2 | **2.3x Nisaba** | Tie (~1.0x) |
| 92 | Composite Op: Copy (Direct Overwrite) | **20.0** | 984.5 | **1004.5** | 41.8 | 953.8 | 995.6 | **2.1x Nisaba** | Tie (~1.0x) |
| 93 | UI Dashboard Gauge (Arc Track + Needle) | **27.3** | 968.7 | **996.0** | 191.9 | 1033.1 | 1225.0 | **7.0x Nisaba** | **1.23x Nisaba** |
| 94 | Audio Spectrum Visualizer (64 Bars) | **103.5** | 1149.5 | **1253.0** | 269.0 | 1072.9 | 1342.0 | **2.6x Nisaba** | **1.07x Nisaba** |
| 95 | Circular Progress Rings (8 Meters) | **52.5** | 949.7 | **1002.2** | 375.8 | 1152.2 | 1528.0 | **7.2x Nisaba** | **1.52x Nisaba** |
| 96 | Modern Card Stack (5 Elevated Cards) | **32.5** | 1844.5 | **1877.0** | 80.2 | 1627.1 | 1707.3 | **2.5x Nisaba** | 1.10x Skia |
| 97 | CAD Cross-Hatch Pattern (80 Angled Lines) | **38.3** | 966.8 | **1005.1** | 40.2 | 1170.7 | 1210.9 | Tie (~1.0x) | **1.20x Nisaba** |
| 98 | Floating Action Button (FAB 10 Buttons) | **51.6** | 975.6 | **1027.3** | 117.9 | 891.2 | 1009.1 | **2.3x Nisaba** | Tie (~1.0x) |
| 99 | Anti-Aliasing Geometry Grid (50 Diamonds) | **36.9** | 975.3 | **1012.2** | 112.9 | 919.0 | 1031.9 | **3.1x Nisaba** | Tie (~1.0x) |
| 100 | Master Vector Stress: Mixed Mega-Scene | **65.9** | 1539.2 | **1605.1** | 72.2 | 1178.5 | 1250.8 | **1.1x Nisaba** | 1.28x Skia |
| **--** | **TOTAL FRAME OVERHEAD** | **4020.7** | **108815.9** | **112836.6** | **18872.6** | **104492.3** | **123364.9** | **4.69x Nisaba** | **1.09x Nisaba** |

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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 972.1 µs, Skia: 806.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 978.9 µs, Skia: 1039.2 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 984.1 µs, Skia: 1307.7 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 828.0 µs, Skia: 980.5 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1396.9 µs, Skia: 1298.2 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1419.3 µs, Skia: 1185.0 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1243.5 µs, Skia: 1114.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1017.4 µs, Skia: 997.1 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1180.8 µs, Skia: 1015.6 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 998.1 µs, Skia: 1111.5 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1637.7 µs, Skia: 1477.9 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1536.2 µs, Skia: 1353.3 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2148.0 µs, Skia: 1674.6 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1401.3 µs, Skia: 1549.5 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1053.7 µs, Skia: 1503.4 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 997.7 µs, Skia: 1999.0 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 996.4 µs, Skia: 2099.7 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1258.2 µs, Skia: 1062.2 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1077.5 µs, Skia: 630.9 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 997.9 µs, Skia: 996.7 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1015.1 µs, Skia: 1508.4 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 999.2 µs, Skia: 2650.4 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 992.9 µs, Skia: 1352.3 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1000.8 µs, Skia: 1755.5 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 998.2 µs, Skia: 1037.7 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1309.1 µs, Skia: 1711.6 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 997.7 µs, Skia: 1595.9 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1011.1 µs, Skia: 2038.9 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 1041.4 µs, Skia: 1479.4 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1338.2 µs, Skia: 2607.2 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 998.6 µs, Skia: 1552.4 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 978.3 µs, Skia: 1607.1 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 992.0 µs, Skia: 1605.3 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 1017.0 µs, Skia: 1018.6 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 1016.9 µs, Skia: 1029.7 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1007.9 µs, Skia: 1065.1 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 997.6 µs, Skia: 1959.0 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 1007.3 µs, Skia: 1006.7 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 997.1 µs, Skia: 1253.0 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 974.8 µs, Skia: 1813.9 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1130.6 µs, Skia: 1013.0 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1017.4 µs, Skia: 1229.2 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1238.6 µs, Skia: 1005.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1218.6 µs, Skia: 1071.3 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1018.0 µs, Skia: 1110.0 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1089.7 µs, Skia: 1006.2 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1259.2 µs, Skia: 1033.0 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 998.4 µs, Skia: 1016.1 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1064.1 µs, Skia: 1334.7 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1006.8 µs, Skia: 1418.1 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1292.6 µs, Skia: 1126.6 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1326.7 µs, Skia: 1099.7 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1327.9 µs, Skia: 1008.6 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1444.6 µs, Skia: 1210.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1196.5 µs, Skia: 1027.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 997.2 µs, Skia: 998.4 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1187.3 µs, Skia: 1007.2 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1387.2 µs, Skia: 1136.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2150.9 µs, Skia: 1906.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1000.5 µs, Skia: 1913.3 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1284.6 µs, Skia: 1007.7 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1016.4 µs, Skia: 996.7 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1018.4 µs, Skia: 997.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1326.8 µs, Skia: 1048.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1046.6 µs, Skia: 997.3 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1005.8 µs, Skia: 1036.9 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 993.1 µs, Skia: 1303.4 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 998.7 µs, Skia: 998.8 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1204.6 µs, Skia: 1001.5 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 990.2 µs, Skia: 1008.7 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 997.7 µs, Skia: 998.6 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 1001.0 µs, Skia: 997.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1017.3 µs, Skia: 934.9 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 997.2 µs, Skia: 997.5 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 998.1 µs, Skia: 997.2 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 996.4 µs, Skia: 998.4 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1027.1 µs, Skia: 713.9 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1169.8 µs, Skia: 1073.9 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1009.9 µs, Skia: 1807.3 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1064.4 µs, Skia: 667.8 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1003.6 µs, Skia: 574.6 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 997.5 µs, Skia: 989.1 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1087.0 µs, Skia: 996.5 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1202.0 µs, Skia: 1011.2 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1012.9 µs, Skia: 1223.7 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1017.6 µs, Skia: 1225.8 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1018.7 µs, Skia: 977.4 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1248.7 µs, Skia: 1006.8 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1017.3 µs, Skia: 998.0 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1017.5 µs, Skia: 1308.5 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1004.7 µs, Skia: 988.3 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1021.8 µs, Skia: 997.0 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 998.5 µs, Skia: 1227.9 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1255.7 µs, Skia: 1333.6 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1004.2 µs, Skia: 1492.8 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1872.6 µs, Skia: 1682.3 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1001.7 µs, Skia: 1228.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 996.3 µs, Skia: 1017.2 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 975.5 µs, Skia: 1173.7 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1119.4 µs, Skia: 1248.8 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         21.7 µs  950.4 µs    972.1 µs   12.1 µs  794.6 µs    806.7 µs         1.8x Skia          1.21x Skia
2. 100 Grid Lines (1px Hairlines)            41.9 µs  937.0 µs    978.9 µs   45.1 µs  994.2 µs   1039.2 µs       1.1x Nisaba        1.06x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      27.7 µs  956.4 µs    984.1 µs  251.3 µs 1056.5 µs   1307.7 µs       9.1x Nisaba        1.33x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      23.0 µs  804.9 µs    828.0 µs   66.3 µs  914.2 µs    980.5 µs       2.9x Nisaba        1.18x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   21.6 µs 1375.2 µs   1396.9 µs   77.7 µs 1220.5 µs   1298.2 µs       3.6x Nisaba          1.08x Skia
6. 50 Simple Rounded Rects (12px Radius)     21.7 µs 1397.6 µs   1419.3 µs   63.9 µs 1121.1 µs   1185.0 µs       3.0x Nisaba          1.20x Skia
7. 50 Varying-Radius Rounded Rects           21.2 µs 1222.3 µs   1243.5 µs   48.8 µs 1065.6 µs   1114.5 µs       2.3x Nisaba          1.12x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   84.6 µs  932.7 µs   1017.4 µs   59.9 µs  937.3 µs    997.1 µs         1.4x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          25.8 µs 1155.0 µs   1180.8 µs   70.5 µs  945.1 µs   1015.6 µs       2.7x Nisaba          1.16x Skia
10. 25 Concentric Rings (2px Stroke)         87.2 µs  910.9 µs    998.1 µs   44.6 µs 1066.9 µs   1111.5 µs         2.0x Skia        1.11x Nisaba
11. 50 Two-Stop Linear Gradients             95.3 µs 1542.4 µs   1637.7 µs  191.1 µs 1286.8 µs   1477.9 µs       2.0x Nisaba          1.11x Skia
12. 50 Diagonal Gradients (45° Angle)      101.9 µs 1434.3 µs   1536.2 µs  181.1 µs 1172.2 µs   1353.3 µs       1.8x Nisaba          1.14x Skia
13. 25 Radial Glow Gradients (300px)         28.5 µs 2119.5 µs   2148.0 µs  100.2 µs 1574.4 µs   1674.6 µs       3.5x Nisaba          1.28x Skia
14. 20 Box Blur / Drop Shadows (15px)        26.0 µs 1375.4 µs   1401.3 µs  101.0 µs 1448.5 µs   1549.5 µs       3.9x Nisaba        1.11x Nisaba
15. 10 Concave 10-Point Stars (Fill)         41.8 µs 1011.9 µs   1053.7 µs  382.5 µs 1120.9 µs   1503.4 µs       9.1x Nisaba        1.43x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   30.3 µs  967.4 µs    997.7 µs  869.5 µs 1129.5 µs   1999.0 µs      28.7x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       87.8 µs  908.7 µs    996.4 µs  878.7 µs 1221.0 µs   2099.7 µs      10.0x Nisaba        2.11x Nisaba
18. 50 Rotated Cards (Affine Transforms)     96.3 µs 1161.9 µs   1258.2 µs   61.7 µs 1000.5 µs   1062.2 µs         1.6x Skia          1.18x Skia
19. 25 Scissor Clipped Viewports             22.9 µs 1054.7 µs   1077.5 µs   64.7 µs  566.2 µs    630.9 µs       2.8x Nisaba          1.71x Skia
20. 50 UI Text Labels (Inter-Regular)       107.8 µs  890.1 µs    997.9 µs   95.5 µs  901.2 µs    996.7 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   70.4 µs  944.7 µs   1015.1 µs  351.8 µs 1156.6 µs   1508.4 µs       5.0x Nisaba        1.49x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     44.7 µs  954.5 µs    999.2 µs 1247.3 µs 1403.1 µs   2650.4 µs      27.9x Nisaba        2.65x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       23.9 µs  969.0 µs    992.9 µs  177.4 µs 1174.9 µs   1352.3 µs       7.4x Nisaba        1.36x Nisaba
24. Circular Arcs with arcTo (20 Corners)    31.4 µs  969.4 µs   1000.8 µs  518.1 µs 1237.3 µs   1755.5 µs      16.5x Nisaba        1.75x Nisaba
25. Directional Arcs with arc() (30 Arcs)    57.6 µs  940.6 µs    998.2 µs  152.2 µs  885.6 µs   1037.7 µs       2.6x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     76.5 µs 1232.6 µs   1309.1 µs  534.4 µs 1177.2 µs   1711.6 µs       7.0x Nisaba        1.31x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    43.4 µs  954.3 µs    997.7 µs  476.6 µs 1119.3 µs   1595.9 µs      11.0x Nisaba        1.60x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    51.2 µs  959.9 µs   1011.1 µs  830.9 µs 1208.1 µs   2038.9 µs      16.2x Nisaba        2.02x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   16.5 µs 1024.9 µs   1041.4 µs  265.6 µs 1213.8 µs   1479.4 µs      16.1x Nisaba        1.42x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      26.4 µs 1311.9 µs   1338.2 µs 1235.2 µs 1372.0 µs   2607.2 µs      46.9x Nisaba        1.95x Nisaba
31. Stroke Join: Miter (12px Stroke)         18.4 µs  980.1 µs    998.6 µs  391.7 µs 1160.8 µs   1552.4 µs      21.2x Nisaba        1.55x Nisaba
32. Stroke Join: Bevel (12px Stroke)         21.4 µs  956.9 µs    978.3 µs  398.8 µs 1208.3 µs   1607.1 µs      18.6x Nisaba        1.64x Nisaba
33. Stroke Join: Round (12px Stroke)         21.1 µs  970.9 µs    992.0 µs  410.2 µs 1195.1 µs   1605.3 µs      19.5x Nisaba        1.62x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      26.3 µs  990.8 µs   1017.0 µs   46.5 µs  972.1 µs   1018.6 µs       1.8x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    24.4 µs  992.5 µs   1016.9 µs   46.0 µs  983.8 µs   1029.7 µs       1.9x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     34.0 µs  973.9 µs   1007.9 µs   67.4 µs  997.7 µs   1065.1 µs       2.0x Nisaba        1.06x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   22.4 µs  975.1 µs    997.6 µs  711.0 µs 1248.0 µs   1959.0 µs      31.7x Nisaba        1.96x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   30.7 µs  976.6 µs   1007.3 µs   69.5 µs  937.2 µs   1006.7 µs       2.3x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   17.3 µs  979.8 µs    997.1 µs  148.4 µs 1104.6 µs   1253.0 µs       8.6x Nisaba        1.26x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   30.2 µs  944.6 µs    974.8 µs  562.4 µs 1251.5 µs   1813.9 µs      18.6x Nisaba        1.86x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          21.6 µs 1108.9 µs   1130.6 µs   42.9 µs  970.1 µs   1013.0 µs       2.0x Nisaba          1.12x Skia
42. 50 Stroked Ellipses (2.5px Border)      109.6 µs  907.8 µs   1017.4 µs   41.1 µs 1188.1 µs   1229.2 µs         2.7x Skia        1.21x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         22.7 µs 1215.9 µs   1238.6 µs   57.7 µs  947.9 µs   1005.6 µs       2.5x Nisaba          1.23x Skia
44. 50 Asymmetric Teardrop Rounded Rects     23.7 µs 1195.0 µs   1218.6 µs   51.2 µs 1020.1 µs   1071.3 µs       2.2x Nisaba          1.14x Skia
45. High-Density Disks Cloud (500 Disks)     56.6 µs  961.4 µs   1018.0 µs  209.6 µs  900.4 µs   1110.0 µs       3.7x Nisaba        1.09x Nisaba
46. High-Density Stroked Rings (250 Rings)   38.6 µs 1051.2 µs   1089.7 µs  133.1 µs  873.1 µs   1006.2 µs       3.5x Nisaba          1.08x Skia
47. 250 Filled Rectangles Batch              37.5 µs 1221.7 µs   1259.2 µs  129.9 µs  903.2 µs   1033.0 µs       3.5x Nisaba          1.22x Skia
48. 150 Stroked Rectangles (2px Border)      68.1 µs  930.2 µs    998.4 µs   87.0 µs  929.1 µs   1016.1 µs       1.3x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                89.2 µs  974.9 µs   1064.1 µs  298.4 µs 1036.2 µs   1334.7 µs       3.3x Nisaba        1.25x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      54.3 µs  952.5 µs   1006.8 µs  299.6 µs 1118.5 µs   1418.1 µs       5.5x Nisaba        1.41x Nisaba
51. 30 Vertical Card Linear Gradients        64.2 µs 1228.4 µs   1292.6 µs  142.2 µs  984.3 µs   1126.6 µs       2.2x Nisaba          1.15x Skia
52. 30 Horizontal Bar Progress Gradients     60.1 µs 1266.7 µs   1326.7 µs  142.0 µs  957.7 µs   1099.7 µs       2.4x Nisaba          1.21x Skia
53. 15 Concentric Radial Spotlights          63.3 µs 1264.6 µs   1327.9 µs   87.3 µs  921.3 µs   1008.6 µs       1.4x Nisaba          1.32x Skia
54. 25 Soft Ambient Card Shadows             31.1 µs 1413.5 µs   1444.6 µs  121.2 µs 1089.3 µs   1210.5 µs       3.9x Nisaba          1.19x Skia
55. 15 Neon Button Glows (Cyan Intense)      24.8 µs 1171.7 µs   1196.5 µs   83.0 µs  944.4 µs   1027.5 µs       3.3x Nisaba          1.16x Skia
56. 16 Multi-Angle Gradient Fan Slices       33.2 µs  963.9 µs    997.2 µs  109.2 µs  889.2 µs    998.4 µs       3.3x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    16.9 µs 1170.4 µs   1187.3 µs   30.5 µs  976.7 µs   1007.2 µs       1.8x Nisaba          1.18x Skia
58. 20 Inset Well Shadows (Recessed)         51.3 µs 1335.9 µs   1387.2 µs   99.0 µs 1037.4 µs   1136.5 µs       1.9x Nisaba          1.22x Skia
59. 10 Pulsing Circular Radar Wave Glows     49.0 µs 2101.9 µs   2150.9 µs   56.4 µs 1849.9 µs   1906.3 µs       1.2x Nisaba          1.13x Skia
60. Waveform Polyline Stroked with Gradient   51.4 µs  949.1 µs   1000.5 µs  579.1 µs 1334.3 µs   1913.3 µs      11.3x Nisaba        1.91x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   21.7 µs 1263.0 µs   1284.6 µs   34.4 µs  973.3 µs   1007.7 µs       1.6x Nisaba          1.27x Skia
62. Scaled Image Quad (2x 512x512 Blit)      20.4 µs  996.0 µs   1016.4 µs   30.3 µs  966.3 µs    996.7 µs       1.5x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      21.1 µs  997.3 µs   1018.4 µs   34.8 µs  962.6 µs    997.4 µs       1.6x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   21.5 µs 1305.3 µs   1326.8 µs   37.6 µs 1010.4 µs   1048.0 µs       1.8x Nisaba          1.27x Skia
65. Multi-Avatar Grid (16 Avatars)           56.8 µs  989.8 µs   1046.6 µs   43.0 µs  954.3 µs    997.3 µs         1.3x Skia         Tie (~1.0x)
66. Texture Pattern on Star Path             26.1 µs  979.7 µs   1005.8 µs  133.0 µs  903.9 µs   1036.9 µs       5.1x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    24.0 µs  969.1 µs    993.1 µs  168.4 µs 1135.0 µs   1303.4 µs       7.0x Nisaba        1.31x Nisaba
68. Minified Texture Quad (0.25x Downscale)   47.0 µs  951.8 µs    998.7 µs   67.4 µs  931.4 µs    998.8 µs       1.4x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        26.5 µs 1178.2 µs   1204.6 µs   64.3 µs  937.1 µs   1001.5 µs       2.4x Nisaba          1.20x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   32.2 µs  958.0 µs    990.2 µs   48.4 µs  960.4 µs   1008.7 µs       1.5x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   29.3 µs  968.4 µs    997.7 µs   35.2 µs  963.3 µs    998.6 µs       1.2x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   32.2 µs  968.7 µs   1001.0 µs   49.7 µs  947.8 µs    997.6 µs       1.5x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   68.9 µs  948.5 µs   1017.3 µs   45.5 µs  889.4 µs    934.9 µs         1.5x Skia          1.09x Skia
74. Deep Hierarchical State Stack (20 Levels)   24.8 µs  972.5 µs    997.2 µs   38.6 µs  958.8 µs    997.5 µs       1.6x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       24.1 µs  974.0 µs    998.1 µs   54.0 µs  943.2 µs    997.2 µs       2.2x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   40.3 µs  956.1 µs    996.4 µs   41.6 µs  956.7 µs    998.4 µs               Tie         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        20.3 µs 1006.8 µs   1027.1 µs   54.9 µs  659.0 µs    713.9 µs       2.7x Nisaba          1.44x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   20.2 µs 1149.6 µs   1169.8 µs   41.9 µs 1032.1 µs   1073.9 µs       2.1x Nisaba          1.09x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   88.5 µs  921.5 µs   1009.9 µs  891.6 µs  915.7 µs   1807.3 µs      10.1x Nisaba        1.79x Nisaba
80. Scissored Gradient Card (Clipped)        63.5 µs 1000.8 µs   1064.4 µs   81.1 µs  586.7 µs    667.8 µs       1.3x Nisaba          1.59x Skia
81. Circular Overflow Clip (25 Boxes)        24.2 µs  979.4 µs   1003.6 µs   71.9 µs  502.7 µs    574.6 µs       3.0x Nisaba          1.75x Skia
82. Scissor Invalidation / Reset (25 Cycles)   18.3 µs  979.3 µs    997.5 µs   48.0 µs  941.1 µs    989.1 µs       2.6x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    19.7 µs 1067.3 µs   1087.0 µs   42.6 µs  954.0 µs    996.5 µs       2.2x Nisaba          1.09x Skia
84. Composite Op: Lighter / Plus (30 Particles)   21.3 µs 1180.6 µs   1202.0 µs   43.9 µs  967.3 µs   1011.2 µs       2.1x Nisaba          1.19x Skia
85. Composite Op: Source-In (Alpha Masking)   17.3 µs  995.6 µs   1012.9 µs   40.1 µs 1183.6 µs   1223.7 µs       2.3x Nisaba        1.21x Nisaba
86. Composite Op: Source-Out (Cutout)        18.5 µs  999.1 µs   1017.6 µs   40.9 µs 1184.9 µs   1225.8 µs       2.2x Nisaba        1.20x Nisaba
87. Composite Op: Atop (Target Bounds)       17.7 µs 1000.9 µs   1018.7 µs   45.9 µs  931.5 µs    977.4 µs       2.6x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   18.8 µs 1229.9 µs   1248.7 µs   48.6 µs  958.2 µs   1006.8 µs       2.6x Nisaba          1.24x Skia
89. Composite Op: Dest-Out (Eraser Mask)     19.0 µs  998.3 µs   1017.3 µs   43.7 µs  954.3 µs    998.0 µs       2.3x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        20.8 µs  996.7 µs   1017.5 µs   42.7 µs 1265.8 µs   1308.5 µs       2.0x Nisaba        1.29x Nisaba
91. Composite Op: Xor (Exclusive Blend)      18.3 µs  986.4 µs   1004.7 µs   41.4 µs  946.8 µs    988.3 µs       2.3x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    19.5 µs 1002.4 µs   1021.8 µs   40.5 µs  956.4 µs    997.0 µs       2.1x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   26.6 µs  971.8 µs    998.5 µs  191.3 µs 1036.6 µs   1227.9 µs       7.2x Nisaba        1.23x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     103.4 µs 1152.3 µs   1255.7 µs  262.5 µs 1071.1 µs   1333.6 µs       2.5x Nisaba        1.06x Nisaba
95. Circular Progress Rings (8 Meters)       47.8 µs  956.4 µs   1004.2 µs  359.2 µs 1133.6 µs   1492.8 µs       7.5x Nisaba        1.49x Nisaba
96. Modern Card Stack (5 Elevated Cards)     33.2 µs 1839.4 µs   1872.6 µs   79.3 µs 1603.0 µs   1682.3 µs       2.4x Nisaba          1.11x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   40.3 µs  961.4 µs   1001.7 µs   41.0 µs 1187.7 µs   1228.7 µs               Tie        1.23x Nisaba
98. Floating Action Button (FAB 10 Buttons)   52.9 µs  943.3 µs    996.3 µs  118.5 µs  898.7 µs   1017.2 µs       2.2x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   36.7 µs  938.8 µs    975.5 µs  123.6 µs 1050.1 µs   1173.7 µs       3.4x Nisaba        1.20x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   62.4 µs 1057.1 µs   1119.4 µs   72.8 µs 1176.0 µs   1248.8 µs       1.2x Nisaba        1.12x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          4016.1 µs108188.7 µs 112204.8 µs18902.1 µs104906.2 µs 123808.3 µs      4.71x Nisaba        1.10x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 40 suites | Skia won in 35 suites | Ties: 25
         Overall Frame Throughput (End-to-End): 1.10x Nisaba
         Overall CPU Command Submission:       4.71x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 1010.3 µs, Skia: 931.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1007.0 µs, Skia: 1057.7 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 995.9 µs, Skia: 1357.0 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1147.3 µs, Skia: 997.4 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1760.9 µs, Skia: 1348.1 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1327.1 µs, Skia: 1124.5 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1388.0 µs, Skia: 1087.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 997.6 µs, Skia: 997.9 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1096.2 µs, Skia: 997.1 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 998.3 µs, Skia: 1118.7 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1621.4 µs, Skia: 1723.4 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1506.8 µs, Skia: 1356.1 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2046.7 µs, Skia: 1565.3 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1343.1 µs, Skia: 1426.7 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1122.0 µs, Skia: 1487.5 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 997.9 µs, Skia: 1994.4 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1008.1 µs, Skia: 2077.0 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1211.3 µs, Skia: 1062.0 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1038.2 µs, Skia: 632.1 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 998.8 µs, Skia: 997.3 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1020.7 µs, Skia: 1516.6 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1073.1 µs, Skia: 2673.0 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 1000.7 µs, Skia: 1341.4 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1002.1 µs, Skia: 1776.0 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1007.4 µs, Skia: 1068.1 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1412.3 µs, Skia: 1718.5 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1002.4 µs, Skia: 1595.3 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 997.1 µs, Skia: 2069.2 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 987.6 µs, Skia: 1460.3 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1465.0 µs, Skia: 2604.8 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1008.2 µs, Skia: 1525.9 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 829.4 µs, Skia: 1534.5 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 997.0 µs, Skia: 1581.5 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 997.6 µs, Skia: 1005.1 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 1007.7 µs, Skia: 1013.0 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1103.5 µs, Skia: 998.2 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 998.4 µs, Skia: 1986.1 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 998.2 µs, Skia: 1016.6 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 997.1 µs, Skia: 1256.1 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1014.7 µs, Skia: 1789.5 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1109.1 µs, Skia: 996.7 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 998.7 µs, Skia: 1264.6 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1247.9 µs, Skia: 1025.0 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1387.3 µs, Skia: 956.5 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 999.3 µs, Skia: 1164.9 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1118.7 µs, Skia: 1005.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1284.6 µs, Skia: 1011.8 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 997.4 µs, Skia: 1047.7 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1013.5 µs, Skia: 1326.9 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1003.2 µs, Skia: 1410.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1300.1 µs, Skia: 1093.0 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1353.9 µs, Skia: 1158.5 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1309.3 µs, Skia: 1022.0 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1469.4 µs, Skia: 1218.8 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1077.8 µs, Skia: 1008.2 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 998.4 µs, Skia: 1035.3 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1246.1 µs, Skia: 1003.2 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1398.0 µs, Skia: 1107.6 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2176.4 µs, Skia: 1906.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1040.4 µs, Skia: 1837.4 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1316.1 µs, Skia: 1001.3 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1048.3 µs, Skia: 997.2 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1016.9 µs, Skia: 996.2 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1151.4 µs, Skia: 1008.4 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1046.4 µs, Skia: 989.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 996.9 µs, Skia: 1018.5 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1004.9 µs, Skia: 1311.7 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 998.5 µs, Skia: 956.4 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1217.1 µs, Skia: 1009.1 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 998.0 µs, Skia: 1006.3 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 997.1 µs, Skia: 997.2 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 995.8 µs, Skia: 996.3 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1047.2 µs, Skia: 998.1 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 996.0 µs, Skia: 1007.2 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 995.5 µs, Skia: 996.5 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 994.5 µs, Skia: 999.2 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1005.8 µs, Skia: 689.4 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1343.5 µs, Skia: 1069.4 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1010.2 µs, Skia: 1816.8 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1057.9 µs, Skia: 683.6 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1008.3 µs, Skia: 583.3 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 997.9 µs, Skia: 835.4 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1013.8 µs, Skia: 998.6 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1175.9 µs, Skia: 998.8 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1025.7 µs, Skia: 1293.3 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1007.2 µs, Skia: 1299.5 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 998.2 µs, Skia: 997.3 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1013.7 µs, Skia: 997.8 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1017.2 µs, Skia: 868.2 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1014.6 µs, Skia: 1278.7 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1006.6 µs, Skia: 1038.6 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 992.2 µs, Skia: 997.0 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 988.5 µs, Skia: 1226.5 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1255.0 µs, Skia: 1330.1 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1003.3 µs, Skia: 1588.3 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1915.3 µs, Skia: 1703.1 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 995.6 µs, Skia: 1218.3 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1018.3 µs, Skia: 1016.8 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1024.4 µs, Skia: 1031.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1628.0 µs, Skia: 1259.3 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         18.6 µs  991.7 µs   1010.3 µs   10.6 µs  921.1 µs    931.7 µs         1.7x Skia          1.08x Skia
2. 100 Grid Lines (1px Hairlines)            37.9 µs  969.2 µs   1007.0 µs   41.2 µs 1016.5 µs   1057.7 µs       1.1x Nisaba        1.05x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      29.9 µs  966.1 µs    995.9 µs  236.7 µs 1120.3 µs   1357.0 µs       7.9x Nisaba        1.36x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      23.8 µs 1123.5 µs   1147.3 µs   76.4 µs  921.0 µs    997.4 µs       3.2x Nisaba          1.15x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   28.5 µs 1732.4 µs   1760.9 µs   72.9 µs 1275.2 µs   1348.1 µs       2.6x Nisaba          1.31x Skia
6. 50 Simple Rounded Rects (12px Radius)     23.1 µs 1304.0 µs   1327.1 µs   57.0 µs 1067.5 µs   1124.5 µs       2.5x Nisaba          1.18x Skia
7. 50 Varying-Radius Rounded Rects           24.1 µs 1363.9 µs   1388.0 µs   50.1 µs 1037.4 µs   1087.5 µs       2.1x Nisaba          1.28x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   71.5 µs  926.1 µs    997.6 µs   58.8 µs  939.1 µs    997.9 µs         1.2x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          24.9 µs 1071.3 µs   1096.2 µs   70.7 µs  926.4 µs    997.1 µs       2.8x Nisaba          1.10x Skia
10. 25 Concentric Rings (2px Stroke)         88.9 µs  909.4 µs    998.3 µs   37.6 µs 1081.1 µs   1118.7 µs         2.4x Skia        1.12x Nisaba
11. 50 Two-Stop Linear Gradients             94.4 µs 1527.0 µs   1621.4 µs  188.5 µs 1534.9 µs   1723.4 µs       2.0x Nisaba        1.06x Nisaba
12. 50 Diagonal Gradients (45° Angle)       94.4 µs 1412.4 µs   1506.8 µs  186.8 µs 1169.3 µs   1356.1 µs       2.0x Nisaba          1.11x Skia
13. 25 Radial Glow Gradients (300px)         26.2 µs 2020.6 µs   2046.7 µs   99.1 µs 1466.3 µs   1565.3 µs       3.8x Nisaba          1.31x Skia
14. 20 Box Blur / Drop Shadows (15px)        27.3 µs 1315.8 µs   1343.1 µs   93.2 µs 1333.5 µs   1426.7 µs       3.4x Nisaba        1.06x Nisaba
15. 10 Concave 10-Point Stars (Fill)         40.0 µs 1082.1 µs   1122.0 µs  382.0 µs 1105.5 µs   1487.5 µs       9.6x Nisaba        1.33x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   28.7 µs  969.2 µs    997.9 µs  893.3 µs 1101.1 µs   1994.4 µs      31.1x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       77.5 µs  930.6 µs   1008.1 µs  857.4 µs 1219.6 µs   2077.0 µs      11.1x Nisaba        2.06x Nisaba
18. 50 Rotated Cards (Affine Transforms)     89.5 µs 1121.8 µs   1211.3 µs   65.0 µs  997.0 µs   1062.0 µs         1.4x Skia          1.14x Skia
19. 25 Scissor Clipped Viewports             25.8 µs 1012.5 µs   1038.2 µs   62.3 µs  569.8 µs    632.1 µs       2.4x Nisaba          1.64x Skia
20. 50 UI Text Labels (Inter-Regular)       105.3 µs  893.4 µs    998.8 µs   94.4 µs  902.8 µs    997.3 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   67.2 µs  953.5 µs   1020.7 µs  362.2 µs 1154.4 µs   1516.6 µs       5.4x Nisaba        1.49x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     46.6 µs 1026.5 µs   1073.1 µs 1264.5 µs 1408.4 µs   2673.0 µs      27.2x Nisaba        2.49x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       21.4 µs  979.3 µs   1000.7 µs  173.0 µs 1168.5 µs   1341.4 µs       8.1x Nisaba        1.34x Nisaba
24. Circular Arcs with arcTo (20 Corners)    32.6 µs  969.6 µs   1002.1 µs  512.9 µs 1263.1 µs   1776.0 µs      15.8x Nisaba        1.77x Nisaba
25. Directional Arcs with arc() (30 Arcs)    60.9 µs  946.5 µs   1007.4 µs  150.4 µs  917.7 µs   1068.1 µs       2.5x Nisaba        1.06x Nisaba
26. Compound Path with Holes (15 Donuts)     74.1 µs 1338.2 µs   1412.3 µs  538.7 µs 1179.7 µs   1718.5 µs       7.3x Nisaba        1.22x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    41.6 µs  960.8 µs   1002.4 µs  476.3 µs 1119.0 µs   1595.3 µs      11.4x Nisaba        1.59x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    50.3 µs  946.8 µs    997.1 µs  861.5 µs 1207.7 µs   2069.2 µs      17.1x Nisaba        2.08x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   16.6 µs  971.0 µs    987.6 µs  261.3 µs 1199.0 µs   1460.3 µs      15.7x Nisaba        1.48x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      26.5 µs 1438.5 µs   1465.0 µs 1232.8 µs 1372.0 µs   2604.8 µs      46.5x Nisaba        1.78x Nisaba
31. Stroke Join: Miter (12px Stroke)         19.5 µs  988.7 µs   1008.2 µs  391.3 µs 1134.6 µs   1525.9 µs      20.1x Nisaba        1.51x Nisaba
32. Stroke Join: Bevel (12px Stroke)         20.2 µs  809.3 µs    829.4 µs  392.1 µs 1142.4 µs   1534.5 µs      19.4x Nisaba        1.85x Nisaba
33. Stroke Join: Round (12px Stroke)         23.2 µs  973.8 µs    997.0 µs  408.8 µs 1172.7 µs   1581.5 µs      17.6x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      24.0 µs  973.6 µs    997.6 µs   48.2 µs  956.9 µs   1005.1 µs       2.0x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    26.6 µs  981.1 µs   1007.7 µs   44.0 µs  969.0 µs   1013.0 µs       1.7x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     36.2 µs 1067.4 µs   1103.5 µs   63.5 µs  934.6 µs    998.2 µs       1.8x Nisaba          1.11x Skia
37. Miter Limit Clamping (10° Acute Spikes)   22.4 µs  976.0 µs    998.4 µs  706.5 µs 1279.6 µs   1986.1 µs      31.5x Nisaba        1.99x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   33.2 µs  965.0 µs    998.2 µs   68.7 µs  947.9 µs   1016.6 µs       2.1x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   18.8 µs  978.3 µs    997.1 µs  146.1 µs 1109.9 µs   1256.1 µs       7.8x Nisaba        1.26x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   29.2 µs  985.6 µs   1014.7 µs  548.1 µs 1241.4 µs   1789.5 µs      18.8x Nisaba        1.76x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          23.0 µs 1086.1 µs   1109.1 µs   41.1 µs  955.6 µs    996.7 µs       1.8x Nisaba          1.11x Skia
42. 50 Stroked Ellipses (2.5px Border)      110.1 µs  888.6 µs    998.7 µs   40.9 µs 1223.7 µs   1264.6 µs         2.7x Skia        1.27x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         21.5 µs 1226.4 µs   1247.9 µs   60.3 µs  964.7 µs   1025.0 µs       2.8x Nisaba          1.22x Skia
44. 50 Asymmetric Teardrop Rounded Rects     24.3 µs 1363.0 µs   1387.3 µs   46.9 µs  909.6 µs    956.5 µs       1.9x Nisaba          1.45x Skia
45. High-Density Disks Cloud (500 Disks)     56.4 µs  942.9 µs    999.3 µs  214.5 µs  950.4 µs   1164.9 µs       3.8x Nisaba        1.17x Nisaba
46. High-Density Stroked Rings (250 Rings)   36.8 µs 1081.9 µs   1118.7 µs  133.3 µs  871.7 µs   1005.1 µs       3.6x Nisaba          1.11x Skia
47. 250 Filled Rectangles Batch              41.3 µs 1243.3 µs   1284.6 µs  126.5 µs  885.3 µs   1011.8 µs       3.1x Nisaba          1.27x Skia
48. 150 Stroked Rectangles (2px Border)      68.4 µs  928.9 µs    997.4 µs   85.1 µs  962.7 µs   1047.7 µs       1.2x Nisaba        1.05x Nisaba
49. 40 Crosshair Aim Reticles                84.1 µs  929.3 µs   1013.5 µs  301.0 µs 1025.9 µs   1326.9 µs       3.6x Nisaba        1.31x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      47.9 µs  955.2 µs   1003.2 µs  299.6 µs 1110.8 µs   1410.4 µs       6.2x Nisaba        1.41x Nisaba
51. 30 Vertical Card Linear Gradients        68.0 µs 1232.2 µs   1300.1 µs  145.2 µs  947.8 µs   1093.0 µs       2.1x Nisaba          1.19x Skia
52. 30 Horizontal Bar Progress Gradients     59.2 µs 1294.7 µs   1353.9 µs  145.5 µs 1013.0 µs   1158.5 µs       2.5x Nisaba          1.17x Skia
53. 15 Concentric Radial Spotlights          59.0 µs 1250.4 µs   1309.3 µs   88.9 µs  933.1 µs   1022.0 µs       1.5x Nisaba          1.28x Skia
54. 25 Soft Ambient Card Shadows             31.5 µs 1437.9 µs   1469.4 µs  112.6 µs 1106.2 µs   1218.8 µs       3.6x Nisaba          1.21x Skia
55. 15 Neon Button Glows (Cyan Intense)      26.7 µs 1051.1 µs   1077.8 µs   79.8 µs  928.5 µs   1008.2 µs       3.0x Nisaba          1.07x Skia
56. 16 Multi-Angle Gradient Fan Slices       35.2 µs  963.2 µs    998.4 µs  107.4 µs  927.9 µs   1035.3 µs       3.1x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    17.8 µs 1228.3 µs   1246.1 µs   30.8 µs  972.4 µs   1003.2 µs       1.7x Nisaba          1.24x Skia
58. 20 Inset Well Shadows (Recessed)         50.8 µs 1347.2 µs   1398.0 µs  101.5 µs 1006.0 µs   1107.6 µs       2.0x Nisaba          1.26x Skia
59. 10 Pulsing Circular Radar Wave Glows     53.5 µs 2122.9 µs   2176.4 µs   60.6 µs 1845.7 µs   1906.3 µs       1.1x Nisaba          1.14x Skia
60. Waveform Polyline Stroked with Gradient   57.9 µs  982.6 µs   1040.4 µs  565.8 µs 1271.6 µs   1837.4 µs       9.8x Nisaba        1.77x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   18.4 µs 1297.7 µs   1316.1 µs   33.7 µs  967.6 µs   1001.3 µs       1.8x Nisaba          1.31x Skia
62. Scaled Image Quad (2x 512x512 Blit)      18.1 µs 1030.2 µs   1048.3 µs   28.5 µs  968.6 µs    997.2 µs       1.6x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      23.3 µs  993.6 µs   1016.9 µs   35.6 µs  960.6 µs    996.2 µs       1.5x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   23.2 µs 1128.3 µs   1151.4 µs   35.4 µs  973.0 µs   1008.4 µs       1.5x Nisaba          1.14x Skia
65. Multi-Avatar Grid (16 Avatars)           59.9 µs  986.5 µs   1046.4 µs   42.8 µs  946.4 µs    989.2 µs         1.4x Skia          1.06x Skia
66. Texture Pattern on Star Path             24.1 µs  972.8 µs    996.9 µs  131.9 µs  886.6 µs   1018.5 µs       5.5x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    21.1 µs  983.9 µs   1004.9 µs  166.8 µs 1144.9 µs   1311.7 µs       7.9x Nisaba        1.31x Nisaba
68. Minified Texture Quad (0.25x Downscale)   44.8 µs  953.7 µs    998.5 µs   67.8 µs  888.6 µs    956.4 µs       1.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        21.2 µs 1195.9 µs   1217.1 µs   65.3 µs  943.8 µs   1009.1 µs       3.1x Nisaba          1.21x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   31.2 µs  966.8 µs    998.0 µs   49.3 µs  957.0 µs   1006.3 µs       1.6x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   28.0 µs  969.1 µs    997.1 µs   36.1 µs  961.1 µs    997.2 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   30.6 µs  965.3 µs    995.8 µs   49.1 µs  947.1 µs    996.3 µs       1.6x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   61.9 µs  985.3 µs   1047.2 µs   44.4 µs  953.7 µs    998.1 µs         1.4x Skia         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   23.5 µs  972.5 µs    996.0 µs   41.2 µs  965.9 µs   1007.2 µs       1.8x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       22.5 µs  973.1 µs    995.5 µs   54.1 µs  942.4 µs    996.5 µs       2.4x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   39.0 µs  955.5 µs    994.5 µs   40.0 µs  959.3 µs    999.2 µs               Tie         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        20.9 µs  984.9 µs   1005.8 µs   55.4 µs  633.9 µs    689.4 µs       2.7x Nisaba          1.46x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   20.7 µs 1322.8 µs   1343.5 µs   42.5 µs 1026.9 µs   1069.4 µs       2.1x Nisaba          1.26x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   87.2 µs  923.1 µs   1010.2 µs  872.5 µs  944.4 µs   1816.8 µs      10.0x Nisaba        1.80x Nisaba
80. Scissored Gradient Card (Clipped)        69.1 µs  988.8 µs   1057.9 µs   79.6 µs  604.1 µs    683.6 µs       1.2x Nisaba          1.55x Skia
81. Circular Overflow Clip (25 Boxes)        23.5 µs  984.9 µs   1008.3 µs   68.2 µs  515.1 µs    583.3 µs       2.9x Nisaba          1.73x Skia
82. Scissor Invalidation / Reset (25 Cycles)   23.2 µs  974.7 µs    997.9 µs   47.5 µs  787.9 µs    835.4 µs       2.0x Nisaba          1.19x Skia
83. Composite Op: Source-Over (30 Shapes)    17.9 µs  995.9 µs   1013.8 µs   41.9 µs  956.7 µs    998.6 µs       2.3x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   21.2 µs 1154.7 µs   1175.9 µs   47.6 µs  951.2 µs    998.8 µs       2.2x Nisaba          1.18x Skia
85. Composite Op: Source-In (Alpha Masking)   16.6 µs 1009.2 µs   1025.7 µs   47.0 µs 1246.2 µs   1293.3 µs       2.8x Nisaba        1.26x Nisaba
86. Composite Op: Source-Out (Cutout)        16.8 µs  990.4 µs   1007.2 µs   41.2 µs 1258.3 µs   1299.5 µs       2.5x Nisaba        1.29x Nisaba
87. Composite Op: Atop (Target Bounds)       18.0 µs  980.1 µs    998.2 µs   41.4 µs  956.0 µs    997.3 µs       2.3x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   19.2 µs  994.5 µs   1013.7 µs   41.0 µs  956.8 µs    997.8 µs       2.1x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     18.3 µs  998.9 µs   1017.2 µs   43.3 µs  824.9 µs    868.2 µs       2.4x Nisaba          1.17x Skia
90. Composite Op: Dest-Atop (Inverse)        18.1 µs  996.5 µs   1014.6 µs   39.3 µs 1239.4 µs   1278.7 µs       2.2x Nisaba        1.26x Nisaba
91. Composite Op: Xor (Exclusive Blend)      18.7 µs  987.9 µs   1006.6 µs   43.9 µs  994.8 µs   1038.6 µs       2.4x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    20.0 µs  972.3 µs    992.2 µs   44.1 µs  952.9 µs    997.0 µs       2.2x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   27.3 µs  961.2 µs    988.5 µs  194.0 µs 1032.5 µs   1226.5 µs       7.1x Nisaba        1.24x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     101.5 µs 1153.5 µs   1255.0 µs  264.5 µs 1065.6 µs   1330.1 µs       2.6x Nisaba        1.06x Nisaba
95. Circular Progress Rings (8 Meters)       51.9 µs  951.4 µs   1003.3 µs  399.6 µs 1188.7 µs   1588.3 µs       7.7x Nisaba        1.58x Nisaba
96. Modern Card Stack (5 Elevated Cards)     29.9 µs 1885.5 µs   1915.3 µs   80.3 µs 1622.8 µs   1703.1 µs       2.7x Nisaba          1.12x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   38.9 µs  956.7 µs    995.6 µs   40.7 µs 1177.6 µs   1218.3 µs               Tie        1.22x Nisaba
98. Floating Action Button (FAB 10 Buttons)   51.2 µs  967.1 µs   1018.3 µs  114.7 µs  902.1 µs   1016.8 µs       2.2x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   35.0 µs  989.4 µs   1024.4 µs  111.1 µs  920.0 µs   1031.1 µs       3.2x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   68.2 µs 1559.9 µs   1628.0 µs   71.7 µs 1187.6 µs   1259.3 µs       1.1x Nisaba          1.29x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          3968.4 µs109442.4 µs 113410.8 µs18867.5 µs104709.6 µs 123577.2 µs      4.75x Nisaba        1.09x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 39 suites | Skia won in 37 suites | Ties: 24
         Overall Frame Throughput (End-to-End): 1.09x Nisaba
         Overall CPU Command Submission:       4.75x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 906.7 µs, Skia: 861.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1008.8 µs, Skia: 1077.0 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1125.4 µs, Skia: 1313.6 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1170.0 µs, Skia: 996.5 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1402.9 µs, Skia: 1345.1 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1297.0 µs, Skia: 1179.7 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1116.8 µs, Skia: 908.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 997.9 µs, Skia: 1006.9 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1176.8 µs, Skia: 1015.7 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 997.0 µs, Skia: 1136.3 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1668.1 µs, Skia: 1486.6 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1501.5 µs, Skia: 1353.4 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2076.6 µs, Skia: 1516.4 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1241.9 µs, Skia: 1438.3 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1117.7 µs, Skia: 1496.2 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 998.9 µs, Skia: 1996.5 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 997.4 µs, Skia: 2207.3 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1201.4 µs, Skia: 1075.7 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1027.8 µs, Skia: 614.9 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 999.6 µs, Skia: 1006.9 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1012.4 µs, Skia: 1498.6 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1040.8 µs, Skia: 2649.2 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 995.1 µs, Skia: 1338.9 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 996.9 µs, Skia: 1773.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 999.5 µs, Skia: 1067.5 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1403.7 µs, Skia: 1712.1 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 999.6 µs, Skia: 1582.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 998.8 µs, Skia: 2094.8 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 906.5 µs, Skia: 1465.0 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1520.7 µs, Skia: 2616.6 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 991.2 µs, Skia: 1572.4 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1006.6 µs, Skia: 1583.5 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1000.4 µs, Skia: 1593.6 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 997.3 µs, Skia: 1006.8 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 998.8 µs, Skia: 1035.2 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1028.7 µs, Skia: 996.1 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 998.4 µs, Skia: 1995.7 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 998.1 µs, Skia: 997.2 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 994.2 µs, Skia: 1233.7 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 907.2 µs, Skia: 1737.6 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1124.7 µs, Skia: 1005.1 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 998.9 µs, Skia: 1268.0 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1207.7 µs, Skia: 1017.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1335.5 µs, Skia: 954.7 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1066.6 µs, Skia: 1188.4 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1126.3 µs, Skia: 997.6 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1283.3 µs, Skia: 1040.9 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 997.9 µs, Skia: 1065.2 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 999.4 µs, Skia: 1320.2 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 998.3 µs, Skia: 1415.2 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1312.3 µs, Skia: 1086.5 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1332.2 µs, Skia: 1167.5 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1300.6 µs, Skia: 1017.9 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1382.7 µs, Skia: 1204.7 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1150.8 µs, Skia: 1024.0 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1006.7 µs, Skia: 1048.9 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1282.6 µs, Skia: 997.9 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1373.1 µs, Skia: 1136.0 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2108.0 µs, Skia: 1844.8 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1011.7 µs, Skia: 1854.8 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1185.4 µs, Skia: 1000.6 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1018.2 µs, Skia: 998.6 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1023.4 µs, Skia: 992.1 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1414.3 µs, Skia: 996.5 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1058.4 µs, Skia: 1004.5 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 996.3 µs, Skia: 1056.1 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 997.3 µs, Skia: 1313.2 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1027.9 µs, Skia: 1017.3 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1191.6 µs, Skia: 1020.4 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 998.9 µs, Skia: 997.0 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1003.6 µs, Skia: 995.5 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 996.6 µs, Skia: 1007.1 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1056.5 µs, Skia: 998.5 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 996.2 µs, Skia: 996.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 995.4 µs, Skia: 988.9 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 1006.0 µs, Skia: 996.5 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1019.2 µs, Skia: 940.6 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1143.1 µs, Skia: 1059.6 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1005.6 µs, Skia: 1808.3 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1116.8 µs, Skia: 671.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1009.7 µs, Skia: 572.8 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1024.6 µs, Skia: 1018.0 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1016.6 µs, Skia: 995.6 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1078.9 µs, Skia: 1004.7 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1010.3 µs, Skia: 1297.0 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1035.1 µs, Skia: 1354.6 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1009.8 µs, Skia: 997.2 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1011.5 µs, Skia: 997.0 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1026.6 µs, Skia: 1024.0 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1017.1 µs, Skia: 1269.6 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 856.4 µs, Skia: 924.0 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 996.9 µs, Skia: 1004.1 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 996.3 µs, Skia: 1212.3 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1255.9 µs, Skia: 1326.2 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 996.4 µs, Skia: 1535.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1835.5 µs, Skia: 1716.0 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 997.5 µs, Skia: 1217.4 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1007.3 µs, Skia: 1002.3 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1062.9 µs, Skia: 1030.7 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1682.7 µs, Skia: 1256.4 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         15.6 µs  891.1 µs    906.7 µs    9.7 µs  852.0 µs    861.7 µs         1.6x Skia         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            40.3 µs  968.5 µs   1008.8 µs   45.6 µs 1031.4 µs   1077.0 µs       1.1x Nisaba        1.07x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      28.7 µs 1096.7 µs   1125.4 µs  237.3 µs 1076.3 µs   1313.6 µs       8.3x Nisaba        1.17x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      24.2 µs 1145.8 µs   1170.0 µs   76.2 µs  920.3 µs    996.5 µs       3.1x Nisaba          1.17x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   27.9 µs 1375.1 µs   1402.9 µs   72.2 µs 1272.9 µs   1345.1 µs       2.6x Nisaba         Tie (~1.0x)
6. 50 Simple Rounded Rects (12px Radius)     19.3 µs 1277.7 µs   1297.0 µs   56.9 µs 1122.8 µs   1179.7 µs       3.0x Nisaba          1.10x Skia
7. 50 Varying-Radius Rounded Rects           21.9 µs 1094.9 µs   1116.8 µs   41.5 µs  867.1 µs    908.5 µs       1.9x Nisaba          1.23x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   79.8 µs  918.0 µs    997.9 µs   61.1 µs  945.8 µs   1006.9 µs         1.3x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          30.5 µs 1146.4 µs   1176.8 µs   74.8 µs  940.9 µs   1015.7 µs       2.5x Nisaba          1.16x Skia
10. 25 Concentric Rings (2px Stroke)         89.0 µs  908.0 µs    997.0 µs   43.7 µs 1092.6 µs   1136.3 µs         2.0x Skia        1.14x Nisaba
11. 50 Two-Stop Linear Gradients            101.5 µs 1566.5 µs   1668.1 µs  183.0 µs 1303.6 µs   1486.6 µs       1.8x Nisaba          1.12x Skia
12. 50 Diagonal Gradients (45° Angle)      101.4 µs 1400.1 µs   1501.5 µs  185.9 µs 1167.4 µs   1353.4 µs       1.8x Nisaba          1.11x Skia
13. 25 Radial Glow Gradients (300px)         29.0 µs 2047.6 µs   2076.6 µs   96.3 µs 1420.2 µs   1516.4 µs       3.3x Nisaba          1.37x Skia
14. 20 Box Blur / Drop Shadows (15px)        26.5 µs 1215.5 µs   1241.9 µs   97.0 µs 1341.3 µs   1438.3 µs       3.7x Nisaba        1.16x Nisaba
15. 10 Concave 10-Point Stars (Fill)         40.7 µs 1077.1 µs   1117.7 µs  374.9 µs 1121.3 µs   1496.2 µs       9.2x Nisaba        1.34x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   30.6 µs  968.3 µs    998.9 µs  919.4 µs 1077.1 µs   1996.5 µs      30.0x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       88.2 µs  909.3 µs    997.4 µs  924.8 µs 1282.5 µs   2207.3 µs      10.5x Nisaba        2.21x Nisaba
18. 50 Rotated Cards (Affine Transforms)     91.2 µs 1110.2 µs   1201.4 µs   65.7 µs 1010.0 µs   1075.7 µs         1.4x Skia          1.12x Skia
19. 25 Scissor Clipped Viewports             24.8 µs 1003.0 µs   1027.8 µs   65.5 µs  549.4 µs    614.9 µs       2.6x Nisaba          1.67x Skia
20. 50 UI Text Labels (Inter-Regular)       106.3 µs  893.3 µs    999.6 µs   96.8 µs  910.1 µs   1006.9 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   68.8 µs  943.6 µs   1012.4 µs  355.6 µs 1143.0 µs   1498.6 µs       5.2x Nisaba        1.48x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     46.8 µs  994.0 µs   1040.8 µs 1253.2 µs 1395.9 µs   2649.2 µs      26.8x Nisaba        2.55x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       20.8 µs  974.3 µs    995.1 µs  171.3 µs 1167.6 µs   1338.9 µs       8.2x Nisaba        1.35x Nisaba
24. Circular Arcs with arcTo (20 Corners)    34.4 µs  962.5 µs    996.9 µs  504.2 µs 1269.1 µs   1773.3 µs      14.7x Nisaba        1.78x Nisaba
25. Directional Arcs with arc() (30 Arcs)    61.4 µs  938.1 µs    999.5 µs  148.0 µs  919.5 µs   1067.5 µs       2.4x Nisaba        1.07x Nisaba
26. Compound Path with Holes (15 Donuts)     72.0 µs 1331.7 µs   1403.7 µs  532.2 µs 1179.9 µs   1712.1 µs       7.4x Nisaba        1.22x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    42.7 µs  956.8 µs    999.6 µs  474.0 µs 1108.4 µs   1582.4 µs      11.1x Nisaba        1.58x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    48.5 µs  950.4 µs    998.8 µs  839.8 µs 1255.0 µs   2094.8 µs      17.3x Nisaba        2.10x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   17.2 µs  889.3 µs    906.5 µs  257.1 µs 1207.9 µs   1465.0 µs      14.9x Nisaba        1.62x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      30.3 µs 1490.4 µs   1520.7 µs 1249.2 µs 1367.4 µs   2616.6 µs      41.3x Nisaba        1.72x Nisaba
31. Stroke Join: Miter (12px Stroke)         17.1 µs  974.1 µs    991.2 µs  412.0 µs 1160.3 µs   1572.4 µs      24.1x Nisaba        1.59x Nisaba
32. Stroke Join: Bevel (12px Stroke)         20.3 µs  986.4 µs   1006.6 µs  386.2 µs 1197.3 µs   1583.5 µs      19.0x Nisaba        1.57x Nisaba
33. Stroke Join: Round (12px Stroke)         24.4 µs  976.0 µs   1000.4 µs  412.2 µs 1181.4 µs   1593.6 µs      16.9x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      25.3 µs  972.1 µs    997.3 µs   45.5 µs  961.4 µs   1006.8 µs       1.8x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    24.3 µs  974.5 µs    998.8 µs   45.5 µs  989.7 µs   1035.2 µs       1.9x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     35.5 µs  993.2 µs   1028.7 µs   64.8 µs  931.2 µs    996.1 µs       1.8x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   22.8 µs  975.6 µs    998.4 µs  705.1 µs 1290.6 µs   1995.7 µs      30.9x Nisaba        2.00x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   31.8 µs  966.3 µs    998.1 µs   69.0 µs  928.2 µs    997.2 µs       2.2x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   19.4 µs  974.8 µs    994.2 µs  156.3 µs 1077.4 µs   1233.7 µs       8.1x Nisaba        1.24x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   28.7 µs  878.5 µs    907.2 µs  547.1 µs 1190.5 µs   1737.6 µs      19.1x Nisaba        1.92x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          21.0 µs 1103.7 µs   1124.7 µs   42.7 µs  962.4 µs   1005.1 µs       2.0x Nisaba          1.12x Skia
42. 50 Stroked Ellipses (2.5px Border)      109.7 µs  889.2 µs    998.9 µs   45.2 µs 1222.8 µs   1268.0 µs         2.4x Skia        1.27x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         23.5 µs 1184.2 µs   1207.7 µs   61.5 µs  956.1 µs   1017.6 µs       2.6x Nisaba          1.19x Skia
44. 50 Asymmetric Teardrop Rounded Rects     26.4 µs 1309.1 µs   1335.5 µs   52.2 µs  902.5 µs    954.7 µs       2.0x Nisaba          1.40x Skia
45. High-Density Disks Cloud (500 Disks)     64.8 µs 1001.8 µs   1066.6 µs  221.2 µs  967.2 µs   1188.4 µs       3.4x Nisaba        1.11x Nisaba
46. High-Density Stroked Rings (250 Rings)   41.5 µs 1084.8 µs   1126.3 µs  135.2 µs  862.4 µs    997.6 µs       3.3x Nisaba          1.13x Skia
47. 250 Filled Rectangles Batch              40.0 µs 1243.3 µs   1283.3 µs  129.1 µs  911.8 µs   1040.9 µs       3.2x Nisaba          1.23x Skia
48. 150 Stroked Rectangles (2px Border)      70.9 µs  927.0 µs    997.9 µs   84.7 µs  980.5 µs   1065.2 µs       1.2x Nisaba        1.07x Nisaba
49. 40 Crosshair Aim Reticles                85.0 µs  914.4 µs    999.4 µs  298.6 µs 1021.6 µs   1320.2 µs       3.5x Nisaba        1.32x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      49.8 µs  948.5 µs    998.3 µs  306.6 µs 1108.6 µs   1415.2 µs       6.2x Nisaba        1.42x Nisaba
51. 30 Vertical Card Linear Gradients        70.8 µs 1241.5 µs   1312.3 µs  144.1 µs  942.4 µs   1086.5 µs       2.0x Nisaba          1.21x Skia
52. 30 Horizontal Bar Progress Gradients     59.5 µs 1272.7 µs   1332.2 µs  157.2 µs 1010.3 µs   1167.5 µs       2.6x Nisaba          1.14x Skia
53. 15 Concentric Radial Spotlights          60.8 µs 1239.7 µs   1300.6 µs   91.7 µs  926.2 µs   1017.9 µs       1.5x Nisaba          1.28x Skia
54. 25 Soft Ambient Card Shadows             29.4 µs 1353.3 µs   1382.7 µs  120.3 µs 1084.3 µs   1204.7 µs       4.1x Nisaba          1.15x Skia
55. 15 Neon Button Glows (Cyan Intense)      23.8 µs 1127.0 µs   1150.8 µs   81.1 µs  943.0 µs   1024.0 µs       3.4x Nisaba          1.12x Skia
56. 16 Multi-Angle Gradient Fan Slices       37.0 µs  969.7 µs   1006.7 µs  105.0 µs  943.9 µs   1048.9 µs       2.8x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    16.8 µs 1265.8 µs   1282.6 µs   31.8 µs  966.1 µs    997.9 µs       1.9x Nisaba          1.29x Skia
58. 20 Inset Well Shadows (Recessed)         50.8 µs 1322.4 µs   1373.1 µs  101.7 µs 1034.3 µs   1136.0 µs       2.0x Nisaba          1.21x Skia
59. 10 Pulsing Circular Radar Wave Glows     49.4 µs 2058.6 µs   2108.0 µs   60.0 µs 1784.8 µs   1844.8 µs       1.2x Nisaba          1.14x Skia
60. Waveform Polyline Stroked with Gradient   56.5 µs  955.2 µs   1011.7 µs  562.5 µs 1292.3 µs   1854.8 µs      10.0x Nisaba        1.83x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   19.5 µs 1165.9 µs   1185.4 µs   35.8 µs  964.8 µs   1000.6 µs       1.8x Nisaba          1.18x Skia
62. Scaled Image Quad (2x 512x512 Blit)      19.9 µs  998.3 µs   1018.2 µs   28.4 µs  970.2 µs    998.6 µs       1.4x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      21.8 µs 1001.6 µs   1023.4 µs   35.6 µs  956.6 µs    992.1 µs       1.6x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   22.4 µs 1391.8 µs   1414.3 µs   34.9 µs  961.6 µs    996.5 µs       1.6x Nisaba          1.42x Skia
65. Multi-Avatar Grid (16 Avatars)           56.5 µs 1001.9 µs   1058.4 µs   41.7 µs  962.8 µs   1004.5 µs         1.4x Skia          1.05x Skia
66. Texture Pattern on Star Path             23.3 µs  973.0 µs    996.3 µs  132.3 µs  923.8 µs   1056.1 µs       5.7x Nisaba        1.06x Nisaba
67. 20px Curved Path Stroked with Texture    23.0 µs  974.2 µs    997.3 µs  161.3 µs 1151.9 µs   1313.2 µs       7.0x Nisaba        1.32x Nisaba
68. Minified Texture Quad (0.25x Downscale)   46.6 µs  981.3 µs   1027.9 µs   68.8 µs  948.5 µs   1017.3 µs       1.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        23.6 µs 1168.0 µs   1191.6 µs   61.6 µs  958.8 µs   1020.4 µs       2.6x Nisaba          1.17x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   34.1 µs  964.7 µs    998.9 µs   48.3 µs  948.7 µs    997.0 µs       1.4x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   27.6 µs  976.0 µs   1003.6 µs   35.2 µs  960.3 µs    995.5 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   30.9 µs  965.7 µs    996.6 µs   49.1 µs  958.0 µs   1007.1 µs       1.6x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   65.5 µs  991.0 µs   1056.5 µs   44.5 µs  954.0 µs    998.5 µs         1.5x Skia          1.06x Skia
74. Deep Hierarchical State Stack (20 Levels)   23.9 µs  972.2 µs    996.2 µs   41.5 µs  955.3 µs    996.8 µs       1.7x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       23.4 µs  971.9 µs    995.4 µs   53.1 µs  935.8 µs    988.9 µs       2.3x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   39.0 µs  967.0 µs   1006.0 µs   42.2 µs  954.4 µs    996.5 µs       1.1x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        19.7 µs  999.4 µs   1019.2 µs   57.6 µs  883.0 µs    940.6 µs       2.9x Nisaba          1.08x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   21.5 µs 1121.6 µs   1143.1 µs   38.5 µs 1021.0 µs   1059.6 µs       1.8x Nisaba          1.08x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   90.4 µs  915.2 µs   1005.6 µs  869.7 µs  938.6 µs   1808.3 µs       9.6x Nisaba        1.80x Nisaba
80. Scissored Gradient Card (Clipped)        65.2 µs 1051.6 µs   1116.8 µs   78.8 µs  592.6 µs    671.4 µs       1.2x Nisaba          1.66x Skia
81. Circular Overflow Clip (25 Boxes)        25.3 µs  984.4 µs   1009.7 µs   68.3 µs  504.5 µs    572.8 µs       2.7x Nisaba          1.76x Skia
82. Scissor Invalidation / Reset (25 Cycles)   18.4 µs 1006.2 µs   1024.6 µs   48.0 µs  970.0 µs   1018.0 µs       2.6x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    18.0 µs  998.6 µs   1016.6 µs   42.7 µs  952.9 µs    995.6 µs       2.4x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   19.2 µs 1059.7 µs   1078.9 µs   41.7 µs  963.0 µs   1004.7 µs       2.2x Nisaba          1.07x Skia
85. Composite Op: Source-In (Alpha Masking)   20.2 µs  990.1 µs   1010.3 µs   40.2 µs 1256.8 µs   1297.0 µs       2.0x Nisaba        1.28x Nisaba
86. Composite Op: Source-Out (Cutout)        19.2 µs 1015.9 µs   1035.1 µs   41.7 µs 1312.9 µs   1354.6 µs       2.2x Nisaba        1.31x Nisaba
87. Composite Op: Atop (Target Bounds)       18.4 µs  991.4 µs   1009.8 µs   42.4 µs  954.8 µs    997.2 µs       2.3x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   19.0 µs  992.5 µs   1011.5 µs   44.2 µs  952.8 µs    997.0 µs       2.3x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     18.3 µs 1008.3 µs   1026.6 µs   41.6 µs  982.4 µs   1024.0 µs       2.3x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        18.9 µs  998.2 µs   1017.1 µs   40.0 µs 1229.6 µs   1269.6 µs       2.1x Nisaba        1.25x Nisaba
91. Composite Op: Xor (Exclusive Blend)      18.5 µs  838.0 µs    856.4 µs   42.3 µs  881.7 µs    924.0 µs       2.3x Nisaba        1.08x Nisaba
92. Composite Op: Copy (Direct Overwrite)    18.2 µs  978.7 µs    996.9 µs   40.3 µs  963.8 µs   1004.1 µs       2.2x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   26.3 µs  970.0 µs    996.3 µs  192.1 µs 1020.3 µs   1212.3 µs       7.3x Nisaba        1.22x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     103.3 µs 1152.6 µs   1255.9 µs  263.7 µs 1062.5 µs   1326.2 µs       2.6x Nisaba        1.06x Nisaba
95. Circular Progress Rings (8 Meters)       52.1 µs  944.3 µs    996.4 µs  364.0 µs 1171.4 µs   1535.4 µs       7.0x Nisaba        1.54x Nisaba
96. Modern Card Stack (5 Elevated Cards)     31.2 µs 1804.2 µs   1835.5 µs   80.2 µs 1635.8 µs   1716.0 µs       2.6x Nisaba          1.07x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   40.5 µs  957.0 µs    997.5 µs   41.0 µs 1176.3 µs   1217.4 µs               Tie        1.22x Nisaba
98. Floating Action Button (FAB 10 Buttons)   52.8 µs  954.5 µs   1007.3 µs  117.7 µs  884.6 µs   1002.3 µs       2.2x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   37.4 µs 1025.5 µs   1062.9 µs  113.6 µs  917.2 µs   1030.7 µs       3.0x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   63.7 µs 1618.9 µs   1682.7 µs   73.3 µs 1183.1 µs   1256.4 µs       1.2x Nisaba          1.34x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          4033.8 µs108369.2 µs 112403.0 µs18926.9 µs104927.2 µs 123854.1 µs      4.69x Nisaba        1.10x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 40 suites | Skia won in 34 suites | Ties: 26
         Overall Frame Throughput (End-to-End): 1.10x Nisaba
         Overall CPU Command Submission:       4.69x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 822.5 µs, Skia: 866.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1007.9 µs, Skia: 1086.1 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1038.9 µs, Skia: 1319.3 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1175.0 µs, Skia: 998.8 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1510.7 µs, Skia: 1331.5 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1501.2 µs, Skia: 1202.9 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1505.3 µs, Skia: 1086.8 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 989.0 µs, Skia: 997.1 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1084.9 µs, Skia: 1008.4 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1007.4 µs, Skia: 1084.2 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1627.6 µs, Skia: 1460.0 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1507.4 µs, Skia: 1358.6 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2296.7 µs, Skia: 1584.6 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 969.2 µs, Skia: 1307.4 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1116.0 µs, Skia: 1486.7 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 995.8 µs, Skia: 1998.4 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1001.5 µs, Skia: 2079.1 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1215.9 µs, Skia: 1083.5 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1001.5 µs, Skia: 631.4 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 998.4 µs, Skia: 996.8 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1012.0 µs, Skia: 1533.9 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1022.5 µs, Skia: 2686.6 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 997.3 µs, Skia: 1336.6 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 997.4 µs, Skia: 1770.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1028.0 µs, Skia: 1005.2 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1404.5 µs, Skia: 1726.3 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1000.9 µs, Skia: 1612.7 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 997.4 µs, Skia: 2104.4 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 996.9 µs, Skia: 1426.7 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1477.6 µs, Skia: 2618.5 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 997.7 µs, Skia: 1602.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 996.8 µs, Skia: 1571.0 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 996.7 µs, Skia: 1602.1 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 1007.5 µs, Skia: 1007.9 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 987.7 µs, Skia: 998.0 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1027.9 µs, Skia: 998.4 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 999.5 µs, Skia: 1996.5 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.8 µs, Skia: 996.4 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 998.5 µs, Skia: 1279.6 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1024.1 µs, Skia: 1761.1 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1143.4 µs, Skia: 1009.5 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1009.6 µs, Skia: 1277.5 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1173.2 µs, Skia: 1095.1 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1374.7 µs, Skia: 1025.8 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1019.2 µs, Skia: 1097.7 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1085.2 µs, Skia: 1010.5 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1242.8 µs, Skia: 998.7 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1018.7 µs, Skia: 999.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1004.0 µs, Skia: 1297.9 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1003.6 µs, Skia: 1372.1 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1262.9 µs, Skia: 1151.4 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1349.9 µs, Skia: 1103.1 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1242.8 µs, Skia: 1013.5 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1406.5 µs, Skia: 1011.7 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1189.8 µs, Skia: 1008.3 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1028.9 µs, Skia: 1017.3 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1038.7 µs, Skia: 966.5 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1348.0 µs, Skia: 1073.6 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2195.2 µs, Skia: 1823.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 932.1 µs, Skia: 1854.1 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1193.1 µs, Skia: 997.9 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1028.5 µs, Skia: 988.8 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1008.3 µs, Skia: 988.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1195.2 µs, Skia: 1003.8 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1048.1 µs, Skia: 997.8 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 997.0 µs, Skia: 1030.6 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 994.6 µs, Skia: 1317.8 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 989.0 µs, Skia: 1027.0 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1131.8 µs, Skia: 1004.5 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 1007.1 µs, Skia: 998.0 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 998.2 µs, Skia: 997.8 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 997.1 µs, Skia: 1007.7 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 977.3 µs, Skia: 998.4 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 996.1 µs, Skia: 1013.6 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 1000.7 µs, Skia: 998.0 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 996.1 µs, Skia: 988.1 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1221.7 µs, Skia: 822.1 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1210.7 µs, Skia: 1046.2 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1009.6 µs, Skia: 1973.6 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1097.8 µs, Skia: 651.7 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1119.4 µs, Skia: 570.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 886.8 µs, Skia: 730.7 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 987.1 µs, Skia: 888.0 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 937.0 µs, Skia: 763.5 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1108.5 µs, Skia: 1248.3 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1137.0 µs, Skia: 1319.5 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 766.4 µs, Skia: 951.7 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1035.3 µs, Skia: 992.5 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1026.3 µs, Skia: 997.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1013.9 µs, Skia: 1267.1 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1056.3 µs, Skia: 1017.5 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 997.8 µs, Skia: 997.7 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1003.5 µs, Skia: 1217.0 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1241.3 µs, Skia: 1342.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1001.1 µs, Skia: 1522.2 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1897.3 µs, Skia: 1753.2 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1005.8 µs, Skia: 1218.6 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1009.0 µs, Skia: 1007.6 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1003.2 µs, Skia: 1009.3 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1650.1 µs, Skia: 1262.8 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         15.4 µs  807.1 µs    822.5 µs   13.1 µs  853.6 µs    866.7 µs         1.2x Skia        1.05x Nisaba
2. 100 Grid Lines (1px Hairlines)            40.5 µs  967.4 µs   1007.9 µs   41.7 µs 1044.4 µs   1086.1 µs               Tie        1.08x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      32.9 µs 1006.0 µs   1038.9 µs  237.6 µs 1081.7 µs   1319.3 µs       7.2x Nisaba        1.27x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      26.2 µs 1148.8 µs   1175.0 µs   77.9 µs  920.9 µs    998.8 µs       3.0x Nisaba          1.18x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   25.9 µs 1484.9 µs   1510.7 µs   74.1 µs 1257.3 µs   1331.5 µs       2.9x Nisaba          1.13x Skia
6. 50 Simple Rounded Rects (12px Radius)     20.7 µs 1480.5 µs   1501.2 µs   58.6 µs 1144.3 µs   1202.9 µs       2.8x Nisaba          1.25x Skia
7. 50 Varying-Radius Rounded Rects           22.8 µs 1482.5 µs   1505.3 µs   46.4 µs 1040.4 µs   1086.8 µs       2.0x Nisaba          1.39x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   86.4 µs  902.7 µs    989.0 µs   54.8 µs  942.4 µs    997.1 µs         1.6x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          24.1 µs 1060.8 µs   1084.9 µs   70.3 µs  938.1 µs   1008.4 µs       2.9x Nisaba          1.08x Skia
10. 25 Concentric Rings (2px Stroke)         88.2 µs  919.2 µs   1007.4 µs   41.0 µs 1043.2 µs   1084.2 µs         2.2x Skia        1.08x Nisaba
11. 50 Two-Stop Linear Gradients             98.1 µs 1529.5 µs   1627.6 µs  181.4 µs 1278.6 µs   1460.0 µs       1.8x Nisaba          1.11x Skia
12. 50 Diagonal Gradients (45° Angle)      100.2 µs 1407.2 µs   1507.4 µs  181.2 µs 1177.4 µs   1358.6 µs       1.8x Nisaba          1.11x Skia
13. 25 Radial Glow Gradients (300px)         26.8 µs 2269.8 µs   2296.7 µs  106.2 µs 1478.4 µs   1584.6 µs       4.0x Nisaba          1.45x Skia
14. 20 Box Blur / Drop Shadows (15px)        31.0 µs  938.3 µs    969.2 µs   96.7 µs 1210.7 µs   1307.4 µs       3.1x Nisaba        1.35x Nisaba
15. 10 Concave 10-Point Stars (Fill)         39.4 µs 1076.6 µs   1116.0 µs  381.6 µs 1105.1 µs   1486.7 µs       9.7x Nisaba        1.33x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   30.8 µs  965.1 µs    995.8 µs  875.4 µs 1123.0 µs   1998.4 µs      28.4x Nisaba        2.01x Nisaba
17. 1000-Point Waveform (Dense Stroke)       86.9 µs  914.6 µs   1001.5 µs  852.0 µs 1227.1 µs   2079.1 µs       9.8x Nisaba        2.08x Nisaba
18. 50 Rotated Cards (Affine Transforms)     89.5 µs 1126.4 µs   1215.9 µs   67.4 µs 1016.1 µs   1083.5 µs         1.3x Skia          1.12x Skia
19. 25 Scissor Clipped Viewports             22.4 µs  979.0 µs   1001.5 µs   62.7 µs  568.6 µs    631.4 µs       2.8x Nisaba          1.59x Skia
20. 50 UI Text Labels (Inter-Regular)       108.1 µs  890.3 µs    998.4 µs   96.2 µs  900.6 µs    996.8 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   70.7 µs  941.2 µs   1012.0 µs  370.8 µs 1163.1 µs   1533.9 µs       5.2x Nisaba        1.52x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     45.8 µs  976.7 µs   1022.5 µs 1270.8 µs 1415.8 µs   2686.6 µs      27.7x Nisaba        2.63x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       21.1 µs  976.3 µs    997.3 µs  177.8 µs 1158.8 µs   1336.6 µs       8.4x Nisaba        1.34x Nisaba
24. Circular Arcs with arcTo (20 Corners)    36.5 µs  960.9 µs    997.4 µs  505.2 µs 1265.1 µs   1770.3 µs      13.8x Nisaba        1.77x Nisaba
25. Directional Arcs with arc() (30 Arcs)    63.9 µs  964.0 µs   1028.0 µs  152.7 µs  852.5 µs   1005.2 µs       2.4x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     74.7 µs 1329.8 µs   1404.5 µs  534.9 µs 1191.4 µs   1726.3 µs       7.2x Nisaba        1.23x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    44.2 µs  956.7 µs   1000.9 µs  479.9 µs 1132.8 µs   1612.7 µs      10.9x Nisaba        1.61x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    48.7 µs  948.7 µs    997.4 µs  861.0 µs 1243.5 µs   2104.4 µs      17.7x Nisaba        2.11x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   14.2 µs  982.7 µs    996.9 µs  262.0 µs 1164.7 µs   1426.7 µs      18.4x Nisaba        1.43x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      29.4 µs 1448.2 µs   1477.6 µs 1234.3 µs 1384.2 µs   2618.5 µs      42.0x Nisaba        1.77x Nisaba
31. Stroke Join: Miter (12px Stroke)         18.6 µs  979.1 µs    997.7 µs  397.5 µs 1204.6 µs   1602.1 µs      21.3x Nisaba        1.61x Nisaba
32. Stroke Join: Bevel (12px Stroke)         18.7 µs  978.1 µs    996.8 µs  385.9 µs 1185.1 µs   1571.0 µs      20.6x Nisaba        1.58x Nisaba
33. Stroke Join: Round (12px Stroke)         25.1 µs  971.6 µs    996.7 µs  403.8 µs 1198.4 µs   1602.1 µs      16.1x Nisaba        1.61x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      24.6 µs  982.9 µs   1007.5 µs   46.1 µs  961.8 µs   1007.9 µs       1.9x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    25.1 µs  962.6 µs    987.7 µs   46.1 µs  951.9 µs    998.0 µs       1.8x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     36.0 µs  991.8 µs   1027.9 µs   63.4 µs  935.0 µs    998.4 µs       1.8x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   23.9 µs  975.7 µs    999.5 µs  699.9 µs 1296.6 µs   1996.5 µs      29.3x Nisaba        2.00x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   32.6 µs  965.1 µs    997.8 µs   69.8 µs  926.6 µs    996.4 µs       2.1x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   16.7 µs  981.8 µs    998.5 µs  145.0 µs 1134.6 µs   1279.6 µs       8.7x Nisaba        1.28x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   32.3 µs  991.8 µs   1024.1 µs  547.1 µs 1214.0 µs   1761.1 µs      16.9x Nisaba        1.72x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          19.6 µs 1123.8 µs   1143.4 µs   41.5 µs  968.0 µs   1009.5 µs       2.1x Nisaba          1.13x Skia
42. 50 Stroked Ellipses (2.5px Border)      111.3 µs  898.3 µs   1009.6 µs   40.0 µs 1237.5 µs   1277.5 µs         2.8x Skia        1.27x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         22.7 µs 1150.4 µs   1173.2 µs   65.1 µs 1030.0 µs   1095.1 µs       2.9x Nisaba          1.07x Skia
44. 50 Asymmetric Teardrop Rounded Rects     26.2 µs 1348.6 µs   1374.7 µs   46.1 µs  979.7 µs   1025.8 µs       1.8x Nisaba          1.34x Skia
45. High-Density Disks Cloud (500 Disks)     56.7 µs  962.5 µs   1019.2 µs  206.3 µs  891.4 µs   1097.7 µs       3.6x Nisaba        1.08x Nisaba
46. High-Density Stroked Rings (250 Rings)   38.1 µs 1047.1 µs   1085.2 µs  132.8 µs  877.7 µs   1010.5 µs       3.5x Nisaba          1.07x Skia
47. 250 Filled Rectangles Batch              39.0 µs 1203.9 µs   1242.8 µs  132.5 µs  866.1 µs    998.7 µs       3.4x Nisaba          1.24x Skia
48. 150 Stroked Rectangles (2px Border)      70.1 µs  948.6 µs   1018.7 µs   84.2 µs  915.1 µs    999.3 µs       1.2x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                83.6 µs  920.4 µs   1004.0 µs  293.8 µs 1004.1 µs   1297.9 µs       3.5x Nisaba        1.29x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      50.6 µs  953.0 µs   1003.6 µs  295.6 µs 1076.6 µs   1372.1 µs       5.8x Nisaba        1.37x Nisaba
51. 30 Vertical Card Linear Gradients        64.7 µs 1198.2 µs   1262.9 µs  139.9 µs 1011.6 µs   1151.4 µs       2.2x Nisaba          1.10x Skia
52. 30 Horizontal Bar Progress Gradients     62.4 µs 1287.5 µs   1349.9 µs  144.9 µs  958.2 µs   1103.1 µs       2.3x Nisaba          1.22x Skia
53. 15 Concentric Radial Spotlights          53.7 µs 1189.1 µs   1242.8 µs   82.4 µs  931.2 µs   1013.5 µs       1.5x Nisaba          1.23x Skia
54. 25 Soft Ambient Card Shadows             29.4 µs 1377.0 µs   1406.5 µs  106.1 µs  905.6 µs   1011.7 µs       3.6x Nisaba          1.39x Skia
55. 15 Neon Button Glows (Cyan Intense)      26.3 µs 1163.5 µs   1189.8 µs   81.4 µs  927.0 µs   1008.3 µs       3.1x Nisaba          1.18x Skia
56. 16 Multi-Angle Gradient Fan Slices       37.2 µs  991.7 µs   1028.9 µs  104.0 µs  913.3 µs   1017.3 µs       2.8x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    15.6 µs 1023.1 µs   1038.7 µs   34.3 µs  932.2 µs    966.5 µs       2.2x Nisaba          1.07x Skia
58. 20 Inset Well Shadows (Recessed)         52.4 µs 1295.5 µs   1348.0 µs   96.4 µs  977.2 µs   1073.6 µs       1.8x Nisaba          1.26x Skia
59. 10 Pulsing Circular Radar Wave Glows     51.8 µs 2143.4 µs   2195.2 µs   67.9 µs 1755.4 µs   1823.3 µs       1.3x Nisaba          1.20x Skia
60. Waveform Polyline Stroked with Gradient   57.2 µs  874.9 µs    932.1 µs  560.2 µs 1293.9 µs   1854.1 µs       9.8x Nisaba        1.99x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   19.1 µs 1173.9 µs   1193.1 µs   34.4 µs  963.4 µs    997.9 µs       1.8x Nisaba          1.20x Skia
62. Scaled Image Quad (2x 512x512 Blit)      20.1 µs 1008.4 µs   1028.5 µs   27.6 µs  961.2 µs    988.8 µs       1.4x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      22.8 µs  985.5 µs   1008.3 µs   37.1 µs  951.3 µs    988.4 µs       1.6x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   23.2 µs 1172.0 µs   1195.2 µs   34.8 µs  969.0 µs   1003.8 µs       1.5x Nisaba          1.19x Skia
65. Multi-Avatar Grid (16 Avatars)           59.0 µs  989.1 µs   1048.1 µs   44.7 µs  953.2 µs    997.8 µs         1.3x Skia         Tie (~1.0x)
66. Texture Pattern on Star Path             24.3 µs  972.7 µs    997.0 µs  132.3 µs  898.3 µs   1030.6 µs       5.4x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    25.3 µs  969.3 µs    994.6 µs  163.3 µs 1154.5 µs   1317.8 µs       6.5x Nisaba        1.32x Nisaba
68. Minified Texture Quad (0.25x Downscale)   44.5 µs  944.6 µs    989.0 µs   70.2 µs  956.8 µs   1027.0 µs       1.6x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        26.3 µs 1105.4 µs   1131.8 µs   65.9 µs  938.6 µs   1004.5 µs       2.5x Nisaba          1.13x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   34.6 µs  972.5 µs   1007.1 µs   49.9 µs  948.1 µs    998.0 µs       1.4x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   28.1 µs  970.1 µs    998.2 µs   37.4 µs  960.3 µs    997.8 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   29.9 µs  967.3 µs    997.1 µs   50.7 µs  957.0 µs   1007.7 µs       1.7x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   66.8 µs  910.5 µs    977.3 µs   45.6 µs  952.8 µs    998.4 µs         1.5x Skia         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   24.0 µs  972.1 µs    996.1 µs   42.1 µs  971.6 µs   1013.6 µs       1.8x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       27.2 µs  973.5 µs   1000.7 µs   55.5 µs  942.6 µs    998.0 µs       2.0x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   44.7 µs  951.4 µs    996.1 µs   43.1 µs  945.0 µs    988.1 µs               Tie         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        25.5 µs 1196.2 µs   1221.7 µs   64.4 µs  757.7 µs    822.1 µs       2.5x Nisaba          1.49x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   22.8 µs 1187.9 µs   1210.7 µs   38.8 µs 1007.5 µs   1046.2 µs       1.7x Nisaba          1.16x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   88.3 µs  921.4 µs   1009.6 µs  988.1 µs  985.4 µs   1973.6 µs      11.2x Nisaba        1.95x Nisaba
80. Scissored Gradient Card (Clipped)        68.4 µs 1029.4 µs   1097.8 µs   83.3 µs  568.4 µs    651.7 µs       1.2x Nisaba          1.68x Skia
81. Circular Overflow Clip (25 Boxes)        22.4 µs 1097.0 µs   1119.4 µs   71.6 µs  498.4 µs    570.0 µs       3.2x Nisaba          1.96x Skia
82. Scissor Invalidation / Reset (25 Cycles)   28.5 µs  858.3 µs    886.8 µs   79.2 µs  651.4 µs    730.7 µs       2.8x Nisaba          1.21x Skia
83. Composite Op: Source-Over (30 Shapes)    30.7 µs  956.4 µs    987.1 µs   57.8 µs  830.2 µs    888.0 µs       1.9x Nisaba          1.11x Skia
84. Composite Op: Lighter / Plus (30 Particles)   27.0 µs  910.0 µs    937.0 µs   45.8 µs  717.8 µs    763.5 µs       1.7x Nisaba          1.23x Skia
85. Composite Op: Source-In (Alpha Masking)   23.6 µs 1085.0 µs   1108.5 µs   47.9 µs 1200.4 µs   1248.3 µs       2.0x Nisaba        1.13x Nisaba
86. Composite Op: Source-Out (Cutout)        22.0 µs 1115.0 µs   1137.0 µs   52.9 µs 1266.6 µs   1319.5 µs       2.4x Nisaba        1.16x Nisaba
87. Composite Op: Atop (Target Bounds)       16.9 µs  749.6 µs    766.4 µs   43.0 µs  908.6 µs    951.7 µs       2.6x Nisaba        1.24x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   18.3 µs 1017.0 µs   1035.3 µs   42.3 µs  950.2 µs    992.5 µs       2.3x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     18.2 µs 1008.1 µs   1026.3 µs   42.4 µs  954.9 µs    997.3 µs       2.3x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        18.6 µs  995.3 µs   1013.9 µs   40.9 µs 1226.2 µs   1267.1 µs       2.2x Nisaba        1.25x Nisaba
91. Composite Op: Xor (Exclusive Blend)      17.5 µs 1038.9 µs   1056.3 µs   40.9 µs  976.6 µs   1017.5 µs       2.3x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    20.1 µs  977.7 µs    997.8 µs   42.6 µs  955.1 µs    997.7 µs       2.1x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   23.7 µs  979.8 µs   1003.5 µs  194.1 µs 1023.0 µs   1217.0 µs       8.2x Nisaba        1.21x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     108.1 µs 1133.2 µs   1241.3 µs  268.9 µs 1074.0 µs   1342.9 µs       2.5x Nisaba        1.08x Nisaba
95. Circular Progress Rings (8 Meters)       52.2 µs  948.9 µs   1001.1 µs  403.1 µs 1119.0 µs   1522.2 µs       7.7x Nisaba        1.52x Nisaba
96. Modern Card Stack (5 Elevated Cards)     35.4 µs 1861.8 µs   1897.3 µs   78.3 µs 1674.9 µs   1753.2 µs       2.2x Nisaba          1.08x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   35.9 µs  969.9 µs   1005.8 µs   41.0 µs 1177.7 µs   1218.6 µs       1.1x Nisaba        1.21x Nisaba
98. Floating Action Button (FAB 10 Buttons)   51.9 µs  957.2 µs   1009.0 µs  117.3 µs  890.3 µs   1007.6 µs       2.3x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   36.4 µs  966.7 µs   1003.2 µs  114.5 µs  894.8 µs   1009.3 µs       3.1x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   66.1 µs 1584.0 µs   1650.1 µs   72.5 µs 1190.3 µs   1262.8 µs       1.1x Nisaba          1.31x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          4085.9 µs108305.8 µs 112391.7 µs19031.9 µs103707.2 µs 122739.1 µs      4.66x Nisaba        1.09x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 38 suites | Skia won in 35 suites | Ties: 27
         Overall Frame Throughput (End-to-End): 1.09x Nisaba
         Overall CPU Command Submission:       4.66x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 970.8 µs, Skia: 995.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1007.8 µs, Skia: 1099.0 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1098.0 µs, Skia: 1323.2 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1166.7 µs, Skia: 996.2 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1377.6 µs, Skia: 1320.4 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1327.0 µs, Skia: 1165.8 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1315.9 µs, Skia: 1088.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 998.2 µs, Skia: 1016.6 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1079.2 µs, Skia: 1007.5 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 997.3 µs, Skia: 1117.6 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1687.7 µs, Skia: 1431.8 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1498.1 µs, Skia: 1357.5 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2066.5 µs, Skia: 1537.2 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1282.5 µs, Skia: 1434.3 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1128.1 µs, Skia: 1496.0 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 997.0 µs, Skia: 1998.5 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1007.2 µs, Skia: 2136.9 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1234.0 µs, Skia: 1085.4 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1057.4 µs, Skia: 641.4 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 998.4 µs, Skia: 1008.2 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1007.9 µs, Skia: 1511.6 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1119.8 µs, Skia: 2624.7 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 998.4 µs, Skia: 1341.0 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 999.3 µs, Skia: 1762.5 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1038.1 µs, Skia: 1027.7 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1409.7 µs, Skia: 1721.3 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1002.1 µs, Skia: 1621.8 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 987.1 µs, Skia: 2059.5 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 964.6 µs, Skia: 1449.4 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1487.9 µs, Skia: 2589.8 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 992.1 µs, Skia: 1731.2 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1014.0 µs, Skia: 1542.9 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 1000.5 µs, Skia: 1573.3 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 998.0 µs, Skia: 1009.3 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 997.3 µs, Skia: 1046.5 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1017.1 µs, Skia: 1058.5 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 998.5 µs, Skia: 1989.3 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 996.4 µs, Skia: 997.8 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 997.7 µs, Skia: 1237.6 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1011.3 µs, Skia: 1758.2 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1146.3 µs, Skia: 1016.6 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 999.9 µs, Skia: 1277.7 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1228.2 µs, Skia: 1026.3 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 801.7 µs, Skia: 848.1 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1008.2 µs, Skia: 1116.0 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1133.7 µs, Skia: 1027.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1264.8 µs, Skia: 1007.5 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 987.7 µs, Skia: 1068.9 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1028.7 µs, Skia: 1322.8 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1016.9 µs, Skia: 1418.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1296.9 µs, Skia: 1155.5 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1358.4 µs, Skia: 1187.4 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1327.5 µs, Skia: 1090.4 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1497.7 µs, Skia: 1348.2 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1251.4 µs, Skia: 1036.2 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1089.1 µs, Skia: 1019.1 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1169.8 µs, Skia: 978.0 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1371.9 µs, Skia: 1288.0 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2198.1 µs, Skia: 1499.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1035.3 µs, Skia: 1804.1 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1328.8 µs, Skia: 998.2 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1057.8 µs, Skia: 998.2 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1016.6 µs, Skia: 983.6 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1287.8 µs, Skia: 998.1 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1007.1 µs, Skia: 987.8 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 997.3 µs, Skia: 1027.0 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 997.0 µs, Skia: 1299.3 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1048.9 µs, Skia: 1017.1 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1038.7 µs, Skia: 926.9 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 869.0 µs, Skia: 958.7 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 976.0 µs, Skia: 976.9 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 921.1 µs, Skia: 956.9 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1008.1 µs, Skia: 865.7 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 827.5 µs, Skia: 994.3 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 977.3 µs, Skia: 929.7 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 912.7 µs, Skia: 1105.1 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1048.5 µs, Skia: 657.4 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1396.5 µs, Skia: 1137.6 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1017.7 µs, Skia: 1906.1 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1308.6 µs, Skia: 732.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1045.7 µs, Skia: 584.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 956.8 µs, Skia: 1038.3 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1048.4 µs, Skia: 997.8 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1155.1 µs, Skia: 1056.5 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1044.3 µs, Skia: 835.8 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 966.5 µs, Skia: 1059.1 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 978.0 µs, Skia: 1058.5 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1223.6 µs, Skia: 1077.8 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1078.9 µs, Skia: 997.1 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1025.0 µs, Skia: 1272.9 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1086.5 µs, Skia: 1024.8 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 997.4 µs, Skia: 998.1 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 966.7 µs, Skia: 1228.2 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1264.8 µs, Skia: 1327.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1005.0 µs, Skia: 1571.5 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1880.6 µs, Skia: 1752.6 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1006.2 µs, Skia: 1207.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1008.6 µs, Skia: 1017.6 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1019.2 µs, Skia: 1037.0 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1650.2 µs, Skia: 1257.2 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         14.4 µs  956.4 µs    970.8 µs    9.5 µs  986.4 µs    995.9 µs         1.5x Skia         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            40.1 µs  967.7 µs   1007.8 µs   42.1 µs 1056.9 µs   1099.0 µs               Tie        1.09x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      30.7 µs 1067.3 µs   1098.0 µs  237.0 µs 1086.2 µs   1323.2 µs       7.7x Nisaba        1.21x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      26.7 µs 1140.0 µs   1166.7 µs   73.4 µs  922.8 µs    996.2 µs       2.8x Nisaba          1.17x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   26.0 µs 1351.6 µs   1377.6 µs   66.8 µs 1253.6 µs   1320.4 µs       2.6x Nisaba         Tie (~1.0x)
6. 50 Simple Rounded Rects (12px Radius)     20.3 µs 1306.7 µs   1327.0 µs   55.6 µs 1110.2 µs   1165.8 µs       2.7x Nisaba          1.14x Skia
7. 50 Varying-Radius Rounded Rects           22.2 µs 1293.7 µs   1315.9 µs   48.8 µs 1039.7 µs   1088.5 µs       2.2x Nisaba          1.21x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   80.5 µs  917.8 µs    998.2 µs   60.9 µs  955.7 µs   1016.6 µs         1.3x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          23.9 µs 1055.3 µs   1079.2 µs   69.1 µs  938.4 µs   1007.5 µs       2.9x Nisaba          1.07x Skia
10. 25 Concentric Rings (2px Stroke)         85.6 µs  911.7 µs    997.3 µs   40.4 µs 1077.2 µs   1117.6 µs         2.1x Skia        1.12x Nisaba
11. 50 Two-Stop Linear Gradients            101.6 µs 1586.1 µs   1687.7 µs  180.4 µs 1251.4 µs   1431.8 µs       1.8x Nisaba          1.18x Skia
12. 50 Diagonal Gradients (45° Angle)       98.1 µs 1400.1 µs   1498.1 µs  188.2 µs 1169.3 µs   1357.5 µs       1.9x Nisaba          1.10x Skia
13. 25 Radial Glow Gradients (300px)         26.0 µs 2040.4 µs   2066.5 µs  100.4 µs 1436.9 µs   1537.2 µs       3.9x Nisaba          1.34x Skia
14. 20 Box Blur / Drop Shadows (15px)        25.6 µs 1256.9 µs   1282.5 µs   95.3 µs 1339.0 µs   1434.3 µs       3.7x Nisaba        1.12x Nisaba
15. 10 Concave 10-Point Stars (Fill)         39.5 µs 1088.7 µs   1128.1 µs  375.6 µs 1120.4 µs   1496.0 µs       9.5x Nisaba        1.33x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   29.2 µs  967.8 µs    997.0 µs  914.5 µs 1083.9 µs   1998.5 µs      31.3x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       90.9 µs  916.3 µs   1007.2 µs  890.9 µs 1246.0 µs   2136.9 µs       9.8x Nisaba        2.12x Nisaba
18. 50 Rotated Cards (Affine Transforms)     91.4 µs 1142.6 µs   1234.0 µs   61.7 µs 1023.7 µs   1085.4 µs         1.5x Skia          1.14x Skia
19. 25 Scissor Clipped Viewports             23.1 µs 1034.3 µs   1057.4 µs   64.1 µs  577.3 µs    641.4 µs       2.8x Nisaba          1.65x Skia
20. 50 UI Text Labels (Inter-Regular)       106.0 µs  892.3 µs    998.4 µs   97.0 µs  911.2 µs   1008.2 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   70.4 µs  937.5 µs   1007.9 µs  356.0 µs 1155.5 µs   1511.6 µs       5.1x Nisaba        1.50x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     44.1 µs 1075.7 µs   1119.8 µs 1244.9 µs 1379.8 µs   2624.7 µs      28.2x Nisaba        2.34x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       19.3 µs  979.1 µs    998.4 µs  172.7 µs 1168.3 µs   1341.0 µs       8.9x Nisaba        1.34x Nisaba
24. Circular Arcs with arcTo (20 Corners)    36.5 µs  962.8 µs    999.3 µs  509.5 µs 1253.0 µs   1762.5 µs      14.0x Nisaba        1.76x Nisaba
25. Directional Arcs with arc() (30 Arcs)    62.0 µs  976.0 µs   1038.1 µs  148.7 µs  879.0 µs   1027.7 µs       2.4x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     70.2 µs 1339.6 µs   1409.7 µs  533.1 µs 1188.2 µs   1721.3 µs       7.6x Nisaba        1.22x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    41.8 µs  960.4 µs   1002.1 µs  477.1 µs 1144.8 µs   1621.8 µs      11.4x Nisaba        1.62x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    50.7 µs  936.4 µs    987.1 µs  810.9 µs 1248.5 µs   2059.5 µs      16.0x Nisaba        2.09x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   17.5 µs  947.1 µs    964.6 µs  257.7 µs 1191.7 µs   1449.4 µs      14.7x Nisaba        1.50x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      27.6 µs 1460.3 µs   1487.9 µs 1232.4 µs 1357.3 µs   2589.8 µs      44.6x Nisaba        1.74x Nisaba
31. Stroke Join: Miter (12px Stroke)         17.1 µs  975.0 µs    992.1 µs  426.2 µs 1305.0 µs   1731.2 µs      24.9x Nisaba        1.74x Nisaba
32. Stroke Join: Bevel (12px Stroke)         20.2 µs  993.9 µs   1014.0 µs  389.8 µs 1153.1 µs   1542.9 µs      19.3x Nisaba        1.52x Nisaba
33. Stroke Join: Round (12px Stroke)         23.8 µs  976.7 µs   1000.5 µs  404.4 µs 1168.9 µs   1573.3 µs      17.0x Nisaba        1.57x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      24.4 µs  973.6 µs    998.0 µs   44.8 µs  964.5 µs   1009.3 µs       1.8x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    25.1 µs  972.2 µs    997.3 µs   46.9 µs  999.6 µs   1046.5 µs       1.9x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     37.3 µs  979.8 µs   1017.1 µs   57.2 µs 1001.3 µs   1058.5 µs       1.5x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   23.5 µs  975.0 µs    998.5 µs  709.0 µs 1280.2 µs   1989.3 µs      30.2x Nisaba        1.99x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   34.8 µs  961.6 µs    996.4 µs   64.9 µs  932.8 µs    997.8 µs       1.9x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   16.7 µs  981.0 µs    997.7 µs  158.8 µs 1078.8 µs   1237.6 µs       9.5x Nisaba        1.24x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   29.4 µs  981.9 µs   1011.3 µs  543.1 µs 1215.1 µs   1758.2 µs      18.5x Nisaba        1.74x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          22.1 µs 1124.1 µs   1146.3 µs   42.8 µs  973.7 µs   1016.6 µs       1.9x Nisaba          1.13x Skia
42. 50 Stroked Ellipses (2.5px Border)      106.7 µs  893.2 µs    999.9 µs   42.2 µs 1235.5 µs   1277.7 µs         2.5x Skia        1.28x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         20.9 µs 1207.3 µs   1228.2 µs   60.8 µs  965.5 µs   1026.3 µs       2.9x Nisaba          1.20x Skia
44. 50 Asymmetric Teardrop Rounded Rects     21.5 µs  780.2 µs    801.7 µs   47.7 µs  800.4 µs    848.1 µs       2.2x Nisaba        1.06x Nisaba
45. High-Density Disks Cloud (500 Disks)     55.5 µs  952.6 µs   1008.2 µs  210.1 µs  905.9 µs   1116.0 µs       3.8x Nisaba        1.11x Nisaba
46. High-Density Stroked Rings (250 Rings)   39.4 µs 1094.3 µs   1133.7 µs  132.1 µs  895.0 µs   1027.1 µs       3.4x Nisaba          1.10x Skia
47. 250 Filled Rectangles Batch              40.5 µs 1224.3 µs   1264.8 µs  128.0 µs  879.5 µs   1007.5 µs       3.2x Nisaba          1.26x Skia
48. 150 Stroked Rectangles (2px Border)      66.0 µs  921.7 µs    987.7 µs   83.3 µs  985.5 µs   1068.9 µs       1.3x Nisaba        1.08x Nisaba
49. 40 Crosshair Aim Reticles                90.6 µs  938.1 µs   1028.7 µs  294.9 µs 1027.9 µs   1322.8 µs       3.3x Nisaba        1.29x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      51.0 µs  965.9 µs   1016.9 µs  304.3 µs 1114.1 µs   1418.4 µs       6.0x Nisaba        1.39x Nisaba
51. 30 Vertical Card Linear Gradients        66.5 µs 1230.4 µs   1296.9 µs  150.1 µs 1005.4 µs   1155.5 µs       2.3x Nisaba          1.12x Skia
52. 30 Horizontal Bar Progress Gradients     58.9 µs 1299.4 µs   1358.4 µs  146.3 µs 1041.1 µs   1187.4 µs       2.5x Nisaba          1.14x Skia
53. 15 Concentric Radial Spotlights          69.1 µs 1258.4 µs   1327.5 µs   95.1 µs  995.3 µs   1090.4 µs       1.4x Nisaba          1.22x Skia
54. 25 Soft Ambient Card Shadows             37.4 µs 1460.3 µs   1497.7 µs  186.8 µs 1161.4 µs   1348.2 µs       5.0x Nisaba          1.11x Skia
55. 15 Neon Button Glows (Cyan Intense)      33.1 µs 1218.3 µs   1251.4 µs   87.8 µs  948.4 µs   1036.2 µs       2.6x Nisaba          1.21x Skia
56. 16 Multi-Angle Gradient Fan Slices       39.9 µs 1049.2 µs   1089.1 µs  108.1 µs  911.0 µs   1019.1 µs       2.7x Nisaba          1.07x Skia
57. Full-Screen Diagonal Horizon Gradient    17.7 µs 1152.1 µs   1169.8 µs   34.3 µs  943.7 µs    978.0 µs       1.9x Nisaba          1.20x Skia
58. 20 Inset Well Shadows (Recessed)         50.4 µs 1321.5 µs   1371.9 µs  124.6 µs 1163.4 µs   1288.0 µs       2.5x Nisaba          1.07x Skia
59. 10 Pulsing Circular Radar Wave Glows     53.8 µs 2144.3 µs   2198.1 µs   71.2 µs 1428.1 µs   1499.3 µs       1.3x Nisaba          1.47x Skia
60. Waveform Polyline Stroked with Gradient   56.0 µs  979.3 µs   1035.3 µs  558.6 µs 1245.5 µs   1804.1 µs      10.0x Nisaba        1.74x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   18.6 µs 1310.2 µs   1328.8 µs   33.1 µs  965.1 µs    998.2 µs       1.8x Nisaba          1.33x Skia
62. Scaled Image Quad (2x 512x512 Blit)      17.9 µs 1039.8 µs   1057.8 µs   27.9 µs  970.3 µs    998.2 µs       1.6x Nisaba          1.06x Skia
63. 45° Rotated Pattern Fill in Circle      21.3 µs  995.3 µs   1016.6 µs   38.8 µs  944.8 µs    983.6 µs       1.8x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   22.4 µs 1265.4 µs   1287.8 µs   33.8 µs  964.4 µs    998.1 µs       1.5x Nisaba          1.29x Skia
65. Multi-Avatar Grid (16 Avatars)           55.1 µs  952.0 µs   1007.1 µs   43.4 µs  944.4 µs    987.8 µs         1.3x Skia         Tie (~1.0x)
66. Texture Pattern on Star Path             25.5 µs  971.8 µs    997.3 µs  134.3 µs  892.7 µs   1027.0 µs       5.3x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    21.2 µs  975.8 µs    997.0 µs  162.1 µs 1137.2 µs   1299.3 µs       7.6x Nisaba        1.30x Nisaba
68. Minified Texture Quad (0.25x Downscale)   41.7 µs 1007.2 µs   1048.9 µs   70.5 µs  946.6 µs   1017.1 µs       1.7x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        24.8 µs 1013.9 µs   1038.7 µs   61.6 µs  865.2 µs    926.9 µs       2.5x Nisaba          1.12x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   42.1 µs  826.9 µs    869.0 µs   95.3 µs  863.4 µs    958.7 µs       2.3x Nisaba        1.10x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   28.2 µs  947.8 µs    976.0 µs   37.8 µs  939.1 µs    976.9 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   29.5 µs  891.5 µs    921.1 µs   47.5 µs  909.4 µs    956.9 µs       1.6x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   68.3 µs  939.7 µs   1008.1 µs   42.4 µs  823.3 µs    865.7 µs         1.6x Skia          1.16x Skia
74. Deep Hierarchical State Stack (20 Levels)   25.2 µs  802.3 µs    827.5 µs   43.1 µs  951.3 µs    994.3 µs       1.7x Nisaba        1.20x Nisaba
75. Reset Transform Stress (50 Cycles)       22.3 µs  955.0 µs    977.3 µs   55.5 µs  874.1 µs    929.7 µs       2.5x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   41.8 µs  870.9 µs    912.7 µs   50.0 µs 1055.0 µs   1105.1 µs       1.2x Nisaba        1.21x Nisaba
77. Multi-Scissor Grid (16 Viewports)        20.2 µs 1028.3 µs   1048.5 µs   58.9 µs  598.5 µs    657.4 µs       2.9x Nisaba          1.59x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   21.7 µs 1374.8 µs   1396.5 µs   41.5 µs 1096.1 µs   1137.6 µs       1.9x Nisaba          1.23x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   84.4 µs  933.4 µs   1017.7 µs  975.3 µs  930.8 µs   1906.1 µs      11.6x Nisaba        1.87x Nisaba
80. Scissored Gradient Card (Clipped)        75.7 µs 1232.9 µs   1308.6 µs   87.0 µs  645.3 µs    732.4 µs       1.1x Nisaba          1.79x Skia
81. Circular Overflow Clip (25 Boxes)        24.6 µs 1021.2 µs   1045.7 µs   79.5 µs  504.5 µs    584.0 µs       3.2x Nisaba          1.79x Skia
82. Scissor Invalidation / Reset (25 Cycles)   20.7 µs  936.1 µs    956.8 µs   48.4 µs  989.9 µs   1038.3 µs       2.3x Nisaba        1.09x Nisaba
83. Composite Op: Source-Over (30 Shapes)    20.1 µs 1028.3 µs   1048.4 µs   39.9 µs  957.8 µs    997.8 µs       2.0x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   17.9 µs 1137.1 µs   1155.1 µs   54.3 µs 1002.2 µs   1056.5 µs       3.0x Nisaba          1.09x Skia
85. Composite Op: Source-In (Alpha Masking)   32.0 µs 1012.2 µs   1044.3 µs   64.5 µs  771.3 µs    835.8 µs       2.0x Nisaba          1.25x Skia
86. Composite Op: Source-Out (Cutout)        28.7 µs  937.8 µs    966.5 µs   50.9 µs 1008.2 µs   1059.1 µs       1.8x Nisaba        1.10x Nisaba
87. Composite Op: Atop (Target Bounds)       22.2 µs  955.8 µs    978.0 µs   48.5 µs 1010.0 µs   1058.5 µs       2.2x Nisaba        1.08x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   25.3 µs 1198.3 µs   1223.6 µs   48.1 µs 1029.7 µs   1077.8 µs       1.9x Nisaba          1.14x Skia
89. Composite Op: Dest-Out (Eraser Mask)     23.3 µs 1055.6 µs   1078.9 µs   41.3 µs  955.8 µs    997.1 µs       1.8x Nisaba          1.08x Skia
90. Composite Op: Dest-Atop (Inverse)        19.8 µs 1005.2 µs   1025.0 µs   44.0 µs 1228.9 µs   1272.9 µs       2.2x Nisaba        1.24x Nisaba
91. Composite Op: Xor (Exclusive Blend)      18.7 µs 1067.8 µs   1086.5 µs   40.8 µs  984.0 µs   1024.8 µs       2.2x Nisaba          1.06x Skia
92. Composite Op: Copy (Direct Overwrite)    21.2 µs  976.1 µs    997.4 µs   45.0 µs  953.1 µs    998.1 µs       2.1x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   29.6 µs  937.1 µs    966.7 µs  194.3 µs 1033.9 µs   1228.2 µs       6.6x Nisaba        1.27x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     101.9 µs 1162.9 µs   1264.8 µs  268.8 µs 1059.0 µs   1327.9 µs       2.6x Nisaba         Tie (~1.0x)
95. Circular Progress Rings (8 Meters)       55.4 µs  949.5 µs   1005.0 µs  389.5 µs 1182.0 µs   1571.5 µs       7.0x Nisaba        1.56x Nisaba
96. Modern Card Stack (5 Elevated Cards)     33.9 µs 1846.8 µs   1880.6 µs   81.1 µs 1671.4 µs   1752.6 µs       2.4x Nisaba          1.07x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   36.3 µs  969.8 µs   1006.2 µs   38.3 µs 1169.4 µs   1207.7 µs       1.1x Nisaba        1.20x Nisaba
98. Floating Action Button (FAB 10 Buttons)   49.6 µs  959.0 µs   1008.6 µs  118.5 µs  899.1 µs   1017.6 µs       2.4x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   34.7 µs  984.5 µs   1019.2 µs  111.3 µs  925.6 µs   1037.0 µs       3.2x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   66.1 µs 1584.1 µs   1650.2 µs   71.5 µs 1185.8 µs   1257.2 µs       1.1x Nisaba          1.31x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          4081.4 µs108914.2 µs 112995.6 µs19187.4 µs104116.5 µs 123303.9 µs      4.70x Nisaba        1.09x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 41 suites | Skia won in 38 suites | Ties: 21
         Overall Frame Throughput (End-to-End): 1.09x Nisaba
         Overall CPU Command Submission:       4.70x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 803.6 µs, Skia: 586.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 971.3 µs, Skia: 1077.2 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 995.5 µs, Skia: 1373.3 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1236.9 µs, Skia: 1030.9 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1833.0 µs, Skia: 1424.2 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1485.8 µs, Skia: 1224.3 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 913.7 µs, Skia: 1058.1 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1007.0 µs, Skia: 1058.3 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1092.8 µs, Skia: 1049.2 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1078.5 µs, Skia: 1116.7 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1632.5 µs, Skia: 1443.2 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1567.1 µs, Skia: 1367.4 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2287.6 µs, Skia: 1601.5 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1396.4 µs, Skia: 1414.0 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1099.9 µs, Skia: 1480.7 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 994.7 µs, Skia: 1997.7 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1013.5 µs, Skia: 2150.8 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1196.3 µs, Skia: 1049.3 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1029.8 µs, Skia: 647.3 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 999.2 µs, Skia: 993.5 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1011.2 µs, Skia: 1532.4 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1020.6 µs, Skia: 2562.9 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 994.0 µs, Skia: 1337.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 998.4 µs, Skia: 1741.8 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1028.2 µs, Skia: 1083.7 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1422.6 µs, Skia: 1667.0 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 996.1 µs, Skia: 1602.0 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1009.1 µs, Skia: 2217.3 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 993.9 µs, Skia: 1467.1 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1479.6 µs, Skia: 2599.1 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 997.4 µs, Skia: 1549.7 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1025.3 µs, Skia: 1546.4 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 988.8 µs, Skia: 1571.0 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 996.8 µs, Skia: 1008.0 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 1006.6 µs, Skia: 1107.9 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1017.7 µs, Skia: 1007.5 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1009.0 µs, Skia: 1947.2 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 998.4 µs, Skia: 998.2 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1006.5 µs, Skia: 1234.4 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1023.3 µs, Skia: 1729.0 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1137.1 µs, Skia: 1016.2 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 998.3 µs, Skia: 1274.7 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1248.7 µs, Skia: 1073.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1328.5 µs, Skia: 998.0 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1015.0 µs, Skia: 1115.4 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1077.4 µs, Skia: 996.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1287.2 µs, Skia: 1021.9 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1000.8 µs, Skia: 1027.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1004.0 µs, Skia: 1320.6 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 998.4 µs, Skia: 1392.1 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1291.3 µs, Skia: 1081.9 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1107.9 µs, Skia: 1185.2 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1309.8 µs, Skia: 1012.2 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1438.3 µs, Skia: 1216.6 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1202.5 µs, Skia: 1009.4 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1028.4 µs, Skia: 1014.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1287.5 µs, Skia: 996.4 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1397.3 µs, Skia: 1122.4 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2147.7 µs, Skia: 1687.0 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1021.6 µs, Skia: 1806.0 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1276.5 µs, Skia: 997.5 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1026.5 µs, Skia: 997.2 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1015.8 µs, Skia: 878.7 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1179.1 µs, Skia: 1037.8 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1026.9 µs, Skia: 996.9 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 996.8 µs, Skia: 1064.4 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1005.9 µs, Skia: 1307.2 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1022.7 µs, Skia: 1017.1 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1147.3 µs, Skia: 1007.2 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 997.7 µs, Skia: 996.9 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1001.1 µs, Skia: 997.0 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 1006.2 µs, Skia: 1028.0 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1017.7 µs, Skia: 997.4 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 997.8 µs, Skia: 997.9 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 995.7 µs, Skia: 996.6 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 996.9 µs, Skia: 1008.5 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 998.7 µs, Skia: 939.5 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 974.3 µs, Skia: 784.1 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1010.7 µs, Skia: 1812.7 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1082.8 µs, Skia: 675.9 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 997.5 µs, Skia: 579.9 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1027.8 µs, Skia: 1008.3 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1008.7 µs, Skia: 1005.7 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1163.8 µs, Skia: 999.0 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1006.2 µs, Skia: 1277.9 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1011.7 µs, Skia: 1325.2 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1047.0 µs, Skia: 994.8 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1017.5 µs, Skia: 997.0 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1015.4 µs, Skia: 997.5 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1004.2 µs, Skia: 1296.4 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 948.4 µs, Skia: 914.5 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1008.0 µs, Skia: 997.0 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 997.8 µs, Skia: 1223.6 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1250.3 µs, Skia: 1331.5 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1008.5 µs, Skia: 1506.7 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1689.1 µs, Skia: 1637.1 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1007.4 µs, Skia: 1200.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1008.9 µs, Skia: 1007.2 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1008.5 µs, Skia: 1026.6 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1662.7 µs, Skia: 1254.3 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         18.0 µs  785.6 µs    803.6 µs    9.4 µs  577.2 µs    586.7 µs         1.9x Skia          1.37x Skia
2. 100 Grid Lines (1px Hairlines)            35.7 µs  935.6 µs    971.3 µs   40.5 µs 1036.7 µs   1077.2 µs       1.1x Nisaba        1.11x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      27.8 µs  967.7 µs    995.5 µs  250.5 µs 1122.9 µs   1373.3 µs       9.0x Nisaba        1.38x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      28.5 µs 1208.4 µs   1236.9 µs   86.0 µs  944.9 µs   1030.9 µs       3.0x Nisaba          1.20x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   30.0 µs 1802.9 µs   1833.0 µs   81.2 µs 1343.0 µs   1424.2 µs       2.7x Nisaba          1.29x Skia
6. 50 Simple Rounded Rects (12px Radius)     19.8 µs 1466.0 µs   1485.8 µs   89.8 µs 1134.6 µs   1224.3 µs       4.5x Nisaba          1.21x Skia
7. 50 Varying-Radius Rounded Rects           23.0 µs  890.7 µs    913.7 µs   47.5 µs 1010.6 µs   1058.1 µs       2.1x Nisaba        1.16x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   82.3 µs  924.7 µs   1007.0 µs   61.5 µs  996.8 µs   1058.3 µs         1.3x Skia        1.05x Nisaba
9. 100 Alpha Circles / Disks (Fill)          24.3 µs 1068.5 µs   1092.8 µs   70.1 µs  979.1 µs   1049.2 µs       2.9x Nisaba         Tie (~1.0x)
10. 25 Concentric Rings (2px Stroke)         98.5 µs  980.0 µs   1078.5 µs   43.0 µs 1073.6 µs   1116.7 µs         2.3x Skia         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients            115.1 µs 1517.4 µs   1632.5 µs  189.9 µs 1253.3 µs   1443.2 µs       1.7x Nisaba          1.13x Skia
12. 50 Diagonal Gradients (45° Angle)      109.2 µs 1458.0 µs   1567.1 µs  191.8 µs 1175.6 µs   1367.4 µs       1.8x Nisaba          1.15x Skia
13. 25 Radial Glow Gradients (300px)         27.5 µs 2260.0 µs   2287.6 µs  117.0 µs 1484.6 µs   1601.5 µs       4.3x Nisaba          1.43x Skia
14. 20 Box Blur / Drop Shadows (15px)        27.2 µs 1369.2 µs   1396.4 µs   99.3 µs 1314.7 µs   1414.0 µs       3.7x Nisaba         Tie (~1.0x)
15. 10 Concave 10-Point Stars (Fill)         39.5 µs 1060.4 µs   1099.9 µs  382.1 µs 1098.6 µs   1480.7 µs       9.7x Nisaba        1.35x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   33.4 µs  961.3 µs    994.7 µs  874.8 µs 1122.9 µs   1997.7 µs      26.2x Nisaba        2.01x Nisaba
17. 1000-Point Waveform (Dense Stroke)       92.9 µs  920.6 µs   1013.5 µs  854.6 µs 1296.2 µs   2150.8 µs       9.2x Nisaba        2.12x Nisaba
18. 50 Rotated Cards (Affine Transforms)     91.0 µs 1105.3 µs   1196.3 µs   64.3 µs  985.0 µs   1049.3 µs         1.4x Skia          1.14x Skia
19. 25 Scissor Clipped Viewports             23.2 µs 1006.6 µs   1029.8 µs   66.3 µs  581.0 µs    647.3 µs       2.9x Nisaba          1.59x Skia
20. 50 UI Text Labels (Inter-Regular)       106.5 µs  892.7 µs    999.2 µs   91.1 µs  902.4 µs    993.5 µs         1.2x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   67.6 µs  943.6 µs   1011.2 µs  373.0 µs 1159.5 µs   1532.4 µs       5.5x Nisaba        1.52x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     45.4 µs  975.2 µs   1020.6 µs 1220.8 µs 1342.1 µs   2562.9 µs      26.9x Nisaba        2.51x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       20.8 µs  973.1 µs    994.0 µs  175.4 µs 1162.4 µs   1337.8 µs       8.4x Nisaba        1.35x Nisaba
24. Circular Arcs with arcTo (20 Corners)    32.6 µs  965.8 µs    998.4 µs  483.7 µs 1258.0 µs   1741.8 µs      14.8x Nisaba        1.74x Nisaba
25. Directional Arcs with arc() (30 Arcs)    60.2 µs  968.0 µs   1028.2 µs  142.6 µs  941.1 µs   1083.7 µs       2.4x Nisaba        1.05x Nisaba
26. Compound Path with Holes (15 Donuts)     73.7 µs 1348.9 µs   1422.6 µs  531.2 µs 1135.8 µs   1667.0 µs       7.2x Nisaba        1.17x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    41.7 µs  954.3 µs    996.1 µs  470.6 µs 1131.4 µs   1602.0 µs      11.3x Nisaba        1.61x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    51.3 µs  957.8 µs   1009.1 µs  876.2 µs 1341.0 µs   2217.3 µs      17.1x Nisaba        2.20x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   16.3 µs  977.5 µs    993.9 µs  254.9 µs 1212.2 µs   1467.1 µs      15.6x Nisaba        1.48x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      26.9 µs 1452.8 µs   1479.6 µs 1233.1 µs 1365.9 µs   2599.1 µs      45.9x Nisaba        1.76x Nisaba
31. Stroke Join: Miter (12px Stroke)         16.1 µs  981.3 µs    997.4 µs  398.1 µs 1151.6 µs   1549.7 µs      24.8x Nisaba        1.55x Nisaba
32. Stroke Join: Bevel (12px Stroke)         17.2 µs 1008.2 µs   1025.3 µs  389.8 µs 1156.7 µs   1546.4 µs      22.7x Nisaba        1.51x Nisaba
33. Stroke Join: Round (12px Stroke)         19.4 µs  969.4 µs    988.8 µs  407.5 µs 1163.5 µs   1571.0 µs      21.0x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      26.8 µs  970.0 µs    996.8 µs   44.7 µs  963.3 µs   1008.0 µs       1.7x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    23.3 µs  983.3 µs   1006.6 µs   46.9 µs 1061.0 µs   1107.9 µs       2.0x Nisaba        1.10x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     35.2 µs  982.5 µs   1017.7 µs   60.3 µs  947.2 µs   1007.5 µs       1.7x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   21.6 µs  987.4 µs   1009.0 µs  678.3 µs 1268.9 µs   1947.2 µs      31.4x Nisaba        1.93x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   31.9 µs  966.6 µs    998.4 µs   68.3 µs  929.8 µs    998.2 µs       2.1x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   16.8 µs  989.7 µs   1006.5 µs  153.4 µs 1081.0 µs   1234.4 µs       9.1x Nisaba        1.23x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   30.0 µs  993.2 µs   1023.3 µs  537.4 µs 1191.7 µs   1729.0 µs      17.9x Nisaba        1.69x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          19.6 µs 1117.6 µs   1137.1 µs   43.7 µs  972.6 µs   1016.2 µs       2.2x Nisaba          1.12x Skia
42. 50 Stroked Ellipses (2.5px Border)      108.0 µs  890.3 µs    998.3 µs   38.2 µs 1236.6 µs   1274.7 µs         2.8x Skia        1.28x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         21.3 µs 1227.3 µs   1248.7 µs   61.6 µs 1012.0 µs   1073.6 µs       2.9x Nisaba          1.16x Skia
44. 50 Asymmetric Teardrop Rounded Rects     23.1 µs 1305.4 µs   1328.5 µs   49.2 µs  948.9 µs    998.0 µs       2.1x Nisaba          1.33x Skia
45. High-Density Disks Cloud (500 Disks)     54.7 µs  960.3 µs   1015.0 µs  215.2 µs  900.2 µs   1115.4 µs       3.9x Nisaba        1.10x Nisaba
46. High-Density Stroked Rings (250 Rings)   38.6 µs 1038.8 µs   1077.4 µs  132.8 µs  863.4 µs    996.1 µs       3.4x Nisaba          1.08x Skia
47. 250 Filled Rectangles Batch              38.5 µs 1248.7 µs   1287.2 µs  127.9 µs  894.0 µs   1021.9 µs       3.3x Nisaba          1.26x Skia
48. 150 Stroked Rectangles (2px Border)      66.6 µs  934.2 µs   1000.8 µs   80.6 µs  946.8 µs   1027.3 µs       1.2x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                83.7 µs  920.3 µs   1004.0 µs  290.3 µs 1030.3 µs   1320.6 µs       3.5x Nisaba        1.32x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      44.4 µs  954.0 µs    998.4 µs  296.5 µs 1095.6 µs   1392.1 µs       6.7x Nisaba        1.39x Nisaba
51. 30 Vertical Card Linear Gradients        66.2 µs 1225.1 µs   1291.3 µs  145.2 µs  936.8 µs   1081.9 µs       2.2x Nisaba          1.19x Skia
52. 30 Horizontal Bar Progress Gradients     62.3 µs 1045.6 µs   1107.9 µs  145.2 µs 1040.1 µs   1185.2 µs       2.3x Nisaba        1.07x Nisaba
53. 15 Concentric Radial Spotlights          56.5 µs 1253.3 µs   1309.8 µs   89.5 µs  922.7 µs   1012.2 µs       1.6x Nisaba          1.29x Skia
54. 25 Soft Ambient Card Shadows             29.5 µs 1408.8 µs   1438.3 µs  112.5 µs 1104.1 µs   1216.6 µs       3.8x Nisaba          1.18x Skia
55. 15 Neon Button Glows (Cyan Intense)      25.3 µs 1177.1 µs   1202.5 µs   79.9 µs  929.4 µs   1009.4 µs       3.2x Nisaba          1.19x Skia
56. 16 Multi-Angle Gradient Fan Slices       32.4 µs  996.0 µs   1028.4 µs  103.6 µs  911.0 µs   1014.7 µs       3.2x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    15.5 µs 1272.0 µs   1287.5 µs   30.3 µs  966.2 µs    996.4 µs       2.0x Nisaba          1.29x Skia
58. 20 Inset Well Shadows (Recessed)         48.9 µs 1348.4 µs   1397.3 µs   97.2 µs 1025.2 µs   1122.4 µs       2.0x Nisaba          1.24x Skia
59. 10 Pulsing Circular Radar Wave Glows     48.2 µs 2099.5 µs   2147.7 µs   63.7 µs 1623.4 µs   1687.0 µs       1.3x Nisaba          1.27x Skia
60. Waveform Polyline Stroked with Gradient   57.3 µs  964.3 µs   1021.6 µs  565.0 µs 1241.0 µs   1806.0 µs       9.9x Nisaba        1.77x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   17.6 µs 1259.0 µs   1276.5 µs   31.4 µs  966.1 µs    997.5 µs       1.8x Nisaba          1.28x Skia
62. Scaled Image Quad (2x 512x512 Blit)      19.1 µs 1007.4 µs   1026.5 µs   27.2 µs  970.0 µs    997.2 µs       1.4x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      22.9 µs  992.9 µs   1015.8 µs   33.7 µs  845.1 µs    878.7 µs       1.5x Nisaba          1.16x Skia
64. Translucent Texture Overlay (50% Alpha)   20.3 µs 1158.8 µs   1179.1 µs   37.8 µs 1000.1 µs   1037.8 µs       1.9x Nisaba          1.14x Skia
65. Multi-Avatar Grid (16 Avatars)           55.0 µs  971.9 µs   1026.9 µs   43.1 µs  953.9 µs    996.9 µs         1.3x Skia         Tie (~1.0x)
66. Texture Pattern on Star Path             22.2 µs  974.6 µs    996.8 µs  128.2 µs  936.2 µs   1064.4 µs       5.8x Nisaba        1.07x Nisaba
67. 20px Curved Path Stroked with Texture    22.2 µs  983.7 µs   1005.9 µs  162.6 µs 1144.5 µs   1307.2 µs       7.3x Nisaba        1.30x Nisaba
68. Minified Texture Quad (0.25x Downscale)   45.1 µs  977.6 µs   1022.7 µs   69.8 µs  947.4 µs   1017.1 µs       1.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        24.5 µs 1122.8 µs   1147.3 µs   63.5 µs  943.7 µs   1007.2 µs       2.6x Nisaba          1.14x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   30.9 µs  966.8 µs    997.7 µs   51.7 µs  945.2 µs    996.9 µs       1.7x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   31.5 µs  969.6 µs   1001.1 µs   36.7 µs  960.2 µs    997.0 µs       1.2x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   30.6 µs  975.6 µs   1006.2 µs   47.7 µs  980.2 µs   1028.0 µs       1.6x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   65.7 µs  952.0 µs   1017.7 µs   43.9 µs  953.5 µs    997.4 µs         1.5x Skia         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   24.2 µs  973.5 µs    997.8 µs   40.4 µs  957.5 µs    997.9 µs       1.7x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       22.9 µs  972.8 µs    995.7 µs   55.8 µs  940.8 µs    996.6 µs       2.4x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   39.6 µs  957.3 µs    996.9 µs   37.2 µs  971.2 µs   1008.5 µs         1.1x Skia         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        21.4 µs  977.4 µs    998.7 µs   57.3 µs  882.2 µs    939.5 µs       2.7x Nisaba          1.06x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   22.2 µs  952.1 µs    974.3 µs   35.9 µs  748.2 µs    784.1 µs       1.6x Nisaba          1.24x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   85.2 µs  925.6 µs   1010.7 µs  878.1 µs  934.6 µs   1812.7 µs      10.3x Nisaba        1.79x Nisaba
80. Scissored Gradient Card (Clipped)        64.9 µs 1017.9 µs   1082.8 µs   79.1 µs  596.8 µs    675.9 µs       1.2x Nisaba          1.60x Skia
81. Circular Overflow Clip (25 Boxes)        21.1 µs  976.4 µs    997.5 µs   69.8 µs  510.2 µs    579.9 µs       3.3x Nisaba          1.72x Skia
82. Scissor Invalidation / Reset (25 Cycles)   19.7 µs 1008.1 µs   1027.8 µs   48.9 µs  959.4 µs   1008.3 µs       2.5x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    18.6 µs  990.1 µs   1008.7 µs   41.8 µs  963.8 µs   1005.7 µs       2.3x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   18.5 µs 1145.3 µs   1163.8 µs   45.2 µs  953.8 µs    999.0 µs       2.4x Nisaba          1.16x Skia
85. Composite Op: Source-In (Alpha Masking)   16.3 µs  989.9 µs   1006.2 µs   39.9 µs 1238.0 µs   1277.9 µs       2.4x Nisaba        1.27x Nisaba
86. Composite Op: Source-Out (Cutout)        17.9 µs  993.7 µs   1011.7 µs   43.1 µs 1282.1 µs   1325.2 µs       2.4x Nisaba        1.31x Nisaba
87. Composite Op: Atop (Target Bounds)       18.8 µs 1028.2 µs   1047.0 µs   42.6 µs  952.2 µs    994.8 µs       2.3x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   18.7 µs  998.8 µs   1017.5 µs   44.1 µs  952.9 µs    997.0 µs       2.4x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     18.8 µs  996.6 µs   1015.4 µs   41.9 µs  955.6 µs    997.5 µs       2.2x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        18.2 µs  986.0 µs   1004.2 µs   41.7 µs 1254.7 µs   1296.4 µs       2.3x Nisaba        1.29x Nisaba
91. Composite Op: Xor (Exclusive Blend)      19.1 µs  929.3 µs    948.4 µs   45.9 µs  868.5 µs    914.5 µs       2.4x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    17.9 µs  990.1 µs   1008.0 µs   41.9 µs  955.1 µs    997.0 µs       2.3x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   27.4 µs  970.4 µs    997.8 µs  190.2 µs 1033.3 µs   1223.6 µs       6.9x Nisaba        1.23x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     102.1 µs 1148.1 µs   1250.3 µs  267.6 µs 1063.9 µs   1331.5 µs       2.6x Nisaba        1.06x Nisaba
95. Circular Progress Rings (8 Meters)       53.7 µs  954.8 µs   1008.5 µs  384.0 µs 1122.7 µs   1506.7 µs       7.2x Nisaba        1.49x Nisaba
96. Modern Card Stack (5 Elevated Cards)     28.8 µs 1660.3 µs   1689.1 µs   81.4 µs 1555.7 µs   1637.1 µs       2.8x Nisaba         Tie (~1.0x)
97. CAD Cross-Hatch Pattern (80 Angled Lines)   37.9 µs  969.5 µs   1007.4 µs   41.7 µs 1159.0 µs   1200.7 µs       1.1x Nisaba        1.19x Nisaba
98. Floating Action Button (FAB 10 Buttons)   51.5 µs  957.4 µs   1008.9 µs  122.7 µs  884.4 µs   1007.2 µs       2.4x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   34.8 µs  973.7 µs   1008.5 µs  111.7 µs  914.9 µs   1026.6 µs       3.2x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   65.4 µs 1597.3 µs   1662.7 µs   73.7 µs 1180.7 µs   1254.3 µs       1.1x Nisaba          1.33x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          3982.0 µs108670.5 µs 112652.6 µs18805.9 µs104336.3 µs 123142.2 µs      4.72x Nisaba        1.09x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 40 suites | Skia won in 31 suites | Ties: 29
         Overall Frame Throughput (End-to-End): 1.09x Nisaba
         Overall CPU Command Submission:       4.72x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 726.8 µs, Skia: 777.4 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 997.7 µs, Skia: 1058.2 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1088.7 µs, Skia: 1277.3 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1245.4 µs, Skia: 1014.5 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1624.4 µs, Skia: 1362.8 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1561.1 µs, Skia: 1195.7 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1550.3 µs, Skia: 1046.9 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 997.3 µs, Skia: 1007.1 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1187.6 µs, Skia: 1024.8 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 998.8 µs, Skia: 1126.9 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1675.2 µs, Skia: 1489.9 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1514.2 µs, Skia: 1358.0 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2167.4 µs, Skia: 1627.1 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 849.3 µs, Skia: 1305.2 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1130.0 µs, Skia: 1468.7 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 997.7 µs, Skia: 1999.7 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1022.6 µs, Skia: 2117.5 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1219.4 µs, Skia: 1106.1 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1061.6 µs, Skia: 650.7 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 998.9 µs, Skia: 997.7 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1012.4 µs, Skia: 1526.5 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1020.1 µs, Skia: 2607.7 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 997.6 µs, Skia: 1336.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 998.4 µs, Skia: 1761.5 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1004.4 µs, Skia: 1057.1 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1418.3 µs, Skia: 1715.2 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 997.1 µs, Skia: 1601.8 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 998.2 µs, Skia: 2046.8 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 966.6 µs, Skia: 1445.5 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1493.2 µs, Skia: 2639.3 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1000.9 µs, Skia: 1598.0 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1017.8 µs, Skia: 1556.2 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 993.0 µs, Skia: 1581.5 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 1006.7 µs, Skia: 1008.7 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 1015.6 µs, Skia: 990.2 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 878.9 µs, Skia: 1007.8 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1002.8 µs, Skia: 1977.8 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 998.1 µs, Skia: 1008.6 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1007.4 µs, Skia: 1295.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1000.1 µs, Skia: 1791.2 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1046.9 µs, Skia: 997.4 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1008.7 µs, Skia: 1287.1 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1039.9 µs, Skia: 1013.4 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1307.6 µs, Skia: 1037.2 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1032.5 µs, Skia: 1106.1 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1086.3 µs, Skia: 1005.9 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1299.8 µs, Skia: 1000.8 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1181.6 µs, Skia: 1027.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 997.5 µs, Skia: 1323.4 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1002.0 µs, Skia: 1407.1 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1298.1 µs, Skia: 1085.6 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1371.5 µs, Skia: 1162.6 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1283.0 µs, Skia: 1006.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1463.0 µs, Skia: 1213.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1194.5 µs, Skia: 1019.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1048.5 µs, Skia: 1037.6 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1257.8 µs, Skia: 1005.9 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1388.3 µs, Skia: 1146.2 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2096.4 µs, Skia: 1547.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1027.7 µs, Skia: 1833.5 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1245.4 µs, Skia: 1009.2 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1012.2 µs, Skia: 997.4 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 996.6 µs, Skia: 1017.0 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1320.9 µs, Skia: 907.2 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1046.2 µs, Skia: 997.1 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1007.5 µs, Skia: 1045.7 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 999.0 µs, Skia: 1314.8 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1027.0 µs, Skia: 1016.7 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1066.5 µs, Skia: 996.3 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 997.0 µs, Skia: 998.5 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 997.0 µs, Skia: 998.2 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 997.7 µs, Skia: 1134.4 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 926.9 µs, Skia: 1037.0 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 987.4 µs, Skia: 998.5 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 998.1 µs, Skia: 997.3 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 997.8 µs, Skia: 995.9 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1005.1 µs, Skia: 737.3 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1310.4 µs, Skia: 1045.1 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1012.7 µs, Skia: 1791.7 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1085.2 µs, Skia: 666.8 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1007.9 µs, Skia: 571.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1028.7 µs, Skia: 997.6 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1031.7 µs, Skia: 996.5 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1185.8 µs, Skia: 998.5 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1006.0 µs, Skia: 1301.1 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1025.4 µs, Skia: 1051.4 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 886.0 µs, Skia: 982.3 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1018.2 µs, Skia: 996.3 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1007.1 µs, Skia: 998.6 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1018.3 µs, Skia: 1305.8 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1085.7 µs, Skia: 997.3 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1026.8 µs, Skia: 995.5 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1014.2 µs, Skia: 1216.9 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1262.6 µs, Skia: 1333.0 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1008.8 µs, Skia: 1512.3 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1849.6 µs, Skia: 1687.2 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1012.3 µs, Skia: 1227.2 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1018.4 µs, Skia: 997.8 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 978.8 µs, Skia: 1003.5 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1682.8 µs, Skia: 1249.2 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         14.9 µs  712.0 µs    726.8 µs   10.4 µs  767.0 µs    777.4 µs         1.4x Skia        1.07x Nisaba
2. 100 Grid Lines (1px Hairlines)            41.0 µs  956.7 µs    997.7 µs   42.7 µs 1015.5 µs   1058.2 µs               Tie        1.06x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      31.1 µs 1057.7 µs   1088.7 µs  233.8 µs 1043.4 µs   1277.3 µs       7.5x Nisaba        1.17x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      24.7 µs 1220.7 µs   1245.4 µs   75.1 µs  939.3 µs   1014.5 µs       3.0x Nisaba          1.23x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   25.9 µs 1598.5 µs   1624.4 µs   67.3 µs 1295.5 µs   1362.8 µs       2.6x Nisaba          1.19x Skia
6. 50 Simple Rounded Rects (12px Radius)     18.9 µs 1542.2 µs   1561.1 µs   57.7 µs 1138.0 µs   1195.7 µs       3.1x Nisaba          1.31x Skia
7. 50 Varying-Radius Rounded Rects           22.9 µs 1527.4 µs   1550.3 µs   42.5 µs 1004.4 µs   1046.9 µs       1.9x Nisaba          1.48x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   85.1 µs  912.2 µs    997.3 µs   61.6 µs  945.5 µs   1007.1 µs         1.4x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          22.7 µs 1164.9 µs   1187.6 µs   70.0 µs  954.8 µs   1024.8 µs       3.1x Nisaba          1.16x Skia
10. 25 Concentric Rings (2px Stroke)         86.5 µs  912.3 µs    998.8 µs   38.7 µs 1088.2 µs   1126.9 µs         2.2x Skia        1.13x Nisaba
11. 50 Two-Stop Linear Gradients            103.8 µs 1571.4 µs   1675.2 µs  184.1 µs 1305.8 µs   1489.9 µs       1.8x Nisaba          1.12x Skia
12. 50 Diagonal Gradients (45° Angle)       96.2 µs 1418.1 µs   1514.2 µs  181.0 µs 1176.9 µs   1358.0 µs       1.9x Nisaba          1.12x Skia
13. 25 Radial Glow Gradients (300px)         30.1 µs 2137.3 µs   2167.4 µs  102.7 µs 1524.3 µs   1627.1 µs       3.4x Nisaba          1.33x Skia
14. 20 Box Blur / Drop Shadows (15px)        25.6 µs  823.7 µs    849.3 µs   96.7 µs 1208.5 µs   1305.2 µs       3.8x Nisaba        1.54x Nisaba
15. 10 Concave 10-Point Stars (Fill)         41.0 µs 1089.0 µs   1130.0 µs  378.0 µs 1090.6 µs   1468.7 µs       9.2x Nisaba        1.30x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   32.2 µs  965.5 µs    997.7 µs  863.0 µs 1136.7 µs   1999.7 µs      26.8x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       86.6 µs  936.0 µs   1022.6 µs  891.0 µs 1226.5 µs   2117.5 µs      10.3x Nisaba        2.07x Nisaba
18. 50 Rotated Cards (Affine Transforms)     94.0 µs 1125.4 µs   1219.4 µs   62.1 µs 1043.9 µs   1106.1 µs         1.5x Skia          1.10x Skia
19. 25 Scissor Clipped Viewports             23.6 µs 1038.0 µs   1061.6 µs   64.3 µs  586.4 µs    650.7 µs       2.7x Nisaba          1.63x Skia
20. 50 UI Text Labels (Inter-Regular)       105.4 µs  893.5 µs    998.9 µs   94.1 µs  903.6 µs    997.7 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   68.7 µs  943.7 µs   1012.4 µs  361.0 µs 1165.5 µs   1526.5 µs       5.3x Nisaba        1.51x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     44.5 µs  975.5 µs   1020.1 µs 1233.3 µs 1374.4 µs   2607.7 µs      27.7x Nisaba        2.56x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       19.7 µs  977.9 µs    997.6 µs  173.5 µs 1163.2 µs   1336.8 µs       8.8x Nisaba        1.34x Nisaba
24. Circular Arcs with arcTo (20 Corners)    32.8 µs  965.6 µs    998.4 µs  509.8 µs 1251.7 µs   1761.5 µs      15.6x Nisaba        1.76x Nisaba
25. Directional Arcs with arc() (30 Arcs)    61.9 µs  942.5 µs   1004.4 µs  147.6 µs  909.4 µs   1057.1 µs       2.4x Nisaba        1.05x Nisaba
26. Compound Path with Holes (15 Donuts)     71.5 µs 1346.8 µs   1418.3 µs  527.6 µs 1187.6 µs   1715.2 µs       7.4x Nisaba        1.21x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    45.6 µs  951.4 µs    997.1 µs  473.7 µs 1128.2 µs   1601.8 µs      10.4x Nisaba        1.61x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    54.0 µs  944.2 µs    998.2 µs  844.9 µs 1201.9 µs   2046.8 µs      15.6x Nisaba        2.05x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   18.2 µs  948.5 µs    966.6 µs  256.0 µs 1189.5 µs   1445.5 µs      14.1x Nisaba        1.50x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      25.9 µs 1467.3 µs   1493.2 µs 1235.9 µs 1403.4 µs   2639.3 µs      47.7x Nisaba        1.77x Nisaba
31. Stroke Join: Miter (12px Stroke)         16.3 µs  984.6 µs   1000.9 µs  415.4 µs 1182.7 µs   1598.0 µs      25.4x Nisaba        1.60x Nisaba
32. Stroke Join: Bevel (12px Stroke)         19.9 µs  997.9 µs   1017.8 µs  391.7 µs 1164.5 µs   1556.2 µs      19.6x Nisaba        1.53x Nisaba
33. Stroke Join: Round (12px Stroke)         20.4 µs  972.6 µs    993.0 µs  410.0 µs 1171.5 µs   1581.5 µs      20.1x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      26.0 µs  980.7 µs   1006.7 µs   40.0 µs  968.6 µs   1008.7 µs       1.5x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    21.5 µs  994.0 µs   1015.6 µs   45.5 µs  944.7 µs    990.2 µs       2.1x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     31.5 µs  847.4 µs    878.9 µs   62.9 µs  944.9 µs   1007.8 µs       2.0x Nisaba        1.15x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   23.7 µs  979.1 µs   1002.8 µs  680.4 µs 1297.3 µs   1977.8 µs      28.7x Nisaba        1.97x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   33.7 µs  964.5 µs    998.1 µs   65.7 µs  942.9 µs   1008.6 µs       2.0x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   17.3 µs  990.1 µs   1007.4 µs  163.0 µs 1132.5 µs   1295.5 µs       9.4x Nisaba        1.29x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   28.6 µs  971.5 µs   1000.1 µs  557.6 µs 1233.6 µs   1791.2 µs      19.5x Nisaba        1.79x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          18.4 µs 1028.5 µs   1046.9 µs   41.1 µs  956.3 µs    997.4 µs       2.2x Nisaba         Tie (~1.0x)
42. 50 Stroked Ellipses (2.5px Border)      107.1 µs  901.6 µs   1008.7 µs   42.0 µs 1245.1 µs   1287.1 µs         2.5x Skia        1.28x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         19.3 µs 1020.5 µs   1039.9 µs   57.9 µs  955.5 µs   1013.4 µs       3.0x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     22.2 µs 1285.4 µs   1307.6 µs   44.5 µs  992.7 µs   1037.2 µs       2.0x Nisaba          1.26x Skia
45. High-Density Disks Cloud (500 Disks)     55.6 µs  976.9 µs   1032.5 µs  206.1 µs  900.1 µs   1106.1 µs       3.7x Nisaba        1.07x Nisaba
46. High-Density Stroked Rings (250 Rings)   37.7 µs 1048.6 µs   1086.3 µs  133.9 µs  872.0 µs   1005.9 µs       3.6x Nisaba          1.08x Skia
47. 250 Filled Rectangles Batch              38.6 µs 1261.2 µs   1299.8 µs  131.4 µs  869.4 µs   1000.8 µs       3.4x Nisaba          1.30x Skia
48. 150 Stroked Rectangles (2px Border)      73.2 µs 1108.3 µs   1181.6 µs   85.0 µs  942.3 µs   1027.3 µs       1.2x Nisaba          1.15x Skia
49. 40 Crosshair Aim Reticles                81.1 µs  916.4 µs    997.5 µs  292.7 µs 1030.7 µs   1323.4 µs       3.6x Nisaba        1.33x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      50.5 µs  951.4 µs   1002.0 µs  297.9 µs 1109.2 µs   1407.1 µs       5.9x Nisaba        1.40x Nisaba
51. 30 Vertical Card Linear Gradients        67.9 µs 1230.2 µs   1298.1 µs  141.9 µs  943.6 µs   1085.6 µs       2.1x Nisaba          1.20x Skia
52. 30 Horizontal Bar Progress Gradients     57.0 µs 1314.5 µs   1371.5 µs  148.2 µs 1014.4 µs   1162.6 µs       2.6x Nisaba          1.18x Skia
53. 15 Concentric Radial Spotlights          60.4 µs 1222.6 µs   1283.0 µs   84.6 µs  921.5 µs   1006.1 µs       1.4x Nisaba          1.28x Skia
54. 25 Soft Ambient Card Shadows             29.3 µs 1433.7 µs   1463.0 µs  116.6 µs 1096.9 µs   1213.5 µs       4.0x Nisaba          1.21x Skia
55. 15 Neon Button Glows (Cyan Intense)      26.1 µs 1168.4 µs   1194.5 µs   83.4 µs  936.1 µs   1019.5 µs       3.2x Nisaba          1.17x Skia
56. 16 Multi-Angle Gradient Fan Slices       36.4 µs 1012.1 µs   1048.5 µs  103.1 µs  934.4 µs   1037.6 µs       2.8x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    14.2 µs 1243.6 µs   1257.8 µs   32.1 µs  973.8 µs   1005.9 µs       2.3x Nisaba          1.25x Skia
58. 20 Inset Well Shadows (Recessed)         50.6 µs 1337.7 µs   1388.3 µs   99.0 µs 1047.2 µs   1146.2 µs       2.0x Nisaba          1.21x Skia
59. 10 Pulsing Circular Radar Wave Glows     48.5 µs 2047.9 µs   2096.4 µs   62.2 µs 1485.1 µs   1547.3 µs       1.3x Nisaba          1.35x Skia
60. Waveform Polyline Stroked with Gradient   57.0 µs  970.8 µs   1027.7 µs  567.0 µs 1266.6 µs   1833.5 µs      10.0x Nisaba        1.78x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   17.8 µs 1227.6 µs   1245.4 µs   32.8 µs  976.4 µs   1009.2 µs       1.8x Nisaba          1.23x Skia
62. Scaled Image Quad (2x 512x512 Blit)      18.5 µs  993.7 µs   1012.2 µs   29.2 µs  968.2 µs    997.4 µs       1.6x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      23.5 µs  973.1 µs    996.6 µs   35.8 µs  981.2 µs   1017.0 µs       1.5x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   21.6 µs 1299.3 µs   1320.9 µs   33.5 µs  873.6 µs    907.2 µs       1.6x Nisaba          1.46x Skia
65. Multi-Avatar Grid (16 Avatars)           54.7 µs  991.5 µs   1046.2 µs   42.4 µs  954.7 µs    997.1 µs         1.3x Skia         Tie (~1.0x)
66. Texture Pattern on Star Path             20.8 µs  986.7 µs   1007.5 µs  131.7 µs  914.0 µs   1045.7 µs       6.3x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    21.5 µs  977.6 µs    999.0 µs  162.2 µs 1152.6 µs   1314.8 µs       7.6x Nisaba        1.32x Nisaba
68. Minified Texture Quad (0.25x Downscale)   45.0 µs  982.0 µs   1027.0 µs   70.6 µs  946.1 µs   1016.7 µs       1.6x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        22.4 µs 1044.0 µs   1066.5 µs   61.3 µs  935.0 µs    996.3 µs       2.7x Nisaba          1.07x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   36.0 µs  961.0 µs    997.0 µs   50.4 µs  948.1 µs    998.5 µs       1.4x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   29.2 µs  967.8 µs    997.0 µs   35.4 µs  962.7 µs    998.2 µs       1.2x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   27.9 µs  969.8 µs    997.7 µs   49.4 µs 1085.0 µs   1134.4 µs       1.8x Nisaba        1.14x Nisaba
73. Combined Affine Transforms (30 Shapes)   66.9 µs  860.0 µs    926.9 µs   46.4 µs  990.6 µs   1037.0 µs         1.4x Skia        1.12x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   26.5 µs  960.9 µs    987.4 µs   37.4 µs  961.1 µs    998.5 µs       1.4x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       23.4 µs  974.7 µs    998.1 µs   54.3 µs  943.0 µs    997.3 µs       2.3x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   43.2 µs  954.6 µs    997.8 µs   43.0 µs  952.9 µs    995.9 µs               Tie         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        20.7 µs  984.4 µs   1005.1 µs   55.3 µs  682.0 µs    737.3 µs       2.7x Nisaba          1.36x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   22.6 µs 1287.8 µs   1310.4 µs   38.7 µs 1006.5 µs   1045.1 µs       1.7x Nisaba          1.25x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   83.0 µs  929.7 µs   1012.7 µs  878.6 µs  913.0 µs   1791.7 µs      10.6x Nisaba        1.77x Nisaba
80. Scissored Gradient Card (Clipped)        66.2 µs 1019.0 µs   1085.2 µs   77.6 µs  589.2 µs    666.8 µs       1.2x Nisaba          1.63x Skia
81. Circular Overflow Clip (25 Boxes)        25.8 µs  982.1 µs   1007.9 µs   68.3 µs  502.7 µs    571.0 µs       2.6x Nisaba          1.77x Skia
82. Scissor Invalidation / Reset (25 Cycles)   21.0 µs 1007.7 µs   1028.7 µs   44.4 µs  953.2 µs    997.6 µs       2.1x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    19.5 µs 1012.2 µs   1031.7 µs   41.3 µs  955.2 µs    996.5 µs       2.1x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   21.2 µs 1164.6 µs   1185.8 µs   43.3 µs  955.2 µs    998.5 µs       2.0x Nisaba          1.19x Skia
85. Composite Op: Source-In (Alpha Masking)   18.2 µs  987.8 µs   1006.0 µs   39.7 µs 1261.4 µs   1301.1 µs       2.2x Nisaba        1.29x Nisaba
86. Composite Op: Source-Out (Cutout)        19.3 µs 1006.2 µs   1025.4 µs   42.0 µs 1009.4 µs   1051.4 µs       2.2x Nisaba         Tie (~1.0x)
87. Composite Op: Atop (Target Bounds)       18.6 µs  867.4 µs    886.0 µs   42.8 µs  939.4 µs    982.3 µs       2.3x Nisaba        1.11x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   18.2 µs 1000.0 µs   1018.2 µs   40.5 µs  955.8 µs    996.3 µs       2.2x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     19.3 µs  987.8 µs   1007.1 µs   38.5 µs  960.1 µs    998.6 µs       2.0x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        17.9 µs 1000.4 µs   1018.3 µs   41.3 µs 1264.6 µs   1305.8 µs       2.3x Nisaba        1.28x Nisaba
91. Composite Op: Xor (Exclusive Blend)      19.1 µs 1066.5 µs   1085.7 µs   42.6 µs  954.7 µs    997.3 µs       2.2x Nisaba          1.09x Skia
92. Composite Op: Copy (Direct Overwrite)    23.4 µs 1003.4 µs   1026.8 µs   41.4 µs  954.1 µs    995.5 µs       1.8x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   26.2 µs  988.0 µs   1014.2 µs  199.6 µs 1017.3 µs   1216.9 µs       7.6x Nisaba        1.20x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     102.5 µs 1160.1 µs   1262.6 µs  270.5 µs 1062.5 µs   1333.0 µs       2.6x Nisaba        1.06x Nisaba
95. Circular Progress Rings (8 Meters)       51.2 µs  957.7 µs   1008.8 µs  361.6 µs 1150.7 µs   1512.3 µs       7.1x Nisaba        1.50x Nisaba
96. Modern Card Stack (5 Elevated Cards)     34.2 µs 1815.3 µs   1849.6 µs   80.5 µs 1606.7 µs   1687.2 µs       2.4x Nisaba          1.10x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   38.4 µs  973.8 µs   1012.3 µs   38.3 µs 1189.0 µs   1227.2 µs               Tie        1.21x Nisaba
98. Floating Action Button (FAB 10 Buttons)   51.9 µs  966.4 µs   1018.4 µs  116.0 µs  881.8 µs    997.8 µs       2.2x Nisaba         Tie (~1.0x)
99. Anti-Aliasing Geometry Grid (50 Diamonds)   40.0 µs  938.8 µs    978.8 µs  112.3 µs  891.2 µs   1003.5 µs       2.8x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   66.4 µs 1616.4 µs   1682.8 µs   72.7 µs 1176.4 µs   1249.2 µs       1.1x Nisaba          1.35x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          3989.0 µs109076.3 µs 113065.4 µs18727.0 µs104223.3 µs 122950.4 µs      4.69x Nisaba        1.09x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 41 suites | Skia won in 33 suites | Ties: 26
         Overall Frame Throughput (End-to-End): 1.09x Nisaba
         Overall CPU Command Submission:       4.69x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 920.4 µs, Skia: 882.3 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1007.0 µs, Skia: 1095.5 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1014.5 µs, Skia: 1307.4 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1214.3 µs, Skia: 1003.4 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1574.5 µs, Skia: 1375.4 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1158.9 µs, Skia: 1147.6 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1533.9 µs, Skia: 1154.4 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1006.6 µs, Skia: 995.4 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1178.1 µs, Skia: 1013.3 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 996.9 µs, Skia: 1146.2 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1618.6 µs, Skia: 1484.3 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1507.9 µs, Skia: 1349.6 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2246.3 µs, Skia: 1627.8 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1320.3 µs, Skia: 1422.0 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1141.4 µs, Skia: 1486.7 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1009.0 µs, Skia: 2018.6 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1039.2 µs, Skia: 2068.6 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1191.2 µs, Skia: 1098.8 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1055.0 µs, Skia: 642.7 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 997.8 µs, Skia: 995.4 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1010.4 µs, Skia: 1563.4 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 1041.3 µs, Skia: 2589.6 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 1002.0 µs, Skia: 1346.1 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1000.9 µs, Skia: 1783.2 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 993.0 µs, Skia: 1038.2 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1398.9 µs, Skia: 1715.1 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1001.0 µs, Skia: 1603.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 999.9 µs, Skia: 2111.1 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 1001.1 µs, Skia: 1467.1 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1498.0 µs, Skia: 2638.2 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1077.3 µs, Skia: 1509.8 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1001.6 µs, Skia: 1534.1 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 995.1 µs, Skia: 1596.1 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 998.7 µs, Skia: 1033.9 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 1008.0 µs, Skia: 1025.5 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1045.5 µs, Skia: 995.6 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 998.1 µs, Skia: 1957.2 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.6 µs, Skia: 1005.5 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 993.6 µs, Skia: 1290.1 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1018.1 µs, Skia: 1738.1 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1117.9 µs, Skia: 1016.4 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1015.0 µs, Skia: 1257.4 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1326.3 µs, Skia: 888.4 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1355.8 µs, Skia: 1001.4 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1008.0 µs, Skia: 1148.0 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1130.9 µs, Skia: 995.4 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1312.6 µs, Skia: 1084.8 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 991.2 µs, Skia: 1041.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1009.2 µs, Skia: 1327.1 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1003.6 µs, Skia: 1411.7 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1292.1 µs, Skia: 1171.8 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1357.9 µs, Skia: 1152.6 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1289.5 µs, Skia: 1010.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1436.0 µs, Skia: 1206.8 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1215.2 µs, Skia: 1068.8 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1029.3 µs, Skia: 994.4 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1173.3 µs, Skia: 991.8 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1385.6 µs, Skia: 1108.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2161.8 µs, Skia: 1717.8 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1002.1 µs, Skia: 1853.4 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1290.8 µs, Skia: 1000.5 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1008.8 µs, Skia: 994.7 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1037.5 µs, Skia: 994.9 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1135.8 µs, Skia: 999.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1078.0 µs, Skia: 998.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 999.7 µs, Skia: 1024.0 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 996.7 µs, Skia: 1348.7 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1034.2 µs, Skia: 1000.7 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1182.5 µs, Skia: 1019.0 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 999.1 µs, Skia: 996.5 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 997.2 µs, Skia: 996.5 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 1027.1 µs, Skia: 997.0 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1028.8 µs, Skia: 993.5 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 997.7 µs, Skia: 998.5 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 995.7 µs, Skia: 995.2 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 997.1 µs, Skia: 1017.3 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1051.2 µs, Skia: 727.6 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1407.7 µs, Skia: 1110.3 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1008.5 µs, Skia: 1860.0 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1097.4 µs, Skia: 669.0 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1001.7 µs, Skia: 570.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1017.2 µs, Skia: 995.7 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1017.0 µs, Skia: 998.0 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1158.1 µs, Skia: 995.0 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1010.2 µs, Skia: 1257.6 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 959.6 µs, Skia: 1318.1 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 997.8 µs, Skia: 994.4 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1037.0 µs, Skia: 996.6 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1017.2 µs, Skia: 995.8 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 996.2 µs, Skia: 1309.5 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1046.7 µs, Skia: 994.9 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 996.7 µs, Skia: 987.8 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 997.2 µs, Skia: 1224.3 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1244.8 µs, Skia: 1396.3 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1001.7 µs, Skia: 1525.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 2052.6 µs, Skia: 1717.1 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1002.8 µs, Skia: 1166.9 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1068.4 µs, Skia: 1007.7 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1018.1 µs, Skia: 1005.4 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1637.7 µs, Skia: 1236.4 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         14.9 µs  905.5 µs    920.4 µs   10.0 µs  872.3 µs    882.3 µs         1.5x Skia         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            37.1 µs  969.9 µs   1007.0 µs   43.4 µs 1052.1 µs   1095.5 µs       1.2x Nisaba        1.09x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      27.8 µs  986.7 µs   1014.5 µs  233.6 µs 1073.9 µs   1307.4 µs       8.4x Nisaba        1.29x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      26.2 µs 1188.1 µs   1214.3 µs   72.6 µs  930.8 µs   1003.4 µs       2.8x Nisaba          1.21x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   24.1 µs 1550.4 µs   1574.5 µs   70.0 µs 1305.4 µs   1375.4 µs       2.9x Nisaba          1.14x Skia
6. 50 Simple Rounded Rects (12px Radius)     21.8 µs 1137.1 µs   1158.9 µs   55.5 µs 1092.0 µs   1147.6 µs       2.5x Nisaba         Tie (~1.0x)
7. 50 Varying-Radius Rounded Rects           22.3 µs 1511.5 µs   1533.9 µs   46.0 µs 1108.4 µs   1154.4 µs       2.1x Nisaba          1.33x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   81.8 µs  924.9 µs   1006.6 µs   57.1 µs  938.3 µs    995.4 µs         1.4x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          23.6 µs 1154.5 µs   1178.1 µs   67.1 µs  946.2 µs   1013.3 µs       2.8x Nisaba          1.16x Skia
10. 25 Concentric Rings (2px Stroke)         84.6 µs  912.3 µs    996.9 µs   36.0 µs 1110.3 µs   1146.2 µs         2.4x Skia        1.15x Nisaba
11. 50 Two-Stop Linear Gradients             96.9 µs 1521.8 µs   1618.6 µs  180.3 µs 1304.0 µs   1484.3 µs       1.9x Nisaba          1.09x Skia
12. 50 Diagonal Gradients (45° Angle)       96.1 µs 1411.8 µs   1507.9 µs  186.4 µs 1163.2 µs   1349.6 µs       1.9x Nisaba          1.12x Skia
13. 25 Radial Glow Gradients (300px)         28.4 µs 2217.9 µs   2246.3 µs  103.3 µs 1524.5 µs   1627.8 µs       3.6x Nisaba          1.38x Skia
14. 20 Box Blur / Drop Shadows (15px)        26.8 µs 1293.5 µs   1320.3 µs   99.0 µs 1323.0 µs   1422.0 µs       3.7x Nisaba        1.08x Nisaba
15. 10 Concave 10-Point Stars (Fill)         42.3 µs 1099.1 µs   1141.4 µs  376.6 µs 1110.1 µs   1486.7 µs       8.9x Nisaba        1.30x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   34.3 µs  974.7 µs   1009.0 µs  860.6 µs 1158.0 µs   2018.6 µs      25.1x Nisaba        2.00x Nisaba
17. 1000-Point Waveform (Dense Stroke)       90.7 µs  948.4 µs   1039.2 µs  836.9 µs 1231.7 µs   2068.6 µs       9.2x Nisaba        1.99x Nisaba
18. 50 Rotated Cards (Affine Transforms)     95.1 µs 1096.2 µs   1191.2 µs   62.7 µs 1036.1 µs   1098.8 µs         1.5x Skia          1.08x Skia
19. 25 Scissor Clipped Viewports             21.6 µs 1033.3 µs   1055.0 µs   64.2 µs  578.5 µs    642.7 µs       3.0x Nisaba          1.64x Skia
20. 50 UI Text Labels (Inter-Regular)       105.7 µs  892.2 µs    997.8 µs   90.6 µs  904.8 µs    995.4 µs         1.2x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   65.7 µs  944.7 µs   1010.4 µs  378.4 µs 1185.0 µs   1563.4 µs       5.8x Nisaba        1.55x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     45.6 µs  995.8 µs   1041.3 µs 1230.2 µs 1359.4 µs   2589.6 µs      27.0x Nisaba        2.49x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       18.2 µs  983.8 µs   1002.0 µs  169.0 µs 1177.2 µs   1346.1 µs       9.3x Nisaba        1.34x Nisaba
24. Circular Arcs with arcTo (20 Corners)    33.5 µs  967.4 µs   1000.9 µs  507.7 µs 1275.5 µs   1783.2 µs      15.2x Nisaba        1.78x Nisaba
25. Directional Arcs with arc() (30 Arcs)    60.2 µs  932.8 µs    993.0 µs  142.3 µs  896.0 µs   1038.2 µs       2.4x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     72.3 µs 1326.6 µs   1398.9 µs  532.7 µs 1182.4 µs   1715.1 µs       7.4x Nisaba        1.23x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    43.4 µs  957.6 µs   1001.0 µs  472.1 µs 1131.3 µs   1603.4 µs      10.9x Nisaba        1.60x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    50.3 µs  949.6 µs    999.9 µs  864.9 µs 1246.2 µs   2111.1 µs      17.2x Nisaba        2.11x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   15.7 µs  985.4 µs   1001.1 µs  255.8 µs 1211.3 µs   1467.1 µs      16.3x Nisaba        1.47x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      26.7 µs 1471.3 µs   1498.0 µs 1239.8 µs 1398.4 µs   2638.2 µs      46.4x Nisaba        1.76x Nisaba
31. Stroke Join: Miter (12px Stroke)         18.5 µs 1058.9 µs   1077.3 µs  403.3 µs 1106.5 µs   1509.8 µs      21.8x Nisaba        1.40x Nisaba
32. Stroke Join: Bevel (12px Stroke)         15.3 µs  986.2 µs   1001.6 µs  392.3 µs 1141.8 µs   1534.1 µs      25.6x Nisaba        1.53x Nisaba
33. Stroke Join: Round (12px Stroke)         21.7 µs  973.4 µs    995.1 µs  405.6 µs 1190.5 µs   1596.1 µs      18.7x Nisaba        1.60x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      23.8 µs  974.9 µs    998.7 µs   43.6 µs  990.3 µs   1033.9 µs       1.8x Nisaba         Tie (~1.0x)
35. Stroke Cap: Square (30 Segments 12px)    26.0 µs  982.0 µs   1008.0 µs   43.6 µs  982.0 µs   1025.5 µs       1.7x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     33.1 µs 1012.5 µs   1045.5 µs   59.9 µs  935.7 µs    995.6 µs       1.8x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   21.4 µs  976.7 µs    998.1 µs  697.9 µs 1259.3 µs   1957.2 µs      32.5x Nisaba        1.96x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   33.5 µs  964.1 µs    997.6 µs   66.1 µs  939.4 µs   1005.5 µs       2.0x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   17.4 µs  976.2 µs    993.6 µs  148.4 µs 1141.7 µs   1290.1 µs       8.5x Nisaba        1.30x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   30.4 µs  987.7 µs   1018.1 µs  548.9 µs 1189.2 µs   1738.1 µs      18.0x Nisaba        1.71x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          24.8 µs 1093.1 µs   1117.9 µs   40.2 µs  976.2 µs   1016.4 µs       1.6x Nisaba          1.10x Skia
42. 50 Stroked Ellipses (2.5px Border)      107.9 µs  907.1 µs   1015.0 µs   42.5 µs 1214.9 µs   1257.4 µs         2.5x Skia        1.24x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         23.0 µs 1303.4 µs   1326.3 µs   58.6 µs  829.8 µs    888.4 µs       2.6x Nisaba          1.49x Skia
44. 50 Asymmetric Teardrop Rounded Rects     24.3 µs 1331.5 µs   1355.8 µs   44.1 µs  957.4 µs   1001.4 µs       1.8x Nisaba          1.35x Skia
45. High-Density Disks Cloud (500 Disks)     56.5 µs  951.5 µs   1008.0 µs  207.5 µs  940.4 µs   1148.0 µs       3.7x Nisaba        1.14x Nisaba
46. High-Density Stroked Rings (250 Rings)   38.8 µs 1092.1 µs   1130.9 µs  126.6 µs  868.8 µs    995.4 µs       3.3x Nisaba          1.14x Skia
47. 250 Filled Rectangles Batch              39.7 µs 1272.9 µs   1312.6 µs  126.0 µs  958.8 µs   1084.8 µs       3.2x Nisaba          1.21x Skia
48. 150 Stroked Rectangles (2px Border)      68.5 µs  922.7 µs    991.2 µs   83.2 µs  958.1 µs   1041.3 µs       1.2x Nisaba        1.05x Nisaba
49. 40 Crosshair Aim Reticles                83.2 µs  926.0 µs   1009.2 µs  297.5 µs 1029.6 µs   1327.1 µs       3.6x Nisaba        1.32x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      51.1 µs  952.5 µs   1003.6 µs  297.8 µs 1113.9 µs   1411.7 µs       5.8x Nisaba        1.41x Nisaba
51. 30 Vertical Card Linear Gradients        61.9 µs 1230.2 µs   1292.1 µs  155.3 µs 1016.5 µs   1171.8 µs       2.5x Nisaba          1.10x Skia
52. 30 Horizontal Bar Progress Gradients     59.2 µs 1298.7 µs   1357.9 µs  144.0 µs 1008.6 µs   1152.6 µs       2.4x Nisaba          1.18x Skia
53. 15 Concentric Radial Spotlights          61.1 µs 1228.4 µs   1289.5 µs   88.3 µs  921.8 µs   1010.1 µs       1.4x Nisaba          1.28x Skia
54. 25 Soft Ambient Card Shadows             31.0 µs 1405.0 µs   1436.0 µs  117.3 µs 1089.5 µs   1206.8 µs       3.8x Nisaba          1.19x Skia
55. 15 Neon Button Glows (Cyan Intense)      25.0 µs 1190.2 µs   1215.2 µs   83.2 µs  985.6 µs   1068.8 µs       3.3x Nisaba          1.14x Skia
56. 16 Multi-Angle Gradient Fan Slices       33.2 µs  996.1 µs   1029.3 µs  100.6 µs  893.8 µs    994.4 µs       3.0x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    16.4 µs 1156.9 µs   1173.3 µs   31.6 µs  960.2 µs    991.8 µs       1.9x Nisaba          1.18x Skia
58. 20 Inset Well Shadows (Recessed)         51.6 µs 1334.0 µs   1385.6 µs   98.3 µs 1010.2 µs   1108.5 µs       1.9x Nisaba          1.25x Skia
59. 10 Pulsing Circular Radar Wave Glows     52.4 µs 2109.4 µs   2161.8 µs   62.4 µs 1655.4 µs   1717.8 µs       1.2x Nisaba          1.26x Skia
60. Waveform Polyline Stroked with Gradient   55.1 µs  946.9 µs   1002.1 µs  571.0 µs 1282.3 µs   1853.4 µs      10.4x Nisaba        1.85x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   18.9 µs 1271.9 µs   1290.8 µs   29.0 µs  971.4 µs   1000.5 µs       1.5x Nisaba          1.29x Skia
62. Scaled Image Quad (2x 512x512 Blit)      20.4 µs  988.4 µs   1008.8 µs   28.2 µs  966.5 µs    994.7 µs       1.4x Nisaba         Tie (~1.0x)
63. 45° Rotated Pattern Fill in Circle      22.8 µs 1014.7 µs   1037.5 µs   36.9 µs  958.0 µs    994.9 µs       1.6x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   18.6 µs 1117.2 µs   1135.8 µs   36.0 µs  962.9 µs    999.0 µs       1.9x Nisaba          1.14x Skia
65. Multi-Avatar Grid (16 Avatars)           57.7 µs 1020.3 µs   1078.0 µs   43.2 µs  955.1 µs    998.2 µs         1.3x Skia          1.08x Skia
66. Texture Pattern on Star Path             25.9 µs  973.8 µs    999.7 µs  132.0 µs  892.0 µs   1024.0 µs       5.1x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    20.7 µs  976.0 µs    996.7 µs  182.3 µs 1166.4 µs   1348.7 µs       8.8x Nisaba        1.35x Nisaba
68. Minified Texture Quad (0.25x Downscale)   44.7 µs  989.5 µs   1034.2 µs   68.5 µs  932.2 µs   1000.7 µs       1.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        24.6 µs 1157.9 µs   1182.5 µs   63.9 µs  955.1 µs   1019.0 µs       2.6x Nisaba          1.16x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   32.8 µs  966.2 µs    999.1 µs   50.5 µs  946.0 µs    996.5 µs       1.5x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   27.0 µs  970.2 µs    997.2 µs   35.7 µs  960.8 µs    996.5 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   32.1 µs  995.0 µs   1027.1 µs   48.3 µs  948.7 µs    997.0 µs       1.5x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   63.9 µs  964.9 µs   1028.8 µs   41.0 µs  952.6 µs    993.5 µs         1.6x Skia         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   25.0 µs  972.7 µs    997.7 µs   40.4 µs  958.1 µs    998.5 µs       1.6x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       23.9 µs  971.9 µs    995.7 µs   51.3 µs  943.9 µs    995.2 µs       2.1x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   38.2 µs  959.0 µs    997.1 µs   42.6 µs  974.7 µs   1017.3 µs       1.1x Nisaba         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        22.7 µs 1028.6 µs   1051.2 µs   53.5 µs  674.2 µs    727.6 µs       2.4x Nisaba          1.44x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   19.1 µs 1388.6 µs   1407.7 µs   37.2 µs 1073.1 µs   1110.3 µs       2.0x Nisaba          1.27x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   89.8 µs  918.6 µs   1008.5 µs  886.4 µs  973.7 µs   1860.0 µs       9.9x Nisaba        1.84x Nisaba
80. Scissored Gradient Card (Clipped)        67.4 µs 1030.1 µs   1097.4 µs   81.6 µs  587.5 µs    669.0 µs       1.2x Nisaba          1.64x Skia
81. Circular Overflow Clip (25 Boxes)        24.0 µs  977.7 µs   1001.7 µs   70.7 µs  499.3 µs    570.0 µs       2.9x Nisaba          1.76x Skia
82. Scissor Invalidation / Reset (25 Cycles)   22.3 µs  994.9 µs   1017.2 µs   43.2 µs  952.4 µs    995.7 µs       1.9x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    17.8 µs  999.2 µs   1017.0 µs   41.7 µs  956.2 µs    998.0 µs       2.4x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   19.9 µs 1138.2 µs   1158.1 µs   42.7 µs  952.3 µs    995.0 µs       2.2x Nisaba          1.16x Skia
85. Composite Op: Source-In (Alpha Masking)   19.1 µs  991.1 µs   1010.2 µs   39.7 µs 1218.0 µs   1257.6 µs       2.1x Nisaba        1.24x Nisaba
86. Composite Op: Source-Out (Cutout)        19.3 µs  940.3 µs    959.6 µs   41.6 µs 1276.5 µs   1318.1 µs       2.2x Nisaba        1.37x Nisaba
87. Composite Op: Atop (Target Bounds)       18.3 µs  979.5 µs    997.8 µs   40.7 µs  953.7 µs    994.4 µs       2.2x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   21.9 µs 1015.2 µs   1037.0 µs   40.2 µs  956.4 µs    996.6 µs       1.8x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     19.4 µs  997.8 µs   1017.2 µs   40.3 µs  955.5 µs    995.8 µs       2.1x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        17.4 µs  978.7 µs    996.2 µs   41.5 µs 1268.0 µs   1309.5 µs       2.4x Nisaba        1.31x Nisaba
91. Composite Op: Xor (Exclusive Blend)      18.9 µs 1027.8 µs   1046.7 µs   41.2 µs  953.6 µs    994.9 µs       2.2x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    20.2 µs  976.6 µs    996.7 µs   41.6 µs  946.1 µs    987.8 µs       2.1x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   26.4 µs  970.7 µs    997.2 µs  185.4 µs 1038.9 µs   1224.3 µs       7.0x Nisaba        1.23x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     103.5 µs 1141.2 µs   1244.8 µs  283.2 µs 1113.1 µs   1396.3 µs       2.7x Nisaba        1.12x Nisaba
95. Circular Progress Rings (8 Meters)       52.9 µs  948.8 µs   1001.7 µs  361.8 µs 1163.5 µs   1525.4 µs       6.8x Nisaba        1.52x Nisaba
96. Modern Card Stack (5 Elevated Cards)     33.2 µs 2019.4 µs   2052.6 µs   79.3 µs 1637.7 µs   1717.1 µs       2.4x Nisaba          1.20x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   36.4 µs  966.4 µs   1002.8 µs   37.9 µs 1129.0 µs   1166.9 µs               Tie        1.16x Nisaba
98. Floating Action Button (FAB 10 Buttons)   47.0 µs 1021.4 µs   1068.4 µs  117.4 µs  890.3 µs   1007.7 µs       2.5x Nisaba          1.06x Skia
99. Anti-Aliasing Geometry Grid (50 Diamonds)   36.5 µs  981.6 µs   1018.1 µs  107.9 µs  897.5 µs   1005.4 µs       3.0x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   67.7 µs 1570.0 µs   1637.7 µs   70.7 µs 1165.8 µs   1236.4 µs               Tie          1.32x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          3977.4 µs110097.9 µs 114075.3 µs18687.6 µs105051.1 µs 123738.7 µs      4.70x Nisaba        1.08x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 37 suites | Skia won in 34 suites | Ties: 29
         Overall Frame Throughput (End-to-End): 1.08x Nisaba
         Overall CPU Command Submission:       4.70x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 833.3 µs, Skia: 731.6 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 970.0 µs, Skia: 1076.8 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1019.1 µs, Skia: 1295.9 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1217.7 µs, Skia: 1008.3 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1531.6 µs, Skia: 1357.6 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1197.2 µs, Skia: 1001.6 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1557.3 µs, Skia: 1081.7 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 996.8 µs, Skia: 1010.2 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1150.7 µs, Skia: 1002.2 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 998.0 µs, Skia: 1068.4 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1616.6 µs, Skia: 1473.0 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1503.3 µs, Skia: 1351.3 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2186.6 µs, Skia: 1685.8 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1302.2 µs, Skia: 1401.2 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1138.2 µs, Skia: 1478.5 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 995.9 µs, Skia: 2127.7 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 999.5 µs, Skia: 2047.0 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1209.6 µs, Skia: 1082.6 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1051.1 µs, Skia: 626.6 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 999.2 µs, Skia: 995.4 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 996.7 µs, Skia: 1551.4 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 998.9 µs, Skia: 2627.1 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 996.1 µs, Skia: 1338.7 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1002.0 µs, Skia: 1773.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1029.8 µs, Skia: 1014.8 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1408.4 µs, Skia: 1706.1 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1004.4 µs, Skia: 1709.2 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 993.3 µs, Skia: 2056.6 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 1001.1 µs, Skia: 1453.7 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1470.4 µs, Skia: 2648.4 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 999.0 µs, Skia: 1573.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 1000.4 µs, Skia: 1552.9 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 995.5 µs, Skia: 1578.4 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 997.5 µs, Skia: 1065.7 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 997.0 µs, Skia: 1047.8 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 998.0 µs, Skia: 1003.6 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 979.2 µs, Skia: 1977.3 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 997.9 µs, Skia: 1139.5 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 534.4 µs, Skia: 1244.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1002.0 µs, Skia: 1779.3 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1164.5 µs, Skia: 997.7 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 999.3 µs, Skia: 1259.4 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1119.4 µs, Skia: 1002.4 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1306.8 µs, Skia: 1008.2 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1014.5 µs, Skia: 1134.1 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1107.5 µs, Skia: 1001.6 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1295.0 µs, Skia: 1045.3 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1028.1 µs, Skia: 1003.6 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 999.7 µs, Skia: 1328.5 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 998.1 µs, Skia: 1416.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1306.5 µs, Skia: 1162.4 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1351.4 µs, Skia: 1144.2 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1300.6 µs, Skia: 1001.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1460.3 µs, Skia: 1224.9 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1218.3 µs, Skia: 1033.8 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1025.6 µs, Skia: 992.8 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1239.0 µs, Skia: 999.4 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1396.9 µs, Skia: 1142.7 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 2119.6 µs, Skia: 1917.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1000.7 µs, Skia: 1844.1 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1207.4 µs, Skia: 996.8 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1051.6 µs, Skia: 924.0 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 998.5 µs, Skia: 1014.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1334.0 µs, Skia: 996.6 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1055.9 µs, Skia: 996.7 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 996.6 µs, Skia: 1034.8 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1007.2 µs, Skia: 1314.9 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1017.3 µs, Skia: 996.7 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1156.3 µs, Skia: 998.5 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 995.9 µs, Skia: 996.1 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1007.6 µs, Skia: 981.8 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 998.3 µs, Skia: 1024.1 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 997.5 µs, Skia: 1005.2 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 996.6 µs, Skia: 995.6 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 995.2 µs, Skia: 994.4 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 996.3 µs, Skia: 818.1 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 997.3 µs, Skia: 717.8 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1260.8 µs, Skia: 1091.2 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1004.3 µs, Skia: 1824.1 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1056.5 µs, Skia: 671.3 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 992.2 µs, Skia: 567.7 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 968.2 µs, Skia: 979.0 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1036.7 µs, Skia: 997.3 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1049.9 µs, Skia: 996.7 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1025.0 µs, Skia: 1347.0 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1020.4 µs, Skia: 1295.8 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1017.6 µs, Skia: 994.9 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1018.1 µs, Skia: 995.6 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1026.6 µs, Skia: 995.5 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1054.6 µs, Skia: 1074.4 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1057.3 µs, Skia: 993.8 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1008.6 µs, Skia: 984.9 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1001.4 µs, Skia: 1237.0 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1236.8 µs, Skia: 1356.9 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 996.7 µs, Skia: 1513.3 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1810.9 µs, Skia: 1707.2 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1014.8 µs, Skia: 1215.2 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1078.6 µs, Skia: 1018.3 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1011.7 µs, Skia: 1004.4 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1686.6 µs, Skia: 1235.5 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         14.0 µs  819.2 µs    833.3 µs    9.6 µs  722.0 µs    731.6 µs         1.5x Skia          1.14x Skia
2. 100 Grid Lines (1px Hairlines)            39.5 µs  930.4 µs    970.0 µs   46.2 µs 1030.6 µs   1076.8 µs       1.2x Nisaba        1.11x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      30.5 µs  988.6 µs   1019.1 µs  232.2 µs 1063.7 µs   1295.9 µs       7.6x Nisaba        1.27x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      25.9 µs 1191.8 µs   1217.7 µs   73.6 µs  934.8 µs   1008.3 µs       2.8x Nisaba          1.21x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   24.8 µs 1506.9 µs   1531.6 µs   69.6 µs 1288.0 µs   1357.6 µs       2.8x Nisaba          1.13x Skia
6. 50 Simple Rounded Rects (12px Radius)     20.4 µs 1176.8 µs   1197.2 µs   55.3 µs  946.3 µs   1001.6 µs       2.7x Nisaba          1.20x Skia
7. 50 Varying-Radius Rounded Rects           22.6 µs 1534.7 µs   1557.3 µs   47.3 µs 1034.4 µs   1081.7 µs       2.1x Nisaba          1.44x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   85.3 µs  911.5 µs    996.8 µs   55.9 µs  954.3 µs   1010.2 µs         1.5x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          24.7 µs 1126.0 µs   1150.7 µs   66.2 µs  936.0 µs   1002.2 µs       2.7x Nisaba          1.15x Skia
10. 25 Concentric Rings (2px Stroke)         84.5 µs  913.5 µs    998.0 µs   40.1 µs 1028.3 µs   1068.4 µs         2.1x Skia        1.07x Nisaba
11. 50 Two-Stop Linear Gradients            100.7 µs 1515.8 µs   1616.6 µs  180.4 µs 1292.6 µs   1473.0 µs       1.8x Nisaba          1.10x Skia
12. 50 Diagonal Gradients (45° Angle)       99.8 µs 1403.5 µs   1503.3 µs  181.0 µs 1170.3 µs   1351.3 µs       1.8x Nisaba          1.11x Skia
13. 25 Radial Glow Gradients (300px)         26.7 µs 2159.9 µs   2186.6 µs  103.2 µs 1582.5 µs   1685.8 µs       3.9x Nisaba          1.30x Skia
14. 20 Box Blur / Drop Shadows (15px)        27.3 µs 1274.9 µs   1302.2 µs   95.2 µs 1306.0 µs   1401.2 µs       3.5x Nisaba        1.08x Nisaba
15. 10 Concave 10-Point Stars (Fill)         43.2 µs 1095.0 µs   1138.2 µs  369.6 µs 1108.9 µs   1478.5 µs       8.6x Nisaba        1.30x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   29.5 µs  966.4 µs    995.9 µs  954.3 µs 1173.5 µs   2127.7 µs      32.3x Nisaba        2.14x Nisaba
17. 1000-Point Waveform (Dense Stroke)       91.3 µs  908.2 µs    999.5 µs  867.1 µs 1179.9 µs   2047.0 µs       9.5x Nisaba        2.05x Nisaba
18. 50 Rotated Cards (Affine Transforms)     90.8 µs 1118.8 µs   1209.6 µs   65.8 µs 1016.9 µs   1082.6 µs         1.4x Skia          1.12x Skia
19. 25 Scissor Clipped Viewports             24.3 µs 1026.8 µs   1051.1 µs   64.2 µs  562.4 µs    626.6 µs       2.6x Nisaba          1.68x Skia
20. 50 UI Text Labels (Inter-Regular)       106.5 µs  892.7 µs    999.2 µs   92.2 µs  903.2 µs    995.4 µs         1.2x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   67.4 µs  929.3 µs    996.7 µs  354.9 µs 1196.5 µs   1551.4 µs       5.3x Nisaba        1.56x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     44.6 µs  954.3 µs    998.9 µs 1242.5 µs 1384.6 µs   2627.1 µs      27.9x Nisaba        2.63x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       19.5 µs  976.6 µs    996.1 µs  172.2 µs 1166.6 µs   1338.7 µs       8.8x Nisaba        1.34x Nisaba
24. Circular Arcs with arcTo (20 Corners)    32.6 µs  969.4 µs   1002.0 µs  514.3 µs 1259.0 µs   1773.3 µs      15.8x Nisaba        1.77x Nisaba
25. Directional Arcs with arc() (30 Arcs)    61.2 µs  968.6 µs   1029.8 µs  141.6 µs  873.2 µs   1014.8 µs       2.3x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     74.5 µs 1333.9 µs   1408.4 µs  533.7 µs 1172.4 µs   1706.1 µs       7.2x Nisaba        1.21x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    44.2 µs  960.3 µs   1004.4 µs  483.7 µs 1225.6 µs   1709.2 µs      10.9x Nisaba        1.70x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    54.1 µs  939.3 µs    993.3 µs  845.9 µs 1210.7 µs   2056.6 µs      15.6x Nisaba        2.07x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   17.8 µs  983.3 µs   1001.1 µs  263.0 µs 1190.7 µs   1453.7 µs      14.8x Nisaba        1.45x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      25.2 µs 1445.2 µs   1470.4 µs 1261.5 µs 1386.9 µs   2648.4 µs      50.0x Nisaba        1.80x Nisaba
31. Stroke Join: Miter (12px Stroke)         16.7 µs  982.3 µs    999.0 µs  395.0 µs 1178.1 µs   1573.1 µs      23.7x Nisaba        1.57x Nisaba
32. Stroke Join: Bevel (12px Stroke)         16.2 µs  984.3 µs   1000.4 µs  394.8 µs 1158.1 µs   1552.9 µs      24.4x Nisaba        1.55x Nisaba
33. Stroke Join: Round (12px Stroke)         20.3 µs  975.2 µs    995.5 µs  404.1 µs 1174.3 µs   1578.4 µs      19.9x Nisaba        1.59x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      24.5 µs  973.0 µs    997.5 µs   45.6 µs 1020.1 µs   1065.7 µs       1.9x Nisaba        1.07x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    24.1 µs  972.9 µs    997.0 µs   47.4 µs 1000.4 µs   1047.8 µs       2.0x Nisaba        1.05x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     35.9 µs  962.1 µs    998.0 µs   58.6 µs  945.0 µs   1003.6 µs       1.6x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   23.7 µs  955.5 µs    979.2 µs  680.9 µs 1296.3 µs   1977.3 µs      28.7x Nisaba        2.02x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   35.4 µs  962.5 µs    997.9 µs   69.5 µs 1070.0 µs   1139.5 µs       2.0x Nisaba        1.14x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   15.8 µs  518.6 µs    534.4 µs  148.5 µs 1096.0 µs   1244.5 µs       9.4x Nisaba        2.33x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   28.8 µs  973.2 µs   1002.0 µs  536.7 µs 1242.6 µs   1779.3 µs      18.6x Nisaba        1.78x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          19.7 µs 1144.8 µs   1164.5 µs   40.3 µs  957.4 µs    997.7 µs       2.0x Nisaba          1.17x Skia
42. 50 Stroked Ellipses (2.5px Border)      110.0 µs  889.2 µs    999.3 µs   40.6 µs 1218.8 µs   1259.4 µs         2.7x Skia        1.26x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         22.9 µs 1096.5 µs   1119.4 µs   56.7 µs  945.7 µs   1002.4 µs       2.5x Nisaba          1.12x Skia
44. 50 Asymmetric Teardrop Rounded Rects     22.9 µs 1283.9 µs   1306.8 µs   54.0 µs  954.2 µs   1008.2 µs       2.4x Nisaba          1.30x Skia
45. High-Density Disks Cloud (500 Disks)     59.2 µs  955.3 µs   1014.5 µs  208.7 µs  925.3 µs   1134.1 µs       3.5x Nisaba        1.12x Nisaba
46. High-Density Stroked Rings (250 Rings)   40.7 µs 1066.8 µs   1107.5 µs  129.6 µs  872.0 µs   1001.6 µs       3.2x Nisaba          1.11x Skia
47. 250 Filled Rectangles Batch              39.3 µs 1255.8 µs   1295.0 µs  130.9 µs  914.4 µs   1045.3 µs       3.3x Nisaba          1.24x Skia
48. 150 Stroked Rectangles (2px Border)      68.1 µs  960.0 µs   1028.1 µs   79.3 µs  924.3 µs   1003.6 µs       1.2x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                81.9 µs  917.7 µs    999.7 µs  299.9 µs 1028.5 µs   1328.5 µs       3.7x Nisaba        1.33x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      47.0 µs  951.2 µs    998.1 µs  299.4 µs 1117.0 µs   1416.4 µs       6.4x Nisaba        1.42x Nisaba
51. 30 Vertical Card Linear Gradients        62.8 µs 1243.7 µs   1306.5 µs  153.8 µs 1008.6 µs   1162.4 µs       2.4x Nisaba          1.12x Skia
52. 30 Horizontal Bar Progress Gradients     61.7 µs 1289.6 µs   1351.4 µs  147.6 µs  996.7 µs   1144.2 µs       2.4x Nisaba          1.18x Skia
53. 15 Concentric Radial Spotlights          61.2 µs 1239.4 µs   1300.6 µs   90.4 µs  910.7 µs   1001.1 µs       1.5x Nisaba          1.30x Skia
54. 25 Soft Ambient Card Shadows             28.3 µs 1432.0 µs   1460.3 µs  114.8 µs 1110.1 µs   1224.9 µs       4.1x Nisaba          1.19x Skia
55. 15 Neon Button Glows (Cyan Intense)      25.4 µs 1193.0 µs   1218.3 µs   81.6 µs  952.2 µs   1033.8 µs       3.2x Nisaba          1.18x Skia
56. 16 Multi-Angle Gradient Fan Slices       36.0 µs  989.5 µs   1025.6 µs   96.9 µs  895.9 µs    992.8 µs       2.7x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    19.2 µs 1219.8 µs   1239.0 µs   33.4 µs  966.0 µs    999.4 µs       1.7x Nisaba          1.24x Skia
58. 20 Inset Well Shadows (Recessed)         52.7 µs 1344.2 µs   1396.9 µs   96.6 µs 1046.0 µs   1142.7 µs       1.8x Nisaba          1.22x Skia
59. 10 Pulsing Circular Radar Wave Glows     53.0 µs 2066.5 µs   2119.6 µs   67.6 µs 1849.7 µs   1917.3 µs       1.3x Nisaba          1.11x Skia
60. Waveform Polyline Stroked with Gradient   52.1 µs  948.6 µs   1000.7 µs  562.7 µs 1281.4 µs   1844.1 µs      10.8x Nisaba        1.84x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   18.1 µs 1189.3 µs   1207.4 µs   32.3 µs  964.5 µs    996.8 µs       1.8x Nisaba          1.21x Skia
62. Scaled Image Quad (2x 512x512 Blit)      20.7 µs 1030.9 µs   1051.6 µs   31.3 µs  892.7 µs    924.0 µs       1.5x Nisaba          1.14x Skia
63. 45° Rotated Pattern Fill in Circle      22.5 µs  976.0 µs    998.5 µs   36.6 µs  977.8 µs   1014.4 µs       1.6x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   22.2 µs 1311.8 µs   1334.0 µs   34.0 µs  962.6 µs    996.6 µs       1.5x Nisaba          1.34x Skia
65. Multi-Avatar Grid (16 Avatars)           59.3 µs  996.6 µs   1055.9 µs   41.5 µs  955.1 µs    996.7 µs         1.4x Skia          1.06x Skia
66. Texture Pattern on Star Path             20.7 µs  975.9 µs    996.6 µs  128.3 µs  906.6 µs   1034.8 µs       6.2x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    25.8 µs  981.4 µs   1007.2 µs  166.9 µs 1148.0 µs   1314.9 µs       6.5x Nisaba        1.31x Nisaba
68. Minified Texture Quad (0.25x Downscale)   47.8 µs  969.5 µs   1017.3 µs   68.8 µs  927.9 µs    996.7 µs       1.4x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        24.3 µs 1132.0 µs   1156.3 µs   64.5 µs  934.0 µs    998.5 µs       2.7x Nisaba          1.16x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   33.2 µs  962.7 µs    995.9 µs   47.9 µs  948.2 µs    996.1 µs       1.4x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   26.9 µs  980.7 µs   1007.6 µs   34.0 µs  947.7 µs    981.8 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   30.1 µs  968.2 µs    998.3 µs   47.7 µs  976.4 µs   1024.1 µs       1.6x Nisaba         Tie (~1.0x)
73. Combined Affine Transforms (30 Shapes)   65.0 µs  932.5 µs    997.5 µs   47.0 µs  958.2 µs   1005.2 µs         1.4x Skia         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   26.6 µs  970.0 µs    996.6 µs   41.0 µs  954.5 µs    995.6 µs       1.5x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       22.8 µs  972.5 µs    995.2 µs   53.0 µs  941.4 µs    994.4 µs       2.3x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   40.6 µs  955.7 µs    996.3 µs   41.6 µs  776.6 µs    818.1 µs               Tie          1.22x Skia
77. Multi-Scissor Grid (16 Viewports)        21.1 µs  976.2 µs    997.3 µs   56.4 µs  661.4 µs    717.8 µs       2.7x Nisaba          1.39x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   18.9 µs 1241.9 µs   1260.8 µs   39.4 µs 1051.8 µs   1091.2 µs       2.1x Nisaba          1.16x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   85.4 µs  918.9 µs   1004.3 µs  868.8 µs  955.2 µs   1824.1 µs      10.2x Nisaba        1.82x Nisaba
80. Scissored Gradient Card (Clipped)        67.4 µs  989.1 µs   1056.5 µs   80.6 µs  590.7 µs    671.3 µs       1.2x Nisaba          1.57x Skia
81. Circular Overflow Clip (25 Boxes)        24.9 µs  967.4 µs    992.2 µs   66.5 µs  501.2 µs    567.7 µs       2.7x Nisaba          1.75x Skia
82. Scissor Invalidation / Reset (25 Cycles)   21.2 µs  947.0 µs    968.2 µs   45.4 µs  933.6 µs    979.0 µs       2.1x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    20.9 µs 1015.7 µs   1036.7 µs   40.3 µs  957.0 µs    997.3 µs       1.9x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   21.4 µs 1028.5 µs   1049.9 µs   43.5 µs  953.2 µs    996.7 µs       2.0x Nisaba          1.05x Skia
85. Composite Op: Source-In (Alpha Masking)   19.2 µs 1005.7 µs   1025.0 µs   41.7 µs 1305.3 µs   1347.0 µs       2.2x Nisaba        1.31x Nisaba
86. Composite Op: Source-Out (Cutout)        19.3 µs 1001.1 µs   1020.4 µs   41.9 µs 1253.9 µs   1295.8 µs       2.2x Nisaba        1.27x Nisaba
87. Composite Op: Atop (Target Bounds)       19.8 µs  997.7 µs   1017.6 µs   40.9 µs  954.0 µs    994.9 µs       2.1x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   17.3 µs 1000.7 µs   1018.1 µs   42.1 µs  953.5 µs    995.6 µs       2.4x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     20.7 µs 1005.9 µs   1026.6 µs   42.5 µs  953.0 µs    995.5 µs       2.1x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        20.3 µs 1034.3 µs   1054.6 µs   43.5 µs 1030.9 µs   1074.4 µs       2.1x Nisaba         Tie (~1.0x)
91. Composite Op: Xor (Exclusive Blend)      19.3 µs 1038.0 µs   1057.3 µs   40.1 µs  953.7 µs    993.8 µs       2.1x Nisaba          1.06x Skia
92. Composite Op: Copy (Direct Overwrite)    20.0 µs  988.6 µs   1008.6 µs   41.2 µs  943.7 µs    984.9 µs       2.1x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   32.0 µs  969.4 µs   1001.4 µs  188.2 µs 1048.7 µs   1237.0 µs       5.9x Nisaba        1.24x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     103.9 µs 1132.9 µs   1236.8 µs  271.6 µs 1085.2 µs   1356.9 µs       2.6x Nisaba        1.10x Nisaba
95. Circular Progress Rings (8 Meters)       53.6 µs  943.1 µs    996.7 µs  367.1 µs 1146.2 µs   1513.3 µs       6.8x Nisaba        1.52x Nisaba
96. Modern Card Stack (5 Elevated Cards)     33.6 µs 1777.3 µs   1810.9 µs   82.5 µs 1624.7 µs   1707.2 µs       2.5x Nisaba          1.06x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   42.0 µs  972.7 µs   1014.8 µs   40.9 µs 1174.2 µs   1215.2 µs               Tie        1.20x Nisaba
98. Floating Action Button (FAB 10 Buttons)   53.9 µs 1024.7 µs   1078.6 µs  119.0 µs  899.2 µs   1018.3 µs       2.2x Nisaba          1.06x Skia
99. Anti-Aliasing Geometry Grid (50 Diamonds)   36.9 µs  974.8 µs   1011.7 µs  110.8 µs  893.6 µs   1004.4 µs       3.0x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   66.4 µs 1620.2 µs   1686.6 µs   72.6 µs 1162.9 µs   1235.5 µs       1.1x Nisaba          1.37x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          4025.2 µs108545.9 µs 112571.2 µs18791.9 µs104515.8 µs 123307.7 µs      4.67x Nisaba        1.10x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 38 suites | Skia won in 39 suites | Ties: 23
         Overall Frame Throughput (End-to-End): 1.10x Nisaba
         Overall CPU Command Submission:       4.67x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 891.5 µs, Skia: 893.8 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1087.6 µs, Skia: 1077.8 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1035.0 µs, Skia: 1297.4 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1218.0 µs, Skia: 1007.4 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1728.8 µs, Skia: 1385.1 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1515.2 µs, Skia: 1201.8 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1575.2 µs, Skia: 1088.2 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1002.0 µs, Skia: 1003.6 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1172.8 µs, Skia: 1039.6 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1007.2 µs, Skia: 1147.7 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1629.1 µs, Skia: 1447.3 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1497.5 µs, Skia: 1329.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2347.1 µs, Skia: 1486.2 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1644.1 µs, Skia: 1414.0 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1093.9 µs, Skia: 1489.0 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 998.1 µs, Skia: 2017.1 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1028.5 µs, Skia: 2054.5 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1211.5 µs, Skia: 1073.9 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1019.1 µs, Skia: 634.9 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1027.6 µs, Skia: 1008.7 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1041.9 µs, Skia: 1517.0 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 998.8 µs, Skia: 2576.2 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 1000.5 µs, Skia: 1367.2 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1003.9 µs, Skia: 1788.2 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 997.8 µs, Skia: 1015.4 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1414.7 µs, Skia: 1725.2 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1017.1 µs, Skia: 1588.1 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1028.9 µs, Skia: 2012.4 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 988.4 µs, Skia: 1467.1 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1507.6 µs, Skia: 2654.2 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 1019.9 µs, Skia: 1563.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 985.2 µs, Skia: 1558.4 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 999.4 µs, Skia: 1656.9 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 626.9 µs, Skia: 1021.8 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 1025.3 µs, Skia: 1028.0 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 998.9 µs, Skia: 1007.9 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1002.7 µs, Skia: 1976.5 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 1018.5 µs, Skia: 1008.2 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 1011.1 µs, Skia: 1243.5 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1003.2 µs, Skia: 1745.1 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 1140.3 µs, Skia: 1014.7 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1048.0 µs, Skia: 1278.8 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1258.8 µs, Skia: 1017.2 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1202.2 µs, Skia: 1007.7 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1028.8 µs, Skia: 1151.5 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1069.0 µs, Skia: 1054.8 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1233.9 µs, Skia: 1026.8 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 997.9 µs, Skia: 998.0 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1010.9 µs, Skia: 1309.0 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 998.2 µs, Skia: 1431.3 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1298.9 µs, Skia: 1108.4 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1329.8 µs, Skia: 1125.2 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1296.8 µs, Skia: 1024.2 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1411.2 µs, Skia: 1229.7 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1208.1 µs, Skia: 1038.8 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1018.1 µs, Skia: 998.4 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1223.6 µs, Skia: 997.7 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1354.7 µs, Skia: 1224.9 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1235.2 µs, Skia: 1575.1 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1000.2 µs, Skia: 1819.9 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1240.1 µs, Skia: 995.0 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1008.3 µs, Skia: 839.3 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1007.4 µs, Skia: 1016.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1254.5 µs, Skia: 1001.8 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1018.2 µs, Skia: 997.9 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 998.2 µs, Skia: 1011.5 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 984.5 µs, Skia: 1271.4 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1027.6 µs, Skia: 997.6 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1168.4 µs, Skia: 1006.4 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 996.6 µs, Skia: 998.7 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 998.4 µs, Skia: 989.6 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 793.8 µs, Skia: 999.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1026.7 µs, Skia: 998.3 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 997.8 µs, Skia: 998.0 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 996.3 µs, Skia: 997.7 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 996.4 µs, Skia: 979.3 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1024.0 µs, Skia: 705.0 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1242.0 µs, Skia: 1075.7 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1010.0 µs, Skia: 1815.0 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1096.8 µs, Skia: 672.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1029.2 µs, Skia: 583.4 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 998.2 µs, Skia: 1015.0 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1017.5 µs, Skia: 996.5 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1056.8 µs, Skia: 996.1 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 997.8 µs, Skia: 1334.2 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 752.8 µs, Skia: 1289.4 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1036.9 µs, Skia: 997.0 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1016.3 µs, Skia: 999.0 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1026.8 µs, Skia: 998.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 1017.2 µs, Skia: 1327.0 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1017.5 µs, Skia: 1018.0 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 999.0 µs, Skia: 996.8 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 995.9 µs, Skia: 1236.7 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1262.8 µs, Skia: 1341.4 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 996.1 µs, Skia: 1512.6 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1967.0 µs, Skia: 1717.3 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1007.3 µs, Skia: 1208.2 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1059.0 µs, Skia: 998.6 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1019.2 µs, Skia: 997.7 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1651.0 µs, Skia: 1247.6 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)         15.9 µs  875.6 µs    891.5 µs    9.4 µs  884.3 µs    893.8 µs         1.7x Skia         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            39.7 µs 1047.9 µs   1087.6 µs   42.6 µs 1035.3 µs   1077.8 µs       1.1x Nisaba         Tie (~1.0x)
3. 20 Thick Diagonal Lines (16px Round)      30.6 µs 1004.5 µs   1035.0 µs  234.4 µs 1063.0 µs   1297.4 µs       7.7x Nisaba        1.25x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      23.9 µs 1194.0 µs   1218.0 µs   75.4 µs  932.0 µs   1007.4 µs       3.2x Nisaba          1.21x Skia
5. 100 Alpha Rectangles (Overlapping Blend)   26.6 µs 1702.2 µs   1728.8 µs   69.8 µs 1315.3 µs   1385.1 µs       2.6x Nisaba          1.25x Skia
6. 50 Simple Rounded Rects (12px Radius)     22.3 µs 1492.9 µs   1515.2 µs   60.0 µs 1141.8 µs   1201.8 µs       2.7x Nisaba          1.26x Skia
7. 50 Varying-Radius Rounded Rects           20.3 µs 1554.8 µs   1575.2 µs   49.0 µs 1039.2 µs   1088.2 µs       2.4x Nisaba          1.45x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   84.7 µs  917.3 µs   1002.0 µs   59.5 µs  944.1 µs   1003.6 µs         1.4x Skia         Tie (~1.0x)
9. 100 Alpha Circles / Disks (Fill)          23.7 µs 1149.1 µs   1172.8 µs   70.3 µs  969.3 µs   1039.6 µs       3.0x Nisaba          1.13x Skia
10. 25 Concentric Rings (2px Stroke)         86.4 µs  920.8 µs   1007.2 µs   42.8 µs 1104.9 µs   1147.7 µs         2.0x Skia        1.14x Nisaba
11. 50 Two-Stop Linear Gradients             90.0 µs 1539.1 µs   1629.1 µs  173.3 µs 1274.0 µs   1447.3 µs       1.9x Nisaba          1.13x Skia
12. 50 Diagonal Gradients (45° Angle)       94.1 µs 1403.4 µs   1497.5 µs  181.2 µs 1148.7 µs   1329.9 µs       1.9x Nisaba          1.13x Skia
13. 25 Radial Glow Gradients (300px)         32.6 µs 2314.5 µs   2347.1 µs  100.3 µs 1385.9 µs   1486.2 µs       3.1x Nisaba          1.58x Skia
14. 20 Box Blur / Drop Shadows (15px)        28.7 µs 1615.3 µs   1644.1 µs   96.0 µs 1318.0 µs   1414.0 µs       3.3x Nisaba          1.16x Skia
15. 10 Concave 10-Point Stars (Fill)         40.4 µs 1053.5 µs   1093.9 µs  369.6 µs 1119.4 µs   1489.0 µs       9.1x Nisaba        1.36x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   33.1 µs  965.0 µs    998.1 µs  906.2 µs 1110.9 µs   2017.1 µs      27.4x Nisaba        2.02x Nisaba
17. 1000-Point Waveform (Dense Stroke)       93.8 µs  934.7 µs   1028.5 µs  858.7 µs 1195.8 µs   2054.5 µs       9.2x Nisaba        2.00x Nisaba
18. 50 Rotated Cards (Affine Transforms)     92.7 µs 1118.8 µs   1211.5 µs   66.2 µs 1007.7 µs   1073.9 µs         1.4x Skia          1.13x Skia
19. 25 Scissor Clipped Viewports             22.5 µs  996.5 µs   1019.1 µs   68.0 µs  566.8 µs    634.9 µs       3.0x Nisaba          1.61x Skia
20. 50 UI Text Labels (Inter-Regular)       108.9 µs  918.8 µs   1027.6 µs   91.8 µs  916.9 µs   1008.7 µs         1.2x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)   69.7 µs  972.2 µs   1041.9 µs  355.6 µs 1161.4 µs   1517.0 µs       5.1x Nisaba        1.46x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     48.0 µs  950.7 µs    998.8 µs 1218.7 µs 1357.5 µs   2576.2 µs      25.4x Nisaba        2.58x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       23.6 µs  976.9 µs   1000.5 µs  181.9 µs 1185.4 µs   1367.2 µs       7.7x Nisaba        1.37x Nisaba
24. Circular Arcs with arcTo (20 Corners)    35.6 µs  968.3 µs   1003.9 µs  498.4 µs 1289.8 µs   1788.2 µs      14.0x Nisaba        1.78x Nisaba
25. Directional Arcs with arc() (30 Arcs)    61.1 µs  936.7 µs    997.8 µs  145.2 µs  870.2 µs   1015.4 µs       2.4x Nisaba         Tie (~1.0x)
26. Compound Path with Holes (15 Donuts)     70.6 µs 1344.1 µs   1414.7 µs  518.1 µs 1207.1 µs   1725.2 µs       7.3x Nisaba        1.22x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    44.2 µs  972.9 µs   1017.1 µs  490.8 µs 1097.3 µs   1588.1 µs      11.1x Nisaba        1.56x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    56.2 µs  972.6 µs   1028.9 µs  831.4 µs 1181.0 µs   2012.4 µs      14.8x Nisaba        1.96x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   17.3 µs  971.0 µs    988.4 µs  258.4 µs 1208.6 µs   1467.1 µs      14.9x Nisaba        1.48x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      28.3 µs 1479.4 µs   1507.6 µs 1243.9 µs 1410.3 µs   2654.2 µs      44.0x Nisaba        1.76x Nisaba
31. Stroke Join: Miter (12px Stroke)         21.4 µs  998.5 µs   1019.9 µs  391.0 µs 1172.1 µs   1563.1 µs      18.3x Nisaba        1.53x Nisaba
32. Stroke Join: Bevel (12px Stroke)         21.2 µs  964.0 µs    985.2 µs  390.6 µs 1167.8 µs   1558.4 µs      18.4x Nisaba        1.58x Nisaba
33. Stroke Join: Round (12px Stroke)         21.0 µs  978.4 µs    999.4 µs  412.0 µs 1244.9 µs   1656.9 µs      19.6x Nisaba        1.66x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      25.2 µs  601.7 µs    626.9 µs   47.4 µs  974.4 µs   1021.8 µs       1.9x Nisaba        1.63x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    25.2 µs 1000.1 µs   1025.3 µs   48.3 µs  979.6 µs   1028.0 µs       1.9x Nisaba         Tie (~1.0x)
36. Stroke Cap: Round (30 Segments 12px)     36.3 µs  962.6 µs    998.9 µs   63.7 µs  944.2 µs   1007.9 µs       1.8x Nisaba         Tie (~1.0x)
37. Miter Limit Clamping (10° Acute Spikes)   18.9 µs  983.8 µs   1002.7 µs  695.6 µs 1280.9 µs   1976.5 µs      36.8x Nisaba        1.97x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   33.6 µs  984.9 µs   1018.5 µs   72.5 µs  935.6 µs   1008.2 µs       2.2x Nisaba         Tie (~1.0x)
39. Ultra-Heavy Geometric Ribbon (40px Wide)   20.8 µs  990.3 µs   1011.1 µs  149.5 µs 1094.0 µs   1243.5 µs       7.2x Nisaba        1.23x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   30.0 µs  973.1 µs   1003.2 µs  548.1 µs 1197.0 µs   1745.1 µs      18.2x Nisaba        1.74x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          20.8 µs 1119.5 µs   1140.3 µs   43.4 µs  971.4 µs   1014.7 µs       2.1x Nisaba          1.12x Skia
42. 50 Stroked Ellipses (2.5px Border)      108.1 µs  939.9 µs   1048.0 µs   43.9 µs 1234.9 µs   1278.8 µs         2.5x Skia        1.22x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         25.8 µs 1232.9 µs   1258.8 µs   60.8 µs  956.5 µs   1017.2 µs       2.4x Nisaba          1.24x Skia
44. 50 Asymmetric Teardrop Rounded Rects     23.6 µs 1178.6 µs   1202.2 µs   48.9 µs  958.9 µs   1007.7 µs       2.1x Nisaba          1.19x Skia
45. High-Density Disks Cloud (500 Disks)     55.4 µs  973.5 µs   1028.8 µs  208.6 µs  942.9 µs   1151.5 µs       3.8x Nisaba        1.12x Nisaba
46. High-Density Stroked Rings (250 Rings)   38.3 µs 1030.7 µs   1069.0 µs  139.5 µs  915.3 µs   1054.8 µs       3.6x Nisaba         Tie (~1.0x)
47. 250 Filled Rectangles Batch              40.7 µs 1193.2 µs   1233.9 µs  129.3 µs  897.5 µs   1026.8 µs       3.2x Nisaba          1.20x Skia
48. 150 Stroked Rectangles (2px Border)      68.0 µs  929.9 µs    997.9 µs   82.8 µs  915.3 µs    998.0 µs       1.2x Nisaba         Tie (~1.0x)
49. 40 Crosshair Aim Reticles                85.2 µs  925.7 µs   1010.9 µs  291.9 µs 1017.1 µs   1309.0 µs       3.4x Nisaba        1.29x Nisaba
50. 25 Regular Octagons (Fill + Stroke)      43.2 µs  955.0 µs    998.2 µs  303.2 µs 1128.1 µs   1431.3 µs       7.0x Nisaba        1.43x Nisaba
51. 30 Vertical Card Linear Gradients        71.6 µs 1227.3 µs   1298.9 µs  153.0 µs  955.3 µs   1108.4 µs       2.1x Nisaba          1.17x Skia
52. 30 Horizontal Bar Progress Gradients     62.4 µs 1267.5 µs   1329.8 µs  147.7 µs  977.5 µs   1125.2 µs       2.4x Nisaba          1.18x Skia
53. 15 Concentric Radial Spotlights          60.0 µs 1236.9 µs   1296.8 µs   91.3 µs  932.9 µs   1024.2 µs       1.5x Nisaba          1.27x Skia
54. 25 Soft Ambient Card Shadows             29.5 µs 1381.7 µs   1411.2 µs  119.5 µs 1110.3 µs   1229.7 µs       4.1x Nisaba          1.15x Skia
55. 15 Neon Button Glows (Cyan Intense)      27.8 µs 1180.3 µs   1208.1 µs   86.4 µs  952.4 µs   1038.8 µs       3.1x Nisaba          1.16x Skia
56. 16 Multi-Angle Gradient Fan Slices       35.1 µs  983.0 µs   1018.1 µs  103.2 µs  895.2 µs    998.4 µs       2.9x Nisaba         Tie (~1.0x)
57. Full-Screen Diagonal Horizon Gradient    20.5 µs 1203.1 µs   1223.6 µs   36.6 µs  961.1 µs    997.7 µs       1.8x Nisaba          1.23x Skia
58. 20 Inset Well Shadows (Recessed)         47.9 µs 1306.8 µs   1354.7 µs  107.7 µs 1117.2 µs   1224.9 µs       2.2x Nisaba          1.11x Skia
59. 10 Pulsing Circular Radar Wave Glows     55.6 µs 1179.6 µs   1235.2 µs   62.5 µs 1512.7 µs   1575.1 µs       1.1x Nisaba        1.28x Nisaba
60. Waveform Polyline Stroked with Gradient   54.5 µs  945.7 µs   1000.2 µs  569.0 µs 1250.9 µs   1819.9 µs      10.4x Nisaba        1.82x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   19.7 µs 1220.3 µs   1240.1 µs   33.8 µs  961.2 µs    995.0 µs       1.7x Nisaba          1.25x Skia
62. Scaled Image Quad (2x 512x512 Blit)      19.0 µs  989.3 µs   1008.3 µs   26.4 µs  812.9 µs    839.3 µs       1.4x Nisaba          1.20x Skia
63. 45° Rotated Pattern Fill in Circle      22.1 µs  985.3 µs   1007.4 µs   37.3 µs  979.1 µs   1016.4 µs       1.7x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   22.2 µs 1232.3 µs   1254.5 µs   37.0 µs  964.8 µs   1001.8 µs       1.7x Nisaba          1.25x Skia
65. Multi-Avatar Grid (16 Avatars)           58.7 µs  959.5 µs   1018.2 µs   44.2 µs  953.6 µs    997.9 µs         1.3x Skia         Tie (~1.0x)
66. Texture Pattern on Star Path             24.8 µs  973.3 µs    998.2 µs  131.5 µs  879.9 µs   1011.5 µs       5.3x Nisaba         Tie (~1.0x)
67. 20px Curved Path Stroked with Texture    21.4 µs  963.1 µs    984.5 µs  176.8 µs 1094.6 µs   1271.4 µs       8.3x Nisaba        1.29x Nisaba
68. Minified Texture Quad (0.25x Downscale)   47.3 µs  980.3 µs   1027.6 µs   69.0 µs  928.6 µs    997.6 µs       1.5x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        24.3 µs 1144.1 µs   1168.4 µs   65.8 µs  940.7 µs   1006.4 µs       2.7x Nisaba          1.16x Skia
70. Pure Rotation Cluster (36 Spokes 10°)   31.4 µs  965.2 µs    996.6 µs   49.9 µs  948.8 µs    998.7 µs       1.6x Nisaba         Tie (~1.0x)
71. Pure Scale Zoom Progression (20 Rects)   29.1 µs  969.2 µs    998.4 µs   37.0 µs  952.6 µs    989.6 µs       1.3x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   28.7 µs  765.1 µs    793.8 µs   51.1 µs  948.5 µs    999.6 µs       1.8x Nisaba        1.26x Nisaba
73. Combined Affine Transforms (30 Shapes)   69.4 µs  957.3 µs   1026.7 µs   43.3 µs  955.0 µs    998.3 µs         1.6x Skia         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   23.7 µs  974.1 µs    997.8 µs   40.1 µs  958.0 µs    998.0 µs       1.7x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       25.3 µs  971.0 µs    996.3 µs   56.6 µs  941.1 µs    997.7 µs       2.2x Nisaba         Tie (~1.0x)
76. Planetary Orbit Hierarchy (10 Systems)   40.9 µs  955.5 µs    996.4 µs   42.6 µs  936.7 µs    979.3 µs               Tie         Tie (~1.0x)
77. Multi-Scissor Grid (16 Viewports)        20.3 µs 1003.6 µs   1024.0 µs   55.6 µs  649.4 µs    705.0 µs       2.7x Nisaba          1.45x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   21.1 µs 1220.9 µs   1242.0 µs   41.3 µs 1034.3 µs   1075.7 µs       2.0x Nisaba          1.15x Skia
79. Scissored Stroked Waves (1000 Pts Clipped)   87.1 µs  922.9 µs   1010.0 µs  867.2 µs  947.8 µs   1815.0 µs      10.0x Nisaba        1.80x Nisaba
80. Scissored Gradient Card (Clipped)        65.7 µs 1031.1 µs   1096.8 µs   79.3 µs  593.1 µs    672.4 µs       1.2x Nisaba          1.63x Skia
81. Circular Overflow Clip (25 Boxes)        25.4 µs 1003.7 µs   1029.2 µs   69.8 µs  513.6 µs    583.4 µs       2.7x Nisaba          1.76x Skia
82. Scissor Invalidation / Reset (25 Cycles)   20.4 µs  977.8 µs    998.2 µs   48.4 µs  966.5 µs   1015.0 µs       2.4x Nisaba         Tie (~1.0x)
83. Composite Op: Source-Over (30 Shapes)    17.4 µs 1000.1 µs   1017.5 µs   41.9 µs  954.6 µs    996.5 µs       2.4x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   20.8 µs 1036.0 µs   1056.8 µs   43.8 µs  952.3 µs    996.1 µs       2.1x Nisaba          1.06x Skia
85. Composite Op: Source-In (Alpha Masking)   18.0 µs  979.8 µs    997.8 µs   42.7 µs 1291.5 µs   1334.2 µs       2.4x Nisaba        1.34x Nisaba
86. Composite Op: Source-Out (Cutout)        19.5 µs  733.3 µs    752.8 µs   43.9 µs 1245.5 µs   1289.4 µs       2.3x Nisaba        1.71x Nisaba
87. Composite Op: Atop (Target Bounds)       19.7 µs 1017.2 µs   1036.9 µs   43.7 µs  953.4 µs    997.0 µs       2.2x Nisaba         Tie (~1.0x)
88. Composite Op: Dest-Over (Under-Drawing)   20.7 µs  995.7 µs   1016.3 µs   42.8 µs  956.1 µs    999.0 µs       2.1x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     19.9 µs 1006.8 µs   1026.8 µs   44.3 µs  954.0 µs    998.3 µs       2.2x Nisaba         Tie (~1.0x)
90. Composite Op: Dest-Atop (Inverse)        16.8 µs 1000.5 µs   1017.2 µs   45.6 µs 1281.3 µs   1327.0 µs       2.7x Nisaba        1.30x Nisaba
91. Composite Op: Xor (Exclusive Blend)      19.0 µs  998.5 µs   1017.5 µs   43.8 µs  974.2 µs   1018.0 µs       2.3x Nisaba         Tie (~1.0x)
92. Composite Op: Copy (Direct Overwrite)    19.9 µs  979.1 µs    999.0 µs   39.2 µs  957.7 µs    996.8 µs       2.0x Nisaba         Tie (~1.0x)
93. UI Dashboard Gauge (Arc Track + Needle)   27.1 µs  968.8 µs    995.9 µs  190.1 µs 1046.7 µs   1236.7 µs       7.0x Nisaba        1.24x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     104.9 µs 1157.8 µs   1262.8 µs  268.9 µs 1072.5 µs   1341.4 µs       2.6x Nisaba        1.06x Nisaba
95. Circular Progress Rings (8 Meters)       53.8 µs  942.3 µs    996.1 µs  368.0 µs 1144.6 µs   1512.6 µs       6.8x Nisaba        1.52x Nisaba
96. Modern Card Stack (5 Elevated Cards)     31.5 µs 1935.5 µs   1967.0 µs   79.1 µs 1638.2 µs   1717.3 µs       2.5x Nisaba          1.15x Skia
97. CAD Cross-Hatch Pattern (80 Angled Lines)   36.8 µs  970.5 µs   1007.3 µs   41.4 µs 1166.8 µs   1208.2 µs       1.1x Nisaba        1.20x Nisaba
98. Floating Action Button (FAB 10 Buttons)   53.6 µs 1005.4 µs   1059.0 µs  117.0 µs  881.5 µs    998.6 µs       2.2x Nisaba          1.06x Skia
99. Anti-Aliasing Geometry Grid (50 Diamonds)   40.4 µs  978.9 µs   1019.2 µs  112.1 µs  885.6 µs    997.7 µs       2.8x Nisaba         Tie (~1.0x)
100. Master Vector Stress: Mixed Mega-Scene   66.7 µs 1584.4 µs   1651.0 µs   70.8 µs 1176.8 µs   1247.6 µs       1.1x Nisaba          1.32x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          4047.4 µs108547.9 µs 112595.3 µs18797.7 µs104430.1 µs 123227.8 µs      4.64x Nisaba        1.09x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 37 suites | Skia won in 34 suites | Ties: 29
         Overall Frame Throughput (End-to-End): 1.09x Nisaba
         Overall CPU Command Submission:       4.64x Nisaba
===================================================================================================================================================
```
</details>

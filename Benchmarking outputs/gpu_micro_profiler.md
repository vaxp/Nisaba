# Benchmark Report: Nisaba GPU Engine vs Google Skia (Ganesh GL)

**Execution Date:** 2026-10-17 11:28:05  
**Runs Executed:** 10 consecutive runs  
**Hardware Platform:** Intel Core i7-8565U (x86_64, Linux)  

---

## Executive Summary & Aggregate Averages

Across **10 consecutive executions** of the 100-suite GPU micro-profiling benchmark:
- **Overall CPU Command Submission Overhead:** **7.31x FASTER (Nisaba)**
  * Nisaba Mean CPU Overhead: `7721.4 µs` (7.72 ms)
  * Skia Mean CPU Overhead: `56475.1 µs` (56.48 ms)
- **Overall End-to-End Frame Throughput:** **1.71x FASTER (Nisaba)**
  * Nisaba Mean Total Frame Time: `104052.9 µs` (104.05 ms)
  * Skia Mean Total Frame Time: `177654.0 µs` (177.65 ms)

---

## 10-Run Mean Results Across All 100 GPU Suites

| # | Suite Name | Nisaba CPU (µs) | Nisaba GPU (µs) | Nisaba Total (µs) | Skia CPU (µs) | Skia GPU (µs) | Skia Total (µs) | CPU Ratio | Total Ratio |
| :---: | :--- | :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| 1 | Solid Background Clear (1080x720) | **4.6** | 581.2 | **585.9** | 21.3 | 547.0 | 568.3 | **4.6x Nisaba** | Tie (~1.0x) |
| 2 | 100 Grid Lines (1px Hairlines) | **38.0** | 698.4 | **736.3** | 130.3 | 1027.3 | 1157.6 | **3.4x Nisaba** | **1.57x Nisaba** |
| 3 | 20 Thick Diagonal Lines (16px Round) | **75.3** | 855.7 | **931.0** | 791.6 | 1307.5 | 2099.1 | **10.5x Nisaba** | **2.25x Nisaba** |
| 4 | 100 Solid Rectangles (Varied Bounds) | **78.5** | 866.6 | **945.1** | 207.6 | 933.0 | 1140.6 | **2.6x Nisaba** | **1.21x Nisaba** |
| 5 | 100 Alpha Rectangles (Overlapping Blend) | **67.9** | 1064.0 | **1131.9** | 235.0 | 1130.3 | 1365.2 | **3.5x Nisaba** | **1.21x Nisaba** |
| 6 | 50 Simple Rounded Rects (12px Radius) | **50.0** | 1059.1 | **1109.1** | 192.7 | 1115.4 | 1308.1 | **3.9x Nisaba** | **1.18x Nisaba** |
| 7 | 50 Varying-Radius Rounded Rects | **53.6** | 1054.8 | **1108.4** | 151.7 | 1030.8 | 1182.5 | **2.8x Nisaba** | **1.07x Nisaba** |
| 8 | 50 Stroked Rounded Rects (1.5px Border) | **69.1** | 919.9 | **989.0** | 226.9 | 1144.4 | 1371.4 | **3.3x Nisaba** | **1.39x Nisaba** |
| 9 | 100 Alpha Circles / Disks (Fill) | **79.4** | 957.0 | **1036.4** | 303.6 | 1201.1 | 1504.6 | **3.8x Nisaba** | **1.45x Nisaba** |
| 10 | 25 Concentric Rings (2px Stroke) | **135.3** | 1028.0 | **1163.3** | 162.0 | 1146.5 | 1308.6 | **1.2x Nisaba** | **1.12x Nisaba** |
| 11 | 50 Two-Stop Linear Gradients | **73.2** | 1271.9 | **1345.1** | 882.2 | 1570.8 | 2453.0 | **12.0x Nisaba** | **1.82x Nisaba** |
| 12 | 50 Diagonal Gradients (45° Angle) | **64.9** | 1176.7 | **1241.7** | 873.1 | 1487.4 | 2360.5 | **13.4x Nisaba** | **1.90x Nisaba** |
| 13 | 25 Radial Glow Gradients (300px) | **63.8** | 2677.4 | **2741.1** | 508.5 | 2118.9 | 2627.4 | **8.0x Nisaba** | Tie (~1.0x) |
| 14 | 20 Box Blur / Drop Shadows (15px) | **36.4** | 1098.5 | **1134.9** | 362.8 | 1289.8 | 1652.6 | **10.0x Nisaba** | **1.46x Nisaba** |
| 15 | 10 Concave 10-Point Stars (Fill) | **152.6** | 1265.1 | **1417.7** | 1450.0 | 1492.1 | 2942.1 | **9.5x Nisaba** | **2.08x Nisaba** |
| 16 | 10 Concave 10-Point Stars (3px Stroke) | **84.7** | 819.2 | **903.9** | 2036.2 | 1545.2 | 3581.5 | **24.0x Nisaba** | **3.96x Nisaba** |
| 17 | 1000-Point Waveform (Dense Stroke) | **179.0** | 950.0 | **1129.0** | 1811.0 | 1569.3 | 3380.3 | **10.1x Nisaba** | **2.99x Nisaba** |
| 18 | 50 Rotated Cards (Affine Transforms) | **47.0** | 899.9 | **946.9** | 243.9 | 1203.9 | 1447.8 | **5.2x Nisaba** | **1.53x Nisaba** |
| 19 | 25 Scissor Clipped Viewports | **37.9** | 839.8 | **877.7** | 175.7 | 762.3 | 938.1 | **4.6x Nisaba** | **1.07x Nisaba** |
| 20 | 50 UI Text Labels (Inter-Regular) | **288.3** | 1048.4 | **1336.7** | 277.6 | 1056.7 | 1334.3 | Tie (~1.0x) | Tie (~1.0x) |
| 21 | 25 Quadratic Bezier Curves (Fill+Stroke) | **236.3** | 1250.6 | **1487.0** | 1046.0 | 1472.6 | 2518.6 | **4.4x Nisaba** | **1.69x Nisaba** |
| 22 | 20 Cubic Bezier Splines (4px Stroke) | **69.4** | 770.7 | **840.1** | 2374.3 | 1666.9 | 4041.2 | **34.2x Nisaba** | **4.81x Nisaba** |
| 23 | Complex Bezier Ribbon (24px Alpha) | **29.6** | 760.2 | **789.9** | 589.8 | 1278.0 | 1867.8 | **19.9x Nisaba** | **2.36x Nisaba** |
| 24 | Circular Arcs with arcTo (20 Corners) | **77.6** | 815.4 | **893.1** | 1351.1 | 1518.2 | 2869.4 | **17.4x Nisaba** | **3.21x Nisaba** |
| 25 | Directional Arcs with arc() (30 Arcs) | **149.0** | 969.9 | **1118.9** | 437.0 | 1210.7 | 1647.8 | **2.9x Nisaba** | **1.47x Nisaba** |
| 26 | Compound Path with Holes (15 Donuts) | **259.2** | 1381.7 | **1640.9** | 1407.9 | 1422.9 | 2830.8 | **5.4x Nisaba** | **1.73x Nisaba** |
| 27 | Trefoil Figure-8 Cubic Knot (8 Loops) | **93.9** | 842.9 | **936.8** | 1233.2 | 1416.8 | 2650.1 | **13.1x Nisaba** | **2.83x Nisaba** |
| 28 | Archimedean Spiral Polyline (500 Pts) | **102.4** | 819.7 | **922.1** | 2049.3 | 1574.8 | 3624.1 | **20.0x Nisaba** | **3.93x Nisaba** |
| 29 | Multi-Contour Disjoint Paths (25 Sub-Paths) | **21.0** | 699.9 | **720.9** | 915.7 | 1386.8 | 2302.5 | **43.6x Nisaba** | **3.19x Nisaba** |
| 30 | Sharp Zigzag Mountain (50 Vertices) | **67.0** | 1046.0 | **1112.9** | 2553.7 | 1673.5 | 4227.1 | **38.1x Nisaba** | **3.80x Nisaba** |
| 31 | Stroke Join: Miter (12px Stroke) | **20.1** | 682.3 | **702.5** | 1013.5 | 1346.6 | 2360.1 | **50.4x Nisaba** | **3.36x Nisaba** |
| 32 | Stroke Join: Bevel (12px Stroke) | **23.6** | 728.2 | **751.9** | 1207.2 | 1403.3 | 2610.6 | **51.1x Nisaba** | **3.47x Nisaba** |
| 33 | Stroke Join: Round (12px Stroke) | **32.8** | 716.4 | **749.1** | 1256.9 | 1426.5 | 2683.4 | **38.4x Nisaba** | **3.58x Nisaba** |
| 34 | Stroke Cap: Butt (30 Segments 12px) | **29.2** | 681.8 | **711.0** | 130.3 | 954.7 | 1084.9 | **4.5x Nisaba** | **1.53x Nisaba** |
| 35 | Stroke Cap: Square (30 Segments 12px) | **38.9** | 799.8 | **838.7** | 146.3 | 987.4 | 1133.6 | **3.8x Nisaba** | **1.35x Nisaba** |
| 36 | Stroke Cap: Round (30 Segments 12px) | **123.2** | 996.4 | **1119.6** | 229.9 | 1127.2 | 1357.1 | **1.9x Nisaba** | **1.21x Nisaba** |
| 37 | Miter Limit Clamping (10° Acute Spikes) | **46.8** | 826.1 | **873.0** | 1979.2 | 1542.4 | 3521.6 | **42.3x Nisaba** | **4.03x Nisaba** |
| 38 | Sub-Pixel Hairline Strokes (0.25px Lines) | **42.8** | 755.0 | **797.8** | 194.6 | 1002.5 | 1197.1 | **4.5x Nisaba** | **1.50x Nisaba** |
| 39 | Ultra-Heavy Geometric Ribbon (40px Wide) | **28.3** | 758.6 | **786.9** | 587.5 | 1297.2 | 1884.7 | **20.8x Nisaba** | **2.40x Nisaba** |
| 40 | Stepped Variable Stroke Widths (1-20px) | **83.7** | 914.6 | **998.2** | 1533.3 | 1491.2 | 3024.6 | **18.3x Nisaba** | **3.03x Nisaba** |
| 41 | 50 Ellipses (rx=60, ry=25 Fill) | **29.6** | 758.0 | **787.6** | 118.0 | 918.2 | 1036.2 | **4.0x Nisaba** | **1.32x Nisaba** |
| 42 | 50 Stroked Ellipses (2.5px Border) | **48.2** | 1075.7 | **1123.9** | 138.5 | 1034.7 | 1173.2 | **2.9x Nisaba** | Tie (~1.0x) |
| 43 | 50 Pill / Capsule Badges (r=h/2) | **64.8** | 953.0 | **1017.7** | 190.6 | 1043.7 | 1234.3 | **2.9x Nisaba** | **1.21x Nisaba** |
| 44 | 50 Asymmetric Teardrop Rounded Rects | **53.5** | 1009.5 | **1063.0** | 165.6 | 1030.5 | 1196.1 | **3.1x Nisaba** | **1.13x Nisaba** |
| 45 | High-Density Disks Cloud (500 Disks) | **223.7** | 1123.0 | **1346.7** | 793.8 | 1379.6 | 2173.4 | **3.5x Nisaba** | **1.61x Nisaba** |
| 46 | High-Density Stroked Rings (250 Rings) | **104.2** | 1018.6 | **1122.8** | 527.9 | 1296.0 | 1823.9 | **5.1x Nisaba** | **1.62x Nisaba** |
| 47 | 250 Filled Rectangles Batch | **131.9** | 1026.8 | **1158.8** | 484.4 | 1253.3 | 1737.7 | **3.7x Nisaba** | **1.50x Nisaba** |
| 48 | 150 Stroked Rectangles (2px Border) | **106.0** | 882.1 | **988.1** | 298.2 | 1141.1 | 1439.3 | **2.8x Nisaba** | **1.46x Nisaba** |
| 49 | 40 Crosshair Aim Reticles | **265.5** | 1113.1 | **1378.6** | 989.2 | 1431.3 | 2420.5 | **3.7x Nisaba** | **1.76x Nisaba** |
| 50 | 25 Regular Octagons (Fill + Stroke) | **257.5** | 1366.6 | **1624.0** | 1042.3 | 1471.0 | 2513.3 | **4.0x Nisaba** | **1.55x Nisaba** |
| 51 | 30 Vertical Card Linear Gradients | **42.6** | 1043.8 | **1086.4** | 524.3 | 1229.8 | 1754.0 | **12.3x Nisaba** | **1.61x Nisaba** |
| 52 | 30 Horizontal Bar Progress Gradients | **42.2** | 1051.3 | **1093.5** | 553.3 | 1323.6 | 1876.9 | **13.1x Nisaba** | **1.72x Nisaba** |
| 53 | 15 Concentric Radial Spotlights | **33.4** | 955.4 | **988.7** | 325.1 | 1192.0 | 1517.1 | **9.7x Nisaba** | **1.53x Nisaba** |
| 54 | 25 Soft Ambient Card Shadows | **47.5** | 1100.3 | **1147.8** | 565.5 | 1428.7 | 1994.2 | **11.9x Nisaba** | **1.74x Nisaba** |
| 55 | 15 Neon Button Glows (Cyan Intense) | **39.1** | 967.5 | **1006.6** | 377.9 | 1340.8 | 1718.7 | **9.7x Nisaba** | **1.71x Nisaba** |
| 56 | 16 Multi-Angle Gradient Fan Slices | **108.9** | 1072.0 | **1180.9** | 455.4 | 1280.2 | 1735.7 | **4.2x Nisaba** | **1.47x Nisaba** |
| 57 | Full-Screen Diagonal Horizon Gradient | **31.1** | 958.0 | **989.1** | 115.7 | 929.8 | 1045.5 | **3.7x Nisaba** | **1.06x Nisaba** |
| 58 | 20 Inset Well Shadows (Recessed) | **203.3** | 1348.0 | **1551.3** | 482.2 | 1419.4 | 1901.5 | **2.4x Nisaba** | **1.23x Nisaba** |
| 59 | 10 Pulsing Circular Radar Wave Glows | **51.2** | 1601.5 | **1652.7** | 287.2 | 1517.6 | 1804.8 | **5.6x Nisaba** | **1.09x Nisaba** |
| 60 | Waveform Polyline Stroked with Gradient | **164.7** | 1054.6 | **1219.3** | 1714.5 | 1504.2 | 3218.8 | **10.4x Nisaba** | **2.64x Nisaba** |
| 61 | Repeated Texture Tiling (64x64 Pattern) | **21.1** | 821.5 | **842.5** | 98.9 | 890.2 | 989.1 | **4.7x Nisaba** | **1.17x Nisaba** |
| 62 | Scaled Image Quad (2x 512x512 Blit) | **26.7** | 835.3 | **862.0** | 89.1 | 871.2 | 960.3 | **3.3x Nisaba** | **1.11x Nisaba** |
| 63 | 45° Rotated Pattern Fill in Circle | **26.7** | 931.5 | **958.3** | 109.1 | 882.1 | 991.2 | **4.1x Nisaba** | Tie (~1.0x) |
| 64 | Translucent Texture Overlay (50% Alpha) | **32.7** | 950.5 | **983.2** | 113.2 | 958.8 | 1072.0 | **3.5x Nisaba** | **1.09x Nisaba** |
| 65 | Multi-Avatar Grid (16 Avatars) | **33.8** | 829.3 | **863.1** | 134.8 | 964.5 | 1099.3 | **4.0x Nisaba** | **1.27x Nisaba** |
| 66 | Texture Pattern on Star Path | **51.5** | 889.1 | **940.6** | 467.6 | 1189.2 | 1656.8 | **9.1x Nisaba** | **1.76x Nisaba** |
| 67 | 20px Curved Path Stroked with Texture | **37.8** | 845.5 | **883.3** | 697.2 | 1364.5 | 2061.7 | **18.4x Nisaba** | **2.33x Nisaba** |
| 68 | Minified Texture Quad (0.25x Downscale) | **59.7** | 895.5 | **955.2** | 243.5 | 1072.5 | 1316.0 | **4.1x Nisaba** | **1.38x Nisaba** |
| 69 | Pure Translation Stack (50 Cards) | **80.2** | 959.3 | **1039.5** | 267.1 | 1220.9 | 1487.9 | **3.3x Nisaba** | **1.43x Nisaba** |
| 70 | Pure Rotation Cluster (36 Spokes 10°) | **54.6** | 857.5 | **912.1** | 198.8 | 1072.0 | 1270.8 | **3.6x Nisaba** | **1.39x Nisaba** |
| 71 | Pure Scale Zoom Progression (20 Rects) | **80.9** | 996.2 | **1077.1** | 123.4 | 955.3 | 1078.7 | **1.5x Nisaba** | Tie (~1.0x) |
| 72 | Shear / Skew Parallelograms (30 Quads) | **48.0** | 846.4 | **894.4** | 187.3 | 1055.3 | 1242.7 | **3.9x Nisaba** | **1.39x Nisaba** |
| 73 | Combined Affine Transforms (30 Shapes) | **53.0** | 890.6 | **943.6** | 163.5 | 1066.4 | 1229.9 | **3.1x Nisaba** | **1.30x Nisaba** |
| 74 | Deep Hierarchical State Stack (20 Levels) | **49.9** | 917.6 | **967.5** | 166.6 | 1076.1 | 1242.7 | **3.3x Nisaba** | **1.28x Nisaba** |
| 75 | Reset Transform Stress (50 Cycles) | **64.4** | 905.5 | **969.9** | 220.7 | 1098.0 | 1318.7 | **3.4x Nisaba** | **1.36x Nisaba** |
| 76 | Planetary Orbit Hierarchy (10 Systems) | **43.7** | 843.5 | **887.1** | 143.6 | 952.9 | 1096.5 | **3.3x Nisaba** | **1.24x Nisaba** |
| 77 | Multi-Scissor Grid (16 Viewports) | **42.0** | 849.0 | **891.1** | 233.1 | 912.7 | 1145.8 | **5.5x Nisaba** | **1.29x Nisaba** |
| 78 | Hierarchical Intersecting Scissors (4 Levels) | **47.0** | 978.5 | **1025.5** | 164.1 | 1186.9 | 1351.0 | **3.5x Nisaba** | **1.32x Nisaba** |
| 79 | Scissored Stroked Waves (1000 Pts Clipped) | **340.0** | 1195.1 | **1535.1** | 2282.8 | 1464.6 | 3747.3 | **6.7x Nisaba** | **2.44x Nisaba** |
| 80 | Scissored Gradient Card (Clipped) | **27.9** | 896.8 | **924.7** | 307.2 | 968.4 | 1275.6 | **11.0x Nisaba** | **1.38x Nisaba** |
| 81 | Circular Overflow Clip (25 Boxes) | **39.7** | 800.1 | **839.9** | 279.8 | 902.9 | 1182.7 | **7.0x Nisaba** | **1.41x Nisaba** |
| 82 | Scissor Invalidation / Reset (25 Cycles) | **45.4** | 865.1 | **910.5** | 158.0 | 984.0 | 1142.1 | **3.5x Nisaba** | **1.25x Nisaba** |
| 83 | Composite Op: Source-Over (30 Shapes) | **43.4** | 828.1 | **871.4** | 120.2 | 859.0 | 979.2 | **2.8x Nisaba** | **1.12x Nisaba** |
| 84 | Composite Op: Lighter / Plus (30 Particles) | **37.7** | 816.7 | **854.4** | 153.9 | 998.2 | 1152.1 | **4.1x Nisaba** | **1.35x Nisaba** |
| 85 | Composite Op: Source-In (Alpha Masking) | **42.5** | 859.5 | **902.0** | 149.4 | 1171.3 | 1320.7 | **3.5x Nisaba** | **1.46x Nisaba** |
| 86 | Composite Op: Source-Out (Cutout) | **40.0** | 797.3 | **837.2** | 163.1 | 1197.7 | 1360.9 | **4.1x Nisaba** | **1.63x Nisaba** |
| 87 | Composite Op: Atop (Target Bounds) | **41.3** | 833.4 | **874.8** | 146.1 | 960.2 | 1106.3 | **3.5x Nisaba** | **1.26x Nisaba** |
| 88 | Composite Op: Dest-Over (Under-Drawing) | **41.8** | 894.3 | **936.2** | 153.1 | 1014.2 | 1167.3 | **3.7x Nisaba** | **1.25x Nisaba** |
| 89 | Composite Op: Dest-Out (Eraser Mask) | **38.7** | 839.3 | **877.9** | 156.2 | 1022.5 | 1178.8 | **4.0x Nisaba** | **1.34x Nisaba** |
| 90 | Composite Op: Dest-Atop (Inverse) | **43.0** | 831.9 | **875.0** | 164.8 | 1157.6 | 1322.4 | **3.8x Nisaba** | **1.51x Nisaba** |
| 91 | Composite Op: Xor (Exclusive Blend) | **39.3** | 811.5 | **850.9** | 158.3 | 1007.2 | 1165.5 | **4.0x Nisaba** | **1.37x Nisaba** |
| 92 | Composite Op: Copy (Direct Overwrite) | **45.9** | 863.0 | **908.9** | 152.3 | 1018.0 | 1170.4 | **3.3x Nisaba** | **1.29x Nisaba** |
| 93 | UI Dashboard Gauge (Arc Track + Needle) | **80.4** | 976.8 | **1057.3** | 866.1 | 1416.7 | 2282.8 | **10.8x Nisaba** | **2.16x Nisaba** |
| 94 | Audio Spectrum Visualizer (64 Bars) | **112.6** | 1212.6 | **1325.2** | 1034.7 | 1497.2 | 2531.9 | **9.2x Nisaba** | **1.91x Nisaba** |
| 95 | Circular Progress Rings (8 Meters) | **117.1** | 878.9 | **996.0** | 1259.3 | 1438.7 | 2698.1 | **10.8x Nisaba** | **2.71x Nisaba** |
| 96 | Modern Card Stack (5 Elevated Cards) | **53.7** | 1338.1 | **1391.7** | 349.8 | 1500.1 | 1849.9 | **6.5x Nisaba** | **1.33x Nisaba** |
| 97 | CAD Cross-Hatch Pattern (80 Angled Lines) | **69.1** | 837.3 | **906.4** | 133.1 | 1049.0 | 1182.1 | **1.9x Nisaba** | **1.30x Nisaba** |
| 98 | Floating Action Button (FAB 10 Buttons) | **118.6** | 1146.0 | **1264.6** | 422.3 | 1301.3 | 1723.6 | **3.6x Nisaba** | **1.36x Nisaba** |
| 99 | Anti-Aliasing Geometry Grid (50 Diamonds) | **84.2** | 834.0 | **918.2** | 344.9 | 1157.3 | 1502.1 | **4.1x Nisaba** | **1.64x Nisaba** |
| 100 | Master Vector Stress: Mixed Mega-Scene | **61.9** | 1106.9 | **1168.8** | 267.5 | 1258.1 | 1525.6 | **4.3x Nisaba** | **1.31x Nisaba** |
| **--** | **TOTAL FRAME OVERHEAD** | **7721.4** | **96331.6** | **104052.9** | **56475.1** | **121178.9** | **177654.0** | **7.31x Nisaba** | **1.71x Nisaba** |

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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 698.9 µs, Skia: 504.4 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 655.7 µs, Skia: 981.1 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 920.9 µs, Skia: 2366.0 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 975.5 µs, Skia: 1081.0 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1157.2 µs, Skia: 1876.5 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1088.4 µs, Skia: 1191.7 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 870.2 µs, Skia: 963.4 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 589.4 µs, Skia: 1430.7 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1201.3 µs, Skia: 1735.5 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1309.6 µs, Skia: 1301.9 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1254.5 µs, Skia: 2994.0 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1488.3 µs, Skia: 2821.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2650.3 µs, Skia: 2598.1 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 995.4 µs, Skia: 1616.3 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1140.8 µs, Skia: 2581.7 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 736.7 µs, Skia: 3385.2 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 860.4 µs, Skia: 3065.9 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 839.2 µs, Skia: 1198.8 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 825.1 µs, Skia: 1052.0 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1452.5 µs, Skia: 1305.9 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1688.6 µs, Skia: 2516.7 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 664.2 µs, Skia: 3800.8 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 692.6 µs, Skia: 1598.9 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 766.4 µs, Skia: 2706.5 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 883.4 µs, Skia: 1620.6 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1657.6 µs, Skia: 2228.2 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 870.4 µs, Skia: 2350.6 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 759.4 µs, Skia: 3439.9 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 703.2 µs, Skia: 2141.4 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1059.7 µs, Skia: 3865.3 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 765.6 µs, Skia: 2129.6 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 872.7 µs, Skia: 3906.3 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 972.2 µs, Skia: 4042.4 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 931.9 µs, Skia: 1509.3 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 976.9 µs, Skia: 1476.5 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1306.9 µs, Skia: 1760.8 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 960.6 µs, Skia: 5190.6 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 741.4 µs, Skia: 1342.1 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 775.5 µs, Skia: 1803.2 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 811.0 µs, Skia: 2692.9 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 851.6 µs, Skia: 1166.5 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1187.6 µs, Skia: 1011.5 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 944.2 µs, Skia: 926.7 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1003.2 µs, Skia: 1059.0 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1305.7 µs, Skia: 1575.6 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 944.1 µs, Skia: 1417.0 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1079.6 µs, Skia: 1750.3 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 830.2 µs, Skia: 1182.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1060.3 µs, Skia: 2028.9 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1426.2 µs, Skia: 1888.9 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 902.2 µs, Skia: 1389.3 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1002.5 µs, Skia: 1333.2 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 906.9 µs, Skia: 1112.8 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 832.0 µs, Skia: 1755.7 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 887.9 µs, Skia: 1380.4 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 917.9 µs, Skia: 1418.2 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 882.2 µs, Skia: 900.6 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1408.2 µs, Skia: 1573.8 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1515.9 µs, Skia: 1552.7 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 938.4 µs, Skia: 2780.5 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 808.5 µs, Skia: 1081.3 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 758.0 µs, Skia: 927.0 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1069.8 µs, Skia: 1173.6 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 893.9 µs, Skia: 949.3 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 879.7 µs, Skia: 1231.3 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 837.8 µs, Skia: 1579.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 777.5 µs, Skia: 1735.0 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 752.9 µs, Skia: 1327.5 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1010.8 µs, Skia: 1469.8 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 786.7 µs, Skia: 1189.8 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1155.7 µs, Skia: 953.2 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 829.3 µs, Skia: 1353.2 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 686.4 µs, Skia: 803.1 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 1007.5 µs, Skia: 1055.3 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 924.5 µs, Skia: 1047.3 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 799.4 µs, Skia: 992.8 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 832.0 µs, Skia: 1052.0 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 895.3 µs, Skia: 1112.8 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1291.3 µs, Skia: 3022.5 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 963.4 µs, Skia: 1554.6 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 1050.3 µs, Skia: 1527.9 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 956.1 µs, Skia: 1057.1 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 610.4 µs, Skia: 772.2 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 686.7 µs, Skia: 915.4 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 738.0 µs, Skia: 1550.2 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 630.2 µs, Skia: 982.0 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 611.5 µs, Skia: 898.8 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1002.2 µs, Skia: 1154.6 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 878.8 µs, Skia: 1222.2 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 851.9 µs, Skia: 1293.9 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 934.6 µs, Skia: 1140.3 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 903.5 µs, Skia: 1168.9 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1096.2 µs, Skia: 1736.7 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1193.1 µs, Skia: 2088.2 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 910.1 µs, Skia: 2330.6 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1575.0 µs, Skia: 1718.5 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 969.8 µs, Skia: 1152.5 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1370.9 µs, Skia: 1728.4 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 924.2 µs, Skia: 1600.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1162.8 µs, Skia: 1338.3 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          5.5 µs  693.4 µs    698.9 µs   15.5 µs  489.0 µs    504.4 µs       2.8x Nisaba          1.39x Skia
2. 100 Grid Lines (1px Hairlines)            22.6 µs  633.1 µs    655.7 µs   97.6 µs  883.5 µs    981.1 µs       4.3x Nisaba        1.50x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      76.8 µs  844.1 µs    920.9 µs  888.8 µs 1477.3 µs   2366.0 µs      11.6x Nisaba        2.57x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      87.3 µs  888.2 µs    975.5 µs  216.6 µs  864.4 µs   1081.0 µs       2.5x Nisaba        1.11x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   77.0 µs 1080.2 µs   1157.2 µs  403.7 µs 1472.8 µs   1876.5 µs       5.2x Nisaba        1.62x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     44.8 µs 1043.7 µs   1088.4 µs  166.5 µs 1025.2 µs   1191.7 µs       3.7x Nisaba        1.09x Nisaba
7. 50 Varying-Radius Rounded Rects           36.2 µs  834.1 µs    870.2 µs   87.9 µs  875.5 µs    963.4 µs       2.4x Nisaba        1.11x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   31.8 µs  557.5 µs    589.4 µs  276.2 µs 1154.5 µs   1430.7 µs       8.7x Nisaba        2.43x Nisaba
9. 100 Alpha Circles / Disks (Fill)         110.7 µs 1090.6 µs   1201.3 µs  401.4 µs 1334.1 µs   1735.5 µs       3.6x Nisaba        1.44x Nisaba
10. 25 Concentric Rings (2px Stroke)        173.9 µs 1135.7 µs   1309.6 µs  158.9 µs 1143.0 µs   1301.9 µs         1.1x Skia         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             81.7 µs 1172.8 µs   1254.5 µs 1184.1 µs 1809.9 µs   2994.0 µs      14.5x Nisaba        2.39x Nisaba
12. 50 Diagonal Gradients (45° Angle)       87.3 µs 1401.0 µs   1488.3 µs 1185.8 µs 1636.1 µs   2821.9 µs      13.6x Nisaba        1.90x Nisaba
13. 25 Radial Glow Gradients (300px)         70.3 µs 2580.0 µs   2650.3 µs  556.0 µs 2042.0 µs   2598.1 µs       7.9x Nisaba         Tie (~1.0x)
14. 20 Box Blur / Drop Shadows (15px)        22.7 µs  972.7 µs    995.4 µs  338.5 µs 1277.8 µs   1616.3 µs      14.9x Nisaba        1.62x Nisaba
15. 10 Concave 10-Point Stars (Fill)         83.2 µs 1057.6 µs   1140.8 µs 1195.2 µs 1386.5 µs   2581.7 µs      14.4x Nisaba        2.26x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   61.9 µs  674.8 µs    736.7 µs 1814.5 µs 1570.7 µs   3385.2 µs      29.3x Nisaba        4.60x Nisaba
17. 1000-Point Waveform (Dense Stroke)      139.4 µs  721.0 µs    860.4 µs 1504.9 µs 1561.0 µs   3065.9 µs      10.8x Nisaba        3.56x Nisaba
18. 50 Rotated Cards (Affine Transforms)     36.2 µs  803.0 µs    839.2 µs  182.9 µs 1015.9 µs   1198.8 µs       5.1x Nisaba        1.43x Nisaba
19. 25 Scissor Clipped Viewports             33.6 µs  791.4 µs    825.1 µs  204.5 µs  847.4 µs   1052.0 µs       6.1x Nisaba        1.27x Nisaba
20. 50 UI Text Labels (Inter-Regular)       282.2 µs 1170.3 µs   1452.5 µs  287.8 µs 1018.1 µs   1305.9 µs               Tie          1.11x Skia
21. 25 Quadratic Bezier Curves (Fill+Stroke)  276.3 µs 1412.3 µs   1688.6 µs 1119.0 µs 1397.6 µs   2516.7 µs       4.0x Nisaba        1.49x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     50.8 µs  613.4 µs    664.2 µs 2142.7 µs 1658.1 µs   3800.8 µs      42.2x Nisaba        5.72x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       19.8 µs  672.8 µs    692.6 µs  450.7 µs 1148.2 µs   1598.9 µs      22.8x Nisaba        2.31x Nisaba
24. Circular Arcs with arcTo (20 Corners)    64.6 µs  701.8 µs    766.4 µs 1183.2 µs 1523.4 µs   2706.5 µs      18.3x Nisaba        3.53x Nisaba
25. Directional Arcs with arc() (30 Arcs)    77.7 µs  805.7 µs    883.4 µs  371.3 µs 1249.3 µs   1620.6 µs       4.8x Nisaba        1.83x Nisaba
26. Compound Path with Holes (15 Donuts)    231.8 µs 1425.8 µs   1657.6 µs  992.6 µs 1235.6 µs   2228.2 µs       4.3x Nisaba        1.34x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    95.0 µs  775.3 µs    870.4 µs  973.0 µs 1377.6 µs   2350.6 µs      10.2x Nisaba        2.70x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    68.3 µs  691.1 µs    759.4 µs 1822.8 µs 1617.1 µs   3439.9 µs      26.7x Nisaba        4.53x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   16.5 µs  686.7 µs    703.2 µs  860.2 µs 1281.3 µs   2141.4 µs      52.1x Nisaba        3.05x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      66.8 µs  992.9 µs   1059.7 µs 2265.4 µs 1599.9 µs   3865.3 µs      33.9x Nisaba        3.65x Nisaba
31. Stroke Join: Miter (12px Stroke)         21.7 µs  743.9 µs    765.6 µs  795.0 µs 1334.6 µs   2129.6 µs      36.6x Nisaba        2.78x Nisaba
32. Stroke Join: Bevel (12px Stroke)         33.0 µs  839.7 µs    872.7 µs 2232.7 µs 1673.6 µs   3906.3 µs      67.7x Nisaba        4.48x Nisaba
33. Stroke Join: Round (12px Stroke)         55.0 µs  917.2 µs    972.2 µs 2397.5 µs 1644.8 µs   4042.4 µs      43.6x Nisaba        4.16x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      53.8 µs  878.1 µs    931.9 µs  224.5 µs 1284.8 µs   1509.3 µs       4.2x Nisaba        1.62x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    52.5 µs  924.3 µs    976.9 µs  216.6 µs 1260.0 µs   1476.5 µs       4.1x Nisaba        1.51x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    161.3 µs 1145.6 µs   1306.9 µs  318.4 µs 1442.3 µs   1760.8 µs       2.0x Nisaba        1.35x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   65.4 µs  895.2 µs    960.6 µs 3529.1 µs 1661.5 µs   5190.6 µs      54.0x Nisaba        5.40x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   43.3 µs  698.1 µs    741.4 µs  185.5 µs 1156.6 µs   1342.1 µs       4.3x Nisaba        1.81x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   28.7 µs  746.8 µs    775.5 µs  547.9 µs 1255.3 µs   1803.2 µs      19.1x Nisaba        2.33x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   72.0 µs  739.0 µs    811.0 µs 1214.4 µs 1478.5 µs   2692.9 µs      16.9x Nisaba        3.32x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          34.0 µs  817.6 µs    851.6 µs  155.2 µs 1011.3 µs   1166.5 µs       4.6x Nisaba        1.37x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       37.7 µs 1149.8 µs   1187.6 µs  112.0 µs  899.5 µs   1011.5 µs       3.0x Nisaba          1.17x Skia
43. 50 Pill / Capsule Badges (r=h/2)         62.4 µs  881.8 µs    944.2 µs  120.4 µs  806.3 µs    926.7 µs       1.9x Nisaba         Tie (~1.0x)
44. 50 Asymmetric Teardrop Rounded Rects     53.4 µs  949.8 µs   1003.2 µs  134.7 µs  924.2 µs   1059.0 µs       2.5x Nisaba        1.06x Nisaba
45. High-Density Disks Cloud (500 Disks)    193.3 µs 1112.3 µs   1305.7 µs  445.4 µs 1130.2 µs   1575.6 µs       2.3x Nisaba        1.21x Nisaba
46. High-Density Stroked Rings (250 Rings)   60.6 µs  883.5 µs    944.1 µs  326.4 µs 1090.6 µs   1417.0 µs       5.4x Nisaba        1.50x Nisaba
47. 250 Filled Rectangles Batch              99.7 µs  979.9 µs   1079.6 µs  481.7 µs 1268.6 µs   1750.3 µs       4.8x Nisaba        1.62x Nisaba
48. 150 Stroked Rectangles (2px Border)     103.8 µs  726.5 µs    830.2 µs  208.6 µs  973.7 µs   1182.3 µs       2.0x Nisaba        1.42x Nisaba
49. 40 Crosshair Aim Reticles               155.4 µs  904.9 µs   1060.3 µs  655.6 µs 1373.3 µs   2028.9 µs       4.2x Nisaba        1.91x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     192.3 µs 1233.9 µs   1426.2 µs  555.0 µs 1333.9 µs   1888.9 µs       2.9x Nisaba        1.32x Nisaba
51. 30 Vertical Card Linear Gradients        24.3 µs  877.9 µs    902.2 µs  303.0 µs 1086.3 µs   1389.3 µs      12.5x Nisaba        1.54x Nisaba
52. 30 Horizontal Bar Progress Gradients     29.3 µs  973.2 µs   1002.5 µs  310.6 µs 1022.6 µs   1333.2 µs      10.6x Nisaba        1.33x Nisaba
53. 15 Concentric Radial Spotlights          24.6 µs  882.3 µs    906.9 µs  207.2 µs  905.7 µs   1112.8 µs       8.4x Nisaba        1.23x Nisaba
54. 25 Soft Ambient Card Shadows             22.4 µs  809.6 µs    832.0 µs  414.3 µs 1341.4 µs   1755.7 µs      18.5x Nisaba        2.11x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      31.2 µs  856.7 µs    887.9 µs  258.5 µs 1121.8 µs   1380.4 µs       8.3x Nisaba        1.55x Nisaba
56. 16 Multi-Angle Gradient Fan Slices       72.0 µs  845.9 µs    917.9 µs  318.7 µs 1099.5 µs   1418.2 µs       4.4x Nisaba        1.54x Nisaba
57. Full-Screen Diagonal Horizon Gradient    24.8 µs  857.4 µs    882.2 µs   87.4 µs  813.2 µs    900.6 µs       3.5x Nisaba         Tie (~1.0x)
58. 20 Inset Well Shadows (Recessed)        155.4 µs 1252.8 µs   1408.2 µs  368.4 µs 1205.4 µs   1573.8 µs       2.4x Nisaba        1.12x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     38.0 µs 1477.9 µs   1515.9 µs  203.0 µs 1349.7 µs   1552.7 µs       5.3x Nisaba         Tie (~1.0x)
60. Waveform Polyline Stroked with Gradient  127.2 µs  811.2 µs    938.4 µs 1360.7 µs 1419.8 µs   2780.5 µs      10.7x Nisaba        2.96x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   22.7 µs  785.8 µs    808.5 µs  113.8 µs  967.5 µs   1081.3 µs       5.0x Nisaba        1.34x Nisaba
62. Scaled Image Quad (2x 512x512 Blit)      22.1 µs  735.8 µs    758.0 µs   79.0 µs  848.0 µs    927.0 µs       3.6x Nisaba        1.22x Nisaba
63. 45° Rotated Pattern Fill in Circle      30.0 µs 1039.8 µs   1069.8 µs  126.5 µs 1047.1 µs   1173.6 µs       4.2x Nisaba        1.10x Nisaba
64. Translucent Texture Overlay (50% Alpha)   29.2 µs  864.6 µs    893.9 µs   79.0 µs  870.3 µs    949.3 µs       2.7x Nisaba        1.06x Nisaba
65. Multi-Avatar Grid (16 Avatars)           33.3 µs  846.5 µs    879.7 µs  159.3 µs 1072.0 µs   1231.3 µs       4.8x Nisaba        1.40x Nisaba
66. Texture Pattern on Star Path             33.3 µs  804.6 µs    837.8 µs  387.8 µs 1191.4 µs   1579.2 µs      11.7x Nisaba        1.88x Nisaba
67. 20px Curved Path Stroked with Texture    30.5 µs  747.0 µs    777.5 µs  486.3 µs 1248.8 µs   1735.0 µs      15.9x Nisaba        2.23x Nisaba
68. Minified Texture Quad (0.25x Downscale)   38.0 µs  714.9 µs    752.9 µs  217.5 µs 1110.0 µs   1327.5 µs       5.7x Nisaba        1.76x Nisaba
69. Pure Translation Stack (50 Cards)        81.0 µs  929.7 µs   1010.8 µs  234.9 µs 1234.9 µs   1469.8 µs       2.9x Nisaba        1.45x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   50.4 µs  736.3 µs    786.7 µs  188.8 µs 1001.0 µs   1189.8 µs       3.7x Nisaba        1.51x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   75.1 µs 1080.6 µs   1155.7 µs   82.1 µs  871.1 µs    953.2 µs       1.1x Nisaba          1.21x Skia
72. Shear / Skew Parallelograms (30 Quads)   43.5 µs  785.8 µs    829.3 µs  174.3 µs 1178.9 µs   1353.2 µs       4.0x Nisaba        1.63x Nisaba
73. Combined Affine Transforms (30 Shapes)   25.4 µs  661.0 µs    686.4 µs   99.8 µs  703.3 µs    803.1 µs       3.9x Nisaba        1.17x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   43.4 µs  964.1 µs   1007.5 µs  112.1 µs  943.3 µs   1055.3 µs       2.6x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       59.8 µs  864.7 µs    924.5 µs  104.6 µs  942.7 µs   1047.3 µs       1.7x Nisaba        1.13x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   34.9 µs  764.5 µs    799.4 µs  114.8 µs  878.0 µs    992.8 µs       3.3x Nisaba        1.24x Nisaba
77. Multi-Scissor Grid (16 Viewports)        31.8 µs  800.2 µs    832.0 µs  178.3 µs  873.7 µs   1052.0 µs       5.6x Nisaba        1.26x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   33.5 µs  861.9 µs    895.3 µs  112.2 µs 1000.6 µs   1112.8 µs       3.4x Nisaba        1.24x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  252.5 µs 1038.8 µs   1291.3 µs 1605.1 µs 1417.5 µs   3022.5 µs       6.4x Nisaba        2.34x Nisaba
80. Scissored Gradient Card (Clipped)        30.1 µs  933.2 µs    963.4 µs  445.4 µs 1109.2 µs   1554.6 µs      14.8x Nisaba        1.61x Nisaba
81. Circular Overflow Clip (25 Boxes)        55.4 µs  994.9 µs   1050.3 µs  373.9 µs 1154.0 µs   1527.9 µs       6.7x Nisaba        1.45x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   52.2 µs  903.9 µs    956.1 µs  136.1 µs  921.0 µs   1057.1 µs       2.6x Nisaba        1.11x Nisaba
83. Composite Op: Source-Over (30 Shapes)    21.7 µs  588.7 µs    610.4 µs   68.3 µs  703.9 µs    772.2 µs       3.2x Nisaba        1.27x Nisaba
84. Composite Op: Lighter / Plus (30 Particles)   21.9 µs  664.8 µs    686.7 µs   69.6 µs  845.9 µs    915.4 µs       3.2x Nisaba        1.33x Nisaba
85. Composite Op: Source-In (Alpha Masking)   31.6 µs  706.4 µs    738.0 µs  180.9 µs 1369.3 µs   1550.2 µs       5.7x Nisaba        2.10x Nisaba
86. Composite Op: Source-Out (Cutout)        30.6 µs  599.6 µs    630.2 µs   95.7 µs  886.3 µs    982.0 µs       3.1x Nisaba        1.56x Nisaba
87. Composite Op: Atop (Target Bounds)       18.9 µs  592.5 µs    611.5 µs   90.1 µs  808.7 µs    898.8 µs       4.8x Nisaba        1.47x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   38.4 µs  963.7 µs   1002.2 µs  157.2 µs  997.4 µs   1154.6 µs       4.1x Nisaba        1.15x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     42.3 µs  836.5 µs    878.8 µs  157.1 µs 1065.1 µs   1222.2 µs       3.7x Nisaba        1.39x Nisaba
90. Composite Op: Dest-Atop (Inverse)        41.1 µs  810.8 µs    851.9 µs  160.5 µs 1133.4 µs   1293.9 µs       3.9x Nisaba        1.52x Nisaba
91. Composite Op: Xor (Exclusive Blend)      40.2 µs  894.4 µs    934.6 µs  153.1 µs  987.2 µs   1140.3 µs       3.8x Nisaba        1.22x Nisaba
92. Composite Op: Copy (Direct Overwrite)    43.8 µs  859.7 µs    903.5 µs  157.1 µs 1011.9 µs   1168.9 µs       3.6x Nisaba        1.29x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   61.1 µs 1035.1 µs   1096.2 µs  499.8 µs 1236.9 µs   1736.7 µs       8.2x Nisaba        1.58x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      94.0 µs 1099.1 µs   1193.1 µs  673.3 µs 1414.9 µs   2088.2 µs       7.2x Nisaba        1.75x Nisaba
95. Circular Progress Rings (8 Meters)      118.1 µs  792.0 µs    910.1 µs 1029.6 µs 1300.9 µs   2330.6 µs       8.7x Nisaba        2.56x Nisaba
96. Modern Card Stack (5 Elevated Cards)     64.7 µs 1510.3 µs   1575.0 µs  276.1 µs 1442.4 µs   1718.5 µs       4.3x Nisaba        1.09x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   61.3 µs  908.5 µs    969.8 µs  131.1 µs 1021.4 µs   1152.5 µs       2.1x Nisaba        1.19x Nisaba
98. Floating Action Button (FAB 10 Buttons)  115.1 µs 1255.9 µs   1370.9 µs  381.0 µs 1347.4 µs   1728.4 µs       3.3x Nisaba        1.26x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   94.4 µs  829.8 µs    924.2 µs  363.3 µs 1236.8 µs   1600.1 µs       3.8x Nisaba        1.73x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   57.4 µs 1105.4 µs   1162.8 µs  213.8 µs 1124.5 µs   1338.3 µs       3.7x Nisaba        1.15x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          6766.3 µs91647.9 µs  98414.3 µs52506.1 µs117860.1 µs 170366.1 µs      7.76x Nisaba        1.73x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 90 suites | Skia won in 4 suites | Ties: 6
         Overall Frame Throughput (End-to-End): 1.73x Nisaba
         Overall CPU Command Submission:       7.76x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 562.4 µs, Skia: 498.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 743.0 µs, Skia: 1265.7 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 958.8 µs, Skia: 2178.0 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 863.3 µs, Skia: 893.9 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1016.0 µs, Skia: 1165.8 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1070.2 µs, Skia: 1187.8 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1027.1 µs, Skia: 843.2 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1026.7 µs, Skia: 1593.3 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1118.5 µs, Skia: 1686.5 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1134.1 µs, Skia: 1440.8 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1392.4 µs, Skia: 2747.5 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1254.5 µs, Skia: 2606.8 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2597.5 µs, Skia: 2838.9 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1172.9 µs, Skia: 1798.9 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1787.9 µs, Skia: 4065.8 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1276.2 µs, Skia: 3775.2 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1567.2 µs, Skia: 3485.0 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 798.2 µs, Skia: 1595.8 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 743.6 µs, Skia: 805.1 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1343.5 µs, Skia: 1527.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1245.4 µs, Skia: 2775.5 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 947.8 µs, Skia: 4076.9 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 978.4 µs, Skia: 2023.7 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1089.2 µs, Skia: 2907.0 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1091.9 µs, Skia: 1754.1 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1530.6 µs, Skia: 2990.6 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1172.3 µs, Skia: 2494.1 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1073.5 µs, Skia: 3749.4 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 889.2 µs, Skia: 2477.8 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1330.9 µs, Skia: 4666.4 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 703.8 µs, Skia: 2415.7 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 693.3 µs, Skia: 2474.3 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 644.3 µs, Skia: 2692.7 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 605.1 µs, Skia: 886.5 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 955.8 µs, Skia: 1191.7 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1268.0 µs, Skia: 1673.4 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 971.7 µs, Skia: 3465.4 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 980.4 µs, Skia: 1611.1 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 778.0 µs, Skia: 1568.6 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1131.0 µs, Skia: 2922.3 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 847.3 µs, Skia: 899.4 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1216.5 µs, Skia: 1084.1 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 843.1 µs, Skia: 1435.1 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1163.0 µs, Skia: 1465.7 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1582.0 µs, Skia: 1938.6 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1141.6 µs, Skia: 1841.3 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1370.1 µs, Skia: 2118.5 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 981.6 µs, Skia: 1365.7 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1344.2 µs, Skia: 2521.7 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1531.3 µs, Skia: 2692.0 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1134.2 µs, Skia: 2071.4 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 987.8 µs, Skia: 1817.4 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 960.8 µs, Skia: 1493.0 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1283.0 µs, Skia: 2141.4 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1083.3 µs, Skia: 1644.3 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1294.4 µs, Skia: 1587.6 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 840.8 µs, Skia: 821.0 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1342.6 µs, Skia: 1702.2 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1512.2 µs, Skia: 1888.2 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1397.0 µs, Skia: 3370.2 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 697.5 µs, Skia: 982.2 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 746.1 µs, Skia: 677.8 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1064.3 µs, Skia: 1134.6 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1034.7 µs, Skia: 1088.7 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 701.7 µs, Skia: 867.1 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 975.1 µs, Skia: 1575.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1079.8 µs, Skia: 2399.8 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1013.2 µs, Skia: 1535.8 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1142.5 µs, Skia: 1566.7 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 1107.8 µs, Skia: 1560.5 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1328.1 µs, Skia: 1350.5 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 975.7 µs, Skia: 1353.4 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 979.7 µs, Skia: 1483.9 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 968.9 µs, Skia: 1306.0 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 999.1 µs, Skia: 1404.7 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 1087.1 µs, Skia: 1238.3 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 846.1 µs, Skia: 1323.7 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1076.5 µs, Skia: 1389.0 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1719.6 µs, Skia: 3385.5 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 975.6 µs, Skia: 1226.3 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 790.8 µs, Skia: 1195.7 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 998.6 µs, Skia: 1231.5 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 957.5 µs, Skia: 820.7 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 673.3 µs, Skia: 786.9 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 908.9 µs, Skia: 1272.1 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 772.6 µs, Skia: 1385.9 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 886.7 µs, Skia: 992.8 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 910.3 µs, Skia: 775.8 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 967.4 µs, Skia: 1124.0 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 855.3 µs, Skia: 1439.4 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 908.5 µs, Skia: 1481.0 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 986.1 µs, Skia: 1403.3 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1192.5 µs, Skia: 2663.0 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1577.5 µs, Skia: 3289.5 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 830.9 µs, Skia: 1995.5 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1229.2 µs, Skia: 1873.5 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 780.0 µs, Skia: 995.4 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 919.2 µs, Skia: 1304.7 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 793.5 µs, Skia: 1227.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1041.9 µs, Skia: 1519.5 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.9 µs  558.5 µs    562.4 µs   20.1 µs  478.8 µs    498.9 µs       5.1x Nisaba          1.13x Skia
2. 100 Grid Lines (1px Hairlines)            41.6 µs  701.4 µs    743.0 µs  155.5 µs 1110.2 µs   1265.7 µs       3.7x Nisaba        1.70x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      73.6 µs  885.2 µs    958.8 µs  913.7 µs 1264.3 µs   2178.0 µs      12.4x Nisaba        2.27x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      58.3 µs  805.0 µs    863.3 µs  133.3 µs  760.6 µs    893.9 µs       2.3x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   46.4 µs  969.6 µs   1016.0 µs  172.5 µs  993.3 µs   1165.8 µs       3.7x Nisaba        1.15x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     38.2 µs 1032.1 µs   1070.2 µs  105.1 µs 1082.7 µs   1187.8 µs       2.8x Nisaba        1.11x Nisaba
7. 50 Varying-Radius Rounded Rects           37.2 µs  989.9 µs   1027.1 µs   71.5 µs  771.8 µs    843.2 µs       1.9x Nisaba          1.22x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   76.0 µs  950.6 µs   1026.7 µs  280.9 µs 1312.4 µs   1593.3 µs       3.7x Nisaba        1.55x Nisaba
9. 100 Alpha Circles / Disks (Fill)          98.4 µs 1020.1 µs   1118.5 µs  366.1 µs 1320.4 µs   1686.5 µs       3.7x Nisaba        1.51x Nisaba
10. 25 Concentric Rings (2px Stroke)        147.3 µs  986.8 µs   1134.1 µs  181.3 µs 1259.4 µs   1440.8 µs       1.2x Nisaba        1.27x Nisaba
11. 50 Two-Stop Linear Gradients             88.2 µs 1304.2 µs   1392.4 µs 1030.6 µs 1716.8 µs   2747.5 µs      11.7x Nisaba        1.97x Nisaba
12. 50 Diagonal Gradients (45° Angle)       79.2 µs 1175.2 µs   1254.5 µs 1055.5 µs 1551.2 µs   2606.8 µs      13.3x Nisaba        2.08x Nisaba
13. 25 Radial Glow Gradients (300px)         71.4 µs 2526.1 µs   2597.5 µs  618.6 µs 2220.3 µs   2838.9 µs       8.7x Nisaba        1.09x Nisaba
14. 20 Box Blur / Drop Shadows (15px)        49.7 µs 1123.2 µs   1172.9 µs  479.0 µs 1319.9 µs   1798.9 µs       9.6x Nisaba        1.53x Nisaba
15. 10 Concave 10-Point Stars (Fill)        210.7 µs 1577.2 µs   1787.9 µs 2362.5 µs 1703.3 µs   4065.8 µs      11.2x Nisaba        2.27x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)  146.5 µs 1129.7 µs   1276.2 µs 2199.7 µs 1575.5 µs   3775.2 µs      15.0x Nisaba        2.96x Nisaba
17. 1000-Point Waveform (Dense Stroke)      353.9 µs 1213.3 µs   1567.2 µs 1960.7 µs 1524.3 µs   3485.0 µs       5.5x Nisaba        2.22x Nisaba
18. 50 Rotated Cards (Affine Transforms)     37.5 µs  760.7 µs    798.2 µs  305.9 µs 1290.0 µs   1595.8 µs       8.2x Nisaba        2.00x Nisaba
19. 25 Scissor Clipped Viewports             23.1 µs  720.5 µs    743.6 µs  129.1 µs  676.0 µs    805.1 µs       5.6x Nisaba        1.08x Nisaba
20. 50 UI Text Labels (Inter-Regular)       282.8 µs 1060.7 µs   1343.5 µs  362.3 µs 1165.3 µs   1527.6 µs       1.3x Nisaba        1.14x Nisaba
21. 25 Quadratic Bezier Curves (Fill+Stroke)  185.6 µs 1059.8 µs   1245.4 µs 1250.5 µs 1525.0 µs   2775.5 µs       6.7x Nisaba        2.23x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     71.9 µs  875.9 µs    947.8 µs 2314.3 µs 1762.7 µs   4076.9 µs      32.2x Nisaba        4.30x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       51.3 µs  927.1 µs    978.4 µs  568.6 µs 1455.1 µs   2023.7 µs      11.1x Nisaba        2.07x Nisaba
24. Circular Arcs with arcTo (20 Corners)   116.4 µs  972.8 µs   1089.2 µs 1369.5 µs 1537.5 µs   2907.0 µs      11.8x Nisaba        2.67x Nisaba
25. Directional Arcs with arc() (30 Arcs)   147.4 µs  944.5 µs   1091.9 µs  536.8 µs 1217.3 µs   1754.1 µs       3.6x Nisaba        1.61x Nisaba
26. Compound Path with Holes (15 Donuts)    240.1 µs 1290.5 µs   1530.6 µs 1523.5 µs 1467.1 µs   2990.6 µs       6.3x Nisaba        1.95x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)   135.1 µs 1037.2 µs   1172.3 µs 1084.9 µs 1409.2 µs   2494.1 µs       8.0x Nisaba        2.13x Nisaba
28. Archimedean Spiral Polyline (500 Pts)   133.0 µs  940.5 µs   1073.5 µs 2004.9 µs 1744.5 µs   3749.4 µs      15.1x Nisaba        3.49x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   35.6 µs  853.6 µs    889.2 µs  969.3 µs 1508.5 µs   2477.8 µs      27.2x Nisaba        2.79x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      75.0 µs 1256.0 µs   1330.9 µs 2969.8 µs 1696.6 µs   4666.4 µs      39.6x Nisaba        3.51x Nisaba
31. Stroke Join: Miter (12px Stroke)         20.3 µs  683.5 µs    703.8 µs  992.2 µs 1423.5 µs   2415.7 µs      48.9x Nisaba        3.43x Nisaba
32. Stroke Join: Bevel (12px Stroke)         20.9 µs  672.4 µs    693.3 µs 1059.7 µs 1414.6 µs   2474.3 µs      50.7x Nisaba        3.57x Nisaba
33. Stroke Join: Round (12px Stroke)         27.4 µs  616.9 µs    644.3 µs 1285.7 µs 1407.0 µs   2692.7 µs      47.0x Nisaba        4.18x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      19.2 µs  585.9 µs    605.1 µs  105.0 µs  781.5 µs    886.5 µs       5.5x Nisaba        1.47x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    50.5 µs  905.3 µs    955.8 µs  164.4 µs 1027.2 µs   1191.7 µs       3.3x Nisaba        1.25x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    158.1 µs 1109.9 µs   1268.0 µs  302.2 µs 1371.2 µs   1673.4 µs       1.9x Nisaba        1.32x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   64.7 µs  907.0 µs    971.7 µs 1952.7 µs 1512.7 µs   3465.4 µs      30.2x Nisaba        3.57x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   52.0 µs  928.4 µs    980.4 µs  326.6 µs 1284.6 µs   1611.1 µs       6.3x Nisaba        1.64x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   27.2 µs  750.8 µs    778.0 µs  435.7 µs 1132.9 µs   1568.6 µs      16.0x Nisaba        2.02x Nisaba
40. Stepped Variable Stroke Widths (1-20px)  113.4 µs 1017.5 µs   1131.0 µs 1359.9 µs 1562.4 µs   2922.3 µs      12.0x Nisaba        2.58x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          33.5 µs  813.8 µs    847.3 µs   99.0 µs  800.5 µs    899.4 µs       3.0x Nisaba        1.06x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       57.4 µs 1159.1 µs   1216.5 µs  125.4 µs  958.6 µs   1084.1 µs       2.2x Nisaba          1.12x Skia
43. 50 Pill / Capsule Badges (r=h/2)         54.7 µs  788.4 µs    843.1 µs  217.4 µs 1217.7 µs   1435.1 µs       4.0x Nisaba        1.70x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     69.6 µs 1093.5 µs   1163.0 µs  212.8 µs 1252.9 µs   1465.7 µs       3.1x Nisaba        1.26x Nisaba
45. High-Density Disks Cloud (500 Disks)    317.5 µs 1264.4 µs   1582.0 µs  699.0 µs 1239.7 µs   1938.6 µs       2.2x Nisaba        1.23x Nisaba
46. High-Density Stroked Rings (250 Rings)   99.4 µs 1042.2 µs   1141.6 µs  596.7 µs 1244.6 µs   1841.3 µs       6.0x Nisaba        1.61x Nisaba
47. 250 Filled Rectangles Batch             180.9 µs 1189.2 µs   1370.1 µs  646.1 µs 1472.4 µs   2118.5 µs       3.6x Nisaba        1.55x Nisaba
48. 150 Stroked Rectangles (2px Border)      91.8 µs  889.9 µs    981.6 µs  251.7 µs 1114.0 µs   1365.7 µs       2.7x Nisaba        1.39x Nisaba
49. 40 Crosshair Aim Reticles               267.2 µs 1077.0 µs   1344.2 µs 1008.1 µs 1513.6 µs   2521.7 µs       3.8x Nisaba        1.88x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     183.1 µs 1348.2 µs   1531.3 µs 1129.8 µs 1562.1 µs   2692.0 µs       6.2x Nisaba        1.76x Nisaba
51. 30 Vertical Card Linear Gradients        52.5 µs 1081.7 µs   1134.2 µs  662.6 µs 1408.8 µs   2071.4 µs      12.6x Nisaba        1.83x Nisaba
52. 30 Horizontal Bar Progress Gradients     44.2 µs  943.6 µs    987.8 µs  542.4 µs 1275.0 µs   1817.4 µs      12.3x Nisaba        1.84x Nisaba
53. 15 Concentric Radial Spotlights          26.8 µs  934.0 µs    960.8 µs  318.1 µs 1174.9 µs   1493.0 µs      11.9x Nisaba        1.55x Nisaba
54. 25 Soft Ambient Card Shadows             58.5 µs 1224.6 µs   1283.0 µs  621.3 µs 1520.1 µs   2141.4 µs      10.6x Nisaba        1.67x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      43.8 µs 1039.5 µs   1083.3 µs  398.0 µs 1246.3 µs   1644.3 µs       9.1x Nisaba        1.52x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      129.1 µs 1165.3 µs   1294.4 µs  393.7 µs 1193.9 µs   1587.6 µs       3.0x Nisaba        1.23x Nisaba
57. Full-Screen Diagonal Horizon Gradient    22.9 µs  817.9 µs    840.8 µs   80.4 µs  740.6 µs    821.0 µs       3.5x Nisaba         Tie (~1.0x)
58. 20 Inset Well Shadows (Recessed)        163.6 µs 1179.0 µs   1342.6 µs  387.4 µs 1314.8 µs   1702.2 µs       2.4x Nisaba        1.27x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     51.5 µs 1460.7 µs   1512.2 µs  317.6 µs 1570.6 µs   1888.2 µs       6.2x Nisaba        1.25x Nisaba
60. Waveform Polyline Stroked with Gradient  202.7 µs 1194.3 µs   1397.0 µs 1777.6 µs 1592.6 µs   3370.2 µs       8.8x Nisaba        2.41x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   15.1 µs  682.4 µs    697.5 µs  112.6 µs  869.6 µs    982.2 µs       7.5x Nisaba        1.41x Nisaba
62. Scaled Image Quad (2x 512x512 Blit)      29.3 µs  716.8 µs    746.1 µs   44.8 µs  633.0 µs    677.8 µs       1.5x Nisaba          1.10x Skia
63. 45° Rotated Pattern Fill in Circle      35.4 µs 1028.8 µs   1064.3 µs  147.6 µs  987.0 µs   1134.6 µs       4.2x Nisaba        1.07x Nisaba
64. Translucent Texture Overlay (50% Alpha)   40.5 µs  994.2 µs   1034.7 µs  120.0 µs  968.6 µs   1088.7 µs       3.0x Nisaba        1.05x Nisaba
65. Multi-Avatar Grid (16 Avatars)           19.7 µs  682.0 µs    701.7 µs   98.5 µs  768.6 µs    867.1 µs       5.0x Nisaba        1.24x Nisaba
66. Texture Pattern on Star Path             47.1 µs  928.0 µs    975.1 µs  417.7 µs 1157.5 µs   1575.2 µs       8.9x Nisaba        1.62x Nisaba
67. 20px Curved Path Stroked with Texture    56.3 µs 1023.5 µs   1079.8 µs  939.8 µs 1460.0 µs   2399.8 µs      16.7x Nisaba        2.22x Nisaba
68. Minified Texture Quad (0.25x Downscale)   73.7 µs  939.5 µs   1013.2 µs  341.0 µs 1194.8 µs   1535.8 µs       4.6x Nisaba        1.52x Nisaba
69. Pure Translation Stack (50 Cards)       103.1 µs 1039.3 µs   1142.5 µs  289.2 µs 1277.5 µs   1566.7 µs       2.8x Nisaba        1.37x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   70.5 µs 1037.3 µs   1107.8 µs  241.9 µs 1318.6 µs   1560.5 µs       3.4x Nisaba        1.41x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   94.1 µs 1234.0 µs   1328.1 µs  159.7 µs 1190.8 µs   1350.5 µs       1.7x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   63.7 µs  912.0 µs    975.7 µs  233.7 µs 1119.6 µs   1353.4 µs       3.7x Nisaba        1.39x Nisaba
73. Combined Affine Transforms (30 Shapes)   71.8 µs  907.9 µs    979.7 µs  204.1 µs 1279.9 µs   1483.9 µs       2.8x Nisaba        1.51x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   53.6 µs  915.3 µs    968.9 µs  187.4 µs 1118.6 µs   1306.0 µs       3.5x Nisaba        1.35x Nisaba
75. Reset Transform Stress (50 Cycles)       70.7 µs  928.4 µs    999.1 µs  246.1 µs 1158.6 µs   1404.7 µs       3.5x Nisaba        1.41x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   58.9 µs 1028.2 µs   1087.1 µs  174.4 µs 1063.9 µs   1238.3 µs       3.0x Nisaba        1.14x Nisaba
77. Multi-Scissor Grid (16 Viewports)        48.1 µs  798.0 µs    846.1 µs  275.8 µs 1047.9 µs   1323.7 µs       5.7x Nisaba        1.56x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   55.1 µs 1021.3 µs   1076.5 µs  173.3 µs 1215.7 µs   1389.0 µs       3.1x Nisaba        1.29x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  399.6 µs 1320.0 µs   1719.6 µs 2045.9 µs 1339.6 µs   3385.5 µs       5.1x Nisaba        1.97x Nisaba
80. Scissored Gradient Card (Clipped)        27.7 µs  947.8 µs    975.6 µs  347.7 µs  878.6 µs   1226.3 µs      12.5x Nisaba        1.26x Nisaba
81. Circular Overflow Clip (25 Boxes)        36.3 µs  754.5 µs    790.8 µs  296.9 µs  898.8 µs   1195.7 µs       8.2x Nisaba        1.51x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   53.2 µs  945.5 µs    998.6 µs  203.6 µs 1027.8 µs   1231.5 µs       3.8x Nisaba        1.23x Nisaba
83. Composite Op: Source-Over (30 Shapes)    51.8 µs  905.6 µs    957.5 µs   78.8 µs  742.0 µs    820.7 µs       1.5x Nisaba          1.17x Skia
84. Composite Op: Lighter / Plus (30 Particles)   24.2 µs  649.1 µs    673.3 µs  104.4 µs  682.5 µs    786.9 µs       4.3x Nisaba        1.17x Nisaba
85. Composite Op: Source-In (Alpha Masking)   47.5 µs  861.3 µs    908.9 µs  124.4 µs 1147.7 µs   1272.1 µs       2.6x Nisaba        1.40x Nisaba
86. Composite Op: Source-Out (Cutout)        44.3 µs  728.3 µs    772.6 µs  177.4 µs 1208.5 µs   1385.9 µs       4.0x Nisaba        1.79x Nisaba
87. Composite Op: Atop (Target Bounds)       35.5 µs  851.2 µs    886.7 µs  125.4 µs  867.3 µs    992.8 µs       3.5x Nisaba        1.12x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   30.1 µs  880.1 µs    910.3 µs   75.4 µs  700.4 µs    775.8 µs       2.5x Nisaba          1.17x Skia
89. Composite Op: Dest-Out (Eraser Mask)     33.9 µs  933.5 µs    967.4 µs  131.7 µs  992.2 µs   1124.0 µs       3.9x Nisaba        1.16x Nisaba
90. Composite Op: Dest-Atop (Inverse)        36.6 µs  818.7 µs    855.3 µs  176.6 µs 1262.8 µs   1439.4 µs       4.8x Nisaba        1.68x Nisaba
91. Composite Op: Xor (Exclusive Blend)      50.6 µs  857.9 µs    908.5 µs  200.9 µs 1280.1 µs   1481.0 µs       4.0x Nisaba        1.63x Nisaba
92. Composite Op: Copy (Direct Overwrite)    51.0 µs  935.0 µs    986.1 µs  195.9 µs 1207.4 µs   1403.3 µs       3.8x Nisaba        1.42x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   97.0 µs 1095.5 µs   1192.5 µs 1102.5 µs 1560.5 µs   2663.0 µs      11.4x Nisaba        2.23x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     148.6 µs 1428.9 µs   1577.5 µs 1585.7 µs 1703.7 µs   3289.5 µs      10.7x Nisaba        2.09x Nisaba
95. Circular Progress Rings (8 Meters)       84.9 µs  746.0 µs    830.9 µs  760.2 µs 1235.3 µs   1995.5 µs       9.0x Nisaba        2.40x Nisaba
96. Modern Card Stack (5 Elevated Cards)     32.0 µs 1197.2 µs   1229.2 µs  370.7 µs 1502.8 µs   1873.5 µs      11.6x Nisaba        1.52x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   48.8 µs  731.1 µs    780.0 µs   96.7 µs  898.7 µs    995.4 µs       2.0x Nisaba        1.28x Nisaba
98. Floating Action Button (FAB 10 Buttons)   52.9 µs  866.4 µs    919.2 µs  219.6 µs 1085.0 µs   1304.7 µs       4.2x Nisaba        1.42x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   48.2 µs  745.4 µs    793.5 µs  216.4 µs 1010.7 µs   1227.1 µs       4.5x Nisaba        1.55x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   58.2 µs  983.8 µs   1041.9 µs  313.2 µs 1206.3 µs   1519.5 µs       5.4x Nisaba        1.46x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          8359.6 µs98531.6 µs 106891.2 µs59322.8 µs123020.8 µs 182343.6 µs      7.10x Nisaba        1.71x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 91 suites | Skia won in 6 suites | Ties: 3
         Overall Frame Throughput (End-to-End): 1.71x Nisaba
         Overall CPU Command Submission:       7.10x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 571.6 µs, Skia: 428.0 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 908.5 µs, Skia: 1453.5 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 716.1 µs, Skia: 1716.1 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1046.6 µs, Skia: 1301.8 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1216.4 µs, Skia: 1234.6 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1252.0 µs, Skia: 1603.6 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1220.1 µs, Skia: 1469.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1190.6 µs, Skia: 1488.8 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1103.2 µs, Skia: 1736.3 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1156.9 µs, Skia: 1367.8 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1320.7 µs, Skia: 2463.1 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 940.4 µs, Skia: 1979.5 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2682.6 µs, Skia: 2640.1 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1093.4 µs, Skia: 1384.5 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1189.4 µs, Skia: 2546.9 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 806.0 µs, Skia: 3059.6 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 965.8 µs, Skia: 2990.0 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 793.5 µs, Skia: 1409.0 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 857.2 µs, Skia: 607.9 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1070.1 µs, Skia: 1181.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1524.7 µs, Skia: 2811.6 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 985.8 µs, Skia: 3735.1 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 830.1 µs, Skia: 2135.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 815.0 µs, Skia: 3194.4 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1195.3 µs, Skia: 1532.4 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1968.3 µs, Skia: 2535.9 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 831.2 µs, Skia: 2984.7 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 884.5 µs, Skia: 2931.4 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 612.8 µs, Skia: 2100.6 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 929.3 µs, Skia: 3710.0 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 616.4 µs, Skia: 2434.8 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 611.1 µs, Skia: 1790.5 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 597.8 µs, Skia: 2170.4 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 672.6 µs, Skia: 811.3 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 987.4 µs, Skia: 1076.5 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1278.7 µs, Skia: 1430.8 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 933.9 µs, Skia: 3031.0 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 530.1 µs, Skia: 1123.8 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 698.5 µs, Skia: 2225.4 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 651.0 µs, Skia: 2633.0 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 728.6 µs, Skia: 1102.0 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1036.5 µs, Skia: 943.1 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1003.9 µs, Skia: 1313.3 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1004.6 µs, Skia: 1381.4 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1693.7 µs, Skia: 2848.2 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1572.8 µs, Skia: 2290.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1348.8 µs, Skia: 2102.5 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1401.5 µs, Skia: 1802.9 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1604.8 µs, Skia: 3288.5 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 2186.8 µs, Skia: 3404.8 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1093.0 µs, Skia: 2379.2 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1098.1 µs, Skia: 2411.6 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1113.1 µs, Skia: 1879.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1160.3 µs, Skia: 2123.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1060.6 µs, Skia: 1856.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1253.7 µs, Skia: 1957.4 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 992.6 µs, Skia: 1158.7 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1684.0 µs, Skia: 2161.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1837.6 µs, Skia: 1696.6 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1348.4 µs, Skia: 2707.8 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 821.8 µs, Skia: 1172.4 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1073.0 µs, Skia: 993.0 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1010.0 µs, Skia: 951.1 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1189.9 µs, Skia: 1233.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 859.1 µs, Skia: 1363.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1047.3 µs, Skia: 2124.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1109.9 µs, Skia: 2375.0 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1366.5 µs, Skia: 1605.6 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1126.1 µs, Skia: 1531.0 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 1106.8 µs, Skia: 1445.1 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 966.5 µs, Skia: 1279.0 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 1095.8 µs, Skia: 1341.9 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1026.4 µs, Skia: 1273.8 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 845.6 µs, Skia: 1470.9 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 1062.2 µs, Skia: 1487.7 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 892.1 µs, Skia: 1320.7 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 966.6 µs, Skia: 1369.0 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1054.0 µs, Skia: 1524.6 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1685.1 µs, Skia: 5330.7 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 883.2 µs, Skia: 1751.0 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 892.3 µs, Skia: 1187.4 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 942.4 µs, Skia: 1173.4 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 934.2 µs, Skia: 1175.1 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 901.9 µs, Skia: 1211.7 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 965.4 µs, Skia: 1197.7 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 810.9 µs, Skia: 1286.4 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 848.4 µs, Skia: 996.0 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1070.0 µs, Skia: 1114.5 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 812.8 µs, Skia: 1507.6 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 974.1 µs, Skia: 1482.2 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 932.4 µs, Skia: 1291.2 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1022.6 µs, Skia: 1345.4 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1001.4 µs, Skia: 2759.8 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1554.5 µs, Skia: 2580.2 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 805.2 µs, Skia: 2193.6 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1249.5 µs, Skia: 1938.7 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 1193.6 µs, Skia: 1529.5 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1418.0 µs, Skia: 1778.8 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 632.8 µs, Skia: 1321.3 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1061.0 µs, Skia: 1830.4 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          4.1 µs  567.5 µs    571.6 µs   14.8 µs  413.2 µs    428.0 µs       3.6x Nisaba          1.34x Skia
2. 100 Grid Lines (1px Hairlines)            49.0 µs  859.4 µs    908.5 µs  166.1 µs 1287.5 µs   1453.5 µs       3.4x Nisaba        1.60x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      37.2 µs  678.9 µs    716.1 µs  516.3 µs 1199.9 µs   1716.1 µs      13.9x Nisaba        2.40x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      90.4 µs  956.2 µs   1046.6 µs  239.9 µs 1061.8 µs   1301.8 µs       2.7x Nisaba        1.24x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   95.8 µs 1120.6 µs   1216.4 µs  185.7 µs 1048.9 µs   1234.6 µs       1.9x Nisaba         Tie (~1.0x)
6. 50 Simple Rounded Rects (12px Radius)     82.0 µs 1170.1 µs   1252.0 µs  279.6 µs 1324.1 µs   1603.6 µs       3.4x Nisaba        1.28x Nisaba
7. 50 Varying-Radius Rounded Rects           69.4 µs 1150.7 µs   1220.1 µs  211.0 µs 1258.5 µs   1469.5 µs       3.0x Nisaba        1.20x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   90.5 µs 1100.1 µs   1190.6 µs  262.1 µs 1226.7 µs   1488.8 µs       2.9x Nisaba        1.25x Nisaba
9. 100 Alpha Circles / Disks (Fill)          97.6 µs 1005.5 µs   1103.2 µs  354.7 µs 1381.6 µs   1736.3 µs       3.6x Nisaba        1.57x Nisaba
10. 25 Concentric Rings (2px Stroke)        149.3 µs 1007.6 µs   1156.9 µs  185.8 µs 1182.0 µs   1367.8 µs       1.2x Nisaba        1.18x Nisaba
11. 50 Two-Stop Linear Gradients             76.7 µs 1244.0 µs   1320.7 µs  910.8 µs 1552.3 µs   2463.1 µs      11.9x Nisaba        1.87x Nisaba
12. 50 Diagonal Gradients (45° Angle)       39.4 µs  900.9 µs    940.4 µs  651.8 µs 1327.7 µs   1979.5 µs      16.5x Nisaba        2.10x Nisaba
13. 25 Radial Glow Gradients (300px)         61.5 µs 2621.1 µs   2682.6 µs  509.2 µs 2130.9 µs   2640.1 µs       8.3x Nisaba         Tie (~1.0x)
14. 20 Box Blur / Drop Shadows (15px)        30.3 µs 1063.1 µs   1093.4 µs  244.4 µs 1140.1 µs   1384.5 µs       8.1x Nisaba        1.27x Nisaba
15. 10 Concave 10-Point Stars (Fill)        109.7 µs 1079.7 µs   1189.4 µs 1121.0 µs 1426.0 µs   2546.9 µs      10.2x Nisaba        2.14x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   55.4 µs  750.6 µs    806.0 µs 1544.4 µs 1515.2 µs   3059.6 µs      27.9x Nisaba        3.80x Nisaba
17. 1000-Point Waveform (Dense Stroke)      116.4 µs  849.5 µs    965.8 µs 1477.0 µs 1513.0 µs   2990.0 µs      12.7x Nisaba        3.10x Nisaba
18. 50 Rotated Cards (Affine Transforms)     27.3 µs  766.3 µs    793.5 µs  202.3 µs 1206.6 µs   1409.0 µs       7.4x Nisaba        1.78x Nisaba
19. 25 Scissor Clipped Viewports             42.9 µs  814.3 µs    857.2 µs  103.1 µs  504.8 µs    607.9 µs       2.4x Nisaba          1.41x Skia
20. 50 UI Text Labels (Inter-Regular)       193.3 µs  876.8 µs   1070.1 µs  211.0 µs  970.5 µs   1181.6 µs       1.1x Nisaba        1.10x Nisaba
21. 25 Quadratic Bezier Curves (Fill+Stroke)  229.2 µs 1295.5 µs   1524.7 µs 1285.6 µs 1526.0 µs   2811.6 µs       5.6x Nisaba        1.84x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     85.5 µs  900.3 µs    985.8 µs 2096.9 µs 1638.2 µs   3735.1 µs      24.5x Nisaba        3.79x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       31.3 µs  798.8 µs    830.1 µs  690.5 µs 1445.2 µs   2135.8 µs      22.1x Nisaba        2.57x Nisaba
24. Circular Arcs with arcTo (20 Corners)    81.0 µs  734.0 µs    815.0 µs 1599.2 µs 1595.2 µs   3194.4 µs      19.7x Nisaba        3.92x Nisaba
25. Directional Arcs with arc() (30 Arcs)   209.5 µs  985.8 µs   1195.3 µs  368.3 µs 1164.1 µs   1532.4 µs       1.8x Nisaba        1.28x Nisaba
26. Compound Path with Holes (15 Donuts)    368.9 µs 1599.4 µs   1968.3 µs 1139.2 µs 1396.7 µs   2535.9 µs       3.1x Nisaba        1.29x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    53.7 µs  777.5 µs    831.2 µs 1436.0 µs 1548.7 µs   2984.7 µs      26.7x Nisaba        3.59x Nisaba
28. Archimedean Spiral Polyline (500 Pts)   103.5 µs  780.9 µs    884.5 µs 1405.8 µs 1525.6 µs   2931.4 µs      13.6x Nisaba        3.31x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   14.0 µs  598.8 µs    612.8 µs  843.8 µs 1256.8 µs   2100.6 µs      60.2x Nisaba        3.43x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      56.3 µs  873.0 µs    929.3 µs 2070.5 µs 1639.5 µs   3710.0 µs      36.8x Nisaba        3.99x Nisaba
31. Stroke Join: Miter (12px Stroke)         17.1 µs  599.2 µs    616.4 µs 1117.3 µs 1317.5 µs   2434.8 µs      65.2x Nisaba        3.95x Nisaba
32. Stroke Join: Bevel (12px Stroke)         21.5 µs  589.6 µs    611.1 µs  647.0 µs 1143.5 µs   1790.5 µs      30.1x Nisaba        2.93x Nisaba
33. Stroke Join: Round (12px Stroke)         20.6 µs  577.2 µs    597.8 µs  892.7 µs 1277.7 µs   2170.4 µs      43.4x Nisaba        3.63x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      17.7 µs  654.9 µs    672.6 µs   93.9 µs  717.5 µs    811.3 µs       5.3x Nisaba        1.21x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    35.4 µs  952.0 µs    987.4 µs  134.2 µs  942.4 µs   1076.5 µs       3.8x Nisaba        1.09x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    130.2 µs 1148.6 µs   1278.7 µs  233.0 µs 1197.8 µs   1430.8 µs       1.8x Nisaba        1.12x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   37.3 µs  896.6 µs    933.9 µs 1585.1 µs 1445.8 µs   3031.0 µs      42.5x Nisaba        3.25x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   26.0 µs  504.1 µs    530.1 µs  178.7 µs  945.1 µs   1123.8 µs       6.9x Nisaba        2.12x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   26.8 µs  671.7 µs    698.5 µs  775.3 µs 1450.1 µs   2225.4 µs      28.9x Nisaba        3.19x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   41.1 µs  609.9 µs    651.0 µs 1161.4 µs 1471.6 µs   2633.0 µs      28.3x Nisaba        4.04x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          21.7 µs  706.9 µs    728.6 µs  117.9 µs  984.2 µs   1102.0 µs       5.4x Nisaba        1.51x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       45.1 µs  991.4 µs   1036.5 µs  125.3 µs  817.8 µs    943.1 µs       2.8x Nisaba          1.10x Skia
43. 50 Pill / Capsule Badges (r=h/2)         51.1 µs  952.9 µs   1003.9 µs  190.2 µs 1123.2 µs   1313.3 µs       3.7x Nisaba        1.31x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     49.4 µs  955.2 µs   1004.6 µs  191.6 µs 1189.8 µs   1381.4 µs       3.9x Nisaba        1.38x Nisaba
45. High-Density Disks Cloud (500 Disks)    307.5 µs 1386.3 µs   1693.7 µs 1187.8 µs 1660.4 µs   2848.2 µs       3.9x Nisaba        1.68x Nisaba
46. High-Density Stroked Rings (250 Rings)  166.2 µs 1406.6 µs   1572.8 µs  781.3 µs 1508.7 µs   2290.1 µs       4.7x Nisaba        1.46x Nisaba
47. 250 Filled Rectangles Batch             176.1 µs 1172.7 µs   1348.8 µs  744.2 µs 1358.3 µs   2102.5 µs       4.2x Nisaba        1.56x Nisaba
48. 150 Stroked Rectangles (2px Border)     160.8 µs 1240.7 µs   1401.5 µs  438.4 µs 1364.4 µs   1802.9 µs       2.7x Nisaba        1.29x Nisaba
49. 40 Crosshair Aim Reticles               381.7 µs 1223.2 µs   1604.8 µs 1677.4 µs 1611.1 µs   3288.5 µs       4.4x Nisaba        2.05x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     410.2 µs 1776.6 µs   2186.8 µs 1728.0 µs 1676.8 µs   3404.8 µs       4.2x Nisaba        1.56x Nisaba
51. 30 Vertical Card Linear Gradients        60.4 µs 1032.6 µs   1093.0 µs  840.7 µs 1538.4 µs   2379.2 µs      13.9x Nisaba        2.18x Nisaba
52. 30 Horizontal Bar Progress Gradients     57.9 µs 1040.2 µs   1098.1 µs  849.1 µs 1562.5 µs   2411.6 µs      14.7x Nisaba        2.20x Nisaba
53. 15 Concentric Radial Spotlights          45.5 µs 1067.6 µs   1113.1 µs  462.1 µs 1417.0 µs   1879.1 µs      10.2x Nisaba        1.69x Nisaba
54. 25 Soft Ambient Card Shadows             53.9 µs 1106.4 µs   1160.3 µs  674.1 µs 1449.4 µs   2123.5 µs      12.5x Nisaba        1.83x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      45.8 µs 1014.8 µs   1060.6 µs  434.6 µs 1421.9 µs   1856.5 µs       9.5x Nisaba        1.75x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      130.8 µs 1122.9 µs   1253.7 µs  568.1 µs 1389.3 µs   1957.4 µs       4.3x Nisaba        1.56x Nisaba
57. Full-Screen Diagonal Horizon Gradient    35.8 µs  956.8 µs    992.6 µs  137.1 µs 1021.6 µs   1158.7 µs       3.8x Nisaba        1.17x Nisaba
58. 20 Inset Well Shadows (Recessed)        245.0 µs 1439.0 µs   1684.0 µs  581.3 µs 1580.3 µs   2161.5 µs       2.4x Nisaba        1.28x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     60.6 µs 1777.0 µs   1837.6 µs  314.4 µs 1382.3 µs   1696.6 µs       5.2x Nisaba          1.08x Skia
60. Waveform Polyline Stroked with Gradient  196.7 µs 1151.6 µs   1348.4 µs 1280.4 µs 1427.3 µs   2707.8 µs       6.5x Nisaba        2.01x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   18.5 µs  803.4 µs    821.8 µs  125.2 µs 1047.2 µs   1172.4 µs       6.8x Nisaba        1.43x Nisaba
62. Scaled Image Quad (2x 512x512 Blit)      33.6 µs 1039.4 µs   1073.0 µs   98.4 µs  894.6 µs    993.0 µs       2.9x Nisaba          1.08x Skia
63. 45° Rotated Pattern Fill in Circle      22.7 µs  987.3 µs   1010.0 µs   71.4 µs  879.7 µs    951.1 µs       3.1x Nisaba          1.06x Skia
64. Translucent Texture Overlay (50% Alpha)   44.5 µs 1145.4 µs   1189.9 µs  149.9 µs 1083.1 µs   1233.0 µs       3.4x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           34.2 µs  824.9 µs    859.1 µs  198.1 µs 1165.1 µs   1363.2 µs       5.8x Nisaba        1.59x Nisaba
66. Texture Pattern on Star Path             76.6 µs  970.7 µs   1047.3 µs  731.4 µs 1392.8 µs   2124.2 µs       9.6x Nisaba        2.03x Nisaba
67. 20px Curved Path Stroked with Texture    54.7 µs 1055.2 µs   1109.9 µs  945.9 µs 1429.0 µs   2375.0 µs      17.3x Nisaba        2.14x Nisaba
68. Minified Texture Quad (0.25x Downscale)   87.5 µs 1279.0 µs   1366.5 µs  347.9 µs 1257.8 µs   1605.6 µs       4.0x Nisaba        1.17x Nisaba
69. Pure Translation Stack (50 Cards)       100.6 µs 1025.4 µs   1126.1 µs  302.6 µs 1228.4 µs   1531.0 µs       3.0x Nisaba        1.36x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   71.1 µs 1035.7 µs   1106.8 µs  234.0 µs 1211.1 µs   1445.1 µs       3.3x Nisaba        1.31x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   86.3 µs  880.2 µs    966.5 µs  171.2 µs 1107.8 µs   1279.0 µs       2.0x Nisaba        1.32x Nisaba
72. Shear / Skew Parallelograms (30 Quads)   60.8 µs 1035.0 µs   1095.8 µs  238.5 µs 1103.4 µs   1341.9 µs       3.9x Nisaba        1.22x Nisaba
73. Combined Affine Transforms (30 Shapes)   65.5 µs  960.9 µs   1026.4 µs  207.4 µs 1066.4 µs   1273.8 µs       3.2x Nisaba        1.24x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   51.1 µs  794.5 µs    845.6 µs  198.8 µs 1272.1 µs   1470.9 µs       3.9x Nisaba        1.74x Nisaba
75. Reset Transform Stress (50 Cycles)       81.5 µs  980.8 µs   1062.2 µs  265.5 µs 1222.2 µs   1487.7 µs       3.3x Nisaba        1.40x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   54.0 µs  838.2 µs    892.1 µs  200.8 µs 1119.9 µs   1320.7 µs       3.7x Nisaba        1.48x Nisaba
77. Multi-Scissor Grid (16 Viewports)        47.5 µs  919.1 µs    966.6 µs  281.5 µs 1087.5 µs   1369.0 µs       5.9x Nisaba        1.42x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   51.4 µs 1002.5 µs   1054.0 µs  191.0 µs 1333.6 µs   1524.6 µs       3.7x Nisaba        1.45x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  405.9 µs 1279.2 µs   1685.1 µs 3759.7 µs 1571.0 µs   5330.7 µs       9.3x Nisaba        3.16x Nisaba
80. Scissored Gradient Card (Clipped)        18.6 µs  864.7 µs    883.2 µs  454.6 µs 1296.4 µs   1751.0 µs      24.5x Nisaba        1.98x Nisaba
81. Circular Overflow Clip (25 Boxes)        33.3 µs  859.0 µs    892.3 µs  265.2 µs  922.3 µs   1187.4 µs       8.0x Nisaba        1.33x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   52.9 µs  889.6 µs    942.4 µs  147.5 µs 1026.0 µs   1173.4 µs       2.8x Nisaba        1.25x Nisaba
83. Composite Op: Source-Over (30 Shapes)    51.6 µs  882.6 µs    934.2 µs  147.0 µs 1028.0 µs   1175.1 µs       2.8x Nisaba        1.26x Nisaba
84. Composite Op: Lighter / Plus (30 Particles)   38.4 µs  863.5 µs    901.9 µs  169.0 µs 1042.7 µs   1211.7 µs       4.4x Nisaba        1.34x Nisaba
85. Composite Op: Source-In (Alpha Masking)   40.0 µs  925.4 µs    965.4 µs  142.1 µs 1055.6 µs   1197.7 µs       3.5x Nisaba        1.24x Nisaba
86. Composite Op: Source-Out (Cutout)        37.3 µs  773.6 µs    810.9 µs  163.1 µs 1123.3 µs   1286.4 µs       4.4x Nisaba        1.59x Nisaba
87. Composite Op: Atop (Target Bounds)       47.2 µs  801.2 µs    848.4 µs  116.3 µs  879.8 µs    996.0 µs       2.5x Nisaba        1.17x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   52.9 µs 1017.1 µs   1070.0 µs  147.3 µs  967.3 µs   1114.5 µs       2.8x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     39.6 µs  773.2 µs    812.8 µs  214.0 µs 1293.6 µs   1507.6 µs       5.4x Nisaba        1.85x Nisaba
90. Composite Op: Dest-Atop (Inverse)        55.9 µs  918.2 µs    974.1 µs  193.8 µs 1288.4 µs   1482.2 µs       3.5x Nisaba        1.52x Nisaba
91. Composite Op: Xor (Exclusive Blend)      51.6 µs  880.8 µs    932.4 µs  201.3 µs 1089.8 µs   1291.2 µs       3.9x Nisaba        1.38x Nisaba
92. Composite Op: Copy (Direct Overwrite)    57.9 µs  964.8 µs   1022.6 µs  186.8 µs 1158.6 µs   1345.4 µs       3.2x Nisaba        1.32x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   96.3 µs  905.1 µs   1001.4 µs 1243.9 µs 1515.9 µs   2759.8 µs      12.9x Nisaba        2.76x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     150.4 µs 1404.1 µs   1554.5 µs 1096.5 µs 1483.7 µs   2580.2 µs       7.3x Nisaba        1.66x Nisaba
95. Circular Progress Rings (8 Meters)       79.8 µs  725.4 µs    805.2 µs  916.2 µs 1277.4 µs   2193.6 µs      11.5x Nisaba        2.72x Nisaba
96. Modern Card Stack (5 Elevated Cards)     41.5 µs 1208.0 µs   1249.5 µs  407.9 µs 1530.8 µs   1938.7 µs       9.8x Nisaba        1.55x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   99.9 µs 1093.7 µs   1193.6 µs  217.6 µs 1311.8 µs   1529.5 µs       2.2x Nisaba        1.28x Nisaba
98. Floating Action Button (FAB 10 Buttons)  170.8 µs 1247.3 µs   1418.0 µs  437.4 µs 1341.4 µs   1778.8 µs       2.6x Nisaba        1.25x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   52.3 µs  580.5 µs    632.8 µs  236.8 µs 1084.5 µs   1321.3 µs       4.5x Nisaba        2.09x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   50.7 µs 1010.3 µs   1061.0 µs  351.1 µs 1479.3 µs   1830.4 µs       6.9x Nisaba        1.73x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          8593.9 µs99096.5 µs 107690.4 µs58643.9 µs126470.5 µs 185114.5 µs      6.82x Nisaba        1.72x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 90 suites | Skia won in 6 suites | Ties: 4
         Overall Frame Throughput (End-to-End): 1.72x Nisaba
         Overall CPU Command Submission:       6.82x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 826.4 µs, Skia: 801.7 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 707.3 µs, Skia: 1339.1 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 949.2 µs, Skia: 1986.0 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1118.8 µs, Skia: 1229.4 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1072.0 µs, Skia: 1333.5 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1041.5 µs, Skia: 1208.7 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1085.2 µs, Skia: 1105.9 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 977.5 µs, Skia: 1463.8 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 891.5 µs, Skia: 1041.0 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 976.9 µs, Skia: 1421.8 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1264.1 µs, Skia: 1942.7 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1034.0 µs, Skia: 2226.1 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2637.8 µs, Skia: 2543.3 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1158.3 µs, Skia: 2019.8 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1687.6 µs, Skia: 3916.6 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1023.5 µs, Skia: 4176.0 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1484.2 µs, Skia: 3936.6 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 891.9 µs, Skia: 1611.1 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 813.9 µs, Skia: 938.4 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1375.3 µs, Skia: 1313.1 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1468.3 µs, Skia: 2437.2 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 770.5 µs, Skia: 4203.5 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 790.5 µs, Skia: 1913.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1279.9 µs, Skia: 3024.0 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 880.0 µs, Skia: 1799.6 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1875.4 µs, Skia: 4563.5 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 1373.7 µs, Skia: 3762.8 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1102.8 µs, Skia: 5997.7 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 848.0 µs, Skia: 3108.9 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1289.6 µs, Skia: 5030.1 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 693.6 µs, Skia: 2393.6 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 734.2 µs, Skia: 2267.9 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 775.7 µs, Skia: 3171.3 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 568.4 µs, Skia: 968.8 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 794.6 µs, Skia: 1086.9 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 964.2 µs, Skia: 1049.2 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 660.5 µs, Skia: 3078.4 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 692.7 µs, Skia: 989.9 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 803.9 µs, Skia: 1590.8 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 947.3 µs, Skia: 3204.8 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 867.1 µs, Skia: 923.0 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 982.9 µs, Skia: 1053.1 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 914.1 µs, Skia: 1178.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1057.0 µs, Skia: 991.5 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1052.7 µs, Skia: 2161.7 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1032.4 µs, Skia: 1720.5 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1022.2 µs, Skia: 1756.4 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 981.8 µs, Skia: 1288.9 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1238.5 µs, Skia: 2288.9 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1368.0 µs, Skia: 2074.2 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1106.3 µs, Skia: 1571.1 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1067.9 µs, Skia: 1845.9 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 964.6 µs, Skia: 1467.2 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1065.8 µs, Skia: 1808.8 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1081.7 µs, Skia: 1591.7 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1002.2 µs, Skia: 1498.7 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 855.2 µs, Skia: 938.9 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1347.0 µs, Skia: 1757.2 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1518.0 µs, Skia: 1706.3 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1062.9 µs, Skia: 2666.2 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 919.0 µs, Skia: 1001.0 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 792.1 µs, Skia: 1009.6 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 854.8 µs, Skia: 888.7 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 994.0 µs, Skia: 1014.0 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 1007.0 µs, Skia: 1073.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 889.9 µs, Skia: 1407.1 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 717.1 µs, Skia: 1650.6 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 732.0 µs, Skia: 1108.0 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 924.1 µs, Skia: 1197.5 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 648.5 µs, Skia: 918.6 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 957.0 µs, Skia: 939.6 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 855.3 µs, Skia: 1131.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 826.0 µs, Skia: 1083.3 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 899.9 µs, Skia: 992.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 812.0 µs, Skia: 1149.5 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 714.6 µs, Skia: 1128.6 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 937.3 µs, Skia: 920.8 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 913.5 µs, Skia: 1136.1 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1498.0 µs, Skia: 3319.9 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 892.8 µs, Skia: 1463.5 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 927.0 µs, Skia: 1556.5 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 1081.2 µs, Skia: 1264.7 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 971.3 µs, Skia: 1345.1 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 797.8 µs, Skia: 1368.6 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1063.8 µs, Skia: 1586.6 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 960.4 µs, Skia: 1517.7 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 1065.2 µs, Skia: 1157.7 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 997.2 µs, Skia: 1407.4 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1045.8 µs, Skia: 1385.9 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 916.0 µs, Skia: 1404.8 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 895.2 µs, Skia: 1317.5 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 995.4 µs, Skia: 1376.1 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1252.9 µs, Skia: 2685.6 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1393.5 µs, Skia: 3291.1 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1257.5 µs, Skia: 3982.1 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1486.3 µs, Skia: 1979.4 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 729.9 µs, Skia: 970.5 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1088.0 µs, Skia: 1555.5 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 960.8 µs, Skia: 1452.0 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1145.7 µs, Skia: 1589.1 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          8.1 µs  818.3 µs    826.4 µs   34.5 µs  767.2 µs    801.7 µs       4.3x Nisaba         Tie (~1.0x)
2. 100 Grid Lines (1px Hairlines)            41.4 µs  665.9 µs    707.3 µs  139.6 µs 1199.6 µs   1339.1 µs       3.4x Nisaba        1.89x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      76.3 µs  872.8 µs    949.2 µs  733.8 µs 1252.2 µs   1986.0 µs       9.6x Nisaba        2.09x Nisaba
4. 100 Solid Rectangles (Varied Bounds)     116.8 µs 1001.9 µs   1118.8 µs  223.4 µs 1006.0 µs   1229.4 µs       1.9x Nisaba        1.10x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   46.7 µs 1025.4 µs   1072.0 µs  218.9 µs 1114.6 µs   1333.5 µs       4.7x Nisaba        1.24x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     45.8 µs  995.7 µs   1041.5 µs  157.6 µs 1051.0 µs   1208.7 µs       3.4x Nisaba        1.16x Nisaba
7. 50 Varying-Radius Rounded Rects           46.8 µs 1038.4 µs   1085.2 µs  132.4 µs  973.5 µs   1105.9 µs       2.8x Nisaba         Tie (~1.0x)
8. 50 Stroked Rounded Rects (1.5px Border)   61.3 µs  916.2 µs    977.5 µs  227.2 µs 1236.6 µs   1463.8 µs       3.7x Nisaba        1.50x Nisaba
9. 100 Alpha Circles / Disks (Fill)          39.3 µs  852.2 µs    891.5 µs  150.6 µs  890.3 µs   1041.0 µs       3.8x Nisaba        1.17x Nisaba
10. 25 Concentric Rings (2px Stroke)         83.7 µs  893.2 µs    976.9 µs  186.1 µs 1235.6 µs   1421.8 µs       2.2x Nisaba        1.46x Nisaba
11. 50 Two-Stop Linear Gradients             50.6 µs 1213.5 µs   1264.1 µs  570.9 µs 1371.8 µs   1942.7 µs      11.3x Nisaba        1.54x Nisaba
12. 50 Diagonal Gradients (45° Angle)       44.8 µs  989.3 µs   1034.0 µs  786.4 µs 1439.7 µs   2226.1 µs      17.6x Nisaba        2.15x Nisaba
13. 25 Radial Glow Gradients (300px)         58.1 µs 2579.7 µs   2637.8 µs  424.3 µs 2119.0 µs   2543.3 µs       7.3x Nisaba         Tie (~1.0x)
14. 20 Box Blur / Drop Shadows (15px)        44.9 µs 1113.4 µs   1158.3 µs  500.5 µs 1519.2 µs   2019.8 µs      11.2x Nisaba        1.74x Nisaba
15. 10 Concave 10-Point Stars (Fill)        212.5 µs 1475.1 µs   1687.6 µs 2256.9 µs 1659.7 µs   3916.6 µs      10.6x Nisaba        2.32x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)  128.4 µs  895.0 µs   1023.5 µs 2454.8 µs 1721.3 µs   4176.0 µs      19.1x Nisaba        4.08x Nisaba
17. 1000-Point Waveform (Dense Stroke)      267.1 µs 1217.1 µs   1484.2 µs 2190.8 µs 1745.7 µs   3936.6 µs       8.2x Nisaba        2.65x Nisaba
18. 50 Rotated Cards (Affine Transforms)     54.8 µs  837.1 µs    891.9 µs  285.7 µs 1325.4 µs   1611.1 µs       5.2x Nisaba        1.81x Nisaba
19. 25 Scissor Clipped Viewports             22.2 µs  791.7 µs    813.9 µs  180.2 µs  758.1 µs    938.4 µs       8.1x Nisaba        1.15x Nisaba
20. 50 UI Text Labels (Inter-Regular)       298.6 µs 1076.7 µs   1375.3 µs  286.7 µs 1026.5 µs   1313.1 µs               Tie         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  234.7 µs 1233.6 µs   1468.3 µs  983.2 µs 1454.0 µs   2437.2 µs       4.2x Nisaba        1.66x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     62.6 µs  707.9 µs    770.5 µs 2536.9 µs 1666.6 µs   4203.5 µs      40.5x Nisaba        5.46x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       24.9 µs  765.7 µs    790.5 µs  603.8 µs 1310.0 µs   1913.8 µs      24.3x Nisaba        2.42x Nisaba
24. Circular Arcs with arcTo (20 Corners)   118.4 µs 1161.5 µs   1279.9 µs 1379.0 µs 1644.9 µs   3024.0 µs      11.6x Nisaba        2.36x Nisaba
25. Directional Arcs with arc() (30 Arcs)   119.1 µs  760.9 µs    880.0 µs  508.7 µs 1290.9 µs   1799.6 µs       4.3x Nisaba        2.05x Nisaba
26. Compound Path with Holes (15 Donuts)    331.9 µs 1543.5 µs   1875.4 µs 2927.6 µs 1635.9 µs   4563.5 µs       8.8x Nisaba        2.43x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)   190.2 µs 1183.6 µs   1373.7 µs 2136.0 µs 1626.8 µs   3762.8 µs      11.2x Nisaba        2.74x Nisaba
28. Archimedean Spiral Polyline (500 Pts)   121.3 µs  981.5 µs   1102.8 µs 4313.8 µs 1683.9 µs   5997.7 µs      35.6x Nisaba        5.44x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   37.3 µs  810.8 µs    848.0 µs 1590.8 µs 1518.0 µs   3108.9 µs      42.7x Nisaba        3.67x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)     111.5 µs 1178.1 µs   1289.6 µs 3331.1 µs 1699.0 µs   5030.1 µs      29.9x Nisaba        3.90x Nisaba
31. Stroke Join: Miter (12px Stroke)         21.6 µs  672.0 µs    693.6 µs 1104.5 µs 1289.1 µs   2393.6 µs      51.2x Nisaba        3.45x Nisaba
32. Stroke Join: Bevel (12px Stroke)         22.9 µs  711.4 µs    734.2 µs  918.1 µs 1349.8 µs   2267.9 µs      40.1x Nisaba        3.09x Nisaba
33. Stroke Join: Round (12px Stroke)         39.9 µs  735.9 µs    775.7 µs 1645.0 µs 1526.3 µs   3171.3 µs      41.3x Nisaba        4.09x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      23.3 µs  545.0 µs    568.4 µs  103.2 µs  865.6 µs    968.8 µs       4.4x Nisaba        1.70x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    32.5 µs  762.1 µs    794.6 µs  128.5 µs  958.4 µs   1086.9 µs       4.0x Nisaba        1.37x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     93.3 µs  870.9 µs    964.2 µs  179.7 µs  869.6 µs   1049.2 µs       1.9x Nisaba        1.09x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   39.0 µs  621.4 µs    660.5 µs 1553.5 µs 1524.9 µs   3078.4 µs      39.8x Nisaba        4.66x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   40.3 µs  652.4 µs    692.7 µs  166.8 µs  823.1 µs    989.9 µs       4.1x Nisaba        1.43x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   25.9 µs  778.0 µs    803.9 µs  437.4 µs 1153.4 µs   1590.8 µs      16.9x Nisaba        1.98x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   63.8 µs  883.5 µs    947.3 µs 1638.9 µs 1565.9 µs   3204.8 µs      25.7x Nisaba        3.38x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          36.9 µs  830.2 µs    867.1 µs   99.1 µs  824.0 µs    923.0 µs       2.7x Nisaba        1.06x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       34.5 µs  948.4 µs    982.9 µs  104.5 µs  948.6 µs   1053.1 µs       3.0x Nisaba        1.07x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         40.9 µs  873.3 µs    914.1 µs  161.5 µs 1017.1 µs   1178.6 µs       3.9x Nisaba        1.29x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     47.7 µs 1009.3 µs   1057.0 µs  137.6 µs  853.8 µs    991.5 µs       2.9x Nisaba          1.07x Skia
45. High-Density Disks Cloud (500 Disks)    173.6 µs  879.1 µs   1052.7 µs  726.8 µs 1434.8 µs   2161.7 µs       4.2x Nisaba        2.05x Nisaba
46. High-Density Stroked Rings (250 Rings)   98.0 µs  934.4 µs   1032.4 µs  454.9 µs 1265.6 µs   1720.5 µs       4.6x Nisaba        1.67x Nisaba
47. 250 Filled Rectangles Batch             106.6 µs  915.6 µs   1022.2 µs  454.6 µs 1301.8 µs   1756.4 µs       4.3x Nisaba        1.72x Nisaba
48. 150 Stroked Rectangles (2px Border)      99.4 µs  882.4 µs    981.8 µs  255.8 µs 1033.1 µs   1288.9 µs       2.6x Nisaba        1.31x Nisaba
49. 40 Crosshair Aim Reticles               233.4 µs 1005.1 µs   1238.5 µs  868.5 µs 1420.4 µs   2288.9 µs       3.7x Nisaba        1.85x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     176.4 µs 1191.6 µs   1368.0 µs  765.4 µs 1308.9 µs   2074.2 µs       4.3x Nisaba        1.52x Nisaba
51. 30 Vertical Card Linear Gradients        33.2 µs 1073.1 µs   1106.3 µs  397.3 µs 1173.8 µs   1571.1 µs      12.0x Nisaba        1.42x Nisaba
52. 30 Horizontal Bar Progress Gradients     36.1 µs 1031.8 µs   1067.9 µs  535.7 µs 1310.2 µs   1845.9 µs      14.9x Nisaba        1.73x Nisaba
53. 15 Concentric Radial Spotlights          32.8 µs  931.8 µs    964.6 µs  288.5 µs 1178.7 µs   1467.2 µs       8.8x Nisaba        1.52x Nisaba
54. 25 Soft Ambient Card Shadows             37.7 µs 1028.1 µs   1065.8 µs  440.2 µs 1368.6 µs   1808.8 µs      11.7x Nisaba        1.70x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      36.3 µs 1045.3 µs   1081.7 µs  278.1 µs 1313.6 µs   1591.7 µs       7.7x Nisaba        1.47x Nisaba
56. 16 Multi-Angle Gradient Fan Slices       76.8 µs  925.4 µs   1002.2 µs  342.1 µs 1156.6 µs   1498.7 µs       4.5x Nisaba        1.50x Nisaba
57. Full-Screen Diagonal Horizon Gradient    25.3 µs  829.9 µs    855.2 µs   95.0 µs  843.9 µs    938.9 µs       3.7x Nisaba        1.10x Nisaba
58. 20 Inset Well Shadows (Recessed)        169.6 µs 1177.4 µs   1347.0 µs  372.3 µs 1384.9 µs   1757.2 µs       2.2x Nisaba        1.30x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     33.5 µs 1484.5 µs   1518.0 µs  259.4 µs 1446.9 µs   1706.3 µs       7.7x Nisaba        1.12x Nisaba
60. Waveform Polyline Stroked with Gradient  122.7 µs  940.2 µs   1062.9 µs 1286.0 µs 1380.2 µs   2666.2 µs      10.5x Nisaba        2.51x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   25.1 µs  893.8 µs    919.0 µs   96.3 µs  904.7 µs   1001.0 µs       3.8x Nisaba        1.09x Nisaba
62. Scaled Image Quad (2x 512x512 Blit)      24.4 µs  767.8 µs    792.1 µs   86.7 µs  922.9 µs   1009.6 µs       3.6x Nisaba        1.27x Nisaba
63. 45° Rotated Pattern Fill in Circle      26.9 µs  827.9 µs    854.8 µs  101.6 µs  787.1 µs    888.7 µs       3.8x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   30.5 µs  963.5 µs    994.0 µs  112.6 µs  901.4 µs   1014.0 µs       3.7x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           35.3 µs  971.7 µs   1007.0 µs  129.4 µs  943.8 µs   1073.2 µs       3.7x Nisaba        1.07x Nisaba
66. Texture Pattern on Star Path             55.7 µs  834.2 µs    889.9 µs  352.9 µs 1054.2 µs   1407.1 µs       6.3x Nisaba        1.58x Nisaba
67. 20px Curved Path Stroked with Texture    25.5 µs  691.6 µs    717.1 µs  428.3 µs 1222.3 µs   1650.6 µs      16.8x Nisaba        2.30x Nisaba
68. Minified Texture Quad (0.25x Downscale)   42.6 µs  689.3 µs    732.0 µs  188.0 µs  920.0 µs   1108.0 µs       4.4x Nisaba        1.51x Nisaba
69. Pure Translation Stack (50 Cards)        58.3 µs  865.9 µs    924.1 µs  174.1 µs 1023.4 µs   1197.5 µs       3.0x Nisaba        1.30x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   30.9 µs  617.6 µs    648.5 µs  111.3 µs  807.3 µs    918.6 µs       3.6x Nisaba        1.42x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   58.4 µs  898.6 µs    957.0 µs  108.9 µs  830.6 µs    939.6 µs       1.9x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   42.4 µs  812.9 µs    855.3 µs  151.7 µs  979.8 µs   1131.6 µs       3.6x Nisaba        1.32x Nisaba
73. Combined Affine Transforms (30 Shapes)   44.2 µs  781.8 µs    826.0 µs  138.5 µs  944.8 µs   1083.3 µs       3.1x Nisaba        1.31x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   39.4 µs  860.5 µs    899.9 µs  121.3 µs  871.5 µs    992.8 µs       3.1x Nisaba        1.10x Nisaba
75. Reset Transform Stress (50 Cycles)       49.2 µs  762.8 µs    812.0 µs  179.8 µs  969.7 µs   1149.5 µs       3.7x Nisaba        1.42x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   35.9 µs  678.7 µs    714.6 µs  129.4 µs  999.2 µs   1128.6 µs       3.6x Nisaba        1.58x Nisaba
77. Multi-Scissor Grid (16 Viewports)        37.0 µs  900.3 µs    937.3 µs  179.8 µs  741.1 µs    920.8 µs       4.9x Nisaba         Tie (~1.0x)
78. Hierarchical Intersecting Scissors (4 Levels)   38.0 µs  875.5 µs    913.5 µs  123.3 µs 1012.7 µs   1136.1 µs       3.2x Nisaba        1.24x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  270.5 µs 1227.5 µs   1498.0 µs 1987.6 µs 1332.3 µs   3319.9 µs       7.3x Nisaba        2.22x Nisaba
80. Scissored Gradient Card (Clipped)        26.9 µs  865.9 µs    892.8 µs  341.5 µs 1122.0 µs   1463.5 µs      12.7x Nisaba        1.64x Nisaba
81. Circular Overflow Clip (25 Boxes)        55.5 µs  871.4 µs    927.0 µs  393.7 µs 1162.8 µs   1556.5 µs       7.1x Nisaba        1.68x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   53.1 µs 1028.0 µs   1081.2 µs  205.2 µs 1059.5 µs   1264.7 µs       3.9x Nisaba        1.17x Nisaba
83. Composite Op: Source-Over (30 Shapes)    57.8 µs  913.5 µs    971.3 µs  194.0 µs 1151.1 µs   1345.1 µs       3.4x Nisaba        1.38x Nisaba
84. Composite Op: Lighter / Plus (30 Particles)   52.6 µs  745.2 µs    797.8 µs  211.4 µs 1157.2 µs   1368.6 µs       4.0x Nisaba        1.72x Nisaba
85. Composite Op: Source-In (Alpha Masking)   52.8 µs 1011.0 µs   1063.8 µs  192.8 µs 1393.8 µs   1586.6 µs       3.7x Nisaba        1.49x Nisaba
86. Composite Op: Source-Out (Cutout)        51.5 µs  909.0 µs    960.4 µs  196.3 µs 1321.4 µs   1517.7 µs       3.8x Nisaba        1.58x Nisaba
87. Composite Op: Atop (Target Bounds)       54.5 µs 1010.8 µs   1065.2 µs  180.0 µs  977.7 µs   1157.7 µs       3.3x Nisaba        1.09x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   50.8 µs  946.4 µs    997.2 µs  193.0 µs 1214.4 µs   1407.4 µs       3.8x Nisaba        1.41x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     50.6 µs  995.2 µs   1045.8 µs  191.5 µs 1194.3 µs   1385.9 µs       3.8x Nisaba        1.33x Nisaba
90. Composite Op: Dest-Atop (Inverse)        49.9 µs  866.1 µs    916.0 µs  192.8 µs 1212.0 µs   1404.8 µs       3.9x Nisaba        1.53x Nisaba
91. Composite Op: Xor (Exclusive Blend)      51.2 µs  844.0 µs    895.2 µs  193.8 µs 1123.7 µs   1317.5 µs       3.8x Nisaba        1.47x Nisaba
92. Composite Op: Copy (Direct Overwrite)    54.8 µs  940.6 µs    995.4 µs  186.1 µs 1190.0 µs   1376.1 µs       3.4x Nisaba        1.38x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)  101.8 µs 1151.2 µs   1252.9 µs 1117.7 µs 1567.9 µs   2685.6 µs      11.0x Nisaba        2.14x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     139.1 µs 1254.4 µs   1393.5 µs 1568.6 µs 1722.4 µs   3291.1 µs      11.3x Nisaba        2.36x Nisaba
95. Circular Progress Rings (8 Meters)      174.6 µs 1082.9 µs   1257.5 µs 2139.6 µs 1842.5 µs   3982.1 µs      12.3x Nisaba        3.17x Nisaba
96. Modern Card Stack (5 Elevated Cards)     69.5 µs 1416.8 µs   1486.3 µs  411.9 µs 1567.5 µs   1979.4 µs       5.9x Nisaba        1.33x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   55.2 µs  674.7 µs    729.9 µs  101.4 µs  869.1 µs    970.5 µs       1.8x Nisaba        1.33x Nisaba
98. Floating Action Button (FAB 10 Buttons)   84.8 µs 1003.2 µs   1088.0 µs  320.1 µs 1235.4 µs   1555.5 µs       3.8x Nisaba        1.43x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   67.4 µs  893.4 µs    960.8 µs  306.8 µs 1145.2 µs   1452.0 µs       4.5x Nisaba        1.51x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   56.7 µs 1089.1 µs   1145.7 µs  275.1 µs 1314.0 µs   1589.1 µs       4.9x Nisaba        1.39x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          7535.3 µs95199.6 µs 102734.9 µs62665.7 µs121547.7 µs 184213.3 µs      8.32x Nisaba        1.79x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 91 suites | Skia won in 1 suites | Ties: 8
         Overall Frame Throughput (End-to-End): 1.79x Nisaba
         Overall CPU Command Submission:       8.32x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 544.0 µs, Skia: 515.1 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 683.4 µs, Skia: 805.1 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 776.9 µs, Skia: 1859.7 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 796.1 µs, Skia: 819.3 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 910.8 µs, Skia: 1060.8 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 981.9 µs, Skia: 1075.4 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 973.4 µs, Skia: 1037.3 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 605.4 µs, Skia: 900.0 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 939.3 µs, Skia: 1346.7 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 898.0 µs, Skia: 1074.0 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1111.4 µs, Skia: 2095.9 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1190.9 µs, Skia: 1950.4 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2656.2 µs, Skia: 2171.2 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1104.4 µs, Skia: 1584.3 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1384.4 µs, Skia: 2615.6 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 833.4 µs, Skia: 3282.4 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1064.9 µs, Skia: 3050.1 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 957.0 µs, Skia: 1253.9 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 922.4 µs, Skia: 1342.7 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1414.3 µs, Skia: 1397.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1688.1 µs, Skia: 2540.7 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 730.1 µs, Skia: 3587.4 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 608.1 µs, Skia: 1408.6 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 569.4 µs, Skia: 2990.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1011.4 µs, Skia: 1739.5 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1646.9 µs, Skia: 2667.5 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 786.6 µs, Skia: 2426.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 801.9 µs, Skia: 3283.5 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 644.9 µs, Skia: 1926.3 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1234.9 µs, Skia: 3859.3 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 635.4 µs, Skia: 2310.7 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 704.4 µs, Skia: 2207.0 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 711.7 µs, Skia: 2379.8 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 822.6 µs, Skia: 1127.5 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 719.1 µs, Skia: 1180.9 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1200.1 µs, Skia: 1090.1 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 856.9 µs, Skia: 3171.6 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 643.8 µs, Skia: 1013.1 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 801.1 µs, Skia: 2180.9 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1033.2 µs, Skia: 2563.4 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 735.6 µs, Skia: 899.8 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1300.1 µs, Skia: 1178.5 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 902.2 µs, Skia: 1543.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1211.0 µs, Skia: 1509.3 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1487.7 µs, Skia: 2388.4 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1038.6 µs, Skia: 1535.6 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 844.9 µs, Skia: 1576.4 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 819.6 µs, Skia: 1360.7 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1123.2 µs, Skia: 2130.7 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1532.6 µs, Skia: 1889.5 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1004.8 µs, Skia: 1651.0 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1123.8 µs, Skia: 1623.9 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 815.1 µs, Skia: 1439.0 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1260.5 µs, Skia: 1979.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1023.0 µs, Skia: 1941.1 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1408.0 µs, Skia: 1920.3 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1024.2 µs, Skia: 1307.8 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1722.5 µs, Skia: 2064.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1571.1 µs, Skia: 1795.2 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1424.4 µs, Skia: 4539.8 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1028.8 µs, Skia: 961.9 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 830.1 µs, Skia: 922.8 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 871.5 µs, Skia: 1016.7 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 901.0 µs, Skia: 1144.3 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 818.0 µs, Skia: 1244.3 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1085.7 µs, Skia: 2113.5 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 976.5 µs, Skia: 2510.4 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1128.9 µs, Skia: 1557.1 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1173.9 µs, Skia: 1554.1 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 990.8 µs, Skia: 1476.0 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1010.2 µs, Skia: 1398.6 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 1052.2 µs, Skia: 1479.9 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 987.7 µs, Skia: 1784.8 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 1148.5 µs, Skia: 1340.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 1044.4 µs, Skia: 1403.0 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 934.1 µs, Skia: 1243.1 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 1007.9 µs, Skia: 1438.0 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1052.5 µs, Skia: 1415.4 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1733.6 µs, Skia: 3950.9 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 826.9 µs, Skia: 1099.5 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 905.6 µs, Skia: 1154.3 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 765.1 µs, Skia: 953.2 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 842.4 µs, Skia: 788.8 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 892.2 µs, Skia: 1325.8 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 900.9 µs, Skia: 1012.2 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 729.8 µs, Skia: 1382.5 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 835.9 µs, Skia: 1167.1 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1010.7 µs, Skia: 1356.1 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 658.6 µs, Skia: 753.3 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 952.9 µs, Skia: 1333.3 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 1064.9 µs, Skia: 1237.4 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 1018.8 µs, Skia: 831.8 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1086.4 µs, Skia: 2160.3 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1100.8 µs, Skia: 2602.5 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1147.2 µs, Skia: 2827.8 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1417.5 µs, Skia: 2018.1 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 954.2 µs, Skia: 1144.0 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1196.4 µs, Skia: 1590.2 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 754.7 µs, Skia: 1212.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1192.9 µs, Skia: 1038.2 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.7 µs  540.2 µs    544.0 µs   16.3 µs  498.8 µs    515.1 µs       4.4x Nisaba          1.06x Skia
2. 100 Grid Lines (1px Hairlines)            27.8 µs  655.6 µs    683.4 µs   81.9 µs  723.2 µs    805.1 µs       2.9x Nisaba        1.18x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      52.1 µs  724.8 µs    776.9 µs  575.1 µs 1284.6 µs   1859.7 µs      11.0x Nisaba        2.39x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      42.9 µs  753.3 µs    796.1 µs  118.6 µs  700.6 µs    819.3 µs       2.8x Nisaba         Tie (~1.0x)
5. 100 Alpha Rectangles (Overlapping Blend)   38.9 µs  871.9 µs    910.8 µs  124.9 µs  935.8 µs   1060.8 µs       3.2x Nisaba        1.16x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     33.2 µs  948.7 µs    981.9 µs  150.0 µs  925.3 µs   1075.4 µs       4.5x Nisaba        1.10x Nisaba
7. 50 Varying-Radius Rounded Rects           41.0 µs  932.4 µs    973.4 µs  124.6 µs  912.7 µs   1037.3 µs       3.0x Nisaba        1.07x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   32.6 µs  572.9 µs    605.4 µs  101.2 µs  798.7 µs    900.0 µs       3.1x Nisaba        1.49x Nisaba
9. 100 Alpha Circles / Disks (Fill)          59.1 µs  880.2 µs    939.3 µs  259.4 µs 1087.3 µs   1346.7 µs       4.4x Nisaba        1.43x Nisaba
10. 25 Concentric Rings (2px Stroke)         81.7 µs  816.2 µs    898.0 µs  103.6 µs  970.4 µs   1074.0 µs       1.3x Nisaba        1.20x Nisaba
11. 50 Two-Stop Linear Gradients             50.2 µs 1061.2 µs   1111.4 µs  643.8 µs 1452.1 µs   2095.9 µs      12.8x Nisaba        1.89x Nisaba
12. 50 Diagonal Gradients (45° Angle)       49.2 µs 1141.7 µs   1190.9 µs  604.9 µs 1345.4 µs   1950.4 µs      12.3x Nisaba        1.64x Nisaba
13. 25 Radial Glow Gradients (300px)         52.3 µs 2603.9 µs   2656.2 µs  296.2 µs 1875.0 µs   2171.2 µs       5.7x Nisaba          1.22x Skia
14. 20 Box Blur / Drop Shadows (15px)        33.5 µs 1070.9 µs   1104.4 µs  301.1 µs 1283.2 µs   1584.3 µs       9.0x Nisaba        1.43x Nisaba
15. 10 Concave 10-Point Stars (Fill)        134.0 µs 1250.4 µs   1384.4 µs 1062.2 µs 1553.5 µs   2615.6 µs       7.9x Nisaba        1.89x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   54.9 µs  778.6 µs    833.4 µs 1809.2 µs 1473.2 µs   3282.4 µs      33.0x Nisaba        3.94x Nisaba
17. 1000-Point Waveform (Dense Stroke)      166.8 µs  898.2 µs   1064.9 µs 1543.7 µs 1506.4 µs   3050.1 µs       9.3x Nisaba        2.86x Nisaba
18. 50 Rotated Cards (Affine Transforms)     47.7 µs  909.3 µs    957.0 µs  191.8 µs 1062.0 µs   1253.9 µs       4.0x Nisaba        1.31x Nisaba
19. 25 Scissor Clipped Viewports             38.4 µs  884.0 µs    922.4 µs  217.6 µs 1125.1 µs   1342.7 µs       5.7x Nisaba        1.46x Nisaba
20. 50 UI Text Labels (Inter-Regular)       320.1 µs 1094.2 µs   1414.3 µs  302.0 µs 1095.6 µs   1397.6 µs         1.1x Skia         Tie (~1.0x)
21. 25 Quadratic Bezier Curves (Fill+Stroke)  290.3 µs 1397.8 µs   1688.1 µs 1034.0 µs 1506.8 µs   2540.7 µs       3.6x Nisaba        1.51x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     52.6 µs  677.5 µs    730.1 µs 2020.5 µs 1566.9 µs   3587.4 µs      38.4x Nisaba        4.91x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       18.7 µs  589.4 µs    608.1 µs  326.6 µs 1081.9 µs   1408.6 µs      17.5x Nisaba        2.32x Nisaba
24. Circular Arcs with arcTo (20 Corners)    44.4 µs  525.0 µs    569.4 µs 1440.3 µs 1550.0 µs   2990.3 µs      32.4x Nisaba        5.25x Nisaba
25. Directional Arcs with arc() (30 Arcs)   112.2 µs  899.2 µs   1011.4 µs  492.9 µs 1246.6 µs   1739.5 µs       4.4x Nisaba        1.72x Nisaba
26. Compound Path with Holes (15 Donuts)    243.5 µs 1403.4 µs   1646.9 µs 1198.6 µs 1468.9 µs   2667.5 µs       4.9x Nisaba        1.62x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    69.4 µs  717.2 µs    786.6 µs 1026.3 µs 1400.1 µs   2426.4 µs      14.8x Nisaba        3.08x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    84.6 µs  717.3 µs    801.9 µs 1820.1 µs 1463.4 µs   3283.5 µs      21.5x Nisaba        4.09x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   17.4 µs  627.5 µs    644.9 µs  639.7 µs 1286.5 µs   1926.3 µs      36.8x Nisaba        2.99x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      71.1 µs 1163.7 µs   1234.9 µs 2086.7 µs 1772.7 µs   3859.3 µs      29.3x Nisaba        3.13x Nisaba
31. Stroke Join: Miter (12px Stroke)         17.0 µs  618.3 µs    635.4 µs  975.2 µs 1335.5 µs   2310.7 µs      57.2x Nisaba        3.64x Nisaba
32. Stroke Join: Bevel (12px Stroke)         20.2 µs  684.2 µs    704.4 µs  962.3 µs 1244.7 µs   2207.0 µs      47.5x Nisaba        3.13x Nisaba
33. Stroke Join: Round (12px Stroke)         27.4 µs  684.3 µs    711.7 µs 1012.7 µs 1367.0 µs   2379.8 µs      36.9x Nisaba        3.34x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      39.5 µs  783.1 µs    822.6 µs  147.2 µs  980.3 µs   1127.5 µs       3.7x Nisaba        1.37x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    33.7 µs  685.4 µs    719.1 µs  173.8 µs 1007.2 µs   1180.9 µs       5.2x Nisaba        1.64x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    138.8 µs 1061.3 µs   1200.1 µs  194.6 µs  895.5 µs   1090.1 µs       1.4x Nisaba          1.10x Skia
37. Miter Limit Clamping (10° Acute Spikes)   36.0 µs  820.9 µs    856.9 µs 1666.6 µs 1505.0 µs   3171.6 µs      46.3x Nisaba        3.70x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   30.1 µs  613.7 µs    643.8 µs  144.6 µs  868.5 µs   1013.1 µs       4.8x Nisaba        1.57x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   32.3 µs  768.8 µs    801.1 µs  761.7 µs 1419.2 µs   2180.9 µs      23.6x Nisaba        2.72x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   84.2 µs  949.0 µs   1033.2 µs 1079.2 µs 1484.1 µs   2563.4 µs      12.8x Nisaba        2.48x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          25.4 µs  710.2 µs    735.6 µs  103.5 µs  796.3 µs    899.8 µs       4.1x Nisaba        1.22x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       63.1 µs 1237.1 µs   1300.1 µs  127.4 µs 1051.1 µs   1178.5 µs       2.0x Nisaba          1.10x Skia
43. 50 Pill / Capsule Badges (r=h/2)         64.4 µs  837.8 µs    902.2 µs  264.7 µs 1278.9 µs   1543.6 µs       4.1x Nisaba        1.71x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     76.3 µs 1134.7 µs   1211.0 µs  227.7 µs 1281.6 µs   1509.3 µs       3.0x Nisaba        1.25x Nisaba
45. High-Density Disks Cloud (500 Disks)    291.1 µs 1196.6 µs   1487.7 µs  936.7 µs 1451.7 µs   2388.4 µs       3.2x Nisaba        1.61x Nisaba
46. High-Density Stroked Rings (250 Rings)  102.1 µs  936.6 µs   1038.6 µs  419.2 µs 1116.4 µs   1535.6 µs       4.1x Nisaba        1.48x Nisaba
47. 250 Filled Rectangles Batch              70.6 µs  774.3 µs    844.9 µs  358.4 µs 1217.9 µs   1576.4 µs       5.1x Nisaba        1.87x Nisaba
48. 150 Stroked Rectangles (2px Border)      84.6 µs  734.9 µs    819.6 µs  238.3 µs 1122.4 µs   1360.7 µs       2.8x Nisaba        1.66x Nisaba
49. 40 Crosshair Aim Reticles               202.0 µs  921.2 µs   1123.2 µs  772.9 µs 1357.8 µs   2130.7 µs       3.8x Nisaba        1.90x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     209.1 µs 1323.5 µs   1532.6 µs  645.1 µs 1244.4 µs   1889.5 µs       3.1x Nisaba        1.23x Nisaba
51. 30 Vertical Card Linear Gradients        37.0 µs  967.7 µs   1004.8 µs  450.8 µs 1200.2 µs   1651.0 µs      12.2x Nisaba        1.64x Nisaba
52. 30 Horizontal Bar Progress Gradients     34.6 µs 1089.2 µs   1123.8 µs  412.9 µs 1211.0 µs   1623.9 µs      11.9x Nisaba        1.44x Nisaba
53. 15 Concentric Radial Spotlights          24.9 µs  790.2 µs    815.1 µs  290.7 µs 1148.2 µs   1439.0 µs      11.7x Nisaba        1.77x Nisaba
54. 25 Soft Ambient Card Shadows             56.2 µs 1204.3 µs   1260.5 µs  533.3 µs 1446.2 µs   1979.5 µs       9.5x Nisaba        1.57x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      46.4 µs  976.5 µs   1023.0 µs  456.4 µs 1484.8 µs   1941.1 µs       9.8x Nisaba        1.90x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      117.3 µs 1290.6 µs   1408.0 µs  537.5 µs 1382.8 µs   1920.3 µs       4.6x Nisaba        1.36x Nisaba
57. Full-Screen Diagonal Horizon Gradient    34.0 µs  990.3 µs   1024.2 µs  152.0 µs 1155.8 µs   1307.8 µs       4.5x Nisaba        1.28x Nisaba
58. 20 Inset Well Shadows (Recessed)        243.8 µs 1478.6 µs   1722.5 µs  557.4 µs 1507.1 µs   2064.5 µs       2.3x Nisaba        1.20x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     54.5 µs 1516.6 µs   1571.1 µs  316.2 µs 1479.1 µs   1795.2 µs       5.8x Nisaba        1.14x Nisaba
60. Waveform Polyline Stroked with Gradient  208.7 µs 1215.7 µs   1424.4 µs 2816.3 µs 1723.5 µs   4539.8 µs      13.5x Nisaba        3.19x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   28.8 µs 1000.0 µs   1028.8 µs   93.9 µs  868.0 µs    961.9 µs       3.3x Nisaba          1.07x Skia
62. Scaled Image Quad (2x 512x512 Blit)      26.0 µs  804.1 µs    830.1 µs   76.8 µs  846.0 µs    922.8 µs       3.0x Nisaba        1.11x Nisaba
63. 45° Rotated Pattern Fill in Circle      21.5 µs  850.0 µs    871.5 µs  116.5 µs  900.3 µs   1016.7 µs       5.4x Nisaba        1.17x Nisaba
64. Translucent Texture Overlay (50% Alpha)   32.3 µs  868.7 µs    901.0 µs  145.6 µs  998.8 µs   1144.3 µs       4.5x Nisaba        1.27x Nisaba
65. Multi-Avatar Grid (16 Avatars)           33.1 µs  784.9 µs    818.0 µs  150.5 µs 1093.8 µs   1244.3 µs       4.5x Nisaba        1.52x Nisaba
66. Texture Pattern on Star Path             76.1 µs 1009.6 µs   1085.7 µs  756.7 µs 1356.8 µs   2113.5 µs       9.9x Nisaba        1.95x Nisaba
67. 20px Curved Path Stroked with Texture    54.3 µs  922.1 µs    976.5 µs  955.7 µs 1554.7 µs   2510.4 µs      17.6x Nisaba        2.57x Nisaba
68. Minified Texture Quad (0.25x Downscale)   83.0 µs 1045.9 µs   1128.9 µs  390.1 µs 1167.0 µs   1557.1 µs       4.7x Nisaba        1.38x Nisaba
69. Pure Translation Stack (50 Cards)        91.3 µs 1082.6 µs   1173.9 µs  299.8 µs 1254.4 µs   1554.1 µs       3.3x Nisaba        1.32x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   66.3 µs  924.5 µs    990.8 µs  258.7 µs 1217.3 µs   1476.0 µs       3.9x Nisaba        1.49x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   87.7 µs  922.5 µs   1010.2 µs  160.7 µs 1237.9 µs   1398.6 µs       1.8x Nisaba        1.38x Nisaba
72. Shear / Skew Parallelograms (30 Quads)   57.1 µs  995.1 µs   1052.2 µs  242.2 µs 1237.6 µs   1479.9 µs       4.2x Nisaba        1.41x Nisaba
73. Combined Affine Transforms (30 Shapes)   65.3 µs  922.5 µs    987.7 µs  203.4 µs 1581.4 µs   1784.8 µs       3.1x Nisaba        1.81x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   56.3 µs 1092.2 µs   1148.5 µs  189.4 µs 1151.4 µs   1340.8 µs       3.4x Nisaba        1.17x Nisaba
75. Reset Transform Stress (50 Cycles)       72.2 µs  972.2 µs   1044.4 µs  255.5 µs 1147.5 µs   1403.0 µs       3.5x Nisaba        1.34x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   52.3 µs  881.7 µs    934.1 µs  187.2 µs 1055.8 µs   1243.1 µs       3.6x Nisaba        1.33x Nisaba
77. Multi-Scissor Grid (16 Viewports)        50.3 µs  957.5 µs   1007.9 µs  319.0 µs 1119.0 µs   1438.0 µs       6.3x Nisaba        1.43x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   51.5 µs 1001.0 µs   1052.5 µs  190.4 µs 1224.9 µs   1415.4 µs       3.7x Nisaba        1.34x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  414.7 µs 1318.9 µs   1733.6 µs 2372.1 µs 1578.8 µs   3950.9 µs       5.7x Nisaba        2.28x Nisaba
80. Scissored Gradient Card (Clipped)        18.9 µs  808.0 µs    826.9 µs  201.4 µs  898.1 µs   1099.5 µs      10.6x Nisaba        1.33x Nisaba
81. Circular Overflow Clip (25 Boxes)        53.1 µs  852.4 µs    905.6 µs  234.9 µs  919.5 µs   1154.3 µs       4.4x Nisaba        1.27x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   28.0 µs  737.2 µs    765.1 µs  106.2 µs  847.1 µs    953.2 µs       3.8x Nisaba        1.25x Nisaba
83. Composite Op: Source-Over (30 Shapes)    30.9 µs  811.5 µs    842.4 µs  105.9 µs  682.9 µs    788.8 µs       3.4x Nisaba          1.07x Skia
84. Composite Op: Lighter / Plus (30 Particles)   37.5 µs  854.7 µs    892.2 µs  196.9 µs 1128.9 µs   1325.8 µs       5.3x Nisaba        1.49x Nisaba
85. Composite Op: Source-In (Alpha Masking)   48.2 µs  852.7 µs    900.9 µs   92.4 µs  919.8 µs   1012.2 µs       1.9x Nisaba        1.12x Nisaba
86. Composite Op: Source-Out (Cutout)        31.9 µs  697.9 µs    729.8 µs  133.5 µs 1249.0 µs   1382.5 µs       4.2x Nisaba        1.89x Nisaba
87. Composite Op: Atop (Target Bounds)       30.7 µs  805.2 µs    835.9 µs  146.5 µs 1020.6 µs   1167.1 µs       4.8x Nisaba        1.40x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   41.9 µs  968.8 µs   1010.7 µs  192.7 µs 1163.4 µs   1356.1 µs       4.6x Nisaba        1.34x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     27.2 µs  631.4 µs    658.6 µs   92.4 µs  660.9 µs    753.3 µs       3.4x Nisaba        1.14x Nisaba
90. Composite Op: Dest-Atop (Inverse)        44.0 µs  908.9 µs    952.9 µs  159.9 µs 1173.4 µs   1333.3 µs       3.6x Nisaba        1.40x Nisaba
91. Composite Op: Xor (Exclusive Blend)      44.5 µs 1020.4 µs   1064.9 µs  170.6 µs 1066.8 µs   1237.4 µs       3.8x Nisaba        1.16x Nisaba
92. Composite Op: Copy (Direct Overwrite)    52.9 µs  966.0 µs   1018.8 µs   85.9 µs  745.9 µs    831.8 µs       1.6x Nisaba          1.22x Skia
93. UI Dashboard Gauge (Arc Track + Needle)   81.3 µs 1005.1 µs   1086.4 µs  719.0 µs 1441.3 µs   2160.3 µs       8.8x Nisaba        1.99x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      92.7 µs 1008.1 µs   1100.8 µs 1005.5 µs 1597.0 µs   2602.5 µs      10.8x Nisaba        2.36x Nisaba
95. Circular Progress Rings (8 Meters)      137.8 µs 1009.5 µs   1147.2 µs 1281.4 µs 1546.4 µs   2827.8 µs       9.3x Nisaba        2.46x Nisaba
96. Modern Card Stack (5 Elevated Cards)     54.8 µs 1362.8 µs   1417.5 µs  397.5 µs 1620.6 µs   2018.1 µs       7.3x Nisaba        1.42x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   89.0 µs  865.1 µs    954.2 µs  128.4 µs 1015.6 µs   1144.0 µs       1.4x Nisaba        1.20x Nisaba
98. Floating Action Button (FAB 10 Buttons)  111.5 µs 1085.0 µs   1196.4 µs  309.4 µs 1280.8 µs   1590.2 µs       2.8x Nisaba        1.33x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   55.9 µs  698.8 µs    754.7 µs  238.5 µs  973.5 µs   1212.1 µs       4.3x Nisaba        1.61x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   53.7 µs 1139.3 µs   1192.9 µs  111.9 µs  926.3 µs   1038.2 µs       2.1x Nisaba          1.15x Skia
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          7456.4 µs94547.0 µs 102003.4 µs51924.5 µs120186.3 µs 172110.8 µs      6.96x Nisaba        1.69x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 90 suites | Skia won in 8 suites | Ties: 2
         Overall Frame Throughput (End-to-End): 1.69x Nisaba
         Overall CPU Command Submission:       6.96x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 686.1 µs, Skia: 503.6 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 581.2 µs, Skia: 1310.4 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1192.9 µs, Skia: 2032.9 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 773.4 µs, Skia: 1038.0 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1158.3 µs, Skia: 1227.6 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 887.2 µs, Skia: 1020.0 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 895.2 µs, Skia: 1064.5 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 741.0 µs, Skia: 1169.8 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 911.2 µs, Skia: 1224.5 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1241.9 µs, Skia: 1183.6 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1438.0 µs, Skia: 2082.3 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1155.7 µs, Skia: 2001.0 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2629.5 µs, Skia: 2365.8 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1036.2 µs, Skia: 1535.7 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1005.4 µs, Skia: 2449.2 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 750.3 µs, Skia: 3030.1 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 984.4 µs, Skia: 2961.0 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 909.7 µs, Skia: 1590.3 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1114.4 µs, Skia: 795.6 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1122.3 µs, Skia: 1269.9 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1355.5 µs, Skia: 2501.0 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 767.0 µs, Skia: 3636.8 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 920.0 µs, Skia: 1906.0 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 685.2 µs, Skia: 2208.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 920.1 µs, Skia: 1727.6 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1414.5 µs, Skia: 2390.8 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 856.5 µs, Skia: 2367.7 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 794.0 µs, Skia: 2996.2 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 700.2 µs, Skia: 2294.6 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1030.9 µs, Skia: 3965.1 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 922.3 µs, Skia: 2469.3 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 796.2 µs, Skia: 2799.7 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 747.8 µs, Skia: 2637.9 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 705.7 µs, Skia: 1131.9 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 778.5 µs, Skia: 994.2 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 893.8 µs, Skia: 1192.9 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 783.4 µs, Skia: 2783.7 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 631.4 µs, Skia: 1022.4 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 832.8 µs, Skia: 2018.4 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1050.4 µs, Skia: 2944.1 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 707.5 µs, Skia: 967.3 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1145.7 µs, Skia: 1443.9 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 976.6 µs, Skia: 1235.8 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1110.6 µs, Skia: 1429.6 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1522.4 µs, Skia: 2445.1 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1101.5 µs, Skia: 1803.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 970.0 µs, Skia: 1286.0 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 651.6 µs, Skia: 1389.3 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1449.1 µs, Skia: 2016.8 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1925.4 µs, Skia: 2234.2 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1114.3 µs, Skia: 1679.7 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1016.9 µs, Skia: 2149.2 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1077.8 µs, Skia: 1250.5 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1031.4 µs, Skia: 2079.6 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 1129.9 µs, Skia: 1824.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1330.7 µs, Skia: 1957.2 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1097.6 µs, Skia: 1100.5 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1844.0 µs, Skia: 2193.0 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1628.0 µs, Skia: 2073.5 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1375.7 µs, Skia: 3852.0 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 750.6 µs, Skia: 743.1 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 794.8 µs, Skia: 1097.6 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 801.3 µs, Skia: 749.8 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 838.7 µs, Skia: 948.9 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 816.1 µs, Skia: 1096.8 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 909.1 µs, Skia: 1497.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 916.9 µs, Skia: 2048.4 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 756.4 µs, Skia: 1404.2 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1107.4 µs, Skia: 1611.7 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 705.5 µs, Skia: 956.0 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1037.3 µs, Skia: 806.3 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 586.2 µs, Skia: 758.2 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 889.7 µs, Skia: 902.2 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 972.7 µs, Skia: 1222.5 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 721.3 µs, Skia: 1122.4 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 975.0 µs, Skia: 896.8 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 964.1 µs, Skia: 798.7 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 799.9 µs, Skia: 1288.6 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1363.3 µs, Skia: 3157.8 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1118.8 µs, Skia: 1158.2 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 527.1 µs, Skia: 1008.1 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 751.8 µs, Skia: 801.1 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 784.4 µs, Skia: 938.4 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 780.9 µs, Skia: 915.5 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 804.9 µs, Skia: 1381.1 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 720.4 µs, Skia: 1574.9 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 878.4 µs, Skia: 1147.9 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 804.7 µs, Skia: 914.4 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 824.3 µs, Skia: 1044.0 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 860.3 µs, Skia: 1115.2 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 681.1 µs, Skia: 1126.5 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 803.7 µs, Skia: 874.6 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 801.8 µs, Skia: 2016.6 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1361.1 µs, Skia: 1791.4 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 816.0 µs, Skia: 2375.4 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1327.0 µs, Skia: 1596.3 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 982.5 µs, Skia: 1308.7 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1235.6 µs, Skia: 1852.4 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1075.4 µs, Skia: 1659.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1107.3 µs, Skia: 1550.5 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          5.1 µs  681.0 µs    686.1 µs   17.2 µs  486.3 µs    503.6 µs       3.4x Nisaba          1.36x Skia
2. 100 Grid Lines (1px Hairlines)            29.8 µs  551.4 µs    581.2 µs  142.6 µs 1167.8 µs   1310.4 µs       4.8x Nisaba        2.25x Nisaba
3. 20 Thick Diagonal Lines (16px Round)     122.2 µs 1070.7 µs   1192.9 µs  688.1 µs 1344.8 µs   2032.9 µs       5.6x Nisaba        1.70x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      56.6 µs  716.9 µs    773.4 µs  164.1 µs  873.9 µs   1038.0 µs       2.9x Nisaba        1.34x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   53.3 µs 1105.0 µs   1158.3 µs  168.6 µs 1059.0 µs   1227.6 µs       3.2x Nisaba        1.06x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     31.9 µs  855.3 µs    887.2 µs  125.3 µs  894.7 µs   1020.0 µs       3.9x Nisaba        1.15x Nisaba
7. 50 Varying-Radius Rounded Rects           37.1 µs  858.1 µs    895.2 µs  120.8 µs  943.7 µs   1064.5 µs       3.3x Nisaba        1.19x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   50.3 µs  690.7 µs    741.0 µs  158.6 µs 1011.1 µs   1169.8 µs       3.2x Nisaba        1.58x Nisaba
9. 100 Alpha Circles / Disks (Fill)          55.1 µs  856.1 µs    911.2 µs  228.7 µs  995.8 µs   1224.5 µs       4.2x Nisaba        1.34x Nisaba
10. 25 Concentric Rings (2px Stroke)        149.5 µs 1092.4 µs   1241.9 µs  139.0 µs 1044.6 µs   1183.6 µs         1.1x Skia         Tie (~1.0x)
11. 50 Two-Stop Linear Gradients             78.7 µs 1359.3 µs   1438.0 µs  701.7 µs 1380.6 µs   2082.3 µs       8.9x Nisaba        1.45x Nisaba
12. 50 Diagonal Gradients (45° Angle)       53.7 µs 1102.0 µs   1155.7 µs  669.8 µs 1331.2 µs   2001.0 µs      12.5x Nisaba        1.73x Nisaba
13. 25 Radial Glow Gradients (300px)         54.4 µs 2575.1 µs   2629.5 µs  382.0 µs 1983.8 µs   2365.8 µs       7.0x Nisaba          1.11x Skia
14. 20 Box Blur / Drop Shadows (15px)        28.7 µs 1007.6 µs   1036.2 µs  287.8 µs 1247.8 µs   1535.7 µs      10.0x Nisaba        1.48x Nisaba
15. 10 Concave 10-Point Stars (Fill)        102.4 µs  903.1 µs   1005.4 µs 1070.1 µs 1379.1 µs   2449.2 µs      10.5x Nisaba        2.44x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   63.6 µs  686.7 µs    750.3 µs 1515.7 µs 1514.4 µs   3030.1 µs      23.8x Nisaba        4.04x Nisaba
17. 1000-Point Waveform (Dense Stroke)      123.4 µs  861.0 µs    984.4 µs 1464.3 µs 1496.6 µs   2961.0 µs      11.9x Nisaba        3.01x Nisaba
18. 50 Rotated Cards (Affine Transforms)     35.9 µs  873.8 µs    909.7 µs  254.3 µs 1336.0 µs   1590.3 µs       7.1x Nisaba        1.75x Nisaba
19. 25 Scissor Clipped Viewports             52.4 µs 1062.1 µs   1114.4 µs  116.2 µs  679.3 µs    795.6 µs       2.2x Nisaba          1.40x Skia
20. 50 UI Text Labels (Inter-Regular)       193.7 µs  928.6 µs   1122.3 µs  225.1 µs 1044.7 µs   1269.9 µs       1.2x Nisaba        1.13x Nisaba
21. 25 Quadratic Bezier Curves (Fill+Stroke)  205.0 µs 1150.5 µs   1355.5 µs  941.5 µs 1559.5 µs   2501.0 µs       4.6x Nisaba        1.85x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     69.4 µs  697.6 µs    767.0 µs 2076.1 µs 1560.6 µs   3636.8 µs      29.9x Nisaba        4.74x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       35.5 µs  884.5 µs    920.0 µs  703.0 µs 1203.0 µs   1906.0 µs      19.8x Nisaba        2.07x Nisaba
24. Circular Arcs with arcTo (20 Corners)    37.6 µs  647.6 µs    685.2 µs  839.7 µs 1368.6 µs   2208.3 µs      22.3x Nisaba        3.22x Nisaba
25. Directional Arcs with arc() (30 Arcs)    90.2 µs  829.9 µs    920.1 µs  437.1 µs 1290.4 µs   1727.6 µs       4.8x Nisaba        1.88x Nisaba
26. Compound Path with Holes (15 Donuts)    184.9 µs 1229.7 µs   1414.5 µs 1096.8 µs 1294.0 µs   2390.8 µs       5.9x Nisaba        1.69x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    82.3 µs  774.2 µs    856.5 µs 1010.5 µs 1357.2 µs   2367.7 µs      12.3x Nisaba        2.76x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    75.0 µs  719.1 µs    794.0 µs 1521.7 µs 1474.5 µs   2996.2 µs      20.3x Nisaba        3.77x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   18.9 µs  681.3 µs    700.2 µs  900.6 µs 1394.0 µs   2294.6 µs      47.7x Nisaba        3.28x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      47.8 µs  983.1 µs   1030.9 µs 2223.2 µs 1742.0 µs   3965.1 µs      46.5x Nisaba        3.85x Nisaba
31. Stroke Join: Miter (12px Stroke)         25.3 µs  897.0 µs    922.3 µs 1094.0 µs 1375.3 µs   2469.3 µs      43.2x Nisaba        2.68x Nisaba
32. Stroke Join: Bevel (12px Stroke)         19.1 µs  777.2 µs    796.2 µs 1331.2 µs 1468.5 µs   2799.7 µs      69.8x Nisaba        3.52x Nisaba
33. Stroke Join: Round (12px Stroke)         29.1 µs  718.7 µs    747.8 µs 1256.0 µs 1381.9 µs   2637.9 µs      43.2x Nisaba        3.53x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      30.4 µs  675.3 µs    705.7 µs  118.8 µs 1013.1 µs   1131.9 µs       3.9x Nisaba        1.60x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    31.0 µs  747.5 µs    778.5 µs  117.4 µs  876.8 µs    994.2 µs       3.8x Nisaba        1.28x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     93.3 µs  800.4 µs    893.8 µs  193.3 µs  999.6 µs   1192.9 µs       2.1x Nisaba        1.33x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   35.6 µs  747.8 µs    783.4 µs 1334.7 µs 1449.0 µs   2783.7 µs      37.5x Nisaba        3.55x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   24.8 µs  606.6 µs    631.4 µs  144.4 µs  878.0 µs   1022.4 µs       5.8x Nisaba        1.62x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   24.7 µs  808.0 µs    832.8 µs  674.5 µs 1343.8 µs   2018.4 µs      27.3x Nisaba        2.42x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   96.1 µs  954.4 µs   1050.4 µs 1517.2 µs 1426.9 µs   2944.1 µs      15.8x Nisaba        2.80x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          25.2 µs  682.2 µs    707.5 µs  121.1 µs  846.2 µs    967.3 µs       4.8x Nisaba        1.37x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       59.5 µs 1086.2 µs   1145.7 µs  165.9 µs 1278.0 µs   1443.9 µs       2.8x Nisaba        1.26x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         55.0 µs  921.7 µs    976.6 µs  218.6 µs 1017.2 µs   1235.8 µs       4.0x Nisaba        1.27x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     57.8 µs 1052.8 µs   1110.6 µs  210.9 µs 1218.7 µs   1429.6 µs       3.6x Nisaba        1.29x Nisaba
45. High-Density Disks Cloud (500 Disks)    281.7 µs 1240.7 µs   1522.4 µs  989.6 µs 1455.5 µs   2445.1 µs       3.5x Nisaba        1.61x Nisaba
46. High-Density Stroked Rings (250 Rings)  105.6 µs  995.8 µs   1101.5 µs  481.3 µs 1321.8 µs   1803.1 µs       4.6x Nisaba        1.64x Nisaba
47. 250 Filled Rectangles Batch             104.2 µs  865.8 µs    970.0 µs  266.7 µs 1019.3 µs   1286.0 µs       2.6x Nisaba        1.33x Nisaba
48. 150 Stroked Rectangles (2px Border)      52.1 µs  599.5 µs    651.6 µs  263.5 µs 1125.9 µs   1389.3 µs       5.1x Nisaba        2.13x Nisaba
49. 40 Crosshair Aim Reticles               259.4 µs 1189.7 µs   1449.1 µs  756.7 µs 1260.2 µs   2016.8 µs       2.9x Nisaba        1.39x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     366.1 µs 1559.3 µs   1925.4 µs  741.5 µs 1492.7 µs   2234.2 µs       2.0x Nisaba        1.16x Nisaba
51. 30 Vertical Card Linear Gradients        51.8 µs 1062.5 µs   1114.3 µs  495.6 µs 1184.1 µs   1679.7 µs       9.6x Nisaba        1.51x Nisaba
52. 30 Horizontal Bar Progress Gradients     36.9 µs  980.0 µs   1016.9 µs  687.3 µs 1461.9 µs   2149.2 µs      18.6x Nisaba        2.11x Nisaba
53. 15 Concentric Radial Spotlights          45.0 µs 1032.8 µs   1077.8 µs  231.5 µs 1019.0 µs   1250.5 µs       5.1x Nisaba        1.16x Nisaba
54. 25 Soft Ambient Card Shadows             47.2 µs  984.2 µs   1031.4 µs  628.7 µs 1450.9 µs   2079.6 µs      13.3x Nisaba        2.02x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      46.5 µs 1083.5 µs   1129.9 µs  413.9 µs 1410.5 µs   1824.5 µs       8.9x Nisaba        1.61x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      116.8 µs 1213.9 µs   1330.7 µs  548.6 µs 1408.6 µs   1957.2 µs       4.7x Nisaba        1.47x Nisaba
57. Full-Screen Diagonal Horizon Gradient    41.0 µs 1056.6 µs   1097.6 µs  139.8 µs  960.7 µs   1100.5 µs       3.4x Nisaba         Tie (~1.0x)
58. 20 Inset Well Shadows (Recessed)        275.1 µs 1568.9 µs   1844.0 µs  621.9 µs 1571.1 µs   2193.0 µs       2.3x Nisaba        1.19x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     57.7 µs 1570.3 µs   1628.0 µs  322.5 µs 1750.9 µs   2073.5 µs       5.6x Nisaba        1.27x Nisaba
60. Waveform Polyline Stroked with Gradient  205.4 µs 1170.3 µs   1375.7 µs 2264.8 µs 1587.1 µs   3852.0 µs      11.0x Nisaba        2.80x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   15.4 µs  735.2 µs    750.6 µs   74.2 µs  668.8 µs    743.1 µs       4.8x Nisaba         Tie (~1.0x)
62. Scaled Image Quad (2x 512x512 Blit)      22.1 µs  772.8 µs    794.8 µs  110.4 µs  987.2 µs   1097.6 µs       5.0x Nisaba        1.38x Nisaba
63. 45° Rotated Pattern Fill in Circle      16.9 µs  784.3 µs    801.3 µs   67.9 µs  681.9 µs    749.8 µs       4.0x Nisaba          1.07x Skia
64. Translucent Texture Overlay (50% Alpha)   23.8 µs  814.9 µs    838.7 µs   98.8 µs  850.0 µs    948.9 µs       4.2x Nisaba        1.13x Nisaba
65. Multi-Avatar Grid (16 Avatars)           33.9 µs  782.2 µs    816.1 µs  133.6 µs  963.2 µs   1096.8 µs       3.9x Nisaba        1.34x Nisaba
66. Texture Pattern on Star Path             45.7 µs  863.5 µs    909.1 µs  379.4 µs 1117.8 µs   1497.2 µs       8.3x Nisaba        1.65x Nisaba
67. 20px Curved Path Stroked with Texture    31.7 µs  885.2 µs    916.9 µs  611.6 µs 1436.8 µs   2048.4 µs      19.3x Nisaba        2.23x Nisaba
68. Minified Texture Quad (0.25x Downscale)   45.6 µs  710.8 µs    756.4 µs  257.9 µs 1146.3 µs   1404.2 µs       5.7x Nisaba        1.86x Nisaba
69. Pure Translation Stack (50 Cards)        97.1 µs 1010.3 µs   1107.4 µs  279.1 µs 1332.6 µs   1611.7 µs       2.9x Nisaba        1.46x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   29.8 µs  675.7 µs    705.5 µs  117.4 µs  838.6 µs    956.0 µs       3.9x Nisaba        1.36x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   70.0 µs  967.3 µs   1037.3 µs   55.7 µs  750.6 µs    806.3 µs         1.3x Skia          1.29x Skia
72. Shear / Skew Parallelograms (30 Quads)   23.9 µs  562.2 µs    586.2 µs   90.3 µs  667.9 µs    758.2 µs       3.8x Nisaba        1.29x Nisaba
73. Combined Affine Transforms (30 Shapes)   29.3 µs  860.4 µs    889.7 µs  108.7 µs  793.5 µs    902.2 µs       3.7x Nisaba         Tie (~1.0x)
74. Deep Hierarchical State Stack (20 Levels)   54.3 µs  918.4 µs    972.7 µs  172.8 µs 1049.6 µs   1222.5 µs       3.2x Nisaba        1.26x Nisaba
75. Reset Transform Stress (50 Cycles)       30.9 µs  690.4 µs    721.3 µs  179.0 µs  943.4 µs   1122.4 µs       5.8x Nisaba        1.56x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   43.5 µs  931.5 µs    975.0 µs   97.0 µs  799.8 µs    896.8 µs       2.2x Nisaba          1.09x Skia
77. Multi-Scissor Grid (16 Viewports)        45.7 µs  918.4 µs    964.1 µs  150.2 µs  648.5 µs    798.7 µs       3.3x Nisaba          1.21x Skia
78. Hierarchical Intersecting Scissors (4 Levels)   27.0 µs  772.9 µs    799.9 µs  151.4 µs 1137.2 µs   1288.6 µs       5.6x Nisaba        1.61x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  272.0 µs 1091.3 µs   1363.3 µs 1750.5 µs 1407.2 µs   3157.8 µs       6.4x Nisaba        2.32x Nisaba
80. Scissored Gradient Card (Clipped)        34.9 µs 1083.9 µs   1118.8 µs  254.0 µs  904.2 µs   1158.2 µs       7.3x Nisaba         Tie (~1.0x)
81. Circular Overflow Clip (25 Boxes)        23.1 µs  504.0 µs    527.1 µs  234.2 µs  773.9 µs   1008.1 µs      10.2x Nisaba        1.91x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   32.4 µs  719.3 µs    751.8 µs   91.1 µs  710.0 µs    801.1 µs       2.8x Nisaba        1.07x Nisaba
83. Composite Op: Source-Over (30 Shapes)    35.2 µs  749.2 µs    784.4 µs  109.2 µs  829.2 µs    938.4 µs       3.1x Nisaba        1.20x Nisaba
84. Composite Op: Lighter / Plus (30 Particles)   33.1 µs  747.8 µs    780.9 µs  116.8 µs  798.7 µs    915.5 µs       3.5x Nisaba        1.17x Nisaba
85. Composite Op: Source-In (Alpha Masking)   38.3 µs  766.5 µs    804.9 µs  153.2 µs 1227.9 µs   1381.1 µs       4.0x Nisaba        1.72x Nisaba
86. Composite Op: Source-Out (Cutout)        26.4 µs  693.9 µs    720.4 µs  190.1 µs 1384.8 µs   1574.9 µs       7.2x Nisaba        2.19x Nisaba
87. Composite Op: Atop (Target Bounds)       42.1 µs  836.3 µs    878.4 µs  182.6 µs  965.3 µs   1147.9 µs       4.3x Nisaba        1.31x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   31.6 µs  773.1 µs    804.7 µs   76.4 µs  838.1 µs    914.4 µs       2.4x Nisaba        1.14x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     31.7 µs  792.6 µs    824.3 µs  139.7 µs  904.3 µs   1044.0 µs       4.4x Nisaba        1.27x Nisaba
90. Composite Op: Dest-Atop (Inverse)        38.1 µs  822.1 µs    860.3 µs  133.6 µs  981.6 µs   1115.2 µs       3.5x Nisaba        1.30x Nisaba
91. Composite Op: Xor (Exclusive Blend)      23.1 µs  658.0 µs    681.1 µs  149.2 µs  977.3 µs   1126.5 µs       6.5x Nisaba        1.65x Nisaba
92. Composite Op: Copy (Direct Overwrite)    37.8 µs  766.0 µs    803.7 µs   68.3 µs  806.3 µs    874.6 µs       1.8x Nisaba        1.09x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   49.2 µs  752.6 µs    801.8 µs  750.1 µs 1266.5 µs   2016.6 µs      15.2x Nisaba        2.51x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     105.7 µs 1255.3 µs   1361.1 µs  574.2 µs 1217.2 µs   1791.4 µs       5.4x Nisaba        1.32x Nisaba
95. Circular Progress Rings (8 Meters)       99.9 µs  716.1 µs    816.0 µs  972.7 µs 1402.7 µs   2375.4 µs       9.7x Nisaba        2.91x Nisaba
96. Modern Card Stack (5 Elevated Cards)     54.4 µs 1272.6 µs   1327.0 µs  285.0 µs 1311.3 µs   1596.3 µs       5.2x Nisaba        1.20x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   75.0 µs  907.5 µs    982.5 µs  145.8 µs 1162.9 µs   1308.7 µs       1.9x Nisaba        1.33x Nisaba
98. Floating Action Button (FAB 10 Buttons)  108.3 µs 1127.4 µs   1235.6 µs  503.7 µs 1348.7 µs   1852.4 µs       4.7x Nisaba        1.50x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   90.5 µs  984.8 µs   1075.4 µs  393.4 µs 1265.7 µs   1659.1 µs       4.3x Nisaba        1.54x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   58.8 µs 1048.5 µs   1107.3 µs  246.3 µs 1304.1 µs   1550.5 µs       4.2x Nisaba        1.40x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          6951.6 µs91809.1 µs  98760.7 µs49398.1 µs116114.2 µs 165512.3 µs      7.11x Nisaba        1.68x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 88 suites | Skia won in 7 suites | Ties: 5
         Overall Frame Throughput (End-to-End): 1.68x Nisaba
         Overall CPU Command Submission:       7.11x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 578.5 µs, Skia: 886.5 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 1027.6 µs, Skia: 1448.6 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1214.5 µs, Skia: 2848.2 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1241.7 µs, Skia: 1625.3 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1294.8 µs, Skia: 1795.3 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1198.4 µs, Skia: 1295.0 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1167.1 µs, Skia: 1394.1 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1381.4 µs, Skia: 1470.8 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1092.9 µs, Skia: 1703.2 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1449.7 µs, Skia: 1307.9 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1319.8 µs, Skia: 2641.5 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1348.0 µs, Skia: 2606.9 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2705.8 µs, Skia: 2774.7 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1230.4 µs, Skia: 1859.4 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1649.0 µs, Skia: 3976.5 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 1129.8 µs, Skia: 4545.4 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1096.2 µs, Skia: 4062.9 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1004.1 µs, Skia: 1540.7 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 753.5 µs, Skia: 496.2 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1009.7 µs, Skia: 1520.2 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1502.1 µs, Skia: 2020.6 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 899.4 µs, Skia: 3695.2 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 701.8 µs, Skia: 1712.2 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 864.5 µs, Skia: 2794.3 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1474.5 µs, Skia: 1909.4 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1405.9 µs, Skia: 2681.4 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 914.9 µs, Skia: 2332.4 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 860.9 µs, Skia: 3245.8 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 622.5 µs, Skia: 1820.8 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1081.9 µs, Skia: 3696.5 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 661.0 µs, Skia: 1867.7 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 811.6 µs, Skia: 2891.9 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 821.4 µs, Skia: 2078.2 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 639.9 µs, Skia: 1057.7 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 848.7 µs, Skia: 1227.8 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1064.4 µs, Skia: 1378.4 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 929.0 µs, Skia: 2926.9 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 848.6 µs, Skia: 979.8 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 840.5 µs, Skia: 2187.9 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1338.2 µs, Skia: 4675.0 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 633.5 µs, Skia: 1283.4 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1113.4 µs, Skia: 1388.6 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1195.0 µs, Skia: 1529.6 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1059.0 µs, Skia: 1264.7 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1336.5 µs, Skia: 1746.1 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 991.7 µs, Skia: 1932.7 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1194.0 µs, Skia: 1384.3 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 716.5 µs, Skia: 1495.7 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1420.9 µs, Skia: 2167.2 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1268.9 µs, Skia: 2150.0 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 928.9 µs, Skia: 1600.6 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1109.2 µs, Skia: 1563.9 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1012.6 µs, Skia: 1791.0 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1221.4 µs, Skia: 1788.4 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 956.4 µs, Skia: 1657.5 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1160.7 µs, Skia: 1667.6 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 921.6 µs, Skia: 978.8 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1484.5 µs, Skia: 1848.1 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1718.8 µs, Skia: 1971.4 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1153.2 µs, Skia: 2854.7 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 709.2 µs, Skia: 895.2 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 912.7 µs, Skia: 1055.4 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1062.7 µs, Skia: 1063.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 985.0 µs, Skia: 982.4 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 770.2 µs, Skia: 1053.1 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 963.3 µs, Skia: 1286.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 633.0 µs, Skia: 2258.5 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1071.4 µs, Skia: 1052.8 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 852.6 µs, Skia: 1493.7 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 936.8 µs, Skia: 1465.1 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1075.4 µs, Skia: 1095.5 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 1068.9 µs, Skia: 1405.6 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1114.5 µs, Skia: 1332.5 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 935.9 µs, Skia: 1259.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 989.8 µs, Skia: 1520.8 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 1149.4 µs, Skia: 1427.4 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 977.7 µs, Skia: 1187.8 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1101.9 µs, Skia: 1504.4 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1725.6 µs, Skia: 3345.7 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 804.1 µs, Skia: 1235.3 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 902.8 µs, Skia: 1032.0 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 916.1 µs, Skia: 1011.3 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 865.6 µs, Skia: 883.2 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 968.9 µs, Skia: 1060.0 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 842.9 µs, Skia: 1095.5 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 879.5 µs, Skia: 1264.7 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 892.2 µs, Skia: 993.9 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 825.9 µs, Skia: 816.1 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 780.4 µs, Skia: 982.1 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 798.7 µs, Skia: 1149.9 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 798.5 µs, Skia: 1100.9 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 903.7 µs, Skia: 1203.3 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 948.7 µs, Skia: 2211.6 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1154.3 µs, Skia: 1976.4 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 830.9 µs, Skia: 2303.7 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1294.0 µs, Skia: 1538.8 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 603.4 µs, Skia: 971.3 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1214.5 µs, Skia: 1781.0 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 987.3 µs, Skia: 1050.8 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1180.1 µs, Skia: 1601.6 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          5.6 µs  572.9 µs    578.5 µs   39.9 µs  846.7 µs    886.5 µs       7.1x Nisaba        1.53x Nisaba
2. 100 Grid Lines (1px Hairlines)            64.3 µs  963.3 µs   1027.6 µs  197.4 µs 1251.2 µs   1448.6 µs       3.1x Nisaba        1.41x Nisaba
3. 20 Thick Diagonal Lines (16px Round)     111.6 µs 1103.0 µs   1214.5 µs 1375.8 µs 1472.5 µs   2848.2 µs      12.3x Nisaba        2.35x Nisaba
4. 100 Solid Rectangles (Varied Bounds)     124.1 µs 1117.6 µs   1241.7 µs  361.6 µs 1263.8 µs   1625.3 µs       2.9x Nisaba        1.31x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)  109.7 µs 1185.1 µs   1294.8 µs  393.9 µs 1401.4 µs   1795.3 µs       3.6x Nisaba        1.39x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     73.5 µs 1124.9 µs   1198.4 µs  189.0 µs 1106.0 µs   1295.0 µs       2.6x Nisaba        1.08x Nisaba
7. 50 Varying-Radius Rounded Rects           74.8 µs 1092.3 µs   1167.1 µs  218.8 µs 1175.3 µs   1394.1 µs       2.9x Nisaba        1.19x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   97.2 µs 1284.2 µs   1381.4 µs  264.5 µs 1206.3 µs   1470.8 µs       2.7x Nisaba        1.06x Nisaba
9. 100 Alpha Circles / Disks (Fill)          89.2 µs 1003.8 µs   1092.9 µs  365.3 µs 1337.9 µs   1703.2 µs       4.1x Nisaba        1.56x Nisaba
10. 25 Concentric Rings (2px Stroke)        157.8 µs 1291.9 µs   1449.7 µs  182.9 µs 1124.9 µs   1307.9 µs       1.2x Nisaba          1.11x Skia
11. 50 Two-Stop Linear Gradients             80.0 µs 1239.8 µs   1319.8 µs 1036.7 µs 1604.8 µs   2641.5 µs      13.0x Nisaba        2.00x Nisaba
12. 50 Diagonal Gradients (45° Angle)       75.3 µs 1272.7 µs   1348.0 µs 1073.0 µs 1533.9 µs   2606.9 µs      14.3x Nisaba        1.93x Nisaba
13. 25 Radial Glow Gradients (300px)         70.8 µs 2635.0 µs   2705.8 µs  596.8 µs 2177.9 µs   2774.7 µs       8.4x Nisaba         Tie (~1.0x)
14. 20 Box Blur / Drop Shadows (15px)        51.3 µs 1179.1 µs   1230.4 µs  518.2 µs 1341.2 µs   1859.4 µs      10.1x Nisaba        1.51x Nisaba
15. 10 Concave 10-Point Stars (Fill)        211.8 µs 1437.3 µs   1649.0 µs 2273.0 µs 1703.4 µs   3976.5 µs      10.7x Nisaba        2.41x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)  138.4 µs  991.4 µs   1129.8 µs 3025.3 µs 1520.1 µs   4545.4 µs      21.9x Nisaba        4.02x Nisaba
17. 1000-Point Waveform (Dense Stroke)      130.2 µs  966.0 µs   1096.2 µs 2353.4 µs 1709.5 µs   4062.9 µs      18.1x Nisaba        3.71x Nisaba
18. 50 Rotated Cards (Affine Transforms)     68.1 µs  936.0 µs   1004.1 µs  275.3 µs 1265.4 µs   1540.7 µs       4.0x Nisaba        1.53x Nisaba
19. 25 Scissor Clipped Viewports             24.6 µs  728.9 µs    753.5 µs   88.2 µs  407.9 µs    496.2 µs       3.6x Nisaba          1.52x Skia
20. 50 UI Text Labels (Inter-Regular)       160.8 µs  849.0 µs   1009.7 µs  340.7 µs 1179.4 µs   1520.2 µs       2.1x Nisaba        1.51x Nisaba
21. 25 Quadratic Bezier Curves (Fill+Stroke)  263.9 µs 1238.2 µs   1502.1 µs  595.1 µs 1425.5 µs   2020.6 µs       2.3x Nisaba        1.35x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     78.2 µs  821.2 µs    899.4 µs 2088.1 µs 1607.2 µs   3695.2 µs      26.7x Nisaba        4.11x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       19.1 µs  682.7 µs    701.8 µs  479.3 µs 1232.9 µs   1712.2 µs      25.1x Nisaba        2.44x Nisaba
24. Circular Arcs with arcTo (20 Corners)    68.7 µs  795.8 µs    864.5 µs 1343.5 µs 1450.8 µs   2794.3 µs      19.6x Nisaba        3.23x Nisaba
25. Directional Arcs with arc() (30 Arcs)   228.6 µs 1245.9 µs   1474.5 µs  578.4 µs 1331.0 µs   1909.4 µs       2.5x Nisaba        1.29x Nisaba
26. Compound Path with Holes (15 Donuts)    204.0 µs 1201.9 µs   1405.9 µs 1203.9 µs 1477.5 µs   2681.4 µs       5.9x Nisaba        1.91x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    93.8 µs  821.1 µs    914.9 µs 1022.5 µs 1309.9 µs   2332.4 µs      10.9x Nisaba        2.55x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    76.3 µs  784.5 µs    860.9 µs 1733.9 µs 1511.9 µs   3245.8 µs      22.7x Nisaba        3.77x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   13.0 µs  609.5 µs    622.5 µs  556.6 µs 1264.1 µs   1820.8 µs      42.7x Nisaba        2.92x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      57.6 µs 1024.3 µs   1081.9 µs 2117.3 µs 1579.2 µs   3696.5 µs      36.7x Nisaba        3.42x Nisaba
31. Stroke Join: Miter (12px Stroke)         14.5 µs  646.4 µs    661.0 µs  661.1 µs 1206.7 µs   1867.7 µs      45.4x Nisaba        2.83x Nisaba
32. Stroke Join: Bevel (12px Stroke)         18.7 µs  792.9 µs    811.6 µs 1367.7 µs 1524.2 µs   2891.9 µs      73.0x Nisaba        3.56x Nisaba
33. Stroke Join: Round (12px Stroke)         37.3 µs  784.1 µs    821.4 µs  784.2 µs 1294.0 µs   2078.2 µs      21.0x Nisaba        2.53x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      24.5 µs  615.4 µs    639.9 µs   95.6 µs  962.1 µs   1057.7 µs       3.9x Nisaba        1.65x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    41.4 µs  807.4 µs    848.7 µs  162.0 µs 1065.8 µs   1227.8 µs       3.9x Nisaba        1.45x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    115.8 µs  948.6 µs   1064.4 µs  241.1 µs 1137.3 µs   1378.4 µs       2.1x Nisaba        1.29x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   56.7 µs  872.3 µs    929.0 µs 1476.8 µs 1450.1 µs   2926.9 µs      26.0x Nisaba        3.15x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   38.1 µs  810.5 µs    848.6 µs  138.4 µs  841.4 µs    979.8 µs       3.6x Nisaba        1.15x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   32.0 µs  808.5 µs    840.5 µs  772.1 µs 1415.8 µs   2187.9 µs      24.1x Nisaba        2.60x Nisaba
40. Stepped Variable Stroke Widths (1-20px)  137.2 µs 1201.0 µs   1338.2 µs 3037.9 µs 1637.1 µs   4675.0 µs      22.1x Nisaba        3.49x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          23.2 µs  610.2 µs    633.5 µs  180.5 µs 1102.9 µs   1283.4 µs       7.8x Nisaba        2.03x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       59.4 µs 1054.0 µs   1113.4 µs  197.1 µs 1191.5 µs   1388.6 µs       3.3x Nisaba        1.25x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         96.5 µs 1098.6 µs   1195.0 µs  270.2 µs 1259.4 µs   1529.6 µs       2.8x Nisaba        1.28x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     48.4 µs 1010.6 µs   1059.0 µs  153.9 µs 1110.7 µs   1264.7 µs       3.2x Nisaba        1.19x Nisaba
45. High-Density Disks Cloud (500 Disks)    199.9 µs 1136.6 µs   1336.5 µs  609.6 µs 1136.6 µs   1746.1 µs       3.0x Nisaba        1.31x Nisaba
46. High-Density Stroked Rings (250 Rings)   84.9 µs  906.8 µs    991.7 µs  540.9 µs 1391.8 µs   1932.7 µs       6.4x Nisaba        1.95x Nisaba
47. 250 Filled Rectangles Batch             158.1 µs 1035.9 µs   1194.0 µs  318.5 µs 1065.8 µs   1384.3 µs       2.0x Nisaba        1.16x Nisaba
48. 150 Stroked Rectangles (2px Border)      63.5 µs  653.0 µs    716.5 µs  337.6 µs 1158.1 µs   1495.7 µs       5.3x Nisaba        2.09x Nisaba
49. 40 Crosshair Aim Reticles               254.1 µs 1166.8 µs   1420.9 µs  782.8 µs 1384.5 µs   2167.2 µs       3.1x Nisaba        1.53x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     199.0 µs 1069.9 µs   1268.9 µs  702.9 µs 1447.1 µs   2150.0 µs       3.5x Nisaba        1.69x Nisaba
51. 30 Vertical Card Linear Gradients        34.1 µs  894.8 µs    928.9 µs  425.4 µs 1175.2 µs   1600.6 µs      12.5x Nisaba        1.72x Nisaba
52. 30 Horizontal Bar Progress Gradients     34.9 µs 1074.4 µs   1109.2 µs  396.9 µs 1166.9 µs   1563.9 µs      11.4x Nisaba        1.41x Nisaba
53. 15 Concentric Radial Spotlights          29.6 µs  983.0 µs   1012.6 µs  384.7 µs 1406.3 µs   1791.0 µs      13.0x Nisaba        1.77x Nisaba
54. 25 Soft Ambient Card Shadows             57.4 µs 1164.0 µs   1221.4 µs  499.3 µs 1289.1 µs   1788.4 µs       8.7x Nisaba        1.46x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      33.1 µs  923.3 µs    956.4 µs  370.3 µs 1287.1 µs   1657.5 µs      11.2x Nisaba        1.73x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      114.3 µs 1046.4 µs   1160.7 µs  433.1 µs 1234.5 µs   1667.6 µs       3.8x Nisaba        1.44x Nisaba
57. Full-Screen Diagonal Horizon Gradient    28.1 µs  893.4 µs    921.6 µs  100.3 µs  878.5 µs    978.8 µs       3.6x Nisaba        1.06x Nisaba
58. 20 Inset Well Shadows (Recessed)        176.3 µs 1308.2 µs   1484.5 µs  475.7 µs 1372.5 µs   1848.1 µs       2.7x Nisaba        1.24x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     55.6 µs 1663.2 µs   1718.8 µs  319.6 µs 1651.8 µs   1971.4 µs       5.7x Nisaba        1.15x Nisaba
60. Waveform Polyline Stroked with Gradient  135.5 µs 1017.8 µs   1153.2 µs 1406.5 µs 1448.2 µs   2854.7 µs      10.4x Nisaba        2.48x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   15.5 µs  693.7 µs    709.2 µs   85.2 µs  810.0 µs    895.2 µs       5.5x Nisaba        1.26x Nisaba
62. Scaled Image Quad (2x 512x512 Blit)      30.2 µs  882.6 µs    912.7 µs  118.1 µs  937.2 µs   1055.4 µs       3.9x Nisaba        1.16x Nisaba
63. 45° Rotated Pattern Fill in Circle      36.8 µs 1025.9 µs   1062.7 µs  131.1 µs  932.3 µs   1063.4 µs       3.6x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   30.2 µs  954.7 µs    985.0 µs   97.9 µs  884.5 µs    982.4 µs       3.2x Nisaba         Tie (~1.0x)
65. Multi-Avatar Grid (16 Avatars)           35.0 µs  735.3 µs    770.2 µs  110.1 µs  943.0 µs   1053.1 µs       3.1x Nisaba        1.37x Nisaba
66. Texture Pattern on Star Path             37.9 µs  925.4 µs    963.3 µs  303.3 µs  982.9 µs   1286.2 µs       8.0x Nisaba        1.34x Nisaba
67. 20px Curved Path Stroked with Texture    20.7 µs  612.3 µs    633.0 µs  881.7 µs 1376.9 µs   2258.5 µs      42.6x Nisaba        3.57x Nisaba
68. Minified Texture Quad (0.25x Downscale)   75.1 µs  996.2 µs   1071.4 µs  135.9 µs  916.9 µs   1052.8 µs       1.8x Nisaba         Tie (~1.0x)
69. Pure Translation Stack (50 Cards)        49.0 µs  803.6 µs    852.6 µs  290.0 µs 1203.7 µs   1493.7 µs       5.9x Nisaba        1.75x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   64.8 µs  872.0 µs    936.8 µs  240.4 µs 1224.7 µs   1465.1 µs       3.7x Nisaba        1.56x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   91.1 µs  984.3 µs   1075.4 µs  161.1 µs  934.4 µs   1095.5 µs       1.8x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   66.1 µs 1002.9 µs   1068.9 µs  225.8 µs 1179.8 µs   1405.6 µs       3.4x Nisaba        1.31x Nisaba
73. Combined Affine Transforms (30 Shapes)   66.6 µs 1047.9 µs   1114.5 µs  191.9 µs 1140.6 µs   1332.5 µs       2.9x Nisaba        1.20x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   53.9 µs  882.0 µs    935.9 µs  184.6 µs 1075.2 µs   1259.8 µs       3.4x Nisaba        1.35x Nisaba
75. Reset Transform Stress (50 Cycles)       76.4 µs  913.4 µs    989.8 µs  265.6 µs 1255.2 µs   1520.8 µs       3.5x Nisaba        1.54x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   56.1 µs 1093.3 µs   1149.4 µs  184.8 µs 1242.6 µs   1427.4 µs       3.3x Nisaba        1.24x Nisaba
77. Multi-Scissor Grid (16 Viewports)        49.9 µs  927.7 µs    977.7 µs  269.0 µs  918.9 µs   1187.8 µs       5.4x Nisaba        1.21x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   53.1 µs 1048.8 µs   1101.9 µs  185.2 µs 1319.2 µs   1504.4 µs       3.5x Nisaba        1.37x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  399.3 µs 1326.3 µs   1725.6 µs 1856.6 µs 1489.1 µs   3345.7 µs       4.6x Nisaba        1.94x Nisaba
80. Scissored Gradient Card (Clipped)        18.4 µs  785.7 µs    804.1 µs  265.2 µs  970.1 µs   1235.3 µs      14.4x Nisaba        1.54x Nisaba
81. Circular Overflow Clip (25 Boxes)        37.0 µs  865.8 µs    902.8 µs  226.3 µs  805.8 µs   1032.0 µs       6.1x Nisaba        1.14x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   36.4 µs  879.7 µs    916.1 µs  127.4 µs  883.9 µs   1011.3 µs       3.5x Nisaba        1.10x Nisaba
83. Composite Op: Source-Over (30 Shapes)    35.3 µs  830.2 µs    865.6 µs  112.8 µs  770.3 µs    883.2 µs       3.2x Nisaba         Tie (~1.0x)
84. Composite Op: Lighter / Plus (30 Particles)   35.0 µs  933.9 µs    968.9 µs  126.1 µs  934.0 µs   1060.0 µs       3.6x Nisaba        1.09x Nisaba
85. Composite Op: Source-In (Alpha Masking)   32.2 µs  810.7 µs    842.9 µs  121.4 µs  974.1 µs   1095.5 µs       3.8x Nisaba        1.30x Nisaba
86. Composite Op: Source-Out (Cutout)        34.1 µs  845.4 µs    879.5 µs  143.3 µs 1121.4 µs   1264.7 µs       4.2x Nisaba        1.44x Nisaba
87. Composite Op: Atop (Target Bounds)       41.7 µs  850.4 µs    892.2 µs  111.6 µs  882.3 µs    993.9 µs       2.7x Nisaba        1.11x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   30.7 µs  795.2 µs    825.9 µs   99.5 µs  716.5 µs    816.1 µs       3.2x Nisaba         Tie (~1.0x)
89. Composite Op: Dest-Out (Eraser Mask)     26.8 µs  753.6 µs    780.4 µs  133.1 µs  849.0 µs    982.1 µs       5.0x Nisaba        1.26x Nisaba
90. Composite Op: Dest-Atop (Inverse)        35.0 µs  763.7 µs    798.7 µs  107.2 µs 1042.8 µs   1149.9 µs       3.1x Nisaba        1.44x Nisaba
91. Composite Op: Xor (Exclusive Blend)      41.7 µs  756.9 µs    798.5 µs  152.1 µs  948.8 µs   1100.9 µs       3.7x Nisaba        1.38x Nisaba
92. Composite Op: Copy (Direct Overwrite)    42.6 µs  861.1 µs    903.7 µs  158.8 µs 1044.5 µs   1203.3 µs       3.7x Nisaba        1.33x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   75.7 µs  873.0 µs    948.7 µs  790.2 µs 1421.4 µs   2211.6 µs      10.4x Nisaba        2.33x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      83.4 µs 1070.9 µs   1154.3 µs  717.9 µs 1258.6 µs   1976.4 µs       8.6x Nisaba        1.71x Nisaba
95. Circular Progress Rings (8 Meters)       83.2 µs  747.7 µs    830.9 µs  945.7 µs 1358.1 µs   2303.7 µs      11.4x Nisaba        2.77x Nisaba
96. Modern Card Stack (5 Elevated Cards)     40.6 µs 1253.4 µs   1294.0 µs  223.6 µs 1315.2 µs   1538.8 µs       5.5x Nisaba        1.19x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   51.7 µs  551.7 µs    603.4 µs   68.1 µs  903.2 µs    971.3 µs       1.3x Nisaba        1.61x Nisaba
98. Floating Action Button (FAB 10 Buttons)  117.7 µs 1096.8 µs   1214.5 µs  461.5 µs 1319.5 µs   1781.0 µs       3.9x Nisaba        1.47x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   92.4 µs  894.9 µs    987.3 µs  203.4 µs  847.4 µs   1050.8 µs       2.2x Nisaba        1.06x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   64.5 µs 1115.6 µs   1180.1 µs  265.2 µs 1336.4 µs   1601.6 µs       4.1x Nisaba        1.36x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          7788.3 µs97261.4 µs 105049.7 µs56009.3 µs120983.0 µs 176992.2 µs      7.19x Nisaba        1.68x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 91 suites | Skia won in 2 suites | Ties: 7
         Overall Frame Throughput (End-to-End): 1.68x Nisaba
         Overall CPU Command Submission:       7.19x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 442.4 µs, Skia: 493.5 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 618.3 µs, Skia: 968.3 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 942.7 µs, Skia: 1903.5 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 767.1 µs, Skia: 1018.9 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1045.1 µs, Skia: 1214.4 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1162.6 µs, Skia: 1610.7 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1408.2 µs, Skia: 1505.7 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1242.8 µs, Skia: 1546.8 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1108.9 µs, Skia: 1579.6 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1411.3 µs, Skia: 1299.7 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1422.5 µs, Skia: 2738.1 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1277.6 µs, Skia: 2719.3 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2767.4 µs, Skia: 2621.1 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1081.1 µs, Skia: 1930.9 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1313.6 µs, Skia: 2356.3 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 834.8 µs, Skia: 3412.8 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1117.8 µs, Skia: 3322.8 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1041.2 µs, Skia: 1283.1 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 771.4 µs, Skia: 875.3 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1081.6 µs, Skia: 1164.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1448.5 µs, Skia: 2631.9 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 983.2 µs, Skia: 4720.2 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 807.3 µs, Skia: 1699.8 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 863.9 µs, Skia: 2814.0 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1063.8 µs, Skia: 1622.1 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1602.2 µs, Skia: 2881.0 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 883.3 µs, Skia: 2513.8 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 851.1 µs, Skia: 3766.1 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 807.9 µs, Skia: 2190.6 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1138.0 µs, Skia: 5148.8 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 708.3 µs, Skia: 2296.1 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 724.0 µs, Skia: 2458.3 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 760.1 µs, Skia: 2636.6 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 702.8 µs, Skia: 1007.2 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 719.2 µs, Skia: 998.6 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 880.9 µs, Skia: 1114.5 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 812.8 µs, Skia: 3666.4 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 877.3 µs, Skia: 1183.0 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 834.9 µs, Skia: 1802.6 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 885.7 µs, Skia: 2974.4 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 907.3 µs, Skia: 1029.4 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1226.9 µs, Skia: 1191.2 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1087.2 µs, Skia: 1219.8 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 1110.5 µs, Skia: 1018.3 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1184.8 µs, Skia: 2048.4 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1039.5 µs, Skia: 1522.1 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1032.5 µs, Skia: 1604.3 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 920.3 µs, Skia: 1361.6 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1396.0 µs, Skia: 2101.1 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1302.4 µs, Skia: 2130.8 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1075.9 µs, Skia: 1816.3 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1023.4 µs, Skia: 1701.0 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 980.3 µs, Skia: 1351.1 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1272.1 µs, Skia: 1976.5 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 927.3 µs, Skia: 1459.7 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 864.9 µs, Skia: 1388.6 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1089.0 µs, Skia: 976.6 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1435.3 µs, Skia: 1527.8 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1877.7 µs, Skia: 1739.2 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1010.9 µs, Skia: 3166.1 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 912.9 µs, Skia: 935.0 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 800.2 µs, Skia: 934.6 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 1030.7 µs, Skia: 929.9 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 980.9 µs, Skia: 1095.8 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 905.7 µs, Skia: 1007.0 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 954.6 µs, Skia: 1704.2 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 1012.7 µs, Skia: 1749.2 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 905.7 µs, Skia: 1152.9 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 1177.2 µs, Skia: 1597.6 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 962.7 µs, Skia: 1472.8 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1052.2 µs, Skia: 1083.0 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 921.3 µs, Skia: 1306.0 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1016.7 µs, Skia: 1418.4 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 1173.4 µs, Skia: 1198.2 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 930.0 µs, Skia: 1467.6 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 696.9 µs, Skia: 812.5 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 875.8 µs, Skia: 1374.8 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1249.5 µs, Skia: 1398.6 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1137.9 µs, Skia: 3921.7 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 945.8 µs, Skia: 1168.0 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 986.6 µs, Skia: 1448.8 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 947.9 µs, Skia: 1578.7 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 1044.5 µs, Skia: 1208.9 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 888.6 µs, Skia: 1540.1 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1033.7 µs, Skia: 1652.0 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 920.0 µs, Skia: 1467.5 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 957.3 µs, Skia: 1253.1 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 879.7 µs, Skia: 1328.5 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1046.6 µs, Skia: 1330.5 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 849.1 µs, Skia: 1442.8 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 810.7 µs, Skia: 916.2 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 809.8 µs, Skia: 1069.5 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1066.6 µs, Skia: 2435.8 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1310.6 µs, Skia: 1969.0 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 965.9 µs, Skia: 2885.9 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1359.7 µs, Skia: 2035.6 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 955.0 µs, Skia: 1122.3 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1296.2 µs, Skia: 1901.5 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 927.5 µs, Skia: 1896.1 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1245.6 µs, Skia: 1740.0 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.1 µs  439.4 µs    442.4 µs   14.1 µs  479.4 µs    493.5 µs       4.6x Nisaba        1.12x Nisaba
2. 100 Grid Lines (1px Hairlines)            24.5 µs  593.8 µs    618.3 µs   99.4 µs  869.0 µs    968.3 µs       4.1x Nisaba        1.57x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      60.1 µs  882.6 µs    942.7 µs  699.1 µs 1204.4 µs   1903.5 µs      11.6x Nisaba        2.02x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      65.2 µs  701.9 µs    767.1 µs  186.5 µs  832.4 µs   1018.9 µs       2.9x Nisaba        1.33x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   58.0 µs  987.1 µs   1045.1 µs  151.6 µs 1062.7 µs   1214.4 µs       2.6x Nisaba        1.16x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     40.1 µs 1122.6 µs   1162.6 µs  314.4 µs 1296.3 µs   1610.7 µs       7.8x Nisaba        1.39x Nisaba
7. 50 Varying-Radius Rounded Rects           76.9 µs 1331.3 µs   1408.2 µs  229.4 µs 1276.3 µs   1505.7 µs       3.0x Nisaba        1.07x Nisaba
8. 50 Stroked Rounded Rects (1.5px Border)   88.1 µs 1154.6 µs   1242.8 µs  263.3 µs 1283.4 µs   1546.8 µs       3.0x Nisaba        1.24x Nisaba
9. 100 Alpha Circles / Disks (Fill)          96.5 µs 1012.4 µs   1108.9 µs  350.1 µs 1229.5 µs   1579.6 µs       3.6x Nisaba        1.42x Nisaba
10. 25 Concentric Rings (2px Stroke)        154.3 µs 1257.0 µs   1411.3 µs  188.3 µs 1111.5 µs   1299.7 µs       1.2x Nisaba          1.09x Skia
11. 50 Two-Stop Linear Gradients             76.1 µs 1346.4 µs   1422.5 µs 1020.3 µs 1717.8 µs   2738.1 µs      13.4x Nisaba        1.92x Nisaba
12. 50 Diagonal Gradients (45° Angle)       79.1 µs 1198.5 µs   1277.6 µs 1077.3 µs 1641.9 µs   2719.3 µs      13.6x Nisaba        2.13x Nisaba
13. 25 Radial Glow Gradients (300px)         69.6 µs 2697.8 µs   2767.4 µs  568.6 µs 2052.4 µs   2621.1 µs       8.2x Nisaba          1.06x Skia
14. 20 Box Blur / Drop Shadows (15px)        31.7 µs 1049.5 µs   1081.1 µs  457.5 µs 1473.4 µs   1930.9 µs      14.5x Nisaba        1.79x Nisaba
15. 10 Concave 10-Point Stars (Fill)        136.6 µs 1177.1 µs   1313.6 µs 1000.5 µs 1355.8 µs   2356.3 µs       7.3x Nisaba        1.79x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   56.4 µs  778.5 µs    834.8 µs 1873.9 µs 1538.9 µs   3412.8 µs      33.2x Nisaba        4.09x Nisaba
17. 1000-Point Waveform (Dense Stroke)      184.5 µs  933.4 µs   1117.8 µs 1726.0 µs 1596.8 µs   3322.8 µs       9.4x Nisaba        2.97x Nisaba
18. 50 Rotated Cards (Affine Transforms)     50.2 µs  991.1 µs   1041.2 µs  210.1 µs 1073.0 µs   1283.1 µs       4.2x Nisaba        1.23x Nisaba
19. 25 Scissor Clipped Viewports             38.9 µs  732.5 µs    771.4 µs  177.4 µs  697.9 µs    875.3 µs       4.6x Nisaba        1.13x Nisaba
20. 50 UI Text Labels (Inter-Regular)       191.6 µs  890.0 µs   1081.6 µs  215.1 µs  949.5 µs   1164.6 µs       1.1x Nisaba        1.08x Nisaba
21. 25 Quadratic Bezier Curves (Fill+Stroke)  225.3 µs 1223.2 µs   1448.5 µs 1143.3 µs 1488.6 µs   2631.9 µs       5.1x Nisaba        1.82x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     83.1 µs  900.2 µs    983.2 µs 3015.2 µs 1705.1 µs   4720.2 µs      36.3x Nisaba        4.80x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       27.5 µs  779.8 µs    807.3 µs  524.1 µs 1175.7 µs   1699.8 µs      19.1x Nisaba        2.11x Nisaba
24. Circular Arcs with arcTo (20 Corners)    63.7 µs  800.2 µs    863.9 µs 1409.6 µs 1404.4 µs   2814.0 µs      22.1x Nisaba        3.26x Nisaba
25. Directional Arcs with arc() (30 Arcs)   125.5 µs  938.3 µs   1063.8 µs  434.0 µs 1188.1 µs   1622.1 µs       3.5x Nisaba        1.52x Nisaba
26. Compound Path with Holes (15 Donuts)    223.2 µs 1378.9 µs   1602.2 µs 1443.9 µs 1437.1 µs   2881.0 µs       6.5x Nisaba        1.80x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    79.3 µs  804.0 µs    883.3 µs 1212.4 µs 1301.3 µs   2513.8 µs      15.3x Nisaba        2.85x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    94.0 µs  757.1 µs    851.1 µs 2234.8 µs 1531.3 µs   3766.1 µs      23.8x Nisaba        4.43x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   20.0 µs  787.9 µs    807.9 µs  773.0 µs 1417.6 µs   2190.6 µs      38.6x Nisaba        2.71x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      63.7 µs 1074.2 µs   1138.0 µs 3327.9 µs 1820.9 µs   5148.8 µs      52.2x Nisaba        4.52x Nisaba
31. Stroke Join: Miter (12px Stroke)         22.2 µs  686.2 µs    708.3 µs 1008.0 µs 1288.2 µs   2296.1 µs      45.5x Nisaba        3.24x Nisaba
32. Stroke Join: Bevel (12px Stroke)         23.6 µs  700.4 µs    724.0 µs 1066.4 µs 1392.0 µs   2458.3 µs      45.1x Nisaba        3.40x Nisaba
33. Stroke Join: Round (12px Stroke)         31.5 µs  728.7 µs    760.1 µs 1144.8 µs 1491.8 µs   2636.6 µs      36.4x Nisaba        3.47x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      26.5 µs  676.3 µs    702.8 µs  102.2 µs  905.0 µs   1007.2 µs       3.9x Nisaba        1.43x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    28.1 µs  691.1 µs    719.2 µs  109.2 µs  889.4 µs    998.6 µs       3.9x Nisaba        1.39x Nisaba
36. Stroke Cap: Round (30 Segments 12px)     83.9 µs  797.0 µs    880.9 µs  155.4 µs  959.1 µs   1114.5 µs       1.9x Nisaba        1.27x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   38.5 µs  774.4 µs    812.8 µs 2009.7 µs 1656.7 µs   3666.4 µs      52.3x Nisaba        4.51x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   42.7 µs  834.7 µs    877.3 µs  173.2 µs 1009.7 µs   1183.0 µs       4.1x Nisaba        1.35x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   31.1 µs  803.8 µs    834.9 µs  469.6 µs 1333.0 µs   1802.6 µs      15.1x Nisaba        2.16x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   59.4 µs  826.4 µs    885.7 µs 1602.7 µs 1371.7 µs   2974.4 µs      27.0x Nisaba        3.36x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          34.3 µs  873.1 µs    907.3 µs  103.2 µs  926.3 µs   1029.4 µs       3.0x Nisaba        1.13x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       35.7 µs 1191.3 µs   1226.9 µs  124.0 µs 1067.2 µs   1191.2 µs       3.5x Nisaba         Tie (~1.0x)
43. 50 Pill / Capsule Badges (r=h/2)         65.7 µs 1021.5 µs   1087.2 µs  180.8 µs 1039.0 µs   1219.8 µs       2.8x Nisaba        1.12x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     46.1 µs 1064.4 µs   1110.5 µs  137.8 µs  880.5 µs   1018.3 µs       3.0x Nisaba          1.09x Skia
45. High-Density Disks Cloud (500 Disks)    181.3 µs 1003.4 µs   1184.8 µs  663.8 µs 1384.6 µs   2048.4 µs       3.7x Nisaba        1.73x Nisaba
46. High-Density Stroked Rings (250 Rings)   75.3 µs  964.1 µs   1039.5 µs  340.1 µs 1182.0 µs   1522.1 µs       4.5x Nisaba        1.46x Nisaba
47. 250 Filled Rectangles Batch              97.5 µs  935.0 µs   1032.5 µs  383.3 µs 1221.0 µs   1604.3 µs       3.9x Nisaba        1.55x Nisaba
48. 150 Stroked Rectangles (2px Border)      90.6 µs  829.7 µs    920.3 µs  285.5 µs 1076.2 µs   1361.6 µs       3.2x Nisaba        1.48x Nisaba
49. 40 Crosshair Aim Reticles               257.5 µs 1138.5 µs   1396.0 µs  716.7 µs 1384.5 µs   2101.1 µs       2.8x Nisaba        1.51x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     161.1 µs 1141.4 µs   1302.4 µs  750.5 µs 1380.4 µs   2130.8 µs       4.7x Nisaba        1.64x Nisaba
51. 30 Vertical Card Linear Gradients        31.5 µs 1044.4 µs   1075.9 µs  564.0 µs 1252.3 µs   1816.3 µs      17.9x Nisaba        1.69x Nisaba
52. 30 Horizontal Bar Progress Gradients     31.5 µs  991.9 µs   1023.4 µs  392.6 µs 1308.4 µs   1701.0 µs      12.5x Nisaba        1.66x Nisaba
53. 15 Concentric Radial Spotlights          26.4 µs  954.0 µs    980.3 µs  261.5 µs 1089.6 µs   1351.1 µs       9.9x Nisaba        1.38x Nisaba
54. 25 Soft Ambient Card Shadows             32.9 µs 1239.2 µs   1272.1 µs  529.5 µs 1447.0 µs   1976.5 µs      16.1x Nisaba        1.55x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      30.1 µs  897.2 µs    927.3 µs  256.3 µs 1203.4 µs   1459.7 µs       8.5x Nisaba        1.57x Nisaba
56. 16 Multi-Angle Gradient Fan Slices       67.7 µs  797.1 µs    864.9 µs  310.4 µs 1078.2 µs   1388.6 µs       4.6x Nisaba        1.61x Nisaba
57. Full-Screen Diagonal Horizon Gradient    25.2 µs 1063.8 µs   1089.0 µs   91.2 µs  885.4 µs    976.6 µs       3.6x Nisaba          1.12x Skia
58. 20 Inset Well Shadows (Recessed)        129.9 µs 1305.3 µs   1435.3 µs  283.0 µs 1244.8 µs   1527.8 µs       2.2x Nisaba        1.06x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     45.7 µs 1831.9 µs   1877.7 µs  182.4 µs 1556.8 µs   1739.2 µs       4.0x Nisaba          1.08x Skia
60. Waveform Polyline Stroked with Gradient  112.0 µs  898.9 µs   1010.9 µs 1636.7 µs 1529.4 µs   3166.1 µs      14.6x Nisaba        3.13x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   21.5 µs  891.4 µs    912.9 µs   76.0 µs  858.9 µs    935.0 µs       3.5x Nisaba         Tie (~1.0x)
62. Scaled Image Quad (2x 512x512 Blit)      20.0 µs  780.3 µs    800.2 µs   81.0 µs  853.6 µs    934.6 µs       4.1x Nisaba        1.17x Nisaba
63. 45° Rotated Pattern Fill in Circle      23.9 µs 1006.8 µs   1030.7 µs   83.5 µs  846.4 µs    929.9 µs       3.5x Nisaba          1.11x Skia
64. Translucent Texture Overlay (50% Alpha)   24.3 µs  956.6 µs    980.9 µs   98.7 µs  997.0 µs   1095.8 µs       4.1x Nisaba        1.12x Nisaba
65. Multi-Avatar Grid (16 Avatars)           37.9 µs  867.9 µs    905.7 µs  104.3 µs  902.8 µs   1007.0 µs       2.8x Nisaba        1.11x Nisaba
66. Texture Pattern on Star Path             50.1 µs  904.5 µs    954.6 µs  468.0 µs 1236.2 µs   1704.2 µs       9.3x Nisaba        1.79x Nisaba
67. 20px Curved Path Stroked with Texture    39.2 µs  973.4 µs   1012.7 µs  503.9 µs 1245.3 µs   1749.2 µs      12.8x Nisaba        1.73x Nisaba
68. Minified Texture Quad (0.25x Downscale)   41.8 µs  863.8 µs    905.7 µs  175.1 µs  977.8 µs   1152.9 µs       4.2x Nisaba        1.27x Nisaba
69. Pure Translation Stack (50 Cards)        93.6 µs 1083.6 µs   1177.2 µs  288.2 µs 1309.4 µs   1597.6 µs       3.1x Nisaba        1.36x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   69.7 µs  892.9 µs    962.7 µs  232.8 µs 1240.0 µs   1472.8 µs       3.3x Nisaba        1.53x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   86.6 µs  965.6 µs   1052.2 µs  118.2 µs  964.9 µs   1083.0 µs       1.4x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   40.0 µs  881.3 µs    921.3 µs  186.9 µs 1119.1 µs   1306.0 µs       4.7x Nisaba        1.42x Nisaba
73. Combined Affine Transforms (30 Shapes)   62.9 µs  953.8 µs   1016.7 µs  194.4 µs 1223.9 µs   1418.4 µs       3.1x Nisaba        1.40x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   55.2 µs 1118.2 µs   1173.4 µs  156.7 µs 1041.5 µs   1198.2 µs       2.8x Nisaba         Tie (~1.0x)
75. Reset Transform Stress (50 Cycles)       64.6 µs  865.4 µs    930.0 µs  243.6 µs 1224.0 µs   1467.6 µs       3.8x Nisaba        1.58x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   27.8 µs  669.1 µs    696.9 µs   95.0 µs  717.4 µs    812.5 µs       3.4x Nisaba        1.17x Nisaba
77. Multi-Scissor Grid (16 Viewports)        41.2 µs  834.6 µs    875.8 µs  286.7 µs 1088.1 µs   1374.8 µs       7.0x Nisaba        1.57x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   57.1 µs 1192.4 µs   1249.5 µs  175.0 µs 1223.6 µs   1398.6 µs       3.1x Nisaba        1.12x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  217.8 µs  920.1 µs   1137.9 µs 2261.8 µs 1659.9 µs   3921.7 µs      10.4x Nisaba        3.45x Nisaba
80. Scissored Gradient Card (Clipped)        42.1 µs  903.7 µs    945.8 µs  260.4 µs  907.6 µs   1168.0 µs       6.2x Nisaba        1.23x Nisaba
81. Circular Overflow Clip (25 Boxes)        52.6 µs  934.0 µs    986.6 µs  359.7 µs 1089.1 µs   1448.8 µs       6.8x Nisaba        1.47x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   53.5 µs  894.3 µs    947.9 µs  234.2 µs 1344.5 µs   1578.7 µs       4.4x Nisaba        1.67x Nisaba
83. Composite Op: Source-Over (30 Shapes)    61.1 µs  983.4 µs   1044.5 µs  182.5 µs 1026.4 µs   1208.9 µs       3.0x Nisaba        1.16x Nisaba
84. Composite Op: Lighter / Plus (30 Particles)   55.1 µs  833.5 µs    888.6 µs  214.7 µs 1325.4 µs   1540.1 µs       3.9x Nisaba        1.73x Nisaba
85. Composite Op: Source-In (Alpha Masking)   54.2 µs  979.5 µs   1033.7 µs  200.1 µs 1451.9 µs   1652.0 µs       3.7x Nisaba        1.60x Nisaba
86. Composite Op: Source-Out (Cutout)        51.5 µs  868.5 µs    920.0 µs  197.2 µs 1270.2 µs   1467.5 µs       3.8x Nisaba        1.60x Nisaba
87. Composite Op: Atop (Target Bounds)       51.1 µs  906.2 µs    957.3 µs  183.0 µs 1070.2 µs   1253.1 µs       3.6x Nisaba        1.31x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   50.2 µs  829.5 µs    879.7 µs  189.6 µs 1138.9 µs   1328.5 µs       3.8x Nisaba        1.51x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     53.5 µs  993.1 µs   1046.6 µs  189.9 µs 1140.5 µs   1330.5 µs       3.5x Nisaba        1.27x Nisaba
90. Composite Op: Dest-Atop (Inverse)        47.8 µs  801.3 µs    849.1 µs  198.4 µs 1244.4 µs   1442.8 µs       4.2x Nisaba        1.70x Nisaba
91. Composite Op: Xor (Exclusive Blend)      21.8 µs  788.9 µs    810.7 µs  102.9 µs  813.3 µs    916.2 µs       4.7x Nisaba        1.13x Nisaba
92. Composite Op: Copy (Direct Overwrite)    44.5 µs  765.4 µs    809.8 µs  139.2 µs  930.3 µs   1069.5 µs       3.1x Nisaba        1.32x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   87.7 µs  978.9 µs   1066.6 µs  932.4 µs 1503.3 µs   2435.8 µs      10.6x Nisaba        2.28x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      97.6 µs 1213.1 µs   1310.6 µs  595.2 µs 1373.9 µs   1969.0 µs       6.1x Nisaba        1.50x Nisaba
95. Circular Progress Rings (8 Meters)       94.5 µs  871.4 µs    965.9 µs 1510.0 µs 1375.9 µs   2885.9 µs      16.0x Nisaba        2.99x Nisaba
96. Modern Card Stack (5 Elevated Cards)     51.6 µs 1308.1 µs   1359.7 µs  427.2 µs 1608.4 µs   2035.6 µs       8.3x Nisaba        1.50x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   49.1 µs  905.9 µs    955.0 µs  121.1 µs 1001.2 µs   1122.3 µs       2.5x Nisaba        1.18x Nisaba
98. Floating Action Button (FAB 10 Buttons)  135.2 µs 1161.0 µs   1296.2 µs  503.5 µs 1398.0 µs   1901.5 µs       3.7x Nisaba        1.47x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   94.6 µs  832.9 µs    927.5 µs  517.9 µs 1378.3 µs   1896.1 µs       5.5x Nisaba        2.04x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   78.0 µs 1167.6 µs   1245.6 µs  344.2 µs 1395.8 µs   1740.0 µs       4.4x Nisaba        1.40x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          6954.6 µs96968.0 µs 103922.6 µs55808.4 µs122163.5 µs 177971.9 µs      8.02x Nisaba        1.71x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 90 suites | Skia won in 6 suites | Ties: 4
         Overall Frame Throughput (End-to-End): 1.71x Nisaba
         Overall CPU Command Submission:       8.02x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 434.9 µs, Skia: 471.5 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 587.1 µs, Skia: 812.8 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 626.1 µs, Skia: 1757.6 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 804.9 µs, Skia: 905.3 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1097.7 µs, Skia: 1004.7 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1051.6 µs, Skia: 1079.4 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1074.7 µs, Skia: 1020.8 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 992.3 µs, Skia: 1084.3 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 969.7 µs, Skia: 1263.7 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 841.9 µs, Skia: 1085.6 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1482.0 µs, Skia: 2330.5 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1285.4 µs, Skia: 1989.5 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 3487.0 µs, Skia: 3015.1 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1261.4 µs, Skia: 1606.7 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1499.9 µs, Skia: 2475.4 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 717.3 µs, Skia: 3692.6 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 1182.7 µs, Skia: 3558.2 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1081.9 µs, Skia: 1698.1 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 1102.2 µs, Skia: 1528.4 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1768.2 µs, Skia: 1464.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1825.6 µs, Skia: 2504.8 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 843.8 µs, Skia: 4502.4 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 647.5 µs, Skia: 1795.6 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 1035.4 µs, Skia: 2842.8 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1241.7 µs, Skia: 1503.8 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1964.6 µs, Skia: 2472.8 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 737.1 µs, Skia: 2859.7 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 803.8 µs, Skia: 3793.9 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 701.0 µs, Skia: 2371.9 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1032.1 µs, Skia: 4817.8 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 718.0 µs, Skia: 2700.5 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 775.0 µs, Skia: 2717.2 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 824.0 µs, Skia: 2485.6 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 843.3 µs, Skia: 1386.8 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 906.7 µs, Skia: 1026.3 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 1363.0 µs, Skia: 1685.7 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 1125.8 µs, Skia: 4481.2 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 1019.3 µs, Skia: 1414.2 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 927.4 µs, Skia: 1848.6 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 1296.7 µs, Skia: 2996.7 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 746.3 µs, Skia: 969.9 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 988.7 µs, Skia: 1011.8 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 893.3 µs, Skia: 1162.5 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 984.8 µs, Skia: 1018.2 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 1377.7 µs, Skia: 2001.7 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1001.1 µs, Skia: 2065.9 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1198.3 µs, Skia: 1695.7 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1231.4 µs, Skia: 1395.7 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1554.2 µs, Skia: 2258.9 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1736.9 µs, Skia: 3239.4 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1099.3 µs, Skia: 1194.8 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1205.7 µs, Skia: 2004.2 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 990.9 µs, Skia: 1524.0 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1242.3 µs, Skia: 2021.4 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 923.3 µs, Skia: 1851.7 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1264.8 µs, Skia: 2059.2 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1023.4 µs, Skia: 1136.4 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1613.9 µs, Skia: 2133.9 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1705.6 µs, Skia: 1883.7 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1025.8 µs, Skia: 2949.8 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 749.7 µs, Skia: 1175.6 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 898.8 µs, Skia: 1073.2 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 906.3 µs, Skia: 906.4 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 983.3 µs, Skia: 1078.8 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 908.4 µs, Skia: 1035.8 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 1005.4 µs, Skia: 1750.8 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 736.4 µs, Skia: 2216.5 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 1109.9 µs, Skia: 1177.3 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 911.8 µs, Skia: 1599.9 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 1102.2 µs, Skia: 1403.5 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 1203.7 µs, Skia: 1165.2 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 959.3 µs, Skia: 1505.4 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 1137.6 µs, Skia: 1318.0 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 883.1 µs, Skia: 1409.8 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 1265.2 µs, Skia: 1551.0 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 1023.4 µs, Skia: 1219.6 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 865.8 µs, Skia: 1325.6 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 1144.0 µs, Skia: 1361.0 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1779.9 µs, Skia: 5238.3 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 1030.2 µs, Skia: 1294.3 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 717.7 µs, Skia: 1188.4 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 805.0 µs, Skia: 1217.4 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 830.4 µs, Skia: 1044.7 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 1017.8 µs, Skia: 1439.6 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 1042.8 µs, Skia: 1485.3 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 946.4 µs, Skia: 1555.1 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 978.6 µs, Skia: 1301.0 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 1098.4 µs, Skia: 1347.9 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 1076.9 µs, Skia: 1463.6 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 903.1 µs, Skia: 1442.0 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 887.0 µs, Skia: 1408.2 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 951.1 µs, Skia: 1272.2 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 1164.0 µs, Skia: 2626.1 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1455.0 µs, Skia: 3211.7 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1279.7 µs, Skia: 3907.7 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1680.4 µs, Skia: 2041.0 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 991.9 µs, Skia: 1573.1 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1596.3 µs, Skia: 2008.8 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1125.3 µs, Skia: 2116.4 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1393.9 µs, Skia: 1625.3 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.7 µs  431.2 µs    434.9 µs   13.2 µs  458.3 µs    471.5 µs       3.6x Nisaba        1.08x Nisaba
2. 100 Grid Lines (1px Hairlines)            25.9 µs  561.2 µs    587.1 µs   81.1 µs  731.7 µs    812.8 µs       3.1x Nisaba        1.38x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      45.2 µs  580.9 µs    626.1 µs  526.1 µs 1231.6 µs   1757.6 µs      11.6x Nisaba        2.81x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      49.5 µs  755.4 µs    804.9 µs  139.5 µs  765.8 µs    905.3 µs       2.8x Nisaba        1.12x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)   46.3 µs 1051.4 µs   1097.7 µs  148.5 µs  856.1 µs   1004.7 µs       3.2x Nisaba          1.09x Skia
6. 50 Simple Rounded Rects (12px Radius)     34.4 µs 1017.3 µs   1051.6 µs  132.1 µs  947.3 µs   1079.4 µs       3.8x Nisaba         Tie (~1.0x)
7. 50 Varying-Radius Rounded Rects           39.3 µs 1035.4 µs   1074.7 µs  108.0 µs  912.8 µs   1020.8 µs       2.7x Nisaba          1.05x Skia
8. 50 Stroked Rounded Rects (1.5px Border)   52.0 µs  940.3 µs    992.3 µs  163.3 µs  921.0 µs   1084.3 µs       3.1x Nisaba        1.09x Nisaba
9. 100 Alpha Circles / Disks (Fill)          54.4 µs  915.4 µs    969.7 µs  194.9 µs 1068.8 µs   1263.7 µs       3.6x Nisaba        1.30x Nisaba
10. 25 Concentric Rings (2px Stroke)         82.2 µs  759.7 µs    841.9 µs   95.8 µs  989.8 µs   1085.6 µs       1.2x Nisaba        1.29x Nisaba
11. 50 Two-Stop Linear Gradients             71.3 µs 1410.7 µs   1482.0 µs  737.9 µs 1592.6 µs   2330.5 µs      10.4x Nisaba        1.57x Nisaba
12. 50 Diagonal Gradients (45° Angle)       59.4 µs 1226.0 µs   1285.4 µs  583.8 µs 1405.7 µs   1989.5 µs       9.8x Nisaba        1.55x Nisaba
13. 25 Radial Glow Gradients (300px)         58.9 µs 3428.0 µs   3487.0 µs  566.8 µs 2448.3 µs   3015.1 µs       9.6x Nisaba          1.16x Skia
14. 20 Box Blur / Drop Shadows (15px)        30.1 µs 1231.3 µs   1261.4 µs  263.0 µs 1343.7 µs   1606.7 µs       8.7x Nisaba        1.27x Nisaba
15. 10 Concave 10-Point Stars (Fill)        156.0 µs 1343.9 µs   1499.9 µs 1079.1 µs 1396.2 µs   2475.4 µs       6.9x Nisaba        1.65x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   64.1 µs  653.3 µs    717.3 µs 2172.3 µs 1520.3 µs   3692.6 µs      33.9x Nisaba        5.15x Nisaba
17. 1000-Point Waveform (Dense Stroke)      171.9 µs 1010.7 µs   1182.7 µs 2030.8 µs 1527.4 µs   3558.2 µs      11.8x Nisaba        3.01x Nisaba
18. 50 Rotated Cards (Affine Transforms)     54.8 µs 1027.0 µs   1081.9 µs  305.4 µs 1392.6 µs   1698.1 µs       5.6x Nisaba        1.57x Nisaba
19. 25 Scissor Clipped Viewports             62.2 µs 1040.0 µs   1102.2 µs  357.8 µs 1170.7 µs   1528.4 µs       5.8x Nisaba        1.39x Nisaba
20. 50 UI Text Labels (Inter-Regular)       529.0 µs 1239.2 µs   1768.2 µs  329.7 µs 1134.8 µs   1464.6 µs         1.6x Skia          1.21x Skia
21. 25 Quadratic Bezier Curves (Fill+Stroke)  318.0 µs 1507.6 µs   1825.6 µs 1080.4 µs 1424.4 µs   2504.8 µs       3.4x Nisaba        1.37x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     72.2 µs  771.6 µs    843.8 µs 2771.5 µs 1730.9 µs   4502.4 µs      38.4x Nisaba        5.34x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       23.4 µs  624.0 µs    647.5 µs  602.5 µs 1193.1 µs   1795.6 µs      25.7x Nisaba        2.77x Nisaba
24. Circular Arcs with arcTo (20 Corners)    85.1 µs  950.2 µs   1035.4 µs 1300.6 µs 1542.3 µs   2842.8 µs      15.3x Nisaba        2.75x Nisaba
25. Directional Arcs with arc() (30 Arcs)   174.7 µs 1067.0 µs   1241.7 µs  369.4 µs 1134.4 µs   1503.8 µs       2.1x Nisaba        1.21x Nisaba
26. Compound Path with Holes (15 Donuts)    386.1 µs 1578.5 µs   1964.6 µs 1180.3 µs 1292.5 µs   2472.8 µs       3.1x Nisaba        1.26x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    66.9 µs  670.2 µs    737.1 µs 1392.5 µs 1467.2 µs   2859.7 µs      20.8x Nisaba        3.88x Nisaba
28. Archimedean Spiral Polyline (500 Pts)    83.3 µs  720.4 µs    803.8 µs 2156.0 µs 1637.8 µs   3793.9 µs      25.9x Nisaba        4.72x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   18.8 µs  682.3 µs    701.0 µs  915.3 µs 1456.6 µs   2371.9 µs      48.7x Nisaba        3.38x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      64.0 µs  968.2 µs   1032.1 µs 3113.9 µs 1703.8 µs   4817.8 µs      48.7x Nisaba        4.67x Nisaba
31. Stroke Join: Miter (12px Stroke)         26.5 µs  691.5 µs    718.0 µs 1238.7 µs 1461.8 µs   2700.5 µs      46.7x Nisaba        3.76x Nisaba
32. Stroke Join: Bevel (12px Stroke)         25.5 µs  749.5 µs    775.0 µs 1309.6 µs 1407.6 µs   2717.2 µs      51.4x Nisaba        3.51x Nisaba
33. Stroke Join: Round (12px Stroke)         28.8 µs  795.1 µs    824.0 µs 1060.9 µs 1424.7 µs   2485.6 µs      36.8x Nisaba        3.02x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      32.4 µs  810.9 µs    843.3 µs  202.4 µs 1184.4 µs   1386.8 µs       6.3x Nisaba        1.64x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    53.3 µs  853.5 µs    906.7 µs  136.4 µs  889.9 µs   1026.3 µs       2.6x Nisaba        1.13x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    152.9 µs 1210.1 µs   1363.0 µs  310.5 µs 1375.2 µs   1685.7 µs       2.0x Nisaba        1.24x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   66.5 µs 1059.3 µs   1125.8 µs 2911.3 µs 1569.8 µs   4481.2 µs      43.8x Nisaba        3.98x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   63.8 µs  955.4 µs   1019.3 µs  275.6 µs 1138.6 µs   1414.2 µs       4.3x Nisaba        1.39x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   32.1 µs  895.3 µs    927.4 µs  560.9 µs 1287.7 µs   1848.6 µs      17.5x Nisaba        1.99x Nisaba
40. Stepped Variable Stroke Widths (1-20px)  112.1 µs 1184.6 µs   1296.7 µs 1457.8 µs 1538.9 µs   2996.7 µs      13.0x Nisaba        2.31x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          26.6 µs  719.7 µs    746.3 µs   93.6 µs  876.2 µs    969.9 µs       3.5x Nisaba        1.30x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       37.3 µs  951.4 µs    988.7 µs  111.3 µs  900.5 µs   1011.8 µs       3.0x Nisaba         Tie (~1.0x)
43. 50 Pill / Capsule Badges (r=h/2)         61.5 µs  831.8 µs    893.3 µs  181.9 µs  980.6 µs   1162.5 µs       3.0x Nisaba        1.30x Nisaba
44. 50 Asymmetric Teardrop Rounded Rects     49.0 µs  935.8 µs    984.8 µs  150.7 µs  867.4 µs   1018.2 µs       3.1x Nisaba         Tie (~1.0x)
45. High-Density Disks Cloud (500 Disks)    201.4 µs 1176.3 µs   1377.7 µs  603.7 µs 1397.9 µs   2001.7 µs       3.0x Nisaba        1.45x Nisaba
46. High-Density Stroked Rings (250 Rings)   77.9 µs  923.2 µs   1001.1 µs  639.1 µs 1426.8 µs   2065.9 µs       8.2x Nisaba        2.06x Nisaba
47. 250 Filled Rectangles Batch             129.4 µs 1068.8 µs   1198.3 µs  464.1 µs 1231.7 µs   1695.7 µs       3.6x Nisaba        1.42x Nisaba
48. 150 Stroked Rectangles (2px Border)     151.2 µs 1080.2 µs   1231.4 µs  273.3 µs 1122.5 µs   1395.7 µs       1.8x Nisaba        1.13x Nisaba
49. 40 Crosshair Aim Reticles               260.6 µs 1293.6 µs   1554.2 µs  938.1 µs 1320.8 µs   2258.9 µs       3.6x Nisaba        1.45x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     304.8 µs 1432.1 µs   1736.9 µs 1648.6 µs 1590.8 µs   3239.4 µs       5.4x Nisaba        1.87x Nisaba
51. 30 Vertical Card Linear Gradients        39.2 µs 1060.1 µs   1099.3 µs  275.7 µs  919.1 µs   1194.8 µs       7.0x Nisaba        1.09x Nisaba
52. 30 Horizontal Bar Progress Gradients     55.0 µs 1150.7 µs   1205.7 µs  599.4 µs 1404.7 µs   2004.2 µs      10.9x Nisaba        1.66x Nisaba
53. 15 Concentric Radial Spotlights          33.4 µs  957.5 µs    990.9 µs  344.9 µs 1179.0 µs   1524.0 µs      10.3x Nisaba        1.54x Nisaba
54. 25 Soft Ambient Card Shadows             54.8 µs 1187.5 µs   1242.3 µs  647.4 µs 1374.0 µs   2021.4 µs      11.8x Nisaba        1.63x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      33.1 µs  890.2 µs    923.3 µs  464.5 µs 1387.3 µs   1851.7 µs      14.0x Nisaba        2.01x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      132.7 µs 1132.1 µs   1264.8 µs  576.5 µs 1482.6 µs   2059.2 µs       4.3x Nisaba        1.63x Nisaba
57. Full-Screen Diagonal Horizon Gradient    36.3 µs  987.2 µs   1023.4 µs  139.9 µs  996.5 µs   1136.4 µs       3.9x Nisaba        1.11x Nisaba
58. 20 Inset Well Shadows (Recessed)        239.6 µs 1374.3 µs   1613.9 µs  606.0 µs 1527.9 µs   2133.9 µs       2.5x Nisaba        1.32x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     60.2 µs 1645.4 µs   1705.6 µs  318.1 µs 1565.6 µs   1883.7 µs       5.3x Nisaba        1.10x Nisaba
60. Waveform Polyline Stroked with Gradient  130.2 µs  895.7 µs   1025.8 µs 1496.3 µs 1453.5 µs   2949.8 µs      11.5x Nisaba        2.88x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   18.7 µs  731.0 µs    749.7 µs  136.8 µs 1038.8 µs   1175.6 µs       7.3x Nisaba        1.57x Nisaba
62. Scaled Image Quad (2x 512x512 Blit)      23.7 µs  875.1 µs    898.8 µs  113.2 µs  960.0 µs   1073.2 µs       4.8x Nisaba        1.19x Nisaba
63. 45° Rotated Pattern Fill in Circle      26.8 µs  879.4 µs    906.3 µs  104.2 µs  802.3 µs    906.4 µs       3.9x Nisaba         Tie (~1.0x)
64. Translucent Texture Overlay (50% Alpha)   30.5 µs  952.8 µs    983.3 µs  105.1 µs  973.8 µs   1078.8 µs       3.4x Nisaba        1.10x Nisaba
65. Multi-Avatar Grid (16 Avatars)           33.8 µs  874.7 µs    908.4 µs  134.2 µs  901.6 µs   1035.8 µs       4.0x Nisaba        1.14x Nisaba
66. Texture Pattern on Star Path             54.5 µs  951.0 µs   1005.4 µs  515.1 µs 1235.6 µs   1750.8 µs       9.5x Nisaba        1.74x Nisaba
67. 20px Curved Path Stroked with Texture    36.6 µs  699.7 µs    736.4 µs  754.4 µs 1462.2 µs   2216.5 µs      20.6x Nisaba        3.01x Nisaba
68. Minified Texture Quad (0.25x Downscale)   77.4 µs 1032.5 µs   1109.9 µs  185.6 µs  991.7 µs   1177.3 µs       2.4x Nisaba        1.06x Nisaba
69. Pure Translation Stack (50 Cards)        63.8 µs  848.0 µs    911.8 µs  299.2 µs 1300.7 µs   1599.9 µs       4.7x Nisaba        1.75x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   64.2 µs 1037.9 µs   1102.2 µs  270.8 µs 1132.8 µs   1403.5 µs       4.2x Nisaba        1.27x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   92.8 µs 1110.9 µs   1203.7 µs  161.8 µs 1003.4 µs   1165.2 µs       1.7x Nisaba         Tie (~1.0x)
72. Shear / Skew Parallelograms (30 Quads)   60.5 µs  898.8 µs    959.3 µs  240.5 µs 1264.9 µs   1505.4 µs       4.0x Nisaba        1.57x Nisaba
73. Combined Affine Transforms (30 Shapes)   66.7 µs 1070.9 µs   1137.6 µs  191.6 µs 1126.4 µs   1318.0 µs       2.9x Nisaba        1.16x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   52.6 µs  830.5 µs    883.1 µs  207.9 µs 1201.9 µs   1409.8 µs       4.0x Nisaba        1.60x Nisaba
75. Reset Transform Stress (50 Cycles)       83.0 µs 1182.2 µs   1265.2 µs  299.7 µs 1251.3 µs   1551.0 µs       3.6x Nisaba        1.23x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   52.9 µs  970.5 µs   1023.4 µs  180.9 µs 1038.7 µs   1219.6 µs       3.4x Nisaba        1.19x Nisaba
77. Multi-Scissor Grid (16 Viewports)        47.6 µs  818.2 µs    865.8 µs  285.5 µs 1040.0 µs   1325.6 µs       6.0x Nisaba        1.53x Nisaba
78. Hierarchical Intersecting Scissors (4 Levels)   54.3 µs 1089.7 µs   1144.0 µs  183.6 µs 1177.4 µs   1361.0 µs       3.4x Nisaba        1.19x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  458.8 µs 1321.2 µs   1779.9 µs 3624.6 µs 1613.7 µs   5238.3 µs       7.9x Nisaba        2.94x Nisaba
80. Scissored Gradient Card (Clipped)        42.0 µs  988.1 µs   1030.2 µs  338.0 µs  956.3 µs   1294.3 µs       8.0x Nisaba        1.26x Nisaba
81. Circular Overflow Clip (25 Boxes)        26.9 µs  690.7 µs    717.7 µs  280.0 µs  908.4 µs   1188.4 µs      10.4x Nisaba        1.66x Nisaba
82. Scissor Invalidation / Reset (25 Cycles)   39.3 µs  765.7 µs    805.0 µs  181.3 µs 1036.1 µs   1217.4 µs       4.6x Nisaba        1.51x Nisaba
83. Composite Op: Source-Over (30 Shapes)    48.8 µs  781.6 µs    830.4 µs  108.2 µs  936.5 µs   1044.7 µs       2.2x Nisaba        1.26x Nisaba
84. Composite Op: Lighter / Plus (30 Particles)   50.6 µs  967.1 µs   1017.8 µs  210.7 µs 1228.9 µs   1439.6 µs       4.2x Nisaba        1.41x Nisaba
85. Composite Op: Source-In (Alpha Masking)   53.8 µs  988.9 µs   1042.8 µs  189.0 µs 1296.3 µs   1485.3 µs       3.5x Nisaba        1.42x Nisaba
86. Composite Op: Source-Out (Cutout)        53.5 µs  892.9 µs    946.4 µs  193.1 µs 1362.0 µs   1555.1 µs       3.6x Nisaba        1.64x Nisaba
87. Composite Op: Atop (Target Bounds)       53.8 µs  924.7 µs    978.6 µs  188.5 µs 1112.5 µs   1301.0 µs       3.5x Nisaba        1.33x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   54.7 µs 1043.7 µs   1098.4 µs  186.3 µs 1161.6 µs   1347.9 µs       3.4x Nisaba        1.23x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     52.5 µs 1024.4 µs   1076.9 µs  191.2 µs 1272.3 µs   1463.6 µs       3.6x Nisaba        1.36x Nisaba
90. Composite Op: Dest-Atop (Inverse)        48.3 µs  854.7 µs    903.1 µs  195.4 µs 1246.6 µs   1442.0 µs       4.0x Nisaba        1.60x Nisaba
91. Composite Op: Xor (Exclusive Blend)      48.9 µs  838.2 µs    887.0 µs  204.6 µs 1203.6 µs   1408.2 µs       4.2x Nisaba        1.59x Nisaba
92. Composite Op: Copy (Direct Overwrite)    52.1 µs  899.0 µs    951.1 µs  192.1 µs 1080.2 µs   1272.2 µs       3.7x Nisaba        1.34x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   97.5 µs 1066.5 µs   1164.0 µs 1145.2 µs 1480.9 µs   2626.1 µs      11.7x Nisaba        2.26x Nisaba
94. Audio Spectrum Visualizer (64 Bars)     144.5 µs 1310.6 µs   1455.0 µs 1544.6 µs 1667.1 µs   3211.7 µs      10.7x Nisaba        2.21x Nisaba
95. Circular Progress Rings (8 Meters)      176.1 µs 1103.6 µs   1279.7 µs 2164.4 µs 1743.3 µs   3907.7 µs      12.3x Nisaba        3.05x Nisaba
96. Modern Card Stack (5 Elevated Cards)     76.9 µs 1603.5 µs   1680.4 µs  421.0 µs 1619.9 µs   2041.0 µs       5.5x Nisaba        1.21x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)  100.2 µs  891.7 µs    991.9 µs  197.1 µs 1376.0 µs   1573.1 µs       2.0x Nisaba        1.59x Nisaba
98. Floating Action Button (FAB 10 Buttons)  173.2 µs 1423.1 µs   1596.3 µs  655.9 µs 1353.0 µs   2008.8 µs       3.8x Nisaba        1.26x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)  155.1 µs  970.2 µs   1125.3 µs  637.7 µs 1478.7 µs   2116.4 µs       4.1x Nisaba        1.88x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   88.6 µs 1305.3 µs   1393.9 µs  327.5 µs 1297.8 µs   1625.3 µs       3.7x Nisaba        1.17x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          8728.0 µs101606.5 µs 110334.4 µs62695.4 µs125017.9 µs 187713.2 µs      7.18x Nisaba        1.70x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 91 suites | Skia won in 4 suites | Ties: 5
         Overall Frame Throughput (End-to-End): 1.70x Nisaba
         Overall CPU Command Submission:       7.18x Nisaba
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

Running suite: 1. Solid Background Clear (1080x720)... Done. (Nisaba: 513.7 µs, Skia: 579.9 µs)
Running suite: 2. 100 Grid Lines (1px Hairlines)... Done. (Nisaba: 851.3 µs, Skia: 1191.0 µs)
Running suite: 3. 20 Thick Diagonal Lines (16px Round)... Done. (Nisaba: 1012.0 µs, Skia: 2342.6 µs)
Running suite: 4. 100 Solid Rectangles (Varied Bounds)... Done. (Nisaba: 1063.7 µs, Skia: 1493.5 µs)
Running suite: 5. 100 Alpha Rectangles (Overlapping Blend)... Done. (Nisaba: 1350.5 µs, Skia: 1739.3 µs)
Running suite: 6. 50 Simple Rounded Rects (12px Radius)... Done. (Nisaba: 1356.9 µs, Skia: 1808.6 µs)
Running suite: 7. 50 Varying-Radius Rounded Rects... Done. (Nisaba: 1362.7 µs, Skia: 1420.2 µs)
Running suite: 8. 50 Stroked Rounded Rects (1.5px Border)... Done. (Nisaba: 1142.7 µs, Skia: 1565.6 µs)
Running suite: 9. 100 Alpha Circles / Disks (Fill)... Done. (Nisaba: 1027.6 µs, Skia: 1729.4 µs)
Running suite: 10. 25 Concentric Rings (2px Stroke)... Done. (Nisaba: 1212.9 µs, Skia: 1602.6 µs)
Running suite: 11. 50 Two-Stop Linear Gradients... Done. (Nisaba: 1445.6 µs, Skia: 2494.8 µs)
Running suite: 12. 50 Diagonal Gradients (45° Angle)... Done. (Nisaba: 1441.8 µs, Skia: 2703.4 µs)
Running suite: 13. 25 Radial Glow Gradients (300px)... Done. (Nisaba: 2597.2 µs, Skia: 2706.0 µs)
Running suite: 14. 20 Box Blur / Drop Shadows (15px)... Done. (Nisaba: 1215.0 µs, Skia: 1189.4 µs)
Running suite: 15. 10 Concave 10-Point Stars (Fill)... Done. (Nisaba: 1518.8 µs, Skia: 2437.4 µs)
Running suite: 16. 10 Concave 10-Point Stars (3px Stroke)... Done. (Nisaba: 931.0 µs, Skia: 3455.3 µs)
Running suite: 17. 1000-Point Waveform (Dense Stroke)... Done. (Nisaba: 966.6 µs, Skia: 3370.7 µs)
Running suite: 18. 50 Rotated Cards (Affine Transforms)... Done. (Nisaba: 1152.5 µs, Skia: 1297.6 µs)
Running suite: 19. 25 Scissor Clipped Viewports... Done. (Nisaba: 872.9 µs, Skia: 939.2 µs)
Running suite: 20. 50 UI Text Labels (Inter-Regular)... Done. (Nisaba: 1729.5 µs, Skia: 1197.6 µs)
Running suite: 21. 25 Quadratic Bezier Curves (Fill+Stroke)... Done. (Nisaba: 1123.1 µs, Skia: 2446.3 µs)
Running suite: 22. 20 Cubic Bezier Splines (4px Stroke)... Done. (Nisaba: 808.9 µs, Skia: 4453.2 µs)
Running suite: 23. Complex Bezier Ribbon (24px Alpha)... Done. (Nisaba: 922.5 µs, Skia: 2483.9 µs)
Running suite: 24. Circular Arcs with arcTo (20 Corners)... Done. (Nisaba: 961.9 µs, Skia: 3212.2 µs)
Running suite: 25. Directional Arcs with arc() (30 Arcs)... Done. (Nisaba: 1427.1 µs, Skia: 1268.5 µs)
Running suite: 26. Compound Path with Holes (15 Donuts)... Done. (Nisaba: 1343.1 µs, Skia: 2896.3 µs)
Running suite: 27. Trefoil Figure-8 Cubic Knot (8 Loops)... Done. (Nisaba: 941.9 µs, Skia: 2408.6 µs)
Running suite: 28. Archimedean Spiral Polyline (500 Pts)... Done. (Nisaba: 1289.3 µs, Skia: 3036.9 µs)
Running suite: 29. Multi-Contour Disjoint Paths (25 Sub-Paths)... Done. (Nisaba: 679.3 µs, Skia: 2591.8 µs)
Running suite: 30. Sharp Zigzag Mountain (50 Vertices)... Done. (Nisaba: 1002.1 µs, Skia: 3512.2 µs)
Running suite: 31. Stroke Join: Miter (12px Stroke)... Done. (Nisaba: 600.2 µs, Skia: 2583.4 µs)
Running suite: 32. Stroke Join: Bevel (12px Stroke)... Done. (Nisaba: 796.0 µs, Skia: 2592.7 µs)
Running suite: 33. Stroke Join: Round (12px Stroke)... Done. (Nisaba: 636.2 µs, Skia: 2539.4 µs)
Running suite: 34. Stroke Cap: Butt (30 Segments 12px)... Done. (Nisaba: 617.7 µs, Skia: 962.2 µs)
Running suite: 35. Stroke Cap: Square (30 Segments 12px)... Done. (Nisaba: 699.8 µs, Skia: 1076.8 µs)
Running suite: 36. Stroke Cap: Round (30 Segments 12px)... Done. (Nisaba: 976.1 µs, Skia: 1195.2 µs)
Running suite: 37. Miter Limit Clamping (10° Acute Spikes)... Done. (Nisaba: 695.0 µs, Skia: 3420.4 µs)
Running suite: 38. Sub-Pixel Hairline Strokes (0.25px Lines)... Done. (Nisaba: 1013.1 µs, Skia: 1291.3 µs)
Running suite: 39. Ultra-Heavy Geometric Ribbon (40px Wide)... Done. (Nisaba: 575.9 µs, Skia: 1620.9 µs)
Running suite: 40. Stepped Variable Stroke Widths (1-20px)... Done. (Nisaba: 837.9 µs, Skia: 2638.9 µs)
Running suite: 41. 50 Ellipses (rx=60, ry=25 Fill)... Done. (Nisaba: 851.7 µs, Skia: 1120.8 µs)
Running suite: 42. 50 Stroked Ellipses (2.5px Border)... Done. (Nisaba: 1040.7 µs, Skia: 1426.3 µs)
Running suite: 43. 50 Pill / Capsule Badges (r=h/2)... Done. (Nisaba: 1417.8 µs, Skia: 798.1 µs)
Running suite: 44. 50 Asymmetric Teardrop Rounded Rects... Done. (Nisaba: 926.4 µs, Skia: 823.4 µs)
Running suite: 45. High-Density Disks Cloud (500 Disks)... Done. (Nisaba: 924.2 µs, Skia: 2580.2 µs)
Running suite: 46. High-Density Stroked Rings (250 Rings)... Done. (Nisaba: 1364.5 µs, Skia: 2110.6 µs)
Running suite: 47. 250 Filled Rectangles Batch... Done. (Nisaba: 1527.2 µs, Skia: 2102.2 µs)
Running suite: 48. 150 Stroked Rectangles (2px Border)... Done. (Nisaba: 1346.7 µs, Skia: 1750.2 µs)
Running suite: 49. 40 Crosshair Aim Reticles... Done. (Nisaba: 1594.4 µs, Skia: 3402.5 µs)
Running suite: 50. 25 Regular Octagons (Fill + Stroke)... Done. (Nisaba: 1961.8 µs, Skia: 3429.1 µs)
Running suite: 51. 30 Vertical Card Linear Gradients... Done. (Nisaba: 1405.3 µs, Skia: 2187.1 µs)
Running suite: 52. 30 Horizontal Bar Progress Gradients... Done. (Nisaba: 1299.5 µs, Skia: 2318.9 µs)
Running suite: 53. 15 Concentric Radial Spotlights... Done. (Nisaba: 1065.2 µs, Skia: 1863.0 µs)
Running suite: 54. 25 Soft Ambient Card Shadows... Done. (Nisaba: 1109.6 µs, Skia: 2267.2 µs)
Running suite: 55. 15 Neon Button Glows (Cyan Intense)... Done. (Nisaba: 992.9 µs, Skia: 1980.0 µs)
Running suite: 56. 16 Multi-Angle Gradient Fan Slices... Done. (Nisaba: 1311.3 µs, Skia: 1901.9 µs)
Running suite: 57. Full-Screen Diagonal Horizon Gradient... Done. (Nisaba: 1164.3 µs, Skia: 1135.5 µs)
Running suite: 58. 20 Inset Well Shadows (Recessed)... Done. (Nisaba: 1630.6 µs, Skia: 2053.5 µs)
Running suite: 59. 10 Pulsing Circular Radar Wave Glows... Done. (Nisaba: 1642.2 µs, Skia: 1740.7 µs)
Running suite: 60. Waveform Polyline Stroked with Gradient... Done. (Nisaba: 1456.4 µs, Skia: 3300.4 µs)
Running suite: 61. Repeated Texture Tiling (64x64 Pattern)... Done. (Nisaba: 1027.5 µs, Skia: 943.3 µs)
Running suite: 62. Scaled Image Quad (2x 512x512 Blit)... Done. (Nisaba: 1014.3 µs, Skia: 912.1 µs)
Running suite: 63. 45° Rotated Pattern Fill in Circle... Done. (Nisaba: 911.5 µs, Skia: 1097.8 µs)
Running suite: 64. Translucent Texture Overlay (50% Alpha)... Done. (Nisaba: 1030.9 µs, Skia: 1184.9 µs)
Running suite: 65. Multi-Avatar Grid (16 Avatars)... Done. (Nisaba: 964.8 µs, Skia: 1021.2 µs)
Running suite: 66. Texture Pattern on Star Path... Done. (Nisaba: 737.8 µs, Skia: 1530.6 µs)
Running suite: 67. 20px Curved Path Stroked with Texture... Done. (Nisaba: 873.3 µs, Skia: 1673.3 µs)
Running suite: 68. Minified Texture Quad (0.25x Downscale)... Done. (Nisaba: 715.4 µs, Skia: 1238.6 µs)
Running suite: 69. Pure Translation Stack (50 Cards)... Done. (Nisaba: 968.9 µs, Skia: 1257.4 µs)
Running suite: 70. Pure Rotation Cluster (36 Spokes 10°)... Done. (Nisaba: 773.6 µs, Skia: 820.9 µs)
Running suite: 71. Pure Scale Zoom Progression (20 Rects)... Done. (Nisaba: 984.5 µs, Skia: 716.3 µs)
Running suite: 72. Shear / Skew Parallelograms (30 Quads)... Done. (Nisaba: 599.7 µs, Skia: 791.3 µs)
Running suite: 73. Combined Affine Transforms (30 Shapes)... Done. (Nisaba: 771.2 µs, Skia: 898.9 µs)
Running suite: 74. Deep Hierarchical State Stack (20 Levels)... Done. (Nisaba: 839.2 µs, Skia: 1171.1 µs)
Running suite: 75. Reset Transform Stress (50 Cycles)... Done. (Nisaba: 950.7 µs, Skia: 1032.5 µs)
Running suite: 76. Planetary Orbit Hierarchy (10 Systems)... Done. (Nisaba: 599.5 µs, Skia: 685.4 µs)
Running suite: 77. Multi-Scissor Grid (16 Viewports)... Done. (Nisaba: 637.3 µs, Skia: 667.8 µs)
Running suite: 78. Hierarchical Intersecting Scissors (4 Levels)... Done. (Nisaba: 968.3 µs, Skia: 1379.8 µs)
Running suite: 79. Scissored Stroked Waves (1000 Pts Clipped)... Done. (Nisaba: 1416.7 µs, Skia: 2800.5 µs)
Running suite: 80. Scissored Gradient Card (Clipped)... Done. (Nisaba: 806.2 µs, Skia: 805.4 µs)
Running suite: 81. Circular Overflow Clip (25 Boxes)... Done. (Nisaba: 698.4 µs, Skia: 528.1 µs)
Running suite: 82. Scissor Invalidation / Reset (25 Cycles)... Done. (Nisaba: 940.3 µs, Skia: 1132.4 µs)
Running suite: 83. Composite Op: Source-Over (30 Shapes)... Done. (Nisaba: 873.6 µs, Skia: 814.9 µs)
Running suite: 84. Composite Op: Lighter / Plus (30 Particles)... Done. (Nisaba: 935.9 µs, Skia: 957.9 µs)
Running suite: 85. Composite Op: Source-In (Alpha Masking)... Done. (Nisaba: 718.3 µs, Skia: 973.9 µs)
Running suite: 86. Composite Op: Source-Out (Cutout)... Done. (Nisaba: 1002.1 µs, Skia: 1192.0 µs)
Running suite: 87. Composite Op: Atop (Target Bounds)... Done. (Nisaba: 793.5 µs, Skia: 1154.4 µs)
Running suite: 88. Composite Op: Dest-Over (Under-Drawing)... Done. (Nisaba: 762.6 µs, Skia: 1457.7 µs)
Running suite: 89. Composite Op: Dest-Out (Eraser Mask)... Done. (Nisaba: 687.9 µs, Skia: 974.6 µs)
Running suite: 90. Composite Op: Dest-Atop (Inverse)... Done. (Nisaba: 788.1 µs, Skia: 1120.2 µs)
Running suite: 91. Composite Op: Xor (Exclusive Blend)... Done. (Nisaba: 595.7 µs, Skia: 635.9 µs)
Running suite: 92. Composite Op: Copy (Direct Overwrite)... Done. (Nisaba: 694.5 µs, Skia: 1158.5 µs)
Running suite: 93. UI Dashboard Gauge (Arc Track + Needle)... Done. (Nisaba: 962.1 µs, Skia: 1532.0 µs)
Running suite: 94. Audio Spectrum Visualizer (64 Bars)... Done. (Nisaba: 1152.0 µs, Skia: 2519.0 µs)
Running suite: 95. Circular Progress Rings (8 Meters)... Done. (Nisaba: 1116.4 µs, Skia: 2178.3 µs)
Running suite: 96. Modern Card Stack (5 Elevated Cards)... Done. (Nisaba: 1298.7 µs, Skia: 1758.7 µs)
Running suite: 97. CAD Cross-Hatch Pattern (80 Angled Lines)... Done. (Nisaba: 904.0 µs, Skia: 1053.3 µs)
Running suite: 98. Floating Action Button (FAB 10 Buttons)... Done. (Nisaba: 1310.6 µs, Skia: 1734.9 µs)
Running suite: 99. Anti-Aliasing Geometry Grid (50 Diamonds)... Done. (Nisaba: 1000.3 µs, Skia: 1486.4 µs)
Running suite: 100. Master Vector Stress: Mixed Mega-Scene... Done. (Nisaba: 1157.1 µs, Skia: 1422.9 µs)


===================================================================================================================================================
                                NISABA GPU vs GOOGLE SKIA (GANESH GL) — GRANULAR BENCHMARK RESULTS                                
===================================================================================================================================================
Layer Operation Name                       Nisaba CPU Nisaba GPU Nisaba Total   Skia CPU   Skia GPU   Skia Total        CPU Winner        Total Winner
---------------------------------------------------------------------------------------------------------------------------------------------------
1. Solid Background Clear (1080x720)          3.6 µs  510.1 µs    513.7 µs   27.7 µs  552.2 µs    579.9 µs       7.6x Nisaba        1.13x Nisaba
2. 100 Grid Lines (1px Hairlines)            52.8 µs  798.5 µs    851.3 µs  141.8 µs 1049.3 µs   1191.0 µs       2.7x Nisaba        1.40x Nisaba
3. 20 Thick Diagonal Lines (16px Round)      98.0 µs  914.0 µs   1012.0 µs  999.2 µs 1343.4 µs   2342.6 µs      10.2x Nisaba        2.31x Nisaba
4. 100 Solid Rectangles (Varied Bounds)      93.6 µs  970.1 µs   1063.7 µs  292.9 µs 1200.6 µs   1493.5 µs       3.1x Nisaba        1.40x Nisaba
5. 100 Alpha Rectangles (Overlapping Blend)  106.6 µs 1243.9 µs   1350.5 µs  381.3 µs 1358.0 µs   1739.3 µs       3.6x Nisaba        1.29x Nisaba
6. 50 Simple Rounded Rects (12px Radius)     75.9 µs 1281.0 µs   1356.9 µs  307.4 µs 1501.3 µs   1808.6 µs       4.0x Nisaba        1.33x Nisaba
7. 50 Varying-Radius Rounded Rects           77.4 µs 1285.3 µs   1362.7 µs  212.3 µs 1207.9 µs   1420.2 µs       2.7x Nisaba         Tie (~1.0x)
8. 50 Stroked Rounded Rects (1.5px Border)  111.0 µs 1031.7 µs   1142.7 µs  272.0 µs 1293.6 µs   1565.6 µs       2.4x Nisaba        1.37x Nisaba
9. 100 Alpha Circles / Disks (Fill)          93.4 µs  934.2 µs   1027.6 µs  364.4 µs 1365.0 µs   1729.4 µs       3.9x Nisaba        1.68x Nisaba
10. 25 Concentric Rings (2px Stroke)        173.3 µs 1039.6 µs   1212.9 µs  198.6 µs 1404.0 µs   1602.6 µs       1.1x Nisaba        1.32x Nisaba
11. 50 Two-Stop Linear Gradients             78.8 µs 1366.9 µs   1445.6 µs  985.2 µs 1509.6 µs   2494.8 µs      12.5x Nisaba        1.73x Nisaba
12. 50 Diagonal Gradients (45° Angle)       81.8 µs 1360.0 µs   1441.8 µs 1042.3 µs 1661.1 µs   2703.4 µs      12.7x Nisaba        1.88x Nisaba
13. 25 Radial Glow Gradients (300px)         70.2 µs 2527.0 µs   2597.2 µs  566.6 µs 2139.4 µs   2706.0 µs       8.1x Nisaba         Tie (~1.0x)
14. 20 Box Blur / Drop Shadows (15px)        41.1 µs 1174.0 µs   1215.0 µs  237.9 µs  951.5 µs   1189.4 µs       5.8x Nisaba         Tie (~1.0x)
15. 10 Concave 10-Point Stars (Fill)        169.0 µs 1349.8 µs   1518.8 µs 1080.0 µs 1357.4 µs   2437.4 µs       6.4x Nisaba        1.60x Nisaba
16. 10 Concave 10-Point Stars (3px Stroke)   77.5 µs  853.5 µs    931.0 µs 1952.5 µs 1502.8 µs   3455.3 µs      25.2x Nisaba        3.71x Nisaba
17. 1000-Point Waveform (Dense Stroke)      136.3 µs  830.3 µs    966.6 µs 1858.8 µs 1511.9 µs   3370.7 µs      13.6x Nisaba        3.49x Nisaba
18. 50 Rotated Cards (Affine Transforms)     58.0 µs 1094.5 µs   1152.5 µs  225.8 µs 1071.8 µs   1297.6 µs       3.9x Nisaba        1.13x Nisaba
19. 25 Scissor Clipped Viewports             40.6 µs  832.3 µs    872.9 µs  183.3 µs  755.9 µs    939.2 µs       4.5x Nisaba        1.08x Nisaba
20. 50 UI Text Labels (Inter-Regular)       430.6 µs 1298.9 µs   1729.5 µs  215.1 µs  982.6 µs   1197.6 µs         2.0x Skia          1.44x Skia
21. 25 Quadratic Bezier Curves (Fill+Stroke)  135.2 µs  987.9 µs   1123.1 µs 1027.6 µs 1418.6 µs   2446.3 µs       7.6x Nisaba        2.18x Nisaba
22. 20 Cubic Bezier Splines (4px Stroke)     67.3 µs  741.6 µs    808.9 µs 2680.7 µs 1772.5 µs   4453.2 µs      39.8x Nisaba        5.51x Nisaba
23. Complex Bezier Ribbon (24px Alpha)       44.9 µs  877.6 µs    922.5 µs  949.3 µs 1534.6 µs   2483.9 µs      21.2x Nisaba        2.69x Nisaba
24. Circular Arcs with arcTo (20 Corners)    96.5 µs  865.3 µs    961.9 µs 1646.8 µs 1565.4 µs   3212.2 µs      17.1x Nisaba        3.34x Nisaba
25. Directional Arcs with arc() (30 Arcs)   205.2 µs 1221.9 µs   1427.1 µs  273.2 µs  995.3 µs   1268.5 µs       1.3x Nisaba          1.13x Skia
26. Compound Path with Holes (15 Donuts)    177.5 µs 1165.5 µs   1343.1 µs 1372.9 µs 1523.4 µs   2896.3 µs       7.7x Nisaba        2.16x Nisaba
27. Trefoil Figure-8 Cubic Knot (8 Loops)    73.6 µs  868.3 µs    941.9 µs 1038.4 µs 1370.2 µs   2408.6 µs      14.1x Nisaba        2.56x Nisaba
28. Archimedean Spiral Polyline (500 Pts)   184.9 µs 1104.4 µs   1289.3 µs 1479.0 µs 1558.0 µs   3036.9 µs       8.0x Nisaba        2.36x Nisaba
29. Multi-Contour Disjoint Paths (25 Sub-Paths)   18.6 µs  660.7 µs    679.3 µs 1107.7 µs 1484.1 µs   2591.8 µs      59.4x Nisaba        3.82x Nisaba
30. Sharp Zigzag Mountain (50 Vertices)      55.7 µs  946.4 µs   1002.1 µs 2031.0 µs 1481.1 µs   3512.2 µs      36.4x Nisaba        3.50x Nisaba
31. Stroke Join: Miter (12px Stroke)         15.0 µs  585.2 µs    600.2 µs 1149.3 µs 1434.1 µs   2583.4 µs      76.4x Nisaba        4.30x Nisaba
32. Stroke Join: Bevel (12px Stroke)         30.8 µs  765.1 µs    796.0 µs 1177.8 µs 1415.0 µs   2592.7 µs      38.2x Nisaba        3.26x Nisaba
33. Stroke Join: Round (12px Stroke)         30.6 µs  605.7 µs    636.2 µs 1089.8 µs 1449.6 µs   2539.4 µs      35.6x Nisaba        3.99x Nisaba
34. Stroke Cap: Butt (30 Segments 12px)      24.3 µs  593.4 µs    617.7 µs  109.8 µs  852.3 µs    962.2 µs       4.5x Nisaba        1.56x Nisaba
35. Stroke Cap: Square (30 Segments 12px)    30.9 µs  668.9 µs    699.8 µs  120.3 µs  956.5 µs   1076.8 µs       3.9x Nisaba        1.54x Nisaba
36. Stroke Cap: Round (30 Segments 12px)    104.9 µs  871.3 µs    976.1 µs  171.2 µs 1024.0 µs   1195.2 µs       1.6x Nisaba        1.22x Nisaba
37. Miter Limit Clamping (10° Acute Spikes)   28.5 µs  666.5 µs    695.0 µs 1772.0 µs 1648.4 µs   3420.4 µs      62.2x Nisaba        4.92x Nisaba
38. Sub-Pixel Hairline Strokes (0.25px Lines)   66.7 µs  946.4 µs   1013.1 µs  211.8 µs 1079.4 µs   1291.3 µs       3.2x Nisaba        1.27x Nisaba
39. Ultra-Heavy Geometric Ribbon (40px Wide)   22.0 µs  553.9 µs    575.9 µs  440.0 µs 1180.9 µs   1620.9 µs      20.0x Nisaba        2.81x Nisaba
40. Stepped Variable Stroke Widths (1-20px)   57.5 µs  780.4 µs    837.9 µs 1263.9 µs 1375.1 µs   2638.9 µs      22.0x Nisaba        3.15x Nisaba
41. 50 Ellipses (rx=60, ry=25 Fill)          35.6 µs  816.0 µs    851.7 µs  106.9 µs 1013.9 µs   1120.8 µs       3.0x Nisaba        1.32x Nisaba
42. 50 Stroked Ellipses (2.5px Border)       52.2 µs  988.5 µs   1040.7 µs  192.5 µs 1233.8 µs   1426.3 µs       3.7x Nisaba        1.37x Nisaba
43. 50 Pill / Capsule Badges (r=h/2)         96.0 µs 1321.8 µs   1417.8 µs  100.7 µs  697.3 µs    798.1 µs               Tie          1.78x Skia
44. 50 Asymmetric Teardrop Rounded Rects     37.5 µs  888.8 µs    926.4 µs   97.8 µs  725.6 µs    823.4 µs       2.6x Nisaba          1.13x Skia
45. High-Density Disks Cloud (500 Disks)     89.6 µs  834.6 µs    924.2 µs 1075.7 µs 1504.5 µs   2580.2 µs      12.0x Nisaba        2.79x Nisaba
46. High-Density Stroked Rings (250 Rings)  171.5 µs 1193.1 µs   1364.5 µs  699.2 µs 1411.4 µs   2110.6 µs       4.1x Nisaba        1.55x Nisaba
47. 250 Filled Rectangles Batch             196.0 µs 1331.3 µs   1527.2 µs  726.4 µs 1375.8 µs   2102.2 µs       3.7x Nisaba        1.38x Nisaba
48. 150 Stroked Rectangles (2px Border)     162.1 µs 1184.6 µs   1346.7 µs  429.8 µs 1320.4 µs   1750.2 µs       2.7x Nisaba        1.30x Nisaba
49. 40 Crosshair Aim Reticles               383.3 µs 1211.0 µs   1594.4 µs 1715.5 µs 1687.0 µs   3402.5 µs       4.5x Nisaba        2.13x Nisaba
50. 25 Regular Octagons (Fill + Stroke)     372.5 µs 1589.3 µs   1961.8 µs 1756.1 µs 1673.0 µs   3429.1 µs       4.7x Nisaba        1.75x Nisaba
51. 30 Vertical Card Linear Gradients        61.8 µs 1343.5 µs   1405.3 µs  827.7 µs 1359.4 µs   2187.1 µs      13.4x Nisaba        1.56x Nisaba
52. 30 Horizontal Bar Progress Gradients     61.7 µs 1237.7 µs   1299.5 µs  806.4 µs 1512.5 µs   2318.9 µs      13.1x Nisaba        1.78x Nisaba
53. 15 Concentric Radial Spotlights          44.9 µs 1020.3 µs   1065.2 µs  461.7 µs 1401.3 µs   1863.0 µs      10.3x Nisaba        1.75x Nisaba
54. 25 Soft Ambient Card Shadows             54.2 µs 1055.3 µs   1109.6 µs  666.6 µs 1600.6 µs   2267.2 µs      12.3x Nisaba        2.04x Nisaba
55. 15 Neon Button Glows (Cyan Intense)      44.5 µs  948.3 µs    992.9 µs  448.1 µs 1531.8 µs   1980.0 µs      10.1x Nisaba        1.99x Nisaba
56. 16 Multi-Angle Gradient Fan Slices      131.3 µs 1180.0 µs   1311.3 µs  525.6 µs 1376.3 µs   1901.9 µs       4.0x Nisaba        1.45x Nisaba
57. Full-Screen Diagonal Horizon Gradient    37.8 µs 1126.5 µs   1164.3 µs  134.1 µs 1001.4 µs   1135.5 µs       3.5x Nisaba         Tie (~1.0x)
58. 20 Inset Well Shadows (Recessed)        234.5 µs 1396.1 µs   1630.6 µs  568.6 µs 1484.9 µs   2053.5 µs       2.4x Nisaba        1.26x Nisaba
59. 10 Pulsing Circular Radar Wave Glows     54.9 µs 1587.3 µs   1642.2 µs  318.8 µs 1421.9 µs   1740.7 µs       5.8x Nisaba        1.06x Nisaba
60. Waveform Polyline Stroked with Gradient  205.7 µs 1250.7 µs   1456.4 µs 1819.7 µs 1480.6 µs   3300.4 µs       8.8x Nisaba        2.27x Nisaba
61. Repeated Texture Tiling (64x64 Pattern)   29.8 µs  997.8 µs   1027.5 µs   75.1 µs  868.2 µs    943.3 µs       2.5x Nisaba          1.09x Skia
62. Scaled Image Quad (2x 512x512 Blit)      35.9 µs  978.3 µs   1014.3 µs   82.5 µs  829.6 µs    912.1 µs       2.3x Nisaba          1.11x Skia
63. 45° Rotated Pattern Fill in Circle      26.3 µs  885.2 µs    911.5 µs  141.0 µs  956.9 µs   1097.8 µs       5.4x Nisaba        1.20x Nisaba
64. Translucent Texture Overlay (50% Alpha)   41.6 µs  989.2 µs   1030.9 µs  124.8 µs 1060.1 µs   1184.9 µs       3.0x Nisaba        1.15x Nisaba
65. Multi-Avatar Grid (16 Avatars)           41.6 µs  923.2 µs    964.8 µs  129.8 µs  891.4 µs   1021.2 µs       3.1x Nisaba        1.06x Nisaba
66. Texture Pattern on Star Path             37.9 µs  699.9 µs    737.8 µs  363.8 µs 1166.7 µs   1530.6 µs       9.6x Nisaba        2.07x Nisaba
67. 20px Curved Path Stroked with Texture    28.8 µs  844.5 µs    873.3 µs  464.0 µs 1209.3 µs   1673.3 µs      16.1x Nisaba        1.92x Nisaba
68. Minified Texture Quad (0.25x Downscale)   32.6 µs  682.8 µs    715.4 µs  195.8 µs 1042.8 µs   1238.6 µs       6.0x Nisaba        1.73x Nisaba
69. Pure Translation Stack (50 Cards)        63.9 µs  905.0 µs    968.9 µs  213.9 µs 1043.5 µs   1257.4 µs       3.3x Nisaba        1.30x Nisaba
70. Pure Rotation Cluster (36 Spokes 10°)   28.7 µs  744.9 µs    773.6 µs   92.4 µs  728.4 µs    820.9 µs       3.2x Nisaba        1.06x Nisaba
71. Pure Scale Zoom Progression (20 Rects)   66.6 µs  917.9 µs    984.5 µs   54.6 µs  661.7 µs    716.3 µs         1.2x Skia          1.37x Skia
72. Shear / Skew Parallelograms (30 Quads)   21.6 µs  578.1 µs    599.7 µs   89.5 µs  701.8 µs    791.3 µs       4.1x Nisaba        1.32x Nisaba
73. Combined Affine Transforms (30 Shapes)   32.3 µs  738.9 µs    771.2 µs   94.8 µs  804.1 µs    898.9 µs       2.9x Nisaba        1.17x Nisaba
74. Deep Hierarchical State Stack (20 Levels)   39.1 µs  800.1 µs    839.2 µs  135.3 µs 1035.8 µs   1171.1 µs       3.5x Nisaba        1.40x Nisaba
75. Reset Transform Stress (50 Cycles)       56.1 µs  894.6 µs    950.7 µs  167.6 µs  864.9 µs   1032.5 µs       3.0x Nisaba        1.09x Nisaba
76. Planetary Orbit Hierarchy (10 Systems)   20.7 µs  578.8 µs    599.5 µs   72.0 µs  613.5 µs    685.4 µs       3.5x Nisaba        1.14x Nisaba
77. Multi-Scissor Grid (16 Viewports)        20.9 µs  616.3 µs    637.3 µs  105.2 µs  562.7 µs    667.8 µs       5.0x Nisaba         Tie (~1.0x)
78. Hierarchical Intersecting Scissors (4 Levels)   49.5 µs  918.8 µs    968.3 µs  155.8 µs 1224.0 µs   1379.8 µs       3.1x Nisaba        1.43x Nisaba
79. Scissored Stroked Waves (1000 Pts Clipped)  309.2 µs 1107.5 µs   1416.7 µs 1563.8 µs 1236.7 µs   2800.5 µs       5.1x Nisaba        1.98x Nisaba
80. Scissored Gradient Card (Clipped)        19.3 µs  787.0 µs    806.2 µs  164.2 µs  641.2 µs    805.4 µs       8.5x Nisaba         Tie (~1.0x)
81. Circular Overflow Clip (25 Boxes)        24.1 µs  674.3 µs    698.4 µs  133.2 µs  394.9 µs    528.1 µs       5.5x Nisaba          1.32x Skia
82. Scissor Invalidation / Reset (25 Cycles)   52.7 µs  887.6 µs    940.3 µs  147.8 µs  984.6 µs   1132.4 µs       2.8x Nisaba        1.20x Nisaba
83. Composite Op: Source-Over (30 Shapes)    39.3 µs  834.3 µs    873.6 µs   95.0 µs  719.9 µs    814.9 µs       2.4x Nisaba          1.07x Skia
84. Composite Op: Lighter / Plus (30 Particles)   28.3 µs  907.6 µs    935.9 µs  119.7 µs  838.2 µs    957.9 µs       4.2x Nisaba         Tie (~1.0x)
85. Composite Op: Source-In (Alpha Masking)   26.1 µs  692.2 µs    718.3 µs   97.6 µs  876.3 µs    973.9 µs       3.7x Nisaba        1.36x Nisaba
86. Composite Op: Source-Out (Cutout)        38.6 µs  963.5 µs   1002.1 µs  141.5 µs 1050.5 µs   1192.0 µs       3.7x Nisaba        1.19x Nisaba
87. Composite Op: Atop (Target Bounds)       37.6 µs  755.9 µs    793.5 µs  137.2 µs 1017.2 µs   1154.4 µs       3.6x Nisaba        1.45x Nisaba
88. Composite Op: Dest-Over (Under-Drawing)   37.1 µs  725.5 µs    762.6 µs  213.4 µs 1244.3 µs   1457.7 µs       5.8x Nisaba        1.91x Nisaba
89. Composite Op: Dest-Out (Eraser Mask)     28.7 µs  659.1 µs    687.9 µs  121.9 µs  852.7 µs    974.6 µs       4.2x Nisaba        1.42x Nisaba
90. Composite Op: Dest-Atop (Inverse)        33.8 µs  754.3 µs    788.1 µs  129.6 µs  990.7 µs   1120.2 µs       3.8x Nisaba        1.42x Nisaba
91. Composite Op: Xor (Exclusive Blend)      19.8 µs  575.8 µs    595.7 µs   54.5 µs  581.4 µs    635.9 µs       2.7x Nisaba        1.07x Nisaba
92. Composite Op: Copy (Direct Overwrite)    21.6 µs  672.9 µs    694.5 µs  153.3 µs 1005.2 µs   1158.5 µs       7.1x Nisaba        1.67x Nisaba
93. UI Dashboard Gauge (Arc Track + Needle)   56.7 µs  905.4 µs    962.1 µs  359.9 µs 1172.0 µs   1532.0 µs       6.3x Nisaba        1.59x Nisaba
94. Audio Spectrum Visualizer (64 Bars)      70.3 µs 1081.7 µs   1152.0 µs  985.3 µs 1533.6 µs   2519.0 µs      14.0x Nisaba        2.19x Nisaba
95. Circular Progress Rings (8 Meters)      122.4 µs  994.0 µs   1116.4 µs  873.6 µs 1304.7 µs   2178.3 µs       7.1x Nisaba        1.95x Nisaba
96. Modern Card Stack (5 Elevated Cards)     50.8 µs 1247.9 µs   1298.7 µs  276.8 µs 1482.0 µs   1758.7 µs       5.4x Nisaba        1.35x Nisaba
97. CAD Cross-Hatch Pattern (80 Angled Lines)   61.0 µs  842.9 µs    904.0 µs  123.2 µs  930.1 µs   1053.3 µs       2.0x Nisaba        1.17x Nisaba
98. Floating Action Button (FAB 10 Buttons)  116.3 µs 1194.3 µs   1310.6 µs  431.1 µs 1303.7 µs   1734.9 µs       3.7x Nisaba        1.32x Nisaba
99. Anti-Aliasing Geometry Grid (50 Diamonds)   91.4 µs  908.9 µs   1000.3 µs  334.5 µs 1151.9 µs   1486.4 µs       3.7x Nisaba        1.49x Nisaba
100. Master Vector Stress: Mixed Mega-Scene   52.8 µs 1104.3 µs   1157.1 µs  226.5 µs 1196.4 µs   1422.9 µs       4.3x Nisaba        1.23x Nisaba
---------------------------------------------------------------------------------------------------------------------------------------------------
TOTAL FRAME OVERHEAD (ALL SUITES)          8080.0 µs96647.9 µs 104727.9 µs55776.8 µs118425.2 µs 174201.9 µs      6.90x Nisaba        1.66x Nisaba
===================================================================================================================================================
SUMMARY: Total Frame Performance: Nisaba won in 84 suites | Skia won in 9 suites | Ties: 7
         Overall Frame Throughput (End-to-End): 1.66x Nisaba
         Overall CPU Command Submission:       6.90x Nisaba
===================================================================================================================================================
```
</details>

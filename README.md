<div align="center">

![Nisaba — Full-Stack 2D Graphics Engine](assets/Nisaba-background-banner.jpeg)

# Nisaba — Full-Stack 2D Graphics Engine
### Sovereign C++20 2D Vector Graphics, Typography, UI Layout & Hardware GPU Engine

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Build System](https://img.shields.io/badge/Build-Meson%20%2B%20Ninja-green.svg)](https://mesonbuild.com)
[![Dependencies](https://img.shields.io/badge/Dependencies-Zero%20(Sovereign)-brightgreen.svg)](#technological-sovereignty)
[![Platforms](https://img.shields.io/badge/Platforms-Linux%20%7C%20Windows%20%7C%20Android%20%7C%20WASM%20%7C%20DRM%2FKMS-orange.svg)](#cross-platform)

</div>

**Nisaba** (`nisaba`) is an embedded-first, zero-dependency, ultra-lightweight 2D graphics rendering and layout engine written from scratch in **Modern C++ (C++20)**, featuring multi-backend sovereign rendering (CPU software rasterizer, native **Vulkan** GPU pipeline, and **OpenGL ES 3.2** GPU pipeline), exact geometric path boolean operations (`nisaba::path_ops`), stroke-to-fill path outlining (`nisaba::stroker`), multi-format embedded framebuffers (RGB565, Alpha8, Gray8, BGRA8888, RGBA8888), physical linear-space sRGB color blending, a deterministic 7-stage Flexible Box and UI layout engine (`nisaba-layout`), complete sovereign image codec subsystems, a sovereign Lottie vector animation engine (`nisaba::lottie`), an optional sovereign native OS platform and windowing subsystem (`backend_os`), a sovereign Markdown document layout and multi-page rendering engine (`nisaba::markdown`), and a full-specification sovereign PDF document engine (`nisaba::pdf`). Purpose-built as the native rendering foundation for the **`vaxp`** cross-platform organization.

Nisaba delivers high-end vector graphics, geometric path boolean algebra (Union, Difference, Intersect, Xor), high-precision 256-level subpixel anti-aliasing, hardware 4x MSAA, analytical SDF Gaussian shadows, screen-space dithering, two-point conical & compose multi-shaders, 4x5 affine color matrix filtering, Porter-Duff compositing, full W3C SVG vector graphics with gradients and masks, real-time Bodymovin/Lottie vector animations, a sovereign multilingual typography subsystem with native TrueType and PostScript CFF/OpenType layout, zero-dependency PNG/JPEG/QOI image codecs, a sovereign CommonMark/GFM Markdown engine, a full-specification ISO 32000 PDF document engine, an adaptive flexbox and stack layout solver, and a sovereign cross-platform OS windowing layer with **absolute technological sovereignty** (zero external third-party dependencies, 100% free of GLFW/SDL).


---

## Visual Showcases

### Sovereign PNG & JPEG Image Codecs Showcase (VAXP Logo Integration)
![Nisaba Image Codecs Showcase](showcase/nisaba_image_codec_showcase.png)

### Vector & Shader Rendering Showcase
![Nisaba Vector Showcase](showcase/embedded_hmi_showcase.png)

### Multilingual Typography & Text Layout Showcase
![Nisaba Text Showcase](showcase/nisaba_text_showcase.png)

### Modern Glassmorphism & Visual Effects Showcase
![Nisaba Effects Showcase](showcase/nisaba_effects_showcase.png)

### Sovereign SVG Icons & Vector Graphics Showcase
![Nisaba SVG Showcase](showcase/nisaba_svg_showcase.png)

### High-Performance SVG Caching & SIMD Pre-Baking Subsystem Showcase (`SvgCache` & `BakedSvg`)
![Nisaba SVG Cache Showcase](showcase/showcase_svg_cache.png)

### 3D Perspective Transformations & Spatial Warp Showcase
![Nisaba 3D Perspective Showcase](showcase/nisaba_perspective_showcase.png)

### 2D Vertex Mesh & Freeform Gradient Meshes Showcase
![Nisaba 2D Mesh Showcase](showcase/nisaba_mesh_showcase.png)

### Hardware-Accelerated GPU Rendering Showcase (Vulkan & OpenGL ES 3.2 with 4x MSAA)
![Nisaba GPU Showcase](showcase/example_calculator.png)

### Sovereign Lottie Vector Animation Engine Showcase (`nisaba::lottie` & Hardware GPU Acceleration)
![Nisaba Lottie GPU Showcase](showcase/nisaba_lottie_gpu_showcase.png)

### Sovereign Adaptive UI Layout Engine Showcase (`nisaba-layout`)
Nisaba's native C++20 Flexible Box and UI Layout Engine powering reactive desktop, embedded, and dashboard layouts with deterministic subpixel layout resolution.

| Responsive Multi-Row Complex Dashboard | Stack & Absolute Positioning (Z-Layers, Badges & Insets) |
| :---: | :---: |
| ![Complex Dashboard Showcase](showcase/layout_complex_dashboard.png) | ![Stack & Absolute Positioning Showcase](showcase/layout_absolute_stack.png) |

| Flex Direction (Row, Column, RowReverse, ColumnReverse) | Justify Content (Distribution Along Main Axis) |
| :---: | :---: |
| ![Flex Direction](showcase/layout_flex_direction.png) | ![Justify Content](showcase/layout_justify_content.png) |

| Cross-Axis Alignment & Stretch (`align-items` / `align-self`) | Flex Wrap & Dimensional Gaps (`gap`, `row-gap`, `column-gap`) |
| :---: | :---: |
| ![Align Items](showcase/layout_align_items.png) | ![Flex Wrap & Gap](showcase/layout_flex_wrap_gap.png) |

| Space Distribution (`flex-grow`, `flex-shrink`, `flex-basis`) | Full CSS Box Model (`margin: auto`, padding, border, box-sizing) |
| :---: | :---: |
| ![Flex Grow & Shrink](showcase/layout_flex_grow_shrink.png) | ![Box Model & Margin Auto](showcase/layout_box_model.png) |

### Sovereign Markdown Rendering & Document Layout Showcase (`nisaba::markdown`)
![Nisaba Markdown Showcase](showcase/nisaba_markdown_showcase.png)

### Sovereign Full-Specification PDF Studio & GPU Viewer Showcase (`nisaba::pdf` & `nisaba::gpu`)
![Nisaba PDF Viewer Showcase](showcase/nisaba_pdf_viewer_showcase.png)

---

## Key Architectural Pillars

### 1. Absolute Technological Sovereignty (Zero External Dependencies)
- Implemented strictly using standard ISO C++20 and standard compiler toolchains.
- Completely free of external graphics, rasterization, or mathematical libraries.
- Features native, self-contained BMP and PPM decoders and encoders directly inside the core.

### 2. True Zero-Copy Framebuffer Rendering with Stride
- Supports direct rendering into arbitrary raw memory pointers (`uint8_t*`) with hardware row stride / pitch alignment.
- Ready for immediate integration with native display scanout buffers:
  - Linux DRM/KMS Dumb Buffers (`DRM_FORMAT_XRGB8888` / `ARGB8888`)
  - Linux Framebuffer (`/dev/fb0`)
  - X11/Wayland Shared Memory (`wl_shm`)
  - Windows DIB Sections
  - Android `ANativeWindow_Buffer`
  - WebAssembly `ImageData`

### 3. Hardware Acceleration via SIMD
- High-performance vector paths processing 4 pixels concurrently utilizing SSE2 / AVX2 (x86_64) and ARM NEON (ARM/AArch64).
- 100% portable scalar generic C++20 fallback for microcontrollers and platforms lacking SIMD extensions.

### 4. Arbitrary Vector Clip Paths
- Clip any drawing operation within arbitrary, complex vector paths (`Canvas::clip_path`, `Canvas::clip_rect`).
- Complete hierarchical state stack to preserve and restore clip regions and transformation matrices (`Canvas::save()`, `Canvas::restore()`).

### 5. Sovereign Multilingual Typography & OpenType Subsystem (`nisaba::text`)
- **Native Binary TTF/OTF Font Parser**: Directly parses TrueType and OpenType vector outlines and emits them as native `nisaba::Path` curves without external font libraries.
- **PostScript CFF Type 2 Charstrings**: Native bytecode parsing of `.otf` font outlines supporting stack evaluation, local and global subroutines (`callsubr`, `callgsubr`), stem hints, transient storage, and exact cubic Bézier curve extraction (`rrcurveto`, `rcurveline`, `hhcurveto`, `vvcurveto`) without conversion approximations.
- **OpenType Advanced Typography Engine (`GSUB` & `GPOS`)**:
  - **`GSUB` (Glyph Substitution)**: Supports Format 1 & 2 single substitutions, standard typographic ligatures (`fi`, `fl`, `ffi`, `ffl`), and contextual chaining substitutions.
  - **`GPOS` (Glyph Positioning)**: Direct pair kerning evaluation across Coverage-based and Class-based pair adjustment tables.
- **Bidirectional Text Layout (BiDi)**: Implements Unicode Bidirectional Algorithm rules (UAX #9) and 4-form contextual Arabic shaping (Isolated, Initial, Medial, Final).
- **East Asian (CJK) & Global Scripts Support**: Specialized Kinsoku Shori line-breaking rules and spaceless boundary classification for Japanese, Chinese, and Korean alongside Cyrillic and Latin.
- **Multi-Line Text Buffer**: Dynamic paragraph layout with configurable wrapping (`Wrap::Word`, `Wrap::Glyph`) and alignment (`Align::Left`, `Right`, `Center`, `Justify`).
- **Subpixel Anti-Aliased Glyph Cache**: High-precision 4-bin subpixel horizontal positioning cache utilizing Nisaba's native scanline rasterizer.
- **Interactive Cursor & Selection**: Text editing primitives including caret positioning, selection ranges, and spatial hit-testing (`Cursor`, `Selection`, `HitResult`).
- **Built-in Monospace 8x16 Font**: Embedded in-memory bitmap font for zero-dependency boot consoles, debug logging, and realtime FPS telemetry on headless or embedded screens.

### 6. Sovereign Visual Effects, Shadows & Color Matrix Filters (`nisaba::effects`)
- **Fast Separable Gaussian Blur**: 3-pass sliding accumulator box blur operating in $O(1)$ per-pixel time regardless of radius size on both 32-bit RGBA surfaces and 8-bit alpha masks.
- **Elevation Drop Shadows & Glowing Halos**: Padded subpixel vector mask convolutions with customizable offsets, blur sigma, spread, and ambient neon glow halos (`DropShadow`).
- **Modern Glassmorphism Panels**: Single-line frosted glass card rendering with real backdrop blur refraction, translucent frost overlay tint, and specular edge reflection borders (`Canvas::draw_glass_panel`).
- **4x5 Affine Color Transformation Matrix (`ColorMatrix`)**:
  - Full $4 \times 5$ affine color transformation matrix $[M_{4 \times 4} \mid T_{4 \times 1}]$ evaluating:
    $$\begin{pmatrix} R' \\ G' \\ B' \\ A' \end{pmatrix} = M \cdot \begin{pmatrix} R \\ G \\ B \\ A \end{pmatrix} + T$$
  - Provides instant color space transforms: saturation scaling, hue rotation, sepia, luminance projection, and brightness/contrast adjustments in a single pass.
  - Applied directly in-place across `Pixmap` surfaces or dynamically during rasterization via `Paint::set_color_filter()`.
- **In-Place Color Filters**: Instant adjustments for brightness, contrast, grayscale luminance, sepia tones, and color inversion.

### 7. Sovereign W3C SVG Vector Graphics, Icon Subsystem & High-Performance Caching (`nisaba::svg`)
- **Complete W3C SVG Path Lexer**: Full support for standard SVG path commands: `M/m`, `L/l`, `H/h`, `V/v`, `C/c`, `S/s`, `Q/q`, `T/t`, `A/a`, `Z/z`.
- **Analytical Elliptical Arc Decomposition**: Converts SVG `A/a` arc parametrization into exact cubic Bézier curves with $C^1$ continuity.
- **Zero-Dependency SVG/XML Document Parser**: Parses `<path>`, `<rect>`, `<circle>`, `<ellipse>`, `<line>`, `<polyline>`, and `<polygon>` elements, handling fills, strokes, line caps, line joins, opacities, transformations, and `viewBox` scaling.
- **Complete Paint Servers & Gradients**: Full support for `<linearGradient>` and `<radialGradient>` with arbitrary color stops, `gradientUnits` (`userSpaceOnUse`, `objectBoundingBox`), `gradientTransform`, and spread modes (`pad`, `reflect`, `repeat`).
- **Resource Re-use & Definitions**: Resolves `<defs>` dictionaries and dynamic `<use>` instancing with cascading transforms and inherited style attributes.
- **Vector Clipping & Masking**: Evaluates `<clipPath>` arbitrary vector clipping masks across child groups and supports luminosity and alpha `<mask>` elements.
- **Embedded Raster Images**: Decodes and renders `<image>` elements from filesystem paths and inline RFC 2397 base64 data URIs (`data:image/png;base64,...`).
- **Affine Skew Transformations**: Exact support for `skewX(angle)` and `skewY(angle)` within SVG transformation chains.
- **Direct Canvas Integration**: Render arbitrary SVG icons and vector illustrations into any target rectangle preserving aspect ratio (`Canvas::draw_svg`).
- **High-Performance SVG Caching (`SvgCache`)**:
  - Eliminates repeated runtime Bézier parsing, curve tessellation, and scanline rasterization by caching pre-rasterized vector assets in an intelligent LRU (Least Recently Used) cache.
  - Generates robust 64-bit FNV-1a composite keys combining source path/XML hash, target dimensions, and dynamic tint values.
  - Achieves a **39.5x throughput boost** over raw uncached vector rasterization (latency reduced from **138.0 μs down to 3.49 μs** per draw call).
- **Pre-Baked Surfaces (`BakedSvg`)**:
  - Pre-bakes SVG documents once to high-quality raster surfaces (`Pixmap`), turning subsequent draws into instant $O(1)$ SIMD row blits via `simd::blend_source_over_span`.
  - Direct integration into `Canvas::draw_baked_svg(baked_svg, x, y, opacity)`.
- **Dynamic Runtime Tint Modulation (Tinting)**:
  - Enables dynamic color modulation for monochrome SVG icons (e.g. UI status icons, battery, settings, search) at arbitrary resolutions and colors without modifying the source XML or duplicating asset files.
- **GPU Pattern Blitting**:
  - Seamless drawing onto hardware-accelerated `GpuCanvas` textures with pattern rendering, maintaining blistering 120+ FPS frame rates even under dense multi-icon loads.

### 8. Sovereign 3D Perspective & Spatial Warping Engine (`nisaba::math` & `nisaba::raster`)
- **4x4 Projective Transformation Matrix (`Transform4x4`)**: Full 3D affine and projective mathematics including 3D Euler rotations (pitch, yaw, roll), scale, translation, and perspective camera projection with focal distance $d$ ($W = 1 - z/d$).
- **Paul Heckbert 8-DOF Quad Homography Solver**: Direct analytical computation of the 3x3 projective homography mapping any source rectangle to an arbitrary non-planar quadrilateral without diagonal distortion or affine tearing.
- **Perspective-Correct Scanline Rasterizer**: High-performance linear scanline interpolation in homogeneous space $(u/w, v/w, 1/w)$ with per-pixel reciprocal division, bilinear texture filtering, and subpixel edge antialiasing.
- **Seamless Canvas Integration**: Draw warped quads and 3D rotated surfaces with single-line API calls (`Canvas::draw_pixmap_perspective`, `Canvas::draw_pixmap_3d`).

### 9. Sovereign 2D Vertex Mesh & Freeform Gradient Meshes (`nisaba::mesh` & `nisaba::raster`)
- **Flexible Vertex Modes (`VertexMode`)**: Support for `Triangles`, `TriangleStrip`, and `TriangleFan` with optional 32-bit indexed buffers and automated grid tessellation (`Vertices::create_grid`).
- **Differential Barycentric Triangle Rasterizer**: Analytical scanline edge walking with constant horizontal derivatives $(dR, dG, dB, dA)$ and $(dU, dV)$, achieving fast $O(1)$ per-pixel attribute interpolation.
- **Per-Vertex Gouraud Shading**: Rich, multi-point color interpolation for low-poly artwork, vector primitives, and specular lighting.
- **Freeform 2D Mesh Deformation & Bilinear UV Mapping**: Wrap and deform source textures across organic waving, stretching, or wrinkled 2D meshes without shearing or pixelation (`Canvas::draw_textured_vertices`).
- **Photorealistic Bicubic Coons Patches (`CoonsPatch` & `GradientMesh`)**: Analytical blending of 4 cubic Bézier boundary curves and 4 corner colors, matching vector illustration standards (`Canvas::draw_gradient_mesh`).

### 10. Sovereign Hardware-Accelerated GPU Pipeline (`nisaba::gpu`)
- **Multi-Backend Sovereign Architecture (`GpuBackendType`)**:
  - Unified hardware abstraction layer supporting both native **Vulkan** (`GpuBackendType::Vulkan`) and **OpenGL ES 3.2 / Headless EGL** (`GpuBackendType::OpenGL`).
  - Automatic intelligent runtime arbitration (`GpuBackendType::Auto`) prioritizing modern Vulkan compute/graphics with graceful fallback to OpenGL ES.
  - Environment variable dynamic override (`NISABA_GPU_BACKEND=vulkan` or `NISABA_GPU_BACKEND=opengl`) for testing and hardware diagnostics without recompilation.
- **Native Vulkan 1.0+ GPU Pipeline (`VulkanDevice`, `VulkanSurface`, `VulkanRenderer`)**:
  - Direct hardware binding to the native Vulkan loader with zero third-party framework dependencies.
  - Self-contained GPU memory management (`VkDeviceMemory`), staging host-visible buffers for zero-stall host-to-device transfers, and linear blit resolves.
  - Custom compiled SPIR-V bytecode shaders embedded directly into engine binaries (`vulkan_shaders.hpp`).
  - Dedicated multi-attachment `VkRenderPass` architecture: hardware 4x MSAA color buffers, combined depth/stencil attachments (`VK_FORMAT_D24_UNORM_S8_UINT` / `VK_FORMAT_D32_SFLOAT_S8_UINT`), and automated single-sample resolve transitions.
  - Dynamic viewport and scissor states (`VkPipelineDynamicStateCreateInfo`) matching Nisaba's vector clipping hierarchy.
- **Headless EGL & OpenGL ES 3.2 Driver Interface (`GLDevice`, `GLSurface`, `GL3Renderer`)**:
  - Direct, zero-dependency binding to GPU hardware drivers via headless EGL, enabling high-performance hardware rendering and headless offscreen compute across Linux, Android, DRM/KMS, and desktop displays.
- **4x Hardware Multi-Sample Anti-Aliasing (MSAA)**: Native multisampled color and depth/stencil renderbuffers/attachments with automated blit resolve to single-sample surfaces during frame readback, guaranteeing smooth antialiased vector geometry.
- **Triangular Screen-Space Dithering (Anti-Banding)**: High-precision temporal-stable triangular probability density function (TPDF) dither embedded directly into linear, radial, and angular sweep gradient shaders (both GLSL and SPIR-V), eradicating color banding artifacts.
- **Analytical Signed Distance Field (SDF) Gaussian Shadows & Neon Halos**: Closed-form mathematical evaluation of the 2D Gaussian convolution integral via the error function ($\text{erf}$), generating soft ambient drop shadows and vibrant glowing halos in a single GPU draw call.
- **Real-Time Glassmorphism & Translucent Panels (`GpuCanvas::draw_glass_panel`)**: Modern frosted glass UI surfaces incorporating analytical elevation drop shadows, tinted translucent bodies, and specular edge reflection borders.
- **Direct W3C SVG Vector Graphics on GPU (`GpuCanvas::draw_svg`)**: Native execution and GPU rasterization of arbitrary W3C SVG paths and icons with hardware MSAA and zero external dependencies.
- **Hardware GPU Text Rendering & Dynamic Glyph Atlas (`GpuCanvas::draw_text`, `GpuCanvas::draw_text_buffer`, `GpuGlyphAtlas`)**:
  - Full-specification hardware typography on GPU via dynamic Skyline 2D bin-packed alpha atlases ($1024 \times 1024$ dynamically expandable on-demand up to $2048 \times 2048$).
  - 1-pixel boundary padding eliminating texture bleeding, 4-bin subpixel horizontal positioning, and minimal dirty-region partial GPU texture flushes.
  - Unified single-batch draw calls (`GpuCanvas::draw_text_buffer`) batching full shaped paragraphs and multi-line text layouts into a single GPU draw call with per-vertex color tinting (`Vertex::color`) for multicolored text spans and syntax highlighting.
- **Ultra-Low CPU Command Overhead & Sustained High Frame Rates**:
  - Highly optimized GPU command recording and state caching delivering near-zero CPU submission overhead and sustained 120+ FPS throughput across complex vector scenes.
- **Continuous Ribbon Stroking & High-Density Tessellation**: Adaptive curved geometry tessellation with averaged vertex miter normals for continuous, break-free ribbons and strokes at arbitrary angles.
- **Seamless Dual CPU/GPU Coexistence**: Complete architectural isolation ensuring 0 regressions on CPU rasterization while sharing mathematical types (`Rect`, `Point`, `Color`, `Path`, `Transform`) and textures (`Pixmap`).

### 11. Sovereign PNG & JPEG Image Codecs Subsystem (`nisaba::image`)
- **Zero-Dependency Deflate Compression Subsystem (`nisaba::image::deflate`)**:
  - Full RFC 1950 (Zlib wrapper) and RFC 1951 (Deflate stream) compliance.
  - Compile-time `constexpr` 256-entry CRC-32 (ISO 3309) and loop-unrolled Adler-32 accumulators.
  - Direct $O(1)$ primary Huffman table (9-bit lookup) for immediate single-cycle symbol resolution.
  - Sovereign LZ77 compressor with a 32 KB circular window (128 KB total memory footprint) that stays resident within CPU L2 cache, eliminating dynamic heap allocations.
  - Multi-level compression: Fast UI Compositor Mode (Level 1, ~50 ms) for real-time window compositing and screen captures, and Production Storage Mode (Level 6) for maximum compression.
- **Sovereign PNG Subsystem (`nisaba::image::png`)**:
  - Full standard chunk parser and generator: `IHDR`, `PLTE`, `tRNS`, `IDAT`, `IEND` with multi-chunk streaming assembly.
  - Comprehensive color models: RGBA8, RGB8, Grayscale (1, 2, 4, 8, 16-bit), Grayscale+Alpha, and Indexed / 8-bit Colormap (Palette) with full alpha channel transparency.
  - All 5 scanline unfilters: `None`, `Sub`, `Up`, `Average`, and `Paeth` with overflow-safe arithmetic.
  - Full Adam7 7-pass interlaced image decoding.
  - Adaptive scanline filter selection using Sum of Absolute Differences (SAD).
  - Direct row-write optimization for RGBA8 and RGB8, bypassing per-pixel function call and branching overhead.
- **Sovereign JPEG Subsystem (`nisaba::image::jpeg`)**:
  - Complete JFIF parser (`SOI`, `SOF0`, `DQT`, `DHT`, `SOS`, `DRI`, `RST0-7`, `APP0`, `EOI`) with automatic byte-stuffing handling (`0xFF 0x00`).
  - Chrominance subsampling support for 4:4:4, 4:2:2, 4:2:0, and Grayscale.
  - Exact Separable 2D Forward and Inverse DCT ($D = T \cdot in \cdot T^T$ and $X = T^T \cdot D \cdot T$) achieving studio master reference fidelity (**PSNR = 44.95 dB**).
  - High-speed 9-bit direct Huffman lookup combined with ISO 10918-1 bounded canonical prefix search.
  - 16-bit fixed-point ITU-R BT.601 integer color transformation ($Y'CbCr \leftrightarrow RGB$).
  - Reciprocal quantization multiplier tables eliminating run-time integer divisions.
  - Single-instruction bit-width category evaluation via C++20 `std::bit_width` (compiles to hardware `BSR`/`LZCNT`).
  - Baseline sequential encoder with configurable quality ($1 \dots 100$).
- **OS-Grade Security & Fuzz-Proofing**:
  - Dimension limits ($W, H \le 32768$) preventing integer overflow exploits during buffer allocations.
  - Component count ($1 \dots 4$) and sampling factor ($H, V \le 4$) validation preventing infinite loops.
  - Zero-division protection against empty or malformed quantization tables.
  - Safe progressive JPEG (`SOF2`) handshake returning clean error codes without process crashes.
- **Sovereign QOI Subsystem (`nisaba::image::qoi`)**:
  - Full Quite OK Image Format specification implementation in pure C++20.
  - All 6 core operations supported: `QOI_OP_RUN`, `QOI_OP_INDEX` (64-entry running color cache), `QOI_OP_DIFF`, `QOI_OP_LUMA`, `QOI_OP_RGB`, `QOI_OP_RGBA`.
  - Ultra-fast lossless performance: encodes in **~7.9 ms** and decodes in **~7.4 ms** on 1400x900 HD surfaces (**12.8x faster than PNG**).
  - Direct integration into `Pixmap::load_qoi()`, `pixmap.save_qoi()`, and auto-detection via `qoif` magic bytes.
- **Unified I/O & `Pixmap` Auto-Sniffing (`nisaba::image::image_io`)**:
  - `Pixmap::load_file(path)`: automatic magic-byte sniffing (PNG, JPEG, QOI, BMP, PPM).
  - `Pixmap::load_from_memory(data)`: in-memory decoding.
  - `pixmap.save_png(path, level)`, `pixmap.save_jpeg(path, quality)`, and `pixmap.save_qoi(path)`.
  - `pixmap.save_image(path)`: auto-format detection by file extension (.png, .jpg, .jpeg, .qoi, .bmp, .ppm).

### 12. Sovereign Adaptive Flexbox & UI Layout Engine (`nisaba-layout` / `nisaba::layout`)
- **Deterministic 7-Stage Layout Pipeline (`LayoutPipeline`)**:
  - **Stage 1 (Geometric Context & Direction Resolution)**: Resolves bidirectional writing mode (`Direction::LTR` / `RTL`), available container content bounds, and physical padding/border insets.
  - **Stage 2 (Intrinsic Item Measurement & Hypothetical Main Sizes)**: Calculates hypothetical item lengths across main axis with support for leaf measurement callbacks (`MeasureCallback`), fixed/percent units, and aspect ratio preservation.
  - **Stage 3 (Line Segmentation)**: Partitions items into flex lines for multi-line configurations (`Wrap::Wrap`, `Wrap::WrapReverse`) incorporating discrete cross and main axis gaps.
  - **Stage 4 (Flexible Free-Space Distribution)**: Exact W3C-compliant resolution distributing positive free space via `flex-grow` and resolving negative overflow via `flex-shrink` * flex-basis product weighting, constrained by min/max dimensions.
  - **Stage 5 (Cross-Axis Sizing & Stretch)**: Resolves line heights/widths, stretches unconstrained children (`Align::Stretch`), and computes baseline offsets (`Align::Baseline`).
  - **Stage 6 (Alignment & Positioning)**: Distributes items along main axis (`Justify::FlexStart`, `Center`, `FlexEnd`, `SpaceBetween`, `SpaceAround`, `SpaceEvenly`), applies cross-axis alignment (`align-items`, `align-self`, `align-content`), and executes automatic margin centering (`margin: auto`).
  - **Stage 7 (Absolute Positioning & Z-Stacking)**: Positions out-of-flow children (`PositionType::Absolute`) based on 4-corner insets (`top`, `left`, `right`, `bottom`), percentages, explicit dimensions, and layered Z-order stacks without affecting sibling flow.
- **Cache-Friendly Flat Geometry Representation**:
  - Memory-aligned flat C++20 structures (`StyleLength`, `StyleSizeLength`, 8 bytes each) eliminating pointer indirection and heap churn.
  - Bidirectional parent-child tree linking (`Node`) with safe, zero-leak stack and heap lifetime management.
- **Complete W3C CSS Box Model Compliance**:
  - Full support for `BoxSizing::BorderBox` and `BoxSizing::ContentBox`.
  - Directional margins, borders, paddings, and discrete gap control (`Gutter::Row`, `Gutter::Column`, `Gutter::All`).
  - Dimensional constraints: min/max dimensions, auto dimensions, percentage scaling, and explicit aspect ratios.
- **Subpixel Quantization & Integral Alignment (`quantizeLayoutMetrics`)**:
  - Quantizes physical positions and dimensions to device pixels via rounded offsets while tracking fractional subpixels, preventing visual blurring or raster seam artifacts on high-DPI displays.
- **Direct Canvas Integration**:
  - Resolves layouts and maps computed bounds (`layout.position()`, `layout.dimension()`) directly into `nisaba::Canvas` draw calls in a single deterministic pass.

### 13. Sovereign Native Platform & Windowing Subsystem (`nisaba::backend_os`)
- **Strictly Optional & Non-Intrusive Architecture**:
  - Located in the dedicated top-level directory `backend_os/` and compiled only when explicitly enabled via `-Denable_backend_os=true` (disabled by default).
  - Preserves Nisaba's core identity as a lightweight, independent 2D graphics and layout engine—**not** an opinionated, monolithic application framework. It is leveraged strictly when native desktop or embedded display windows and hardware event loops are required.
- **Pure Native Cross-Platform OS Backends (100% Zero GLFW / SDL Dependencies)**:
  - **Linux Wayland**: Pure XDG Shell toplevels (`xdg_toplevel`), desktop layer surfaces (`zwlr_layer_shell_v1`), popups (`xdg_popup`), and decoration negotiation (`zxdg_decoration_manager_v1`).
  - **Linux X11**: Pure Xlib/XCB + EGL/GLX with Client-Side Decorations (CSD) and window manager state synchronization (`_NET_WM_*`).
  - **Linux DRM/KMS**: Bare-metal direct scanout via `libdrm`, `gbm`, and `libinput` for automotive infotainment, smart displays, and embedded robotics without an X11 or Wayland server.
  - **Windows Win32**: Win32 API with dark mode titlebars, high-DPI awareness, DWM blur-behind, and seamless message loop.
  - **Android**: Direct `ANativeActivity` and `ANativeWindow` glue with touch event translation and lifecycle management.
  - **WebAssembly**: HTML5 Canvas and Emscripten event pipeline with WebGL context binding.
- **Unified Application Lifecycle & Event Loop (`nisaba::backend_os::App`)**:
  - Encapsulates OS event pumps, dispatch loops, and multi-window orchestration behind an intuitive, RAII-governed `App` interface.
  - Real-time frame statistics and profiling (`FrameStats`): smoothed instantaneous FPS, exact frame duration in milliseconds, and 95th-percentile (P95) latency tracking over the last 120 frames.
  - Dual execution modes: continuous automated render loop (`App::run()`) or discrete manual frame stepping (`App::stepFrame()`) for embedded controllers.
  - Automatic VSync synchronization and configurable target frame rates (`target_fps`).
- **Native Windowing & Glassmorphism (`backend_os::Window`)**:
  - Full support for 32-bit ARGB compositing with true alpha transparency (`WindowConfig::transparent`).
  - Native compositor blur-behind hints (`WindowConfig::blur` and `Window::setBlurBehind`).
  - Client-Side Decorations (CSD) with native interactive dragging (`Window::beginMove`), edge resizing (`Window::beginResize`), maximize, minimize, and fullscreen toggles.
- **Sovereign Native Popups & Overlays (`backend_os::NativePopup` & `App::addPopup`)**:
  - Lightweight secondary surfaces for context menus, dropdowns, tooltips, and floating palettes.
  - Built-in automatic dismissal on outside clicks, anchor coordinate tracking, and desktop layer shell placement.
- **Zero-Allocation Type-Safe Signals (`backend_os::Signal`)**:
  - Decoupled, header-only signal-slot dispatching for mouse movement, clicks, scrolls, keyboard inputs, window resizes, and close requests without heavy callback dependencies.
- **Comprehensive Desktop Subsystems**:
  - **Clipboard**: Full UTF-8 text and multi-format MIME data exchange (`getClipboardText`, `setClipboardText`, `getClipboardDataForMime`).
  - **Drag and Drop**: Native OS drag initiation and payload handling (`startDrag`, `cancelDrag`).
  - **Display Outputs & High-DPI**: Monitor enumeration, physical geometry, refresh rates, and subpixel scale factor queries (`backend_os::Output`).
  - **Safe Area Insets**: Query display notch, status bar, and gesture navigation bar boundaries across mobile and desktop screens.
  - **Cross-Platform Permissions**: Dynamic permission requests for camera, audio, and storage on embedded and mobile targets (`backend_os::Permissions`).
  - **Radical Independence**: 100% free of GLFW, SDL, or external framework bloat.

### 14. Sovereign Tiled-Span Hybrid Damage Tracking Subsystem (`nisaba::damage`)
- **Radical Sub-Element & Pixel-Span Precision**:
  - Blends high-performance 2D micro-tiles with contiguous 1D horizontal scanline spans (`Span`), achieving fine-grained dirty tracking at sub-component and pixel granularity.
  - Zero heap churn during steady-state invalidation passes: utilizes dense flat 64-bit bitmasks for $O(1)$ spatial tile marking and queries.
- **Micro-Tile Grid (`MicroTileGrid<TileSize = 16>`)**:
  - Decomposes the framebuffer into uniform $16 \times 16$ pixel micro-tiles.
  - Fast bitwise arithmetic: translates arbitrary bounding boxes into grid tile coordinates using bit shifts and 64-bit bitmask operations.
- **Horizontal Span Coalescing (`TiledSpanTracker<16>`)**:
  - Scans dirty micro-tiles to generate contiguous horizontal scanline spans (`Span { y, x0, x1 }`) and coalesced rectangles (`DamageRegion` / `std::vector<ScreenIntRect>`).
  - Completely eliminates rasterizer traversal, shading, and blending passes across unchanged static UI areas.
- **Multi-Buffer History & Page-Flipping (`BufferAgeTracker`)**:
  - Solves the multi-buffering damage problem across double-buffered and triple-buffered display pipelines (Wayland, DRM/KMS, X11, EGL/Vulkan).
  - Tracks per-buffer damage history using `buffer_age`: automatically unions historical damage from previous frames so that newly swapped backbuffers receive complete, artifact-free updates with zero ghosting or tearing.
- **Direct Canvas & Native Wayland Compositor Integration**:
  - Restricts software rendering passes directly via `Canvas::clip_rect(const ScreenIntRect&)`.
  - Propagates damage rectangles to the display server via `backend_os::Window::setDamage()` and Wayland `wl_surface_damage_buffer()`, slashing compositor blit overhead and GPU memory bus bandwidth.
- **Empirical 10-Iteration Benchmark Validation (Full Repaint vs. Damage Tracking)**:
  - Validated over 10 consecutive iterations per scenario on a full HD $1280 \times 800$ glassmorphic interactive dashboard (`benchmarks/benchmark_damage_tracking.cpp`):
    - **Micro-Pulse (Blinking cursor $4 \times 20$ px)**: **678x - 775x faster** (Mean latency dropped from **5.42 ms** down to **0.008 ms / 8 $\mu$s**, **99.85% frame time saved**, **135,863 FPS**).
    - **Sub-Component (Counter & Gauge $190 \times 35$ px)**: **252x - 265x faster** (Mean latency dropped from **5.04 ms** down to **0.020 ms / 20 $\mu$s**, **99.60% frame time saved**, **56,573 FPS**).
    - **Multi-Zone (4 concurrent micro-spots)**: **92x - 106x faster** (Mean latency dropped from **5.08 ms** down to **0.055 ms / 55 $\mu$s**, **98.92% frame time saved**, **20,092 FPS**).

### 15. Sovereign Lottie Vector Animation Engine (`nisaba::lottie` & `nisaba::json`)
- **100% Technological Sovereignty & Zero Dependencies**:
  - Sovereign C++20 Bodymovin/Lottie JSON parser and real-time vector animation playback engine.
- **Embedded Sovereign High-Speed JSON Parser (`nisaba::json`)**:
  - Zero-allocation recursive-descent DOM parser with full standard UTF-8 parsing, IEEE-754 numeric conversion, and strict validation (`JsonValue`).
- **Complete Lottie Vector Primitives & Layer Tree**:
  - **Layers**: Precomp, Solid, Image, Null, Shape, and Text layers with opacity, nesting, hierarchical parenting, and in/out frame bounds.
  - **Vector Shapes**: Groups (`gr`), Bezier Paths (`sh`), Ellipses (`el`), Rounded Rectangles (`rc`), Solid Fills (`fl`), Vector Strokes (`st` with line caps, joins, and miter limits), and Transforms (`tr`).
  - **Dynamic Modifiers**: Trim Paths (`tm`) supporting simultaneous and individual modes with animated start, end, and offset properties for elastic vector spinners and line reveals.
- **Parametric Properties & Multidimensional Easing Curves (`Property<T>`)**:
  - Multi-keyframe animation tracks with cubic Bezier easing curves (`in_tangent`, `out_tangent`) for elastic, bounce, and deceleration dynamics.
  - Continuous parametric interpolation across scalar floats, 2D vectors (`Point`), and 4D colors (`Color`).
- **Polymorphic CPU & Hardware GPU Vector Canvas (`ICanvas`)**:
  - Unified drawing abstraction enabling identical vector animation code to execute across software rasterizers (`Canvas`) and direct hardware GPU pipelines (`GpuCanvas`).
  - Raw hardware vector render finishes in **~0.25 ms** per frame on GPU across concurrent animations, achieving **200+ FPS** uncapped real-time framerates.
- **Interactive Animation Player Lifecycle (`Player`)**:
  - Real-time playback controller supporting `play()`, `pause()`, `seek_frame()`, `seek_progress()`, speed scaling (`0.25x` to `4.0x`), and continuous looping.

### 16. Sovereign Markdown Document Layout & Rendering Subsystem (`nisaba::markdown`)
- **100% Sovereign Architecture & Zero Dependencies**:
  - Native C++20 document parsing, layout solving, and visual presentation engine built strictly according to CommonMark and GitHub Flavored Markdown (GFM) standards.
  - Zero external markup libraries, zero third-party bindings, and zero runtime overhead.
- **Abstract Syntax Tree (AST) Document Representation (`MarkdownNode`)**:
  - Full structural node hierarchy: `Document`, `Heading` (levels 1 through 6), `Paragraph`, `Blockquote`, `List` (ordered and unordered with bullet style resolution), `ListItem`, `CodeBlock` (indented and fenced with language identifiers), `ThematicBreak` (horizontal rules), and `Table`.
  - Rich inline formatting spans: `Text`, `Emphasis` (italic), `Strong` (bold), `Strikethrough`, `Code` (inline monospace), `Link` (interactive URI targets), `Image` (embedded graphics), and `AutoLink`.
  - Comprehensive GFM Tables: header identification, per-column text alignment (`Left`, `Center`, `Right`), and multi-row data cell grid layout.
- **Extensible Theming & Design System (`MarkdownTheme`)**:
  - Fully configurable typography hierarchy: heading font sizes, line heights, paragraph spacing, and text margins.
  - Granular color styling for dark and light surfaces: background colors, body text, link highlights, code syntax background, and quote borders.
  - Specialized callout and blockquote styling: customizable border thickness, left padding, and tinted backdrop surfaces.
  - Table styling: cell padding, header fill, alternating row striping, and grid border strokes.
- **Multi-Page Layout & Multilingual Typography Integration (`MarkdownRenderer`)**:
  - Direct integration with Nisaba's sovereign typography engine (`FontSystem`, `Buffer`) for complex bidirectional Arabic shaping, CJK character support, and subpixel glyph positioning.
  - Automated page breaking and vertical pagination with configurable margins (`top`, `bottom`, `left`, `right`).
  - Running headers, footers, page numbering, and visual separators.
- **Sovereign Vector PDF Export Pipeline (`MarkdownPdfExporter`)**:
  - Compiles parsed Markdown AST directly into standard vector PDF documents.
  - Preserves true vector outlines for typography, vector geometric primitives for table borders, rules, and callout containers, and embeds high-resolution raster images without fidelity loss.

### 17. Sovereign Full-Specification PDF Document Engine (`nisaba::pdf`)
- **Full ISO 32000 Standard Architecture & Zero Dependencies**:
  - Sovereign C++20 PDF reading, parsing, rendering, and vector generation engine built from the ground up without external libraries.
  - Strict memory safety, bounded allocations, and resilience against malformed streams.
- **Lexer & Recursive Object Model (`PdfParser` & `PdfValue`)**:
  - Recursive-descent parser supporting all PDF primitive and composite types: Booleans, Integers, Reals, Literal Strings with escape decoding, Hexadecimal Strings, Names (`/Name`), Arrays, Dictionaries, Indirect Object References (`R`), and binary Streams.
  - Deferred and cached object resolution (`PdfParser::resolve`) with cycle detection preventing circular reference exploits.
- **Comprehensive Typography & Font Subsystem (`PdfCMap`, `CIDFontType2`, `FontSystem`)**:
  - **`ToUnicode` CMap Stream Parser**: Decodes embedded character maps (`beginbfchar`/`endbfchar`, `beginbfrange`/`endbfrange`), resolving character codes directly to Unicode codepoints including multi-byte surrogate pairs.
  - **Type 0 & Composite Fonts**: Handles `/CIDFontType2` and `Identity-H` two-byte character sequences with exact glyph advance calculation and font metrics.
  - **Embedded Font Extraction**: Automatically extracts `/FontFile2` (TrueType) and `/FontFile3` streams from `/FontDescriptor` dictionaries and injects them directly into Nisaba's native vector `FontSystem`.
  - **Full Text Operator Support**: State matrix operations (`BT`, `ET`, `Tf`, `Tm`, `Td`, `TD`, `T*`) and string display operators (`Tj`, `TJ` with inter-character kerning arrays, `'`, `"`).
- **Modern Object Architecture & Cross-Reference Streams**:
  - **Cross-Reference Streams (`/Type /XRef`)**: Reads compressed binary cross-reference streams alongside traditional text xref tables, resolving Type 0 (free), Type 1 (uncompressed), and Type 2 (compressed within object streams) entries.
  - **Object Streams (`/Type /ObjStm`)**: Parses consolidated object stream containers (`/N` object count, `/First` byte offset) with offset indexing for instant object lookups.
  - **Predictor Decompression**: Implements all standard predictor functions (PNG Predictors 10-15: Sub, Up, Average, Paeth; and TIFF Predictor 2 horizontal difference) across multi-channel streams.
  - **Incremental Update Chains**: Traverses document revision histories via `/Prev` xref pointers with cycle detection.
- **Advanced Vector Graphics, Shadings & Patterns**:
  - **Vector Graphics Pipeline**: Full execution of path construction (`m`, `l`, `c`, `v`, `y`, `h`, `re`), clipping (`W`, `W*`), and painting operators (`S`, `s`, `f`, `F`, `f*`, `B`, `B*`, `b`, `b*`, `n`).
  - **Gradients & Shadings (Types 2 & 3)**: Axial (linear) and Radial shadings with domain extension (`/Extend [true true]`) and multi-stage stitching and exponential transition functions (Types 0, 2, 3).
  - **Freeform & Coons Mesh Shadings (Types 4, 5, 6, 7)**: Gouraud-shaded triangle meshes and bicubic Coons patch meshes utilizing Nisaba's sovereign `Mesh` and `CoonsPatch` engines.
  - **Tiling & Shading Patterns (Types 1 & 2)**: Re-entrant stream cell rendering with bilinear tiling repetition (`SpreadMode::Repeat`) and pattern coordinate transformation matrices.
- **Transparency, Blend Modes & Color Spaces**:
  - **All 16 Blend Modes**: Direct hardware and software compositing for `Normal`, `Multiply`, `Screen`, `Overlay`, `Darken`, `Lighten`, `ColorDodge`, `ColorBurn`, `HardLight`, `SoftLight`, `Difference`, `Exclusion`, `Hue`, `Saturation`, `Color`, `Luminosity`.
  - **Transparency Groups (`/Group`)**: Isolated and non-isolated offscreen compositing surfaces with alpha blending and opacity factors (`ca`, `CA`).
  - **Soft Masks (`/SMask`)**: Luminosity masks ($Y = 0.2126 R + 0.7152 G + 0.0722 B$) and direct alpha masks.
  - **Advanced Color Spaces**: Full support for DeviceRGB, DeviceCMYK, DeviceGray, CalRGB, CalGray, Lab, ICCBased, Indexed (1, 2, 4, 8 bits with lookup palette decoding), Separation, and DeviceN with tint transform functions.
  - **Image Stencils (`ImageMask`)**: 1-bit monochrome stencil masking rendered in current fill paint.
- **Sovereign Codecs & Media Decoders (`nisaba::pdf::codec`)**:
  - `FlateDecode` (Zlib/Deflate with automatic header validation).
  - `CCITTFaxDecode` (Group 3 and Group 4 2D facsimile decompression with 2D Pass, Horizontal, and Vertical modes for archival and scanned documents).
  - `ASCII85Decode` (standard 5-tuple base-85 with 'z' run handling), `ASCIIHexDecode`, and `RunLengthDecode`.
- **Sovereign Standard Security Handler & Cryptography (`nisaba::pdf::crypto`)**:
  - Native standard password authentication (ISO 32000-1 Algorithms 2, 4, and 7) for user and owner encryption keys.
  - Pure C++20 cryptographic primitives: MD5 (RFC 1321) and SHA-256 (FIPS 180-4).
  - Stream and string ciphers: RC4 (40-bit and 128-bit) with per-object key derivation (Algorithm 1), AES-128 (CBC mode), and AES-256 (CBC mode) with PKCS#7 unpadding.
  - Automatic transparent decryption of strings and streams during object resolution.
- **Hardware-Accelerated GPU Rendering & Navigation (`nisaba::pdf::navigation`)**:
  - Direct vector rendering onto hardware GPU pipelines (`PdfReader::render_page_gpu`) via `GpuCanvas` and Vulkan/OpenGL ES, achieving smooth 120+ FPS continuous viewport scrolling.
  - Hierarchical bookmarks and table of contents (`/Outlines`) with multi-byte text decoding (PDFDocEncoding, UTF-16BE, UTF-8 BOM), styles (bold/italic), and RGB colors.
  - Interactive hyperlink annotations (`/Annots` with `/Link`) supporting internal page jumps (`/GoTo`), external web links (`/URI`), external files (`/GoToR`), and named destinations with automatic page resolution (`resolve_destination_page`).
- **Sovereign Vector PDF Generation (`PdfWriter`)**:
  - Programmatic creation of compact, compliant vector PDF files with embedded TrueType fonts, vector paths, images, and cross-reference tables.

### 18. Sovereign Geometric Path Boolean Operations (`nisaba::path_ops`)
- **Planar Graph & Sweep-Line Decomposition**:
  - Exact planar graph representation decomposing intersecting vector paths into a non-overlapping planar segment arrangement.
  - Sweep-line intersection solver identifying all line-line, line-curve, and curve-curve intersection vertices with exact parameter splitting.
- **Topological Winding Classification**:
  - Evaluates segment midpoints against input paths utilizing non-zero and even-odd winding accumulation to determine topological containment ($in_A, in_B$).
- **Complete Boolean Algebra (`PathOp`)**:
  - `PathOp::Union`: Computes the geometric union ($A \cup B$).
  - `PathOp::Difference`: Computes the geometric difference ($A \setminus B$).
  - `PathOp::Intersect`: Computes the mutual intersection ($A \cap B$).
  - `PathOp::Xor`: Computes the symmetric difference ($A \oplus B$).
- **Cycle Extraction & Directed Contours**:
  - Extracts closed clockwise and counter-clockwise loop cycles using angle-ordered edge incidence tables, outputting valid, wound `nisaba::Path` objects.
  - Seamlessly accessible through functional APIs (`nisaba::path_ops::compute_op`) and object convenience methods (`Path::unite`, `Path::difference`, `Path::intersect`, `Path::xor_op`).

### 19. Sovereign Path Outlining & Clean Stroke-to-Fill Geometry (`nisaba::stroker`)
- **Exact Vector Contour Expansion (`Path::stroke_to_fill`)**:
  - Expands stroked vector geometries into closed fillable polygon boundaries based on configurable `StrokeStyle` parameters.
  - Full support for stroke caps (`LineCap::Butt`, `Round`, `Square`), joins (`LineJoin::Miter`, `Round`, `Bevel`), and miter cut-off limits.
  - Generates parallel offset curves and exact circular arc caps/joins for both open paths and closed subpaths.
- **Topological Self-Intersection Resolution (`Path::stroke_to_fill_clean`)**:
  - Automatically pipes generated stroke boundary polygons through the planar `PathOps` Union solver.
  - Resolves overlapping ribbon folds, self-intersecting loops, and tight-corner miter self-intersections into single, continuous, non-overlapping vector boundaries with positive winding.

### 20. Multi-Format Framebuffer & Embedded Color Architectures (`PixelFormat`)
- **Native Color Format Hierarchy**:
  - `PixelFormat::RGBA8888`: Standard 32-bit linear/sRGB 4-channel color representation.
  - `PixelFormat::BGRA8888`: Native 32-bit hardware scanout format for display servers and compositors.
  - `PixelFormat::RGB565`: High-performance 16-bit packed format (5 bits Red, 6 bits Green, 5 bits Blue) reducing memory bandwidth and buffer size by 50% for microcontrollers and memory-restricted LCD displays.
  - `PixelFormat::Alpha8` & `PixelFormat::Gray8`: Single-channel 8-bit surfaces for alpha masks, font glyph caches, and monochrome displays (75% memory reduction).
- **Zero-Allocation Dedicated Span Blitters**:
  - Direct bitwise assembly and span operations: `blit_row_rgb565`, `blit_row_alpha8`, `blit_row_gray8`, and `blit_row_bgra8888`.
  - Fast bit-shifting channel unpack and repack arithmetic ($R_5 \leftrightarrow R_8$, $G_6 \leftrightarrow G_8$, $B_5 \leftrightarrow B_8$) with exact rounding and saturation.
  - Unified memory lifecycle: allocate formatted surfaces via `Pixmap::create(w, h, format)` or wrap external display hardware buffers via `PixmapMut::from_raw_parts(ptr, w, h, pitch, format)`.

### 21. Linear sRGB Blending & Photometric Alpha Compositing (`nisaba::color`)
- **Physical Light Linearity (IEC 61966-2-1)**:
  - Eliminates gamma distortion, dark boundary fringes, and optical edge dimming inherent in non-linear color blending by performing alpha compositing in physical linear light space.
- **Ultra-Compact Dual 12-Bit Lookup Tables (`ColorSpaceLUT`)**:
  - `srgb_to_linear_u12`: Maps 8-bit non-linear sRGB values to 12-bit linear photometric intensities.
  - `linear_u12_to_srgb`: Maps 12-bit linear values back to 8-bit non-linear sRGB with rounded quantization.
  - Total memory footprint is strictly **4.5 KB**, guaranteeing permanent residency inside the CPU L1 data cache without eviction.
- **Selective Runtime Control**:
  - Toggled per-draw via `Paint::set_linear_blending(bool)` / `paint.linear_blending`.
  - Integrated directly across solid colors, gradients, and custom shaders within the scanline blitter pipeline.

### 22. Advanced Shaders & Multi-Shader Compositing (`nisaba::shaders`)
- **Two-Point Conical Gradients (`TwoPointConicalGradient`)**:
  - Analytical evaluation of conical light cones and spotlight effects by solving the quadratic equation $A t^2 + 2 B t + C = 0$ across arbitrary focal centers and radii $(c_1, r_1)$ and $(c_2, r_2)$.
  - Supports extended focal radii, fully inverted conical cones, arbitrary color stops, and spread modes (`Pad`, `Reflect`, `Repeat`).
- **Compose Shaders (`ComposeShader` / `BlendShader`)**:
  - Dynamic compositing of two independent source shaders ($A$ and $B$) using any of Nisaba's 29 Porter-Duff or advanced blend modes.
  - Enables complex procedural materials, textured gradient masks, lighting modulation, and multi-pass shader composition in a single rasterization pass.

---

## Core Engine Capabilities

- **Vector Geometry & Boolean Algebra**:
  - Quadratic, Cubic, and Conic Bézier curves; *de Casteljau* subdivision, root solving, extrema, cusps, and tight bounding boxes.
  - **Arc-Length Parameterization & Measurement (`nisaba::PathMeasure`)**: Sub-millimeter contour length calculation, analytical tangent and position evaluation, and de Casteljau subsegment carving for animated spinners, progress rings, and vector morphing.
  - **Geometric Path Operations (`nisaba::path_ops`)**: Exact planar sweep-line solver (`Union`, `Difference`, `Intersect`, `Xor`).
  - **Stroke-to-Fill Outlining (`nisaba::stroker`)**: Closed polygon contour generation with clean planar self-intersection union resolution.
- **Path Stroking & Dashing**: Flexible line caps (`Butt`, `Round`, `Square`), line joins (`Miter`, `MiterClip`, `Round`, `Bevel`), arbitrary dash patterns with phase offset.
- **Anti-Aliased Scanline Rasterization**: High-precision 256-level subpixel coverage anti-aliasing with non-zero (`Winding`) and `EvenOdd` fill rules.
- **Comprehensive Pixel Formats**: Native 32-bit `RGBA8888`, native display 32-bit `BGRA8888`, 16-bit packed `RGB565`, and 8-bit `Alpha8` / `Gray8` single-channel buffers with zero-allocation bitwise row blitters.
- **Photometric Color Pipeline**:
  - **Linear sRGB Blending**: IEC 61966-2-1 optical light linearity via compact 4.5 KB L1 cache lookup tables.
  - **4x5 Affine ColorMatrix**: 20-element color filtering for saturation, hue rotation, sepia, and channel mixing.
- **Shaders & Gradients**:
  - `LinearGradient` (multi-stop arbitrary direction)
  - `RadialGradient` (1-point circular and 2-point focal)
  - `TwoPointConicalGradient` (analytical quadratic conical spotlight solver)
  - `SweepGradient` (360-degree angular color wheel)
  - `ComposeShader` / `BlendShader` (multi-shader composition combining two shaders via any blend mode)
  - `Pattern` with high-performance **Bilinear Filtering**
  - Spread modes: `Pad`, `Reflect`, `Repeat`
- **29 Blend Modes**: All 12 Porter-Duff modes (`Clear`, `Source`, `Dest`, `SourceOver`, `DestOver`, `SourceIn`, `DestIn`, `SourceOut`, `DestOut`, `SourceAtop`, `DestAtop`, `Xor`) plus 17 advanced blend modes (`Plus`, `Modulate`, `Screen`, `Overlay`, `Darken`, `Lighten`, `ColorDodge`, `ColorBurn`, `HardLight`, `SoftLight`, `Difference`, `Exclusion`, `Multiply`, `Hue`, `Saturation`, `Color`, `Luminosity`).
- **Complete W3C SVG Support**: `<path>`, `<rect>`, `<circle>`, `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`, `<defs>`, `<use>`, `<clipPath>`, `<mask>`, `<linearGradient>`, `<radialGradient>`, `<image>` (including RFC 2397 base64 inline data URIs), and affine `skewX`/`skewY` transforms.
- **Multilingual Typography & OpenType**: TrueType outlines (.ttf), PostScript CFF Type 2 Charstrings (.otf), GSUB ligatures and contextual substitutions, GPOS pair kerning, Unicode BiDi, and Arabic shaping.

---
## License & Sovereignty

The **Nisaba** engine is developed exclusively for the **`vaxp`** organization. Engineered to adhere to the highest standards of architectural purity, speed, and reliability, providing an uncompromising sovereign foundation for modern software engineering, robotics, and embedded systems.

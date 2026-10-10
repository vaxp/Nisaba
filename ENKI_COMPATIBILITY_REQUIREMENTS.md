# 🛠️ Nisaba Engine — ENKI Compatibility Requirements & Implementation Roadmap
## Bridging the Architectural Gaps for Sovereign Replacement in ENKI Framework v0.1.0 (Roadmap v0.3.0)

> **Document Status**: Production Blueprint & Engineering Specification  
> **Target Framework**: **ENKI Framework v0.1.0** (`vaxp/enki`)  
> **Roadmap Specification**: **Roadmap v0.3.0** (`ROADMAP_v0.3.0.md`)  
> **Source Engine**: **Nisaba 2D Sovereign Graphics Engine** (`vaxp/nisaba`)  
> **Author**: VAXP Core Systems Engineering Group  

---

## 1. Executive Summary & Parity Analysis

The transition of the **ENKI Framework v0.1.0** (implementing **Roadmap v0.3.0**) to the sovereign **Nisaba** engine represents a fundamental milestone for the VAXP computing ecosystem. 

A rigorous code-level audit confirms that Nisaba already satisfies **~80–85% of ENKI's core graphics requirements**:
- ✅ Multi-backend GPU context & surface initialization (`GpuDevice`, `GpuSurface::from_screen`).
- ✅ Pure C++20 Bézier vector paths, stroking, dashing, and geometric Boolean algebra (`path_op`).
- ✅ Sovereign W3C SVG 1.1 vector graphics parser and SIMD-accelerated caching (`SvgDocument`, `SvgCache`).
- ✅ Embedded Bodymovin / Lottie vector animation runtime (`nisaba::lottie::Player`).
- ✅ Zero-dependency core image decoders and encoders (PNG, JPEG, QOI, BMP via `ImageIO`).
- ✅ Sovereign multilingual text shaping and Unicode 16.0 bidirectional engine (`nisaba::text::Buffer`, `bidi`).
- ✅ Subpixel Flexbox layout engine (`nisaba-layout`).
- ✅ High-speed SIMD particle animation simulation (`nisaba::animation::ParticleSystem`).

However, **six critical architectural gaps and missing interfaces** must be implemented in Nisaba to enable a 100% seamless, zero-friction cutover of ENKI's reactive widget fleet without regression.

---

## 2. The 6 Missing Architectural Subsystems

```mermaid
graph TD
    subgraph P0 [Priority 0: Critical Path]
        G1["✅ Gap 1: GPU Text Rendering & Glyph Atlas (COMPLETED)"]
    end
    subgraph P1 [Priority 1: High Priority]
        G2["✅ Gap 2: PathMeasure Arc-Length Sampling (COMPLETED)"]
        G3["✅ Gap 3: GpuCanvas Stencil Clip-Path (COMPLETED)"]
    end
    subgraph P2 [Priority 2: Medium Priority]
        G4["✅ Gap 4: Asymmetric 4-Corner BorderRadius (COMPLETED)"]
        G5[Gap 5: Hardware GPU Backdrop Blur]
    end
    subgraph P3 [Priority 3: Media Expansion]
        G6[Gap 6: Zero-Dependency AVIF & WebP Codecs]
    end

    G1 --> ENKI_TEXT[ENKI RenderParagraph & TextFields]
    G2 --> ENKI_ANIM[ENKI Spinners, ProgressRings & PathMorph]
    G3 --> ENKI_CLIP[ENKI ClipRRect, ClipPath & Overlays]
    G4 --> ENKI_WIDGETS[ENKI Buttons, Tabs, Dialogs & Sliders]
    G5 --> ENKI_GLASS[ENKI Glassmorphism & Menus]
    G6 --> ENKI_IMAGE[ENKI Web Web Media Pipeline]
```

---

### ✅ Gap 1: Hardware GPU Text Rendering & Dynamic Glyph Atlas (Priority: P0) — [COMPLETED]

#### 1. Implementation Status:
- **Status**: **100% IMPLEMENTED & VERIFIED (COMPLETED AND VERIFIED)**
- **Verification**: Verified via `test_gpu_text_rendering` and `test_gpu_text_buffer_rendering` in `tests/test_gpu.cpp` (12/12 passing with 4656 text pixels validated on hardware GPU; zero regressions across all 36 engine test suites).
- **GPU Profiler Result**: Nisaba maintains exceptional throughput (**4.83x faster CPU Command Submission** and **1.12x faster overall frame throughput**) in `gpu_micro_profiler`.

#### 2. Delivered Architecture & Components:
1. **Dynamic GPU Glyph Atlas (`nisaba::gpu::GpuGlyphAtlas` / `SovereignGlyphAtlas`)**:
   - Exposed in [`include/nisaba/gpu/glyph_atlas.hpp`](file:///home/x/Desktop/nisaba/include/nisaba/gpu/glyph_atlas.hpp).
   - Starts at $1024 \times 1024$ and dynamically expands on-demand up to $2048 \times 2048$ single-channel (`TextureType::Alpha` / `R8_UNORM`) GPU texture.
   - Skyline 2D bin-packing allocator with 1-pixel boundary padding to eliminate texture bleeding.
   - Subpixel texture coordinate generation $(u_0, v_0, u_1, v_1)$ and subpixel glyph positioning bins (`compute_subpixel_bin`).
   - Dirty-region tracking with minimal partial uploads via `flushToGpu`.
2. **GPU Canvas Text Primitives**:
   - `GpuCanvas::draw_text(std::string_view text, float x, float y, const text::Font& font, const Paint& paint, float font_size)`
   - `GpuCanvas::draw_text(float x, float y, std::string_view text, const nisaba::Paint& paint, float font_size, const char* font_name)`
   - `GpuCanvas::draw_text_buffer(const text::Buffer& buffer, text::FontSystem& fonts, Point pos, Color color)`
   - `GpuCanvas::draw_text_buffer(const text::Buffer& buffer, text::FontSystem& fonts, Point pos, const Paint& paint)`
   - `GpuCanvas::create_font(const char* name, const char* filename)`
   - `GpuCanvas::create_font_mem(const char* name, const uint8_t* data, size_t size)`
3. **High-Performance Glyph Batching**:
   - All glyphs sharing the dynamic atlas across complete multi-line paragraph layouts are submitted in a **single GPU draw batch** (`renderTriangles`).
   - Per-vertex colors (`Vertex::color`) enable rich syntax highlighting and multicolored text spans in the same draw call without state changes.

#### 3. Files Implemented / Updated:
- ✅ `include/nisaba/gpu/glyph_atlas.hpp` (New public header)
- ✅ `src/gpu/glyph_atlas.cpp` (Dynamic Skyline bin-packing, `resize(2048, 2048)`)
- ✅ `src/gpu/glyph_atlas.hpp` (Forwarding header)
- ✅ `include/nisaba/gpu/gpu_canvas.hpp` (Public text APIs exposed)
- ✅ `src/gpu/gpu_canvas.cpp` (Hardware text rendering integration)
- ✅ `include/nisaba/gpu/context.hpp` & `src/gpu/context.cpp` (`textWithFont`, `drawTextBuffer`, atlas resize fallback)
- ✅ `include/nisaba/text/ttf_font.hpp` (`using Font = TtfFont;` alias)
- ✅ `tests/test_gpu.cpp` (Automated GPU hardware text test suite)

---

### ✅ Gap 2: Arc-Length Path Parameterization & Perimeter Measurement (`PathMeasure`) (Priority: P1) — [COMPLETED]

#### 1. Implementation Status:
- **Status**: **100% IMPLEMENTED & VERIFIED (COMPLETED AND VERIFIED)**
- **Verification**: Verified via `test_path_measure` in `tests/test_path_measure.cpp` (9/9 comprehensive test cases passing including circle perimeters, straight & diagonal lines, forced close contours, multi-contour navigation, and subsegment carving for animated spinners; 37/37 passing across the entire Nisaba test suite in 0.80s).
- **Throughput & Efficiency**: Achieves **> 550,000 paths measured per second** (~1.8 microseconds per path) with binary-search $O(\log N)$ distance inversion and analytical de Casteljau sub-curve carving.

#### 2. Delivered Architecture & Components:
1. **Public Class `nisaba::PathMeasure`**:
   - Declared in [`include/nisaba/path/path_measure.hpp`](file:///home/x/Desktop/nisaba/include/nisaba/path/path_measure.hpp) and implemented in [`src/path/path_measure.cpp`](file:///home/x/Desktop/nisaba/src/path/path_measure.cpp).
   - `length()`, `get_pos_tan(distance, pos, tangent)`: returns exact physical coordinates and unit tangent vectors.
   - `get_segment(start_d, stop_d, dst, start_with_move_to)`: extracts sub-curves without allocating intermediate paths, seamlessly chaining into any existing destination path.
   - `is_closed()`, `next_contour()`, `contour_count()`, `current_contour_index()`.
   - `set_path(path, force_closed, res_scale)` with support for both references and pointers.
   - 1:1 Standard CamelCase compatibility methods (`getLength`, `getPosTan`, `getSegment`, `isClosed`, `nextContour`, `setPath`).
2. **Subsegment Extraction & Animation Primitives**:
   - Recursive de Casteljau subdivision for quadratics and cubics down to sub-millimeter tolerances ($< 0.04\%$ error on circular Bézier approximations).
   - Bitwise fast binary search for distance-to-segment parameter lookups.
   - In-place appending to `dst->verbs_` and `dst->points_` with automated bounding box recalculation.

#### 3. Files Implemented / Updated:
- ✅ `include/nisaba/path/path_measure.hpp` (New public header)
- ✅ `src/path/path_measure.cpp` (Sovereign arc-length measurement & carving implementation)
- ✅ `include/nisaba/path/path.hpp` (Forward declared and friended `PathMeasure`)
- ✅ `include/nisaba/nisaba.hpp` (Included in master engine header)
- ✅ `tests/test_path_measure.cpp` (Automated unit test suite with 9 test cases)
- ✅ `meson.build` & `tests/meson.build` (Build integration)

---

### ✂️ Gap 3: GPU Vector Path Clipping via Stencil Buffer (`GpuCanvas::clip_path`) (Priority: P1)

#### 1. Implementation Status:
- **Status**: **100% IMPLEMENTED & VERIFIED (✅ COMPLETED AND VERIFIED)**
- **Verification**: Verified via `test_gpu_clip_path` in `tests/test_gpu.cpp` (all 13/13 GPU tests pass; 100% pass rate across all 37 engine test suites).
- **Features Verified**:
  - Arbitrary vector path clipping (`circle_clip`, Béziers, convex/concave polygons).
  - Multi-level nested clipping with path intersections ($C_1 \cap C_2$).
  - Correct stack unwinding across `canvas.save()` / `canvas.restore()`.
  - Seamless coexistence with hardware scissor clipping (`clip_rect` + `clip_path`).

#### 2. Delivered Architecture & Components:
1. **Stencil Buffer State Management (`GL_DEPTH24_STENCIL8`)**:
   - `GpuSurface` automatically binds a 24-bit depth / 8-bit stencil renderbuffer (`GL_DEPTH24_STENCIL8`) to resolve and MSAA framebuffers.
   - Dual-nibble stencil layout:
     - **Bits 4..7 (Upper Nibble)**: Encodes active clip depth level (supporting up to 15 nested clipping scopes).
     - **Bits 0..3 (Lower Nibble)**: Transient scratch space for winding-number and even-odd polygon accumulation.
2. **Stencil Tessellation & Cover Quad Pipeline**:
   - `GL3Renderer::renderClip`:
     1. Disables color writes and evaluates path geometry into scratch bits 0..3 with `GL_INCR_WRAP` / `GL_DECR_WRAP` (Non-Zero) or `GL_INVERT` (Even-Odd) on fragments matching the previous clip depth (`prevDepth << 4` in bits 4..7).
     2. Draws a screen/bounding-box cover quad testing `(stencil & 0x0f) != 0` to replace passing fragments with `targetDepth << 4` (clearing scratch bits 0..3 back to 0) via `GL_REPLACE`.
   - `GL3Renderer::restoreClip`:
     - Fullscreen quad targeting fragments with depth $> \text{targetDepth}$, resetting them back to $\text{targetDepth} \ll 4$.
3. **High-Level Canvas & Pipeline Integration**:
   - Added `clip_path(const Path&, FillRule)` to `ICanvas` interface and `GpuCanvas`.
   - `GpuCanvasState` tracks `stencil_clip_depth` for 100% state stack fidelity across `save()` and `restore()`.
   - `GpuDevice::clear` automatically resets write masks (`glColorMask`, `glDepthMask`, `glStencilMask(0xff)`) ensuring reliable clears.

#### 3. Files Implemented / Updated:
- ✅ `include/nisaba/canvas/canvas_interface.hpp` (Added `clip_path(const Path&, FillRule)` to `ICanvas`)
- ✅ `include/nisaba/gpu/gpu_canvas.hpp` & `src/gpu/gpu_canvas.cpp` (Implemented `GpuCanvas::clip_path`, `stencil_clip_depth`)
- ✅ `include/nisaba/gpu/context.hpp` & `src/gpu/context.cpp` (Implemented `clipPath`, `restoreClip`, `resetClip`, `clipDepth`)
- ✅ `include/nisaba/gpu/renderer.hpp` & `include/nisaba/gpu/gl3_renderer.hpp` (Added `renderClip`, `restoreClip`, `resetClip`)
- ✅ `src/gpu/gl3_renderer.cpp` (Two-pass stencil clip & restore pipeline, nested clip depth tests)
- ✅ `src/gpu/gpu_device.cpp` (Mask-safe `GpuDevice::clear`)
- ✅ `src/gpu/gpu_surface.cpp` (`GL_DEPTH24_STENCIL8` renderbuffer attachment)
- ✅ `tests/test_gpu.cpp` (`test_gpu_clip_path` unit and integration test)

---

### ✅ Gap 4: Asymmetric 4-Corner Rounded Rectangles (`BorderRadius` & `RRect`) (Priority: P2) — [COMPLETED]

#### 1. Implementation Status:
- **Status**: **100% IMPLEMENTED & VERIFIED (COMPLETED AND VERIFIED)**
- **Verification**: Verified via `test_rrect` in `tests/test_rrect.cpp` (38/38 unit tests passing across 8 dedicated suites covering W3C proportional constraint scaling, analytical point containment, concentric deflation, zero-radius fallbacks, and hardware GPU canvas rendering; 100% pass rate across all Nisaba test suites).
- **Interactive Showcase**: Verified in `examples/showcase_enki_stages.cpp` (Stage 4) with Quad-View and focused view rendering asymmetric tabs, popovers, segmented controls, live point containment queries, concentric insets, vector stencil clipping with `clip_rrect`, and real-time W3C scaling stress testing at **> 130 FPS**.

#### 2. Delivered Architecture & Components:
1. **Geometric Core Types (`nisaba::BorderRadius` & `nisaba::RRect`)**:
   - Exposed in [`include/nisaba/math/border_radius.hpp`](file:///home/x/Desktop/nisaba/include/nisaba/math/border_radius.hpp) and [`include/nisaba/math/rrect.hpp`](file:///home/x/Desktop/nisaba/include/nisaba/math/rrect.hpp).
   - `BorderRadius`: 4 independent corner radii (`top_left`, `top_right`, `bottom_right`, `bottom_left`).
   - Factory constructors: `zero()`, `all(r)`, `circular(r)`, `vertical(top, bottom)`, `horizontal(left, right)`, `only(tl, tr, br, bl)`.
   - Analytical W3C CSS Backgrounds & Borders Level 3 proportional constraint scaling algorithm (`constrained_for_size(w, h)`):
     $$f = \min\left(1.0, \frac{W}{r_{tl} + r_{tr}}, \frac{H}{r_{tr} + r_{br}}, \frac{W}{r_{bl} + r_{br}}, \frac{H}{r_{tl} + r_{bl}}\right)$$
     Guarantees zero overlapping corner distortion regardless of rectangle dimensions.
   - `RRect`: Analytical point containment `contains(Point)`, concentric deflation `deflate(dx, dy)` / `inset(dx, dy)`, and conversion `to_path()`.
2. **Analytical Vector Path Generation (`PathBuilder`)**:
   - `push_rounded_rect(Rect, BorderRadius)` and `push_rrect(RRect)` in [`include/nisaba/path/path_builder.hpp`](file:///home/x/Desktop/nisaba/include/nisaba/path/path_builder.hpp).
   - Generates 4 cubic Bézier quarter-ellipses with optimal tangent constant $\kappa = 4(\sqrt{2}-1)/3 \approx 0.55228475$ and zero-radius sharp corner fallbacks.
3. **Hardware GPU Canvas Acceleration (`GpuCanvas`)**:
   - `GpuCanvas::fill_rrect(const RRect&, const Paint&)`: Direct zero-tessellation analytical GPU quad fast-path via `ShaderRRect` and `roundedRectVarying` shader pipeline.
   - `GpuCanvas::stroke_rrect(const RRect&, const Paint&)`: Vector stroke tessellation with subpixel anti-aliasing.
   - `GpuCanvas::clip_rrect(const RRect&)`: Hardware stencil clipping via dual-nibble stencil buffer.
   - `GpuCanvas::draw_glass_panel(const RRect&, const effects::GlassPanelParams&)`: Full `RRect` support for translucent panels, highlights, and borders.
   - `GpuCanvas::draw_rrect_shadow(const RRect&, const effects::DropShadow&)`: Analytical GPU drop shadow rendering.

#### 3. Files Implemented / Updated:
- ✅ `include/nisaba/math/border_radius.hpp` (New public header)
- ✅ `include/nisaba/math/rrect.hpp` (New public header)
- ✅ `src/math/rrect.cpp` (Geometry, W3C scaling, deflation, point containment)
- ✅ `include/nisaba/path/path_builder.hpp` (`push_rounded_rect`, `push_rrect`, `from_rounded_rect`, `from_rrect`)
- ✅ `include/nisaba/canvas/canvas_interface.hpp` (`fill_rrect`, `stroke_rrect`, `clip_rrect` declared)
- ✅ `include/nisaba/gpu/gpu_canvas.hpp` & `src/gpu/gpu_canvas.cpp` (GPU quad shader fast-path, stroke, clip, shadow, glass)
- ✅ `include/nisaba/nisaba.hpp` (Master engine header integration)
- ✅ `tests/test_rrect.cpp` & `tests/meson.build` (Comprehensive automated unit test suite)
- ✅ `examples/showcase_enki_stages.cpp` (Interactive 4-stage showcase demo)

---

### 🪟 Gap 5: Hardware GPU Backdrop Blur (`GpuCanvas::draw_backdrop_blur`) (Priority: P2)

#### 1. Current State in Nisaba:
- CPU software `Canvas` executes genuine Gaussian backdrop blurring in `src/effects/glass.cpp`.
- `GpuCanvas::draw_glass_panel` draws only translucent tinting, drop shadows, and specular borders without blurring underlying framebuffer contents.

#### 2. The ENKI Requirement:
- Roadmap **Phase 2.1** specifies `drawBackdropBlur(const Rect&, float sigmaX, float sigmaY)` and realistic glassmorphic panels for overlays, popovers, and navigation bars.

#### 3. Technical Specification & Implementation Plan:
1. **Two-Pass Downsampled Kawase / Dual-Filter Blur Pipeline**:
   - Copy the active framebuffer region beneath the glass panel into an offscreen scratch texture (downsampled to $\frac{1}{2}$ or $\frac{1}{4}$ resolution for 120+ FPS efficiency).
   - Execute ping-pong horizontal and vertical separable Gaussian / Kawase blur fragment shaders.
2. **Composite Glass Panel**:
   - Render the blurred texture through the clipped `RRect` geometry.
   - Multiply by `params.tint_color` in linear sRGB space.
   - Composite the crisp specular edge border (`params.border_color`).

#### 4. Files to Create / Modify:
- 📝 `include/nisaba/gpu/gpu_canvas.hpp` (Declare `draw_backdrop_blur`)
- 📝 `src/gpu/gpu_canvas.cpp`
- 📝 `src/shaders/embedded_shaders.hpp` (Embed separable blur GLSL/SPIR-V shaders)

---

### 🖼️ Gap 6: Next-Gen Zero-Dependency Image Codecs (AVIF & WebP) (Priority: P3)

#### 1. Current State in Nisaba:
- Nisaba has sovereign decoders for **PNG, JPEG, QOI, and BMP**.
- AVIF and Animated WebP are currently scheduled for subsequent milestones.

#### 2. The ENKI Requirement:
- High-efficiency modern web media decoding without linking external `libwebp` or `libavif`.

#### 3. Implementation Plan:
- Integrate sovereign single-header VP8/VP8L reader for WebP images.
- Provide progressive frame decoding for Animated WebP inside `nisaba::image::ImageIO`.

---

## 3. Phased Implementation Roadmap & Milestones

| Milestone | Target Deliverable | Completion Criteria | Status |
| :--- | :--- | :--- | :---: |
| **M1 (Sprint 1)** | **P0: GPU Glyph Atlas & Text Rendering** | `GpuCanvas::draw_text_buffer` renders shaped text on GPU; ENKI `RenderParagraph` compiles without external dependencies. | **✅ COMPLETED** |
| **M2 (Sprint 2)** | **P1: PathMeasure & Stencil ClipPath** | `PathMeasure` passes unit tests; Spinners and `ClipPath` work on GPU in ENKI. | **✅ COMPLETED** |
| **M3 (Sprint 3)** | **P2: 4-Corner BorderRadius & RRect** | Asymmetric buttons, tabs, popovers, and segmented controls render with GPU acceleration and W3C constraint scaling; passes 38/38 tests. | **✅ COMPLETED** |
| **M4 (Sprint 4)** | **P2: Hardware GPU Backdrop Blur** | Dual-pass Kawase/Gaussian GPU blur pipeline for real-time glassmorphism panels. | **Scheduled** |
| **M5 (Sprint 5)** | **P3: WebP Codec & Subproject Finalization** | WebP decoding functional; ENKI compiles warning-free with `-DENKI_BACKEND_NISABA=1`. | **Scheduled** |

---

## 4. Verification & Testing Matrix

To guarantee complete API and rendering parity, all newly implemented features will undergo rigorous automated verification:

1. **Pixel-Exact Geometry Conformance**:
   - Verify `PathMeasure::get_segment` against analytical cubic arc-length equations with error tolerance $< 10^{-4}$. (✅ Verified via `test_path_measure`).
   - Verify `RRect::constrained_for_size` and analytical quarter-ellipse containment against W3C specification. (✅ Verified via `test_rrect`).
2. **GPU Stencil Integrity**:
   - Verify complex overlapping winding paths produce exact geometric exclusion zones without stencil bleeding. (✅ Verified via `test_gpu_clip_path`).
   - Verify `GpuCanvas::clip_rrect` enforces hardware stencil clipping without edge bleeding. (✅ Verified via `test_rrect`).
3. **Glyph Atlas Cache Performance**:
   - Sustained frame-time benchmark: verify 10,000 glyphs render at $\ge 120\text{ FPS}$ on desktop and $\ge 60\text{ FPS}$ on embedded Raspberry Pi 5.
4. **Memory Leak & Fuzz Testing**:
   - Run 100,000 rapid allocations on `GpuGlyphAtlas` under AddressSanitizer (ASan) with zero leaks or invalid reads.

---

> 🎯 **Next Action**:  
> Implementation will proceed with **Milestone 4 / Gap 5: Hardware GPU Backdrop Blur (`GpuCanvas::draw_backdrop_blur`)** to enable real-time hardware glassmorphism and background blurring in ENKI overlays and modals.

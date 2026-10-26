# Nisaba SVG Engine (`nisaba::svg`) — Technical Reference

## 1. Architecture Overview

`nisaba::svg` is the vector graphics parsing, styling, scene-graph evaluation, filter compositing, caching, and rendering subsystem of the Nisaba engine. It operates without external XML, CSS, or vector rendering dependencies and outputs directly to both `nisaba::Canvas` (CPU SIMD scanline rasterization) and `nisaba::gpu::GpuCanvas` (OpenGL / Vulkan hardware-accelerated rendering).

### Subsystem Structure

| Header / Source | Responsibility |
| :--- | :--- |
| `include/nisaba/svg/path_parser.hpp`<br>`src/svg/path_parser.cpp` | Zero-allocation SVG path data (`d="..."`) lexer (`PathLexer`) and parser (`PathParser`), including implicit command repetition, cubic/quadratic Bézier reflection (`S`/`s`, `T`/`t`), and W3C F.6.5 elliptical arc-to-cubic parameterization (`A`/`a`). |
| `include/nisaba/svg/svg_document.hpp`<br>`src/svg/svg_document.cpp` | XML tokenizer, entity decoder, unit converter, W3C color parser, internal `<style>` CSS parser & specificity engine, hierarchical scene graph (`SvgDocument`, `SvgElement`), paint servers (`SvgGradient`, `SvgPattern`), clipping/masking (`SvgClipPath`, `SvgMask`), markers (`SvgMarker`), typography (`SvgTextSpan`), filter graph execution (`SvgFilter`, `SvgFilterPrimitive`), CPU/GPU rendering (`render`, `render_gpu`), and vector hit-testing (`hit_test`, `get_element_bounds`). |
| `include/nisaba/svg/baked_svg.hpp`<br>`src/svg/baked_svg.cpp` | Pre-rasterized RGBA8888 surface (`BakedSvg`) for $O(1)$ SIMD scanline blitting (`draw`) with optional bilinear scaling and opacity modulation. |
| `include/nisaba/svg/svg_cache.hpp`<br>`src/svg/svg_cache.cpp` | Hash-indexed LRU cache (`SvgCache`) keyed by `SvgCacheKey` (document hash, target dimensions, and `SvgTheme` state hash). |

---

## 2. Lexical Analysis, Path Grammar & Coordinate Units

### 2.1 Zero-Allocation Path Lexer (`PathLexer` & `PathParser`)
`PathParser::parse(std::string_view d)` and `PathParser::parse_into(std::string_view d, PathBuilder& builder)` process W3C SVG 1.1 / 2.0 path strings in a single pass without heap allocations:
- **Supported Commands:**
  - MoveTo (`M`, `m`), with subsequent coordinate pairs implicitly promoted to LineTo (`L`, `l`).
  - LineTo (`L`, `l`), Horizontal LineTo (`H`, `h`), Vertical LineTo (`V`, `v`).
  - Cubic Bézier (`C`, `c`) and Smooth Cubic Bézier (`S`, `s`), reflecting the previous cubic control point $\mathbf{p}_{c1} = 2\mathbf{p}_{\text{cur}} - \mathbf{p}_{c2,\text{prev}}$ when preceded by `C`/`c`/`S`/`s`.
  - Quadratic Bézier (`Q`, `q`) and Smooth Quadratic Bézier (`T`, `t`), reflecting the previous quadratic control point when preceded by `Q`/`q`/`T`/`t`.
  - Elliptical Arc (`A`, `a`) and ClosePath (`Z`, `z`).
- **Minified Grammar Support (`PathLexer::next_number` / `next_flag`):**
  - Parses sign-delimited numbers without whitespace (`10-20` $\to$ `10.0f`, `-20.0f`).
  - Parses decimal-delimited numbers without whitespace (`.5.5` $\to$ `0.5f`, `0.5f`).
  - Parses scientific notation (`1e-4`, `-2.5E+2`).
  - `next_flag` consumes single-character boolean arc flags (`'0'` or `'1'`) directly, handling concatenated arc flag sequences such as `a25,25-30 0150-25`.
- **Elliptical Arc Conversion (`emit_arc_as_cubics`):**
  - Converts endpoint parameterization $(x_1, y_1, r_x, r_y, \phi, f_A, f_S, x_2, y_2)$ to center parameterization $(c_x, c_y, \theta_1, \Delta\theta)$.
  - Automatically scales radii $(r_x, r_y)$ by $\sqrt{\lambda}$ when $\lambda = \frac{x_1'^2}{r_x^2} + \frac{y_1'^2}{r_y^2} > 1$.
  - Subdivides $\Delta\theta$ into segments of at most $\frac{\pi}{2}$ radians and approximates each circular arc segment using cubic Bézier control points scaled by $\alpha = \frac{4}{3}\tan\left(\frac{\Delta\theta_{\text{seg}}}{4}\right)$.

### 2.2 Physical & Relative Length Units (`parse_float` / `parse_coord`)
All numeric attributes and style properties support CSS/SVG unit suffixes evaluated at 96 DPI:
- **Absolute Units:** `px` ($1.0$), `pt` ($\frac{96}{72} = 1.3333$), `pc` ($16.0$), `in` ($96.0$), `cm` ($\frac{96}{2.54} \approx 37.7953$), `mm` ($\frac{96}{25.4} \approx 3.7795$).
- **Font-Relative Units:** `em` (scaled by current element `font_size`), `rem` ($16.0$), `ex` ($0.5 \times \text{font\_size}$).
- **Viewport-Relative Units:** `%` (resolved against reference width, height, or normalized viewport diagonal $\frac{\sqrt{W^2 + H^2}}{\sqrt{2}}$ for radial quantities).
- **Angular Units:** `deg` ($1.0$), `rad` ($\frac{180}{\pi}$), `grad` ($0.9$), `turn` ($360.0$).

### 2.3 XML Entity Decoding & CDATA
`decode_xml_entities` resolves standard XML character references (`&amp;`, `&lt;`, `&gt;`, `&quot;`, `&apos;`, `&nbsp;`), decimal numeric character references (`&#NNNN;`), and hexadecimal numeric references (`&#xHHHH;`), encoding the resulting Unicode codepoints into UTF-8. `<![CDATA[ ... ]]>` blocks inside `<style>` elements are extracted verbatim.

---

## 3. Color Model, CSS Cascade & Dynamic Theming

### 3.1 Color Parsing (`parse_color`)
`parse_color` supports all W3C SVG / CSS Color Module Level 4 syntax forms:
1. **Hexadecimal Notation:**
   - `#RGB` (4-bit nibbles expanded via $c \times 17$)
   - `#RGBA` (4-bit nibbles with alpha)
   - `#RRGGBB` (8-bit channels)
   - `#RRGGBBAA` (8-bit channels with alpha)
2. **Functional RGB (`rgb()` / `rgba()`):**
   - Supports comma-separated (`rgb(255, 128, 0)`) and CSS Color 4 space/slash-separated (`rgb(255 128 0 / 50%)`) syntax.
   - Accepts integer $[0, 255]$ and percentage $[0\%, 100\%]$ channel values, plus float $[0.0, 1.0]$ or percentage alpha.
3. **Functional HSL (`hsl()` / `hsla()`):**
   - Converts hue $H \in (-\infty, \infty)$ (wrapped to $[0^\circ, 360^\circ)$), saturation $S \in [0\%, 100\%]$, lightness $L \in [0\%, 100\%]$, and optional alpha $A$ to sRGB via `hsl_to_rgb`.
4. **W3C Named Colors (`W3C_NAMED_COLORS`):**
   - Compile-time sorted table of all 148 standard W3C SVG/CSS named colors (plus `transparent` and `none`), queried via $O(\log N)$ binary search (`std::lower_bound`).

### 3.2 Dynamic Keywords (`currentColor` & `var(--*)`) and `SvgTheme`
- **`currentColor`:** Inherits the computed `color` property (`SvgPaintStyle::current_color`) down the element hierarchy.
- **CSS Custom Properties (`var(--name, fallback)`):**
  - Custom properties declared inside `:root` or `svg` rules in `<style>` blocks populate `SvgDocument::root_css_variables()`.
  - `parse_paint_property` records the variable identifier (`fill_css_var` / `stroke_css_var`) and optional fallback color (`fill_fallback` / `stroke_fallback`).
- **Runtime Theme Injection (`SvgTheme`):**
  ```cpp
  struct SvgTheme {
      std::optional<Color> current_color;
      std::optional<Color> tint_color;
      std::unordered_map<std::string, Color> css_variables;
      std::unordered_map<std::string, Color> id_color_overrides;
  };
  ```
  `resolve_effective_color` resolves colors at render time in priority order:
  1. `theme.tint_color` (global monochrome tint override, preserving original alpha)
  2. `theme.id_color_overrides[elem.id]` (per-element ID color override)
  3. `theme.css_variables[css_var_name]` $\to$ `doc.root_css_variables()[css_var_name]` $\to$ `fallback_col`
  4. `theme.current_color` $\to$ `elem.style.current_color` (when `is_current_color == true`)
  5. Element's parsed static `Color`

### 3.3 Internal `<style>` CSS Parser & Specificity Cascade
`parse_css_stylesheet` parses rules inside `<style>` elements into `SvgCssRule` records. Each rule's selector is assigned a specificity weight via `compute_selector_specificity`:
- ID selector (`#id`): $+100$
- Class selector (`.class`): $+10$
- Element type selector (`rect`, `path`, `circle`, etc.): $+1$
- Universal selector (`*`): $0$
- Comma-separated selector lists (`.a, .b, #c`) expand into independent rules.

For every element, styling is applied in four cascading stages:
1. Inherited parent scope `SvgPaintStyle` (for inheritable properties; non-inherited properties `opacity`, `clip_path_url`, `mask_url`, `filter_url`, and `blend_mode` reset to initial defaults).
2. XML presentation attributes (`fill="..."`, `stroke="..."`, etc.).
3. Matching `<style>` CSS rules sorted by ascending `specificity` via `std::stable_sort`.
4. Inline `style="..."` attribute declarations.

---

## 4. Document Structure, Viewports & Instancing

### 4.1 Viewport & Aspect Ratio (`SvgPreserveAspectRatio`)
`SvgPreserveAspectRatio::compute_transform(const Rect& view_box, const Rect& viewport)` computes the affine `Transform` mapping `viewBox` coordinates into `viewport`:
- **`SvgAspectAlign::None`:** Non-uniform scaling $s_x = \frac{W_{\text{vp}}}{W_{\text{vb}}}$, $s_y = \frac{H_{\text{vp}}}{H_{\text{vb}}}$.
- **Uniform Scaling (`XMinYMin` .. `XMaxYMax`):**
  - `SvgAspectMeetOrSlice::Meet`: $s = \min(s_x, s_y)$
  - `SvgAspectMeetOrSlice::Slice`: $s = \max(s_x, s_y)$ (combined with viewport rectangle clipping during rendering)
  - Alignment offsets $(\Delta x, \Delta y)$ position the scaled box at `Min` ($0$), `Mid` ($0.5 \times \text{spare}$), or `Max` ($1.0 \times \text{spare}$).

### 4.2 Hierarchical Groups (`<g>`), `<symbol>`, Nested `<svg>` & `<use>`
`SvgElement` represents nodes in the scene graph via `SvgElementType`:
- **`SvgElementType::Path`:** Geometric primitives (`<path>`, `<rect>`, `<circle>`, `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`) converted to `nisaba::Path`.
- **`SvgElementType::Group`:**
  - Simple passthrough `<g>` tags (no `id`, `opacity == 1.0`, no `clip-path`, `mask`, `filter`, or `mix-blend-mode`) flatten their children directly into the parent list with pre-concatenated transforms.
  - Groups requiring visual isolation (`opacity < 1.0`, `blend_mode != BlendMode::SourceOver`, `filter`, `clip-path`, `mask`, or referenced `id`) are preserved as hierarchical `SvgElementType::Group` nodes containing `std::vector<SvgElement> children`.
  - During rendering, isolated groups allocate an offscreen `Pixmap`, render all child elements at full opacity into the group surface, apply any attached `<filter>`, and composite the group surface onto the parent canvas once using `elem.style.opacity` and `elem.style.blend_mode`.
- **`<symbol>` & `<defs>`:**
  - Elements inside `<defs>` and `<symbol>` blocks are excluded from direct rendering and registered in `defined_elements_` by their `id`.
  - `<symbol>` stores its `viewBox` and `preserveAspectRatio` on the group definition.
- **`SvgElementType::Use` (`<use>`):**
  - Supports both single-element references and recursive multi-element `<g>` / `<symbol>` references with cycle protection (`use_depth <= 16`).
  - Applies `(use_x, use_y)` translation and maps `(use_width, use_height)` onto the referenced `<symbol>`'s `group_viewport`.
  - Propagates inherited `fill`, `stroke`, `stroke_width`, and `current_color` from the `<use>` element when not explicitly specified on the target definition.

---

## 5. Advanced Stroke Pipeline & Markers

### 5.1 Stroke Styling & Dashing
`SvgPaintStyle` configures the stroke pipeline:
- **Caps & Joins:** `stroke-linecap` (`butt`, `round`, `square`), `stroke-linejoin` (`miter`, `round`, `bevel`), and `stroke-miterlimit` (`float miter_limit`).
- **Dash Patterns (`stroke-dasharray` & `stroke-dashoffset`):**
  - Parses comma- or whitespace-separated dash intervals (`std::vector<float> dash_array`) or `none`.
  - Per W3C SVG specification, odd-length `dash_array` vectors are duplicated to form an even on/off cycle.
  - Converted to `StrokeDash` and evaluated via `nisaba::dash_path` prior to `Canvas::stroke_path`.
- **Non-Scaling Stroke (`vector-effect="non-scaling-stroke"`):**
  - Computes the determinant scale factor $s = \sqrt{|s_x s_y - k_x k_y|}$ of the active `Canvas` CTM and divides the stroke width by $s$ so the rendered pixel width remains invariant under viewport or group scaling.
- **Paint Order (`paint-order`):**
  - Configurable execution sequence `std::array<SvgPaintPhase, 3>` over `{Fill, Stroke, Markers}` (e.g., `paint-order="stroke fill markers"` renders the stroke behind the fill).

### 5.2 Vertex Markers (`<marker>`)
`SvgMarker` defines reusable arrowheads and vertex ornaments:
- **Attributes:** `viewBox`, `refX`, `refY`, `markerWidth`, `markerHeight`, `markerUnits` (`strokeWidth` vs. `userSpaceOnUse`), `preserveAspectRatio`, and `orient` (`auto`, `auto-start-reverse`, or explicit angle).
- **Tangent Computation (`SvgDocument::render_markers`):**
  - Iterates path segments (`MoveTo`, `LineTo`, `QuadTo`, `CubicTo`, `Close`) and extracts incoming $(\mathbf{t}_{\text{in}})$ and outgoing $(\mathbf{t}_{\text{out}})$ unit tangent vectors at every vertex.
  - At interior vertices (`marker-mid`), orientation bisects $\mathbf{t}_{\text{in}} + \mathbf{t}_{\text{out}}$.
  - At start vertices (`marker-start`), `orient="auto-start-reverse"` rotates the tangent by $+180^\circ$.

---

## 6. Paint Servers: Gradients & Patterns

### 6.1 Linear & Focal Radial Gradients (`SvgGradient`)
- **Units & Spread Modes:**
  - `gradientUnits`: `objectBoundingBox` (default) or `userSpaceOnUse`.
  - `spreadMethod`: `pad` (`SpreadMode::Pad`), `reflect` (`SpreadMode::Reflect`), or `repeat` (`SpreadMode::Repeat`).
  - `gradientTransform`: Arbitrary 2D affine matrix `Transform`.
- **Elliptical `objectBoundingBox` Mapping:**
  - When `gradientUnits == ObjectBoundingBox` and the target geometry bounding box is non-square ($W \ne H$) or `gradientTransform` is non-identity, coordinates are evaluated in normalized $[0, 1]^2$ space and mapped to user space via `Transform::from_row(bw, 0, 0, bh, bx, by).pre_concat(gradient_transform)`, producing true elliptical radial gradients on rectangular shapes.
- **Focal Point Radial Gradients (`fx`, `fy`, `fr`):**
  - When the focal point $(f_x, f_y)$ differs from $(c_x, c_y)$ or focal radius $f_r > 0$, `create_shader_from_gradient` constructs a two-point conical gradient shader via `Shader::create_two_point_conical(focal, fr, center, r, stops, spread_mode, grad_ts)`.
- **Multi-Hop Inheritance (`href` / `xlink:href`):**
  - Resolves chains of gradient references (up to depth 16), inheriting `stops`, `gradientUnits`, `spreadMethod`, `gradientTransform`, and coordinates (`x1..y2`, `cx..fr`) whenever not explicitly overridden on the child gradient.

### 6.2 Pattern Tiling (`SvgPattern`)
- `<pattern>` elements store child `SvgElement` nodes, tile dimensions (`x`, `y`, `width`, `height`), `patternUnits`, `patternContentUnits`, `patternTransform`, and optional `viewBox`.
- During rendering, `resolve_paint_server` allocates a tile `Pixmap`, renders the pattern's child elements into the tile using a temporary `Canvas`, and wraps the resulting bitmap in a repeating pattern shader (`Shader::from_pixmap(tile, TileMode::Repeat, FilterQuality::Bilinear, pattern_ts)`).

---

## 7. Clipping & Masking (`<clipPath>` & `<mask>`)

### 7.1 Vector Clipping (`SvgClipPath`)
- Aggregates child geometry into `combined_path` with local transforms applied.
- Supports `clipPathUnits="userSpaceOnUse"` and `clipPathUnits="objectBoundingBox"` (scaling the normalized $[0, 1]$ clip path by the target element's local bounding box before intersecting with the canvas clip stack).
- Supports `clip-rule="nonzero"` (`FillRule::Winding`) and `clip-rule="evenodd"` (`FillRule::EvenOdd`).

### 7.2 Luminance & Alpha Masking (`SvgMask`)
- Renders mask child elements into an offscreen `Pixmap` of canvas dimensions, inheriting the active `Canvas` CTM (`mask_canvas.set_transform(canvas.transform())`) so mask coordinates align with transformed elements and `viewBox` scaling.
- Supports `maskContentUnits="objectBoundingBox"` and `mask-type`:
  - `MaskType::Luminance` (default): Computes BT.709 luminance $Y = 0.2126 R + 0.7152 G + 0.0722 B$ multiplied by alpha.
  - `MaskType::Alpha` (`mask-type="alpha"`): Uses the mask pixel's alpha channel directly.

---

## 8. Vector Typography (`<text>`, `<tspan>`, `<textPath>`)

`SvgDocument::build_text_path` converts `<text>` elements into vector outlines using `nisaba::text`:
- **Supported Attributes:**
  - Positioning: `x`, `y`, `dx`, `dy`, `rotate` on both `<text>` and nested `<tspan>` elements.
  - Font Selection: `font-family` (`Family`), `font-size`, `font-weight` (`normal`, `bold`, `100`–`900`), `font-style` (`normal`, `italic`, `oblique`), `letter-spacing`.
  - Alignment: `text-anchor` (`start`, `middle`, `end`) and `dominant-baseline` / `alignment-baseline` (`auto`, `middle`, `central`, `hanging`, `text-before-edge`).
- **Shaping & Outline Generation:**
  - Segments bidirectional runs via `text::Bidi::segment_runs` and applies contextual Arabic/RTL shaping via `text::Bidi::shape_text`.
  - Matches fonts through `text::FontSystem::match_font` with per-codepoint fallback via `find_glyph_or_fallback`.
  - Extracts glyph outlines via `TtfFont::get_glyph_path_styled` (including synthetic bold and italic when required).
- **Text on Path (`<textPath href="#id" startOffset="...">`):**
  - Resolves the referenced `<path>` from `defined_elements_` and measures arc length via `nisaba::PathMeasure`.
  - For each glyph at arc distance $d = \text{start\_offset} + x_{\text{pen}} + \frac{1}{2}\text{advance}$, queries `PathMeasure::get_pos_tan(d, pos, tan)` and transforms the glyph outline by translating to `pos` and rotating by $\text{atan2}(\text{tan}.y, \text{tan}.x)$.

---

## 9. Filter Graph Pipeline (`<filter>`) & Blend Modes

### 9.1 Filter Graph Execution (`execute_filter_graph`)
`SvgFilter` stores an ordered DAG of `SvgFilterPrimitive` nodes operating on named intermediate `Pixmap` buffers (`in`, `in2`, `result`), with built-in sources `SourceGraphic` and `SourceAlpha`:

| Filter Primitive | Implementation |
| :--- | :--- |
| `feGaussianBlur` | Applies separable SIMD box/Gaussian blur (`effects::gaussian_blur_pixmap`) with `stdDeviation` $(\sigma_x, \sigma_y)$. |
| `feOffset` | Translates input buffer by $(dx, dy)$ pixels. |
| `feFlood` | Fills target buffer with `flood-color` modulated by `flood-opacity`. |
| `feColorMatrix` | Supports `type="matrix"` (20-float $4 \times 5$ matrix), `type="saturate"`, `type="hueRotate"`, and `type="luminanceToAlpha"` via `effects::ColorMatrix`. |
| `feDropShadow` | Generates alpha shadow from input, tints with `flood-color`/`flood-opacity`, blurs by `stdDeviation`, offsets by $(dx, dy)$, and composites original graphic on top. |
| `feBlend` | Composites `in` and `in2` using `mode` (`normal`, `multiply`, `screen`, `overlay`, `darken`, `lighten`, `color-dodge`, `color-burn`, `hard-light`, `soft-light`, `difference`, `exclusion`). |
| `feComposite` | Supports Porter-Duff operators `over`, `in`, `out`, `atop`, `xor`, and `operator="arithmetic"` ($R = k_1 \cdot i_1 \cdot i_2 + k_2 \cdot i_1 + k_3 \cdot i_2 + k_4$). |
| `feMerge` / `feMergeNode` | Sequentially composites arbitrary named intermediate buffers (`<feMergeNode in="..." />`) in document order. |

### 9.2 CSS Blend Modes (`mix-blend-mode`)
Elements and groups support `mix-blend-mode` (`multiply`, `screen`, `overlay`, `darken`, `lighten`, `color-dodge`, `color-burn`, `hard-light`, `soft-light`, `difference`, `exclusion`, `hue`, `saturation`, `color`, `luminosity`, `plus-lighter`), mapped to `nisaba::BlendMode`.

---

## 10. GPU Rendering, Caching & DOM Introspection

### 10.1 Hardware GPU Rendering (`SvgDocument::render_gpu`)
`SvgDocument::render_gpu(gpu::GpuCanvas& canvas, const Rect* target_bounds, const SvgTheme& theme)` provides rendering parity on `GpuCanvas`:
- Maps `preserveAspectRatio` viewBox transforms and hardware stencil `clip_path` (`FillRule::Winding` / `FillRule::EvenOdd`).
- Converts `<text>` elements to vector paths via `build_text_path` and renders fills, gradients (`create_shader_from_gradient`), dashed strokes (`dash_path`), and `non-scaling-stroke` directly through `GpuCanvas::fill_path` and `GpuCanvas::stroke_path`.
- Automatically falls back to an offscreen CPU layer upload (`GpuCanvas::draw_pixmap`) for subtrees utilizing complex pixel filter graphs (`<filter>`) or luminance masks (`<mask>`).

### 10.2 Pre-Rasterized Baking (`BakedSvg`) & LRU Caching (`SvgCache`)
- **`BakedSvg::bake`:** Rasterizes an `SvgDocument` at target dimensions $(W, H)$ with an optional `Color` tint or `SvgTheme` into an owned `Pixmap`. `BakedSvg::draw` blits the pre-multiplied pixels onto a destination `Canvas` using SIMD scanline operations when unscaled, or bilinear sampling when scaled.
- **`SvgCache`:** Maintains an $O(1)$ hash-indexed LRU cache (`std::unordered_map<SvgCacheKey, ...>` + `std::list<SvgCacheKey>`). `SvgCache::hash_theme` hashes `tint_color`, `current_color`, `css_variables`, and `id_color_overrides` into `SvgCacheKey::tint_rgba`, ensuring distinct cache entries per theme configuration.

### 10.3 Vector Hit-Testing & Bounding Box Queries
- **`SvgDocument::hit_test(Point pt, const Rect* target_bounds)`:**
  - Maps `pt` through the inverse `preserveAspectRatio` transform into `viewBox` space, then traverses the scene graph in reverse paint order (topmost element first).
  - Applies inverse element/group transforms (`Transform::invert`) at each node.
  - Tests filled geometry using analytical ray-casting across linear, quadratic, and cubic Bézier segments (`point_in_path_geometry`) respecting `FillRule::Winding` and `FillRule::EvenOdd`.
  - Tests stroked geometry by expanding the stroke envelope via `Path::stroke_to_fill`.
  - Returns the `id` of the topmost hit element or enclosing group (`std::optional<std::string>`).
- **`SvgDocument::get_element_bounds(std::string_view id)`:**
  - Queries `defined_elements_` by `id` and computes the transformed axis-aligned `Rect` bounding box in user coordinates.

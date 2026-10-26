# Nisaba Lottie Subsystem (`nisaba::lottie`) — Technical Reference

## 1. Architecture Overview

`nisaba::lottie` is the vector animation subsystem of the Nisaba engine. It parses, evaluates, and renders Bodymovin JSON (`.json`) and dotLottie (`.lottie` ZIP archive) documents directly onto any `nisaba::ICanvas` target (`nisaba::Canvas` for CPU rasterization and `nisaba::gpu::GpuCanvas` for OpenGL ES 3.2 / Vulkan hardware acceleration).

### Subsystem Structure

| Header / Source | Responsibility |
| :--- | :--- |
| `include/nisaba/json/json.hpp`<br>`src/json/json.cpp` | Recursive-descent JSON parser (`nisaba::json::JsonValue`) using contiguous `std::vector<std::pair<std::string, JsonValue>>` object storage, `std::from_chars` numeric parsing, and UTF-16 surrogate pair decoding. |
| `include/nisaba/lottie/lottie_types.hpp` | Enumerations (`LayerType`, `MatteType`, `MaskMode`, `FillRule`, `LineCap`, `LineJoin`, `TrimType`, `GradientType`, `MergeMode`, `CompositeOrder`, `LoopMode`, `EffectType`, `TextJustify`) and blend mode mapping (`map_lottie_blend_mode`). |
| `include/nisaba/lottie/property.hpp`<br>`src/lottie/property.cpp` | Generic animatable `Property<T>`, temporal cubic Bezier easing solver, spatial cubic Bezier evaluator with arc-length reparameterization, expression evaluator (`LottieExpression`), `BezierData` morphing, and 2D/3D `LottieTransform`. |
| `include/nisaba/lottie/shape.hpp`<br>`src/lottie/shape.cpp` | Vector geometry generators (`sh`, `rc`, `el`, `sr`), painting styles (`fl`, `st`, `gf`, `gs`), path modifiers (`tm`, `mm`, `rd`, `op`, `pb`, `zz`, `rp`), `ShapeGroup` (`gr`), and `RenderContext`. |
| `include/nisaba/lottie/layer.hpp`<br>`src/lottie/layer.cpp` | Layer hierarchy (`PrecompLayer`, `SolidLayer`, `ImageLayer`, `NullLayer`, `ShapeLayer`, `TextLayer`), vector/feathered `Mask`, `Track Mattes`, `LayerEffect`, `TextDocument` & `TextAnimator`, and 3D perspective compositing. |
| `include/nisaba/lottie/animation.hpp`<br>`src/lottie/animation.cpp` | Root `Animation` document, `.lottie` ZIP container extraction, Base64 data-URI decoding, asset registry (`LottieAssetRegistry`), dynamic keypath/slot overrides, and coordinate hit-testing. |
| `include/nisaba/lottie/player.hpp`<br>`src/lottie/player.cpp` | Playback state machine (`Player`), frame/progress seeking, marker segment playback, loop modes (`Once`, `Loop`, `Bounce`), and lifecycle callbacks. |

---

## 2. Document Ingestion & Container Formats

### 2.1 JSON Format (`.json`)
- Parsed via `nisaba::json::JsonValue::parse(std::string_view)`.
- Supports standard RFC 8259 JSON, single-line (`//`) and block (`/* ... */`) comments, and `\uXXXX` escape sequences (including UTF-16 high/low surrogate pairs converted to UTF-8).
- Object members are stored in a flat `std::vector<std::pair<std::string, JsonValue>>` for cache-local linear lookup on small key sets typical of Lottie nodes (`"ty"`, `"k"`, `"a"`, `"p"`, `"s"`, `"o"`).

### 2.2 dotLottie Container Format (`.lottie`)
- `Animation::load_from_bytes` and `Animation::load_from_file` automatically detect the ZIP local file header / EOCD signature (`PK\x03\x04` / `0x06054b50`).
- Decompresses entries stored with method `0` (Store) or method `8` (DEFLATE) using `nisaba::image::deflate_decompress`.
- Parses `manifest.json` to resolve the primary animation path (`animations/<id>.json`) or falls back to the first `.json` animation payload in the archive.
- Resolves packaged image assets (`images/*`) directly from the decompressed archive buffers into `nisaba::Pixmap`.

### 2.3 Embedded & External Assets
- **Precompositions (`"assets"` with `"layers"`):** Parsed into `LottieAssetRegistry::precomps` with pre-linked parent and matte pointers.
- **Images (`"assets"` with `"p"`):**
  - Inline RFC 2397 data URIs (`data:image/png;base64,...`, `data:image/jpeg;base64,...`, `data:image/webp;base64,...`) are decoded in-memory and loaded via `Pixmap::load_from_memory`.
  - External files are resolved relative to `base_dir` + `"u"` + `"p"` via `Pixmap::load_file`.
- **Fonts (`"fonts.list"`) & Pre-baked Characters (`"chars"`):**
  - Font descriptors (`fName`, `fFamily`, `fStyle`, `fPath`, `ascent`) are registered in `LottieAssetRegistry::fonts` and optionally loaded into `nisaba::text::FontSystem` (supporting file paths and `data:` base64 font streams).
  - Pre-baked vector glyph outlines (`"chars"`) are parsed as `ShapeGroup` trees indexed by character cluster, family, and style.

---

## 3. Property System, Interpolation & Expressions

### 3.1 `Property<T>` Evaluation
`Property<T>` supports the following value types:
- `FloatProperty` (`Property<float>`)
- `PointProperty` (`Property<Point>`)
- `ColorProperty` (`Property<Color>`)
- `ShapeProperty` (`Property<BezierData>`)
- `FloatArrayProperty` (`Property<std::vector<float>>`)

Keyframes (`Keyframe<T>`) are evaluated in O(1) amortized time during sequential playback using a mutable cursor hint (`last_kf_hint_`), falling back to `std::upper_bound` binary search on non-sequential seeks.

### 3.2 Temporal Cubic Bezier Easing
- Evaluates 1D/multi-dimensional timing curves defined by `"o"` (`out_handle`) and `"i"` (`in_handle`) in `[0, 1]` normalized time.
- `solve_bezier_easing(x, out_h, in_h)` solves $B_x(t) = x$ using 5 iterations of Newton-Raphson refinement (with bisection fallback when $\lvert dB_x/dt \rvert < 10^{-5}$), then evaluates $B_y(t)$.
- Supports discrete step keyframes (`"h": 1` / `is_hold = true`).

### 3.3 Spatial Cubic Bezier Motion Paths
- Point keyframes containing `"to"` (`spatial_out_tangent`) and `"ti"` (`spatial_in_tangent`) are evaluated via `eval_spatial_bezier`.
- Computes a 16-segment arc-length lookup table (LUT) along the cubic curve $\mathbf{P}(u)$ and maps the eased temporal parameter $t \in [0, 1]$ to uniform arc distance $s = t \cdot L_{\text{total}}$ before evaluating position and analytical tangent $\mathbf{P}'(u)$.

### 3.4 Shape Morphing (`BezierData`)
- Interpolates vertex positions (`"v"`), incoming tangents (`"i"`), and outgoing tangents (`"o"`) between keyframes.
- `BezierData::to_path(bool reverse_direction)` constructs a `nisaba::Path` using `MoveTo`, `LineTo` (when both adjacent tangents are zero), and `CubicTo` segments, supporting counter-clockwise winding reversal when `"d": 3`.

### 3.5 Expressions (`"x"`)
`LottieExpression::parse` recognizes and evaluates common After Effects expressions without an external JavaScript engine:
- `loopOut("cycle")`, `loopOut("pingpong")`, `loopOut("offset")`
- `loopIn("cycle")`, `loopIn("pingpong")`
- `wiggle(freq, amp)` (continuous coherent trigonometric noise over frame time)
- Inertial bounce expressions (`amp`, `freq`, `decay` damped harmonic oscillation after the final keyframe)

---

## 4. Transforms (2D & 3D)

### 4.1 2D Affine Transform (`LottieTransform::matrix`)
Constructs a 2D `nisaba::Transform` at frame $t$ in standard After Effects composition order:
1. Anchor point translation: $T(-a_x, -a_y)$
2. Scale: $S(s_x / 100, s_y / 100)$
3. Skew (`"sk"`, `"sa"`): $R(-\text{sa}) \cdot \text{Skew}(\tan(-\text{sk})) \cdot R(\text{sa})$
4. Rotation (`"r"` + optional `auto_orient` tangent angle $\text{atan2}(d_y, d_x)$): $R(\theta)$
5. Position translation (`"p"`, or split `"px"` / `"py"` when `"s": true`): $T(p_x, p_y)$

### 4.2 3D Transform (`LottieTransform::matrix_3d`)
When `"ddd": 1` is set on a layer or its ancestors:
- Evaluates 3D anchor $(a_x, a_y, a_z)$, 3D scale $(s_x, s_y, s_z)$, 3D orientation (`"or"`), and Euler axis rotations (`"rx"`, `"ry"`, `"rz"`), producing a `nisaba::Transform4x4`.
- Applies a centered perspective projection matrix (`Transform4x4::from_perspective`) and composites the layer via `ICanvas::draw_pixmap_3d`.

---

## 5. Vector Shapes, Styles & Path Modifiers

All shape items inherit from `LottieShapeItem` and support visibility (`"hd"`), winding direction (`"d": 1` CW, `"d": 3` CCW), and per-item `BlendMode` (`"bm"`). Unanimated geometry primitives cache their constructed `nisaba::Path` at parse time (`cached_static_path`).

### 5.1 Geometry Primitives
- **`PathShape` (`"sh"`):** Freeform Bezier path (`"ks"`).
- **`RectShape` (`"rc"`):** Parametric rectangle (`"p"`, `"s"`) with clamped cubic-arc corner roundness (`"r"`).
- **`EllipseShape` (`"el"`):** Parametric 4-cubic-segment ellipse (`"p"`, `"s"`).
- **`StarShape` (`"sr"`):** Polystar (`"sy": 1`) and Polygon (`"sy": 2`) with point count (`"pt"`), inner/outer radii (`"ir"`, `"or"`), rotation (`"r"`), and perpendicular cubic tangents for inner/outer roundness (`"is"`, `"os"`).

### 5.2 Painting Styles
- **`FillShape` (`"fl"`):** Solid color fill (`"c"`), opacity (`"o"`), and fill rule (`"r": 1` NonZero/Winding, `"r": 2` EvenOdd).
- **`StrokeShape` (`"st"`):** Solid color stroke (`"c"`), opacity (`"o"`), width (`"w"`), line cap (`"lc": 1` Butt, `2` Round, `3` Square), line join (`"lj": 1` Miter, `2` Round, `3` Bevel), miter limit (`"ml"`), and animated dash array (`"d"` with `"d"`, `"g"`, `"o"`).
- **`GradientFillShape` (`"gf"`) & `GradientStrokeShape` (`"gs"`):**
  - Parses `"g"` (`GradientColorStops`) with color stop count `"p"` and flat array `"k"` containing $[t_0, r_0, g_0, b_0, \dots]$ followed by opacity stops $[t_0, a_0, \dots]$.
  - Merges and cross-interpolates RGB and alpha stops at all distinct stop offsets.
  - Supports `GradientType::Linear` (`"t": 1` via `nisaba::LinearGradient`), concentric `GradientType::Radial` (`"t": 2` via `nisaba::RadialGradient`), and focal radial gradients (when highlight length `"h"` $\neq 0$, using `nisaba::Shader::create_two_point_conical` with highlight angle `"a"`).

### 5.3 Path Modifiers
Modifiers operate sequentially on the active `std::vector<Path>` within a `ShapeGroup`:
- **`TrimPathShape` (`"tm"`):** Exact arc-length path slicing via `nisaba::PathMeasure`. Supports `"s"` (start %), `"e"` (end %), `"o"` (offset degrees), `TrimType::Simultaneously` (`"m": 1`, trims each contour independently in normalized $[0, 1]$ space), and `TrimType::Individually` (`"m": 2`, trims across the cumulative arc length of all paths in the group).
- **`MergePathsShape` (`"mm"`):** Boolean path operations wired to `nisaba::path_ops`:
  - `1` (`Merge`): Path concatenation
  - `2` (`Add`): `PathOp::Union`
  - `3` (`Subtract`): `PathOp::Difference`
  - `4` (`Intersect`): `PathOp::Intersect`
  - `5` (`ExcludeIntersections`): `PathOp::Xor`
- **`RoundCornersShape` (`"rd"`):** Replaces sharp polygon vertices with cubic Bezier fillets of radius `"r"`, clamped to half the adjacent edge lengths.
- **`OffsetPathShape` (`"op"`):** Offsets path contours outward ($\text{amount} > 0$, union with stroked outline of width $2 \cdot \lvert\text{amount}\rvert$) or inward ($\text{amount} < 0$, difference with stroked outline) with configurable `LineJoin` (`"lj"`) and miter limit (`"ml"`).
- **`PuckerBloatShape` (`"pb"`):** Pulls vertices toward the path bounding-box center while pushing cubic/quadratic control points outward (or vice versa) by percentage `"a"`.
- **`ZigZagShape` (`"zz"`):** Samples contours at uniform arc-length intervals via `PathMeasure::get_pos_tan` and displaces points along the unit normal by amplitude `"s"` and ridge frequency `"r"` (supporting corner `"pt": 1` and smooth cubic wave `"pt": 2`).
- **`RepeaterShape` (`"rp"`):** Duplicates preceding group items `"c"` times with offset `"o"`, cumulative transform matrix $T^i$ (`anchor`, `position`, `scale`, `rotation`), interpolated instance opacity between `"so"` and `"eo"`, and composite order (`"m": 1` Above, `"m": 2` Below).

---

## 6. Layer Hierarchy, Compositing & Effects

### 6.1 Layer Types
| `"ty"` | Class | Description |
| :---: | :--- | :--- |
| `0` | `PrecompLayer` | References a composition in `LottieAssetRegistry::precomps` by `"refId"`, supporting clipping bounds (`"w"`, `"h"`), time stretch (`"sr"`), start time (`"st"`), and time remapping (`"tm"`). |
| `1` | `SolidLayer` | Rectangular solid color layer (`"sw"`, `"sh"`, hex color `"sc"`). |
| `2` | `ImageLayer` | Renders a decoded `LottieImageAsset` (`Pixmap`) referenced by `"refId"` with bilinear filtering. |
| `3` | `NullLayer` | Non-rendering transform parent controller. |
| `4` | `ShapeLayer` | Evaluates and renders a tree of `LottieShapeItem` nodes. |
| `5` | `TextLayer` | Evaluates `TextDocument` keyframes (`"t.d"`), BiDi/Arabic shaping, and `TextAnimator` range selectors (`"t.a"`). |

### 6.2 Pre-Linked Hierarchy
At document load time, `link_layer_hierarchy` resolves:
- `parent_ptr`: Direct pointer to the parent layer (`"parent"` index), enabling O(1) transform chain traversal without per-frame hash map allocations.
- `matte_layer_ptr`: Direct pointer to the track matte source layer (`"tp"` index, or the preceding layer $i - 1$ in the layer stack).

### 6.3 Vector & Feathered Masks (`"masksProperties"`)
- Supports `MaskMode::Add` (`"a"`), `Subtract` (`"s"`), `Intersect` (`"i"`), `Difference` (`"f"`), and `None` (`"n"`).
- Supports inverted masks (`"inv": true`), mask expansion (`"x"` via stroke union/difference), and 2D feathering (`"f"` via `nisaba::effects::gaussian_blur_mask`).
- Pure vector masks use direct `ICanvas::clip_path`; feathered masks composite through an 8-bit `nisaba::Mask`.

### 6.4 Track Mattes (`"tt"` / `"td"` / `"tp"`)
- Renders the target layer and the matte source layer into offscreen `Pixmap` surfaces.
- Converts the matte surface into an 8-bit coverage mask via `nisaba::Mask::from_pixmap`:
  - `MatteType::Alpha` (`1`): `MaskType::Alpha`
  - `MatteType::InvertedAlpha` (`2`): `MaskType::Alpha` + `Mask::invert()`
  - `MatteType::Luma` (`3`): `MaskType::Luminance` (BT.709 luminance)
  - `MatteType::InvertedLuma` (`4`): `MaskType::Luminance` + `Mask::invert()`
- Modulates the target layer's premultiplied RGBA pixels by the matte coverage before compositing onto the destination canvas.

### 6.5 Layer Effects (`"ef"`)
Processed on the layer's offscreen surface using `nisaba::effects`:
- **`GaussianBlur` (`ty = 29`):** 3-pass box/Gaussian blur via `effects::gaussian_blur_pixmap`.
- **`DropShadow` (`ty = 25`):** Extracts alpha mask, blurs by softness (`sigma`), offsets by distance and angle, tints with shadow color/opacity via `effects::draw_shadow_mask`, and composites the original layer over the shadow.
- **`Fill` (`ty = 21`):** Recolors RGB channels while preserving alpha multiplied by fill opacity via `effects::ColorMatrix`.
- **`Tint` (`ty = 20`) & `Tritone` (`ty = 23`):** Maps BT.709 luminance ($Y = 0.2126 R + 0.7152 G + 0.0722 B$) between shadow/black and highlight/white colors blended by amount `%` via `effects::ColorMatrix`.

### 6.6 Typography & Text Animators (`TextLayer`)
- **Text Document (`TextDocument`):** Supports multi-line text (`\r` / `\n`), font family/name (`"f"`), font size (`"s"`), line height (`"lh"`), tracking (`"tr"`), baseline shift (`"ls"`), justification (`"j": 0` Left, `1` Right, `2` Center), fill color (`"fc"`), stroke color (`"sc"`), stroke width (`"sw"`), and stroke-over-fill ordering (`"of"`).
- **BiDi & Arabic Shaping:** Segments lines into directional runs via `nisaba::text::Bidi::segment_runs` and performs contextual Arabic glyph joining (Isolated, Initial, Medial, Final, and Lam-Alif ligatures) via `Bidi::shape_text`.
- **Glyph Outline Resolution:** Renders pre-baked Lottie `"chars"` shapes when present, or extracts TrueType/CFF outlines from `nisaba::text::TtfFont::get_glyph_path`.
- **Range Selectors (`TextAnimator`):** Evaluates per-character coverage factors in percentage (`"r": 1`) or index (`"r": 2`) units with `start` (`"s"`), `end` (`"e"`), `offset` (`"o"`), and selector shapes (`Square`, `Ramp Up`, `Ramp Down`, `Triangle`, `Round`, `Smooth`), modulating per-glyph position, scale, rotation, opacity, fill/stroke color, stroke width, and tracking.

---

## 7. Runtime Interactivity, Overrides & Playback API

### 7.1 Dynamic Property Overrides & Slots
`Animation` and `Player` provide runtime mutation methods without re-parsing the JSON document:
- `set_color_override(keypath, Color)`: Overrides `FillShape`, `StrokeShape`, or `SolidLayer` colors matching dot-separated keypaths (e.g., `"LayerName.GroupName.FillName"`, `"*.Fill 1"`, or `"*"`).
- `set_scalar_override(keypath, float)`: Overrides layer/group/fill opacity, stroke width, or corner roundness.
- `set_point_override(keypath, Point)`: Overrides layer/group position or rectangle/ellipse size.
- `set_text_override(layer_name, std::string)`: Replaces the active text string on matching `TextLayer` instances.
- `set_slot_color(slot_id, Color)`: Updates all properties tagged with a Lottie Slot ID (`"sid"`).
- `clear_overrides()`: Restores all overridden properties to their original keyframe/static state.

### 7.2 Hit-Testing
- `Animation::hit_test(float x, float y, float frame)` and `Player::hit_test(float x, float y)` traverse active layers from top to bottom (index `0` to `N - 1`), invert each layer's global transform into local coordinates, and perform winding/ray-casting intersection tests against shape paths, solid bounds, image bounds, and text bounds, returning the name of the hit layer or shape.

### 7.3 Code Usage Example

```cpp
#include "nisaba/lottie/lottie.hpp"
#include "nisaba/canvas/canvas.hpp"
#include "nisaba/canvas/pixmap.hpp"

using namespace nisaba;

void render_lottie_example() {
    // 1. Load from .json or .lottie (ZIP) file
    auto anim = lottie::Animation::load_from_file("assets/animation.lottie");
    if (!anim) return;

    // 2. Apply live theme overrides or slot colors
    anim->set_slot_color("primary_accent", Color::from_rgba8(0, 180, 255, 255));
    anim->set_color_override("IconLayer.MainGroup.Fill 1", Color::from_rgba8(255, 80, 120, 255));
    anim->set_text_override("TitleLayer", "مرحبا بك في نيسابا");

    // 3. Configure Player state machine
    lottie::Player player(anim);
    player.set_loop_mode(lottie::LoopMode::Bounce);
    player.play_marker("intro_segment");

    // 4. Advance and render onto CPU Canvas or GPU GpuCanvas
    auto surface = Pixmap::create(512, 512);
    Canvas canvas(*surface);

    player.advance(1.0f / 60.0f);
    if (auto dest = Rect::from_xywh(0.0f, 0.0f, 512.0f, 512.0f)) {
        player.render(canvas, *dest, /*preserve_aspect_ratio=*/true);
    }

    // 5. Perform coordinate hit-testing
    if (auto hit_name = player.hit_test(256.0f, 256.0f)) {
        // Interacted with layer/shape: *hit_name
    }
}
```

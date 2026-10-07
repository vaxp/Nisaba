# Nisaba Automated Unit Tests
# tests/meson.build

# Math unit tests
test_math_exe = executable('test_math',
  'test_math.cpp',
  dependencies : nisaba_dep
)
test('math_test', test_math_exe)

# Path unit tests
test_path_exe = executable('test_path',
  'test_path.cpp',
  dependencies : nisaba_dep
)
test('path_test', test_path_exe)

# PathMeasure arc-length & parameterization unit tests
test_path_measure_exe = executable('test_path_measure',
  'test_path_measure.cpp',
  dependencies : nisaba_dep
)
test('path_measure_test', test_path_measure_exe)

# PathOps boolean operations unit tests
test_path_ops_exe = executable('test_path_ops',
  'test_path_ops.cpp',
  dependencies : nisaba_dep
)
test('path_ops_test', test_path_ops_exe)

# Stroke & Dash unit tests
test_stroke_exe = executable('test_stroke',
  'test_stroke.cpp',
  dependencies : nisaba_dep
)
test('stroke_test', test_stroke_exe)

# Stroke to Fill (Path Outlining) unit tests
test_stroke_to_fill_exe = executable('test_stroke_to_fill',
  'test_stroke_to_fill.cpp',
  dependencies : nisaba_dep
)
test('stroke_to_fill_test', test_stroke_to_fill_exe)

# Color & Fixed-point unit tests
test_color_exe = executable('test_color',
  'test_color.cpp',
  dependencies : nisaba_dep
)
test('color_test', test_color_exe)

# Pixmap unit tests
test_pixmap_exe = executable('test_pixmap',
  'test_pixmap.cpp',
  dependencies : nisaba_dep
)
test('pixmap_test', test_pixmap_exe)

# Diverse Pixel Formats (RGBA8888, BGRA8888, RGB565, Alpha8) unit tests
test_pixmap_formats_exe = executable('test_pixmap_formats',
  'test_pixmap_formats.cpp',
  dependencies : nisaba_dep
)
test('pixmap_formats_test', test_pixmap_formats_exe)

# Scanline rasterizer unit tests
test_raster_exe = executable('test_raster',
  'test_raster.cpp',
  dependencies : nisaba_dep
)
test('raster_test', test_raster_exe)

# Canvas, Shaders & Pipeline unit tests
test_canvas_exe = executable('test_canvas',
  'test_canvas.cpp',
  dependencies : nisaba_dep
)
test('canvas_test', test_canvas_exe)

# Advanced features tests (Zero-copy stride, SIMD accuracy, Clip path, Text/Glyphs)
test_advanced_exe = executable('test_advanced',
  'test_advanced.cpp',
  dependencies : nisaba_dep
)
test('advanced_test', test_advanced_exe)

# Sovereign Text Engine tests
test_text_exe = executable('test_text',
  'test_text.cpp',
  dependencies : nisaba_dep
)
test('text_test', test_text_exe)

# PostScript / CFF & Advanced OpenType (GSUB/GPOS) tests
test_cff_opentype_exe = executable('test_cff_opentype',
  'test_cff_opentype.cpp',
  dependencies : nisaba_dep
)
test('cff_opentype_test', test_cff_opentype_exe)

# Linear sRGB & Gamma-Correct Blending tests
test_linear_blending_exe = executable('test_linear_blending',
  'test_linear_blending.cpp',
  dependencies : nisaba_dep
)
test('linear_blending_test', test_linear_blending_exe)

# Advanced Shaders (Compose/Blend, TwoPointConicalGradient) & ColorMatrix Filters tests
test_shaders_advanced_exe = executable('test_shaders_advanced',
  'test_shaders_advanced.cpp',
  dependencies : nisaba_dep
)
test('shaders_advanced_test', test_shaders_advanced_exe)

# Visual Effects, Blur & Shadows tests
test_effects_exe = executable('test_effects',
  'test_effects.cpp',
  dependencies : nisaba_dep
)
test('effects_test', test_effects_exe)

# Sovereign SVG Renderer tests
test_svg_exe = executable('test_svg',
  'test_svg.cpp',
  dependencies : nisaba_dep
)
test('svg_test', test_svg_exe)

# SVG Completeness tests (Gradients, ClipPath, Mask, Defs, Use, Image)
test_svg_completeness_exe = executable('test_svg_completeness',
  'test_svg_completeness.cpp',
  dependencies : nisaba_dep
)
test('svg_completeness_test', test_svg_completeness_exe)

# 3D Perspective & Spatial Transform tests
test_perspective_exe = executable('test_perspective',
  'test_perspective.cpp',
  dependencies : nisaba_dep
)
test('perspective_test', test_perspective_exe)

# 2D Vertex Mesh & Freeform Gradient Mesh tests
test_mesh_exe = executable('test_mesh',
  'test_mesh.cpp',
  dependencies : nisaba_dep
)
test('mesh_test', test_mesh_exe)

# Sovereign Deflate/Inflate & Checksum tests
test_deflate_exe = executable('test_deflate',
  'test_deflate.cpp',
  dependencies : nisaba_dep
)
test('deflate_test', test_deflate_exe)

# Sovereign PNG Codec tests
test_png_exe = executable('test_png',
  'test_png.cpp',
  dependencies : nisaba_dep
)
test('png_test', test_png_exe)

# Sovereign JPEG Codec tests
test_jpeg_exe = executable('test_jpeg',
  'test_jpeg.cpp',
  dependencies : nisaba_dep
)
test('jpeg_test', test_jpeg_exe)

# Sovereign Image I/O & Auto-detection tests
test_image_io_exe = executable('test_image_io',
  'test_image_io.cpp',
  dependencies : nisaba_dep
)
test('image_io_test', test_image_io_exe)

# Sovereign QOI Codec tests
test_qoi_exe = executable('test_qoi',
  'test_qoi.cpp',
  dependencies : nisaba_dep
)
test('qoi_test', test_qoi_exe)

if get_option('enable_gpu')
  test_gpu_exe = executable('test_gpu',
    'test_gpu.cpp',
    dependencies : [nisaba_dep, extra_deps]
  )
  test('gpu_test', test_gpu_exe)

  test_gpu_damage_exe = executable('test_gpu_damage',
    'test_gpu_damage.cpp',
    dependencies : [nisaba_dep, extra_deps]
  )
  test('gpu_damage_test', test_gpu_damage_exe)

  if get_option('enable_vulkan') and vulkan_dep.found()
    test_vulkan_exe = executable('test_vulkan',
      'test_vulkan.cpp',
      dependencies : [nisaba_dep, extra_deps]
    )
    test('vulkan_test', test_vulkan_exe)
  endif
endif


if get_option('enable_backend_os')
  test_backend_os_exe = executable('test_backend_os',
    'test_backend_os.cpp',
    dependencies : [nisaba_dep, nisaba_backend_os_dep]
  )
  test('backend_os_test', test_backend_os_exe)
endif

# Damage tracking subsystem unit tests
test_damage_tracker_exe = executable('test_damage_tracker',
  'test_damage_tracker.cpp',
  dependencies : nisaba_dep
)
test('damage_tracker_test', test_damage_tracker_exe)

# Animation subsystem unit tests
if get_option('enable_animation')
  test_animation_exe = executable('test_animation',
    'test_animation.cpp',
    dependencies : nisaba_dep
  )
  test('animation_test', test_animation_exe)
endif

# Text Caching subsystem unit tests
test_text_cache_exe = executable('test_text_cache',
  'test_text_cache.cpp',
  dependencies : nisaba_dep
)
test('text_cache_test', test_text_cache_exe)

# SVG Caching subsystem unit tests
test_svg_cache_exe = executable('test_svg_cache',
  'test_svg_cache.cpp',
  dependencies : nisaba_dep
)
test('svg_cache_test', test_svg_cache_exe)

# Sovereign Lottie vector animation engine unit tests
if get_option('enable_lottie')
  test_lottie_exe = executable('test_lottie',
    'test_lottie.cpp',
    dependencies : nisaba_dep
  )
  test('lottie_test', test_lottie_exe)
endif

if get_option('enable_markdown')
  test_markdown_exe = executable('test_markdown',
    'test_markdown.cpp',
    dependencies : nisaba_dep
  )
  test('markdown_test', test_markdown_exe)
endif

if get_option('enable_pdf')
  test_pdf_exe = executable('test_pdf',
    'test_pdf.cpp',
    dependencies : nisaba_dep
  )
  test('pdf_test', test_pdf_exe)
endif



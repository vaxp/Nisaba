project('nisaba', 'cpp',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'cpp_std=c++20',
    'optimization=2',
    'default_library=both',
  ]
)

add_project_arguments('-D_USE_MATH_DEFINES', language: ['c', 'cpp'])

cpp_comp = meson.get_compiler('cpp')
 if cpp_comp.has_argument('-march=native')
  add_project_arguments('-march=native', language: ['c', 'cpp'])
 endif

nisaba_inc = [include_directories('include')]

nisaba_sources = files(
  'src/path/dash.cpp',
  'src/path/stroker.cpp',
  'src/path/path_ops.cpp',
  'src/path/path_measure.cpp',
  'src/canvas/pixmap.cpp',
  'src/raster/line_clipper.cpp',
  'src/raster/edge.cpp',
  'src/raster/edge_builder.cpp',
  'src/raster/edge_clipper.cpp',
  'src/raster/scan.cpp',
  'src/canvas/mask.cpp',
  'src/shaders/shader.cpp',
  'src/color/color_space_lut.cpp',
  'src/pipeline/blitter.cpp',
  'src/canvas/painter.cpp',
  'src/text/attrs.cpp',
  'src/text/ttf_font.cpp',
  'src/text/cff_font.cpp',
  'src/text/opentype_tables.cpp',
  'src/text/bidi.cpp',
  'src/text/layout.cpp',
  'src/text/glyph_cache.cpp',
  'src/text/font_system.cpp',
  'src/text/buffer.cpp',
  'src/text/cursor.cpp',
  'src/text/baked_text.cpp',
  'src/text/text_cache.cpp',
  'src/effects/blur.cpp',
  'src/effects/color_filter.cpp',
  'src/effects/color_matrix.cpp',
  'src/effects/drop_shadow.cpp',
  'src/effects/glass.cpp',
  'src/svg/path_parser.cpp',
  'src/svg/svg_document.cpp',
  'src/svg/baked_svg.cpp',
  'src/svg/svg_cache.cpp',
  'src/math/transform4x4.cpp',
  'src/raster/perspective_raster.cpp',
  'src/mesh/mesh.cpp',
  'src/raster/mesh_raster.cpp',
  'src/image/deflate.cpp',
  'src/image/png.cpp',
  'src/image/jpeg.cpp',
  'src/image/qoi.cpp',
  'src/image/image_io.cpp',
)

gl_dep = dependency('gl', required : false)
egl_dep = dependency('egl', required : false)
dl_dep = dependency('dl', required : false)
glew_dep = dependency('glew', required : get_option('enable_gpu'))
vulkan_dep = dependency('vulkan', required : get_option('enable_vulkan'))

extra_deps = []
if dl_dep.found()
  extra_deps += dl_dep
endif
if gl_dep.found()
  extra_deps += gl_dep
endif
if egl_dep.found()
  extra_deps += egl_dep
endif
if glew_dep.found()
  extra_deps += glew_dep
endif
if vulkan_dep.found()
  extra_deps += vulkan_dep
endif

if get_option('enable_gpu')
  nisaba_inc += include_directories('src/gpu')
  add_project_arguments('-DNISABA_GLEW', language: ['c', 'cpp'])
  nisaba_sources += files(
    'src/gpu/types.cpp',
    'src/gpu/tessellator.cpp',
    'src/gpu/glyph_atlas.cpp',
    'src/gpu/context.cpp',
    'src/gpu/gl3_renderer.cpp',
    'src/gpu/gpu_device.cpp',
    'src/gpu/gpu_buffer.cpp',
    'src/gpu/gpu_texture.cpp',
    'src/gpu/gpu_surface.cpp',
    'src/gpu/gpu_tessellator.cpp',
    'src/gpu/gpu_canvas.cpp',
  )
  if get_option('enable_vulkan') and vulkan_dep.found()
    add_project_arguments('-DNISABA_HAS_VULKAN=1', language: ['c', 'cpp'])
    nisaba_sources += files(
      'src/gpu/vulkan/vulkan_device.cpp',
      'src/gpu/vulkan/vulkan_surface.cpp',
      'src/gpu/vulkan/vulkan_renderer.cpp',
    )
  endif
endif


if get_option('enable_layout')
  nisaba_inc += include_directories('nisaba-layout/include')
  nisaba_sources += files(
    'nisaba-layout/src/core/node.cpp',
    'nisaba-layout/src/core/config.cpp',
    'nisaba-layout/src/core/layout_results.cpp',
    'nisaba-layout/src/solver/flex_solver.cpp',
    'nisaba-layout/src/solver/pipeline.cpp',
    'nisaba-layout/src/solver/baseline.cpp',
    'nisaba-layout/src/solver/pixel_grid.cpp',
    'nisaba-layout/src/debug/assert.cpp',
    'nisaba-layout/src/debug/log.cpp',
    'nisaba-layout/src/event/event.cpp',
  )
endif

if get_option('enable_animation')
  nisaba_sources += files(
    'src/animation/curves.cpp',
    'src/animation/spring_simulation.cpp',
    'src/animation/spring_controller.cpp',
    'src/animation/timeline.cpp',
    'src/animation/particle_system.cpp',
  )
endif

if get_option('enable_lottie')
  nisaba_sources += files(
    'src/json/json.cpp',
    'src/lottie/property.cpp',
    'src/lottie/shape.cpp',
    'src/lottie/layer.cpp',
    'src/lottie/animation.cpp',
    'src/lottie/player.cpp',
  )
endif

if get_option('enable_markdown')
  nisaba_sources += files(
    'src/markdown/html_parser.cpp',
    'src/markdown/syntax_highlighter.cpp',
    'src/markdown/parser.cpp',
    'src/markdown/document.cpp',
    'src/markdown/style.cpp',
    'src/markdown/renderer.cpp',
  )
endif

if get_option('enable_pdf')
  nisaba_sources += files(
    'src/pdf/pdf_types.cpp',
    'src/pdf/pdf_writer.cpp',
    'src/pdf/pdf_canvas.cpp',
    'src/pdf/pdf_document.cpp',
    'src/pdf/pdf_parser.cpp',
    'src/pdf/pdf_cmap.cpp',
    'src/pdf/pdf_interpreter.cpp',
    'src/pdf/pdf_reader.cpp',
    'src/pdf/pdf_codec.cpp',
    'src/pdf/pdf_crypto.cpp',
    'src/pdf/pdf_navigation.cpp',
  )
endif

if get_option('enable_markdown') and get_option('enable_pdf')
  nisaba_sources += files(
    'src/markdown/pdf_exporter.cpp',
  )
endif

nisaba_lib = both_libraries('nisaba',
  nisaba_sources,
  include_directories : nisaba_inc,
  dependencies : extra_deps,
  install : true
)

nisaba_dep = declare_dependency(
  include_directories : nisaba_inc,
  link_with : nisaba_lib.get_static_lib(),
  dependencies : extra_deps
)

# Headers installation
install_subdir('include/nisaba', install_dir : get_option('includedir'))

# Pkg-config generation
pkg = import('pkgconfig')
pkg.generate(
  nisaba_lib,
  name : 'nisaba',
  description : 'Nisaba 2D Software CPU Rendering Engine for vaxp',
  version : meson.project_version(),
  subdirs : 'nisaba'
)

if get_option('enable_backend_os')
  subdir('backend_os')
endif

if get_option('build_tests')
  subdir('tests')
endif

if get_option('build_examples')
  subdir('examples')
endif

if get_option('build_benchmarks')
  subdir('benchmarks')
endif


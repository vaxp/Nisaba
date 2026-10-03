project('nisaba-layout', 'cpp',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'cpp_std=c++20',
    'optimization=2',
    'default_library=both',
  ]
)

nisaba_layout_inc = include_directories('include', '../include')

nisaba_layout_sources = files(
  'src/core/node.cpp',
  'src/core/config.cpp',
  'src/core/layout_results.cpp',
  'src/solver/flex_solver.cpp',
  'src/solver/pipeline.cpp',
  'src/solver/baseline.cpp',
  'src/solver/pixel_grid.cpp',
  'src/debug/assert.cpp',
  'src/debug/log.cpp',
  'src/event/event.cpp',
)

nisaba_layout_lib = both_libraries('nisaba_layout',
  nisaba_layout_sources,
  include_directories : nisaba_layout_inc,
  install : true
)

nisaba_layout_dep = declare_dependency(
  include_directories : nisaba_layout_inc,
  link_with : nisaba_layout_lib
)

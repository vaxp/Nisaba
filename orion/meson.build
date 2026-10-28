project('orion', 'cpp',
  version : '0.1.0',
  default_options : [
    'warning_level=3',
    'cpp_std=c++20',
    'optimization=2',
    'default_library=both',
    'b_lundef=false',
  ]
)

orion_inc = include_directories('include')

orion_sources = files(
  'src/animation/animation_clip.cpp',
  'src/animation/animator.cpp',
  'src/animation/skeleton.cpp',
  'src/animation/skinned_mesh.cpp',
  'src/bridge/physics_bridge.cpp',
  'src/camera/camera.cpp',
  'src/camera/frustum.cpp',
  'src/camera/orbit_controller.cpp',
  'src/geometry/mesh.cpp',
  'src/geometry/primitives.cpp',
  'src/gltf/gltf_loader.cpp',
  'src/gpu/gl_shader_program.cpp',
  'src/gpu/gl_state_guard.cpp',
  'src/gpu/gpu_backend3d.cpp',
  'src/gpu/gpu_resource_cache.cpp',
  'src/gpu/gpu_scene_depth_pass.cpp',
  'src/gpu/gpu_shadow_pass.cpp',
  'src/gpu/render_surface3d.cpp',
  'src/ibl/dfg_lut.cpp',
  'src/ibl/environment_map.cpp',
  'src/ibl/spherical_harmonics.cpp',
  'src/lighting/cluster_grid.cpp',
  'src/lighting/light.cpp',
  'src/lighting/lighting_manager.cpp',
  'src/lighting/shadow_map.cpp',
  'src/material/brdf.cpp',
  'src/material/pbr_material.cpp',
  'src/material/texture.cpp',
  'src/postprocess/bloom_pass.cpp',
  'src/postprocess/gtao_pass.cpp',
  'src/postprocess/postprocess_pipeline.cpp',
  'src/scene/renderable.cpp',
  'src/scene/renderer.cpp',
  'src/scene/scene.cpp',
  'src/scene/view.cpp',
  'src/math/math.cpp',
  'src/physics/body/body.cpp',
  'src/physics/collision/shape.cpp',
  'src/physics/collision/gjk.cpp',
  'src/physics/collision/epa.cpp',
  'src/physics/collision/narrow_phase.cpp',
  'src/physics/solver/impulse_solver.cpp',
  'src/physics/scene/physics_scene.cpp',
  'src/physics/scene/island.cpp',
  'src/physics/broadphase/broad_phase.cpp',
  'src/orionjs/orionjs.cpp',
  'src/orionimg/deflate.cpp',
  'src/orionimg/png.cpp',
  'src/orionimg/jpeg.cpp',
  'src/orionimg/qoi.cpp',
  'src/orionimg/webp.cpp',
  'src/orionimg/bmp.cpp',
  'src/orionimg/ppm.cpp',
  'src/orionimg/hdr.cpp',
  'src/orionimg/orionimg.cpp',
)

gl_dep = dependency('gl', required : true)
glew_dep = dependency('glew', required : false)

orion_deps = [gl_dep]
if glew_dep.found()
  orion_deps += glew_dep
endif

orion_lib = both_libraries('orion',
  orion_sources,
  include_directories : orion_inc,
  dependencies : orion_deps,
  install : true
)

orion_dep = declare_dependency(
  include_directories : orion_inc,
  link_with : orion_lib,
  dependencies : orion_deps
)


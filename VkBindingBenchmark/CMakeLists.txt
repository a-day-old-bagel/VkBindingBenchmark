
add_executable( vkbb_binding_benchmark
  camera.cpp camera.h
  Common.h
  config.h
  debug.h
  file_utils.cpp file_utils.h
  main.cpp
  material_loading.h
  mesh_loading.cpp mesh_loading.h
  null_store.cpp null_store.h
  rendering.cpp rendering.h
  shader_inputs.h
  ssbo_store.cpp ssbo_store.h
  timing.h
  ubo_store.cpp ubo_store.h
  vkh.cpp vkh.h
  vkh_alloc.h
  vkh_initializers.h
  vkh_material.cpp vkh_material.h
  vkh_mesh.cpp vkh_mesh.h
  vkh_setup.h
  vkh_texture.h
  vkh_types.h
  os_sdl2.h state.h )
target_link_libraries( vkbb_binding_benchmark
  vkbb_external
  ${vkbb_extra_linker_flags}
)
add_dependencies( vkbb_binding_benchmark
  vkbb_run_shaderPipeline
)
add_custom_command(
  TARGET vkbb_binding_benchmark POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_SOURCE_DIR}/data/mesh
  $<TARGET_FILE_DIR:vkbb_binding_benchmark>/meshes
)
add_custom_command(
  TARGET vkbb_binding_benchmark POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
  ${CMAKE_BINARY_DIR}/data/shaders
  $<TARGET_FILE_DIR:vkbb_binding_benchmark>/shaders
)

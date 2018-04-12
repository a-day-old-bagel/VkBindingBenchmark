
# This section decides whether to use the new C++17 <experimental/filesystem> library to deal with files
# and directories instead of the previous Windows-only code. This can be overridden from the command line or gui.
if ( MSVC )
  option( VKBB_USE_CPP17_FILESYSTEM "Use the <experimental/filesystem> library" OFF )
else ( MSVC )
  option( VKBB_USE_CPP17_FILESYSTEM "Use the <experimental/filesystem> library" ON )
  if ( VKBB_USE_CPP17_FILESYSTEM )
    set( vkbb_shader_pipeline_extra_linker_flags stdc++fs )
  endif ( VKBB_USE_CPP17_FILESYSTEM )
endif ( MSVC )
if ( VKBB_USE_CPP17_FILESYSTEM )
  add_definitions( -DVKBB_USE_CPP17_FILESYSTEM )
endif ( VKBB_USE_CPP17_FILESYSTEM )

# These paths are used when compiling the shaders to SPIR-V and generating reflection files.
# Use -D options when building with CMake to specify non-default values, or use the GUI.
set( vkbb_glsl_dir "${CMAKE_SOURCE_DIR}/data/shader" CACHE STRING "Where the GLSL shaders are stored" )
set( vkbb_spv_dir "${CMAKE_BINARY_DIR}/data/shaders/" CACHE STRING "Where to output the SPV shaders" )
set( vkbb_refl_dir "${CMAKE_BINARY_DIR}/data/shaders/" CACHE STRING "Where to output the reflections" )
set( vkbb_conf_dir "${CMAKE_BINARY_DIR}/data/" CACHE STRING "Where to output gpuInfo.conf" )

# This is the name of the configuration dump file
set( vkbb_gpuInfo_conf ${vkbb_conf_dir}/gpuInfo.conf)

# This points to the newly compiled glslangValidator
set( vkbb_glslangValidator $<TARGET_FILE:glslangValidator> )

# Get the configuration dump from glslangValidator
add_custom_command(
  OUTPUT ${vkbb_gpuInfo_conf}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${vkbb_conf_dir}/"
  COMMAND ${vkbb_glslangValidator} -c > ${vkbb_gpuInfo_conf}
  DEPENDS glslangValidator
)
# This target represents the completion of the task above
add_custom_target(
  vkbb_gpuInfo_conf
  DEPENDS ${vkbb_gpuInfo_conf}
)

# Find the GLSL sources, put them in a list
file( GLOB_RECURSE vkbb_glsl_sources
  "${vkbb_glsl_dir}/*.frag"
  "${vkbb_glsl_dir}/*.vert"
  "${vkbb_glsl_dir}/*.tese"
  "${vkbb_glsl_dir}/*.tesc"
  "${vkbb_glsl_dir}/*.geom"
  "${vkbb_glsl_dir}/*.comp"
)

# Compile each GLSL source to SPV
foreach ( glsl_source ${vkbb_glsl_sources} )
  get_filename_component( FILE_NAME ${glsl_source} NAME )
  set( spirv_output "${vkbb_spv_dir}/${FILE_NAME}.spv" )
  add_custom_command(
    OUTPUT ${spirv_output}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${vkbb_spv_dir}/"
    COMMAND ${vkbb_glslangValidator} -V -o ${spirv_output} ${glsl_source} ${vkbb_gpuInfo_conf}
    DEPENDS ${glsl_source} glslangValidator vkbb_gpuInfo_conf
  )
  list( APPEND vkbb_spirv_binaries ${spirv_output} )
endforeach ( glsl_source )
# This target represents the completion of the task above
add_custom_target(
  vkbb_compile_shaders
  DEPENDS ${vkbb_spirv_binaries}
)

# compile the stuff in ShaderPipeline
add_executable( vkbb_shader_pipeline
  cmake_main.cpp
  filesystem_utils.h
  shaderdata.h
  string_utils.h
)
target_link_libraries( vkbb_shader_pipeline
  vkbb_external
  ${vkbb_shader_pipeline_extra_linker_flags}
)
add_dependencies( vkbb_shader_pipeline
  vkbb_gpuInfo_conf
  vkbb_compile_shaders
)

# Finally, use the ShaderPipeline executable to generate the reflection files...
set( vkbb_shaderPipeline $<TARGET_FILE:vkbb_shader_pipeline> )

# Make a list of the expected refl outputs
foreach ( glsl_source ${vkbb_glsl_sources} )
  get_filename_component( FILE_NAME ${glsl_source} NAME )
  set( refl_output "${vkbb_refl_dir}/${FILE_NAME}.refl" )
  list( APPEND vkbb_refl_files ${refl_output} )
endforeach ( glsl_source )

# Run the ShaderPipeline executable, if there's anything to do.
if ( vkbb_refl_files )
  add_custom_command(
    OUTPUT ${vkbb_refl_files}
    COMMAND ${vkbb_shaderPipeline} ${vkbb_spv_dir} ${vkbb_refl_dir}
    DEPENDS ${vkbb_spv_sources} vkbb_shader_pipeline
  )
endif( vkbb_refl_files )
add_custom_target(
  vkbb_run_shaderPipeline
  DEPENDS ${vkbb_refl_files}
)

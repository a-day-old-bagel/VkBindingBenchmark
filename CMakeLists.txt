
# require an actual recent version of CMake
cmake_minimum_required( VERSION 3.9 FATAL_ERROR )
# call the project "Vulkan Binding Benchmark", or "vkbb"
project( vkbb )

# use C++ 17
set( CMAKE_CXX_STANDARD 17 )
# search for extra modules (like find modules) in the cmake directory
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

# compile if necessary and include external dependencies
add_subdirectory( external )

# Manually add any extra flags for now to fix minor grievances - improve later.
if ( MSVC )
  # This flag is because VS has to be told to use modern C++ in five different ways before it believes you.
  list( APPEND vkbb_extra_compiler_flags " /std:c++latest" )
else ( MSVC )
  # This is so that <experimental/filesystem> will link in linux.
  list( APPEND vkbb_extra_linker_flags " stdc++fs" )
endif ( MSVC )
list( APPEND CMAKE_CXX_FLAGS ${vkbb_extra_compiler_flags} )

# Build the ShaderPipeline executable (Shaders are compiled to SPIR-V here as well, and are supposed to automatically
# recompile if any of the GLSL source changes)
add_subdirectory( ShaderPipeline )
# Build the BindingBenchmark executable
add_subdirectory( VkBindingBenchmark )

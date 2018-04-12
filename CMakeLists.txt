cmake_minimum_required( VERSION 3.10 FATAL_ERROR )
project( vkbb )

set( CMAKE_CXX_STANDARD 17 )
set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

add_subdirectory( external )
add_subdirectory( ShaderPipeline )
add_subdirectory( VkBindingBenchmark )

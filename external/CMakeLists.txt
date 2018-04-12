
# Build the external libs that need building

# SDL2
find_package( SDL2 REQUIRED )
list( APPEND vkbb_external_includes
  ${SDL2_INCLUDE_DIR}
)

# Vulkan
find_package( Vulkan REQUIRED )
list( APPEND vkbb_external_includes
  ${Vulkan_INCLUDE_DIR}
)

# XCB (if present)
find_package( X11_XCB )
if ( X11_XCB_FOUND )
  set( XCB_LIBS ${X11_XCB_LIBRARIES} )
  list( APPEND vkbb_external_includes
    ${X11_XCB_INCLUDE_DIRS}
  )
endif ()

## ASSIMP
#find_package( Assimp REQUIRED )
#list( APPEND vkbb_external_includes
#  ${assimp_INCLUDE_DIRS}
#)

# SPIRV_CROSS
option( SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS "Instead of throwing exceptions assert" ON )
add_subdirectory( spirv-cross )

# GLSLANG
option( SKIP_GLSLANG_INSTALL "Skip installation" ON )
option( ENABLE_AMD_EXTENSIONS "Enables support of AMD-specific extensions" ON )
option( ENABLE_GLSLANG_BINARIES "Builds glslangValidator and spirv-remap" ON )
option( ENABLE_NV_EXTENSIONS "Enables support of Nvidia-specific extensions" ON )
option( ENABLE_HLSL "Enables HLSL input support" OFF )
option( ENABLE_OPT "Enables spirv-opt capability if present" ON )
add_subdirectory( ./glslang )

# REALTIMEUTILS
add_subdirectory( ./realtimeutils )

# Build the external dependencies together into a linkable interface

add_library( vkbb_external INTERFACE )

target_include_directories( vkbb_external INTERFACE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include/assimp
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include/assimp/Compiler
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include/assimp/port
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include/assimp/port/AndroidJNI
  ${CMAKE_CURRENT_SOURCE_DIR}/argh
  ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson
  ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/error
  ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/internal
  ${CMAKE_CURRENT_SOURCE_DIR}/stb
  ${CMAKE_CURRENT_SOURCE_DIR}/glm

  SYSTEM

  ${vkbb_external_includes}
)

target_link_libraries( vkbb_external INTERFACE
  ${SDL2_LIBRARY}
  #  SDL2 SDL2main
  ${Vulkan_LIBRARY}
  ${XCB_LIBS}
#  ${assimp_LIBRARIES}
  assimp
  spirv-cross-glsl
  realtimeutils
  )

message( STATUS "SDL2 INCLUDE DIR: ${SDL2_INCLUDE_DIR}" )

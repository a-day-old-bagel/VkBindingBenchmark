
# Build the external libs that need building

# SDL_VULKAN
#add_subdirectory( ./sdl_vulkan )
find_package( SDL2 REQUIRED )
find_package( Vulkan REQUIRED )
include_directories( SYSTEM
  ${SDL2_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIR}
  )

# ASSIMP
find_package( Assimp REQUIRED )
include_directories(
  ${assimp_INCLUDE_DIRS}
)

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

set( vkbb_dirs_to_include
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
  )
if ( WIN32 )
  set( vkbb_dirs_to_include "${vkbb_dirs_to_include} rapidjson/msinttypes" )
endif ( WIN32 )

target_include_directories( vkbb_external INTERFACE
  ${vkbb_dirs_to_include}
  )

find_package( X11_XCB )
if ( X11_XCB_FOUND )
  set( X_LIBS ${X11_XCB_LIBRARIES} )
  include_directories( SYSTEM ${X11_XCB_INCLUDE_DIRS} )
endif ()

target_link_libraries( vkbb_external INTERFACE
  #  SDL_vulkan
  ${SDL2_LIBRARY}
  #  SDL2 SDL2main
  ${Vulkan_LIBRARY}
  ${X_LIBS}
  ${assimp_LIBRARIES}
  assimp
  spirv-cross-glsl
  realtimeutils
  )

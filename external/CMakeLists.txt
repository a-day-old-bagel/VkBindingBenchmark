
# Find the libraries that should be installed on the user's system
# In Windows, this will require downloading the SDL2 Development Libraries and installing the Vulkan SDK,
# and making sure your environment variables are set up to point to them correctly

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
  add_definitions( -DHAVE_X11_XCB )
  set( XCB_LIBS ${X11_XCB_LIBRARIES} )
  list( APPEND vkbb_external_includes
    ${X11_XCB_INCLUDE_DIRS}
  )
endif ()

# Assimp
option( BUILD_SHARED_LIBS "Build package with shared libraries." OFF )
option( BUILD_FRAMEWORK "Build package as Mac OS X Framework bundle." OFF )
option( ASSIMP_DOUBLE_PRECISION "Set to ON to enable double precision processing" OFF )
option( ASSIMP_OPT_BUILD_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF )
option( ASSIMP_ANDROID_JNIIOSYSTEM "Android JNI IOSystem support is active" OFF )
option( ASSIMP_NO_EXPORT "Disable Assimp's export functionality." ON )
option( ASSIMP_BUILD_ZLIB "Build your own zlib" OFF )
option( ASSIMP_BUILD_ASSIMP_TOOLS "If the supplementary tools for Assimp are built in addition to the library." OFF )
option( ASSIMP_BUILD_SAMPLES "If the official samples are built as well (needs Glut)." OFF )
option( ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." OFF )
option( ASSIMP_COVERALLS "Enable this to measure test coverage." OFF )
option( ASSIMP_WERROR "Treat warnings as errors." OFF )
option( ASSIMP_ASAN "Enable AddressSanitizer." OFF )
option( ASSIMP_UBSAN "Enable Undefined Behavior sanitizer." OFF )
option( SYSTEM_IRRXML "Use system installed Irrlicht/IrrXML library." OFF )
option( BUILD_DOCS "Build documentation using Doxygen." OFF )
option( ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT "default value of all ASSIMP_BUILD_XXX_IMPORTER value" FALSE )
option( ASSIMP_BUILD_OBJ_IMPORTER "build obj importer" TRUE )
add_subdirectory( assimp )
list( APPEND vkbb_external_includes
  ${CMAKE_BINARY_DIR}/external/assimp/include
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

# Assemble the external dependencies together into a linkable interface that also automatically includes all
# the necessary directories
add_library( vkbb_external INTERFACE )
target_include_directories( vkbb_external INTERFACE SYSTEM
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/assimp/include
  ${CMAKE_CURRENT_SOURCE_DIR}/argh
  ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson
  ${CMAKE_CURRENT_SOURCE_DIR}/stb
  ${CMAKE_CURRENT_SOURCE_DIR}/glm
  ${vkbb_external_includes}
)
target_link_libraries( vkbb_external INTERFACE
  ${SDL2_LIBRARY}
  ${Vulkan_LIBRARY}
  ${XCB_LIBS}
  assimp
  spirv-cross-glsl
  realtimeutils
)

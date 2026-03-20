cmake_minimum_required(VERSION 3.28.5 FATAL_ERROR)

project(craysim_minimal LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# FindOptiX.cmake lives in CMake/
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

# Locate the NVRT distribution.  Search the SDK first, then look in the system.
set(OptiX_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../" CACHE PATH "Path to OptiX installed location.")

# Find the libraries which will be needed in addition to compound-ray (libEyeRenderer3.so)
find_package(glfw3 3.2...3.4 REQUIRED)
find_package(OpenGL)
find_package(OpenMP) # (OpenMP may be used to build navmesh)
find_package(Freetype REQUIRED)
find_package(HDF5 REQUIRED)
find_package(OptiX REQUIRED)
find_package(compound-ray)

# Where is mathplot? (allows headers and modules to be found)
set(MPLOT_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/extern/mathplot" CACHE PATH "The path to mathplot")

# Assume a gcc compiler (or a gcc mimic like Clang). OpenGL 4.3 required so no Apple.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors")
# Tell clang to be quiet about brace initialisers:
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Additional GL compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

# Tell the program where the morph fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPLOT_FONTS_DIR=\"\\\"${MPLOT_INCLUDE_PATH}/fonts\\\"\"")

if(HDF5_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HDF5_DEFINITIONS}")
endif()

# find_package(glfw3...) does not set any 'GLFW_LIBS' variable, so set one manually
if(UNIX)
  set(GLFW_LIB_NAME glfw)
else()
  set(GLFW_LIB_NAME glfw3)
endif()

#
# Include paths for module compilation
#

# mathplot includes
include_directories(
  ${HDF5_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${MPLOT_INCLUDE_PATH}/mplot/glad
  ${MPLOT_INCLUDE_PATH}
)

# Maths include path. Allows #include <sm/config> (the only non-modules maths header at the moment)
include_directories(${PROJECT_SOURCE_DIR}/extern/maths)

# Add the path to the OptiX headers to our include paths. This is
# required to compile craysim.virtual, which includes compound-ray
# headers, which refer to some OptiX data structures
include_directories("${OptiX_INCLUDE}")

# allows #include <tinygltf/tiny_gltf.h> (an oces dependency)
include_directories ("${PROJECT_SOURCE_DIR}/extern")

#
# Define lists of module files to build
#

set(EXTERN_DIR ${PROJECT_SOURCE_DIR}/extern)
set(OCES_MODULE_PATH ${EXTERN_DIR}/oces_viewer)
set(CRAYSIM_MODULE_PATH ${EXTERN_DIR}/craysim)
set(MATHS_MODULE_PATH ${EXTERN_DIR}/maths)

# Modules in this repo
set(MY_MODULES
  ${CRAYSIM_MODULE_PATH}/modules/tk_spline.cppm           # GPL v2
  ${CRAYSIM_MODULE_PATH}/modules/craysim_visual.cppm
  ${CRAYSIM_MODULE_PATH}/modules/craysim_random_walk.cppm # Uses tk_spline, GPL v2
  ${CRAYSIM_MODULE_PATH}/modules/craysim_antbody.cppm
)
# The OCES reader code
set(OCES_MODULES ${OCES_MODULE_PATH}/oces/reader)
# sebsjames/maths
set(MATHS_CORE_MODULES
  ${MATHS_MODULE_PATH}/sm/mathconst
  ${MATHS_MODULE_PATH}/sm/constexpr_math
  ${MATHS_MODULE_PATH}/sm/polysolve
  ${MATHS_MODULE_PATH}/sm/bessel_i0
  ${MATHS_MODULE_PATH}/sm/trait_tests
  ${MATHS_MODULE_PATH}/sm/range
  ${MATHS_MODULE_PATH}/sm/random
  ${MATHS_MODULE_PATH}/sm/vec
  ${MATHS_MODULE_PATH}/sm/vvec
  ${MATHS_MODULE_PATH}/sm/quaternion
  ${MATHS_MODULE_PATH}/sm/mat
)
set(MATHS_MATHPLOT_MODULES
  ${MATHS_MODULE_PATH}/sm/scale
  ${MATHS_MODULE_PATH}/sm/hdfdata
  ${MATHS_MODULE_PATH}/sm/winder
  ${MATHS_MODULE_PATH}/sm/grid
  ${MATHS_MODULE_PATH}/sm/algo
  ${MATHS_MODULE_PATH}/sm/histo
  ${MATHS_MODULE_PATH}/sm/geometry
  ${MATHS_MODULE_PATH}/sm/geometry_polyhedra
  ${MATHS_MODULE_PATH}/sm/flags
  ${MATHS_MODULE_PATH}/sm/centroid
  ${MATHS_MODULE_PATH}/sm/util
  ${MATHS_MODULE_PATH}/sm/base64
  ${MATHS_MODULE_PATH}/sm/crc32
)
# sebsjames/mathplot
set(MATHPLOT_CORE_MODULES
  ${MPLOT_INCLUDE_PATH}/mplot/fps/profiler.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualOwnable.h
  ${MPLOT_INCLUDE_PATH}/mplot/Visual.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualGlfw.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualFace.h
  ${MPLOT_INCLUDE_PATH}/mplot/TextFeatures.h
  ${MPLOT_INCLUDE_PATH}/mplot/TextGeometry.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualResources.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualCommon.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualFont.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualTextModel.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualModel.h
  ${MPLOT_INCLUDE_PATH}/mplot/VisualDataModel.h
  ${MPLOT_INCLUDE_PATH}/mplot/NavMesh.h
  ${MPLOT_INCLUDE_PATH}/mplot/ColourMap.h
  ${MPLOT_INCLUDE_PATH}/mplot/colourmaps_cet.h
  ${MPLOT_INCLUDE_PATH}/mplot/colourmaps_crameri.h
  ${MPLOT_INCLUDE_PATH}/mplot/ColourMap_Lists.h
  ${MPLOT_INCLUDE_PATH}/mplot/CoordArrows.h
  ${MPLOT_INCLUDE_PATH}/mplot/win_t.h
  ${MPLOT_INCLUDE_PATH}/mplot/colour.h
  ${MPLOT_INCLUDE_PATH}/mplot/tools.h
  ${MPLOT_INCLUDE_PATH}/mplot/unicode.h
  ${MPLOT_INCLUDE_PATH}/mplot/loadpng.h
  ${MPLOT_INCLUDE_PATH}/mplot/gl/version.h
  ${MPLOT_INCLUDE_PATH}/mplot/gl/util_mx.h
  ${MPLOT_INCLUDE_PATH}/mplot/graphing.h
  ${MPLOT_INCLUDE_PATH}/mplot/graphstyles.h
  ${MPLOT_INCLUDE_PATH}/mplot/DatasetStyle.h
)
set(CRAYSIM_VM_MODULES
  ${MPLOT_INCLUDE_PATH}/mplot/compoundray/interop.h
  ${MPLOT_INCLUDE_PATH}/mplot/compoundray/Ommatidium.h
  ${MPLOT_INCLUDE_PATH}/mplot/compoundray/EyeVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/VerticesVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/NormalsVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/InstancedScatterVisual.h
)

#
# Compile craysim_minimal program
#

# All the modules that are going to be built for this executable:
set(CRAYSIM_MINIMAL_MODULES
  ${MY_MODULES}
  ${OCES_MODULES}
  ${MATHS_CORE_MODULES}
  ${MATHS_MATHPLOT_MODULES}
  ${MATHPLOT_CORE_MODULES}
  ${CRAYSIM_VM_MODULES}
)
# We set the language to C++ for each module file as they may not have a .cppm or .ixx file suffix
set_source_files_properties (${CRAYSIM_MINIMAL_MODULES} PROPERTIES LANGUAGE CXX)

# For mathplot, we must build the glad library here
add_library(glad STATIC ${MPLOT_INCLUDE_PATH}/mplot/glad/gl.c)

# We have to build the mathplot fonts data library - this packs up
# some TTF fonts into an object file; they're unpacked at runtime.
add_library(mplot_fontdata STATIC ${MPLOT_INCLUDE_PATH}/mplot/VisualFaceAsm.cpp)

# Our example program
add_executable(craysim_minimal main.cpp)
target_sources(craysim_minimal
  PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${PROJECT_SOURCE_DIR}
  FILES ${CRAYSIM_MINIMAL_MODULES}
)
target_link_libraries(craysim_minimal
  compound-ray::EyeRenderer3
  ${HDF5_C_LIBRARIES}
  ${GLFW_LIB_NAME}
  Freetype::Freetype
  glad
  mplot_fontdata
)

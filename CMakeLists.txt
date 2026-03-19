cmake_minimum_required(VERSION 3.28.5 FATAL_ERROR)

project(craysim_minimal LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if( POLICY CMP0072 )
  # FindOpenGL prefers GLVND by default when available
  cmake_policy(SET CMP0072 NEW)
endif()

if( POLICY CMP0074 )
  # find_package uses <PackageName>_ROOT variables.
  cmake_policy(SET CMP0074 NEW)
endif()

# Mathplot uses OpenGL
find_package(OpenGL)

# Use a packaged GLFW
find_package(glfw3 3.2...3.4 REQUIRED)
# find_package(glfw3...) does not set any 'GLFW_LIBS' variable, so set one manually
if(UNIX)
  set(GLFW_LIB_NAME glfw)
else()
  set(GLFW_LIB_NAME glfw3)
endif()

# FindOptiX.cmake lives in CMake/
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

# Set the default build to Release.  Note this doesn't do anything for the VS
# default build target which defaults to Debug when you first start it.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
      "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# Tells CMake to build all the libraries as shared libraries by default.  This can be
# overrided by individual libraries later.
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Locate the NVRT distribution.  Search the SDK first, then look in the system.
set(OptiX_INSTALL_DIR "${CMAKE_SOURCE_DIR}/../" CACHE PATH "Path to OptiX installed location.")

# Search for the OptiX libraries and include files.
find_package(OptiX REQUIRED)

# Add the path to the OptiX headers to our include paths.
include_directories("${OptiX_INCLUDE}" "${CMAKE_CURRENT_SOURCE_DIR}/cuda")

# we have build/sampleConfig.h build/include/sse_support.h to include, but leave out
include_directories(${CMAKE_CURRENT_SOURCE_DIR} "${CMAKE_BINARY_DIR}/include" ${CMAKE_CURRENT_BINARY_DIR} ${CUDA_INCLUDE_DIRS})

#
# Mathplot incantations
#

# Assume a gcc compiler (or a gcc mimic like Clang). OpenGL 4.3 required so no Apple.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -O3 -Wno-unused-result -Wno-unknown-pragmas -Wfatal-errors")
# Tell clang to be quiet about brace initialisers:
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-braces")
endif()

# Add OpenMP flags here, if necessary (OpenMP may be used to build navmesh)
find_package(OpenMP)
if(OpenMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# Additional GL compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL3_PROTOTYPES -DGL_GLEXT_PROTOTYPES")

# Find the libraries which will be needed in addition to those required by compound-ray
find_package(Freetype REQUIRED)
find_package(HDF5 REQUIRED)
if(HDF5_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HDF5_DEFINITIONS}")
  include_directories(${HDF5_INCLUDE_DIR})
endif()

# Define collections of includes that have to be made for mathplot
set(MPLOT_INC_GL ${OPENGL_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS})
include_directories(${MPLOT_INC_GL})
# Assuming that you installed mathplot in-tree (i.e. 'next to' schnakenberg.cpp).
set(MPLOT_INCLUDE_PATH "${PROJECT_SOURCE_DIR}/extern/mathplot" CACHE PATH "The path to mathplot")
# Tell the program where the morph fonts are, to compile them into the binary
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMPLOT_FONTS_DIR=\"\\\"${MPLOT_INCLUDE_PATH}/fonts\\\"\"")
set(SM_INCLUDE_PATH "${MPLOT_INCLUDE_PATH}/maths" CACHE PATH "The path to maths/sm")
include_directories(BEFORE ${MPLOT_INCLUDE_PATH}/include) # Allows GL3/gl3.h to be found
include_directories(BEFORE ${MPLOT_INCLUDE_PATH} ${SM_INCLUDE_PATH}) # Allows mplot/Header.h to be found

set(MPLOT_LIBS_CORE ${HDF5_C_LIBRARIES})
set(MPLOT_LIBS_GL ${GLFW_LIB_NAME} Freetype::Freetype)

#
# mathplot incantations complete
#

#
# Define lists of module files to build
#

set(OCES_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/extern/oces_viewer)
set(CRAYSIM_INCLUDE_PATH ${PROJECT_SOURCE_DIR}/extern/craysim)

# Modules in this repo
set(MY_MODULES
  ${CRAYSIM_INCLUDE_PATH}/modules/tk_spline.cppm           # GPL v2
  ${CRAYSIM_INCLUDE_PATH}/modules/craysim_visual.cppm
  ${CRAYSIM_INCLUDE_PATH}/modules/craysim_random_walk.cppm # Uses tk_spline, GPL v2
  ${CRAYSIM_INCLUDE_PATH}/modules/craysim_antbody.cppm
)
# The OCES reader code
set(OCES_MODULES ${OCES_INCLUDE_PATH}/oces/reader)
# sebsjames/maths
set(MATHS_CORE_MODULES
  ${MPLOT_INCLUDE_PATH}/maths/sm/mathconst
  ${MPLOT_INCLUDE_PATH}/maths/sm/constexpr_math
  ${MPLOT_INCLUDE_PATH}/maths/sm/polysolve
  ${MPLOT_INCLUDE_PATH}/maths/sm/bessel_i0
  ${MPLOT_INCLUDE_PATH}/maths/sm/trait_tests
  ${MPLOT_INCLUDE_PATH}/maths/sm/range
  ${MPLOT_INCLUDE_PATH}/maths/sm/random
  ${MPLOT_INCLUDE_PATH}/maths/sm/vec
  ${MPLOT_INCLUDE_PATH}/maths/sm/vvec
  ${MPLOT_INCLUDE_PATH}/maths/sm/quaternion
  ${MPLOT_INCLUDE_PATH}/maths/sm/mat
)
set(MATHS_MATHPLOT_MODULES
  ${MPLOT_INCLUDE_PATH}/maths/sm/scale
  ${MPLOT_INCLUDE_PATH}/maths/sm/hdfdata
  ${MPLOT_INCLUDE_PATH}/maths/sm/winder
  ${MPLOT_INCLUDE_PATH}/maths/sm/grid
  ${MPLOT_INCLUDE_PATH}/maths/sm/algo
  ${MPLOT_INCLUDE_PATH}/maths/sm/histo
  ${MPLOT_INCLUDE_PATH}/maths/sm/geometry
  ${MPLOT_INCLUDE_PATH}/maths/sm/geometry_polyhedra
  ${MPLOT_INCLUDE_PATH}/maths/sm/flags
  ${MPLOT_INCLUDE_PATH}/maths/sm/centroid
  ${MPLOT_INCLUDE_PATH}/maths/sm/util
  ${MPLOT_INCLUDE_PATH}/maths/sm/base64
  ${MPLOT_INCLUDE_PATH}/maths/sm/crc32
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
  ${MPLOT_INCLUDE_PATH}/mplot/RodVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/VectorVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/VerticesVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/SphereVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/NormalsVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/compoundray/interop.h
  ${MPLOT_INCLUDE_PATH}/mplot/compoundray/Ommatidium.h
  ${MPLOT_INCLUDE_PATH}/mplot/compoundray/EyeVisual.h
  ${MPLOT_INCLUDE_PATH}/mplot/InstancedScatterVisual.h
)

# By CMake magic, this sorts out include directories including (e.g.)
# /usr/local/include/compound-ray and gives us a link target which is
# compound-ray::EyeRenderer3
find_package (compound-ray)

#
# Compile craysim program
#

# Building the oces::reader module will require some additional includes:
include_directories(BEFORE ${PROJECT_SOURCE_DIR})
include_directories (BEFORE "${PROJECT_SOURCE_DIR}/extern/oces_viewer")
include_directories (BEFORE "${PROJECT_SOURCE_DIR}/extern/oces_viewer/extern")

# Each C++ module file has to be listed here and set as language CXX
# (as they may not have a .cppm or .ixx file suffix)
set_source_files_properties (
  ${MY_MODULES}
  ${OCES_MODULES}
  ${MATHS_CORE_MODULES}
  ${MATHS_MATHPLOT_MODULES}
  ${MATHPLOT_CORE_MODULES}
  ${CRAYSIM_VM_MODULES}
  PROPERTIES LANGUAGE CXX
)

# For mathplot, we must build the glad library here
include_directories(BEFORE ${MPLOT_INCLUDE_PATH}/mplot/glad)
add_library(glad STATIC ${MPLOT_INCLUDE_PATH}/mplot/glad/gl.c)

# We have to build the mathplot fonts data library - this packs up
# some TTF fonts into an object file; they're unpacked at runtime.
add_library(mplot_fontdata STATIC ${MPLOT_INCLUDE_PATH}/mplot/VisualFaceAsm.cpp)

# Our example program
set(CRAYSIM_MINIMAL_MODULES ${MY_MODULES} ${OCES_MODULES} ${MATHS_CORE_MODULES} ${MATHS_MATHPLOT_MODULES} ${MATHPLOT_CORE_MODULES} ${CRAYSIM_VM_MODULES})
add_executable(craysim_minimal main.cpp)
target_sources(craysim_minimal PUBLIC FILE_SET CXX_MODULES BASE_DIRS ${PROJECT_SOURCE_DIR} FILES ${CRAYSIM_MINIMAL_MODULES})
# Link CUDA, mathplot dependencies and libEyeRenderer3.so omit: ${CUDA_LIBRARIES}
target_link_libraries(craysim_minimal ${MPLOT_LIBS_CORE} ${MPLOT_LIBS_GL} compound-ray::EyeRenderer3 glad mplot_fontdata)

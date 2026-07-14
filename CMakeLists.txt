#
# craysim_minimal example CMakeLists.txt file. Use this as a template
# for craysim based simulators.
#

cmake_minimum_required(VERSION 3.28.5 FATAL_ERROR)

project(craysim_minimal LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# It appears to be critical to set this for some component of the model. Without it, I see an error like:
# craysim_minimal: /home/seb/src/craysim_minimal/extern/tinygltf/tiny_gltf.h:347: \
#  const Value &tinygltf::Value::Get(const std::string &) const: Assertion `IsObject()' failed.
# Aborted (core dumped)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release"
    CACHE STRING "Choose build type from: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)

# The file FindOptiX.cmake is provided by craysim, and also module_definitions.cmake
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/extern/craysim/cmake" ${CMAKE_MODULE_PATH})

# Path to the NVidia OptiX SDK directory e.g. ~/src/NVIDIA-OptiX-SDK-8.0.0-linux64-x86_64
set(OptiX_INSTALL_DIR "${PROJECT_SOURCE_DIR}/../" CACHE PATH "Path to OptiX installed location.")

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

# allows #include <nlohmann/json.hpp> (sm::config dependency)
include_directories ("${PROJECT_SOURCE_DIR}/extern/json/include")

#
# Define lists of module files to build
#
# We obtain module variable definitions from some of our dependencies
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/extern/mathplot/cmake" ${CMAKE_MODULE_PATH})
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/extern/maths/cmake" ${CMAKE_MODULE_PATH})

# Include maths module defs
include(${PROJECT_SOURCE_DIR}/extern/maths/cmake/module_definitions.cmake)
setup_module_variables_for_maths (${PROJECT_SOURCE_DIR}/extern/maths ${PROJECT_SOURCE_DIR}/extern/json)
# Include mathplot module defs
include(${PROJECT_SOURCE_DIR}/extern/mathplot/cmake/module_definitions.cmake)
setup_module_variables_for_mathplot_maths (${PROJECT_SOURCE_DIR}/extern/maths)
setup_module_variables_for_mathplot (${PROJECT_SOURCE_DIR}/extern/mathplot ${PROJECT_SOURCE_DIR}/extern/json)
# craysim module defs
include(${PROJECT_SOURCE_DIR}/extern/craysim/cmake/module_definitions.cmake)
setup_module_variables_for_craysim_maths (${PROJECT_SOURCE_DIR}/extern/maths ${PROJECT_SOURCE_DIR}/extern/json)
setup_module_variables_for_craysim_mathplot (${PROJECT_SOURCE_DIR}/extern/mathplot)
setup_module_variables_for_craysim (${PROJECT_SOURCE_DIR}/extern/craysim)

set(EXTERN_DIR ${PROJECT_SOURCE_DIR}/extern)
set(OCES_MODULE_PATH ${EXTERN_DIR}/oces_viewer)

#set(CRAYSIM_MODULE_PATH ${EXTERN_DIR}/craysim)
#set(MATHS_MODULE_PATH ${EXTERN_DIR}/maths)

# The OCES reader code
set(OCES_MODULES ${OCES_MODULE_PATH}/oces/reader.cppm)

#
# Compile craysim_minimal program
#

# All the modules that are going to be built for this executable:
set(CRAYSIM_MINIMAL_MODULES
  ${MPLOT_MATHS_CORE_MODULES}
  ${MPLOT_MATHS_VISUALMODEL_MODULES}
  ${MPLOT_CORE_MODULES}
  ${CRAYSIM_MATHS_MODULES}
  ${CRAYSIM_MATHPLOT_MODULES}
  ${CRAYSIM_MODULES}
  ${OCES_MODULES}
)
# We set the language to C++ for each module file as they may not all have a .cppm or .ixx file suffix
set_source_files_properties (${CRAYSIM_MINIMAL_MODULES} PROPERTIES LANGUAGE CXX)

# Remove any module duplicates from CRAYSIM_MINIMAL_MODULES
list(REMOVE_DUPLICATES CRAYSIM_MINIMAL_MODULES)

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

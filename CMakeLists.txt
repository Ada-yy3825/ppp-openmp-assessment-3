cmake_minimum_required(VERSION 3.20)
project(PPPAssessment3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenMP REQUIRED)

# Shared warning profile for all student targets.
add_library(ppp_warnings INTERFACE)
target_compile_options(ppp_warnings INTERFACE
  -Wall -Wextra -Wpedantic
  -Wno-unused-parameter
)

# Pin OpenMP 5.1 at compile time for Clang. GCC does NOT accept
# -fopenmp-version= (its supported OpenMP level tracks the compiler version
# itself; GCC 12+ already covers 5.1-level features).
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(OpenMP::OpenMP_CXX INTERFACE -fopenmp-version=51)
endif()

# A3 — 3D Jacobi (core)
add_executable(stencil core/stencil.cpp)
target_include_directories(stencil PRIVATE core)
target_link_libraries(stencil PRIVATE OpenMP::OpenMP_CXX ppp_warnings)

# Extension targets: only registered if the student's source is present.
# This lets CI build whichever extension the student actually submitted.
foreach(ext numa_first_touch false_sharing simd)
  file(GLOB _ext_srcs "extension/${ext}/*.cpp")
  if(_ext_srcs)
    # Convention: each .cpp in an extension dir is its own self-contained
    # program with its own main(). Target name: ext_<ext>_<filename>.
    foreach(_src ${_ext_srcs})
      get_filename_component(_name ${_src} NAME_WE)
      set(_target "ext_${ext}_${_name}")
      add_executable(${_target} ${_src})
      target_include_directories(${_target} PRIVATE core)
      target_link_libraries(${_target} PRIVATE OpenMP::OpenMP_CXX ppp_warnings)
    endforeach()
  endif()
endforeach()

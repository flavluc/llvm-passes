cmake_minimum_required(VERSION 3.4.3)
project(dcc-888-tp-1)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

find_package(LLVM REQUIRED CONFIG)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

add_library (DeadCodeElimination SHARED DeadCodeElimination.cpp)
add_library (RangeAnalysis SHARED lib/RangeAnalysis.cpp)
add_library (vSSA SHARED lib/vSSA.cpp)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

cmake_minimum_required(VERSION 3.4.3)
project(dead-code-elimination)

find_package(LLVM REQUIRED CONFIG)

execute_process (COMMAND llvm-config --cxxflags --ldflags OUTPUT_VARIABLE LLVM_CONFIG_N)

set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})

string (REPLACE "\n" " " LLVM_CONFIG "${LLVM_CONFIG_N}")

set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_FLAGS "-O3 -g ${LLVM_CONFIG} -undefined")

if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

include_directories (include)
add_library(DeadCodeElimination SHARED DeadCodeElimination.cpp)

add_library (RangeAnalysis SHARED
  lib/RangeAnalysis.cpp
  lib/vSSA.cpp
)

target_link_libraries(DeadCodeElimination
  "$<$<PLATFORM_ID:Darwin>:-undefined dynamic_lookup>")

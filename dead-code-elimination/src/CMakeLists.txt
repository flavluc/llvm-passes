cmake_minimum_required(VERSION 3.0)

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

if("${LLVM_TOOLS_BINARY_DIR}" STREQUAL "")
	message(FATAL_ERROR "\${LLVM_TOOLS_BINARY_DIR} was not set, try setting CMAKE_PREFIX_PATH")
endif()

find_program(CLANG  clang  NO_DEFAULT_PATH PATHS "${LLVM_TOOLS_BINARY_DIR}")
find_program(CLANGXX clang++ NO_DEFAULT_PATH PATHS "${LLVM_TOOLS_BINARY_DIR}")

if (("${CLANG}" STREQUAL "CLANG-NOTFOUND") OR ("${CLANGXX}" STREQUAL "CLANGXX-NOTFOUND"))

	message(FATAL_ERROR "Could not find clang, try setting CMAKE_PREFIX_PATH")
endif()

set(CMAKE_CXX_FLAGS "-std=c++17 ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-fno-rtti ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS "-fno-exceptions -undefined dynamic_lookup ${CMAKE_CXX_FLAGS}")

add_library(DCETP2 MODULE DeadCodeElimination.cpp)

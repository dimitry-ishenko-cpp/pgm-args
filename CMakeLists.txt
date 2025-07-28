cmake_minimum_required(VERSION 3.16) # focal
project(pgm_args VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(lib)

option(BUILD_EXAMPLES "Build examples" OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    set(GOOGLE_TEST_DIR "" CACHE PATH "Path to GoogleTest source")
    add_subdirectory(test)
endif()

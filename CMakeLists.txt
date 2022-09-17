cmake_minimum_required(VERSION 3.1)
project(pgm_args VERSION 0.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

####################
# headers
include(GNUInstallDirs)
install(FILES args.hpp args.ipp
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

####################
# example
add_executable(example
    args.hpp args.ipp example/main.cpp
)

####################
# tests
option(BUILD_TESTS "Build tests" ON)
set(GOOGLE_TEST_DIR "" CACHE PATH "Path to GoogleTest source")

if(BUILD_TESTS)
    enable_testing()

    if(GOOGLE_TEST_DIR)
        add_subdirectory(${GOOGLE_TEST_DIR} googletest)
    else()
        find_package(GTest REQUIRED)
    endif()

    add_executable(test_args test/test_args.cpp)
    target_link_libraries(test_args GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(test_args)
endif()

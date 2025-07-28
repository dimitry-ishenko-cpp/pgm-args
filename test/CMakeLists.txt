# test

enable_testing()

if(GOOGLE_TEST_DIR)
    add_subdirectory(${GOOGLE_TEST_DIR} googletest)
else()
    find_package(GTest REQUIRED)
endif()

add_executable(test_args test_args.cpp)
target_link_libraries(test_args pgm::args GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_args)

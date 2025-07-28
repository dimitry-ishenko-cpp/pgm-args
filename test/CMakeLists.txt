# test

add_executable(test_args test_args.cpp)
target_link_libraries(test_args pgm::args GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_args)

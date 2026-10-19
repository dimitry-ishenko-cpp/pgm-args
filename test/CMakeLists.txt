# test

add_executable(test_pgm_args
    test_basic.cpp
    test_params.cpp
    test_conv.cpp
)
target_link_libraries(test_pgm_args pgm::args GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_pgm_args)

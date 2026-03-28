add_executable(genesys_test_util
    test_util.cpp
)

target_link_libraries(genesys_test_util
    PRIVATE
        genesys_kernel_util
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(genesys_test_util)

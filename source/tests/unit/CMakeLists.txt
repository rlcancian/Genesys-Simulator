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

add_executable(genesys_test_simulator_support
    test_simulator_support.cpp
)

target_link_libraries(genesys_test_simulator_support
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(genesys_test_simulator_support)

add_executable(genesys_test_simulator_runtime
    test_simulator_runtime.cpp
)

target_link_libraries(genesys_test_simulator_runtime
    PRIVATE
        genesys_kernel_simulator_runtime
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_simulator_runtime)

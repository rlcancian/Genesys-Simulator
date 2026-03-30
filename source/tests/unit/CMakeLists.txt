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


add_executable(genesys_test_support_modelmanager
    test_support_modelmanager.cpp
)

target_link_libraries(genesys_test_support_modelmanager
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_modelmanager)

add_executable(genesys_test_support_persistence
    test_support_persistence.cpp
)

target_link_libraries(genesys_test_support_persistence
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_persistence)

add_executable(genesys_test_runtime_pluginmanager
    test_runtime_pluginmanager.cpp
)

target_link_libraries(genesys_test_runtime_pluginmanager
    PRIVATE
        genesys_kernel_simulator_runtime
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_runtime_pluginmanager)

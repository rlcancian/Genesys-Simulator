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


add_executable(genesys_test_support_oneventmanager
    test_support_oneventmanager.cpp
)

target_link_libraries(genesys_test_support_oneventmanager
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_oneventmanager)

add_executable(genesys_test_support_parsermanager
    test_support_parsermanager.cpp
)

target_link_libraries(genesys_test_support_parsermanager
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_parsermanager)


add_executable(genesys_test_support_tracemanager
    test_support_tracemanager.cpp
)

target_link_libraries(genesys_test_support_tracemanager
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_tracemanager)

add_executable(genesys_test_support_connectionmanager
    test_support_connectionmanager.cpp
)

target_link_libraries(genesys_test_support_connectionmanager
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_connectionmanager)


add_executable(genesys_test_support_simulationscenario
    test_support_simulationscenario.cpp
)

target_link_libraries(genesys_test_support_simulationscenario
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_simulationscenario)

add_executable(genesys_test_support_experimentmanager
    test_support_experimentmanager.cpp
)

target_link_libraries(genesys_test_support_experimentmanager
    PRIVATE
        genesys_kernel_simulator_support
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_support_experimentmanager)

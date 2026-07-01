add_executable(genesys_smoke_simulator_start
    smoke_simulator_start.cpp
)

target_link_libraries(genesys_smoke_simulator_start
    PRIVATE
        genesys_kernel_simulator_runtime
        genesys_kernel_util
        genesys_plugins_components_minimal
        genesys_kernel_simulator_runtime
)

add_test(NAME smoke_simulator_start COMMAND genesys_smoke_simulator_start)
set_tests_properties(smoke_simulator_start PROPERTIES LABELS "smoke")

add_executable(genesys_test_continuous_system
    ../test_continuous_system.cpp
)

target_link_libraries(genesys_test_continuous_system
    PRIVATE
        genesys_kernel_util
        genesys_plugins_data
        genesys_plugins_components_minimal
        genesys_kernel_simulator_runtime
        genesys_kernel_simulator_support
)

add_test(NAME test_continuous_system COMMAND genesys_test_continuous_system)
set_tests_properties(test_continuous_system PROPERTIES LABELS "smoke")

add_executable(genesys_test_lsode
    ../test_lsode.cpp
)

target_link_libraries(genesys_test_lsode
    PRIVATE
        genesys_kernel_util
        genesys_plugins_data
        genesys_plugins_components_minimal
        genesys_kernel_simulator_runtime
        genesys_kernel_simulator_support
)

add_test(NAME test_lsode COMMAND genesys_test_lsode)
set_tests_properties(test_lsode PROPERTIES LABELS "smoke")

add_custom_target(genesys_smoke_tests
    DEPENDS genesys_smoke_simulator_start genesys_test_continuous_system genesys_test_lsode
)

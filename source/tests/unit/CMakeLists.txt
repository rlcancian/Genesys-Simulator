include(GoogleTest)

function(genesys_add_unit_test test_target test_source)
    add_executable(${test_target}
        ${test_source}
    )

    target_link_libraries(${test_target}
        PRIVATE
            ${ARGN}
            GTest::gtest_main
    )

    gtest_discover_tests(${test_target}
        PROPERTIES
            LABELS "unit"
    )

    set_property(TARGET ${test_target} PROPERTY FOLDER tests/unit)
endfunction()

genesys_add_unit_test(genesys_test_util
    test_util.cpp
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_simulator_support
    test_simulator_support.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_simulator_runtime
    test_simulator_runtime.cpp
    genesys_kernel_simulator_runtime
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_modelmanager
    test_support_modelmanager.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_persistence
    test_support_persistence.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_runtime_pluginmanager
    test_runtime_pluginmanager.cpp
    genesys_kernel_simulator_runtime
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_oneventmanager
    test_support_oneventmanager.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_parsermanager
    test_support_parsermanager.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_tracemanager
    test_support_tracemanager.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_connectionmanager
    test_support_connectionmanager.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_simulationscenario
    test_support_simulationscenario.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

genesys_add_unit_test(genesys_test_support_experimentmanager
    test_support_experimentmanager.cpp
    genesys_kernel_simulator_support
    genesys_kernel_util
)

add_custom_target(genesys_kernel_unit_tests
    DEPENDS
        genesys_test_util
        genesys_test_simulator_support
        genesys_test_simulator_runtime
        genesys_test_support_modelmanager
        genesys_test_support_persistence
        genesys_test_runtime_pluginmanager
        genesys_test_support_oneventmanager
        genesys_test_support_parsermanager
        genesys_test_support_tracemanager
        genesys_test_support_connectionmanager
        genesys_test_support_simulationscenario
        genesys_test_support_experimentmanager
)

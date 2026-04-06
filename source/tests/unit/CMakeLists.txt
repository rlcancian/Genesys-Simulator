include(GoogleTest)
find_package(Python3 COMPONENTS Interpreter REQUIRED)

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
    genesys_plugins_components_minimal
    genesys_kernel_simulator_runtime
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
    genesys_plugins_components_minimal
    genesys_kernel_simulator_runtime
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

set(GENESYS_KERNEL_SIMULATOR_HEADERS_DIR "${CMAKE_SOURCE_DIR}/source/kernel/simulator")
set(GENESYS_GENERATED_UNIT_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/generated")
set(GENESYS_METHOD_INVENTORY_TEST "${GENESYS_GENERATED_UNIT_TESTS_DIR}/test_kernel_simulator_method_inventory.generated.cpp")
set(GENESYS_METHOD_INVENTORY_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_kernel_simulator_test_stubs.py")

add_custom_command(
    OUTPUT "${GENESYS_METHOD_INVENTORY_TEST}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENESYS_GENERATED_UNIT_TESTS_DIR}"
    COMMAND ${Python3_EXECUTABLE} "${GENESYS_METHOD_INVENTORY_SCRIPT}"
            --headers-dir "${GENESYS_KERNEL_SIMULATOR_HEADERS_DIR}"
            --output "${GENESYS_METHOD_INVENTORY_TEST}"
    DEPENDS "${GENESYS_METHOD_INVENTORY_SCRIPT}"
    COMMENT "Generating kernel simulator method inventory tests"
    VERBATIM
)

add_custom_target(genesys_generate_kernel_simulator_method_tests
    DEPENDS "${GENESYS_METHOD_INVENTORY_TEST}"
)

add_executable(genesys_test_kernel_simulator_method_inventory
    "${GENESYS_METHOD_INVENTORY_TEST}"
)

add_dependencies(genesys_test_kernel_simulator_method_inventory
    genesys_generate_kernel_simulator_method_tests
)

target_link_libraries(genesys_test_kernel_simulator_method_inventory
    PRIVATE
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_kernel_simulator_method_inventory
    PROPERTIES
        LABELS "unit;autogen"
)

set_property(TARGET genesys_test_kernel_simulator_method_inventory PROPERTY FOLDER tests/unit)

add_dependencies(genesys_kernel_unit_tests genesys_test_kernel_simulator_method_inventory)

add_custom_target(genesys_kernel_unit_tests_run
    COMMAND $<TARGET_FILE:genesys_test_util>
    COMMAND $<TARGET_FILE:genesys_test_simulator_support>
    COMMAND $<TARGET_FILE:genesys_test_simulator_runtime>
    COMMAND $<TARGET_FILE:genesys_test_support_modelmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_persistence>
    COMMAND $<TARGET_FILE:genesys_test_runtime_pluginmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_oneventmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_parsermanager>
    COMMAND $<TARGET_FILE:genesys_test_support_tracemanager>
    COMMAND $<TARGET_FILE:genesys_test_support_connectionmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_simulationscenario>
    COMMAND $<TARGET_FILE:genesys_test_support_experimentmanager>
    COMMAND $<TARGET_FILE:genesys_test_kernel_simulator_method_inventory>
    DEPENDS
        genesys_kernel_unit_tests
    USES_TERMINAL
)

set_property(TARGET genesys_kernel_unit_tests_run PROPERTY FOLDER tests/unit)

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

genesys_add_unit_test(genesys_test_parser_expressions
    test_parser_expressions.cpp
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


genesys_add_unit_test(genesys_test_statistics
    test_statistics.cpp
    genesys_kernel_statistics
    genesys_kernel_util
)

find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)
if(Qt6_FOUND)
    set(GENESYS_QT_TEST_PACKAGE Qt6)
else()
    find_package(Qt5 QUIET COMPONENTS Core Gui Widgets)
    if(Qt5_FOUND)
        set(GENESYS_QT_TEST_PACKAGE Qt5)
    endif()
endif()

if(DEFINED GENESYS_QT_TEST_PACKAGE)
    add_executable(genesys_test_gui_render_strategy
        test_gui_render_strategy.cpp
        ${CMAKE_SOURCE_DIR}/source/applications/gui/qt/GenesysQtGUI/graphicals/GraphicalModelItemRenderStrategy.cpp
        ${CMAKE_SOURCE_DIR}/source/applications/gui/qt/GenesysQtGUI/systempreferences.cpp
    )

    target_include_directories(genesys_test_gui_render_strategy
        PRIVATE
            ${CMAKE_SOURCE_DIR}/source
            ${CMAKE_SOURCE_DIR}/source/applications/gui/qt/GenesysQtGUI
    )

    target_link_libraries(genesys_test_gui_render_strategy
        PRIVATE
            GTest::gtest
            ${GENESYS_QT_TEST_PACKAGE}::Core
            ${GENESYS_QT_TEST_PACKAGE}::Gui
            ${GENESYS_QT_TEST_PACKAGE}::Widgets
    )

    gtest_discover_tests(genesys_test_gui_render_strategy
        PROPERTIES
            LABELS "unit;gui"
    )

    set_property(TARGET genesys_test_gui_render_strategy PROPERTY FOLDER tests/unit)
else()
    message(STATUS "Qt was not found; skipping GUI render strategy unit test.")
endif()

add_executable(genesys_test_tools_hypothesistester
    test_tools_hypothesistester.cpp
    ${CMAKE_SOURCE_DIR}/source/tools/HypothesisTesterDefaultImpl1.cpp
    ${CMAKE_SOURCE_DIR}/source/tools/ProbabilityDistribution.cpp
    ${CMAKE_SOURCE_DIR}/source/tools/ProbabilityDistributionBase.cpp
    ${CMAKE_SOURCE_DIR}/source/tools/SolverDefaultImpl1.cpp
)

target_include_directories(genesys_test_tools_hypothesistester
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_tools_hypothesistester
    PRIVATE
        genesys_kernel_simulator_runtime
        genesys_kernel_statistics
        genesys_kernel_util
        GTest::gtest_main
)

gtest_discover_tests(genesys_test_tools_hypothesistester
    PROPERTIES
        LABELS "unit"
)

set_property(TARGET genesys_test_tools_hypothesistester PROPERTY FOLDER tests/unit)

add_custom_target(genesys_kernel_unit_tests
    DEPENDS
        genesys_test_util
        genesys_test_simulator_support
        genesys_test_simulator_runtime
        genesys_test_parser_expressions
        genesys_test_support_modelmanager
        genesys_test_support_persistence
        genesys_test_runtime_pluginmanager
        genesys_test_support_oneventmanager
        genesys_test_support_parsermanager
        genesys_test_support_tracemanager
        genesys_test_support_connectionmanager
        genesys_test_support_simulationscenario
        genesys_test_support_experimentmanager
        genesys_test_statistics
        genesys_test_tools_hypothesistester
)

if(GENESYS_BUILD_WEB_APPLICATION)
    genesys_add_unit_test(genesys_test_web_api_router
        test_web_api_router.cpp
        genesys_web_core
    )

    add_dependencies(genesys_kernel_unit_tests genesys_test_web_api_router)
endif()

if(TARGET genesys_test_gui_render_strategy)
    add_dependencies(genesys_kernel_unit_tests genesys_test_gui_render_strategy)
endif()

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
    COMMAND $<TARGET_FILE:genesys_test_parser_expressions>
    COMMAND $<TARGET_FILE:genesys_test_support_modelmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_persistence>
    COMMAND $<TARGET_FILE:genesys_test_runtime_pluginmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_oneventmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_parsermanager>
    COMMAND $<TARGET_FILE:genesys_test_support_tracemanager>
    COMMAND $<TARGET_FILE:genesys_test_support_connectionmanager>
    COMMAND $<TARGET_FILE:genesys_test_support_simulationscenario>
    COMMAND $<TARGET_FILE:genesys_test_support_experimentmanager>
    COMMAND $<TARGET_FILE:genesys_test_statistics>
    COMMAND $<TARGET_FILE:genesys_test_tools_hypothesistester>
    COMMAND $<TARGET_FILE:genesys_test_kernel_simulator_method_inventory>
    DEPENDS
        genesys_kernel_unit_tests
    USES_TERMINAL
)

if(GENESYS_BUILD_WEB_APPLICATION)
    add_custom_command(TARGET genesys_kernel_unit_tests_run
        POST_BUILD
        COMMAND $<TARGET_FILE:genesys_test_web_api_router>
    )
endif()

if(TARGET genesys_test_gui_render_strategy)
    add_custom_command(TARGET genesys_kernel_unit_tests_run
        POST_BUILD
        COMMAND $<TARGET_FILE:genesys_test_gui_render_strategy>
    )
endif()

set_property(TARGET genesys_kernel_unit_tests_run PROPERTY FOLDER tests/unit)

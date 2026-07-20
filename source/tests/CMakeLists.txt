add_subdirectory(unit)

# The AI plugin tests use only local fakes/dummy connectors and must participate
# in both the ordinary aggregate build and the direct kernel unit-test runner.
if(TARGET genesys_kernel_unit_tests AND TARGET genesys_test_ai_plugins)
    add_dependencies(genesys_kernel_unit_tests genesys_test_ai_plugins)
endif()

if(TARGET genesys_kernel_unit_tests_run AND TARGET genesys_test_ai_plugins)
    add_custom_target(genesys_test_ai_plugins_run
        COMMAND $<TARGET_FILE:genesys_test_ai_plugins>
        DEPENDS genesys_test_ai_plugins
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_ai_plugins_run)
endif()

# Characterize the legacy SolverDefaultImpl1 contracts independently of the
# broader tools library. This target intentionally links the implementation
# directly so failures can be attributed to the legacy solver itself.
include(GoogleTest)
add_executable(genesys_test_tools_legacy_solver_regression
    unit/test_tools_legacy_solver_regression.cpp
    ${CMAKE_SOURCE_DIR}/source/tools/Continuous/SolverDefaultImpl1.cpp
)

target_include_directories(genesys_test_tools_legacy_solver_regression
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_tools_legacy_solver_regression
    PRIVATE
        GTest::gtest_main
)

target_compile_features(genesys_test_tools_legacy_solver_regression PRIVATE cxx_std_23)

set_property(TARGET genesys_test_tools_legacy_solver_regression PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_tools_legacy_solver_regression
    PROPERTIES
        LABELS "unit;numerical;legacy-solver"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_tools_legacy_solver_regression)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_tools_legacy_solver_regression_run
        COMMAND $<TARGET_FILE:genesys_test_tools_legacy_solver_regression>
        DEPENDS genesys_test_tools_legacy_solver_regression
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_tools_legacy_solver_regression_run)
endif()

option(GENESYS_BUILD_SMOKE_TESTS "Build smoke tests under source/tests/smoke" ON)

if(GENESYS_BUILD_SMOKE_TESTS)
    add_subdirectory(smoke)
endif()

add_custom_target(genesys_tests
    DEPENDS
        genesys_kernel_unit_tests
)

if(GENESYS_BUILD_SMOKE_TESTS)
    add_dependencies(genesys_tests genesys_smoke_tests)
endif()

add_executable(genesys_diffusion_ascii_demo
    diffusion_demo.cpp
)

target_include_directories(genesys_diffusion_ascii_demo
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_diffusion_ascii_demo
    PRIVATE
        genesys_tools
        genesys_kernel_simulator_runtime
        genesys_kernel_util
        genesys_plugins_data
)

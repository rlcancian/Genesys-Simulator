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

# The historical Search/Remove tests dispatch into the model calendar. Keep a
# focused active suite that initializes its queue fixtures explicitly and drains
# scheduled events before asserting routed entities.
add_executable(genesys_test_search_remove_runtime
    unit/test_search_remove_runtime.cpp
)

target_include_directories(genesys_test_search_remove_runtime
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_search_remove_runtime
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_search_remove_runtime PRIVATE cxx_std_23)

set_property(TARGET genesys_test_search_remove_runtime PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_search_remove_runtime
    PROPERTIES
        LABELS "unit;runtime;search-remove"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_search_remove_runtime)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_search_remove_runtime_run
        COMMAND $<TARGET_FILE:genesys_test_search_remove_runtime>
        DEPENDS genesys_test_search_remove_runtime
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_search_remove_runtime_run)
endif()

# Queue operations are public and may occur before the model-wide related-data
# lifecycle. Verify that statistics collectors are initialized safely on demand.
add_executable(genesys_test_queue_statistics_lifecycle
    unit/test_queue_statistics_lifecycle.cpp
)

target_include_directories(genesys_test_queue_statistics_lifecycle
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_queue_statistics_lifecycle
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_queue_statistics_lifecycle PRIVATE cxx_std_23)

set_property(TARGET genesys_test_queue_statistics_lifecycle PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_queue_statistics_lifecycle
    PROPERTIES
        LABELS "unit;runtime;queue;statistics;lifecycle"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_queue_statistics_lifecycle)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_queue_statistics_lifecycle_run
        COMMAND $<TARGET_FILE:genesys_test_queue_statistics_lifecycle>
        DEPENDS genesys_test_queue_statistics_lifecycle
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_queue_statistics_lifecycle_run)
endif()

# ModalModel/Network architecture, Phase 1: DefaultNetwork core (port schema,
# activation counter, check, persistence, replication reset). See
# docs/ai_assistants/reference/GENESYS_MODAL_MODEL_NETWORK_ARCHITECTURE.md.
add_executable(genesys_test_default_network
    unit/test_default_network.cpp
)

target_include_directories(genesys_test_default_network
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_default_network
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_default_network PRIVATE cxx_std_23)

set_property(TARGET genesys_test_default_network PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_default_network
    PROPERTIES
        LABELS "unit;modalmodel;network;defaultnetwork"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_default_network)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_default_network_run
        COMMAND $<TARGET_FILE:genesys_test_default_network>
        DEPENDS genesys_test_default_network
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_default_network_run)
endif()

# ModalModel node abstraction migration: DefaultNode now owns modal-network
# metadata as a data definition instead of a process component.
add_executable(genesys_test_default_node
    unit/test_default_node.cpp
)

target_include_directories(genesys_test_default_node
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_default_node
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_default_node PRIVATE cxx_std_23)

set_property(TARGET genesys_test_default_node PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_default_node
    PROPERTIES
        LABELS "unit;modalmodel;node;defaultnode"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_default_node)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_default_node_run
        COMMAND $<TARGET_FILE:genesys_test_default_node>
        DEPENDS genesys_test_default_node
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_default_node_run)
endif()

# ModalModel/Network bridge: ModalModelDefault adapts process entities to an
# attached DefaultNetwork through input/output bindings.
add_executable(genesys_test_modal_model_default_network
    unit/test_modal_model_default_network.cpp
)

target_include_directories(genesys_test_modal_model_default_network
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_modal_model_default_network
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_modal_model_default_network PRIVATE cxx_std_23)

set_property(TARGET genesys_test_modal_model_default_network PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_modal_model_default_network
    PROPERTIES
        LABELS "unit;modalmodel;network;bridge"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_modal_model_default_network)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_modal_model_default_network_run
        COMMAND $<TARGET_FILE:genesys_test_modal_model_default_network>
        DEPENDS genesys_test_modal_model_default_network
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_modal_model_default_network_run)
endif()

# ModalModel/Network architecture, Phase 2: EFSMNetwork owns finite-state
# network state and one-step activation semantics as a data definition.
add_executable(genesys_test_efsm_network
    unit/test_efsm_network.cpp
)

target_include_directories(genesys_test_efsm_network
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_efsm_network
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_efsm_network PRIVATE cxx_std_23)

set_property(TARGET genesys_test_efsm_network PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_efsm_network
    PROPERTIES
        LABELS "unit;modalmodel;network;efsm"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_efsm_network)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_efsm_network_run
        COMMAND $<TARGET_FILE:genesys_test_efsm_network>
        DEPENDS genesys_test_efsm_network
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_efsm_network_run)
endif()

# ModalModel/Network architecture: mathematical graph networks are structural
# DefaultNetwork specializations with graph algorithms and no Entity movement
# semantics.
add_executable(genesys_test_graph_network
    unit/test_graph_network.cpp
)

target_include_directories(genesys_test_graph_network
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_graph_network
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_graph_network PRIVATE cxx_std_23)

set_property(TARGET genesys_test_graph_network PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_graph_network
    PROPERTIES
        LABELS "unit;modalmodel;network;graph"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_graph_network)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_graph_network_run
        COMMAND $<TARGET_FILE:genesys_test_graph_network>
        DEPENDS genesys_test_graph_network
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_graph_network_run)
endif()

# Station enter/leave operations are public and may occur before the model-wide
# related-data lifecycle. Characterize statistics-enabled and disabled behavior.
add_executable(genesys_test_station_statistics_lifecycle
    unit/test_station_statistics_lifecycle.cpp
)

target_include_directories(genesys_test_station_statistics_lifecycle
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_station_statistics_lifecycle
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_station_statistics_lifecycle PRIVATE cxx_std_23)

set_property(TARGET genesys_test_station_statistics_lifecycle PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_station_statistics_lifecycle
    PROPERTIES
        LABELS "unit;runtime;station;statistics;lifecycle"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_station_statistics_lifecycle)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_station_statistics_lifecycle_run
        COMMAND $<TARGET_FILE:genesys_test_station_statistics_lifecycle>
        DEPENDS genesys_test_station_statistics_lifecycle
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_station_statistics_lifecycle_run)
endif()

# Delay dispatch is a public runtime path and may occur before the model-wide
# related-data lifecycle. Characterize statistics-enabled and disabled behavior.
add_executable(genesys_test_delay_statistics_lifecycle
    unit/test_delay_statistics_lifecycle.cpp
)

target_include_directories(genesys_test_delay_statistics_lifecycle
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_delay_statistics_lifecycle
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_delay_statistics_lifecycle PRIVATE cxx_std_23)

set_property(TARGET genesys_test_delay_statistics_lifecycle PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_delay_statistics_lifecycle
    PROPERTIES
        LABELS "unit;runtime;delay;statistics;lifecycle"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_delay_statistics_lifecycle)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_delay_statistics_lifecycle_run
        COMMAND $<TARGET_FILE:genesys_test_delay_statistics_lifecycle>
        DEPENDS genesys_test_delay_statistics_lifecycle
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_delay_statistics_lifecycle_run)
endif()

# Resource seize/release operations consume statistics and cost-accounting
# objects that are currently created by the related-data lifecycle. Characterize
# enabled, disabled, and repeated-operation behavior before changing production.
add_executable(genesys_test_resource_accounting_lifecycle
    unit/test_resource_accounting_lifecycle.cpp
)

target_include_directories(genesys_test_resource_accounting_lifecycle
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_resource_accounting_lifecycle
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_resource_accounting_lifecycle PRIVATE cxx_std_23)

set_property(TARGET genesys_test_resource_accounting_lifecycle PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_resource_accounting_lifecycle
    PROPERTIES
        LABELS "unit;runtime;resource;statistics;accounting;lifecycle"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_resource_accounting_lifecycle)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_resource_accounting_lifecycle_run
        COMMAND $<TARGET_FILE:genesys_test_resource_accounting_lifecycle>
        DEPENDS genesys_test_resource_accounting_lifecycle
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_resource_accounting_lifecycle_run)
endif()

# Plugin metadata completion creates a private temporary Model. Keep this one
# focused executable sanitizer-enabled under GNU/Clang so its process exit proves
# that the temporary model and its model-owned plugin instances are released.
add_executable(genesys_test_simulator_plugin_completion_lifetime
    unit/test_simulator_plugin_completion_lifetime.cpp
)

target_include_directories(genesys_test_simulator_plugin_completion_lifetime
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_simulator_plugin_completion_lifetime
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_simulator_plugin_completion_lifetime PRIVATE cxx_std_23)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(genesys_test_simulator_plugin_completion_lifetime
        PRIVATE
            -fsanitize=address
            -fno-omit-frame-pointer
    )
    target_link_options(genesys_test_simulator_plugin_completion_lifetime
        PRIVATE
            -fsanitize=address
    )
endif()

set_property(TARGET genesys_test_simulator_plugin_completion_lifetime PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_simulator_plugin_completion_lifetime
    PROPERTIES
        LABELS "unit;runtime;plugin;ownership;lifetime;asan;lsan"
        ENVIRONMENT "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1;LSAN_OPTIONS=exitcode=23"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_simulator_plugin_completion_lifetime)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_simulator_plugin_completion_lifetime_run
        COMMAND ${CMAKE_COMMAND} -E env
            "ASAN_OPTIONS=detect_leaks=1:halt_on_error=1"
            "LSAN_OPTIONS=exitcode=23"
            $<TARGET_FILE:genesys_test_simulator_plugin_completion_lifetime>
        DEPENDS genesys_test_simulator_plugin_completion_lifetime
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_simulator_plugin_completion_lifetime_run)
endif()

# OptimizerDefaultImpl1 currently owns seven heap-allocated List wrappers. Keep
# the ownership contract explicit until a later RAII/value migration is mapped.
add_executable(genesys_test_optimizer_ownership_contract
    unit/test_optimizer_ownership_contract.cpp
)

target_include_directories(genesys_test_optimizer_ownership_contract
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_test_optimizer_ownership_contract
    PRIVATE
        GTest::gtest_main
        "$<LINK_GROUP:RESCAN,genesys_tools,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_test_optimizer_ownership_contract PRIVATE cxx_std_23)

set_property(TARGET genesys_test_optimizer_ownership_contract PROPERTY FOLDER tests/unit)

gtest_discover_tests(genesys_test_optimizer_ownership_contract
    PROPERTIES
        LABELS "unit;tools;optimizer;ownership;compile-contract"
)

if(TARGET genesys_kernel_unit_tests)
    add_dependencies(genesys_kernel_unit_tests genesys_test_optimizer_ownership_contract)
endif()

if(TARGET genesys_kernel_unit_tests_run)
    add_custom_target(genesys_test_optimizer_ownership_contract_run
        COMMAND $<TARGET_FILE:genesys_test_optimizer_ownership_contract>
        DEPENDS genesys_test_optimizer_ownership_contract
        USES_TERMINAL
    )
    add_dependencies(genesys_kernel_unit_tests_run genesys_test_optimizer_ownership_contract_run)
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

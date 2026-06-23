include(GoogleTest)

if(NOT TARGET genesys_tools_analysis)
    add_subdirectory(${CMAKE_SOURCE_DIR}/source/tools ${CMAKE_BINARY_DIR}/source/tools-for-integration-tests)
endif()

function(genesys_add_tools_integration_test test_target test_source)
    add_executable(${test_target}
        ${test_source}
    )

    target_link_libraries(${test_target}
        PRIVATE
            ${ARGN}
            GTest::gtest_main
    )

    target_compile_definitions(${test_target}
        PRIVATE
            GENESYS_REPOSITORY_ROOT="${CMAKE_SOURCE_DIR}"
    )

    gtest_discover_tests(${test_target}
        TEST_PREFIX "integration.tools."
        PROPERTIES
            LABELS "integration;tools"
    )

    set_property(TARGET ${test_target} PROPERTY FOLDER tests/integration/tools)
endfunction()

genesys_add_tools_integration_test(genesys_integration_tools_analysis_workflow
    tools/analysis/test_analysis_tool_integration.cpp
    genesys_tools_analysis
)

add_custom_target(genesys_tools_integration_tests
    DEPENDS
        genesys_integration_tools_analysis_workflow
)

add_custom_target(genesys_integration_tests
    DEPENDS
        genesys_tools_integration_tests
)

add_subdirectory(unit)

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
    diffusion_ascii_demo.cpp
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

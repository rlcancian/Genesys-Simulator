add_subdirectory(unit)
add_subdirectory(smoke)

add_custom_target(genesys_tests
    DEPENDS
        genesys_kernel_unit_tests
        genesys_smoke_simulator_start
)

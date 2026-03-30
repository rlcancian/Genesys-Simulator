add_executable(genesys_smoke_simulator_start
    smoke_simulator_start.cpp
)

target_link_libraries(genesys_smoke_simulator_start
    PRIVATE
        genesys_kernel_simulator_runtime
        genesys_kernel_util
)

add_test(NAME smoke_simulator_start COMMAND genesys_smoke_simulator_start)

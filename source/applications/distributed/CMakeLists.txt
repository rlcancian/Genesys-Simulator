add_library(genesys_distributed_core STATIC
    core/DistributedLibraryInfo.cpp
    core/Json.cpp
    core/WorkerHttpClient.cpp
    core/WorkerRegistry.cpp
    core/WorkerDiscoveryService.cpp
    core/DistributedScheduler.cpp
    core/RemoteSimulationExecutor.cpp
    core/LocalSimulationExecutor.cpp
    core/DistributedExecutionManager.cpp
)

target_include_directories(genesys_distributed_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Local execution runs an in-process simulator, so the library links the kernel runtime
# (same set the web application links). The link group is propagated to consumers.
find_package(Threads REQUIRED)

target_link_libraries(genesys_distributed_core
    PUBLIC
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
        Threads::Threads
)

target_compile_features(genesys_distributed_core PUBLIC cxx_std_23)

add_executable(genesys_distributed_app
    app/main.cpp
)

target_link_libraries(genesys_distributed_app
    PRIVATE
        genesys_distributed_core
)

target_compile_features(genesys_distributed_app PRIVATE cxx_std_23)

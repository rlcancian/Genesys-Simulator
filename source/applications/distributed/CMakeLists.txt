add_library(genesys_distributed_core STATIC
    core/DistributedLibraryInfo.cpp
    core/Json.cpp
    core/WorkerHttpClient.cpp
    core/WorkerRegistry.cpp
    core/WorkerDiscoveryService.cpp
    core/DistributedScheduler.cpp
    core/RemoteSimulationExecutor.cpp
)

target_include_directories(genesys_distributed_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}
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

add_library(genesys_kernel_simulator_support STATIC
    LicenceManager.cpp
    ParserManager.cpp
    TraceManager.cpp
    ExperimentManager.cpp
    SimulationExperiment.cpp
    ModelInfo.cpp
    Persistence.cpp
    ModelManager.cpp
)

target_include_directories(genesys_kernel_simulator_support
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_kernel_simulator_support
    PUBLIC
        genesys_kernel_util
)

target_compile_features(genesys_kernel_simulator_support PUBLIC cxx_std_20)

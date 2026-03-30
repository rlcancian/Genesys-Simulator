set(GENESYS_KERNEL_SIMULATOR_SUPPORT_SOURCES
    LicenceManager.cpp
    ParserManager.cpp
    ParserChangesInformation.cpp
    PluginInformation.cpp
    Plugin.cpp
    TraceManager.cpp
    OnEventManager.cpp
    ConnectionManager.cpp
    Event.cpp
    PropertyManager.cpp
    ExperimentManager.cpp
    SimulationExperiment.cpp
    SimulationScenario.cpp
    ModelInfo.cpp
    Persistence.cpp
    ModelManager.cpp
        ../../plugins/PluginConnectorDummyImpl1.cpp
)

add_library(genesys_kernel_simulator_support STATIC
    ${GENESYS_KERNEL_SIMULATOR_SUPPORT_SOURCES}
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

if(NOT GENESYS_BOOTSTRAP_MINIMAL_RUNTIME)
    message(FATAL_ERROR
        "GENESYS_BOOTSTRAP_MINIMAL_RUNTIME=OFF is temporarily unsupported in the current CMake graph. "
        "Enable it (ON) until plugin component targets are integrated into the runtime link path."
    )
endif()

file(GLOB GENESYS_KERNEL_SIMULATOR_RUNTIME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

list(REMOVE_ITEM GENESYS_KERNEL_SIMULATOR_RUNTIME_SOURCES
    "../../plugins/PluginConnectorDummyImpl1.cpp"
)

add_library(genesys_kernel_simulator_runtime STATIC
    ${GENESYS_KERNEL_SIMULATOR_RUNTIME_SOURCES}
)

target_include_directories(genesys_kernel_simulator_runtime
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_kernel_simulator_runtime
    PUBLIC
        genesys_kernel_util
        genesys_kernel_statistics
        genesys_parser
        genesys_plugins_data
)

target_compile_features(genesys_kernel_simulator_runtime PUBLIC cxx_std_20)

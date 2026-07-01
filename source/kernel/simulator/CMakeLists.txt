file(GLOB GENESYS_KERNEL_SIMULATOR_MODEL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/model/*.cpp"
)

file(GLOB GENESYS_KERNEL_SIMULATOR_ESSENTIAL_PLUGINS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/essentialPlugins/*.cpp"
)

file(GLOB GENESYS_KERNEL_SIMULATOR_PERSISTENCE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/persistence/*.cpp"
)

set(GENESYS_KERNEL_SIMULATOR_SUPPORT_SOURCES
    LicenceManager.cpp
    ParserManager.cpp
    ParserManagerRuntime.cpp
    ParserChangesInformation.cpp
    PluginInformation.cpp
    SystemDependencyResolver.cpp
    Plugin.cpp
    TraceManager.cpp
    OnEventManager.cpp
    ConnectionManager.cpp
    Event.cpp
    ExperimentManager.cpp
    SimulationExperiment.cpp
    SimulationScenario.cpp
    Persistence.cpp
)

list(APPEND GENESYS_KERNEL_SIMULATOR_SUPPORT_SOURCES
    ${GENESYS_KERNEL_SIMULATOR_MODEL_SOURCES}
    ${GENESYS_KERNEL_SIMULATOR_ESSENTIAL_PLUGINS_SOURCES}
    ${GENESYS_KERNEL_SIMULATOR_PERSISTENCE_SOURCES}
    ParserDefaultImpl2.cpp
    SimulationReporterDefaultImpl1.cpp
    SourceModelComponent.cpp
    SinkModelComponent.cpp
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

target_compile_features(genesys_kernel_simulator_support PUBLIC cxx_std_23)

file(GLOB GENESYS_KERNEL_SIMULATOR_RUNTIME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

list(REMOVE_ITEM GENESYS_KERNEL_SIMULATOR_RUNTIME_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/PluginConnectorDummyBootstrap.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Persistence.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ParserDefaultImpl2.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SimulationReporterDefaultImpl1.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SourceModelComponent.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/SinkModelComponent.cpp"
)

list(APPEND GENESYS_KERNEL_SIMULATOR_RUNTIME_SOURCES
        "${CMAKE_SOURCE_DIR}/source/plugins/PluginConnectorStaticImpl1.cpp"
        "${CMAKE_SOURCE_DIR}/source/plugins/PluginConnectorDummyImpl1.cpp"
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
        genesys_plugins_data
        genesys_plugins_components_minimal
        genesys_kernel_simulator_support
        genesys_kernel_statistics
        genesys_parser
)

target_compile_features(genesys_kernel_simulator_runtime PUBLIC cxx_std_23)

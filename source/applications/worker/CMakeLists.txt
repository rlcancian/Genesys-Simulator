include(GNUInstallDirs)

add_library(genesys_worker_core STATIC
    api/ApiRouter.cpp
    auth/TokenService.cpp
    http/SimpleHttpServer.cpp
    service/WebWorkerRuntime.cpp
    service/SimulatorSessionService.cpp
    session/SessionManager.cpp
    worker/WorkerJobManager.cpp
)

target_include_directories(genesys_worker_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(genesys_worker_core
    PRIVATE
        genesys_kernel_simulator_runtime
)

target_compile_features(genesys_worker_core PUBLIC cxx_std_23)

add_executable(genesys_worker_app
    main.cpp
    BaseGenesysWebApplication.cpp
)

target_link_libraries(genesys_worker_app
    PRIVATE
        "$<LINK_GROUP:RESCAN,genesys_worker_core,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_worker_app PRIVATE cxx_std_23)

set_target_properties(genesys_worker_app
    PROPERTIES
        OUTPUT_NAME genesys-worker
)

install(TARGETS genesys_worker_app
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/genesys/system/bin
)

add_executable(genesys_worker ALIAS genesys_worker_app)

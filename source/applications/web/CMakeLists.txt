add_library(genesys_web_core STATIC
    api/ApiRouter.cpp
    auth/TokenService.cpp
    http/SimpleHttpServer.cpp
    service/SimulatorSessionService.cpp
    session/SessionManager.cpp
)

target_include_directories(genesys_web_core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(genesys_web_core
    PRIVATE
        genesys_kernel_simulator_runtime
)

target_compile_features(genesys_web_core PUBLIC cxx_std_23)

add_executable(genesys_webhook
    main.cpp
    BaseGenesysWebApplication.cpp
)

target_link_libraries(genesys_webhook
    PRIVATE
        genesys_web_core
)

target_compile_features(genesys_webhook PRIVATE cxx_std_23)

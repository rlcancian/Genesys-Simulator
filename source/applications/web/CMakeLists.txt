add_executable(genesys_webhook
    main.cpp
    BaseGenesysWebApplication.cpp
    SimpleHttpServer.cpp
)

target_include_directories(genesys_webhook
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_webhook
    PRIVATE
        genesys_kernel_simulator_runtime
)

target_compile_features(genesys_webhook PRIVATE cxx_std_20)

set(GENESYS_TERMINAL_APPLICATION_SOURCES
    main.cpp
    GenesysShell/GenesysShell.cpp
    examples/smarts/Smart_SeizeDelayReleaseMany.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Create.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Dispose.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Seize.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/SeizableItem.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/QueueableItem.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Delay.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Release.cpp
)

add_executable(genesys_terminal_application
    ${GENESYS_TERMINAL_APPLICATION_SOURCES}
)

target_include_directories(genesys_terminal_application
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_terminal_application
    PRIVATE
        genesys_kernel_util
        genesys_kernel_statistics
        genesys_parser
        genesys_plugins_data
        genesys_kernel_simulator_support
        genesys_kernel_simulator_runtime
)

target_compile_features(genesys_terminal_application PRIVATE cxx_std_20)

add_dependencies(genesys_terminal_application genesys_kernel)

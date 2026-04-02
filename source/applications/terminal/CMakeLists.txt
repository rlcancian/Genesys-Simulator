option(GENESYS_TERMINAL_BUILD_ALL_SOURCES "Build all terminal examples and sources recursively" OFF)

if(GENESYS_TERMINAL_BUILD_ALL_SOURCES)
    file(GLOB_RECURSE GENESYS_TERMINAL_APPLICATION_LOCAL_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    )

    set(GENESYS_TERMINAL_APPLICATION_SOURCES
        ${GENESYS_TERMINAL_APPLICATION_LOCAL_SOURCES}
        ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
    )
else()
    set(GENESYS_TERMINAL_APPLICATION_SOURCES
        main.cpp
        GenesysShell/GenesysShell.cpp
        examples/smarts/Smart_SeizeDelayReleaseMany.cpp
        examples/teaching/AnElectronicAssemblyAndTestSystem.cpp
        ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
    )
endif()

add_executable(genesys_terminal_application
    ${GENESYS_TERMINAL_APPLICATION_SOURCES}
)

target_include_directories(genesys_terminal_application
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_definitions(genesys_terminal_application
    PRIVATE
        GENESYS_TERMINAL_USE_TRAITS_APP=1
)

target_link_libraries(genesys_terminal_application
    PRIVATE
        genesys_kernel_util
        genesys_kernel_statistics
        genesys_parser
        genesys_plugins_data
        genesys_plugins_components_minimal
        genesys_kernel_simulator_support
        genesys_kernel_simulator_runtime
)

target_compile_features(genesys_terminal_application PRIVATE cxx_std_23)

add_dependencies(genesys_terminal_application genesys_kernel)

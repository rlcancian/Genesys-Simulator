include(GNUInstallDirs)

add_executable(genesys_shell
    main.cpp
    GenesysShell.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
)

target_include_directories(genesys_shell
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(genesys_shell PRIVATE cxx_std_23)

target_compile_definitions(genesys_shell
    PRIVATE
        GENESYS_TERMINAL_APP_CLASS=GenesysShell
        GENESYS_TERMINAL_APP_HEADER="GenesysShell.h"
)

target_link_libraries(genesys_shell
    PRIVATE
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

add_dependencies(genesys_shell genesys_kernel)

# Preserve the historical build-tree executable name used by focused CI while
# installing the public runtime name expected by the dispatcher contract.
install(PROGRAMS "$<TARGET_FILE:genesys_shell>"
    DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/genesys/system/bin
    RENAME genesys-shell
)

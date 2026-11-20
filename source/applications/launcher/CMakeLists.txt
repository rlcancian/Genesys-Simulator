include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Network Widgets)

set(GENESYS_LAUNCHER_VERSION "0.0.0" CACHE STRING
    "Version of the stable GenESyS launcher/dispatcher contract")
set(GENESYS_SYSTEM_PACKAGE_VERSION "0.0.0" CACHE STRING
    "Version of the GenESyS system package providing the launcher")
set(GENESYS_SYSTEM_RUNTIME_ROOT "${CMAKE_INSTALL_FULL_LIBEXECDIR}/genesys/system" CACHE PATH
    "System fallback runtime root used by the GenESyS dispatcher")
set(GENESYS_SYSTEM_CONFIG_PATH "/etc/genesys/update.conf" CACHE FILEPATH
    "System-wide GenESyS update policy path")
set(GENESYS_UPDATE_KEYRING_PATH "${CMAKE_INSTALL_FULL_DATADIR}/genesys/keys/update.gpg" CACHE FILEPATH
    "OpenPGP keyring used by gpgv for GenESyS runtime updates")

add_library(genesys_launcher_runtime STATIC
    Version.cpp
    RuntimePaths.cpp
    LauncherConfig.cpp
    PlatformInfo.cpp
    LauncherLogger.cpp
    RuntimeSelector.cpp
    ProcessLauncher.cpp
    DispatchController.cpp
)

target_include_directories(genesys_launcher_runtime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(genesys_launcher_runtime PUBLIC Qt6::Core)
target_compile_features(genesys_launcher_runtime PUBLIC cxx_std_23)
target_compile_definitions(genesys_launcher_runtime
    PUBLIC
        GENESYS_LAUNCHER_VERSION="${GENESYS_LAUNCHER_VERSION}"
        GENESYS_SYSTEM_PACKAGE_VERSION="${GENESYS_SYSTEM_PACKAGE_VERSION}"
        GENESYS_SYSTEM_RUNTIME_ROOT_DEFAULT="${GENESYS_SYSTEM_RUNTIME_ROOT}"
        GENESYS_SYSTEM_CONFIG_PATH_DEFAULT="${GENESYS_SYSTEM_CONFIG_PATH}"
        GENESYS_UPDATE_KEYRING_PATH_DEFAULT="${GENESYS_UPDATE_KEYRING_PATH}"
)

add_library(genesys_launcher_update STATIC
    NetworkTransport.cpp
    UpdateManifest.cpp
    UpdateChecker.cpp
    UpdateDownloader.cpp
    UpdateVerifier.cpp
    ArchiveExtractor.cpp
    UserRuntimeManager.cpp
    LauncherController.cpp
)

target_include_directories(genesys_launcher_update PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(genesys_launcher_update
    PUBLIC
        genesys_launcher_runtime
        Qt6::Core
        Qt6::Network
)
target_compile_features(genesys_launcher_update PUBLIC cxx_std_23)

add_executable(genesys-dispatch
    main_dispatch.cpp
)
target_link_libraries(genesys-dispatch PRIVATE genesys_launcher_runtime Qt6::Core)
target_compile_features(genesys-dispatch PRIVATE cxx_std_23)

add_executable(genesys-launcher
    main_launcher.cpp
    UpdateDialog.cpp
)
target_link_libraries(genesys-launcher
    PRIVATE
        genesys_launcher_runtime
        genesys_launcher_update
        Qt6::Core
        Qt6::Network
        Qt6::Widgets
)
target_compile_features(genesys-launcher PRIVATE cxx_std_23)

install(TARGETS genesys-launcher genesys-dispatch
    RUNTIME DESTINATION ${CMAKE_INSTALL_LIBEXECDIR}/genesys
)

# Public entry points are deliberately tiny wrappers.  They are generated from
# the configured install prefix so a staged CMake install and the Debian
# package use the same launcher/dispatcher contract.
set(_genesys_wrapper_dir "${CMAKE_CURRENT_BINARY_DIR}/wrappers")
file(MAKE_DIRECTORY "${_genesys_wrapper_dir}")
set(GENESYS_LAUNCHER_EXECUTABLE_PATH "${CMAKE_INSTALL_FULL_LIBEXECDIR}/genesys/genesys-launcher")
set(GENESYS_DISPATCH_EXECUTABLE_PATH "${CMAKE_INSTALL_FULL_LIBEXECDIR}/genesys/genesys-dispatch")

set(GENESYS_WRAPPER_APPLICATION "genesys-gui")
configure_file(
    wrappers/launcher-wrapper.in
    "${_genesys_wrapper_dir}/genesys-gui"
    @ONLY
)

foreach(_application IN ITEMS genesys-shell genesys-worker genesys-web genesys-mcp)
    set(GENESYS_WRAPPER_APPLICATION "${_application}")
    configure_file(
        wrappers/dispatch-wrapper.in
        "${_genesys_wrapper_dir}/${_application}"
        @ONLY
    )
endforeach()

install(PROGRAMS
    "${_genesys_wrapper_dir}/genesys-gui"
    "${_genesys_wrapper_dir}/genesys-shell"
    "${_genesys_wrapper_dir}/genesys-worker"
    "${_genesys_wrapper_dir}/genesys-web"
    "${_genesys_wrapper_dir}/genesys-mcp"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# The Debian package owns the administrative policy.  Installing this file via
# CMake keeps the generic staging tree and debhelper's DESTDIR view identical.
install(FILES "${CMAKE_SOURCE_DIR}/packaging/linux/update.conf"
    DESTINATION "${CMAKE_INSTALL_FULL_SYSCONFDIR}/genesys"
)

if(GENESYS_BUILD_LAUNCHER_TESTS)
    add_subdirectory(tests)
endif()

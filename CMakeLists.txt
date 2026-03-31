cmake_minimum_required(VERSION 3.24)

project(GenesysTerminalSingleBuild LANGUAGES CXX)

option(GENESYS_BUILD_TERMINAL_MAIN "Build only source/applications/terminal/main.cpp executable" ON)

if(NOT GENESYS_BUILD_TERMINAL_MAIN)
    message(FATAL_ERROR "GENESYS_BUILD_TERMINAL_MAIN must remain ON in the emergency single-build setup")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB GENESYS_KERNEL_UTIL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/kernel/util/*.cpp"
)
file(GLOB GENESYS_KERNEL_STATISTICS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/kernel/statistics/*.cpp"
)
file(GLOB GENESYS_KERNEL_SIMULATOR_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/kernel/simulator/*.cpp"
)
file(GLOB GENESYS_PARSER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/parser/*.cpp"
)
file(GLOB GENESYS_PLUGINS_DATA_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/source/plugins/data/*.cpp"
)

set(GENESYS_TERMINAL_ONLY_SOURCES
    ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/terminal/main.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/terminal/GenesysShell/GenesysShell.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/terminal/examples/smarts/Smart_SeizeDelayReleaseMany.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Create.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Dispose.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Seize.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/SeizableItem.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/QueueableItem.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Delay.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/components/Release.cpp
    ${CMAKE_SOURCE_DIR}/source/plugins/PluginConnectorStaticImpl1.cpp
)

add_executable(genesys_terminal_main
    ${GENESYS_KERNEL_UTIL_SOURCES}
    ${GENESYS_KERNEL_STATISTICS_SOURCES}
    ${GENESYS_KERNEL_SIMULATOR_SOURCES}
    ${GENESYS_PARSER_SOURCES}
    ${GENESYS_PLUGINS_DATA_SOURCES}
    ${GENESYS_TERMINAL_ONLY_SOURCES}
)

target_include_directories(genesys_terminal_main
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_features(genesys_terminal_main PRIVATE cxx_std_23)

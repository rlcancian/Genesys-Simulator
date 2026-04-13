cmake_minimum_required(VERSION 3.24)

project(GenesysSimulator LANGUAGES CXX)

if(DEFINED GENESYS_TERMINAL_APPLICATION AND NOT DEFINED GENESYS_BUILD_TERMINAL_APPLICATION)
    message(DEPRECATION "GENESYS_TERMINAL_APPLICATION is deprecated. Use GENESYS_BUILD_TERMINAL_APPLICATION instead.")
    set(GENESYS_BUILD_TERMINAL_APPLICATION_DEFAULT "${GENESYS_TERMINAL_APPLICATION}")
else()
    set(GENESYS_BUILD_TERMINAL_APPLICATION_DEFAULT ON)
endif()

option(GENESYS_BUILD_TERMINAL_APPLICATION "Build terminal application target" ${GENESYS_BUILD_TERMINAL_APPLICATION_DEFAULT})
option(GENESYS_BUILD_GUI_APPLICATION "Build Qt GUI application" OFF)
option(GENESYS_BUILD_WEB_APPLICATION "Build web webhook application" OFF)
option(GENESYS_BUILD_TESTS "Build tests" ON)
option(GENESYS_BUILD_KERNEL "Build kernel static libraries" ON)
option(GENESYS_BUILD_PARSER "Build parser static library" ON)
option(GENESYS_BUILD_PLUGINS "Build plugins static libraries" ON)
option(GENESYS_BUILD_TOOLS "Build tools static libraries" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

message(STATUS "Configuring GenesysSimulator")
message(STATUS "GENESYS_BUILD_GUI=${GENESYS_BUILD_GUI_APPLICATION}")
message(STATUS "GENESYS_BUILD_TESTS=${GENESYS_BUILD_TESTS}")
message(STATUS "GENESYS_BUILD_PLUGINS=${GENESYS_BUILD_PLUGINS} (STATIC)")
message(STATUS "GENESYS_BUILD_WEB=${GENESYS_BUILD_WEB_APPLICATION}")
message(STATUS "GENESYS_BUILD_TERMINAL=${GENESYS_BUILD_TERMINAL_APPLICATION}")
message(STATUS "GENESYS_BUILD_KERNEL=${GENESYS_BUILD_KERNEL} (STATIC)")
message(STATUS "GENESYS_BUILD_PARSER=${GENESYS_BUILD_PARSER} (STATIC)")


if(GENESYS_BUILD_KERNEL)
    add_subdirectory(source/kernel/util)
    add_subdirectory(source/kernel/statistics)
    add_subdirectory(source/kernel/simulator)
endif()

if(GENESYS_BUILD_PLUGINS)
    add_subdirectory(source/plugins)
endif()

if(GENESYS_BUILD_PARSER)
    add_subdirectory(source/parser)
endif()

if(GENESYS_BUILD_TERMINAL_APPLICATION)
    add_subdirectory(source/applications/terminal)
endif()

if(GENESYS_BUILD_KERNEL AND GENESYS_BUILD_PARSER AND GENESYS_BUILD_PLUGINS)
    add_custom_target(genesys_kernel
        DEPENDS
            genesys_kernel_util
            genesys_kernel_statistics
            genesys_parser
            genesys_plugins_data
            genesys_plugins_components_minimal
            genesys_kernel_simulator_support
            genesys_kernel_simulator_runtime
    )
endif()

if(GENESYS_BUILD_WEB_APPLICATION)
    add_subdirectory(source/applications/web)
endif()

if(GENESYS_BUILD_GUI_APPLICATION)
    add_subdirectory(source/applications/gui/qt/GenesysQtGUI)
endif()

if(GENESYS_BUILD_TOOLS)
    add_subdirectory(source/tools)
endif()

if(GENESYS_BUILD_TESTS)
    find_package(GTest CONFIG QUIET)
    if(GTest_FOUND)
        if(TARGET GTest::gtest AND TARGET GTest::gtest_main)
            message(STATUS "Using system GoogleTest package (targets: GTest::gtest, GTest::gtest_main).")
        elseif(TARGET GTest::GTest AND TARGET GTest::Main)
            message(STATUS "Using system GoogleTest package (targets: GTest::GTest, GTest::Main). Creating compatibility aliases.")
            add_library(GTest::gtest ALIAS GTest::GTest)
            add_library(GTest::gtest_main ALIAS GTest::Main)
        else()
            message(WARNING "GTest package was found, but expected targets are missing. Falling back to bundled source/gtest.")
            set(GTest_FOUND OFF)
        endif()
    endif()

    if(NOT GTest_FOUND)
        message(STATUS "GTest package not found. Falling back to bundled source/gtest.")
        add_library(gtest STATIC source/gtest/src/gtest-all.cc)
        target_include_directories(gtest
            PUBLIC
                ${CMAKE_SOURCE_DIR}/source/gtest
        )
        target_compile_features(gtest PUBLIC cxx_std_23)

        add_library(gtest_main STATIC source/gtest/src/gtest_main.cc)
        target_link_libraries(gtest_main PUBLIC gtest)

        add_library(GTest::gtest ALIAS gtest)
        add_library(GTest::gtest_main ALIAS gtest_main)
    endif()

    add_subdirectory(source/tests)

    if(TARGET genesys_kernel)
        add_dependencies(genesys_tests genesys_kernel)
    endif()
endif()

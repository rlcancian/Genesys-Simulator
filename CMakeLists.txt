cmake_minimum_required(VERSION 3.24)

project(GenesysSimulator LANGUAGES CXX)

option(GENESYS_BUILD_GUI "Build Qt GUI application" OFF)
option(GENESYS_BUILD_TESTS "Build tests" ON)
option(GENESYS_BUILD_PLUGINS "Build plugins as shared libraries" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

message(STATUS "Configuring GenesysSimulator")
message(STATUS "GENESYS_BUILD_GUI=${GENESYS_BUILD_GUI}")
message(STATUS "GENESYS_BUILD_TESTS=${GENESYS_BUILD_TESTS}")
message(STATUS "GENESYS_BUILD_PLUGINS=${GENESYS_BUILD_PLUGINS}")

# Próxima etapa:
# - add_subdirectory(source/kernel)
# - add_subdirectory(source/parser)
# - add_subdirectory(source/tools)
# - add_subdirectory(source/applications/terminal)
# - add_subdirectory(source/tests)
# - add_subdirectory(source/applications/gui/qt/GenesysQtGUI)
# - add_subdirectory(source/plugins)

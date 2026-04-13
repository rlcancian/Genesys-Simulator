set(GENESYS_TERMINAL_APP "GenesysShell" CACHE STRING "Terminal application class to compile as the executable behavior")
set_property(CACHE GENESYS_TERMINAL_APP PROPERTY STRINGS
    GenesysShell
    Smart_ModalModelFSM
    Smart_SeizeDelayRelease
    Smart_SeizeDelayReleaseMany
    Smart_SeizeDelayReleaseNoDataDefs
    AnElectronicAssemblyAndTestSystem
)
set(GENESYS_TERMINAL_EXAMPLE "smarts/Smart_AssignWriteSeizes.cpp" CACHE STRING "Relative .cpp path under source/applications/terminal/examples to compile as terminal application")
set(GENESYS_TERMINAL_EXAMPLE_CLASS "" CACHE STRING "Optional class name override for GENESYS_TERMINAL_EXAMPLE")

set(GENESYS_TERMINAL_APPLICATION_SOURCES
    main.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
)

set(GENESYS_TERMINAL_SELECTED_CLASS "${GENESYS_TERMINAL_APP}")
set(GENESYS_TERMINAL_SELECTED_HEADER "GenesysShell/GenesysShell.h")

if(GENESYS_TERMINAL_EXAMPLE)
    string(REGEX REPLACE "^examples/" "" GENESYS_TERMINAL_EXAMPLE_RELATIVE "${GENESYS_TERMINAL_EXAMPLE}")

    if(GENESYS_TERMINAL_EXAMPLE_RELATIVE MATCHES "(^|/)\\.\\.(/|$)")
        message(FATAL_ERROR "GENESYS_TERMINAL_EXAMPLE must stay inside source/applications/terminal/examples")
    endif()
    if(NOT GENESYS_TERMINAL_EXAMPLE_RELATIVE MATCHES "\\.cpp$")
        message(FATAL_ERROR "GENESYS_TERMINAL_EXAMPLE must point to a .cpp file relative to source/applications/terminal/examples")
    endif()

    set(GENESYS_TERMINAL_EXAMPLE_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/examples/${GENESYS_TERMINAL_EXAMPLE_RELATIVE}")
    string(REGEX REPLACE "\\.cpp$" ".h" GENESYS_TERMINAL_EXAMPLE_HEADER_RELATIVE "${GENESYS_TERMINAL_EXAMPLE_RELATIVE}")
    set(GENESYS_TERMINAL_EXAMPLE_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/examples/${GENESYS_TERMINAL_EXAMPLE_HEADER_RELATIVE}")

    if(NOT EXISTS "${GENESYS_TERMINAL_EXAMPLE_SOURCE}")
        message(FATAL_ERROR "Selected terminal example source does not exist: ${GENESYS_TERMINAL_EXAMPLE_SOURCE}")
    endif()
    if(NOT EXISTS "${GENESYS_TERMINAL_EXAMPLE_HEADER}")
        message(FATAL_ERROR "Selected terminal example header does not exist: ${GENESYS_TERMINAL_EXAMPLE_HEADER}")
    endif()

    get_filename_component(GENESYS_TERMINAL_EXAMPLE_STEM "${GENESYS_TERMINAL_EXAMPLE_SOURCE}" NAME_WE)
    if(GENESYS_TERMINAL_EXAMPLE_CLASS)
        set(GENESYS_TERMINAL_SELECTED_CLASS "${GENESYS_TERMINAL_EXAMPLE_CLASS}")
    else()
        set(GENESYS_TERMINAL_SELECTED_CLASS "${GENESYS_TERMINAL_EXAMPLE_STEM}")
    endif()
    set(GENESYS_TERMINAL_SELECTED_HEADER "examples/${GENESYS_TERMINAL_EXAMPLE_HEADER_RELATIVE}")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        "${GENESYS_TERMINAL_EXAMPLE_SOURCE}"
    )
elseif(GENESYS_TERMINAL_APP STREQUAL "GenesysShell")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        GenesysShell/GenesysShell.cpp
    )
elseif(GENESYS_TERMINAL_APP STREQUAL "Smart_ModalModelFSM")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        examples/smarts/Smart_ModalModelFSM.cpp
    )
elseif(GENESYS_TERMINAL_APP STREQUAL "Smart_SeizeDelayRelease")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        examples/smarts/Smart_SeizeDelayRelease.cpp
    )
elseif(GENESYS_TERMINAL_APP STREQUAL "Smart_SeizeDelayReleaseMany")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        examples/smarts/Smart_SeizeDelayReleaseMany.cpp
    )
elseif(GENESYS_TERMINAL_APP STREQUAL "Smart_SeizeDelayReleaseNoDataDefs")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        examples/smarts/Smart_SeizeDelayReleaseNoDataDefs.cpp
    )
elseif(GENESYS_TERMINAL_APP STREQUAL "AnElectronicAssemblyAndTestSystem")
    list(APPEND GENESYS_TERMINAL_APPLICATION_SOURCES
        examples/teaching/AnElectronicAssemblyAndTestSystem.cpp
    )
else()
    message(FATAL_ERROR "Unsupported GENESYS_TERMINAL_APP='${GENESYS_TERMINAL_APP}'. Choose one of the values listed in the cache property.")
endif()

message(STATUS "GENESYS_TERMINAL_APP=${GENESYS_TERMINAL_APP}")
message(STATUS "GENESYS_TERMINAL_EXAMPLE=${GENESYS_TERMINAL_EXAMPLE}")
message(STATUS "GENESYS_TERMINAL_SELECTED_CLASS=${GENESYS_TERMINAL_SELECTED_CLASS}")
message(STATUS "GENESYS_TERMINAL_SELECTED_HEADER=${GENESYS_TERMINAL_SELECTED_HEADER}")

add_executable(genesys_terminal_application
    ${GENESYS_TERMINAL_APPLICATION_SOURCES}
)

target_include_directories(genesys_terminal_application
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_definitions(genesys_terminal_application
    PRIVATE
        GENESYS_TERMINAL_APP_CLASS=${GENESYS_TERMINAL_SELECTED_CLASS}
        GENESYS_TERMINAL_APP_HEADER="${GENESYS_TERMINAL_SELECTED_HEADER}"
)

target_link_libraries(genesys_terminal_application
    PRIVATE
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_terminal_application PRIVATE cxx_std_23)

add_dependencies(genesys_terminal_application genesys_kernel)

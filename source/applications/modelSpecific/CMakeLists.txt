set(GENESYS_MODELSPECIFIC_APP "" CACHE STRING "Relative .cpp path under source/applications/modelSpecific to compile as a model-specific application")
set(GENESYS_MODELSPECIFIC_APP_CLASS "" CACHE STRING "Optional class name override for GENESYS_MODELSPECIFIC_APP")
set(GENESYS_MODELSPECIFIC_APP_HEADER "" CACHE STRING "Optional header path override for GENESYS_MODELSPECIFIC_APP, relative to source/applications/modelSpecific")

set(GENESYS_TERMINAL_EXAMPLE "" CACHE STRING "Deprecated alias for GENESYS_MODELSPECIFIC_APP")
set(GENESYS_TERMINAL_EXAMPLE_CLASS "" CACHE STRING "Deprecated alias for GENESYS_MODELSPECIFIC_APP_CLASS")

if(GENESYS_TERMINAL_EXAMPLE AND NOT GENESYS_MODELSPECIFIC_APP)
    message(DEPRECATION "GENESYS_TERMINAL_EXAMPLE is deprecated. Use GENESYS_MODELSPECIFIC_APP instead.")
    set(GENESYS_MODELSPECIFIC_APP "${GENESYS_TERMINAL_EXAMPLE}")
endif()

if(GENESYS_TERMINAL_EXAMPLE_CLASS AND NOT GENESYS_MODELSPECIFIC_APP_CLASS)
    message(DEPRECATION "GENESYS_TERMINAL_EXAMPLE_CLASS is deprecated. Use GENESYS_MODELSPECIFIC_APP_CLASS instead.")
    set(GENESYS_MODELSPECIFIC_APP_CLASS "${GENESYS_TERMINAL_EXAMPLE_CLASS}")
endif()

set(GENESYS_MODELSPECIFIC_APP_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
)

set(GENESYS_MODELSPECIFIC_SELECTED_CLASS "")
set(GENESYS_MODELSPECIFIC_SELECTED_HEADER "")

if(GENESYS_MODELSPECIFIC_APP)
    set(GENESYS_MODELSPECIFIC_APP_RELATIVE "${GENESYS_MODELSPECIFIC_APP}")

    if(GENESYS_MODELSPECIFIC_APP_RELATIVE MATCHES "(^|/)\\.\\.(/|$)")
        message(FATAL_ERROR "GENESYS_MODELSPECIFIC_APP must stay inside source/applications/modelSpecific")
    endif()
    if(NOT GENESYS_MODELSPECIFIC_APP_RELATIVE MATCHES "\\.cpp$")
        message(FATAL_ERROR "GENESYS_MODELSPECIFIC_APP must point to a .cpp file relative to source/applications/modelSpecific")
    endif()

    set(GENESYS_MODELSPECIFIC_APP_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${GENESYS_MODELSPECIFIC_APP_RELATIVE}")
    if(GENESYS_MODELSPECIFIC_APP_HEADER)
        set(GENESYS_MODELSPECIFIC_APP_HEADER_RELATIVE "${GENESYS_MODELSPECIFIC_APP_HEADER}")
    else()
        string(REGEX REPLACE "\\.cpp$" ".h" GENESYS_MODELSPECIFIC_APP_HEADER_RELATIVE "${GENESYS_MODELSPECIFIC_APP_RELATIVE}")
    endif()

    if(GENESYS_MODELSPECIFIC_APP_HEADER_RELATIVE MATCHES "(^|/)\\.\\.(/|$)")
        message(FATAL_ERROR "GENESYS_MODELSPECIFIC_APP_HEADER must stay inside source/applications/modelSpecific")
    endif()
    set(GENESYS_MODELSPECIFIC_APP_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${GENESYS_MODELSPECIFIC_APP_HEADER_RELATIVE}")

    if(NOT EXISTS "${GENESYS_MODELSPECIFIC_APP_SOURCE}")
        message(FATAL_ERROR "Selected model-specific application source does not exist: ${GENESYS_MODELSPECIFIC_APP_SOURCE}")
    endif()
    if(NOT EXISTS "${GENESYS_MODELSPECIFIC_APP_HEADER_PATH}")
        message(FATAL_ERROR "Selected model-specific application header does not exist: ${GENESYS_MODELSPECIFIC_APP_HEADER_PATH}")
    endif()

    get_filename_component(GENESYS_MODELSPECIFIC_APP_STEM "${GENESYS_MODELSPECIFIC_APP_SOURCE}" NAME_WE)
    if(GENESYS_MODELSPECIFIC_APP_CLASS)
        set(GENESYS_MODELSPECIFIC_SELECTED_CLASS "${GENESYS_MODELSPECIFIC_APP_CLASS}")
    else()
        set(GENESYS_MODELSPECIFIC_SELECTED_CLASS "${GENESYS_MODELSPECIFIC_APP_STEM}")
    endif()
    set(GENESYS_MODELSPECIFIC_SELECTED_HEADER "applications/modelSpecific/${GENESYS_MODELSPECIFIC_APP_HEADER_RELATIVE}")
    list(APPEND GENESYS_MODELSPECIFIC_APP_SOURCES
        "${GENESYS_MODELSPECIFIC_APP_SOURCE}"
    )
else()
    message(FATAL_ERROR "GENESYS_MODELSPECIFIC_APP must be set to a model-specific example .cpp file.")
endif()

message(STATUS "GENESYS_MODELSPECIFIC_APP=${GENESYS_MODELSPECIFIC_APP}")
message(STATUS "GENESYS_MODELSPECIFIC_SELECTED_CLASS=${GENESYS_MODELSPECIFIC_SELECTED_CLASS}")
message(STATUS "GENESYS_MODELSPECIFIC_SELECTED_HEADER=${GENESYS_MODELSPECIFIC_SELECTED_HEADER}")

add_executable(genesys_modelspecific_app
    ${GENESYS_MODELSPECIFIC_APP_SOURCES}
)

target_include_directories(genesys_modelspecific_app
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_definitions(genesys_modelspecific_app
    PRIVATE
        GENESYS_TERMINAL_APP_CLASS=${GENESYS_MODELSPECIFIC_SELECTED_CLASS}
        GENESYS_TERMINAL_APP_HEADER="${GENESYS_MODELSPECIFIC_SELECTED_HEADER}"
)

target_link_libraries(genesys_modelspecific_app
    PRIVATE
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_modelspecific_app PRIVATE cxx_std_23)
add_dependencies(genesys_modelspecific_app genesys_kernel)

add_custom_command(
    TARGET genesys_modelspecific_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:genesys_modelspecific_app>
        $<TARGET_FILE_DIR:genesys_modelspecific_app>/genesys_terminal_application
    COMMENT "Preserving legacy genesys_terminal_application binary name for model-specific flows"
)

add_custom_target(genesys_terminal_application
    DEPENDS genesys_modelspecific_app
)

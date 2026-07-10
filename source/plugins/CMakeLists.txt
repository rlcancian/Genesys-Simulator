add_subdirectory(data)
add_subdirectory(components)

file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/components/*.cpp"
)

add_library(genesys_plugins_components_minimal STATIC
    ${GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES}
)

target_include_directories(genesys_plugins_components_minimal
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_plugins_components_minimal
    PUBLIC
        genesys_plugins_data
        genesys_kernel_util
)

target_compile_features(genesys_plugins_components_minimal PUBLIC cxx_std_23)

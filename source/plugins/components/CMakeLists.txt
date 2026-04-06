file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_plugins_components STATIC
    ${GENESYS_PLUGINS_COMPONENTS_SOURCES}
)

target_include_directories(genesys_plugins_components
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_plugins_components
    PUBLIC
        genesys_kernel_util
)

target_compile_features(genesys_plugins_components PUBLIC cxx_std_23)

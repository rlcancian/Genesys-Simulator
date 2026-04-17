file(GLOB_RECURSE GENESYS_PLUGINS_DATA_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_plugins_data STATIC
    ${GENESYS_PLUGINS_DATA_SOURCES}
)

target_include_directories(genesys_plugins_data
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_plugins_data
    PUBLIC
        genesys_kernel_util
)

target_compile_features(genesys_plugins_data PUBLIC cxx_std_23)

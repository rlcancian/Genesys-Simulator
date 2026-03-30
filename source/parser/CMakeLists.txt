file(GLOB GENESYS_PARSER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_parser STATIC
    ${GENESYS_PARSER_SOURCES}
)

target_include_directories(genesys_parser
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_parser
    PUBLIC
        genesys_kernel_util
        genesys_plugins_data
)

target_compile_features(genesys_parser PUBLIC cxx_std_20)

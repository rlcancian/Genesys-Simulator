option(GENESYS_PARSER_REGENERATE "Regenerate parser/scanner sources with bison/flex" OFF)

if(GENESYS_PARSER_REGENERATE)
    find_package(BISON REQUIRED)
    find_package(FLEX REQUIRED)

    BISON_TARGET(
        GenesysParser
        ${CMAKE_CURRENT_SOURCE_DIR}/parserBisonFlex/bisonparser.yy
        ${CMAKE_CURRENT_SOURCE_DIR}/GenesysParser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/GenesysParser.h
        REPORT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parserBisonFlex/bisonparser.report
        OPTIONS --locations --debug --feature=caret -Wall --verbose
    )

    FLEX_TARGET(
        GenesysScanner
        ${CMAKE_CURRENT_SOURCE_DIR}/parserBisonFlex/lexerparser.ll
        ${CMAKE_CURRENT_SOURCE_DIR}/Genesys++-scanner.cpp
    )
endif()

file(GLOB GENESYS_PARSER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_parser STATIC
    ${GENESYS_PARSER_SOURCES}
)

if(GENESYS_PARSER_REGENERATE)
    add_dependencies(genesys_parser ${BISON_GenesysParser_TARGET} ${FLEX_GenesysScanner_TARGET})
endif()

target_include_directories(genesys_parser
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_parser
    PUBLIC
        genesys_kernel_util
        genesys_plugins_data
)

target_compile_features(genesys_parser PUBLIC cxx_std_23)

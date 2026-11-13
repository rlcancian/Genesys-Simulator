option(GENESYS_PARSER_REGENERATE "Regenerate parser/scanner sources with bison/flex" OFF)

if(GENESYS_PARSER_REGENERATE)
    find_package(BISON REQUIRED)
    find_package(FLEX REQUIRED)

    BISON_TARGET(
        GenesysParser
        ${CMAKE_CURRENT_SOURCE_DIR}/parserBisonFlex/bisonparser.yy
        ${CMAKE_CURRENT_SOURCE_DIR}/GenesysParser.cpp
        DEFINES_FILE ${CMAKE_CURRENT_SOURCE_DIR}/GenesysParser.h
        VERBOSE
        REPORT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/parserBisonFlex/bisonparser.report
        COMPILE_FLAGS "--locations --debug --feature=caret -Wall"
    )

    FLEX_TARGET(
        GenesysScanner
        ${CMAKE_CURRENT_SOURCE_DIR}/parserBisonFlex/lexerparser.ll
        ${CMAKE_CURRENT_SOURCE_DIR}/Genesys++-scanner.cpp
    )
    ADD_FLEX_BISON_DEPENDENCY(GenesysScanner GenesysParser)
endif()

file(GLOB GENESYS_PARSER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_parser STATIC
    ${GENESYS_PARSER_SOURCES}
)

if(GENESYS_PARSER_REGENERATE)
    # BISON_TARGET/FLEX_TARGET define custom commands and output files, not
    # CMake targets. Listing the generated outputs wires the build dependencies.
    target_sources(genesys_parser PRIVATE
        ${BISON_GenesysParser_OUTPUTS}
        ${FLEX_GenesysScanner_OUTPUTS}
    )
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

if(NOT TARGET genesys_tools_analysis)
    add_subdirectory(${CMAKE_SOURCE_DIR}/source/tools ${CMAKE_BINARY_DIR}/source/tools-for-examples)
endif()

add_executable(genesys_examples_analysis_tools
    analysis_tools_example.cpp
)

target_link_libraries(genesys_examples_analysis_tools
    PRIVATE
        genesys_tools_analysis
)

target_compile_features(genesys_examples_analysis_tools PRIVATE cxx_std_23)

set_property(TARGET genesys_examples_analysis_tools PROPERTY FOLDER examples)

if(NOT TARGET genesys_tools)
    add_subdirectory(${CMAKE_SOURCE_DIR}/source/tools ${CMAKE_BINARY_DIR}/source/tools-for-examples)
endif()

add_executable(genesys_fit_analysis
    fit_analysis.cpp
)

target_link_libraries(genesys_fit_analysis
    PRIVATE
        genesys_tools
)

target_compile_features(genesys_fit_analysis PRIVATE cxx_std_23)

set_property(TARGET genesys_fit_analysis PROPERTY FOLDER examples)

if(NOT TARGET genesys_tools_analysis)
    add_subdirectory(${CMAKE_SOURCE_DIR}/source/tools ${CMAKE_BINARY_DIR}/source/tools-for-examples)
endif()

add_executable(genesys_examples_analysis_tools
    analysis_tools_example.cpp
)

add_executable(genesys_examples_simulation_analysis
    simulation_analysis_example.cpp
)

target_link_libraries(genesys_examples_analysis_tools
    PRIVATE
        genesys_tools_analysis
)

target_link_libraries(genesys_examples_simulation_analysis
    PRIVATE
        "$<LINK_GROUP:RESCAN,genesys_tools_analysis,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime>"
)

target_compile_features(genesys_examples_analysis_tools PRIVATE cxx_std_23)
target_compile_features(genesys_examples_simulation_analysis PRIVATE cxx_std_23)

add_custom_target(genesys_examples
    DEPENDS
        genesys_examples_analysis_tools
        genesys_examples_simulation_analysis
)

set_property(TARGET genesys_examples_analysis_tools PROPERTY FOLDER examples)
set_property(TARGET genesys_examples_simulation_analysis PROPERTY FOLDER examples)
set_property(TARGET genesys_examples PROPERTY FOLDER examples)

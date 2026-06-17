add_library(genesys_tools_analysis STATIC
    analysis/DatasetLoader.cpp
    analysis/FitterDefaultImpl.cpp
    analysis/FitterDummyImpl.cpp
    analysis/HypothesisTesterDefaultImpl.cpp
    analysis/DataAnalyserDefaultImpl.cpp
    analysis/SimulationResultsDataset.cpp
    analysis/SimulationResultsParser.cpp
    analysis/ProbabilityDistribution.cpp
    analysis/ProbabilityDistributionBase.cpp
    SolverDefaultImpl1.cpp
)

target_include_directories(genesys_tools_analysis
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_features(genesys_tools_analysis PUBLIC cxx_std_23)

add_library(genesys_tools STATIC
    OptimizerDefaultImpl1.cpp
    FactorialDesign/FactorialDesign.cpp
)

target_include_directories(genesys_tools
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_tools
    PUBLIC
        genesys_tools_analysis
)

if(TARGET genesys_kernel_util AND TARGET genesys_kernel_statistics AND TARGET genesys_kernel_simulator_runtime)
    target_link_libraries(genesys_tools
        PUBLIC
        genesys_kernel_util
        genesys_kernel_statistics
        genesys_kernel_simulator_runtime
    )
endif()

target_compile_features(genesys_tools PUBLIC cxx_std_23)

if(GENESYS_BUILD_TOOLS)
    add_executable(genesys_tools_application
        main.cpp
    )

    target_link_libraries(genesys_tools_application
        PRIVATE
            genesys_tools
    )

    target_compile_features(genesys_tools_application PRIVATE cxx_std_23)
endif()

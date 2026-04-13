add_library(genesys_tools STATIC
    FitterDummyImpl.cpp
    HypothesisTesterDefaultImpl1.cpp
    OptimizerDefaultImpl1.cpp
    ProbabilityDistribution.cpp
    ProbabilityDistributionBase.cpp
    SolverDefaultImpl1.cpp
    FactorialDesign/FactorialDesign.cpp
)

target_include_directories(genesys_tools
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_tools
    PUBLIC
        genesys_kernel_util
        genesys_kernel_statistics
        genesys_kernel_simulator_runtime
)

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

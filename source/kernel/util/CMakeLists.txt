add_library(genesys_kernel_util STATIC
    Util.cpp
)

target_include_directories(genesys_kernel_util
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_compile_features(genesys_kernel_util PUBLIC cxx_std_20)

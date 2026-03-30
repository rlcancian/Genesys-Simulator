file(GLOB GENESYS_KERNEL_STATISTICS_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(genesys_kernel_statistics STATIC
    ${GENESYS_KERNEL_STATISTICS_SOURCES}
)

target_include_directories(genesys_kernel_statistics
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_kernel_statistics
    PUBLIC
        genesys_kernel_util
)

target_compile_features(genesys_kernel_statistics PUBLIC cxx_std_20)

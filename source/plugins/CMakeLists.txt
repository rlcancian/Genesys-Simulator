include(GNUInstallDirs)

add_subdirectory(data)
add_subdirectory(components)

file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/components/*.cpp"
)

add_library(genesys_plugins_components_minimal STATIC
    ${GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES}
)

target_include_directories(genesys_plugins_components_minimal
    PUBLIC
        ${CMAKE_SOURCE_DIR}/source
)

target_link_libraries(genesys_plugins_components_minimal
    PUBLIC
        genesys_plugins_data
        genesys_kernel_util
)

target_compile_features(genesys_plugins_components_minimal PUBLIC cxx_std_23)
target_compile_definitions(genesys_plugins_components_minimal PRIVATE
    "GENESYS_CPPFORG_INSTALL_INCLUDE_ROOT=\"${CMAKE_INSTALL_DATADIR}/genesys/source\"")

# CppForG compiles C++ hooks after installation, so ship the matching kernel
# and plugin headers beside the application data tree.
install(DIRECTORY "${CMAKE_SOURCE_DIR}/source/"
    DESTINATION "${CMAKE_INSTALL_DATADIR}/genesys/source"
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.tpp"
        PATTERN "*.inl"
        PATTERN "*.ipp"
        PATTERN "*.inc"
)

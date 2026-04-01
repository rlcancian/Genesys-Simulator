add_subdirectory(data)

set(GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Create.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Dispose.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Seize.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/SeizableItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/QueueableItem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Delay.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/components/Release.cpp"
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

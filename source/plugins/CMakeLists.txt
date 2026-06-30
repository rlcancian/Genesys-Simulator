add_subdirectory(data)

if(GENESYS_DYNAMIC_PLUGINS)

    add_library(genesys_plugin_connector STATIC
        PluginConnectorDynamicLibraryLoader.cpp
        StaticCppCompiler.cpp
    )
    target_include_directories(genesys_plugin_connector PUBLIC ${CMAKE_SOURCE_DIR}/source)
    target_link_libraries(genesys_plugin_connector PUBLIC genesys_kernel_util genesys_plugins_data)
    target_compile_features(genesys_plugin_connector PUBLIC cxx_std_23)

    file(GLOB GENESYS_PLUGIN_GROUP_DIRS LIST_DIRECTORIES true
        "${CMAKE_CURRENT_SOURCE_DIR}/components/*")
    
    foreach(group_dir ${GENESYS_PLUGIN_GROUP_DIRS})
        if(IS_DIRECTORY ${group_dir})
            get_filename_component(group_name ${group_dir} NAME)
            file(GLOB group_sources CONFIGURE_DEPENDS "${group_dir}/*.cpp")
            if(group_sources)
                message(STATUS "Plugin group ${group_name}: found ${group_sources}")
                add_library(plugin_${group_name} SHARED ${group_sources})
                target_include_directories(plugin_${group_name} PRIVATE ${CMAKE_SOURCE_DIR}/source)
                target_compile_features(plugin_${group_name} PRIVATE cxx_std_23)
                set_target_properties(plugin_${group_name} PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins)
            endif()
        endif()
    endforeach()

    set(GENESYS_ALL_DYNAMIC_PLUGIN_TARGETS "")
    file(GLOB GENESYS_PLUGIN_GROUP_DIRS LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/components/*")
    foreach(group_dir ${GENESYS_PLUGIN_GROUP_DIRS})
        if(IS_DIRECTORY ${group_dir})
            get_filename_component(group_name ${group_dir} NAME)
            if(TARGET plugin_${group_name})
                list(APPEND GENESYS_ALL_DYNAMIC_PLUGIN_TARGETS plugin_${group_name})
            endif()
        endif()
    endforeach()
    
    add_custom_target(genesys_plugins_dynamic ALL DEPENDS ${GENESYS_ALL_DYNAMIC_PLUGIN_TARGETS})

    add_library(genesys_plugins_components_minimal INTERFACE)
else()

    add_subdirectory(components)
    file(GLOB_RECURSE GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/components/*.cpp"
    )

    add_library(genesys_plugin_connector STATIC
        PluginConnectorDummyImpl1.cpp
    )
    target_include_directories(genesys_plugin_connector PUBLIC ${CMAKE_SOURCE_DIR}/source)
    target_link_libraries(genesys_plugin_connector PUBLIC genesys_kernel_util genesys_plugins_data)
    target_compile_features(genesys_plugin_connector PUBLIC cxx_std_23)
    
    add_library(genesys_plugins_components_minimal STATIC
        ${GENESYS_PLUGINS_COMPONENTS_MINIMAL_SOURCES}
    )
    target_include_directories(genesys_plugins_components_minimal PUBLIC ${CMAKE_SOURCE_DIR}/source)
    target_link_libraries(genesys_plugins_components_minimal PUBLIC genesys_plugins_data genesys_kernel_util)
    target_compile_features(genesys_plugins_components_minimal PUBLIC cxx_std_23)
endif()

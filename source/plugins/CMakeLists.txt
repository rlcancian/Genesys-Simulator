add_subdirectory(data)

if(GENESYS_DYNAMIC_PLUGINS)

    # Data files referenced across multiple component groups - kept in a small shared static lib
    add_library(genesys_plugins_data_shared STATIC
        data/Grouping/EntityGroup.cpp
        data/DiscreteProcessing/Queue.cpp
        data/DiscreteProcessing/Resource.cpp
        data/DiscreteProcessing/Schedule.cpp
        data/MaterialHandling/Station.cpp
        data/ExternalIntegration/CppCompiler.cpp
        data/ExternalIntegration/SPICERunner.cpp
    )
    target_include_directories(genesys_plugins_data_shared PUBLIC ${CMAKE_SOURCE_DIR}/source)
    target_link_libraries(genesys_plugins_data_shared PUBLIC genesys_kernel_util)
    target_compile_features(genesys_plugins_data_shared PUBLIC cxx_std_23)

    add_library(genesys_plugin_connector STATIC
        PluginConnectorDynamicLibraryLoader.cpp
        StaticCppCompiler.cpp
    )
    target_include_directories(genesys_plugin_connector PUBLIC ${CMAKE_SOURCE_DIR}/source)
    target_link_libraries(genesys_plugin_connector PUBLIC genesys_kernel_util)
    target_link_libraries(genesys_plugin_connector PRIVATE genesys_plugins_data_shared)
    target_compile_features(genesys_plugin_connector PUBLIC cxx_std_23)

    file(GLOB GENESYS_PLUGIN_GROUP_DIRS LIST_DIRECTORIES true
        "${CMAKE_CURRENT_SOURCE_DIR}/components/*")

    foreach(group_dir ${GENESYS_PLUGIN_GROUP_DIRS})
        if(IS_DIRECTORY ${group_dir})
            get_filename_component(group_name ${group_dir} NAME)
            file(GLOB_RECURSE group_sources CONFIGURE_DEPENDS "${group_dir}/*.cpp")

            # this group's own data files (excluding the ones already in the shared lib)
            set(group_data_dir "${CMAKE_CURRENT_SOURCE_DIR}/data/${group_name}")
            set(group_data_sources "")
            if(IS_DIRECTORY ${group_data_dir})
                file(GLOB group_data_sources CONFIGURE_DEPENDS "${group_data_dir}/*.cpp")
                list(FILTER group_data_sources EXCLUDE REGEX "EntityGroup\\.cpp$")
                list(FILTER group_data_sources EXCLUDE REGEX "Queue\\.cpp$")
                list(FILTER group_data_sources EXCLUDE REGEX "Resource\\.cpp$")
                list(FILTER group_data_sources EXCLUDE REGEX "Schedule\\.cpp$")
                list(FILTER group_data_sources EXCLUDE REGEX "Station\\.cpp$")
                list(FILTER group_data_sources EXCLUDE REGEX "CppCompiler\\.cpp$")
                list(FILTER group_data_sources EXCLUDE REGEX "SPICERunner\\.cpp$")
            endif()

            if(group_sources)
                message(STATUS "Plugin group ${group_name}: found ${group_sources}")
                add_library(plugin_${group_name} SHARED ${group_sources} ${group_data_sources})
                target_include_directories(plugin_${group_name} PRIVATE ${CMAKE_SOURCE_DIR}/source)
                target_link_libraries(plugin_${group_name} PRIVATE genesys_kernel_util genesys_plugins_data_shared)
                target_compile_features(plugin_${group_name} PRIVATE cxx_std_23)
                set_target_properties(plugin_${group_name} PROPERTIES
                    LIBRARY_OUTPUT_DIRECTORY /usr/local/lib/genesys/plugins)
            endif()
        endif()
    endforeach()

    set(GENESYS_ALL_DYNAMIC_PLUGIN_TARGETS "")
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

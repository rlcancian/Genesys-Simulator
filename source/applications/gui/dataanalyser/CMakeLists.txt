find_package(Qt6 QUIET COMPONENTS Core Gui Widgets)
if(Qt6_FOUND)
    set(GENESYS_QT_PACKAGE Qt6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)
    set(GENESYS_QT_PACKAGE Qt5)
endif()

include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_executable(genesys_dataanalyser_gui_application
    main.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/gui/genesys/tools/dataanalyzer/DataAnalyzerWindow.cpp
)

target_include_directories(genesys_dataanalyser_gui_application
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_SOURCE_DIR}/source/applications/gui/genesys
        ${CMAKE_SOURCE_DIR}/source/applications/gui/genesys/tools/dataanalyzer
)

target_link_libraries(genesys_dataanalyser_gui_application
    PRIVATE
        ${GENESYS_QT_PACKAGE}::Core
        ${GENESYS_QT_PACKAGE}::Gui
        ${GENESYS_QT_PACKAGE}::Widgets
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime,genesys_tools>"
)

target_compile_features(genesys_dataanalyser_gui_application PRIVATE cxx_std_23)

set_target_properties(genesys_dataanalyser_gui_application
    PROPERTIES
        OUTPUT_NAME genesys-dataanalyser-gui
)

if(TARGET genesys_kernel)
    add_dependencies(genesys_dataanalyser_gui_application genesys_kernel)
endif()

if(TARGET genesys_tools)
    add_dependencies(genesys_dataanalyser_gui_application genesys_tools)
endif()

install(TARGETS genesys_dataanalyser_gui_application
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_target(genesys_dataanalyser_gui DEPENDS genesys_dataanalyser_gui_application)

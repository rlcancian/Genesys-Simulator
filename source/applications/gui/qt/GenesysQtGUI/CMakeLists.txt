find_package(Qt6 QUIET COMPONENTS Core Gui Widgets PrintSupport Network)
if(Qt6_FOUND)
    set(GENESYS_QT_PACKAGE Qt6)
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets PrintSupport Network)
    set(GENESYS_QT_PACKAGE Qt5)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

file(GLOB_RECURSE GENESYS_QT_GUI_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

list(FILTER GENESYS_QT_GUI_SOURCES EXCLUDE REGEX "/build/")
list(FILTER GENESYS_QT_GUI_SOURCES EXCLUDE REGEX "/qcustomplot\\.cpp$")

set(GENESYS_QT_GUI_RESOURCES
    GenesysQtGUI_resources.qrc
    propertyeditor/qtpropertybrowser/qtpropertybrowser.qrc
)

set(GENESYS_QT_GUI_TERMINAL_FALLBACK_SOURCES
    ${CMAKE_SOURCE_DIR}/source/applications/BaseGenesysTerminalApplication.cpp
    ${CMAKE_SOURCE_DIR}/source/applications/terminal/GenesysShell/GenesysShell.cpp
)

add_executable(genesys_qt_gui_application
    ${GENESYS_QT_GUI_SOURCES}
    ${GENESYS_QT_GUI_RESOURCES}
    ${GENESYS_QT_GUI_TERMINAL_FALLBACK_SOURCES}
)

target_include_directories(genesys_qt_gui_application
    PRIVATE
        ${CMAKE_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/codeeditor
        ${CMAKE_CURRENT_SOURCE_DIR}/propertyeditor
        ${CMAKE_CURRENT_SOURCE_DIR}/propertyeditor/qtpropertybrowser
)

target_link_libraries(genesys_qt_gui_application
    PRIVATE
        ${GENESYS_QT_PACKAGE}::Core
        ${GENESYS_QT_PACKAGE}::Gui
        ${GENESYS_QT_PACKAGE}::Widgets
        ${GENESYS_QT_PACKAGE}::PrintSupport
        ${GENESYS_QT_PACKAGE}::Network
        "$<LINK_GROUP:RESCAN,genesys_kernel_util,genesys_kernel_statistics,genesys_parser,genesys_plugins_data,genesys_plugins_components_minimal,genesys_kernel_simulator_support,genesys_kernel_simulator_runtime,genesys_tools>"
)

target_compile_features(genesys_qt_gui_application PRIVATE cxx_std_23)

set_target_properties(genesys_qt_gui_application
    PROPERTIES
        AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/dialogs"
)

if(TARGET genesys_kernel)
    add_dependencies(genesys_qt_gui_application genesys_kernel)
endif()

add_custom_target(genesys_gui DEPENDS genesys_qt_gui_application)

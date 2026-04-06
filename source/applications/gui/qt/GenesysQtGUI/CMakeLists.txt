find_program(GENESYS_QMAKE_EXECUTABLE NAMES qmake6 qmake-qt5 qmake)

if(NOT GENESYS_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_APPLICATION=ON, but qmake was not found in PATH. Install Qt qmake or set PATH/QMAKE_EXECUTABLE.")
endif()

set(GENESYS_QTGUI_BUILD_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/build_qtgui.sh")

add_custom_target(genesys_qt_gui_application
    COMMAND ${GENESYS_QTGUI_BUILD_SCRIPT}
            --config $<CONFIG>
            --build-dir ${CMAKE_CURRENT_SOURCE_DIR}/build/cmake-$<LOWER_CASE:$<CONFIG>>
            --jobs $<IF:$<BOOL:${CMAKE_BUILD_PARALLEL_LEVEL}>,${CMAKE_BUILD_PARALLEL_LEVEL},1>
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    USES_TERMINAL
    COMMENT "Building GenesysQtGUI via qmake"
)

add_custom_target(genesys_gui DEPENDS genesys_qt_gui_application)

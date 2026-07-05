# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "source/applications/gui/qt/GenesysQtGUI/CMakeFiles/genesys_qt_gui_application_autogen.dir/AutogenUsed.txt"
  "source/applications/gui/qt/GenesysQtGUI/CMakeFiles/genesys_qt_gui_application_autogen.dir/ParseCache.txt"
  "source/applications/gui/qt/GenesysQtGUI/genesys_qt_gui_application_autogen"
  )
endif()

# Main GenESyS GUI application.
#
# This directory is the planned stable location for the main graphical
# application. During the transition it delegates to the historical Qt-specific
# implementation directory so targets, install names, and source paths remain
# unchanged.

add_subdirectory(../qt/GenesysQtGUI GenesysQtGUI)

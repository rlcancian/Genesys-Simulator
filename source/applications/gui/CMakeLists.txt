# GUI applications umbrella.
#
# Each graphical application must keep its own CMakeLists.txt and target(s).
# This umbrella coordinates GUI applications without collecting their sources
# into a single executable.
#
# The current main GenESyS GUI remains in its existing location during the
# first migration step. Future GUI applications should be added as sibling
# subdirectories with independent targets and build options.

option(GENESYS_BUILD_GUI_GENESYS "Build the main GenESyS Qt GUI application" ON)
option(GENESYS_BUILD_GUI_HTTPWORKER "Build the graphical HTTP worker control application" OFF)
option(GENESYS_BUILD_GUI_DATAANALYSER "Build the graphical data analyser application" OFF)
option(GENESYS_BUILD_GUI_OPTIMIZER "Build the graphical optimizer application" OFF)
option(GENESYS_BUILD_GUI_AI_ASSISTANT "Build the graphical AI assistant application" OFF)
option(GENESYS_BUILD_GUI_DOEXPERIMENTS "Build the graphical design-of-experiments application" OFF)

if(GENESYS_BUILD_GUI_GENESYS)
    add_subdirectory(qt/GenesysQtGUI)
endif()

if(GENESYS_BUILD_GUI_HTTPWORKER)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_HTTPWORKER is planned, but source/applications/gui/httpworker is not implemented yet.")
endif()

if(GENESYS_BUILD_GUI_DATAANALYSER)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_DATAANALYSER is planned, but source/applications/gui/dataanalyser is not implemented yet.")
endif()

if(GENESYS_BUILD_GUI_OPTIMIZER)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_OPTIMIZER is planned, but source/applications/gui/optimizer is not implemented yet.")
endif()

if(GENESYS_BUILD_GUI_AI_ASSISTANT)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_AI_ASSISTANT is planned, but source/applications/gui/ai_assistant is not implemented yet.")
endif()

if(GENESYS_BUILD_GUI_DOEXPERIMENTS)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_DOEXPERIMENTS is planned, but source/applications/gui/doexperiments is not implemented yet.")
endif()

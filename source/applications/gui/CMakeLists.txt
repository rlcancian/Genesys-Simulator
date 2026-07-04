# GUI applications umbrella.
#
# Each graphical application must keep its own CMakeLists.txt and target(s).
# This umbrella coordinates GUI applications without collecting their sources
# into a single executable.
#
# The main GenESyS GUI is routed through source/applications/gui/genesys.
# During the transition that directory delegates to the historical Qt-specific
# implementation path. Future GUI applications should be added as sibling
# subdirectories with independent targets and build options.

option(GENESYS_BUILD_GUI_GENESYS "Build the main GenESyS Qt GUI application" ON)
option(GENESYS_BUILD_GUI_HTTPWORKER "Build the graphical HTTP worker control application" OFF)
option(GENESYS_BUILD_GUI_DATAANALYSER "Build the graphical data analyser application" OFF)
option(GENESYS_BUILD_GUI_OPTIMIZER "Build the graphical optimizer application" OFF)
option(GENESYS_BUILD_GUI_AI_ASSISTANT "Build the graphical AI assistant application" OFF)
option(GENESYS_BUILD_GUI_DOEXPERIMENTS "Build the graphical design-of-experiments application" OFF)

if(GENESYS_BUILD_GUI_GENESYS)
    add_subdirectory(genesys)
endif()

if(GENESYS_BUILD_GUI_HTTPWORKER)
    add_subdirectory(httpworker)
endif()

if(GENESYS_BUILD_GUI_DATAANALYSER)
    add_subdirectory(dataanalyser)
endif()

if(GENESYS_BUILD_GUI_OPTIMIZER)
    add_subdirectory(optimizer)
endif()

if(GENESYS_BUILD_GUI_AI_ASSISTANT)
    add_subdirectory(ai_assistant)
endif()

if(GENESYS_BUILD_GUI_DOEXPERIMENTS)
    message(FATAL_ERROR "GENESYS_BUILD_GUI_DOEXPERIMENTS is planned, but source/applications/gui/doexperiments is not implemented yet.")
endif()

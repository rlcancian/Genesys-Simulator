# GUI applications umbrella.
#
# Each graphical application must keep its own CMakeLists.txt and target(s).
# This umbrella coordinates GUI applications without collecting their sources
# into a single executable.
#
# The current main GenESyS GUI remains in its existing location during the
# first migration step. Future GUI applications should be added as sibling
# subdirectories with independent targets and build options.

add_subdirectory(qt/GenesysQtGUI)

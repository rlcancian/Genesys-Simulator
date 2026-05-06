# QtCreator helper project for the CMake/Ninja-based GenESyS build.
#
# Open this file in QtCreator when you want the old qmake front-end to drive
# the existing CMake presets directly, without rebuilding the application tree
# through qmake itself.
#
# Default target:
#   - build_gui_application
#
# Available targets:
#   - configure_gui_application
#   - build_gui_application
#   - run_gui_application
#   - configure_web_application
#   - build_web_application
#   - run_web_application
#   - configure_terminal_application
#   - build_terminal_application
#   - run_terminal_application
#   - configure_terminal_example
#   - build_terminal_example
#   - run_terminal_example
#   - configure_tests_kernel_unit
#   - build_tests_kernel_unit
#   - run_tests_kernel_unit
#   - configure_tests_smoke
#   - build_tests_smoke
#   - run_tests_smoke
#
# The "run_*" targets are plain shell commands, so they can be used from the
# QtCreator build step to launch the built executable after the preset build.

TEMPLATE = aux
CONFIG += ordered
CONFIG -= qt warn_on

SOURCE_ROOT = $$clean_path($$PWD/../../../../../)
BUILD_ROOT = $$SOURCE_ROOT/build

CMAKE_BIN = cmake

GUI_BUILD_DIR = $$BUILD_ROOT/gui-app
GUI_BINARY = $$GUI_BUILD_DIR/source/applications/gui/qt/GenesysQtGUI/genesys_qt_gui_application

WEB_BUILD_DIR = $$BUILD_ROOT/genesys_web_app
WEB_BINARY = $$WEB_BUILD_DIR/source/applications/web/genesys_web_app

TERMINAL_BUILD_DIR = $$BUILD_ROOT/terminal-app
TERMINAL_BINARY = $$TERMINAL_BUILD_DIR/source/applications/terminal/genesys_terminal_application

TERMINAL_EXAMPLE_BUILD_DIR = $$BUILD_ROOT/terminal-example
TERMINAL_EXAMPLE_BINARY = $$TERMINAL_EXAMPLE_BUILD_DIR/source/applications/terminal/genesys_terminal_application

TESTS_KERNEL_BUILD_DIR = $$BUILD_ROOT/tests-kernel-unit
TESTS_SMOKE_BUILD_DIR = $$BUILD_ROOT/tests-smoke

QMAKE_EXTRA_TARGETS += \
    configure_gui_application \
    build_gui_application \
    run_gui_application \
    configure_web_application \
    build_web_application \
    run_web_application \
    configure_terminal_application \
    build_terminal_application \
    run_terminal_application \
    configure_terminal_example \
    build_terminal_example \
    run_terminal_example \
    configure_tests_kernel_unit \
    build_tests_kernel_unit \
    run_tests_kernel_unit \
    configure_tests_smoke \
    build_tests_smoke \
    run_tests_smoke

configure_gui_application.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --preset gui-app
build_gui_application.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset gui-app
run_gui_application.commands = cd $$GUI_BUILD_DIR/source/applications/gui/qt/GenesysQtGUI && ./genesys_qt_gui_application
build_gui_application.depends = configure_gui_application
run_gui_application.depends = build_gui_application

configure_web_application.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --preset genesys_web_app
build_web_application.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset genesys_web_app
run_web_application.commands = cd $$WEB_BUILD_DIR/source/applications/web && ./genesys_web_app --port 8080
build_web_application.depends = configure_web_application
run_web_application.depends = build_web_application

configure_terminal_application.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --preset terminal-app
build_terminal_application.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset terminal-app
run_terminal_application.commands = cd $$TERMINAL_BUILD_DIR/source/applications/terminal && ./genesys_terminal_application
build_terminal_application.depends = configure_terminal_application
run_terminal_application.depends = build_terminal_application

configure_terminal_example.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --preset terminal-example
build_terminal_example.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset terminal-example
run_terminal_example.commands = cd $$TERMINAL_EXAMPLE_BUILD_DIR/source/applications/terminal && ./genesys_terminal_application
build_terminal_example.depends = configure_terminal_example
run_terminal_example.depends = build_terminal_example

configure_tests_kernel_unit.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --preset tests-kernel-unit
build_tests_kernel_unit.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset tests-kernel-unit
run_tests_kernel_unit.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset tests-kernel-unit-run
build_tests_kernel_unit.depends = configure_tests_kernel_unit
run_tests_kernel_unit.depends = build_tests_kernel_unit

configure_tests_smoke.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --preset tests-smoke
build_tests_smoke.commands = cd $$SOURCE_ROOT && $$CMAKE_BIN --build --preset tests-smoke
run_tests_smoke.commands = cd $$SOURCE_ROOT && ctest --preset tests-smoke
build_tests_smoke.depends = configure_tests_smoke
run_tests_smoke.depends = build_tests_smoke

TARGET = genesys_qt_gui_application
DESTDIR = $$GUI_BUILD_DIR/source/applications/gui/qt/GenesysQtGUI
PRE_TARGETDEPS += build_gui_application

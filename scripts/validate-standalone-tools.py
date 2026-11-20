#!/usr/bin/env python3
"""Source-level invariants for the three independent Tools GUI applications.

This complements CMake build/CTest and the Xvfb runtime test; it is not
runtime evidence by itself.
"""
from __future__ import annotations

import pathlib
import re
import sys
import xml.etree.ElementTree as ET

ROOT = pathlib.Path(__file__).resolve().parents[1]
GUI = ROOT / "source/applications/gui"
MAIN = GUI / "genesys"
TOOLS = {
    "DataAnalyzer": ("dataanalyser", "genesys_dataanalyser_gui_application",
                     "genesys-dataanalyser-gui", "DataAnalyzerWindow", "DataAnalyzerWindow.cpp"),
    "Optimizator": ("optimizer", "genesys_optimizer_gui_application",
                    "genesys-optimizer-gui", "OptimizerWindow", "OptimizerWindow.cpp"),
    "AIAssistant": ("ai_assistant", "genesys_ai_assistant_gui_application",
                    "genesys-ai-assistant-gui", "AIAssistantWindow", "AIAssistantWindow.cpp"),
}


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def slot_body(source: str, signature: str) -> str:
    match = re.search(r"void\s+" + re.escape(signature) + r"\s*\(\s*\)\s*\{",
                      source, re.DOTALL)
    require(match is not None, f"missing wrapper or handler: {signature}")
    end = source.find("\n}", match.end())
    require(end != -1, f"missing function end: {signature}")
    return source[match.end():end]


def main() -> int:
    ui = ET.parse(MAIN / "mainwindow.ui").getroot()
    menu = ui.find(".//widget[@name='menuTools']")
    require(menu is not None, "Tools menu missing")
    menu_actions = [e.attrib["name"] for e in menu.findall("addaction")]
    wrappers = (MAIN / "mainwindow_controller.cpp").read_text()
    controller = (MAIN / "controllers/DialogUtilityController.cpp").read_text()
    gui_cmake = (MAIN / "CMakeLists.txt").read_text()
    debian_rules = (ROOT / "debian/rules").read_text()
    debian_manifest = (ROOT / "debian/genesys-gui.install").read_text().splitlines()
    portable_builder = (ROOT / "packaging/installer/build-portable.sh").read_text()

    for action, (directory, target, exe, window, window_source) in TOOLS.items():
        require(f"actionTools{action}" in menu_actions, f"{action}: missing QAction")
        main_cpp = (GUI / directory / "main.cpp").read_text()
        cmake = (GUI / directory / "CMakeLists.txt").read_text()
        for fragment in ("int main(", "QApplication app(", "Simulator simulator;",
                         f"{window} window(", "window.show()", "app.exec()"):
            require(fragment in main_cpp, f"{exe}: missing entrypoint invariant {fragment}")
        require(f"add_executable({target}" in cmake, f"{exe}: missing CMake target")
        require(f"OUTPUT_NAME {exe}" in cmake, f"{exe}: wrong output name")
        require(f"RUNTIME DESTINATION ${{CMAKE_INSTALL_BINDIR}}" in cmake,
                f"{exe}: install destination drift")
        require(f"/{window_source}" in cmake or window_source in cmake,
                f"{exe}: window not in application target")
        if window != "AIAssistantWindow":
            escaped_window_source = window_source.replace(".", r"\\.")
            require(escaped_window_source in gui_cmake or window_source in gui_cmake,
                    f"{exe}: main GUI no longer excludes external window")
        else:
            require("AIAssistantWindow.cpp" not in gui_cmake,
                    "AI Assistant window incorrectly aggregated into main GUI")

        body = slot_body(wrappers, f"MainWindow::on_actionTools{action}_triggered")
        require(f"_dialogUtilityController->onActionTools{action}Triggered()" in body,
                f"{action}: QAction does not delegate to Tools handler")
        handler = slot_body(controller, f"DialogUtilityController::onActionTools{action}Triggered")
        require("ToolLauncher::launchDetached(" in handler and f'"{exe}"' in handler,
                f"{action}: action does not launch the standalone executable")
        require("new " + window not in handler and "_simulator" not in handler,
                f"{action}: handler owns a window or depends on the active simulator")
        require(f"-DGENESYS_BUILD_GUI_{directory.upper()}=ON" in debian_rules,
                f"{exe}: Debian build does not include standalone tool")
        require(f"usr/bin/{exe}" in debian_manifest,
                f"{exe}: Debian package missing executable")
        require(exe in portable_builder, f"{exe}: portable distribution missing executable")

    print("PASS: three independent entrypoints/targets, Tools action mapping, "
          "main GUI exclusions and Debian/portable source contracts")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except AssertionError as err:
        print(f"FAIL: {err}", file=sys.stderr)
        sys.exit(1)

# GUI Graphical Plugin Audit

## Scope

This audit covers step 1 and step 2 of the graphical-plugin plan:

1. Audit current GUI extension points in `GenesysQtGUI`.
2. Define a generic, domain-agnostic graphical plugin contract.

No GUI behavior was changed in this step. No biosimulation-specific graphical feature was added.

## Current GUI Extension Reality

## Composition Root

- `source/applications/gui/qt/GenesysQtGUI/mainwindow.h`
- `source/applications/gui/qt/GenesysQtGUI/mainwindow_controller.cpp`

`MainWindow` is the composition root and currently owns/wires all GUI flows by delegating to extracted controllers and services.

## Existing Delegated Controllers

- `controllers/PluginCatalogController.*`
- `controllers/SceneToolController.*`
- `controllers/GraphicalContextMenuController.*`
- `controllers/ModelInspectorController.*`
- `controllers/PropertyEditorController.*`
- `controllers/ModelLifecycleController.*`
- `controllers/SimulationCommandController.*`

These controllers organize responsibilities, but there is no unified API for third-party graphical extension modules.

## Plugin Catalog Behavior

- `PluginCatalogController` builds a categorized plugin tree using simulator plugin metadata.
- It does not provide a generic registration API for adding menu actions, docks, or windows from GUI-side extension modules.
- Category color and expansion policies are hardcoded in the controller.

## Context Menu and Scene Tooling

- `GraphicalContextMenuController` builds context menus by reusing fixed `MainWindow` actions.
- `SceneToolController` maps fixed actions to scene operations (draw, animate, align, zoom, visibility).
- Both are strongly action-driven and currently assume static wiring from `MainWindow` and `mainwindow.ui`.

## Plugin Manager Dialog

- `dialogs/dialogpluginmanager.cpp` supports loading/removing simulator plugins and dependency checks.
- It currently operates over simulation/kernel plugin insertion and not over GUI graphical extension contribution points.

## Build Characteristics Relevant to Extensions

- `source/applications/gui/qt/GenesysQtGUI/CMakeLists.txt` uses recursive source globbing.
- `source/applications/gui/qt/GenesysQtGUI/GenesysQtGUI.pro` also uses recursive source/header inclusion.

This means extension contracts can be introduced as headers now without immediate integration churn.

## Gap Summary

The GUI has internal modularization (controllers/services) but does not yet have:

- a generic graphical plugin contract;
- a contribution registry for menu/toolbar/dock/window extension points;
- runtime dependency gating between GUI contributions and model plugins;
- a lifecycle model for loading/unloading GUI contributions independently.

## Step 2 Output: Generic Contract

File added:

- `source/applications/gui/qt/GenesysQtGUI/extensions/GuiExtensionContracts.h`

Contract elements introduced:

- `GuiExtensionRuntimeContext`
- `GuiActionContribution`
- `GuiDockContribution`
- `GuiWindowContribution`
- `GuiExtensionRegistry`
- `GuiExtensionPlugin` (interface)

Design intent:

- Keep the contract generic (not tied to biosimulation).
- Allow future plugins to contribute actions/docks/windows.
- Allow future dependency checks through `requiredModelPlugins()`.
- Keep integration deferred to a later step where `MainWindow` consumes a registry.

## Immediate Next Integration Step (Not Done Yet)

When moving to the next phase, implement a GUI extension manager that:

- discovers `GuiExtensionPlugin` providers;
- filters by `requiredModelPlugins()`;
- materializes actions/docks/windows into `MainWindow` at runtime;
- removes/disables contributions when dependencies are unavailable.


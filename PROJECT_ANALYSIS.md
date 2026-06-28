# Genesys Simulator — Project Analysis Notes

## Overview

**Genesys Simulator** is a C++23 discrete-event simulation platform with a plugin-driven architecture.
It has three frontends: Terminal CLI (GenesysShell), Qt GUI, and Web.

---

## Two Plugin Systems

### 1. Kernel Simulation Plugins
- Managed by `PluginManager` (source/kernel/simulator/)
- Loaded from `autoloadplugins.txt` at startup
- Represent simulation components: `Seize`, `Queue`, `BioNetwork`, `BacteriaColony`, etc.
- Each plugin has a `PluginTypename`, `Category`, flags (Source/Sink/Component/DataElement), fields, description
- Appear in the GUI's `treeWidget_Plugins` (left sidebar) — draggable into the model canvas

**Terminal (`plugin --list`)** shows all loaded kernel plugins with full metadata.

### 2. GUI Extension Plugins
- Registered at compile time via `REGISTER_GUI_EXTENSION_PLUGIN(PluginType)` macro
- Live in `source/applications/gui/qt/GenesysQtGUI/extensions/`
- Contribute **menu items**, **dock panels**, and **dialogs** to the Qt GUI
- Each extension declares `requiredModelPlugins()` — a list of kernel plugin typenames
- Only activated when all required kernel plugins are loaded

---

## GUI Extension Infrastructure (already implemented)

| Class | File | Purpose |
|---|---|---|
| `GuiExtensionPlugin` | `GuiExtensionContracts.h` | Abstract base for GUI extensions |
| `GuiExtensionRegistry` | `GuiExtensionContracts.h` | Collects contributions (actions/docks/windows) |
| `GuiExtensionPluginCatalog` | `GuiExtensionPluginCatalog.h/cpp` | Static registry + JSON config (`gui_extensions.json`) |
| `GuiExtensionManager` | `GuiExtensionManager.h/cpp` | Materializes contributions into MainWindow |
| `GuiExtensionRuntimeContext` | `GuiExtensionContracts.h` | Runtime access to Simulator, MainWindow, Scene |

`GuiExtensionManager::rebuild()` iterates registered extensions, checks dependencies, then:
- `_applyActionContribution()` → adds `QAction` to menu + toolbar
- `_applyWindowContribution()` → adds `QAction` to menu that opens a dialog
- `_applyDockContribution()` → adds `QDockWidget` to MainWindow

---

## Registered GUI Extension Plugins

| Plugin Class | extensionId | requiredModelPlugins | Contributes |
|---|---|---|---|
| CoreGuiExtensionPlugin | `core.gui.extension` | none | (empty — placeholder) |
| ModelPluginInspectorGuiExtensionPlugin | `gui.extensions.model.plugin.inspector` | none | Tools/Extensions → "Loaded Model Plugins" dialog |
| GuiExtensionConfigurationGuiExtensionPlugin | `gui.extensions.configuration.manager` | none | Tools/Extensions → "Manage Graphical Extensions..." |
| BioNetworkAwareGuiExtensionPlugin | `gui.extensions.dependency.bionetwork` | `bionetwork` | Tools/Extensions → "BioNetwork Extension Status" |
| BioNetworkEditorGuiExtensionPlugin | `gui.extensions.bio.network.editor` | `bionetwork` | Tools/Extensions/Biological → network CRUD actions |
| BioNetworkExecutionGuiExtensionPlugin | `gui.extensions.bio.network.execution` | `bionetwork` | Tools/Extensions/Biological → network run/inspect actions |
| BioSpeciesManagerGuiExtensionPlugin | `gui.extensions.bio.species.manager` | `biospecies` | Tools/Extensions/Biological → species CRUD |
| BioReactionKineticsGuiExtensionPlugin | `gui.extensions.bio.reaction.kinetics` | `bioreaction` | Tools/Extensions/Biological → reaction CRUD |
| BioParameterManagerGuiExtensionPlugin | various | `bioparameter` | Tools/Extensions/Biological → parameter actions |
| BioReactionStoichiometryGuiExtensionPlugin | various | `bioreaction` | Tools/Extensions/Biological → stoichiometry |
| BioSimulationResultsGuiExtensionPlugin | various | `bionetwork` | Tools/Extensions/Biological → results dialog |
| BioSBMLInteroperabilityGuiExtensionPlugin | various | various | Tools/Extensions/Biological → SBML import/export |
| BacteriaColonyViewerGuiExtensionPlugin | `gui.extensions.gro.bacteria.colony.viewer` | `bacteriacolony` | Tools/Extensions/Biological → Colony Viewer dialog |

---

## The Key Gap: Startup Timing Bug

### Startup sequence in `MainWindow::MainWindow()`:

```
Line  506: _rebuildViewDependentControllers()
              └─ ends at line 1684: _refreshGuiExtensions()  ← RUNS HERE, no plugins loaded yet
Line  585: autoInsertPlugins("autoloadplugins.txt")           ← plugins load HERE
Line  592: _pluginCatalogController->applyCategoryExpansionPolicy()
           (no _refreshGuiExtensions() call after this!)
```

**Problem**: `_refreshGuiExtensions()` fires before any kernel plugins are loaded.
All Bio* extensions have `requiredModelPlugins()` → dependency check fails → no menu items appear.

### Dependency normalization (works correctly)
`normalizePluginId()` in `GuiExtensionManager.cpp` lowercases and strips `.so`:
- Kernel typename `"BioNetwork"` → normalized to `"bionetwork"` ✓
- Required `"bionetwork"` → matches ✓
- Kernel typename `"BacteriaColony"` → `"bacteriacolony"` ✓

---

## Plan: What Needs to Be Done

### Step 1 — Fix the startup timing (1-line fix)
In `mainwindow.cpp`, after the autoload block (after line 592), add:

```cpp
_refreshGuiExtensions();
```

**File**: `source/applications/gui/qt/GenesysQtGUI/mainwindow.cpp`
**After line**: ~592 (`_pluginCatalogController->applyCategoryExpansionPolicy();`)

This makes `_refreshGuiExtensions()` run again after kernel plugins are loaded,
so Bio* extensions with satisfied dependencies get their menus/actions added.

### Step 2 — Verify `autoloadplugins.txt` includes Bio kernel plugins
The GUI extension plugins can only activate if their kernel plugin dependencies are loaded.
The Bio extensions require these kernel plugins to exist in `autoloadplugins.txt`:
- `BioNetwork` (for BioNetworkEditor, BioNetworkExecution, BioSimulationResults, BioSBML, BioNetworkAware extensions)
- `BioSpecies` (for BioSpeciesManager)
- `BioReaction` (for BioReactionKinetics, BioReactionStoichiometry)
- `BioParameter` (for BioParameterManager)
- `BacteriaColony` (for BacteriaColonyViewer)

### Step 3 — (Optional) Verify plugin typename matching
After the fix, run the app and check `Tools → Extensions` menu.
If Bio menus don't appear, check that `plugin->getPluginInfo()->getPluginTypename()`
returns strings like `"BioNetwork"` (matches after normalization).

### Step 4 — (Optional) Add missing autoload file to source control
If `autoloadplugins.txt` is not in the repo but needed for Bio domain functionality,
add a default one alongside the binary or in the resources.

---

## Key File Locations

| What | Where |
|---|---|
| Startup autoload + timing fix location | `mainwindow.cpp` lines 581–592 |
| GUI extensions rebuild | `mainwindow.cpp::_refreshGuiExtensions()` lines 1687–1704 |
| Dependency check | `GuiExtensionManager.cpp::_isPluginDependenciesSatisfied()` lines 95–124 |
| Kernel plugin typename normalization | `GuiExtensionManager.cpp::normalizePluginId()` lines 20–33 |
| Extension registration macro | `GuiExtensionPluginCatalog.h` line 28 |
| All registered extensions | `source/applications/gui/qt/GenesysQtGUI/extensions/*.cpp` |

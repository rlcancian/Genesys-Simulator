---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
date: 2026-11-16
status: in-progress-ci-pending
branch: WiP202611/gui-extensions-f1-dependencies
base_commit: 6d1c4be0b385ab7b07ea4d995e2e105fb4d7ee03
tracks: AUTO-GUI-003,HUM-ARCH-004,HUM-ARCH-006
---

# GUI graphical extensions — F1 shared dependency resolution

## Objective and fixed baseline

Implement F1 on `WorkInProgress` without depending on unmerged plans. This is **source code and targeted tests**, not the F2 animation-factory implementation.

Current baseline had a divergent policy: `GuiExtensionManager::_isPluginDependenciesSatisfied` checked AND legacy `requiredModelPlugins()`, whereas `GuiExtensionPluginCatalog::resolvedPlugins()` only applied enable/disable configuration and did not enforce model requirements for diagram editors, visualization or rendering dispatch.

## Implemented F1 source boundaries

- `source/applications/gui/genesys/extensions/GuiExtensionDependencyResolver.h`: shared, deterministic resolver with canonical normalized IDs; legacy AND; typed `ModelPlugin` / `GuiExtension` / `SystemCapability`; required vs optional groups; opt-in explicit ALL/ANY; least-fixed-point transitive GUI dependencies and cyclic rejection; duplicate/empty/missing/cycle diagnostics; numeric dotted minimum-version comparisons with **unknown version failing closed** when explicitly requested.
- `GuiExtensionContracts.h`: additive nonpure `GuiExtensionPlugin::dependencyGroups()`, backward-compatible with all 24 concrete extensions; contribution-level opt-in `eligibility` metadata on all 11 categories, copied through `GuiExtensionRegistry::appendEligibleFrom`.
- `GuiExtensionManager.{h,cpp}`: unified activation and contribution filtering, retaining legacy public API; available model version metadata.
- `GuiExtensionPluginCatalog.{h,cpp}`: same resolver for all existing dispatch paths, with diagnostic `resolution()` and per-contribution filters; static registration stays unchanged; an unknown initial loaded-model inventory retains legacy behavior until `setAvailableModelPluginIds` provides an authoritative list. A known empty list does **not** count as an unknown list.
- `mainwindow.cpp`: forwards actual loaded model-plugin type IDs and versions from `PluginManager`/`PluginInformation` to both Manager and Catalog when GUI extensions are refreshed. No kernel source modification.
- `source/tests/unit/test_gui_extension_dependencies.cpp`: 23 Google Test test cases; included in existing Qt-backed `genesys_test_gui_gmdd_layout` test target.

## Contract semantics and remaining risk

1. `requiredModelPlugins()`: strict legacy ALL when inventory is known; never reinterpret legacy two-plugin requirements as OR. For truly independent requirements use explicit typed ANY metadata later on each concrete extension.
2. GUI-to-GUI dependencies are matched by stable extension ID, not C++ inheritance; activation follows least fixed point, so pure cycles cannot activate. Results order dependency-first, stable for unrelated plugins.
3. Optional group missing: does not block the plugin or independent contribution. Each contribution may declare a required group separately; the registry filters that contribution without modifying other kinds.
4. Version constraints: numeric dotted minima, e.g. `1.2.3`, are accepted, no arbitrary strings or installation commands. If a versioned requirement lacks authoritative version metadata, it is not silently satisfied. Full package/ABI version negotiation remains out of scope.
5. Static catalogue calls without a known model inventory keep the old loaded-plugin behavior for `requiredModelPlugins()`; explicit new required groups still check their prerequisites. When the main GUI supplies the inventory, the same gating policy applies in both dispatchers. This transitional compatibility behavior should be removed only after all call paths have explicit reliable context.
6. Global catalog state means multi-window/simulator lifecycle should receive a later scoped review; this F1 does not introduce dynamic unload or modify kernel model ownership.

## Validation matrix

| Acceptance criterion | State |
|---|---|
| Branch created directly from pinned WorkInProgress SHA | PASS (GitHub evidence) |
| Shared source implementation and Qt/GTest additions published | PASS (GitHub diff) |
| No kernel/parser/dynamic-loading changes | PASS (changed-file inspection required before merge) |
| Header/API and code compile | **UNKNOWN — pending GitHub Actions** |
| F1 23 targeted Qt/GTest scenarios | **UNKNOWN — pending GitHub Actions** |
| Full ordinary tests-unit and affected GUI build | **UNKNOWN — pending GitHub Actions** |
| Kernel unit, smoke, packaging and integration checks | **UNKNOWN — pending GitHub Actions** |
| Interactive GUI desktop smoke / sanitizers | **NOT RUN** (may require local setup if CI coverage insufficient) |
| PR merge into WorkInProgress | **NOT YET** |

A merged PR or compiler exit code alone is not F1 completion. Verify emitted tests, no regression and the final PR head checks before assigning `done_confirmed`.

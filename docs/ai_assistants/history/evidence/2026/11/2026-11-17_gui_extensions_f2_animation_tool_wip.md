---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
date: 2026-11-17
status: implementation-started-validation-pending
branch: WiP202611/gui-extensions-f2-animation-tool
base_branch: WiP202611/gui-extensions-f1-dependencies
base_commit: 8196fc7167dc07b91c54e31de6901469d1f48a32
tracks: AUTO-GUI-003,HUM-ARCH-004,HUM-ARCH-006
---

# F2 — independently registered GUI animation tools (implementation draft)

## Explicit execution approval and gating

The maintainer authorized **starting F2 before F1 has completed mandatory validation**, with the intention of doing **additional local GUI tests after F2**. This does **not** waive final PR integration gates: F1 remains unmerged, and F2's branch is explicitly based on the unmerged F1 head; do not merge F2 to `WorkInProgress` before F1 is approved and integrated, and do not claim F2 is validated by compilation alone.

## Current source-backed implementation

- `GuiExtensionContracts.h`: additive `GuiAnimationToolContribution` with stable ID, label, optional `View/Animate` menu and toolbar locations, eligibility, enabled predicate and a `std::unique_ptr<QGraphicsItem>` factory. Added `addAnimationTool`, `animationTools` and registry filtering/clear support; **existing `GuiExtensionPlugin` required methods unchanged**.
- `GuiExtensionManager.cpp`: generic translation of the above contribution into the existing `GuiActionContribution`/QAction mechanism. With an active `ModelGraphicsScene` and `QUndoStack`, the factory returns an unowned candidate; the generic host submits it to the existing `AddUndoCommand` and `ModelGraphicsScene::addDrawing` lifecycle, then relinquishes the factory's `unique_ptr`. Tool activation is safely disabled without scene/undo stack.
- `extensions/plugins/scene/SceneMarkerGuiExtensionPlugin.cpp`: independent statically registered `GuiExtensionPlugin` subclass, no simulation plugin requirement or hardcoded GUI host class references. It contributes `View/Animate > Scene Marker` and a concrete movable/selectable `QGraphicsRectItem` derivative with an interactive resize handle and custom drawing.
- `source/tests/unit/test_gui_animation_tool_f2.cpp`: four Qt/Google Test cases for plugin/factory registration, scene insertion + undo/redo + geometry change, unavailable scene/undo and manager rebuild without duplicate QAction; added to the existing GMDD Qt/GTest CMake target.
- No kernel/parser files, model plugins, dynamic loading or persisted `.gui` format changes. Existing scene drawing modes and controllers are **not** modified to know `SceneMarkerGuiExtensionPlugin`.

## Acceptance and remaining work

| Criterion | Current status |
|---|---|
| Independent static exemplar + registered factory and menu integration | IMPLEMENTED in source (runtime UNKNOWN) |
| Generic Qt graphics item insertion with existing undo stack | IMPLEMENTED in source (runtime UNKNOWN) |
| Movable/selectable item + custom resize handle | IMPLEMENTED in source (manual GUI interaction UNKNOWN) |
| No concrete exemplar includes in MainWindow/scene/controllers | PASS by scoped code inspection |
| Four focused Qt/Google Test scenarios authored | PASS in source; execution UNKNOWN |
| F1 mandatory CI and its PR merge | UNKNOWN / NOT YET MERGED at draft creation |
| F2 GUI CMake/Ninja build, Qt tests, interactive resize/drag, sanitizers | NOT YET VALIDATED |
| F2 persistence / event bridge | OUT OF SCOPE; planned F3 |
| Release or merge to WorkInProgress | NOT AUTHORIZED BEFORE F1 INTEGRATION AND FINAL CHECKS |

## Focused validation plan after F2 code review

1. Rebase/merge F1 into `WorkInProgress` **after** its mandatory CI passes, and retarget/rebase F2 onto the resulting integration head.
2. Configure GUI and `tests-unit` with Qt6/C++23, compile via CMake/Ninja; run `GuiAnimationToolF2.*` and `GuiExtensionDependencyF1.*` alongside GUI/GMDD regression and ordinary unit/kernel/smoke suites.
3. On desktop GUI, open a model scene; use **View > Animations > Scene Marker**; verify creation, selection, drag, resize bottom-right, Delete, Undo, Redo, scene switch and model close; test action disabled without active undo stack.
4. Validate ownership on cancellation/model change/scene destruction with ASan/UBSan/LSan where available. Confirm old animation tools unchanged.
5. Review final GitHub Actions and attach command output + actual exit status to this evidence record; only then mark F2 done and merge.

**Known limitation:** insertion is menu-triggered at the center of the current scene, rather than a drag-to-draw gesture. The exemplar intentionally exercises a real factory and editable Qt item before any F3 event or serialization contract. If a drawing gesture is mandatory for F2 acceptance, add that in a separate bounded change only after confirming the existing scene's mouse event and undo ownership invariants.

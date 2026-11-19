---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
status: historical
---

# GUI captions and inspector selection — 2026-11-17

Base: `615d57400`, direct `WorkInProgress` execution explicitly requested by maintainer.
Initial working tree clean; fetched origin and confirmed zero ahead/behind.

Label is primary and Name secondary, with Name-only fallback for empty/whitespace
labels. Both item contexts read live properties. Data-definition captions are
normalized after extension configuration; specialized shapes, details and semantic
metadata remain intact. Renderer typography and geometry are unchanged.

Selection previously looked up rows before ensuring population; tab changes could
rebuild and invalidate a matched row. Populate under tree signal blocking before
lookup and block tab signals during navigation. Continue pointer-identity matching,
ancestor expansion and scrolling. Clear stale row selection. Skip inspector
navigation during property-editor post-commit processing. Existing simulation,
multiple-selection and unsupported-item guards remain.

Executed: CMake/Ninja `gui-app`, `tests-unit`; new offscreen GTest regressions
`GuiGmddLayout.ObjectCaptionsPreferLabelAndReadLiveProperties` and
`ModelInspectorSelectionRegression.PopulatesHiddenTreesAndKeepsSelectionAcrossTabs`
PASS. The latter exercises real MainWindow tabs/trees, empty trees, repeated
component/data alternation and multiple selection. Caption cases cover distinct,
empty and long labels and live name changes. Existing renderer tests exercise
Classic/Organic and specialized glyphs. Manual make.sh regenerates published PDF.

The initial test fixture kept a graphical definition across automatic layout
refresh; GDB identified stale QGraphicsItem use in the test. Fixture now installs
the selectable definition immediately before selection. The first full CTest run
overlapped relinking and produced executable-launch errors; superseded by a
post-build full run recorded below.

Limits: no human visual review; no dedicated new simulation pause/run or saved-model
reopen selection interaction test. These scenarios remain UNKNOWN as interaction
claims; existing guards/persistence tests are supporting evidence only.

Final stable-build validation: `QT_QPA_PLATFORM=offscreen ctest --preset tests-unit
--output-on-failure`: 2147/2147 executed tests PASS, four existing disabled tests,
126.30 seconds. Standalone renderer suite: 11/11 PASS. GUI and unit builds PASS;
manual build PASS; diff whitespace validation PASS. No kernel source changed.

## Property-reference navigation crash follow-up

User supplied SIGSEGV in `QTreeWidgetItem::setExpanded`, called by
`sceneSelectionChanged` at function offset `+0x5fc`, originating from the property
reference combo's SelectAndEdit action. Current GUI binary has the setExpanded
call at `+0x6e8` (return `+0x6ed`), so the supplied frame does not match the current
binary's instruction layout. This suggests an earlier executable; the crashing
process itself was unavailable for byte-for-byte verification.

Added real MainWindow regression
`ModelInspectorSelectionRegression.PropertyReferenceComboSelectsAndEditsInMainWindow`:
Seize references Queue; populate both inspector tabs, edit the Queue property,
activate `Selecionar Queue e Editar`, assert pointer identity in tree and scene,
correct tab and unchanged kernel reference. Differential execution with only
mainwindow_scene.cpp from base 615d57400 failed (tree currentItem became null).
Current 192a958ee implementation passed. The differential run demonstrated the
old invalidation behavior but did not reproduce SIGSEGV itself. No additional
production fix is inferred from a passing reproduction; retain the existing fix
and lock down the previously uncovered property-reference navigation path.
Manual impact: none for this follow-up; regression/evidence only, existing manual
already describes navigation behavior.
Final follow-up checks: both inspector selection regressions repeated five times,
10/10 PASS under Qt offscreen. Unit target rebuilt successfully. GUI target rebuilt
successfully. No production source changes in follow-up.

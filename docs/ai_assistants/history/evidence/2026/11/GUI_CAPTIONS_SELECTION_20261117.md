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

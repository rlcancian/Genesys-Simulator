---
document_type: evidence
authority: scoped
owner: ai-assistant
date: 2026-11-09
task: Wait Queue and SignalData explicit selector correction
status: provisional
---

# Wait Queue and SignalData selector correction

## Scope

Branch `WiP20261109/wait-queue-signaldata-selector`, based on `WorkInProgress` at `f13e128906fdf9c791fb3d0ff171f67c90b86497`.

## Confirmed diagnosis

- `Wait::_createNonEditableDataDefinitions()` created a Queue when the reference was null and registered it through the owning internal-data bucket.
- `Wait::_createEditableDataDefinitions()` and `Signal::_createEditableDataDefinitions()` created SignalData objects implicitly.
- The Property Editor changed an existing reference into edit/remove actions, hiding alternative compatible instances.
- `SimulationControlGenericClass::setValue()` created objects for unresolved names, conflating reference resolution with explicit creation.
- Explicit creation from a browser bound directly to a kernel object failed when no graphical scene was present because the editor could not obtain the model from the edited object.

## Changes

- Wait Queue and selected SignalData references now use optional editable attached associations, not owning internal associations.
- Wait and Signal no longer create Queue or SignalData during association materialization.
- Wait removes SignalData callbacks on reassignment, deactivation, and destruction; it registers callbacks only for `WaitForSignal` and preserves configured SignalData across mode changes and persistence.
- Wait and Signal dispatch paths guard null required references; model checks continue to diagnose missing configuration.
- Generic model-data controls reject unknown reference names. Object construction occurs only through the explicit creation operation.
- The parent-Model fallback for reference lookup and creation is restricted to the three explicit selector controls; legacy generic controls retain their scene-based behavior.
- Three controls opt into keeping compatible object choices visible while a reference is set; other controls keep their existing selector behavior.
- Explicit reference creation resolves the model from the edited object's owning Model when a graphical scene is absent.

## Validation

- `cmake --preset tests-unit`: PASS (configuration).
- `cmake --build --preset tests-unit --parallel 4`: PASS.
- Focused CTest selection for Wait/Signal materialization, handler rechecks, validation, shared SignalData selection, Queue selector replacement/creation, and generic selector regressions: PASS, 13/13.
- Full serial `ctest --test-dir build/tests-unit --output-on-failure`: PASS (2,048 registered test cases; CTest exit status 0). An earlier run exposed five failing editor tests; selector model lookup was narrowed to the three explicit controls, and all five now pass both individually and in the full rerun.
- `cmake --preset gui-app`: PASS (configuration).
- `cmake --build --preset gui-app --parallel 4`: PASS.
- The entire 2,046-test CTest run at 12-way parallelism previously exposed `SourceCodePropertyEditor` failures and was interrupted. The serial full-suite rerun passed after restricting scene-less resolution to the three target selectors.
- The focused `tests-kernel-unit` selection passed 7/7. Final `tests-kernel-unit` build/test target: PASS (11/11).
- `git diff --check`: PASS.

## Remaining validation

- Hosted checks on the final PR head; end-to-end Signal/Wait execution, full `.gen`/`.gui` identity coverage, legacy Queue fixture, sanitizers, broader Seize regression, and interactive main-GUI workflow remain unverified.
- Shared Wait/Signal end-to-end simulation test.
- `.gen` and `.gui` round-trip identity tests, including legacy auto-created Wait Queue fixtures.
- Ownership sanitizer coverage and broader Seize/other generic selector regression.
- Interactive main-GUI insertion/editing workflow.

Do not treat this evidence record or the draft PR as acceptance that the full criteria are complete.

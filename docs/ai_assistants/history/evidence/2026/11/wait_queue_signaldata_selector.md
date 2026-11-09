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
- Three controls opt into keeping compatible object choices visible while a reference is set; other controls keep their existing selector behavior.
- Explicit reference creation resolves the model from the edited object's owning Model when a graphical scene is absent.

## Validation

- `cmake --preset tests-unit`: PASS (configuration).
- `cmake --build --preset tests-unit --parallel 4`: PASS.
- Focused CTest selection for Wait materialization, handler rechecks, Wait validation, shared SignalData selection, and Queue selector replacement/creation: PASS, 8/8 (including explicit SignalData creation/reuse and Queue plus SignalData name-based identity restoration).
- `cmake --preset gui-app`: PASS (configuration).
- `cmake --build --preset gui-app --parallel 4`: PASS.
- The entire 2,046-test CTest run was attempted with 12-way parallelism and interrupted at 253/2046 after `SourceCodePropertyEditor` tests failed. The cause is unknown; this is not a full-suite pass.
- The focused `tests-kernel-unit` selection passed 7/7. The `tests-kernel-unit` build target also completed its configured unit-test execution successfully (11/11 reported by its test summary).
- `git diff --check`: PASS.

## Remaining validation

- Full unit suite and smoke suite on the final PR head; the smoke preset run is still in progress.
- Shared Wait/Signal end-to-end simulation test.
- `.gen` and `.gui` round-trip identity tests, including legacy auto-created Wait Queue fixtures.
- Ownership sanitizer coverage and broader Seize/other generic selector regression.
- Interactive main-GUI insertion/editing workflow.

Do not treat this evidence record or the draft PR as acceptance that the full criteria are complete.

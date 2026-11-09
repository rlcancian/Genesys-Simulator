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
- Focused CTest selection for Wait materialization, handler rechecks, Wait validation, shared SignalData selection, and Queue selector replacement/creation: PASS, 6/6.
- `cmake --preset gui-app`: PASS (configuration).
- `cmake --build --preset gui-app --parallel 4`: PASS.
- A parallel run of the entire 2,046-test suite was interrupted after unrelated GUI tests failed under concurrent execution; it is not a full-suite pass. Re-run the full suite under the repository's expected scheduling and triage failures.
- `git diff --check`: PASS.

## Remaining validation

- Full unit, kernel-unit, and smoke suites on the final PR head.
- SignalData explicit-creation and shared Wait/Signal end-to-end simulation test.
- `.gen` and `.gui` round-trip identity tests, including legacy auto-created Wait Queue fixtures.
- Ownership sanitizer coverage and broader Seize/other generic selector regression.
- Interactive main-GUI insertion/editing workflow.

Do not treat this evidence record or the draft PR as acceptance that the full criteria are complete.

# Release runtime abort containment — 2026-11-17

Base 8e98e031b, WorkInProgress, clean initial tree and zero ahead/behind after fetch.
User supplied SIGABRT in Release::_onDispatchEvent at +0x14c. Symbolication of the
current GUI maps that return address immediately after the busy-count assert.
The assert predates this session (git blame: 15f3a0475e, 2022-08-04).

Reproduced exact assertion failure with a real Resource (1 busy unit) and Release
request for 2 units. Original focused test aborted. Replace the assert with a
runtime_error containing Release/Resource names and requested/busy quantities;
check unresolved Resource before dereference. Existing ModelSimulation event
catch reports the exception. Do not clamp quantity, manufacture occupancy, forward
the rejected entity, or change valid-release behavior. This is containment and
an actionable diagnostic, not correction of the user's model configuration.

Three focused runtime tests PASS: over-release leaves resource unchanged and
schedules no event; valid release frees units and routes the entity; complete
Create -> Release -> Dispose simulation reports the error without process abort.
First full unit run: 2154 PASS, one test expectation failed because errorMessages
stores contextual text while exception details are delivered in TraceErrorEvent.
Corrected test to assert contextual component diagnostic; no production change.
All three focused tests then PASS (0.172s). GUI gui-app built PASS; manual make.sh
PASS, published PDF regenerated. No parser/property-editor source changed.

Open question: user's model file not supplied. Wrong Release quantity, mismatched
Seize/Release resources, alternate paths, or set-member selection require that
model to distinguish. No claim that its simulation now produces valid results.

Final full unit run: 2153 PASS / 2155 executed, four disabled; two EFSM persistence
load tests failed under parallel execution. Sequential ctest --rerun-failed passed
both (2/2, 0.54s). They also passed in the preceding full run. This is evidence of
parallel-run sensitivity, not proof of its cause; no EFSM source changed. All three
Release regressions passed in the full run. Whitespace diff check PASS.

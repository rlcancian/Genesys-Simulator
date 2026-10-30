---
document_type: validation-evidence
authority: informative
owner: project-maintainer
created: 2026-10-30
status: active
tracks: 511
---

# PR #550 — Three-domain continuous calendar evidence

## 1. Scope

Added a regression test proving that `ODESolver`, `DiffusionField`, and
`BioNetwork` can run concurrently from replication-start callbacks and complete
through one real `ModelSimulation` event calendar with distinct
`StepsToSynchronize` values (2, 3, and 1 respectively).

## 2. Evidence

- **Confirmed in code:** the test is
  `ContinuousCalendarTest.OdeDiffusionAndBioNetworkShareOneRealEventCalendar`
  in `source/tests/unit/test_plugins_continuous_diffusion.cpp`.
- **Confirmed by execution:** the focused test passed after anchoring the
  biochemical definitions through a connected `BioSimulate` component. All
  three definitions reached `Completed`; ODE state reached 4, diffusion local
  time reached 4, substrate plus product remained 10 within `1e-12`, and the
  global clock ended at 4.
- **Confirmed by execution:** the complete focused target ran 40 tests; all 40
  passed.
- **Confirmed by execution:** GitHub checks for source HEAD
  `f19c4eed8f37c243231d71e6c31acaeaa467271f` were all green when queried,
  including unit, kernel, smoke, GUI startup, packaging, and documentation
  governance workflows. This new local test had not yet been pushed at evidence
  capture, so those checks do not cover it.

## 3. Bounded conclusion

The test confirms coexistence and completion of the three continuous data
models on a shared real simulation calendar under different synchronization
intervals. It does not test calendar interruptions, component-activated ODE or
diffusion runs, cross-model input mutation, simultaneous timestamp ordering,
full persistence round-trips, cancellation/destruction, or all replication
combinations. Existing tests cover some of these cases separately; the full
acceptance matrix remains incomplete.

## 4. Reproduction

```text
cmake --build --preset tests-unit --target genesys_test_plugins_continuous_diffusion --parallel 2
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.OdeDiffusionAndBioNetworkShareOneRealEventCalendar
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion
```

Final classification remains `PARTIALLY_VALIDATED`; this evidence does not
satisfy the open `HUM-SCI-002` invalidation condition or human review required
by the repository governance.

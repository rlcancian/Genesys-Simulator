---
document_type: validation-evidence
authority: informative
owner: continuous-models-implementation
created: 2026-10-30
status: active
tracks: 550
---

# PR #550 — DiffusionSimulate calendar interruption evidence

## 1. Escopo analisado

Added a real-kernel event-calendar regression for the concrete
`DiffusionSimulate` component. The test complements the existing real-calendar
ODE and BioSimulate interruptions and the generic diffusion lifecycle fixture.
No production code or numerical method changed.

## 2. Diagnóstico inicial

**Confirmed in code:** existing interruption coverage exercised
`ContinuousSystemComponent` and `BioSimulate` as concrete components, while the
diffusion calendar continuation used a small `ContinuousModelComponent` test
probe. The direct `DiffusionSimulateComponentRunsField` test exercised the
concrete plugin synchronously, without calendar interruption.

**Confirmed by execution:** `Model::check()` rejected a registered
`DiffusionSimulate` instance in this unit target because its runtime plugin type
was not present in the test's `PluginManager`. The implementation is linked
statically into the test, but the test setup does not register the dynamic
plugin descriptor. The fixture keeps an independently valid checked source to
sink graph and schedules the entity directly into the concrete component; this
exercises its actual dispatch/completion methods without fabricating plugin
registration or changing production code.

## 3. Evidências e problemas encontrados

- **Confirmed by execution:**
  `ContinuousCalendarTest.DiffusionSimulateRetainsEntityAcrossCalendarInterruption`
  passed on the real `ModelSimulation` calendar.
- **Confirmed by execution:** the discrete probe ran at global time 5.5 and
  observed the last confirmed diffusion time 5.0.
- **Confirmed by execution:** the same requested entity was forwarded exactly
  once to the downstream `Dispose` at time 10.0; `DiffusionField` and
  `DiffusionSimulate` reported successful completion at local time 10.0.
- **Confirmed by execution:** the complete continuous diffusion/Observable/
  calendar test target passed 41/41.
- **Confirmed by execution:** local `tests-unit` passed 1,942 with four disabled
  and zero failures (1,946 enumerated); `tests-kernel-unit` passed 1,941 with
  four disabled and zero failures (1,945 enumerated); smoke passed 3/3.
- **Confirmed by execution:** focused ASan/UBSan calendar and Observable tests
  passed 21/21 with LeakSanitizer disabled.
- **Confirmed in code:** no production source, event comparator, model clock,
  plugin registration, or numerical algorithm was changed by this test.

## 4. Impacto arquitetural

This closes one concrete component-path gap: the diffusion plugin now has an
executed interruption/retention/continuation/forwarding regression using the
actual simulation calendar. The static unit fixture cannot establish runtime
factory discovery or persisted-model loading for `DiffusionSimulate`; those
remain covered by separate plugin/GUI gates and are not claimed here.

## 5. Solução implementada

The fixture schedules one entity to `DiffusionSimulate` and one ordinary
continuous-time observer event at 5.5. The diffusion field has `StepSize=1`,
`StopTime=10`, and `StepsToSynchronize=10`. The event checks confirmed time 5;
the component resumes the same run and forwards the same entity once at 10.

The test removes the component and discrete probe from the model's registered
component list because the static test target has no dynamic descriptor for the
concrete diffusion plugin and the standalone observer has no ordinary graph
input. It retains a valid checked Create-to-Dispose graph, while injecting the
same scheduled entity directly into the component. This intentionally limits
the conclusion to calendar execution and concrete component lifecycle.

## 6. Patch e commit

- `c9881a20b` — `test(continuous): cover diffusion component calendar resume`.
- Source/test base at implementation: `27f03ad13a993662d5630637b52a0f4dd8e07222`.
- GitHub checks for `c9881a20b` had not yet been run at evidence creation.

## 7. Validação

```text
cmake --build --preset tests-unit --target genesys_test_plugins_continuous_diffusion --parallel 2
build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter=ContinuousCalendarTest.DiffusionSimulateRetainsEntityAcrossCalendarInterruption
# passed

build/tests-unit/source/tests/unit/genesys_test_plugins_continuous_diffusion
# 41/41 passed

ASAN_OPTIONS=detect_leaks=0 build/observable-asan/source/tests/unit/genesys_test_plugins_continuous_diffusion --gtest_filter='ContinuousCalendarTest.*:ContinuousObservableTest.*'
# 21/21 passed (ASan/UBSan; LSan disabled)

ctest --preset tests-unit --output-on-failure
# 1,942 passed; 4 disabled; 0 failed

ctest --preset tests-kernel-unit --output-on-failure
# 1,941 passed; 4 disabled; 0 failed

ctest --preset tests-smoke --output-on-failure
# 3/3 passed
```

## 8. Riscos remanescentes

- The test does not prove dynamic plugin discovery or GUI factory instantiation
  for `DiffusionSimulate`.
- It does not expand the cancellation/destruction, multiple-entity, or
  persistence matrices for all domains.
- Ordinary events that mutate inputs already used by precomputed continuous
  work remain subject to the unresolved `HUM-SCI-002` invalidation condition.
- PR #550 remains draft and still requires independent review of its complete
  historical diff.

## 9. Próximos passos

1. Run the focused documentation validation and push the test with its evidence.
2. Verify all required CI checks on the resulting final SHA.
3. Preserve draft status until independent review and the recorded human
   scientific condition are addressed.

**Maturity:** `PARTIALLY_VALIDATED`.

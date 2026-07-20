# Handoff — GenESyS 2026 consolidation

## 1. Final task state

- Overall status: `partially-validated`.
- Repository: `rlcancian/Genesys-Simulator`.
- Consolidation base branch: `WorkInProgress`.
- Original inspected base commit: `b25a4d2ec31abe27f1bf3597a5135fa4828fbc35`.
- Work branch: `audit/genesys-2026-consolidation-plan-20260720`.
- Pull request: draft PR #469, `docs: add GenESyS 2026 consolidation plan`, targeting `WorkInProgress`.
- Changes in the PR remain documentation-only.
- Local configure/build/test execution: not available to this ChatGPT Web session.
- Remote CI checkpoint validated:
  - head commit: `a60ca65aae4147a7d9b14bdadd9e8d39958bcaaf`;
  - merge commit: `67d895f67307a6ba614e0c4cc5be88fbf24390ab`;
  - workflow run: `29771060564`, run number 107;
  - conclusion: `success`.
- `tests-unit`: configure, build, and CTest passed.
- GUI GMDD focused diagnostics: configure/build passed and all three focused tests passed.
- `tests-kernel-unit` and `tests-smoke`: still not executed in the available CI checkpoint.
- Release readiness: not established.

## 2. What was done

- Read the root `README.md` and mandatory `docs/ai_assistants/README.md`.
- Inspected the root CMake/Ninja/C++23 build graph, presets, applications, plugins, tools, tests, and CI.
- Inspected selected numerical, ownership, security-sensitive, incomplete, and scientific areas.
- Sampled relevant 2026 pull requests while separating historical claims from current branch evidence.
- Created the main consolidation plan, module inventory, test matrix, handoff, human-decision record, and later decision addendum.
- Recorded future plans for numerical/statistical references, multiobjective optimization, and AI virtual-cell research.
- Recorded the Qt6-only decision, stable C ABI decision for future dynamic plugins, controlled-intranet worker profile, and Level 3 minimum maturity policy.
- Verified that `20261` exists and is the renamed former `2026-1` branch.
- Compared `20261` to `WorkInProgress`: at the checkpoint, `WorkInProgress` was 29 commits ahead and 0 behind.
- Inspected the latest PR CI and downloaded the GUI GMDD diagnostic artifact.
- Created `genesys_2026_phase0_ci_evidence_20260720.md` with the executed evidence and remaining validation gaps.
- Updated this handoff to replace historical CI assumptions with current evidence.

## 3. Relevant technical comments

- [Build] The canonical build remains CMake + Ninja with C++23.
- [Branches] Historical references to `2026-1` refer to the branch now named `20261`; the future second-semester stable branch is `20262`.
- [Promotion] Promotion to `20262` is planned only near the end of the second semester of 2026.
- [CI] A green `tests-unit` run proves the current registered/executed unit baseline for the validated head, but does not prove that every declared executable is included in the aggregate.
- [CI] The previously reported 16 failures out of 1657 tests are historical and were not reproduced by workflow run `29771060564`.
- [GUI] The three GMDD tests previously documented as failing all passed in the current focused diagnostic artifact.
- [Plugins] Plugins remain compiled together during consolidation; future independent dynamic plugins must use a stable C ABI with opaque handles and versioned function tables.
- [Qt] Qt6-only is decided; remaining Qt5 fallback removal is an implementation task.
- [Numerics] `SolverDefaultImpl1` still contains confirmed correctness/UB concerns that are not invalidated by a green aggregate test run.
- [Statistics] Current tests passing does not establish scientific validity without authoritative references and reference-backed vectors.
- [Optimizer] Current backend remains an internal scaffold. Supported functionality must eventually reach at least Level 3 — Beta.
- [Worker] Intended deployment is a controlled academic intranet, not direct public-Internet exposure.
- [Whole-cell] Future direction is neuro-symbolic-mechanistic AI virtual-cell research; current functionality must not overclaim predictive biological validity.

## 4. Problems encountered

### 4.1 No local execution environment

- Type: environment.
- Impact: no local `tests-kernel-unit`, `tests-smoke`, application startup, sanitizer, package, or network validation.
- Status: `needs-local-validation`.
- Mitigation: use GitHub Actions evidence where available and preserve exact run/head identifiers.

### 4.2 Partial Phase 0 coverage

- Type: build/test coverage.
- Evidence: the current workflow executes `tests-unit` plus the focused GUI GMDD job.
- Missing: `tests-kernel-unit`, `tests-smoke`, application presets, sanitizers, packages.
- Status: `partially-validated`.
- Next action: execute the two missing baseline presets before claiming Phase 0 complete.

### 4.3 Aggregate completeness remains uncertain

- Type: CMake/test aggregation.
- Evidence: `genesys_test_ai_plugins` was declared but not found in the inspected aggregate dependencies/direct runner.
- Impact: a green aggregate can omit a declared test executable.
- Status: `blocked` until explicit target/CTest inclusion is inspected or corrected.

### 4.4 Historical status drift

- Type: documentation.
- Evidence: older files describe three GUI GMDD failures and 13 non-GUI failures.
- Current evidence: the current `tests-unit` job and the three-test GUI diagnostic passed.
- Decision: treat old failure lists as historical snapshots unless reproduced on a specified current commit.
- Status: `partially-corrected`; additional historical documents may remain stale.

### 4.5 Numerical correctness remains unresolved

- Type: code/scientific correctness.
- Area: `source/tools/Continuous/SolverDefaultImpl1.*` and statistical callers.
- Evidence previously confirmed: uninitialized `_stepSize`, incomplete/incorrect RK4/Simpson behavior, overloads returning `0.0`, non-standard VLA.
- Impact: unit success does not prove untested mathematical paths correct.
- Status: `blocked` for scientific validity.

### 4.6 Plugin target overlap

- Type: build architecture.
- Evidence: component sources are recursively compiled into full and minimal static targets.
- Decision: keep current static architecture temporarily, but first produce an exact source-to-target/link map and eliminate unjustified overlap.
- Status: `in-progress` planning; implementation not started.

### 4.7 Security-sensitive implementation

- Type: security.
- Evidence: worker token generation uses a non-cryptographic PRNG; AI secret storage composes a secret-bearing shell command string.
- Deployment impact: blocks approved intranet deployment until hardened.
- Status: `needs-implementation`.

## 5. Corrections and adjustments made

No functional source, CMake, test, workflow, packaging, or historical file was changed.

Documentation changes include:

- main consolidation plan;
- test matrix;
- module inventory;
- operational handoff;
- human decisions and later addendum;
- branch policy correction for `20261`/`20262`;
- plugin stable-C-ABI direction;
- Qt6-only policy;
- controlled-intranet worker policy;
- Level 3 minimum maturity policy;
- deferred numerical/statistical reference plan;
- multiobjective Optimizer research plan;
- AI virtual-cell research direction;
- executed Phase 0 CI evidence.

Current CI-status corrections:

- ordinary `tests-unit` CI changed from unknown/needs-CI-validation to `validated` for head `a60ca65...`;
- three GUI GMDD tests changed from historical blocked status to `validated` for that head;
- full Phase 0 remains `partially-validated` because two baseline presets were not run.

## 6. Files created or changed

Principal consolidation artifacts:

- `docs/ai_assistants/genesys_2026_consolidation_plan.md`;
- `docs/ai_assistants/genesys_2026_test_matrix.md`;
- `docs/ai_assistants/genesys_2026_module_inventory.md`;
- `docs/ai_assistants/genesys_2026_consolidation_handoff.md`;
- `docs/ai_assistants/genesys_2026_phase0_ci_evidence_20260720.md`;
- `docs/ai_assistants/genesys_2026_human_decisions.md`;
- `docs/ai_assistants/genesys_2026_decisions_addendum_20260720.md`;
- `docs/ai_assistants/genesys_numerical_statistical_references_plan.md`;
- `docs/ai_assistants/genesys_multiobjective_optimizer_future_plan.md`;
- `docs/ai_assistants/genesys_ai_virtual_cell_research_direction.md`.

Stable guides updated include:

- `docs/ai_assistants/README.md`;
- `docs/ai_assistants/current_plans.md`;
- `docs/ai_assistants/branch_workflow.md`;
- `docs/ai_assistants/plugins_development.md`;
- `docs/ai_assistants/applications_development.md`;
- `docs/ai_assistants/tools_and_statistics.md`;
- `docs/ai_assistants/whole_cell_and_sbml.md`.

## 7. Validation performed

### 7.1 Current successful workflow

Workflow run `29771060564`:

- event: pull request;
- validated PR merge commit: `67d895f67307a6ba614e0c4cc5be88fbf24390ab`;
- validated branch head: `a60ca65aae4147a7d9b14bdadd9e8d39958bcaaf`;
- runner: Ubuntu 24.04.4 LTS / `ubuntu-24.04` image;
- `QT_QPA_PLATFORM=offscreen`;
- `tests-unit` configure: passed;
- `tests-unit` build: passed;
- `tests-unit` CTest: passed;
- workflow conclusion: success.

### 7.2 GUI GMDD evidence

Artifact `genesys-gui-gmdd-diagnostics`, ID `8472919224`:

- `CONFIGURE_RC=0`;
- `BUILD_RC=0`;
- `GTEST_RC=0`;
- 3 tests executed;
- 3 tests passed;
- total Google Test time reported: 542 ms.

Passing tests:

- `GuiGmddLayout.SeizeEditableReferencesStayAboveAndLowerDefinitionsUseTwoRows`;
- `GuiGmddLayout.SerializerRoundTripRestoresComponentColorAndDataDefinitionPosition`;
- `GuiGmddLayout.SerializerRoundTripRestoresViewStateGeometriesAndGroups`.

### 7.3 Not yet validated

- exact current CTest count;
- explicit inclusion of every declared unit target;
- `tests-kernel-unit`;
- `tests-smoke`;
- shell/worker/GUI standalone startup;
- all independent GUI presets;
- model-specific sweep;
- ASan/LSan/UBSan/Valgrind;
- Debian package lifecycle;
- Qt5 removal;
- worker security controls;
- numerical/statistical reference correctness;
- whole-cell scientific benchmark validity;
- stable C ABI implementation.

## 8. State of the consolidation plan

- Phase 0: `partially-validated`.
  - ordinary `tests-unit` CI: validated for one specified head;
  - focused GUI GMDD tests: validated for the same head;
  - `tests-kernel-unit`: pending;
  - `tests-smoke`: pending;
  - aggregate completeness: pending.
- Phase 1: not started as a bounded implementation phase.
- Dynamic plugin implementation: deferred; ABI direction decided as stable C.
- Qt5 removal: policy decided, implementation pending.
- Numerical/statistical references: deferred acquisition plan recorded.
- Optimizer: Level 3 target recorded; exact thesis-derived initial algorithm pending future material.
- Worker: controlled-intranet profile decided; exact authentication and hardening implementation pending.
- Whole-cell/AI virtual-cell: research direction recorded; project-specific scientific scope pending.
- `20262` promotion: future end-of-semester activity, not a current gate.

## 9. Pending work

### 9.1 Immediate validation work

1. Run `tests-kernel-unit`.
2. Run `tests-smoke`.
3. Capture `ctest -N` and exact test/target inclusion.
4. Verify `genesys_test_ai_plugins` inclusion.
5. Build representative application presets.

### 9.2 Priority technical work after baseline

- add focused failing regression tests for `SolverDefaultImpl1`;
- apply the minimum solver correction or quarantine invalid APIs;
- map and then remove unjustified full/minimal plugin source overlap;
- correct worker CSPRNG and secret handling;
- close optimizer copy/ownership hazards without falsely claiming algorithm maturity;
- verify the temporary `Model` allocation/lifetime finding;
- decide parser `FunctionRegistry` disposition.

### 9.3 Deferred research inputs

- numerical/statistical bibliography, converted PDFs, datasets, parameterizations, and expected values;
- professor's thesis chapters, pseudocode/source, algorithm variants, benchmarks, and licensing status;
- detailed AI virtual-cell project scope, modules, reference models, datasets, and validation claims.

### 9.4 Remaining human choices

Already decided:

- stable C ABI for future dynamic plugins;
- Qt6-only;
- minimum Level 3 target for supported functionality;
- controlled academic intranet worker profile;
- future end-of-semester `20262` promotion timing.

Still pending when implementation approaches:

- exact worker authentication mechanism, preferably mTLS or short-lived signed tokens over TLS;
- exact first Optimizer algorithm and benchmark package from the professor's research;
- method-specific numerical/statistical references;
- initial whole-cell scientific claim level and benchmark package;
- final `20262` promotion checklist near the end of the semester.

## 10. Recommended next actions

1. Complete Phase 0 by executing `tests-kernel-unit` and `tests-smoke` without functional changes.
2. Inspect CMake/CTest aggregation and ensure every intended test target executes.
3. Preserve the green run as a checkpoint; do not continue citing old GUI failures as current.
4. Open a separate bounded implementation branch for legacy solver regression tests.
5. Keep Qt5 removal, plugin target cleanup, worker hardening, and optimizer development in separate reviewable PRs.
6. Do not begin dynamic plugin implementation until the current static target graph is non-overlapping and lifecycle contracts are mapped.

## 11. Guidance for the next AI assistant

First read:

1. `docs/ai_assistants/README.md`;
2. `genesys_2026_decisions_addendum_20260720.md`;
3. `genesys_2026_phase0_ci_evidence_20260720.md`;
4. `genesys_2026_consolidation_plan.md`;
5. `genesys_2026_test_matrix.md`;
6. `genesys_2026_module_inventory.md`;
7. this handoff.

Use `20261`, not `2026-1`, when referring to the current semester-stable branch. Treat old names only as historical identifiers.

Do not claim Phase 0 complete until `tests-kernel-unit`, `tests-smoke`, and aggregate inclusion are verified. Do not infer scientific correctness from green unit tests. Do not modify the legacy solver before adding tests that demonstrate each defect. Do not implement dynamic plugins yet. Do not weaken the Qt6-only, Level 3, controlled-intranet, or no-overclaim scientific policies.

## 12. Limitations and uncertainties

- No local compiler/runtime was available to this assistant.
- The current workflow does not run all baseline presets.
- Exact current unit-test count was not extracted through the connector.
- A green aggregate does not prove omitted targets are executed.
- Historical PR descriptions may contain unverified claims.
- Only selected high-risk code was deeply inspected.
- Numerical, statistical, biochemical, and biological correctness still require independent references and domain review.
- GUI application startup was not executed; only the focused GUI test binary was validated.
- Worker networking and authentication were not executed.
- New documentation-only commits after validated head `a60ca65...` may trigger a newer CI run that must be checked separately.

## 13. Operational result

Result: the consolidation plan and human decisions are documented in draft PR #469; the ordinary `tests-unit` CI and all three focused GUI GMDD tests are now confirmed green for head `a60ca65aae4147a7d9b14bdadd9e8d39958bcaaf`. Phase 0 is partially validated, with `tests-kernel-unit`, `tests-smoke`, exact aggregate inclusion, application presets, sanitizers, and packaging still pending.

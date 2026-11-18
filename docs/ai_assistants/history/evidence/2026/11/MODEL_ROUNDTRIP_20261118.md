---
document_type: evidence
authority: scoped
owner: ai-assistant
date: 2026-11-18
status: validated-with-boundaries
---

# Model graphical round-trip and Resource Set runtime — 2026-11-18

Direct maintainer-authorized work on `WorkInProgress`, base
`7fc3eeccdc4bdc1cd482f4d42941efa9d3abc2a0`. Local C++23/Qt6,
CMake/Ninja, Qt offscreen. No model originals were edited.

## Confirmed causes and bounded corrections

- GraphicalModelBuilder created edges only while visiting a new destination.
  Visited now guards vertex creation; each logical edge is created independently,
  including convergence and cycles. Graphical input/output cardinalities include
  logical port ranks, preserving nonzero input channels and sparse output keys.
- Grid rendering inverted its internal flag after drawing. An idempotent setter
  now owns the state and actual lines; saving reads scene/view state rather than
  potentially stale actions. Saved component colors take precedence over category
  colors. Flow connections refresh after final geometry and port restoration.
- Seize loaded a string with an unsigned numeric default, producing the legacy
  one-byte NUL signature. A string default fixes new loads; that exact legacy
  signature is normalized with a warning. Mixed NUL content is preserved and
  rejected by checking, not silently repaired.
- Seize/Release duplicated unsafe selection: uninitialized/shadowed bestIndex,
  positive-minimum sentinel, request-count rather than Set-count cyclic/random
  bounds, and unchecked expression-to-unsigned conversion. A shared request-boundary
  helper validates candidates, expressions, quantities and indices. Ties and zero
  remaining capacity select a valid candidate; invalid/empty Sets fail explicitly.
  Cyclical state advances after successful operations. Parser grammar is unchanged.
- Set discarded members instantiated later in file/ID order. Pending names now
  resolve in persisted order after definitions exist; incomplete references remain
  savable and fail checking. Members retain their existing non-owning lifetime.
- Release read allocation-category attributes not necessarily declared by Delay.
  It now declares the category attributes it can consume. GUI/terminal error
  handlers retain exception detail, including component/property/expression.

## Reference model and configuration boundary

Original `/home/rafaelcancian/Repositories/SistemaComputacional05.gui` SHA-256:
`52bc91ad766a578c9eec7d703a73a09fd0717d590b4d9419e1ef0645401d6fac`.
The 04 original also remains unchanged:
`3b767fa57b23d0a41a070234e1ce92a466997db728c43cbf0056ca502895210e`.

The unusual Set name `Processador4.TotalTimeFailed` already exists in both input
files. No renaming was performed; the historical intent is UNKNOWN. Both Assign
branches and their logical/graphical ports survive two consecutive save/load cycles.

Executing the original 05 configuration for its 30 replications produces 90 caught
over-release errors and zero syntax errors after legacy priority normalization.
Its Release policy chooses largest remaining capacity rather than the member
saved by Seize. This configuration is not silently changed by loading.

An explicitly modified **validation copy** uses Release SPECIFICMEMBER with
SaveAttribute `meuProcessador`. Its service-time assignments use `MAX(0, original)`
as an explicit nonnegative-time modeling assumption. Across 30 replications:
10,763 Release events, zero runtime errors and zero backwards clock transitions.
The copy is under ignored `build/audits/roundtrip-20261118/`; it is not the original.
No claim of statistical/scientific correctness is inferred from those counts.

## Validation and acceptance

| Criterion | Status | Evidence |
|---|---|---|
| Logical edges have matching graphical endpoints/ports | PASS | Multiset equality of actual GraphicalConnection endpoints and every logical edge in synthetic and supplied models |
| Assign_2 and Assign_3 to Seize survive reopening | PASS | Supplied-model two-cycle round-trip compares complete graph |
| Convergence, cycles, multiple Sources and dynamic ports | PASS | Builder regression with three incoming paths, input ranks 0/1/2, Decide branches and cycle |
| No duplicate flow/association connections | PASS | Edge multiset equality and existing direct-association tests |
| Grid/Rule/Snap/Guides and element filters | PASS | Two cycles with opposite values; actual grid line visibility checked, including stale action |
| Zoom, viewport, positions, ports and colors | PASS | Transform/scroll/geometry comparisons; deferred restore isolated from a second model view |
| Group, overlay and animation compatibility | PASS | Existing graphical serialization regression suite |
| Seize empty/nonempty expressions and legacy NUL | PASS | Two logical round-trips; mixed NUL fails checker |
| Release index bounds/ties/zero/empty/invalid members | PASS | Focused helper/runtime tests, random bounds, cyclic progression, caught over-release |
| Observed syntax errors traced and corrected | PASS | Wrong priority default confirmed; supplied-model run reports zero syntax errors; contextual diagnostics regression |
| Logical identities, Set order and names | PASS | Two cycles, late Resource/Queue references, complete endpoint/name/label/class snapshots |
| Original model executes without errors | FAIL | 90 over-release errors: Release configuration does not retain seized identity |
| Explicitly corrected validation copy executes | PASS | 30 replications, 10,763 releases, zero errors/backwards clock transitions |
| Screenshot comparison and historical naming intent | UNKNOWN | Requested images unavailable locally; earlier naming/edit history unavailable |

Executed commands/results:

- `cmake --build --preset tests-unit --parallel 4`; CTest with
  `GENESYS_REFERENCE_GUI` set to the real 05 path and
  `GENESYS_VALIDATED_GUI_COPY` set to the ignored copy path.
  Final full suite: **2,167/2,167 executed PASS**, four disabled (230.78 seconds).
- Final focused Qt run (`GraphicalRoundTrip.*` plus convergence/grid builder
  regressions): 7/7 PASS (34.44 seconds), including the real 05 model and console
  exception detail. Earlier complete freeze: 2,166 executed PASS, four disabled.
- `cmake --build --preset gui-app --parallel 4`: PASS. Bounded offscreen app
  startup with isolated `XDG_CONFIG_HOME`: no crash before expected timeout.
- `tests-kernel-unit`: focused Seize/Release/Set persistence and selection tests.
  7/7 PASS (7.40 seconds).
- `tests-smoke`: 3/3 PASS.
- ASan+UBSan focused C++23 selection harness: PASS, including 1,000 random
  selections, all-busy ties, invalid indices and incompatible/empty Sets.
  Only the inline helper/harness was instrumented; linked kernel archives were
  not instrumented. LSan was disabled. This is not a whole-application sanitizer run.
- Updated manual sources compiled successfully and `docs/ManualGenESyS.pdf`
  regenerated. Documentation validator and `git diff --check` executed at closeout.
  Global documentation validation remains FAIL due to five pre-existing evidence
  files lacking YAML front matter (DIRECT_GUI_ASSOCIATIONS, GUI_CAPTIONS_SELECTION,
  GUI_LABEL_WRAPPING, INLINE_EXPRESSIONS, RELEASE_ABORT). This task's new evidence
  includes required metadata; those unrelated historical documents were preserved.

Remaining boundaries: no interactive screenshot comparison; no certification of
Resource utilization/time-accounting statistics; no general change to distributions,
parser grammar, model format, or allocation identity policies. Existing runtime
rules remain explicit configuration choices. Results apply to tested supported
geometry/state, not an unimplemented arbitrary component resize contract.

Runtime/persistence commit: `44859947f397f13f0b04377374234112108f58a3`.
The graphical/test/documentation commit is the commit containing this evidence;
its hash and verified upstream synchronization are reported at delivery.

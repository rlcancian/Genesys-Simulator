---
document_type: reference
authority: technical-plan
owner: project-maintainer
last_reviewed: 2026-10-08
review_cadence: on-phase-completion
status: active
---

# Bacteria Colony / Gro Integration Plan

## 1. Objective and scope

Bring the existing GenESyS integration of the `gro` bacterial micro-colony
language (University of Washington, SOS Lab) to a technically consistent,
modular, testable and persistable state.

This is a **continuation**, not a from-scratch implementation. A substantial
integration already exists under
`source/plugins/data/BiochemicalSimulation/Gro*` and
`source/plugins/components/BiochemicalSimulation/BacteriaColony.*`, with a
GUI viewer and 46+ focused unit tests.

**Scope redefinition (2026-10-08, maintainer instruction — see §2):** the
goal is **not** full compatibility with the original Gro language, its
underlying CCL runtime, every original builtin, or all 23 original example
programs. The goal is a **GenESyS-native implementation of a coherent,
useful, scientifically defensible and well-tested subset** of the
functionality needed to model and simulate bacterial colonies inside
GenESyS. The original `~/Repositories/bacteria_programming_language`
checkout remains an essential *behavioral reference* (syntax, semantics,
contracts, algorithms, oracles) but is explicitly **not a specification
GenESyS must reproduce in full**.

Working branch: `WiP20261008/BacteriaColony`, based on `WorkInProgress` at
`41419bcc75e232387d307053369e5e495a194e3a`.

## 2. Maintainer scope decisions

Recorded 2026-10-08. These are closed decisions; a future assistant must
not reopen, second-guess, or silently revert them without new, explicit
human instruction.

1. Full Gro compatibility is **not** a goal.
2. The 23 original example programs are a compatibility/reference corpus,
   not a 100% acceptance gate.
3. Only the Gro language subset actually needed by the GenESyS bacteria
   -colony capabilities selected for this integration should be
   implemented (see §6, §9).
4. **Chipmunk2D will not be incorporated.** This decision is closed; do not
   re-present it as an option to decide later.
5. **No other external physics engine** (Box2D, Bullet, or equivalent)
   will be incorporated either.
6. GenESyS will use a **simpler, GenESyS-native internal 2D bacterial
   mechanics model** (see §12).
7. Rigid-body fidelity (forces, torques, true rigid-body collision) is
   **not required**.
8. Unsupported high-complexity Gro language constructs (e.g. `fun`,
   `let/in/end`, lambdas, general list/record values, `foreach`/`cross`,
   the `maptocells ... end` keyword form) may remain **explicitly
   unsupported** when no selected GenESyS capability depends on them.
9. The completion gate is based on **selected GenESyS capabilities** (§9,
   §21), not percentage compatibility with the original Gro.

There is consequently **no open stop gate about adopting an external
physics engine**: that question is closed by decision 4/5 above, not
pending.

## 3. Original Gro reference baseline

Reference checkout: `~/Repositories/bacteria_programming_language`
(`rlcancian/bacteria_programming_language`, local HEAD `d3ef577`, branch
`master` tracking `origin/master`). Not modified by this work. Retained in
full as the behavioral reference described in §1/§2 — not a target to
replicate wholesale.

Confirmed by direct inspection of `Gro.cpp`, `Programs.h/.cpp`, `World.h/.cpp`,
`Signal.cpp`, `Micro.h`, `Cell.h/.cpp`, `EColi.h/.cpp`, `reaction.cpp`,
`include/gro.gro`, `include/standard.gro`, and all 23 files under
`examples/`:

- **Builtins registered by `register_gro_functions()`** (`Programs.cpp`):
  `set`, `zoom`, `ecoli` (`yeast` present but commented out/disabled),
  `signal`, `set_signal`, `set_signal_rect`, `get_signal`, `emit_signal`,
  `absorb_signal`, `reaction`, `get_signal_matrix`, `reset`, `stop`, `start`,
  `stats`, `snapshot`, `message`, `clear_messages`, `set_theme`, `dump`,
  `time`, `die`, `divide` (C++ name `force_divide`), `geometry`, `run`,
  `tumble`, `map_to_cells`, `print`, `clear`, `chemostat`, `barrier`,
  `fopen`, `fprint`.
- **Signal channels are real, independent, index-addressed objects.**
  `new_signal()` constructs a `Signal` and pushes it onto
  `World::signal_list`, returning `world->num_signals() - 1` (the index).
  Every other signal builtin (`get_signal`, `emit_signal`, `absorb_signal`,
  `set_signal`, `set_signal_rect`, `get_signal_matrix`, `reaction`) takes
  that integer handle and indexes `signal_list[n]` directly
  (`World::get_signal_value/emit_signal/absorb_signal`, `Programs.cpp`).
- **Diffusion/degradation** (`Signal::integrate(float dt)`, `Signal.cpp`):
  8-neighbor stencil, interior cells only (`i` in `[1, numx-2]`, `j` in
  `[1, numy-2]`; the one-cell boundary ring is never updated, i.e. frozen
  boundary):
  ```
  dsig[i][j] = -6*kdiff*sig[i][j] - kdeg*sig[i][j]
             + kdiff * ( 0.5*sig[i+1][j-1] + sig[i+1][j] + 0.5*sig[i+1][j+1]
                       +     sig[i][j-1]                 +     sig[i][j+1]
                       + 0.5*sig[i-1][j-1] + sig[i-1][j] + 0.5*sig[i-1][j+1] )
  sig[i][j] += dt * dsig[i][j];      // then clamped to >= 0
  ```
  This is an explicit finite-difference reaction-diffusion stencil, **not**
  a Finite Element Method. `inc(x,y,c)` (emission) requires strictly
  interior cells (`i>1 && i<numx-1 && j>1 && j<numy-1`); `dec(x,y,c)`
  (absorption) allows the full range including the boundary ring — this
  asymmetry is original, not a porting artifact.
- **Reactions** (`reaction.cpp`): mass-action per grid cell,
  `v = rate * product(reactant concentrations at (i,j))`; all reactants
  decremented by `v*dt`, all products incremented by `v*dt`; applied once
  per reaction per grid cell, over the *entire* grid (not just interior
  cells), *before* diffusion in `World::update()`. Not required by the
  selected GenESyS capability subset (§6/§9); retained here only as
  reference in case a future model needs it.
- **`World::update()` pipeline order** (confirmed exactly):
  `prog->world_update()` → per-cell `cell->update()` (growth, Gro program
  step) then `cell->divide()` (adds daughter if produced) → reactions over
  the whole signal grid → diffusion/degradation (`signal->integrate(dt)`)
  for every signal → death removal (`marked_for_death`) → chemostat flow
  force + out-of-bounds removal → 3× `cpSpaceStep` (Chipmunk physics,
  **not** reproduced — see §2 decision 4/5) → `t += dt`. The whole
  `update()` is skipped (one-shot warning, `stop_flag=true`) if
  `population->size() >= population_max`.
- **EColi growth/division** (`EColi.cpp`):
  ```
  lambda  = sqrt(10 * growth_rate^2 / division_size_variance)
  div_vol = division_size_mean - growth_rate*10/lambda
  // per step:
  volume += growth_rate * rand_exponential(mean=dt) * volume;   // growth_rate=0 => exactly zero growth
  if (volume > div_vol && frand() < lambda*dt) div_count++;
  // divide() when div_count>=10 or force_div:
  frac = 0.5 + 0.1*(frand()-0.5);         // ~U(0.45,0.55)
  parent.volume = frac*oldvol; daughter.volume = (1-frac)*oldvol;
  // position/orientation split along current heading with random jitter da
  // gro_program split via split_gro_program(parent, frac) -> SymbolTable::divide(frac)
  // q[]/rep[] split proportionally (ceil for parent, floor for daughter)
  // both parent and daughter: set_division_indicator(true); daughter also set_daughter_indicator(true)
  ```
  `just_divided`/`daughter` are read by `gro_Program::update()` at the start
  of the *next* step after division and cleared (`set_division_indicator
  (false)`) in that same read, i.e. visible for exactly one step. This
  behavioral *contract* (growth-rate-zero invariant, division fraction
  randomization, one-step division-flag visibility) is adopted as the
  GenESyS reference oracle; the Chipmunk-coupled geometry/physics details
  are not.
- **Movement** (`run`/`tumble`, `Programs.cpp`): both operate directly on
  the Chipmunk `cpBody` (`cpBodyApplyForce`/`cpBodySetTorque`). GenESyS does
  **not** reproduce this — see §12 for the GenESyS-native kinematic
  contract. Only the behavioral intent (`run` propels along current
  heading, `tumble` reorients) is carried over.
- **Barriers/chemostat**: `barrier(...)` registers a real Chipmunk static
  segment shape *and* records it for rendering; `chemostat(bool)` toggles
  `chemostat_mode`, which both adds Chipmunk wall segments and applies a
  one-directional flow force plus out-of-bounds removal. GenESyS does not
  reproduce the Chipmunk mechanics; see §12 for the simplified, deferred
  treatment.
- **Known original-Gro defect (not to be replicated):** `include/standard.gro`
  sets the default parameter `"ecoli_division_size_var"`, but
  `EColi::compute_parameter_derivatives()` reads
  `"ecoli_division_size_variance"`. The default is therefore never applied
  unless a user program sets the correctly-spelled key explicitly. This is
  a genuine upstream bug, documented here for provenance; GenESyS must use
  one consistent, correctly-spelled parameter name and is not obligated to
  reproduce the typo.
- **Yeast** is present in source (`Yeast.h/.cpp`) but `new_yeast`/`register
  _ccl_function("yeast", ...)` are commented out in `Programs.cpp`. The
  `examples/yeast_example.gro` file therefore does not run against the
  original `gro` binary either. Yeast is out of scope here.
- Full example corpus (`examples/*.gro`, 23 files) was read in full; see
  §8 for the per-example compatibility matrix, now including a
  `TargetForGenesys` column per §2 decision 2.

## 4. Licensing/provenance boundary

`~/Repositories/bacteria_programming_language/LICENSE.md`: UW Open Source
License (noncommercial, requires attribution, requires derivative works to
be clearly marked and distributed under a license prohibiting commercial
use, requires the license text to accompany copies).

Rule applied in this work: **no direct code copying** from the reference
repository into GenESyS. Semantics are treated as a *behavioral contract*
to reimplement independently in GenESyS's own C++ style and test
infrastructure, not as source to port verbatim. No `.gro` example file is
copied into the GenESyS tree as a fixture; GenESyS-authored fixtures use
equivalent syntax written from scratch for the specific construct under
test (§9). If any future step appears to require incorporating UW source
verbatim, this plan requires stopping and presenting the maintainer with
the exact code, the reason, a copy-free alternative, and the relevant
license obligations (governance §18, stop gate 1) before proceeding.

Note: an untracked directory `models/gro_examples/` containing ~19 of the
23 original `.gro` files (pre-existing in the working tree, not created by
this work, not committed) was found during Phase 0. It is explicitly not
part of this plan's fixtures and must not be treated as a GenESyS-authored
corpus; its provenance/disposition is a separate question for the
maintainer, not resolved here.

## 5. Current GenESyS implementation inventory

Confirmed by full-file inspection (2026-10-08, read-only, no production
changes at the time of inspection):

| File | Lines | Role |
|---|---|---|
| `plugins/data/BiochemicalSimulation/GroProgram.h/.cpp` | 421/228 | `ModelDataDefinition` storing Gro source text; permissive lexical check |
| `plugins/data/BiochemicalSimulation/GroProgramAst.h` | 54 | Untyped AST: `SourceForm{RawStatements,ProgramBlock}`, `NamedProgram{name,parameters,bodySource,statements}`, `Statement{sourceText}` |
| `plugins/data/BiochemicalSimulation/GroProgramParser.h/.cpp` | 35/611 | Structural/delimiter-balancing tokenizer; no real grammar |
| `plugins/data/BiochemicalSimulation/GroProgramCompiler.h/.cpp` | 23/937 | Compiles statements to a 4-kind IR; composition via static inlining |
| `plugins/data/BiochemicalSimulation/GroProgramIr.h` | 76 | `Command::Kind = {RawStatement, FunctionCall, Assignment, IfStatement}` |
| `plugins/data/BiochemicalSimulation/GroProgramRuntime.h/.cpp` | 131/1079 | Executes IR against `GroProgramRuntimeState`; builtin dispatch |
| `plugins/data/BiochemicalSimulation/BacteriaSignalGrid.h/.cpp` | 82/266 | `ModelDataDefinition`: one scalar signal field (width/height/initialSignal/diffusionRate/decayRate/initialValues); no multi-channel support |
| `plugins/components/BiochemicalSimulation/BacteriaColony.h/.cpp` | 301/2379 | `ModelComponent`; owns population, one optional `BacteriaSignalGrid*`, one optional `BioNetwork*`, colony/bacterium-scoped Gro execution, growth/motion/division/barrier/chemostat bookkeeping |
| `applications/gui/genesys/extensions/BacteriaColonyViewerGuiExtensionPlugin.cpp` | 1174 | Qt viewer: heatmap, bacteria, trails, selection, Step/Start/Stop |
| `source/tests/unit/test_runtime_pluginmanager.cpp` | — | 46 focused `TEST()` cases covering parser/compiler/runtime/colony behavior |
| `source/tests/unit/test_simulator_runtime.cpp` | — | 1 test confirming `GroProgram::SourceCode` uses a code-editor property hint |
| `models/Smart_GroColonyGrowth.gen`, `Smart_GroColonyLifecycle.gen`, `Smart_BacteriaColony_GRO.gen` | — | Persisted `.gen` fixtures; the third uses the richer `program main()/colony()` + `BacteriaSignalGrid` shape with single-argument `emit_signal` |

Registration: `groprogram.so`, `bacteriasignalgrid.so`, `bacteriacolony.so`
statically registered in `PluginConnectorDummyImpl1.cpp`.

Dependency note: `WiP2026108/PE_Fix` (PR #545) touched `GenSerializer.cpp`,
`ObjectPropertyBrowser.cpp` and `ModelLanguageSynchronizer.cpp`, including a
test explicitly named "cover real GroProgram model round trip". **PR #545
was merged into `WorkInProgress` (merge `dce34847`) and integrated into
this branch on 2026-10-08** (merge `c7816bf2`); it is no longer an open
dependency. The persistence phase (§17 item 12) should rebase on the
current serializer as shipped, not re-fix the same path.

## 6. Language subset strategy

Per §2 decision 3, every missing or partial construct is evaluated against
four questions before any implementation effort: (1) which concrete
GenESyS bacteria-modeling capability needs it; (2) is there a simpler way
to express that capability; (3) does a *selected* fixture (§9) actually
depend on it; (4) would implementing it require turning the current
frontend into a substantial CCL reimplementation. A construct that fails
this test is recorded `deferred` or `intentionally-unsupported`, not
implemented.

Current status (`supported`/`partial`/`missing`/`broken`), evidence, and
the subset decision:

| Construct | Status | Evidence | Needed for selected subset? |
|---|---|---|---|
| `//` and `/* */` comments | supported | `GroProgramParser::skipWhitespaceAndComments` | yes (already works) |
| String literals (balance/skip only) | supported | string-aware scanning | yes (already works) |
| Balanced `()`, `[]`, `{}` | supported | `GroProgramParser::findMatchingDelimiter` | yes (already works) |
| `program name(params) := { ... };` | supported | `tryParseNamedProgram` | yes (already works) |
| `condition : { actions }` rule | supported (`IfStatement`, no `else`) | `tryCompileRuleStatement` | yes (already works) |
| `:=` assignment | supported | `findTopLevelAssignment` | yes (already works) |
| `if (cond) { } else { }` (C-style) | supported | `tryCompileIfStatement` | yes (already works) |
| Unsupported syntax silently becomes `RawStatement` | **fixed 2026-10-08** (`a2bd10b6`) | `GroProgramRuntime` already collected `unsupportedCommands`/`skippedRawStatements` correctly; the gap was that `BacteriaColony::_onDispatchEvent` and the viewer discarded them when reporting success — both now surface non-empty counts | done |
| `needs a, b;` (comma form) | **fixed 2026-10-08** (`daffe5bf`) | `GroProgramCompiler::tryConsumeNeedsStatement` now consumes the whole clause to its `;` instead of mis-splitting on the comma | done |
| Record field **read** inside an expression/condition (`p.mode = GO`) | **already supported — confirmed by executed evidence, not a gap** | `NumericExpressionParser::parseIdentifier` already accepts dotted identifiers and `resolveIdentifierValue` resolves them against the same flat `state.variables` map the flattened assignment writes to; the pre-existing test `GroProgramRuntimeSupportsGroRuleSyntaxAndPersistentRecordFields` already exercises `p.mode = 0 & get_signal(ahl) > 0.01 : {...}` and passes | done — Phase 0's "unknown" classification for this item was incorrect |
| `if EXPR then EXPR else EXPR end` (Gro expression form) | missing | no `then`/`end` handling | no — the existing C-style `if/else` statement already covers the needed conditional-branching capability |
| Records `[ field := value, ... ]` as a declaration | partial (flattened to `x.field := value`) | `compileSimpleStatement` | yes (already partially works; pair with the read-access fix above) |
| Lists `{ a, b, c }` as a value | missing | raw text only | no — no selected capability needs list-valued state |
| Indexing `a[i]` | missing | no handling | no — not needed once lists are out of scope |
| `sharing a, b, c` | no real scoping (flat global map) | `stripSharingClause` | no — acceptable as a documented simplification; real per-program scoping is not needed by the selected subset |
| `fun name args . body;` / higher-order functions / lambdas | missing | zero occurrences | no — every selected fixture can express its logic with plain rules/expressions |
| `foreach`/`range`/`cross`/`let/in/end`/`map` | missing | zero occurrences | no — bulk seeding and statistics aggregation are not required; the selected corpus seeds a handful of bacteria with explicit `ecoli(...)` calls, matching the existing `Smart_BacteriaColony_GRO.gen` fixture style |
| `maptocells EXPR end` (keyword form) | missing (only `map_to_cells(expr)` call form exists) | — | no — the existing call-form is sufficient if ever needed |
| `@` (cons), `#` (concat) | missing | — | no — tied to the deferred list feature |
| `rate(k)` | supported (ordinary call) | `parseFunctionCall` | yes (already works) |
| `&`, `|`, `!` | supported | `NumericExpressionParser` | yes (already works) |
| `<>` string concatenation | unknown | not yet probed | maybe — cheap to confirm/add if genuinely missing; useful for readable viewer/message output, not required for simulation correctness |
| `<<` record-override operator | unknown, GUI-theme-only in the examples that use it | not yet probed | no — only exercised by GUI theme records, out of scope |
| Program composition `A() + B() sharing ...` | supported via static inlining | compiler composition path | no further work needed; existing flat-scope behavior is accepted (see `sharing` row) |

## 7. Builtin compatibility matrix

Status values: `equivalent`, `partial`, `different-semantics`, `missing`,
`intentionally-unsupported`, `gui-only`, `unsafe/deferred`, `extension`
(GenESyS-only). A `Needed?` column records the §6 subset decision.

| Gro builtin | GenESyS status | Current GenESyS behavior (confirmed in code) | Needed for selected subset? |
|---|---|---|---|
| `signal(kdiff,kdeg)` | **fixed 2026-10-08** (`58783cba`/`5c1df906`, §17 phase 3) | `VAR := signal(kdiff,kdeg)` assigns a real, stable ordinal channel handle; handle 1 keeps the legacy field, handle N>=2 gets its own independent field | **yes — core of capability B (§9)**, done |
| `get_signal(n)` | **fixed 2026-10-08** (phase 3) | Resolves `n` to its own channel (legacy for 0/1, independent field for N>=2); an invalid `n` is a diagnosed error, not a silent fallback | **yes**, done |
| `emit_signal`/`absorb_signal` | **fixed 2026-10-08** (phase 3) | The leading handle argument (2-arg form) is validated and routed to its own channel; invalid handles are diagnosed | **yes**, done |
| `set_signal`/`set_signal_rect` | **fixed 2026-10-08** (phase 3) | The leading `n` argument is validated and routed to its own channel at the colony-mutation level | done |
| `reaction(reactants,products,rate)` | missing | Zero occurrences | **no — not required by any selected fixture (§9)**; retained as documented reference only |
| `get_signal_matrix(n)` | different-semantics | Requires zero arguments today (signature mismatch) | no — not required by the selected subset; defer |
| `ecoli([...], program p())` | partial | Structurally equivalent; see §12 for the coordinate gap | yes (already mostly works) |
| `die()` / `die(n)` | extension + different-semantics | Optional `amount` argument (extension) | yes (already works; document deviation, §19) |
| `divide()` | different-semantics (aggregate mode) | Doubles the whole population outside bacterium-scoped mode | yes in bacterium-scoped mode (already works); aggregate-mode semantics documented as a deviation (§19) |
| `run(v)` / `tumble(v)` | different-semantics | Synthetic scalar formulas, no physics | **yes — redefine per §12, not reimplement physics** |
| `geometry()` | missing | Not in dispatch table | no — bacterium-scoped context already exposes position/direction as plain variables |
| `time()` | **fixed 2026-10-08** (`24cd00f9`) | Now returns `state.colonyTime` from the expression evaluator; previously aborted the whole statement/program if used in an expression | done |
| `stats(...)` / `stop()` / `start()` / `print(...)` / `clear()` | missing | Not in dispatch table | no — GUI/control conveniences, not required |
| `message(n, text)` | partial | Quadrant discarded | no further work required |
| `clear_messages(n)` | missing | Not in dispatch table | no |
| `set(name, value)` | equivalent (structurally) | Special-cased `dt`/signal-grid keys; rest generic | yes (already works) |
| `barrier(x1,y1,x2,y2)` | partial, inert | Stored, not applied to motion | no — deferred; corpus C's "domain limits" means boundary reflection, already implemented, not barrier segments |
| `chemostat(bool)` | partial, inert | Stored, not applied to motion | no — deferred (§12/§20) |
| `reset()` | partial | Clears variables/population/tick count | yes (already works) |
| `zoom(...)` / `set_theme(...)` / `snapshot(...)` | gui-only, missing | Not in dispatch table | no |
| `fopen` / `fprint` / `dump` | unsafe/deferred | Correctly absent | no — security decision required first regardless (governance §15) |
| `map_to_cells(expr)` | partial | Call-form only | no further work required |
| `tick()`, `grow(n)`, `set_population(n)`, `dump_signal_field(r,c)` | extension | GenESyS-only | yes (already works; document as extensions, §19) |

## 8. Original example compatibility matrix

All 23 files under `~/Repositories/bacteria_programming_language/examples/`
were read in full and cross-checked against §6/§7. Per §2 decision 2, this
matrix is a **coverage map, not a 100% acceptance gate**. `TargetForGenesys`
records whether the example's *concept* informs the selected subset (§9),
independent of whether the literal file would parse today.

| Example | Status (unchanged from Phase 0) | TargetForGenesys | Reason |
|---|---|---|---|
| `growth.gro` | untested | partial | growth/rate/`divide()` concepts align with corpus A; not selected verbatim |
| `signal_demo.gro` | unsupported (`foreach`/`range`) | no | bulk procedural signal creation via loops not required |
| `signal_grid.gro` | unsupported (`foreach`/`cross`/lists/indexing) | no | bitmap-to-signal loading is not a selected capability |
| `morphogenesis.gro` | unsupported (`fun`, expression `if/then/else/end`, curried calls, `needs` bug) | no | full composable state-machine-via-`fun` exceeds the subset; corpus D covers a simpler record-based internal state instead |
| `chemotaxis.gro` | unsupported (top-level `foreach`) | partial | `run`/`tumble`/`get_signal` align with corpus C; bulk 50-cell seeding via loop not required |
| `barriers.gro` | untested (compiles, inert barriers) | no | barriers deferred (§7); corpus C's domain limits are boundary reflection, already present |
| `foreach.gro` | unsupported | no | pure loop-seeding demo, not a modeling capability |
| `maptocells.gro` | unsupported (`fun`/`let`/lambda/keyword form) | no | statistics-aggregation DSL not required |
| `bandpass.gro` | unsupported (`fun`) | no | band-pass filter function not required |
| `coupled_oscillator.gro` | partial/untested (record-read blocker resolved 2026-10-08, §6) | partial | oscillator-with-internal-state concept aligns with corpus D |
| `dilution.gro` | unsupported (`needs`-as-scoping still out of subset; comma-splitting bug fixed 2026-10-08) | partial | rate-based production/dilution aligns loosely with corpus D; not selected verbatim |
| `edge.gro` | partial/untested (record-read blocker resolved 2026-10-08, §6) | partial | 2-arg `emit_signal` still aliases the single field until §10/phase 3 lands |
| `game.gro` | unsupported (`stats`/`stop`) | no | interactive game demo, not a modeling capability |
| `geometry.gro` | unsupported (`fopen`/`geometry`/`time`/lists) | no | file I/O and list state not required |
| `gfp.gro` | unsupported (`needs`-as-scoping still out of subset; comma-splitting bug fixed 2026-10-08) | partial | GFP production/degradation rate pattern aligns with corpus D |
| `inducer.gro` | unsupported (`clear_messages`) | partial | rate()-driven production pattern aligns with corpus D |
| `signal_dump.gro` | unsupported (`reaction`, `<<`, `foreach`, `fopen`) | no | reaction-diffusion + file dump not required |
| `skin.gro` | unsupported (`<<`, expression `if/then/else/end`, `start`) | no | pattern-formation state machine exceeds the subset |
| `spatial_oscillations.gro` | partial/untested (record-read blocker resolved 2026-10-08, §6) | **yes — primary acceptance-corpus inspiration** | record-based internal state, `just_divided`/`daughter`, `emit_signal`, `die()` — matches corpus D almost directly |
| `spots.gro` | unsupported (list literal for `nutrient`) | partial | two-cell signal-exchange concept aligns with corpus B, but as two explicit `signal()` declarations, not a list |
| `symbiosis.gro` | partial/untested (record-read blocker resolved 2026-10-08, §6) | **yes — primary acceptance-corpus inspiration** | two signal channels, record-based state, leader/follower pattern — matches corpus B+D directly |
| `wave.gro` | unsupported (`reaction`, `<<`, `foreach`, lists) | no | reaction-diffusion pattern formation not required |
| `yeast_example.gro` | not-applicable | no | `yeast()` disabled upstream too; out of scope |

`spatial_oscillations.gro` and `symbiosis.gro` are the strongest conceptual
templates for the GenESyS-authored acceptance fixtures in §9 — written from
scratch, not copied, per §4.

## 9. GenESyS acceptance corpus

This is the real completion target (§2 decision 9), replacing "percentage
of 23 original examples passing." Each capability below must have an
executed, passing, GenESyS-authored fixture/test before the integration can
be called complete for that capability.

### A. Growth and division

- bacterium created (`ecoli`-equivalent seeding);
- parameterized growth;
- **`growthRate = 0` ⇒ no volume growth** (closes the confirmed §11 defect);
- division with parent/daughter volume split;
- `justDivided`/`daughter` state visible for exactly one step post-division.
- Existing coverage: most of this is already exercised by
  `BacteriaColonyAppliesVisibleGrowthToBacteria`,
  `BacteriumScopedGroPreservesDivisionFlagsAndAbsorbSignalAlias`,
  `BacteriaColonySupportsBacteriumScopedDivide`. **Gap: no test for
  `growthRate=0`.**

### B. Signals

- creation of a signal channel with a real, stable handle;
- **at least two independent channels that do not alias** (closes the
  confirmed §6/§14 defect);
- emission, absorption, read, all respecting the handle;
- diffusion; degradation.
- Existing coverage: single-field diffusion/decay and command dispatch are
  tested; **done 2026-10-08 (§17 phase 3)** — two (or more) independent
  channels, non-aliasing emission/absorption/read/`set_signal`/
  `set_signal_rect`, invalid-handle rejection, no cross-replication leak,
  and dimension-resize coherence are now covered by the tests listed in
  §17 phase 3. Diffusion/degradation per additional channel reuses the
  existing formula (§10); a numerical/formulation review remains Phase 4.

### C. Spatial interaction

- continuous position;
- movement via the GenESyS-native kinematic model (§12);
- `run`/`tumble` redefined per §12 (no Chipmunk);
- domain-limit behavior (boundary reflection, already implemented);
- **`speed = 0` ⇒ no displacement** and **`dt`/`simulationStep = 0` ⇒ no
  displacement** (closes the confirmed §12 defects).
- Existing coverage: `BacteriaColonyProducesVisibleMotionSignalsAndFluorescence`,
  `BacteriaColonyAppliesRunAndTumbleToBacteriumScopedState`. **Gap: no test
  for `speed=0` or zero-step invariants.**

### D. Program-colony integration

- a selected Gro-subset program controls one bacterium;
- internal record-like state (`p.field`) read and written, including
  inside conditions;
- reacts to local signal;
- drives growth/movement from program logic.
- Existing coverage: `BacteriaColonyExecutesSignalAwareBacteriumProgram`
  covers the signal-reaction half; `GroProgramRuntimeSupportsGroRuleSyntaxAndPersistentRecordFields`
  already confirms record-field read-in-condition at the runtime level
  (**verified 2026-10-08, executed evidence, not just static reading**).
  Remaining gap: no end-to-end `BacteriaColony`-level fixture yet combining
  record-based state + signal reaction + growth/movement in one program,
  authored specifically for this corpus (phase 10, §17).

### E. Viewer fidelity

- population, position, orientation rendered faithfully;
- signal field rendered faithfully;
- division and death reflected;
- selection preserved across refresh;
- no viewer-invented physics or hidden second simulation model.
- Existing coverage: extensive GUI implementation exists (§13); **no
  automated test exists for viewer-state-adapter fidelity or for the
  concurrent manual-step/event-calendar interaction risk noted in §14.**

## 10. Signal / reaction-diffusion contract

Current: one unnamed scalar field per colony (`BacteriaSignalGrid`), 4
-neighbor (von Neumann) relaxation, no explicit `dt`
(`BacteriaColony.cpp:1183-1228`):
```
relaxed = current + diffusionRate * (neighborAverage - current)
updated = max(0, relaxed * (1 - decayRate))
```
Boundary cells are updated with a smaller neighbor count (not frozen, not
reflected, not periodic). This describes the legacy field that handle 0/1
still use unchanged.

**Multi-channel addressing: done 2026-10-08 (§17 phase 3).** `signal(
kdiff,kdeg)` creates/resolves a real, independently addressable channel
and returns a stable handle; `get_signal`/`emit_signal`/`absorb_signal`/
`set_signal`/`set_signal_rect` operate on the channel named by that handle,
with explicit, tested rejection of invalid handles (see §17 phase 3 for the
full contract). Each additional channel (handle >= 2) runs the exact same
4-neighbor/no-`dt` relaxation formula above, applied independently per
channel with that channel's own `diffusionRate`/`decayRate` — the
discretization itself was **not** revised by phase 3 (that is Phase 4's
scope, §17 item 4); only the structure needed for each channel to own its
own configuration and field was added. No change was made to the existing
field formulation, so no `.gen`-fixture migration note is required by this
phase (§15/§20).
`reaction()` is explicitly **not** part of the selected subset (§7) and is
retained here only as reference.

## 11. Bacterial growth/division contract

Confirmed defect (`BacteriaColony.cpp:2144-2170`,
`_applyBacteriumGrowth()`): `deltaVolume = clamp(growthRate * stepScale,
0.01, 0.35)`. With `growthRate` driven to exactly `0.0` (explicit
`ecoli_growth_rate=0`, generation 0, no local signal contribution), the
minimum clamp of `0.01` still applies, producing deterministic non-zero
growth. **CONFIRMED** from the formula alone, part of acceptance corpus A.

Division flag timing (`just_divided`/`daughter`): confirmed functionally
equivalent to the original's one-step visibility window, though
implemented via a different set/clear sequence spread across
`BacteriaColony.cpp:1806-1809` (clear) and `:1988-2032` (set after a
`divide()` mutation in the same step). Classified `no bug found`, pending
an explicit regression test (corpus A) before this code is touched.

## 12. 2D spatial/motion/mechanics contract (GenESyS-native)

Per §2 decisions 4–7, this contract replaces Chipmunk with a simple,
explicit, testable 2D kinematic model. Current state variables
(`BacteriumState`) already include continuous `positionX`/`positionY`,
`direction` (radians), `speed` and `volume` — reconciliation starts from
these, not from an assumed-fresh state shape.

**Confirmed defects (both reproducible from the formula alone, both must
be closed under corpus C, §9):**

1. `_updateBacteriumSpatialMotion()` (`BacteriaColony.cpp:2301-2336`):
   `if (!isfinite(speed) || speed <= 0.0) speed = 0.08 + 0.01*generation;`
   — a deliberate `speed := 0` is replaced by a positive fallback, so the
   bacterium keeps moving. Violates the required invariant "`speed == 0` ⇒
   position does not change."
2. Same function: `stepScale = max(0.35, simulationStep * 2.5);` — when
   `simulationStep` (the colony's step-time parameter) is `0`, `stepScale`
   is clamped to `0.35`, not `0`, so motion still occurs with zero elapsed
   step. Violates the required invariant "`dt == 0` ⇒ position does not
   change." This is the same clamp-floor anti-pattern as the growth defect
   in §11, confirmed by formula during this scope-refinement pass, not
   previously documented.

**Coordinate handling** (two inconsistent conventions, both confirmed):
`ecoli([x:=...,y:=...])` seed coordinates are shifted to be non-negative
and rounded to integer grid cells immediately (`:975-1020`), destroying
continuous position before any execution; `set_signal`/`set_signal_rect`
instead use a centered-origin convention (`toCenteredGridIndex`, `:162
-172`). `_setBacteriumPosition()` always recomputes `gridX/gridY` via
`llround(position)` for signal sampling — single-point sampling, no
heading-aware multi-point sampling.

**GenESyS-native kinematic contract (to implement):**
- State: `positionX`, `positionY` (continuous `double`), `direction`
  (radians, consistent with the existing `cos(direction)`/`sin(direction)`
  convention), `speed` (nonnegative scalar; `0` means stationary, with
  **no** positive fallback). `generation`-dependent speed increments found
  in the current growth code are a heuristic to review for removal, not a
  required part of the kinematic contract itself.
- Update rule, using the real elapsed step (no artificial minimum floor):
  `x(t+step) = x(t) + speed*cos(direction)*step`;
  `y(t+step) = y(t) + speed*sin(direction)*step`, where `step` is the
  colony's actual `simulationStep` for that dispatch, reconciled with
  existing units (already the convention `_updateBacteriumSpatialMotion`
  intends, modulo the floor defects above).
- `run(v)`: sets `speed := v` (propel along current heading at speed `v`);
  no force/torque, no Chipmunk.
- `tumble(v)`: reorients `direction` by a randomized amount parameterized
  by `v`, using **the GenESyS kernel RNG/sampler infrastructure** for
  reproducibility — **not** the current ad hoc FNV/SplitMix hash used by
  `rate(k)` in `GroProgramRuntime` (`deterministicUnitInterval`,
  confirmed self-contained and not kernel-seeded; flagged here as a
  reproducibility gap to close when `run`/`tumble` are touched, consistent
  with mandate requirement "randomização usa a infraestrutura RNG
  reprodutível do GenESyS").
- Domain limits: keep the existing boundary-reflection convention
  (direction mirrored at grid edges) — this already satisfies corpus C's
  "limites do domínio" requirement; no change needed structurally, only
  documentation.
- **Barriers**: deferred. Not required by corpus C (which asks for domain
  limits, already covered by boundary reflection, not line-segment
  barriers). If later required: model as 2D segment-crossing detection
  with position back-projection or velocity-normal removal (simple,
  deterministic, documented) — never a rigid-body engine.
- **Chemostat**: deferred. Not required by the selected subset. If later
  required: a simple directional velocity bias and/or boundary-region exit
  removal — never Chipmunk-equivalent mechanics.
- **Bacteria-bacteria collision**: explicitly **not** required. Bacteria
  may overlap; this is an accepted simplification per §2 decision 7, not a
  defect to track.

## 13. GenESyS event-time integration

Confirmed pipeline for one bacterium-scoped colony step
(`_executeBacteriumScopedGroProgram`, `BacteriaColony.cpp:1742-1847`): per
live bacterium — execute Gro program, sync spatial/growth state, clear
division flags, apply signal mutations, apply population mutations
(grow/divide/die), recompute grid position — then, once per colony step
after the full population loop: refresh update time, apply the single
signal-field diffusion/decay step, update spatial motion for every
bacterium, rebuild grid positions.

`executeGroProgram()` reparses and recompiles the full source text from
scratch on **every** call (`BacteriaColony.cpp:559-643`), with no cache by
hash/identity/revision — confirmed, not hypothesis. This is a performance
concern, addressed only after correctness phases close (a stale cache
during active semantic changes would be worse than the current cost).

Both the real event-calendar dispatch (`BacteriaColony::_onDispatchEvent`)
and the GUI viewer's "Step colony" button and "Start run" timer call the
exact same `executeGroProgram()` method (confirmed, §14 below); the colony
owns no separate internal clock contract beyond `ModelSimulation`'s own
time, so there is exactly one "advance simulated time" path — the risk is
two *triggers* for colony mutation, not two incompatible time domains.

## 14. GUI/viewer contract

`BacteriaColonyViewerGuiExtensionPlugin.cpp`: "Step colony" and the
"Start run" `QTimer` both call `_executeSelectedColonyStep()`, which calls
`colony->executeGroProgram()` directly (`:740-748`) — the same method the
event calendar calls via `_onDispatchEvent`. The code already contains an
explicit, correct comment (`:698-699`) stating that manual viewer
execution mutates colony state for inspection but does **not** advance
`ModelSimulation::getSimulatedTime()`. There is one state-mutation entry
point, triggered either by the event calendar (which also advances
simulated time) or by the viewer (which does not); the real risk is a user
running the viewer's manual controls *concurrently* with active
event-calendar replay on the same `BacteriaColony` instance. This must be
verified with a focused test (corpus E, §9), not assumed.

## 15. Persistence requirements

Confirmed persisted fields: `GroProgram.SourceCode` (code-editor-hinted
string property); `BacteriaSignalGrid` (`width`, `height`,
`initialSignal`, `diffusionRate`, `decayRate`, optional `initialValues`
CSV); `BacteriaColony` (`gridWidth`, `gridHeight`, `initialPopulation`,
`simulationStep`, `numSteps`, `groProgram` reference, optional
`signalGrid`/`bioNetwork` references, `nextId`).

Three real fixtures already exist (`models/Smart_GroColonyGrowth.gen`,
`Smart_GroColonyLifecycle.gen`, `Smart_BacteriaColony_GRO.gen`); the third
already demonstrates the single-argument `emit_signal` call shape that
must remain loadable (or be given an explicit, tested migration) once
multi-channel signals (§10) land.

`WiP2026108/PE_Fix` (PR #545) implemented SimulLang/GenSerializer text
round-tripping, including a GroProgram model round-trip test, and was
merged into `WorkInProgress` (merge `dce34847`), integrated into this
branch on 2026-10-08 (merge `c7816bf2`). The persistence phase of this
plan must rebase on the current serializer as shipped, not re-fix the
same persistence path.

## 16. Test/oracle matrix

Existing coverage: 46 focused `TEST()` cases in
`test_runtime_pluginmanager.cpp` covering plugin registration, parser
lexical boundaries, compiler IR shape, runtime command dispatch for every
currently-implemented builtin, and colony-level growth/motion/division/
signal/BioNetwork integration behavior (full list recorded in the Phase 0
evidence underlying this document).

Confirmed **not** covered by any existing test (regression gaps to close
*before* touching the corresponding production code, mapped to corpus
§9):
- corpus A: `ecoli_growth_rate = 0` → volume must not grow;
- corpus C: `speed = 0` → position must not change;
- corpus C: `simulationStep = 0` → position must not change (newly
  identified in this pass, §12);
- corpus B: two independent signal handles must not alias each other —
  **closed 2026-10-08, §17 phase 3**;
- corpus E: viewer manual step/run controls running concurrently with
  event-calendar dispatch on the same colony.

Closed 2026-10-08 (Phase 1, minimal frontend blockers): `needs`
comma-splitting (`daffe5bf`); unsupported/raw constructs silently
discarded by both real callers instead of being surfaced (`a2bd10b6`);
record-field read inside a boolean condition was reclassified from "gap"
to "already supported, now also locked by an explicit regression" after
executed verification — it required no production change.

`reaction()`, `fun`/`needs`-as-a-language-feature/`foreach`/
`maptocells ... end` remain unimplemented by §2/§6/§7 decision and are
**not** tracked as regression gaps — they are out of the selected subset.
The `needs`-comma-splitting **parser bug** (distinct from the broader
`needs`-as-scoping *feature*, which remains unimplemented by choice) was
closed in Phase 1 (`daffe5bf`, confirmed above); it is not an open gap.

## 17. Phased implementation plan (revised 2026-10-08)

Reordered per the maintainer's scope decision: deliver useful bacterial
-modeling capabilities before pursuing any further language compatibility.
Each phase: diagnose → regression test(s) → minimal implementation →
focused validation → regression validation (`tests-unit`/
`tests-kernel-unit`/`tests-smoke` as applicable) → update this document →
small, single-concern commit.

0. **Baseline, compatibility matrices, scope decisions** — done
   (`dbab2286`, `b7665852`).
1. **Minimal frontend blockers** — done 2026-10-08: (a) explicit
   diagnostic when a statement falls through to `RawStatement`/unsupported
   `FunctionCall` instead of a silently-discarded success (`a2bd10b6`);
   (b) fixed the `needs` comma-splitting bug (`daffe5bf`); (c) record-field
   **read** inside expressions/conditions was found already supported by
   executed evidence, requiring no change (unblocks corpus D and the two
   primary §8 acceptance-inspiration examples). Validated with full
   `tests-unit`/`tests-kernel-unit` (1832/1832 executed, 0 failed, 4
   preexisting disabled) and `tests-smoke` (3/3); `gui-app` rebuilt clean
   to validate the viewer-side change. Nothing else from §6's "no" column
   was touched.
2. **Selected runtime/builtins** — done 2026-10-08: `time()` no longer
   hard-errors (`24cd00f9`), validated with full `tests-unit`/
   `tests-kernel-unit`/`tests-smoke` (1833/1833 executed, 0 failed, 4
   preexisting disabled; 3/3 smoke). No other defensive gap was found in
   this pass; any further one discovered while implementing phases 3–6
   will be fixed there instead of reopening this phase. Builtins marked
   "no" in §7 remain unimplemented by decision.
3. **Signal channels** — done 2026-10-08 (`58783cba`, `5c1df906`, plus the
   handle-validation/leak/resize closeout in this pass): `signal(kdiff,
   kdeg)` assigns each declaration an ordinal handle (1st, 2nd, ... in
   program order; `GroProgramRuntimeState::signalDeclarationOrdinal`,
   reset every execution pass). `BacteriaColony` also records the declaring
   source scope for each ordinal during a replication so independent named
   programs cannot silently reuse the same handle; this is per-channel
   runtime metadata, not a symbol registry. Handle 0 (no explicit handle, pre-multi-channel raw-IR
   convention) and handle 1 (the first `signal(...)` declaration) both
   resolve to the colony's pre-existing single field (`_signalField`/
   `"local_signal"`), so every pre-multi-channel program, fixture and test
   keeps its exact prior behavior unchanged. Handle N >= 2 addresses a
   genuinely independent `BacteriaColony::AdditionalSignalChannel` (own
   diffusion rate, decay rate and `std::vector<double>` field, sized to the
   current grid), lazily allocated by an `EnsureSignalChannel` colony
   mutation the first time its declaration executes.
   `get_signal`/`emit_signal`/`absorb_signal`/`set_signal`/`set_signal_rect`
   all resolve their handle argument through the same
   `tryResolveSignalChannelHandle` contract (mirrored in
   `GroProgramRuntime.cpp` for the Gro-expression/command layer and
   `BacteriaColony::_tryResolveSignalChannelHandle` for the two
   colony-mutation-level commands): a handle must be within `1e-9` of an
   integer, non-negative, and either `<= 1` (legacy) or `<=` the number of
   channels actually declared so far in the current execution pass —
   negative, non-integer, and not-yet-declared handles are explicit
   `result.succeeded = false` / mutation-rejection errors ("received an
   invalid signal channel handle (...)"), never a silent fallback to the
   legacy channel. `BacteriaColony::_initBetweenReplications()` and the
   `reset()` builtin's colony mutation now clear
   `_additionalSignalChannels` explicitly (fixing a confirmed leak: the
   lazy-allocate-if-size-mismatched path in `_ensureAdditionalSignalChannel`
   does not zero a channel whose field is already correctly sized, so a
   value emitted in one replication was otherwise still readable at the
   start of the next). `_resetRuntimeSignalField()` now also resizes every
   already-declared additional channel's field to the current grid
   dimensions (fixing a confirmed dimension-incoherence bug where a channel
   declared before a `signal_grid_width`/`signal_grid_height` mutation kept
   its stale, pre-resize field length). `reaction()` and `get_signal_matrix`
   signature repair remain out of scope per §7; the diffusion/decay formula
   itself (per-channel 4-neighbor relaxation, no `dt`) is unchanged from
   §10 and is Phase 4's concern, not this phase's.
   Regression evidence: `GroProgramRuntimeAssignsIndependentOrdinalSignalChannelHandles`,
   `BacteriaColonyMaintainsTwoIndependentSignalChannelsWithoutAliasing`
   (both from the prior pass) plus, added in this pass,
   `GroProgramRuntimeRejectsInvalidSignalChannelHandles`,
   `BacteriaColonySetSignalAndSetSignalRectRespectChannelHandle`,
   `BacteriaColonyRejectsSetSignalWithUndeclaredChannelHandle`,
   `BacteriaColonyAbsorbsSignalIndependentlyPerChannel`,
   `BacteriaColonyDoesNotLeakAdditionalSignalChannelsBetweenReplications`,
   `BacteriaColonyResizesAdditionalSignalChannelsWithGridDimensionChanges`,
   `BacteriaColonyMaintainsSignalChannelIndependenceAcrossMultipleSteps`
   (`source/tests/unit/test_runtime_pluginmanager.cpp`). The four new
   production-facing tests were confirmed RED against the pre-fix code
   (invalid handles silently accepted and aliased to the legacy channel;
   `set_signal` with an undeclared handle silently accepted; channel 2
   read back a leaked value across a simulated replication boundary;
   a channel declared before a grid resize silently dropped a write at a
   coordinate outside its stale size) before the fix, and GREEN after.
   No `.gen` persistence format change was needed or made: additional
   channels are pure runtime state (`_additionalSignalChannels` is not a
   persisted field of `BacteriaColony` or `BacteriaSignalGrid`), reconstructed
   every replication by re-running the program's own `signal(...)`
   declarations; the three existing fixtures (§15) are unaffected and were
   not re-validated by this pass beyond the pre-existing single-channel
   regression coverage. Validated with full `tests-unit`/`tests-kernel-unit`
   (1856/1858 executed, 0 failed, 4 preexisting disabled, plus the 2
   preexisting unrelated `PropertyEditorDoubleCommit` locale failures from
   PR #545 confirmed present on bare `WorkInProgress` before this branch's
   changes) and `tests-smoke` (3/3); `gui-app` rebuilt clean. Deferred to
   Phase 4 by design: the diffusion/decay formulation itself, `dt` usage,
   boundary handling, and any numerical/stability review.

   **Correction round 2026-10-08 (independent review, `338032de` ->
   `8868903d`):** an independent review of the above closeout found three
   further issues, confirmed and resolved as follows (commits `1b0dbf7a`
   fix, `8868903d` tests):
   - **Problem A (fixed):** `_applySignalFieldStep()` only called
     `_applyAdditionalSignalChannelsStep()` in the branch taken when no
     `BacteriaSignalGrid` is attached. Any colony with a real
     `BacteriaSignalGrid` attached silently stopped stepping its
     additional (handle >= 2) channels entirely (the same early return
     also fired whenever the legacy field's own coefficients happened to
     be zero). Fixed by calling `_applyAdditionalSignalChannelsStep()`
     unconditionally, exactly once, before any legacy-field branching.
     Regression: `BacteriaColonyStepsAdditionalSignalChannelsWithSignalGridAttached`
     (confirmed RED before the fix: the emitted value stayed frozen
     instead of diffusing).
   - **Problem B (implemented under the maintainer's explicit decision):**
     the first `signal(kdiff,kdeg)` declaration (handle 1) controls the
     legacy field when there is no attached `BacteriaSignalGrid`. If a
     grid is attached, its persisted coefficients remain authoritative;
     a mismatch emits one trace diagnostic per replication. Additional
     channels keep their own Gro coefficients. Fields are stepped once
     per colony step and the existing relaxation equation is unchanged.
     `BacteriaColonyExecutesSeededNamedGroPrograms` now expects `0.0`
     after `leader()` emits `5.0` with `kdeg=1`: `follower()` still reads
     the `5.0` during the per-bacterium loop before the once-per-step
     decay. Focused tests cover zero, full and intermediate coefficients,
     grid precedence, no declaration, replication reset, `reset()` metadata
     reset, and one trace diagnostic per replication. No `.gen` serialization
     field or format changed.
   - **Problem C (fixed):** handle stability relied entirely on every
     caller happening to construct a fresh `GroProgramRuntimeState` per
     execution pass; `GroProgramRuntime::execute()` itself never reset
     `signalDeclarationOrdinal`, so a caller reusing the same state object
     across repeated executions of the same IR would see a declaration's
     handle drift upward on every call (confirmed RED via
     `GroProgramRuntimeKeepsSignalHandleStableAcrossRepeatedExecuteCallsOnReusedState`:
     handle went 1 -> 2 -> 3 across three calls before the fix).
     `execute()` now resets the ordinal explicitly at entry — a no-op for
     every current caller, all of which already pass fresh state.
     Investigating the "global declarations -> prelude -> named bacterium
     program -> `get_signal(handle)`" path separately surfaced a real
     interaction bug with the Problem-3-closeout handle validation: a
     named program that only *consumes* a handle declared once by the
     colony-wide prelude (without itself containing a `signal(...)` call)
     has `signalDeclarationOrdinal == 0` in its own fresh per-bacterium
     pass, so validation rejected an otherwise-valid handle as
     "not yet declared" (confirmed RED via
     `BacteriaColonyPropagatesGlobalSignalHandlesToMultipleNamedPrograms`).
     Fixed by adding `GroProgramRuntimeState::knownSignalChannelCount`,
     set by `BacteriaColony` at every state-construction site from its
     own persistent `_additionalSignalChannels.size()` — not a new
     name-based registry, just the same count `BacteriaColony` already
     tracked, now also visible to validation as a lower bound alongside
     the per-pass ordinal. Reset-between-replications handle stability
     (`BacteriaColonyKeepsGlobalSignalHandleStableAcrossReplications`) was
     already correct before this fix; the test locks the contract.
   - **Signal declaration safeguards (2026-10-09):** only unconditional
     declarations are supported. A structural IR walk rejects `signal(...)`
     anywhere inside `thenCommands` or `elseCommands`, at any nesting depth,
     before evaluating conditions; this includes branches that are currently
     false. Repeated execution of one `program bacterium()` or one named
     program shares that program's ordinal handles across its bacteria. A
     different named program cannot independently declare an already-owned
     ordinal, even with identical coefficients; differing coefficients for
     an ordinal in one scope also remain an error. The diagnostic directs
     authors to declare shared channels once in global scope and consume the
     resulting handles in named programs. Inspection found no tracked `.gen`
     fixture using `signal()` and the existing global-sharing / single-program
     tests remain valid. The guard stores only each channel's declaration
     scope; no symbol registry or Gro syntax was added.
   - Validated after the correction round: `tests-unit`/`tests-kernel-unit`
     1861/1863 executed, 0 failed, 4 preexisting disabled, same 2
     preexisting unrelated `PropertyEditorDoubleCommit` locale failures;
     `tests-smoke` 3/3; `gui-app` rebuilt clean.

   **Problem B verification (2026-10-09):** the focused signal/runtime suite
   is GREEN (61/61). Against reference HEAD `5cb05b3`, applying the updated
   tests without the implementation produced RED in the first-channel
   coefficient, replication-reset, mismatch-diagnostic, conditional-declaration,
   and cross-program-conflict cases. With the implementation, `tests-unit`
   and `tests-kernel-unit` each completed 1,874/1,874 runnable tests, 0 failed,
   with 4 historical tests disabled; `tests-smoke` passed 3/3 and `gui-app`
   built. The three `PropertyEditorDoubleCommit` cases passed in both current
   full presets. The current fixture loader check passes for
   `Smart_GroColonyGrowth.gen` and `Smart_GroColonyLifecycle.gen`.
   `Smart_BacteriaColony_GRO.gen` is byte-for-byte unchanged but its loader
   returns null both on `5cb05b3` and on this implementation branch; this is
   recorded as a baseline fixture-loading issue, not as a passing compatibility
   check or a regression introduced here. None of the three fixture files was
   edited, and the serializer and `.gen` format were not changed.

   **Final identity review (2026-10-09):** RED reproduction at `fb4e7635`
   used two separately named programs, each declaring `signal(0,0)` and
   emitting 5 and 7 at separate seeded coordinates. Execution succeeded;
   both values appeared in the legacy handle-1 field and handle 2 stayed zero,
   proving silent aliasing despite independent declarations. The same tests
   confirmed that two bacteria running one `program bacterium()` declaration
   share its channel and that two named programs consuming global handles
   remain independent. The equal-rate reproducer is now a rejection test with
   an actionable global-scope diagnostic; the different-rate case is also
   rejected. A conditional declaration in `if(false)` was accepted before the
   IR walk and is now rejected alongside the true-branch case.

   Final local validation on this revision: focused signal/runtime tests
   passed 8/8; full `tests-unit` and `tests-kernel-unit` each passed 1,877
   runnable tests with 0 failures and 4 disabled; `tests-smoke` passed 3/3;
   the `gui-app` build succeeded. `scripts/validate-ai-docs.py` and
   `git diff --check` passed. These are local results; no CI run was triggered
   or inferred. No `.gen` fixture or serializer was changed. The previous
   baseline investigation of `Smart_BacteriaColony_GRO.gen` remains separate:
   its loader returned null both at `5cb05b3` and at the prior Phase 3 build;
   this task does not address that issue. The existing relaxation equation
   remains unchanged and Phase 4 is **not started**.
4. **Reaction-diffusion review**: revisit the existing 4-neighbor/no-`dt`
   field formulation only as needed to support multi-channel fields from
   phase 3; document whatever formulation is kept or changed, with the
   `.gen`-fixture impact note required by §10.
5. **Physical coordinates**: remove the shift-and-round seeding path;
   reconcile with the centered convention used by `set_signal`.
6. **Growth/division**: remove the `0.01` minimum growth clamp (§11);
   regression-first (corpus A).
7. **Simple internal mechanics**: implement the §12 GenESyS-native
   kinematic contract; remove the `speed<=0` and `stepScale` positive
   -floor fallbacks; redefine `run`/`tumble` on top of it using the kernel
   RNG; regression-first (corpus C).
8. **Event-step integration**: make the per-colony-step ordering explicit
   and tested against the mechanics/signals changes above.
9. **Viewer**: add the concurrent-trigger regression (corpus E); keep the
   viewer strictly a read/observe + explicit-step-request surface.
10. **Selected end-to-end corpus**: author and pass the GenESyS-native
    fixtures for corpus A–E (§9), inspired by but not copied from
    `spatial_oscillations.gro`/`symbiosis.gro`.
11. **Performance**: only after correctness phases close; address the
    reparse-every-call cost (§13) with a safe cache keyed by source
    identity.
12. **Persistence**: rebase on the serializer shipped by `WiP2026108/PE_Fix`
    (PR #545, merged); validate the three existing `.gen` fixtures continue
    to load (or document/
    migrate any intentional change from phases 3/4/5/6).
13. **Documentation/manual/completion gate**: reconcile this document,
    `STATUS.md`, the AI changelog, and manual impact per governance §10.

## 18. Known deviations (intentional, to document going forward)

- `die(n)` accepts an amount argument; the original `die()` takes none.
- `divide()` outside bacterium-scoped mode affects the whole aggregate
  population rather than a single cell — a GenESyS-specific aggregate-mode
  semantic, not a bug.
- `tick()`, `grow(n)`, `set_population(n)`, `dump_signal_field(r,c)` are
  GenESyS extensions with no Gro-original equivalent.
- `rate(k)` uses `1 - exp(-k*dt)` (a proper Poisson-process per-step
  probability) instead of the original's `k*dt > rand()/RAND_MAX`
  approximation — kept as a documented improvement. Its RNG source should
  still move to the kernel sampler per §12.
- No Chipmunk-equivalent rigid-body physics, forces, torques, or
  bacteria-bacteria collision — by maintainer decision (§2), not a gap.

## 19. Explicitly unsupported / deferred behavior

- `fopen`/`fprint`/`dump` to arbitrary filesystem paths: deferred pending a
  security decision (unrelated to this plan's scope narrowing).
- GUI-only builtins (`zoom`, `set_theme`, `snapshot`): intentionally
  absent from the headless runtime.
- `reaction()`, `get_signal_matrix` argument-count repair, `geometry()`,
  `stats()`, `stop()`/`start()`, `clear_messages()`, `print()`/`clear()`:
  intentionally unsupported — no selected capability needs them (§7).
- Full academic Gro grammar (`fun`, `let/in/end`, lambdas, general
  lists/records as first-class values, `foreach`/`cross`, the
  `maptocells ... end` keyword form): intentionally unsupported (§2
  decision 8, §6).
- External 2D physics engine (Chipmunk or equivalent), rigid-body
  collision, barriers, chemostat physical effects: deferred/not required
  by the selected subset (§2 decisions 4–7, §12).

## 20. Open decisions / stop gates

Closed, not open: adoption of an external physics engine (§2 decisions
4/5 — do not reopen).

Genuinely open/to watch:
- Conditional `signal(...)` declarations and independent declarations of one
  ordinal by different named programs are rejected by the selected subset;
  see the Phase 3 safeguard note in §17. Unconditional declarations within
  one program and global handles consumed by multiple programs remain
  supported.
- Phase 4 (reaction-diffusion formulation review) remains **not started**.
  The current discrete relaxation equation, its boundary behavior, and
  any use of `dt` remain outside this Phase 3 decision.
- **Resolved for Phase 3:** signal channel fields and coefficients are
  runtime state reconstructed from declarations; no `.gen` format change or
  migration was needed. The three existing fixtures remain unchanged; the
  independent baseline loader caveat for `Smart_BacteriaColony_GRO.gen` is
  recorded in the Problem B verification above.
- `WiP2026108/PE_Fix` (PR #545) merged 2026-10-08; the persistence phase
  (§17 item 12) now rebases on the current serializer as shipped — this
  item is resolved, not open.
- If a future maintainer-selected fixture genuinely requires barriers or
  chemostat physical effects beyond what §12's simple geometric policy can
  express reasonably, present options (simple internal extension vs.
  continued deferral) — but **not** an external physics engine, which
  remains closed.

## 21. Completion criteria (revised 2026-10-08)

Per §2 decision 9, completion is **capability-based**, not a percentage of
original-Gro compatibility. `BACTERIA-COLONY-INTEGRATION-COMPLETE` requires
executed evidence (not static reading) for:

- **Language subset**: the subset declared in §6 has defined syntax,
  explicit diagnostics for anything outside it, and a consistent
  parser/compiler/runtime, documented.
- **Bacterial simulation**: population, growth (including the
  zero-growth invariant), division with parent/daughter semantics, death
  where selected, continuous positions, `run`/`tumble` under the §12
  kinematic contract (including the zero-speed/zero-step invariants),
  deterministic/reproducible mechanics using the kernel RNG.
- **Signals**: independent signal channels, emit/absorb/read respecting
  handles, diffusion, degradation, with numerical tests.
- **Simulation integration**: explicit event-calendar semantics, no
  duplicate/hidden time progression, correct reset between replications.
- **GUI**: the viewer represents actual runtime state, creates no
  alternate physics/clock, and renders signals/positions/division/death
  consistently.
- **Engineering**: persistence, tests, sanitizers where relevant,
  documentation, regression, CI.

Gro features explicitly marked out-of-subset (§19) do **not** block
`BACTERIA-COLONY-INTEGRATION-COMPLETE`, provided the documentation is
unambiguous about what was intentionally left out and why. Until every
corpus A–E capability in §9 has executed, passing evidence, report state
as `BACTERIA-COLONY-INTEGRATION-PARTIAL` with the exact remaining
capability, blocker, evidence, required decision and next action, using
this document's §17 phase numbering.

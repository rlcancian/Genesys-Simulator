---
document_type: reference
authority: technical-plan
owner: project-maintainer
last_reviewed: 2026-10-09
review_cadence: on-phase-completion
status: active
---

# Bacteria Colony / Gro Integration Plan

## Approved first-version rebaseline: four milestones, nine cycles

**Approved by explicit maintainer instruction dated 2026-10-09.** This
rebaseline replaces the pending proposal formerly in §22 and authorizes the
nine-cycle execution scope recorded there and in `AUTO-BACTERIA-001`.
Historical inventories, decisions, tests, commits and evidence in §§1–21
remain available with their original phase numbers; they are historical
evidence, not additional requirements for a larger version.

The maintainer ratified Alternative A from §10 for this first version: retain
the current dimensionless per-update relaxation as a phenomenological
concentration-propagation operator, constrain finite `kdiff` and `kdeg` to
`[0,1]`, and make no physical-PDE or conservation claim. Alternatives B and C
are postponed. The equation and operation order remain unchanged.

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

**Phase 4 numerical characterization (2026-10-09):** on the current
implementation branch, for a rectangular `W × H` array `u^n[x,y]`, define
`N(x,y)` as the in-bounds orthogonal (left/right/up/down) neighbors and
`m(x,y)=|N(x,y)|`. With per-step Gro/grid coefficients `α=diffusionRate`
and `β=decayRate`, the exact simultaneous update is
```
r[x,y] = u^n[x,y] + α * (sum(u^n[q] for q in N(x,y))/m(x,y) - u^n[x,y])  if m(x,y)>0
       = u^n[x,y]                                                               if m(x,y)=0
u^(n+1)[x,y] = max(0, r[x,y] * (1-β))
```
Every right-hand-side value comes from the pre-step field; the new field is
committed after all cells are visited. A 1×1 grid therefore has no diffusion
term, only degradation. There is no explicit boundary condition object: the
finite grid truncates the neighborhood and divides by the local degree. This
is a degree-normalized graph averaging operator, not a regular-grid
finite-difference Laplacian or a symmetric face-flux update.

The colony processes Gro emissions/absorptions/sets during the bacterium
loop, then applies each field's diffusion/degradation operator once after
that loop. Legacy handle 0/1 uses the attached `BacteriaSignalGrid` rates if
present, otherwise the first declared `signal(kdiff,kdeg)` rates (zero/zero
if no declaration ran); each additional channel is updated once using its
own declaration rates. Emission thus participates in the same step's field
update. The signal operator does not read `simulationStep`, the model time
unit, or a cell-size parameter. Accordingly, `α` and `β` are dimensionless
fractions per colony update in the implementation; the identifiers “rate”
do not establish physical units. `simulationStep` remains relevant to other
colony behaviors and Gro's `dt`, but changing it alone leaves this field
update unchanged.

The current edge averaging is not mass-conservative under the ordinary
unweighted sum. For a 3×3 center impulse of 10 with `α=0.5`, `β=0`, the
center becomes 5 and each orthogonal neighbor becomes 5/3; total concentration
becomes 35/3. For a corner impulse of 10 on 3×3 the total becomes 25/3.
On 2×2, every cell has degree 2, so the same corner impulse redistributes
to `[5, 2.5; 2.5, 0]` and the sum remains 10. These different outcomes are
explained by the variable degree, not source/sink terms. On a connected
non-isolated grid with `β=0`, the degree-weighted sum `Σ m(x,y)u[x,y]` is
invariant under this random-walk relaxation; the ordinary concentration sum
is not. With `α,β ∈ [0,1]`
and nonnegative finite input, the local relaxation is a convex combination
followed by multiplication by a factor in `[0,1]`; it preserves
nonnegativity and does not amplify the maximum norm. This is a bounded
per-step heuristic under that coefficient range, not evidence of
consistency/convergence to a physical PDE. The `BacteriaSignalGrid` check
rejects finite out-of-range values using comparisons, but does not explicitly
reject non-finite values; the Gro declaration path also has no interval or
finiteness guard. For out-of-range Gro values, the convexity/positivity
argument no longer applies; the final `max(0,...)` clips negatives and can
alter mass or produce nonphysical growth. NaN/infinity therefore remain an
input-validation risk, not an accepted scientific behavior.

**Scientific alternatives and decision gate (Phase 4; no equation changed):**

| Alternative | Equation and interpretation | Numerical properties and cost | API, Gro, persistence impact |
|---|---|---|---|
| A — keep relaxation | The exact graph update above; `α` and `β` are empirical dimensionless fractions per update. | O(WH) work and O(WH) scratch storage per field; bounded and positive for finite `0≤α,β≤1`. Not ordinary mass-conserving diffusion on irregular-degree boundaries; independent of `dt` and cell size. | Smallest change: document units/interval, validate coefficients and test its graph semantics. Existing Gro values and `.gen` structure stay compatible; changing `simulationStep` does not change field evolution. It must be described as a phenomenological relaxation, not a calibrated physical reaction-diffusion PDE. |
| B — conservative grid diffusion | For uniform spacing `h`, explicit no-flux finite volumes give `u_p^(n+1)=u_p^n + (D Δt/h²) Σ_{q~p}(u_q^n-u_p^n) - λ Δt u_p^n`; missing exterior faces carry zero flux. | Shared face fluxes cancel in the sum, so diffusion conserves total amount for equal cell volumes when reaction is absent. O(WH) per explicit step. A sufficient positivity bound is `4DΔt/h² + λΔt ≤ 1`; a conservative spectral-stability bound is `8DΔt/h² + λΔt ≤ 2`. Smaller `h` or larger `Δt` can require substeps. | Requires cell spacing and actual elapsed time in the signal API/configuration; existing `signal(kdiff,kdeg)` values need a migration/interpretation decision. Grid data could remain structurally serializable, but existing numeric behavior changes. Explicitly choose boundary behavior (zero flux is proposed here). |
| C — physical reaction-diffusion | Specify `∂u/∂t = D∇²u - λu + S` with calibrated `D` (length²/time), `λ` (1/time), source units, domain spacing, and boundary conditions; discretize in space and integrate in time. | Explicit integration has a timestep stability/positivity limit related to `DΔt/h²` and `λΔt`; adaptive substeps or an implicit positive solver can relax the stability restriction, with more computation and solver/error-control complexity. | Requires defined physical units, cell spacing, time-unit conversion and coefficient meaning. Existing Gro and persisted grid values cannot retain their old per-step semantics without an explicit migration/version policy. `.gen` format need not change mechanically, but semantic compatibility cannot be claimed merely because fields still load. |

Alternative A is the only one justified by the current API and persisted
values: no source establishes length/time units or a calibrated diffusion
constant. The maintainer approved A for the first-version scope on
2026-10-09: keep the current equation as a documented dimensionless per-step
relaxation and reject non-finite or out-of-domain coefficients. This does not
establish unweighted mass conservation or physical time/space scaling; if
either becomes an acceptance criterion, a new scientific decision is required
before an equation change.
This distinction follows standard explicit heat-equation stability
analysis and finite-volume flux conservation (see [MIT notes on 2D explicit
heat finite differences](https://dspace.mit.edu/bitstream/handle/1721.1/35256/22-00JSpring-2002/NR/rdonlyres/Nuclear-Engineering/22-00JIntroduction-to-Modeling-and-SimulationSpring2002/55114EA2-9B81-4FD8-90D5-5F64F21D23D0/0/lecture_16.pdf)
and the [TU Delft finite-volume introduction](https://mude.citg.tudelft.nl/book/2024/fvm/fvm.html)).

Characterization tests added at `source/tests/unit/test_runtime_pluginmanager.cpp`
cover 1×1, 2×2 and 3×3 grids; uniform, center-impulse and corner-impulse
fields; zero diffusion/degradation; complete degradation; intermediate
coefficients; different `simulationStep` values; and distinct numeric
evolution for two independently configured channels. These tests lock down
legacy behavior, including the non-conservative edge counterexamples; they
are not scientific validation of a continuum model. They passed locally on
2026-10-09. The full preset results and exact build environment are recorded
under §17 item 4 after completion of the regression run.

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

## 17. Historical phased implementation record (revised 2026-10-08)

This section preserves the prior numbered plan and its evidence without
renumbering. It is the historical record of phases 0–13; the proposed
forward execution sequence is §22 and becomes authoritative only after
maintainer approval.

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
   remains unchanged; Phase 4 is now in numerical diagnosis with no production
   equation change.
4. **Reaction-diffusion numerical review** — diagnosis recorded 2026-10-09
   in §10; no production equation change. The current operator is a
   degree-normalized, dimensionless per-step graph relaxation; it is not
   ordinary mass-conserving grid diffusion and has no `dt`/cell-spacing
   consistency. Added independent small-grid characterization at 1×1,
   2×2 and 3×3, including boundary mass counterexamples, uniform and impulse
   fields, coefficient endpoints, multiple channels, and varying `dt`.
   Local `tests-unit`: 1,878 passed/0 failed, 4 disabled;
   `tests-kernel-unit`: 1,878 passed/0 failed, 4 disabled;
   `tests-smoke`: 3/3 passed; `gui-app` build succeeded (no work required).
   CMake 3.28.3/Ninja 1.11.1, GNU C++ 13.3.0, C++23 with extensions off.
   No CI result is claimed. Alternatives A/B/C and the maintainer's approval
   of A for the first-version scope are in §10. A remains an explicitly
   phenomenological per-step operator; selecting B/C or assigning physical
   meaning to coefficients requires a new maintainer decision before
   production changes. Existing tests
   characterize current behavior (so no implementation RED/GREEN cycle was
   applicable). No `.gen`, serializer, or model file was changed. Phase 5
   remains not started.
   **Cycle 1 signal-parameter closeout (2026-10-09):** the approved
   Alternative A domain is now enforced. RED tests demonstrated that the
   grid accepted NaN through ordinary comparisons, Gro accepted finite
   out-of-range coefficients, and Gro could leave ordinal/EnsureSignalChannel
   mutations for an invalid declaration. `BacteriaSignalGrid::_check()` now
   rejects non-finite and out-of-range diffusion/decay values; Gro declaration
   execution reports a finite `[0,1]` diagnostic before changing its ordinal,
   assigned variable, or channel mutation list. Tests include endpoints,
   intermediate values, negative/>1 values, NaN/infinity and explicit
   no-mutation assertions. GREEN: both new tests pass and all 86
   `RuntimePluginManagerClassTest` cases pass locally after the change. The
   focused `tests-unit` preset was configured with CMake 3.28.3, Ninja 1.11.1,
   GCC 13.3.0, Qt 6.4.2 available and C++23 extensions disabled. No full
   preset regression or CI result is claimed at this cycle checkpoint. The
   relaxation equation, `.gen` fields and serializer are unchanged.
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

## 20. Historical decisions / stop gates

Closed, not open: adoption of an external physics engine (§2 decisions
4/5 — do not reopen).

Genuinely open/to watch:
- Conditional `signal(...)` declarations and independent declarations of one
  ordinal by different named programs are rejected by the selected subset;
  see the Phase 3 safeguard note in §17. Unconditional declarations within
  one program and global handles consumed by multiple programs remain
  supported.
- Phase 4 diagnosis is recorded in §10 and historical §17 item 4. Production
  still uses the characterized relaxation unchanged. The current proposal in
  §22 recommends A as a dimensionless per-step heuristic; maintainer
  ratification is pending. B/C remain postponed. Do not assign physical units
  to existing coefficients or implement a different equation without a new
  decision. Historical Phase 5 (Physical Coordinates) remains not started;
  its proposed work is mapped to M2 in §22.
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

## 21. Historical completion criteria (revised 2026-10-08)

The criteria below preserve the earlier A–E acceptance corpus. The concise
first-version gate proposed for the new plan is in §22.4; it does not mark
any capability complete without executed evidence.

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

## 22. Approved simplified plan: four milestones, nine cycles

Status: **approved and active** by explicit maintainer instruction dated
2026-10-09. `AUTO-BACTERIA-001` is running on the existing feature branch.
Historical phase citations in §§1–21 stay unchanged.

### 22.1 Requirement classification

The categories below describe current code and executed evidence at the
feature-branch head recorded in the active cycle reports. “Implemented” is reserved for the tested behavior
listed here; source presence alone is not validation.

| Classification | Requirements and evidence |
|---|---|
| **Already implemented and validated** | The selected Gro parser/compiler/runtime subset, persistent record-like variables and conditional syntax have focused runtime tests. Signal handles are independent; emit/absorb/read/set route by handle; invalid and conditional declarations are rejected; global handles can be shared through the supported global declaration pattern. Phase 3 precedence and once-per-replication mismatch trace behavior have focused tests. Evidence is listed in historical §17 and the actual tests in `source/tests/unit/test_runtime_pluginmanager.cpp`. Growth has visible-effect tests; bacterium-scoped division and death have focused tests, but the zero-growth/complete lifecycle contract is not thereby validated. `Smart_GroColonyGrowth.gen` and `Smart_GroColonyLifecycle.gen` have an executed fixture-load test. |
| **Obrigatório para a primeira versão** | Finite `[0,1]` signal-coefficient validation and documentation of approved Alternative A; continuous-coordinate seeding and consistent grid sampling; zero-growth and zero-displacement invariants; division/death/orientation/movement contract and reproducible `run`/`tumble`; one coherent update order across Gro, bacteria, fields and event calendar; GUI view of runtime state; minimum end-to-end evidence for corpus A–E, reusing existing tests; supported-model load/save validation; causal resolution or explicit supported-model classification for the known `Smart_BacteriaColony_GRO.gen` loading failure; full required regressions and documented scientific limits. |
| **Adiado para versão posterior** | Alternatives B/C; physical coordinates/units beyond the continuous kinematic contract; mass-action `reaction()`; remaining academic Gro grammar/builtins; barrier collision geometry and chemostat physical effects; advanced bacteria collision; performance optimization unless a measurement finds a material delivery bottleneck. These are not extra first-version phases. |
| **Descartado por decisão arquitetural existente** | Chipmunk2D or any external rigid-body/physics engine; true rigid-body forces, torques and collision; full compatibility with the original Gro implementation or all 23 original examples. Do not reopen these decisions absent new maintainer instruction. |

Alternative A and its finite `kdiff,kdeg ∈ [0,1]` domain are approved for
this first version. Cycle 1 validates both Gro declarations and attached
grid configuration; the equation and operation order remain unchanged.
Alternatives B/C are postponed, and this approval does not raise the claim
above a phenomenological, non-conservative relaxation.

### 22.2 Status of the four milestones

| Milestone | Real status at the reference HEAD | Closure condition |
|---|---|---|
| **M1 — Gro e sinais** | **Cycle 1 complete.** The selected frontend subset, handles, channel independence, Phase 3 coefficient precedence and Phase 4 characterization remain covered. Gro declarations and attached-grid settings now reject non-finite/out-of-range coefficients; the approved phenomenological contract is recorded. | Keep existing signal regressions green while later cycles integrate colony and viewer behavior. |
| **M2 — Dinâmica bacteriana** | **In progress (Cycles 2–5 complete).** Gro seed coordinates remain continuous and centered; zero-growth/zero-step growth invariants, optional persisted threshold division, centralized volume-conserving division, `speed*dt` movement, kernel-sampler `tumble`, boundary reflection, and bounded geometric separation are implemented and covered by focused runtime tests. | Coordinates, growth, division, death, motion, orientation, zero invariants, boundary rule and reproducible RNG have focused and integrated tests. |
| **M3 — Integração e GUI** | **Partial foundation.** Gro execution, signal changes and colony stepping exist. Event calendar and viewer call the same execution method, but a manual viewer step does not advance model time; the state/time contract and interaction with calendar replay are not fully validated. The viewer exists, but corpus-E fidelity and combined A–E end-to-end coverage remain open. | Integrated tests demonstrate one coherent runtime/event/viewer state contract and visible diagnostics; only the smallest missing end-to-end tests are added. |
| **M4 — Validação e entrega** | **Partial.** Gro persistence round-trip infrastructure exists. Growth and lifecycle `.gen` fixtures have an executed load test. The plan records `Smart_BacteriaColony_GRO.gen` loader failure on both Phase 3 reference and implementation branch; this was not a passing compatibility result. Completion-level current CI and performance evidence are not established by the historical snapshots. | Supported fixtures round-trip without editing them to mask failures; known loader issue is fixed or its support status is explicitly decided; required final-head regressions/GUI/CI and documentation criteria pass. Performance work occurs only after measured evidence. |

### 22.3 Crosswalk: historical phases 0–13 to M1–M4

This crosswalk maps the fourteen old phase numbers without changing their
meaning or citations. “Done” below means the historical record says done at
its recorded commit; it is not an automatic `done_confirmed` claim under
current governance.

| Historical phase | Historical scope/status | New milestone |
|---:|---|---|
| 0 | Baseline, compatibility matrices and scope; completed as recorded. | M1 foundation; evidence also feeds M4. |
| 1 | Minimal frontend blockers; completed and validated in the historical record. | M1. |
| 2 | Selected runtime/builtins (`time()`); completed as recorded. | M1. |
| 3 | Signal handles, independent channels, identity safeguards and precedence; completed and tested as recorded. | M1. |
| 4 | Reaction-diffusion review; diagnosis/characterization recorded; no equation change. Alternative A approved for first-version scope. | M1. |
| 5 | Continuous coordinate/grid-mapping request; implementation and validation completed in Cycle 2. Physical units remain out of scope. | M2. |
| 6 | Growth/division was open in the historical phase record; zero-growth, zero-step and optional hybrid volume-conserving division completed in Cycles 3–4. | M2. |
| 7 | Kinematics, `run`/`tumble`, kernel RNG and zero-motion behavior were open in the historical phase record; the simplified contract is implemented and tested in Cycle 5. | M2. |
| 8 | Event-step ordering; not closed. | M3. |
| 9 | Viewer/runtime integration and trigger risk; not closed. | M3. |
| 10 | Selected A–E end-to-end acceptance corpus; partially covered by reusable tests, not closed. | M3. |
| 11 | Performance/cache; conditional on measured bottleneck, not a release prerequisite by itself. | M4, only if measurement justifies it. |
| 12 | Persistence and the three example models; two load tests pass, third known loader issue remains. | M4. |
| 13 | Documentation, manual impact and completion gate; continuous closeout work. | M4. |

Historical test names, commit identifiers, RED/GREEN evidence, loader
limitations and prior claims remain in §§1–21. The new milestones are not a
replacement for that evidence archive.

### 22.4 Approved first-version acceptance criteria

The first version may be called complete only when all seven criteria have
executed evidence at the final branch head:

1. Supported bacterial models can be created and run through the supported
   Gro subset.
2. Bacteria grow, divide, die and move under the simplified contract;
   zero growth and zero speed/elapsed step produce zero corresponding
   changes, and stochastic orientation is reproducible with GenESyS RNG.
3. Gro controls bacteria and can emit, absorb and read multiple independent
   signal channels with stable identity and declared coefficient validation.
4. Runtime, event-calendar and GUI snapshots describe the same colony state;
   viewer stepping has no hidden second clock or alternate physics.
5. Each model designated supported saves and loads; the three current
   examples are investigated without editing/re-saving a fixture to make a
   test pass.
6. Focused and required unit, kernel, smoke, GUI and CI regressions pass;
   any disabled or unrelated failures are separately identified, not called
   passed.
7. The signal relaxation's mathematical contract, coefficient domain and
   scientific limitations are explicit. Alternative A is approved for this
   first version; B/C remain outside its scope.

These criteria do not assert predictive biological validity. Any later
scientific claim requires its own reference-backed validation package.

### 22.5 Approved nine-cycle execution sequence

The following sequence is the active plan approved on 2026-10-09. Historical
proposal text that follows it is retained as provenance only and is
superseded where its cycle grouping or stop gates differ. Each cycle has a
bounded commit/evidence checkpoint; completion of a cycle is not a reason to
stop the mission.

#### Cycle 1 — Close signals

- **Objective/files:** validate finite `kdiff`/`kdeg` in `[0,1]` for Gro and
  `BacteriaSignalGrid`; preserve the approved grid precedence, independent
  channels and equation; document A as phenomenological. Touch signal grid,
  Gro runtime, focused tests and this plan.
- **Tests/exit:** endpoints, intermediates, negative/>1, NaN/Inf, no partial
  channel mutation, Phase 3 precedence/identity and small-grid
  characterizations pass; invalid values have explicit diagnostics.
- **Dependencies/risks:** none. Do not alter operator mathematics, `.gen`
  fields or serializer. Stop only if a supported tracked model depends on an
  invalid coefficient.

#### Cycle 2 — Coordinates and geometry

- **Objective/files:** preserve continuous Gro seed coordinates, define one
  coordinate origin and deterministic cell mapping, and keep rendered
  geometry derived from coherent center/length/width/orientation state.
- **Tests/exit:** fractional, negative, boundary and signal-sampling cases
  prove continuous position survives seed → step → render and all grid
  access remains in bounds.
- **Dependencies/risks:** Cycle 1. Existing seed layouts may shift; stop if
  origin/boundary policy cannot be inferred from the approved centered-grid
  convention.

**Cycle 2 result (2026-10-09): COMPLETE.** Gro seed `x,y` are retained as
continuous doubles. The grid origin is centered: coordinate `x` maps to
`round(x + (width - 1)/2)` and is clamped to the grid; the inverse cell-center
mapping is `index - (width - 1)/2`. Automatically sized grids cover both
positive and negative seed coordinates without shifting the stored positions.
`set_signal`, bacterium sampling, seed placement and boundary reflection use
the same convention. Fractional/negative seeds, explicit-grid sampling,
automatic dimensions, and reset/respawn behavior are covered. This is a
coordinate convention, not a claim of physical units or calibrated cell size.

RED/GREEN evidence: the new coordinate/sampling tests failed against the
previous shifted/rounded implementation; after the change, the focused
coordinate, seed-initialization, named-program and signal-aware tests passed,
and all 88 `RuntimePluginManagerClassTest` CTest cases passed locally.
Commits: `ef949faf` (runtime) and `149ae1a8` (test expectation updates);
the coordinate regression tests were introduced in `0f64fa8e`. No `.gen`
fixture, serializer, or persisted format changed. Test expectations that
previously treated seed `y` as a raw grid index now assert its continuous
position and the corresponding centered sampling result.

#### Cycle 3 — Growth invariants

- **Objective/files:** remove artificial growth floors in `BacteriaColony`
  and keep volume and visual length coherent.
- **Tests/exit:** explicit zero growth and zero elapsed step produce zero
  volume delta; positive configured growth remains visible; existing reset
  and lifecycle tests pass.
- **Dependencies/risks:** Cycle 2 for geometric sizing. Do not introduce a
  new biological growth law; stop if existing supported fixtures encode a
  conflicting growth contract.

**Cycle 3 result (2026-10-09): COMPLETE.** An explicitly configured
`ecoli_growth_rate <= 0` now disables automatic volume growth, including
generation/signal additions and the old minimum-volume increment. A finite
simulation step of zero is accepted by `GroProgramRuntime` for direct runtime
execution; the automatic growth path returns without changing volume, size or
speed. The `BacteriaColony` model check still requires a positive step for
scheduled simulation, so this does not make zero-step models calendar-valid.
Implicit/default and explicitly positive growth retain the prior formula and
existing visible-growth tests pass.

RED/GREEN evidence: new regressions first observed a `0.01` volume increment
at explicit zero growth and runtime rejection at `dt=0`; after correction,
both zero-invariant tests and the existing positive-growth test passed. All
90 `RuntimePluginManagerClassTest` CTest cases passed locally. Commits:
`57cdc53a` (runtime) and `0e20d035` (tests). The relaxation field remains a
per-execution operator, independent of this growth guard. No persistence
format or `.gen` fixture changed.

#### Cycle 4 — Hybrid division

- **Objective/files:** add persisted automatic-division enable/threshold
  properties (disabled by default), retain Gro `divide()`, and route both
  through one division routine.
- **Tests/exit:** automatic/manual/simultaneous triggers, same-step
  de-duplication, no reprocessing daughters, volume conservation, inherited
  state/program/genealogy, and boundary placement pass without altering
  `grow(n)` semantics.
- **Dependencies/risks:** Cycles 2–3. Stop if correct persistence requires an
  incompatible `.gen` migration.

**Cycle 4 result (2026-10-09): COMPLETE.** `BacteriaColony` now exposes and
persists `AutomaticDivisionEnabled` (default `false`) and
`DivisionThresholdVolume` (default `2.0`, twice the default initial volume).
Gro and threshold-triggered division use one routine. It halves the finite,
nonnegative parent volume without volume floors, derives both visual sizes
from volume, places the pair close along their shared orientation, and copies
the program, arguments and bacterium-local variables to the daughter. The
fluorescent markers retain the pre-existing 0.85 daughter scale and the
daughter tick count resets. The existing `grow(n)` branch
still creates its existing population additions and does not call the
division routine.

One division per bacterium per step is enforced and repeated Gro division
commands receive a diagnostic. A step snapshot prevents a new daughter from
running its program in its birth step, while `just_divided`/`daughter` remain
visible on its first subsequent program execution. Automatic and Gro division
cannot both split the same parent in one step. The aggregate `main()` path
also retains the population target produced by Gro rather than overwriting it
with the old pre-command count.

RED/GREEN evidence: the automatic-threshold regression first remained at one
bacterium when division was enabled; after implementation, the focused
division suite passed. Tests cover the default-off property, threshold
activation, persisted property round-trip, Gro plus automatic same-step
deduplication, duplicate Gro-command diagnostics, new-daughter step
exclusion, aggregate and bacterium-scoped volume conservation, genealogy and
the existing division-marker/signal behavior. All 96
`RuntimePluginManagerClassTest` CTest cases passed locally. Commits:
`cc6b4417` (implementation), `ee896800` (tests) and `086bbfe8` (division
marker lifecycle correction). No serializer or `.gen` fixture was modified;
the new fields are additive and absent values load from class defaults.

**Checkpoint A (2026-10-09):** Cycles 1–4 complete. Runtime coefficient
validation, centered continuous seed mapping, zero-growth invariants, and
optional hybrid division are implemented. The feature-branch code/test head
is `086bbfe8`; this checkpoint's plan update follows as a documentation-only
commit. Focused local regression: 96/96 `RuntimePluginManagerClassTest`
passed, including signal precedence and channel identity, the two supported
Growth/Lifecycle `.gen` fixture loads, and the added division cases. The
previous cycle commits are `44db7a96`, `efda4eba`, `4d6bdce4`, `ef949faf`,
`149ae1a8`, `0d3f8e57`, `57cdc53a`, `0e20d035`, `b0617be1`, `cc6b4417`,
`ee896800` and `086bbfe8` (plus task/backlog and earlier synchronized
history). The GUI and the full CMake test presets are not part of this
checkpoint's evidence; they remain scheduled for later cycles. The known
`Smart_BacteriaColony_GRO.gen` loader failure remains unresolved and is not
claimed compatible. Continue automatically with Cycle 5.

#### Cycle 5 — Motion and spatial occupancy

- **Objective/files:** implement `x += speed*cos(direction)*dt`,
  `y += speed*sin(direction)*dt`; define `run`/`tumble` using kernel RNG;
  add cheap deterministic limited position correction for oriented bodies.
- **Tests/exit:** zero speed/step invariants, positive kinematics, seeded
  reproducibility, boundary reflection and multiple-body partial-overlap
  handling pass; one isolated stationary bacterium receives no correction.
- **Dependencies/risks:** Cycles 2–4. No forces, torques, external engine or
  exact rigid-body guarantee. Stop if the real kernel RNG contract cannot
  provide deterministic reset behavior.

**Cycle 5 result (2026-10-09): COMPLETE.** `BacteriaColony` now applies
continuous kinematics as `x += speed*cos(direction)*dt` and
`y += speed*sin(direction)*dt`; non-finite/negative speed and non-finite or
negative dt are treated as zero. The previous minimum speed and minimum step
motion fallbacks were removed. A single boundary crossing reflects the
remaining displacement and changes orientation; the position is then kept
inside the centered grid. This is a bounded kinematic rule, not a wall-force
model.

`tumble(angle)` obtains its sign from the model's existing `unif(0,1)` parser
sampler through `Model::parseExpression`; successive tumbles compose from
the sampled orientation. The standalone `GroProgramRuntime` remains usable
and deterministic when executed without a `Model`; a `BacteriaColony` uses
the kernel sampler for actual motion. `run(v)` sets speed to `v` while
preserving heading, and `tumble(v)` preserves speed. Growth changes size and
volume but no longer introduces movement speed. The colony applies a deterministic single
pair pass using the support radii of oriented rectangular bodies. Each pair
correction is capped at 0.1 coordinate units and removes only a fraction of
the estimated overlap; it does not model forces or guarantee non-overlap.
There is no correction for an isolated cell. The O(n²) pass is not yet
benchmarked at representative large populations; assess this at Cycle 9
before considering a spatial index.

Evidence: `cmake --build --preset tests-unit -j4` passed; focused motion
cases passed, including exact positive `speed*dt`, zero speed, zero step,
boundary reflection, repeated-model RNG reproducibility and bounded
correction, alongside existing `run`/`tumble` and visible-motion tests. The
runtime tests now assert the §12 contract directly: `run(v)` sets speed to
`v`; a tumble changes direction and preserves speed. The prior runtime test
only checked that run increased speed and therefore did not distinguish the
approved semantics. The complete local
`RuntimePluginManagerClassTest` CTest group passed 101/101. Its first run
exposed an old exact-position expectation in
`BacteriaColonyMainCanResetAndRespawnSeeds`; the fixture was made explicit
about zero speed and zero growth so it continues to test reset/respawn rather
than relying on the removed implicit movement fallback. No production
expectation was weakened. The suite also passed after that semantic fixture
adjustment. Production and test commits are `3b3a61aa` and `4228cee2`.
These are local results, not CI evidence.

#### Cycle 6 — Event-calendar integration

- **Objective/files:** establish exactly-once ordering for Gro, bacteria,
  every signal field and event dispatch using the existing colony state.
- **Tests/exit:** signal emission/read/relaxation order, one field update per
  step, replication reset, event time advancement and manual viewer-step
  behavior are asserted.
- **Dependencies/risks:** Cycles 1–5. If concurrent cross-thread mutation is
  possible without a defined synchronization contract, stop that dependent
  path rather than adding ad hoc locking.

**Cycle 6 result (2026-10-09): COMPLETE.** No separate clock or duplicate
field update was needed. The focused calendar regression now runs a
`BacteriaColony` through `Create` and `Dispose` for two replications: two
events at model times 0 and 0.25 each execute the named `main`/`bacterium`
program and relax the attached one-cell field once (8 -> 4 -> 2); the next
replication starts again from the persisted initial value. Runtime execution
count, final event time 0.25, and reset field value are asserted. A direct
manual `executeGroProgram()` assertion verifies one field relaxation and
unchanged model time; the Qt viewer already reports this same contract.

The viewer now refuses its manual Step/Run action while the model simulation
is running or paused, and stops its timer with an explanatory status. The
normal GUI simulation entry point runs synchronously on the UI thread, so
there is no supported simultaneous thread path to serialize here; an
off-thread embedding still has no new locking contract. The GUI target build
will be recorded with Cycle 7/9 validation.

#### Cycle 7 — Viewer and communication demonstration

- **Objective/files:** add channel selection and one-at-a-time heatmap to the
  existing Qt6 viewer; keep bacteria overlaid and channel lists correct
  across declaration/reset; author a small native two-signal scenario.
- **Tests/exit:** GUI build and applicable adapter/interaction checks prove
  selected-field fidelity plus emit/read/degrade independence while the
  viewer uses the same runtime state.
- **Dependencies/risks:** Cycles 2–6. Do not create another renderer, clock
  or physics model.

#### Cycle 8 — Persistence and complete demonstration

- **Objective/files:** validate supported model save/load and complete
  scenario; investigate `Smart_BacteriaColony_GRO.gen` causally without
  re-saving or modifying historical fixtures.
- **Tests/exit:** Growth, Lifecycle and Smart fixture outcomes are recorded;
  format stays compatible; new persisted division properties round-trip; the
  demo reloads and executes.
- **Dependencies/risks:** Cycles 2–7. If the Smart load issue needs format or
  semantic migration, document exact cause/options and stop that fix only.

#### Cycle 9 — Regression and delivery

- **Objective/files:** close acceptance criteria, affected manuals, plan,
  backlog and dated evidence; measure performance only if needed.
- **Tests/exit:** focused tests, `tests-unit`, `tests-kernel-unit`,
  `tests-smoke`, `gui-app`, persistence and applicable CI are reported with
  exact local/CI provenance and unrelated failures separated.
- **Dependencies/risks:** Cycles 1–8. Do not label complete with a required
  failed or unrun gate; no speculative optimization.

### 22.6 Superseded proposal sequence (historical)

The following nine-iteration proposal predates the maintainer's 2026-10-09
approval. It remains as historical provenance only; Cycle 1–9 above control
the active work. Reuse existing tests and keep changes reviewable.

#### Iteration 1 — Ratify the M1 signal contract

- **Objective:** obtain maintainer approval for Alternative A, its
  dimensionless per-update interpretation, the finite `[0,1]` parameter
  domain and the exact diagnostic behavior for invalid values.
- **Files/modules:** this plan; the scientific-decision record/backlog only
  if the maintainer requests that canonical update. No production source.
- **Tests:** rerun the existing Phase 4 small-grid characterization and
  Phase 3 signal precedence/identity tests as decision evidence; do not alter
  their expectations.
- **Exit:** decision is recorded explicitly; otherwise remain at this gate.
- **Dependencies:** none.
- **Risks:** current Gro programs outside the tracked supported subset may
  pass out-of-range values; do not infer compatibility from untracked
  original examples.
- **Stop:** no implementation of coefficient validation or scientific
  interpretation change without approval.

#### Iteration 2 — Validate signal numeric parameters and document M1

- **Objective:** after Iteration 1 approval, reject non-finite/out-of-domain
  values consistently for `BacteriaSignalGrid` and Gro `signal(kdiff,kdeg)`;
  keep the equation and operator ordering unchanged.
- **Files/modules:** `BacteriaSignalGrid.cpp`, signal declaration validation
  in `GroProgramRuntime`/`BacteriaColony`, existing
  `test_runtime_pluginmanager.cpp`, §10.
- **Tests:** finite endpoints 0/1, intermediate values, negative and >1
  values, NaN/infinity paths where representable; unchanged 1×1/2×2/3×3
  numeric characterizations; channel precedence/identity regressions.
- **Exit:** invalid inputs fail diagnostically before mutating channel
  metadata; all current selected-program tests remain green.
- **Dependencies:** approved Iteration 1 decision.
- **Risks:** avoid partial validation where Grid accepts a value Gro rejects
  or vice versa; no changes to `.gen` field names or format.
- **Stop:** if any tracked model uses an out-of-domain value, establish
  whether it is supported before rejecting it; do not silently clamp.

#### Iteration 3 — Continuous coordinates and signal-grid mapping

- **Objective:** preserve continuous Gro seed coordinates and define their
  mapping to discrete signal cells consistently with signal set/sample
  operations.
- **Files/modules:** `BacteriaColony.cpp/.h` seed parsing, position
  synchronization and grid-index helpers; focused colony tests.
- **Tests:** non-integer and negative/centered seeds, edge mapping,
  repeatable signal sampling, persistence of supported model coordinates.
- **Exit:** position remains continuous through seed → execute → render;
  field access is deterministic and in bounds.
- **Dependencies:** M1 completion; plan approval.
- **Risks:** old seed-shift/round behavior may change initial layouts.
- **Stop:** if coordinate origin or out-of-domain seed policy is ambiguous,
  request maintainer choice before changing visible model placement.

#### Iteration 4 — Growth, division and death invariants

- **Objective:** remove growth-floor behavior for zero growth, retain the
  approved parent/daughter division behavior, and make death/update ordering
  explicit without broad physiology changes.
- **Files/modules:** `BacteriaColony` growth/population mutation code and
  focused unit tests.
- **Tests:** zero and positive growth, volume non-negativity, division
  partition/flags, death removal, replication/reset and existing lifecycle
  tests.
- **Exit:** zero input growth yields zero volume increase; selected division
  and death paths have asserted state outcomes and no lifecycle regression.
- **Dependencies:** Iteration 3 coordinate contract where positions are
  split or reseeded.
- **Risks:** growth units and the existing heuristic may be under-specified;
  do not add a biological growth law.
- **Stop:** if preserving division behavior conflicts with an established
  model fixture, stop for a scoped compatibility decision.

#### Iteration 5 — Kinematic motion, orientation and reproducible RNG

- **Objective:** implement the existing planned simple 2D kinematic
  contract; make zero speed and zero elapsed step stationary; define
  `run`/`tumble` with GenESyS sampler reproducibility.
- **Files/modules:** `BacteriaColony` motion/`run`/`tumble` dispatch,
  `GroProgramRuntime` if mutation plumbing is needed, focused tests.
- **Tests:** zero speed, zero step, positive displacement against
  `cos/sin`, boundary reflection, seeded repeatability/reset of RNG and
  orientation range.
- **Exit:** same seed/reset yields the same motion; no movement at either
  zero invariant; the boundary rule remains explicit.
- **Dependencies:** Iterations 3–4; approved kinematic contract in §12.
- **Risks:** `simulationStep` and colony time-unit conversion may not be
  identical in every dispatch path.
- **Stop:** stop if a separate time-domain decision is needed; do not infer
  physical speed units.

#### Iteration 6 — Event-calendar and one-step state contract

- **Objective:** demonstrate one coherent state transition per colony
  update across Gro, bacteria, signal fields and event scheduling.
- **Files/modules:** `BacteriaColony` dispatch/update path, viewer step
  request boundary if needed, integration tests.
- **Tests:** ordering of signal emission/read/relaxation; exactly-once field
  update; replication reset; event-calendar time advances once; manual viewer
  step reports unchanged model time.
- **Exit:** tests establish which state is visible within a step and after
  the event; no duplicate signal or clock advancement.
- **Dependencies:** Iterations 2–5.
- **Risks:** viewer manual execution and active event replay may contend for
  the same instance.
- **Stop:** if cross-thread execution is possible and synchronization
  semantics are not specified, stop rather than add ad hoc locking.

#### Iteration 7 — Viewer fidelity and minimum corpus A–E

- **Objective:** close only missing end-to-end assertions, reusing existing
  tests and ensuring Qt6 viewer snapshots mirror the colony runtime.
- **Files/modules:** `BacteriaColonyViewerGuiExtensionPlugin.cpp`, existing
  runtime/viewer tests, selected model fixtures only if authored from
  scratch is necessary.
- **Tests:** minimal end-to-end coverage for A growth/division/death, B
  independent signals, C movement, D Gro-controlled bacterium and E viewer
  state/time/selection; avoid duplicating focused unit oracles.
- **Exit:** corpus A–E criteria each point to passing focused or end-to-end
  evidence; viewer displays actual state and does not advance a second clock.
- **Dependencies:** Iterations 3–6.
- **Risks:** GUI adapter tests may establish rendering-state fidelity but
  not full interactive usability; report that boundary.
- **Stop:** do not build a second physics engine or event scheduler to ease
  viewer testing.

#### Iteration 8 — Persistence and the known model-loader failure

- **Objective:** validate save/load of supported models and diagnose
  `Smart_BacteriaColony_GRO.gen` on the current branch, fixing only a
  demonstrated loader defect within the accepted format.
- **Files/modules:** current serializer/model loader only if causally
  implicated, BacteriaColony/Gro references, persistence tests; do not edit
  or re-save fixture files to make tests pass.
- **Tests:** round-trip Gro source, signal grid properties, colony
  references and execution; all three tracked examples; byte-level
  verification that fixtures remain unchanged.
- **Exit:** supported fixtures load and execute after load; any fixture
  excluded from support has an explicit maintainer-approved classification.
- **Dependencies:** iterations that change persisted behavior, especially
  3–7.
- **Risks:** compatibility fix could widen into serializer migration.
- **Stop:** if the issue requires a `.gen` format or semantic migration,
  present options and stop before changing the serializer or fixture.

#### Iteration 9 — Final regression, CI, manuals and conditional performance

- **Objective:** establish final-head evidence and close delivery records.
- **Files/modules:** tests/presets and workflows only for demonstrated gaps;
  this plan, canonical status/backlog/changelog as applicable, affected
  Developer/User manual only if the approved implementation changes their
  contract.
- **Tests:** focused A–E tests, `tests-unit`, `tests-kernel-unit`,
  `tests-smoke`, `gui-app`, applicable CI and persistence checks; record
  exact enabled/disabled/failing counts and branch/commit provenance.
- **Exit:** every criterion in §22.4 has executed evidence, risks are
  listed, and only then propose the first-version completion state.
- **Dependencies:** M1–M3 and Iteration 8.
- **Risks:** stale CTest inventory, pre-existing unrelated failures,
  environment/CI divergence; distinguish local evidence from CI.
- **Stop:** do not label complete with a failed/unrun required gate. Profile
  the current reparse-per-call path; optimize only if a representative
  benchmark proves it is material. If not, document it as a known cost and
  defer it.

**Historical documentation/manual impact assessment:** this statement
applied only to the earlier plan-reorganization proposal. It does not apply
to the approved runtime work. Cycle 2 establishes a centered continuous
coordinate convention; Cycle 9 must assess the User and Developer Manuals
against the implemented controls and document any user-visible behavior
that needs to be made explicit.

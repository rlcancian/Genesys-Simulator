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
modular, testable and persistable state, with documented semantics derived
from the original Gro source and its example corpus.

This is a **continuation**, not a from-scratch implementation. A substantial
integration already exists under
`source/plugins/data/BiochemicalSimulation/Gro*` and
`source/plugins/components/BiochemicalSimulation/BacteriaColony.*`, with a
GUI viewer and 46+ focused unit tests. The goal is not to compile the
original `gro` application inside GenESyS; it is to give the existing
GenESyS-native language frontend/runtime a documented, verified subset of
Gro syntax and semantics, with explicit, tested deviations where full
fidelity is not adopted.

Non-goals (unless a later maintainer decision changes them): Chipmunk or any
external 2D physics engine; a full academic implementation of the Gro
grammar (`fun`, `let/in/end`, lambdas, general list/record values,
`foreach`/`cross`/`maptocells ... end`); FEM/PDE solvers; arbitrary
filesystem access via `fopen`/`fprint`; GUI-only builtins inside the
headless runtime.

Working branch: `WiP20261008/BacteriaColony`, based on `WorkInProgress` at
`41419bcc75e232387d307053369e5e495a194e3a`.

## 2. Original Gro reference baseline

Reference checkout: `~/Repositories/bacteria_programming_language`
(`rlcancian/bacteria_programming_language`, local HEAD `d3ef577`, branch
`master` tracking `origin/master`). Not modified by this work.

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
  cells), *before* diffusion in `World::update()`.
- **`World::update()` pipeline order** (confirmed exactly):
  `prog->world_update()` → per-cell `cell->update()` (growth, Gro program
  step) then `cell->divide()` (adds daughter if produced) → reactions over
  the whole signal grid → diffusion/degradation (`signal->integrate(dt)`)
  for every signal → death removal (`marked_for_death`) → chemostat flow
  force + out-of-bounds removal → 3× `cpSpaceStep` (physics) → `t += dt`.
  The whole `update()` is skipped (with a one-shot warning and
  `stop_flag=true`) if `population->size() >= population_max`.
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
  (false)`) in that same read, i.e. visible for exactly one step.
- **Movement** (`run`/`tumble`, `Programs.cpp`): both operate directly on
  the Chipmunk `cpBody` (`cpBodyApplyForce`/`cpBodySetTorque`), damping the
  orthogonal component; there is no GenESyS-equivalent custom integrator in
  the original — physics is entirely delegated to Chipmunk, stepped 3×
  per `World::update()`.
- **Barriers/chemostat**: `barrier(...)` both registers a real Chipmunk
  static segment shape (`cpSpaceAddShape`) *and* records it in
  `World::barriers` for rendering; `chemostat(bool)` toggles
  `chemostat_mode`, which both adds four static Chipmunk wall segments in
  `World::init()` and applies a one-directional flow force
  (`chemostat_flow`) plus out-of-bounds cell removal every step. Both have
  real physical effect in the original.
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
  §7 for the per-example compatibility matrix against the *current*
  GenESyS implementation.

## 3. Licensing/provenance boundary

`~/Repositories/bacteria_programming_language/LICENSE.md`: UW Open Source
License (noncommercial, requires attribution, requires derivative works to
be clearly marked and distributed under a license prohibiting commercial
use, requires the license text to accompany copies).

Rule applied in this work: **no direct code copying** from the reference
repository into GenESyS. Semantics (grammar shapes, builtin signatures,
the diffusion/degradation stencil, the growth/division formulation, the
`World::update()` pipeline order) are treated as a *behavioral contract* to
reimplement independently in GenESyS's own C++ style and test
infrastructure, not as source to port verbatim. No `.gro` example file is
copied into the GenESyS tree as a fixture; GenESyS-authored fixtures use
equivalent syntax written from scratch for the specific construct under
test. If any future step appears to require incorporating UW source
verbatim, this plan requires stopping and presenting the maintainer with
the exact code, the reason, a copy-free alternative, and the relevant
license obligations (governance §18, stop gate 1) before proceeding.

## 4. Current GenESyS implementation inventory

Confirmed by full-file inspection (`a2e1719b` investigation, 2026-10-08,
read-only, no production changes):

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

Dependency note: `WiP2026108/PE_Fix` (PR #545, draft, open at inspection
time) already touches `GenSerializer.cpp`, `ObjectPropertyBrowser.cpp` and
`ModelLanguageSynchronizer.cpp`, including a test explicitly named "cover
real GroProgram model round trip". This work treats PE_Fix as an external
dependency for Phase 11 (persistence) and does not duplicate its scope
while it remains in draft.

## 5. Language compatibility matrix

Status values: `supported`, `partial`, `missing`, `broken` (reaches the
parser/compiler but produces wrong structure), `unknown` (not yet executed
against real input).

| Construct | Status | Evidence |
|---|---|---|
| `//` and `/* */` comments | supported | `GroProgramParser::skipWhitespaceAndComments` |
| String literals (balance/skip only) | supported | string-aware scanning in `findMatchingDelimiter`/`parse` |
| Balanced `()`, `[]`, `{}` | supported | `GroProgramParser::findMatchingDelimiter` |
| `program name(params) := { ... };` | supported | `tryParseNamedProgram` |
| `program name(params) := <single-stmt>;` (no braces) | supported | same method, brace-less branch |
| `program name(params)` as a first-class **expression/value** | missing as a dedicated construct; works operationally only through textual name resolution inside `ecoli(...)`'s second raw argument | parser has no AST node for it |
| `condition : { actions }` rule | supported (compiles to `IfStatement`, no `else`) | `GroProgramCompiler::tryCompileRuleStatement` |
| `:=` assignment | supported | `findTopLevelAssignment` |
| `=` as comparison vs. legacy assignment | partial (heuristic) | `GroProgramCompiler.cpp` disambiguation |
| `if (cond) { } else { }` (C-style statement) | supported | `tryCompileIfStatement` |
| `if EXPR then EXPR else EXPR end` (Gro expression form, used by `fun`) | missing | no `then`/`end` keyword handling anywhere |
| Records `[ field := value, ... ]` | partial — only as direct RHS of `x := [...]`, flattened to `x.field := value`; read access to a record field **inside an expression/condition** (`p.mode = GO`) is unconfirmed/unknown | `compileSimpleStatement` |
| Lists `{ a, b, c }` as a value | missing | preserved only as raw text inside arguments |
| Indexing `a[i]` | missing | no handling outside record/parameter brackets |
| `sharing a, b, c` | parsed only to strip text; **no real scoping** — all composed sub-programs share one flat global variable map, so `sharing` is a no-op rather than selective | `stripSharingClause`, `compileNamedProgramBody` |
| `needs a, b;` | broken — `needs q, t;` is split into two bogus statements `"needs q"` and `"t"` because the top-level statement splitter cuts on both `;` and `,` | `consumeSimpleStatement` |
| `fun name args . body;` | missing | zero occurrences of `fun` handling |
| `foreach v in EXPR do ... end` | missing | zero occurrences |
| `range`, `cross` | missing as constructs (identifiers only, inert) | zero occurrences |
| `let ... in ... end` | missing | zero occurrences |
| `map`, lambda `\x.expr` | missing | zero occurrences |
| `maptocells EXPR end` (original keyword form) | missing; GenESyS only has `map_to_cells(EXPR)` as an ordinary function call | `GroProgramRuntime.cpp` |
| `@` (cons), `#` (concat) | missing | not present anywhere |
| `rate(k)` | supported, as an ordinary function call (not a language keyword) | `GroProgramRuntime::parseFunctionCall` |
| `&`, `|`, `!` logical operators | supported | `NumericExpressionParser` |
| `<>` string concatenation | unknown (not exercised by the current fork-level inspection; needs a focused probe) | — |
| `<<` record-override operator | unknown, used only in theme records in examples (GUI-only path); not exercised | — |
| Program composition `A() + B() sharing ...` | supported via static textual inlining (not independent `Program`/`SymbolTable` objects) | compiler composition path |

A not-raw (`RawStatement`) outcome is produced for every supported
construct; everything else silently becomes a `RawStatement` with **no
diagnostic**, confirmed at `GroProgramCompiler::compileSimpleStatement`.
This is the single most load-bearing gap for Phase 1: unsupported syntax is
currently indistinguishable from a deliberate no-op statement.

## 6. Builtin compatibility matrix

Status values follow governance evidence discipline: `equivalent`,
`partial`, `different-semantics`, `missing`, `intentionally-unsupported`,
`gui-only`, `unsafe/deferred`, `extension` (GenESyS-only, no Gro
equivalent).

| Gro builtin | GenESyS status | Current GenESyS behavior (confirmed in code) |
|---|---|---|
| `signal(kdiff,kdeg)` | different-semantics | Returns `arguments.front()` verbatim; creates no channel, no handle (`GroProgramRuntime.cpp:455-462`) |
| `get_signal(n)` | different-semantics | Ignores `n`; always returns `local_signal` from the single colony-wide field (`:446-453`) |
| `emit_signal`/`absorb_signal`/`consume_signal` | different-semantics | Only the last argument (value) is used; a leading handle argument is silently dropped (`:1000-1020`) |
| `set_signal(n,x,y,v)` / `set_signal_rect` | partial | All arguments including `n` are preserved into `ColonyMutation.numericArguments`, but `BacteriaColony` has only one field, so `n` is effectively unused downstream |
| `reaction(reactants,products,rate)` | missing | Zero occurrences in `BiochemicalSimulation/` |
| `get_signal_matrix(n)` | different-semantics | Current signature requires **zero** arguments; call with an argument errors (`:914-926`) |
| `ecoli([...], program p())` | partial | Structurally equivalent (2 args, record + program reference); see §10 for the coordinate-handling gap |
| `die()` / `die(n)` | extension + different-semantics | GenESyS accepts an optional `amount` argument (original `die()` takes none and only marks the current cell) |
| `divide()` | different-semantics | Outside bacterium-scoped mode, doubles the whole aggregate population instead of splitting one cell |
| `run(v)` / `tumble(v)` | different-semantics | Synthetic scalar formulas (clamped velocity add / random-angle jitter); no physics engine involved at all |
| `geometry()` | missing | Not in dispatch table |
| `time()` | different-semantics (worse than missing) | Explicitly rejected by `parseFunctionCall` with a hard error if used in an expression |
| `stats(...)` | missing | Not in dispatch table |
| `message(n, text)` | partial | Quadrant argument `n` accepted but discarded; stored as a flat message string |
| `clear_messages(n)` | missing | Not in dispatch table |
| `set(name, value)` | equivalent (structurally) | Special-cased `dt`/`signal_grid_width`/`signal_grid_height`; everything else goes to a generic variable map |
| `barrier(x1,y1,x2,y2)` | partial | Recorded in `_barriers`, exposed to the viewer; **no physical effect** (confirmed: no motion code reads `_barriers`) |
| `chemostat(bool)` | partial | `_chemostatMode` stored; **no physical effect** (confirmed: nothing reads it outside the setter/getter) |
| `reset()` | partial | Clears variables/population/tick count; see §11 for colony-level effect |
| `stop()` / `start()` | missing | Not in dispatch table |
| `print(...)` / `clear()` | missing | Not in dispatch table |
| `zoom(...)` / `set_theme(...)` / `snapshot(...)` | gui-only, currently missing even as a documented no-op | Not in dispatch table; headless runtime correctly has no Qt dependency, but there is no explicit "GUI-only, intentionally inert" diagnostic either |
| `fopen` / `fprint` / `dump` | unsafe/deferred, correctly absent | Governance §15 requires an explicit security decision before any file I/O surface; current absence is the correct state, not yet documented as intentional |
| `map_to_cells(expr)` | partial | Implemented as an ordinary function call; the original's dedicated `maptocells EXPR end` keyword syntax is not recognized |
| `tick()`, `grow(n)`, `set_population(n)`, `dump_signal_field(r,c)` | extension | GenESyS-only commands with no Gro-original equivalent |

## 7. Original example compatibility matrix

All 23 files under `~/Repositories/bacteria_programming_language/examples/`
were read in full and cross-checked against §5/§6. No example has been
executed yet against the current GenESyS parser/compiler/runtime — all
"likely" verdicts below are **strong indications from static reading**,
not executed evidence, and must be confirmed with real fixtures in Phase 1.

| Example | Status | First real blocker |
|---|---|---|
| `growth.gro` | untested (no confirmed-missing construct) | unexecuted; `tostring` support unconfirmed |
| `signal_demo.gro` | unsupported | top-level and in-program `foreach ... in (range n) do ... end` |
| `signal_grid.gro` | unsupported | `foreach p in cross (range 16) (range 16) do ... end`, nested list literal, `bitmap[p[1]][p[0]]` indexing |
| `morphogenesis.gro` | unsupported | `fun`, `if...then...else...end` expression, curried call `gr(a)(b)`, broken `needs q, t;` |
| `chemotaxis.gro` | unsupported | top-level `foreach q in range 50 do ecoli(...) end` |
| `barriers.gro` | untested (likely compiles) | barriers are inert at runtime (semantic gap, not a parse blocker) |
| `foreach.gro` | unsupported | top-level `foreach q in range 100 do ecoli(...) end` |
| `maptocells.gro` | unsupported | `fun`, `let ... in ... end`, lambda `\x.expr`, `maptocells EXPR end` keyword form |
| `bandpass.gro` | unsupported | `fun f a . ...` |
| `coupled_oscillator.gro` | partial/untested | record-field read in a condition (`p.mode = GO`) unconfirmed; `snapshot` missing |
| `dilution.gro` | unsupported | broken `needs gfp;` path, nested parenthesized composition untested |
| `edge.gro` | partial/untested | record-field read in a condition unconfirmed; `emit_signal` channel silently discarded (semantic, not parse) |
| `game.gro` | unsupported | `stats(...)`, `stop()` missing |
| `geometry.gro` | unsupported | `fopen`/`fprint`/`geometry()` missing, `time()` hard errors, list literal + `@` |
| `gfp.gro` | unsupported | `needs gfp, mRNA;` (comma-separated `needs` is worse-broken) |
| `inducer.gro` | unsupported (closest near-miss) | `clear_messages(1)` missing |
| `signal_dump.gro` | unsupported | `reaction(...)` missing, `<<` record override, `foreach`, `fopen`/`fprint` |
| `skin.gro` | unsupported | `<<` record override, `if...then...else...end` expression, `start()` missing |
| `spatial_oscillations.gro` | partial/untested (near-miss) | record-field read in a condition unconfirmed |
| `spots.gro` | unsupported | list literal `nutrient := { signal(...), signal(...) }` |
| `symbiosis.gro` | partial/untested (near-miss) | record-field read in a condition unconfirmed |
| `wave.gro` | unsupported | `reaction(...)` missing, `<<`, `foreach`, lists |
| `yeast_example.gro` | not-applicable | `yeast()` is disabled in the *original* Gro too; out of scope here |

Observation for Phase 1 prioritization: the single open question "is a
record field readable inside a boolean condition/expression, not just
assignable as a flattened LHS" gates `coupled_oscillator`, `edge`,
`spatial_oscillations` and `symbiosis` simultaneously, and the `needs`
comma-splitting bug gates `morphogenesis`, `dilution` and `gfp`. These are
high-leverage, low-risk fixes to resolve early.

## 8. Signal / reaction-diffusion contract

Current: one unnamed scalar field per colony (`BacteriaSignalGrid`), 4
-neighbor (von Neumann) relaxation, no explicit `dt`
(`BacteriaColony.cpp:1183-1228`):
```
relaxed = current + diffusionRate * (neighborAverage - current)
updated = max(0, relaxed * (1 - decayRate))
```
Boundary cells are updated with a smaller neighbor count (not frozen, not
reflected, not periodic). No multi-channel addressing exists: `signal()`
does not create anything, and `get_signal`/`emit_signal`/`absorb_signal`
ignore any handle argument.

Target contract (Phase 3, not yet implemented): `signal(kdiff,kdeg)` must
create or reference a real, independently addressable channel and return a
stable handle; `get_signal`/`emit_signal`/`absorb_signal`/`set_signal`/
`set_signal_rect`/`get_signal_matrix` must operate on the channel named by
that handle. The GenESyS discretization does not have to copy the original
8-neighbor/`dt`-scaled stencil verbatim, but any change to the current
4-neighbor/no-`dt` formulation is a behavior change for the existing three
`.gen` fixtures and must be treated as such (documented impact + migration
note), not a silent fix.

## 9. Bacterial growth/division contract

Confirmed defect (`BacteriaColony.cpp:2144-2170`,
`_applyBacteriumGrowth()`): `deltaVolume = clamp(growthRate * stepScale,
0.01, 0.35)`. With `growthRate` driven to exactly `0.0` (explicit
`ecoli_growth_rate=0`, generation 0, no local signal contribution), the
minimum clamp of `0.01` still applies, producing deterministic non-zero
growth. This is reproducible from the formula alone (no randomness
involved) and is treated as `CONFIRMED`, not hypothesis.

Division flag timing (`just_divided`/`daughter`): confirmed functionally
equivalent to the original's one-step visibility window, though
implemented via a different set/clear sequence spread across
`BacteriaColony.cpp:1806-1809` (clear) and `:1988-2032` (set after a
`divide()` mutation in the same step) rather than the original's
clear-before-next-step-read. Classified `no bug found`, pending an explicit
regression test before Phase 5 touches this code.

## 10. 2D spatial/motion/mechanics contract

Confirmed defect (`BacteriaColony.cpp:2301-2336`,
`_updateBacteriumSpatialMotion()`): `if (!isfinite(speed) || speed <= 0.0)
speed = 0.08 + 0.01*generation;` — a deliberately-set `speed := 0` in a Gro
program is silently replaced by a positive fallback velocity, so the
bacterium keeps moving. `CONFIRMED`, not hypothesis.

Coordinate handling: two **inconsistent** conventions are confirmed in the
current code simultaneously:
- `ecoli([x:=...,y:=...])` seed coordinates are shifted to be non-negative
  (`shiftX/shiftY = -min(0, minSeedCoordinate)`) and rounded to integer
  grid cells immediately, destroying continuous position before any
  execution (`BacteriaColony.cpp:975-1020`).
- `set_signal`/`set_signal_rect` instead use `toCenteredGridIndex()`
  (`:162-172`), which assumes the grid origin is centered.
`_setBacteriumPosition()` always recomputes `gridX/gridY` via
`llround(position)`, so even though `BacteriumState.positionX/Y` are
`double`, signal sampling/emission always collapses to one rounded grid
cell — the original's 3-point sampling along the cell's heading
(`World::get_signal_value`) has no equivalent.

Barriers and chemostat are confirmed **inert** for motion: `_barriers` and
`_chemostatMode` are stored but never read by
`_updateBacteriumSpatialMotion()` or `_applyBacteriumGrowth()`.

No external physics engine (Chipmunk or otherwise) is adopted by this
plan; §10/Phase 6 of the task mandate requires presenting options to the
maintainer if an internal mechanics approach proves insufficient for a
required feature — not yet reached.

## 11. GenESyS event-time integration

Confirmed pipeline for one bacterium-scoped colony step
(`_executeBacteriumScopedGroProgram`, `BacteriaColony.cpp:1742-1847`): per
live bacterium — execute Gro program, sync spatial/growth state, clear
division flags, apply signal mutations, apply population mutations
(grow/divide/die), recompute grid position — then, once per colony step
after the full population loop: refresh update time, apply the single
signal-field diffusion/decay step, update spatial motion for every
bacterium, rebuild grid positions.

Compared to the original's `World::update()` order (world program → cells
update+divide → reactions → diffusion → death removal → chemostat →
physics → time advance): reactions have no equivalent stage at all
(because `reaction()` is unimplemented); chemostat has no motion-affecting
stage; diffusion-then-motion ordering is directionally similar to the
original's reaction/diffusion-then-physics ordering.

`executeGroProgram()` reparses and recompiles the full source text from
scratch on **every** call (`BacteriaColony.cpp:559-643`), with no cache by
hash/identity/revision — confirmed, not hypothesis. This is a Phase 10
(performance) concern, not a Phase 0 blocker, and must not be fixed by
caching until correctness phases are closed (a stale cache during active
semantic changes would be worse than the current cost).

Both the real event-calendar dispatch (`BacteriaColony::_onDispatchEvent`)
and the GUI viewer's "Step colony" button and "Start run" timer call the
exact same `executeGroProgram()` method (confirmed, §12 below); the colony
owns no separate internal clock contract beyond `ModelSimulation`'s own
time, so there is exactly one "advance simulated time" path — the risk is
two *triggers* for colony mutation, not two incompatible time domains.

## 12. GUI/viewer contract

`BacteriaColonyViewerGuiExtensionPlugin.cpp`: "Step colony" and the
"Start run" `QTimer` both call `_executeSelectedColonyStep()`, which calls
`colony->executeGroProgram()` directly (`:740-748`) — the same method the
event calendar calls via `_onDispatchEvent`. The code already contains an
explicit, correct comment (`:698-699`) stating that manual viewer
execution mutates colony state for inspection but does **not** advance
`ModelSimulation::getSimulatedTime()`. This is confirmed self-consistent:
there is one state-mutation entry point, triggered either by the event
calendar (which also advances simulated time) or by the viewer (which does
not). The risk flagged by the task mandate — a hidden second clock — is not
present; the real risk is a user running the viewer's manual
Step/Start-run controls *concurrently* with active event-calendar replay
on the same `BacteriaColony` instance, which would interleave two
uncoordinated triggers of the same mutation path. This must be verified
with a focused test in Phase 8, not assumed.

The viewer already implements colony/signal selection, heatmap rendering,
per-bacterium rendering with orientation/fluorescence, trails, a legend,
zoom, and polling via `QTimer` (no event-driven widget callback contract
exists yet in the generic GUI-extension framework, confirmed from prior
session evidence, not re-verified in this pass).

## 13. Persistence requirements

Confirmed persisted fields: `GroProgram.SourceCode` (code-editor-hinted
string property, per `test_simulator_runtime.cpp`); `BacteriaSignalGrid`
(`width`, `height`, `initialSignal`, `diffusionRate`, `decayRate`,
optional `initialValues` CSV); `BacteriaColony` (`gridWidth`, `gridHeight`,
`initialPopulation`, `simulationStep`, `numSteps`, `groProgram` reference,
optional `signalGrid`/`bioNetwork` references, `nextId`).

Three real fixtures already exist (`models/Smart_GroColonyGrowth.gen`,
`Smart_GroColonyLifecycle.gen`, `Smart_BacteriaColony_GRO.gen`); the third
is the richest and already demonstrates the single-argument `emit_signal`
call shape that must remain loadable (or be given an explicit, tested
migration) if Phase 3 changes `emit_signal`'s arity/semantics.

`WiP2026108/PE_Fix` (draft PR #545) is working on SimulLang/GenSerializer
text round-tripping, including a GroProgram model round-trip test. Phase 11
of this plan must rebase on that work once merged, rather than
independently re-fixing the same persistence path.

## 14. Test/oracle matrix

Existing coverage: 46 focused `TEST()` cases in
`test_runtime_pluginmanager.cpp` (full list recorded in the Phase 0
inspection evidence) covering plugin registration, parser lexical
boundaries, compiler IR shape, runtime command dispatch for every
currently-implemented builtin, and colony-level growth/motion/division/
signal/BioNetwork integration behavior.

Confirmed **not** covered by any existing test (regression gaps to close
before touching the corresponding production code):
- `ecoli_growth_rate = 0` → volume must not grow (§9 defect);
- `speed = 0` → position must not change (§10 defect);
- two independent signal handles (`s0`/`s1`) must not alias each other
  (§8/§6 defect);
- `reaction()` (entirely unimplemented, §6);
- `fun`/`needs`/`foreach`/`maptocells ... end` (entirely unimplemented,
  §5);
- the viewer's manual step/run controls running concurrently with
  event-calendar dispatch on the same colony (§12).

## 15. Phased implementation plan

Following the mandate's Phase 0–12 structure. Each phase: diagnose →
regression test(s) → minimal implementation → focused validation →
regression validation (`tests-unit`/`tests-kernel-unit`/`tests-smoke` as
applicable) → update this document → small, single-concern commit.

0. **Baseline and compatibility matrix** — this document (current phase,
   documentation-only).
1. **Frontend** (parser/AST/IR/compiler) — priority order informed by §7:
   (a) explicit diagnostic for unsupported syntax instead of silent
   `RawStatement`; (b) fix the `needs` comma-splitting bug; (c) resolve
   whether record-field reads belong in expressions (unblocks 4 examples
   at once); (d) rule `else` support; (e) `fun`/higher-order
   functions/`foreach`/`range`/`cross`/`let`/lambdas/lists/indexing, in
   that order of example-corpus leverage.
2. **Runtime/builtins** — close `geometry()`, `stats()`,
   `stop()`/`start()`, `clear_messages()`, `time()` (must stop hard-erroring),
   per the categorization in §6; `fopen`/`fprint` remain deferred pending a
   security decision (governance §15, stop gate 7).
3. **Multiple signal channels + reaction-diffusion** — give `signal()` a
   real handle/channel identity; reconcile with `BacteriaSignalGrid`'s
   current single-field design without inventing one `ModelDataDefinition`
   per channel by default; implement `reaction()`.
4. **Physical coordinates and sampling** — remove the shift-and-round
   seeding path; reconcile it with the centered convention used by
   `set_signal`; consider multi-point sampling along heading.
5. **Growth/division** — remove the `0.01` minimum clamp that forces
   growth at `growthRate=0`; add the regression first.
6. **Motion/mechanics** — remove the `speed<=0` positive-fallback; decide
   minimal internal mechanics vs. external engine only if barriers/
   chemostat effects are required and cannot be done internally
   (stop gate 2 if an external engine looks necessary).
7. **World-step pipeline** — make the per-step ordering explicit and
   tested; add the reaction stage once §3 lands.
8. **Viewer** — add the concurrent-trigger regression from §14; keep the
   viewer strictly a read/observe + explicit-step-request surface.
9. **End-to-end corpus** — re-run the §7 matrix with real fixtures once
   §1–§8 land; reclassify every "untested"/"partial" entry with executed
   evidence.
10. **Performance** — only after correctness phases close; address the
    reparse-every-call cost (§11) with a safe cache keyed by source
    identity.
11. **Persistence** — rebase on `WiP2026108/PE_Fix` once merged; validate
    the three existing `.gen` fixtures continue to load (or document/
    migrate any intentional change from Phase 3/5/6).
12. **Documentation/manual/completion gate** — reconcile this document,
    `STATUS.md`, the AI changelog, and manual impact per governance §10.

## 16. Known deviations (intentional, to document going forward)

- `die(n)` accepts an amount argument; the original `die()` takes none.
- `divide()` outside bacterium-scoped mode affects the whole aggregate
  population rather than a single cell — a GenESyS-specific aggregate-mode
  semantic, not a bug, but must be documented as a deviation, not silently
  conflated with the original's per-cell `divide()`.
- `tick()`, `grow(n)`, `set_population(n)`, `dump_signal_field(r,c)` are
  GenESyS extensions with no Gro-original equivalent.
- `rate(k)` uses `1 - exp(-k*dt)` (a proper Poisson-process per-step
  probability) instead of the original's `k*dt > rand()/RAND_MAX`
  approximation — a strictly more correct formulation, kept as a
  documented improvement rather than reverted to the original's
  approximation.

## 17. Explicitly unsupported behavior

- `fopen`/`fprint`/`dump` to arbitrary filesystem paths: deferred pending a
  security decision (no change proposed by this plan).
- GUI-only builtins (`zoom`, `set_theme`, `snapshot`) in the headless
  runtime: intentionally absent; Phase 2 should make this an explicit,
  tested no-op/diagnostic rather than an undocumented absence.
- Full academic Gro grammar (`fun`, `let/in/end`, general lambdas/lists/
  records as first-class values, `foreach`/`cross`/`maptocells ... end`):
  out of scope unless the example-corpus leverage identified in §7 changes
  the maintainer's priority.
- External 2D physics engine (Chipmunk or equivalent): not adopted without
  an explicit maintainer decision (stop gate 2).

## 18. Open decisions / stop gates

No stop gate has been triggered yet. Candidates to watch as phases
progress:
- Phase 3 may reveal that giving `BacteriaSignalGrid` multi-channel support
  requires a persistence-format decision affecting the three existing
  `.gen` fixtures — present impact/migration before changing the saved
  format, do not change it silently (governance §15, mandate §18 item 5).
- Phase 6 may reveal that barrier/chemostat physical effects cannot be
  done with a minimal internal 2D mechanics approach — present the A/B/C
  options from the mandate (§8/Phase 6) before adding any external physics
  dependency (stop gate 2).
- Phase 11 depends on `WiP2026108/PE_Fix` merging first; if it stalls,
  escalate rather than duplicating its persistence fixes.

## 19. Completion criteria

This integration is `BACTERIA-COLONY-INTEGRATION-COMPLETE` only when all
four independent levels hold with executed evidence, not static reading:
**language** (corpus mapped, supported subset executes with documented
semantics), **simulation** (signals/growth/division/movement/event-timing
have contracts and passing tests), **GUI** (viewer faithfully reflects
runtime state, no hidden second simulation model), **engineering** (build/
regression/persistence/ownership/documentation/manual impact coherent).
Until then, report state as `BACTERIA-COLONY-INTEGRATION-PARTIAL` with the
exact remaining feature, blocker, evidence, required decision and next
action — per phase, using this document's §15 numbering.

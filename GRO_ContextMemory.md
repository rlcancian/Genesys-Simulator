# GRO Context Memory

This is the canonical and only active context memory file for the GRO AI line.

Older or generic memory files must not be used as active context memory for this
AI. In particular, `ContextMemory.md`, `ContextMemmory.md`, and
`documentation/developers/GRO_context.md` are obsolete for active GRO memory.
If older memory files contain instructions that contradict this file, those
instructions are considered obsolete or consolidated here.

## Canonical Status

- **Agent name:** GRO.
- **Primary role:** coordinated AI developer responsible for the Gro language
  and bacteria/colony simulation integration line in GenESyS.
- **Canonical memory file:** `GRO_ContextMemory.md` in the repository root.
- **Base branch:** `WiP20261`.
- **GRO working branch:** `WiP20261_GRO`.
- **Current consolidated base:** `origin/WiP20261` at `0f72b673` as of the
  latest GRO synchronization.
- **Current GRO branch head:** `origin/WiP20261_GRO` has the Gro colony flow
  examples checkpoint published at `8a3a42f0`.
- **Current state:** the GRO work line is active again and is proceeding in
  explicit, user-confirmed phases on `WiP20261_GRO`.
- **Immediate objective:** continue the Gro biosimulator integration only one
  phase at a time, preserving the existing plan and stopping after each phase
  for explicit user confirmation.
- **Future work rule:** any future GRO work must start from the updated
  consolidated base `WiP20261`, not from stale local branch state or older
  memory assumptions.
- **Integration PR status:** historical PR `#370` from `WiP20261_GRO` to
  `WiP20261` was closed and merged. Current post-merge GRO work is newer than
  that PR and must not be assumed integrated into `WiP20261` until explicitly
  requested and verified.

## Operational Working Protocol

- **Response format:** every GRO response must use this exact envelope:
  - first line contains only `GRO`;
  - technical body follows after that;
  - last line contains only `----------`.
- **Repository reality check:** before acting, re-read this memory as needed and
  reanalyze the real repository state with Git/status/file inspection. Do not
  rely on stale memory alone.
- **Continuity rule:** recover the current plan from this memory, identify what
  has already been completed, and preserve continuity with that plan unless the
  user explicitly changes direction.
- **Phased work rule:** work phase by phase. Execute only the next logical phase
  that is technically safe and in scope.
- **Stop rule:** after completing a phase, stop. Report what was done, update
  this memory, and ask for explicit user confirmation before starting the next
  phase. Do not continue automatically.
- **Memory update rule:** at the end of each completed phase, update
  `GRO_ContextMemory.md` with the work completed, current state, relevant
  commits, validations, limitations, and the suggested next phase.
- **Stage policy:** stage changes intentionally. Group only coherent changes in
  the same stage set, avoid staging unrelated files together, and do not leave
  important phase-complete work untracked or unstaged without explaining why.
- **Commit policy:** create clear, auditable commits for completed phases or
  validated logical subunits. Commit messages must be objective and technical.
  Do not mix independent changes in one commit. Do not leave a completed phase
  without a commit unless there is an explicit technical reason.
- **Push policy:** push/publication is separate from stage and commit. Do not
  push automatically just because a commit was made. Push only when explicitly
  requested by the user or when the current operational flow explicitly calls
  for publication. Always state what is only local and what was published.
- **Scope control:** do not open a new functional front while performing memory,
  synchronization, or diagnostic work. Keep changes limited to the confirmed
  phase.
- **Plugin structure rule:** do not reorganize plugin directories or categories
  unless the user explicitly reopens that structural work. Work on the current
  repository structure as it exists.

## Git Policy

- Work only on `WiP20261_GRO`.
- Treat `WiP20261` as the shared integration base.
- Use the existing `origin` remote.
- Do not merge `WiP20261_GRO` back into `WiP20261` unless the user explicitly
  asks for that integration.
- Do not open or attempt a new integration into `WiP20261` unless the user
  explicitly asks for it.
- When the GRO line is reactivated for real technical work, begin from an
  updated `WiP20261` base. Do not continue from stale local branch state.
- Before implementation phases and before requested product publication, run
  `git fetch origin` and compare `origin/WiP20261` with `WiP20261_GRO`. Merge
  `origin/WiP20261` into `WiP20261_GRO` with normal merge when synchronization
  is needed; prefer merge over rebase for traceability.
- Make small, coherent commits that represent completed phases or validated
  logical subunits.
- Stage and commit are routine local operations when they follow the operational
  policy above.
- Push is not automatic. It requires explicit user request or an explicit
  publication step in the active workflow.
- Ask for confirmation only for destructive operations, relevant ambiguity, or
  exceptional risk.
- If local unfinished work blocks synchronization, use a temporary stash instead
  of mixing incomplete work into the base update.

## Communication And Documentation Policy

- Conversation with the user is in Portuguese.
- Source code, identifiers, Doxygen, code comments, and internal technical
  documentation added to the repository must remain in English.
- Do not use shared or generic root memory files as active memory for GRO.
- Do not use `documentation/developers/GRO_context.md` as active memory.
- Do not implement product code before inspecting the relevant current code and
  restoring context from this file.

## Technical Background

- GenESyS is a generic and extensible simulator organized around kernel
  abstractions and plugins.
- `ModelComponent` is the base class for simulation behavior blocks.
- `ModelDataDefinition` is the base class for reusable model data and supporting
  infrastructure.
- `ModelSimulation` owns global event-driven simulation progression and exposes
  `getSimulatedTime()`.
- `PluginManager`, `Plugin`, `PluginInformation`, and `PluginConnector_if`
  handle plugin discovery, registration, metadata, and instantiation.
- `PluginConnectorDummyImpl1` is the current static connector table for built-in
  plugin registration. It is integration-sensitive because multiple plugin
  lines may register new plugin filenames and factories there.
- Existing parser infrastructure (`Parser_if`, `ParserDefaultImpl2`,
  `genesyspp_driver`, Flex/Bison sources) is expression-oriented and model-aware;
  it is not the intended place for the Gro parser.
- Gro parsing and Gro runtime support should remain plugin-side unless a future
  kernel integration need is explicitly justified.

## Product Decisions

- Start with `BacteriaColony` rather than individual `Bacteria` as the first
  product component.
- Represent Gro source as reusable model data, `GroProgram`, so multiple
  colonies or future bacteria components can share the same program.
- Keep Gro parser/helper code under plugin-side code, likely near bacteria/Gro
  components, not in the kernel.
- Aim for full Gro syntax recognition when practical, while allowing partial
  semantics for unsupported constructs.
- Begin with discrete grid/cellular-automata style space and preserve a path to
  future continuous 2D space.
- Treat biological colony time as internal to the colony initially. It should be
  analogous to Gro time and synchronized across bacteria inside the colony, but
  independent from `ModelSimulation::getSimulatedTime()` until a later hybrid
  integration decision is made.
- Current working preference: bacteria are internal colony state first; separate
  bacteria as individual `ModelComponent` instances can be reconsidered later if
  the model architecture requires it.

## Implemented On `WiP20261_GRO`

### `GroProgram`

- **Files:** `source/plugins/data/BiologicalModeling/GroProgram.h`,
  `source/plugins/data/BiologicalModeling/GroProgram.cpp`.
- **Type:** `ModelDataDefinition`.
- **Purpose:** store reusable Gro source code attached to a GenESyS model.
- **Current behavior:** stores `SourceCode` as a string and performs a
  permissive lexical sanity check: non-empty source, balanced delimiters, closed
  string literals, and closed block comments.
- **Current limitation:** no full Gro grammar, biological reaction semantics, or
  integrated colony runtime binding exists yet.

### Gro Parser, AST, IR, And Runtime Helpers

- **Files:**
  - `source/plugins/data/BiologicalModeling/GroProgramParser.h`
  - `source/plugins/data/BiologicalModeling/GroProgramParser.cpp`
  - `source/plugins/data/BiologicalModeling/GroProgramAst.h`
  - `source/plugins/data/BiologicalModeling/GroProgramIr.h`
  - `source/plugins/data/BiologicalModeling/GroProgramCompiler.h`
  - `source/plugins/data/BiologicalModeling/GroProgramCompiler.cpp`
  - `source/plugins/data/BiologicalModeling/GroProgramRuntime.h`
  - `source/plugins/data/BiologicalModeling/GroProgramRuntime.cpp`
- **Current behavior:** the parser performs permissive lexical validation,
  fills a minimal AST for `program name() { ... }` or raw balanced statements,
  compiles top-level statements to a small IR, and executes `tick()` in an
  isolated runtime helper over caller-provided state.
- **Current behavior:** the parser performs permissive lexical validation,
  fills a minimal AST for `program name() { ... }` or raw balanced statements,
  compiles top-level statements to a small IR, and executes a first isolated
  command set over caller-provided state:
  - `tick()`: advances internal colony time by `simulationStep`;
  - `grow()` and `grow(n)`: increase population by one or by a positive integer
    amount;
  - `divide()`: doubles the current population;
  - `die()` and `die(n)`: decrease population by one or by a positive integer
    amount, failing if the command would remove more bacteria than available;
  - `set_population(n)`: replaces the current population with a positive
    integer value.
- **Current limitation:** runtime execution is intentionally minimal and is not
  yet connected to the GenESyS event scheduler, cellular automata, full Gro
  biological semantics, or GUI editing workflows. It now reports structured
  population mutations so `BacteriaColony` can apply them to owned bacteria
  state.

### `BacteriaColony`

- **Files:** `source/plugins/components/BiologicalModeling/BacteriaColony.h`,
  `source/plugins/components/BiologicalModeling/BacteriaColony.cpp`.
- **Type:** `ModelComponent`.
- **Purpose:** first Gro-inspired biological simulation component for bacteria
  colony behavior.
- **Current properties/state:** `GroProgram` reference, `SimulationStep`,
  `InitialColonyTime`, current internal colony time, `InitialPopulation`,
  current population summary, internal `BacteriumState` vector, `GridWidth`,
  and `GridHeight`.
- **Current behavior:** can initialize colony runtime state between replications,
  advance internal colony time by one configured simulation step, and execute a
  configured `GroProgram` once through the plugin-side
  `GroProgramParser` -> `GroProgramCompiler` -> `GroProgramRuntime` pipeline.
  On successful execution, the colony copies back internal colony time and
  population size from `GroProgramRuntimeState`.
- **Internal bacteria state:** each bacterium currently has a stable per-
  replication id, parent id, generation, division count, birth time, last update
  time, last division time, alive flag, and deterministic discrete grid
  coordinates. Population-changing runtime commands now carry mutation records
  that let the colony preserve simple lineage for `divide()`, remove live
  bacteria for `die()`, and keep the population summary and owned bacteria
  state synchronized.
- **Dispatch behavior:** when a `GroProgram` is configured, `_onDispatchEvent`
  executes that program once and traces success/failure. When no `GroProgram` is
  configured, it preserves the previous fallback behavior of advancing internal
  colony time by `SimulationStep`. After processing an arriving entity, it sends
  the entity through the front output connection.
- **Connectivity metadata:** registered as a normal flow component with one
  required input and one required output.
- **Current limitation:** it does not yet schedule periodic events in the
  GenESyS future event calendar and does not yet model per-bacterium command
  behavior, spatial cellular automata, signals, reactions, or complete Gro
  biological semantics.

### Static Plugin Registration

- **File:** `source/plugins/PluginConnectorDummyImpl1.cpp`.
- **Changes:** added includes and static connector mappings for:
  - `groprogram.so` -> `GroProgram::GetPluginInformation`
  - `bacteriacolony.so` -> `BacteriaColony::GetPluginInformation`
- **Integration dependency:** this file also contains upstream biological plugin
  registrations (`BioNetwork`, `BioParameter`, `BioReaction`, `BioSpecies`) from
  `origin/WiP20261`; preserve them during future merges.

### Tests

- **File:** `source/tests/unit/test_runtime_pluginmanager.cpp`.
- **Changes:** runtime plugin manager coverage checks discovery/connection of
  `groprogram.so` and `bacteriacolony.so`; it also creates `GroProgram` and
  `BacteriaColony`, configures the colony, initializes replication state, and
  verifies internal time advancement from `2.0` to `2.25`.

## Relevant Files Changed By GRO

- `GRO_ContextMemory.md`
- `documentation/developers/GRO_context.md` (obsolete memory file removed after
  migration)
- `source/plugins/data/BiologicalModeling/GroProgram.h`
- `source/plugins/data/BiologicalModeling/GroProgram.cpp`
- `source/plugins/data/BiologicalModeling/GroProgramParser.h`
- `source/plugins/data/BiologicalModeling/GroProgramParser.cpp`
- `source/plugins/data/BiologicalModeling/GroProgramAst.h`
- `source/plugins/data/BiologicalModeling/GroProgramIr.h`
- `source/plugins/data/BiologicalModeling/GroProgramCompiler.h`
- `source/plugins/data/BiologicalModeling/GroProgramCompiler.cpp`
- `source/plugins/data/BiologicalModeling/GroProgramRuntime.h`
- `source/plugins/data/BiologicalModeling/GroProgramRuntime.cpp`
- `source/plugins/components/BiologicalModeling/BacteriaColony.h`
- `source/plugins/components/BiologicalModeling/BacteriaColony.cpp`
- `source/plugins/PluginConnectorDummyImpl1.cpp`
- `source/tests/unit/test_runtime_pluginmanager.cpp`

## Branch And Commit State

- `WiP20261_GRO` was created from updated `WiP20261` and published to
  `origin/WiP20261_GRO`.
- The branch tracks `origin/WiP20261_GRO`.
- Historical GRO commits:
  - `41666d6f Add GRO developer coordination memory`
  - `b42c572f Add initial Gro bacteria colony plugins`
  - `6d1281d1 Record GRO branch workflow validation`
  - `a169884b Record GRO branch publication`
  - `b5a6f9d8 Consolidate GRO context memory`
  - `ec689a3b Record GRO session closure`
- Base synchronization commits:
  - `2114c92c Merge WiP20261_GRO into WiP20261 (#370)`
  - `d2e1255b Record GRO base synchronization`
  - `9994d96c Record GRO synchronization commit hash`
  - `8a50089d Record actual GRO PR state`
  - `2f280158 Move GRO context memory to repository root`
  - `10e6937c Merge remote-tracking branch 'origin/WiP20261' into WiP20261_GRO`
- Latest known validation before this migration:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
- Recent active GRO phase commits:
  - `cc97c548 Normalize GRO operational memory`
  - `ee855170 Add Gro program parser boundary`
  - `799feb71 Add minimal Gro program AST`
  - `e00c8108 Add initial Gro semantic IR`
  - `55058527 Merge remote-tracking branch 'origin/WiP20261' into WiP20261_GRO`
  - `be4d0337 Add isolated Gro runtime helper`
  - `3b59b3c2 Expand Gro runtime commands`
  - `8985889e Connect Gro runtime to bacteria colony`
  - `24d4be21 Add internal bacteria colony state`
  - `85d146de Track Gro population mutations in colony state`
  - `ba9c182a Add bacteria lifecycle metrics`
  - `e2c68795 Record bacteria lifecycle metrics checkpoint`
  - `3101d042 Add Gro die command support`
  - `8ca92ff7 Record Gro die command checkpoint`
  - `61c75ed3 Add Gro colony flow examples`
  - `8a3a42f0 Record Gro colony examples checkpoint`
- Latest known validation after active GRO phases:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager`
    succeeded with 10 tests.
- Latest structural synchronization:
  - merged `origin/WiP20261_KERNEL_GUI` at `f232882e` into
    `WiP20261_GRO`;
  - preserved the official plugin category organization;
  - GRO plugin files now live under the official `BiologicalModeling`
    category folders;
  - no merge conflicts occurred;
  - `cmake --preset tests-kernel-unit` succeeded;
  - `cmake --build --preset tests-kernel-unit-run` succeeded.

## Current Branch State

- Current branch: `WiP20261_GRO`.
- Current consolidated base: `origin/WiP20261` at `0f72b673`.
- Current GRO branch head: `origin/WiP20261_GRO` after the Gro colony flow
  examples checkpoint publication at `8a3a42f0`.
- Latest synchronization with `origin/WiP20261` fast-forwarded local
  `WiP20261_GRO` to `0f72b673`; no conflicts occurred.
- Conflict status during the latest synchronization: no conflicts occurred.
- Latest completed GRO phase: demonstration examples for the stable GRO slice
  and minimal flow-port enablement for `BacteriaColony`.
- Recent user-authorized work completed:
  1. expanded the isolated runtime command set;
  2. connected the isolated runtime pipeline to `BacteriaColony`;
  3. added the first owned per-bacterium state vector synchronized with colony
     population changes;
  4. added runtime population mutation records and simple colony lineage
     metadata for `divide()`;
  5. added bacterium age access, parent division count, and last division time.
  6. added minimal `die()` / `die(n)` support that removes bacteria from the
     current live colony state.
  7. exposed `BacteriaColony` as a one-input/one-output flow component and
     added two executable terminal smart examples.
- The `die()` command checkpoint was published to `origin/WiP20261_GRO` at
  `8ca92ff7`.
- The flow examples checkpoint was published to `origin/WiP20261_GRO` at
  `8a3a42f0`.
- Ask the user for explicit confirmation before proceeding to the next technical
  phase.

## Pending Work

- Do not continue automatically. Wait for explicit user confirmation before the
  next technical phase.
- Suggested next technical phase: either add a small example/preset discovery
  improvement for terminal GRO examples, or return to product work with event
  scheduler integration around the now stateful flow component. Keep cellular
  automata, signals, reactions, and complete Gro semantics as later phases.
- Decide whether `BacteriaColony` should schedule periodic internal events using
  `SimulationStep` or remain API/plugin-driven until parser/runtime semantics
  are clearer.
- Extend internal bacteria state with behavior fields only after an explicit
  phase decision.
- Define discrete grid/cellular automata data structures and how they relate to
  existing cellular automata plugins.
- Later, evaluate continuous 2D space support inspired by Gro.
- Later, coordinate GUI support for editing long Gro source text.

## Risks And Attention Points

- `PluginConnectorDummyImpl1.cpp` is conflict-prone because it centralizes static
  plugin registration.
- Parser infrastructure may be touched by other work, but Gro should not be
  coupled to the current expression parser without a specific architectural
  decision.
- GUI support for `GroProgram::SourceCode` may require a dedicated long-text or
  code editor; the current property is only a generic string control.
- Creating each bacterium as a separate `ModelComponent` may be too heavy at
  scale; colony-owned internal bacteria state remains the preferred first model.
- Internal colony time must not be confused with `ModelSimulation` global time.
- Upstream biological data plugins introduced on `WiP20261` may overlap with
  future Gro biochemical modeling concepts; inspect before duplicating concepts.
- Stale memory files are a coordination risk. GRO must use only
  `GRO_ContextMemory.md` in the repository root as active memory.
- Because the base already absorbed the functional GRO content, future work
  should begin from a fresh synchronization decision rather than assuming this
  branch still needs integration.
- Local branch history can become misleading after base absorption. Treat
  `WiP20261` as the source of truth for future starts.

## Session Log

### 2026-04-17 - Operational Memory Normalization

- User paused technical biosimulator work and requested operational memory
  normalization before any next technical phase.
- Re-read `GRO_ContextMemory.md` and found useful project context but several
  operational points were incomplete or stale:
  - response envelope was not explicit;
  - phase/stop/confirmation policy was not centralized;
  - stage/commit/push policy was not clearly separated;
  - push policy still allowed automatic routine publication;
  - branch status still contained obsolete standby wording;
  - implemented parser/AST/IR/runtime helpers were not fully summarized in the
    stable memory sections.
- Added the `Operational Working Protocol` section.
- Updated current branch/base state, implemented helper summaries, relevant
  files, pending work, and recent commit/validation records.
- No biosimulator product functionality was changed in this normalization pass.

### 2026-04-17 - Initial Gro Plugin Slice

- Created the initial `GroProgram` model data definition.
- Created the initial `BacteriaColony` model component.
- Registered both plugins in the static connector.
- Added focused runtime plugin manager tests.
- Validated focused plugin-manager test and later the complete
  `tests-kernel-unit-run` target.

### 2026-04-17 - Dedicated Branch Workflow

- Established `WiP20261_GRO` as the exclusive GRO branch.
- Synchronized `WiP20261` from `origin/WiP20261`.
- Published `WiP20261_GRO` with upstream tracking.
- Resolved a connector conflict with upstream biological plugins.
- Pushed branch-local commits.

### 2026-04-17 - Canonical Memory Cleanup In `documentation/developers`

- `documentation/developers/GRO_context.md` became the temporary canonical
  persistent memory file at that time.
- `documentation/developers/communication.md` was deprecated and removed from
  the branch.
- Git policy was updated to allow autonomous routine stage, commit, fetch,
  merge, pull, and push operations.

### 2026-04-17 - Base Synchronization For Integration

- User reported that the shared base branch `WiP20261` advanced and that the
  integration PR `WiP20261_GRO` -> `WiP20261` was not mergeable at that moment.
- Ran `git fetch origin`; `origin/WiP20261` advanced from `758127ef` to
  `2114c92c`.
- Observed that `origin/WiP20261` already contained the GRO branch history via
  merge commit `2114c92c` (`Merge WiP20261_GRO into WiP20261 (#370)`).
- Merged `origin/WiP20261` into local `WiP20261_GRO`.
- Merge result: fast-forward from `ec689a3b` to `2114c92c`.
- Conflict status: no conflicts occurred.
- Conflicted files: none.
- Files brought in by the fast-forward: `documentation/developers/TINKERCELL_context.md`.
- GitHub verification after push found PR `#370`
  (`WiP20261_GRO` -> `WiP20261`) closed and merged, with merge commit
  `2114c92c`. A search for an open PR with head `WiP20261_GRO` and base
  `WiP20261` returned no results.

### 2026-04-17 - Root Canonical Memory Migration

- User instructed that this AI must no longer use a generic shared root
  `ContextMemory.md` or a memory file under `documentation/developers/`.
- Identified the previously active memory file:
  `documentation/developers/GRO_context.md`.
- No generic root `ContextMemory.md` existed in the GenESyS repository at the
  time of migration.
- Migrated useful GRO context into root `GRO_ContextMemory.md`.
- `GRO_ContextMemory.md` is now the canonical and only active memory file for
  this AI.
- `documentation/developers/GRO_context.md` was removed from this branch after
  migration and must not be used as active memory.

### 2026-04-17 - Standby After Base Absorption

- User stated that the base branch `WiP20261` has already absorbed the relevant
  functional content from `WiP20261_GRO`.
- New canonical operating mode: do not implement new functionality and do not
  attempt a new integration now.
- Confirmed active memory remains root `GRO_ContextMemory.md`.
- Confirmed old active memory files must not be used:
  `documentation/developers/GRO_context.md`, generic `ContextMemory.md`, and any
  other older memory/context file.
- Ran `git fetch origin`; `origin/WiP20261` advanced to `5df02726`.
- Merged `origin/WiP20261` into `WiP20261_GRO`.
- Merge result: merge commit `10e6937c`.
- Conflict status: no conflicts occurred.
- Files brought in by the merge: `TINKERCELL_ContextMemory.md`.
- Branch state after this session should be stable and in standby, with only
  memory consolidation committed after the base synchronization.

### 2026-04-17 - Post-Merge Canonical Standby Memory

- User reaffirmed that the base branch `WiP20261` has already absorbed the
  relevant functional content from GRO.
- Recorded `WiP20261` as the current consolidated base.
- Recorded that there is no pending GRO integration at this moment.
- Recorded that future GRO work must start from the updated consolidated base
  `WiP20261`, not from stale local branch state or older memory assumptions.
- No new functionality was implemented.
- No new merge was attempted in this session.
- Only `GRO_ContextMemory.md` was updated.

### 2026-04-17 - Synchronization With Official Plugin Category Layout

- User identified `f232882e` (`Organize plugins by declared category`) as the
  official structural base from `KERNEL_GUI`.
- Ran `git fetch origin`; `origin/WiP20261_KERNEL_GUI` advanced to `f232882e`.
- Merged `origin/WiP20261_KERNEL_GUI` into `WiP20261_GRO`.
- Merge result: merge commit `13f83fbd`.
- Conflict status: no conflicts occurred.
- The official layout moved GRO files to:
  - `source/plugins/data/BiologicalModeling/GroProgram.h`
  - `source/plugins/data/BiologicalModeling/GroProgram.cpp`
  - `source/plugins/components/BiologicalModeling/BacteriaColony.h`
  - `source/plugins/components/BiologicalModeling/BacteriaColony.cpp`
- Includes and tests now reference the official `BiologicalModeling` paths.
- `source/plugins/data/CMakeLists.txt` from the official base now uses recursive
  plugin data source discovery, so `GroProgram.cpp` remains in the build after
  the category move.
- Validation after merge:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.

### 2026-04-17 - Gro Parser Boundary Phase

- User suspended any further plugin directory/category reorganization and
  instructed GRO to continue phased integration on the repository structure as
  currently present.
- Re-read this canonical memory and resumed the existing GRO plan.
- Current next phase selected: define the plugin-side Gro parser/helper boundary
  before adding AST, semantic execution, bacteria state, cellular automata, or
  GUI integration.
- Ran `git fetch origin` and fast-forwarded local `WiP20261_GRO` from
  `d1ec21c9` to `76c938a6`, matching the current consolidated `origin/WiP20261`
  base that had already merged GRO.
- Added `GroProgramParser` under the existing current path
  `source/plugins/data/BiologicalModeling/`, without moving plugin files or
  changing category layout.
- `GroProgram::validateSyntax` now delegates to `GroProgramParser::parse`,
  preserving the previous permissive lexical validation behavior and diagnostic
  messages.
- Added focused unit coverage for the parser boundary in
  `source/tests/unit/test_runtime_pluginmanager.cpp`.
- Validation after this phase:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager`
    succeeded with 8 tests.

### 2026-04-17 - Gro Isolated Runtime Helper Phase

- User confirmed continuation after the initial semantic IR phase.
- Revalidated current branch state on `WiP20261_GRO`; the branch was clean and
  matched `origin/WiP20261_GRO` before changes.
- Ran `git fetch origin`; `origin/WiP20261` advanced from `76c938a6` to
  `f769ec61` with GUI preference/theme work.
- Merged `origin/WiP20261` into local `WiP20261_GRO` before continuing GRO
  implementation.
- Merge result: merge commit with no conflicts.
- No GUI files brought by the base merge were modified by GRO.
- Executed only the next phase: create an isolated runtime helper that consumes
  `GroProgramIr` without binding to `BacteriaColony` or the GenESyS event
  scheduler.
- Added `GroProgramRuntime` under the existing current path
  `source/plugins/data/BiologicalModeling/`, without moving plugin files or
  changing category layout.
- `GroProgramRuntime::execute` currently supports:
  - `tick()` as the first executable command, advancing caller-provided
    runtime state by `simulationStep`.
  - reporting unsupported function calls without failing execution.
  - reporting raw statements skipped by the runtime.
  - rejecting invalid `tick(...)` arguments and non-positive simulation steps.
- No `BacteriaColony` scheduler binding, bacteria state, cellular automata, GUI
  work, or plugin directory reorganization was added in this phase.
- Validation after this phase:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager`
    succeeded with 10 tests.

### 2026-04-17 - Gro Runtime Command Expansion And Colony Bridge

- User explicitly requested two ordered steps in the same turn: first expand the
  Gro command set, then connect the isolated runtime to `BacteriaColony`.
- Confirmed that the active environment root was the original Gro repository,
  while the requested `BacteriaColony` work belongs to the GenESyS clone at
  `/home/rafaelcancian/CLionProjects/Genesys-Simulator`; continued the
  implementation in the GenESyS clone because that is where the active GRO
  integration branch and `BacteriaColony` live.
- Expanded `GroProgramRuntimeState` with `populationSize`.
- Expanded `GroProgramRuntime::execute` to support:
  - `tick()`;
  - `grow()` and `grow(n)`;
  - `divide()`;
  - `set_population(n)`.
- Added validation for positive integer command arguments and overflow checks
  for population-changing commands.
- Created local commit `3b59b3c2` (`Expand Gro runtime commands`) for the
  command expansion subphase.
- Connected `BacteriaColony` to the parser/compiler/runtime pipeline through
  `BacteriaColony::executeGroProgram()`.
- Updated `BacteriaColony::_onDispatchEvent` to execute the configured
  `GroProgram` when one exists, while preserving the old internal-time advance
  fallback when no program is configured.
- Added focused unit coverage for executing a configured Gro program through
  `BacteriaColony` and applying the resulting internal time and population
  state.
- Validation after both ordered steps:
  - `git diff --check` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
- No push/publication was performed.
- Suggested next phase remains separate from this bridge: define the next
  biological-state or scheduler integration step explicitly before continuing.

### 2026-04-17 - Internal Bacteria State Checkpoint

- User requested starting with internal bacteria state and authorized proceeding
  until a stable checkpoint, with explicit stage/commit and a question about
  pausing or continuing at the checkpoint.
- Ran `git fetch origin`; `origin/WiP20261` remained at `f769ec61` and local
  `WiP20261_GRO` already contained that base.
- Added the first `BacteriaColony::BacteriumState` representation owned by the
  colony instead of creating bacteria as separate `ModelComponent` instances.
- Each internal bacterium currently stores:
  - stable per-replication id;
  - birth time;
  - last update time;
  - deterministic grid coordinates;
  - alive flag.
- `BacteriaColony` now initializes the internal bacteria vector between
  replications, updates bacterium timestamps when colony time advances, and
  resizes the internal vector after successful runtime population changes.
- Grid coordinates are assigned deterministically from vector index over the
  configured discrete grid, wrapping when population exceeds grid capacity.
- Added unit coverage ensuring initial internal bacteria state is created and
  remains synchronized after `tick()`, `grow`, `divide`, and `set_population`.
- Validation after this checkpoint:
  - `git diff --check` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
- Functional checkpoint commit:
  - `24d4be21 Add internal bacteria colony state`
- No push/publication was performed.
- Suggested next phase: either add the first per-bacterium behavior fields and
  command effects, or wire scheduler execution around the now stateful colony.

### 2026-04-17 - Population Mutation Lineage Checkpoint

- User asked whether GRO could advance to the next checkpoint in one to three
  phases and authorized proceeding if feasible.
- Ran `git fetch origin`; `origin/WiP20261` advanced to `dca0397f`.
- Fast-forwarded local `WiP20261_GRO` to `dca0397f`, which already contains the
  previously published GRO checkpoint and newer KERNEL_GUI/TINKERCELL work.
- No conflicts occurred during the fast-forward.
- Executed a bounded checkpoint in two technical subphases:
  1. `GroProgramRuntime::ExecutionResult` now exposes structured population
     mutation records for `grow`, `divide`, and `set_population`;
  2. `BacteriaColony` applies those mutation records to the owned
     `BacteriumState` vector instead of blindly resizing after runtime
     execution.
- `BacteriumState` now records `parentId` and `generation`.
- `divide()` now creates daughter bacteria with parent id and generation
  derived from the current parents.
- `grow()` and `set_population()` remain explicit population operations:
  `grow()` appends unparented generation-zero bacteria and `set_population()`
  truncates or extends to the requested count.
- Added unit coverage for runtime mutation records and simple colony lineage
  after combined `tick`, `grow`, `divide`, and `set_population` execution.
- Validation after this checkpoint:
  - `git diff --check` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
- Functional checkpoint commit:
  - `85d146de Track Gro population mutations in colony state`
- No push/publication was performed.
- Suggested next phase: either begin actual per-bacterium command behavior
  beyond lineage metadata, or integrate the stateful colony with scheduler
  execution.

### 2026-04-17 - Bacteria Lifecycle Metrics Checkpoint

- User instructed GRO to continue to the next stable checkpoint because the
  previous checkpoint had been committed.
- Ran `git fetch origin`; `origin/WiP20261` did not advance, while
  `origin/WiP20261_KERNEL_GUI` advanced independently.
- Executed one bounded technical phase over the existing stateful colony:
  enrich each internal bacterium with lifecycle metrics without touching
  scheduler integration, cellular automata, signals, reactions, or plugin
  directory layout.
- Added `divisionCount` and `lastDivisionTime` to `BacteriumState`.
- Added `BacteriaColony::getBacteriumAge(std::size_t)` for current internal
  colony-time age calculation.
- During `divide()` mutation application, parent bacteria now increment
  `divisionCount` and record `lastDivisionTime`; daughter bacteria remain
  generation +1 with `divisionCount == 0`.
- Added focused unit assertions for initial age, parent division metrics, and
  daughter lifecycle defaults.
- Validation after this checkpoint:
  - `git diff --check` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
- Functional checkpoint commit:
  - `ba9c182a Add bacteria lifecycle metrics`
- No push/publication was performed.
- Suggested next phase: either add the first per-bacterium behavior command
  effects beyond lifecycle metrics, or add event scheduler integration around
  the now stateful colony.

### 2026-04-17 - Gro Die Command Checkpoint

- User requested a push first and then continuation to the next phase.
- Pushed `WiP20261_GRO` to `origin/WiP20261_GRO`, publishing the previous
  lifecycle metrics checkpoint at `e2c68795`.
- Ran `git fetch origin`; `origin/WiP20261` did not advance, while
  `origin/WiP20261_KERNEL_GUI` advanced independently.
- Executed one bounded technical phase: add minimal Gro `die()` semantics to
  the isolated runtime and connect the resulting mutation to
  `BacteriaColony`.
- Added `PopulationMutationType::Die`.
- `GroProgramRuntime::execute` now supports:
  - `die()`: removes one bacterium from the runtime population;
  - `die(n)`: removes a positive integer amount;
  - failure when the command would remove more bacteria than currently
    available.
- `BacteriaColony` now applies `Die` population mutations by removing bacteria
  from the owned internal vector, keeping the live population summary and
  internal bacteria count synchronized.
- Added focused unit coverage for isolated runtime `die()` behavior, invalid
  over-removal, and colony-side internal state after `die(2)`.
- Validation after this checkpoint:
  - `git diff --check` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
- Functional checkpoint commit:
  - `3101d042 Add Gro die command support`
- No push/publication was performed for the `die()` checkpoint yet.
- Suggested next phase: either add scheduler integration for repeated colony
  execution, or refine death semantics if dead-cell history must be retained
  before scheduler work.

### 2026-04-17 - Gro Colony Flow Examples Checkpoint

- User redirected the next phase away from expanding biosimulator semantics and
  toward executable demonstration examples for the already stable GRO slice.
- Ran `git fetch origin`; `origin/WiP20261` advanced to `0f72b673`.
- Local `WiP20261_GRO` fast-forwarded to `0f72b673` before product changes,
  preserving the consolidated base and already published GRO history.
- Analyzed terminal smart examples, using `Smart_Process`, `Smart_Delay`, and
  `Smart_CellularAutomata` as style references for programmatic model creation,
  plugin lookup, component insertion, connection, save, and simulation.
- Reanalyzed `BacteriaColony` and found that it was still exposed as a
  source/sink-style component with zero inputs and zero outputs, and that its
  dispatch path removed received entities after processing.
- Updated `BacteriaColony` to expose one required input and one required output.
- Updated `BacteriaColony` dispatch semantics so an arriving entity triggers one
  configured Gro/runtime step, or the fallback internal time step, and then is
  forwarded through the front output connection.
- Added focused unit coverage proving the plugin metadata exposes one input and
  one output and that a `Create -> BacteriaColony -> Dispose` model forwards an
  entity after a runtime step.
- Added executable terminal smart examples:
  - `Smart_GroColonyGrowth`, demonstrating `tick()` and `grow(2)` through a
    flow model;
  - `Smart_GroColonyLifecycle`, demonstrating `tick()`, `divide()`, and
    `die(1)` through a flow model with lifecycle metrics.
- Validation after this checkpoint:
  - `git diff --check` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
  - `cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=smarts/Smart_GroColonyGrowth.cpp`
    succeeded.
  - `cmake --build --preset terminal-example` succeeded for
    `Smart_GroColonyGrowth`.
  - `./build/terminal-example/source/applications/terminal/genesys_terminal_application`
    succeeded for `Smart_GroColonyGrowth`, forwarding three entities to
    `Dispose_GrowthPulse` and printing colony time `1.5`, population `8`, and
    bacteria count `8`.
  - `cmake --preset terminal-example -DGENESYS_TERMINAL_EXAMPLE=smarts/Smart_GroColonyLifecycle.cpp`
    succeeded.
  - `cmake --build --preset terminal-example` succeeded for
    `Smart_GroColonyLifecycle`.
  - `./build/terminal-example/source/applications/terminal/genesys_terminal_application`
    succeeded for `Smart_GroColonyLifecycle`, forwarding two entities to
    `Dispose_LifecyclePulse` and printing colony time `7`, population `9`,
    first division count `2`, and youngest generation `2`.
- Functional checkpoint commit:
  - `61c75ed3 Add Gro colony flow examples`
- Publication after this checkpoint:
  - pushed `WiP20261_GRO` to `origin/WiP20261_GRO`;
  - remote advanced from `8ca92ff7` to `8a3a42f0`.
- Suggested next phase: either add a small terminal example discovery/preset
  improvement for GRO examples, or return to event scheduler integration around
  the now stateful flow component.

### 2026-04-17 - End-Of-Day Publication And Pause

- User requested publication of the current local commits, memory update, and
  end-of-day pause.
- Published `WiP20261_GRO` to `origin/WiP20261_GRO`.
- Published range: `8ca92ff7..8a3a42f0`.
- Published checkpoint commits:
  - `61c75ed3 Add Gro colony flow examples`
  - `8a3a42f0 Record Gro colony examples checkpoint`
- Current working rule after closure: do not start a new technical phase until
  the user explicitly resumes.
- Suggested next phase remains unchanged: either a small terminal example
  discovery/preset improvement for GRO examples, or event scheduler integration
  around the now stateful flow component.

### 2026-04-17 - Gro Initial Semantic IR Phase

- User confirmed continuation after the minimal AST/IR phase.
- Revalidated current branch state on `WiP20261_GRO`; the branch was clean and
  matched `origin/WiP20261_GRO` before changes.
- Ran `git fetch origin`; no new remote changes were reported.
- Executed only the next phase: create a non-executable semantic IR layer from
  the parsed `GroProgramAst`.
- Added `GroProgramIr` and `GroProgramCompiler` under the existing current path
  `source/plugins/data/BiologicalModeling/`, without moving plugin files or
  changing category layout.
- `GroProgramCompiler::compile` now converts AST statements into generic IR
  commands:
  - `FunctionCall` for recognized `identifier(...)` statements.
  - `RawStatement` for balanced statements not yet understood semantically.
  - top-level comma-separated function arguments are preserved as source text.
- No execution binding, bacteria state, cellular automata, GUI work, or plugin
  directory reorganization was added in this phase.
- Validation after this phase:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager`
    succeeded with 9 tests.

### 2026-04-17 - Gro Minimal AST/IR Phase

- User confirmed continuation after the parser boundary phase.
- Revalidated current branch state on `WiP20261_GRO`; the branch was clean and
  matched `origin/WiP20261_GRO` before changes.
- Ran `git fetch origin`; no new remote changes were reported.
- Executed only the next phase: define the first minimal AST/IR target for Gro
  parsing.
- Added `GroProgramAst` under the existing current path
  `source/plugins/data/BiologicalModeling/`, without moving plugin files or
  changing category layout.
- `GroProgramParser::Result` now carries a `GroProgramAst`.
- The parser now keeps the permissive lexical acceptance behavior but fills an
  AST with:
  - `ProgramBlock` when it recognizes `program name() { ... }`.
  - `RawStatements` for other balanced source fragments.
  - top-level semicolon-delimited statement source text.
- No semantic execution, bacteria state, cellular automata, GUI work, or plugin
  directory reorganization was added in this phase.
- Validation after this phase:
  - `cmake --preset tests-kernel-unit` succeeded.
  - `cmake --build --preset tests-kernel-unit-run` succeeded.
  - `./build/tests-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager`
    succeeded with 8 tests.

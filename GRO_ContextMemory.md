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
- **Current consolidated base:** `WiP20261`.
- **Current state:** the relevant functional GRO content has already been
  absorbed by the base branch `WiP20261`; there is no pending GRO integration at
  this moment.
- **Immediate objective:** keep `WiP20261_GRO` stable and in standby. Do not
  implement new functionality and do not attempt a new integration unless the
  user explicitly reactivates this work line.
- **Future work rule:** any future GRO work must start from the updated
  consolidated base `WiP20261`, not from stale local branch state or older
  memory assumptions.
- **Integration PR status:** PR `#370` from `WiP20261_GRO` to `WiP20261` exists
  and GitHub reported it closed and merged. Its merge commit is `2114c92c`.
  After that merge, `WiP20261_GRO` received context-only commits so the current
  branch differs from `origin/WiP20261` only by GRO memory updates.

## Git Policy

- Work only on `WiP20261_GRO`.
- Treat `WiP20261` as the shared integration base.
- Use the existing `origin` remote.
- Do not merge `WiP20261_GRO` back into `WiP20261` unless the user explicitly
  asks for that integration.
- Do not implement new GRO functionality while the branch is in standby.
- Do not open or attempt a new integration while the base already contains the
  relevant functional content, unless the user explicitly asks for it.
- When the GRO line is reactivated for real technical work, begin from an
  updated `WiP20261` base. Do not continue from stale local branch state.
- Before important future product pushes, run `git fetch origin` and merge
  `origin/WiP20261` into `WiP20261_GRO` using merge, not rebase. While the
  branch is explicitly in standby, do not perform another merge unless the user
  asks for synchronization.
- Make small, coherent commits.
- GRO has autonomy to run routine Git operations without asking the user:
  stage, commit, fetch, merge, pull, and push.
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

- **Files:** `source/plugins/data/GroProgram.h`,
  `source/plugins/data/GroProgram.cpp`.
- **Type:** `ModelDataDefinition`.
- **Purpose:** store reusable Gro source code attached to a GenESyS model.
- **Current behavior:** stores `SourceCode` as a string and performs a
  permissive lexical sanity check: non-empty source, balanced delimiters, closed
  string literals, and closed block comments.
- **Current limitation:** no full Gro parser, AST, interpreter, semantic model,
  reaction semantics, or execution runtime exists yet.

### `BacteriaColony`

- **Files:** `source/plugins/components/bacteria/BacteriaColony.h`,
  `source/plugins/components/bacteria/BacteriaColony.cpp`.
- **Type:** `ModelComponent`.
- **Purpose:** first Gro-inspired biological simulation component for bacteria
  colony behavior.
- **Current properties/state:** `GroProgram` reference, `SimulationStep`,
  `InitialColonyTime`, current internal colony time, `InitialPopulation`,
  current population summary, `GridWidth`, and `GridHeight`.
- **Current behavior:** can initialize colony runtime state between replications
  and advance internal colony time by one configured simulation step.
- **Connectivity metadata:** registered as a self-contained source/sink-style
  component with zero required inputs and outputs for the first plugin slice.
- **Current limitation:** it does not yet schedule periodic events in the
  GenESyS future event calendar and does not execute Gro biological semantics.

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
- `source/plugins/data/GroProgram.h`
- `source/plugins/data/GroProgram.cpp`
- `source/plugins/components/bacteria/BacteriaColony.h`
- `source/plugins/components/bacteria/BacteriaColony.cpp`
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

## Current Branch State

- Current branch: `WiP20261_GRO`.
- Current consolidated base: `WiP20261`.
- The relevant functional content from `WiP20261_GRO` has already been absorbed
  by `WiP20261`.
- No GRO integration is pending at this moment.
- The branch is in standby. It should remain stable, synchronized, and
  available for future GRO work, but no new product functionality should be
  added now.
- Any future technical work should start from the updated consolidated base
  `WiP20261`, not from stale local state preserved on `WiP20261_GRO`.
- Latest synchronization with `origin/WiP20261` produced merge commit
  `10e6937c` and brought in `TINKERCELL_ContextMemory.md`.
- Conflict status during the latest synchronization: no conflicts occurred.
- The current changes after that synchronization are memory-only updates to
  `GRO_ContextMemory.md`.

## Pending Work

- No immediate implementation should be performed while the branch is in
  standby.
- Define the Gro parser/helper boundary under plugin-side code.
- Decide the first grammar target: full syntax recognition with partial
  semantics, or a narrower MVP syntax subset.
- Define the AST or intermediate representation needed to connect Gro syntax to
  GenESyS model data and components.
- Decide whether `BacteriaColony` should schedule periodic internal events using
  `SimulationStep` or remain API/plugin-driven until parser/runtime semantics
  are clearer.
- Define internal bacteria state representation inside a colony.
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

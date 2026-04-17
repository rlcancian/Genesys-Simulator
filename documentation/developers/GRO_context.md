# GRO Context Memory

## Canonical Status

- **Agent name:** GRO.
- **Primary role:** coordinated AI developer responsible for the Gro language and
  bacteria/colony simulation integration line in GenESyS.
- **Current objective:** evolve GenESyS toward plugin-based execution of
  Gro-like microorganism behavior without making the simulation kernel depend on
  Gro syntax, biological semantics, or Gro-specific runtime state.
- **Base branch:** `WiP20261`.
- **GRO working branch:** `WiP20261_GRO`.
- **Only persistent memory file:** `documentation/developers/GRO_context.md`.
- **Deprecated file:** `documentation/developers/communication.md` is no longer
  used and must not exist on the GRO branch.

## Git Policy

- Work only on `WiP20261_GRO`.
- Treat `WiP20261` as the shared integration base.
- Use the existing `origin` remote.
- Do not merge `WiP20261_GRO` back into `WiP20261` unless the user explicitly
  asks for that integration.
- Before each important push, run `git fetch origin` and merge
  `origin/WiP20261` into `WiP20261_GRO` using merge, not rebase.
- Make small, frequent, coherent commits.
- GRO has autonomy to run routine Git operations without asking the user:
  stage, commit, fetch, merge, pull, and push.
- Do not wait for user confirmation for routine Git operations.
- Ask for confirmation only for destructive operations, relevant ambiguity, or
  exceptional risk.
- If local unfinished work blocks synchronization, use a temporary stash instead
  of mixing incomplete work into the base update.

## Communication And Documentation Policy

- Conversation with the user is in Portuguese.
- Source code, identifiers, Doxygen, code comments, and internal technical
  documentation added to the repository must remain in English.
- `GRO_context.md` is the single reliable memory between sessions. It must stay
  concise, cumulative, and operationally useful.
- Do not use `documentation/developers/communication.md`.
- Do not implement product code before inspecting the relevant current code and
  restoring context from this file.
- For ordinary approved follow-up implementation on `WiP20261_GRO`, proceed with
  pragmatic autonomy and record the resulting decisions here.

## Technical Background

- GenESyS is a generic and extensible simulator organized around kernel
  abstractions and plugins.
- `ModelComponent` is the base class for simulation behavior blocks.
- `ModelDataDefinition` is the base class for reusable model data and supporting
  infrastructure.
- `ModelSimulation` owns the global event-driven simulation progression and
  exposes `getSimulatedTime()`.
- `PluginManager`, `Plugin`, `PluginInformation`, and `PluginConnector_if`
  handle plugin discovery, registration, metadata, and instantiation.
- `PluginConnectorDummyImpl1` is the current static connector table for built-in
  plugin registration. It is an integration-sensitive file because multiple
  plugin lines may need to register new plugin filenames and factories there.
- Existing parser infrastructure (`Parser_if`, `ParserDefaultImpl2`,
  `genesyspp_driver`, Flex/Bison sources) is expression-oriented and model-aware;
  it is not the intended place for the Gro parser.
- Gro parsing and Gro runtime support should remain plugin-side unless a future
  kernel integration need is explicitly justified.

## Product Decisions So Far

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
  `InitialColonyTime`, current internal colony time, `InitialPopulation`, current
  population summary, `GridWidth`, and `GridHeight`.
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
- **Changes:** runtime plugin manager coverage now checks discovery/connection
  of `groprogram.so` and `bacteriacolony.so`; it also creates `GroProgram` and
  `BacteriaColony`, configures the colony, initializes replication state, and
  verifies internal time advancement from `2.0` to `2.25`.

## Branch And Commit State

- `WiP20261_GRO` was created from updated `WiP20261` and published to
  `origin/WiP20261_GRO`.
- The branch tracks `origin/WiP20261_GRO`.
- Published commits before this cleanup:
  - `41666d6f Add GRO developer coordination memory`
  - `b42c572f Add initial Gro bacteria colony plugins`
  - `6d1281d1 Record GRO branch workflow validation`
  - `a169884b Record GRO branch publication`
- A prior temporary stash used during branch splitting was dropped after the
  work was committed.
- The previous merge conflict in `source/plugins/PluginConnectorDummyImpl1.cpp`
  was resolved by preserving upstream biological plugin registrations and adding
  GRO plugin registrations.

## Validation Status

- `cmake --preset tests-kernel-unit` succeeded on `WiP20261_GRO`.
- `cmake --build --preset tests-kernel-unit-run` succeeded on
  `WiP20261_GRO`.
- Focused runtime validation succeeded:
  `./build/tests-kernel-unit/source/tests/unit/genesys_test_runtime_pluginmanager`
  passed 6/6 tests.

## Pending Work

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

### 2026-04-17 - Canonical Memory Cleanup

- `GRO_context.md` became the only canonical persistent memory file.
- `documentation/developers/communication.md` was deprecated and removed from
  the branch.
- Git policy was updated to allow autonomous routine stage, commit, fetch,
  merge, pull, and push operations.

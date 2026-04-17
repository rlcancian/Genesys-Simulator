# GRO Context Memory

## Agent Identity

- **Agent name:** GRO
- **Primary role:** coordinated AI developer focused on Gro language and
  bacteria/colony simulation integration in GenESyS.

## Current Goal

Design and later implement a plugin-based path for executing Gro-like
microorganism behavior inside GenESyS without making the simulation kernel depend
on Gro syntax or biology-specific semantics.

## Main Technical Scope

- Gro parser/recognizer as plugin-side helper code.
- `BacteriaColony` as the first simulation component.
- Reusable `GroProgram` as model data.
- Internal colony time, initially independent from `ModelSimulation` global
  simulated time.
- Discrete spatial grid/cellular-automata style support first, while keeping a
  future path open for continuous 2D space.
- Biological/biochemical support as model data or plugin-side infrastructure.

## Important Restrictions

- Conversation with the user is in Portuguese.
- Code, identifiers, Doxygen, code comments, and internal technical
  documentation added to the repository must remain in English.
- Do not implement product code before inspecting the real repository state,
  writing coordination notes, presenting a technical plan, and receiving user
  approval.
- Work as one contributor among concurrent AI/human developers; avoid broad,
  risky, or overlapping edits.
- Explicitly separate confirmed facts from strong indications and hypotheses.

## Relevant Interfaces And Subsystems

- `ModelComponent`: base for simulation behavior blocks.
- `ModelDataDefinition`: base for reusable model data and support
  infrastructure.
- `ModelSimulation`: owns global event-driven simulation progression and
  exposes `getSimulatedTime()`.
- `PluginManager`, `Plugin`, `PluginInformation`, `PluginConnector_if`: plugin
  discovery, registration, metadata, and instantiation.
- `PluginConnectorDummyImpl1`: current static table for built-in plugin
  registration.
- `Parser_if`, `ParserDefaultImpl2`, `genesyspp_driver`: current expression
  parser infrastructure, not intended to become the Gro parser directly.
- Persistence serializers and `PluginInformation` templates: likely affected by
  new plugin fields.
- GUI property editing: future area for editing Gro source text.

## Current State Summary

- The GenESyS branch `WiP20261` is cloned at
  `/home/rafaelcancian/CLionProjects/Genesys-Simulator`.
- The clone was clean when this coordination memory was created.
- Existing documentation and wiki describe GenESyS as a multi-domain,
  plugin-oriented simulator.
- Current parser infrastructure is expression-oriented and model-aware.
- Current plugin registration primarily uses `PluginConnectorDummyImpl1`, a
  static in-binary connector table.

## Main Decisions Already Taken

- Start with `BacteriaColony` rather than individual `Bacteria` as the first
  product component.
- Represent Gro source as reusable model data, likely `GroProgram`, so multiple
  colonies or bacteria can share it.
- Keep Gro parser/helper code under plugin-side folders, likely near bacteria
  components, instead of in the kernel.
- Aim for full Gro syntax recognition when practical, with intentionally partial
  semantics allowed for unsupported constructs.
- Begin with discrete grid/cellular-automata style space, preserving a path to
  future continuous 2D space.
- Treat colony biological time as internal to the colony initially; it should be
  synchronized across bacteria inside the colony and analogous to Gro time, but
  independent from `ModelSimulation::getSimulatedTime()` unless later hybrid
  integration requires alignment.

## Open Pending Items

- Validate the best folder layout for `BacteriaColony`, `GroProgram`, parser
  helper classes, and spatial support against current CMake/plugin patterns.
- Decide whether initial bacteria are internal colony state objects or separate
  `ModelComponent` instances. Current working preference: internal state first,
  separate components later if needed.
- Define the first minimal Gro syntax subset and how unsupported semantic
  constructs will be reported.
- Define persistence fields for Gro source, simulation step, colony time, grid
  dimensions, and initial population.
- Identify minimal tests for parser acceptance, plugin registration, persistence,
  and one colony tick.

## Risks And Attention Points

- Static plugin registration is centralized and conflict-prone.
- Parser infrastructure may be under concurrent work by other agents.
- GUI property editing for long Gro source text may need later coordination.
- Creating many bacteria as `ModelComponent` objects may be too heavy at scale;
  a colony-managed internal population is likely safer for the first slice.
- Internal colony time must not silently conflict with global event-driven time.

## Interaction Log

### 2026-04-17 - USER - Coordination Protocol

- **Context extracted:** The user established that this agent is named `GRO` and
  must coordinate with other AI/human developers through
  `documentation/developers/communication.md` and
  `documentation/developers/GRO_context.md`.
- **Decision:** Create and maintain the required coordination files in English.
- **Action requested:** Inspect the repository, read existing coordination files
  if present, update/create them as needed, and return only a grounded technical
  plan before product implementation.
- **Response/action:** Created the coordination files and recorded the initial
  planned Gro integration scope. Product code remains unchanged.
- **Next steps:** Present a technical plan for the first Gro/BacteriaColony slice
  and wait for user approval before modifying product code.

### 2026-04-17 - GRO - Repository Context Setup

- **Theme:** mandatory persistent coordination setup.
- **Context extracted:** `documentation/developers/` did not exist in the local
  clone. The working tree was clean before creating the coordination files.
- **Decision:** create the required files and stage them for Git tracking.
- **Action taken:** added `communication.md` and `GRO_context.md`; no product
  code was changed.
- **Next steps:** wait for user approval before implementing the first Gro
  plugin slice.

### 2026-04-17 - USER - Implementation Approval

- **Theme:** proceed with the first approved Gro integration slice.
- **Context extracted:** The user approved continuing after the grounded plan and
  repository inspection.
- **Decision:** implement a small, reviewable plugin-only slice, avoiding kernel
  changes.
- **Action requested:** proceed with the implementation.
- **Planned response/action:** add `GroProgram`, add `BacteriaColony`, register
  both in the static connector, add focused tests, run available validation, and
  update coordination records.
- **Open hypotheses:** Gro syntax handling remains intentionally permissive until
  a real parser/helper is introduced under plugin-side code.

### 2026-04-17 - GRO - Initial Gro Plugin Slice Implemented

- **Theme:** first concrete Gro/BacteriaColony plugin implementation.
- **Context extracted:** Existing CMake globbing automatically includes direct
  `source/plugins/data/*.cpp` files and recursively includes
  `source/plugins/components/**/*.cpp`. Static discovery still depends on
  `PluginConnectorDummyImpl1`.
- **Decision:** keep the first slice plugin-only and avoid kernel changes.
- **Action taken:** created `GroProgram` as a reusable `ModelDataDefinition`
  storing Gro source text; created `BacteriaColony` as a `ModelComponent` with a
  `GroProgram` reference, internal colony clock, simulation step, initial
  population, and discrete grid dimensions; registered `groprogram.so` and
  `bacteriacolony.so` in `PluginConnectorDummyImpl1`; added runtime plugin
  manager coverage.
- **Response/result:** focused test executable
  `genesys_test_runtime_pluginmanager` builds and passes 6/6 tests, including
  creating both new plugins and advancing colony time from 2.0 to 2.25.
- **Validation limitation:** the full `tests-kernel-unit-run` target is blocked
  by an unrelated compile failure in `test_support_persistence.cpp` where
  `FakeModelPersistenceB` lacks `setHasChanged(bool)`.
- **Next steps:** design the Gro parser/helper boundary, define the accepted Gro
  grammar subset versus full syntax recognition, and decide whether colony
  stepping should be driven by GenESyS future events or remain explicit/API-based
  for the next slice.

### 2026-04-17 - USER - Dedicated Branch Workflow

- **Theme:** isolate GRO work from the shared base branch.
- **Context extracted:** The shared base branch is `WiP20261`; this agent must
  now work only on `WiP20261_GRO`, using the existing `origin` remote.
- **Decision:** use `WiP20261_GRO` as the exclusive GRO development branch,
  publish it with upstream tracking, and never merge it back into `WiP20261`
  without an explicit user request.
- **Action requested:** check status, return to/synchronize `WiP20261`, create
  or reuse `WiP20261_GRO`, publish it, and continue development only there.
- **Response/action:** stashed the pending GRO changes, fast-forwarded local
  `WiP20261` to `origin/WiP20261`, created and published `WiP20261_GRO`, then
  applied the stash on `WiP20261_GRO`.
- **Conflict resolution:** one conflict occurred in
  `source/plugins/PluginConnectorDummyImpl1.cpp`. It was resolved by keeping the
  upstream biological plugin registrations and adding the GRO plugin
  registrations.
- **Next steps:** commit the coordination update and the Gro plugin slice in
  small commits, validate, fetch/merge from `origin/WiP20261`, and push
  `WiP20261_GRO`.

### 2026-04-17 - GRO - Branch-Local Commits And Validation

- **Theme:** first clean `WiP20261_GRO` commits.
- **Context extracted:** `origin/WiP20261` now contains upstream biological data
  plugins (`BioNetwork`, `BioParameter`, `BioReaction`, `BioSpecies`) and a fix
  for the earlier persistence unit-test compile blocker.
- **Decision:** keep branch history split into small commits: one coordination
  commit and one implementation commit.
- **Action taken:** committed `documentation/developers/communication.md` and
  `documentation/developers/GRO_context.md` as `41666d6f`; committed
  `GroProgram`, `BacteriaColony`, static connector registration, and runtime
  plugin-manager tests as `b42c572f`.
- **Validation:** fetched `origin`, merged `origin/WiP20261` into
  `WiP20261_GRO` with no new changes, ran `cmake --preset tests-kernel-unit`,
  and ran `cmake --build --preset tests-kernel-unit-run` successfully.
- **Publication:** pushed `WiP20261_GRO` to `origin`; the branch tracks
  `origin/WiP20261_GRO`. The temporary stash used during the branch split was
  dropped after the work was committed.
- **Next steps:** continue all future GRO development on `WiP20261_GRO` only;
  fetch and merge `origin/WiP20261` before each important push.

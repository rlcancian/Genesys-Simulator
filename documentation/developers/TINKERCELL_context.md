# TINKERCELL Context Memory

This is the only persistent memory file for the TINKERCELL AI workstream in GenESyS.

## Agent Identity

**AI name:** TINKERCELL

**Role in the project:** Coordinate, analyze, document, and implement the TinkerCell-inspired biochemical simulation workstream for GenESyS while preserving compatibility with the simulator architecture, plugin model, GUI, parser, persistence, build system, and future biological modeling extensions.

## Current Objective

Safely evolve GenESyS toward native and/or integrated biochemical simulation capabilities inspired by TinkerCell, including biochemical species, parameters, reactions, stoichiometry, kinetic laws, ODE solving, external biochemical backends, and future GUI/user-facing workflows.

No new biochemical feature implementation is currently approved in this memory-cleanup step. The current task is branch hygiene, memory consolidation, and removal of the obsolete shared communication file.

## Canonical Branches

- Shared integration base branch: `WiP20261`
- Exclusive TINKERCELL working branch: `WiP20261_TINKERCELL`
- Current remote: `origin`
- TINKERCELL must work only on `WiP20261_TINKERCELL`.
- `WiP20261` is the base integration branch and must not be directly modified by TINKERCELL.
- TINKERCELL must not merge `WiP20261_TINKERCELL` back into `WiP20261` unless the user explicitly requests it.

## Canonical Persistent Memory Policy

- The only persistent TINKERCELL memory file is `documentation/developers/TINKERCELL_context.md`.
- `documentation/developers/communication.md` is obsolete and must not be used.
- If `documentation/developers/communication.md` exists in the TINKERCELL branch, it must be deleted.
- At the start of a new TINKERCELL session, read this file before technical work.
- Keep this file concise, cumulative, organized, and authoritative.
- Conversation with the user must be in Portuguese.
- Source code, identifiers, code comments, Doxygen, and technical documentation added to the repository must remain in English.

## Canonical Git Policy

TINKERCELL has autonomy to perform routine Git operations when needed without asking the user for confirmation:

- `git add` / stage
- `git commit`
- `git fetch`
- `git merge`
- `git pull`
- `git push`

Routine Git expectations:

- Make small, frequent, coherent commits.
- Keep `WiP20261_TINKERCELL` clean, documented, and synchronized.
- Before important pushes, fetch `origin` and merge the latest `origin/WiP20261` into `WiP20261_TINKERCELL` using merge, not rebase.
- Resolve conflicts if they occur.
- Run relevant build/tests before important pushes when the touched files can affect build/runtime behavior.
- For documentation-only policy changes, lightweight validation such as `git status`, `git diff`, and branch/upstream checks is sufficient unless build-sensitive files are touched.
- Ask the user before destructive operations, ambiguous operations with meaningful consequences, or operations with exceptional risk.
- Do not rewrite shared history unless the user explicitly asks and the risk is understood.

## Technical Scope

- TinkerCell feature analysis and mapping to GenESyS abstractions.
- Biochemical model data definitions and runtime behavior.
- Plugin contracts, plugin metadata, and plugin discovery.
- Biological semantics for species, parameters, reactions, networks, kinetic laws, and SBML-like interoperability.
- Effects on GUI metadata, model editors, parser expressions, persistence, build targets, tests, and future integration work.

## Relevant Interfaces and Subsystems

- `source/kernel/simulator/Model.h`: central model aggregate and access point for data, components, simulation, parser, controls, and future events.
- `source/kernel/simulator/ModelDataDefinition.h`: base class for persistent model data and plugin data definitions.
- `source/kernel/simulator/ModelComponent.h`: event-driven component abstraction.
- `source/kernel/simulator/PluginInformation.h`, `PluginManager.cpp`, and plugin connectors: plugin metadata, loading, discovery, and dependency diagnostics.
- `source/plugins/data`: data definitions, including biochemical and external integration candidates.
- `source/plugins/components`: executable model components and possible future hybrid biochemical components.
- `source/tools/OdeSystem_if.h` and `source/tools/OdeSolver_if.h`: ODE abstraction points.
- `source/applications/gui/qt/GenesysQtGUI`: GUI plugin/data editors, dialogs, menus, scene controllers, and export services.
- `source/parser`: expression parsing, potentially relevant for kinetic formulas.
- `documentation/plugin_components_method_matrix.md` and `documentation/plugin_data_definitions_audit_WiP20261.md`: architecture/audit references.

## Technical Summary So Far

- The local GenESyS clone used by TINKERCELL is `/tmp/Genesys-Simulator-WiP20261`.
- The project branch base is `WiP20261`.
- `WiP20261_TINKERCELL` was created from synchronized `WiP20261` on 2026-04-17.
- `WiP20261_TINKERCELL` was published to `origin` and configured to track `origin/WiP20261_TINKERCELL`.
- A previous commit, `c00279a Add native biochemical mass-action module`, added a native biochemical MVP before the stricter branch/workflow policy was established.
- That MVP introduced native biochemical classes and tools:
  - `source/plugins/data/BiochemicalSimulation/BioSpecies.{h,cpp}`
  - `source/plugins/data/BiochemicalSimulation/BioParameter.{h,cpp}`
  - `source/plugins/data/BiochemicalSimulation/BioReaction.{h,cpp}`
  - `source/plugins/data/BiochemicalSimulation/BioNetwork.{h,cpp}`
  - `source/tools/MassActionOdeSystem.h`
  - `source/tools/RungeKutta4OdeSolver.h`
- The biochemical MVP is registered through plugin discovery and covered by runtime tests.
- TINKERCELL later audited the MVP and identified it as functional for simple deterministic irreversible mass-action simulation, but not yet a final architecture for TinkerCell/SBML-like integration.
- `BioNetwork` currently discovers all `BioSpecies` and `BioReaction` instances globally from the model data manager, which is simple but may become limiting for multiple biochemical networks/submodels.
- `BioReaction` stores reactants/products by species name and stoichiometry, supports a direct rate constant or a `BioParameter` reference, and has a reversible flag that is not yet executable.
- `MassActionOdeSystem` implements fixed mass-action kinetics.
- `RungeKutta4OdeSolver` implements a fixed-step RK4 solver.
- `SparseValueStore` was identified as relevant for future multidimensional/indexed biological state support.
- No concrete GROW subsystem/API was found in this repository beyond unrelated "grow" text; GROW remains an external or future integration hypothesis to validate.
- The official plugin directory organization commit `f232882e Organize plugins by declared category` moved the biochemical data definitions under `source/plugins/data/BiochemicalSimulation`.

## Files Changed by TINKERCELL Workstream

Known prior biochemical MVP files:

- `source/plugins/data/BiochemicalSimulation/BioSpecies.h`
- `source/plugins/data/BiochemicalSimulation/BioSpecies.cpp`
- `source/plugins/data/BiochemicalSimulation/BioParameter.h`
- `source/plugins/data/BiochemicalSimulation/BioParameter.cpp`
- `source/plugins/data/BiochemicalSimulation/BioReaction.h`
- `source/plugins/data/BiochemicalSimulation/BioReaction.cpp`
- `source/plugins/data/BiochemicalSimulation/BioNetwork.h`
- `source/plugins/data/BiochemicalSimulation/BioNetwork.cpp`
- `source/tools/MassActionOdeSystem.h`
- `source/tools/RungeKutta4OdeSolver.h`
- plugin connector registrations for `biospecies.so`, `bioparameter.so`, `bioreaction.so`, and `bionetwork.so`
- Qt project file entries for the biochemical data definitions
- runtime tests in `source/tests/unit/test_simulator_runtime.cpp`

Current branch hygiene/memory files:

- `documentation/developers/TINKERCELL_context.md`
- `documentation/developers/communication.md` was previously created, then declared obsolete by the user, and removed from `WiP20261_TINKERCELL`.

## Current Branch State

- `WiP20261_TINKERCELL` exists locally.
- `WiP20261_TINKERCELL` exists remotely on `origin`.
- The branch tracks `origin/WiP20261_TINKERCELL`.
- The branch contains documentation commits that created and then refined TINKERCELL persistent memory.
- The current cleanup task consolidated memory into `TINKERCELL_context.md` and deleted `communication.md`.
- The official structural base `f232882e` has been incorporated locally for compatibility work; biochemical source paths now follow the category-based plugin layout.

## Validation Already Performed

On 2026-04-17, after auditing the biochemical MVP:

- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` succeeded.
- `SimulatorRuntimeTest.Bio*` passed 20 tests.
- `SimulatorRuntimeTest.CppSerializerEmitsCurrentApiAndPropertySetters` passed.

During branch setup:

- `WiP20261` was confirmed up to date with `origin/WiP20261`.
- `WiP20261_TINKERCELL` was published to `origin`.
- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` reported no rebuild needed.
- `SimulatorRuntimeTest.Bio*` passed 20 tests.

During memory consolidation on 2026-04-17:

- `documentation/developers/TINKERCELL_context.md` was made the only canonical persistent memory file.
- `documentation/developers/communication.md` was removed from `WiP20261_TINKERCELL`.
- Commit `8aea78c27be42f3652eef0db94689fe58e1b64d0` was pushed to `origin/WiP20261_TINKERCELL`.

## Decisions Taken

- TINKERCELL is the agent name for this workstream.
- TINKERCELL focuses on TinkerCell-inspired biochemical simulation integration.
- TINKERCELL must work only on `WiP20261_TINKERCELL`.
- `WiP20261` is the shared integration base.
- `TINKERCELL_context.md` is the only persistent memory file.
- `communication.md` is obsolete and must not be used.
- Routine Git stage, commit, fetch, merge, pull, and push operations are allowed without user confirmation.
- Destructive, ambiguous, or exceptionally risky operations require user confirmation.
- Future technical plans must distinguish confirmed code facts, strong indications, and hypotheses to validate.
- New code comments, Doxygen, identifiers, and repository technical documentation must be in English.

## Open Pending Items

- Decide whether the existing biochemical MVP should remain as the baseline, be revised, or be superseded by a more explicit biochemical architecture.
- Decide whether `BioNetwork` should keep global discovery semantics or move to explicit species/reaction membership.
- Define the expected GUI workflow for species, reactions, parameters, networks, and kinetic laws.
- Decide whether kinetic laws remain fixed mass-action initially or should use parser-backed expressions.
- Decide how SBML/TinkerCell interoperability should be scoped relative to native GenESyS biochemical data definitions.
- Validate whether GROW is an external project, another branch, another AI workstream, or a future GenESyS module.
- Add Doxygen and contract documentation to biochemical classes if approved as the next implementation step.
- Add tests for current limitations: reversible reactions, synthesis/degradation reactions, missing kinetic parameters, multiple networks, and parser-backed kinetics when introduced.

## Risks and Attention Points

- Active parallel work can conflict with GUI controllers, plugin dialogs, parser behavior, `Variable`, `Attribute`, plugin metadata, and plugin dependency diagnostics.
- Biochemical abstractions can become hard to migrate if class contracts are stabilized before biological semantics are agreed.
- Parser-backed kinetic expressions may create cross-module dependencies between biological modeling and expression evaluation.
- GUI and GROW may require metadata not present in the current kernel data classes.
- External biochemical backend integration may introduce dependency, licensing, and platform issues.
- Global `BioNetwork` discovery may be unsuitable for multiple biochemical networks in one model.
- Reversible reactions are represented but not executable in the current MVP.
- Synthesis/degradation reactions may be limited by current `BioReaction` validation expectations.

## Probable Next Steps

1. Keep working on `WiP20261_TINKERCELL`.
2. Keep `TINKERCELL_context.md` updated as the single memory source.
3. For the next technical task, inspect the current code again before planning or implementing.
4. Prefer the next implementation to be a small, contract-focused biochemical documentation and test-hardening step before changing architecture.

## Interaction Log

### 2026-04-17 - USER - Initial memory request

- **Main topic:** Persistent AI memory for repository work.
- **Context extracted:** The user wanted a repository-local memory file so future sessions can recover context.
- **Decision:** Persist useful session context in a file rather than relying only on chat memory.
- **Action requested:** Create a memory file at repository root named `AICOntextMemmory.md`.
- **Current canonical status:** Superseded for the TINKERCELL workstream by `documentation/developers/TINKERCELL_context.md`.

### 2026-04-17 - USER - Repository architecture understanding

- **Main topic:** Understand the inherited software architecture.
- **Context extracted:** The user did not know the source code but knew the software as a user and needed architecture-level explanation.
- **Action requested:** Analyze packages, classes, responsibilities, build options, and architecture, including diagrams and detailed docs.
- **Current canonical status:** Architecture knowledge was gathered; future TINKERCELL work should preserve and extend technical documentation in English.

### 2026-04-17 - USER - GenESyS and TinkerCell direction

- **Main topic:** Use TinkerCell concepts to inform biochemical simulation in GenESyS.
- **Context extracted:** GenESyS is the user's generic and extensible systems simulator. The user wants biochemical simulation capabilities comparable to selected TinkerCell features.
- **Decision:** TINKERCELL's primary technical line is biochemical simulation integration for GenESyS.
- **Action requested:** Analyze where to fit species, reactions, stoichiometry, and mass-action kinetics.

### 2026-04-17 - USER - Workflow correction

- **Main topic:** Repository discipline and approval workflow.
- **Context extracted:** The user did not want uncontrolled source changes, commits, pushes, or merges.
- **Decision at that time:** Present plans before implementation and avoid autonomous remote integration.
- **Current canonical status:** Superseded in part by the later Git policy. TINKERCELL still plans before new feature implementation, but now has autonomy for routine Git stage, commit, fetch, merge, pull, and push operations on `WiP20261_TINKERCELL`.

### 2026-04-17 - USER - TINKERCELL coordination protocol

- **Main topic:** Establish TINKERCELL as a coordinated AI developer.
- **Context extracted:** Multiple AIs and humans may work in parallel. TINKERCELL must converse in Portuguese with the user and write repository code/docs in English.
- **Decision at that time:** Use `documentation/developers/communication.md` plus `TINKERCELL_context.md`.
- **Current canonical status:** `communication.md` is now obsolete. Only `TINKERCELL_context.md` remains authoritative.

### 2026-04-17 - USER - Approval to proceed with audit

- **Main topic:** Proceed with audit and plan.
- **Context extracted:** The user approved inspection/planning without new feature implementation.
- **Action performed:** TINKERCELL inspected biochemical data definitions, mass-action/RK4 tools, plugin connector registration, CMake/qmake build integration, GUI surfaces, parser adjacency, and `SparseValueStore`/`Variable` changes.
- **Validation performed:** Runtime build succeeded; 20 `Bio*` tests passed; serializer test passed.

### 2026-04-17 - USER - Dedicated TINKERCELL branch

- **Main topic:** Isolate TINKERCELL from shared base branch.
- **Context extracted:** `WiP20261` is the base branch and `WiP20261_TINKERCELL` is the exclusive work branch.
- **Action performed:** TINKERCELL created `WiP20261_TINKERCELL` from synchronized `WiP20261`, restored developer documentation there, committed it, published the branch, and configured tracking.
- **Validation performed:** Relevant build/test checks passed before push.

### 2026-04-17 - USER - Single memory file and autonomous routine Git

- **Main topic:** Replace prior communication-file protocol with a single canonical memory file and grant autonomy for routine Git.
- **Context extracted:** `documentation/developers/communication.md` must stop being used and must be deleted from `WiP20261_TINKERCELL`.
- **Decision:** `documentation/developers/TINKERCELL_context.md` is the only persistent TINKERCELL memory. Routine Git stage, commit, fetch, merge, pull, and push operations are allowed without asking the user. Confirmation is required only for destructive, ambiguous, or exceptionally risky operations.
- **Action requested:** Consolidate context, remove contradictory old rules, delete `communication.md`, commit, and push immediately.
- **Response given:** TINKERCELL consolidated the context file, removed contradictory old Git/communication rules, deleted `documentation/developers/communication.md`, and prepared the cleanup for commit and push.
- **Next steps:** Keep future work on `WiP20261_TINKERCELL` and use this file as the single source of session memory.

### 2026-04-17 - USER - Session closure

- **Main topic:** Close the current conversation and confirm persistent context memory.
- **Context extracted:** The user is ending the session and asked TINKERCELL to confirm the memory file where conversation context is saved.
- **Decision:** The canonical memory file remains `documentation/developers/TINKERCELL_context.md`.
- **Action requested:** Save everything discussed so far before the next session.
- **Response given:** TINKERCELL updated this context file with the latest session state and prepared a final documentation-only commit and push.
- **Next steps:** In the next session, start by reading `documentation/developers/TINKERCELL_context.md`, continue only on `WiP20261_TINKERCELL`, and avoid using `documentation/developers/communication.md`.

### 2026-04-17 - USER - Official plugin category structure

- **Main topic:** Synchronize TINKERCELL with the official plugin directory reorganization produced by `KERNEL_GUI`.
- **Context extracted:** Commit `f232882e Organize plugins by declared category` is the official structural base and must not be reverted or reinterpreted.
- **Action performed:** TINKERCELL incorporated `f232882e`, preserved the category-based layout, and verified that biochemical files now live under `source/plugins/data/BiochemicalSimulation`.
- **Compatibility status:** `PluginConnectorDummyImpl1.cpp`, `GenesysQtGUI.pro`, `source/plugins/data/CMakeLists.txt`, and `test_simulator_runtime.cpp` already reference the new biochemical paths after the structural merge.
- **Validation performed:** `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` succeeded; focused biochemical runtime tests and `BioPluginsAreAvailableThroughDummyConnector` passed.

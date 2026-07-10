# TINKERCELL Context Memory

Historical auxiliary context for the TINKERCELL AI workstream in GenESyS.

The canonical active memory is now `TINKERCELL_ContextMemory.md` at the repository root of the `WiP20261/Genesys-Simulator` checkout. This file remains as developer documentation and historical context only.

## Agent Identity

**AI name:** TINKERCELL

**Role in the project:** Coordinate, analyze, document, and implement the TinkerCell-inspired biochemical simulation workstream for GenESyS while preserving compatibility with the simulator architecture, plugin model, GUI, parser, persistence, build system, and future biological modeling extensions.

## Current Objective

Safely evolve GenESyS toward native and/or integrated biochemical simulation capabilities inspired by TinkerCell, including biochemical species, parameters, reactions, stoichiometry, kinetic laws, ODE solving, external biochemical backends, and future GUI/user-facing workflows.

The active workstream is the native biochemical simulation line: keep GenESyS generic and extensible while adding biochemical model definitions, deterministic simulation, structured result capture, analysis hooks, and later GUI/domain-specific extensions. User confirmation is required before starting broad new phases; when the user explicitly requests implementation, proceed within the requested scope.

## Canonical Branches

- Shared integration base branch: `WiP20261`
- Current implementation branch for this session: `WiP20261`
- Historical TINKERCELL branch: `WiP20261_TINKERCELL`
- Current remote: `origin`
- Follow the branch explicitly selected by the user for the current session.
- Do not merge, push, or rewrite history unless the user explicitly requests it.
- Treat branch instructions in the live conversation as more authoritative than historical entries in this memory.

## Historical Persistent Memory Policy

- The only active persistent TINKERCELL memory file is `TINKERCELL_ContextMemory.md` at the repository root.
- This file is no longer the active memory source.
- `documentation/developers/communication.md` is obsolete and must not be used.
- If `documentation/developers/communication.md` exists in the TINKERCELL branch, it must be deleted.
- At the start of a new TINKERCELL session, read the root `TINKERCELL_ContextMemory.md` before technical work.
- Keep this file concise, cumulative, organized, and authoritative.
- Conversation with the user must be in Portuguese.
- Source code, identifiers, code comments, Doxygen, and technical documentation added to the repository must remain in English.

## Canonical Git Policy

- Inspect `git status` before editing.
- Do not stage, commit, merge, push, pull with merge effects, or rewrite history unless the user explicitly requests it.
- Fetching and status/diff inspection are acceptable for orientation when needed.
- Never use destructive Git operations unless explicitly requested and the risk is understood.
- Run relevant build/tests after code changes when feasible, and report any pre-existing failures separately from new failures.

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

- The current local GenESyS clone inspected in this session is `/home/rafaelcancian/Laboratory/Software/Educational_Projects/GenESyS/GitHub/WiP20261/Genesys-Simulator`.
- The current branch base is `WiP20261`.
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
- TINKERCELL later audited the MVP and identified it as functional for deterministic mass-action simulation, but not yet a final architecture for TinkerCell/SBML-like integration.
- `BioNetwork` can discover all `BioSpecies` and `BioReaction` instances globally from the model data manager, which remains a compatibility fallback.
- `BioNetwork` now also supports optional explicit BioSpecies and BioReaction membership lists by name, while preserving global discovery when each list is empty for compatibility.
- `BioReaction` stores reactants/products by species name and stoichiometry, supports a direct rate constant, a `BioParameter` reference, or optional forward/reverse kinetic-law expressions.
- Reversible `BioReaction` execution is implemented and covered by runtime tests.
- `MassActionOdeSystem` implements fixed mass-action kinetics.
- `MassActionOdeSystem` now also evaluates optional reaction-level kinetic-law expressions through `BioKineticLawExpression`.
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
- `source/tools/BioKineticLawExpression.h`
- `source/tools/BioSimulationResult.h`
- `source/tools/BioSimulationAnalysis.h`
- plugin connector registrations for `biospecies.so`, `bioparameter.so`, `bioreaction.so`, and `bionetwork.so`
- Qt project file entries for the biochemical data definitions
- runtime tests in `source/tests/unit/test_simulator_runtime.cpp`

Current branch hygiene/memory files:

- `documentation/developers/TINKERCELL_context.md`
- `documentation/developers/communication.md` was previously created, then declared obsolete by the user, and removed from `WiP20261_TINKERCELL`.

## Current Branch State

- This session is working in the local `WiP20261` checkout selected by the user.
- Historical `WiP20261_TINKERCELL` notes remain in the interaction log for context only.
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

During contract/test hardening on 2026-04-17:

- BioSpecies, BioParameter, BioReaction, and BioNetwork headers received concise contract documentation.
- Runtime tests were added for missing BioParameter references, unsupported reversible reactions, and preservation of boundary/constant species during BioNetwork simulation.
- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` succeeded.
- 10 focused biochemical runtime tests passed.

During explicit BioNetwork membership implementation on 2026-04-17:

- `BioNetwork` gained persistent optional membership lists for species and reactions.
- New public APIs: `addSpecies`, `addReaction`, `clearSpecies`, `clearReactions`, `getSpeciesNames`, and `getReactionNames`.
- Empty membership lists preserve the previous global model discovery behavior.
- Explicit missing or empty references now fail coherently during validation/simulation.
- Runtime tests were added for scoped simulation, persistence round trip, and missing explicit members.
- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` succeeded.
- `SimulatorRuntimeTest.Bio*` passed 26 focused tests.

During kinetic-law expression implementation on 2026-04-17:

- TINKERCELL chose a small biochemical-specific evaluator instead of modifying the generated global GenESyS parser.
- Added `source/tools/BioKineticLawExpression.h`.
- Supported syntax: numeric literals, BioSpecies/BioParameter identifiers, parentheses, unary signs, `+`, `-`, `*`, `/`, `^`, and functions `abs`, `exp`, `log`, `sqrt`, `min`, `max`, and `pow`.
- `BioReaction` gained persistent optional `kineticLawExpression` support.
- `BioNetwork` validates kinetic-law symbols against the network species membership and available BioParameters.
- Empty `kineticLawExpression` keeps legacy mass-action behavior.
- Added runtime tests for kinetic-law persistence, expression-based simulation, and rejection of species referenced outside the explicit network membership.
- `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` succeeded.
- `SimulatorRuntimeTest.Bio*` passed 29 focused tests.
- `SimulatorRuntimeTest.MassActionOdeSystem*` and `SimulatorRuntimeTest.RungeKutta4OdeSolver*` passed 2 focused tests.

During BioSimulationResult implementation on 2026-04-21:

- Baseline `cmake --build --preset tests-kernel-unit-run` failed before new edits because two unrelated `RSimulator` category tests expected `External statistical integration` while plugin metadata returned `ExternalIntegration`.
- Added `source/tools/BioSimulationResult.h` as the structured native biochemical time-course result contract.
- `BioNetwork` now records the initial sample and every completed ODE step into `BioSimulationResult`.
- `BioNetwork` exposes `getLastSimulationResult()` and `getSpeciesTimeCourseDataset(...)` for analysis/GUI integration through `SimulationResultsDataset`.
- `SimulatorRuntimeTest.Bio*` passed 41 focused tests after the change.
- The unrelated `RSimulator` category tests were aligned with the current plugin category `ExternalIntegration`, matching the organized plugin directory/category convention.
- Full `cmake --build --preset tests-kernel-unit-run` passed after the `BioSimulationResult` work and `RSimulator` category-test alignment.

During non-GUI biochemical analysis implementation on 2026-04-21:

- Added `source/tools/BioSimulationAnalysis.h` as a reusable non-GUI analysis layer over `MassActionOdeSystem` and `BioSimulationResult`.
- `MassActionOdeSystem` now exposes forward, reverse, and net reaction-rate evaluators so analysis code can reuse the same kinetic-law and mass-action semantics as the ODE solver.
- `BioSimulationAnalysis` supports stoichiometry matrix generation, reaction-rate time-course generation, conversion of reaction-rate series to `SimulationResultsDataset`, steady-state checks from species derivatives, and local finite-difference parameter-sensitivity scans.
- `BioNetwork` exposes `buildOdeSystemForAnalysis(...)`, `getStoichiometryMatrix(...)`, `getReactionRateTimeCourse(...)`, `checkLastSampleSteadyState(...)`, and `scanLocalParameterSensitivity(...)`.
- Added runtime tests for stoichiometry matrix construction, reaction-rate time-course output, steady-state checking, and local parameter-sensitivity scans.
- Full `cmake --build --preset tests-kernel-unit-run` passed after this analysis work.

## Decisions Taken

- TINKERCELL is the agent name for this workstream.
- TINKERCELL focuses on TinkerCell-inspired biochemical simulation integration.
- Work on the branch explicitly selected by the user for the active session; this session uses `WiP20261`.
- `TINKERCELL_context.md` is the only persistent memory file.
- `communication.md` is obsolete and must not be used.
- Do not stage, commit, merge, push, or rewrite history without explicit user request.
- Future technical plans must distinguish confirmed code facts, strong indications, and hypotheses to validate.
- New code comments, Doxygen, identifiers, and repository technical documentation must be in English.

## Open Pending Items

- Define the future GUI plugin/extension architecture so domain-specific graphical tools are only available when their domain plugins are loaded.
- Validate whether GROW is an external project, another branch, another AI workstream, or a future GenESyS module.
- Add chart-oriented visual rendering for biochemical analysis reports (currently text-oriented).
- Expand SBML coverage for broader MathML and package interoperability while preserving deterministic native biochemical behavior.

## Risks and Attention Points

- Active parallel work can conflict with GUI controllers, plugin dialogs, parser behavior, `Variable`, `Attribute`, plugin metadata, and plugin dependency diagnostics.
- Biochemical abstractions can become hard to migrate if class contracts are stabilized before biological semantics are agreed.
- Parser-backed kinetic expressions may create cross-module dependencies between biological modeling and expression evaluation.
- GUI and GROW may require metadata not present in the current kernel data classes.
- External biochemical backend integration may introduce dependency, licensing, and platform issues.
- Global `BioNetwork` discovery remains only as compatibility fallback; explicit membership is safer for multiple biochemical networks in one model.
- Explicit `BioNetwork` membership is now available at API/persistence/runtime level, but GUI/editor controls for editing those lists are not implemented yet.
- Kinetic-law expressions are available at API/persistence/runtime level, but GUI/editor controls for editing them are not implemented yet.
- Reversible, synthesis, and degradation reactions are executable and covered by tests.
- Missing kinetic parameter references are explicitly covered by tests.
- Boundary-condition and constant species preservation is now explicitly covered by tests.
- The full kernel-unit test preset is currently passing on branch `WiP20261`.

## Probable Next Steps

1. Add SBML/TinkerCell interoperability decisions and implementation scope for import/export.
2. Add GUI/editor workflows for BioNetwork membership, BioReaction stoichiometry, and kinetic-law expressions.
3. Define the future GUI plugin/extension layer before adding domain-specific menus, windows, and graphical tools.
4. Keep `TINKERCELL_context.md` updated as the single memory source.

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

### 2026-04-17 - USER - Resume phased TinkerCell integration

- **Main topic:** Continue TinkerCell-inspired biochemical integration without plugin directory reorganization.
- **Context extracted:** The next logical phase from memory was a small, contract-focused documentation and test-hardening step before changing architecture.
- **Action performed:** TINKERCELL documented the BioSpecies, BioParameter, BioReaction, and BioNetwork contracts in their headers and added focused tests for known current limitations/guarantees.
- **Validation performed:** `genesys_test_simulator_runtime` rebuilt successfully and 10 focused biochemical runtime tests passed.
- **Next phase candidate:** Decide whether to improve network membership semantics, kinetic-law expressiveness, or SBML/TinkerCell interoperability scope; do not proceed without explicit user confirmation.

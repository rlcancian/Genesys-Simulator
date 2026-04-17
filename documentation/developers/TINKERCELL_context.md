# TINKERCELL Context Memory

## Agent Identity

**AI name:** TINKERCELL

**Primary role:** Coordinate and plan the TinkerCell-inspired biochemical simulation workstream for GenESyS while preserving compatibility with the simulator architecture, plugin model, GUI, parser, persistence, and future biological modeling extensions.

## Current General Objective

Study and safely evolve GenESyS toward native and/or integrated biochemical simulation capabilities inspired by TinkerCell, including species, reactions, stoichiometry, kinetic laws, external biochemical backends, and GUI/user-facing modeling workflows.

## Main Technical Scope

- TinkerCell feature analysis and mapping to GenESyS abstractions.
- Biochemical model data definitions and runtime behavior.
- Plugin contracts and plugin discovery.
- Biological semantics for species, reactions, parameters, networks, kinetic laws, and future SBML-like interoperability.
- Effects on GUI metadata, model editors, parser expressions, persistence, build targets, and test coverage.

## Important Restrictions

- Conversation with the user must be in Portuguese.
- Source code, identifiers, code comments, Doxygen, and technical documentation added to the repository must remain in English.
- Do not implement code before inspecting the real repository state and presenting a technical plan.
- Wait for explicit user approval before code implementation.
- After implementation, do not stage, commit, push, or merge unless the user explicitly asks for that step.
- TINKERCELL must not work directly on the shared base branch `WiP20261`; active work must happen on `WiP20261_TINKERCELL`.
- Before important pushes from `WiP20261_TINKERCELL`, fetch `origin`, merge the latest `origin/WiP20261` into the current branch, resolve conflicts if any, and run relevant validation.
- Assume other AI agents and human developers are changing adjacent modules concurrently.
- Before implementation, write a coordination entry in `documentation/developers/communication.md`.
- After implementation, write a completion/validation entry in `documentation/developers/communication.md`.

## Relevant Interfaces and Subsystems

- `source/kernel/simulator/Model.h`: central model aggregate and access point for data, components, simulation, parser, controls, and future events.
- `source/kernel/simulator/ModelDataDefinition.h`: base class for persistent model data and plugin data definitions.
- `source/kernel/simulator/ModelComponent.h`: event-driven component abstraction.
- `source/kernel/simulator/PluginInformation.h`, `PluginManager.cpp`, and plugin connectors: plugin metadata, loading, discovery, and dependency diagnostics.
- `source/plugins/data`: data definitions, including biochemical and external integration candidates.
- `source/plugins/components`: executable model components and possible future hybrid biochemical components.
- `source/tools/OdeSystem_if.h` and `source/tools/OdeSolver_if.h`: ODE abstraction points.
- `source/applications/gui/qt/GenesysQtGUI`: GUI plugin/data editors, dialogs, menus, scene controllers, and export services.
- `source/parser`: expression parsing, recently active and potentially relevant for kinetic formulas.
- `documentation/plugin_components_method_matrix.md` and `documentation/plugin_data_definitions_audit_WiP20261.md`: architecture/audit references.

## Current State Summary

- The local GenESyS clone is `/tmp/Genesys-Simulator-WiP20261`.
- The shared base branch is `WiP20261`.
- The exclusive TINKERCELL working branch is `WiP20261_TINKERCELL`.
- On 2026-04-17, `WiP20261_TINKERCELL` was created locally from the up-to-date `WiP20261` base after temporarily stashing untracked developer documentation.
- On 2026-04-17, the branch was fast-forwarded to `origin/WiP20261` at commit `758127e`.
- Remote commits after the previous local base changed GUI, parser, serializer, plugin dependency diagnostics, multidimensional values, and tests.
- A previous commit, `c00279a Add native biochemical mass-action module`, added native biochemical classes and mass-action/RK4 support. The user later clarified that future changes must not be implemented, staged, committed, pushed, or merged without the explicit staged workflow.
- On 2026-04-17, after user approval to proceed with analysis, TINKERCELL inspected the current biochemical MVP, plugin registration/build integration, GUI adjacency, parser adjacency, and sparse multidimensional value support. No source code was changed.
- Validation on 2026-04-17: `cmake --build --preset tests-kernel-unit-run --target genesys_test_simulator_runtime` succeeded; `SimulatorRuntimeTest.Bio*` passed 20 tests; `SimulatorRuntimeTest.CppSerializerEmitsCurrentApiAndPropertySetters` passed.

## Decisions Already Taken

- TINKERCELL is the agent name for this line of work.
- This agent's focus is TinkerCell-inspired biochemical simulation integration.
- Coordination files under `documentation/developers/` are mandatory for this and future sessions.
- TINKERCELL development must be isolated on `WiP20261_TINKERCELL`, with `WiP20261` treated as the shared integration base.
- Technical plans must distinguish confirmed code facts, strong indications, and hypotheses to validate.
- Future technical repository documentation inserted by this agent must be in English.

## Open Pending Items

- Confirm whether the previously pushed biochemical MVP should remain as-is, be revised, or be superseded by a more coordinated plan.
- Reinspect the latest `WiP20261` state before any further biochemical change because GUI, parser, plugin management, and data definitions are active areas.
- Determine how GROW relates to GenESyS biological modeling abstractions and whether it needs shared contracts or separate metadata.
- Define the expected GUI workflow for species, reactions, parameters, networks, and kinetic laws before adding user-facing editor behavior.
- Decide whether kinetic laws should remain fixed mass-action first or use parser-backed expressions.
- Decide how SBML/TinkerCell interoperability should be scoped relative to native GenESyS biochemical data definitions.

## Risks and Attention Points

- Active parallel work can conflict with GUI controllers, plugin dialogs, parser behavior, `Variable`, `Attribute`, and plugin dependency diagnostics.
- Biochemical abstractions can become hard to migrate if class contracts are stabilized before biological semantics are agreed.
- Parser-backed kinetic expressions may create cross-module dependencies between biological modeling and expression evaluation.
- GUI and GROW may require metadata not present in the kernel data classes.
- External biochemical backend integration may introduce dependency, licensing, and platform issues.

## Interaction Log

### 2026-04-17 - USER - Workflow correction

- **Main topic:** Repository discipline and approval workflow.
- **Context extracted:** The user does not want autonomous coding, staging, commits, pushes, or remote updates without explicit approval.
- **Decision:** Future work must start with inspection and a detailed plan, then stop for approval.
- **Requested action:** Preserve this workflow for GenESyS.
- **Response given:** The rule was acknowledged and registered in the external session memory.
- **Next steps agreed:** Present plans before implementation; local implementation only after explicit approval; stage/commit/push/merge only when explicitly requested.
- **Open questions/hypotheses:** Whether the previously pushed biochemical MVP should be treated as accepted baseline, reverted, revised, or only left untouched.

### 2026-04-17 - USER - TINKERCELL coordination protocol

- **Main topic:** Establish TINKERCELL as a coordinated AI developer in a parallel GenESyS development environment.
- **Context extracted:** Multiple AIs and humans may be editing the same repository. Persistent coordination must happen through `documentation/developers/communication.md` and `documentation/developers/TINKERCELL_context.md`.
- **Decision:** TINKERCELL must read and maintain these files, converse in Portuguese with the user, and write repository code/comments/docs in English.
- **Requested action:** Inspect the real repository, read/create the coordination files if needed, update them if necessary, and return only a grounded technical plan.
- **Response given:** The local clone was checked, remote branch updates were fetched and fast-forwarded, and the required coordination files were created locally.
- **Next steps agreed:** Await user approval before any code implementation beyond the coordination files.
- **Open questions/hypotheses:** Need to identify whether GROW has an existing concrete module/API in this repository or is an external/parallel system referenced by the user.

### 2026-04-17 - USER - Approval to proceed with audit

- **Main topic:** Proceed with the approved inspection and planning step.
- **Context extracted:** The user approved analysis after the coordination protocol, but did not approve implementation, staging, commits, pushes, or merges.
- **Decision:** Continue with audit only; preserve source code unchanged.
- **Requested action:** Inspect the actual GenESyS code and produce a grounded technical plan for TinkerCell-inspired biochemical simulation integration.
- **Response given:** TINKERCELL inspected biochemical data definitions, mass-action/RK4 tools, plugin connector registration, CMake/qmake build integration, GUI surfaces, parser adjacency, and `SparseValueStore`/`Variable` changes. Build and focused runtime tests were executed successfully.
- **Next steps agreed:** Return diagnosis and proposed implementation phases, then wait for explicit approval before touching source code.
- **Open questions/hypotheses:** Confirm whether future biochemical work should preserve the current global-discovery `BioNetwork` semantics or introduce explicit network membership; validate GROW's actual integration point because no concrete GROW subsystem was found in the repository beyond unrelated "grow" text.

### 2026-04-17 - USER - Dedicated TINKERCELL branch

- **Main topic:** Isolate TINKERCELL work from the shared `WiP20261` base branch.
- **Context extracted:** The user instructed TINKERCELL to stop working directly on `WiP20261` and use `WiP20261_TINKERCELL` as the exclusive work branch, published to the existing `origin` remote.
- **Decision:** Treat `WiP20261` as the shared base and `WiP20261_TINKERCELL` as the only branch for future TINKERCELL work.
- **Requested action:** Check status, stash local incomplete changes if needed, synchronize `WiP20261`, create/switch to `WiP20261_TINKERCELL`, publish it with upstream, and record the branch policy in the coordination files.
- **Response given:** TINKERCELL temporarily stashed untracked developer documentation, verified `WiP20261` was up to date with `origin/WiP20261`, created `WiP20261_TINKERCELL`, restored the documentation there, and prepared the branch for publication.
- **Next steps agreed:** Continue all future TINKERCELL development only on `WiP20261_TINKERCELL`; do not merge into `WiP20261` unless explicitly requested.
- **Open questions/hypotheses:** None for branch setup; future implementation work still requires plan approval before source changes.

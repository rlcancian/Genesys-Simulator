# ModelComponent DataDefinition Property Audit — 2026-11-08

## Scope and method

- Branch: `WiP202611/datadef-component-properties`.
- Base: `origin/WorkInProgress` at `82acab46238b1fea689202b24185bf2e2e8b3c00`.
- Audited `source/plugins/components/`, the relevant DataDefinition helpers, `SimulationControlGenericClass`, Property Editor controls, model loading order, and `.gen` round-trips.
- Internal DataDefinitions are loaded before components. Components must reconnect to those registered instances after their persisted names are loaded; constructing a fresh child in a loader can create duplicate definitions or leave the editor bound to an unrelated instance.
- `Counter` and `StatisticsCollector` remain excluded from editable reference controls as requested.

## Confirmed defects and corrections

1. `CppForG` and `DiffEquations` created owned `CppCompiler` objects before loading the persisted component name. Their loaders now defer child creation, reconnect the persisted compiler by `<component>.CppCompiler`, and create a child only for legacy files without one. A real `.gen` round-trip checks compiler settings, pointer identity, and exactly two compilers for two components.
2. `DummyComponent`, `PythonForG`, and `RSimulator` did not reliably reconnect their internal `DummyElement`, `PythonRuntime`, and `RSimulatorRunner` after load. Load now resolves the already-loaded child by its persisted owner-derived name. Tests verify the child pointer, configuration, and single registered instance before ModelCheck.
3. `AIAssistant::_loadInstance()` restored the legacy `AISupport` name but left its typed property null until check/execution. It now resolves and attaches the existing object during load. A `.gen` round-trip verifies the same registered instance is presented before check.
4. `Match::_createEditableDataDefinitions()` created fresh queues after load, duplicating the persisted internal queues. It now reattaches queues by stable owner-derived name and creates only missing queues. A real `.gen` round-trip verifies queue count, ordering, and attribute configuration without duplicates.
5. `Match::NumberOfQueues` remains editable without a runtime guard. Queue-count changes are model-editing operations made before ModelCheck and simulation; a waiting-entity runtime guard is not part of this contract.

## Coverage matrix

`Control verified` means the current component/helper registers a typed or equivalent Property Editor control and its setter/getter maps to the referenced object/configuration. `Focused UI test` identifies direct editor interaction in this branch. `Round-trip` names executable persistence evidence. Rows marked `UNKNOWN` are not claimed complete.

| ModelComponent / helper | DataDefinition | Association | Property | Selection / editing | Persistence evidence | Test / result |
|---|---|---|---|---|---|---|
| AIAssistant | AISupport | shared reference | AISupport | typed selector; focused UI selection | `.gen`, identity restored before check | focused editor + round-trip — PASS |
| ContinuousSystemComponent | ODESolver | shared reference | ODESolver | typed selector; focused UI selection | `.gen`, identity restored before check | focused editor + round-trip — PASS |
| ModalModelDefault | DefaultNetwork subclasses | shared reference | Network | polymorphic selector includes EFSMNetwork | `.gen` network/binding round-trip covered by existing tests | focused selector + existing round-trip — PASS |
| DummyComponent | DummyElement | exclusive internal | DummyElement (read-only reference; child settings inline) | internal identity cannot be replaced | `.gen`, one child reattached | focused editor + round-trip — PASS |
| CppForG | CppCompiler | exclusive internal | CppCompiler (read-only reference; child settings inline) | child settings editable | `.gen`, compiler fields and identity | focused editor + round-trip — PASS |
| DiffEquations | CppCompiler | exclusive internal | CppCompiler (read-only reference; child settings inline) | child settings editable | `.gen`, compiler fields and identity | focused editor + round-trip — PASS |
| PythonForG | PythonRuntime | exclusive internal | PythonRuntime (read-only reference; child settings inline) | child settings editable without Python execution | `.gen`, executable setting and identity | focused editor + round-trip — PASS |
| RSimulator | RSimulatorRunner | exclusive internal; component configuration is source of truth | internal runner settings | synchronized by component; no arbitrary replacement | `.gen`, executable/prelude/commands and identity | focused editor + round-trip — PASS |
| SPICECircuit | SPICERunner | shared, non-owning reference | Runner | typed selector | `.gen`, shared identity; callback lifetime regression | focused lifetime + round-trip — PASS |
| Search | Queue | shared reference selected by mode | SearchIn | typed mode-specific selector | existing `.gen` test | focused editor — PASS |
| Search | EntityGroup | shared reference selected by mode | SearchIn | typed mode-specific selector; selection preserves mode | `.gen`, shared identity | focused editor + round-trip — PASS |
| Remove / inherited by PickUp | Queue | shared reference selected by mode | RemoveFrom | typed mode-specific selector | existing `.gen` test | focused editor — PASS |
| Remove / inherited by PickUp | EntityGroup | shared reference selected by mode | RemoveFrom | typed mode-specific selector; selection preserves mode | `.gen`, shared identity | focused editor + round-trip — PASS |
| Match | internal Queue set | exclusive internal collection | NumberOfQueues, QueueOrderRule, QueueOrderAttributeName | common settings edit all queues; Queue enum reused | `.gen`, count, queue identity, rule and attribute | focused editor + runtime/round-trip — PASS |
| Batch | EntityType | shared reference | GroupedEntityType | typed selector | existing component persistence | control and component path audited — PASS |
| Batch | Queue, EntityGroup | exclusive internal implementation data | not user-selectable | internal creation only; no replacement control required | created/registered by existing internal-data lifecycle | internal contract audited — PASS |
| Create / SourceModelComponent | EntityType | shared reference | EntityType (inherited source property) | typed selector | existing source component persistence | existing component tests — PASS |
| Create | Formula | shared reference | TimeBetweenCreationsFormula | typed selector | existing component persistence | control audited — PASS |
| Create | Schedule | shared reference | TimeBetweenCreationsSchedule | typed selector | existing component persistence | control audited — PASS |
| LSODE | Variable | shared reference | Variable, TimeVariable | typed selectors | existing component persistence | controls audited — PASS |
| DiffusionSimulate | DiffusionField | shared reference | DiffusionField | typed selector | existing component persistence | controls audited — PASS |
| Buffer | SignalData | shared/attached by mode | SignalData | typed selector and existing validation | existing component persistence | kernel regressions — PASS |
| QueueableItem (Process/Seize/Release helper) | Queue | shared helper reference | Queue | typed nested property | helper persistence | existing tests — PASS |
| QueueableItem (Process/Seize/Release helper) | Set | shared helper reference | Set of Queue members | typed nested property | helper persistence | existing tests — PASS |
| SeizableItem (Process/Seize/Release helper) | Resource | shared helper reference | Resource | typed nested property | helper persistence | existing tests — PASS |
| SeizableItem (Process/Seize/Release helper) | Set | shared helper reference | Set of Resource members | typed nested property | helper persistence | existing tests — PASS |
| PickableStationItem (PickStation helper) | Station | shared helper reference | Station | typed nested property | helper persistence | control audited — PASS |
| PickableStationItem (PickStation helper) | Resource | shared helper reference | Resource | typed nested property | helper persistence | control audited — PASS |
| PickableStationItem (PickStation helper) | Queue | shared helper reference | Queue | typed nested property | helper persistence | control audited — PASS |
| Access / Exit / Start / Stop | Conveyor | shared reference | Conveyor | typed selector | existing component persistence | controls audited — PASS |
| Enter / Leave / Route | Station | shared reference | Station | typed selector | existing component persistence | controls audited — PASS |
| Move | Transporter | shared reference | Transporter | typed selector | existing component persistence | control audited — PASS |
| Move | Station | shared reference | Station | typed selector | existing component persistence | control audited — PASS |
| Store / Unstore | Storage | shared reference | Storage | typed selector | existing component persistence | controls audited — PASS |
| Goto / Route | LabelDefinition | shared/attached reference | LabelDefinition | typed selector | existing `.gen` Transfer tests | existing tests — PASS |
| Label | LabelDefinition | exclusive internal | LabelDefinition | owned identity; not replaceable arbitrarily | existing `.gen`/`.gui` Transfer tests | existing tests — PASS |
| MarkovChain | transition matrix, initial distribution, current state | typed/custom references | MarkovChain data selectors | custom selector filters supported types | existing component persistence | control and runtime validation — PASS |
| BacteriaColony | GroProgram, BioNetwork, BacteriaSignalGrid | shared references | corresponding typed properties | typed controls | existing component persistence | controls audited — PASS |
| BioRunnerCommand | BioSimulatorRunner | shared reference | BioSimulatorRunner | typed selector | existing component persistence | control audited — PASS |
| BioSimulate / BioSteadyState | BioNetwork | shared reference | BioNetwork | typed selectors | existing component persistence | controls audited — PASS |
| GeneticCircuitSimulate / GeneticExpressionStep | GeneticCircuit | shared reference | GeneticCircuit | typed selectors | existing component persistence | controls audited — PASS |
| MetabolicFluxBalance | MetabolicNetwork | shared reference | MetabolicNetwork | typed selector | existing component persistence | control audited — PASS |
| Whole-cell components (15 audited) | WholeCellState | shared reference | WholeCellState | typed selectors; component-specific attached items remain separate | existing component persistence | control/setter/association paths audited — PASS |
| Clone / Create / Dispose / Enter / Leave / Move / Route / Delay / Record | Counter / StatisticsCollector | automatic statistics only | none by decision | deliberately excluded | current automatic lifecycle preserved | explicit exclusion — PASS |
| Wait / Signal | SignalData | shared reference | SignalData | implementation belongs to dependency PR #570; not duplicated here | not present in this branch | open Draft dependency — UNKNOWN |

## Match port contract limitation

Queue order and queue-count persistence are now validated, and queue reductions are treated as normal pre-check model editing. The current `Match` execution releases entities through a common output connection, while the requested future contract calls for output count to correspond to queue count. The existing one-output model files do not define how their connection should map to multiple outputs. No routing migration or invented per-queue output semantics was added. Exact input/output port synchronization and backward-compatible connection migration remain `UNKNOWN` and require a separately defined migration rule; this is a stop gate, not a passing criterion.

## Validation on the current working tree

- CMake/Ninja target `genesys_test_gui_gmdd_layout`: built successfully.
- CMake/Ninja targets `genesys_test_simulator_runtime` and `genesys_kernel_unit_tests`: built successfully.
- `cmake --build --preset gui-app --parallel 8`: PASS; built the full Qt6 GUI target through 370 Ninja steps. Existing warnings were emitted by `FitterDummyImpl.cpp` for non-void methods without return statements and by the vendored Qt property browser for deprecated `QString::count()`; neither warning originates in this change.
- `ctest --test-dir build/tests-unit -R 'PropertyEditorModelDataReference|SimulatorRuntimeTest\\.Match' --output-on-failure`: 25/25 passed.
- `ctest --test-dir build/tests-unit -R 'Search|Remove|Queueable|Seizable|Buffer|Batch|Match|PropertyEditorModelDataReference|ModalModelDefaultNetwork|RSimulator|PythonRuntime|CppCompiler|SPICECircuit' --output-on-failure`: 119/119 executed passed; four pre-existing disabled Search/Remove tests were not run.
- Focused `.gen` round-trips now verify identity and absence of duplicate internal definitions for `AISupport`, `ODESolver`, `DummyElement`, `PythonRuntime`, `RSimulatorRunner`, `CppCompiler`, and `Match` queues.
- No manual interaction in the full desktop GUI was performed; Qt tests exercise the Property Editor widgets under the available test environment.
- Wait/Signal remains dependent on PR #570, still open Draft at HEAD `b10adbf599d8990d55520fef6fa60c069763d872` when checked.

## Outcome

Audit and focused corrections are complete for the cases marked PASS above. The overall backlog item remains `running` because Wait/Signal is owned by its concurrent PR and Match output-port migration is unresolved. The queue-count editor is intended for pre-check model editing; no runtime guard is needed. No documentation PDF regeneration is required; no LaTeX source was changed.

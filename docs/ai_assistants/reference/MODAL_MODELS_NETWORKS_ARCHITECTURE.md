---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-08-30
review_cadence: on-modal-contract-change
status: active
tracks: 511
---

# Modal Models, Networks, Ports, and Heterogeneous Model-of-Computation Boundary

## 1. Purpose

This document records the maintainer-approved architectural direction for modal/network-based modeling in GenESyS. It consolidates the design decisions for `ModalModel`, network data definitions, nodes, transitions/arcs, ports, entity adaptation, EFSMs, finite-state discrete-time Markov chains, and Colored Petri Nets.

This is a target architecture and semantic contract. Current code does **not** yet fully implement it. Current implementation/maturity evidence belongs in [`../STATUS.md`](../STATUS.md); pending implementation work belongs in the canonical backlogs.

The design is intentionally influenced by Ptolemy II modal/FSM modeling, while preserving the existing GenESyS process-oriented discrete-event semantics at the outer model boundary.

The central goal is to support multiple models of computation without pretending that all of them are ordinary process-flow components connected by entity-routing `Connection`s.

## 2. Core architectural distinction

GenESyS currently models an ordinary discrete-event process as a graph of `ModelComponent`s connected by `Connection`s. An entity travels through that process, and its arrival at a `ModelComponent` causes the component's event behavior to execute.

A modal/network model is different.

A modal network contains semantic nodes, states, places, transitions, arcs, markings, state variables, probabilities, or other formalism-specific objects. Those objects are **not** ordinary process steps through which a GenESyS entity travels.

Therefore the architecture distinguishes two graphs:

```text
Process-oriented GenESyS graph

ModelComponent --Connection--> ModalModel --Connection--> ModelComponent
                                  |
                                  | attached association
                                  v
                            DefaultNetwork

Modal/network graph

DefaultNetwork
  +-- nodes/states/places
  +-- transitions/arcs
  +-- network state
  +-- input/output interface
```

The `ModalModel` is the adapter between these two worlds.

## 3. Current code baseline to migrate

At the time this reference was written, the current implementation under `source/plugins/components/ModalModel/` contains, among other classes:

- `ModalModelDefault`;
- `ModalModelFSM`;
- `ModalModelPetriNet`;
- `DefaultNode`;
- `DefaultNodeTransition`;
- `EFSMTransition`;
- `PetriTransition`;
- `FSMState`;
- `PetriPlace`;
- `Submodel`;
- cellular-automata-related classes.

The current `ModalModelDefault` directly owns lists of nodes and transitions and stores modal current-node information through entity attributes. The current `DefaultNode` is a `ModelComponent`. The current `PetriTransition` assumes one source place and one destination place.

Those details are historical implementation state, not the desired final semantics described below.

## 4. Main design principles

### 4.1 `ModalModel` remains a `ModelComponent`

`ModalModel` participates in the ordinary GenESyS process flow. Entities arrive through ordinary component input ports and, depending on the network outputs produced by an activation, may leave through ordinary component output ports.

Therefore it remains a `ModelComponent`.

### 4.2 `DefaultNetwork` is a `ModelDataDefinition`

The network itself is reusable model data, not a process-flow step.

A `DefaultNetwork` is therefore a `ModelDataDefinition`. It owns or aggregates the semantic objects that constitute the network and owns the network's persistent runtime state.

This allows multiple `ModalModel` components at different locations in the process model to activate the **same** network.

Example:

```text
Process location A --> ModalModel A --\
                                      \
                                       > shared EFSMNetwork
                                      /
Process location B --> ModalModel B --/
```

The state belongs to the shared network. It does not belong to either `ModalModel` and does not belong to the triggering entity.

### 4.3 `DefaultNode` is a `ModelDataDefinition`, not a `ModelComponent`

A node/state/place inside a modal network is not part of the ordinary entity process flow and must not require a `ConnectionManager` or `_onDispatchEvent()` merely to exist in a network.

The target hierarchy is conceptually:

```text
ModelDataDefinition
  +-- DefaultNetwork
  |    +-- EFSMNetwork
  |    +-- MarkovChainNetwork
  |    +-- ColoredPetriNetNetwork
  |
  +-- DefaultNode
       +-- EFSMState
       +-- MarkovState
       +-- PetriPlace
```

Exact class names may be adjusted after inspecting persistence, plugin registration, and compatibility constraints, but this semantic separation is approved.

### 4.4 Formalisms must preserve their own invariants

Do not force EFSM, Markov-chain, and Petri-net semantics into one overly generic transition representation.

For EFSMs and finite-state Markov chains, a transition is naturally an edge from a source state to a destination state.

For a Petri net, a transition is itself a vertex in a bipartite graph and is connected to places by input/output arcs. A Petri transition must therefore not be reduced to a binary `(sourcePlace, destinationPlace)` edge.

A common implementation base may exist only if it does not erase these formal distinctions.

## 5. Responsibility of `ModalModel`

The generic `ModalModel` should be intentionally small. Its responsibilities are:

1. hold an **attached** reference to a `DefaultNetwork`;
2. mirror the network's declared external input/output port schema;
3. receive a GenESyS entity through an ordinary input port;
4. adapt that arrival into a network activation frame;
5. invoke the attached network exactly once for that activation;
6. adapt network outputs back to entity-based process flow;
7. route/clone/consume the entity according to the outputs produced;
8. persist its network association and per-port bindings;
9. validate that its external port configuration is consistent with the attached network.

`ModalModel` does **not** own:

- current EFSM/Markov state;
- Petri marking;
- nodes or transitions;
- network-local variables;
- transition probabilities;
- Petri tokens;
- the activation counter.

When it needs network state for visualization or reporting, it queries the attached `DefaultNetwork` through an appropriate read interface.

## 6. Responsibility of `DefaultNetwork`

`DefaultNetwork` is the root abstraction for a reusable stateful network.

It should provide only infrastructure genuinely common to network formalisms, such as:

- identity and persistence inherited from `ModelDataDefinition`;
- declared input/output port schema;
- activation lifecycle;
- common validation hooks;
- reset-between-replications lifecycle;
- an internal activation `Counter`;
- common tracing/reporting metadata;
- access to the network's contained semantic objects when meaningful.

Formalism-specific semantics belong in derived network classes.

A network may be attached by more than one `ModalModel` and therefore must not store assumptions about a single caller.

## 7. Network activation counter

`DefaultNetwork` must contain an internal/composed `Counter` that reports the number of network activations.

Its semantic contract is:

```text
ActivationCount += 1
```

for every accepted call to the network activation operation, regardless of whether:

- an EFSM transition fires;
- a Markov state changes;
- a CPN transition/binding fires;
- no transition is enabled.

This counter counts **activations**, not successful transitions/firings. Formalism-specific statistics may later add transition counts, state visits, token throughput, and related measures.

The exact existing `Counter` lifecycle/association API must be reused according to GenESyS internal-data ownership conventions rather than reimplemented ad hoc.

## 8. Network ports are a formal interface

### 8.1 Network owns the schema

A `DefaultNetwork` declares its logical input and output ports.

Conceptually:

```text
EFSMNetwork Controller
  inputs:
    0 Temperature
    1 Pressure
    2 Reset

  outputs:
    0 State
    1 Alarm
```

The network is the source of truth for the logical interface.

### 8.2 Attached `ModalModel`s mirror the network ports

Every `ModalModel` attached to that network exposes corresponding GenESyS component input/output ports.

Thus, if a network has four logical inputs and three logical outputs, each attached `ModalModel` exposes four process-facing input ports and three process-facing output ports.

This deliberately resembles Ptolemy II, where ports of a modal model are visible at the modal boundary and controller/refinement boundary.

### 8.3 Each `ModalModel` may bind inputs differently

Because several `ModalModel`s may activate one shared network, each adapter may use different expressions to obtain the value supplied to the corresponding network input.

Example:

```text
ModalModel A / input Temperature
    binding: Entity.SensorA

ModalModel B / input Temperature
    binding: Entity.EstimatedTemperature * Calibration
```

The binding should use the ordinary GenESyS expression/parser infrastructure rather than introducing a second expression language.

### 8.4 Network does not receive `Entity*`

A `DefaultNetwork` must not depend on the process entity that caused activation.

If an entity attribute is required as a network input, the `ModalModel` evaluates the configured input binding and transfers the resulting value into the activation frame.

Conceptually:

```text
Entity.Priority
      |
      | evaluated by ModalModel input binding
      v
Network input Priority
```

This keeps network semantics independent from process-flow entities and allows the same network to be activated from several process locations.

### 8.5 External parser references remain possible

Formal network inputs/outputs define the **explicit interface**, but they do not necessarily form a security/sandbox boundary around the parser.

A guard/action may continue to use model-level symbols that the GenESyS parser deliberately exposes, such as model Variables, Queue functions, or other global model data, when that behavior is supported and documented.

However, direct dependency on the current process `Entity` is not part of the network contract. Entity-specific values should enter through explicit `ModalModel` input bindings.

## 9. Activation frame: distinguish interface, presence, and persistent state

Port definition, activation-time values, and network state are different concepts and should not be collapsed.

Conceptually:

```text
PortDefinition
  +-- name
  +-- direction
  +-- dimensional/type metadata

NetworkActivationFrame
  +-- input[0]: present + value
  +-- input[1]: present + value
  +-- ...

Network persistent state
  +-- EFSM current state and variables
  +-- DTMC current state
  +-- CPN marking
  +-- other formalism-specific state
```

Presence is significant. An absent input is not automatically equivalent to zero and is not automatically equivalent to its previous value.

The first implementation may remain compatible with the current numeric parser/value model, but the abstraction must not unnecessarily prevent future richer token/value types.

## 10. Input semantics for the initial `ModalModel` adapter

When an entity reaches `ModalModel` input port `i`:

1. the adapter identifies logical network input `i`;
2. it evaluates that port's configured GenESyS input-binding expression in the current process/entity context;
3. it constructs a network activation frame;
4. input `i` is marked present and receives the evaluated value;
5. other inputs are normally absent in that activation;
6. the adapter invokes `DefaultNetwork::activate(...)` or the final equivalent API exactly once.

The activation-frame abstraction should nevertheless be capable of representing more than one simultaneously present input. This is a deliberate future-proofing requirement for richer models of computation and event coalescing.

## 11. Output semantics and entity adaptation

The result of one network activation contains output-port presence and output values.

Conceptually:

```text
Output 0: present = true,  value = 12
Output 1: present = false
Output 2: present = true,  value = 7
```

Each `ModalModel` defines how each logical output value is represented on the entity leaving the corresponding process-facing output port, initially through configurable entity-attribute binding/mapping.

### 11.1 No outputs present

Approved behavior:

> If no network output is present, no entity leaves the `ModalModel`.

The incoming entity is **consumed**.

Implementation must use the correct GenESyS entity lifecycle/removal semantics so that "consumed" cannot become a leaked/orphaned entity.

### 11.2 Exactly one output present

The original entity may be reused, the output value is applied to its configured output attribute/binding, and it is routed through the corresponding `ModalModel` output port.

### 11.3 Multiple outputs present

If `N > 1` outputs are present:

- one outgoing path may reuse the original entity;
- the remaining `N - 1` paths receive correct GenESyS clones;
- each entity/clone receives only the output data associated with its own selected network output;
- each is sent through the corresponding process output port.

The implementation must reuse the established GenESyS entity-cloning semantics rather than inventing a shallow copy that breaks ownership, attributes, entity type, or statistics.

## 12. Multidimensional values

Network ports should not be architecturally limited to anonymous scalar slots.

GenESyS `Variable` already has scalar/indexed/multidimensional concepts and uses `SparseValueStore`. The network-port/value implementation should investigate reuse or extraction of that infrastructure before creating a competing representation.

The target design should leave room for interfaces such as:

```text
Input X
Input Vector[10]
Input Matrix[3,2]
```

The initial implementation may support the subset safely representable by the current parser and persistence system, provided limitations are explicit and the public abstraction does not unnecessarily block later typed/vector/matrix ports.

## 13. EFSM network semantics

`EFSMNetwork` is a specialization of `DefaultNetwork` and owns its state-machine state.

The first supported EFSM scope should be robust and Ptolemy-inspired, with:

- explicit states;
- exactly identified initial state;
- optional final states;
- current state owned by the network;
- input values/presence;
- guards;
- output actions;
- state/internal-variable update actions;
- state entry/exit actions where retained by the implementation;
- persistence and replication reset semantics;
- deterministic, documented handling of ambiguous enabled transitions.

A separate free-form `activationEvent` string is not required by the approved interface. Input-port presence/value and guards provide the primary triggering interface.

A single activation should commit at most one ordinary EFSM state transition. The design should follow Ptolemy-like deterministic semantics: multiple simultaneously enabled transitions must not be selected silently unless GenESyS defines an explicit, validated extension such as priorities. The exact compatibility policy should be documented and tested before changing persisted behavior.

### 13.1 State refinements are future work

State refinements, preemptive/non-preemptive transitions, error transitions, history, and other advanced Ptolemy modal semantics are intentionally deferred.

The architecture should nevertheless avoid blocking future refinements. A future EFSM state may reference a nested/submodel refinement without requiring the state itself to become a process `ModelComponent`.

## 14. Discrete-time Markov chain semantics

The first Markov formalism is a finite-state **Discrete-Time Markov Chain (DTMC)** only. Continuous-time Markov chains are out of scope for the initial implementation.

`MarkovChainNetwork` owns:

- its states;
- current state;
- initial state;
- transition probabilities or probability expressions;
- stochastic state-transition behavior;
- reproducibility state through the approved GenESyS random-number infrastructure.

One network activation corresponds to **one DTMC step**.

For a state `i`, outgoing probabilities must satisfy the selected mathematical convention, including non-negativity and total probability 1 within a justified numerical tolerance. Absorbing states must be represented deliberately rather than accidentally producing undefined selection behavior.

Use the GenESyS RNG/seed/replication infrastructure. Do not use `std::rand()` for scientific stochastic behavior.

The architecture should not conflate DTMC transition probabilities with future CTMC transition rates.

## 15. Colored Petri Net semantics

The target is a real Colored Petri Net core, not merely a directed graph whose edges carry string-labeled token counts.

A CPN implementation must preserve the bipartite structure:

```text
Place --Arc--> Transition --Arc--> Place
```

A Petri transition is therefore not a `DefaultNodeTransition` from one place directly to another.

The CPN design should support, incrementally but genuinely:

- places;
- transitions as first-class network elements;
- multiple input arcs;
- multiple output arcs;
- color sets / typed token domains;
- colored token values;
- multiset markings;
- guards;
- arc inscriptions/expressions;
- variable bindings sufficient to evaluate guards and inscriptions;
- enabling semantics;
- atomic firing semantics;
- persistence and replication reset;
- deterministic/reproducible conflict-resolution behavior where stochastic choice is allowed.

Exact C++ class hierarchy is an implementation decision, but a CPN must not be forced into a binary `(sourcePlace, destinationPlace)` transition API.

## 16. CPN firing policy

`ColoredPetriNetNetwork` should expose a model-level firing-mode property so the modeler can choose how much enabled behavior one network activation performs.

The approved initial modes are conceptually:

```text
Single
MaximalConcurrentStep
```

### 16.1 `Single`

Determine enabled transition bindings and fire exactly one selected enabled binding.

Conflict/selection policy must be explicit and reproducible. If stochastic choice is supported, it must use GenESyS RNG infrastructure.

### 16.2 `MaximalConcurrentStep`

Fire a maximal set of transition bindings that is **jointly enabled** by the current marking.

"All enabled" must not mean blindly firing every transition that is individually enabled, because individually enabled transitions may compete for the same tokens.

The selected set must respect token/multiset consumption constraints and commit atomically as one CPN step.

This is distinct from an `UntilQuiescence` mode that repeatedly fires transitions until none remain enabled. `UntilQuiescence` is not part of the approved initial scope and may have nontermination/livelock implications.

## 17. Network state belongs to the network

The following state is network-owned, never entity-owned:

- EFSM current state;
- DTMC current state;
- CPN marking;
- network internal variables;
- activation counter;
- formalism-specific runtime state.

The current `Entity.ModalModel.<name>.CurrentNode` and similar entity-attribute mechanism should be removed/deprecated during migration once compatibility impact has been assessed.

In principle, an entity's attributes do not change merely because it passed through a `ModalModel`; attributes change only through explicit input/output adaptation or another explicit model action.

## 18. Sharing one network among several `ModalModel`s

Sharing is a first-class use case, not an accident.

Several process components may activate one network from different locations:

```text
Entity flow A -> ModalModel A --\
                               +--> one shared network state
Entity flow B -> ModalModel B --/
```

Since GenESyS is event-driven, activations at the same simulation timestamp are initially processed according to the simulator's established event ordering. The network must not invent simultaneous process semantics outside the event-calendar contract.

A future heterogeneous-MoC layer may introduce explicit same-tag/same-time coalescing; the current architecture should leave room for multi-input activation frames without silently changing discrete-event ordering today.

## 19. Ownership, attachment, and hierarchy level

The association from `ModalModel` to `DefaultNetwork` is **attached/aggregated**, not exclusive composition, because the same network may be referenced by multiple adapters.

The network owns/composes its internal semantic state according to the final GenESyS internal-data ownership conventions.

Nodes, transitions/arcs, port definitions, internal counters, and internal network variables require explicit ownership and persistence rules. Existing raw-pointer lists must not simply be moved without mapping destruction, model-manager registration, plugin factories, load/save behavior, and sharing.

`ModelDataDefinition::getLevel()/setModelLevel()` remains the hierarchical mechanism. Network-contained definitions should occupy a consistent nested level, but the exact numeric-level convention must follow the real model/persistence lifecycle discovered during implementation rather than an invented rule.

## 20. Parser and expression integration

Network semantics should reuse the existing GenESyS parser rather than create an EFSM-specific or CPN-specific expression language unless a formalism strictly requires additional typed syntax.

Parser use must distinguish:

- process-side input bindings evaluated by `ModalModel` in the triggering entity context;
- network guards/actions evaluated by the network;
- explicitly declared network inputs/outputs;
- network-internal variables;
- deliberately permitted references to other global/model data definitions.

Expression dependencies must be validated through the existing reference/check infrastructure when possible.

Any parser extension required for CPN token values, bindings, or typed inscriptions must be scoped and tested rather than silently overloading current scalar expression semantics.

## 21. GUI implications

The architecture deliberately creates a GUI requirement that does not exist in the ordinary process editor:

> the GUI must eventually be able to draw/edit networks whose nodes are `ModelDataDefinition`s or other network semantic objects, not `ModelComponent`s connected by ordinary `Connection`s.

The UI should eventually support:

- opening the attached network from a `ModalModel`;
- drawing network-specific nodes and relationships;
- editing state/place/transition/arc properties;
- editing network input/output ports;
- automatically reflecting network interface changes on attached `ModalModel`s;
- displaying current state/marking during simulation;
- future state refinements/submodels;
- different renderers/editors for EFSM, DTMC, CPN, cellular automata, and later formalisms.

This GUI work is not required to prove the initial backend semantics and should not force network objects back into `ModelComponent` merely because the current graph editor understands components better.

## 22. `Process`/ordinary submodel versus modal network

Do not confuse modal networks with ordinary hierarchical process submodels.

A composite process component such as `Process` can contain internal `ModelComponent`s such as `Seize`, `Delay`, and `Release`. Those internal components remain ordinary process-flow components connected by ordinary `Connection`s at a nested level.

A modal network instead contains formalism-specific semantic objects and relations.

Therefore:

```text
ordinary composite process
    ModelComponent -> Connection -> ModelComponent

modal/network composition
    DefaultNetwork -> states/places/transitions/arcs/etc.
```

Both can use GenESyS hierarchy/level infrastructure, but they are different models of computation.

## 23. Migration direction from current modal code

The implementation should migrate incrementally rather than perform a blind rewrite.

Expected migration themes are:

1. introduce a real network data-definition abstraction;
2. move node/transition ownership and current network state out of `ModalModelDefault`;
3. convert `DefaultNode` from `ModelComponent` semantics to `ModelDataDefinition` semantics;
4. introduce generic `ModalModel` attachment to a network;
5. move FSM specialization from `ModalModelFSM` into `EFSMNetwork`;
6. move Petri specialization from `ModalModelPetriNet` into `ColoredPetriNetNetwork`;
7. add a `MarkovChainNetwork` for finite-state DTMC behavior;
8. replace current entity-carried current-node state with network-owned state;
9. replace binary Petri source/destination assumptions with explicit Petri arcs;
10. preserve or migrate persistence compatibility deliberately;
11. update plugin registration, factories, CMake aggregation, and tests;
12. deprecate or provide compatibility loading for old type names only where real persisted-model compatibility requires it.

Do not retain old subclasses merely for aesthetic compatibility if they create two competing sources of network state. Conversely, do not delete persisted public types without first checking model fixtures and historical `.gen` compatibility.

## 24. Future evolution of `ConnectionChannel` and `Connection`

The existing process connection contract is entity-centric:

```text
Connection
  -> destination ModelComponent
  -> destination input port number
```

`ConnectionChannel` currently contains a port number and description, and the source comment already anticipates richer semantics such as type and presence/absence of data.

This modal/network architecture should be treated as the first clear use case for a more general heterogeneous model-of-computation boundary.

Possible future evolution includes metadata such as:

```text
ConnectionChannel
  +-- port identity/name
  +-- direction
  +-- value/token type
  +-- dimensions
  +-- presence/absence semantics
  +-- causality/event metadata
```

A later connection/message abstraction may distinguish or generalize payloads such as:

```text
Entity transfer
Token/value transfer
Event/presence notification
Structured message
```

This could allow components or actors from different models of computation to interact without using `Entity` as the universal payload.

This is a **future architectural direction**, not authorization to redesign `Connection` or the event kernel during the first modal-network implementation.

## 25. Future heterogeneous models of computation

The architectural objective is broader than EFSM/DTMC/CPN. `ModalModel` establishes an adapter pattern that can later support other models of computation while preserving GenESyS event scheduling as one outer orchestration mechanism.

Potential future areas include:

- state refinements and hierarchical state machines;
- CTMCs with rate/holding-time semantics and internal scheduled events;
- synchronous/reactive actor semantics;
- dataflow/token-based actors;
- cellular automata and spatial update domains;
- hybrid continuous/discrete models;
- biochemical/reaction networks;
- explicit directors/domain schedulers similar in spirit to Ptolemy II where justified.

Do not introduce a Ptolemy-style director hierarchy prematurely. The first task is to make the modal/network boundary semantically correct and extensible.

## 26. State refinements and nested models: future compatibility requirement

Although `DefaultNode` becomes a `ModelDataDefinition`, future state refinements remain possible.

A state may later **reference** a refinement object/submodel that contains ordinary components or another compatible network. The state does not need to become a `ModelComponent` itself.

This separates:

- state identity and modal semantics;
- refinement executable model;
- outer process-flow adapter.

The design should avoid ownership choices that make this future association impossible.

## 27. Validation requirements

Backend implementation must be validated by formalism-specific invariants, not only by compilation.

### 27.1 Generic modal/network tests

Cover at least:

- network plugin creation/registration;
- attached versus internal ownership;
- persistence round-trip;
- multiple `ModalModel`s sharing one network;
- replication reset;
- activation counter;
- input-port mirroring;
- input expression binding;
- output presence/value mapping;
- zero-output entity consumption without leaks;
- one-output routing;
- multiple-output correct cloning;
- invalid/mismatched port schemas;
- deterministic behavior for identical seeds/configurations.

### 27.2 EFSM tests

Cover:

- initial/current/final state;
- no enabled transition;
- one enabled transition;
- ambiguous enabled transitions according to the chosen deterministic policy;
- guard evaluation;
- input presence/value;
- output actions;
- state variable updates;
- entry/exit actions if supported;
- persistence and reset.

### 27.3 DTMC tests

Cover:

- probability domain validation;
- outgoing total probability;
- deterministic probability-1 transition;
- absorbing states;
- one step per activation;
- seed reproducibility;
- empirical transition frequencies with declared tolerances/reference;
- persistence/reset.

### 27.4 CPN tests

Cover:

- color-set/type validation;
- multiset marking;
- multi-input and multi-output arcs;
- inscriptions and bindings;
- guards;
- enabling;
- atomic token consumption/production;
- conflict cases;
- `Single` firing mode;
- `MaximalConcurrentStep` joint-enabling semantics;
- persistence/reset;
- deterministic/reproducible selection policy.

Use ASan/LSan/UBSan where migration of ownership/lifetime makes them technically useful.

## 28. Scientific and behavioral claim discipline

Successful build/tests do not by themselves prove formal correctness.

For each formalism, implementation work must identify authoritative references and define the supported subset precisely.

Use explicit language such as:

- implemented subset;
- software behavior validated by tests;
- formal invariant validated;
- unsupported feature;
- future extension.

Do not claim compatibility with all Ptolemy II modal semantics or all Colored Petri Net language features unless separately demonstrated.

## 29. Explicit non-goals for the first implementation phase

The first modal-network consolidation should not automatically include:

- Ptolemy state refinements;
- preemptive/non-preemptive refinement scheduling;
- Ptolemy error/history transitions;
- CTMC scheduling/rates;
- arbitrary Ptolemy token type system;
- arbitrary actor/director infrastructure;
- `Connection` kernel redesign;
- generic heterogeneous-message event kernel;
- GUI completion before backend semantics are validated;
- every possible CPN language extension.

These should remain possible future directions rather than hidden requirements that prevent a coherent first implementation.

## 30. Decision summary

The following decisions are approved as of 2026-08-30:

1. `ModalModel` is a `ModelComponent` and an adapter between process/entity flow and network semantics.
2. `DefaultNetwork` is a `ModelDataDefinition` and owns network runtime state.
3. Several `ModalModel`s may attach to and activate the same network.
4. `DefaultNode` is a `ModelDataDefinition`, not a `ModelComponent`.
5. Network state is not stored in the triggering entity.
6. The network owns its input/output interface schema.
7. Attached `ModalModel`s mirror the network's logical inputs/outputs as process-facing ports.
8. A `ModalModel` input port evaluates a configured expression and supplies that value as the corresponding present network input for the current activation.
9. `DefaultNetwork` does not receive `Entity*`.
10. Network activation distinguishes port presence from value and from persistent state.
11. Network guards/actions may deliberately access supported model-level parser symbols, but entity-specific values enter through explicit adapter bindings.
12. No network output present consumes the incoming entity.
13. One output can reuse the original entity; multiple present outputs require correct cloning and one routed entity per present output.
14. `DefaultNetwork` has an internal activation counter incremented on every activation, whether or not a transition/firing occurs.
15. EFSM state belongs to `EFSMNetwork`; the initial supported scope includes state, guards, inputs, output/state-update actions, initial/final states, without state refinements yet.
16. Markov support initially means finite-state DTMC only, with one Markov step per network activation.
17. Colored Petri Net support means a genuine CPN core with places, transition vertices, explicit arcs, typed/color token values, multisets, inscriptions, bindings, guards, and proper firing semantics.
18. CPN firing mode is model-configurable, initially supporting one binding or a maximal jointly enabled concurrent step.
19. Petri transitions are not binary place-to-place edges.
20. Future state refinements should be possible through references to nested executable models without changing a state back into a `ModelComponent`.
21. Future `ConnectionChannel`/`Connection` evolution may support typed token/value/event payloads and presence semantics, but this is not part of the first implementation.

## 31. Authoritative external references

The architecture is influenced by, but not intended to duplicate, the following references:

- Ptolemy II project: <https://ptolemy.berkeley.edu/ptolemyII/index.htm>
- Ptolemaeus, *System Design, Modeling, and Simulation*, chapter on models of computation and FSM/modal models: <https://ptolemy.berkeley.edu/books/Systems/chapters/IIModelsofComputation.pdf>
- Ptolemy II `FSMActor` API/documentation: <https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/ptolemy/domains/modal/kernel/FSMActor.html>
- Ptolemy II `ModalModel` API/documentation: <https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/ptolemy/domains/modal/modal/ModalModel.html>
- Aarhus University Coloured Petri Nets research/reference material: <https://cs.au.dk/cpnets/papers/new-overview>
- CPN Tools project/resources: <https://cpntools.org/>
- Stanford finite-state discrete-time Markov-chain notes/examples: <https://ee263.stanford.edu/lectures/lds.pdf> and <https://web.stanford.edu/class/stats366/exs/Markov1.html>

Use the original scientific/formal sources when settling implementation semantics. Independent software may be used for corroboration but is not automatically the specification.

## 32. Related GenESyS references

Read together with:

- [`../ARCHITECTURE.md`](../ARCHITECTURE.md);
- [`SCIENTIFIC_DOMAINS.md`](SCIENTIFIC_DOMAINS.md);
- [`PLUGINS.md`](PLUGINS.md);
- [`KERNEL_PARSER_OWNERSHIP.md`](KERNEL_PARSER_OWNERSHIP.md);
- [`APPLICATIONS_TOOLS_MODELS.md`](APPLICATIONS_TOOLS_MODELS.md).

If this reference conflicts with a later maintainer-approved update to `ARCHITECTURE.md`, the canonical architecture document has precedence according to repository governance.

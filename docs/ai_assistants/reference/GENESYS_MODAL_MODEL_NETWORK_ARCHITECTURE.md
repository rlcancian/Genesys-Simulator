---
document_type: architecture-reference
status: design-baseline
scope: GenESyS ModalModel, network models, ports, connections, and model-of-computation boundaries
target_branch: WorkInProgress
date: 2026-08-30
---

# GenESyS ModalModel and Network Architecture

## 1. Purpose

This document consolidates the architectural decisions, semantic definitions, design rationale, current-code observations, scientific references, and future directions established for the GenESyS **ModalModel** family.

The central goal is to support graph- and state-based models of computation inside a process-oriented discrete-event simulation model without confusing the semantics of the two domains.

The initial target network formalisms are:

- Extended Finite-State Machines (EFSMs);
- finite-state Discrete-Time Markov Chains (DTMCs);
- formal Coloured Petri Nets (CPNs).

The design should remain extensible to additional network-based models in the future.

This document is intentionally broader than a coding plan. It records **why** the architecture is structured this way, which decisions are already settled, which current GenESyS structures are being migrated, and which future extensions should remain possible.

---

# 2. Fundamental distinction: process model versus modal/network model

GenESyS is fundamentally a discrete-event simulator whose ordinary process-oriented model is built from `ModelComponent` objects connected by `Connection` objects.

An entity follows that process:

```text
Entity
  |
  v
ModelComponent A
  |
  | Connection
  v
ModelComponent B
  |
  | Connection
  v
ModelComponent C
```

A `ModelComponent` represents a point in the process through which an entity may pass. When an entity arrives at a component, the corresponding event dispatches the component behavior, principally through `_onDispatchEvent()`.

A **ModalModel** is also a `ModelComponent`, because an entity reaches it through the ordinary process-oriented GenESyS model.

However, what exists **inside the modal domain** is conceptually different.

A modal/network model is not a set of ordinary GenESyS `ModelComponent`s connected by ordinary `Connection`s. It is a network consisting of formal network elements such as:

- states and transitions in an EFSM;
- states and probabilistic transitions in a DTMC;
- places, transitions, arcs, markings, token values, guards, and bindings in a CPN.

Therefore:

> `Connection` remains the mechanism for process flow between `ModelComponent`s.  
> Network topology and network semantics are represented independently inside `DefaultNetwork` and its specializations.

This separation must remain explicit.

---

# 3. Ordinary composite components are not ModalModels

GenESyS already contains composite process components.

A representative example is `Process`, which internally creates a process chain such as:

```text
Process
  |
  +-- Seize
  |
  +-- Delay
  |
  +-- Release
```

Those internal objects are still `ModelComponent`s. They are connected through ordinary GenESyS `Connection`s and belong to another hierarchical level of the same process-oriented model.

This is a normal **submodel/composite process component**.

A ModalModel is structurally different:

```text
Process-oriented world
-----------------------------------------

Component A
    |
    v
+-------------+
| ModalModel  |
+-------------+
    |
    v
Component B

       |
       | attached
       v

Modal/network world
-----------------------------------------

+----------------------+
|    DefaultNetwork    |
|                      |
| nodes / states       |
| transitions / arcs   |
| internal state       |
+----------------------+
```

The distinction is architectural, not merely visual.

---

# 4. Core architectural decision

The architecture shall separate the **process adapter** from the **network itself**.

## 4.1 ModalModel

`ModalModel` is a `ModelComponent`.

Its responsibility is to bridge:

```text
GenESyS process semantics
Entity + Connection + input/output port
            |
            v
        ModalModel
            |
            v
Network activation semantics
presence + values + network state transition/firing
```

A ModalModel:

- participates in normal entity flow;
- is reached through ordinary `Connection`s;
- has input and output ports;
- is attached to a `DefaultNetwork`;
- translates an incoming entity and input port into a network activation;
- translates network outputs back into GenESyS entities and output ports;
- does **not** own the network's current state;
- does **not** own the network's nodes or transitions;
- does **not** store per-entity modal state.

## 4.2 DefaultNetwork

`DefaultNetwork` is a `ModelDataDefinition`.

It represents the network independently of the process locations from which it may be activated.

It is the source of truth for:

- network topology;
- declared network input ports;
- declared network output ports;
- persistent network state;
- nodes;
- transitions or other formal network relations;
- activation semantics;
- network-specific data;
- internal reporting/statistics.

A single `DefaultNetwork` may be attached to multiple ModalModel components.

Example:

```text
Component A -> ModalModel A ----+
                                |
                                +----> Shared EFSMNetwork
                                |
Component C -> ModalModel B ----+
```

Both ModalModels activate the **same** network and therefore observe/update the same network state.

This is an intentional feature. It allows entities at different places in the process model to trigger actions in one shared EFSM, Markov chain, Petri net, or another network model.

---

# 5. State ownership

The state of a modal/network model belongs to the **network**, not to the triggering entity and not to the ModalModel adapter.

This supersedes the current approach in which `ModalModelDefault::_onDispatchEvent()` reads and writes attributes such as:

```text
Entity.ModalModel.<name>.CurrentNode
Entity.ModalModel.<name>.LastNode
```

That per-entity state representation should be removed from the new architecture.

The intended semantics are:

```text
Entity arrives
    |
    v
ModalModel
    |
    v
DefaultNetwork::activate(...)
    |
    +-- network changes its own state
    |
    v
Entity output routing
```

In principle, passing through a ModalModel does not mutate arbitrary entity attributes except for the explicit output bindings used to carry network output values back into the process-oriented model.

---

# 6. DefaultNode is a ModelDataDefinition, not a ModelComponent

This is a settled architectural decision.

Current GenESyS code has `DefaultNode` derived from `ModelComponent`. That is incompatible with the new conceptual boundary.

A network node is **not a place through which a GenESyS process entity travels**.

Therefore:

```text
ModelDataDefinition
    |
    +-- DefaultNode
```

rather than:

```text
ModelComponent
    |
    +-- DefaultNode
```

Consequences:

- `DefaultNode` no longer requires `ConnectionManager`;
- `DefaultNode` does not implement process-flow `_onDispatchEvent()` semantics;
- nodes are not linked by ordinary GenESyS `Connection`s;
- node topology belongs to the network formalism;
- nodes remain named, persistable, checkable, inspectable, and potentially visible in the GUI;
- node hierarchy/level semantics must follow the final network ownership model rather than process-flow containment.

The current dummy implementation in which a `DefaultNode` simply forwards an entity should disappear.

---

# 7. Network specializations, not ModalModel specializations

The network formalism belongs in the network hierarchy.

Preferred conceptual structure:

```text
ModelComponent
    |
    +-- ModalModel


ModelDataDefinition
    |
    +-- DefaultNetwork
    |      |
    |      +-- EFSMNetwork
    |      |
    |      +-- MarkovChainNetwork
    |      |
    |      +-- ColoredPetriNetNetwork
    |
    +-- DefaultNode
           |
           +-- EFSMState
           |
           +-- MarkovState
           |
           +-- PetriPlace
```

This replaces the current direction in which classes such as `ModalModelFSM` and `ModalModelPetriNet` specialize the process component.

The ModalModel should be mostly formalism-agnostic. The `DefaultNetwork` specialization defines what one activation actually means.

---

# 8. DefaultNetwork interface

A DefaultNetwork shall define an explicit external interface composed of **input ports** and **output ports**.

Conceptually:

```text
DefaultNetwork
    inputs:
        0: InputA
        1: InputB
        2: InputC

    outputs:
        0: OutputX
        1: OutputY
```

The exact C++ representation should be chosen after fitting it to existing GenESyS persistence, simulation controls, value storage, and plugin infrastructure.

The architecture should distinguish three concepts:

```text
PORT DEFINITION
    identity
    name
    direction
    type/shape metadata

ACTIVATION FRAME
    which inputs are present in this activation
    values carried by those inputs

PERSISTENT NETWORK STATE
    current EFSM/Markov state
    CPN marking
    internal variables
    optional last-observed port values
    statistics
```

Presence/absence during one activation must not be confused with persistent network state.

---

# 9. Why explicit ports are required

Earlier alternatives considered using global GenESyS `Variable` objects as the only network interface, or allowing arbitrary parser expressions with no formal inputs/outputs.

The selected architecture instead uses explicit network ports because this:

- makes the interface visible in the model;
- is structurally closer to Ptolemy II;
- enables validation of network dependencies;
- enables future state refinements;
- makes multiple ModalModels attached to the same network understandable;
- creates a clean model-of-computation boundary;
- prepares GenESyS for richer typed/token connections in the future.

Ptolemy II ModalModel/FSMActor provides an important reference: modal actors expose input/output ports, and those ports are mirrored between the modal model, controller, and refinements.

GenESyS does not need to reproduce Ptolemy implementation details, but should preserve this structural clarity where compatible with its own kernel.

---

# 10. ModalModel mirrors the DefaultNetwork port schema

The `DefaultNetwork` is the authority for its interface.

If a network declares:

```text
inputs:
    0 Temperature
    1 Pressure
    2 Reset

outputs:
    0 State
    1 Alarm
```

then every ModalModel attached to that network must expose the corresponding logical ports:

```text
ModalModel:
    input ports 0..2
    output ports 0..1
```

The exact synchronization mechanism between network interface changes and attached ModalModels is an implementation decision, but inconsistencies must not be silently accepted.

The model checker should be able to detect incompatible or stale port schemas.

---

# 11. ModalModel input bindings

A ModalModel receives a GenESyS `Entity` at one of its input ports.

Each ModalModel input port should have a configurable **binding expression** that determines the value presented to the corresponding network input.

Examples:

```text
Network.Temperature <- Entity.Temperature
Network.Priority    <- Entity.Priority
Network.Load        <- Entity.Size * Factor
Network.Signal      <- Var1 + NQ(Queue1)
```

Using a parser expression rather than only a fixed entity-attribute name gives the interface the flexibility already expected from GenESyS expressions.

The important boundary is:

> The ModalModel may evaluate expressions in the context of the current process entity.  
> The DefaultNetwork itself does not receive or depend on `Entity*`.

Thus:

```text
Entity.Priority
      |
      v
ModalModel binding expression
      |
      v
numeric/typed network input value
      |
      v
DefaultNetwork
```

This avoids coupling the network formalism to the GenESyS process entity abstraction.

---

# 12. Input presence semantics

The Ptolemy II notion of **presence** is valuable and should influence the GenESyS architecture.

An input has two relevant properties for a network activation:

```text
present?
value
```

A missing input is not automatically equivalent to zero.

A missing input is also not automatically equivalent to "use the previous input as if it had just arrived".

For the initial GenESyS ModalModel adapter, one ordinary dispatch normally corresponds to one entity arriving on one input port:

```text
_onDispatchEvent(entity, inputPort = i)
```

The initial activation frame therefore normally contains:

```text
input i:
    present = true
    value = evaluated binding

all other inputs:
    present = false
```

The activation frame should nevertheless be designed to support multiple simultaneously present inputs in the future.

This is important for future integration with models of computation where several inputs may be known or present in the same logical firing.

---

# 13. Persistent last port values

A network may retain the last observed value of each input/output for:

- inspection;
- tracing;
- reporting;
- GUI visualization;
- future semantics that explicitly refer to a previous value.

However:

> `lastValue` and `present` are different concepts.

For example:

```text
Previous activation:
    InputA present=true, value=12

Current activation:
    InputA present=false
```

The network may still remember that InputA's last value was 12, but a guard that depends on the **current presence** of InputA must see it as absent.

The exact public API for current value versus last value remains an implementation detail.

---

# 14. ModalModel activation algorithm

The intended generic algorithm is:

```text
ModalModel::_onDispatchEvent(entity, inputPort)

1. identify attached DefaultNetwork
2. validate inputPort against network schema
3. create a fresh NetworkActivationFrame
4. mark all inputs absent
5. evaluate the binding expression for the arriving inputPort
6. mark that network input present and assign its value
7. invoke network.activate(frame)
8. receive NetworkActivationResult
9. determine which outputs are present
10. route/clone/consume the incoming entity according to output presence
```

`DefaultNetwork::activate()` must represent a single logical network activation.

The internal meaning of that activation is determined by the network specialization.

---

# 15. Network activation result

A network activation returns output presence and output values.

Conceptually:

```text
output 0:
    present = true
    value = 12

output 1:
    present = false

output 2:
    present = true
    value = 7
```

Only **present** outputs produce process entities.

An output that is absent is not interpreted as zero.

---

# 16. ModalModel output bindings

Each ModalModel output port should define where the value produced by the network is written into the outgoing process entity.

The initial practical representation can be an entity attribute binding:

```text
Network output State -> Entity.NetworkState
Network output Alarm -> Entity.AlarmCode
```

This provides an explicit adapter from a network value to the current GenESyS entity-based process semantics.

The exact configuration object should be chosen using existing GenESyS persistence and simulation-control patterns.

No silent destination should be invented if a required output binding is ambiguous.

---

# 17. Output routing and entity cloning

The following semantics are settled.

## 17.1 Zero outputs present

If no network output is present after activation:

```text
entity arrives
network activates
no output present
    |
    v
no entity is emitted
```

The incoming entity is **consumed**.

The implementation must use the correct GenESyS entity lifecycle/removal mechanism so that "consume" does not become a memory leak.

## 17.2 One output present

If one output is present:

- write the corresponding output value to its configured outgoing entity attribute;
- send the original entity through the corresponding ModalModel output port.

No clone is required.

## 17.3 Multiple outputs present

If `N > 1` outputs are present:

- one outgoing path may reuse the original entity;
- `N-1` independent clones are created;
- one entity leaves through each present output port;
- each outgoing entity carries the value corresponding to that output.

The implementation must use the existing canonical GenESyS entity-cloning semantics, not an ad-hoc shallow copy.

A subtle requirement is that clones must represent equivalent copies of the **incoming entity before output-specific mutations contaminate later clones**. A safe implementation should either clone the incoming base entity first or otherwise guarantee that output-specific attribute updates do not leak between output branches.

The choice of which output receives the original entity is semantically irrelevant but should be deterministic, e.g. ascending output-port order.

---

# 18. Sharing one network among multiple ModalModels

Multiple process locations may activate the same network:

```text
             +--> ModalModel A --+
Entity flow  |                   |
             |                   v
             |              Shared Network
             |                   ^
             +--> ModalModel B --+
```

All such ModalModels observe and mutate the same network state.

This permits, for example:

- different process locations to trigger the same EFSM;
- multiple external events to step one DTMC;
- different process branches to fire one shared CPN.

In the current discrete-event architecture, activations should follow the simulator's event-order semantics.

If two ModalModel events occur at the same simulation timestamp, the network will observe them in the order determined by the GenESyS event calendar unless a future model-of-computation mechanism defines a simultaneous/superdense semantics.

This ordering should be deterministic and testable.

A future parallel simulation engine would require explicit synchronization/serialization of shared network activation.

---

# 19. Parser access and network encapsulation

The preferred architecture is that formal network logic depends on:

- network inputs;
- network internal variables/state;
- network parameters;
- explicitly supported model data.

A network does **not** receive the triggering `Entity*`.

However, GenESyS parser expressions may technically be able to reference broader model symbols such as:

```text
NQ(Queue1)
Var1
Vector[5]
```

Such direct references should be treated as **external dependencies**, not as network inputs.

Therefore:

- explicit ports are the recommended network interface;
- direct model references may remain possible where the parser naturally supports them;
- the model checker/documentation should make those dependencies visible when feasible;
- entity attributes should reach the network through ModalModel input bindings rather than through direct `Entity*` access.

This preserves flexibility without erasing the architectural boundary.

---

# 20. Network value type and dimensionality

GenESyS `Variable` already supports scalar and indexed/multidimensional values through existing storage infrastructure.

Network ports should not be unnecessarily limited to the conceptual idea of a scalar forever.

The architecture must allow future inputs such as:

```text
Scalar
Vector[10]
Matrix[3,2]
```

The first implementation may remain numeric if required by the current parser/runtime, but the **port schema should not make future typed or structured values impossible**.

Existing `Variable`/`SparseValueStore` infrastructure should be studied and reused where technically appropriate rather than creating an incompatible second multidimensional-value system.

A network port does not necessarily need to be implemented as a globally registered `Variable`; the desired property is compatible value/shape semantics, not namespace duplication.

---

# 21. Internal activation Counter

`DefaultNetwork` shall contain an internal/composed reporting `Counter` using the existing GenESyS statistics infrastructure where appropriate.

The baseline counter means:

> number of times the network was activated.

Therefore it increments once for every successful call to the network activation operation, regardless of whether an EFSM transition, Markov state transition, or CPN firing actually occurs.

Example:

```text
Activation #17
current EFSM state = Closed
no guard enabled
state remains Closed

ActivationCounter = 17
```

This counter is an example of automatic network reporting.

Specializations may later add separate statistics such as:

- transitions fired;
- state visit counts;
- CPN firings;
- binding counts;
- time/state occupancy where meaningful.

The activation counter resets according to normal GenESyS replication lifecycle rules.

---

# 22. Transition abstractions

A single binary "source node -> transition -> destination node" representation is valid for EFSMs and Markov chains but is not a correct universal representation for CPNs.

A useful conceptual hierarchy is:

```text
DefaultTransition
    |
    +-- DefaultNodeTransition
    |       |
    |       +-- EFSMTransition
    |       |
    |       +-- MarkovTransition
    |
    +-- PetriTransition
```

`DefaultNodeTransition` can represent:

```text
source node ---- transition ----> destination node
```

This fits EFSM and DTMC semantics.

A `PetriTransition`, however, is itself a vertex in the bipartite Petri-net graph and must not be forced into a single-source/single-destination edge abstraction.

The exact C++ hierarchy is still subject to implementation review, but the semantic rule is firm:

> Do not collapse distinct formalisms into an abstraction that destroys their mathematical invariants.

This principle is already consistent with the project's scientific-domain guidance.

---

# 23. EFSMNetwork

The initial EFSM target is a robust Extended Finite-State Machine inspired structurally and semantically by the relevant subset of Ptolemy II `FSMActor`/`ModalModel`.

The first implementation shall support:

- a finite set of states;
- one initial state;
- zero or more final states;
- one current state owned by the EFSMNetwork;
- input ports with presence/value semantics;
- output ports with presence/value semantics;
- guards;
- state/internal variables;
- transition actions;
- state-variable updates;
- transition output actions;
- transition source and destination;
- model validation;
- persistence;
- deterministic and reproducible behavior.

The initial implementation does **not** need:

- state refinements;
- transition refinements;
- preemptive versus non-preemptive transitions;
- error transitions;
- full Ptolemy fixed-point/superdense-time semantics.

These remain future extensions.

---

# 24. EFSM firing semantics

For one network activation:

1. read the current activation input values/presence;
2. inspect outgoing transitions from the current state;
3. evaluate their guards;
4. select/take the valid transition according to the defined conflict policy;
5. execute output actions;
6. execute state/internal-variable update actions;
7. update the current state to the transition destination;
8. produce the activation output result.

The implementation should be inspired by the Ptolemy II split between output/choice actions and commit/set actions where useful.

A key Ptolemy behavior is that guards may refer to input values and to input presence.

The GenESyS implementation should therefore make constructs equivalent to:

```text
InputName
InputName_isPresent
```

available to EFSM expressions through a clean parser scope or equivalent mechanism.

The exact syntax should be selected consistently with the GenESyS parser rather than copied blindly from Ptolemy.

---

# 25. EFSM conflict semantics

Ptolemy II treats multiple simultaneously enabled deterministic transitions as an error unless explicit nondeterministic semantics apply.

The current GenESyS modal code instead sorts enabled transitions by priority and takes the first.

This behavior must be reviewed during migration.

Preferred direction:

- do not silently choose an arbitrary transition;
- make deterministic conflict resolution explicit;
- if priorities are retained, specify their semantics;
- support nondeterminism only through an explicit policy and reproducible GenESyS RNG;
- model checker should detect clearly ambiguous configurations where possible.

This remains an implementation decision to be finalized against current tests and desired compatibility.

---

# 26. Future EFSM state refinements

State refinements are intentionally deferred but must remain architecturally possible.

The likely conceptual direction is:

```text
EFSMState
    |
    +-- optional refinement/submodel
```

A refinement may eventually be an internal submodel associated with a state.

The explicit network input/output port schema is useful here because a future refinement can mirror the same interface, similar to Ptolemy II ModalModel/controller/refinement port mirroring.

No refinement implementation is required in the first consolidation.

---

# 27. MarkovChainNetwork: DTMC only in the initial scope

The initial Markov implementation is a finite-state **Discrete-Time Markov Chain (DTMC)**.

Continuous-Time Markov Chains are explicitly deferred.

A finite time-homogeneous DTMC consists of:

- a finite set of states;
- one current state;
- one initial state/distribution according to the final API;
- transition probabilities `P(i,j)`;
- non-negative probabilities;
- each transition-probability row summing to 1 within an explicitly defined numerical tolerance.

One activation of `MarkovChainNetwork` corresponds to **one discrete Markov step**.

Conceptually:

```text
state i
   |
   | activation
   v
sample j according to P(i,*)
   |
   v
state j
```

The current state belongs to `MarkovChainNetwork`.

The implementation must use the reproducible GenESyS RNG infrastructure, never `std::rand()`.

---

# 28. Strict DTMC semantics versus future controlled chains

For the scientifically validated initial DTMC:

> The transition kernel is determined by the current state and the chain's defined parameters.

If network inputs dynamically modify transition probabilities, the model may become a controlled or time-inhomogeneous Markov process rather than a strict time-homogeneous DTMC.

Such extensions may be useful, but must be labeled correctly instead of being silently called a standard DTMC.

A ModalModel input can still **trigger** a DTMC step without necessarily altering its transition matrix.

Future extensions may explicitly support controlled Markov chains or Markov decision processes.

---

# 29. Future CTMC

Continuous-Time Markov Chains are not part of the first implementation.

A correct CTMC would require a substantially different temporal contract:

- transition rates rather than simple step probabilities;
- exponential holding times;
- internal event scheduling;
- integration with the GenESyS future-event calendar;
- clear ownership of autonomous network events.

Therefore CTMC must not be implemented merely as a DTMC plus an arbitrary delay.

The architecture should leave room for future `CTMCNetwork` or another appropriately named specialization.

---

# 30. ColoredPetriNetNetwork must be a real CPN

The target is a **formal Coloured Petri Net**, not merely a graph with places and string-labeled token counts.

The implementation should follow the established CPN concepts described in the literature by Jensen, Kristensen, and related CPN Tools work.

The CPN must model at least:

- places;
- transitions;
- directed arcs;
- color sets / token types;
- typed token values;
- multiset markings;
- input arc inscriptions;
- output arc inscriptions;
- variables and bindings;
- transition guards;
- transition enabling;
- firing;
- atomic token consumption/production;
- persistence;
- reproducible conflict selection where stochastic resolution is used.

---

# 31. CPN topology is bipartite

A Petri net is not:

```text
Place A ---- PetriTransition-as-edge ----> Place B
```

Instead:

```text
Place A ---- Arc ----> Transition T ---- Arc ----> Place B
```

A transition may have:

- multiple input places;
- multiple output places;
- several arcs;
- inscriptions that depend on variable bindings.

Therefore the current assumption equivalent to:

```text
PetriTransition(sourcePlace, destinationPlace)
```

must be replaced.

Conceptually:

```text
PetriTransition
    inputArcs[]
    outputArcs[]
    guard
    bindings

PetriArc
    place
    transition
    direction
    inscription
```

The exact storage layout may differ, but the formal semantics must be preserved.

---

# 32. CPN marking

The network marking is a mapping from places to multisets of colored token values.

Conceptually:

```text
Marking:
    Place P1:
        2 * RedToken(...)
        1 * BlueToken(...)

    Place P2:
        ...
```

The network logically owns the marking.

Implementation may store local token state in `PetriPlace` objects if that gives cleaner ownership/persistence, but the observable semantics belong to `ColoredPetriNetNetwork`.

The current `std::map<std::string, unsigned int>` representation may be useful as an early/simple case, but is insufficient as the final representation of formal typed colored tokens.

---

# 33. CPN bindings and guards

A CPN transition is enabled through a **binding**, not merely by checking whether one source place contains enough anonymous tokens.

A binding assigns concrete token/data values to variables used by:

- input arc inscriptions;
- the transition guard;
- output arc inscriptions.

An enabled binding must satisfy:

1. input places contain the multiset required by the evaluated input arc inscriptions;
2. the guard evaluates to true;
3. all type/color constraints are satisfied.

A firing must atomically:

- remove required token multisets from input places;
- produce the output multisets defined by the output arc inscriptions.

---

# 34. CPN firing policy

The model designer requested a configurable firing policy with at least two conceptual choices:

- fire one enabled transition/binding;
- fire all simultaneously possible enabled activity.

The phrase **"fire all enabled transitions"** requires precise formal semantics because transitions can compete for the same tokens.

Example:

```text
                +--> T1
Place P: 1 token
                +--> T2
```

T1 may be individually enabled and T2 may be individually enabled, while the pair `{T1,T2}` is not jointly enabled.

The preferred formal interpretation for the "all" mode is therefore a **maximal concurrently enabled step**, not a naïve loop that sequentially fires every transition that happens to remain enabled.

Recommended conceptual property:

```text
enum class FiringMode {
    Single,
    MaximalConcurrentStep
};
```

This exact name/API is not yet mandated.

The original design decision — designer-configurable single versus multiple firing — is settled.  
The exact conflict-selection semantics for the multi-firing mode should be finalized during implementation using the CPN literature and deterministic/reproducible rules.

A separate future `UntilQuiescence` mode may be considered, but it must not be confused with concurrent-step semantics and must handle possible nontermination.

---

# 35. CPN external ports

`ColoredPetriNetNetwork` participates in the same DefaultNetwork external port architecture.

The exact semantics by which an external input value affects a CPN should be explicit in the CPN specialization rather than hidden in ModalModel.

Possible uses include:

- making external input values available to guards/bindings;
- injecting typed values into designated interface places;
- controlling declared CPN parameters.

Likewise, outputs may expose selected values, markings, events, or firing results.

The first implementation should choose a minimal coherent contract and document it rather than creating implicit semantics.

---

# 36. DefaultNetwork activation and specialization semantics

The generic network abstraction defines an activation boundary.

Conceptually:

```text
activate(NetworkActivationFrame)
    -> NetworkActivationResult
```

But the meaning differs by formalism.

## EFSM

```text
one activation
    -> evaluate current-state outgoing transitions
    -> take at most the transition(s) allowed by EFSM policy
    -> update EFSM state
    -> produce outputs
```

## DTMC

```text
one activation
    -> exactly one discrete Markov step
    -> update current state
    -> produce outputs if defined
```

## CPN

```text
one activation
    -> determine enabled bindings
    -> apply configured firing policy
    -> update marking
    -> produce outputs if defined
```

This is why formal semantics belong to network specializations.

---

# 37. Current GenESyS code that must be migrated

At the WorkInProgress snapshot inspected during this design discussion, the existing modal code includes at least:

```text
source/plugins/components/ModalModel/
    DefaultNode.*
    DefaultTransitionExtensions.*
    FSMState.*
    ModalModelDefault.*
    ModalModelFSM.*
    ModalModelPetriNet.*
    PetriPlace.*
    Submodel.*
    CellularAutomataComp.*
    ...
```

Important current characteristics include:

- `ModalModelDefault` owns node and transition lists directly;
- it contains `_entryNode` and `_currentNode`;
- it persists embedded node/transition information;
- it uses per-entity attributes to track current modal node;
- it may execute multiple transitions per dispatch;
- its probabilistic transition code uses `std::rand()`;
- `DefaultNode` is currently a `ModelComponent`;
- `DefaultNode::_onDispatchEvent()` is currently a dummy entity-forwarding implementation;
- `ModalModelFSM` is currently only a thin specialization;
- `ModalModelPetriNet` is currently only a thin specialization;
- `FSMState` already has entry/exit action expression fields;
- `EFSMTransition` exists but does not yet provide the complete desired EFSM semantics;
- `PetriTransition` currently assumes one source and one destination place;
- `PetriPlace` currently tracks token counts using a map keyed by string color.

These structures are valuable scaffolding but should be migrated toward the network-centric architecture rather than extended in-place without reconsidering responsibilities.

---

# 38. Migration principle

Do not perform a blind rewrite.

The migration should preserve useful tested behavior while moving responsibilities to the proper layer.

Conceptual migration:

```text
ModalModelDefault
    node list                  -> DefaultNetwork
    transition list            -> DefaultNetwork / specialization
    current node               -> network specialization
    entry node                 -> network specialization
    probabilistic selection    -> DTMC specialization
    per-entity modal state     -> remove
    entity routing             -> ModalModel adapter
```

Similarly:

```text
ModalModelFSM       -> EFSMNetwork + generic ModalModel
ModalModelPetriNet  -> ColoredPetriNetNetwork + generic ModalModel
```

Compatibility shims may be temporarily justified if existing persisted models/tests depend on old type names, but they should be explicit and time-bounded.

---

# 39. Persistence

Persistence must reflect the new ownership boundaries.

The exact serialized format should be derived from existing GenESyS persistence conventions, but semantically it must preserve:

## ModalModel

- reference to attached DefaultNetwork;
- input binding configuration;
- output binding configuration;
- port/schema compatibility information if required.

## DefaultNetwork

- network identity;
- declared input/output interface;
- network parameters;
- internal activation counter/report configuration;
- network-specific topology/state.

## EFSMNetwork

- states;
- transitions;
- initial state;
- current state where runtime persistence requires it;
- state variables;
- guards/actions;
- port definitions.

## MarkovChainNetwork

- states;
- initial/current state;
- transition probabilities;
- relevant output mappings.

## ColoredPetriNetNetwork

- color sets/types;
- places;
- transitions;
- arcs;
- inscriptions;
- guards;
- marking;
- firing policy.

Load/save round trips must be covered by tests.

---

# 40. Replication lifecycle

Network runtime state must be reset correctly between simulation replications.

Expected examples:

## DefaultNetwork

- reset activation counter;
- clear activation-local presence flags;
- restore initial interface/runtime values as defined.

## EFSMNetwork

- restore initial state;
- restore initial state variables.

## MarkovChainNetwork

- restore initial state/distribution according to the final contract;
- reset RNG-dependent runtime state according to GenESyS reproducibility policy.

## ColoredPetriNetNetwork

- restore initial marking;
- clear activation-local bindings;
- restore any network-specific counters.

The distinction between model configuration and replication runtime state must remain explicit.

---

# 41. Ownership and attached relationships

A ModalModel is **attached** to a network rather than owning it exclusively.

This is essential because multiple ModalModels may share the same network.

Therefore deleting one ModalModel must not destroy a network still referenced elsewhere.

Nodes, transitions, arcs, color sets, and other network internals require clear ownership rules compatible with GenESyS `ModelDataDefinition` association mechanisms.

The exact use of:

- internal;
- attached;
- mandatory/non-editable;
- optional/editable;

must be selected from the real GenESyS association infrastructure during implementation.

No raw-pointer lifetime assumption should be added without tracing the existing model manager behavior.

---

# 42. Model hierarchy levels

GenESyS already associates model data/components with hierarchy levels.

Ordinary composite process components such as `Process` use nested ModelComponents at an internal level.

Network elements are different because they are not process components.

The final level rules for:

- DefaultNetwork;
- DefaultNode;
- EFSMState;
- MarkovState;
- PetriPlace;
- PetriTransition;
- PetriArc;

must be aligned with persistence, GUI visibility, and model data management.

The key invariant is:

> hierarchical level must not imply that network elements are ordinary process-flow ModelComponents.

---

# 43. GUI implications

The GUI currently thinks primarily in terms of drawing ModelComponents connected by Connections.

The new architecture will require the GUI to support editing a network whose graphical elements are **ModelDataDefinitions or network-domain objects**, not process ModelComponents.

This is intentionally a GUI problem, not a reason to misclassify `DefaultNode` as a ModelComponent.

A future network editor will need to draw different graph grammars, for example:

## EFSM

```text
State ---- Transition ----> State
```

## DTMC

```text
State ---- P(i,j) --------> State
```

## CPN

```text
Place ---- Arc ----> Transition ---- Arc ----> Place
```

The editor should therefore operate on an abstract network editing model rather than assuming every drawable node is a process-flow component.

---

# 44. Ptolemy II as structural inspiration

The design intentionally borrows several architectural ideas from Ptolemy II while not attempting source-code or API compatibility.

Useful Ptolemy concepts include:

- modal actors with explicit ports;
- finite-state controller separated from external actor context;
- current state owned by the FSM;
- guards evaluated from current inputs and variables;
- output actions that produce output tokens;
- set/commit actions that modify state;
- distinction between present/absent inputs;
- mirrored ports between modal model, controller, and refinements;
- future state refinements;
- explicit models of computation coordinated through domain semantics.

The GenESyS adaptation differs because ordinary GenESyS Connections currently move Entities between ModelComponents rather than arbitrary Ptolemy tokens between Actors.

ModalModel is therefore an **adapter between models of computation**.

---

# 45. Model-of-computation boundary

A major long-term architectural insight from this work is that GenESyS should not assume forever that every connection transports an `Entity`.

Today:

```text
Connection
    -> destination ModelComponent*
    -> input port number

Event/dispatch
    -> Entity*
```

This is appropriate for process-oriented discrete-event models.

But other models of computation naturally communicate through:

- values;
- tokens;
- events;
- signals;
- structured data;
- absent/present status;
- typed messages.

ModalModel demonstrates the need for a clean bridge between these domains.

---

# 46. ConnectionChannel future evolution

The current GenESyS `ConnectionChannel` already contains the architectural seed for this evolution.

Its current concept is approximately:

```text
ConnectionChannel
    portNumber
    portDescription
```

and existing source comments explicitly anticipate a richer port including concepts such as type and presence/absence of data.

A future design may evolve toward:

```text
ConnectionChannel
    port identity
    name/description
    direction
    type
    shape
    presence
    payload/token metadata
```

This should **not** be implemented as part of the initial ModalModel consolidation unless required by a narrow blocker.

It is a future kernel evolution direction.

---

# 47. Future generalized Connection payloads

Long term, GenESyS may benefit from separating:

```text
Connection topology
```

from:

```text
what travels through that connection
```

Possible future payload abstractions include:

```text
Entity payload
Numeric token
Typed token
Signal/event
Structured value
No payload / pure event
```

A future envelope might conceptually carry:

```text
Message/Token
    type
    presence
    value
    timestamp
    metadata
```

This would allow the same kernel to integrate multiple models of computation without forcing every domain value to be encoded as an Entity.

This is a perspective, not a current implementation mandate.

---

# 48. ModalModel as the first explicit MoC adapter

The ModalModel architecture should be considered a prototype of a more general **model-of-computation adapter** pattern.

Current bridge:

```text
Process-oriented discrete-event domain
Entity + Connection
             |
             v
         ModalModel
             |
             v
Network-domain activation
Ports + values + presence
```

Future bridges could integrate:

- continuous-time solvers;
- synchronous/reactive models;
- dataflow actors;
- signal-processing networks;
- biochemical reaction networks;
- cellular automata;
- co-simulation actors;
- external simulation engines.

Each adapter should preserve the invariants of both domains rather than flattening them into one weak abstraction.

---

# 49. Multiple simultaneous inputs and superdense-time future

The initial GenESyS ModalModel will normally see one entity arrive at one input port per dispatched event.

Nevertheless, `NetworkActivationFrame` should be capable of representing:

```text
Input0 present
Input1 present
Input2 absent
...
```

This future-proofs the network layer.

A later kernel/director mechanism may define:

- coalescing events with equal timestamps;
- superdense-time indices;
- synchronous reactions;
- fixed-point iteration;
- domain-specific scheduling.

These are concepts present in systems such as Ptolemy II and may become relevant to richer GenESyS model-of-computation integration.

They are not required now.

---

# 50. Network outputs versus process entity outputs

A network output is conceptually a **value/token/event produced by the network**.

A ModalModel process output is a **GenESyS entity leaving through an output Connection**.

They are not the same thing.

The ModalModel performs the adaptation:

```text
Network output value
        |
        v
write configured attribute
        |
        v
Entity instance
        |
        v
GenESyS output Connection
```

The cloning behavior is therefore specific to the current process-oriented bridge.

If GenESyS later supports value/token Connections directly, a network output may be transferable without creating/cloning an Entity.

---

# 51. Scientific reproducibility

Any stochastic network behavior must use the GenESyS reproducible random-number infrastructure.

Never use:

```text
std::rand()
```

for validated stochastic semantics.

This applies to:

- DTMC transition selection;
- nondeterministic EFSM resolution when intentionally stochastic;
- stochastic CPN conflict resolution if such a policy is supported.

Tests should fix seeds and verify reproducibility.

Statistical tests should distinguish deterministic contract tests from distributional validation.

---

# 52. Validation requirements: common network layer

Every network implementation should test:

- construction/defaults;
- input/output schema validation;
- invalid port access;
- activation-counter behavior;
- persistence round trip;
- replication reset;
- shared use from more than one ModalModel;
- deterministic same-time activation ordering under current event-calendar rules;
- missing/invalid attached network;
- input-binding expression validation;
- output-binding validation;
- zero-output entity consumption;
- one-output routing;
- multiple-output cloning;
- no accidental cross-contamination between cloned output entities.

---

# 53. Validation requirements: EFSM

At minimum:

- initial state;
- final state;
- current-state persistence/reset;
- guard true/false behavior;
- input value access;
- input presence access;
- state-variable update;
- output action;
- absent output;
- transition to destination;
- self-transition;
- no enabled transition;
- ambiguous multiple enabled transitions according to final policy;
- invalid guard/action expression;
- shared EFSM activated by multiple ModalModels;
- save/load round trip.

Reference behavior should be compared to the defined subset of Ptolemy II semantics.

---

# 54. Validation requirements: DTMC

At minimum:

- finite state set;
- valid initial state;
- nonnegative probabilities;
- row sums equal to one within declared tolerance;
- deterministic transition with probability one;
- absorbing state;
- one activation equals one step;
- seeded reproducibility;
- empirical frequency sanity test over many samples;
- invalid probability matrix;
- persistence;
- replication reset;
- shared chain activated through multiple ModalModels.

Scientific tests should explicitly distinguish implementation correctness from asymptotic/statistical expectations.

---

# 55. Validation requirements: CPN

At minimum:

- color-set/type definitions;
- typed token values;
- multiset marking;
- multiple input places;
- multiple output places;
- weighted/multiset arc inscriptions;
- transition guard;
- valid binding enumeration/selection;
- disabled transition due to insufficient marking;
- atomic consume/produce semantics;
- independent concurrent transitions;
- conflicting transitions competing for the same token;
- single-firing mode;
- configured multi-firing mode after its exact semantics are finalized;
- persistence of topology and initial marking;
- replication reset;
- output presence/value behavior;
- shared CPN activation through multiple ModalModels.

Reference fixtures should be constructed from authoritative CPN examples where licensing permits re-expression of the model.

---

# 56. Non-goals for the first consolidation

The first implementation should **not** expand scope into all possible modal/network features.

Specifically deferred unless a concrete blocker requires them:

- full Ptolemy state refinements;
- transition refinements;
- preemptive/non-preemptive transitions;
- error transitions;
- fixed-point semantics;
- superdense time;
- synchronous-reactive director;
- generalized typed Connection payloads;
- replacing Entity as the general process-flow carrier;
- CTMC;
- Markov decision processes;
- full generic actor framework;
- GUI redesign beyond what is necessary to keep existing functionality buildable.

These directions should remain possible without being prematurely implemented.

---

# 57. Current decisions summary

The following decisions are considered **settled** for the architectural baseline:

1. `ModalModel` remains a `ModelComponent`.
2. `DefaultNetwork` is a `ModelDataDefinition`.
3. One DefaultNetwork may be shared by multiple ModalModels.
4. Network state belongs to DefaultNetwork or its specialization.
5. ModalModel does not store the network current state.
6. Modal state must no longer be stored in entity attributes.
7. `DefaultNode` becomes a `ModelDataDefinition`, not a `ModelComponent`.
8. Formalism specialization belongs to Network subclasses, not ModalModel subclasses.
9. Initial network types are EFSM, finite-state DTMC, and formal CPN.
10. EFSM should be structurally inspired by Ptolemy II.
11. Initial EFSM excludes state refinements, preemption, and error transitions.
12. Initial Markov support is DTMC only.
13. A DTMC activation means one discrete chain step.
14. CPN must support true multi-place/multi-arc colored-net semantics.
15. CPN must support typed/color token values, multisets, guards, inscriptions, and bindings.
16. CPN firing policy is designer-configurable between one firing and a multi-firing policy; exact formal multi-firing conflict semantics must be finalized from CPN theory.
17. DefaultNetwork exposes formal input and output ports.
18. ModalModel mirrors its attached Network's logical port schema.
19. An entity arriving at ModalModel input port `i` supplies that network input for the current activation.
20. ModalModel uses a configurable expression to derive the input value from the arriving entity/model context.
21. DefaultNetwork does not receive `Entity*`.
22. Network input presence and value are distinct.
23. The initial adapter normally marks only the arrived input port as present.
24. Activation frames should be capable of representing multiple present inputs in the future.
25. Network outputs have presence and values.
26. Zero present outputs consume the incoming entity.
27. One present output routes the original entity.
28. Multiple present outputs produce one output entity per present port using cloning.
29. Network output values are written through configured output bindings into outgoing entity attributes in the initial bridge.
30. DefaultNetwork has an internal activation Counter.
31. That Counter counts activations even when no transition/firing occurs.
32. Stochastic choices use GenESyS RNG, never `std::rand()`.
33. Ordinary `Connection` semantics remain process/entity-oriented for now.
34. `ConnectionChannel` and Connection payload semantics are candidates for future generalization.
35. ModalModel is treated as an adapter between models of computation.

---

# 58. Decisions still intentionally open

The following details should not be silently invented:

1. exact C++ names for Network port-definition/frame/result classes;
2. exact value container used for scalar/vector/matrix network ports;
3. whether network last input/output values are public model data or inspection-only runtime state;
4. exact EFSM conflict-resolution policy for multiple enabled transitions;
5. exact formal definition of the CPN multi-firing mode;
6. exact representation of color sets and typed CPN token values;
7. exact expression language for CPN arc inscriptions;
8. exact output-binding behavior when a ModalModel port has no configured destination;
9. exact persistence compatibility strategy for old `ModalModelFSM`/`ModalModelPetriNet` models;
10. exact ModelDataDefinition internal/attached ownership flags for nodes, transitions, arcs, and other network elements;
11. exact GUI editing architecture for DataDefinition-based network graphs.

These should be resolved from code evidence, scientific semantics, and explicit maintainer decisions during implementation.

---

# 59. Recommended implementation sequence

A safe implementation sequence is:

## Phase 1 — document and test current behavior

- inventory all current ModalModel-related classes;
- identify existing tests and persistence;
- characterize old behavior before moving responsibilities.

## Phase 2 — introduce network abstraction

- `DefaultNetwork`;
- network interface schema;
- activation counter;
- activation frame/result;
- persistence/check/reset.

## Phase 3 — migrate DefaultNode

- change conceptual base from ModelComponent to ModelDataDefinition;
- remove process connection/dispatch semantics;
- preserve persistence and plugin registration.

## Phase 4 — consolidate generic ModalModel

- attach DefaultNetwork;
- mirror ports;
- input bindings;
- activate network;
- output bindings;
- consume/route/clone entities.

## Phase 5 — EFSMNetwork

- migrate existing FSM scaffolding;
- implement validated EFSM subset;
- add Ptolemy-inspired presence/output semantics.

## Phase 6 — MarkovChainNetwork

- implement finite-state DTMC;
- reproducible transition selection;
- mathematical validation.

## Phase 7 — ColoredPetriNetNetwork

- replace binary Petri transition assumption;
- implement places/transitions/arcs;
- typed colored tokens;
- multisets;
- inscriptions/guards/bindings;
- firing policy;
- validated CPN fixtures.

## Phase 8 — compatibility cleanup

- remove obsolete per-entity state;
- remove/reduce obsolete ModalModel formalism subclasses;
- replace `std::rand()`;
- migrate old tests/models where appropriate.

## Phase 9 — GUI and future architecture notes

- document required network editor;
- defer generalized Connection payload unless separately approved.

Each phase should be independently buildable, testable, reviewable, and reversible.

---

# 60. Architectural philosophy

The main philosophical rule established by this design is:

> **Preserve the semantics of each model of computation at its boundary.**

Do not make a Petri net look like a process flow merely because GenESyS already has Connections.

Do not make an EFSM state look like a ModelComponent merely because the GUI already knows how to draw ModelComponents.

Do not force a DTMC to use generic probabilistic transition behavior if that loses the mathematical invariants of a stochastic transition matrix.

Do not force network outputs to masquerade as process entities internally.

Instead, use explicit adapters.

This makes the system more modular, more scientifically defensible, and more extensible.

---

# 61. Long-term architectural opportunity

ModalModel exposes a broader opportunity for GenESyS.

The simulator may evolve from:

```text
one dominant model of computation:
process-oriented discrete-event entity flow
```

toward:

```text
multiple explicit models of computation:
    process/discrete-event
    modal/EFSM
    DTMC
    CPN
    continuous
    synchronous/reactive
    dataflow
    biochemical
    spatial/cellular
    external co-simulation
```

The kernel does not need to collapse these models into one abstraction.

Instead, it can provide:

- domain-specific semantics;
- common lifecycle/persistence/reporting;
- explicit adapters;
- typed ports/channels;
- deterministic scheduling boundaries.

The ModalModel/DefaultNetwork architecture is the first concrete step toward that direction.

---

# 62. Repository observations used in this design

The architecture was formulated against the current GenESyS `WorkInProgress` code, including:

- `source/kernel/simulator/model/ModelComponent.h`
- `source/kernel/simulator/model/ModelDataDefinition.h`
- `source/kernel/simulator/ConnectionManager.h`
- `source/plugins/components/DiscreteProcessing/Process.cpp`
- `source/plugins/components/ModalModel/ModalModelDefault.{h,cpp}`
- `source/plugins/components/ModalModel/DefaultNode.{h,cpp}`
- `source/plugins/components/ModalModel/DefaultTransitionExtensions.{h,cpp}`
- `source/plugins/components/ModalModel/ModalModelFSM.{h,cpp}`
- `source/plugins/components/ModalModel/FSMState.{h,cpp}`
- `source/plugins/components/ModalModel/ModalModelPetriNet.{h,cpp}`
- `source/plugins/components/ModalModel/PetriPlace.{h,cpp}`
- `source/plugins/data/Logic/Variable.h`
- `source/plugins/components/Logic/Assign.h`
- `source/plugins/data/Logic/AssignmentItem.h`
- `docs/ai_assistants/reference/SCIENTIFIC_DOMAINS.md`

The WorkInProgress snapshot inspected during the design discussion was based on commit:

```text
b4fe16289b1aa4d0924e979cf795c8cc4d562e88
merge: integrate arena equivalence material-handling additions
```

A later implementation must re-check the current HEAD rather than assuming this snapshot remains current.

---

# 63. Scientific and architectural references

## Ptolemy II

Ptolemy II `FSMActor` documentation  
https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/ptolemy/domains/modal/kernel/FSMActor.html

Ptolemy II `ModalModel` documentation  
https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/ptolemy/domains/modal/modal/ModalModel.html

Ptolemy II modal-kernel package documentation  
https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/ptolemy/domains/modal/kernel/package-summary.html

Ptolemy II project  
https://ptolemy.berkeley.edu/ptolemyII/

Relevant concepts taken as inspiration include explicit ports, current-state ownership, guards, output actions, set/commit actions, input presence/absence, controller/refinement interfaces, and composition of models of computation.

## Coloured Petri Nets

Jensen, Kristensen, Wells — *Coloured Petri Nets and CPN Tools for Modelling and Validation of Concurrent Systems*  
https://cs.au.dk/cpnets/papers/new-overview

Kurt Jensen — *An Introduction to the Practical Use of Coloured Petri Nets*  
https://cs.au.dk/fileadmin/site_files/cs/research_areas/centers_and_projects/cpn/An_Introduction_to_the_Practical_Use_of.pdf

Aarhus University CPN resources  
https://cs.au.dk/cpnets

Jensen and Kristensen — *Coloured Petri Nets: Modelling and Validation of Concurrent Systems*  
https://cs.au.dk/cpnets/books/new

Relevant concepts include places, transitions, directed arcs, color sets, typed token values, multisets, arc expressions/inscriptions, guards, bindings, occurrences, concurrency, and state-space semantics.

## Discrete-Time Markov Chains

Stanford material illustrating finite-state stochastic transition matrices and row-sum invariants:  
https://web.stanford.edu/~ashlearn/RLForFinanceBook/book.pdf

Additional Stanford Markov-process lecture material:  
https://lall.stanford.edu/engr207b/lectures/kalman_filter_2011_02_22_01.pdf

The initial GenESyS Markov target is a finite-state, discrete-time, time-homogeneous chain with reproducible stochastic transition selection.

---

# 64. Final design statement

The target architecture can be summarized as:

```text
PROCESS DOMAIN
===========================================================

ModelComponent
    |
    +-- ModalModel
           |
           | attached
           v

NETWORK DOMAIN
===========================================================

ModelDataDefinition
    |
    +-- DefaultNetwork
    |      |
    |      +-- EFSMNetwork
    |      +-- MarkovChainNetwork
    |      +-- ColoredPetriNetNetwork
    |
    +-- DefaultNode
           |
           +-- EFSMState
           +-- MarkovState
           +-- PetriPlace
```

The external interaction is:

```text
Entity arrives at ModalModel input port
        |
        v
ModalModel evaluates input binding
        |
        v
NetworkActivationFrame
        |
        v
DefaultNetwork::activate()
        |
        v
formalism-specific network reaction
        |
        v
NetworkActivationResult
        |
        +-- zero present outputs -> consume entity
        |
        +-- one present output -> route original entity
        |
        +-- multiple present outputs -> clone and route
```

The design intentionally keeps:

```text
Connection / Entity flow
```

outside the network and:

```text
states / places / transitions / arcs / tokens / probabilities
```

inside the network formalism.

This boundary is the key architectural decision.

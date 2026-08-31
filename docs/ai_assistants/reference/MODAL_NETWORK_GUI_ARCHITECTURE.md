---
document_type: technical-reference
authority: proposed-architecture
owner: project-maintainer
last_reviewed: 2026-08-31
status: proposed
tracks: 511
---

# Modal Network GUI Architecture

## 1. Purpose

This document defines the proposed GUI architecture for creating, editing and
visualizing `DefaultNetwork`-based `ModalModel` networks in the GenESyS Qt6 GUI.

It is a planning document. It does not authorize implementation by itself.
Implementation should begin only after maintainer review because the GUI is the
primary modeling interface for end users.

## 2. Current readiness

Current code is ready for GUI planning, but not yet for GUI editing:

- `DefaultNetwork` is a `ModelDataDefinition` with input/output port schema,
  activation frame/result and an activation counter.
- `DefaultNode` is a `ModelDataDefinition`, not a `ModelComponent`.
- `ModalModelDefault` acts as a process-model adapter to an attached
  `DefaultNetwork`.
- `EFSMNetwork`, graph networks, `MarkovChainNetwork` and
  `ColoredPetriNetNetwork` are registered network data definitions.
- Focused and full unit regressions were green at the latest local checkpoint
  recorded in `BACKLOG_AUTONOMOUS.md`.
- Legacy `.gen` model compatibility is no longer a product requirement by
  maintainer decision on 2026-08-31.

What is not ready yet:

- the GUI has no dedicated editor for `DefaultNetwork` topology;
- the process-flow canvas still represents `ModelComponent + Connection`;
- existing graphical data-definition items are not sufficient for network-owned
  nodes, graph edges, EFSM transitions, Markov transitions and CPN arcs;
- model-specific sample apps still contain legacy construction examples that
  should be migrated or hidden before user-facing release.

## 3. Design References

The architecture is based on established visual modeling patterns:

- Ptolemy II separates abstract model structure from its visual editor, Vergil,
  and supports hierarchical heterogeneous models of computation.
- Simulink uses explicit subsystem navigation; users can double-click a block to
  view or edit its contained subsystem and navigate back through hierarchy.
- Stateflow and AnyLogic use graphical statechart editors with states,
  transitions, conditions, actions and property panels.
- Arena uses palette-driven flowchart modeling and separates graphical modules
  from data modules.
- Eclipse Graphiti and Sirius emphasize strict separation between domain model
  data and diagram representation.
- Qt Graphics View provides the native Qt6 scene/view/item/event framework for
  interactive 2D editors.

Useful source references:

- Ptolemy II, Vergil and MoML: https://ptolemy.berkeley.edu/ptolemyII/
- Ptolemy II modal documentation: https://ptolemy.berkeley.edu/ptolemyII/ptII11.0/ptII/doc/codeDoc/ptolemy/domains/modal/modal/ModalModel.html
- Simulink subsystem navigation: https://www.mathworks.com/help/simulink/ug/navigate-model-hierarchies.html
- Simulink subsystem creation: https://www.mathworks.com/help/simulink/ug/creating-subsystems.html
- Stateflow product documentation: https://www.mathworks.com/products/stateflow.html
- AnyLogic graphical editor: https://anylogic.help/anylogic/ui/graphical-editor.html
- AnyLogic statecharts: https://anylogic.help/anylogic/statecharts/statecharts.html
- Rockwell Arena product page: https://www.rockwellautomation.com/en-us/products/software/arena-simulation.html
- Qt Graphics View Framework: https://doc.qt.io/qt-6/graphicsview.html
- Qt `QGraphicsScene`: https://doc.qt.io/qt-6/qgraphicsscene.html
- Eclipse Graphiti overview: https://eclipse.dev/graphiti/documentation/overview.html
- Eclipse Sirius documentation: https://eclipse.dev/sirius/doc/
- Harel, "Statecharts: A Visual Formalism for Complex Systems": https://www.sciencedirect.com/science/article/pii/0167642387900359

## 4. Core UX Decision

Recommended user interaction:

1. The process model remains the top-level canvas.
2. A `ModalModelDefault` appears as a process component with ordinary
   process-flow input/output ports.
3. The component context menu includes `Open Network...`.
4. Double-clicking a `ModalModelDefault` should also open the attached network
   when a network exists; otherwise it should offer to create one.
5. Opening a network creates a new editor level in the same main window, using a
   tab or breadcrumb stack:

```text
Process Model / ModalModelName / NetworkName
```

6. The user edits the network using a formalism-specific palette and property
   panel.
7. Returning to the parent process model keeps the network attached to the
   `ModalModelDefault`.

This follows the proven subsystem/statechart pattern: the outer component is an
adapter in the process-flow model; the inner diagram is a different model of
computation with its own notation.

## 5. Layered Architecture

Use a strict three-layer split:

```text
Kernel domain model
    DefaultNetwork
    GraphNetwork / EFSMNetwork / MarkovChainNetwork / ColoredPetriNetNetwork
    GraphNode / GraphEdge / FSMState / EFSMTransition / MarkovState / CPNTransition / CPNArc / PetriPlace

GUI diagram model
    GraphicalNetworkScene
    GraphicalNetworkNode
    GraphicalNetworkEdge
    GraphicalNetworkPort
    GraphicalNetworkAnnotation
    GraphicalNetworkLayoutRecord

GUI controllers and commands
    NetworkEditorController
    NetworkNavigationController
    NetworkPaletteController
    NetworkPropertyController
    NetworkCommand stack
```

The GUI diagram model stores positions, bends, colors, labels, collapsed state,
selection state and editor metadata. It must not be the source of truth for
simulation semantics.

The kernel domain model stores topology, semantics, persistence, model checking
and activation behavior. It must not depend on Qt.

## 6. New GUI Concepts

### GraphicalNetworkScene

`GraphicalNetworkScene` should wrap a `QGraphicsScene` for exactly one
`DefaultNetwork` instance.

Responsibilities:

- render network-domain nodes and relations;
- expose hit testing, selection, drag, rubber-band selection and guides;
- dispatch mouse events to tools;
- request synchronization after kernel-domain edits;
- keep graphical records separate from kernel persistence.

It should not dispatch entities, schedule simulation events or mutate process
`Connection` objects.

### GraphicalNetworkNode

Visual wrapper for network-owned node-like elements:

- `GraphNode`;
- `FSMState`;
- `MarkovState`;
- `PetriPlace`;
- `CPNTransition`.

It owns only graphical state and a non-owning pointer or stable model reference
to the underlying `ModelDataDefinition`.

### GraphicalNetworkEdge

Visual wrapper for formalism-owned relation-like elements:

- `GraphEdge`;
- `EFSMTransition`;
- `MarkovTransition`;
- `CPNArc`.

It is not `GraphicalConnection` because it is not a process-flow `Connection`.

### NetworkDiagramRecord

Persist GUI-only diagram state separately from kernel `.gen` semantics:

- network name or stable data-definition identifier;
- graphical item positions;
- edge bend points;
- label positions;
- collapsed/expanded state;
- style overrides;
- editor viewport state.

The current `.gui` serializer already persists graphical process items. Extend
that concept rather than placing GUI coordinates inside `DefaultNetwork`.

## 7. Controller and Command Pattern

Use command-based editing for every mutation:

- `CreateNetworkNodeCommand`;
- `DeleteNetworkNodeCommand`;
- `MoveNetworkNodeCommand`;
- `CreateNetworkEdgeCommand`;
- `DeleteNetworkEdgeCommand`;
- `SetNetworkPropertyCommand`;
- `AttachNetworkToModalModelCommand`;
- `ChangeModalBindingCommand`.

Each command must update the kernel model first, then update or rebuild the
diagram representation. Commands must support undo/redo where the current GUI
undo stack can express it.

This mirrors established graphical editor practice from GEF/Graphiti: user
input becomes a request; policy validates it; commands mutate the domain model.

## 8. Formalism-Specific Palettes

The network editor must derive its palette from the attached network type.

### GraphNetwork

Palette:

- node;
- undirected edge;
- directed edge only when editing `DirectedGraphNetwork`;
- weighted edge toggle;
- auto-layout;
- shortest path analysis overlay;
- connectivity analysis overlay.

Notation:

```text
A ----- B
A --4.7-- B
A -----> B
```

No entity movement should be shown unless a future traversal specialization is
attached.

### EFSMNetwork

Palette:

- state;
- initial state marker;
- final state marker;
- transition;
- guard/action label;
- priority label.

Notation:

```text
StateA -- [guard] / action --> StateB
```

Runtime overlay:

- current state highlight;
- last fired transition pulse;
- activation count badge.

### MarkovChainNetwork

Palette:

- Markov state;
- probabilistic transition;
- absorbing-state shortcut.

Notation:

```text
A --0.25--> B
A --0.75--> C
```

Validation feedback:

- each outgoing row must sum to 1 within tolerance;
- invalid rows get an inline warning and property-panel diagnostic.

### ColoredPetriNetNetwork

Palette:

- place;
- transition;
- input arc;
- output arc;
- token/color multiset editor;
- guard editor;
- inscription editor.

Notation:

```text
Place(red:2) --> [Transition] --> Place(blue:1)
```

Visual grammar:

- places are circles or rounded circles;
- transitions are bars or rectangles;
- arcs are only place-to-transition or transition-to-place;
- invalid place-to-place and transition-to-transition links are rejected while
  dragging, not merely at model-check time.

## 9. Navigation Model

Use hierarchical editor levels:

```text
Level 0: process model canvas
Level 1: attached DefaultNetwork canvas
Level 2: future nested state refinement or subnetwork, if approved
```

Required UI elements:

- breadcrumb bar;
- back/up action;
- tab title including network name and formalism;
- minimap for large networks;
- synchronized model inspector tree;
- context menu on `ModalModelDefault` with `Open Network...`,
  `Attach Existing Network...`, `Create EFSM Network...`,
  `Create Markov Chain...`, `Create CPN...`, `Create Graph...`.

If a `ModalModelDefault` has no network, the first open action should display a
network creation chooser rather than silently creating a default formalism.

## 10. Property Editing

Selection drives the property editor:

- selecting a process `ModalModelDefault` edits adapter properties and
  input/output bindings;
- selecting a network node edits node properties;
- selecting a network edge edits relation properties;
- selecting blank network canvas edits network-level properties.

Properties that belong to the scientific formalism must be edited through the
kernel object, not through graphical items.

Examples:

- `MarkovTransition::probability` belongs to the domain object;
- edge bend points belong to the diagram record;
- CPN arc inscription belongs to `CPNArc`;
- color, stroke width and label offset belong to the diagram record unless the
  property has semantic meaning.

## 11. Validation and Feedback

The editor should validate early and continuously:

- disallow invalid endpoints during edge drag;
- show missing required properties inline;
- show model-check diagnostics in an Problems panel;
- keep a per-network validation badge;
- expose one-click navigation from a diagnostic to the offending diagram item.

Validation sources:

- formalism constraints from `_check()`;
- parser checks for guards, actions and bindings;
- topology constraints from the network type;
- GUI-only consistency checks for orphan diagram items.

Do not duplicate scientific validation logic in the GUI. The GUI may pre-check
obvious constraints, but the kernel remains authoritative.

## 12. Runtime Visualization

Runtime visualization should be read-only with respect to topology:

- highlight active EFSM or Markov state;
- pulse fired EFSM transition or CPN transition;
- animate token-count changes in CPN places;
- show activation counts and last result outputs;
- optionally overlay graph analysis results such as selected path or reachable
  set.

The runtime overlay must be separate from model editing state. Pausing or
stopping simulation should leave the editable diagram unchanged.

## 13. Persistence

Persist two independent artifacts:

```text
.gen
    kernel model: components, data definitions, networks, topology, semantics

.gui
    graphical model: process layout, network layout, style, viewport, annotations
```

Rules:

- `.gen` must be sufficient for execution without GUI metadata;
- `.gui` must be recoverable when a referenced domain object is missing by
  dropping or quarantining only the orphan graphical item;
- network diagram entries should reference domain objects by stable name plus
  type, and later by stable IDs if the kernel adds them;
- GUI persistence must preserve multiple diagrams per model.

## 14. Migration Policy

Maintainer decision on 2026-08-31: old persisted `ModalModelFSM` and
`ModalModelPetriNet` models are not a compatibility requirement.

Therefore:

- do not spend GUI implementation effort on automatic old-model migration;
- hide or de-emphasize legacy node-list construction in the GUI;
- keep legacy classes only as temporary code compatibility if source examples or
  tests still compile through them;
- new user-facing GUI workflows must create `DefaultNetwork` data definitions
  and attach them to `ModalModelDefault`.

## 15. Implementation Phases

### Phase G0 - Inventory and contracts

- map current GUI scene, serializer, property editor, plugin catalog and context
  menu paths;
- define `NetworkDiagramRecord` persistence format;
- add tests for network diagram record serialization without launching the GUI.

### Phase G1 - Read-only network viewer

- add `GraphicalNetworkScene`;
- render nodes and edges for existing networks;
- add context menu action `Open Network...` on `ModalModelDefault`;
- add tab or breadcrumb navigation;
- no topology edits yet.

Acceptance:

- build `gui-app`;
- focused GUI model/diagram tests pass;
- opening a ModalModel with an attached network displays the correct network
  type and topology.

### Phase G2 - Create and attach network

- add chooser for network type when a `ModalModelDefault` has no network;
- create `DefaultNetwork` subclasses through plugin/factory paths;
- attach the selected network to the ModalModel adapter;
- expose adapter bindings in the property editor.

Acceptance:

- create EFSM, Markov, CPN and Graph networks from the GUI;
- save and reload `.gen` plus `.gui`;
- no process `Connection` objects are created for network arcs.

### Phase G3 - Interactive topology editing

- add palette tools for node and edge creation;
- implement command-based create/delete/move/edit operations;
- enforce formalism endpoint rules during drag;
- add undo/redo coverage.

Acceptance:

- create/remove nodes and edges in each supported network formalism;
- invalid CPN arcs are rejected before commit;
- model checker agrees with editor validation.

### Phase G4 - Property editing and validation UX

- edit guards, actions, probabilities, weights, inscriptions, initial states and
  markings through the property editor;
- add inline diagnostics and a Problems panel;
- add row-sum feedback for Markov chains and enabled/firing diagnostics for CPN.

### Phase G5 - Runtime visualization

- add read-only simulation overlays;
- show current state, fired transition, CPN token changes and activation count;
- keep edit state and runtime state isolated.

### Phase G6 - Polish and usability

- add auto-layout and alignment tools;
- add minimap and zoom-to-fit;
- add keyboard shortcuts;
- add visual style themes for formalism types;
- validate startup and a minimal manual interaction scenario under Xvfb.

## 16. Non-Goals

- no process `Connection` generalization in the first GUI implementation;
- no full CPN variable binding or typed token system in the GUI before the
  kernel semantics are implemented;
- no old persisted model migration;
- no dynamic plugin ABI redesign;
- no separate web GUI.

## 17. Recommended First Implementation Slice

The first code slice should be G1:

```text
right-click ModalModelDefault
    -> Open Network...
    -> new tab/breadcrumb level
    -> read-only GraphicalNetworkScene renders attached network topology
```

This produces immediate user value, proves the navigation model, and keeps risk
bounded before adding editing commands.


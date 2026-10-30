---
document_type: technical-reference
authority: maintainer-approved-architecture
owner: project-maintainer
last_reviewed: 2026-10-29
status: active
tracks: 511
---

# Modal Network GUI Architecture

## 1. Purpose

This document defines the proposed GUI architecture for creating, editing and
visualizing `DefaultNetwork`-based `ModalModel` networks in the GenESyS Qt6 GUI.

This is a conceptual GUI reference: names of proposed GUI classes and commands
are **not** evidence that they already exist in source.

**Maintainer decisions (2026-10-29, DEC-MODAL-GUI-20261029 and
DEC-MODAL-LEVEL-20261029):** choose **Alternative A: evolve the existing
submodel editor**, including its `ModelGraphicsScene`, level filter and
hierarchical tab/navigation context. A non-root `level` is the ID of the
`ModelDataDefinition` that defines the editing context (which can also be a
`ModelComponent`, since it inherits `ModelDataDefinition`); root is level 0.
A `ModalModelDefault` opens its attached `DefaultNetwork` context, identified
by **the Network ID**, not by the adapter ID.

States, places, graph vertices, transitions and arcs do not become
`ModelComponent` or process `Connection` objects for visualization.
`DefaultNetwork` remains the mathematical source of truth and may be shared
by several `ModalModelDefault` adapters. A common canvas supports different
domain projections and interaction commands; reusing the canvas never means
reusing Entity/Connection process semantics. The first visual slice is a
read-only EFSM representation. See the operational, source-backed
[implementation plan](MODAL_NETWORK_GUI_IMPLEMENTATION_PLAN.md) and
`../BACKLOG_HUMAN.md` (`HUM-MODAL-002`). Other detailed class/interface
names below are conceptual proposals, not claims of implemented APIs.

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

## 5. Layered Architecture — shared submodel editor

Three conceptual layers remain strictly separated:

```text
Kernel domain model (never depends on Qt)
    ModelDataDefinition / ModelComponent (hierarchy context identity)
    DefaultNetwork: EFSM, Graph, Markov DTMC, Colored Petri Net
    Domain nodes and formalism-specific relationships

Common Qt graphical scene/view/navigation
    ModelGraphicsScene / ModelGraphicsView / existing Submodel tabs
    level-filtered context: level=0 or owner ModelDataDefinition::getId()
    GUI-only positions, routes, styles, viewport records

Formalism-specific graphical projection and commands
    Process: ModelComponent + process Connection
    EFSM: FSMState + EFSMTransition (read-only first)
    Graph: GraphNode + GraphEdge
    DTMC: MarkovState + MarkovTransition
    CPN: PetriPlace + CPNTransition + CPNArc
```

A non-root level is **not** an ordinal depth, a domain ownership transfer or a
simulation execution mode. A single Network shared by several ModalModel
adapters retains one mathematical state and one Network-ID editing context.
A context projection may be configured according to its owner type, using a
bounded strategy/policy rather than embedding every formalism in the scene.

The GUI persists presentation metadata but never transition firing,
probabilities, guards, mathematical state or token semantics. Those remain in
the Network domain object. No Qt dependency is introduced into the kernel.

## 6. GUI Concepts — reuse first, specialize where required

### Shared ModelGraphicsScene and submodel tabs

Reuse the already implemented `GraphicalModelViewContext` and tab lifecycle.
`MainWindow::_ensureSubmodelViewContext()` currently sets the level filter
to the passed owner's ID and can therefore be called with the Network
`ModelDataDefinition`, after resolving the attached Network from the selected
`ModalModelDefault`. The existing process `Submodel` component is not a
general-purpose implementation of mathematical topology and retains
incomplete load/save/check methods; do not change its dispatch semantics.

The existing `GraphicalModelBuilder` must grow a bounded, type-aware
projection path: the level filter discovers data definitions even if there are
no process `ModelComponent` objects at that level, and domain topology
generates correct formal relations. Reusing the existing scene does **not**
mean interpreting `GraphicalDiagramConnection` (internal/attached data links)
or `GraphicalConnection` (Entity flow) as a mathematical edge.

### Domain node presentation

A node-like GUI item refers non-owningly to an existing Network element
(`FSMState`, `GraphNode`, `MarkovState`, `PetriPlace`, `CPNTransition`).
The existing `GraphicalModelDataDefinition` and rendering strategy may be
extended or wrapped where appropriate, without modifying the domain type.
Rendering an EFSM state does not create a process `ModelComponent`.

### Domain relation presentation

Formal relations use their domain APIs. `EFSMTransition` and the present
Markov transition type do not derive from `ModelDataDefinition` and thus do
not have a `level` property; the EFSM/DTMC Network's transition collections
supply them. `GraphEdge` and `CPNArc` are `ModelDataDefinition` objects and
can also carry the Network's context level. The renderer must preserve
direction, parallel relations, edge labels and the meaning of the formalism.

A specialized graphical edge item may be introduced if the generic data link
item cannot represent these details. That is a **graphic item specialization
inside the shared scene**, not an independent Network scene.

### NetworkDiagramRecord

Persist GUI-only state in the existing `.gui` overlay, separately from
the canonical kernel/Network semantics:

- parent model and Network identity;
- node positions and formal-relation bend/label points;
- style annotations, collapsed state and editor viewport;
- a versioned record format with tolerant loading of missing visual items.

The file format currently combines the kernel model with a comment-prefixed
GUI overlay, so do not assume separate physical files. The GUI representation
must not be required for headless execution. Graphical identity for unnamed
and parallel transitions remains a gated G0.2 requirement.

### Implementation boundary

Prefer a shared scene plus isolated projection/interaction policies. If
further code inspection demonstrates substantial coupling or untestable
ownership conflicts, re-evaluate this implementation technique explicitly;
do not silently substitute an entirely separate scene for the approved
Alternative A.

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

Use hierarchical editing contexts keyed by owner identity (not ordinal depth):

```text
level = 0                         -> root process model
level = Network_X.getId()         -> Network_X mathematical diagram
level = Component_Y.getId()       -> Component_Y process submodel
level = child_owner.getId()       -> optional nested context, if supported
```

The editor resolves a selected `ModalModelDefault` to its attached Network,
then reuses the existing Submodel tab/level filter with the **Network as owner**.
Two ModalModel components sharing a Network open the same logical context.

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

- extend the existing level-filtered `ModelGraphicsScene`/builder with a
  read-only EFSM projection; do **not** create an independent scene;
- render Network states and typed transitions from the attached domain object;
- add a context action `Open Network...` on `ModalModelDefault`;
- reuse existing Submodel tabs and the Network's `getId()` as context level;
- no topology edits or process-connection behavior yet.

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
    -> resolve the attached DefaultNetwork
    -> reuse existing Submodel tab with level = Network.getId()
    -> read-only EFSM projection in ModelGraphicsScene
```

This produces immediate user value, proves the navigation model, and keeps risk
bounded before adding editing commands.


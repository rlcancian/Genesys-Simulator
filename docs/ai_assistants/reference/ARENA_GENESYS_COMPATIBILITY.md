---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-08-29
review_cadence: on-audit-batch
status: active
---

# Arena ↔ GenESyS Compatibility Matrix

## 1. Purpose and scope

This document is a traceable, incrementally built specification of the relationship
between Rockwell Arena's data modules/flowchart modules and GenESyS's
`ModelDataDefinition`/`ModelComponent` plugins under `source/plugins/` (and, for
entity/attribute foundations, `source/kernel/simulator/essentialPlugins/`).

The goal is **not** to make GenESyS reproduce Arena. It is to:

- record which Arena concepts have a confirmed, partial, missing, or deliberately
  different GenESyS counterpart;
- record exact source/page evidence for every Arena claim;
- separate confirmed-in-code facts from strong indications and open questions;
- flag differences that require a maintainer decision instead of silently resolving
  them by assumption (`GOVERNANCE.md` §11).

This is a living audit document. Most of the full inventory in §4 is still
`unknown` (not yet analyzed). Only entries explicitly detailed in §5 onward are
confirmed. Do not treat an `unknown` row as either "equivalent" or "missing" —
it means the audit has not reached that module yet.

## 2. Arena reference sources (local, non-versioned)

These sources live under `docs/gitignore_Arena_references/` (ignored via the
`gitignore_*` pattern in `.gitignore`; never commit the PDFs/books themselves):

| Short name | File | Pages | Conversion status |
|---|---|---|---|
| Getting Started | `getting-Started-with-Arena.pdf` | 204 | PDF only; Markdown checkpoint stalled at 94/204 pages (46%) |
| Basic Edition Guide | `Arena_Basic_Edition_User_s_Guide11.pdf` | 88 | PDF only; Markdown checkpoint stalled at 84/88 pages (95%, failed at p.6) |
| Kelton et al., *Simulation with Arena*, 6th ed. | `1__Kelton_e_outros_..._6a_ED.pdf` / `.md` | 656 | Full Markdown available (14,674 lines) |
| Arena Variables Guide | `arena-variables-guide-....pdf` / `.md` | 87 | Full Markdown available (2,883 lines) |

For **Getting Started with Arena** and the **Basic Edition Guide**, only a
checkpoint stub exists in Markdown (no usable body text); citations below were
read directly from the PDF page images using the page-range PDF reader and are
paraphrased, not copied. Cite as "*Getting Started with Arena*, p. NN" (printed
page number, not PDF page index — the PDF has ~9 front-matter pages before
printed p. 1).

## 3. Methodology and terminology

- Arena **Data Module** ≈ candidate GenESyS `ModelDataDefinition`
  (`source/plugins/data/**`, plus `source/kernel/simulator/essentialPlugins/`
  for Attribute/EntityType/Entity, which are kernel-level rather than
  plugin-level data definitions).
- Arena **Flowchart Module** ≈ candidate GenESyS `ModelComponent`
  (`source/plugins/components/**`). Audit started under Phase B, §6.
- Equivalence is never assumed from name similarity alone; every entry below
  states what was actually inspected.

Classification levels used in this matrix:

- `equivalent` — parameters and behavior confirmed to match within the noted scope;
- `partial` — some parameters/behavior match, some are missing or narrower;
- `missing` — no GenESyS data definition covers the Arena concept;
- `different` — GenESyS implements the concept with a deliberately different
  design (may be broader or narrower; not automatically a defect);
- `not-applicable` — the Arena concept depends on Arena-only infrastructure
  (e.g. 2D/3D animation) that is out of GenESyS's current scope;
- `unknown` — not yet analyzed.

Evidence tags used inline:

- **[confirmed]** — read directly in current `source/` code on this branch;
- **[strong indication]** — supported by multiple observations, not exhaustively
  traced through every `.cpp` path;
- **[hypothesis]** — plausible, explicitly unconfirmed, needs a follow-up read;
- **[needs-human-decision]** — a maintainer must choose whether/how to close the
  gap; not to be resolved by an autonomous agent.

## 4. Full module inventory (checklist)

Legend: ✅ analyzed in this document (see linked section), ⬜ not yet analyzed.

### 4.1 Data modules

| Arena module | Panel | Status |
|---|---|---|
| Attribute | Basic Process | ✅ [§5.1](#51-attribute) |
| Entity | Basic Process | ✅ [§5.2](#52-entity--entitytype) |
| Queue | Basic Process | ✅ [§5.3](#53-queue) |
| Resource | Basic Process | ✅ [§5.4](#54-resource) |
| Variable | Basic Process | ✅ [§5.5](#55-variable) |
| Schedule | Basic Process | ✅ [§5.6](#56-schedule) |
| Set | Basic Process | ✅ [§5.7](#57-set) |
| Advanced Set | Advanced Process | ✅ [§5.7](#57-set) (same GenESyS class) |
| Expression | Advanced Process | ✅ [§5.8](#58-expression--formula) |
| Failure | Advanced Process | ✅ [§5.9](#59-failure) |
| File | Advanced Process | ✅ [§5.10](#510-file) |
| StateSet | Advanced Process | ✅ [§5.4](#54-resource) (recorded as missing) |
| Statistic | Advanced Process | ✅ covered by kernel `Statistics`/`Collector`/`StatisticsCollector`/`Counter`, attached to Record and to many components/data definitions — per maintainer clarification (2026-08-29); full class-by-class detail deferred, see §9 |
| Storage | Advanced Process | ✅ [§5.11](#511-storage) |
| Sequence | Advanced Transfer | ✅ [§5.12](#512-sequence) |
| Conveyor | Advanced Transfer | ✅ [§7](#7-advanced-transfer-conveyortransporter-subsystem-status) |
| Segment | Advanced Transfer | ✅ [§5.14](#514-segment) |
| Transporter | Advanced Transfer | ✅ [§7](#7-advanced-transfer-conveyortransporter-subsystem-status) |
| Distance | Advanced Transfer | ✅ [§5.13](#513-distance) |
| Network | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Network Link | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Activity Area | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Regulator Set | Flow Process | ✅ `future-domain-feature` (§6.22) |

### 4.2 Flowchart modules (components) — Phase B

| Arena module | Panel | Status |
|---|---|---|
| Create, Dispose, Assign, Process, Decide, Batch, Separate, Record | Basic Process | ✅ [§6](#6-phase-b--components-analyzed-entries) (batches 1-2) — all Basic Process flowchart modules covered |
| Delay, Seize, Release, Hold(→Wait), Match, Pickup, Remove, Search, ReadWrite(→Write), Dropoff, Store, Unstore, Signal, Adjust Variable | Advanced Process | ✅ §6.3, §6.5, §6.10–§6.19 — **all Advanced Process flowchart modules covered** |
| Enter, Leave, PickStation, Route, Station (as flowchart module) | Advanced Transfer (general) | ✅ [§6.20](#620-advanced-transfer-general-enter-leave-pickstation-route-station-concept) |
| Access, Convey, Exit, Start, Stop | Advanced Transfer (conveyor) | ✅ `Access`/`Exit`/`Start`/`Stop` now covered by a minimal executable Conveyor contract; `Convey` remains intentionally collapsed into that contract plus `Route`/station transfer (§6.21, §7) |
| Activate, Allocate, Free, Halt, Move, Request, Transport | Advanced Transfer (transporter) | ✅ `Move` now covered by a minimal executable Transporter contract; the remaining named Arena modules stay classified as future/unsupported under the simplified GenESyS abstraction (§6.21, §7) |
| Tank, Sensor, Flow, Regulate, Seize Regulator, Release Regulator | Flow Process | ✅ `future-domain-feature` (§6.22) |

## 5. Phase A — Data definitions: analyzed entries

### 5.1 Attribute

- **Arena**: "Attribute module", *Getting Started with Arena*, "The Basic Process
  Panel", p. 43. Defines an attribute's Name, Rows, Columns, Data Type
  (Real/String), and Initial Values. Per-entity (unlike Variable, which is global).
- **GenESyS**: `Attribute` — `source/kernel/simulator/essentialPlugins/Attribute.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: Name → `ModelDataDefinition::getName/setName` **[confirmed]**;
  Rows/Columns → `insertDimentionSize()` + `SparseValueStore` indexing
  **[confirmed]**; Initial Values → `getInitialValuesText()/setInitialValuesText()`
  (bracket notation) and `getInitialValue()/setInitialValue()` **[confirmed]**;
  Report Statistics → inherited `ModelDataDefinition::isReportStatistics()/
  setReportStatistics()` **[confirmed]**.
- **Behavior**: values stored in a sparse map keyed by textual index rather than a
  dense array; absent indices read as `0.0` **[confirmed]**.
- **Divergence**: no "String" data type — only numeric (`double`) values are
  modeled **[confirmed]**.
- **Needs human decision**: no.

### 5.2 Entity / EntityType

- **Arena**: "Entity module", *Getting Started with Arena*, "The Basic Process
  Panel", pp. 44-45. Defines Entity Type (name), Initial Picture, Holding
  Cost/Hour, and Initial VA/NVA/Waiting/Transfer/Other Cost.
- **GenESyS**: two distinct kernel classes, previously conflated in comments:
  - `EntityType` (the data definition) — `essentialPlugins/EntityType.{h,cpp}`;
  - `Entity` (the runtime token/instance created per replication) —
    `essentialPlugins/Entity.{h,cpp}`.
- **Level**: `partial` for `EntityType`; `different` (not a data definition at all)
  for `Entity`.
- **Documentation defect found and fixed in this batch**: `Entity.h` carried a
  verbatim copy of Arena's *Entity module* description (a data-module
  description) even though `Entity` is the runtime token class, not the
  configurable data definition — `EntityType` is. Both headers were rewritten to
  describe the class actually in front of them and cross-reference each other.
- **Parameters on `EntityType`**: Initial Picture → `initialPicture()/
  setInitialPicture()` **[confirmed]**; Initial VA/NVA/Waiting/Other Cost →
  `initialVACost()`, `initialNVACost()`, `initialWaitingCost()`,
  `initialOtherCost()` **[confirmed]**.
- **Divergence**: no explicit "Holding Cost/Hour" field, and no explicit
  "Initial Transfer Cost" field/accessor exists on `EntityType` — only four of
  Arena's five cost categories have a dedicated accessor **[confirmed:
  absence — no matching getter/setter in `EntityType.h`]**.
- **Needs human decision**: whether to add Holding Cost/Hour and Transfer Cost as
  a fifth/sixth cost category, or keep the current four-category model —
  `needs-human-decision` (cost-accounting semantics, not a proven bug).

### 5.3 Queue

- **Arena**: "Queue module", *Getting Started with Arena*, "The Basic Process
  Panel", pp. 45-46. Ranking rule (FIFO/LIFO/Lowest-/Highest-Attribute-Value),
  optional evaluated Attribute Name, a "Shared" checkbox, and Report Statistics.
- **GenESyS**: `Queue` — `source/plugins/data/DiscreteProcessing/Queue.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `OrderRule` enum (`FIFO/LIFO/HIGHESTVALUE/SMALLESTVALUE`) →
  `getOrderRule()/setOrderRule()` **[confirmed]**; evaluated attribute →
  `getAttributeName()/setAttributeName()` **[confirmed]**; Report Statistics →
  inherited `isReportStatistics()`, confirmed to gate creation of the
  `NumberInQueue`/`TimeInQueue` `StatisticsCollector` instances in
  `Queue::_createInternalStatisticReporters()` (`Queue.cpp:219-238`)
  **[confirmed]**.
- **Divergence**: no explicit "Shared" boolean. In Arena a queue is private to
  the module that created it unless explicitly marked Shared; in GenESyS a
  `Queue` is simply a referenceable `ModelDataDefinition`, so any number of
  components may already reference the same instance without an opt-in flag —
  recorded as `different` (architectural default), not `missing`.
- **Needs human decision**: no.

### 5.4 Resource

- **Arena**: "Resource module", *Getting Started with Arena*, "The Basic Process
  Panel", pp. 46-48; "StateSet module", "The Advanced Process Panel", pp. 73-74.
  Capacity (fixed or schedule-based), Schedule Name/Rule, Busy/Idle/Per-Use
  costs, StateSet Name + Initial State, Failures list, Report Statistics.
- **GenESyS**: `Resource` — `source/plugins/data/DiscreteProcessing/Resource.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `_capacity` → `getCapacity()/setCapacity()` **[confirmed]**;
  `_capacitySchedule` (a `Schedule*`) → `getCapacitySchedule()/
  setCapacitySchedule()` **[confirmed]**; `_costBusyTimeUnit`,
  `_costIdleTimeUnit`, `_costPerUse` **[confirmed]**; `_failures`
  (`List<Failure*>`) → `insertFailure()/removeFailure()` **[confirmed]**.
- **Divergence 1 (`missing`)**: Arena's StateSet module lets a modeler define an
  arbitrary named set of states, each mapped to an autostate (Idle/Busy/
  Inactive/Failed) or a Failure name. GenESyS only exposes a fixed
  `ResourceState` enum (`IDLE/BUSY/FAILED/INACTIVE/OTHER`) with no user-defined
  state catalog **[confirmed: no StateSet-equivalent class found under
  `source/plugins/data/`]**.
- **Divergence 2 (`hypothesis`, needs `.cpp`/behavior confirmation)**: Arena's
  per-resource "Schedule Rule" (how a capacity decrease behaves against a busy
  unit — ignore/preempt/wait) has no field on `Resource`. The closest candidate
  is `SchedulableItem::Rule` on `Schedule`, attached per schedule item instead
  of per resource; whether this achieves the same runtime effect is unconfirmed
  and is deferred to Phase B (component `_onDispatchEvent()` review).
- **Needs human decision**: whether a StateSet-equivalent (arbitrary named
  resource states) is in scope, or whether the fixed `ResourceState` enum is the
  intended permanent design — `needs-human-decision`.

### 5.5 Variable

- **Arena**: "Variable module", *Getting Started with Arena*, "The Basic Process
  Panel", pp. 48-49. Dimension/initial values (as Attribute), plus Clear Option
  (Statistics/System/None) and external-file linkage (File Name, Recordset, File
  Read Time).
- **GenESyS**: `Variable` — `source/plugins/data/Logic/Variable.{h,cpp}`,
  implemented as a **subclass of `Attribute`**, reusing its dimension/initial-value
  infrastructure and adding an independent runtime `SparseValueStore` (`_values`).
- **Level**: `partial`.
- **Behavior**: `Variable::_initBetweenReplications()` (`Variable.cpp:114-116`)
  resets `_values` from the inherited initial-value store at the start of every
  replication **[confirmed]** — this is the only reset trigger implemented.
- **Divergence**: no Clear Option (Statistics/System/None) and no external-file
  linkage (File Name/Recordset/File Read Time) fields exist **[confirmed:
  absence — no matching members/accessors in `Variable.h`]**.
- **Design note**: modeling `Variable` as an `Attribute` subclass is an
  architectural choice worth flagging for later review — Arena treats
  Attribute (per-entity) and Variable (global) as unrelated concepts; GenESyS
  unifies their storage mechanism. Not recorded as a defect, but relevant to
  `HUM-ARCH-001`-style future consolidation discussions if raised.
- **Needs human decision**: no (informational note only).

### 5.6 Schedule

- **Arena**: "Schedule module", *Getting Started with Arena*, "The Basic Process
  Panel", pp. 50-51. Type (Capacity/Arrival/Other), Time Units, Scale Factor,
  Durations (Value/Duration repeat group, optional infinite repeat via a blank
  last duration).
- **GenESyS**: `Schedule` + `SchedulableItem` —
  `source/plugins/data/DiscreteProcessing/Schedule.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `_schedulableItems` (`List<SchedulableItem*>`), each holding
  `expression` (parsed, not a static value — a deliberate generalization of
  Arena's static Value column), `duration`, and `rule` (IGNORE/PREEMPT/WAIT)
  **[confirmed]**; `_repeatAfterLast` → `isRepeatAfterLast()/
  setRepeatAfterLast()` is the closest correspondence to Arena's "blank last
  duration means infinite repeat" behavior **[confirmed field exists; exact
  repeat semantics not yet traced through `.cpp`]**.
- **Divergence**: no explicit Type (Capacity/Arrival/Other) field — a single
  `Schedule` class appears to serve all three Arena schedule types uniformly
  **[strong indication]**; no Scale Factor field for Arrival/Other schedules
  **[confirmed: absence]**; no explicit Time Units field on `Schedule` itself
  **[hypothesis — may be resolved through the expression's own units or a
  model-wide base time unit; not yet traced]**.
- **Needs human decision**: no (missing Scale Factor is a minor feature gap, not
  an architectural fork).

### 5.7 Set (and Advanced Set)

- **Arena**: "Set module", *Getting Started with Arena*, "The Basic Process
  Panel", p. 51, and "Advanced Set module", "The Advanced Process Panel", p. 69.
  Both describe the same closed enumeration of set kinds: Resource, Counter,
  Tally, Entity (type), Entity Picture, Queue, Storage, Other.
- **GenESyS**: `Set` — `source/plugins/data/Logic/Set.{h,cpp}`.
- **Level**: `different` (broader than Arena, not narrower).
- **Parameters**: `setSetOfType()/getSetOfType()` records the concrete
  `ModelDataDefinition` subclass name of the first inserted member
  **[confirmed]**; `canChangeSetOfType()` — a `Set` is polymorphic only before
  its first member is inserted **[confirmed]**; `setAllowedElementTypes()/
  addAllowedElementType()/getAllowedElementTypes()` let a component owner (e.g.
  `SeizableItem`, `QueueableItem`) restrict a `Set` instance to one or more
  accepted subclasses **[confirmed]**.
- **Divergence**: GenESyS's `Set` is not a fixed enumeration of 8 kinds — it can
  hold any registered `ModelDataDefinition` plugin type, subject to the owner's
  `allowedElementTypes` contract. There is no dedicated "picture" data
  definition, so Arena's Entity Picture sets are `not-applicable`.
- **Needs human decision**: no.

### 5.8 Expression → Formula

- **Arena**: "Expression module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 70. Named expressions (optionally 1D/2D arrays) combining
  numbers, distributions, operators, attributes and variables; optional
  external-file linkage; declared Data Type (Real/String/Native).
- **GenESyS**: `Formula` — `source/plugins/data/Logic/Formula.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `setExpression()/getExpression()` with an optional textual
  index is the closest correspondence to Arena's Rows/Columns indexing
  **[confirmed]**; `getValue()` evaluates the stored expression through the
  parser **[confirmed]**.
- **Divergence**: no declared Data Type and no external-file linkage (File
  Name/Recordset) **[confirmed: absence]**.
- **Needs human decision**: no.

### 5.9 Failure

- **Arena**: "Failure module", *Getting Started with Arena*, "The Advanced
  Process Panel", pp. 71-72. Count-based or time-based failure; Count, Up
  Time(+Units), Down Time(+Units); optional "Uptime in this State only" state
  scoping; Failure Rule used per resource/failure pair.
- **GenESyS**: `Failure` — `source/plugins/data/DiscreteProcessing/Failure.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `FailureType` (`COUNT/TIME`) **[confirmed]**;
  `_countExpression`, `_upTimeExpression/_upTimeTimeUnit`,
  `_downTimeExpression/_downTimeTimeUnit` (parsed expressions rather than
  static numbers — a deliberate generalization) **[confirmed]**; `FailureRule`
  (`IGNORE/PREEMPT/WAIT`) **[confirmed]**.
- **Divergence**: no "Uptime in this State only" field — a time-based failure's
  up-time clock cannot be scoped to a single `ResourceState`; it always runs
  against total simulated time **[confirmed: absence]**.
- **Needs human decision**: no.

### 5.10 File

- **Arena**: "File module", *Getting Started with Arena*, "The Advanced Process
  Panel", pp. 72-73. Backs ReadWrite/Variable/Expression with rich structured
  data-source support: typed file access, ADO connection strings, fixed/free/
  Fortran structure, recordsets, SQL command text, named Excel ranges, Access
  table names.
- **GenESyS**: `File` — `source/plugins/data/InputOutput/File.{h,cpp}`.
- **Level**: `partial` (substantial gap, not a defect — a much smaller feature
  surface by design so far).
- **Parameters**: `AccessMode` enum (`Read/Write/Append/ReadWrite`) →
  `getAccessMode()/setAccessMode()`; `_systemFilename` →
  `getSystemFilename()/setSystemFilename()` **[confirmed]**.
- **Divergence**: none of Arena's structured recordset/ADO/spreadsheet/database
  concepts exist — GenESyS's `File` only models a plain OS file name and an
  access mode **[confirmed: absence]**.
- **Needs human decision**: whether richer structured file I/O is ever in scope
  — `needs-human-decision` only if/when a concrete use case requires it; not
  currently blocking.

### 5.11 Storage

- **Arena**: "Storage module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 78. Near-empty placeholder (only a Name); storages are
  auto-created by Store/Unstore and tracked purely through animation.
- **GenESyS**: `Storage` — `source/plugins/data/MaterialHandling/Storage.{h,cpp}`.
- **Level**: `different` (GenESyS extends the concept).
- **Parameters**: `_totalArea`, `_capacity`, `_unitsPerArea` →
  `getTotalArea()/setTotalArea()`, `getCapacity()/setCapacity()`,
  `getUnitsPerArea()/setUnitsPerArea()` **[confirmed]**.
- **Divergence**: GenESyS's `Storage` carries a real declared
  area/capacity/density contract that Arena's Storage module does not model at
  all (Arena tracks storage occupancy through 2D/3D animation counters, not a
  declared numeric capacity).
- **Needs human decision**: no.

### 5.12 Sequence

- **Arena**: "Sequence module", *Getting Started with Arena*, "The Advanced
  Transfer Panel", pp. 100-101. Ordered visitation list of stations (jobsteps),
  each with an optional Step Name and an optional non-sequential "Next Step"
  jump target, plus per-step attribute/variable/picture assignments. Backed by
  special-purpose `Entity.Sequence`/`Entity.Jobstep`/`Entity.PlannedStation`
  attributes at runtime.
- **GenESyS**: `Sequence` + `SequenceStep` —
  `source/plugins/data/MaterialHandling/Sequence.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `_steps` (`List<SequenceStep*>`); each `SequenceStep` targets
  either a `Station*` or a `Label*` (a GenESyS-specific extension beyond pure
  station-to-station routing) plus a list of `Assignment*` **[confirmed]**.
- **Divergence**: steps are stored as a plain ordered list, so Arena's named
  "Step Name"/"Next Step" out-of-order jump target has no obvious equivalent
  **[hypothesis — not yet traced through `Sequence.cpp`/the routing
  components; deferred to Phase B]**.
- **Needs human decision**: no (pending Phase B confirmation first).

### 5.13 Distance

- **Arena**: "Distance module", *Getting Started with Arena*, "The Advanced
  Transfer Panel", p. 106. Defines named station-pair distances for
  free-path transporters.
- **GenESyS**: `Distance` —
  `source/plugins/data/MaterialHandling/Distance.{h,cpp}`.
- **Level**: `partial`.
- **Parameters/behavior**: a `Distance` stores `DistanceEntry` rows with
  `fromStationName`, `toStationName`, `length`, and `bidirectional`.
  `getDistanceBetween()` resolves direct lookups by station-name pair and
  mirrors the value when `bidirectional` is set. Missing pairs return `-1.0`.
  Persistence round-trip, `_check()`, and plugin-factory registration are now
  covered by focused tests **[confirmed]**.
- **Divergence**: no transporter data definition consumes it yet, and no
  composed/multi-hop path search exists. The current contract is just a
  validated direct lookup table.
- **Implementation note (2026-08-30)**: the class was already present, but the
  dummy plugin connector used by unit tests had not been updated to discover
  `distance.so`; this batch reconciled registration and strengthened `_check()`
  to reject null, empty and negative entries.
- **Needs human decision**: no.

### 5.14 Segment

- **Arena**: "Segment module", *Getting Started with Arena*, "The Advanced
  Transfer Panel", p. 103. Defines an ordered conveyor path as station-to-next-
  station steps with associated lengths.
- **GenESyS**: `Segment` —
  `source/plugins/data/MaterialHandling/Segment.{h,cpp}`.
- **Level**: `partial`.
- **Parameters/behavior**: a `Segment` stores an ordered list of
  `SegmentStep { stationName, lengthToNext }`. `getDistanceBetween(a, b)`
  accumulates forward distance only when both stations appear in order on the
  same segment; reverse or disconnected requests return `-1.0`. Persistence
  round-trip, `_check()`, and plugin-factory registration are now covered by
  focused tests **[confirmed]**.
- **Divergence**: no `Conveyor` data definition exists yet, and the current
  semantics are strictly linear/forward, with no branching or control logic.
- **Implementation note (2026-08-30)**: the class was already present, but the
  dummy plugin connector used by unit tests had not been updated to discover
  `segment.so`; this batch reconciled registration and strengthened `_check()`
  to reject null, empty and negative steps.
- **Needs human decision**: no.

## 6. Phase B — Components: analyzed entries

### 6.1 Create

- **Arena**: "Create module", *Getting Started with Arena*, "The Basic Process
  Panel", pp. 31-32. Starting point for entities: Entity Type, Type
  (Random/Schedule/Constant/Expression), Value, Schedule Name, Expression,
  Units, Entities per Arrival, Max Arrivals, First Creation.
- **GenESyS**: `Create` — `source/plugins/components/Logic/Create.{h,cpp}`,
  built on the kernel base class `SourceModelComponent` —
  `source/kernel/simulator/SourceModelComponent.{h,cpp}`.
- **Level**: `partial` (an architectural generalization, not a narrow gap).
- **Parameters**: `SourceModelComponent` generalizes Entity Type
  (`_entityType`), First Creation (`_firstCreation`), Entities per Arrival
  (`_entitiesPerCreation`), Max Arrivals (`_maxCreationsExpression`, a parsed
  expression rather than a static integer) and Time Between Creations
  (`_timeBetweenCreationsExpression` + `_timeBetweenCreationsTimeUnit`)
  **[confirmed]**. `Create` adds two additional, mutually exclusive
  time-between-creations sources on top of the base expression: a `Schedule*`
  and a `Formula*` **[confirmed]**; `Create::_check()` enforces that exactly
  one of {expression, schedule, formula} is active (`Create.cpp:157-165`)
  **[confirmed]**.
- **Divergence**: Arena's explicit `Type` enum (Random/Schedule/Constant/
  Expression) does not exist as a stored field. GenESyS collapses
  Random/Constant/Expression into one parsed expression string (default
  `"EXPO(1.0)"`, itself equivalent to Arena's default Random behavior) and
  keeps Schedule as a separate, mutually exclusive alternative — a
  deliberate, behavior-preserving generalization, not a gap. GenESyS adds a
  `Formula*` alternative with no direct Arena counterpart.
- **Behavior** `_onDispatchEvent()` **[confirmed]**: sets
  `Entity.ArrivalTime`/`Entity.Type` on the dispatched entity, schedules the
  next arrival(s) via the active time source, respects
  `_entitiesPerCreation` and the max-creations expression, and forwards the
  entity to the front connection. Statistics: `_numberOut` (`Counter`) counts
  entities created, gated by `isReportStatistics()`.
- **Code hygiene note (not a behavior bug)**: `Create.h` declares
  `testePropertyCreateDouble()`/`setTestePropertyCreateDouble()`, an
  apparently leftover debug/test property with no Arena correspondence and no
  use found in `Create.cpp`'s reviewed logic. Left untouched — removing
  unrelated code is out of scope for this audit; flagged here for a future
  cleanup pass.
- **Needs human decision**: no.

### 6.2 Dispose

- **Arena**: "Dispose module", *Getting Started with Arena*, "The Basic
  Process Panel", p. 32. Ending point for entities; optional "Record Entity
  Statistics" covering value-added/non-value-added/wait/transfer/other time
  and cost, plus total time and total cost.
- **GenESyS**: `Dispose` — `source/plugins/components/Logic/Dispose.{h,cpp}`,
  built on `SinkModelComponent`.
- **Level**: `partial`.
- **Parameters**: `isReportStatistics()` (inherited) is the closest
  correspondence to "Record Entity Statistics" **[confirmed]**.
- **Behavior** `_onDispatchEvent()` **[confirmed, `Dispose.cpp:38-51`]**: when
  reporting is enabled, increments a `Counter` (`_numberOut`) and, only if the
  entity's `EntityType` itself has `isReportStatistics()` enabled, records one
  observation into a `TotalTimeInSystem` `StatisticsCollector` attached to
  that `EntityType` (computed as current simulated time minus
  `Entity.ArrivalTime`). The entity is then removed from the model.
- **Divergence (revised in batch 2, see §6.3)**: `Dispose` only reports a
  single collapsed `TotalTimeInSystem` statistic. As §6.3 details, `Delay`
  *does* accumulate per-category time on the entity itself
  (`Entity.Total<Category>Time`, e.g. `Entity.TotalWaitTime`) — but `Dispose`
  does not read or report those attributes, so an entity's category-time
  breakdown is computed and available on the entity, yet never surfaced in
  the final disposal statistics. Cost allocation (VA/NVA/Waiting/Other cost
  fields on `EntityType`, §5.2) is not tapped by either `Delay` or `Dispose`.
- **Needs human decision**: whether `Dispose` should report the
  per-category time totals that `Delay` already accumulates on the entity,
  and whether cost allocation should be implemented at all — see §6.3.

### 6.3 Cross-cutting finding (revised in batch 2): Arena's VA/NVA/Wait/Transfer/Other **time** allocation is implemented; **cost** allocation is not confirmed

**[confirmed, revises the batch-1 `strong indication`]**: batch 1 recorded
this as an open question because `Create`/`Dispose`/`Assign` showed no
accumulation logic. Reading `Delay` (Advanced Process, audited ahead of
schedule because `Process` embeds it — see §6.5) resolves it:

- `Util::AllocationType` (`source/kernel/util/Util.h:92`) is exactly Arena's
  five categories: `ValueAdded/NonValueAdded/Transfer/Wait/Others`
  **[confirmed]**;
- `Delay::_onDispatchEvent()` (`Delay.cpp:101-117`) **does** allocate the
  evaluated delay **time** to the configured category: it adds the delay
  time to a `<EntityType>.<Category>Time` `StatisticsCollector` (gated by the
  EntityType's own `isReportStatistics()`), and separately accumulates a
  running `Entity.Total<Category>Time` attribute on the entity itself
  **[confirmed]**;
- `Seize` also carries its own, independent `_allocationType` (default
  `Others`) and writes it into a per-request attribute
  (`Seize.cpp:227`, marked `//@TODO: Check it!` in the source) **[confirmed
  field/call exists; exact semantics of that attribute not yet traced]**.

So **time** allocation by category is real and functioning, at least through
`Delay`. What remains **not found** anywhere read so far (`EntityType`,
`Create`, `Dispose`, `Assign`, `Process`, `Delay`): any logic that converts
accumulated category **time** into category **cost** using `EntityType`'s
initial VA/NVA/Waiting/Other cost fields (§5.2) or `Resource`'s busy/idle
per-time-unit costs (§5.4) — i.e. Arena's "Associated costs are calculated
and allocated as well" (quoted almost verbatim in `Delay.cpp`'s own
`GetPluginInformation()` help text) has no confirmed GenESyS counterpart.

- **Level**: `partial` (time: `equivalent`-leaning; cost: `missing`, not yet
  exhaustively searched outside the components read so far).
- **Needs human decision**: whether per-category **cost** accumulation
  (turning `Entity.Total<Category>Time` plus a cost rate into a
  `Entity.Total<Category>Cost`) is an intended GenESyS feature, or whether
  cost reporting is deliberately out of scope beyond `Resource`'s own
  busy/idle/per-use cost fields.

### 6.4 Assign

- **Arena**: "Assign module", *Getting Started with Arena*, "The Basic
  Process Panel", pp. 40-41. One or more assignments per module; Type:
  Attribute, Variable (+ Variable Array 1D/2D with Row/Column), Entity Type,
  Entity Picture, or Other (system variables such as resource capacity or
  simulation end time); New Value expression (not applicable for Entity
  Type/Entity Picture).
- **GenESyS**: `Assign` — `source/plugins/components/Logic/Assign.{h,cpp}`,
  holding a `List<Assignment*>`; each `Assignment` is defined in
  `source/plugins/data/Logic/AssignmentItem.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `Assignment` stores a free-form `destination` string, an
  `expression` string, and a single boolean `_isAttributeNotVariable`
  **[confirmed]**. `Assign::_destinationBaseName()`/`_destinationIndex()`
  parse an optional `name[index]` syntax out of `destination`, which is the
  closest correspondence to Arena's separate Row/Column fields for Variable
  Array assignments **[confirmed]**.
- **Behavor** `_onDispatchEvent()` **[confirmed, `Assign.cpp:125-159`]**:
  for each `Assignment`, evaluates `expression` through the parser and writes
  it either to an `Attribute` (creating one on the model if it does not exist
  yet) or to a `Variable` (same auto-creation behavior), depending on
  `isAttributeNotVariable()`.
- **Divergence**: Arena's Entity Type, Entity Picture and Other
  (system-variable) assignment kinds have no dedicated `Assignment` field or
  branch in `Assign::_onDispatchEvent()` — only Attribute and Variable
  destinations (optionally indexed) are supported **[confirmed: only two
  branches exist in `_onDispatchEvent()`]**. Reassigning an entity's
  `EntityType` at runtime, or assigning a "system variable" such as a
  Resource's capacity, is not reachable through this component as currently
  implemented.
- **Needs human decision**: whether Entity Type reassignment and Other
  (system-variable) assignment targets are in scope for `Assign`, or whether
  they are deliberately out of scope (e.g. left to dedicated components) —
  `needs-human-decision`.

### 6.5 Process

- **Arena**: "Process module", *Getting Started with Arena*, "The Basic
  Process Panel", pp. 33-35, plus "Process module — Resource dialog box",
  p. 35. Action: Delay / Seize Delay Release / Seize Delay / Delay Release;
  Priority; one or more Resources (repeat group: Type, Resource/Set Name,
  Quantity, Save Attribute); Delay Type/Units; Allocation (governs the
  module's own processing-time category, independent of any implicit queue
  wait-time category); Report Statistics.
- **GenESyS**: `Process` — `source/plugins/components/DiscreteProcessing/Process.{h,cpp}`,
  a composite `ModelComponent` that always owns and internally chains three
  mandatory sub-components: `Seize` → `Delay` → `Release`
  (`Process::_ensureInternalComponents()`, `Process.cpp:178-210`)
  **[confirmed]**.
- **Level**: `partial`.
- **Parameters**: `Priority`/`PriorityExpression`, `QueueableItem`, and the
  `SeizeRequests` list are all forwarded to the internal `_seize`
  **[confirmed, `Process.cpp:86-120`]**; the delay expression/time unit are
  forwarded to the internal `_delay` **[confirmed]**; `_check()` requires
  matching Seize/Release request cardinality and a consistent internal
  Seize→Delay→Release chain **[confirmed, `Process.cpp:236-252,321-348`]**.
- **Divergence 1**: there is no Action selector — a `Process` instance always
  builds all three sub-components (Seize, Delay, Release) unconditionally.
  Arena's "Delay" and "Delay Release" Action variants (no resource seizure at
  all) have no direct `Process`-level equivalent; the closest workaround is
  using a standalone `Delay` component instead of `Process`.
- **Divergence 2 (confirmed bug candidate, `needs-human-decision`)**:
  `Process::setAllocationType()`/`getAllocationType()` forward to the
  internal `_seize`'s allocation (`Process.cpp:102-108`), **not** to the
  internal `_delay`'s allocation. No code path was found that lets `Process`
  change `_delay`'s `_allocation` field, which therefore stays permanently at
  `Delay`'s own default, `Util::AllocationType::Wait`
  (`Delay.h:82`) **[confirmed: no call to `_delay->setAllocation(...)`
  anywhere in the reviewed sources]**. Net effect: a `Process` component's
  own processing (Delay) time is always categorized and reported as "Wait"
  time, never reachable as "Value Added" (Arena's typical default for
  processing time) or any other category through `Process`'s public API —
  while the exposed `AllocationType` control instead governs `Seize`'s
  allocation, whose role is unclear (see §6.3's open note on `Seize.cpp:227`).
  This looks like a plausible wiring oversight (the control may have been
  intended to reach `_delay`, not `_seize`) rather than a deliberate design
  choice, but it is not being "fixed" here without maintainer confirmation,
  per audit governance.
- **Needs human decision**: is `Process`'s exposed `AllocationType` supposed
  to govern the internal `Delay`'s category (matching Arena's module-level
  "Allocation" field) rather than the internal `Seize`'s? If so this is a
  real, fixable bug; if the current wiring is intentional, the divergence
  from Arena should be documented as `different` instead.

### 6.6 Decide

- **Arena**: "Decide module", *Getting Started with Arena*, "The Basic
  Process Panel", pp. 36-37. Type: 2-way/N-way, by Condition or by Chance;
  structured condition fields (Named/Is/Value with Row/Column for Variable
  Array conditions) or Percentages for Chance branching; one exit per
  condition/percentage plus a single "else"/"false" exit.
- **GenESyS**: `Decide` — `source/plugins/components/Decisions/Decide.{h,cpp}`.
- **Level**: `different` (a clean generalization, not a narrow gap).
- **Parameters**: `_conditions` is a plain `List<std::string>` of parser
  boolean expressions, one per output port in order, plus an implicit final
  "else" port **[confirmed]**.
- **Behavior** `_onDispatchEvent()` **[confirmed, `Decide.cpp:63-83`]**:
  evaluates each condition expression in list order through the parser;
  routes the entity to the first output port whose condition is truthy, or
  to the trailing else port if none match. Each port has its own `Counter`
  when reporting is enabled.
- **Divergence**: Arena's structured condition builder (Named/Is/Value,
  Row/Column, and a dedicated Percentages field for Chance-type branching)
  collapses into one general parser-expression mechanism. There is no
  dedicated "by Chance" percentage input — chance-based branching must be
  expressed as a boolean expression by the modeler (e.g. comparing a random
  draw against a threshold) rather than through a structured percentage
  field. This is a deliberate simplification/generalization, not a missing
  capability, but it does shift authoring ergonomics onto the modeler.
- **Needs human decision**: no.

### 6.7 Batch

- **Arena**: "Batch module", *Getting Started with Arena*, "The Basic
  Process Panel", p. 38. Type: Temporary/Permanent; Batch Size; Rule:
  Any/By Attribute (+ Attribute Name); Save Criterion (representative
  entity's attribute values: First/Last/Sum — GenESyS naming below matches
  closely); Representative Entity (type).
- **GenESyS**: `Batch` — `source/plugins/components/Grouping/Batch.{h,cpp}`.
- **Level**: `equivalent` (header-level; full `_onDispatchEvent()` behavior
  spot-checked, not exhaustively traced).
- **Parameters**: `BatchType` (`Temporary/Permanent`), `Rule`
  (`Any/ByAttribute`) + `_attributeName`, `GroupedAttribs`
  (`FirstEntity/LastEntity/SumAttributes` — matching Arena's Save Criterion
  First/Last/Sum), `_batchSize` (a parsed expression, generalizing Arena's
  static number), `_groupedEntityType` (Representative Entity) **[confirmed,
  `Batch.h:56-127`; enum-to-string mapping confirmed in `Batch.cpp:48-53`]**.
- **Divergence**: none identified at the header/API level in this pass.
- **Needs human decision**: no.

### 6.8 Separate

- **Arena**: "Separate module", *Getting Started with Arena*, "The Basic
  Process Panel", p. 39. Type: Duplicate Original (# of Duplicates, Percent
  Cost to Duplicates split between copies) or Split Existing Batch (Member
  Attributes propagation rule, Attribute Name for the "Take Specific
  Representative Values" option).
- **GenESyS**: `Separate` — `source/plugins/components/Grouping/Separate.{h,cpp}`.
- **Level**: `partial` (one of Arena's two Types is fully unimplemented, not
  a stub — the implemented half is real).
- **Parameters**: `Separate.h` declares **no fields at all**; behavior is
  entirely hardcoded in `_onDispatchEvent()` **[confirmed]**.
- **Behavior** `_onDispatchEvent()` **[confirmed, `Separate.cpp:48-79`]**:
  implements only Arena's "Split Existing Batch" semantics — reads the
  `Entity.Group` marker attribute, looks up the matching `EntityGroup`
  (created by `Batch`), releases every original member entity with its
  group marker cleared, forwards each to the front connection, and removes
  the temporary representative entity. Members are **not** modified with any
  representative-entity attribute values on the way out (no "Member
  Attributes" propagation logic was found).
- **Divergence**: Arena's "Duplicate Original" Type (clone the incoming
  entity N times, optionally splitting VA/NVA/wait/transfer/other cost and
  time between the copies via "Percent Cost to Duplicates") has **no**
  implementation here — there is no entity-cloning code path in
  `Separate::_onDispatchEvent()` at all **[confirmed: absence]**. A modeler
  who needs Arena's duplicate-and-fan-out behavior currently has no
  `Separate`-based way to achieve it.
- **Resolved (maintainer, 2026-08-30)**: duplication is intentionally a
  separate component, `Clone` — see §6.19 for the confirmed audit of
  `Clone.cpp`. `Separate` is not expected to grow "Duplicate Original"
  semantics.
- **Needs human decision**: no (resolved).

### 6.9 Record

- **Arena**: "Record module", *Getting Started with Arena*, "The Basic
  Process Panel", pp. 41-42. Type: Count, Entity Statistics, Time Interval,
  Time Between, or Expression; optional Tally/Counter Set with Set Index.
- **GenESyS**: `Record` — `source/plugins/components/InputOutput/Record.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: no `Type` field exists at all — `Record` only ever
  evaluates one parser `_expression` into a `StatisticsCollector`
  (`_cstatExpression`) **[confirmed]**. GenESyS extends this single mode
  beyond Arena with dataset-export metadata with no Arena counterpart:
  `_datasetName`, `_randomVariableName`, `_variableType`,
  `_description`, and direct-to-file recording (`_filename`,
  `_timeDependent`) for external tooling (e.g. the Data Analyser) **[confirmed,
  `Record.h:68-148`]**.
- **Behavior** `_onDispatchEvent()` **[confirmed, `Record.cpp:210-233`]**:
  evaluates `_expression`, feeds the value to `_cstatExpression` when
  present, and optionally appends it (with or without a simulated-time
  column) to `_filename`.
- **Divergence**: Arena's Count, Entity Statistics, Time Interval and Time
  Between Types have no equivalent — only "Expression" is implemented.
  Tally/Counter Sets ("Record into Set") are entirely absent **[confirmed:
  absence]**. This directly bears on the maintainer's Phase A clarification
  that "Statistic" is covered by `Statistics`/`Collector`/
  `StatisticsCollector`/`Counter`: those kernel classes exist and are used
  elsewhere (`Counter` inside `Create`/`Dispose`/`Decide`, for example), but
  `Record` itself — the Arena module most directly responsible for ad hoc
  observational statistics — only exposes the Expression path, so Count,
  Entity Statistics, Time Interval and Time Between remain gaps at the
  `Record` component level specifically, independent of the underlying
  kernel classes being available.
- **Needs human decision**: whether the other four Record Types are in
  scope for this component, or whether users are expected to reach
  `Counter`/`StatisticsCollector` directly for those cases.

### 6.10 Seize and Release

- **Arena**: "Seize module" and "Release module", *Getting Started with
  Arena*, "The Advanced Process Panel", pp. 62-63 and p. 60. Seize allocates
  one or more resources/set members to an entity (queueing if unavailable);
  Release gives them back.
- **GenESyS**: `Seize`/`Release` —
  `source/plugins/components/DiscreteProcessing/{Seize,Release}.{h,cpp}`.
- **Level**: `equivalent`, and this closes the §6.3 cost-time-allocation
  question for resource-holding time.
- **Parameters**: `SeizableItem` requests (Resource/Set, quantity,
  selection rule, save attribute), `QueueableItem` (Queue/Set/Attribute/
  Expression), `_priority`/`_priorityExpression`, `_allocationType` — all
  confirmed matching Arena's Type/Resource Name/Set Name/Quantity/Selection
  Rule/Save Attribute/Priority/Allocation/Queue Type fields **[confirmed,
  `Seize.h`]**. `Release`'s `SeizableItem` release requests plus
  `RemoveFromType`-independent quantity/rule match Arena's Release fields
  **[confirmed, `Release.h`]**.
- **Behavior (confirms and extends §6.3)** **[confirmed,
  `Seize.cpp:192-238`, `Release.cpp:221-237`]**: `Seize` stamps
  `Entity.Allocation.<ResourceName>` with the configured
  `Util::AllocationType` when a resource is successfully seized; `Release`
  reads that same attribute back when a resource is released, and credits
  the resource's held duration (`resource->getLastTimeSeized()`) to both a
  `<EntityType>.<Category>Time` `StatisticsCollector` and a running
  `Entity.Total<Category>Time` attribute — the exact same two-sink pattern
  as `Delay` (§6.3). So category **time** allocation is confirmed working
  for both processing delay (`Delay`) and resource-holding time
  (`Seize`/`Release`).
- **Documentation-bug note (fixed in this batch)**: `Seize.cpp:227` carried
  a `//@TODO: Check it!` comment as if the attribute write were unverified,
  and `Release.cpp:230` carried a stale `//@TODO: Seize is not setting this
  attribute. Fiz it.` comment claiming the opposite of what `Seize.cpp`
  actually does. Both comments were misleading; removed as a zero-behavior
  documentation correction (not a functional change).
- **Needs human decision**: no (time allocation is confirmed correct); cost
  allocation remains the open item tracked in §6.3.

### 6.11 Hold → Wait

- **Arena**: "Hold module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 54. Type: Wait for Signal / Scan for Condition /
  Infinite Hold; Queue Type (Queue/Set/Internal/Attribute/Expression).
- **GenESyS**: `Wait` — `source/plugins/components/Synchronization/Wait.{h,cpp}`.
  Note the name change: this is Arena's **Hold**, not a separate concept.
- **Level**: `equivalent`.
- **Parameters**: `WaitType` (`WaitForSignal/InfiniteHold/ScanForCondition`)
  matches Arena's Type exactly; `_condition`, `_limitExpression`, attached
  `SignalData`, and a `Queue*` (todo-noted in the source as "should be a
  QueueableItem") all correspond to Arena's Wait for Value/Limit/Condition/
  Queue Type fields **[confirmed, `Wait.h`]**.
- **Divergence**: `_queue` is a plain `Queue*` rather than the richer
  `QueueableItem` (Queue/Set/Attribute/Expression) that `Seize`/`Process`
  use — the header itself flags this with a `@TODO`. Minor, self-acknowledged
  gap.
- **Needs human decision**: no.

### 6.12 Signal

- **Arena**: "Signal module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 65. Sends a signal value to release entities waiting at
  Hold modules, up to a Limit.
- **GenESyS**: `Signal` — `source/plugins/components/Synchronization/Signal.{h,cpp}`.
- **Level**: `equivalent`. `_signalData` (attached `SignalData`) and
  `_limitExpression` match Arena's Signal Value and Limit fields
  **[confirmed, `Signal.h`]**.
- **Needs human decision**: no.

### 6.13 Match

- **Arena**: "Match module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 56. Brings together one entity from each of up to five
  queues, optionally requiring matching attribute values.
- **GenESyS**: `Match` — `source/plugins/components/Synchronization/Match.{h,cpp}`.
- **Level**: `equivalent` (and broader than Arena's five-queue limit).
  `Rule` (`Any/ByAttribute`), `_attributeName`, `_matchSize`,
  `_numberOfQueues` and a `List<Queue*>` match Arena's Number to
  Match/Type/Attribute Name fields, with no hardcoded queue-count ceiling
  **[confirmed, `Match.h`]**.
- **Needs human decision**: no.

### 6.14 Pickup and Remove

- **Arena**: "Remove module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 61 (remove one entity at a given rank from a queue) and
  "Pickup module", p. 57 (remove a consecutive run of entities from a queue
  into the incoming entity's group).
- **GenESyS**: `Remove` —
  `source/plugins/components/Decisions/Remove.{h,cpp}` — and `PickUp`
  (`source/plugins/components/Decisions/PickUp.{h,cpp}`), which **inherits
  from `Remove`** and reuses its rank/queue plumbing.
- **Level**: `different` (broader than Arena, not narrower).
  `RemoveFromType` (`QUEUE/ENTITYGROUP`) lets `Remove` pull from either a
  Queue or an EntityGroup, and `_removeStartRank`/`_removeEndRank` support a
  rank *range* rather than Arena's single-rank-only Remove
  **[confirmed, `Remove.h`]**. (Two disabled historical unit tests,
  `SimulatorRuntimeTest.RemoveEqualStartAndEndRankRemovesExactlyOneAndRoutesCorrectly`
  and `...RemoveRangeRemovesOnlyEntitiesInsideConfiguredInterval`, confirm
  the range behavior was deliberately built and tested.)
- **Needs human decision**: no.

### 6.15 Search

- **Arena**: "Search module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 64. Type: search a Queue, search an EntityGroup, or
  search an arbitrary index range via an Expression (sets the global system
  variable `J`).
- **GenESyS**: `Search` — `source/plugins/components/Decisions/Search.{h,cpp}`.
- **Level**: `partial`.
- **Parameters**: `SearchInType` (`QUEUE/ENTITYGROUP`), `_startRank`/
  `_endRank`, `_searchCondition`, `_saveFounRankAttribute` match Arena's
  Type/Starting-Ending Value/Search Condition fields for the Queue and
  EntityGroup cases **[confirmed, `Search.h`]**.
- **Divergence**: `SearchInType` has only two members
  (`num_elements = 2`) — Arena's third Type, a free-standing Expression
  search over an index range with no queue/group at all, has no
  corresponding enumerator **[confirmed: absence]**.
- **Needs human decision**: whether expression-only search is in scope.

### 6.16 ReadWrite → Write (write-only)

- **Arena**: "ReadWrite module", *Getting Started with Arena*, "The Advanced
  Process Panel", p. 58. Reads or writes values to/from Screen, File,
  Attribute, Variable, or other Arena data sources (File module-backed).
- **GenESyS**: `Write` — `source/plugins/components/InputOutput/Write.{h,cpp}`.
  There is no corresponding "Read" component anywhere in
  `source/plugins/components/InputOutput/` (only `Record` and `Write`
  exist).
- **Level**: `partial`.
- **Parameters**: `WriteToType` (`SCREEN/FILE`) and a list of text elements
  to write **[confirmed, `Write.h`]** — this covers only the "write text to
  Screen or File" slice of Arena's ReadWrite module.
- **Divergence**: no read direction at all, and no Attribute/Variable
  data-source integration matching Arena's `File` data definition (§5.10) —
  `Write` only ever emits literal/expression text, not structured
  read-then-assign round-trips.
- **Needs human decision**: whether a read-capable counterpart is in scope.

### 6.17 Dropoff, Store, Unstore

- **Arena**: "Dropoff module" (*Getting Started with Arena*, "The Advanced
  Process Panel", p. 54 — release N entities from a group); "Store module"
  (p. 66) and "Unstore module" (p. 67) — add/remove entities in a Storage.
- **GenESyS**:
  - `DropOff` — `source/plugins/components/Decisions/DropOff.{h,cpp}`;
  - `Store` / `Unstore` —
    `source/plugins/components/MaterialHandling/{Store,Unstore}.{h,cpp}`;
  - `Storage` runtime state now exercised through
    `source/plugins/data/MaterialHandling/Storage.{h,cpp}`.
- **Level**: `partial`.
- **DropOff behavior (implemented in this batch, 2026-08-30)**:
  `quantityExpression` + `startingRankExpression` now remove a contiguous set
  of members from the representative entity's `EntityGroup`, starting from a
  1-based rank, send removed members to output port 1, and let the
  representative continue on output port 0. Removed members have
  `Entity.Group` cleared. Member-attribute propagation rules from Arena are
  still not implemented.
- **Store/Unstore behavior (implemented in this batch, 2026-08-30)**:
  both now bind to a `Storage` plus `quantityExpression`. `Store` increments a
  `Storage`'s runtime occupation if capacity permits; `Unstore` decrements it
  if sufficient units are present. Both support persistence and validation and
  then forward the entity.
- **Storage cross-reference**: `Storage` now has real runtime occupation
  tracking with reset between replications, so the Phase A data definition is
  no longer detached from runtime use.
- **Divergences**:
  - `DropOff` uses a two-output contract in GenESyS to keep the representative
    and the dropped members on explicit separate paths.
  - `DropOff` currently retains original member attributes only; Arena's richer
    member-attribute reassignment modes remain unsupported.
  - `Store`/`Unstore` currently act immediately; there is no queueing/waiting
    contract when capacity or inventory is insufficient.
- **Needs human decision**: no for this minimum contract.

### 6.18 Adjust Variable

- **Arena**: "Adjust Variable module", *Getting Started with Arena*, "The
  Advanced Process Panel", p. 67. A small utility to increment/decrement a
  variable by an expression inline in the flowchart.
- **GenESyS**: no component with this name or an equivalent single-purpose
  arithmetic-adjust component was found.
- **Level**: `not-applicable`-leaning `missing` — `Assign` (§6.4) can
  already express `MyVar + 1` style self-referential updates through its
  general expression mechanism, so the capability is reachable, just not as
  a dedicated named module.
- **Needs human decision**: no.

### 6.19 Clone confirmed as the substitute for Separate's "Duplicate Original" (per maintainer, 2026-08-30)

**[confirmed]**: `Clone` —
`source/plugins/components/DiscreteProcessing/Clone.{h,cpp}` — evaluates
`_numClonesExpression`, creates that many new entities with the incoming
entity's `EntityType` and a copy of every current `Attribute` value, sends
each clone out output port 1 and the original out port 0, and increments a
`Counter` when reporting is enabled (`Clone.cpp:81-101`). This is
functionally Arena's Separate "Duplicate Original" Type (§6.8), confirmed by
the maintainer as the intended GenESyS component for that behavior.

- **Level**: `equivalent` (as the intentional split: `Separate` = Split
  Existing Batch, `Clone` = Duplicate Original), modulo one gap.
- **Divergence**: Arena's "Percent Cost to Duplicates" (splitting
  accumulated VA/NVA/wait/transfer/other cost/time between the original and
  its clones) has no equivalent in `Clone` — consistent with the still-open
  cost-allocation gap tracked in §6.3.
- **Needs human decision**: no (naming/design split confirmed by maintainer);
  the cost-split sub-feature folds into the existing §6.3 cost question.

### 6.20 Advanced Transfer general: Enter, Leave, PickStation, Route, Station concept

- **Arena**: "Enter", "Leave", "PickStation" and "Route" modules, plus
  Station as the receiving flowchart concept, *Getting Started with Arena*,
  "The Advanced Transfer Panel", printed pp. 89-101.
- **GenESyS**:
  - `Enter` / `Leave` / `PickStation` / `Route` —
    `source/plugins/components/MaterialHandling/`;
  - `Station` data definition —
    `source/plugins/data/MaterialHandling/Station.{h,cpp}`.
- **Level**: `partial`.
- **Enter**: real receiving component, not a stub. It binds a `Station`,
  registers itself as that station's entry component, calls
  `station->enter(entity)` on dispatch, and forwards to its front connection.
  Persistence and `_check()` are implemented. The help text in
  `Enter::GetPluginInformation()` is still `//@TODO`, so the code is ahead of
  its public description.
- **Leave**: real component, not a stub. It validates a bound `Station`, calls
  `station->leave(entity)` on dispatch, and forwards the entity onward.
  It does not perform any conveyor/transporter request or release semantics by
  itself.
- **PickStation**: real component, not a stub. It evaluates one or more
  `PickableStationItem`s using the enabled criteria (expression, queue size,
  busy resources), stores the selected station id in the configured attribute,
  and forwards the entity. This is best classified as a partial equivalence to
  Arena's station-selection logic rather than a full movement module.
- **Route**: real component, not a stub. It supports destination types
  `Station`, `Sequence`, and `Label`; applies any `SequenceStep`
  assignments; computes route delay from `routeTimeExpression`; schedules the
  arrival event to the destination `Enter`/label receiver; and updates
  transfer-time statistics/attributes when an `EntityType` is present.
- **Station concept**: GenESyS models Station as a data definition plus
  `Enter`/`Leave`/`Route` interactions, not as a standalone flowchart module
  with Arena's UI shape. That is an architectural difference, not a missing
  concept.
- **Bugs fixed in this batch (2026-08-30)**:
  - `Route::_check()` incorrectly validated `_station` when destination type
    was `Label`; fixed to validate `_label`.
  - `Route` persistence ignored `_stationExpression`; save/load now preserve it.
- **Focused runtime evidence**: tests now cover label-only validation,
  station-expression persistence, station-expression routing to a resolved
  `Enter`, and delayed routing through a `Label` receiver.
- **Needs human decision**: no for the current contract. The broader
  transporter/conveyor semantics remain separate in §7.

### 6.21 Advanced Transfer conveyor/transporter components

- **Arena**: Conveyor flowchart modules `Access`, `Convey`, `Exit`, `Start`,
  `Stop`, and transporter flowchart modules `Activate`, `Allocate`, `Free`,
  `Halt`, `Move`, `Request`, `Transport`, *Getting Started with Arena*,
  "The Advanced Transfer Panel", printed pp. 79-99.
- **GenESyS status**:
  - `Access`, `Exit`, `Start`, `Stop` —
    `source/plugins/components/MaterialHandling/{Access,Exit,Start,Stop}.{h,cpp}`
    — now implement a minimal executable Conveyor contract over the new
    `Conveyor` data definition (§7): `Start` activates the conveyor and may
    update its velocity, `Access` allocates simplified conveyor capacity,
    `Exit` releases it, and `Stop` deactivates the conveyor.
  - `Move` —
    `source/plugins/components/MaterialHandling/Move.{h,cpp}` — now implements
    the minimum approved transporter runtime: reserve one `Transporter`,
    compute travel time from `Distance / speed`, dispatch the entity to the
    destination station's `Enter` component, and free the transporter on
    arrival through an internal event.
  - no `Convey` component exists as a separate class. For the current minimum
    GenESyS contract, conveyor movement is represented by the conveyor
    allocation controls plus the already-audited station transfer flow
    (`Route`/`Enter`/`Leave`) rather than by a distinct Arena-named
    `Convey` block.
  - no separate `Activate`, `Allocate`, `Free`, `Halt`, `Request`, or
    `Transport` component exists. For the current minimum GenESyS contract,
    those responsibilities are intentionally collapsed into the new minimal
    `Transporter` data definition plus the atomic `Move` component.
- **Level**: `partial`.
- **Focused runtime evidence (2026-08-30)**:
  - `Conveyor` data: validation, direct distance lookup through `Segment`,
    persistence round-trip, and reset-between-replications;
  - conveyor actions: `Start -> Access -> Exit -> Stop` integrated flow with
    velocity update, allocation, release and deactivation;
  - `Transporter` data: distance-driven travel-time calculation, persistence
    round-trip, and reset-between-replications;
  - `Move`: dispatch, delayed arrival, transporter release-on-arrival, and
    persistence round-trip.
- **Divergences**:
  - Conveyor allocation is intentionally simplified to concurrent capacity,
    not Arena's contiguous cell occupancy semantics.
  - Transporter movement is intentionally exposed as one atomic `Move`
    contract rather than Arena's separate request/transport/free family.
- **Needs human decision**: no for this minimum contract.

### 6.22 Flow Process classification

- **Arena**: `Tank`, `Sensor`, `Flow`, `Regulate`, `Regulator Set`,
  `Seize Regulator`, `Release Regulator`, plus related predefined variables,
  belong to Arena's Flow Process template for continuous/semi-continuous bulk
  material systems (*Simulation with Arena*, ch. 11; Arena Variables Guide
  flow/tank/sensor sections).
- **GenESyS**: no corresponding plugin family or data/component set was found
  in the current codebase.
- **Level**: `future-domain-feature`.
- **Reasoning**: this is not just a missing naming layer on top of the current
  discrete-event material-handling code. The Arena references place these
  modules in a separate continuous/flow subsystem centered on tanks,
  regulators, sensors and flow rates. Nothing equivalent was found in the
  current GenESyS scope, and implementing it here would be a new domain
  feature rather than a close-out of the current Arena compatibility front.
- **Needs human decision**: no for this phase; leave outside the current close-
  out scope unless explicitly activated later.

## 7. Advanced Transfer conveyor/transporter subsystem status

This section replaces the earlier obsolete claim that `Segment` and `Distance`
were entirely missing.

- **Present today**:
  - `Sequence` (§5.12);
  - `Distance` (§5.13): direct/bidirectional station-pair lookup table;
  - `Segment` (§5.14): ordered forward conveyor path segments;
  - `Conveyor`: minimal executable conveyor data definition over `Segment`,
    with velocity, active state, simplified capacity/allocation, persistence,
    validation and replication reset;
  - `Transporter`: minimal executable free-path transporter data definition
    over `Distance`, with current station, active/busy state, persistence,
    validation and replication reset;
  - general transfer components `Enter`, `Leave`, `PickStation`, `Route`
    (§6.20);
  - conveyor/transporter runtime components `Access`, `Exit`, `Start`, `Stop`
    and `Move` (§6.21).
- **Still missing on the data side**:
  - `Network`;
  - `NetworkLink`;
  - `ActivityArea`.
- **Still missing or intentionally collapsed on the component side**:
  - conveyor: no separate `Convey` component;
  - transporter: no separate `Activate`, `Allocate`, `Free`, `Halt`,
    `Request`, or `Transport` components beyond the minimum atomic `Move`
    abstraction.

- **Level (data side)**: `partial` overall: `Sequence`, `Distance`,
  `Segment`, `Conveyor` and `Transporter` now exist, while `Network`,
  `NetworkLink`, and `ActivityArea` remain `missing`.
- **Level (component side)**: `partial` overall: the general transfer
  components are real (§6.20), the minimum conveyor/transporter runtime is now
  executable (§6.21), but Arena's fuller named module family is intentionally
  collapsed or absent.
- **Decision (maintainer, 2026-08-29)**: `decision-recorded` — implementing
  Conveyor, Segment, Transporter, Distance, Network, NetworkLink and
  ActivityArea (and completing Access/Exit/Start/Stop, and the
  Activate/Allocate/Free/Halt/Move/Request/Transport transporter components)
  is an approved future direction for GenESyS. This is a substantial new
  feature area, not a bug fix; before autonomous implementation work starts it
  needs its own scoped plan and, per `GOVERNANCE.md`/`BACKLOG_AUTONOMOUS.md`
  process, a tracked backlog entry/issue — not yet created as of this
  revision. Recorded here so Phase B does not need to re-derive this
  decision.

## 8. Parser / Arena Variables Guide cross-reference

- **Pipeline reality (confirmed 2026-08-30)**:
  - `ParserChangesInformation` exists as a structural API
    (`source/kernel/simulator/ParserChangesInformation.{h,cpp}`);
  - `ParserManager` exposes `generateNewParser()` / `connectNewParser()` in
    its header, but the current runtime parser in use is the static
    Bison/Flex grammar under `source/parser/parserBisonFlex/`;
  - audited `_getParserChangesInformation()` overrides on `Queue`, `Set`,
    `Schedule`, `File` and `Storage` currently return empty placeholder
    objects, so the dynamic plugin-driven parser-extension path is not what
    currently provides expression support.
- **Implemented and registered now**:
  - general simulation/kernel: `TNOW`, `TFIN`, `MAXREP`, `NUMREP`, `IDENT`,
    `TAVG(CSTAT)`, `COUNT(COUNTER)`;
  - generic data references by literal name: `Attribute`, `Variable`,
    `Formula`, `StatisticsCollector`, `Counter`, simulation controls and
    simulation responses;
  - queue: `NQ`, `FIRSTINQ`, `SAQUE`, `AQUE`;
  - resource: `MR`, `NR`, `STATE`, `IRF`, `SETSUM`;
  - set: `NUMSET`.
- **Partial / tokenized but not actually implemented**:
  - `LASTINQ`: grammar production exists but has no runtime result body;
  - `RESSEIZES`: grammar production exists but remains an explicit TODO;
  - `RESUTIL`: token exists in the lexer, but no active grammar production was
    found in `pluginFunction`;
  - `ENTATRANK`: token exists, but no active grammar production was found;
  - `NUMGR`, `ATRGR`: tokens exist for `EntityGroup`, but no active grammar
    production was found.
- **Semantics available under a different shape**:
  - entity attributes/variables/formulas are available directly by the named
    `Attribute`/`Variable`/`Formula` objects rather than by a broad Arena
    catalog of predefined aliases;
  - station transfer state is available through `Entity.Station` and the
    `Station` runtime/data model, but the Arena Variables Guide functions such
    as `MSQ`, station-time rollups, and activity-area rollups are not parser
    functions today.
- **Missing for currently supported MaterialHandling concepts**:
  - no parser function was found for `Storage` occupation (`NSTO`);
  - no parser function was found for `Distance`, `Segment`, `Conveyor`, or the
    new minimal `Transporter`;
  - no parser function was found for station/sequence helper functions such as
    `MSQ` or station/activity-area aggregates.
- **Out of scope / intentionally unsupported here**:
  - Flow Process variables (tank, sensor, regulator, flow-rate family) remain
    `future-domain-feature` with the subsystem itself (§6.22);
  - guided-transporter/network variables remain `missing` together with
    `Network` / `NetworkLink`.
- **Phase-C conclusion**: completed for the current GenESyS-supported scope as
  a mapping/classification pass. The main result is that parser support is
  presently hardcoded and narrower than the Arena Variables Guide, with a
  mixture of implemented functions, placeholder tokens and unsupported
  families.

## 9. Phase status

- **Phase A (data definitions)**: closed for this audit's purposes. 16 classes
  analyzed in detail (§5.1–§5.14); Statistic resolved by maintainer
  clarification (2026-08-29) as already covered by the kernel-level
  `Statistics`/`Collector`/`StatisticsCollector`/`Counter` classes, attached to
  the Record component and to many other components/data definitions —
  a full class-by-class detail pass on that group is deferred (reminder
  requested by the maintainer for a later session; do not forget). The
  Advanced Transfer data side is now reconciled as `partial`: `Sequence`,
  `Distance`, `Segment`, `Conveyor`, and `Transporter` are implemented
  (§5.12–§5.14, §7), while `Network`/`NetworkLink`/`ActivityArea` remain
  future work (§7).
- **Phase B (components)**: batches 1-2 of N complete (2026-08-29). Batch 1:
  `Create`, `Dispose`, `Assign`. Batch 2: `Process`, `Decide`, `Batch`,
  `Separate`, `Record` — this closes out every Arena Basic Process flowchart
  module. `Delay` (Advanced Process) was also audited ahead of schedule
  because `Process` embeds it.
  All entries in §6.
  The batch-1 cross-cutting `needs-human-decision` about VA/NVA/Wait/
  Transfer/Other accounting (§6.3) is **resolved for time, still open for
  cost**: `Delay` confirmed to allocate category **time** correctly via
  `Util::AllocationType`, but no category **cost** accumulation was found
  anywhere audited so far, and `Dispose` does not surface the per-category
  time totals `Delay` already computes on the entity. A new likely-bug
  candidate was found in `Process::setAllocationType()` (routes to the
  internal `Seize`, not the internal `Delay`, leaving `Process`'s own
  processing time permanently categorized as "Wait" — §6.5) — flagged
  `needs-human-decision` rather than fixed, since intent is unconfirmed.
  `Separate` was found to implement only Arena's "Split Existing Batch" Type
  (real, working) with "Duplicate Original" entirely absent (§6.8). `Record`
  was found to implement only Arena's "Expression" Type, extended with
  dataset-export metadata beyond Arena, with Count/Entity Statistics/Time
  Interval/Time Between and Tally/Counter Sets all absent (§6.9) — this
  refines, but does not contradict, the maintainer's Phase A clarification
  that Statistics/Collector/StatisticsCollector/Counter already cover the
  Statistic *data module*.
  Batch 3 (2026-08-30) closed out the rest of Advanced Process in one pass
  (per maintainer request): `Seize`/`Release` (§6.10) confirmed the
  category-**time** allocation loop end to end (Seize stamps
  `Entity.Allocation.<Resource>`, Release reads it back and credits the
  held time) and fixed two mutually-contradicting stale `@TODO` comments in
  `Seize.cpp`/`Release.cpp` (doc-only, zero behavior change); `Hold`→`Wait`
  (§6.11), `Signal` (§6.12), `Match` (§6.13) confirmed `equivalent`;
  `Pickup`/`Remove` (§6.14) confirmed `different` (broader — rank ranges,
  EntityGroup source); `Search` (§6.15) confirmed `partial` (no
  expression-only search Type); `ReadWrite`→`Write` (§6.16) confirmed
  `partial` (write-only, no read direction); `Dropoff`/`Store`/`Unstore`
  (§6.17) are now implemented as a minimal runtime contract: `DropOff`
  releases grouped members by rank/quantity, and `Store`/`Unstore` exercise
  `Storage` occupation directly. `Adjust Variable` (§6.18) confirmed
  absent but reachable through `Assign`. Maintainer confirmed (2026-08-30)
  that `Clone` is the intentional substitute for Separate's missing
  "Duplicate Original" (§6.19), closing that Phase-B-batch-2 open question.
  Batch 4 (this continuation, 2026-08-30) reconciled Advanced Transfer's
  already-existing `Distance` and `Segment` data definitions (§5.13–§5.14),
  audited the real general-transfer components `Enter`/`Leave`/`PickStation`/
  `Route` plus the Station concept (§6.20), implemented the minimum approved
  `Conveyor`/`Transporter` contract plus `Access`/`Exit`/`Start`/`Stop`/`Move`
  (§6.21, §7), and fixed two proven `Route` defects:
  label-destination validation and persistence of `stationExpression`.
  **This closes 100% of Arena Basic Process and Advanced Process flowchart
  modules.** Advanced Transfer is now covered as a minimum executable
  GenESyS-compatible subset plus explicit documented divergences; remaining
  work is limited to future features such as `Network`, guided transport, and
  fuller Arena module families that were intentionally not implemented here.
- **Phase C (parser/Arena Variables Guide cross-reference)**: completed as a
  current-state mapping/classification pass in §8. The dynamic parser
  extension API remains structurally present but effectively unused; current
  support is the hardcoded Bison/Flex grammar plus direct named object lookup.
- **Deferred decision note**: the unresolved `Process::AllocationType` versus
  internal `Delay` wiring question from §6.5 is isolated in
  `docs/ai_assistants/reference/PROCESS_ALLOCATIONTYPE_DELAY_DECISION.md` so
  the maintainer can decide it later without blocking the current close-out.
- **Reminders for a later session (explicit maintainer request,
  2026-08-29)**: revisit Statistic/Collector/Counter class-by-class detail
  once the maintainer explains the design further.

Phase A and the already-closed Basic Process / earlier Advanced Process
findings were preserved without reopening them. This continuation did change
in-scope runtime behavior where the code had been stubbed or where concrete
defects were demonstrated (`DropOff`, `Store`, `Unstore`, `Route`,
`Conveyor`/`Transporter` minimum flow). The unresolved `Process`/`Delay`
allocation-wiring observation (§6.5) remains a documented candidate, not an
autonomous fix.

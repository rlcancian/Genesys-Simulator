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
| Conveyor | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Segment | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Transporter | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Distance | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Network | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Network Link | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Activity Area | Advanced Transfer | ✅ `missing`, approved future work (§7) |
| Regulator Set | Flow Process | ⬜ (Flow Process panel likely `not-applicable`; not confirmed) |

### 4.2 Flowchart modules (components) — Phase B

| Arena module | Panel | Status |
|---|---|---|
| Create, Dispose, Assign | Basic Process | ✅ [§6](#6-phase-b--components-analyzed-entries) (batch 1) |
| Process, Decide, Batch, Separate, Record | Basic Process | ⬜ (batch 2) |
| Delay, Dropoff, Hold, Match, Pickup, ReadWrite, Release, Remove, Seize, Search, Signal, Store, Unstore, Adjust Variable | Advanced Process | ⬜ |
| Enter, Leave, PickStation, Route, Station (as flowchart module) | Advanced Transfer (general) | ⬜ |
| Access, Convey, Exit, Start, Stop | Advanced Transfer (conveyor) | ✅ `Access`/`Exit`/`Start`/`Stop` confirmed as incomplete stub templates (§7); `Convey` has no GenESyS equivalent at all |
| Activate, Allocate, Free, Halt, Move, Request, Transport | Advanced Transfer (transporter) | ⬜ (no GenESyS component found under these names; strong indication of `missing`, to confirm) |
| Tank, Sensor, Flow, Regulate, Seize Regulator, Release Regulator | Flow Process | ⬜ (likely `not-applicable`) |

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
- **Divergence (cross-cutting, see also §6.3)**: only a single collapsed
  `TotalTimeInSystem` statistic is produced. Arena's full breakdown
  (value-added/non-value-added/wait/transfer/other **time and cost**,
  separately) has no runtime equivalent here — consistent with §5.2's finding
  that `EntityType`'s VA/NVA/Waiting/Other cost fields are only initial
  values with no accumulation logic found so far.
- **Needs human decision**: whether/when the full Arena-style cost/time
  category breakdown should be implemented — `needs-human-decision` (see
  §6.3 for the consolidated cross-cutting note).

### 6.3 Cross-cutting finding: Arena's VA/NVA/Wait/Transfer/Other cost-and-time accounting has no confirmed runtime implementation

**[strong indication, spans §5.2 and §6.2]**: Arena tracks five cost/time
categories per entity throughout its life (value-added, non-value-added,
wait, transfer, other) and reports both per-category totals and a grand
total, fed by every module an entity passes through (Process, Delay, Seize/
Release wait time, Route/transfer time, etc.). In GenESyS:

- `EntityType` only exposes *initial* values for four of the five categories
  (VA/NVA/Waiting/Other — see §5.2), with no dedicated Transfer Cost field;
- `Dispose` only accumulates a single collapsed `TotalTimeInSystem` value,
  not per-category totals;
- no accumulation logic crediting time/cost to a specific category was found
  in `Create`, `Dispose`, or `Assign` (the three components read so far in
  Phase B).

This does not yet prove the category breakdown is entirely absent — `Process`,
`Delay`, `Seize`/`Release` (Phase B batch 2) are the components that would be
expected to perform such accumulation in Arena and have not been read yet.
Recorded now as a `strong indication` to avoid re-deriving it component by
component, and to flag it explicitly for a `needs-human-decision` once batch
2 confirms or refutes it: is a full VA/NVA/Wait/Transfer/Other cost-accounting
subsystem an intended GenESyS feature, or is total-time/total-cost tracking
(as currently implemented) the deliberate scope?

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

## 7. Confirmed-missing subsystem: Advanced Transfer conveyor/transporter data

**[confirmed]**: `source/plugins/data/` and `source/plugins/components/`
contain no `Conveyor`, `Segment`, `Transporter`, `Distance`, `Network`,
`NetworkLink`, or `ActivityArea` classes under any domain directory
(cross-checked against the full plugin listing taken at the start of this
audit). Arena's Advanced Transfer Panel devotes an entire subsystem to these
(*Getting Started with Arena*, "The Advanced Transfer Panel", pp. 79-108:
Conveyor flowchart modules Access/Convey/Exit/Start/Stop; Transporter
flowchart modules Activate/Allocate/Free/Halt/Move/Request/Transport; data
modules Sequence/Conveyor/Segment/Transporter/Distance/Network/Network
Link/Activity Area).

`source/plugins/components/MaterialHandling/{Access,Exit,Start,Stop}.cpp`
were read in full and are confirmed **incomplete stub templates**, not
finished components under any name: each is ~85 lines, its
`_onDispatchEvent()` body literally traces `"I'm just a dummy model and I'll
just send the entity forward"` and unconditionally passes the entity to the
front connection, and `_check()`/`_loadInstance()`/`_saveInstance()` are all
`// @TODO: not implemented yet` **[confirmed, all four files]**. Per
maintainer clarification (2026-08-29): these four names are intended as the
*action* components applied to a Conveyor or a Transporter unit (access/
release a conveyor segment or transporter unit, start/stop it), sharing
Arena's Conveyor-panel vocabulary because they play the same conceptual role
Arena's Access/Exit/Start/Stop play for a conveyor — not an unrelated reuse of
the names. They cannot be completed until the underlying Conveyor/Transporter
data definitions exist.

- **Level (data side)**: `missing`.
- **Level (Access/Exit/Start/Stop components)**: `partial` — present as
  named stubs only, `needs-human-decision` resolved (see below), implementation
  pending on the data-side subsystem.
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

## 8. Parser / `ParserChangesInformation` mechanism — preliminary note

Several audited classes override `_getParserChangesInformation()`
(`ModelDataDefinition.h:264`) — confirmed on `Queue`, `Schedule`, `Set`, `File`,
`Storage` **[confirmed via header inspection]**. `Queue::_getParserChangesInformation()`
currently returns an empty `ParserChangesInformation` object with commented-out
`getProductionToAdd()`/`getTokensToAdd()` calls (`Queue.cpp:244-249`)
**[confirmed]** — the mechanism exists structurally but is not populated for
`Queue`. Whether other audited classes populate it, and the full
registration/resolution pipeline, is deferred to a transversal parser-focused
pass (task instructions §8) and is **not yet analyzed** in this document.

## 9. Phase status

- **Phase A (data definitions)**: closed for this audit's purposes. 14 classes
  analyzed in detail (§5.1–§5.12); Statistic resolved by maintainer
  clarification (2026-08-29) as already covered by the kernel-level
  `Statistics`/`Collector`/`StatisticsCollector`/`Counter` classes, attached to
  the Record component and to many other components/data definitions —
  a full class-by-class detail pass on that group is deferred (reminder
  requested by the maintainer for a later session; do not forget). The
  Advanced Transfer conveyor/transporter/network data classes are confirmed
  `missing` (§7) with an approved future-implementation direction recorded,
  not yet scheduled.
- **Phase B (components)**: batch 1 of N complete (2026-08-29) — `Create`,
  `Dispose`, `Assign` (Basic Process, Logic domain), §6. Raised a cross-cutting
  `strong indication`/`needs-human-decision` (§6.3) about Arena's
  VA/NVA/Wait/Transfer/Other cost-and-time accounting having no confirmed
  runtime implementation, to be confirmed or refuted by batch 2. Also
  confirmed `Access`/`Exit`/`Start`/`Stop` (Advanced Transfer conveyor
  actions) as incomplete stub templates, and recorded the maintainer's
  approval (2026-08-29) to eventually implement the missing
  Conveyor/Transporter/Network subsystem (§7) as future work, not yet
  scheduled or issue-tracked.
  Batch 2 candidates: `Process`, `Decide`, `Batch`, `Separate`, `Record`
  (remaining Basic Process flowchart modules), then `Delay`, `Seize`,
  `Release` (Advanced Process — highest-confidence 1:1 candidates for the
  DiscreteProcessing domain).
- **Phase C (parser/Arena Variables Guide cross-reference)**: not started;
  §8 above is a preliminary observation only, not a completed pass. Deferred
  by maintainer request (2026-08-29) — reminder needed in a later session.
- **Reminders for a later session (explicit maintainer request,
  2026-08-29)**: (a) revisit Statistic/Collector/Counter class-by-class detail
  once the maintainer explains the design further; (b) revisit Phase C
  (parser + Arena Variables Guide cross-reference).

No code behavior was changed in Phase A or Phase B batch 1 — only
documentation (this file and the corresponding class headers). No bug was
demonstrated that would justify a source change under `GOVERNANCE.md` §5
change-policy.

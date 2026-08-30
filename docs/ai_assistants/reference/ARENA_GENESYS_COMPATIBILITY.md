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
| Create, Dispose, Assign, Process, Decide, Batch, Separate, Record | Basic Process | ✅ [§6](#6-phase-b--components-analyzed-entries) (batches 1-2) — all Basic Process flowchart modules covered |
| Delay | Advanced Process | ✅ [§6.3](#63-cross-cutting-finding-revised-in-batch-2-arenas-vanvawaittransferother-time-allocation-is-implemented-cost-allocation-is-not-confirmed), [§6.5](#65-process) (audited ahead of schedule via `Process`) |
| Seize, Release | Advanced Process | ⬜ (touched only incidentally via `Process`/`Delay`; full audit pending, batch 3) |
| Dropoff, Hold, Match, Pickup, ReadWrite, Remove, Search, Signal, Store, Unstore, Adjust Variable | Advanced Process | ⬜ (batch 3+) |
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
- **Needs human decision**: whether "Duplicate Original" semantics should be
  added to `Separate`, or whether duplication is intended to be achieved
  through a different mechanism (e.g. `Clone`, in
  `source/plugins/components/DiscreteProcessing/Clone.{h,cpp}` — not yet
  audited) — `needs-human-decision`.

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
  Batch 3 candidates: `Seize`, `Release` (finish DiscreteProcessing; resolve
  the `Seize.cpp:227` `//@TODO: Check it!` allocation-attribute question and
  the `Process` allocation-wiring question), then `Dropoff`, `Hold`, `Match`,
  `Pickup`, `ReadWrite`, `Remove`, `Search`, `Signal`, `Store`, `Unstore`,
  `Adjust Variable` (remaining Advanced Process).
- **Phase C (parser/Arena Variables Guide cross-reference)**: not started;
  §8 above is a preliminary observation only, not a completed pass. Deferred
  by maintainer request (2026-08-29) — reminder needed in a later session.
- **Reminders for a later session (explicit maintainer request,
  2026-08-29)**: (a) revisit Statistic/Collector/Counter class-by-class detail
  once the maintainer explains the design further; (b) revisit Phase C
  (parser + Arena Variables Guide cross-reference).

No code behavior was changed in Phase A or Phase B (batches 1-2) — only
documentation (this file and the corresponding class headers). No bug was
demonstrated with enough certainty of intent to justify a source change under
`GOVERNANCE.md` §5 change-policy; the `Process`/`Delay` allocation-wiring
observation (§6.5) is a candidate, not a confirmed fix.

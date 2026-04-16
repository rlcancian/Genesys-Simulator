# Discussion: internal, attached and custom attached ModelDataDefinitions

Date: 2026-04-15

This document records the ongoing design discussion about the responsibilities of the kernel, plugins and Qt GUI when dealing with `ModelDataDefinition` objects used by other model elements.

In this document, "element" means either a `ModelDataDefinition`, a `ModelComponent`, or a subclass of either one.

## Context

The recent Property Editor work exposed a conceptual ambiguity:

- Selecting `Criar Resource` for the `Resource` property of a `SeizableItem` should create and bind only that `Resource`.
- It should not also create and bind a `Queue` to the parent `Seize` `Queue` property.
- A previous corrective attempt avoided that unwanted `Queue` creation, but also made some automatic internal/support data definitions stop being created, for example data definitions expected by `Create`.

The root problem is that `ModelDataDefinition` creation and `internal/attached` registration have historically been mixed together inside `_createInternalAndAttachedData()`, even though not all model data references have the same meaning.

## Kernel invariants

The following points are considered kernel-level invariants or desired invariants:

1. Every `ModelDataDefinition` used by another element must be registered exactly as either internal data or attached data of some owner element.
2. The registry used for that is:
   - `ModelDataDefinition::_internalData`
   - `ModelDataDefinition::_attachedData`
3. If a `ModelDataDefinition` is not reachable through these maps, it may be treated as orphaned and removed from the model.
4. The intended single place for a subclass to register its used data definitions is `ModelDataDefinition::_createInternalAndAttachedData()`, normally through an override.
5. `ModelComponent::CreateInternalData(component)` and `ModelDataDefinition::CreateInternalData(modeldatum)` are wrapper entry points that call the virtual `_createInternalAndAttachedData()`.

The current code confirms the orphan risk:

- `Model::_clearOrphanedDataDefinitions()` starts with all data definitions as orphan candidates.
- It removes candidates that appear in `getInternalData()` or `getAttachedData()` of model data definitions.
- It also removes candidates that appear in `getInternalData()` or `getAttachedData()` of model components.
- Remaining candidates are removed from the `ModelDataManager`.

Therefore, the conversation's claim that `internalData` and `attachedData` are the ownership/reference graph used to avoid orphan deletion is confirmed by the current kernel code.

## Current code caveats

The intended "only `_createInternalAndAttachedData()` mutates internal/attached maps" rule is not fully true in the current codebase.

Examples found during inspection:

- `Assign::_check()` clears and inserts attached data for assignment destinations.
- `Label::_check()` removes and inserts the `EnteringLabelComponent` attached reference.
- `Set::_check()` inserts attached member references.
- `ModalModelDefault::addOutputExpressionReference()` and `removeOutputExpressionReference()` mutate attached references directly.

These may be valid historical choices, but they contradict the intended architectural rule. A future cleanup should move or centralize these mutations so check methods validate state instead of altering the internal/attached graph.

## Categories of used ModelDataDefinitions

The discussion identifies three practical categories of model data definitions used by an element.

### 1. Statistics/output support data

These data definitions exist to collect output statistics.

Typical classes:

- `Counter`
- `StatisticsCollector`

Typical behavior:

- Usually internal.
- Usually created only if `_reportStatistics` is true.
- Owned and defined by the containing element.
- Not user-selected as a modeling property.
- Should not be editable through the Property Editor as independent user-controlled model data.

Important conclusion:

- These should always be reconciled by `_createInternalAndAttachedData()`.
- If `_reportStatistics` is true and the support object is missing, it is legitimate for `_createInternalAndAttachedData()` to create it.

### 2. Shared/expression/support data

These data definitions exist to share runtime information, values or references with other elements.

Typical classes:

- `Attribute`
- Some expression-referenced data definitions
- Possibly other support data selected by plugin logic

Typical behavior:

- Usually attached.
- The owning element determines the need.
- The user is not choosing the data definition as a semantic property of the component.
- Should not normally be editable through the Property Editor as a user-controlled child object.

Important conclusion:

- These should generally be registered whenever they already exist.
- Whether missing support data should be created automatically is a plugin-specific question.
- The current code historically creates some attributes via `_attachedAttributesInsert()`.

### 3. Nature/semantic model data

These data definitions are required by the nature of what the element models.

Examples:

- `Create` uses an `EntityType`.
- `Enter` uses a `Station`.
- `Seize` uses a `Queue` or `Set` through `QueueableItem`.
- `Seize` and `Release` use `Resource` or `Set` through `SeizableItem`.

Typical behavior:

- Usually attached, but not necessarily always.
- The user is expected to choose or create the referenced model data.
- These are the data definitions that should appear as editable model-object properties in the Property Editor.

Important conclusion:

- This is the category that the legacy auto-create flag historically targeted.
- The flag has now been removed from `Model`; the remaining code behaves as if the old flag were always enabled.
- This is a transitional state. The longer-term GUI direction may still require distinguishing automatic relation reconciliation from explicit user-driven creation of editable references.

## Removed legacy auto-create flag

Historical purpose:

- Before the GUI workflow matured, missing model data definitions caused check failures.
- The legacy auto-create flag allowed plugins to create missing nature/semantic data automatically.

Current state:

- `TraitsKernel<Model>` no longer defines an auto-create switch.
- `Model` no longer stores an auto-create attribute and no longer exposes getter/setter methods for it.
- Every old check of the flag was collapsed as if the flag returned `true`.
- `_createInternalAndAttachedData()` implementations now always get their former creation branch when that flag was the only blocker.

Important distinction:

- Removing the flag simplifies the kernel contract, but it does not solve the deeper modeling distinction between support data and user-editable semantic references.
- Future work may need a more explicit per-property or per-relation policy instead of a model-wide switch.

## Property Editor hypothesis

The current Property Editor does not infer properties directly from C++ getter/setter names by scanning methods.

Instead, plugin constructors explicitly create `SimulationControl` objects and add them as editable properties, for example:

- `Enter` creates a `SimulationControlGenericClass<Station*, Model*, Station>` bound to `getStation()` and `setStation()`.
- `Seize` creates `SimulationControlGenericClassNotDC<QueueableItem*, ...>` for `QueueableItem`.
- `QueueableItem` and `SeizableItem` expose their own controls, which is why their nested `Queue`, `Set` or `Resource` properties appear.

So the practical rule is close to the hypothesis, but the mechanism is explicit metadata, not automatic reflection:

- A model data reference is editable if the owner exposes it through a writable `SimulationControl`.
- For model data references, this currently maps to `SimulationControlGenericClass<T*, Model*, T>`.
- The GUI receives this through `GenesysPropertyIntrospection`, including flags such as:
  - `isModelDataDefinitionReference`
  - `supportsExistingObjectSelection`
  - `supportsObjectCreation`
  - `readOnly`

This is a stronger criterion than simply "the object appears in `_internalData` or `_attachedData`".

## Graphical visibility vs editability

Visibility and editability must be separated.

Visibility:

- A `ModelDataDefinition` can be displayed graphically if it is reachable through internal or attached relations and the corresponding View/Show option is enabled.
- `GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer()` currently builds visible graphical data definitions by traversing `getAttachedData()` and `getInternalData()` from graphical model components and nested data definitions.

Editability:

- Most visible data definitions should be display-only in the Property Editor.
- Only data definitions that are exposed by a writable model-object reference property should be editable as user-controlled model data.

Current gap:

- `GraphicalModelDataDefinition` currently does not store an `editable` flag.
- `PropertyEditorController` binds a selected `GraphicalModelDataDefinition` directly to `ObjectPropertyBrowser::setActiveObject(...)`.
- `ObjectPropertyBrowser` currently does not receive enough metadata to know whether that selected graphical data definition is user-editable or read-only.

## Proposed GUI-side editability criterion

A practical first criterion:

1. A `GraphicalModelDataDefinition` is user-editable if the represented `ModelDataDefinition*` is the current value of at least one writable `SimulationControl` exposed by some owner element.
2. The relevant control should be a model data definition reference, not just any inline helper object.
3. The owner may be:
   - a `ModelComponent`
   - a `ModelDataDefinition`
   - an auxiliary inline object such as `QueueableItem` or `SeizableItem`, reached through an editable property tree

This means:

- `Enter::_station` should be editable because `Enter` exposes a writable `Station` property.
- `Enter::_numberIn` should not be editable because it is a statistics `Counter` and is not exposed as a user-editable property.
- `SeizableItem::Resource` should be editable because `SeizableItem` exposes a writable `Resource` property.
- `Seize::QueueableItem::Queue` should not be filled automatically when editing `SeizableItem::Resource`.

## Proposed `GraphicalModelDataDefinition` metadata

Add GUI-only metadata to `GraphicalModelDataDefinition`, for example:

- `bool isEditableInPropertyEditor() const`
- `void setEditableInPropertyEditor(bool)`

Initial assignment strategies:

1. When a data definition is created from a Property Editor combo action such as `Criar Resource`, mark the corresponding graphical item as editable.
2. When rebuilding graphical data definitions from an existing model, infer editability by scanning exposed property descriptors/controls from all model elements and checking whether any writable model data reference points to the same data definition.
3. For data definitions created purely as internal/statistics/support objects, keep editability false by default.

This metadata should remain GUI-layer state. It should not force GUI editability concepts into the kernel.

## Current implementation risks

Several current changes or patterns need revision:

1. Some category 1 statistics objects were historically gated by the legacy auto-create flag. That has now been collapsed to always create when the surrounding semantic condition holds.
2. `_attachedAttributesInsert()` now creates missing attributes whenever they are needed.
3. The Property Editor currently calls `ModelDataDefinition::CreateInternalData(_modelObject)` after creating or selecting references. That is necessary for relation registration, but dangerous if `_createInternalAndAttachedData()` still creates unrelated missing nature/semantic data.
4. Some `_check()` methods mutate attached data, which conflicts with the intended separation between validation and relation registration.
5. `GraphicalModelDataDefinition` has no editability flag today, so selecting support/statistics data currently risks making it editable if the browser is given normal kernel edit controls.

## Working design direction

Kernel/plugins:

- `_createInternalAndAttachedData()` should be idempotent.
- It should always register existing used data definitions in `_internalData` or `_attachedData`.
- It should create category 1 statistics/internal support data when plugin semantics require it.
- It should create category 2 shared/support data when plugin semantics require it.
- With the legacy flag removed, existing category 3 creation code now also executes when its other semantic conditions hold.
- This behavior is accepted as the current checkpoint, but it remains conceptually distinct from the future GUI goal of explicit user-driven reference creation.

GUI:

- Let the Property Editor explicitly create category 3 references through combo actions.
- After explicit creation/selection, call `_createInternalAndAttachedData()` for the owner to register relations and support data.
- Mark/infer which graphical data definitions are editable.
- Show read-only properties for non-editable graphical data definitions.

## Proposed action plan

### Phase 1: Documentation and audit

1. Keep this document updated as the design evolves.
2. Audit all overrides of `_createInternalAndAttachedData()`.
3. Classify each used model data definition as:
   - statistics/internal support
   - shared/expression/support
   - nature/semantic user-selectable reference
4. Record code paths that currently mutate `_internalData` or `_attachedData` outside `_createInternalAndAttachedData()`.

### Phase 2: Restore kernel/plugin semantics

1. Review every `_createInternalAndAttachedData()` now that the model-wide auto-create flag has been removed.
2. Confirm that category 1 objects are created only when `_reportStatistics` or equivalent semantics require it.
3. Confirm that category 2 support objects are created only when plugin semantics require them.
4. Identify category 3 references that should eventually become explicit GUI-created references instead of implicit plugin-created references.
5. Ensure every existing referenced data definition is reinserted into internal/attached maps even when no new object is created.

### Phase 3: Property Editor editability model

1. Add GUI-side editability metadata to `GraphicalModelDataDefinition`.
2. Infer editability from writable `SimulationControl` model-data-reference properties.
3. Preserve read-only display for visible but non-editable graphical model data definitions.
4. Continue to allow explicit creation/selection/removal for category 3 references through combo-box properties.

### Phase 4: Cleanup legacy inconsistencies

1. Move attached/internal registration out of `_check()` where feasible.
2. Make `_createInternalAndAttachedData()` overrides idempotent and structurally consistent.
3. Replace broad implicit category 3 creation with explicit GUI/property policies where appropriate.
4. Add tests for:
   - orphan protection through internal/attached maps
   - no implicit creation of unrelated category 3 references
   - statistics/support data creation
   - Property Editor read-only vs editable graphical data definitions

## Open questions

1. Which category 3 references should stop being created implicitly once the Property Editor workflow is mature enough?
2. Should editability be inferred dynamically every rebuild, persisted in graphical serialization, or both?
3. How should inline helper classes such as `QueueableItem` and `SeizableItem` advertise their model data reference properties in a way that is robust and not dependent on tree traversal details?
4. Should `Model::_createInternalDataDefinitions()` eventually be split into:
   - relation/support reconciliation
   - nature-data auto-creation?

## Implementation note: GUI editability inference

The first practical implementation follows Phase 3 without changing the broad plugin/kernel policy yet:

1. `GraphicalModelDataDefinition` now carries GUI-side editability metadata through `isEditableInPropertyEditor()` and `setEditableInPropertyEditor()`.
2. `PropertyEditorController` dynamically infers editable data definitions from writable `SimulationControl` properties that are model-data-definition references.
3. The inference walks:
   - all `ModelComponent` properties
   - all `ModelDataDefinition` properties
   - inline helper objects exposed by controls
   - object-list elements, including helper items such as `QueueableItem` and `SeizableItem`
4. A data definition is marked editable only when it is currently referenced by a writable model-data-reference property.
5. `ObjectPropertyBrowser` receives this editable set and treats selected graphical data definitions not in the set as read-only kernel objects.
6. Read-only graphical data definitions are still displayed in the Property Editor, but scalar edits, enum edits, object-reference combo actions, list creation, and specialized list editors are blocked.
7. `ModelComponent` and its subclasses are explicitly outside this read-only filter: every component remains editable in the Property Editor. The read-only distinction applies only to selected graphical `ModelDataDefinition`s that are not components.

This keeps editability as GUI state and avoids introducing Property Editor policy into the kernel. It also preserves the existing combo workflow for user-editable category 3 references while making category 1 and category 2 graphical data definitions inspectable but not editable by default.

The broad cleanup of `_createInternalAndAttachedData()` implementations remains postponed. The current implementation is intentionally scoped to the GUI-side distinction between visible data definitions and editable data definitions.

## Implementation note: plugin instance creation

The plugin creation path was corrected so newly created model elements always invoke `CreateInternalData` immediately after construction.

Affected paths:

1. `Plugin::newInstance(Model*, std::string)`
2. `PluginManager::newInstance<T>(Model*, std::string)`

These paths no longer guard `ModelDataDefinition::CreateInternalData(instance)` with `the legacy auto-create flag`.

The reason is conceptual: creating and registering internal/attached data is not the same thing as legacy automatic creation of user-editable nature/semantic data. Every new element must have a chance to reconcile its internal/attached relationships immediately after creation. As of the later cleanup, the legacy flag no longer exists in `Model`, so individual `_createInternalAndAttachedData()` implementations must encode their own semantic conditions directly.

---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: on-plugin-contract-change
status: active
tracks: 511
---

# Plugin Development Reference

## 1. Scope

Technical reference for GenESyS component plugins, data definitions, registration, factories, metadata, persistence, static target composition and future dynamic-package boundaries.

Current architecture and policy remain in [`../ARCHITECTURE.md`](../ARCHITECTURE.md) and [`../GOVERNANCE.md`](../GOVERNANCE.md). Current target evidence and decision blockers remain in [`../STATUS.md`](../STATUS.md) and [`../BACKLOG_HUMAN.md`](../BACKLOG_HUMAN.md).

## 2. Current implementation reality

Current production plugins are statically aggregated. Component and data-definition sources are compiled into static archives and registered through the current connector/runtime graph.

Do not describe the current repository as supporting independently distributed runtime dynamic plugin packages.

Generated evidence has shown that the targets named full and minimal currently compile the same component source domain, while optional GLPK configuration differs. Target correction is blocked on issue #492.

## 3. Plugin contracts

Every plugin must make explicit:

- public type/name and aliases;
- component versus data-definition role;
- factory entry point;
- registration and discovery path;
- model ownership/adoption;
- metadata and field/template completion;
- persistence fields and defaults;
- dependencies on other plugins/data definitions/tools;
- validation requirements;
- optional feature/dependency behavior;
- thread, callback and teardown assumptions.

## 4. Registration and factories

Inspect real registration code before changing names or factories.

Requirements:

- deterministic registration;
- no duplicate or ambiguous type identifiers;
- factory results adopted by the intended model/manager or explicitly owned locally;
- failed construction leaves no partial registration or leaked resources;
- metadata completion must not retain temporary model-owned objects;
- compatibility aliases require an explicit migration reason and tests.

## 5. Persistence

Plugin persistence must preserve:

- stable external type identifiers;
- load/save symmetry;
- supported historical model fixtures;
- defaults for absent fields;
- explicit diagnostics for unsupported/invalid data;
- reference resolution order;
- no silent loss of scientific or execution semantics.

Changing a class name does not automatically justify changing its persisted plugin name.

## 6. Ownership and lifecycle

Before changing plugin pointers or containers, map:

- model/manager ownership;
- attached/internal data-definition ownership;
- observer links;
- temporary factory objects;
- statistics/reporting objects;
- replication callbacks;
- unload/destruction order;
- Qt parent-child ownership for GUI-only plugin surfaces.

Use focused ASan/LSan/UBSan where lifetime changes are involved. A model-local object must not escape through metadata unless a durable ownership contract exists.

## 7. Static target discipline

Until issue #492 is decided:

- do not remove or rename `genesys_plugins_components` or `genesys_plugins_components_minimal`;
- do not create an alias that silently selects an architecture;
- do not move sources between them;
- do not change GLPK definitions/links as incidental cleanup;
- do not mix target consolidation with dynamic ABI work.

Any future static-target correction must validate GLPK present and absent, generated source sets, direct/reverse dependencies, final links, symbols and runtime behavior.

## 8. Future dynamic packages

The approved future in-process boundary is a stable C ABI:

- opaque handles;
- explicit create/destroy;
- versioned function/capability tables;
- size/version fields for extension;
- structured error/status values;
- host-owned or explicitly allocated buffers with matching release operations;
- no STL, Qt types, C++ classes, templates, RTTI assumptions or C++ exceptions crossing the boundary;
- explicit ABI, semantic and package versioning;
- symbol visibility/export policy;
- dependency and unload safety.

A pilot must precede broad migration and must be selected only after the current static overlap is resolved.

## 9. Domain grouping

Current plugin domains include:

- discrete/general components;
- data definitions and resources;
- continuous/hybrid;
- modal/EFSM/Petri-net;
- biochemical/whole-cell;
- electronics/circuit integration;
- external code/Python integration;
- AI-assisted components.

Domain-specific scientific semantics belong in [`SCIENTIFIC_DOMAINS.md`](SCIENTIFIC_DOMAINS.md), not in generic plugin factory code.

## 10. External integration plugins

For CppForG, PythonForG, external processes, generated code or dynamic libraries:

- validate generated source and compiler invocation;
- control temporary paths and cleanup;
- never expose secrets through commands/logs;
- bound resource use and execution time;
- define failure, cancellation and unload behavior;
- keep model ownership separate from language/runtime wrappers;
- do not expose unstable managers or raw ownership casually.

## 11. GUI configuration

Graphical plugin configuration belongs in application/property-editor layers and should communicate through stable model/plugin contracts.

Do not add Qt widget dependencies to kernel plugin classes merely to expose configuration. Reusable property editors should validate type, domain, references, expressions and persistence without changing plugin ownership.

## 12. Validation checklist

1. inspect current registration/factory/persistence code;
2. define compatibility and ownership impact;
3. add focused nominal, invalid, persistence and lifecycle tests;
4. validate ordinary and kernel test graphs;
5. run smoke/application checks when runtime discovery changes;
6. run sanitizers for ownership/unload changes;
7. validate optional dependencies in enabled and disabled modes;
8. record what is not proven.

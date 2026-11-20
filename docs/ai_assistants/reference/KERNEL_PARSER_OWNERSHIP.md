---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: on-kernel-contract-change
status: active
tracks: 511
---

# Kernel, Parser, and Ownership Reference

## 1. Scope

Detailed guidance for safe changes in:

```text
source/kernel/
source/parser/
source/tests/
```

Use [`../ARCHITECTURE.md`](../ARCHITECTURE.md) for durable boundaries and [`../GOVERNANCE.md`](../GOVERNANCE.md) for normative change/validation policy.

## 2. Kernel principles

- preserve simulation semantics unless a defect or approved contract change is demonstrated;
- keep event scheduling, model state, persistence, plugin registration and reporting lifecycles explicit;
- do not turn local modernization into a repository-wide refactor;
- isolate interfaces only where a real client/concrete-class dependency is being reduced;
- introduce namespaces incrementally with full call-site, persistence and plugin-name impact mapping.

## 3. Ownership map before change

For each pointer/reference/container, classify:

- owner;
- observer;
- temporary borrower;
- parent-child Qt ownership where applicable;
- adoption point;
- replacement behavior;
- destruction order;
- callback/captured lifetime;
- exceptional/early-return path;
- copy/move behavior.

Check constructors, managers, `insert`/`newModel`-style adoption, teardown, persistence, plugin factories and tests. Do not infer ownership only from a raw pointer member.

## 4. Preferred correction sequence

1. reproduce with a focused test or sanitizer;
2. capture the red checkpoint without production changes;
3. correct the smallest ownership boundary;
4. validate double-delete/use-after-free as well as leak removal;
5. run focused and aggregate tests;
6. record what remains unproven.

Use RAII and deleted unsafe copy/move operations where ownership is confirmed. Do not convert observing pointers to owning smart pointers.

## 5. QObject lifetime

For Qt objects:

- inspect `QObject` parent assignment before adding explicit deletion;
- avoid double ownership between `std::unique_ptr` and parent-child deletion;
- keep GUI ownership out of kernel domain objects unless the existing design explicitly requires Qt;
- validate queued signals/callbacks when object lifetime changes.

## 6. Simulation lifecycle

Distinguish:

- construction;
- model insertion/adoption;
- related-data creation;
- replication initialization;
- first public operation;
- event scheduling;
- event processing;
- replication end;
- model destruction.

A public operation must either be valid before model-wide initialization or fail deterministically with a documented precondition. Reporting-disabled paths must not dereference absent statistics/accounting objects.

## 7. Parser contracts

Parser changes must preserve or explicitly redefine:

- tokenization and grammar;
- operator precedence/associativity;
- identifier/function lookup;
- indexed/sparse variable and attribute access;
- assignment semantics;
- type/domain errors;
- diagnostics and source location where available;
- compatibility with persisted expressions and representative models.

Never implement a parser feature only through one direct C++ call path if the public expression syntax is part of the requirement.

## 8. Matrix and indexed values

Current sparse indexed semantics must remain stable unless a new phase defines:

- shape/rank model;
- dimension typing;
- scalar/matrix assignment rules;
- missing-index behavior;
- conversion/broadcasting policy;
- persistence format;
- parser grammar and diagnostics.

Matrix-to-matrix assignment and typed dimensions are future contracts, not implied by existing scalar/indexed support.

## 9. Persistence impact

Kernel/parser changes require inspection of:

- saved field names and defaults;
- plugin type identifiers;
- model load/save symmetry;
- backward-compatible missing-field behavior;
- reference resolution;
- expression serialization;
- model-specific generated `.gen` fixtures.

Do not rename public/persisted identifiers as cosmetic modernization.

## 10. Modern C++ use

Apply only where supported by current C++23 toolchain and technically useful:

- `nullptr`, `override`, `final`;
- `= default`, `= delete`, Rule of Zero;
- RAII and `std::unique_ptr` for confirmed ownership;
- `noexcept`, `constexpr`, `[[nodiscard]]` where contracts justify them;
- range-for and algorithms when they improve correctness/readability;
- `std::optional`, `std::variant`, `std::span`, `std::string_view` only with clear lifetime/API benefit.

Avoid style-only churn, broad signature changes and ABI-sensitive template exposure.

## 11. Testing priorities

- nominal behavior;
- empty/zero/null/disabled cases;
- first-use and repeated-use lifecycle;
- invalid inputs and deterministic errors;
- persistence round trips;
- regression for proven defects;
- light integration across parser/kernel/plugin boundaries;
- ASan/LSan/UBSan for lifetime/UB changes.

## 12. Known current boundaries

- four disabled historical Search/Remove blocks are duplicates of active focused coverage;
- focused ownership sanitizer proves only plugin-completion exercised paths;
- broad namespace migration remains deferred;
- dynamic plugin ABI work must not be mixed with kernel ownership stabilization;
- current exact status belongs in [`../STATUS.md`](../STATUS.md).

---
document_type: reference
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-07-22
review_cadence: on-application-contract-change
status: active
tracks: 511
---

# Applications, Tools, and Model-Specific Workflows Reference

## 1. Scope

Detailed guidance for shell, worker, Qt6 applications, reusable tools, model-specific applications, generated models, property editors, optimizer/data-analysis/DOE workstations and application-to-backend boundaries.

Durable architecture is in [`../ARCHITECTURE.md`](../ARCHITECTURE.md); build commands are in [`BUILD_TEST_PACKAGING.md`](BUILD_TEST_PACKAGING.md); current readiness is in [`../STATUS.md`](../STATUS.md).

## 2. Application boundaries

Current application families:

```text
source/applications/
├── shell/
├── worker/
├── modelSpecific/
└── gui/
    ├── genesys/
    ├── httpworker/
    ├── dataanalyser/
    ├── optimizer/
    └── ai_assistant/
```

`doexperiments` remains planned and intentionally unavailable rather than represented by a misleading empty application.

Applications may orchestrate workflows, but reusable simulation/statistics/optimization logic belongs in kernel/plugin/tool layers.

## 3. Qt6 application separation

Independent tools are launched from the main GUI through `QProcess`; the main GUI should not link their concrete window implementations merely to open them.

Preserve:

- stable executable/output names where supported;
- independent CMake presets/targets;
- backend/frontend separation;
- explicit command-line/startup contract;
- error reporting when a child executable is absent;
- no `system()`-based launcher.

A GUI move must update sources, CMake, tests, install/package paths and launch lookup together.

## 4. GUI validation levels

### Startup

- exact executable discovered;
- Qt6/XCB starts under private Xvfb;
- PID-associated window exists;
- bounded liveness;
- controlled teardown;
- no residual process.

### Interaction

- actions/menus/widgets exercised;
- invalid input handled;
- application-owned close/cancellation;
- backend effects verified.

### Functional Level 3

- realistic end-to-end fixture;
- persistence/import/export as applicable;
- error/recovery behavior;
- reproducibility;
- documented limitations;
- numerical/scientific oracle where relevant.

Do not infer higher levels from startup evidence.

## 5. Shell

The shell supports complete command strings supplied as separate argv entries before interactive input.

Preserve:

- deterministic command routing;
- facade separation;
- clean exit;
- clear unknown/invalid-command diagnostics;
- scriptable output where a contract exists.

File-based plugin autoload remains blocked on issue #496. Do not guess the install/search/fallback path.

## 6. Worker

The worker is intended for a controlled academic intranet after explicit security decisions and implementation.

Keep separate:

- public health/readiness endpoints;
- authenticated job/model endpoints;
- bind-address configuration;
- authentication/TLS;
- request/body limits and quotas;
- job/process/filesystem isolation;
- audit logging;
- cancellation/expiry/cleanup.

The GUI controller does not replace backend security.

## 7. Model-specific applications and generated models

Model-specific source applications and generated `.gen` files require a reproducible mapping:

- source application/preset;
- generated output path/name;
- build result;
- bounded run result;
- generation success;
- load/parse validation;
- expected model semantics;
- known external dependencies/timeouts.

A historical sweep is a snapshot, not a permanent guarantee. Revalidate current failures before opening corrections.

Do not silently overwrite curated models. Use temporary output/diffs and record generator provenance.

## 8. Property editors and Expression Builder

Reusable property-editing infrastructure should:

- derive editor behavior from real field metadata;
- support scalar, expression, reference and structured values deliberately;
- validate type/domain/reference constraints;
- preserve commit/cancel semantics;
- avoid direct coupling to one plugin/window;
- maintain persistence and undo/redo expectations where supported;
- keep model ownership outside the editor.

The Expression Builder/property-editor refactor remains an autonomous candidate only after its exact current code and consumers are revalidated.

## 9. Data Analyser

A mature backend should separate:

- data import/typing/missing-value handling;
- descriptive statistics;
- distribution fitting;
- hypothesis tests and intervals;
- charts/diagnostics;
- analysis-study definition;
- export/persistence/provenance.

GUI startup is validated; data workflows and statistical correctness are not yet Level 3.

## 10. Optimizer

The GUI/backend boundary must remain explicit.

Required future workflow:

1. discover/select controls and responses;
2. define objectives and constraints;
3. validate readiness;
4. mutate/evaluate model with reproducible replications;
5. maintain archive/ranking/quality indicators;
6. pause/resume/cancel/checkpoint;
7. persist/export results and diagnostics.

No algorithm is selected by this reference. That human decision remains in `BACKLOG_HUMAN.md`.

## 11. Do Experiments / DOE

Future DOE functionality should define:

- full/fractional factorial and other supported designs;
- factor types/levels/coding;
- replication/randomization/blocking;
- simulation execution and response collection;
- effects/model fitting/ANOVA/diagnostics;
- RSM/optimization integration;
- persistence/export/reproducibility.

Do not create an empty GUI before a bounded backend/product contract exists.

## 12. AI Assistant application

Separate:

- UI/session orchestration;
- provider abstraction;
- credentials/secret storage;
- prompt/context construction;
- tool calls and typed results;
- redaction/logging;
- network failures/cancellation;
- model/plugin mutations requiring explicit validation.

No provider credential may be embedded in source or exposed in logs/argv/browser configuration.

## 13. Installation and launcher compatibility

When changing applications:

- inspect CMake target and `OUTPUT_NAME`;
- preserve installed names or define migration;
- update desktop files/icons/metainfo/package manifests;
- validate relative/resource lookup after installation, not only from build tree;
- ensure `QProcess` lookup matches package layout;
- test missing executable and permission failures.

## 14. Current pending routes

- autonomous technical candidates: [`../BACKLOG_AUTONOMOUS.md`](../BACKLOG_AUTONOMOUS.md);
- security/product/release decisions: [`../BACKLOG_HUMAN.md`](../BACKLOG_HUMAN.md);
- startup validation history: [`../history/evidence/2026/07/VALIDATION_LEDGER.md`](../history/evidence/2026/07/VALIDATION_LEDGER.md).

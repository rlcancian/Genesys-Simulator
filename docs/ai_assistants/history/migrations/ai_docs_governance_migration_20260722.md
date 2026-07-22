---
document_type: migration-record
authority: informative
owner: project-maintainer
date: 2026-07-22
status: phase-d0-in-progress
immutable_after_completion: true
tracks: 511
---

# AI Assistant Documentation Governance Migration — 2026-07-22

## 1. Objective

Restructure `docs/ai_assistants/` into a small canonical governance layer plus bounded runbooks, topic references, current state/backlogs, and historical evidence.

The migration is designed to:

- reduce competing sources of truth;
- make autonomous task selection safe;
- distinguish stable policy from volatile state;
- distinguish executable work from human decisions;
- preserve audit evidence without forcing every agent to read it;
- retain full Git traceability;
- remove temporary historical clutter only after review and retention gates.

## 2. Maintainer authorization and freeze

The maintainer approved the proposed structure, including branch `WiP20260722/ai-docs-governance`.

Repository-changing scheduled GenESyS tasks were paused for the duration of this migration. No other autonomous process is expected to modify the repository while D0 is prepared and reviewed.

Tracking issue: #511.

## 3. Inventory basis

The pre-migration inventory identified approximately:

- 40 Markdown files directly under `docs/ai_assistants/`;
- 8 Markdown files under `docs/ai_assistants/plugins/`;
- 25 Markdown files under `docs/ai_assistants/oldies/`;
- approximately 73 Markdown files total.

The exact count should be revalidated locally or through an authoritative directory-tree action before final deletion/move phases. Connector code search was sufficient for the planning inventory but is not treated as a deletion manifest.

## 4. Approved target structure

```text
docs/ai_assistants/
├── README.md
├── GOVERNANCE.md
├── ARCHITECTURE.md
├── STATUS.md
├── BACKLOG_AUTONOMOUS.md
├── BACKLOG_HUMAN.md
├── runbooks/
│   ├── AUTONOMOUS_AGENT.md
│   ├── LOCAL_AGENT.md
│   └── GITHUB_AGENT.md
├── reference/
│   ├── README.md
│   ├── BUILD_TEST_PACKAGING.md
│   ├── PLUGINS.md
│   ├── SCIENTIFIC_DOMAINS.md
│   ├── APPLICATIONS_TOOLS_MODELS.md
│   └── API_COVERAGE.md
├── history/
│   ├── CHANGELOG_AI.md
│   ├── evidence/
│   └── migrations/
└── archive/
    └── README.md
```

The top-level allowlist after completed consolidation is:

```text
README.md
GOVERNANCE.md
ARCHITECTURE.md
STATUS.md
BACKLOG_AUTONOMOUS.md
BACKLOG_HUMAN.md
```

The allowlist must not be enforced until legacy top-level files have been consolidated and moved safely.

## 5. Document-type model

| Type | Authority | Update pattern | Examples |
|---|---|---|---|
| governance | normative | rare, reviewed | `GOVERNANCE.md` |
| architecture | normative reference | when boundaries/decisions change | `ARCHITECTURE.md` |
| status | current-state | after material merge/status change | `STATUS.md` |
| autonomous backlog | executable task source | on task transition | `BACKLOG_AUTONOMOUS.md` |
| human backlog | decision source | on decision/status change | `BACKLOG_HUMAN.md` |
| runbook | operational | when tools/process change | `runbooks/*.md` |
| reference | topic-specific | when implementation/reference changes | `reference/*.md` |
| evidence | informative, immutable | one record per bounded checkpoint | `history/evidence/...` |
| migration/history | informative, immutable | completed integration/migration | `history/migrations/...` |
| archive | non-current | temporary retention | `archive/` and existing `oldies/` |

## 6. Migration phases

### D0 — Structure and authority

Status: active in issue #511.

Actions:

- create six canonical documents;
- create three runbooks;
- create migration/history/reference/archive indices;
- update `README.md` as the new router;
- preserve all existing files in place;
- do not delete, move, or rewrite legacy documents.

Acceptance:

- reading order and authority are explicit;
- current state has one canonical destination;
- autonomous and human tasks are separated;
- migration map exists;
- ordinary CI is green if triggered;
- PR is reviewed before merge.

### D1 — Normative consolidation

Actions:

- reconcile branch, documentation, architecture, maturity, plugin, Qt6, worker, scientific-claim, and AI-agent policies;
- consolidate current decisions into `GOVERNANCE.md` and `ARCHITECTURE.md`;
- preserve detailed option analyses under history/reference;
- replace superseded normative files with temporary redirect notices before deletion.

### D2 — Current-state consolidation

Actions:

- reconcile `current_plans.md`, test matrix, module inventory, and handoff;
- move volatile counts, blockers, PR state, and next work to `STATUS.md`;
- move executable tasks to autonomous backlog;
- move decisions to human backlog;
- preserve dated snapshots under history.

### D3 — Evidence and migration relocation

Actions:

- move dated evidence to `history/evidence/YYYY/MM/` or domain subdirectories;
- move completed integration/migration notes to `history/migrations/`;
- update links atomically;
- do not rewrite substantive evidence.

### D4 — Reference consolidation

Actions:

- create bounded topic references;
- consolidate build/test/package guidance;
- consolidate plugin guides;
- consolidate scientific-domain guidance;
- consolidate application/tool/model guidance;
- consolidate API coverage/inventory documents.

### D5 — Governance CI

Actions:

- enforce top-level allowlist after migration;
- validate links and front matter;
- ensure evidence location/naming;
- ensure unique backlog IDs and schemas;
- ensure governance/architecture contain no volatile workflow runs;
- ensure oldies retention gate is respected.

### D6 — Historical review and deletion

Actions:

- review every `oldies/` file;
- consolidate or reject unique content explicitly;
- create preservation tag/branch or record exact commits;
- delete only after 2026-11-01 and explicit maintainer approval;
- use a dedicated deletion PR.

## 7. Direct top-level file migration matrix

| Current file | Current role | Volatility | Planned destination/action |
|---|---|---|---|
| `README.md` | large index and basic rules | medium/high | replace with short canonical router in D0 |
| `current_plans.md` | mixed plans, logs, old state | high | split into `STATUS.md`, both backlogs, and history in D2 |
| `branch_workflow.md` | normative branch policy | low | consolidate into `GOVERNANCE.md` in D1 |
| `docker_packaging.md` | operational Docker guidance | medium | consolidate into `reference/BUILD_TEST_PACKAGING.md` in D4 |
| `oldies_inventory.md` | temporary historical inventory | high during migration | merge into one archive review tracker; retire in D6 |
| `consolidation_map.md` | temporary source-to-guide map | high during migration | superseded by this migration record and final archive tracker |
| `python_integration.md` | Python integration policy/reference | medium | consolidate into architecture plus topic reference |
| `kernel_development.md` | kernel modernization/ownership policy | low/medium | consolidate durable rules into `ARCHITECTURE.md`; details into reference |
| `plugins_development.md` | plugin policy/reference | low/medium | consolidate durable rules into architecture; details into `reference/PLUGINS.md` |
| `oldies_review_status.md` | temporary review state | high | merge into archive review tracker; retire in D6 |
| `documentation_governance.md` | normative documentation policy | low | consolidate into `GOVERNANCE.md` in D1 |
| `terminal_facade_command_coverage.md` | API/CLI coverage inventory | medium | move/consolidate into `reference/API_COVERAGE.md` |
| `workinprogress_wip20261_integration.md` | completed branch integration log | immutable | move to `history/migrations/` |
| `ExpressionBuilder_property_editor_plan.md` | future implementation task | medium | convert to bounded autonomous backlog task after revalidation |
| `whole_cell_and_sbml.md` | architecture, policy, plans | mixed | split between architecture, scientific reference, and human backlog |
| `tools_and_statistics.md` | architecture, policy, plans | mixed | split between architecture, references, and backlogs |
| `applications_development.md` | architecture, runbook, plans | mixed | split between architecture, application reference, and backlogs |
| `modal_and_hybrid_simulation.md` | durable domain policy and open design | low/medium | consolidate into `reference/SCIENTIFIC_DOMAINS.md`; decision to human backlog |
| `matrix_values_and_multidimensional_assignments_plan.md` | completed phase plus future phases | high/historical | archive completed snapshot; create bounded remaining tasks after revalidation |
| `models_and_modelspecific_generation.md` | stable runbook plus dated sweep | mixed | split runbook/reference from immutable evidence/history |
| `dcs_ode_pde_workinprogress_integration.md` | completed/partial integration log | historical | move to `history/migrations/`; revalidate remaining tasks |
| `build_ci_tests.md` | runbook plus volatile counts | mixed | commands to reference; current counts to `STATUS.md` |
| `genesys_2026_consolidation_plan.md` | original audit/plan snapshot | historical | move to `history/migrations/2026-consolidation/` |
| `genesys_2026_test_matrix.md` | current state and detailed validation | high | canonical summary to `STATUS.md`; immutable checkpoints to evidence |
| `genesys_2026_module_inventory.md` | dated audit/inventory | high | archive snapshot; later generate current reference where possible |
| `genesys_2026_consolidation_handoff.md` | current state/handoff | high | consolidate into `STATUS.md`; archive final snapshot |
| `genesys_2026_phase0_ci_evidence_20260720.md` | immutable evidence | none | move to `history/evidence/2026/07/` |
| `genesys_2026_runtime_lifecycle_evidence_20260721.md` | immutable evidence | none | move to `history/evidence/2026/07/runtime/` |
| `genesys_2026_ownership_evidence_20260721.md` | immutable evidence | none | move to `history/evidence/2026/07/ownership/` |
| `genesys_plugin_target_overlap_inventory_20260721.md` | immutable architecture evidence | none | move to `history/evidence/plugins/` |
| `genesys_plugin_target_introspection_evidence_20260721.md` | immutable generated evidence | none | move to `history/evidence/plugins/` |
| `genesys_plugin_target_link_evidence_20260722.md` | immutable link/symbol evidence | none | move to `history/evidence/plugins/` |
| `genesys_shell_validation_evidence_20260722.md` | immutable application evidence | none | move to `history/evidence/applications/` |
| `genesys_worker_validation_evidence_20260722.md` | immutable application/security evidence | none | move to `history/evidence/applications/` |
| `genesys_dataanalyser_gui_validation_evidence_20260722.md` | immutable GUI startup evidence | none | move to `history/evidence/applications/` |
| `genesys_optimizer_gui_validation_evidence_20260722.md` | immutable GUI startup evidence | none | move to `history/evidence/applications/` |
| future AI Assistant GUI evidence | immutable GUI startup evidence | none | create directly under `history/evidence/applications/`, not top level |
| `genesys_2026_human_decisions.md` | decisions plus option analysis | low | current decisions to governance/human backlog; analysis to history/reference |
| `genesys_2026_decisions_addendum_20260720.md` | later authoritative decisions | low | consolidate into governance/architecture in D1 |
| `genesys_numerical_statistical_references_plan.md` | human-input scientific plan | low/medium | human backlog plus scientific reference structure |
| `genesys_multiobjective_optimizer_future_plan.md` | human-input research/implementation plan | low/medium | human backlog plus optimization research reference |
| `genesys_ai_virtual_cell_research_direction.md` | strategic architecture/research direction | low | durable direction in architecture; work packages in human backlog/reference |

## 8. Plugin guide migration matrix

| Current file | Planned destination |
|---|---|
| `plugins/README.md` | retire after new reference routing is complete |
| `plugins/electronic.md` | `reference/PLUGINS.md` |
| `plugins/external_integration.md` | `reference/PLUGINS.md` and integration sections |
| `plugins/other_plugins.md` | `reference/PLUGINS.md` |
| `plugins/modal_model.md` | `reference/SCIENTIFIC_DOMAINS.md` |
| `plugins/continuous_hybrid.md` | `reference/SCIENTIFIC_DOMAINS.md` |
| `plugins/biochemical.md` | `reference/SCIENTIFIC_DOMAINS.md` |
| `plugins/whole_cell_model.md` | `reference/SCIENTIFIC_DOMAINS.md` |

The source files must be compared individually before consolidation. This matrix is a destination plan, not proof that all source claims remain current.

## 9. `oldies/` category migration matrix

| Historical category | Files | Planned handling |
|---|---|---|
| repository/wiki/governance | old wiki, issue relay, post-refactor report | extract valid policy/history; otherwise reject explicitly |
| build/CI/terminal | kernel-test roadmap, terminal strategy | consolidate only still-valid commands/constraints |
| kernel/C++/memory | modernization audit, leak review, ModelSimulation review, simulator inventory | preserve unique ownership findings after current-code revalidation |
| plugins/data definitions/GUI plugins | component matrix, data-definition audit, graphical plugin audit/configuration, attached-data discussion | consolidate current contracts; archive obsolete paths |
| tools/statistics/optimizer | Data Analyzer plans, analysis-study design, optimizer roadmap | extract stable domain concepts and bounded tasks |
| modal/hybrid | modal/EFSM/Petri plan, temporal synchronization analysis | consolidate semantics and unresolved decision boundaries |
| SBML/TinkerCell/whole-cell | SBML scope, TinkerCell context, whole-cell project, WCM implementation plan | consolidate scientific context and human backlog inputs |
| web/Python/external integration | web roadmap, Python facade coverage | consolidate current worker/integration contracts |

No historical file is deletion-ready merely because a canonical document now exists.

## 10. Redirect and deletion strategy

A legacy active document should pass through these states:

1. `active-legacy` — still referenced during D0;
2. `consolidated-with-redirect` — content reconciled and file reduced to a migration notice/link;
3. `deletion-candidate` — no active link depends on it and Git history is sufficient;
4. `deleted` — removed in a dedicated reviewed PR.

Dated immutable evidence may be moved directly with link updates because it is not rewritten.

`oldies/` follows its separate retention gate and cannot skip review.

## 11. Front matter convention

Canonical and new supporting documents use YAML front matter with fields appropriate to their type.

Common fields:

```yaml
---
document_type: governance|architecture|status|backlog|runbook|reference|evidence|migration-record
authority: normative|normative-reference|current-state|operational|informative
owner: project-maintainer
status: active|draft|historical|superseded
last_reviewed: YYYY-MM-DD
last_updated: YYYY-MM-DD
tracks: ISSUE_NUMBER
---
```

Evidence additionally records commit, PR, run, artifact, environment, and immutable status when available.

## 12. Validation plan

### D0 manual/PR validation

- compare branch to `WorkInProgress`;
- confirm only documentation files changed;
- confirm all pre-existing files remain;
- verify relative links among canonical documents/runbooks;
- inspect Markdown headings/tables/front matter;
- run ordinary CI if triggered.

### Post-D4 automated validation

Add a bounded documentation-governance workflow that verifies:

- top-level allowlist;
- internal links;
- front matter and allowed document types;
- dated evidence location;
- absence of volatile run IDs in governance/architecture;
- unique backlog IDs;
- required task/decision fields;
- `STATUS.md` recency;
- no premature `oldies/` deletion.

## 13. Rollback

D0 is additive except for the `README.md` routing update. Rollback consists of reverting the D0 merge commit(s); no source or legacy documentation content is deleted.

Later phases must preserve moves as Git renames where practical and remain independently revertible.

## 14. Review gate before effective consolidation

The maintainer should review:

- canonical document authority and wording;
- branch naming convention;
- autonomous stop gates;
- human backlog classification;
- current `STATUS.md` accuracy;
- destination matrix;
- whether reference files should remain five documents or be split further.

Only after approval should D1–D4 move or retire existing files.

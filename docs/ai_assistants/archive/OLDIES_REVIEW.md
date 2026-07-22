---
document_type: archive-review-tracker
authority: informative
owner: project-maintainer
last_reviewed: 2026-07-22
status: active-retention
retention_gate: 2026-11-01
tracks: 511
---

# GenESyS Historical `oldies/` Review Tracker

## 1. Purpose

This is the only active tracker for the 25 historical Markdown files retained under `docs/ai_assistants/oldies/`.

It replaces:

- `oldies_inventory.md`;
- `oldies_review_status.md`;
- `consolidation_map.md`.

The files are retained for audit and possible future extraction. They are not current policy, architecture, status, backlog, or executable instructions.

## 2. Current classification

All 25 files are currently classified as:

```text
retained-review-pending
```

This means:

- the topic has a current canonical destination;
- useful themes have been represented in the new structure;
- the historical source has not yet received the detailed file-by-file review required for deletion;
- no file is deletion-ready;
- claims must be revalidated against current code before reuse.

## 3. Status vocabulary

- `retained-review-pending` — canonical topic destination exists; detailed source review still required;
- `unique-content-to-consolidate` — detailed review found useful material not yet represented canonically;
- `consolidated-reviewed` — detailed review completed and useful material represented or intentionally summarized;
- `obsolete-reviewed` — detailed review completed and content is obsolete, but retention gate/approval still applies;
- `discard-after-gate` — detailed review completed, deletion approved in principle, but date/maintainer/dedicated-PR gates remain;
- `retain-permanently` — explicit historical value justifies permanent archive retention.

## 4. Retained file matrix

| # | Historical file | Topic | Canonical destination | Current classification | Deletion-ready |
|---:|---|---|---|---|---|
| 1 | `old_genesys_wiki_consolidated.md` | repository overview and historical wiki | `../ARCHITECTURE.md`, `../reference/` | `retained-review-pending` | no |
| 2 | `old_report-issue-relay-contract.md` | issue-report relay and credential security | `../GOVERNANCE.md`, `../reference/API_INTEGRATIONS.md` | `retained-review-pending` | no |
| 3 | `old_post-refactor-validation-report.md` | historical build/test/refactor evidence | `../history/evidence/`, `../reference/BUILD_TEST_PACKAGING.md` | `retained-review-pending` | no |
| 4 | `old_kernel-tests-build-roadmap-2026-1.md` | kernel tests/build roadmap | `../reference/BUILD_TEST_PACKAGING.md`, `../BACKLOG_AUTONOMOUS.md` | `retained-review-pending` | no |
| 5 | `old_terminal-build-strategy-2026-04-01.md` | shell/terminal build strategy | `../reference/BUILD_TEST_PACKAGING.md`, `../reference/APPLICATIONS_TOOLS_MODELS.md` | `retained-review-pending` | no |
| 6 | `old_kernel-cpp23-modernization-audit.md` | C++23/kernel modernization | `../reference/KERNEL_PARSER_OWNERSHIP.md` | `retained-review-pending` | no |
| 7 | `old_kernel-memory-leak-review-2026-03-30.md` | memory/lifetime audit | `../reference/KERNEL_PARSER_OWNERSHIP.md`, `../history/evidence/` | `retained-review-pending` | no |
| 8 | `old_modelsimulation-cpp23-review.md` | ModelSimulation review | `../reference/KERNEL_PARSER_OWNERSHIP.md` | `retained-review-pending` | no |
| 9 | `old_phase2-kernel-simulator-inventory.md` | kernel/simulator inventory | `../ARCHITECTURE.md`, `../STATUS.md` | `retained-review-pending` | no |
| 10 | `old_plugin_components_method_matrix.md` | component method/API matrix | `../reference/PLUGINS.md`, `../reference/API_INTEGRATIONS.md` | `retained-review-pending` | no |
| 11 | `old_plugin_data_definitions_audit_WiP20261.md` | data-definition audit | `../reference/PLUGINS.md`, `../reference/KERNEL_PARSER_OWNERSHIP.md` | `retained-review-pending` | no |
| 12 | `old_GUI_GRAPHICAL_PLUGIN_AUDIT.md` | graphical plugin coupling | `../reference/PLUGINS.md`, `../reference/APPLICATIONS_TOOLS_MODELS.md` | `retained-review-pending` | no |
| 13 | `old_GUI_GRAPHICAL_PLUGIN_CONFIGURATION.md` | graphical plugin configuration | `../reference/PLUGINS.md`, `../reference/APPLICATIONS_TOOLS_MODELS.md` | `retained-review-pending` | no |
| 14 | `old_SBML_INTEROPERABILITY_SCOPE.md` | SBML scope/interoperability | `../reference/SCIENTIFIC_DOMAINS.md` | `retained-review-pending` | no |
| 15 | `old_TINKERCELL_context.md` | TinkerCell context | `../reference/SCIENTIFIC_DOMAINS.md` | `retained-review-pending` | no |
| 16 | `old_whole_cell_biosimulator_project.md` | whole-cell project concept | `../reference/SCIENTIFIC_DOMAINS.md`, `../BACKLOG_HUMAN.md` | `retained-review-pending` | no |
| 17 | `old_WCM_IMPLEMENTATION_PLAN.md` | whole-cell implementation plan | `../reference/SCIENTIFIC_DOMAINS.md`, `../BACKLOG_HUMAN.md` | `retained-review-pending` | no |
| 18 | `old_tools-data-analyzer-plan-2026-04-16.md` | Data Analyser roadmap | `../reference/APPLICATIONS_TOOLS_MODELS.md`, `../reference/SCIENTIFIC_DOMAINS.md` | `retained-review-pending` | no |
| 19 | `old_data-analyzer-analysis-study-design-2026-04-16.md` | analysis-study design | `../reference/APPLICATIONS_TOOLS_MODELS.md`, `../reference/SCIENTIFIC_DOMAINS.md` | `retained-review-pending` | no |
| 20 | `old_optimizer-workstation-roadmap-2026-04-16.md` | optimizer workstation roadmap | `../reference/APPLICATIONS_TOOLS_MODELS.md`, `../BACKLOG_HUMAN.md` | `retained-review-pending` | no |
| 21 | `old_modal_model_efsm_petrinet_plan.md` | modal/EFSM/Petri-net design | `../reference/SCIENTIFIC_DOMAINS.md`, `../reference/PLUGINS.md` | `retained-review-pending` | no |
| 22 | `old_temporal-sync-analysis-discrete-continuous-2026-06-03.md` | discrete/continuous time synchronization | `../reference/SCIENTIFIC_DOMAINS.md` | `retained-review-pending` | no |
| 23 | `old_web-application-roadmap-2026-03-30.md` | web/worker application roadmap | `../reference/APPLICATIONS_TOOLS_MODELS.md`, `../reference/API_INTEGRATIONS.md` | `retained-review-pending` | no |
| 24 | `old_PythonForG_SimulatorFacade_coverage.md` | Python/facade exposure | `../reference/API_INTEGRATIONS.md` | `retained-review-pending` | no |
| 25 | `old_Discussion_about_internal_attached_and_custom_attached_datadefinitions.md` | internal/attached data-definition ownership | `../reference/PLUGINS.md`, `../reference/KERNEL_PARSER_OWNERSHIP.md` | `retained-review-pending` | no |

## 5. Detailed review fields

When one file is reviewed, update its row/classification and record in a review note or PR:

- source file;
- reviewer/date;
- historical date/branch/commit if known;
- claims checked against current code;
- unique material extracted;
- stale/rejected statements;
- canonical files updated;
- resulting status;
- deletion recommendation;
- links to PR/issue/evidence.

Do not change a row to `consolidated-reviewed`, `obsolete-reviewed`, or `discard-after-gate` from filename/theme inspection alone.

## 6. Deletion gate

No retained historical file may be deleted until all of the following are true:

1. the file received a detailed review;
2. useful material was consolidated or explicitly rejected;
3. active documentation does not depend on it for current guidance;
4. the date is after **2026-11-01**;
5. the maintainer explicitly approves deletion;
6. deletion occurs in a dedicated PR;
7. Git-history provenance or an archival tag/branch is recorded;
8. documentation-governance CI remains green.

Bulk deletion based only on age, filename, or existence of a canonical reference is prohibited.

## 7. Structural completion versus content review

The AI-assistant documentation structure is allowed to be complete while these 25 files remain retained. Structural completion means:

- this is the only active oldies tracker;
- no current policy/status/backlog depends on oldies;
- all files are listed and classified;
- the retention/deletion gate is enforced;
- detailed historical review remains a separate future maintenance program.

## 8. Source tracker preservation

The three superseded trackers remain recoverable from Git history under their pre-consolidation blob SHAs:

- `oldies_inventory.md`: `cb5d81c9cd41da79c443c352f62f0d5898cca1d8`;
- `oldies_review_status.md`: `591a58dd0b57c6982ce9bfd0aa06ecf2cb091e81`;
- `consolidation_map.md`: `201877a22c64d6aafffc4fb2355169b4f39cec81`.

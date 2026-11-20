---
document_type: validation-evidence
authority: informative
owner: project-maintainer
last_updated: 2026-11-07
status: active
scope: november-2026-pr-560-pr-562-consolidation
---

# November 2026 Consolidation — Report Issue and Standalone Tools

## Scope and integration state

Repository: `rlcancian/Genesys-Simulator`
Integration branch: `WorkInProgress`
Initial HEAD inspected for this consolidation: `82cb4bdb28d698daea14dc01914797cb59f6d1ed`
Final integration HEAD after PR #562: `46bdedfc94ac9ae6144675fe6b2a64c0c5e98460`
Working tree at inspection start: clean; local branch `WorkInProgress` matched `origin/WorkInProgress`.

| Development | PR | Merge commit | Tests / CI | Documentation | State | Limitations |
|---|---|---|---|---|---|---|
| Report Issue | [#560](https://github.com/rlcancian/Genesys-Simulator/pull/560) | `82cb4bdb28d698daea14dc01914797cb59f6d1ed` | 21 QtTests, GUI build, unit/kernel/smoke, package, manual and required PR checks passed on final PR candidate. | User/Developer docs and 398-page manual present in final combined artifact. | **PASS — merged** | No live external relay-to-GitHub test was run; no test issue was created. |
| Standalone Tools | [#562](https://github.com/rlcancian/Genesys-Simulator/pull/562) | `46bdedfc94ac9ae6144675fe6b2a64c0c5e98460` | Final candidate checks all passed, including 2,004 unit, 2,004 kernel, 3 smoke, 6 launcher CTests, real menu launch, Debian package/lifecycle, manual and GUI startup checks. Post-merge Debian package and lifecycle workflow also passed on the exact WIP HEAD. | Architecture/package docs and combined 398-page PDF validated. | **PASS — merged** | Portable runtime was checked by launcher/path and source-contract tests; full portable archive lifecycle was not repeated in this run. |

Prior November integrations remain present in `WorkInProgress`: PRs #559 (save preparation, Decide, source editor, Remove/PickUp and palette), #563 (Octave source editor), #565 (Decide undo/redo), #564 (Transfer/LabelDefinition/Label/Goto), and #567 (Transfer evidence). Their original merge SHAs and scope are recorded in [`VALIDATION_LEDGER.md`](VALIDATION_LEDGER.md) and [`transfer_label_goto_remote_progress.md`](transfer_label_goto_remote_progress.md).

## PR #560 — Report Issue

The final PR candidate was `ed00d14cbcb5c0c99aa5903ddad98a01c557c9b0`; its merge advanced `WorkInProgress` to `82cb4bdb28d698daea14dc01914797cb59f6d1ed`.

- Hosted QtTest target: 21 passed, 0 failed; GUI target compiled — run [42300873480](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42300873480).
- The latest PR checks also passed unit (2,004 passed; 4 disabled), kernel (2,004 passed; 4 disabled), smoke (3/3), Debian package/lifecycle and manual PDF validation — checks on [run 42300873598](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42300873598), [run 42300873500](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42300873500), [run 42300873475](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42300873475), and [run 42300873559](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42300873559).
- Browser fallback builds a GitHub new-issue URL and is explicitly a handoff: opening a browser does not claim that an issue was created. Text too long for a safe URL is transferred through the clipboard with user notice.
- Optional configured relay is asynchronous, HTTPS-only, bounded by timeout and response validation. No live relay endpoint was available for end-to-end verification; no real issue was submitted.
- No model content, SimulLang source, logs, screenshots, file paths, or contact information is collected automatically.

## PR #562 — Standalone Tools

The final feature head was `fc6edbe87a60a7c7affe3091124ba4c153bba8fd`. It reconciled the then-current WIP base using merge commit `5d9934b4db429fb8aa5948b329e915f13c5b81ef` and was merged at `46bdedfc94ac9ae6144675fe6b2a64c0c5e98460`.

### Final-head automated results

- `tests-unit`: 2,004 passed, 0 failed, 4 disabled; [run 42315612700](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42315612700).
- `tests-kernel-unit`: 2,004 passed, 0 failed, 4 disabled; `tests-smoke`: 3/3; [run 42315612646](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42315612646).
- Issue-reporting QtTests: 21/21 and main GUI build; [run 42315612769](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42315612769).
- Standalone menu integration built and installed the main GUI and three tools in a temporary prefix containing spaces, then launched the main GUI from an unrelated working directory under Xvfb/Openbox. The real Tools actions opened `genesys-dataanalyser-gui`, `genesys-optimizer-gui`, and `genesys-ai-assistant-gui` as separate PIDs with visible windows. Each tool was closed while the main editor stayed alive; cleanup left no tested child process. [Run 42315612606](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42315612606), artifact `13550498798` (`standalone-tools-integration`).
- The six `ToolLauncher` CTests passed: Debian-like prefix lookup with spaces, colocated portable lookup, explicit development candidates, missing/non-executable tools, installed-runtime fallback rejection, and arbitrary executable/path-traversal rejection.
- Debian package build and lifecycle passed: [run 42315612665](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42315612665), lifecycle job `128232515878`. Artifact `13550938569` contains the five-package inventory. `genesys-gui` contains all three executable files and their manpages. AppStream validation succeeded with one informational `developer-info-missing` item; Lintian output was empty.
- The manual workflow initially reached its 55-minute timeout during dependency installation and never began LaTeX. The failed job was rerun on the same exact candidate and passed in 2m06s: [run 42315612648](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42315612648), artifact `13553076235`.
- Fresh manual PDF: 398 pages, 8,091,689 bytes, SHA-256 `d1606b21a1b8d75ac28281fa0a68ff33e9d1c5061afba585c82d4e2481df081d`. The PDF SHA matches the committed canonical PDF. Standalone Tools, Report Issue, GUI Issue Reporting Boundary, Transfer endpoints, and Transfer plugin sections were present. Pages 120, 190, 323–324 and 372 were visually inspected.
- Every required PR check on the final feature head passed before merge. PR #562 was marked ready and merged with a merge commit, not squash/rebase.

## Combined regressions and older integrations

The final PR #562 validation tree contained the PR #560 merge and all preceding November integration commits. Therefore its full ordinary and kernel suites executed the regression registrations for model save, Decide, source properties, plugin identity/palette, and Transfer. Those current candidate suites passed. This does not replace focused GUI interaction or sanitizer claims recorded in the original bounded evidence.

The standalone Tools checks do not validate optimizer mathematics, AI-provider service correctness, model transfer between processes, or an external issue relay. These were outside this consolidation.

## Transfer interaction and sanitizer boundary

- The integrated unit/kernel suites passed on the PR #562 candidate, covering the registered Transfer/LabelDefinition/Label/Goto tests. Earlier focused ASan/UBSan Transfer evidence is recorded in [`transfer_label_goto_remote_progress.md`](transfer_label_goto_remote_progress.md), with LSan disabled.
- A complete desktop drag/drop and Property Editor session for creating Label/Goto, editing their association and delay properties, saving/reopening, and simulating was not performed in this consolidation. That GUI criterion remains **UNKNOWN**; automated `.gen`/`.gui` Transfer persistence and execution tests are historical, narrower evidence.
- The historical LeakSanitizer report (~79,400 bytes in 2,611 allocations) remains unresolved as to PluginManager/Buffer lifetime versus test-fixture ownership. It is not a PASS and is not attributed to the Transfer change without a reproducer. No broad PluginManager rewrite was attempted.

## Post-merge check

The post-merge `WorkInProgress` Debian package workflow [42321785787](https://github.com/rlcancian/Genesys-Simulator/actions/runs/42321785787) passed on exact HEAD `46bdedfc94ac9ae6144675fe6b2a64c0c5e98460`. Package build, package inventory, AppStream, Lintian and install/reinstall/remove/purge lifecycle all passed. Artifacts: package inventory `13553434912`, diagnostics `13553604427`, lifecycle evidence `13554360621`.

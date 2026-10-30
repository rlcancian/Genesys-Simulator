---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: merged-and-validated
tracks: HUM-MODAL-002,AUTO-MODAL-003,AUTO-MODAL-004
---

# EFSM editor post-merge integration closeout

## Checkpoint

- Repository: `rlcancian/Genesys-Simulator`
- Integration base: `WorkInProgress` at `17338f8e566f87fc3b06eb422a5dd354e6d57b02`
- Follow-up branch: `WiP20261030/efsm-editor-e2e-closeout`; commit `21cb60d606f48cd72d1e4739caab1945cb83f6c0`
- Follow-up PR #552 merged into `WorkInProgress` at `51851415fb8abe33f08a67d1d571ad233ab081a5`.
- PR #551 merged on 2026-10-30; merged source head: `a9d718b5b085c47a5a9aeee4b41a25a98716b7d7`.
- This follow-up adds a production-path regression and reconciles status records. It does not add editor behavior or modify kernel production code.

## Regression added

`EFSMNetworkEditor.MainWindowSaveCloseAndReopenRestoresAndExecutesEFSM` drives the MainWindow workflow to create and edit an EFSM, saves the `.gui` model, closes and reopens it, and checks restored layout, topology, adapter association, and ordinary process connections. It then invokes the restored `EFSMNetwork::activate()` kernel API and verifies output value `91.0` and current state `PersistedDone`. The guard and output expressions are numeric expressions supported by this runtime path.

This joins the actual GUI edit/save/reload path to the existing kernel activation API in one regression. It does not establish a separate interactive mouse/keyboard session or scientific validity beyond the tested activation case.

## Local validation executed on the follow-up branch

- Focused EFSM GUI suite: **37/37 passed**, including the new MainWindow save/reopen/activation regression.
- Focused `EFSMNetwork` and `ModalModelDefaultNetworkTest` selection: **26/26 passed**.
- `tests-unit`: **1,963 passed, 4 disabled, 0 failed** (1,967 registered).
- `tests-kernel-unit`: **1,963 passed, 4 disabled, 0 failed** (1,967 registered).
- `tests-smoke`: **3/3 passed**.
- `gui-app`: configure and build passed.
- `python3 scripts/validate-ai-docs.py`: passed; 6 canonical top-level Markdown files and 56 active Markdown files checked, plus 25 retained `oldies` files.
- Hosted CI for PR #552: all eight reported checks passed — unit, kernel-unit, smoke, GUI GMDD diagnostics, documentation governance, launcher, Debian package build and Debian package lifecycle.

Commands used for the CTest baselines included `LC_ALL=C LANG=C QT_QPA_PLATFORM=offscreen` for unit and kernel tests. The build presets were configured and built incrementally with parallelism 4. No sanitizer was rerun because this follow-up changes only a test scenario and does not alter ownership or lifetime code; the preceding PR #551 targeted ASan/UBSan evidence remains scoped to its exact tested paths, with LSan disabled.

## Documentation reconciliation

PR #551 and its test/documentation closeout PR #552 are merged; the eight approved milestones are recorded as accepted (8/8 = 100%). The dated ledger, current status, backlog, implementation plan and AI changelog are linked to this closeout. The manuals and `docs/ManualGenESyS.pdf` were not changed by this test-only follow-up; their last regenerated and inspected state is documented in the PR #551 evidence.

## Limits

- No outstanding PR #552 checks; all reported checks passed on its final head.
- No separate manual EFSM mouse/keyboard session is claimed.
- LeakSanitizer was disabled in the earlier targeted sanitizer runs; those results are not repository-wide leak proof.
- No kernel production source or mathematical execution semantics were changed.

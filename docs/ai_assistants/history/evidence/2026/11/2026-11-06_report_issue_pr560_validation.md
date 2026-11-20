---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-11-06
status: local-validation-complete-hosted-checks-pending
tracks: AUTO-GUI-001,PR-560
---

# PR #560 local audit and validation

## Checkpoint

- Repository: `rlcancian/Genesys-Simulator`.
- Integration base: `origin/WorkInProgress` at `bfbbc7b45e69926e88ba5a9cf708874d07c4b948`.
- Feature branch/worktree: `WiP202611/report-issue-gui` at `d1fcb4f1ac09f2b4ae1469d54e23096818e74ff5`, containing the published PR history plus the current WorkInProgress merge.
- PR: [#560](https://github.com/rlcancian/Genesys-Simulator/pull/560), open and draft; local validation is complete, final-head hosted checks remain pending.
- Environment: Ubuntu 24.04, GCC 13.3.0, CMake 3.28.3, Ninja 1.11.1, Qt 6.4.2.

## Review findings and corrections

- The dialog rejected both Cancel and window close while a relay request was pending, and the caller could leave the reply alive if the callback receiver was destroyed. Cancellation now aborts the active reply; receiver destruction aborts and schedules reply cleanup.
- Relay requests now reject query-bearing endpoint URLs and text payloads above the explicit limit. Relay response data is drained incrementally and capped at 64 KiB; oversized responses are rejected.
- The About-menu tooltip described only a configured relay even though browser handoff is the normal fallback. Its text now describes issue reporting without implying a particular transport.
- A live GUI screenshot exposed overlap between the required-fields message and description editor. The dialog's initial/minimum height was increased, with a geometry regression test.
- The user documentation now records the existing 256-character title limit.
- The manual workflow installs `texlive-plain-generic` for `ulem.sty` and removes both PDF outputs before rebuilding so a stale tracked PDF cannot mask a failed build.

## Local validation completed

- Focused `genesys_gui_issue_tests`: 21/21 passed, including final clipboard, close/cancel, and dialog-geometry cases.
- `gui-app`: configure and complete GUI build passed; incremental GUI relink passed after the dialog sizing correction.
- Xvfb interaction: launched the GUI, opened About → Report Issue, confirmed the browser-fallback text, Bug default, fields, required-fields message and layout. The screenshot is [`report-issue-dialog-2026-11-06.png`](report-issue-dialog-2026-11-06.png). Xvfb has no window manager; the interaction used coordinates because the menu/dialog lacked a stable external object API. No real browser or relay submission was made.
- `tests-unit`: build passed; CTest executed 1,987 tests successfully, 0 failed, with 4 disabled (1,991 registered).
- `tests-kernel-unit`: build passed; CTest executed 1,987 tests successfully, 0 failed, with 4 disabled (1,991 registered).
- `tests-smoke`: configure/build passed; CTest passed 3/3.
- Manual: `bash docs/developers/ManualGenESyS_subrepo/make.sh` exited 0 and regenerated `docs/ManualGenESyS.pdf` from current LaTeX sources. The PDF has 394 pages; the issue-reporting sections appear in extracted text on pages 319 and 368. The XeLaTeX log has no fatal errors or undefined citation/reference diagnostics. `latexmk` records Biber. Visual inspection covered the changed user/developer sections. Nonfatal `xdvipdfmx` link-annotation boundary warnings occur on existing Appendix C page 381, outside the changed sections.
- `python3 scripts/validate-ai-docs.py`: passed (6 canonical top-level Markdown files, 60 active Markdown files, 25 retained `oldies` files).
- Focused ASan/UBSan build and QtTest passed (21/21); LeakSanitizer was disabled because the process creates Qt GUI singletons, so no leak-free claim is made. Hosted CI for the final PR head is pending.

## Security and integration boundaries

- Test transports use a fake `QNetworkAccessManager`. No live relay endpoint was available or authorized, and no production GitHub issue was created; relay-to-GitHub E2E remains unverified.
- The normal browser workflow opens the official issue form and explicitly tells users that they must submit it on GitHub. User-entered text may be present in browser history; the dialog discloses this.
- The final merge decision is pending the required tests and final-head hosted checks.

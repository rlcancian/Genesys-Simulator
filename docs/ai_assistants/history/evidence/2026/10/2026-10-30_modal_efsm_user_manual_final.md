---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: local-build-verified
tracks: HUM-MODAL-002,AUTO-MODAL-004
---

# EFSM User Manual final integration wording

## Checkpoint

- Repository: `rlcancian/Genesys-Simulator`
- Base: `WorkInProgress` at `1d31cf7e6021c246f55713144b1014c5b841ec6c`
- Follow-up PR: #555, documentation-only, targeting `WorkInProgress`.

## Change

The user guide no longer says the EFSM editor is available only on the deleted
feature branch. It now describes the editor as integrated in the GenESyS Qt6
GUI and retains the already documented feature limits. `docs/users/README.md`
now calls this the integrated, tested menu workflow.

## Build and inspection

- Doxygen 1.9.8: `doxygen docs/users/DoxyfileUser` exited 0.
- Doxygen emitted 22,529 warning lines, principally unresolved references in
  existing source comments; a search found none attributed to the changed
  `docs/users` files.
- `latexmk -pdf -interaction=nonstopmode -halt-on-error refman.tex` exited 0.
- `docs/users/GenESyS-User-Documentation.pdf`: 3,981 pages, 22,723,475 bytes.
- Extracted text and rendered page 177 were inspected. The guide begins on
  page 177 and shows the integrated GUI wording and current workflow.
- The LaTeX log reports 14 multiply-defined labels from the broader generated
  API reference; it reports no compile failure. This limited wording change
  did not attempt to repair unrelated Doxygen label collisions.

## Boundary

No GUI source, kernel behavior, or model semantics changed. No claim is made
that this manual update independently validates every editor operation; those
are covered by the MainWindow, kernel and full regression evidence linked in
`2026-10-30_modal_efsm_postmerge_closeout.md`.

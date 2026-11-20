---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-30
status: validated-awaiting-integration
tracks: HUM-MODAL-002,AUTO-MODAL-004
---

# EFSM editor final milestone validation

## Checkpoint

- Repository: `rlcancian/Genesys-Simulator`
- Branch: `WiP20261029/modalmodel-architecture`
- Validated code SHA: `2a3ca292414b121ea4e19cd98768cec86b66e0b6`
- PR: [#551](https://github.com/rlcancian/Genesys-Simulator/pull/551), targeting `WorkInProgress`
- The 2026-10-30 maintainer mandate authorizes completion of G1.1, G1.2, G2,
  G3/G4 and G6, followed by integration when validation and governance gates pass.

## Delivered behavior

- Read-only EFSM projection uses the existing `ModelGraphicsScene` and tab
  infrastructure. State and transition items project the mathematical Network;
  they do not create process `Connection`s.
- A `ModalModelDefault` can open the associated Network. Shared adapters reuse
  the same Network context; create/select/replace operations use the existing
  model and plugin mechanisms.
- GUI actions create, move, edit and delete states/transitions, set initial and
  final state flags, edit supported properties/bindings, and use undo/redo
  commands that respect Network ownership.
- `.gen` persists the mathematical model, including explicitly empty transition
  names; legacy records with an omitted name retain the historical load path.
  `.gui` persists the versioned presentation overlay. Round-trip integration
  tests verify model identity, topology, properties, layout, and unchanged
  process connections.
- No separate manual mouse/keyboard session is claimed. The integrated
  MainWindow workflow is automated with Qt offscreen dialogs/actions. A bounded
  direct GUI startup was inspected under Xvfb as recorded in the companion
  MainWindow evidence.

## Local validation

- Focused EFSM Qt suites: **37/37 passed**.
- `tests-unit`: **1,963 passed, 4 disabled, 0 failed** (1,967 registered).
- `tests-kernel-unit`: **1,963 passed, 4 disabled, 0 failed**.
- `tests-smoke`: **3/3 passed**.
- `gui-app`: CMake configure and build passed.
- Targeted ASan/UBSan exercised state/transition deletion, mixed undo/redo and
  save/reopen paths; LeakSanitizer was disabled, so this is not global leak
  validation.
- `python3 scripts/validate-ai-docs.py` passed before this final evidence
  amendment; the documentation commit is revalidated separately.
- Developer Manual: figure validation and official `make.sh` passed; PDF has
  390 pages and the updated navigation section was checked on page 245.
- User Manual: Doxygen/LaTeX build passed; PDF has 3,981 pages and the EFSM
  guide was inspected on page 177. Existing duplicate-label warnings were
  recorded; compilation succeeded.

## Hosted validation for SHA `2a3ca292414b121ea4e19cd98768cec86b66e0b6`

All reported checks passed: unit, kernel-unit, smoke, AI documentation
governance, launcher, worker, shell, three GUI application startup jobs, GUI
GMDD diagnostics, plugin target/link evidence, Debian package build and Debian
package lifecycle. The Debian build took 17m52s; package verification,
AppStream, Lintian and lifecycle completed successfully. No failing or
cancelled check remained when inspected.

## Limits and state

- Expression edit dialogs persist entered values but do not perform parse-only
  validation because the existing expression parser can evaluate expressions.
- ASan/UBSan runs are bounded to the named GUI ownership paths; LSan was off.
- The plan remains eight milestones. The first three are the previously
  accepted baseline; the five remaining milestones are implemented and
  validated under the maintainer mandate, with final PR integration pending.
- At evidence creation, PR #551 remains open and draft. No merge has occurred.

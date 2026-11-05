---
document_type: executed-evidence
authority: informative
owner: project-maintainer
date: 2026-11-05
status: in-progress
scope: Transfer category and LabelDefinition/Label/Goto implementation
---

# Transfer / LabelDefinition / Label / Goto — remote progress

- Repository: `rlcancian/Genesys-Simulator`.
- Integration branch: `WorkInProgress`; base SHA: `3ae85774949ebc87e521a2409eecf8f086bcc43b`.
- Development branch: `WiP202611/transfer-label-goto`.
- Mandate: implement E1–E9 sequentially, produce one Draft PR, **never merge** during remote phase.
- Evidence status: source inspection only so far; no build or test has yet run remotely or locally.

## E1 — Components directory and category

- State: `IMPLEMENTED`, pending compilation/GUI verification.
- Change: moved 12 component classes / 24 .h/.cpp files from `components/MaterialHandling` to `components/Transfer`; preserved C++ class names and behavior.
- Dependencies: 13 source consumers updated, including DummyPlugin connector, Smart/teaching/arenaExamples applications and GUI category mapping.
- Commit SHA: `187725628cd919f0ac47232f858b1d31ad1dd949`.
- Check: static source comparison and indexed reference inventory; tests not executed (`UNKNOWN`).
- Risk: GitHub index may omit references; CI must verify linker/includes.

## E2 — DataDefinitions MaterialHandling → Transfer

- State: `IMPLEMENTED`, static inspection only; compilation `UNKNOWN`.
- Change: seven DataDefinitions / 14 .h/.cpp files moved to `data/Transfer`; their categories changed to `Transfer`.
- Consumers: 13 source files updated (components, Smart examples, connector, teaching, Arena examples).
- Commit SHA: `234b36c8d40ad6a4d37004ffa607b2b330836a91`.
- Behavior and typenames: intended unchanged; tests not yet executed.

## E3 — Rename old data Label to LabelDefinition

- State: `IMPLEMENTED`; compile/test: `UNKNOWN`.
- Moved `data/Logic/Label.{h,cpp}` to `data/Transfer/LabelDefinition.{h,cpp}`; classname, typename, metadata, factories and registry changed.
- Consumers: `Route.{h,cpp}`, `Sequence.{h,cpp}`, `PluginConnectorDummyImpl1.cpp`; static identifier `labeldefinition.so`.
- Receiver pointer remains non-owning, can be set once per live LabelDefinition and is checked against the model's component registry by identity (without dereferencing expired addresses).
- Receiver name is no longer eagerly resolved while DataDefinitions load. New Label component will establish the association after its own load/check.
- The old shadowing label field and duplicate editor control were eliminated; inherited ModelDataDefinition Label text remains available.
- Commit SHA: `ad81f471e1c09a429867d99f55b22a85dedfe039`.
- Tests not yet executed. Outstanding: receiver registration/type and load-order validation in E4/E6/E7.

## E4 — Label receiving ModelComponent

- State: `IMPLEMENTED`; compilation and runtime tests: `UNKNOWN`.
- Files: new `components/Transfer/Label.{h,cpp}`; registered `label.so` in DummyPlugin connector.
- Contract: receiving transfer, zero conventional inputs, one output; one internal owned LabelDefinition, mandatory editable property, persisted definition name, deterministic rebinding in check, no artificial delay.
- Ownership: internalData owns target; pointer validated using registry identity before dereference; second receiver is rejected.
- Commit SHA: backfill at next checkpoint.
- Risks: replacing a definition may invalidate external Goto/Route/Sequence references; must be tested/mitigated before ready-to-merge.

## E5–E9 — Pending

- State: `NOT_STARTED`.
- E5 Goto, E6 integration, E7 tests, E8 remote validation, E9 Draft PR/handoff.

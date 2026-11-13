---
document_type: implementation-plan
authority: technical-reference
owner: project-maintainer
last_reviewed: 2026-11-13
status: execution-authorized
tracks: AUTO-GUI-004
---

# Main GUI — Extraction-first graphical plugin preparation (E0–E5)

## 1. Decision and sequence

Maintainer instruction dated 2026-11-13 (`HUM-ARCH-006`): isolate plugin-specific code **before** completing a generalized graphical plugin API. The previous G0–G7 roadmap focused on animation/window/service categories too early and created excessive audit/validation prerequisites. This plan **reorders work**, preserving the original [architecture invariants](GUI_EXTENSIONS_ARCHITECTURE.md) and the existing `GuiExtensionPlugin`, `GuiExtensionRegistry`, `GuiExtensionManager` and `GuiExtensionPluginCatalog` contracts. It supersedes the *execution order* of [the earlier implementation plan](GUI_EXTENSIONS_IMPLEMENTATION_PLAN.md), not the architectural direction.

Precondition met: PRs #577 and #578 for ModalModel/Network GUI are merged. Working baseline at E0: `WorkInProgress` `2ad98f96311e9c13d27078f9dd83bbffafdabe5d`. Recheck HEAD/overlaps before any later independent slice; do not freeze the repository at this commit.

## 2. Pattern and folder policy

Prefer **alternative B: generic host dispatches registered contributions** to domain code. Moving `if (type == "Decide")` from `MainWindow` into a separate file *without removing the type-specific call from the host* is not the desired result. Prefer extending the existing registered contribution categories only when a real extracted behavior demands it. Existing `GuiContextMenuContribution`, `GuiDiagramProjectionContribution`, `GuiNetworkEditorContribution`, action/window/dock contributions take precedence over new virtual interfaces.

Framework stays in `source/applications/gui/genesys/extensions/`; domain-specific code in the existing `extensions/plugins/` hierarchy, reusing `efsm/`, `graph/`, `cpn/`, `markov/` and adding `biochemical/`, `decide/`, `modal/`, `discrete_processing/` or `genetic/` **only when needed**. A domain-local `*GuiIntegration` class may host extracted coherent operations; an actual `GuiExtensionPlugin` is appropriate when contributing registrations. Keep the model/kernel/parser free of Qt; statically compile GUI extensions for now.

## 3. Extraction sequence and minimal acceptance

| Stage | Functional scope | Targeted evidence |
| --- | --- | --- |
| E0 | Confirm current branch, relevant sources/contracts and integrated ModalModel/Network state | Short source list, no whole-tree audit or generated dependency graph gate |
| **E1** | BioReaction/BioSpecies context selection, reactant/product/modifier actions and scene refresh leave generic context-menu controller | Generic controller loses BioReaction/BioSpecies includes/method; registry-contributed menu; 2 focused Qt/GTest cases + GUI build |
| **E2** | Decide `Conditions` list/ports/connection/undo synchronization extracted from ObjectPropertyBrowser and GraphicalModelComponent special handling | Selection/editing/insert/remove/move/undo preserved; 1–3 focused relevant tests; no new Decide include in generic host |
| **E3** | ModalModel/Network specific context, tree drop policies and Property Editor behavior consolidated into existing EFSM/Graph/CPN/Markov or bounded modal integration | No duplicate editor framework; focused nested context/Property Editor/undo tests; keep current persistence |
| **E4** | Queue/Station animation overlays and genetic circuit rendering specifics relocated by real domain; other domain handlers only as observed | Domain includes removed from migrated generic source; focused scene-state and existing graphical semantics tests |
| **E5** | Collect observed contribution types and boundaries; consolidate only the proven missing GUI contracts | Evidence-backed API design, no forced unused virtual methods; full integration regression before merge |

**Checkpoint policy:** each behavior-sized extraction has a relevant Qt6 build, approximately 1–3 concise automated tests and one bounded interaction scenario when appropriate. Fix real failures; never equate missing evidence with PASS. Full mandatory checks and manual-impact review before integration; don't run unrelated exhaustive suites on every edit. Record evidence concisely in PR/commit notes.

## 4. First implementation — E1 (PR #579)

Implemented in [PR #579](https://github.com/rlcancian/Genesys-Simulator/pull/579) on branch `WiP202611/gui-plugin-specific-extraction`:

- Extract original `GraphicalContextMenuController::addBioReactionLinkMenu` selection and actions to `extensions/plugins/biochemical/BioReactionContextMenuGuiExtensionPlugin.cpp`.
- Use existing `GuiContextMenuContribution` static registration and existing manager dispatch.
- Preserve action wording, species relation logic, duplicate prevention, model dirty flag and scene data-definition resynchronization.
- Add focused GTest checks for registration, unrelated selection and action effects.

**Implementation ≠ validation:** first source patch committed at `3749a32fca9fbbb145c92fd7f6c5ce60657917c5`; CI build/tests were pending when plan was authored. Menu location may shift to the extension contribution section, even if actions/behavior remain equivalent. No manual Qt desktop run was claimed. Do not merge if affected GUI tests fail.

## 5. Non-goals and handoff

Not in this extraction effort: full registration/factory architecture, dynamic loading/unloading or C ABI, extra model behavior, rewriting the Property Editor, replacing GraphicalModelScene, generalized service-provider extensions, broad file moves or kernel/persistence changes.

The earlier unmerged [PR #575](https://github.com/rlcancian/Genesys-Simulator/pull/575) documents the former accelerated implementation roadmap and is *superseded as an implementation branch*, not part of this PR. Reconcile obsolete planning references during E5 rather than merging two inconsistent plans.

Small reviewable commits; draft PR until focused evidence is available; no direct edits to `WorkInProgress` and no stable branch promotion.

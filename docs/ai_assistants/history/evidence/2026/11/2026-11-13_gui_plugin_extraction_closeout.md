---
document_type: implementation-validation-evidence
authority: informative
owner: project-maintainer
tracks: HUM-ARCH-006,AUTO-GUI-004
branch: WiP202611/gui-plugin-specific-extraction
base_commit: 2ad98f96311e9c13d27078f9dd83bbffafdabe5d
status: local-and-hosted-validation-passed
---

# Graphical plugin extraction E0–E5 closeout evidence

## Scope and status

This evidence records implementation on the feature branch and local/hosted validation for PR [#579](https://github.com/rlcancian/Genesys-Simulator/pull/579), targeting `WorkInProgress`. It does not authorize or claim a merge. The source branch is `WiP202611/gui-plugin-specific-extraction`; E0–E4B implementation commits are `7f36e9c`, `c4e31ae4a`, `2bb6dabc4`, `47abb3514`, `690679ef8`, `2c5239cea`, and `b2910bfc9`. Commit `18a86f434` fixes the regression discovered by final full-suite validation; the E5 documentation commit follows.

The execution catalog and residual boundaries are maintained in [`GUI_EXTENSIONS_EXTRACTION_PLAN.md`](../../../../reference/GUI_EXTENSIONS_EXTRACTION_PLAN.md). The older graphical-plugin architecture remains future work; this change keeps static registration and adds only two contracts justified by extracted behavior.

## Implemented and locally validated

| Stage | Implementation | Evidence |
|---|---|---|
| E0 | Repaired the invalid placement of two BioReaction tests and restored the existing serializer assertion. | Focused GUI test/build passed at the E0 checkpoint. |
| E1 | Moved BioReaction/BioSpecies menu behavior behind `GuiContextMenuContribution`; active scene is resolved at dispatch. | Focused menu behavior and scene-switch tests passed; GUI build passed. |
| E2 | Moved Decide Conditions list/port reconciliation to the Decide extension integration. | Focused list, port, ordering, connection-preservation and undo/redo tests passed. |
| E3 | Moved ModalModel/Network context, tree-drop and Property Editor decisions behind registered contributions. | Focused nested context, property, undo and GUI build checks passed. |
| E4A | Moved Queue/PickStation runtime visualization callbacks and Queue animation controller into `discrete_processing`; generic scene dispatches via `GuiComponentVisualizationContribution`. | Focused visualization and affected scene tests passed. |
| E4B | Moved genetic circuit glyph selection/data into `genetic` through `GuiDataDefinitionRenderingContribution`; generic renderer retains style, paint and geometry policy. | Extension glyph test passed; renderer tests passed 11/11; affected GUI and test targets built. |
| E5 correction | The initial full-suite run caught process-component drops being rejected in ordinary Submodel context; changed the drop restriction to query `hasNetworkEditor`, not the broader editing-context capability. | Reproduced failing GUI test, fixed it, then the same isolated test passed and the full suite passed. |

Final local integration commands and results:

- `cmake --preset gui-app` — **PASS**.
- `cmake --build --preset gui-app --parallel 4` — **PASS**, including rebuild after the E5 correction.
- `cmake --preset tests-unit` — **PASS**.
- `cmake --build --preset tests-unit --parallel 4` — **PASS**, full preset build.
- `ctest --preset tests-unit --output-on-failure --output-log /tmp/genesys-tests-unit-final.log` — **PASS**, 2,121 active tests passed, 0 failed; 4 disabled tests not run.
- `ctest --test-dir build/tests-unit -R '^GuiExtensionManager\\.ProcessSubmodelStillAcceptsProcessComponentDrops$' --output-on-failure` — **PASS**, 1/1 after correction.
- Focused `genesys_test_gui_gmdd_layout` extraction tests — **PASS**, 2/2 for the newly added Queue visualization and genetic glyph tests; the full affected GMDD suite had previously passed 155/155 at the Network integration checkpoint.
- `genesys_test_gui_render_strategy` — **PASS**, 11/11.
- `git diff --check` — **PASS**.
- `python3 scripts/validate-ai-docs.py` — **PASS**, 73 active Markdown files and 25 retained historical files checked.
- Interactive desktop session — **UNKNOWN**; not performed locally.

Hosted checks on the corrected/documented PR HEAD `288c20e04ee790c8649e14c2c799fbf2978f3ce5`:

- Configure/build/CTest `tests-unit` — **PASS** (run `43528254790`).
- GUI GMDD diagnosis and Qt6/main-GUI build — **PASS** (run `43528254790`, `43528254967`).
- Kernel-unit and smoke — **PASS** (run `43528254819`).
- AI documentation governance — **PASS** (run `43528254911`).
- Integrated tools and launcher — **PASS** (run `43528254784`, `43528254843`).
- Debian package build, Lintian and package lifecycle — **PASS** (run `43528255078`).
- The earlier failure on `b2910bfc9ab2f510c8b9e773e60453cbefede2a6` was the Submodel drop regression; it is corrected in `18a86f434` and passed locally and in the final hosted unit checks.

## Contribution catalog and boundaries

| Domain | Source → destination | Contract/dispatch | Remaining generic dependency |
|---|---|---|---|
| BioReaction actions | `GraphicalContextMenuController` → biochemical plugin | `GuiContextMenuContribution` | None for the migrated actions. |
| Decide Conditions reconciliation | generic Property Editor/component coordination → Decide extension | existing list editing with bounded reconciliation dispatch | `GraphicalModelComponent` still initializes semantic Decide ports by concrete type. |
| ModalModel/Networks | MainWindow, Property Editor and plugin tree → modal and formalism extensions | existing context, network-editor and projection contributions | Static registration is intentional; future dynamic loading remains out of scope. |
| Queue/PickStation runtime visualization | generic component/scene event handlers → discrete-processing extension | `GuiComponentVisualizationContribution` | Generic scene retains a Queue drawing-tool placeholder, not the runtime overlay controller. |
| Genetic glyphs | generic data-definition renderer → genetic extension | `GuiDataDefinitionRenderingContribution` | None for the migrated glyph specialization. |

The visualization contribution carries component-specific event callbacks and the render contribution supplies glyph data; both arose from concrete extractions. No new domain superclass, dynamic-loader API, or replacement Property Editor was introduced. Generic Qt ownership, selection, scene geometry, renderer styling and static registry remain with the host.

## Final interpretation

E0–E4B implementation is present in the branch with focused local evidence. E5 GUI and unit builds, complete unit suite, correction regression, documentation governance and all hosted PR checks are PASS on the recorded HEAD. This work does not prove a manual desktop interaction session, does not remove the two documented generic host details, and does not complete the future graphical plugin architecture. Merge approval is not established by this record.

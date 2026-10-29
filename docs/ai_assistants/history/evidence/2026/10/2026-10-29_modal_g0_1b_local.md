---
document_type: evidence
authority: informative
owner: project-maintainer
date: 2026-10-29
status: partial-validation
tracks: HUM-MODAL-002,AUTO-MODAL-003
---

# Modal/Network G0.1b — local validation, 2026-10-29

## Provenance

- Repository: `rlcancian/Genesys-Simulator`.
- Branch: `WiP20261029/modalmodel-architecture`.
- PR: [#551](https://github.com/rlcancian/Genesys-Simulator/pull/551), DRAFT into `WorkInProgress`.
- Local checkout HEAD at start and throughout this session: `4965eb88926639b98ea6a59f93a4863970c213a3`
  (identical to `origin/WiP20261029/modalmodel-architecture`; fetch confirmed no new remote commits).
- Environment: local agent, Ubuntu 24.04, toolchain g++ 13.3.0, CMake 3.28.3, Ninja 1.11.1,
  Qt6 6.4.2 (`libqt6*` 6.4.2 packages), Google Test 1.14.0 (system package), ripgrep 15.2.0
  (used only from a pre-existing user-local binary — see Limitations).
- This record supplements, and does not replace, the hosted CI evidence in
  `2026-10-29_modal_g0_1b_ci.md` (run `40961067698`, source SHA `5a37c7ae`).

## 1. CMake/Ninja/CTest baselines (local, final-head `4965eb889`)

All three mandatory presets were configured and built from a clean `build/` tree.

### tests-unit

```
cmake --preset tests-unit
cmake --build --preset tests-unit --target genesys_test_gui_gmdd_layout
QT_QPA_PLATFORM=offscreen ctest --preset tests-unit -R 'GuiSubmodelLevel' --output-on-failure
cmake --build --preset tests-unit --parallel 12
LC_ALL=C LANG=C QT_QPA_PLATFORM=offscreen ctest --preset tests-unit --output-on-failure
```

- Focused build (`genesys_test_gui_gmdd_layout`): succeeded, zero errors (one pre-existing
  benign `QString::count()` deprecation warning, unrelated to this branch's changes).
- Focused `GuiSubmodelLevel.*` run: **6/6 passed** (0.27–0.30s each).
- Full build: succeeded, zero errors.
- Full `tests-unit` CTest run **under the ambient host locale (`pt_BR.UTF-8`)**: 1920 executed,
  **7 failed** — all seven failures were in `PropertyEditorDoubleCommit.*`
  (`source/tests/unit/test_gui_property_editor.cpp`), unrelated to the G0.1b `level`/
  `GraphicalModelBuilder` change set. Root cause confirmed: these tests do not force a
  deterministic locale and are sensitive to the ambient `LC_NUMERIC` (comma vs. period decimal
  separator) of the host shell; they are not exercised by the GUI submodel-level or Network
  editing-level test suites.
- Full `tests-unit` CTest run **under `LC_ALL=C LANG=C`** (matching the GitHub Actions runner's
  default locale): **1920 executed, 0 failed, 4 legacy disabled, 1924 registered** — exact match
  to the hosted CI evidence (`2026-10-29_modal_g0_1b_ci.md`, run `40961067698`).
- All four `Programmatic*BelongToNetworkEditingLevel` domain tests confirmed passed:
  `EFSMNetworkTest`, `GraphNetworkTest`, `MarkovChainNetworkTest`, `ColoredPetriNetNetworkTest`.

### tests-kernel-unit

```
cmake --preset tests-kernel-unit
cmake --build --preset tests-kernel-unit --parallel 12
LC_ALL=C LANG=C QT_QPA_PLATFORM=offscreen ctest --preset tests-kernel-unit --output-on-failure
```

Result: **1920 executed, 0 failed, 4 legacy disabled, 1924 registered** (locale `C`). Identical
inventory and result to `tests-unit`.

### tests-smoke

```
cmake --preset tests-smoke
cmake --build --preset tests-smoke --parallel 12
LC_ALL=C LANG=C ctest --preset tests-smoke --output-on-failure
```

Result: **3/3 passed** (`smoke_simulator_start`, `test_continuous_system`, `test_lsode`).

**Locale finding is recorded as a pre-existing, out-of-scope environment sensitivity**, not a
regression introduced by this branch. It is not fixed here because it is unrelated to the G0.1b
scope (level-only data-definition projection / `GraphicalModelBuilder`); a maintainer should
decide whether to open a dedicated backlog entry for forcing a deterministic locale in
`PropertyEditorDoubleCommit` fixtures that do not already do so.

## 2. `gui-app` build and bounded interaction

```
cmake --preset gui-app
cmake --build --preset gui-app --parallel 12
```

Build succeeded, zero errors (same benign deprecation warning as above). Executable:
`build/gui-app/source/applications/gui/genesys/genesys-gui`.

### 2.1 Bounded Xvfb startup/shutdown (new empty model)

Reused the exact bounded-startup pattern from
`.github/workflows/genesys-optimizer-gui-validation.yml` (Xvfb `-nolisten tcp`, PID-bound window
discovery via `xdotool search --pid`, bounded liveness wait, SIGTERM, exit-code and
residual-process checks), run in an isolated `HOME` to avoid touching the real user's GUI
preferences.

Result: window found and bound to the application PID, process remained alive for the bounded
interval, clean SIGTERM exit (143), **zero residual `genesys-gui` or `Xvfb` processes** after
teardown.

### 2.2 Loading a conventional process model

An isolated `preferences.json` (`modelAtStart=specific`) pointed the GUI at
`models/Smart_Process.gen` (a `Create -> Process (Seize/Delay/Release) -> Dispose` model with a
`Resource` and a `Queue`). Startup log showed no errors/warnings beyond the standard
license/limits banner. Screenshot evidence:
`modal-g0-1b-gui-root-model-2026-10-29.png` — the root canvas correctly renders `Create_1`,
`Process_1`, and `Resource_1`, with the SimulLang/Components/DataDefinitions tabs and Property
Editor panel present.

### 2.3 Selection and Property Editor behavior

Mouse clicks at the on-screen positions of `Process_1` and `Create_1` correctly updated the
right-hand Property Editor panel with the type-specific property tree for each
(`ModelDataDefinition`/`Process` fields for `Process_1`; `ModelDataDefinition`/`Create` fields for
`Create_1`), with no crash and no duplicated graphical items. Screenshot evidence:
`modal-g0-1b-gui-selection-property-editor-2026-10-29.png`.

### 2.4 Existing submodel (level-filtered) infrastructure regression check

Right-clicking `Process_1` opened its context menu, confirming the existing `Open Submodel`
action (`GraphicalContextMenuController`) is present and unaffected
(`modal-g0-1b-gui-context-menu-2026-10-29.png`). Clicking `Open Submodel` opened a new
breadcrumb-style tab labeled `Model 1 > Process_1` **alongside** the preserved root `Model 1`
tab, with the Property Editor still correctly bound
(`modal-g0-1b-gui-submodel-tab-2026-10-29.png`). No crash occurred and the application still
terminated cleanly (SIGTERM, exit 143, no residual processes) afterward.

This exercises the same `GraphicalModelBuilder`/level-filtered scene path that the G0.1b patch
modifies (for an ordinary process-component submodel, not yet a Network), and shows no
regression: the root context, submodel tab creation, and Property Editor binding all behave as
before.

**Limitation — not demonstrated:** no committed `.gen` sample model in this branch yet attaches a
`DefaultNetwork`/`EFSMNetwork` to a `ModalModelDefault` (Network creation/attachment is explicitly
out of scope until G2, per the implementation plan), so this session could not visually exercise
the *new* non-root, zero-process-component Network-level discovery path inside the live
`gui-app` executable — that exact path is covered instead by the six offscreen
`GuiSubmodelLevel.*` Google Tests (section 1), which construct such a context directly. Likewise,
deeper blind keyboard-driven navigation (e.g., opening the `View` menu via `Alt+V` under Xvfb
with no window manager) was attempted and did not reliably register, and is not claimed as
verified.

## 3. Developer Manual (LaTeX) build

```
cd docs/developers/ManualGenESyS_subrepo
./validate_figure_specs.sh   # exit 0
./make.sh                    # latexmk -xelatex, 4 xelatex passes + makeindex + biber
```

- `validate_figure_specs.sh`: passed (exit code 0), no missing `FIGURE-SPEC`/caption/label
  issues reported.
- `make.sh`: succeeded. `latexmk` ran `xelatex` four times (to resolve cross-references/citations
  and the index/bibliography), then `xdvipdfmx`. Final pass (`run 4`) emitted **zero** "undefined
  references" warnings (earlier passes had the normal, expected transient undefined-reference
  warnings before the `.aux`/`.bbl` files stabilized). No `LaTeX Error` or halting `!` error
  occurred; only typographic `Overfull`/`Underfull hbox` warnings (pre-existing, not related to
  the modal/GUI chapter) and `xdvipdfmx` ICC-profile/annotation-boundary warnings (pre-existing,
  unrelated to this branch).
- Output: `docs/ManualGenESyS.pdf`, 388 pages, valid PDF 1.5, 8,071,421 bytes,
  `CreationDate: Thu Oct 29 19:57:16 2026 -03`.
- Section **26.7 "Hierarchical editing contexts for model data (development branch)"** (rendered
  PDF page 242) was visually inspected (rendered to PNG via `pdftoppm`). It accurately describes:
  the `level`-as-editing-context contract; that the Network's ID (not the `ModalModelDefault`
  adapter's ID) is the context key; the `GraphicalModelBuilder::synchronizeGraphicalDataDefinitionsLayer()`
  behavior and its test coverage in `test_gui_submodel_level.cpp`; and explicitly states that
  **mathematical transitions are not yet drawn** and that the full editor/persistence/integration
  remain pending, linking to the implementation plan. No User Manual claim of a working Network
  viewer was introduced. Text is consistent with the actual current code and with the six passing
  `GuiSubmodelLevel.*` tests.

## 4. Documentation drift noted (not fixed, out of scope)

`docs/developers/ManualGenESyS_subrepo/README.md`'s "Chapter map" table lists far fewer chapter
files than currently exist in the directory (e.g. it omits `chapter_gui_architecture.tex`,
`chapter_repository_architecture.tex`, and ~30 other current `.tex` files). This predates this
session's changes and is unrelated to the G0.1b scope; recorded here per the mission's
instruction to log documentation-path divergences rather than silently expand scope to fix them.

## 5. Dependency/environment limitation

`validate_figure_specs.sh` requires `rg` (ripgrep), which was not installed as a system package
and could not be installed via `apt-get` because non-interactive `sudo` requires a password this
agent cannot supply (confirmed with the user; `!sudo apt-get install -y ripgrep` run by the user
in this same non-interactive session also failed for the same TTY/password reason). The script
was run successfully by prepending a pre-existing, already-installed `rg` 15.2.0 binary (bundled
with another already-installed local developer tool on this machine, at a user-owned path
outside the repository) to `PATH` for that one command only; no system package state was
changed, and no project file was modified to work around the dependency. A maintainer with
interactive shell access should run `sudo apt-get install -y ripgrep` to install it as a normal
system package if this workaround is not considered acceptable long-term.

## 6. Acceptance status — milestone 2 (G0.1b)

Combined with the hosted CI evidence (`2026-10-29_modal_g0_1b_ci.md`), the three remaining
milestone-2 gates recorded as outstanding there are now addressed:

1. **`gui-app` build + bounded interaction + Property Editor check** — done, section 2 above.
2. **Manual LaTeX compile + regenerated `docs/ManualGenESyS.pdf`** — done, section 3 above.
3. **Final-head local CMake/Ninja/CTest confirmation** — done, section 1 above, exact match to
   the hosted CI inventory under a matching (`C`) locale.

**Scope exclusions unchanged:** this session does not implement or test an EFSM transition
renderer, a `.gui` Network-layout codec, sanitizer runs (no lifetime/ownership change was made),
or packaging/installation. No source code was modified in this session — only evidence,
screenshots, and the regenerated manual PDF.

**Etapa 2 (G0.1b) validada localmente, aguardando revisão do coordenador.** The frozen MVP
progress remains the entry state of **1/8 = 12.5%** until the coordinator reviews and approves
this evidence; approval would raise it to **2/8 = 25%**.

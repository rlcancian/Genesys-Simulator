# Manual baseline

Date: 2026-07-23
Repository: rlcancian/Genesys-Simulator
Branch: WorkInProgress
Commit: ab34b2c9033a4869acc955e8ab710eda09598400
Manual source dir: `docs/developers/ManualGenESyS_subrepo`
Published PDF: `docs/ManualGenESyS.pdf`
Source PDF: `docs/developers/ManualGenESyS_subrepo/ManualGenESyS.pdf`

## Tree state

The repository worktree was not clean at capture time. Observed pre-existing
changes included:

- deleted legacy files under `docs/developers/ManualGenESyS_subrepo/NOT_USED/`;
- modified `docs/developers/ManualGenESyS_subrepo/capa.png`;
- modified `docs/developers/ManualGenESyS_subrepo/figs/capa.png`;
- deleted chapter-head images under `docs/developers/ManualGenESyS_subrepo/figs/`;
- untracked `docs/developers/ManualGenESyS_subrepo.zip`;
- untracked `docs/developers/ManualGenESyS_subrepo/ManualGenESyS.ptc`.

## Current book structure

### Active book entry points

- `ManualGenESyS.tex`
- `book_content.tex`
- `preface.tex`
- `chapter_genesys_overview.tex`
- `chapter_installation_and_build.tex`
- `chapter_graphical_interface_first_steps.tex`
- `chapter_example_models.tex`
- `chapter_execution_observation_simulang.tex`
- `chapter_source_organization_architecture.tex`
- `chapter_kernel_overview.tex`
- `chapter_components_model_data_plugins.tex`
- `chapter_parser_expressions_language.tex`
- `chapter_applications_tools_tests_evolution.tex`

### Chapter map from `book_content.tex`

- Preface
- What Is GenESyS
- Download, installation, and compilation
- First steps in the graphical interface
- Example models and initial use of the simulator
- GUI, Simulang, execution, and model observation
- Source Code Organization and Overall Architecture
- Simulator Kernel
- Components, model data, and plugins system
- Parser, Expressions, and Internal Language
- Applications, Tools, C++ Examples, Tests, and Project Evolution

### Figure assets present in `figs/`

- `capa.png`
- `capa1.png`
- `capa3.png`
- `capa5_en.json`
- `capa5_en.png`
- `capa6_en.json`
- `capa6_en.png`
- `chapterhead.png`
- `chapterhead_.png`
- `chapterhead_bibliografia.png`
- `chapterhead_sumario.png`
- `descrico_capa.json`
- `rascunho_marca_dagua.png`
- `tikzstyles_livro.tex`

## Build and tool versions

- `make.sh` is the build entry point for the manual.
- `latexmk --version`: 4.83
- `git --version`: 2.43.0
- `XeTeX`: 3.141592653-2.6-0.999995
- `TeX Live`: 2023/Debian
- `xdvipdfmx`: 20220710

## PDF metadata

- Title: `Title`
- Author: `Author`
- Creator: `LaTeX with hyperref`
- Producer: `xdvipdfmx (20220710)`
- CreationDate: `Wed Jul 22 22:46:34 2026 -03`
- Pages: `100`
- Page size: `419.53 x 595.28 pts`
- PDF version: `1.5`

## Current log warnings and defects

The current `ManualGenESyS.log` contains these objective issues:

- `inputenc package ignored with utf8 based engines`
- `biblatex`: deprecated `babel` option
- `biblatex`: `babel/polyglossia` detected but `csquotes` missing
- `LaTeX Warning: Unused global option(s)`
- multiple `Underfull \hbox` and `Underfull \vbox` messages
- multiple `Overfull \hbox` messages, including at line ranges:
  - `24--25`
  - `80--99`
  - `142--143`
  - `182--191`
  - `249--262`
  - `251--268`
  - `252--271`
  - `263--264`
  - `274--275`
  - `294--311`
- empty bibliography warnings:
  - `Type 'book' not found`
  - `Type 'article' not found`
  - `Empty bibliography`
- `Package caption Warning: Unused \captionsetup[sub]`

## Commands used for this baseline

```bash
git rev-parse --abbrev-ref HEAD
git rev-parse HEAD
git status --short --untracked-files=all
pdfinfo docs/developers/ManualGenESyS_subrepo/ManualGenESyS.pdf
latexmk --version
git --version
```

## Notes

- This baseline freezes the current checkpoint before any manual rewrite.
- The current PDF is preserved at both the source location and the published
  location listed above.
- The worktree must be revalidated later because the repository was already
  dirty when the baseline was captured.

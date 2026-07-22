# GenESyS Manual Source

## Purpose

This directory contains the LaTeX source tree for the combined GenESyS manual.
The book is organized in two parts:

- User Manual;
- Developer Manual.

The source is maintained here so that changes can be reviewed, rebuilt, and
kept in sync with the published PDF.

## Published artifact

The canonical published artifact is:

- `docs/ManualGenESyS.pdf`

Running the local build script from this directory must refresh that file.

## Directory structure

Current source layout:

- `ManualGenESyS.tex` - main LaTeX entry point.
- `book_content.tex` - book structure, parts, and chapter includes.
- `preface.tex` - front matter preface.
- `chapter_genesys_overview.tex` - User Manual: overview and reading guide.
- `chapter_installation_and_build.tex` - User Manual: installation and build.
- `chapter_graphical_interface_first_steps.tex` - User Manual: first GUI steps.
- `chapter_example_models.tex` - User Manual: example models.
- `chapter_execution_observation_simulang.tex` - User Manual: execution and observation.
- `chapter_source_organization_architecture.tex` - Developer Manual: repository organization.
- `chapter_kernel_overview.tex` - Developer Manual: simulator kernel.
- `chapter_components_model_data_plugins.tex` - Developer Manual: components, model data, plugins.
- `chapter_parser_expressions_language.tex` - Developer Manual: parser and internal language.
- `chapter_applications_tools_tests_evolution.tex` - Developer Manual: applications, tools, tests, and evolution.
- `capa.tex` - title-page overlay.
- `copyright.tex` - copyright and usage notice.
- `structure.tex` - shared LaTeX macros and style configuration.
- `figs/` - figures, cover assets, and related helper files.
- `NOT_USED/` - legacy retained files that are not part of the active book flow.
- `make.sh` - build script.

## Chapter map

| Source file | Book part | Chapter title | Scope |
| ----------- | --------- | ------------- | ----- |
| `preface.tex` | Front matter | Preface | Book orientation and scope |
| `chapter_genesys_overview.tex` | User Manual | What Is GenESyS | Purpose, workflow, and manual map |
| `chapter_installation_and_build.tex` | User Manual | Download, installation, and compilation | Environment setup and first build |
| `chapter_graphical_interface_first_steps.tex` | User Manual | First steps in the graphical interface | Main window, commands, and workspace |
| `chapter_example_models.tex` | User Manual | Example models and initial use of the simulator | Sample models and controlled first use |
| `chapter_execution_observation_simulang.tex` | User Manual | GUI, Simulang, execution, and model observation | Runtime observation and textual view |
| `chapter_source_organization_architecture.tex` | Developer Manual | Source Code Organization and Overall Architecture | Repository layout and reading strategy |
| `chapter_kernel_overview.tex` | Developer Manual | Simulator Kernel | Core simulation classes and execution flow |
| `chapter_components_model_data_plugins.tex` | Developer Manual | Components, model data, and plugins system | Model composition and extensibility |
| `chapter_parser_expressions_language.tex` | Developer Manual | Parser, Expressions, and Internal Language | Language, grammar, and expression semantics |
| `chapter_applications_tools_tests_evolution.tex` | Developer Manual | Applications, Tools, C++ Examples, Tests, and Project Evolution | Applications, validation, and contribution paths |

## User Manual boundary

The User Manual begins after the preface and covers the first five chapters.
It focuses on how to install, open, read, execute, and inspect GenESyS from the
user perspective.

## Developer Manual boundary

The Developer Manual begins after the User Manual part break and covers the
remaining chapters. It focuses on repository organization, kernel behavior,
model structure, parser semantics, tooling, applications, tests, and
development strategy.

## Naming policy

Chapter filenames must be semantic, ASCII, lowercase, and stable even when the
chapter order changes. The file name should describe the topic, not the ordinal.

## Language policy

The canonical manual content is written in English. Commands, identifiers,
labels, and code symbols are preserved as-is.

## Build instructions

Build the manual from this directory:

```bash
cd docs/developers/ManualGenESyS_source
./make.sh
```

The build script compiles the LaTeX sources and refreshes:

- `docs/ManualGenESyS.pdf`

## Generated files

Generated LaTeX intermediates should stay out of version control. Typical
artifacts include:

- `*.aux`
- `*.bbl`
- `*.bcf`
- `*.blg`
- `*.fdb_latexmk`
- `*.fls`
- `*.idx`
- `*.ilg`
- `*.ind`
- `*.log`
- `*.run.xml`
- `*.toc`
- `*.xdv`

## Update policy

Any material change in GenESyS must trigger a manual-impact review. If the
change affects users, update the User Manual. If it affects internals,
architecture, parser behavior, or development workflow, update the Developer
Manual. If both are affected, update both parts in the same change set or in a
document PR explicitly linked to the code change.

## Validation checklist

- compile the LaTeX book;
- verify there are no unresolved references;
- inspect warnings from the LaTeX build;
- confirm the published PDF path;
- inspect the affected pages visually;
- verify the chapter map;
- check English consistency;
- confirm that technical claims match the codebase.

## Images

The current figure directory is `figs/`. It contains the active cover, chapter
headers, and helper assets used by the book. The future automation plan for
screenshots and developer diagrams is documented in
`docs/ai_assistants/reference/manual_figure_automation_plan.md`.

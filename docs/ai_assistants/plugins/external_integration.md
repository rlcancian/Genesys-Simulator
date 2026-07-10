# External Integration Plugin Guidance

## Scope

This guide covers plugins that integrate GenESyS with external code, external runtimes, generated code, or language bridges.

Observed source areas:

- `source/plugins/components/ExternalIntegration/`
- `source/plugins/data/ExternalIntegration/`

Known sampled files:

- `CppForG.h`
- `CppForG.cpp`
- `CppCompiler.h`
- `CppCompiler.cpp`

Python-facing notes should be consolidated here unless they require a separate Python integration guide.

## Guidance

- Treat process execution, compiler invocation, generated artifacts, and runtime loading as high-risk operational behavior.
- Keep external-integration behavior isolated from ordinary simulation components.
- Validate path handling, temporary output handling, and error reporting before changing integration logic.
- Avoid broad API changes until C++ and Python-facing contracts are mapped.
- Keep packaging implications explicit, especially for Debian/PPA builds.

## Open follow-up

- Revalidate `CppForG` and `CppCompiler` against the current branch.
- Consolidate `old_PythonForG_SimulatorFacade_coverage.md` into either this guide or a dedicated `python_integration.md`.
- Identify whether external integration plugins require sandboxing, safer temp directories, or packaging-specific dependencies.

# Current Plans

## Documentation directory migration

- Date: 2026-07-03
- Branch: `WiP20261`
- Scope: migrate repository documentation from `documentation/` to `docs/`.
- Status: structural migration completed; Doxyfile wrappers added; semantic consolidation still pending.

### Target layout

- `docs/ManualGenESyS.pdf`
- `docs/ai_assistants/`
- `docs/users/`
- `docs/developers/`

After the structural migration, `docs/ManualGenESyS.pdf` is the only ordinary documentation file directly under `docs/`.

### Decisions recorded

- Remove historical `Doxyfile.bak`.
- Do not version Doxygen intermediate/generated trees under `docs/users/generated/` or `docs/developers/generated/`.
- Move historical Markdown sources temporarily to `docs/ai_assistants/oldies/` after consolidation.
- Move `TINKERCELL_context.md` to `docs/ai_assistants/oldies/`.

### Doxygen configuration policy

The historical full Doxygen configurations are preserved as `.legacy` files. The main Doxyfiles are wrappers that include the legacy configuration and override repository-relative paths for the current `docs/` layout.

Run Doxygen from the repository root:

```bash
doxygen docs/users/DoxyfileUser
doxygen docs/developers/DoxyfileDeveloper
```

### Oldies retention

`docs/ai_assistants/oldies/` is temporary. It and its contents should be removed after 2026-11-01, after relevant content has been consolidated into the main AI assistant documents.

### Pending follow-up

- Consolidate historical Markdown content from `oldies/` into stable documents under `docs/ai_assistants/`.
- Validate Doxygen generation from the repository root.
- Generate and commit Doxygen outputs under `docs/users/generated/` and `docs/developers/generated/` if that remains the desired versioning policy.
- Run CMake/Ninja/CTest validation in a local checkout.

## Doxygen generated artifacts policy revision

- Date: 2026-07-03
- Branch: `WiP20261`
- Decision: Doxygen intermediate/generated artifacts must not be versioned under `docs/users/generated/` or `docs/developers/generated/`.
- Rationale: Doxygen generates many HTML, CSS, JavaScript, image, index, LaTeX, man-page, and auxiliary files that create excessive repository noise.
- Desired build behavior: Doxygen working output should run under `build/doxygen/...`.
- Versioned documentation policy: only final PDF documentation should be versioned.
- Target PDF locations:
  - user-facing final PDF: `docs/users/GenESyS-User-Documentation.pdf`
  - developer-facing final PDF: `docs/developers/GenESyS-Developer-Documentation.pdf`
- Packaging policy: Doxygen man pages should be generated under `build/doxygen/.../man` and consumed by Debian/PPA packaging, not committed as ordinary source documentation.
- Naming policy: final documentation artifacts must not encode stale years such as 2022; repository history and file versioning already provide temporal traceability.
- Pending follow-up: adjust Debian packaging/build scripts to generate or collect man pages from the build tree.

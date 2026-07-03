# Current Plans

## Documentation directory migration

- Date: 2026-07-03
- Branch: `WiP20261`
- Scope: migrate repository documentation from `documentation/` to `docs/`.
- Status: structural migration completed; Doxygen policy corrected; semantic consolidation still pending.

### Target layout

- `docs/ManualGenESyS.pdf`
- `docs/ai_assistants/`
- `docs/users/`
- `docs/developers/`

After the structural migration, `docs/ManualGenESyS.pdf` is the only ordinary documentation file directly under `docs/`.

### Decisions recorded

- Remove historical `Doxyfile.bak`.
- Do not version Doxygen intermediate/generated trees under `docs/users/generated/` or `docs/developers/generated/`.
- Version only final PDF documentation artifacts under `docs/users/` and `docs/developers/`.
- Generate Doxygen working output under `build/doxygen/...`.
- Generate Doxygen man pages under the build tree for Debian/PPA packaging workflows.
- Move historical Markdown sources temporarily to `docs/ai_assistants/oldies/` after consolidation.
- Move `TINKERCELL_context.md` to `docs/ai_assistants/oldies/`.

### Doxygen configuration policy

The historical full Doxygen configurations are preserved as `.legacy` files. The canonical Doxyfiles are small wrappers that include the legacy configuration and override repository-relative paths for the current `docs/` layout.

Run Doxygen from the repository root:

```bash
doxygen docs/users/DoxyfileUser
doxygen docs/developers/DoxyfileDeveloper
```

Canonical final PDF locations:

- `docs/users/GenESyS-User-Documentation.pdf`
- `docs/developers/GenESyS-Developer-Documentation.pdf`

Doxygen intermediate outputs remain ignored in:

- `docs/users/generated/`
- `docs/developers/generated/`

### Oldies retention

`docs/ai_assistants/oldies/` is temporary. It and its contents should be removed after 2026-11-01, after relevant content has been consolidated into the main AI assistant documents.

### Pending follow-up

- Consolidate historical Markdown content from `oldies/` into stable documents under `docs/ai_assistants/`.
- Validate Doxygen generation from the repository root.
- Validate CMake/Ninja/CTest in a local checkout.
- Adjust Debian packaging/build scripts to generate or collect Doxygen man pages from the build tree.
- Evaluate whether Debian command man pages, if required for executables, should be maintained separately as section 1 man pages instead of relying only on Doxygen API man pages.

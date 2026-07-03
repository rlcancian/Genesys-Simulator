# Current Plans

## Documentation directory migration

- Date: 2026-07-03
- Branch: `WiP20261`
- Scope: migrate repository documentation from `documentation/` to `docs/`.
- Status: structural migration completed; semantic consolidation still pending.

### Target layout

- `docs/ManualGenESyS.pdf`
- `docs/ai_assistants/`
- `docs/users/`
- `docs/developers/`

After the structural migration, `docs/ManualGenESyS.pdf` is the only ordinary documentation file directly under `docs/`.

### Decisions recorded

- Remove historical `Doxyfile.bak`.
- Keep generated Doxygen documentation under `docs/users/generated/` and `docs/developers/generated/`.
- Move historical Markdown sources temporarily to `docs/ai_assistants/oldies/` after consolidation.
- Move `TINKERCELL_context.md` to `docs/ai_assistants/oldies/`.

### Oldies retention

`docs/ai_assistants/oldies/` is temporary. It and its contents should be removed after 2026-11-01, after relevant content has been consolidated into the main AI assistant documents.

### Pending follow-up

- Consolidate historical Markdown content from `oldies/` into stable documents under `docs/ai_assistants/`.
- Adjust Doxyfile output/input paths if needed after validating Doxygen from the repository root.
- Generate and commit Doxygen outputs under `docs/users/generated/` and `docs/developers/generated/` if that remains the desired versioning policy.
- Run CMake/Ninja/CTest validation in a local checkout.

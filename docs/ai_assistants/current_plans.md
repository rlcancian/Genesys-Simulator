# Current Plans

## Documentation directory migration

- Date: 2026-07-03
- Branch: `WiP20261`
- Scope: migrate repository documentation from `documentation/` to `docs/`.
- Status: in progress.

### Target layout

- `docs/ManualGenESyS.pdf`
- `docs/ai_assistants/`
- `docs/users/`
- `docs/developers/`

### Decisions recorded

- Remove historical `Doxyfile.bak`.
- Keep generated Doxygen documentation under `docs/users/generated/` and `docs/developers/generated/`.
- Move historical Markdown sources temporarily to `docs/ai_assistants/oldies/` after consolidation.
- Move `TINKERCELL_context.md` to `docs/ai_assistants/oldies/`.

### Oldies retention

`docs/ai_assistants/oldies/` is temporary. It and its contents should be removed after 2026-11-01, after relevant content has been consolidated into the main AI assistant documents.

### Implementation limitation

The GitHub connector can create and update text files, but binary-safe tree moves were blocked in this environment. The complete directory migration should be finished in a local checkout using `git mv`.

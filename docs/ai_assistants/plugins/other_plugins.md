# Other Plugin Guidance

## Scope

This guide covers standard and general-purpose GenESyS plugin domains not yet separated into a dedicated domain guide.

Observed component subfolders include:

- `Logic/`
- `Decisions/`
- `Grouping/`
- `InputOutput/`
- `Synchronization/`
- `MaterialHandling/`

This guide also temporarily covers generic data definitions such as queues, resources, variables, schedules, sets, labels, formulas, sequences, stations, failures, files, and storage until more specific documentation is justified.

## Guidance

- Treat these as the standard discrete-event simulation plugin base.
- Preserve compatibility with Arena/SIMAN-like modeling semantics where applicable.
- Validate load/save behavior for data definitions before changing persistence.
- Be careful with empty methods: they may be deliberate no-ops, placeholders, or incomplete implementations.
- Use the component method matrix as triage input, not as proof of current behavior.

## Open follow-up

- Revalidate the component method matrix against the current tree.
- Decide whether MaterialHandling, Synchronization, InputOutput, and Decisions deserve separate guides later.
- Create regression models for high-use standard plugins.

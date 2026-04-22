# GUI Graphical Plugin Configuration

## Objective

Allow declarative control of graphical extension plugins without recompiling:

- enable only a specific subset of extensions;
- disable specific extensions;
- define extension loading order.

This layer is generic and domain-agnostic.

## GUI Management Screen

The GUI now provides a built-in manager at:

- `Tools/Extensions/Manage Graphical Extensions...`

Capabilities:

- enable/disable each graphical extension plugin;
- define explicit order (`Order > 0` pins extension into `order`);
- edit presentation metadata (`category`, `group`, `priority`);
- reload from disk and save to `gui_extensions.json`.
- apply contribution refresh immediately after save.
- also refresh plugin catalog tree immediately after save.

## First biosimulation graphical plugins

The generic extension infrastructure now has first biosimulation consumers:

- `Tools/Extensions/Biochemical/Edit BioNetwork Membership...`
  - edits `BioNetwork` species list, reaction list, and time controls (`start`, `stop`, `step`).
- `Tools/Extensions/Biochemical/Edit BioReaction Kinetics...`
  - edits `BioReaction` reversible flag, direct/reverse constants, direct/reverse parameter names,
    direct/reverse kinetic-law expressions, and modifiers list.

Both entries are dependency-gated by model plugins:

- BioNetwork editor requires `bionetwork`;
- BioReaction editor requires `bioreaction`.

## Configuration file

Default path:

- `<AppConfigLocation>/preferences.json` sibling file named `gui_extensions.json`
- in practice: same directory used by `SystemPreferences::configFilePath()`

Override path (optional):

- environment variable `GENESYS_GUI_EXTENSIONS_CONFIG`

If the file is absent or invalid JSON, GenESyS falls back to static registration order.

## JSON schema (current)

```json
{
  "enabled": [
    "core.gui.extension",
    "gui.extensions.model.plugin.inspector",
    "gui.extensions.dependency.bionetwork"
  ],
  "disabled": [
    "gui.extensions.dependency.bionetwork"
  ],
  "order": [
    "gui.extensions.model.plugin.inspector",
    "core.gui.extension"
  ],
  "presentation": {
    "core.gui.extension": {
      "category": "Core",
      "group": "Infrastructure",
      "priority": 100
    },
    "gui.extensions.dependency.bionetwork": {
      "category": "Biochemical",
      "group": "Diagnostics",
      "priority": 10
    }
  ]
}
```

### Semantics

- `enabled` (optional): allowlist by `extensionId`. If non-empty, only listed IDs are considered.
- `disabled` (optional): explicit denylist by `extensionId`.
- `order` (optional): prioritized order by `extensionId`; unlisted extensions keep their relative registration order after ordered ones.
- `presentation` (optional): per-extension presentation metadata keyed by `extensionId`.
  - `category` (string)
  - `group` (string)
  - `priority` (integer; higher values first)

### Normalization rules

- IDs are matched case-insensitively.
- Whitespace is ignored when matching IDs.

## Resolution pipeline

1. Start from statically registered `GuiExtensionPlugin` list.
2. Apply `enabled` filter (if provided and non-empty).
3. Apply `disabled` filter.
4. Apply `order` precedence.
5. For non-pinned extensions (not explicitly listed in `order`), apply presentation sort:
   - `priority` desc
   - `category` asc
   - `group` asc
   - `displayName` asc
6. Return resolved list to `MainWindow`/`GuiExtensionManager`.

## Notes

- Dependency gating by model plugin (`requiredModelPlugins`) is still enforced at runtime by `GuiExtensionManager`.
- Declarative config controls *which graphical extension plugins are considered* and *in what order*.
- `order` has precedence: listed IDs keep explicit ordering and are not re-sorted by presentation metadata.

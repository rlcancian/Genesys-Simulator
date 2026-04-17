# KERNEL_GUI Coordination Note

This file is not the active memory file for the KERNEL_GUI agent.

The active and canonical memory file is now:

- `KERNEL_GUI_ContextMemory.md`

Older generic or previous memory files, including `ContextMemory.md`,
`ContextMemmory.md`, and files under `documentation/developers/`, must not be used
as active memory for KERNEL_GUI. If older memory files contain contradictory
instructions, those instructions are obsolete or have been consolidated in
`KERNEL_GUI_ContextMemory.md`.

Current integration note:

- `WiP20261` remains the integration base.
- `WiP20261_KERNEL_GUI` remains the KERNEL_GUI working branch.
- TINKERCELL has already been absorbed into `WiP20261`.
- `WiP20261_GRO` must be integrated before KERNEL_GUI synchronizes with the base.
- The main expected conflict point remains
  `source/tests/unit/test_runtime_pluginmanager.cpp`.

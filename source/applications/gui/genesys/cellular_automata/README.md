# Cellular Automata Viewer

This folder contains the first standalone cellular-automata tool for the Genesys Qt GUI.

Current scope:

- 2D demo built from the existing cellular-automata plugin classes
- UI selectors for rule, neighborhood, and state preset
- Forest Fire preset with four enumerated states
- mouse editing with a selectable paint state
- discrete stepping plus timer-driven auto-run
- JSON save/load for lattice size, time, and cell states

Known limitations in this first pass:

- the viewer is intentionally demo-focused, not a universal CA builder yet
- more advanced neighborhood/radius combinations remain plugin-side follow-up work

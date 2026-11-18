# Direct graphical associations — 2026-11-18

Base 58e686671, WorkInProgress. Initial tree clean; fetched origin, zero ahead/behind.
Maintainer explicitly authorized direct implementation, commit and push.

## Diagnosis and implementation

GraphicalModelBuilder::collectPropertyReferencedLayoutLinks recursed into referenced
ModelDataDefinition controls while keeping the initial origin. Thus SeizableItem
-> Set -> ElementSet members became component -> member links. This traversal was
also shared by data-definition origins. Direct ATTACHED aliases could separately
duplicate links emitted through properties or other attached aliases.

Stop link traversal at a ModelDataDefinition reference or concrete model-data list
member. Continue through inline non-model helpers (SeizableItem/QueueableItem), so
their direct references still belong to the component. Leave recursive visible-object
discovery unchanged. Deduplicate layout links by target AND connection type, using
the existing helper for both property and ATTACHED/INTERNAL maps.

Scene synchronization clears and rebuilds association lines. The sole current
production creator is the shared builder via ModelGraphicsScene's creation API;
GraphicalModelSerializer rebuilds these lines dynamically rather than persisting
logical associations as a new format. No renderer, kernel, plugin, parser, logical
persistence field or reference graph changed. No existing item is repositioned by
this patch; new-item placement retains the existing layout path.

## Tests

Added SetAssociationsStayDirectAcrossUpdatesAndExplicitRequests, using real Seize,
Release, Set and four Resources. Inspects pointer identities and connection type:
- exactly one component -> Set and Set -> each member;
- zero transitive component -> member links;
- independent direct member requests survive, repeated requests deduplicate;
- duplicate Set attachment aliases deduplicate;
- add/remove member, repeated synchronization and stable existing Set position;
- change request from Set to Resource; Queue link persists;
- hide/restore Editable and Shared filters keeps association policy.

Extended SerializerRoundTripRestoresConnectedComponentsAndSharedReference with a
Set request plus independent Resource request, graph membership/association checks
after real .gui save/load; existing Queue, SignalData identity and transfer-edge
assertions remain. Resources are constructed before Set to avoid the previously
identified, unrelated kernel forward-reference load defect.

Regression first failed on the old algorithm (transitive/duplicate counts). An
initial fixture setup used a scene convenience API requiring a PropertyEditor;
GDB identified that fixture error. Test now installs QGraphicsItems as the existing
GMDD tests do, and then reproduces the association defect itself.

Final focused Qt offscreen result: GuiGmddLayout.* 11/11 PASS (2.311s), including
recursive ATTACHED/INTERNAL paths, GUI persistence, layout and live caption tests.
GUI gui-app CMake/Ninja build PASS. Manual make.sh PASS; PDF regenerated.

## Reference model and limits

Inspected /home/rafaelcancian/Repositories/SistemaComputacional03.gui in place;
Seize_1 and Release_1 reference SetProcessadores and the file declares its four
Resources. File preserved. Its already-known kernel Set forward-reference load
problem is out of this GUI-only scope and remains: do not claim this patch repairs
that logical loading or simulation behavior. Regression uses the equivalent valid
reference graph, including real GUI persistence and reconstruction. No physical
mouse/keyboard visual session; real Qt scene edge checks provide functional evidence.

Final regression: QT_QPA_PLATFORM=offscreen ctest --preset tests-unit --parallel 4
--output-on-failure: 2156/2156 executed PASS, four pre-existing disabled, 51.43s.
Whitespace review PASS. Original reference model SHA-256 unchanged:
652595f403c0c570e07275a4b9571ce0e973f9cfa8cc107ddcac4f91e5dcd8b2.

Acceptance: PASS direct Seize/Release->Set, Set->members, absence of transitive
shortcuts, explicit Resource references, multiple requests, duplicate suppression,
dynamic changes, .gui persistence/reconstruction, existing-position preservation,
filters/other association types, affected GUI build and tests. PASS scope audit:
no simulation-semantic or logical-persistence source changed. UNKNOWN human visual
interaction and original file's successful logical loading, for the limits above.

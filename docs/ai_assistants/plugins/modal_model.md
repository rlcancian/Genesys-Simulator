# Modal Model Plugin Guidance

## Scope

This guide covers modal model plugins, including state-machine, Petri-net, node, transition, and submodel concepts.

Observed source area:

- `source/plugins/components/ModalModel/`

Known sampled files include:

- `FSMState.h`
- `PetriPlace.h`
- `DefaultNode.h`
- `Submodel.h`
- `ModalModelDefault.cpp`

## Guidance

- Treat modal models as a separate semantic domain from standard queueing/discrete-event plugins.
- Preserve state-transition semantics when changing load/save or runtime behavior.
- Validate interactions with hybrid and temporal synchronization work before changing execution semantics.
- Keep EFSM/Petri-net concerns separate from generic component UI/catalog concerns.

## Open follow-up

- Revalidate the current ModalModel class set and method coverage.
- Consolidate `old_modal_model_efsm_petrinet_plan.md` into a stable modal/hybrid guide.
- Identify minimal regression models for modal-model load/save and execution.

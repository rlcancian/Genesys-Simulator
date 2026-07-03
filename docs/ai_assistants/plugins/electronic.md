# Electronic Plugin Guidance

## Scope

This guide covers electronics-oriented plugins, especially SPICE-related components.

Observed source area:

- `source/plugins/components/ElectronicsSimulation/`

Known sampled files:

- `SPICECircuit.h`
- `SPICECircuit.cpp`

## Guidance

- Treat electronics plugins as domain-specific simulation integrations, not as generic discrete-event components.
- Check external tool/library assumptions before changing runtime behavior.
- Keep serialization and plugin metadata stable unless all model-loading call sites are checked.
- Validate behavior with a minimal electronics model when available.
- Do not mix electronics-specific behavior into generic plugin infrastructure.

## Open follow-up

- Inventory all electronics-related components and data definitions.
- Determine whether SPICE runner/data-definition support exists outside `components/ElectronicsSimulation`.
- Define validation models for SPICE/electronics plugins.

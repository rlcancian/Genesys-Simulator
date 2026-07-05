# Models and Model-Specific Generation

## Purpose

This document is the current AI-assistant guide for regenerating repository `models/` from the model-specific terminal applications under `source/applications/modelSpecific/`, and for validating those applications in a repeatable way.

## Current source of truth

- `source/applications/modelSpecific/` is the canonical source for generated `.gen` examples.
- `models/` should be treated as a generated artifact set derived from the current model-specific applications, not as an independently curated archive.
- Generated-model expectations are discovered from real `model->save("./models/...")` calls in each `.cpp`, with small overrides in `source/applications/modelSpecific/modelSpecificApps.tsv`.

## Current build flow

### Preferred preset

Use the dedicated model-specific preset:

```bash
cmake --preset genesys_modelspecific_app
cmake --build --preset genesys_modelspecific_app
```

This preset configures a dedicated build tree in:

- `build/genesys_modelspecific_app`

The selected application is controlled by:

- `GENESYS_MODELSPECIFIC_APP`
- `GENESYS_MODELSPECIFIC_APP_CLASS`
- `GENESYS_MODELSPECIFIC_APP_HEADER`

### Compatibility with legacy presets

- The legacy `terminal-model-specific` preset still works.
- Legacy `GENESYS_TERMINAL_EXAMPLE` and `GENESYS_TERMINAL_EXAMPLE_CLASS` are still accepted, but now emit a CMake deprecation message and are internally mapped to the new model-specific variables.
- The executable target is now `genesys_modelspecific_app`.
- A compatibility custom target named `genesys_terminal_application` is preserved for older build flows.

## Current CMake selection rules

The model-specific selector lives in:

- `source/applications/modelSpecific/CMakeLists.txt`

Selection rules:

- default header path is the `.h` next to the selected `.cpp`;
- default class name is the source stem;
- exceptions are declared in `modelSpecificApps.tsv`;
- explicit header/class overrides are supported for historical irregularities.

Current exceptions:

- `smarts/Logic/Smart_Parser.cpp`
  - validated executable, no `.gen` generation by design.
- `smarts/Logic/Smart_Plugin.cpp`
  - validated executable, no `.gen` generation by design.
- `smarts/ModalModel/Smart_Modalmodel.cpp`
  - class override to `Smart_ModalModel`.
- `smarts/ModalModel/Smaty_DefaultModalModel.cpp`
  - historical source-stem typo; header/class override required.

## Regeneration script

Use:

```bash
scripts/regenerate-models-from-modelspecific.sh --help
```

Supported options:

- `--help`
- `--dry-run`
- `--clean-models`
- `--keep-existing`
- `--only <relative-cpp-or-id>`
- `--list`
- `--timeout <seconds>`
- `--verbose`

Recommended full regeneration from the repository root:

```bash
scripts/regenerate-models-from-modelspecific.sh
```

Useful focused runs:

```bash
scripts/regenerate-models-from-modelspecific.sh --list
scripts/regenerate-models-from-modelspecific.sh --only smarts/ModalModel/Smaty_DefaultModalModel.cpp --keep-existing
scripts/regenerate-models-from-modelspecific.sh --only smarts/Logic/Smart_Parser.cpp --keep-existing
scripts/regenerate-models-from-modelspecific.sh --dry-run --verbose
```

### Cleanup behavior

- The script only removes regular files under `models/`.
- It preserves `models/.directory`, `models/.gitkeep`, and `models/README*`.
- It does not remove the `models/` directory itself.
- In `--only` mode, it only removes stale outputs expected from the selected application(s).

## Validation model

The current validation flow is sequential and combines:

1. configure the dedicated preset for one application;
2. build `genesys_modelspecific_app`;
3. execute it from the repository root;
4. record whether:
   - build succeeded;
   - execution terminated with exit code `0`;
   - the expected `.gen` was produced.

Logs are written under:

- `build/modelspecific-validation/logs/`

## Current repository evidence

Validation snapshot from `WiP20261` on 2026-07-05:

- discovered candidate executables: `114`
- applications with expected `.gen`: `112`
- intentional non-generators: `2`
- generated `.gen` files after regeneration run: `108`
- applications that built and terminated within the current smoke limit: `52`
- applications with validation failure in the full sweep: `62`

Full-sweep command used for bounded smoke validation:

```bash
scripts/regenerate-models-from-modelspecific.sh --timeout 3
```

Observed failure classes:

- many applications generate the `.gen` and then fail later with `SIGSEGV` (`exit 139`);
- some applications generate the `.gen` but exceed the smoke timeout;
- `arenaSmarts/Smart_EvaluatingConditionsBeforeEnteringQueue.cpp` and `teaching/OperatingSystem03.cpp` aborted with `exit 134`;
- `smarts/BiochemicalSimulation/Smart_GroColonyGrowth.cpp` and `smarts/BiochemicalSimulation/Smart_GroColonyLifecycle.cpp` do not compile because `BacteriaColony` no longer has `setInitialColonyTime`.

The script now reports generation and execution validation separately, so a model can still be accounted for when the application generated it before a later runtime failure.

## Applications currently known to not generate models

- `smarts/Logic/Smart_Parser.cpp`
- `smarts/Logic/Smart_Plugin.cpp`

These are treated as valid non-generators.

## Applications currently known to need source fixes to compile

- `smarts/BiochemicalSimulation/Smart_GroColonyGrowth.cpp`
- `smarts/BiochemicalSimulation/Smart_GroColonyLifecycle.cpp`

Current build failure:

- `BacteriaColony::setInitialColonyTime(...)` no longer exists.

## Save-path fixes applied in this branch

Small model-save corrections were required so generation lands under `./models/` with distinct names:

- `arenaExamples/AirportSecurityExample.cpp`
- `arenaExamples/AirportSecurityExampleExtended.cpp`
- `arenaSmarts/Smart_AutomaticStatisticsCollection.cpp`
- `book/Book_Cap02_Example01.cpp`
- `smarts/Logic/Smart_ParserModelFunctions.cpp`
- `smarts/ModalModel/Smart_CellularAutomata1.cpp`
- `smarts/ModalModel/Smart_Modalmodel.cpp`
- `smarts/Synchronization/Smart_SimulationControlResponse.cpp`
- `teaching/ContinuousModel.cpp`
- `teaching/Half_Adder.cpp`
- `teaching/Loja/Loja01.cpp`
- `teaching/OperatingSystem03.cpp`
- `teaching/Rectifier.cpp`

## Adding a new model-specific application

When adding a new application:

1. add the `.cpp` and `.h` under `source/applications/modelSpecific/`;
2. derive from `BaseGenesysTerminalApplication`;
3. ensure it can be selected through `source/applications/modelSpecific/main.cpp`;
4. save the generated model explicitly under `./models/<ExpectedName>.gen` if it is meant to populate `models/`;
5. if the class name or header path does not match the source stem, add an entry to `source/applications/modelSpecific/modelSpecificApps.tsv`;
6. validate with `scripts/regenerate-models-from-modelspecific.sh --only <app> --keep-existing`.

## Known risks and pending work

- Several model-specific applications still crash after model generation.
- Some applications produce long-running simulations and need a deliberate smoke-time budget.
- Some runs may emit extra side artifacts outside `models/`; these should be cleaned after validation if they are not part of the intended outputs.
- `autoloadplugins.txt` is not found beside the generated executable in the dedicated build tree, but the current static plugin insertion path still allowed the validated cases to proceed.
- CI does not yet run this sweep automatically.

## Recommended next steps

- Convert the full sweep into a CI-friendly smoke stage with a stable timeout budget.
- Triage `exit 139` cases by family instead of one by one in arbitrary order.
- Repair the two Gro/Biochemical build breakages.
- Decide whether historical non-`.gen` files should remain versioned in `models/` at all.

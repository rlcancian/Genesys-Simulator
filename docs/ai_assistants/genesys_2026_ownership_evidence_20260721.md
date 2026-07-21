# GenESyS ownership and sanitizer evidence — 2026-07-21

## 1. Scope

This document records two bounded ownership corrections integrated after the runtime lifecycle work:

- temporary plugin-completion model and helper-owned objects;
- shallow-copy hazard in `OptimizerDefaultImpl1`.

The evidence applies to the recorded GitHub Actions heads and Ubuntu 24.04 toolchain. It does not establish release readiness, optimizer maturity, or scientific correctness.

## 2. Integrated pull requests

| PR | Scope | Merge commit |
|---:|---|---|
| #483 | temporary plugin-completion Model and helper ownership | `6d6dd4edc610ec5271e12cb42a89c37f0325b6c3` |
| #485 | prohibit shallow OptimizerDefaultImpl1 copy/move | `6aca91a55beaf4ee8838c4843159ec4e06456318` |

Issues #482 and #484 were closed as completed.

## 3. Plugin-completion ownership diagnosis

`Simulator::_completePluginsFieldsAndTemplate()` created a raw temporary `Model` without inserting it into `ModelManager` and returned without destruction. The focused test calls completion twice, owns only the returned non-owning list wrappers, verifies that `ModelManager::size()` remains unchanged, and exits under ASan/LeakSanitizer.

### 3.1 Progressive sanitizer evidence

| Checkpoint | Run | Artifact | Result |
|---|---:|---:|---|
| Initial test-only path | `29829754939` | `8494840688` | 27,533 bytes in 470 allocations; exit 23 |
| Temporary Model changed to RAII | `29830108591` | `8494971207` | 2,564 bytes in 38 allocations |
| Counter/StatisticsCollector ownership fixed | `29830995059` | `8495351090` | 80 bytes in 2 allocations |
| EntityType list wrapper fixed | `29831405860` | `8495543015` | exit 0; no sanitizer markers |

### 3.2 Corrections

- temporary `Model` uses local `std::unique_ptr`;
- copied `PersistenceRecord` is destroyed before the temporary model;
- model-owned plugin instances are released when the temporary model leaves scope;
- `Counter` owns/removes/deletes its `SimulationResponseDouble`;
- `StatisticsCollector` owns/removes/deletes five responses;
- `StatisticsCollector` deletes its statistics implementation and externally supplied collector in the correct order;
- `EntityType` deletes only its auxiliary non-owning statistics-list wrapper;
- plugin metadata content, ABI, returned plugin wrappers, and persistence format remain unchanged.

### 3.3 Permanent guard

Workflow:

```text
.github/workflows/genesys-plugin-lifetime-sanitizer.yml
```

Focused executable:

```text
genesys_test_simulator_plugin_completion_lifetime
```

The workflow builds only the focused executable plus dependencies, captures stdout/stderr and exit code, and uploads evidence even when the sanitizer fails.

## 4. Optimizer shallow-copy diagnosis

`OptimizerDefaultImpl1` allocates seven `List` wrappers and deletes them manually. The class previously had implicit copy semantics, allowing shallow duplication of the owning pointers.

Current callers do not require copies. The GUI stores one backend directly as a member of `OptimizerWindow`.

### 4.1 Red checkpoint

Test-only Phase 0 run `29832684178`:

- configure passed;
- smoke passed;
- kernel aggregate compilation failed at the compile-time ownership contract;
- production header unchanged.

### 4.2 Correction

The class now explicitly deletes:

```cpp
OptimizerDefaultImpl1(const OptimizerDefaultImpl1&) = delete;
OptimizerDefaultImpl1& operator=(const OptimizerDefaultImpl1&) = delete;
OptimizerDefaultImpl1(OptimizerDefaultImpl1&&) = delete;
OptimizerDefaultImpl1& operator=(OptimizerDefaultImpl1&&) = delete;
```

No container migration, algorithm implementation, GUI behavior, interface, objective, constraint, or state-machine behavior changed.

### 4.3 Compile-time guard

Executable:

```text
genesys_test_optimizer_ownership_contract
```

The test enforces:

- default constructible;
- destructible;
- not copy constructible;
- not copy assignable;
- not move constructible;
- not move assignable.

Concrete construction/destruction remains validated by the GUI build, where `OptimizerWindow` contains the backend directly.

## 5. Final repository validation

Latest validated head before PR #485 merge:

```text
e3b884d6401337d532b6cfd5ac08fff8ee8c52df
```

### Ordinary and GUI

Run `29833758692`:

- `tests-unit` configure: passed;
- aggregate build: passed;
- CTest: passed;
- focused GUI GMDD diagnostics: passed.

### Phase 0

Run `29833758797`:

- kernel configure/build/direct runner/inventory/CTest: passed;
- smoke: passed;
- 1,721 tests registered;
- 1,717 tests executed and passed;
- 4 historical duplicate Search/Remove tests disabled;
- 0 failures;
- optimizer ownership contract registered as test #25;
- artifact `genesys-phase0-tests-kernel-unit`, ID `8496613101`.

### Focused sanitizer

Run `29833758605` remained green after the optimizer change.

## 6. Interpretation

Confirmed:

- the exercised plugin-completion path is leak-free under the focused ASan/LeakSanitizer workflow;
- helper responses/statistics exposed by that path now have explicit destruction;
- the optimizer backend cannot be shallow-copied or moved.

Not established:

- leak freedom for all simulator paths;
- thread safety;
- complete optimizer functionality;
- correctness of optimizer algorithms, because they remain largely unimplemented;
- public dynamic-plugin ABI stability;
- application/package lifecycle readiness.

## 7. Next bounded workstream

Map the current full/minimal plugin source overlap before changing any target:

1. enumerate source files compiled by `genesys_plugins_components` and `genesys_plugins_components_minimal`;
2. identify exact consumers and link order/groups;
3. identify sources also compiled elsewhere;
4. classify duplicate compilation as intentional, accidental, or transitional;
5. produce a source-to-target/link evidence document;
6. only afterward propose a small target ownership correction or pilot library split.

Do not combine this inventory with dynamic plugin ABI migration, shared-library conversion, Qt cleanup, worker security, or optimizer algorithms.

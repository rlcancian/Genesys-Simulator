# GenESyS standalone shell validation evidence — 2026-07-22

## 1. Scope

This document records the executed validation of the existing `genesys_shell` CMake preset and a deterministic non-interactive command workflow.

The evidence applies to the recorded GitHub Actions merge head and Ubuntu 24.04 toolchain. It does not validate interactive terminal editing, model loading, simulation execution, file-based plugin autoload, packaging, worker behavior, or GUI applications.

## 2. Integrated pull request

PR #495, `ci: validate standalone Genesys shell preset`, was merged into `WorkInProgress` as:

```text
abb992ec2775a64b381b3604d981d0bd18161dc6
```

The source branch `WiP20260722/shell-preset-validation` was deleted automatically after merge.

Issue #494 was closed as completed. The file-based autoload limitation is tracked separately in issue #496.

## 3. Validation workflow

Workflow:

```text
.github/workflows/genesys-shell-validation.yml
```

The workflow:

1. configures the existing `genesys_shell` preset;
2. builds with the corresponding Ninja build preset;
3. discovers the generated executable;
4. invokes four complete shell commands as separate argv entries;
5. enforces a 30-second timeout;
6. validates expected output and clean exit;
7. uploads build/runtime/toolchain evidence.

The command sequence is:

```text
facade get-name
facade get-version
plugin count
exit
```

`GenesysShell::main()` stores each argv entry as one command string, and `GenesysShell::run()` executes those commands before reading interactive input.

## 4. Executed evidence

Validated branch head:

```text
2cd35d9afc58052a5bd7b3b4103fe7385f72d3af
```

Focused shell run:

```text
29885199488
```

Ordinary CI run:

```text
29885199461
```

Both runs completed successfully.

Artifact:

```text
name: genesys-shell-validation
id: 8516303352
digest: sha256:6f4f58d2d7db4e9e8808192f7e2132a2163c96568d949506bba6756007cfb9d5
```

The focused job passed:

- checkout;
- dependency installation;
- toolchain capture;
- shell preset configuration;
- Ninja build;
- executable discovery;
- non-interactive shell execution;
- artifact publication.

## 5. Toolchain and executable

Recorded toolchain:

```text
Ubuntu 24.04
CMake 3.31.6
Ninja 1.13.2
G++ 13.3.0
C++23
```

Generated executable:

```text
build/genesys_shell/source/applications/shell/genesys_shell
```

The configure log confirmed:

- kernel enabled;
- parser enabled;
- statically aggregated plugins enabled;
- shell application enabled;
- GUI, worker, terminal examples, and automated tests disabled for this preset;
- GLPK found;
- experimental Python integration enabled with Python 3.12.3.

## 6. Runtime result

The shell process:

- printed the GenESyS startup banner;
- printed the simulator name;
- printed version `260705 (anaiera)`;
- reported `Installed plugins: 123`;
- executed `exit`;
- printed `Quiting. Bye.`;
- exited within the timeout;
- emitted no unknown-command diagnostic for the scripted commands.

Confirmed scope:

- the preset configures;
- the shell target builds;
- the generated executable starts;
- complete argv commands are accepted;
- the static plugin fallback exposes plugins;
- the scripted command path exits cleanly.

## 7. Confirmed autoload limitation

The runtime log also recorded:

```text
Could not open file "autoloadplugins.txt" (".../build/genesys_shell/source/applications/shell/autoloadplugins.txt")
```

This is explained by the current code:

- `GenesysShell::run()` requests `autoloadplugins.txt` by a relative name;
- `PluginManager::autoInsertPlugins()` resolves relative paths against `Util::RunningPath()`;
- `Util::RunningPath()` returns the executable directory;
- the validated build did not contain `autoloadplugins.txt` beside the executable;
- `WorkInProgress` does not currently expose a retrievable repository-root copy, despite stale references to that path.

The missing file did not prevent operation because plugin discovery fell back to the statically aggregated connector and exposed 123 plugins.

Therefore:

- standalone shell build/start/argv validation is green;
- file-based plugin autoload is **not validated**;
- the missing-file diagnostic is a confirmed deployment/path-contract gap, not a shell build failure.

Issue #496 records the decision boundary among:

- copy/install beside the executable;
- configured application-data directory;
- static-build discovery without file lookup;
- documented multi-location search order.

No production correction should guess this contract.

## 8. Non-claims

This evidence does not establish:

- interactive line editing or command-history correctness;
- model loading or simulation execution;
- dynamic plugin loading;
- file-based autoload correctness;
- package/install layout;
- worker or network behavior;
- GUI startup or user workflows;
- scientific or numerical validity.

## 9. Next bounded application validation

While plugin architecture issue #492 and autoload contract issue #496 await human decisions, the next independent application evidence should validate one of:

1. worker preset build and local-only startup without public exposure;
2. main Qt6 GUI build and headless startup smoke;
3. independent Data Analyser or Optimizer GUI build/startup;
4. Debian package lifecycle.

Each should remain a separate PR with focused evidence and no unrelated production change.

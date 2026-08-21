# GenESyS Debian packaging

This directory contains Linux desktop integration and the maintained Debian packaging contract used by `.github/workflows/genesys-debian-package.yml`.

## Package architecture

The Debian installation is a system-owned base runtime. It never points a global path at a user's home directory and the per-user runtime mechanism never writes package-owned files.

GenESyS is distributed as one Debian binary package:

| Binary package | Purpose |
|---|---|
| `genesys-simulator` | Complete GenESyS system base: stable Launcher and Dispatcher, administrative update policy, Qt GUI, Shell, Worker, legacy `genesys-web` compatibility command, desktop integration, and public `genesys-mcp` dispatcher entry point. |

Installing the single package installs all supported GenESyS applications at once. GUI, Shell and Worker are separate executable applications, but they are not separate Debian packages.

The public entry points are:

```text
/usr/bin/genesys-gui
/usr/bin/genesys-shell
/usr/bin/genesys-worker
/usr/bin/genesys-web
/usr/bin/genesys-mcp
```

The stable entry mechanisms and Debian-provided system fallback are:

```text
/usr/libexec/genesys/genesys-launcher
/usr/libexec/genesys/genesys-dispatch
/usr/libexec/genesys/system/bin/genesys-gui
/usr/libexec/genesys/system/bin/genesys-shell
/usr/libexec/genesys/system/bin/genesys-worker
```

`genesys-gui` invokes the Launcher and is the only public entry point allowed to perform an interactive update check. Shell, Worker, legacy Web and MCP invoke the Dispatcher and never perform update-network activity themselves.

The unified package carries `Breaks`/`Replaces` relationships for the temporary development split packages (`genesys-common`, `genesys-shell`, `genesys-worker`, `genesys-gui`, and `genesys-web`) so a developer installation produced by the earlier WorkInProgress packaging can transition to the single-package layout without file-ownership conflicts. Those names are no longer produced as binary packages.

## Administrative update policy

The package installs `/etc/genesys/update.conf` as the system policy. The packaged policy is deliberately fail-closed for remote updates:

- remote updates are disabled;
- no manifest URL is invented;
- signature verification remains required;
- no public key is installed until the maintainer provisions an approved update key;
- per-user runtime selection remains allowed, so a valid runtime can be provisioned and selected without mutating Debian-owned files;
- system fallback remains enabled.

The system policy has precedence over user configuration in the current Launcher implementation.

## MCP status

`source/applications/mcp/` is a Python application with the `genesys-mcp` entry point and currently declares Python dependencies that are not part of this Debian build contract. The Debian build does not download from PyPI and does not rewrite MCP in C++.

The unified package therefore installs the public `genesys-mcp` Dispatcher wrapper but no system MCP fallback executable. A valid per-user runtime may provide `bin/genesys-mcp`; otherwise the Dispatcher returns the controlled missing-MCP diagnostic implemented by the Launcher application.

## Building and validation

The maintained GitHub Actions workflow is:

```text
.github/workflows/genesys-debian-package.yml
```

It runs on Ubuntu 24.04 and separates package creation from lifecycle validation. The build job creates exactly one `genesys-simulator_*.deb`, checks AppStream metadata, runs Lintian with errors as failures, records package metadata/content and uploads the package. The lifecycle job installs that exact artifact on a fresh runner and validates:

- package contents and ownership by the single `genesys-simulator` package;
- public wrappers and internal paths;
- system fallback through the public commands;
- a bounded GUI startup under Xvfb with updates disabled;
- the bounded Worker `/health` path on an ephemeral port;
- per-user runtime selection with local fake executables;
- system-policy override of user runtime selection;
- malformed/missing user-runtime fallback cases;
- package-file integrity after user-runtime operations;
- reinstall and conffile behavior;
- remove/purge behavior and preservation of user runtime data.

No lifecycle test downloads a GenESyS runtime from GitHub Releases.

For maintainers working from a local clone with an authenticated GitHub CLI, `scripts/run-debian-package-workflow.sh [branch] [output-directory]` dispatches this dedicated workflow and downloads its artifacts. The helper does not rewrite workflow YAML and does not switch to a historical development branch.

### Local build

From the repository root on Ubuntu 24.04, after installing the declared build dependencies:

```text
dpkg-buildpackage -us -uc -b
```

The expected Debian binary output is exactly one package named `genesys-simulator_<version>_<architecture>.deb` in the parent directory of the repository. Inspect it with `dpkg-deb --info` and `dpkg-deb --contents`, and run Lintian before installation.

## Artifacts versus distribution

`genesys-debian-packages` is a GitHub Actions artifact containing the single validation `.deb` plus build metadata. `genesys-debian-diagnostics` contains build/Lintian/AppStream logs and package metadata. `genesys-debian-lifecycle-evidence` contains the full lifecycle validation log and evidence described above. All three are ordinary workflow-run artifacts, not an official GitHub Release and not an APT/PPA repository.

To obtain them from a specific run:

```text
gh run download <run-id> --repo rlcancian/Genesys-Simulator --name genesys-debian-packages --dir ./out
```

or download them from the run's page under **Actions -> GenESyS Debian Package -> <run> -> Artifacts** in the GitHub web UI. Artifacts expire after GitHub's default retention period; they are not a permanent distribution channel.

Official package signing, a runtime signing key, runtime Release bundles, GitHub Release publication, and APT/PPA publication are separate release activities.

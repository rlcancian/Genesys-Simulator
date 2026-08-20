# GenESyS Debian packaging

This directory contains Linux desktop integration and the maintained Debian packaging contract used by `.github/workflows/genesys-debian-package.yml`.

## Package architecture

The Debian installation is a system-owned base runtime. It never points a global path at a user's home directory and the per-user runtime mechanism never writes package-owned files.

| Binary package | Purpose |
|---|---|
| `genesys-common` | Stable `genesys-launcher`, non-interactive `genesys-dispatch`, `/etc/genesys/update.conf`, and the `genesys-mcp` dispatcher entry point. |
| `genesys-shell` | Public `genesys-shell` wrapper and system fallback shell. |
| `genesys-worker` | Public `genesys-worker` wrapper, legacy `genesys-web` command, and system fallback worker. |
| `genesys-gui` | Public interactive `genesys-gui` wrapper, system fallback GUI, desktop entry, AppStream metadata and icon. It depends on Shell and Worker so `apt install genesys-gui` provides the supported base application set. |
| `genesys-web` | Transitional package depending on `genesys-worker`; it owns no executable. |

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

The package therefore installs the public `genesys-mcp` Dispatcher wrapper but no system MCP fallback executable. A valid per-user runtime may provide `bin/genesys-mcp`; otherwise the Dispatcher returns the controlled missing-MCP diagnostic implemented by the Launcher application.

## Building and validation

The maintained GitHub Actions workflow is:

```text
.github/workflows/genesys-debian-package.yml
```

It runs on Ubuntu 24.04 and separates package creation from lifecycle validation. The build job creates the binary packages, checks AppStream metadata, runs Lintian with errors as failures, records package metadata/content and uploads the `.deb` files. The lifecycle job installs those exact artifacts on a fresh runner and validates:

- package contents and ownership;
- public wrappers and internal paths;
- system fallback through the public commands;
- a bounded GUI startup under Xvfb with updates disabled;
- the bounded Worker `/health` path on an ephemeral port;
- per-user runtime selection with local fake executables;
- system-policy override of user runtime selection;
- malformed/missing user-runtime fallback cases;
- package-file integrity after user-runtime operations;
- reinstall behavior;
- remove/purge behavior and preservation of user runtime data.

No lifecycle test downloads a GenESyS runtime from GitHub Releases.

## Artifacts versus distribution

`genesys-debian-packages` is a GitHub Actions artifact containing validation `.deb` files. It is suitable for inspection and manual testing, but it is not an official GitHub Release and it is not an APT/PPA repository.

Official package signing, a runtime signing key, runtime Release bundles, GitHub Release publication, and APT/PPA publication are separate release activities.

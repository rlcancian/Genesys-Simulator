# GenESyS portable Linux distribution

This directory builds an offline Linux x86_64 distribution for a normal user
on an Ubuntu 24.04-compatible system. It is deliberately separate from the
Debian package and from the Launcher remote-update bundle format.

## Build

From the repository root:

```bash
./packaging/installer/build-portable.sh
```

The default snapshot version is `YYYY.MM.DD.<repository-commit-count>`. Use
`--version 2026.10.15` to provide an explicit numeric snapshot identifier.
The full Git commit remains in `PORTABLE-MANIFEST.json`; snapshot versions are
not official project releases and are not a replacement for project version
governance.

Artifacts are written to `packaging/installer/dist/` and are ignored by Git:

- `genesys-<version>-linux-x86_64-portable.tar.gz`;
- the adjacent `.sha256` file;
- optional staging output when `--keep-staging` is used.

The builder uses one dedicated CMake/Ninja tree, C++23, Qt6, static GenESyS
plugins, Python integration disabled, and all mandatory applications plus the
currently implemented independent GUIs. It copies the direct/transitive ELF
runtime closure and Qt platform/image/print plugins without bundling glibc,
the dynamic linker, or vendor graphics drivers.

## Install and use

```bash
tar -xzf genesys-<version>-linux-x86_64-portable.tar.gz
cd genesys-<version>-linux-x86_64-portable
./install.sh
export PATH="$HOME/.local/opt/genesys/bin:$PATH"
genesys-gui
genesys-shell
```

Use `--prefix "$HOME/.local/opt/genesys"` for a custom bootstrap location,
`--no-desktop` to skip the per-user desktop entry, and `--dry-run` to inspect
the destination. Installation verifies the package SHA-256 manifest and uses
staging plus activation with rollback on failure. It rejects root, unsafe
prefixes, non-owned existing installations and network activity.

The prefix contains the stable bootstrap and portable fallback runtime. The
Launcher-managed runtime remains under `$XDG_DATA_HOME/genesys/apps/` with
`$XDG_DATA_HOME/genesys/current`, exactly as the existing XDG contract defines.
Remote updates remain disabled; signature verification remains fail-closed.

Public commands are `genesys-gui`, `genesys-shell`, `genesys-worker`,
`genesys-web`, `genesys-mcp`, `genesys-httpworker-gui`,
`genesys-dataanalyser-gui`, `genesys-optimizer-gui` and
`genesys-ai-assistant-gui`. The Worker is never started by the installer and
is not exposed as a service. `genesys-mcp` is a controlled unavailable-service
diagnostic, not a functional MCP server.

The current shell and runtime plugin contract still looks for
`autoloadplugins.txt` relative to the executable/runtime working location.
This package intentionally does not redefine that global contract or migrate
static plugins to shared libraries; the built-in static plugin set remains
available, while external autoload lists require separate user provisioning.

## Update, rollback and removal

Install a later archive into the same prefix to perform an atomic local
replacement. The installer keeps a prior prefix until activation succeeds;
the Launcher separately retains active and previous XDG runtimes. A failed
activation restores the previous prefix. For a simple removal:

```bash
./uninstall.sh --prefix "$HOME/.local/opt/genesys"
```

Only the marked installation prefix and a desktop entry created by this
installer are removed. Models, results, configuration and XDG Launcher data
are preserved.

## Tests and compatibility

```bash
./packaging/installer/test-portable.sh packaging/installer/dist/genesys-<version>-linux-x86_64-portable.tar.gz
```

The test uses a temporary HOME and a path containing spaces, clears inherited
Qt/library variables, reinstalls, exercises the shell with a packaged model,
starts the GUI under Xvfb, starts and stops the Worker on an ephemeral port,
and uninstalls. A timeout of the GUI test is intentional evidence that the
event loop stayed alive; a startup error is a failure.

The initial compatibility target is Linux x86_64 with a glibc/libstdc++ and
graphics stack at least compatible with the Ubuntu 24.04 build host. The
destination still needs a compatible Linux kernel, glibc, libstdc++, X11/XCB
or Wayland display stack, OpenGL/EGL/ Mesa or vendor driver, and ordinary
filesystem execution permission. `noexec` mounts, older glibc/libstdc++, or
missing display libraries can prevent execution. No claim is made for all
Linux distributions or for physical UFSC laboratory machines until T16 is
executed there.

## Security and provenance

The archive checksum establishes integrity only. It is not an authenticated
signature. No trust key is generated or embedded. The package contains no
credentials, `.env` files or remote installer scripts. Extraction is performed
by `tar`; the installer validates the generated file manifest and never runs
commands from package metadata.

The runtime format inside `libexec/genesys/system/` contains regular executable
files only, `VERSION` and `MANIFEST.json`. The external `.tar.gz` is not passed
to the Launcher `tar.zst` updater and does not alter its remote protocol.

## Manual and laboratory procedure

This portable distribution changes user-visible installation and developer
packaging workflow. Before integration, review the User Manual installation
chapter and Developer Manual packaging/CI chapter under the governed manual
workflow, including PDF regeneration if applicable.

For UFSC validation, record `uname -a`, `/etc/os-release`, architecture,
`ldd --version`, `g++ --version` or installed libstdc++ package, HOME ownership,
mount options (`findmnt -T "$HOME"`), display backend, XCB/Wayland/OpenGL
availability, install paths, SHA-256, GUI liveness, shell model load, Worker
loopback health and uninstall preservation. A development-host result alone
is not T16 PASS.
